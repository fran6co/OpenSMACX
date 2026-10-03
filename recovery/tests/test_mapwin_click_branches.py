"""Source-extracted click branch controls for the private MapWin projection.

These tests compile unchanged balanced blocks from MapWin::click. They cover
the paint redraw split, per-slot callback reloads, cursor dispatch, and the
early Popup guard. They do not execute or approve the full click body, its
base/triad or visibility routes, nor its complete exception-cleanup graph.
"""

from pathlib import Path
import os
import shutil
import subprocess

import pytest


ROOT = Path(__file__).resolve().parents[2]


def balanced_block(text, start):
    opening = text.index("{", start)
    depth = 0
    for end in range(opening, len(text)):
        depth += (text[end] == "{") - (text[end] == "}")
        if depth == 0:
            return text[start:end + 1]
    raise AssertionError("unterminated source block")


def source_blocks():
    source = (ROOT / "src/mapwin.cpp").read_text()
    start = source.index("void MapWin::click(")
    end = source.index("\n}", start) + 2
    click = source[start:end]

    paint_start = click.index("if (field_1DD8C_ != 0)")
    paint = balanced_block(click, paint_start)
    cursor_start = click.index("if (cursor_mode != 0 && ConsoleGlobal.selected_vehicle_ >= 0")
    cursor = balanced_block(click, cursor_start)
    popup_decl, = [line.strip() for line in click.splitlines()
                   if line.strip() == "Popup popup;"]
    guard_start = click.index("if (field_1DD88_ != 0)")
    early_guard = balanced_block(click, guard_start)
    return popup_decl, early_guard, paint, cursor


