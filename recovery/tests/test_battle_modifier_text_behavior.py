"""Exercise the real add_bat body with its source-owned character rows."""
from pathlib import Path
import shutil
import subprocess

import pytest

ROOT = Path(__file__).resolve().parents[2]


@pytest.mark.parametrize("compiler", ["g++", "clang++"])
def test_actual_add_bat_character_rows(compiler, tmp_path):
    executable = shutil.which(compiler)
    if not executable:
        pytest.skip(compiler + " unavailable")
    source = (ROOT / "src/veh.cpp").read_text()
    declaration = "char VehBattleDisplay[8][80];"
    assert declaration in source
    assert "extern " + declaration in (ROOT / "src/veh.h").read_text()
    start = source.index("void __cdecl add_bat(")
    opening = source.index("{", start)
    depth, end = 1, opening + 1
    while depth:
        depth += (source[end] == "{") - (source[end] == "}")
        end += 1
    body = source[start:end]
    harness = r'''
#include <cstring>
#include <cstdio>
#define __cdecl
typedef const char *LPCSTR;
unsigned int VehBattleModCount[2];
int VehBattleModifier[8];
'''
    harness += declaration + "\n" + body
    harness += r'''
int main() {
    if (sizeof(VehBattleDisplay) != 640) return 1;
    std::memset(VehBattleDisplay, 0x5A, sizeof(VehBattleDisplay));
    add_bat(0, 0, "ignored");
    if (VehBattleModCount[0] || VehBattleDisplay[0][0] != 0x5A) return 2;
    for (int side = 0; side < 2; ++side) {
        for (int row = 0; row < 4; ++row) {
            char expected[32];
            std::sprintf(expected, "side%d-row%d", side, row);
            int value = (side ? -1 : 1) * (row + 1);
            add_bat(side, value, expected);
            int index = side * 4 + row;
            if (std::strcmp(VehBattleDisplay[index], expected)) return 3;
            if (VehBattleDisplay[index][79] != 0x5A) return 4;
            if (VehBattleModifier[index] != value) return 5;
            if (VehBattleModCount[side] != static_cast<unsigned>(row + 1)) return 6;
        }
        add_bat(side, 999, "fifth must not replace row4");
        if (VehBattleModCount[side] != 4 || VehBattleModifier[side*4+3] == 999) return 7;
    }
    return 0;
}
'''
    translation = tmp_path / "battle_text.cpp"
    translation.write_text(harness)
    binary = tmp_path / "battle_text"
    result = subprocess.run([executable, "-std=c++98", "-Wall", "-Wextra",
                             str(translation), "-o", str(binary)],
                            capture_output=True, text=True, timeout=30)
    assert result.returncode == 0, result.stdout + result.stderr
    result = subprocess.run([str(binary)], capture_output=True, text=True, timeout=30)
    assert result.returncode == 0, result.stdout + result.stderr
