"""Actual-source controls for AlphaMenu hit-only outputs and capture ordering.

Uses the production AlphaMenu and Spot bodies with bounded host doubles; this
is not a native ABI, original-game runtime or full acceptance test.
"""
from __future__ import annotations

from pathlib import Path
import os
import shutil
import subprocess

import pytest


ROOT = Path(__file__).resolve().parents[2]


def source_body(path: Path, signature: str) -> str:
    text = path.read_text(encoding="utf-8")
    start = text.index(signature)
    opening = text.index("{", start)
    depth = 0
    for pos in range(opening, len(text)):
        if text[pos] == "{":
            depth += 1
        elif text[pos] == "}":
            depth -= 1
            if depth == 0:
                return text[start:pos + 1]
    raise AssertionError(f"unterminated source definition: {signature}")


HARNESS = r'''#include <cassert>
#include <string>
#include <vector>

struct RECT { int left, top, right, bottom; };

struct Spot {
    struct SpotInternal { RECT rect; int type; int position; };
    SpotInternal *spots_;
    int max_count_;
    int add_count_;
    int check(int x, int y, int *spot_pos, int *spot_type);
};
// __SPOT_CHECK_BODY__

struct AlphaMenu;
AlphaMenu *WinPointerOwner3 = 0;
AlphaMenu *WinPointerOwner4 = 0;
int MenuSelection = -99;

struct Trace {
    std::vector<std::string> events;
    int selection_at_base;
    int selection_at_redraw;
    int selection_at_update;
    AlphaMenu *owner3_at_lose;
    AlphaMenu *owner4_at_lose;
};

struct Menu {
    int field_A24_;
    Spot spot_;
    Trace *trace_;
    Menu(Trace *trace) : field_A24_(-1), spot_(), trace_(trace) {}
    void on_mouse_move(int, int);
};

struct AlphaMenu : Menu {
    explicit AlphaMenu(Trace *trace) : Menu(trace) {}
    void redraw();
    void on_update();
    void on_lose_mouse_capture();
    void on_mouse_move(int x, int y);
};

void Menu::on_mouse_move(int, int) {
    trace_->events.push_back("base");
    trace_->selection_at_base = MenuSelection;
}
void AlphaMenu::redraw() {
    trace_->events.push_back("redraw");
    trace_->selection_at_redraw = MenuSelection;
}
void AlphaMenu::on_update() {
    trace_->events.push_back("update");
    trace_->selection_at_update = MenuSelection;
}
void AlphaMenu::on_lose_mouse_capture() {
    trace_->events.push_back("lose");
    trace_->owner3_at_lose = WinPointerOwner3;
    trace_->owner4_at_lose = WinPointerOwner4;
}

// __ALPHA_MENU_BODY__

static void set_spots(AlphaMenu &menu, Spot::SpotInternal *spots, int count) {
    menu.spot_.spots_ = spots;
    menu.spot_.max_count_ = count;
    menu.spot_.add_count_ = count;
}

static void reset_trace(Trace &trace) {
    trace.events.clear();
    trace.selection_at_base = -99;
    trace.selection_at_redraw = -99;
    trace.selection_at_update = -99;
    trace.owner3_at_lose = 0;
    trace.owner4_at_lose = 0;
}

int main() {
    Trace trace = {{}, -99, -99, -99, 0, 0};
    AlphaMenu menu(&trace);
    Spot::SpotInternal entries[2] = {
        {{-10, -10, -1, -1}, 6, 39},
        {{20, 20, 30, 30}, 8, 71}
    };
    set_spots(menu, entries, 1);

    // A negative signed input coordinate is tested by the real Spot::check;
    // its hit writes the output position before AlphaMenu consumes it.
    menu.field_A24_ = -1;
    WinPointerOwner3 = 0;
    WinPointerOwner4 = &menu;
    MenuSelection = 12;
    reset_trace(trace);
    menu.on_mouse_move(-4, -7);
    assert(MenuSelection == 39);
    assert(trace.events == std::vector<std::string>({"redraw", "update"}));
    assert(trace.selection_at_redraw == 39 && trace.selection_at_update == 39);
    assert(WinPointerOwner3 == &menu && WinPointerOwner4 == 0);

    // Existing selected-menu state delegates to Menu first; the hit result is
    // then published before redraw/update callbacks.
    menu.field_A24_ = 3;
    WinPointerOwner3 = 0;
    WinPointerOwner4 = 0;
    MenuSelection = 5;
    reset_trace(trace);
    menu.on_mouse_move(-4, -7);
    assert(MenuSelection == 39);
    assert(trace.events == std::vector<std::string>({"base", "redraw", "update"}));
    assert(trace.selection_at_base == 5);
    assert(trace.selection_at_redraw == 39 && trace.selection_at_update == 39);

    // A miss leaves Spot's output unused, selects -1, redraws/updates, then
    // releases owner3 before the capture callback and clears owner4 afterward.
    menu.field_A24_ = 3;
    WinPointerOwner3 = &menu;
    WinPointerOwner4 = &menu;
    MenuSelection = 44;
    reset_trace(trace);
    menu.on_mouse_move(-4, -30);
    assert(MenuSelection == -1);
    assert(trace.events == std::vector<std::string>({"base", "redraw", "update", "lose"}));
    assert(trace.selection_at_redraw == -1 && trace.selection_at_update == -1);
    assert(trace.owner3_at_lose == 0 && trace.owner4_at_lose == &menu);
    assert(WinPointerOwner3 == 0 && WinPointerOwner4 == 0);

    // A different owner is preserved; this object's owner4 is still cleared,
    // and no loss callback is sent to an owner it does not hold.
    AlphaMenu other(&trace);
    WinPointerOwner3 = &other;
    WinPointerOwner4 = &menu;
    reset_trace(trace);
    menu.on_mouse_move(-4, -30);
    assert(trace.events == std::vector<std::string>({"base", "redraw", "update"}));
    assert(WinPointerOwner3 == &other && WinPointerOwner4 == 0);

    // Direct output controls establish Spot's writes on a negative-y hit and
    // untouched output slots on a miss, using the actual production body.
    Spot direct = {entries, 2, 2};
    int y = -7;
    int type = -1;
    assert(direct.check(-4, y, &y, &type) == 0);
    assert(y == 39 && type == 6);
    int untouched_position = 1234;
    int untouched_type = 5678;
    assert(direct.check(-4, -30, &untouched_position, &untouched_type) == -1);
    assert(untouched_position == 1234 && untouched_type == 5678);
    return 0;
}
'''