HARNESS = r'''#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <vector>

static std::vector<int> events;
static void need(bool value, int line) {
    if (!value) { std::fprintf(stderr, "assertion at %d\n", line); std::exit(93); }
}
#define REQUIRE(x) need((x), __LINE__)
static void note(int event) { events.push_back(event); }

struct Popup { Popup() { note(90); } ~Popup() { note(91); } };
struct MapWin {
    int id, field_1DD8C_, field_1DD74_, field_1DD88_;
    explicit MapWin(int value = 0) : id(value), field_1DD8C_(2),
        field_1DD74_(1), field_1DD88_(0) {}
    void editor(int x, int y, int erase);
    void draw_map(int draw_type);
    void paint_branch(int tile_x, int tile_y);
    void early_guard();
    void cursor_dispatch(int cursor_mode, int tile_x, int tile_y);
};
static const std::size_t MapWinTableSlots = 8;
static MapWin *MapWinTable[MapWinTableSlots];
static int MapEditorPaintId;
static int mutate_in_climate, mutate_in_free_draw, mutate_in_member_draw;
static int paint_x, paint_y, paint_erase;
static std::vector<int> action_args;

static void world_climate() {
    note(20);
    if (mutate_in_climate) MapWinTable[1]->field_1DD74_ = 0;
}
static void draw_map(int draw_type) {
    REQUIRE(draw_type == 1);
    note(21);
    if (mutate_in_free_draw) MapWinTable[1]->field_1DD74_ = 0;
}
struct ConsoleMock {
    int selected_vehicle_;
    int field_23BE4_;
    void set_road_to(int veh, int x, int y, int order) {
        action_args = {1, veh, x, y, order}; note(1);
    }
    void set_bombing_run(int veh, int x, int y) {
        action_args = {6, veh, x, y}; note(6);
    }
};
static ConsoleMock ConsoleGlobal;
struct NetDaemonMock {
    void veh_drop(int veh, int x, int y) {
        action_args = {3, veh, x, y}; note(3);
    }
};
static NetDaemonMock NetDaemonNet;
static void action_patrol(int veh, int x, int y) {
    action_args = {4, veh, x, y}; note(4);
}
static void action_arty(int veh, int x, int y) {
    action_args = {5, veh, x, y}; note(5);
}

void MapWin::editor(int x, int y, int erase) {
    paint_x = x; paint_y = y; paint_erase = erase; note(10);
}
void MapWin::draw_map(int draw_type) {
    REQUIRE(draw_type == 1);
    note(30 + id);
    if (mutate_in_member_draw && id == 1) MapWinTable[2]->field_1DD74_ = 0;
}
void MapWin::paint_branch(int tile_x, int tile_y) {
    PAINT_BLOCK
}
void MapWin::early_guard() {
    POPUP_DECL
    EARLY_GUARD
    note(99);
}
void MapWin::cursor_dispatch(int cursor_mode, int tile_x, int tile_y) {
    CURSOR_BLOCK
    note(98);
}

static void reset() {
    events.clear(); action_args.clear();
    mutate_in_climate = mutate_in_free_draw = mutate_in_member_draw = 0;
    paint_x = paint_y = paint_erase = -1;
    MapEditorPaintId = 1;
    MapWinTable[0] = nullptr; MapWinTable[1] = nullptr; MapWinTable[2] = nullptr;
    ConsoleGlobal.selected_vehicle_ = 17; ConsoleGlobal.field_23BE4_ = -1;
}
static void expect(const std::vector<int> &actual, const std::vector<int> &wanted) {
    REQUIRE(actual == wanted);
}

static void paint_cases() {
    for (int special : {0, 2, 0x12}) {
        reset();
        MapWin primary(0), secondary(1), inactive(2);
        primary.field_1DD8C_ = 3;
        secondary.field_1DD74_ = 1;
        inactive.field_1DD74_ = 0;
        MapWinTable[0] = &primary; MapWinTable[1] = &secondary; MapWinTable[2] = &inactive;
        MapEditorPaintId = special;
        primary.paint_branch(12, 14);
        expect(events, {10, 20, 30, 31});
        REQUIRE(primary.field_1DD8C_ == 0);
        REQUIRE(paint_x == 12 && paint_y == 14 && paint_erase == 2);
    }
    reset();
    MapWin primary(0), secondary(1);
    MapWinTable[0] = &primary; MapWinTable[1] = &secondary;
    MapEditorPaintId = 1;
    primary.paint_branch(12, 14);
    // Native 0x46D97F jumps beyond the member loop: non-special IDs redraw
    // through the free function only. This exact trace rejects a double redraw.
    expect(events, {10, 21});

    // The free redraw is sequenced before the loop and can change a later
    // secondary's active flag; the loop reads that live state on its turn.
    reset();
    MapWin p(0), s(1);
    s.field_1DD74_ = 1;
    MapWinTable[0] = &p; MapWinTable[1] = &s;
    mutate_in_free_draw = 1;
    p.paint_branch(12, 14);
    expect(events, {10, 21});

    // A prior member callback can similarly deactivate a not-yet-visited slot.
    reset();
    MapWin p2(0), s2(1), t2(2);
    s2.field_1DD74_ = 1; t2.field_1DD74_ = 1;
    MapWinTable[0] = &p2; MapWinTable[1] = &s2; MapWinTable[2] = &t2;
    MapEditorPaintId = 0;
    mutate_in_member_draw = 1;
    p2.paint_branch(12, 14);
    expect(events, {10, 20, 30, 31});
}

static void cursor_cases() {
    const int modes[] = {2, 3, 4, 5, 6, 7, 8, 9, 10, 11};
    for (int mode : modes) {
        reset(); MapWin window;
        window.cursor_dispatch(mode, 12, 14);
        if (mode == 2) expect(action_args, {1, 17, 12, 14, 0x1b});
        else if (mode == 3) expect(action_args, {1, 17, 12, 14, 0x1c});
        else if (mode == 4) expect(action_args, {3, 17, 12, 14});
        else if (mode == 8) expect(action_args, {4, 17, 12, 14});
        else if (mode == 10) expect(action_args, {5, 17, 12, 14});
        else if (mode == 11) expect(action_args, {6, 17, 12, 14});
        else REQUIRE(action_args.empty());
        REQUIRE(ConsoleGlobal.field_23BE4_ == 0);
        REQUIRE(events.empty() || events.back() != 98);
    }
    reset(); MapWin window; ConsoleGlobal.selected_vehicle_ = -1;
    window.cursor_dispatch(2, 12, 14);
    expect(events, {98});
    REQUIRE(action_args.empty());
}

static void popup_cleanup() {
    reset(); MapWin early; early.field_1DD88_ = 4;
    early.early_guard(); expect(events, {90, 91});
    REQUIRE(early.field_1DD88_ == 0);
    reset(); MapWin normal; normal.field_1DD88_ = 0;
    normal.early_guard(); expect(events, {90, 99, 91});
}

int main() { paint_cases(); cursor_cases(); popup_cleanup(); }
'''


@pytest.mark.parametrize("compiler", ["g++", "clang++"])
def test_actual_click_branch_controls(compiler, tmp_path):
    executable = shutil.which(compiler)
    if executable is None:
        pytest.skip(f"{compiler} is not installed")
    popup_decl, early_guard, paint, cursor = source_blocks()
    source = (HARNESS.replace("POPUP_DECL", popup_decl)
              .replace("EARLY_GUARD", early_guard)
              .replace("PAINT_BLOCK", paint)
              .replace("CURSOR_BLOCK", cursor))
    cpp = tmp_path / "click_branches.cpp"
    binary = tmp_path / "click_branches"
    cpp.write_text(source)
    subprocess.run([executable, "-std=c++11", "-Wall", "-Wextra", "-Werror",
                    str(cpp), "-o", str(binary)], check=True, timeout=30)
    subprocess.run([str(binary)], check=True, timeout=30)
