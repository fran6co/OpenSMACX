"""Actual-source controls for post-bonus map-tile lookup phases."""
from pathlib import Path
import shutil
import subprocess

import pytest

from test_boom_live_window_reload_oct21 import _masked_code


ROOT = Path(__file__).resolve().parents[2]


def _function_body(source, signature):
    masked = _masked_code(source)
    start = masked.index(signature)
    opening = masked.index("{", start)
    depth = 0
    for index in range(opening, len(masked)):
        if masked[index] == "{":
            depth += 1
        elif masked[index] == "}":
            depth -= 1
            if depth == 0:
                return source[start:index + 1]
    raise AssertionError("unbalanced source function: " + signature)


def _prefix(body, bonus_name):
    start = body.index("Map *tile = yield_tile(x, y);")
    bonus = body.index("BOOL has_" + bonus_name + "_bonus =", start)
    refresh = body.index("tile = yield_tile(x, y);", bonus)
    landmark = body.index("BOOL landmark_bonus =", refresh)
    end = body.index(";", landmark) + 1
    prefix = body[start:end]
    assert prefix.count("tile = yield_tile(x, y);") == 2
    return prefix, prefix.index("BOOL landmark_bonus")


def _source_fragments():
    source = (ROOT / "src" / "base.cpp").read_text()
    map_header = (ROOT / "src" / "map.h").read_text()
    faction_header = (ROOT / "src" / "faction.h").read_text()

    map_record = _function_body(map_header, "struct Map {")
    map_record = map_record[:map_record.rfind("}") + 1] + ";"
    enums = []
    for name in ("TerrainAltitudeBit", "TerrainBit", "TerrainLandmarkBit2"):
        enums.append(_function_body(map_header, "enum " + name + " {") + ";")
    max_players = "static const int MaxPlayerNum = 8;"
    assert max_players in faction_header
    yield_tile = _function_body(source, "static __forceinline Map *yield_tile(")
    yield_owner = _function_body(source,
                                 "static __forceinline int yield_tile_owner(")

    crop_body = _function_body(source, "int __cdecl crop_yield(")
    mine_body = _function_body(source, "int __cdecl mine_yield(")
    energy_body = _function_body(source, "int __cdecl energy_yield(")
    crop, _ = _prefix(crop_body, "nutrient")
    mine, _ = _prefix(mine_body, "mineral")
    energy, _ = _prefix(energy_body, "energy")

    mine_owner = "yield_tile_owner(tile) >= 0"
    assert mine_owner in mine_body[mine_body.index("BOOL landmark_bonus ="):]
    energy_climate = "(tile->climate & 0xE0) >= ALT_BIT_OCEAN_SHELF"
    energy_later = energy_body[energy_body.index("BOOL landmark_bonus ="):]
    assert energy_climate in energy_later
    assert "bit = tile->bit;" in crop
    assert "is_ocean_tile = (tile->climate & 0xE0) < ALT_BIT_SHORE_LINE;" in crop
    assert "has_base_in_tile = yield_tile_owner(tile) >= 0;" in crop
    assert "code = (int)tile->bit2 >> 24;" in mine
    assert "bit = tile->bit;" in mine
    assert "is_ocean_tile = (tile->climate & 0xE0) < ALT_BIT_SHORE_LINE;" in energy
    return {
        "map_record": map_record,
        "enums": "\n".join(enums),
        "max_players": max_players,
        "yield_tile": yield_tile,
        "yield_owner": yield_owner,
        "crop": crop,
        "mine": mine,
        "energy": energy,
        "mine_owner": mine_owner,
        "energy_climate": energy_climate,
    }


