"""Actual Map/Lock bodies use the evidenced typed tile members."""
from pathlib import Path
import re
import shutil
import subprocess

import pytest

from match_recovery.includes import COMMENTS_OR_STRINGS


ROOT = Path(__file__).resolve().parents[2]
MAP_SOURCE = ROOT / "src/map.cpp"
LOCK_SOURCE = ROOT / "src/lock.cpp"


def definition(source: str, signature: str) -> str:
    masked = COMMENTS_OR_STRINGS.sub(
        lambda match: re.sub(r"[^\n]", " ", match[0]), source
    )
    start = masked.find(signature)
    assert start >= 0 and masked.find(signature, start + 1) < 0
    brace = masked.find("{", start)
    depth = 0
    for end in range(brace, len(masked)):
        if masked[end] == "{":
            depth += 1
        elif masked[end] == "}":
            depth -= 1
            if depth == 0:
                return source[start:end + 1]
    raise AssertionError(f"unterminated definition: {signature}")


HARNESS = r"""
#include <cassert>
#include <cstdint>
#include <cstring>
#define __cdecl
struct Map { uint8_t climate, contour, val2, region, visibility, val3, unk_1;
             int8_t territory; uint32_t bit, bit2, bit_visible[7]; };
static_assert(sizeof(Map) == 0x2c, "fixture Map stride");
static Map *MapTiles;
static Map *map_tiles() { return MapTiles; }
static int MapLatitudeBounds, MapLongitudeBounds, MapLongitude;
static uint32_t MapArea;
static Map alternate_storage[1];
static Map *alternate_map = alternate_storage;
static int terrain_clear_calls, map_draw_calls;
static int change_bounds_on_callback;
static int MapSeaLevel;
static uint32_t AltNatural[11];
static int WorldClimateRedrawRequest;
static const uint32_t STATE_OMNISCIENT_VIEW = 1;
struct GameStateFields { uint32_t game_state_; };
struct GameState { GameStateFields fields_; };
static GameState GameDataStateStorage;
struct MapWin { void clear_terrain() { ++terrain_clear_calls; } };
static MapWin overview;
static MapWin *MapWinTable[1] = { &overview };
static int events[512], event_count, random_calls, random_result;
static int altitude_seen, altitude_calls, world_alt_calls, temp_calls, draw_calls;
static uint32_t rocky_after_flags;
static uint8_t rocky_after_climate;
static int minerals_at(int x, int y) { assert(x == 0 && y == 0); events[event_count++] = 1; return 7; }
static void rocky_set(int x, int y, int minerals) {
    assert(x == 0 && y == 0 && minerals == 7); events[event_count++] = 2;
    MapTiles = alternate_map;
    alternate_map[0].bit2 = rocky_after_flags;
    alternate_map[0].climate = rocky_after_climate;
}
static void bit_set(int x, int y, uint32_t bits, int on) {
    assert(x == 0 && y == 0 && on == 1); events[event_count++] = 4;
    MapTiles[0].bit |= bits;
}
static int rand() { ++random_calls; events[event_count++] = 3; return random_result; }
static void do_all_non_input() {
    events[event_count++] = 5;
    if (change_bounds_on_callback) {
        change_bounds_on_callback = 0;
        MapLatitudeBounds = 1;
    }
}
static void alt_set(int x, int y, int altitude) {
    assert(x == 0 && y == 0); altitude_seen = altitude; ++altitude_calls;
}
static void temp_set(int, int, int) { ++temp_calls; }
static void draw_tile(int, int, int) { ++draw_calls; }
static void world_alt_set(int, int, int, int) { ++world_alt_calls; }
static void draw_map(int) { ++map_draw_calls; }

struct Entry { int flag, second, first; };
struct Record { Entry entries_[2]; uint8_t active_; };
struct Lock {
    uint32_t field_E0_, field_E4_, field_E8_;
    Record records_[8];
    void reset_map();
    void clear();
};

// ACTUAL_WORLD_ROCKY
// ACTUAL_WORLD_SHORELINES
// ACTUAL_CLEAR_DIRTY
// ACTUAL_LOCK_RESET
// ACTUAL_LOCK_CLEAR

int main() {
    Map first[1] = {};
    MapTiles = first;
    MapLatitudeBounds = MapLongitudeBounds = MapLongitude = 1;
    first[0].climate = 0x60;
    first[0].bit2 = 1;
    world_rocky();
    assert(event_count == 1 && events[0] == 5); // early landmark guard
    MapTiles = first;
    first[0].bit2 = 0x04004000;
    event_count = 0;
    world_rocky();
    assert(event_count == 1 && events[0] == 5); // second signed high-byte guard
    MapTiles = first;
    first[0].bit2 = 0x09004000;
    rocky_after_flags = 0; rocky_after_climate = 0x20;
    event_count = 0;
    world_rocky();
    assert(event_count > 1); // threshold equality is not below the signed bound

    MapTiles = first;
    first[0].bit2 = 0;
    first[0].bit = 0x12345678;
    alternate_storage[0].bit = 0x12345678;
    rocky_after_flags = 4;
    rocky_after_climate = 0x60;
    random_result = 0;
    event_count = random_calls = 0;
    world_rocky();
    assert(event_count == 5);
    assert(events[0] == 1 && events[1] == 2 && events[2] == 3 && events[3] == 4);
    assert(random_calls == 1 && alternate_storage[0].bit == 0x12355678);
    assert(first[0].bit == 0x12345678); // callback changed the current map

    // River/sea skip, sea-flat, and land-rocky branches retain effects/order.
    event_count = random_calls = 0;
    MapTiles = first;
    rocky_after_flags = 0x84; rocky_after_climate = 0x20;
    world_rocky();
    assert(event_count == 3 && events[2] == 5 && random_calls == 0);
    event_count = random_calls = 0;
    MapTiles = first;
    rocky_after_flags = 0; rocky_after_climate = 0x20;
    world_rocky();
    assert(events[2] == 4);
    assert(alternate_storage[0].bit == 0x12355678);
    event_count = random_calls = 0;
    MapTiles = first;
    rocky_after_flags = 0; rocky_after_climate = 0xA0;
    world_rocky();
    assert(random_calls == 0 && alternate_storage[0].bit == 0x12375678);
    event_count = random_calls = 0;
    MapTiles = first;
    rocky_after_flags = 4; rocky_after_climate = 0xA0;
    random_result = 1;
    world_rocky();
    assert(random_calls == 1 && alternate_storage[0].bit == 0x12375678);

    Map shoreline[1] = {};
    MapTiles = shoreline;
    MapLatitudeBounds = MapLongitudeBounds = MapLongitude = 1;
    MapSeaLevel = 0;
    shoreline[0].contour = 0;
    shoreline[0].climate = 0;
    for (int i = 0; i < 11; ++i) AltNatural[i] = i * 10;
    for (int expected = 0; expected <= 6; ++expected) {
        shoreline[0].contour = static_cast<uint8_t>(expected * 10);
        altitude_calls = event_count = 0;
        world_shorelines();
        assert(altitude_calls == 1 && altitude_seen == expected);
        assert(event_count == 6); // first contour pass and five natural rows
    }
    shoreline[0].contour = 0; MapSeaLevel = 1; altitude_calls = 0;
    world_shorelines();
    assert(altitude_calls == 1 && altitude_seen == 0); // below minimum
    AltNatural[6] = 5; AltNatural[5] = 10; AltNatural[4] = 2;
    MapSeaLevel = 0; shoreline[0].contour = 6; altitude_calls = 0;
    world_shorelines();
    assert(altitude_seen == 6); // nonmonotonic table: first descending hit
    AltNatural[6] = 0x80000000u;
    shoreline[0].contour = 0; MapSeaLevel = 0; altitude_calls = 0;
    world_shorelines();
    assert(altitude_seen == 6); // unsigned storage, signed table comparison
    shoreline[0].contour = 0; MapSeaLevel = 1; altitude_calls = 0;
    world_shorelines();
    assert(altitude_seen == 6); // -1 >= INT32_MIN
    AltNatural[6] = 0x7fffffffu;
    altitude_calls = 0;
    world_shorelines();
    assert(altitude_seen == 0); // -1 < INT32_MAX
    for (int i = 0; i < 7; ++i) AltNatural[i] = i * 10;
    assert(draw_calls == 0 && temp_calls == 0 && terrain_clear_calls == 0);
    Map shoreline_bounds[2] = {};
    MapTiles = shoreline_bounds;
    MapLatitudeBounds = 2; MapLongitudeBounds = MapLongitude = 1;
    shoreline_bounds[0].contour = shoreline_bounds[1].contour = 1;
    altitude_calls = event_count = 0;
    change_bounds_on_callback = 1;
    world_shorelines();
    assert(altitude_calls == 1 && event_count == 6); // callback shrinks bound
    GameDataStateStorage.fields_.game_state_ = STATE_OMNISCIENT_VIEW;
    WorldClimateRedrawRequest = 1;
    MapTiles = shoreline;
    MapLatitudeBounds = MapLongitudeBounds = MapLongitude = 1;
    shoreline[0].contour = 60; shoreline[0].climate = 0xc0;
    int previous_draws = draw_calls;
    world_shorelines();
    assert(draw_calls == previous_draws + 1 && temp_calls > 0);
    assert(terrain_clear_calls == 1 && map_draw_calls == 1 && world_alt_calls == 1);

    Map dirty[4] = {};
    MapTiles = dirty;
    MapLatitudeBounds = 2; MapLongitudeBounds = 3; MapLongitude = 2;
    for (int i = 0; i < 4; ++i) {
        dirty[i].climate = static_cast<uint8_t>(10 + i);
        dirty[i].contour = static_cast<uint8_t>(20 + i);
        dirty[i].val3 = static_cast<uint8_t>(30 + i);
        dirty[i].bit = 0x100 + i;
        dirty[i].bit2 = 0x200 + i;
    }
    Map dirty_before[4];
    std::memcpy(dirty_before, dirty, sizeof dirty);
    clear_dirty();
    for (int i = 0; i < 4; ++i) {
        Map expected = dirty_before[i];
        if (i < 3) expected.bit2 |= 0x400000;
        assert(std::memcmp(&dirty[i], &expected, sizeof(Map)) == 0);
    }

    Map lock_tiles[3] = {};
    MapTiles = lock_tiles; MapArea = 3;
    for (int i = 0; i < 3; ++i) {
        lock_tiles[i].climate = static_cast<uint8_t>(0x10 + i);
        lock_tiles[i].contour = static_cast<uint8_t>(0x20 + i);
        lock_tiles[i].val2 = static_cast<uint8_t>(0x30 + i);
        lock_tiles[i].region = static_cast<uint8_t>(0x40 + i);
        lock_tiles[i].visibility = static_cast<uint8_t>(0x50 + i);
        lock_tiles[i].val3 = 0xff;
        lock_tiles[i].unk_1 = static_cast<uint8_t>(0x60 + i);
        lock_tiles[i].territory = static_cast<int8_t>(-i);
        lock_tiles[i].bit = 0x12340000 + i;
        lock_tiles[i].bit2 = 0x56780000 + i;
        for (int j = 0; j < 7; ++j) lock_tiles[i].bit_visible[j] = j + i;
    }
    Map reset_before[3];
    std::memcpy(reset_before, lock_tiles, sizeof lock_tiles);
    Lock reset_only = {};
    reset_only.reset_map();
    for (int i = 0; i < 3; ++i) {
        Map expected = reset_before[i]; expected.val3 &= 0xc7;
        assert(std::memcmp(&lock_tiles[i], &expected, sizeof(Map)) == 0);
    }
    lock_tiles[0].val3 = lock_tiles[1].val3 = lock_tiles[2].val3 = 0xff;
    Map clear_before[3];
    std::memcpy(clear_before, lock_tiles, sizeof lock_tiles);
    Lock lock = {};
    lock.field_E0_ = 1; lock.field_E4_ = 2; lock.field_E8_ = 3;
    for (int r = 0; r < 8; ++r) {
        lock.records_[r].active_ = 1;
        for (int e = 0; e < 2; ++e) {
            lock.records_[r].entries_[e].flag = 1;
            lock.records_[r].entries_[e].second = 2;
            lock.records_[r].entries_[e].first = 3;
        }
    }
    lock.clear();
    assert(lock.field_E0_ == 0 && lock.field_E4_ == 0 && lock.field_E8_ == 0);
    for (int r = 0; r < 8; ++r) {
        assert(lock.records_[r].active_ == 0);
        for (int e = 0; e < 2; ++e)
            assert(lock.records_[r].entries_[e].flag == 0 &&
                   lock.records_[r].entries_[e].second == -1 &&
                   lock.records_[r].entries_[e].first == -1);
    }
    for (int i = 0; i < 3; ++i) {
        Map expected = clear_before[i]; expected.val3 &= 0xc7;
        assert(std::memcmp(&lock_tiles[i], &expected, sizeof(Map)) == 0);
    }
    lock_tiles[0].val3 = 0xff;
    MapArea = 0;
    Map empty_count_before[3];
    std::memcpy(empty_count_before, lock_tiles, sizeof lock_tiles);
    lock.reset_map();
    assert(std::memcmp(lock_tiles, empty_count_before, sizeof lock_tiles) == 0);
    lock.clear();
    assert(std::memcmp(lock_tiles, empty_count_before, sizeof lock_tiles) == 0);
    MapArea = 0xffffffffu;
    lock.reset_map();
    lock.clear();
    assert(std::memcmp(lock_tiles, empty_count_before, sizeof lock_tiles) == 0);
}
"""


