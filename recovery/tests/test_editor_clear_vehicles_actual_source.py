"""Host controls for the installed, typed editor_clear_vehicles body.

This extracts the method and Map declaration verbatim from src/. It deliberately
refuses the earlier raw-pointer body; candidate snippets are not substituted.
"""
from __future__ import annotations

import os
import re
import shutil
import subprocess
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]


def _block(text: str, start: int) -> str:
    opening = text.index("{", start)
    depth = 0
    for pos in range(opening, len(text)):
        if text[pos] == "{":
            depth += 1
        elif text[pos] == "}":
            depth -= 1
            if depth == 0:
                return text[opening : pos + 1]
    raise AssertionError("unterminated C++ block")


def _installed_sources() -> tuple[str, str]:
    console = (ROOT / "src/console.cpp").read_text()
    marker = "void Console::editor_clear_vehicles()"
    start = console.index(marker)
    body = _block(console, start)
    map_header = (ROOT / "src/map.h").read_text()
    map_marker = "struct Map {"
    map_start = map_header.index(map_marker)
    map_decl = map_marker[:-1] + _block(map_header, map_start)

    assert "reinterpret_cast" not in body and "map_tile_bytes" not in body, (
        "installed method is still the raw-pointer seed; refusing to test a substitute"
    )
    assert "Map *tile = MapTiles" in body, "expected the owner-backed Map snapshot"
    assert "tile->bit" in body and "tile->bit_visible" in body
    assert "static_cast<int>(MapArea)" in body, "signed MapArea behavior must be explicit"
    assert "++tile" in body or "tile++" in body, "typed iteration must advance Map records"
    assert "sizeof(Map)" not in body, "walk should use the typed Map pointer"
    return map_decl, f"void Console::editor_clear_vehicles() {body}"


