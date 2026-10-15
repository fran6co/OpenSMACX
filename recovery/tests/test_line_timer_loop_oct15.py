"""Actual-source controls for line_timer's ordered map-window redraw loop."""
from pathlib import Path
import shutil
import subprocess

import pytest


ROOT = Path(__file__).resolve().parents[2]
SIGNATURE = "void __cdecl line_timer(int a1) {"


def source_body() -> str:
    source = (ROOT / "src/time.cpp").read_text()
    start = source.index(SIGNATURE)
    opening = source.index("{", start)
    depth = 0
    for end in range(opening, len(source)):
        if source[end] == "{":
            depth += 1
        elif source[end] == "}":
            depth -= 1
            if depth == 0:
                return source[start : end + 1]
    raise AssertionError("unterminated production line_timer body")


HARNESS = r'''
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <vector>

#define __cdecl
#define nullptr 0
static const int MapWinTableSlots = 8;
void __cdecl line_timer(int a1);
std::vector<int> Events;
int WorldClimateSkipTerrainClear;
int TimerHoldFlag;
int PlanLineCounter;
enum {
    base_visible = 10, social_visible = 11, redraw = 20,
    draw_base = 1000, draw_replacement = 1060
};
#define require(value) do { if (!(value)) { fprintf(stderr, "harness assertion at %d\\n", __LINE__); exit(1); } } while (0)

struct VisibilityWindow {
    int result;
    int event;
    int is_visible() { Events.push_back(event); return result; }
} TutWinBaseWindow, TutWinSocWindow;

struct PlanWin {
    void on_redraw();
};
struct PlanWindowType : PlanWin {};
PlanWindowType PlanWindow;

struct MapWin {
    int id_;
    unsigned int field_1DD74_;
    unsigned int field_1DD70_;
    void draw_base_dest(int value);
} Windows[8], Replacement;
MapWin *MapWinTable[MapWinTableSlots];
bool MutateLaterSlots;

void PlanWin::on_redraw() {
    require(PlanLineCounter == 5);
    Events.push_back(redraw);
}

void MapWin::draw_base_dest(int value) {
    require(value == 1);
    Events.push_back(id_ == 60 ? draw_replacement : draw_base + id_);
    if (MutateLaterSlots && id_ == 0) {
        // The image reloads later receivers and flags after this call returns.
        Windows[4].field_1DD74_ = 1;
        MapWinTable[6] = &Replacement;
    }
}

void reset() {
    Events.clear();
    WorldClimateSkipTerrainClear = 0;
    TimerHoldFlag = 0;
    PlanLineCounter = 6;
    TutWinBaseWindow.result = 0;
    TutWinBaseWindow.event = base_visible;
    TutWinSocWindow.result = 0;
    TutWinSocWindow.event = social_visible;
    MutateLaterSlots = false;
    for (int slot = 0; slot < MapWinTableSlots; ++slot) {
        Windows[slot].id_ = slot;
        Windows[slot].field_1DD74_ = 0;
        Windows[slot].field_1DD70_ = 0;
        MapWinTable[slot] = &Windows[slot];
    }
    Replacement.id_ = 60;
    Replacement.field_1DD74_ = 1;
    Replacement.field_1DD70_ = 0xC40000;
}

void check_guards() {
    reset();
    WorldClimateSkipTerrainClear = 1;
    TimerHoldFlag = 1;
    TutWinBaseWindow.result = 1;
    TutWinSocWindow.result = 1;
    line_timer(0);
    require(Events.empty() && PlanLineCounter == 6);

    reset();
    TimerHoldFlag = 1;
    TutWinBaseWindow.result = 1;
    TutWinSocWindow.result = 1;
    line_timer(0);
    require(Events.empty() && PlanLineCounter == 6);

    reset();
    TutWinBaseWindow.result = 1;
    TutWinSocWindow.result = 1;
    line_timer(0);
    require(Events.size() == 1 && Events[0] == base_visible);
    require(PlanLineCounter == 6);

    reset();
    TutWinSocWindow.result = 1;
    line_timer(0);
    require(Events.size() == 2 && Events[0] == base_visible &&
            Events[1] == social_visible);
    require(PlanLineCounter == 6);
}

void check_all_slots_null_primary_mask_and_live_reads() {
    reset();
    MapWinTable[2] = 0;
    // Slot zero is drawn despite being inactive. Slot one is active but fails
    // the mask. Slot three draws. Slot four starts inactive and is activated
    // by slot zero's callback. Slot six is replaced by that same callback.
    Windows[0].field_1DD70_ = 0x400000;
    Windows[1].field_1DD74_ = 1;
    Windows[1].field_1DD70_ = 0x20000;
    Windows[3].field_1DD74_ = 1;
    Windows[3].field_1DD70_ = 0x40000;
    Windows[4].field_1DD70_ = 0xC40000;
    Windows[6].field_1DD74_ = 1;
    Windows[6].field_1DD70_ = 0xC40000;
    Windows[7].field_1DD74_ = 1;
    Windows[7].field_1DD70_ = 0x400000;
    MutateLaterSlots = true;
    line_timer(0);
    const int expected[] = {
        base_visible, social_visible, redraw,
        draw_base + 0, draw_base + 3, draw_base + 4, draw_replacement,
        draw_base + 7
    };
    require(Events.size() == sizeof(expected) / sizeof(expected[0]));
    for (unsigned int i = 0; i < sizeof(expected) / sizeof(expected[0]); ++i) {
        require(Events[i] == expected[i]);
    }
    require(PlanLineCounter == 5);
}

int main() {
    check_guards();
    check_all_slots_null_primary_mask_and_live_reads();
    return 0;
}
'''