def actual_tu(mutant: str = "") -> str:
    map_text = MAP_SOURCE.read_text(encoding="utf-8")
    lock_text = LOCK_SOURCE.read_text(encoding="utf-8")
    replacements = {
        "// ACTUAL_WORLD_ROCKY": definition(map_text, "void __cdecl world_rocky()"),
        "// ACTUAL_WORLD_SHORELINES": definition(map_text, "void __cdecl world_shorelines()"),
        "// ACTUAL_CLEAR_DIRTY": definition(map_text, "void __cdecl clear_dirty()"),
        "// ACTUAL_LOCK_RESET": definition(lock_text, "void Lock::reset_map()"),
        "// ACTUAL_LOCK_CLEAR": definition(lock_text, "void Lock::clear()"),
    }
    if mutant == "wrong_field":
        replacements["// ACTUAL_WORLD_ROCKY"] = replacements[
            "// ACTUAL_WORLD_ROCKY"
        ].replace("uint32_t val3 = rec->bit2;", "uint32_t val3 = rec->bit;", 1)
    elif mutant == "no_reload":
        replacements["// ACTUAL_WORLD_ROCKY"] = replacements[
            "// ACTUAL_WORLD_ROCKY"
        ].replace(
            "Map *tile = &map_tiles()[MapLongitude * y + half_x];",
            "Map *tile = rec;", 1,
        )
    elif mutant == "dirty_mask":
        replacements["// ACTUAL_CLEAR_DIRTY"] = replacements[
            "// ACTUAL_CLEAR_DIRTY"
        ].replace("record->bit2 |= 0x400000;", "record->bit2 |= 0x200000;", 1)
    elif mutant == "unsigned_threshold":
        replacements["// ACTUAL_WORLD_SHORELINES"] = replacements[
            "// ACTUAL_WORLD_SHORELINES"
        ].replace("depth >= static_cast<int32_t>(AltNatural[natural_index])",
                  "static_cast<uint32_t>(depth) >= AltNatural[natural_index]", 1)
    tu = HARNESS
    for marker, body in replacements.items():
        assert tu.count(marker) == 1
        tu = tu.replace(marker, body)
    return tu


