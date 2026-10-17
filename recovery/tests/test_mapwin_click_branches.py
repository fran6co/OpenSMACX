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
    cursor_start = click.index("if (cursor_mode != 0", click.index("if (cursor_sprite_1)"))
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
    int id, field_1DD8C_, view_active_, field_1DD88_;
    explicit MapWin(int value = 0) : id(value), field_1DD8C_(2),
        view_active_(1), field_1DD88_(0) {}
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
    if (mutate_in_climate) MapWinTable[1]->view_active_ = 0;
}
static void draw_map(int draw_type) {
    REQUIRE(draw_type == 1);
    note(21);
    if (mutate_in_free_draw) MapWinTable[1]->view_active_ = 0;
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
    if (mutate_in_member_draw && id == 1) MapWinTable[2]->view_active_ = 0;
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
        secondary.view_active_ = 1;
        inactive.view_active_ = 0;
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
    s.view_active_ = 1;
    MapWinTable[0] = &p; MapWinTable[1] = &s;
    mutate_in_free_draw = 1;
    p.paint_branch(12, 14);
    expect(events, {10, 21});

    // A prior member callback can similarly deactivate a not-yet-visited slot.
    reset();
    MapWin p2(0), s2(1), t2(2);
    s2.view_active_ = 1; t2.view_active_ = 1;
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
    // Original46E136 takes local cleanup and returns, without the order reset.
    expect(events, {});
    REQUIRE(action_args.empty());
    REQUIRE(ConsoleGlobal.field_23BE4_ == -1);
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


CURSOR_HARNESS = r'''#include <climits>
#include <cstdio>
#include <cstdlib>
#include <vector>

static std::vector<int> events;
static std::vector<int> action_args;
static int *mutate_reset_target;
static void need(bool value, const char *label, int mode, int vehicle) {
    if (!value) {
        std::fprintf(stderr, "FAIL: %s mode=%d vehicle=%d\n", label, mode, vehicle);
        std::exit(1);
    }
}
static void note(int value) { events.push_back(value); }
struct Popup { Popup() { note(90); } ~Popup() { note(91); } };
struct MapWin {
    void cursor_dispatch(int cursor_mode, int tile_x, int tile_y);
};
struct ConsoleMock {
    int selected_vehicle_;
    int field_23BE4_;
    void mutate() { if (mutate_reset_target) *mutate_reset_target = 77; }
    void set_road_to(int v, int x, int y, int order) {
        action_args = {1, v, x, y, order}; note(1); mutate();
    }
    void set_bombing_run(int v, int x, int y) {
        action_args = {6, v, x, y}; note(6); mutate();
    }
};
static ConsoleMock ConsoleGlobal;
struct NetDaemonMock {
    void veh_drop(int v, int x, int y) {
        action_args = {3, v, x, y}; note(3);
        if (mutate_reset_target) *mutate_reset_target = 77;
    }
};
static NetDaemonMock NetDaemonNet;
static void action_patrol(int v, int x, int y) {
    action_args = {4, v, x, y}; note(4);
    if (mutate_reset_target) *mutate_reset_target = 77;
}
static void action_arty(int v, int x, int y) {
    action_args = {5, v, x, y}; note(5);
    if (mutate_reset_target) *mutate_reset_target = 77;
}

void MapWin::cursor_dispatch(int cursor_mode, int tile_x, int tile_y) {
    POPUP_DECL
    CURSOR_BLOCK
    note(98); // Observable continuation after the extracted cursor-family block.
}

static bool same(const std::vector<int> &a, const std::vector<int> &b) {
    return a == b;
}
static void run_case(int mode, int vehicle) {
    events.clear(); action_args.clear();
    ConsoleGlobal.selected_vehicle_ = vehicle;
    ConsoleGlobal.field_23BE4_ = 0x13579;
    mutate_reset_target = &ConsoleGlobal.field_23BE4_;
    MapWin window;
    window.cursor_dispatch(mode, 12, 14);
    mutate_reset_target = 0;

    if (mode == 0) {
        need(action_args.empty(), "zero mode does not dispatch", mode, vehicle);
        need(ConsoleGlobal.field_23BE4_ == 0x13579,
             "zero mode does not reset order state", mode, vehicle);
        need(same(events, std::vector<int>{90, 98, 91}),
             "zero mode continues and closes popup", mode, vehicle);
        return;
    }
    if (vehicle < 0) {
        need(action_args.empty(), "negative vehicle performs cleanup only", mode, vehicle);
        need(ConsoleGlobal.field_23BE4_ == 0x13579,
             "negative vehicle does not reset order state", mode, vehicle);
        need(same(events, std::vector<int>{90, 91}),
             "negative vehicle exits through popup cleanup", mode, vehicle);
        return;
    }

    std::vector<int> wanted;
    int event = 0;
    switch (mode) {
    case 2: wanted = {1, vehicle, 12, 14, 0x1B}; event = 1; break;
    case 3: wanted = {1, vehicle, 12, 14, 0x1C}; event = 1; break;
    case 4: wanted = {3, vehicle, 12, 14}; event = 3; break;
    case 8: wanted = {4, vehicle, 12, 14}; event = 4; break;
    case 10: wanted = {5, vehicle, 12, 14}; event = 5; break;
    case 11: wanted = {6, vehicle, 12, 14}; event = 6; break;
    default: break;
    }
    need(same(action_args, wanted), "mode dispatch/table arguments", mode, vehicle);
    need(ConsoleGlobal.field_23BE4_ == 0,
         "nonzero nonnegative mode resets order state", mode, vehicle);
    if (event) {
        need(same(events, std::vector<int>{90, event, 91}),
             "action occurs within popup lifetime", mode, vehicle);
    } else {
        need(same(events, std::vector<int>{90, 91}),
             "unmapped nonzero mode resets then exits", mode, vehicle);
    }
}

int main() {
    const int modes[] = {INT_MIN, -23, -1, 0, 1, 2, 3, 4, 5, 6, 7, 8,
                         9, 10, 11, 12, INT_MAX};
    const int vehicles[] = {0, 17, INT_MIN, -23, -1};
    for (int mode : modes)
        for (int vehicle : vehicles)
            run_case(mode, vehicle);
}
'''


def _compile_run(tmp_path: Path, compiler: str, cursor: str, tag: str):
    popup, _, _, _ = source_blocks()
    fixture = CURSOR_HARNESS.replace("POPUP_DECL", popup).replace("CURSOR_BLOCK", cursor)
    cpp = tmp_path / f"mapwin_cursor_{tag}.cpp"
    binary = tmp_path / f"mapwin_cursor_{tag}"
    cpp.write_text(fixture, encoding="utf-8")
    built = subprocess.run(
        [compiler, "-std=c++11", "-Wall", "-Wextra", "-Werror",
         str(cpp), "-o", str(binary)], text=True, capture_output=True, timeout=30)
    assert built.returncode == 0, f"{tag} must compile before behavior is judged:\n{built.stderr}"
    return subprocess.run([str(binary)], text=True, capture_output=True, timeout=30)


@pytest.mark.parametrize("compiler_name", ["g++", "clang++"])
def test_live_cursor_branch_matches_expanded_gate_and_cleanup_matrix(
        compiler_name, tmp_path):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(f"{compiler_name} unavailable")
    _, _, _, cursor = source_blocks()
    result = _compile_run(tmp_path, compiler, cursor, "actual")
    assert result.returncode == 0, result.stderr


def _replace_once(text: str, old: str, new: str, name: str) -> str:
    assert old in text, f"live source no longer has {name} mutation anchor"
    return text.replace(old, new, 1)


@pytest.mark.parametrize("compiler_name", ["g++", "clang++"])
@pytest.mark.parametrize("damage", [
    "old_range_guard",
    "no_negative_vehicle_return",
    "wrong_table_interpretation",
    "reset_on_negative_vehicle",
    "dropped_reset",
    "falls_through_after_reset",
])
def test_compiling_cursor_branch_mutants_fail_named_witnesses(
        compiler_name, damage, tmp_path):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(f"{compiler_name} unavailable")
    _, _, _, actual = source_blocks()
    mutations = {
        "old_range_guard": (
            "if (cursor_mode != 0) {",
            "if (cursor_mode != 0 && ConsoleGlobal.selected_vehicle_ >= 0 && "
            "cursor_mode >= 2 && cursor_mode <= 11) {"),
        "no_negative_vehicle_return": (
            "if (ConsoleGlobal.selected_vehicle_ < 0) return;", ""),
        "reset_on_negative_vehicle": (
            "if (ConsoleGlobal.selected_vehicle_ < 0) return;",
            "if (ConsoleGlobal.selected_vehicle_ < 0) { "
            "ConsoleGlobal.field_23BE4_ = 0; return; }"),
        "dropped_reset": ("ConsoleGlobal.field_23BE4_ = 0;", ";"),
        "falls_through_after_reset": (
            "ConsoleGlobal.field_23BE4_ = 0;\n        return;",
            "ConsoleGlobal.field_23BE4_ = 0;"),
    }
    if damage == "wrong_table_interpretation":
        veh_drop = ("NetDaemonNet.veh_drop(ConsoleGlobal.selected_vehicle_,\n"
                    "                                  tile_x, tile_y);")
        patrol = "action_patrol(ConsoleGlobal.selected_vehicle_, tile_x, tile_y);"
        mutant = _replace_once(actual, veh_drop, "WRONG_CASE_FOUR_ACTION;", damage)
        mutant = _replace_once(mutant, patrol, veh_drop, damage)
        mutant = mutant.replace("WRONG_CASE_FOUR_ACTION;", patrol, 1)
    else:
        before, after = mutations[damage]
        mutant = _replace_once(actual, before, after, damage)
    result = _compile_run(tmp_path, compiler, mutant, damage)
    assert result.returncode != 0, f"behavior witness missed {damage}"
    witnesses = {
        "old_range_guard": "nonzero nonnegative mode resets order state",
        "no_negative_vehicle_return": "negative vehicle does not reset order state",
        "wrong_table_interpretation": "mode dispatch/table arguments",
        "reset_on_negative_vehicle": "negative vehicle does not reset order state",
        "dropped_reset": "nonzero nonnegative mode resets order state",
        "falls_through_after_reset": "unmapped nonzero mode resets then exits",
    }
    assert "FAIL: " + witnesses[damage] in result.stderr, (damage, result.stdout, result.stderr)