def compile_and_run(compiler: str, body: str, tmp_path: Path):
    source = tmp_path / "line_timer.cpp"
    executable = tmp_path / "line_timer"
    source.write_text(HARNESS + "\n" + body + "\n")
    built = subprocess.run(
        [compiler, "-std=c++98", "-Wall", "-Wextra", "-Werror", "-Wno-unused-parameter", str(source), "-o", str(executable)],
        capture_output=True,
        text=True,
        timeout=30,
    )
    assert built.returncode == 0, built.stdout + built.stderr
    return subprocess.run([str(executable)], capture_output=True, text=True, timeout=10)


COMPILERS = [compiler for compiler in (shutil.which("g++"), shutil.which("clang++")) if compiler]


@pytest.mark.parametrize("compiler", COMPILERS or [None])
def test_actual_line_timer_order_live_slots_and_branches(compiler, tmp_path):
    if compiler is None:
        pytest.skip("host C++ compiler unavailable")
    body = source_body()
    ordered = (
        "WorldClimateSkipTerrainClear != 0",
        "TimerHoldFlag != 0",
        "TutWinBaseWindow.is_visible() != 0",
        "TutWinSocWindow.is_visible() != 0",
        "--PlanLineCounter;",
        "PlanWindow.PlanWin::on_redraw();",
        "for (int slot = 0; slot < static_cast<int>(MapWinTableSlots); ++slot)",
    )
    positions = [body.index(anchor) for anchor in ordered]
    assert positions == sorted(positions)
    assert "MapWin *const window = MapWinTable[slot];" in body
    assert "if (slot != 0)" in body
    result = compile_and_run(compiler, body, tmp_path)
    assert result.returncode == 0, result.stdout + result.stderr


@pytest.mark.parametrize("compiler", COMPILERS or [None])
@pytest.mark.parametrize(
    "name,mutate",
    (
        ("miss_last_slot", lambda body: body.replace("slot < static_cast<int>(MapWinTableSlots)", "slot < 7", 1)),
        ("lose_primary_exemption", lambda body: body.replace("if (slot != 0)", "if (slot == 0)", 1)),
        (
            "snapshot_receivers",
            lambda body: body.replace(
                "    for (int slot = 0; slot < static_cast<int>(MapWinTableSlots); ++slot) {",
                "    MapWin *snapshot[8];\n    for (int saved = 0; saved < 8; ++saved) snapshot[saved] = MapWinTable[saved];\n    for (int slot = 0; slot < static_cast<int>(MapWinTableSlots); ++slot) {",
                1,
            ).replace("MapWin *const window = MapWinTable[slot];", "MapWin *const window = snapshot[slot];", 1),
        ),
        (
            "redraw_after_loop",
            lambda body: body.replace(
                "    PlanWindow.PlanWin::on_redraw();\n\n    for (",
                "    for (",
                1,
            ).replace(
                "            window->draw_base_dest(1);\n        }\n    }\n}",
                "            window->draw_base_dest(1);\n        }\n    }\n    PlanWindow.PlanWin::on_redraw();\n}",
                1,
            ),
        ),
        (
            "invert_visibility_guard_order",
            lambda body: body.replace(
                "    if (TutWinBaseWindow.is_visible() != 0) {\n        return;\n    }\n    if (TutWinSocWindow.is_visible() != 0) {",
                "    if (TutWinSocWindow.is_visible() != 0) {\n        return;\n    }\n    if (TutWinBaseWindow.is_visible() != 0) {",
                1,
            ),
        ),
        ("invert_draw_mask", lambda body: body.replace("(draw_flags & 0xC40000) != 0", "(draw_flags & 0xC40000) == 0", 1)),
    ),
)
def test_actual_line_timer_damage_controls_compile_then_fail(compiler, name, mutate, tmp_path):
    if compiler is None:
        pytest.skip("host C++ compiler unavailable")
    body = source_body()
    damaged = mutate(body)
    assert damaged != body, f"{name} mutation anchor missing"
    result = compile_and_run(compiler, damaged, tmp_path)
    assert result.returncode != 0, name
