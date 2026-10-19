"""Start-only identities for four shared scalar cells with pinned witnesses."""
from __future__ import annotations

import hashlib
import os
from pathlib import Path

import pytest

from capstone import CS_ARCH_X86, CS_MODE_32, Cs
from capstone.x86_const import X86_OP_MEM
from match_recovery.model import Span
from match_recovery.object_source import parse_objects
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
EXPECTED = {
    "?WorldClimateSkipTerrainClear@@3IA": (0x0068F21C, "src/map.cpp", "uint32_t WorldClimateSkipTerrainClear = 1;"),
    "?MsgStatus@@3IA": (0x009B7B9C, "src/win.cpp", "uint32_t MsgStatus;"),
    "?ExitTurnLoop@@3HA": (0x009B2068, "src/game.cpp", "int ExitTurnLoop;"),
    "?UnkBitfield1@@3IA": (0x0090D91C, "src/game.cpp", "uint32_t UnkBitfield1;"),
}
PINNED = {
    # timer_daemon gates climate dispatch on the original dword.
    0x00459263: ("mov", "eax, dword ptr [0x68f21c]", 4),
    # Task setup clears MsgStatus and flush_timer reads/updates the same dword.
    0x005FC9F4: ("xor", "edx, edx", None),
    0x005FC9F8: ("mov", "dword ptr [0x9b7b9c], edx", 4),
    0x005FD370: ("mov", "ecx, dword ptr [0x9b7b9c]", 4),
    0x005FD37D: ("or", "ecx, 0x3f", None),
    0x005FD396: ("mov", "dword ptr [0x9b7b9c], ecx", 4),
    # System-close sets ExitTurnLoop; diplomacy waits test its dword value.
    0x0051D7C0: ("mov", "dword ptr [0x9b2068], 1", 4),
    0x00539855: ("mov", "eax, dword ptr [0x9b2068]", 4),
    0x005398E9: ("mov", "eax, dword ptr [0x9b2068]", 4),
    # climate_set and set_dirty set independent low bits in the same dword.
    0x00591AC0: ("mov", "eax, dword ptr [0x90d91c]", 4),
    0x00591AC5: ("or", "al, 1", None),
    0x00591AC7: ("mov", "dword ptr [0x90d91c], eax", 4),
    0x00592AC9: ("mov", "eax, dword ptr [0x90d91c]", 4),
    0x00592ACF: ("or", "al, 4", None),
    0x00592AD2: ("mov", "dword ptr [0x90d91c], eax", 4),
}


def _bindings(text: str, path: str):
    return parse_objects(text, path, PIN, address_space="pe:va")


def _require_bindings(source_texts: dict[str, str]) -> None:
    rows = tuple(row for path, text in source_texts.items() for row in _bindings(text, path))
    expected_rows = [row for row in rows if row.symbol in EXPECTED]
    assert len(expected_rows) == 4
    assert len({row.symbol for row in expected_rows}) == 4
    selected = {row.symbol: row for row in expected_rows}
    assert set(selected) == set(EXPECTED)
    for symbol, (address, path, declaration) in EXPECTED.items():
        row = selected[symbol]
        assert row.origin.address == address
        assert row.origin.image_sha256 == PIN
        assert row.unit == path
        assert row.span is None and row.evidence is None
        assert row.parent_symbol is None and row.target_symbol is None
        assert declaration in source_texts[path]


def test_actual_source_binds_only_the_four_existing_scalar_starts():
    source_texts = {
        path: (ROOT / path).read_text(encoding="utf-8")
        for path in {value[1] for value in EXPECTED.values()}
    }
    _require_bindings(source_texts)


@pytest.mark.parametrize(
    ("symbol", "old_address", "new_address"),
    [
        ("?WorldClimateSkipTerrainClear@@3IA", "0x0068F21C", "0x0068F218"),
        ("?MsgStatus@@3IA", "0x009B7B9C", "0x009B7B98"),
        ("?ExitTurnLoop@@3HA", "0x009B2068", "0x009B2064"),
        ("?UnkBitfield1@@3IA", "0x0090D91C", "0x0090D918"),
    ],
)
def test_missing_or_wrong_start_binding_is_rejected(symbol, old_address, new_address):
    source_texts = {
        path: (ROOT / path).read_text(encoding="utf-8")
        for path in {value[1] for value in EXPECTED.values()}
    }
    owner = EXPECTED[symbol][1]
    marker = f"// RECOVERY_OBJECT: v1 {old_address} {symbol}"
    assert source_texts[owner].count(marker) == 1

    missing = dict(source_texts)
    missing[owner] = missing[owner].replace(marker, "", 1)
    with pytest.raises(AssertionError):
        _require_bindings(missing)

    wrong = dict(source_texts)
    wrong[owner] = wrong[owner].replace(marker, marker.replace(old_address, new_address), 1)
    with pytest.raises(AssertionError):
        _require_bindings(wrong)


def test_duplicate_expected_binding_is_rejected():
    source_texts = {
        path: (ROOT / path).read_text(encoding="utf-8")
        for path in {value[1] for value in EXPECTED.values()}
    }
    owner = "src/game.cpp"
    marker = "// RECOVERY_OBJECT: v1 0x009B2068 ?ExitTurnLoop@@3HA"
    assert source_texts[owner].count(marker) == 1
    duplicated = dict(source_texts)
    duplicated[owner] = duplicated[owner].replace(marker, marker + "\n" + marker, 1)
    with pytest.raises((AssertionError, ValueError)):
        _require_bindings(duplicated)


def test_pinned_operands_support_scalar_addresses_widths_and_roles():
    if not IMAGE.is_file():
        pytest.skip("pinned original image required")
    raw = IMAGE.read_bytes()
    assert hashlib.sha256(raw).hexdigest() == PIN
    image = PeImage(raw, PIN)
    decoder = Cs(CS_ARCH_X86, CS_MODE_32)
    decoder.detail = True
    for address, (mnemonic, operands, width) in PINNED.items():
        instruction, = decoder.disasm(image.read(Span(address, address + 16)),
                                      address, count=1)
        assert (instruction.mnemonic, instruction.op_str) == (mnemonic, operands)
        if width is not None:
            memory = [operand for operand in instruction.operands
                      if operand.type == X86_OP_MEM]
            assert memory and all(operand.size == width for operand in memory)