def _run(tmp_path, mutation=None):
    alpha_body = source_body(
        ROOT / "src/alphamenu.cpp",
        "void AlphaMenu::on_mouse_move(int x, int y) {",
    )
    spot_body = source_body(
        ROOT / "src/spot.cpp",
        "int Spot::check(int x, int y, int *spot_pos, int *spot_type) {",
    )
    if mutation is not None:
        before, after = mutation
        assert alpha_body.count(before) == 1
        alpha_body = alpha_body.replace(before, after, 1)
    assert HARNESS.count("__ALPHA_MENU_BODY__") == 1
    assert HARNESS.count("__SPOT_CHECK_BODY__") == 1
    source = (HARNESS.replace("// __ALPHA_MENU_BODY__", alpha_body)
              .replace("// __SPOT_CHECK_BODY__", spot_body))
    compiler = shutil.which("clang++") or shutil.which("g++")
    assert compiler, "host C++ compiler required for the actual-source control"
    path = tmp_path / "alphamenu-mouse-move-actual-source.cpp"
    binary = tmp_path / "alphamenu-mouse-move-actual-source"
    path.write_text(source, encoding="utf-8")
    built = subprocess.run(
        [compiler, "-std=c++17", "-Wall", "-Wextra", "-Werror",
         "-fsanitize=address,undefined", "-fno-sanitize-recover=all",
         str(path), "-o", str(binary)],
        capture_output=True, text=True, timeout=30,
    )
    assert built.returncode == 0, built.stdout + built.stderr
    result = subprocess.run([str(binary)], capture_output=True, text=True, timeout=10,
                            env=dict(os.environ, ASAN_OPTIONS="detect_leaks=0"))
    return result


def test_actual_alphamenu_and_spot_bodies_have_ordered_runtime_controls(tmp_path):
    result = _run(tmp_path)
    assert result.returncode == 0, result.stdout + result.stderr


@pytest.mark.parametrize("mutation", [
    ("MenuSelection = position;", "MenuSelection = y;"),
    ("WinPointerOwner3 = 0;\n            on_lose_mouse_capture();",
     "on_lose_mouse_capture();\n            WinPointerOwner3 = 0;"),
    ("MenuSelection = -1;\n        redraw();\n        on_update();",
     "MenuSelection = -1;\n        on_update();\n        redraw();"),
])
def test_wrong_output_or_capture_order_is_rejected(tmp_path, mutation):
    result = _run(tmp_path, mutation)
    assert result.returncode != 0, "actual-source behavior control missed damage"

