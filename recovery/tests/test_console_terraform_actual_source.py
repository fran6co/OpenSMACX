"""Bounded direct-source controls for Console::terraform's reviewed gaps."""

from __future__ import annotations

import shutil
import subprocess
from pathlib import Path

import pytest


ROOT = Path(__file__).resolve().parents[2]
SOURCE = ROOT / "src" / "console_commands.cpp"


def function_body(text: str) -> str:
    marker = "void Console::terraform(int vehicle_id, int requested_terraform)"
    start = text.index(marker)
    opening = text.index("{", start)
    depth = 0
    for offset in range(opening, len(text)):
        if text[offset] == "{":
            depth += 1
        elif text[offset] == "}":
            depth -= 1
            if depth == 0:
                return text[opening + 1 : offset]
    raise AssertionError("unterminated Console::terraform body")


def source_block(text: str, start: str, end: str) -> str:
    body = function_body(text)
    return body[body.index(start) : body.index(end, body.index(start))]


def compile_run(tmp_path: Path, source: str, compiler: str) -> subprocess.CompletedProcess[str]:
    cpp = tmp_path / "control.cpp"
    exe = tmp_path / "control"
    cpp.write_text(source)
    returncode = subprocess.run(
        [compiler, "-std=c++98", "-Wall", "-Wextra", "-Werror", str(cpp), "-o", str(exe)],
        text=True,
        capture_output=True,
        check=False,
    )
    assert returncode.returncode == 0, returncode.stdout + returncode.stderr
    return subprocess.run([str(exe)], text=True, capture_output=True, check=False)


def compiler_paths() -> list[str]:
    found = [shutil.which(name) for name in ("g++", "clang++")]
    paths = [path for path in found if path]
    if len(paths) != 2:
        pytest.skip("both host GCC and Clang are required for this control")
    return paths


AUTO_START = "    int terraform_id = requested_terraform;"
AUTO_END = "    // Native auto-selection only rewrites the ordinary road/farm requests;"
STEPS_START = "        int steps = 1;"
STEPS_END = "        int terraform_offset;"


def auto_program(text: str) -> str:
    block = source_block(text, AUTO_START, AUTO_END)
    return r"""
struct Tile { unsigned int bit; };
struct TerraformEntry { int prerequisite_tech[2]; };
enum { TERRA_FARM=0, TERRA_SOIL_ENR=1, TERRA_ROAD=5, TERRA_MAGTUBE=6,
       TERRA_REMOVE_FUNGUS=10 };
enum { BIT_ROAD=4, BIT_FUNGUS=0x20, BIT_FARM=0x8000 };
static TerraformEntry Terraforming[20];
int choose(int requested_terraform, unsigned int bits, int altitude, int is_sea,
           int mag_land, int mag_sea, int soil_land, int soil_sea) {
    Tile storage; storage.bit = bits; Tile *tile = &storage;
    Terraforming[TERRA_MAGTUBE].prerequisite_tech[0] = mag_land;
    Terraforming[TERRA_MAGTUBE].prerequisite_tech[1] = mag_sea;
    Terraforming[TERRA_SOIL_ENR].prerequisite_tech[0] = soil_land;
    Terraforming[TERRA_SOIL_ENR].prerequisite_tech[1] = soil_sea;
""" + block + r"""
    return terraform_id;
}
int main() {
    if (choose(TERRA_ROAD, BIT_ROAD, 0x60, 0, -1, -2, -2, -2) != TERRA_MAGTUBE) return 1;
    if (choose(TERRA_ROAD, BIT_ROAD, 0x60, 1, -2, -1, -2, -2) != TERRA_MAGTUBE) return 2;
    if (choose(TERRA_ROAD, BIT_ROAD, 0x60, 0, -2, -2, -2, -2) != TERRA_ROAD) return 3;
    if (choose(TERRA_FARM, BIT_FUNGUS, 0x3f, 0, -2, -2, -2, -2) != TERRA_FARM) return 4;
    if (choose(TERRA_FARM, BIT_FUNGUS, 0x40, 0, -2, -2, -2, -2) != TERRA_REMOVE_FUNGUS) return 5;
    if (choose(TERRA_FARM, BIT_FUNGUS|BIT_FARM, 0x60, 0, -2, -2, -1, -2) != TERRA_REMOVE_FUNGUS) return 6;
    if (choose(TERRA_FARM, BIT_FARM, 0x5f, 0, -2, -2, -1, -2) != TERRA_FARM) return 7;
    if (choose(TERRA_FARM, BIT_FARM, 0x60, 0, -2, -2, -1, -2) != TERRA_SOIL_ENR) return 8;
    if (choose(TERRA_FARM, BIT_FARM, 0x60, 1, -2, -2, -2, -1) != TERRA_SOIL_ENR) return 9;
    if (choose(TERRA_FARM, BIT_FARM, 0x60, 0, -2, -2, -2, -2) != TERRA_FARM) return 10;
    return 0;
}
"""