HARNESS = r'''
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <vector>
using std::int8_t; using std::uint8_t; using std::uint32_t;

struct Map { MAP_DECL };
static_assert(sizeof(Map) == 0x2c, "source Map stride changed");
static_assert(offsetof(Map, bit) == 8, "Map::bit offset");
static_assert(offsetof(Map, bit2) == 12, "Map::bit2 offset");
static_assert(offsetof(Map, bit_visible) == 16, "Map::bit_visible offset");

Map *MapTiles = nullptr;
uint32_t MapArea = 0;
struct GameFields { uint32_t game_state_; int vehicle_current_count_; int base_current_count_; };
struct GameStorage { GameFields fields_; };
GameStorage GameDataStateStorage{};
struct ConsoleFields { int cursor_active_; int selected_vehicle_; };
ConsoleFields ConsoleGlobal{};

static std::vector<int> events;
static Map *undo_tiles = nullptr;
static uint32_t undo_area = 0;
static uint32_t reset_bits[8]{};
static uint32_t draw_bits[8]{};
static int reset_vehicle_count = 0, reset_base_count = 0, reset_cursor = 0, reset_selected = 0;
static int draw_vehicle_count = 0, draw_base_count = 0, draw_cursor = 0, draw_selected = 0;
static uint32_t reset_game_state = 0, draw_game_state = 0;
static Map *reset_tiles = nullptr, *draw_tiles = nullptr;

void draw_map(int value) {
    assert(value == 1);
    events.push_back(3);
    draw_tiles = MapTiles;
    draw_game_state = GameDataStateStorage.fields_.game_state_;
    draw_vehicle_count = GameDataStateStorage.fields_.vehicle_current_count_;
    draw_base_count = GameDataStateStorage.fields_.base_current_count_;
    draw_cursor = ConsoleGlobal.cursor_active_;
    draw_selected = ConsoleGlobal.selected_vehicle_;
    if (MapArea <= 8 && MapTiles) {
        for (uint32_t i = 0; i < MapArea; ++i) draw_bits[i] = MapTiles[i].bit;
    }
}

struct Console {
    void auto_undo() {
        events.push_back(1);
        MapTiles = undo_tiles;
        MapArea = undo_area;
    }
    void reset_territory() {
        events.push_back(2);
        reset_game_state = GameDataStateStorage.fields_.game_state_;
        reset_vehicle_count = GameDataStateStorage.fields_.vehicle_current_count_;
        reset_base_count = GameDataStateStorage.fields_.base_current_count_;
        reset_cursor = ConsoleGlobal.cursor_active_;
        reset_selected = ConsoleGlobal.selected_vehicle_;
        reset_tiles = MapTiles;
        if (MapArea <= 8 && MapTiles) {
            for (uint32_t i = 0; i < MapArea; ++i) reset_bits[i] = MapTiles[i].bit;
        }
    }
    void editor_clear_vehicles();
};

void Console::editor_clear_vehicles() BODY

static Map make_tile(uint32_t bit, uint32_t bit2, uint32_t seed) {
    Map m{};
    m.climate = static_cast<uint8_t>(seed);
    m.contour = static_cast<uint8_t>(seed + 1);
    m.val2 = static_cast<uint8_t>(seed + 2);
    m.region = static_cast<uint8_t>(seed + 3);
    m.visibility = static_cast<uint8_t>(seed + 4);
    m.val3 = static_cast<uint8_t>(seed + 5);
    m.unk_1 = static_cast<uint8_t>(seed + 6);
    m.territory = static_cast<int8_t>(seed);
    m.bit = bit;
    m.bit2 = bit2;
    for (int i = 0; i != 7; ++i) m.bit_visible[i] = seed * 100 + i;
    return m;
}

static void reset_observers() {
    events.clear();
    reset_tiles = draw_tiles = nullptr;
    for (int i = 0; i != 8; ++i) reset_bits[i] = draw_bits[i] = 0;
    reset_vehicle_count = reset_base_count = reset_cursor = reset_selected = -999;
    draw_vehicle_count = draw_base_count = draw_cursor = draw_selected = -999;
    reset_game_state = draw_game_state = 0;
}

static void test_auto_undo_snapshot_and_full_records() {
    Map stale[2] = {make_tile(0x11111111, 0xA1A1A1A1, 10), make_tile(0x22222222, 0xA2A2A2A2, 11)};
    Map live[4] = {
        make_tile(0xFFFFFFFF, 0xB1B1B1B1, 21),
        make_tile(0x1234567B, 0xB2B2B2B2, 31),
        make_tile(0x80000003, 0xB3B3B3B3, 41),
        make_tile(0xCAFEBABE, 0xB4B4B4B4, 51),
    };
    const Map before[4] = {live[0], live[1], live[2], live[3]};
    MapTiles = stale; MapArea = 2;
    undo_tiles = live; undo_area = 3;
    GameDataStateStorage.fields_ = {0x20, 17, 9};
    ConsoleGlobal = {1, 42};
    reset_observers();
    Console c;
    c.editor_clear_vehicles();

    assert((events == std::vector<int>{1, 2, 3}));
    assert(MapTiles == live);
    assert(GameDataStateStorage.fields_.game_state_ == 0x24);
    assert(GameDataStateStorage.fields_.vehicle_current_count_ == 0);
    assert(GameDataStateStorage.fields_.base_current_count_ == 0);
    assert(ConsoleGlobal.cursor_active_ == 0 && ConsoleGlobal.selected_vehicle_ == -1);
    assert(reset_tiles == live && draw_tiles == live);
    assert(reset_game_state == 0x24 && draw_game_state == 0x24);
    assert(reset_vehicle_count == 0 && draw_vehicle_count == 0);
    assert(reset_base_count == 0 && draw_base_count == 0);
    assert(reset_cursor == 0 && draw_cursor == 0);
    assert(reset_selected == -1 && draw_selected == -1);
    for (int i = 0; i != 3; ++i) {
        const uint32_t expected = before[i].bit & 0xFFFFFFFCu;
        assert(live[i].bit == expected);
        assert(live[i].bit2 == before[i].bit2);
        assert(live[i].climate == before[i].climate && live[i].contour == before[i].contour);
        assert(live[i].val2 == before[i].val2 && live[i].region == before[i].region);
        assert(live[i].visibility == before[i].visibility && live[i].val3 == before[i].val3);
        assert(live[i].unk_1 == before[i].unk_1 && live[i].territory == before[i].territory);
        for (int j = 0; j != 7; ++j) assert(live[i].bit_visible[j] == expected);
        assert(reset_bits[i] == expected && draw_bits[i] == expected);
    }
    assert(live[3].bit == before[3].bit && live[3].bit2 == before[3].bit2);
    for (int j = 0; j != 7; ++j) assert(live[3].bit_visible[j] == before[3].bit_visible[j]);
    assert(stale[0].bit == 0x11111111 && stale[1].bit == 0x22222222);
}

static void test_empty_and_signed_high_bit_bounds() {
    Map one = make_tile(0xFFFFFFFF, 0x1234, 8);
    for (uint32_t area : {0u, 0x80000000u}) {
        MapTiles = &one; MapArea = area;
        undo_tiles = &one; undo_area = area;
        const Map before = one;
        GameDataStateStorage.fields_ = {0, 3, 4};
        ConsoleGlobal = {1, 9};
        reset_observers();
        Console c;
        c.editor_clear_vehicles();
        assert((events == std::vector<int>{1, 2, 3}));
        assert(one.bit == before.bit && one.bit2 == before.bit2);
        for (int i = 0; i != 7; ++i) assert(one.bit_visible[i] == before.bit_visible[i]);
        assert(GameDataStateStorage.fields_.vehicle_current_count_ == 0);
        assert(ConsoleGlobal.cursor_active_ == 0 && ConsoleGlobal.selected_vehicle_ == -1);
    }
}

int main() {
    test_auto_undo_snapshot_and_full_records();
    test_empty_and_signed_high_bit_bounds();
}
'''


def test_actual_source_body_with_gcc_and_clang(tmp_path: Path) -> None:
    map_decl, method = _installed_sources()
    harness = HARNESS.replace("MAP_DECL", map_decl[len("struct Map {") : -1]).replace("BODY", method[len("void Console::editor_clear_vehicles() ") :])
    path = tmp_path / "editor_clear_vehicles.cpp"
    path.write_text(harness)
    compilers = [shutil.which(name) for name in ("g++", "clang++")]
    assert all(compilers), f"required host compilers unavailable: {compilers}"
    for compiler in compilers:
        exe = tmp_path / (Path(compiler).name + "-controls")
        subprocess.run(
            [compiler, "-std=c++11", "-O1", "-g", "-Wall", "-Wextra", "-Werror",
             "-fsanitize=address,undefined", "-fno-omit-frame-pointer", str(path), "-o", str(exe)],
            check=True,
        )
        subprocess.run([str(exe)], check=True, env={**os.environ, "ASAN_OPTIONS": "detect_leaks=0"})
