"""Compile production timer body with controlled branch/geometry dependencies."""

from pathlib import Path
import subprocess


ROOT = Path(__file__).resolve().parents[2]
SOURCE = ROOT / "src/time.cpp"


FIXTURE = r'''#define __cdecl
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <stdlib.h>
typedef void *HINSTANCE;
typedef void *HWND;
typedef long LONG;
enum { GWL_HINSTANCE = -6 };

uint32_t WorldClimateSkipTerrainClear;
int TimerHoldFlag;
struct GameDataStateFields {
    uint32_t game_state_, game_preferences_;
};
struct GameDataState {
    GameDataStateFields fields_;
} GameDataStateStorage;
int IsMultiplayerNet;
int ExitTurnLoop;
int WinModalDepth;
int WinScreenWidth = 800, WinScreenHeight = 600;
int MapLongitudeBounds = 64, MapLatitudeBounds = 64;
int MapIsFlat = 1;
HINSTANCE WinInstance = (HINSTANCE)0x42;
int draw_cursor_calls;
int async_state;
HWND focus_hwnd = (HWND)0x20;
LONG instance_result = 0x42;

int GetAsyncKeyState(int) { return async_state; }
HWND GetFocus() { return focus_hwnd; }
LONG GetWindowLongA(HWND, int) { return instance_result; }

struct Box { int clips; void clip_ids(unsigned long) { ++clips; } };
struct WinLike { int visible; int is_visible() const { return visible; } };
struct Tut { int visible, arrows; int is_visible() const { return visible; } void draw_arrow() { ++arrows; } };
struct Plan { int blinks; void blink() { ++blinks; } };
struct MapWin {
    uint32_t field_1DD74_, field_1DD80_;
    int field_1DDCC_, field_1DDD0_, field_1DDD4_, field_1DDD8_;
    uint32_t field_1DD9C_, field_1DDA0_;
    int field_1DDA8_;
    uint32_t field_21A44_;
    int mouse_x, mouse_y, centers, center_x, center_y;
    void get_mouse_pos(int *x, int *y) { *x = mouse_x; *y = mouse_y; }
    void client_to_screen(int *, int *) {}
    void set_center(int x, int y, int) {
        ++centers; center_x = x; center_y = y;
    }
};
struct Console : MapWin {
    uint32_t field_23BE8_, field_23BE4_, field_23BF8_, field_23BFC_;
    int field_23D80_, field_23D84_, field_23D88_, field_23D8C_;
};
struct MainInterface { Box string_box_; };

WinLike TutWinBaseWindow, TutWinSocWindow;
Tut TutorialWindow;
Plan PlanWindow;
MainInterface MainInterfaceGlobal;
Console ConsoleGlobal;
MapWin *MapWinTable[8];
static const size_t MapWinTableSlots = 8;

void draw_cursor() { ++draw_cursor_calls; }

'''


def extract_body(source_text: str) -> str:
    identity = "// ORIGINAL: 0x0050EA40 ?blink_timer@@YAXH@Z 0x0050EA40-0x0050EE25 FILE"
    assert identity in source_text
    start = source_text.index("void __cdecl blink_timer(int a1)", source_text.index(identity))
    brace = source_text.index("{", start)
    depth = 0
    for end in range(brace, len(source_text)):
        if source_text[end] == "{":
            depth += 1
        elif source_text[end] == "}":
            depth -= 1
            if depth == 0:
                return source_text[start : end + 1]
    raise AssertionError("blink_timer body is unterminated")


MAIN = r'''
int main(int argc, char **argv) {
    const char *mode = argc > 1 ? argv[1] : "normal";
    if (strcmp(mode, "hold") == 0) {
        TimerHoldFlag = 1;
        blink_timer(7);
        return PlanWindow.blinks == 0 && MainInterfaceGlobal.string_box_.clips == 0
            && draw_cursor_calls == 0 ? 0 : 1;
    }
    if (strcmp(mode, "tutorial") == 0) {
        TutWinBaseWindow.visible = 1;
        blink_timer(7);
        return TutorialWindow.arrows == 1 && PlanWindow.blinks == 0
            && MainInterfaceGlobal.string_box_.clips == 0 ? 0 : 2;
    }

    ConsoleGlobal.field_23BE8_ = 1;
    ConsoleGlobal.field_23BE4_ = 1;
    ConsoleGlobal.field_23D80_ = ConsoleGlobal.field_23D84_ =
        ConsoleGlobal.field_23D88_ = ConsoleGlobal.field_23D8C_ = 0;
    GameDataStateStorage.fields_.game_preferences_ = 0x1000;
    MapWin window = {};
    window.field_1DDCC_ = 10;
    window.field_1DDD0_ = 1;
    window.field_1DDD4_ = 10;
    window.field_1DDD8_ = 1;
    window.field_1DD9C_ = 4;
    window.field_1DDA0_ = 4;
    window.field_1DDA8_ = 0;
    window.field_1DD74_ = window.field_1DD80_ = 1;
    window.mouse_x = 100;
    window.mouse_y = 100;
    MapWinTable[0] = &window;

    if (strcmp(mode, "boundary") == 0) {
        window.field_1DDA0_ = 64;
        window.mouse_x = 0;
        window.mouse_y = 100;
    }
    blink_timer(7);
    if (PlanWindow.blinks != 1 || MainInterfaceGlobal.string_box_.clips != 1
        || draw_cursor_calls != 1) return 3;
    if (strcmp(mode, "boundary") == 0) return window.centers == 0 ? 0 : 4;
    return 0;
}
'''


def compile_and_run(tmp_path: Path, body: str, mode: str,
                    compiler: str) -> subprocess.CompletedProcess:
    cpp = tmp_path / "blink_timer_fixture.cpp"
    exe = tmp_path / "blink_timer_fixture"
    cpp.write_text(FIXTURE + body + MAIN)
    subprocess.run([compiler, "-std=c++11", "-Wall", "-Wextra", str(cpp), "-o", str(exe)], check=True)
    return subprocess.run([str(exe), mode], check=False)


def test_actual_source_body_runs_guard_tutorial_and_boundary_controls(tmp_path):
    body = extract_body(SOURCE.read_text())
    for compiler in ("g++", "clang++"):
        for mode in ("hold", "tutorial", "normal", "boundary"):
            result = compile_and_run(tmp_path, body, mode, compiler)
            assert result.returncode == 0, (compiler, mode, result.returncode)


def test_actual_source_mutant_changes_half_open_vertical_bound(tmp_path):
    body = extract_body(SOURCE.read_text())
    mutant = body.replace("if (row >= MapLatitudeBounds) continue;",
                          "if (row > MapLatitudeBounds) continue;")
    assert mutant != body
    for compiler in ("g++", "clang++"):
        result = compile_and_run(tmp_path, mutant, "boundary", compiler)
        assert result.returncode != 0, compiler
