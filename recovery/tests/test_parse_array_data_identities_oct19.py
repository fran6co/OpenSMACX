"""Start-only identities for the existing parse-family arrays."""
from __future__ import annotations

import hashlib
import os
from pathlib import Path

import pytest

from capstone import CS_ARCH_X86, CS_MODE_32, Cs
from capstone.x86_const import X86_OP_MEM
from match_recovery.model import Span
from match_recovery.object_source import parse_objects, read_objects
from match_recovery.pe_coff import PeImage


ROOT = Path(__file__).resolve().parents[2]
PIN = "01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf"
_PIN_CANDIDATES = (
    ROOT / ".opensmacx/game/terranx_original.exe",
    ROOT.parents[2] / ".opensmacx/game/terranx_original.exe",
)
IMAGE = Path(os.environ["OPENSMACX_IMAGE"]) if "OPENSMACX_IMAGE" in os.environ else next(
    (candidate for candidate in _PIN_CANDIDATES if candidate.is_file()),
    _PIN_CANDIDATES[0],
)
SOURCE_PATH = ROOT / "src/general.cpp"
EXPECTED = {
    "?ParseStrPlurality@@3PAHA": (0x009BB570, "int ParseStrPlurality[10];"),
    "?ParseStrGender@@3PAHA": (0x009BB5C0, "int ParseStrGender[10];"),
    "?ParseStrBuffer@@3PAUchar256@@A": (0x009BB5E8, "char256 ParseStrBuffer[10];"),
}
OTHER_EXISTING = "// RECOVERY_OBJECT: v2 0x009BB598 0x009BB5C0 ?ParseNumTable@@3PAHA 0x00625E30"


def _bindings(source: str):
    return parse_objects(source, "src/general.cpp", PIN, address_space="pe:va")


def _require_bindings(source: str) -> None:
    rows = _bindings(source)
    expected_rows = [row for row in rows if row.symbol in EXPECTED]
    assert len(expected_rows) == 3
    assert len({row.symbol for row in expected_rows}) == 3
    selected = {row.symbol: row for row in expected_rows}
    assert set(selected) == set(EXPECTED)
    for symbol, (address, declaration) in EXPECTED.items():
        row = selected[symbol]
        assert row.origin.address == address
        assert row.origin.image_sha256 == PIN
        assert row.unit == "src/general.cpp"
        assert row.span is None and row.evidence is None
        assert row.parent_symbol is None and row.target_symbol is None
        assert declaration in source
    assert sum(row.symbol == "?ParseNumTable@@3PAHA" for row in rows) == 1
    assert OTHER_EXISTING in source


def test_actual_source_binds_only_the_three_existing_array_starts():
    source = SOURCE_PATH.read_text(encoding="utf-8")
    _require_bindings(source)
    rows = read_objects(ROOT, PIN)
    expected_rows = [row for row in rows if row.symbol in EXPECTED]
    assert len(expected_rows) == 3
    assert len({row.symbol for row in expected_rows}) == 3
    assert {row.symbol: row.unit for row in expected_rows} == {
        symbol: "src/general.cpp" for symbol in EXPECTED
    }


@pytest.mark.parametrize("symbol", tuple(EXPECTED))
def test_missing_or_wrong_start_binding_is_rejected(symbol):
    source = SOURCE_PATH.read_text(encoding="utf-8")
    address, _ = EXPECTED[symbol]
    marker = f"// RECOVERY_OBJECT: v1 0x{address:08X} {symbol}"
    assert source.count(marker) == 1

    missing = source.replace(marker + "\n", "", 1)
    with pytest.raises(AssertionError):
        _require_bindings(missing)

    wrong_address = address + 4
    wrong = source.replace(marker, f"// RECOVERY_OBJECT: v1 0x{wrong_address:08X} {symbol}", 1)
    with pytest.raises(AssertionError):
        _require_bindings(wrong)


def test_duplicate_expected_binding_is_rejected():
    source = SOURCE_PATH.read_text(encoding="utf-8")
    marker = "// RECOVERY_OBJECT: v1 0x009BB5C0 ?ParseStrGender@@3PAHA"
    assert source.count(marker) == 1
    duplicate = source.replace(marker, marker + "\n" + marker, 1)
    with pytest.raises((AssertionError, ValueError)):
        _require_bindings(duplicate)


