"""Actual-source controls for Console::editor_fungus's prompt and setup paths.

The branch expectations follow the pinned 0x004E13E0 decode: the first pop's
signed negative branch aborts; for a positive first answer the second pop's
negative branch aborts and its zero branch skips setup; the live loop bound is
reloaded at 0x004E1463. The fixture executes the product body with narrow state
and callback spies; it is not ABI or full-game runtime evidence.
"""
from pathlib import Path
import shutil
import subprocess

import pytest


ROOT = Path(__file__).resolve().parents[2]
SOURCE = ROOT / "src/console_commands.cpp"
SIGNATURE = "void Console::editor_fungus() {"


def source_body() -> str:
    source = SOURCE.read_text(encoding="utf-8")
    start = source.index(SIGNATURE)
    opening = source.index("{", start)
    depth = 0
    for pos in range(opening, len(source)):
        if source[pos] == "{":
            depth += 1
        elif source[pos] == "}":
            depth -= 1
            if depth == 0:
                return source[start:pos + 1]
    raise AssertionError("unterminated Console::editor_fungus body")


HARNESS = r"""
#include <cassert>
#include <stdint.h>
#include <cstring>

enum { E_UNDO=1, E_FIRST, E_SECOND, E_MAP_TILES, E_WORLD_FUNGUS, E_DRAW };
static int events[64], event_count;
static int answer_values[2], answer_count, answer_index;
static int map_tiles_calls, visible_writes, world_calls, draw_calls;
static int world_saw_native_life, draw_saw_native_life, draw_argument;
static int mutate_area;
static int MapArea;
static uint32_t MapNativeLifeForms;
static const char ConsoleStrFungosity[] = "FUNGOSITY";
static const char ConsoleStrFungmotize[] = "FUNGMOTIZE";

static void event(int value) {
    assert(event_count < static_cast<int>(sizeof(events) / sizeof(events[0])));
    events[event_count++] = value;
}
static void expect_events(const int *expected, int count) {
    assert(event_count == count);
    for (int i = 0; i < count; ++i) assert(events[i] == expected[i]);
}
static void auto_undo() { event(E_UNDO); }
static int x_pop_short(const char *prompt, int width, int flags) {
    assert(width == 0x40 && flags == 0 && answer_index < answer_count);
    if (prompt == ConsoleStrFungosity) event(E_FIRST);
    else {
        assert(prompt == ConsoleStrFungmotize);
        event(E_SECOND);
    }
    return answer_values[answer_index++];
}

struct Map;
struct VisibleBit {
    uint32_t value;
    int tile_index;
    int slot;
    VisibleBit &operator=(uint32_t bits);
};
struct Map {
    uint32_t leading_words_[2];
    uint32_t bit;
    VisibleBit bit_visible[8];
};
static Map map_storage[8];

VisibleBit &VisibleBit::operator=(uint32_t bits) {
    value = bits;
    ++visible_writes;
    if (mutate_area && slot == 6) {
        if (tile_index == 0) MapArea = 3;
        else if (tile_index == 1) MapArea = 2;
    }
    return *this;
}

static Map *map_tiles() {
    ++map_tiles_calls;
    event(E_MAP_TILES);
    return map_storage;
}
static void world_fungus() {
    ++world_calls;
    world_saw_native_life = static_cast<int>(MapNativeLifeForms);
    event(E_WORLD_FUNGUS);
    MapNativeLifeForms = 0xFEEDu;
}
static void draw_map(int value) {
    ++draw_calls;
    draw_argument = value;
    draw_saw_native_life = static_cast<int>(MapNativeLifeForms);
    event(E_DRAW);
}
struct Console {
    int faction_id_;
    void editor_fungus();
};

/* PRODUCT_BODY */

static void reset(int first, int second, int count) {
    event_count = answer_index = 0;
    answer_count = count;
    answer_values[0] = first;
    answer_values[1] = second;
    map_tiles_calls = visible_writes = world_calls = draw_calls = 0;
    world_saw_native_life = draw_saw_native_life = draw_argument = -1;
    mutate_area = 0;
    MapArea = 0;
    MapNativeLifeForms = 0;
    std::memset(map_storage, 0, sizeof(map_storage));
    for (int tile = 0; tile < 8; ++tile) {
        for (int slot = 0; slot < 8; ++slot) {
            map_storage[tile].bit_visible[slot].tile_index = tile;
            map_storage[tile].bit_visible[slot].slot = slot;
            map_storage[tile].bit_visible[slot].value = 0xA5000000u + slot;
        }
    }
}

int main() {
    int expected[8];
    Console console;

    // Pinned 0x4E13FC signed jl: first negative aborts after undo + one prompt.
    reset(-1, 0, 1);
    console.faction_id_ = 51;
    MapArea = 4;
    MapNativeLifeForms = 77;
    console.editor_fungus();
    expected[0] = E_UNDO; expected[1] = E_FIRST;
    expect_events(expected, 2);
    assert(answer_index == 1 && map_tiles_calls == 0 && visible_writes == 0);
    assert(world_calls == 0 && draw_calls == 0 && MapNativeLifeForms == 77);
    assert(console.faction_id_ == 51);

    // A zero first answer seeds confirmed=-1, so it takes setup without reprompt.
    // Nonpositive area still has the original MapTiles load before the guard.
    reset(0, 0, 1);
    console.faction_id_ = 51;
    MapArea = -3;
    MapNativeLifeForms = 42;
    console.editor_fungus();
    expected[0] = E_UNDO; expected[1] = E_FIRST;
    expected[2] = E_MAP_TILES; expected[3] = E_DRAW;
    expect_events(expected, 4);
    assert(answer_index == 1 && map_tiles_calls == 1 && visible_writes == 0);
    assert(world_calls == 0 && draw_calls == 1 && draw_argument == 1);
    assert(MapNativeLifeForms == 42 && draw_saw_native_life == 42);

    // Pinned 0x4E1419 signed jl: negative confirmation aborts before setup/draw.
    reset(1, -1, 2);
    console.faction_id_ = 63;
    MapArea = 2;
    MapNativeLifeForms = 55;
    console.editor_fungus();
    expected[0] = E_UNDO; expected[1] = E_FIRST; expected[2] = E_SECOND;
    expect_events(expected, 3);
    assert(answer_index == 2 && map_tiles_calls == 0 && visible_writes == 0);
    assert(world_calls == 0 && draw_calls == 0 && MapNativeLifeForms == 55);

    // Pinned 0x4E141B je: zero confirmation skips tile setup but still calls
    // world_fungus for the positive first answer, then restores before draw.
    reset(2, 0, 2);
    console.faction_id_ = 4;
    MapNativeLifeForms = 73;
    console.editor_fungus();
    expected[0] = E_UNDO; expected[1] = E_FIRST; expected[2] = E_SECOND;
    expected[3] = E_WORLD_FUNGUS; expected[4] = E_DRAW;
    expect_events(expected, 5);
    assert(map_tiles_calls == 0 && visible_writes == 0);
    assert(world_calls == 1 && world_saw_native_life == 1);
    assert(MapNativeLifeForms == 73 && draw_saw_native_life == 73);
    assert(draw_calls == 1 && draw_argument == 1);

    // Positive confirmation loads MapTiles before the area guard. The write
    // spy grows then shrinks MapArea at each tile's final visible-faction slot:
    // live signed-bound rereads visit exactly two tiles from initial_area=1.
    reset(2, 1, 2);
    console.faction_id_ = 8;
    MapArea = 1;
    MapNativeLifeForms = 88;
    map_storage[0].bit = 0xFFFFFFFFu;
    map_storage[1].bit = 0xA5u;
    map_storage[2].bit = 0x60u;
    mutate_area = 1;
    console.editor_fungus();
    expected[0] = E_UNDO; expected[1] = E_FIRST; expected[2] = E_SECOND;
    expected[3] = E_MAP_TILES; expected[4] = E_WORLD_FUNGUS;
    expected[5] = E_DRAW;
    expect_events(expected, 6);
    assert(answer_index == 2 && map_tiles_calls == 1 && MapArea == 2);
    assert(visible_writes == 14 && console.faction_id_ == 8);
    assert(map_storage[0].bit == 0xFFFFFFDFu && map_storage[1].bit == 0x85u);
    for (int slot = 0; slot < 7; ++slot) {
        assert(map_storage[0].bit_visible[slot].value == 0xFFFFFFDFu);
        assert(map_storage[1].bit_visible[slot].value == 0x85u);
    }
    assert(map_storage[0].bit_visible[7].value == 0xA5000007u);
    assert(map_storage[1].bit_visible[7].value == 0xA5000007u);
    assert(map_storage[2].bit == 0x60u && visible_writes == 14);
    assert(world_calls == 1 && world_saw_native_life == 1);
    assert(MapNativeLifeForms == 88 && draw_saw_native_life == 88);
    assert(draw_calls == 1 && draw_argument == 1);

    // Nonpositive area still performs the pre-guard tile load; the positive
    // answer's world pass, restoration, and draw remain reachable afterward.
    reset(1, 1, 2);
    console.faction_id_ = 2;
    MapArea = 0;
    MapNativeLifeForms = 91;
    console.editor_fungus();
    expected[0] = E_UNDO; expected[1] = E_FIRST; expected[2] = E_SECOND;
    expected[3] = E_MAP_TILES; expected[4] = E_WORLD_FUNGUS;
    expected[5] = E_DRAW;
    expect_events(expected, 6);
    assert(map_tiles_calls == 1 && visible_writes == 0);
    assert(world_calls == 1 && world_saw_native_life == 0);
    assert(MapNativeLifeForms == 91 && draw_saw_native_life == 91);
    assert(draw_calls == 1 && draw_argument == 1);
    return 0;
}
"""