HARNESS = r'''#include <cassert>
#include <cstdint>

typedef int BOOL;
#define __forceinline inline
MAP_RECORD
static_assert(sizeof(Map)==0x2c,"real Map layout");
ENUMS
MAX_PLAYERS

static Map before_tiles[8];
static Map after_tiles[8];
static Map *active_tiles=before_tiles;
static Map *replacement_tiles=after_tiles;
static int MapLongitude=4;
static Map *map_tiles() { return active_tiles; }
YIELD_TILE
YIELD_OWNER

static int bonus_result;
static int bonus_calls;
static int bonus_at(int x,int y,int) {
    if(x!=2 || y!=1) return -1;
    ++bonus_calls;
    // Synthetic provider-boundary sentinel only; bonus_at itself is read-only.
    active_tiles=replacement_tiles;
    return bonus_result;
}

static void setup(int selected_bonus) {
    for(int i=0;i<8;++i) {
        before_tiles[i]=Map{};
        after_tiles[i]=Map{};
    }
    active_tiles=before_tiles;
    replacement_tiles=after_tiles;
    bonus_result=selected_bonus;
    bonus_calls=0;
}

static int crop_control() {
    setup(1);
    Map &old_tile=before_tiles[5];
    old_tile.bit=BIT_BASE_IN_TILE | 0x100;
    old_tile.climate=ALT_BIT_OCEAN;
    old_tile.val2=2;
    old_tile.bit2=7u<<24;
    Map &new_tile=after_tiles[5];
    new_tile.bit=0;
    new_tile.climate=ALT_BIT_SHORE_LINE;
    new_tile.bit2=BIT2_JUNGLE;
    const int x=2,y=1;
    CROP_PREFIX
    return tile==&new_tile && bonus_calls==1 && bit==old_tile.bit
        && is_ocean_tile && has_base_in_tile && has_nutrient_bonus
        && landmark_bonus ? 0 : 1;
}

static int mine_control() {
    setup(2);
    Map &old_tile=before_tiles[5];
    old_tile.bit=0x1234;
    old_tile.bit2=7u<<24;
    Map &new_tile=after_tiles[5];
    new_tile.bit=0;
    new_tile.climate=ALT_BIT_SHORE_LINE;
    new_tile.bit2=BIT2_VOLCANO;
    const int x=2,y=1;
    MINE_PREFIX
    const BOOL post_bonus_owner=(MINE_OWNER_EXPR);
    return tile==&new_tile && bonus_calls==1 && bit==old_tile.bit && code==7
        && has_mineral_bonus && landmark_bonus && !post_bonus_owner ? 0 : 2;
}

static int energy_control() {
    setup(3);
    Map &old_tile=before_tiles[5];
    old_tile.bit=BIT_BASE_IN_TILE | 0x200;
    old_tile.bit2=7u<<24;
    old_tile.climate=ALT_BIT_OCEAN;
    old_tile.val2=2;
    Map &new_tile=after_tiles[5];
    new_tile.bit=0;
    new_tile.bit2=BIT2_VOLCANO;
    new_tile.climate=ALT_BIT_SHORE_LINE;
    const int x=2,y=1;
    ENERGY_PREFIX
    const BOOL post_bonus_land=(ENERGY_CLIMATE_EXPR);
    return tile==&new_tile && bonus_calls==1 && bit==old_tile.bit && code==7
        && is_ocean_tile && has_base_in_tile && has_energy_bonus
        && landmark_bonus && post_bonus_land ? 0 : 4;
}

int main() {
    if(crop_control()) return 1;
    if(mine_control()) return 2;
    if(energy_control()) return 3;
    return 0;
}
'''


def _program(fragments):
    return (HARNESS.replace("MAP_RECORD", fragments["map_record"])
            .replace("ENUMS", fragments["enums"])
            .replace("MAX_PLAYERS", fragments["max_players"])
            .replace("YIELD_TILE", fragments["yield_tile"])
            .replace("YIELD_OWNER", fragments["yield_owner"])
            .replace("CROP_PREFIX", fragments["crop"])
            .replace("MINE_PREFIX", fragments["mine"])
            .replace("ENERGY_PREFIX", fragments["energy"])
            .replace("MINE_OWNER_EXPR", fragments["mine_owner"])
            .replace("ENERGY_CLIMATE_EXPR", fragments["energy_climate"]))


def _replace_once(text, old, new):
    assert text.count(old) == 1, "unanchored mutant: " + old
    return text.replace(old, new, 1)


def _mutate_no_refresh(fragments):
    result = dict(fragments)
    for name in ("crop", "mine", "energy"):
        result[name] = _replace_once(
            result[name], "\n    tile = yield_tile(x, y);", "")
    return result


def _mutate_recache_prefix(fragments):
    result = dict(fragments)
    recache = {
        "crop": "bit = tile->bit;\n    is_ocean_tile = (tile->climate & 0xE0) < ALT_BIT_SHORE_LINE;\n    has_base_in_tile = yield_tile_owner(tile) >= 0;\n    ",
        "mine": "code = (int)tile->bit2 >> 24;\n    bit = tile->bit;\n    ",
        "energy": "bit = tile->bit;\n    code = (int)tile->bit2 >> 24;\n    is_ocean_tile = (tile->climate & 0xE0) < ALT_BIT_SHORE_LINE;\n    has_base_in_tile = yield_tile_owner(tile) >= 0;\n    ",
    }
    for name in ("crop", "mine", "energy"):
        result[name] = _replace_once(
            result[name], "\n    tile = yield_tile(x, y);",
            "\n    tile = yield_tile(x, y);\n    " + recache[name])
    return result


def _compile_run(tmp_path, program, compiler, name):
    source = tmp_path / (name + ".cpp")
    binary = tmp_path / name
    source.write_text(program)
    result = subprocess.run(
        [compiler, "-std=c++11", "-Wall", "-Wextra", "-Werror",
         str(source), "-o", str(binary)], capture_output=True, text=True,
        timeout=30,
    )
    assert result.returncode == 0, f"{compiler} {name} did not compile:\n{result.stderr}"
    return subprocess.run([str(binary)], capture_output=True, timeout=10)


def test_yield_functions_refresh_primary_tile_after_bonus_without_recaching_prefix(
        tmp_path):
    compilers = [shutil.which(name) for name in ("g++", "clang++")]
    compilers = [compiler for compiler in compilers if compiler]
    assert compilers
    fragments = _source_fragments()
    mutants = (
        ("no-primary-refresh", _mutate_no_refresh(fragments)),
        ("recache-prefix", _mutate_recache_prefix(fragments)),
    )
    for compiler in compilers:
        actual = _compile_run(tmp_path, _program(fragments), compiler, "actual")
        assert actual.returncode == 0, (
            f"{compiler} actual fragments failed: {actual.stderr.decode(errors='replace')}"
        )
        for name, damaged in mutants:
            result = _compile_run(tmp_path, _program(damaged), compiler, name)
            assert result.returncode != 0, f"{compiler} missed {name}"
