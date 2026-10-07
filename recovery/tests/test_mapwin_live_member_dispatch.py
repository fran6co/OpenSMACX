"""Actual inline broadcasters: member gates and callback-visible table changes."""

from pathlib import Path
import subprocess

import pytest


ROOT = Path(__file__).resolve().parents[2]


def broadcaster(name):
    text = (ROOT / "src/mapwin.h").read_text()
    start = text.index(f"MEASURED inline void __cdecl {name}(")
    brace = text.index("{", start)
    depth = 0
    for end in range(brace, len(text)):
        depth += (text[end] == "{") - (text[end] == "}")
        if depth == 0:
            return text[start:end + 1]
    raise AssertionError("unterminated broadcaster")


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
