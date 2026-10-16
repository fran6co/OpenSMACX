"""Actual global broadcasters: member gates and callback-visible table changes."""

import os
from pathlib import Path
import re
import subprocess

import pytest
from capstone import Cs, CS_ARCH_X86, CS_MODE_32
from match_recovery.model import Span
from match_recovery.pe_coff import PeImage
from match_recovery.source import read_file
from match_recovery.source_inventory import image_pin


ROOT = Path(__file__).resolve().parents[2]


def broadcaster(name):
    text = (ROOT / "src/mapwin.cpp").read_text()
    match = re.search(r"^void __cdecl " + name + r"\([^;{}]*\)\s*\{", text, re.M)
    assert match, name
    start = match.start()
    brace = text.index("{", start)
    depth = 0
    for end in range(brace, len(text)):
        depth += (text[end] == "{") - (text[end] == "}")
        if depth == 0:
            return text[start:end + 1]
    raise AssertionError("unterminated broadcaster")


def test_global_broadcasters_are_declared_in_header_and_owned_by_mapwin_cpp():
    header = (ROOT / "src/mapwin.h").read_text()
    source = (ROOT / "src/mapwin.cpp").read_text()
    for name, address in (("draw_tile", "0x0046AF40"), ("draw_tiles", "0x0046B140")):
        assert f"void __cdecl {name}(int x_coord, int y_coord, int draw_type);" in header
        assert f"MEASURED inline void __cdecl {name}(" not in header
        marker = source.index(f"// ORIGINAL: {address} ?{name}@@YAXHHH@Z")
        body = source.index(f"void __cdecl {name}(", marker)
        assert body > marker
        assert broadcaster(name).startswith(f"void __cdecl {name}(")


def test_pinned_broadcaster_entries_dispatch_through_mapwin_radius():
    image_path = Path(os.environ.get("OPENSMACX_IMAGE", ROOT / ".opensmacx/game/terranx_original.exe"))
    if not image_path.is_file():
        pytest.skip("pinned original required")
    image = PeImage(image_path.read_bytes(), image_pin(ROOT))
    decoder = Cs(CS_ARCH_X86, CS_MODE_32)
    records = {row.address: row for row in read_file(ROOT / "src/mapwin.cpp")}
    for entry, pc in ((0x0046AF40, 0x0046AF71), (0x0046B140, 0x0046B171)):
        record = records[entry]
        spans = record.image_spans
        assert spans
        instructions = {
            instruction.address: instruction
            for start, end in spans
            for instruction in decoder.disasm(image.read(Span(start, end)), start)
        }
        assert any(start <= pc < end for start, end in spans)
        assert instructions[pc].mnemonic == "call"
        assert instructions[pc].op_str == "0x46a2a0"


PREFIX = r'''
#include <stdint.h>
#include <stddef.h>
#define MEASURED
#define __cdecl
struct MapWin {
    uint32_t field_1DD74_;
    int id;
    void draw_radius(int, int, int, int);
};
MapWin *MapWinTable[8];
const size_t MapWinTableSlots = 8;
MapWin replacement = { 0xffffffffu, 4 };
int calls[8], total, expected_radius;
void MapWin::draw_radius(int x, int y, int radius, int flags) {
    if (x != 12 || y != 34 || radius != expected_radius || flags != 7)
        total = -100;
    ++calls[id]; ++total;
    if (id == 1) MapWinTable[3] = &replacement;
}
'''


def run_control(tmp_path, compiler, name, body, null_primary=False):
    primary = "nullptr" if null_primary else "&first"
    expected = "0" if null_primary else "1"
    main = f'''
int main() {{
    MapWin first = {{ 0, 0 }}, active = {{ 2, 1 }};
    MapWin inactive = {{ 0, 2 }}, displaced = {{ 1, 3 }};
    MapWinTable[0] = {primary};
    MapWinTable[1] = &active;
    MapWinTable[2] = &inactive;
    MapWinTable[3] = &displaced;
    expected_radius = {int(name == 'draw_tiles')};
    {name}(12, 34, 7);
    return calls[0] == {expected} && calls[1] == 1 && calls[2] == 0
        && calls[3] == 0 && calls[4] == 1 && total == {2 + int(not null_primary)}
        ? 0 : 1;
}}
'''
    cpp = tmp_path / "control.cpp"
    exe = tmp_path / "control"
    cpp.write_text(PREFIX + body + main)
    subprocess.run([compiler, "-std=c++11", "-O2", str(cpp), "-o", str(exe)],
                   check=True, capture_output=True)
    return subprocess.run([str(exe)], capture_output=True).returncode


@pytest.mark.parametrize("compiler", ["g++", "clang++"])
@pytest.mark.parametrize("name", ["draw_tile", "draw_tiles"])
def test_actual_broadcaster_primary_gate_and_live_table(tmp_path, compiler, name):
    body = broadcaster(name)
    assert "static_cast<const volatile MapWin *>(window)->field_1DD74_" in body
    assert run_control(tmp_path, compiler, name, body) == 0
    assert run_control(tmp_path, compiler, name, body, null_primary=True) == 0


@pytest.mark.parametrize("compiler", ["g++", "clang++"])
@pytest.mark.parametrize("name", ["draw_tile", "draw_tiles"])
def test_controls_reject_activity_and_primary_gate_damage(tmp_path, compiler, name):
    body = broadcaster(name)
    for old, new in [("if (slot != 0)", "if (true)"),
                     ("if (active == 0)", "if (active != 0)")]:
        mutant = body.replace(old, new)
        assert mutant != body
        assert run_control(tmp_path, compiler, name, mutant) != 0
