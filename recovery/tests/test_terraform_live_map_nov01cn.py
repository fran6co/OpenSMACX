"""Bounded actual-source fragment controls for can_terraform map-read phases.

The boundary doubles deliberately replace map storage or change bytes between
fragments. They discriminate original read/snapshot timing; they do NOT assert
that real facility/crop/technology queries mutate the map. No native ABI or
whole-function behavioral equivalence is established by these host controls.
"""
from pathlib import Path
import shutil
import subprocess

import pytest

ROOT = Path(__file__).resolve().parents[2]
SOURCE = ROOT / "src/terraforming.cpp"


def _between(text, first, last):
    assert text.count(first) == 1, first
    start = text.index(first)
    return text[start:text.index(last, start)]


def _fragments(source):
    source = source[source.index("int __cdecl can_terraform("):]
    return {
        "PHASE": _between(source, "    Map *const terrain_tile =", "    if (altitude <"),
        "LAND": _between(source, "    int rainfall =", "    // The bonus runs"),
        "MONOLITH": _between(source, "    if (bit_at(x, y) & BIT_MONOLITH)", "    if (has_forest"),
        "FARM": _between(source,
            "    if (has_tech(Terraforming[TERRA_FARM].prerequisite_tech[0], faction_id) && !has_farm\n",
            "    if (has_tech(Terraforming[TERRA_SOIL_ENR].prerequisite_tech[0], faction_id) && has_farm\n"),
        "FOREST": _between(source,
            "    if (has_tech(Terraforming[TERRA_FOREST].prerequisite_tech[0], faction_id)",
            "    if (has_tech(Terraforming[TERRA_PLANT_FUNGUS].prerequisite_tech[0], faction_id)"),
    }


HARNESS = r'''#include <cassert>
#include <cstdint>
struct Map { uint8_t climate, contour, val2, region, visibility, val3;
             uint32_t bit, bit2; };
Map old_tile={}, live_tile={}; Map *storage;
int reads;
Map *map_loc(int,int) { ++reads; return storage; }
int bit_at(int x,int y) { return map_loc(x,y)->bit; }
uint32_t bit2_at(int x,int y) { return map_loc(x,y)->bit2; }
int alt_at(int x,int y) { return map_loc(x,y)->climate >> 5; }
int crop_yield(int,int,int,int,int) { storage->climate=0xE0; return 1; }
using BOOL=int;
enum { BIT_MONOLITH=0x2000, BIT_THERMAL_BORE=0x1000000,
       BIT_ECH_MIRROR=0x800000, BIT2_UNK_80000000=0x80000000u,
       BIT2_VOLCANO=2, ROCKINESS_ROCKY=2,
       TERRA_FARM=0,TERRA_FOREST=1,ORDER_FARM=4,
       ORDER_SOLAR_COLLECTOR=7, ORDER_NONE=0, ORDER_PLANT_FOREST=8,
       BSTATE_UNK_4000=0x4000, PREF_AUTO_FORMER_PLANT_FORESTS=4 };
struct Terra { int prerequisite_tech[2]; } Terraforming[2]={};
struct Rule { int tech_three_nutrients_sqr; } Rules={};
struct Base { uint32_t state; } Bases[1]={};
struct Console { int faction_id_; } ConsoleGlobal={};
struct State { struct Fields { uint32_t game_preferences_; } fields_; } GameDataStateStorage={};
int has_tech(int,int) { return 1; }
int is_human(int) { return 0; }
void reset() { old_tile={};live_tile={};storage=&live_tile;reads=0; }
void phases() {
    reset(); const int x=0,y=0,faction_id=1;
    Map *tile=&old_tile; (void)tile;
    old_tile.climate=0x60; old_tile.val3=0;
    // Synthetic post-facility storage replacement precedes actual fragment.
    live_tile.climate=0xB9;live_tile.val3=0x80;
    PHASE
    LAND
    assert(altitude==0xA0 && rainfall==3 && rockiness==2);
    assert(crop==1 && elev==3 && storage->climate==0xE0);
    assert(reads==1); // crop must not induce an extra elevation lookup
}
int monolith() {
    const int x=0,y=0; uint32_t bit=old_tile.bit;(void)bit;
    MONOLITH
    return 1;
}
int farm() {
    const int x=0,y=0,faction_id=1,base_id=-1;
    Map *tile=&old_tile;uint32_t bit=old_tile.bit;(void)tile;(void)bit;
    int has_farm=0,rockiness=0,crop=1,bonus=0,has_solar=0,has_mine=0;
    int order=ORDER_NONE,rainfall=0;
    FARM
    return order;
}
int forest() {
    const int x=0,y=0,faction_id=1;
    Map *tile=&old_tile;(void)tile;
    int rockiness=0,has_mine=0,has_forest=0,has_farm=0;
    int elev=1,rainfall=0,is_special=0,plant_value=0,forest_value=10;
    int result=ORDER_NONE;
    FOREST
    return result;
}
int main() {
    phases();
    reset();live_tile.bit=BIT_MONOLITH;assert(monolith()==0);
    reset();old_tile.bit=BIT_MONOLITH;assert(monolith()==1);
    reset();old_tile.bit=BIT_THERMAL_BORE;assert(farm()==ORDER_FARM);
    reset();live_tile.bit=BIT_THERMAL_BORE;assert(farm()==ORDER_NONE);
    reset();old_tile.bit2=BIT2_VOLCANO;assert(farm()==ORDER_FARM);
    reset();live_tile.bit2=BIT2_VOLCANO;assert(farm()==ORDER_NONE);
    reset();old_tile.bit2=BIT2_VOLCANO;assert(forest()==ORDER_PLANT_FOREST);
    reset();live_tile.bit2=BIT2_VOLCANO;assert(forest()==ORDER_NONE);
}
'''