@pytest.mark.parametrize("compiler", ("g++", "clang++"))
def test_actual_typed_map_and_lock_bodies(compiler, tmp_path):
    tool = shutil.which(compiler)
    if tool is None:
        pytest.skip(f"{compiler} unavailable")
    cpp = tmp_path / "map_lock_typed_actual.cpp"
    binary = tmp_path / "map_lock_typed_actual"
    cpp.write_text(actual_tu(), encoding="utf-8")
    built = subprocess.run([tool, "-std=c++11", "-Wall", "-Wextra", str(cpp),
                            "-o", str(binary)], check=False,
                           capture_output=True, text=True, timeout=45)
    assert built.returncode == 0, built.stdout + built.stderr
    ran = subprocess.run([str(binary)], check=False, capture_output=True,
                         text=True, timeout=15)
    assert ran.returncode == 0, ran.stdout + ran.stderr


def test_typed_member_and_original_order_source_controls():
    rocky = definition(MAP_SOURCE.read_text(encoding="utf-8"),
                       "void __cdecl world_rocky()")
    shore = definition(MAP_SOURCE.read_text(encoding="utf-8"),
                       "void __cdecl world_shorelines()")
    dirty = definition(MAP_SOURCE.read_text(encoding="utf-8"),
                       "void __cdecl clear_dirty()")
    lock = LOCK_SOURCE.read_text(encoding="utf-8")
    assert "rec->bit2" in rocky and "tile->bit2" in rocky
    assert "static_cast<int>(hi) < 0x9000000" in rocky
    assert rocky.index("minerals_at(x, y)") < rocky.index("rocky_set(x, y, minerals)")
    assert rocky.index("rocky_set(x, y, minerals)") < rocky.index("Map *tile = &map_tiles()")
    assert "tile->contour - MapSeaLevel" in shore
    assert "tile->climate >> 5" in shore
    assert "depth >= static_cast<int32_t>(AltNatural[natural_index])" in shore
    assert "natural_index >= 0" in shore
    assert "threshold--" not in shore and "reinterpret_cast<int>(threshold)" not in shore
    assert "record->bit2 |= 0x400000" in dirty and "++record;" in dirty
    assert "table[index].val3 &= 0xC7;" in lock
    assert "map_tile_bytes()" not in rocky + shore + dirty + lock


@pytest.mark.parametrize("compiler", ("g++", "clang++"))
@pytest.mark.parametrize("mutant", ("wrong_field", "no_reload", "dirty_mask",
                                     "unsigned_threshold"))
def test_actual_source_mutants_fail_member_and_mask_controls(compiler, mutant, tmp_path):
    tool = shutil.which(compiler)
    if tool is None:
        pytest.skip(f"{compiler} unavailable")
    cpp = tmp_path / f"map_lock_{mutant}.cpp"
    binary = tmp_path / f"map_lock_{mutant}"
    cpp.write_text(actual_tu(mutant), encoding="utf-8")
    built = subprocess.run([tool, "-std=c++11", "-Wall", "-Wextra", str(cpp),
                            "-o", str(binary)], check=False,
                           capture_output=True, text=True, timeout=45)
    assert built.returncode == 0, built.stdout + built.stderr
    ran = subprocess.run([str(binary)], check=False, capture_output=True,
                         text=True, timeout=15)
    assert ran.returncode != 0
