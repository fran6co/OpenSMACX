"""Host controls extracted from the actual mapped WorldWin::click body.

The fixture keeps input APIs and callbacks independent so it can distinguish
the pinned asynchronous key-state read from the synchronous key-state API.
It checks source behavior only; the pinned CFG/IAT evidence remains the target
ABI witness.
"""

from __future__ import annotations

import os
from pathlib import Path
import shutil
import subprocess

import pytest


def project_root() -> Path:
    configured = os.environ.get("OPENSMACX_ROOT")
    if configured:
        return Path(configured).resolve()
    test_path = Path(__file__).resolve()
    if len(test_path.parents) > 2 and (test_path.parents[2] / "src").is_dir():
        return test_path.parents[2]
    raise AssertionError("set OPENSMACX_ROOT; no scratch/proposal source fallback is allowed")


ROOT = project_root()
SOURCE = ROOT / "src/worldwin.cpp"
ADDRESS = "0x004C3D40"
SIGNATURE = "void WorldWin::click(int x, int y, int button, int is_double)"


def extract_click(source: str) -> str:
    marker = f"// ORIGINAL: {ADDRESS} "
    if source.count(marker) != 1:
        raise AssertionError(f"expected one mapped WorldWin::click identity at {ADDRESS}")
    marker_at = source.index(marker)
    start = source.find(SIGNATURE, marker_at)
    if start < 0:
        raise AssertionError("mapped WorldWin::click definition is missing")
    opening = source.find("{", start)
    if opening < 0:
        raise AssertionError("WorldWin::click body is missing")
    depth = 0
    state = "code"
    index = opening
    while index < len(source):
        char = source[index]
        following = source[index + 1] if index + 1 < len(source) else ""
        if state == "code":
            if char == "/" and following == "/":
                state = "line"
                index += 1
            elif char == "/" and following == "*":
                state = "block"
                index += 1
            elif char == '"':
                state = "string"
            elif char == "'":
                state = "char"
            elif char == "{":
                depth += 1
            elif char == "}":
                depth -= 1
                if depth == 0:
                    return source[start:index + 1]
        elif state == "line":
            if char == "\n":
                state = "code"
        elif state == "block":
            if char == "*" and following == "/":
                state = "code"
                index += 1
        elif state in ("string", "char"):
            if char == "\\":
                index += 1
            elif (state == "string" and char == '"') or (
                    state == "char" and char == "'"):
                state = "code"
        index += 1
    raise AssertionError("WorldWin::click body is unterminated")