def steps_program(text: str) -> str:
    block = source_block(text, STEPS_START, STEPS_END)
    return r"""
enum { TERRA_RAISE_LAND=16, TERRA_LOWER_LAND=17 };
static const int RadiusRange[9] = {1,9,25,49,81,121,169,225,289};
struct Tile { unsigned char climate; };
int count_steps(int terraform_id, unsigned char climate) {
    Tile storage; storage.climate = climate; Tile *tile = &storage;
    (void)tile;
    const int altitude = 0;
    (void)altitude;
""" + block + r"""
    return steps;
}
int expected(int id, unsigned char climate) {
    int level = (climate >> 5);
    if (id == TERRA_RAISE_LAND) {
        ++level;
        if (level > 6) level = 6;
        return level > 3 ? RadiusRange[level - 3] : 1;
    }
    --level;
    if (level < 0) level = 0;
    return level < 3 ? RadiusRange[4 - level] : 1;
}
int main() {
    for (int climate = 0; climate <= 255; ++climate) {
        if (count_steps(TERRA_RAISE_LAND, (unsigned char)climate) !=
            expected(TERRA_RAISE_LAND, (unsigned char)climate)) return 1;
        if (count_steps(TERRA_LOWER_LAND, (unsigned char)climate) !=
            expected(TERRA_LOWER_LAND, (unsigned char)climate)) return 2;
    }
    return 0;
}
"""


@pytest.mark.parametrize("compiler", compiler_paths())
def test_auto_selection_is_extracted_from_actual_method(tmp_path: Path, compiler: str) -> None:
    result = compile_run(tmp_path, auto_program(SOURCE.read_text()), compiler)
    assert result.returncode == 0, result.stdout + result.stderr


@pytest.mark.parametrize("compiler", compiler_paths())
def test_radius_step_counts_match_pinned_byte_domain(tmp_path: Path, compiler: str) -> None:
    result = compile_run(tmp_path, steps_program(SOURCE.read_text()), compiler)
    assert result.returncode == 0, result.stdout + result.stderr


@pytest.mark.parametrize("compiler", compiler_paths())
def test_old_radius_indices_are_rejected(tmp_path: Path, compiler: str) -> None:
    text = SOURCE.read_text()
    old = text.replace("RadiusRange[level - 3]", "RadiusRange[level - 4]")
    old = old.replace("steps = RadiusRange[3];", "steps = RadiusRange[2];")
    old = old.replace("RadiusRange[4 - level]", "RadiusRange[3 - level]")
    assert old != text
    result = compile_run(tmp_path, steps_program(old), compiler)
    assert result.returncode != 0


@pytest.mark.parametrize("compiler", compiler_paths())
def test_step_path_reloads_climate_instead_of_using_entry_snapshot(
    tmp_path: Path, compiler: str
) -> None:
    text = SOURCE.read_text()
    current = steps_program(text)
    cached = current.replace("tile->climate", "altitude")
    assert cached != current
    result = compile_run(tmp_path, cached, compiler)
    assert result.returncode != 0


