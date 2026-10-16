"""The repair redraw retains the pinned global draw_tile call boundary."""

from pathlib import Path
import os
import re

import pytest


ROOT = Path(__file__).resolve().parents[2]
SOURCE = ROOT / "src/game.cpp"
FUNCTION_START = 0x00526030
FUNCTION_END = 0x005267A9
CALL_PC = 0x00526781
DRAW_TILE = 0x0046AF40


def _without_comments(text):
    return re.sub(r"/\*.*?\*/|//[^\n]*", "", text, flags=re.S)


def _repair_phase_body(source):
    code = _without_comments(source)
    match = re.search(r"void\s+__cdecl\s+repair_phase\s*\(\s*int\s+faction_id\s*\)\s*\{", code)
    assert match, "repair_phase definition not found"
    start = match.end() - 1
    depth = 0
    for offset in range(start, len(code)):
        if code[offset] == "{":
            depth += 1
        elif code[offset] == "}":
            depth -= 1
            if depth == 0:
                return code[start + 1:offset]
    raise AssertionError("repair_phase body has no closing brace")


def test_repair_phase_uses_original_draw_tile_boundary_and_keeps_arguments():
    source = SOURCE.read_text(encoding="utf-8")
    body = _repair_phase_body(source)

    assert "draw_tile_call" not in source
    assert "static void __cdecl draw_tile_call" not in source
    assert re.findall(r"\bdraw_tile\s*\(\s*x\s*,\s*y\s*,\s*-1\s*\)\s*;", body) == [
        "draw_tile(x, y, -1);"
    ]
    assert "breed_mod_call(base_id, Bases[base_id].faction_id_current)" in body
    assert "static int __cdecl breed_mod_call" in source


def test_pinned_repair_phase_call_targets_global_draw_tile():
    from capstone import Cs, CS_ARCH_X86, CS_MODE_32
    from match_recovery.model import Span
    from match_recovery.pe_coff import PeImage
    from match_recovery.source_inventory import image_pin

    path = Path(os.environ.get("OPENSMACX_IMAGE", ROOT / ".opensmacx/game/terranx_original.exe"))
    if not path.is_file():
        pytest.skip("pinned original required for call-target verification")

    image = PeImage(path.read_bytes(), image_pin(ROOT))
    span = Span(FUNCTION_START, FUNCTION_END)
    decoder = Cs(CS_ARCH_X86, CS_MODE_32)
    decoder.detail = True
    instructions = list(decoder.disasm(image.read(span), span.start))
    assert instructions, "repair_phase original span did not decode"
    cursor = FUNCTION_START
    for item in instructions:
        assert item.address == cursor, f"undecoded byte at {cursor:#x}"
        cursor += item.size
    assert cursor == FUNCTION_END
    instruction, = [item for item in instructions if item.address == CALL_PC]
    assert instruction.mnemonic == "call"
    assert instruction.operands[0].imm == DRAW_TILE
