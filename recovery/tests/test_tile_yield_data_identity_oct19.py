"""Source identity and pinned operand controls for TileYieldRestricted."""
from __future__ import annotations

import hashlib
import os
from pathlib import Path

import pytest
from capstone import CS_ARCH_X86, CS_MODE_32, Cs
from capstone.x86_const import X86_OP_IMM, X86_OP_MEM

from match_recovery.model import Span
from match_recovery.object_source import parse_objects, read_objects
from match_recovery.pe_coff import PeImage


ROOT = Path(__file__).resolve().parents[2]
PIN = "01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf"
SYMBOL = "?TileYieldRestricted@@3HA"
ADDRESS = 0x0090E998
SOURCE = ROOT / "src/base.cpp"
MARKER = f"// RECOVERY_OBJECT: v1 0x{ADDRESS:08X} {SYMBOL}"
FARM_STATUS_MARKER = (
    "// RECOVERY_OBJECT: v2 0x0090E99C 0x0090EA00 "
    "?FarmStatus@@3PAIA 0x004E68B0"
)


def _source_bindings(source: str):
    return parse_objects(source, "src/base.cpp", PIN, address_space="pe:va")


def _require_start_binding(source: str) -> None:
    rows = _source_bindings(source)
    matched = [row for row in rows if row.symbol == SYMBOL]
    assert len(matched) == 1
    row, = matched
    assert row.origin.address == ADDRESS
    assert row.origin.image_sha256 == PIN
    assert row.unit == "src/base.cpp"
    assert row.span is None and row.evidence is None
    assert row.parent_symbol is None and row.target_symbol is None
    lines = source.splitlines()
    assert lines[row.line].strip() == "int TileYieldRestricted;  // 0x0090E998"
    assert FARM_STATUS_MARKER in source


def test_actual_source_has_one_start_only_binding_and_global_source_identity():
    source = SOURCE.read_text(encoding="utf-8")
    _require_start_binding(source)

    rows = read_objects(ROOT, PIN)
    matched = [row for row in rows if row.symbol == SYMBOL]
    assert len(matched) == 1
    row, = matched
    assert row.origin.address == ADDRESS
    assert row.unit == "src/base.cpp"
    assert row.span is None and row.evidence is None
    farm_status = [row for row in rows if row.symbol == "?FarmStatus@@3PAIA"]
    assert len(farm_status) == 1
    assert farm_status[0].origin.address == 0x0090E99C


def test_missing_wrong_and_duplicate_tile_yield_bindings_are_rejected():
    source = SOURCE.read_text(encoding="utf-8")
    assert source.count(MARKER) == 1

    missing = source.replace(MARKER + "\n", "", 1)
    with pytest.raises(AssertionError):
        _require_start_binding(missing)

    wrong = source.replace(
        MARKER, f"// RECOVERY_OBJECT: v1 0x{ADDRESS + 4:08X} {SYMBOL}", 1
    )
    with pytest.raises(AssertionError):
        _require_start_binding(wrong)

    duplicate = source.replace(MARKER, MARKER + "\n" + MARKER, 1)
    with pytest.raises((AssertionError, ValueError)):
        _require_start_binding(duplicate)


def _image_path() -> Path:
    configured = os.environ.get("OPENSMACX_IMAGE")
    if configured:
        path = Path(configured)
        if not path.is_absolute() or not path.is_file():
            pytest.fail(f"configured OPENSMACX_IMAGE is missing or not absolute: {path}")
        return path
    candidates = (
        ROOT / ".opensmacx/game/terranx_original.exe",
        ROOT.parents[2] / ".opensmacx/game/terranx_original.exe",
    )
    return next((candidate for candidate in candidates if candidate.is_file()), candidates[0])


def _pinned_image_bytes(raw: bytes) -> PeImage:
    if hashlib.sha256(raw).hexdigest() != PIN:
        pytest.fail("pinned original image SHA-256 does not match the task pin")
    return PeImage(raw, PIN)


def _pinned_image(path: Path) -> PeImage:
    if not path.is_file():
        pytest.fail(f"pinned original image is missing: {path}")
    return _pinned_image_bytes(path.read_bytes())


def test_missing_or_corrupt_pinned_image_refuses(tmp_path: Path, monkeypatch):
    monkeypatch.setenv("OPENSMACX_IMAGE", str(tmp_path / "missing-configured.exe"))
    with pytest.raises(pytest.fail.Exception, match="configured OPENSMACX_IMAGE"):
        _image_path()
    with pytest.raises(pytest.fail.Exception, match="missing"):
        _pinned_image(tmp_path / "missing-original.exe")
    with pytest.raises(pytest.fail.Exception, match="SHA-256"):
        _pinned_image_bytes(b"not the pinned executable")


def test_default_pin_search_prefers_the_checkout_image(tmp_path: Path, monkeypatch):
    monkeypatch.delenv("OPENSMACX_IMAGE", raising=False)
    checkout = tmp_path / "checkout"
    image = checkout / ".opensmacx/game/terranx_original.exe"
    image.parent.mkdir(parents=True)
    image.write_bytes(b"test fixture")
    monkeypatch.setattr(__import__(__name__), "ROOT", checkout)
    assert _image_path() == image


def _decode(image: PeImage, start: int, end: int):
    decoder = Cs(CS_ARCH_X86, CS_MODE_32)
    decoder.detail = True
    instructions = list(decoder.disasm(image.read(Span(start, end)), start))
    cursor = start
    for instruction in instructions:
        assert instruction.address == cursor, "noncontiguous pinned code decode"
        cursor += instruction.size
    assert cursor == end, "pinned code span did not decode completely"
    return {instruction.address: instruction for instruction in instructions}


def _absolute_zero_store(instruction) -> None:
    assert instruction.mnemonic == "mov"
    assert len(instruction.operands) == 2
    memory, value = instruction.operands
    assert memory.type == X86_OP_MEM and memory.size == 4
    assert memory.mem.base == 0 and memory.mem.index == 0
    assert memory.mem.disp == ADDRESS
    assert value.type == X86_OP_IMM and value.imm == 0


def test_pinned_clear_entries_and_independent_resource_yield_store():
    image = _pinned_image(_image_path())
    tiny_clear = _decode(image, 0x004E7DB0, 0x004E7DBD)
    resource_yield = _decode(image, 0x004E7DC0, 0x004E7E34)

    _absolute_zero_store(tiny_clear[0x004E7DB0])
    assert (tiny_clear[0x004E7DBA].mnemonic,
            tiny_clear[0x004E7DBA].op_str) == ("xor", "eax, eax")
    assert tiny_clear[0x004E7DBC].mnemonic == "ret"
    _absolute_zero_store(resource_yield[0x004E7E26])

    # This is only the pinned image's pre-CRT initial value, not a claim about
    # runtime state or a complete allocation extent.
    assert image.read_initial(Span(ADDRESS, ADDRESS + 4)) == b"\0\0\0\0"