def _decode(image: PeImage, start: int, end: int):
    decoder = Cs(CS_ARCH_X86, CS_MODE_32)
    decoder.detail = True
    decoded = list(decoder.disasm(image.read(Span(start, end)), start))
    cursor = start
    for instruction in decoded:
        assert instruction.address == cursor, "noncontiguous pinned function decode"
        cursor += instruction.size
    assert cursor == end, "incomplete pinned function decode"
    return {instruction.address: instruction for instruction in decoded}


def _read_pinned_bytes(raw: bytes) -> PeImage:
    if hashlib.sha256(raw).hexdigest() != PIN:
        pytest.fail("pinned original image SHA-256 does not match the task pin")
    return PeImage(raw, PIN)


def _read_pinned_image(path: Path) -> PeImage:
    if not path.is_file():
        pytest.fail(f"configured pinned original image is missing: {path}")
    return _read_pinned_bytes(path.read_bytes())


def test_missing_or_corrupted_pinned_image_refuses(tmp_path: Path):
    with pytest.raises(pytest.fail.Exception):
        _read_pinned_image(tmp_path / "missing-parse-array-pin.exe")
    with pytest.raises(pytest.fail.Exception):
        _read_pinned_bytes(b"not the pinned executable")


def test_pinned_parse_array_operands_keep_original_addresses_and_access_widths():
    image = _read_pinned_image(IMAGE)
    parse_say = _decode(image, 0x00625E50, 0x00625EB3)
    parse_says = _decode(image, 0x00625EC0, 0x00625F1A)

    def fact(instructions, address: int) -> tuple[str, str]:
        instruction = instructions[address]
        return instruction.mnemonic, instruction.op_str

    assert fact(parse_say, 0x00625E54) == ("cmp", "eax, 9")
    assert fact(parse_say, 0x00625E57) == ("jle", "0x625e5f")
    expected_stores = {
        0x00625E6D: ("mov", "dword ptr [eax*4 + 0x9bb5c0], ecx"),
        0x00625E82: ("mov", "dword ptr [eax*4 + 0x9bb570], ecx"),
    }
    for address, expected in expected_stores.items():
        instruction = parse_say[address]
        assert (instruction.mnemonic, instruction.op_str) == expected
        memory = [operand for operand in instruction.operands if operand.type == X86_OP_MEM]
        assert len(memory) == 1 and memory[0].size == 4

    assert fact(parse_say, 0x00625E8A) == ("shl", "eax, 8")
    assert fact(parse_say, 0x00625E8D) == ("mov", "ecx, 0x9b90d8")
    assert fact(parse_say, 0x00625E92) == ("lea", "esi, [eax + 0x9bb5e8]")
    assert fact(parse_say, 0x00625E9D) == ("mov", "byte ptr [esi], 0")
    byte_store = [operand for operand in parse_say[0x00625E9D].operands
                  if operand.type == X86_OP_MEM]
    assert len(byte_store) == 1 and byte_store[0].size == 1
    assert fact(parse_say, 0x00625EA0) == ("call", "0x6169a0")
    assert fact(parse_say, 0x00625EA7) == ("call", "0x645470")

    # Independent parser consumer, with its own pointer-null guard, repeats
    # the same gender/plurality tables and the 0x100-byte text-entry stride.
    assert fact(parse_says, 0x00625EC6) == ("je", "0x625f14")
    assert fact(parse_says, 0x00625EDF) == expected_stores[0x00625E6D]
    assert fact(parse_says, 0x00625EF4) == expected_stores[0x00625E82]
    assert fact(parse_says, 0x00625EFC) == ("shl", "eax, 8")
    assert fact(parse_says, 0x00625EFF) == ("lea", "eax, [eax + 0x9bb5e8]")
    assert fact(parse_says, 0x00625F06) == ("mov", "byte ptr [eax], 0")
    assert fact(parse_says, 0x00625F09) == ("call", "0x645470")
