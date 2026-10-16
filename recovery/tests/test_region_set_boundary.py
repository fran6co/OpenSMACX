"""The mapped region setter owns Path calls; brush retains same-TU inlining."""

from pathlib import Path
import os
import re

import pytest

from match_recovery.source import read_file


ROOT = Path(__file__).resolve().parents[2]
PIN = "01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf"


def _body(text, signature):
    start = text.index(signature)
    opening = text.index("{", start)
    depth = 0
    for end in range(opening, len(text)):
        depth += (text[end] == "{") - (text[end] == "}")
        if depth == 0:
            return text[start:end + 1]
    raise AssertionError(f"unterminated body: {signature}")


def test_region_set_is_the_map_cpp_owner_and_path_calls_it_directly():
    header = (ROOT / "src/map.h").read_text()
    map_cpp = (ROOT / "src/map.cpp").read_text()
    path_cpp = (ROOT / "src/path.cpp").read_text()
    declaration = "void __cdecl region_set(int x, int y, int region);"
    definition = "void __cdecl region_set(int x, int y, int region)"
    assert header.count(declaration) == 1
    assert "MEASURED inline void __cdecl region_set" not in header
    assert map_cpp.count(definition) == 1
    region = _body(map_cpp, definition)
    brush = _body(map_cpp, "void __cdecl brush(int x, int y, int altitude)")
    assert "map_loc(x, y)->region = region;" in region
    assert "region_set(" in brush
    assert map_cpp.index(definition) < map_cpp.index("void __cdecl brush(")
    path_code = re.sub(r"/\*.*?\*/|//[^\n]*", "", path_cpp, flags=re.S)
    assert "region_set_call" not in path_code
    assert len(re.findall(r"\bregion_set\s*\(", path_code)) == 4
    for args in ("x, y, region", "x_radius, y_radius, region",
                 "x, 0, pole_region", "x, y_south_pole, pole_region"):
        assert f"region_set({args});" in path_cpp


def _mapped_span(address):
    rows = [row for row in read_file(ROOT / "src/path.cpp")
            if row.address == address]
    if not rows:
        rows = [row for row in read_file(ROOT / "src/map.cpp")
                if row.address == address]
    row, = rows
    spans = row.image_spans
    assert len(spans) == 1, (hex(address), spans)
    from match_recovery.model import Span
    return Span(*spans[0])


def test_pinned_path_calls_target_owner_and_brush_does_not_call_it():
    from capstone import Cs, CS_ARCH_X86, CS_MODE_32
    from capstone.x86 import X86_OP_IMM
    from match_recovery.pe_coff import PeImage
    from match_recovery.source_inventory import image_pin

    image_path = Path(os.environ.get(
        "OPENSMACX_IMAGE", ROOT / ".opensmacx/game/terranx_original.exe"))
    if not image_path.is_file():
        pytest.skip("pinned original required for call-boundary verification")
    image = PeImage(image_path.read_bytes(), image_pin(ROOT))
    decoder = Cs(CS_ARCH_X86, CS_MODE_32)
    decoder.detail = True

    def instructions(address):
        span = _mapped_span(address)
        decoded = list(decoder.disasm(image.read(span), span.start))
        cursor = span.start
        for insn in decoded:
            assert insn.address == cursor, f"undecoded byte at {cursor:#x}"
            cursor += insn.size
        assert cursor == span.end
        return decoded

    setter = 0x00591B90
    expected_calls = {
        0x0059C520: {0x0059C571, 0x0059C698},
        0x0059C790: {0x0059CA49, 0x0059CAC3},
    }
    for owner, expected_pcs in expected_calls.items():
        calls = [insn for insn in instructions(owner)
                 if insn.mnemonic == "call"
                 and insn.operands[0].type == X86_OP_IMM
                 and insn.operands[0].imm == setter]
        assert {insn.address for insn in calls} == expected_pcs
    brush_calls = [insn for insn in instructions(0x005C2440)
                   if insn.mnemonic == "call"
                   and insn.operands[0].type == X86_OP_IMM
                   and insn.operands[0].imm == setter]
    assert brush_calls == []