def _program(source, damage):
    fragments = _fragments(source)
    changes = {
        "entry_tile": ("PHASE", "terrain_tile = map_loc(x, y)", "terrain_tile = tile"),
        "post_crop_altitude": ("LAND", "static_cast<int>(terrain_climate >> 5) - 2", "alt_at(x, y) - 2"),
        "cached_monolith": ("MONOLITH", "bit_at(x, y) & BIT_MONOLITH", "bit & BIT_MONOLITH"),
        "cached_farm": ("FARM", "farm_tile = map_loc(x, y)", "farm_tile = tile"),
        "cached_forest": ("FOREST", "bit2_at(x, y)", "tile->bit2"),
    }
    if damage:
        fragment, old, new = changes[damage]
        assert fragments[fragment].count(old) == 1, (damage, old)
        changed = fragments[fragment].replace(old, new)
        assert changed != fragments[fragment]
        fragments[fragment] = changed
    program = HARNESS
    for marker, fragment in fragments.items():
        assert program.count("    " + marker + "\n") == 1, marker
        program = program.replace("    " + marker + "\n", fragment)
    return program


@pytest.mark.parametrize("damage", [None, "entry_tile", "post_crop_altitude",
    "cached_monolith", "cached_farm", "cached_forest"])
def test_actual_map_read_phases(tmp_path, damage):
    compiler = shutil.which("g++") or shutil.which("clang++")
    if not compiler:
        pytest.skip("host C++ compiler unavailable")
    unit = tmp_path / "control.cpp"
    executable = tmp_path / "control"
    unit.write_text(_program(SOURCE.read_text(), damage))
    compiled = subprocess.run([compiler, "-std=c++11", "-O0", str(unit), "-o", str(executable)],
                              text=True, capture_output=True)
    assert compiled.returncode == 0, compiled.stderr
    result = subprocess.run([str(executable)], text=True, capture_output=True)
    if damage is None:
        assert result.returncode == 0, result.stderr
    else:
        assert result.returncode != 0, "compile-success damage escaped runtime oracle"
