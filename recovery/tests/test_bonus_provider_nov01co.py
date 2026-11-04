"""Small host controls for the canonical bonus provider at 00592030.

Expected cases follow the pinned altitude/seed gates and return cascade, not a
second implementation. Accessor doubles supply a tile even for invalid map
coordinates: this checks that the provider forwards coordinates without a new
clamp, not that the game safely accepts an invalid map index. Host execution
does not establish VC6 ABI, caller visibility, exports or linked retention.
"""
from pathlib import Path
import re
import shutil
import subprocess

import pytest
from match_recovery.includes import COMMENTS_OR_STRINGS

ROOT = Path(__file__).resolve().parents[2]


def _masked(source):
    return COMMENTS_OR_STRINGS.sub(lambda m: re.sub(r"[^\n]", " ", m[0]), source)


def _definition(source, signature):
    masked = _masked(source)
    matches = list(re.finditer(signature, masked))
    assert len(matches) == 1
    start = matches[0].start()
    opening = masked.index("{", start)
    depth = 0
    for pos in range(opening, len(masked)):
        depth += (masked[pos] == "{") - (masked[pos] == "}")
        if depth == 0:
            return source[start:pos + 1]
    raise AssertionError("unterminated definition")


def test_bonus_provider_declaration_and_home():
    header = _masked((ROOT / "src/map.h").read_text())
    declarations = list(re.finditer(r"\bbonus_at\s*\([^;{}]*\)\s*;", header))
    assert len(declarations) == 1
    assert not re.search(r"\bbonus_at\s*\([^;{}]*\)\s*\{", header)
    statement = header[header.rfind(";", 0, declarations[0].start()) + 1:declarations[0].end()]
    assert not re.search(r"\b(?:inline|MEASURED|__declspec)\b", statement)
    _definition((ROOT / "src/map_grid.cpp").read_text(), r"\bint\s+__cdecl\s+bonus_at\s*\(")


PREFIX = r'''#include <cassert>
#include <cstdint>
#define __cdecl
#define UNUSED(x) x
using BOOL=int;
MAP_DECLARATION
Map tile={};
int MapRandSeed;
struct State { struct Fields { uint32_t game_rules_; } fields_; } GameDataStateStorage={};
enum { BIT_RSC_BONUS=0x400, BIT_NUTRIENT_RSC=0x20000000,
       BIT_MINERAL_RSC=0x20000, BIT_ENERGY_RSC=0x10000,
       ALT_OCEAN_SHELF=2, ALT_SHORE_LINE=3,
       RULES_NO_UNITY_SCATTERING=0x2000 };
int expected_x, expected_y, calls;
uint32_t bit_at(int x,int y) {
    assert(x==expected_x && y==expected_y);++calls;return tile.bit;
}
int alt_at(int x,int y) {
    assert(x==expected_x && y==expected_y);++calls;return tile.climate >> 5;
}
'''
MAIN = r'''
struct Case { int x,y,alt; uint32_t bits; int seed; uint32_t rules; int expected; };
int main() {
    const uint32_t forced=BIT_RSC_BONUS;
    // Fixed answers: deep water; seed gate; land rules gate; shelf/land
    // residues; forced nutrient override and energy-before-mineral priority.
    Case cases[]={
      {0,0,1,forced,0,0,0},
      {0,0,2,0,0,0,0}, {0,0,2,0,2,0,0}, {0,0,2,0,16,0,1},
      {0,0,3,0,16,0,0}, {0,0,3,0,16,0x2000,1},
      {0,0,2,forced,0,0,1}, {2,0,2,forced,0,0,3},
      {2,2,3,forced,0,0,2}, {0,2,3,forced,0,0,3},
      {2,2,3,forced|BIT_ENERGY_RSC,0,0,2},
      {2,2,3,forced|BIT_NUTRIENT_RSC,0,0,1},
      {2,2,3,forced|BIT_NUTRIENT_RSC|BIT_MINERAL_RSC,0,0,2},
      {2,2,3,forced|BIT_NUTRIENT_RSC|BIT_ENERGY_RSC|BIT_MINERAL_RSC,0,0,3},
      {-4,0,2,forced,0,0,2}, {1000,0,3,forced,0,0,1}
    };
    for(const auto &c: cases) {
      expected_x=c.x;expected_y=c.y;tile={};
      tile.climate=uint8_t(c.alt << 5);tile.bit=c.bits;
      MapRandSeed=c.seed;GameDataStateStorage.fields_.game_rules_=c.rules;
      calls=0;assert(bonus_at(c.x,c.y,0)==c.expected);assert(calls==2);
      calls=0;assert(bonus_at(c.x,c.y,1)==c.expected);assert(calls==2);
    }
}
'''


@pytest.mark.parametrize("damage", [None, "seed_gate", "deep_water", "override"])
def test_actual_bonus_algorithm(tmp_path, damage):
    compiler = shutil.which("g++") or shutil.which("clang++")
    if not compiler:
        pytest.skip("host C++ compiler unavailable")
    header = (ROOT / "src/map.h").read_text()
    declaration = _definition(header, r"\bstruct\s+Map\s*(?=\{)") + ";"
    body = _definition((ROOT / "src/map_grid.cpp").read_text(), r"\bint\s+__cdecl\s+bonus_at\s*\(")
    changes = {
        "seed_gate": ("!MapRandSeed", "MapRandSeed"),
        "deep_water": ("alt < ALT_OCEAN_SHELF", "alt < 0"),
        "override": ("!ret || bit & BIT_NUTRIENT_RSC", "!ret"),
    }
    if damage:
        old, new = changes[damage]
        assert body.count(old) == 1, (damage, old)
        changed = body.replace(old, new)
        assert changed != body
        body = changed
    unit = tmp_path / "bonus.cpp"
    executable = tmp_path / "bonus"
    unit.write_text(PREFIX.replace("MAP_DECLARATION", declaration) + body + MAIN)
    compiled = subprocess.run([compiler, "-std=c++11", "-O0", str(unit), "-o", str(executable)],
                              text=True, capture_output=True)
    assert compiled.returncode == 0, compiled.stderr
    result = subprocess.run([str(executable)], text=True, capture_output=True)
    if damage is None:
        assert result.returncode == 0, result.stderr
    else:
        assert result.returncode != 0, "compile-success damage escaped output cases"