def program_source(body: str, damage=None) -> str:
    if damage:
        before, after = damage
        assert body.count(before) == 1, f"mutation anchor missing: {before!r}"
        body = body.replace(before, after, 1)
    assert HARNESS.count("/* PRODUCT_BODY */") == 1
    return HARNESS.replace("/* PRODUCT_BODY */", body)


def run_fixture(tmp_path, compiler_name: str, *, damage=None, should_pass=True):
    compiler = shutil.which(compiler_name)
    if not compiler:
        pytest.skip(f"{compiler_name} unavailable")
    binary = tmp_path / ("console-fungus-" + compiler_name.replace("+", "p"))
    built = subprocess.run(
        [compiler, "-std=c++98", "-Wall", "-Wextra", "-Werror", "-x", "c++", "-",
         "-o", str(binary)],
        input=program_source(source_body(), damage), text=True, capture_output=True,
        timeout=30,
    )
    assert built.returncode == 0, built.stderr
    result = subprocess.run([str(binary)], capture_output=True, timeout=10)
    assert (result.returncode == 0) is should_pass, result.stderr.decode(errors="replace")


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
def test_console_fungus_actual_prompt_setup_restore_and_draw(compiler_name, tmp_path):
    run_fixture(tmp_path, compiler_name)


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
@pytest.mark.parametrize(("before", "after"), (
    ("while (i < MapArea)", "while (i < initial_area)"),
    ("if (answer >= 0)", "if (answer > 0)"),
    ("MapNativeLifeForms = saved;", "MapNativeLifeForms = saved + 1;"),
))
def test_console_fungus_actual_source_damage_mutants(
        compiler_name, tmp_path, before, after):
    run_fixture(tmp_path, compiler_name, damage=(before, after), should_pass=False)