def test_final_conflict_bit_test_uses_the_reloaded_map_tile() -> None:
    body = function_body(SOURCE.read_text())
    begin = body.index("Map *const final_tile = map_loc(x, y);")
    end = body.index("if (NetDaemonNet.channels_[1]", begin)
    final_scan = body[begin:end]
    def has_reload(scan: str) -> bool:
        return (
            "const int final_altitude = final_tile->climate & 0xE0;" in scan
            and "Terraforming[terraform_id].bit_incompatible & final_tile->bit" in scan
        )

    assert has_reload(final_scan)
    cached = final_scan.replace("final_tile->climate", "altitude")
    assert not has_reload(cached)


@pytest.mark.parametrize("compiler", compiler_paths())
def test_auto_selection_threshold_mutants_are_rejected(tmp_path: Path, compiler: str) -> None:
    text = SOURCE.read_text()
    original = auto_program(text)
    mutant = original.replace("altitude >= 0x40", "altitude > 0x40")
    assert mutant != original
    result = compile_run(tmp_path, mutant, compiler)
    assert result.returncode != 0

    mutant = original.replace(") >= -1)", ") > -1)")
    assert mutant != original
    result = compile_run(tmp_path, mutant, compiler)
    assert result.returncode != 0


def _braced_definition(text: str, marker: str) -> str:
    start = text.index(marker)
    opening = text.index("{", start)
    depth = 1
    end = opening + 1
    while depth:
        depth += (text[end] == "{") - (text[end] == "}")
        end += 1
    return text[start:end]