FIXTURE = r'''#include <cassert>
#include <vector>

static const int VK_SHIFT = 16;
static const int MapLatitudeBounds = 64;
static const int MapLongitudeBounds = 64;
static int async_key_value;
static int async_key_calls;
static int key_state_value;
static int key_state_calls;
static int pixel_result;
static int pixel_x;
static int pixel_y;
static int pixel_calls;
static bool change_key_on_pixel;
static int key_after_pixel;
static int right_menu_calls;
static std::vector<int> events;

enum Event { CANCEL = 1, ASYNC_KEY = 2, PIXEL = 3, DETAIL_CENTER = 4,
             PRIMARY_CENTER = 5, BUTTON = 6, SHOW = 7, HIDE = 8,
             RIGHT_MENU = 9 };

static short GetAsyncKeyState(int key) {
    assert(key == VK_SHIFT);
    ++async_key_calls;
    events.push_back(ASYNC_KEY);
    return static_cast<short>(async_key_value);
}
short GetKeyState(int key) {
    assert(key == VK_SHIFT);
    ++key_state_calls;
    return static_cast<short>(key_state_value);
}

struct TutorialWindow {
    void cancel_zoom_wait() { events.push_back(CANCEL); }
} TutWinBaseWindow;

struct MapWin;
static MapWin *MapWinTable[2];
struct MapWin {
    int center_x_;
    int center_y_;
    int center_x;
    int center_y;
    int center_mode;
    int show_mode;
    void set_center(int x, int y, int mode) {
        center_x = x; center_y = y; center_mode = mode;
        events.push_back(this == MapWinTable[1] ? DETAIL_CENTER : PRIMARY_CENTER);
    }
    void show(int mode) { show_mode = mode; events.push_back(SHOW); }
};

struct ButtonGroup {
    void set(int id, int value);
} ButtonGroupGlobal;
struct MainInterface {
    ButtonGroup &button_group(int index) { assert(index == 1); return ButtonGroupGlobal; }
} MainInterfaceGlobal;

struct Win { void hide() { events.push_back(HIDE); } };
struct ClimateWindow : Win {} WorldClimateWorldWin;

struct WorldWin {
    int detail_map_selected_;
    void click(int x, int y, int button, int is_double);
    void right_menu(int, int) { ++right_menu_calls; events.push_back(RIGHT_MENU); }
    int pixel_to_tile(int, int, int *out_x, int *out_y) {
        ++pixel_calls;
        events.push_back(PIXEL);
        *out_x = pixel_x;
        *out_y = pixel_y;
        if (change_key_on_pixel) async_key_value = key_after_pixel;
        return pixel_result;
    }
};
static WorldWin *active_world;

void ButtonGroup::set(int id, int value) {
    assert(id == 1007 && value == 0);
    assert(active_world != nullptr);
    assert(active_world->detail_map_selected_ == 1);
    events.push_back(BUTTON);
}

CLICK_DEFINITION

static void reset(WorldWin &world, MapWin &overview, MapWin &detail) {
    events.clear();
    async_key_value = key_state_value = 0;
    async_key_calls = key_state_calls = pixel_calls = right_menu_calls = 0;
    pixel_result = 0; pixel_x = 12; pixel_y = 21;
    change_key_on_pixel = false; key_after_pixel = 0;
    world.detail_map_selected_ = 0;
    overview.center_x_ = 17; overview.center_y_ = 23;
    MapWinTable[0] = &overview; MapWinTable[1] = &detail;
    active_world = &world;
}

static void detail_click_case(int key_value, int is_double) {
    WorldWin world = {};
    MapWin overview = {}, detail = {};
    reset(world, overview, detail);
    async_key_value = key_value;
    change_key_on_pixel = true;
    key_after_pixel = 0;
    world.click(40, 50, 0, is_double);
    assert(async_key_calls == 1 && key_state_calls == 0 && pixel_calls == 1);
    assert(world.detail_map_selected_ == 1);
    assert(detail.center_x == 17 && detail.center_y == 23 && detail.center_mode == 1);
    assert(detail.show_mode == 0);
    const int expected[] = {CANCEL, ASYNC_KEY, PIXEL, DETAIL_CENTER, BUTTON, SHOW, HIDE};
    assert(events == std::vector<int>(expected, expected + sizeof(expected) / sizeof(expected[0])));
}

int main() {
    WorldWin world = {};
    MapWin overview = {}, detail = {};

    // The pinned instruction tests AH: any high-byte bit selects detail.
    // 0x0100 specifically distinguishes that test from a 0x8000 sign test.
    detail_click_case(0x0100, 0);
    detail_click_case(0x8000, 0);
    detail_click_case(0, 1); // double click selects detail without a key state
    for (int raw : {0, 1, -1, 0x100, (-2147483647 - 1)}) {
        detail_click_case(0x0100, raw);
        if (raw != 0) detail_click_case(0, raw);
    }

    // Conversion publishes a new key state after the original capture point.
    reset(world, overview, detail);
    change_key_on_pixel = true;
    key_after_pixel = 0x8000;
    world.click(40, 50, 0, 0);
    assert(async_key_calls == 1 && pixel_calls == 1);
    assert(world.detail_map_selected_ == 0 && overview.center_x == 12);
    assert((events == std::vector<int>{CANCEL, ASYNC_KEY, PIXEL, PRIMARY_CENTER}));

    // A low transition bit alone is not shift-down; synchronous key state is
    // deliberately different and must not be consulted.
    reset(world, overview, detail);
    async_key_value = 0x0001;
    key_state_value = 0x8000;
    world.click(40, 50, 0, 0);
    assert(async_key_calls == 1 && key_state_calls == 0 && pixel_calls == 1);
    assert(world.detail_map_selected_ == 0);
    assert(overview.center_x == 12 && overview.center_y == 21 && overview.center_mode == 1);
    assert((events == std::vector<int>{CANCEL, ASYNC_KEY, PIXEL, PRIMARY_CENTER}));

    // Pixel conversion failure and each invalid tile edge return before any
    // selection, centering, button, show, or hide callback.
    for (int bad_kind = 0; bad_kind != 4; ++bad_kind) {
        reset(world, overview, detail);
        async_key_value = 0x0100;
        if (bad_kind == 0) pixel_result = 1;
        if (bad_kind == 1) pixel_y = -1;
        if (bad_kind == 2) pixel_y = MapLatitudeBounds;
        if (bad_kind == 3) pixel_x = MapLongitudeBounds;
        world.click(40, 50, 0, 0);
        assert(async_key_calls == 1 && key_state_calls == 0 && pixel_calls == 1);
        assert(world.detail_map_selected_ == 0 && events.size() == 3);
    }

    // Right-button routing exits before either key-state API or tile lookup.
    reset(world, overview, detail);
    key_state_value = async_key_value = 0x8000;
    world.click(40, 50, 1, 0);
    assert(right_menu_calls == 1 && async_key_calls == 0 && key_state_calls == 0);
    assert(pixel_calls == 0 && (events == std::vector<int>{CANCEL, RIGHT_MENU}));
    return 0;
}
'''


@pytest.mark.parametrize("compiler", ["g++", "clang++"])
def test_actual_worldwin_click_state_routing_and_order(compiler, tmp_path):
    executable = shutil.which(compiler)
    if executable is None:
        pytest.skip(f"{compiler} is not installed")
    source_text = SOURCE.read_text(encoding="utf-8")
    body = extract_click(source_text)
    assert "GetAsyncKeyState(VK_SHIFT)" in body
    assert "GetKeyState(VK_SHIFT)" not in body
    cpp = tmp_path / "worldwin_click_fixture.cpp"
    binary = tmp_path / "worldwin_click_fixture"
    cpp.write_text(FIXTURE.replace("CLICK_DEFINITION", body), encoding="utf-8")
    subprocess.run([executable, "-std=c++11", "-Wall", "-Wextra", "-Werror",
                    str(cpp), "-o", str(binary)], check=True, timeout=30)
    subprocess.run([str(binary)], check=True, timeout=30)