def rockiness_program(text: str, damage: str | None = None) -> str:
    """Compile genuine query definitions and actual method guard/case text.

    The allocation/stride transition is synthetic input staging between the
    entry snapshot and query stage, not a claimed original runtime callback.
    """
    body = function_body(text)
    start = body.index("    if (!is_sea && (map_loc(x, y)->val3 & 0xC0)")
    end = body.index("    // Existing fungus", start)
    rocky = body[start:end]
    start = body.index("    case TERRA_LEVEL_TERRAIN:")
    end = body.index("    case TERRA_RAISE_LAND:", start)
    level = body[start:end]
    header = (ROOT / "src/map.h").read_text()
    record = _braced_definition(header, "struct Map {") + ";"
    locator = _braced_definition(header, "inline Map *__cdecl map_loc(int x, int y)")
    allocation = _braced_definition(header, "inline Map *&map_tiles()")
    before = rocky, level, locator, allocation
    if damage == "wrong_member":
        rocky, level = rocky.replace("->val3", "->contour"), level.replace("->val3", "->contour")
    elif damage == "entry_tile":
        rocky, level = rocky.replace("map_loc(x, y)", "entry_tile"), level.replace("map_loc(x, y)", "entry_tile")
    elif damage == "stale_allocation":
        allocation = allocation.replace("return MapTiles;", "return entry_tiles;")
    elif damage == "stale_stride":
        locator = locator.replace("MapLongitude", "entry_stride")
    elif damage == "truncating_x_division":
        locator = locator.replace("x >> 1", "x / 2")
    elif damage == "warning_fallthrough":
        rocky, level = rocky.replace("return;", "(void)0;"), level.replace("return;", "(void)0;")
    elif damage == "wrong_warning_arguments":
        rocky = rocky.replace('"NOTINROCKY", 5000', '"NOTINROCKY", 4999')
    assert ((rocky, level, locator, allocation) != before) == (damage is not None), "damage anchor drift"
    return r'''
#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#define __cdecl
#define nullptr 0
''' + record + r'''
// A narrow storage double, not a MapStorageWindow layout claim.
struct MapStorageDouble { Map *tiles_; } MapStorage;
#define MapTiles (MapStorage.tiles_)
Map *entry_tiles, *next_tiles;
int MapLongitude, entry_stride, next_stride;
''' + allocation + "\n" + locator + r'''
int warnings, continuation, warning_kind;
struct NetMessage { void pop(const char *key,int timeout,int flags,const char *arg) {
 assert(timeout==5000&&!flags&&!arg&&"WARNING_ARGUMENTS");
 assert(!strcmp(key,"NOTINROCKY")&&"WARNING_IDENTITY");
 ++warnings;warning_kind=1;
}} NetMsgPrimary;
int X_pop(const char *key,void *callback) {
 assert(!callback&&"WARNING_ARGUMENTS");
 assert(!strcmp(key,"TOOFLAT")&&"WARNING_IDENTITY");
 ++warnings;warning_kind=2;return -7;
}
void query_rocky(int x,int y,int is_sea,unsigned int terrain_bit) {
 Map *const entry_tile=map_loc(x,y);(void)entry_tile;
 MapStorage.tiles_=next_tiles;MapLongitude=next_stride;
''' + rocky + r'''
 ++continuation;
}
enum {TERRA_LEVEL_TERRAIN=18};
void query_level(int x,int y) {
 Map *const entry_tile=map_loc(x,y);(void)entry_tile;
 MapStorage.tiles_=next_tiles;MapLongitude=next_stride;
 switch(TERRA_LEVEL_TERRAIN) {
''' + level + r'''
 default:assert(false);
 }
 ++continuation;
}
Map first_allocation[160], second_allocation[160];
void prepare(int x,int y,int band,int contour_band) {
 memset(first_allocation,0,sizeof(first_allocation));
 memset(second_allocation,0,sizeof(second_allocation));
 for(int i=0;i<160;++i) {
  first_allocation[i].val3=static_cast<uint8_t>((3-band)*64);
  second_allocation[i].val3=static_cast<uint8_t>((3-band)*64);
 }
 entry_tiles=first_allocation+32;MapStorage.tiles_=entry_tiles;
 MapLongitude=entry_stride=3;next_stride=7;next_tiles=second_allocation+32;
 // Independent signed floor-by-two expectation, including negative odd x.
 const int half=x>=0?x/2:-((-x+1)/2);
 Map &selected=second_allocation[32+half+y*7];
 selected.val3=static_cast<uint8_t>(band*64);
 selected.contour=static_cast<uint8_t>(contour_band*64);
 warnings=continuation=warning_kind=0;
}
int main() {
 assert(sizeof(Map)==44&&offsetof(Map,val3)==5&&offsetof(Map,contour)==1);
 const int xs[3]={2,4,-3};
 const unsigned int improvements[4]={0,0x8000,0x200000,4};
 for(int xi=0;xi<3;++xi) for(int band=0;band<4;++band)
 for(int contour_band=0;contour_band<4;++contour_band) {
  for(int sea=0;sea<2;++sea) for(int kind=0;kind<4;++kind) {
   prepare(xs[xi],1,band,contour_band);
   query_rocky(xs[xi],1,sea,improvements[kind]);
   const bool reject=!sea&&band>=2&&(kind==1||kind==2);
   assert(warnings==(reject?1:0)&&"ROCKINESS_SELECTION");
   assert(continuation==(reject?0:1)&&"EARLY_RETURN");
   assert(warning_kind==(reject?1:0)&&"WARNING_IDENTITY");
  }
  prepare(xs[xi],1,band,contour_band);
  query_level(xs[xi],1);
  assert(warnings==(band==0?1:0)&&"ROCKINESS_SELECTION");
  assert(continuation==(band==0?0:1)&&"EARLY_RETURN");
  assert(warning_kind==(band==0?2:0)&&"WARNING_IDENTITY");
 }
 return 0;
}
'''


ROCKINESS_DAMAGE = {
    None: None,
    "wrong_member": "ROCKINESS_SELECTION",
    "entry_tile": "ROCKINESS_SELECTION",
    "stale_allocation": "ROCKINESS_SELECTION",
    "stale_stride": "ROCKINESS_SELECTION",
    "truncating_x_division": "ROCKINESS_SELECTION",
    "warning_fallthrough": "EARLY_RETURN",
    "wrong_warning_arguments": "WARNING_ARGUMENTS",
}


@pytest.mark.parametrize("compiler", compiler_paths())
@pytest.mark.parametrize("damage", ROCKINESS_DAMAGE)
def test_rockiness_uses_actual_fresh_map_query_and_warning_returns(
    tmp_path: Path, compiler: str, damage: str | None
) -> None:
    result = compile_run(tmp_path, rockiness_program(SOURCE.read_text(), damage), compiler)
    if damage is None:
        assert result.returncode == 0, result.stdout + result.stderr
    else:
        assert result.returncode != 0 and ROCKINESS_DAMAGE[damage] in result.stderr, result.stderr
