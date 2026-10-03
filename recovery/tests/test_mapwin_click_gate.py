"""Source-extracted controls for MapWin::click's native pre-tile gate."""

from pathlib import Path
import shutil
import subprocess

import pytest


ROOT = Path(__file__).resolve().parents[2]


def extracted_gate():
    source = (ROOT / "src/mapwin.cpp").read_text()
    start = source.index("void MapWin::click(")
    end = source.index("\n}", start) + 2
    click = source[start:end]
    begin = click.index("    const bool cursor_sprite_1 =")
    finish = click.index("    int tile_x;", begin)
    return click[begin:finish]


HARNESS = r'''#include <cstdio>
#include <cstdlib>
#include <vector>

static void require(bool ok, int line) {
    if (!ok) { std::fprintf(stderr, "assertion at %d\n", line); std::exit(97); }
}
#define REQUIRE(x) require((x), __LINE__)

struct Sprite { int id; };
static Sprite g_CURSOR_SPRITES[12];
static int STATE_SCENARIO_EDITOR = 1;
static int GameState;
static const int VK_CONTROL = 17, VK_SCROLL = 145, VK_SHIFT = 16;
static int ConsoleControlTurnActive;
static int input_state[256];
static std::vector<int> events;
struct MapWin;
static MapWin *go_reset_target;
static Sprite *reset_cursor_to;
static void (*selected_hook)();

static int GetAsyncKeyState(int key) { events.push_back(10 + key); return input_state[key]; }
static int GetKeyState(int key) { events.push_back(10 + key); return input_state[key]; }
struct ConsoleMock {
    int field_23D10_;
    int selected() {
        events.push_back(2);
        const int old_mode = field_23D10_;
        field_23D10_ = 0;
        if (selected_hook != nullptr) selected_hook();
        return old_mode;
    }
} ConsoleGlobal;

struct MapWin {
    Sprite *cursor_sprite_;
    int field_1DE24_, field_1DD84_;
    void pre_tile_gate(int button);
};

static void go_reset() {
    events.push_back(1);
    if (reset_cursor_to != nullptr) go_reset_target->cursor_sprite_ = reset_cursor_to;
}

static MapWin *selected_cursor_target;
static Sprite *selected_cursor_to;
static void mutate_cursor_during_selected() {
    selected_cursor_target->cursor_sprite_ = selected_cursor_to;
}

void MapWin::pre_tile_gate(int button) {
    GATE_BLOCK
    (void)editor_tool; (void)editor_mode; (void)control_down;
    events.push_back(3);
}

static void reset_fixture() {
    events.clear();
    for (int i = 0; i < 256; ++i) input_state[i] = 0;
    GameState = 0;
    ConsoleControlTurnActive = 0;
    ConsoleGlobal.field_23D10_ = 0;
    go_reset_target = nullptr;
    reset_cursor_to = nullptr;
    selected_hook = nullptr;
    selected_cursor_target = nullptr;
    selected_cursor_to = nullptr;
}

static int run_case(int button, int turn_active, int sprite_index,
                    int old_mode, int mutate_after_snapshot,
                    int scenario_editor, int scroll_state) {
    reset_fixture();
    MapWin window;
    window.cursor_sprite_ = sprite_index < 0 ? nullptr : &g_CURSOR_SPRITES[sprite_index];
    window.field_1DE24_ = 7;
    window.field_1DD84_ = 2;
    go_reset_target = &window;
    reset_cursor_to = mutate_after_snapshot < 0 ? nullptr :
                      &g_CURSOR_SPRITES[mutate_after_snapshot];
    ConsoleControlTurnActive = turn_active;
    ConsoleGlobal.field_23D10_ = old_mode;
    if (scenario_editor) GameState = STATE_SCENARIO_EDITOR;
    input_state[VK_CONTROL] = 0x8000;
    input_state[VK_SCROLL] = scroll_state;
    input_state[VK_SHIFT] = 0;
    window.pre_tile_gate(button);
    return !events.empty() && events.back() != 3;
}

static void truth_table() {
    const int sprites[] = {1, 5, 9, 3, -1};
    for (int button = 0; button <= 1; ++button) {
        for (int turn = 0; turn <= 1; ++turn) {
            for (int sprite : sprites) {
                for (int mode : {0, 4}) {
                    const int gated = run_case(button, turn, sprite, mode,
                                               -1, 0, 0);
                    const bool sprite5_gate = sprite == 5 && mode != 0;
                    const bool common_gate = (button != 0 || turn != 0) &&
                        (sprite == 1 || sprite == 9 || mode != 0);
                    if ((gated != 0) != (sprite5_gate || common_gate)) {
                        std::fprintf(stderr, "button=%d turn=%d sprite=%d mode=%d gated=%d\n",
                                     button, turn, sprite, mode, gated);
                    }
                    REQUIRE((gated != 0) == (sprite5_gate || common_gate));
                }
            }
        }
    }
}

static void ordering_and_snapshots() {
    reset_fixture();
    MapWin window;
    window.cursor_sprite_ = &g_CURSOR_SPRITES[1];
    window.field_1DE24_ = 0;
    window.field_1DD84_ = 0;
    go_reset_target = &window;
    reset_cursor_to = &g_CURSOR_SPRITES[5];
    ConsoleControlTurnActive = 0;
    ConsoleGlobal.field_23D10_ = 0;
    window.pre_tile_gate(1);
    const int gated = events.back() != 3;
    REQUIRE(gated != 0); // pre-reset sprite 1 survives a cursor change; mode is zero
    REQUIRE((events == std::vector<int>{1, 27, 2}));

    reset_fixture();
    window.cursor_sprite_ = &g_CURSOR_SPRITES[3];
    go_reset_target = &window;
    reset_cursor_to = &g_CURSOR_SPRITES[5];
    ConsoleGlobal.field_23D10_ = 3;
    selected_cursor_target = &window;
    selected_cursor_to = &g_CURSOR_SPRITES[3];
    selected_hook = mutate_cursor_during_selected;
    window.pre_tile_gate(0);
    REQUIRE(events.back() != 3); // sprite 5 snapshot precedes selected's callback mutation
    REQUIRE((events == std::vector<int>{1, 27, 2}));

    reset_fixture();
    window.cursor_sprite_ = &g_CURSOR_SPRITES[3];
    go_reset_target = &window;
    reset_cursor_to = nullptr;
    ConsoleGlobal.field_23D10_ = 2;
    window.pre_tile_gate(1);
    REQUIRE(events.back() != 3); // selected's return carries mode
    REQUIRE((events == std::vector<int>{1, 27, 2}));

    reset_fixture();
    window.cursor_sprite_ = &g_CURSOR_SPRITES[3];
    go_reset_target = &window;
    ConsoleGlobal.field_23D10_ = 0;
    GameState = STATE_SCENARIO_EDITOR;
    input_state[VK_SCROLL] = 1;
    window.pre_tile_gate(0);
    REQUIRE((events == std::vector<int>{1, 27, 155, 26, 2, 3}));
}

int main() { truth_table(); ordering_and_snapshots(); }
'''


@pytest.mark.parametrize("compiler", ["g++", "clang++"])
def test_actual_click_gate(compiler, tmp_path):
    executable = shutil.which(compiler)
    if executable is None:
        pytest.skip(f"{compiler} is not installed")
    block = extracted_gate()
    source = HARNESS.replace("GATE_BLOCK", block)
    # Declare MapWin before go_reset's pointer use without copying its body.
    cpp = tmp_path / "click_gate.cpp"
    binary = tmp_path / "click_gate"
    cpp.write_text(source)
    subprocess.run([executable, "-std=c++11", "-Wall", "-Wextra", "-Werror",
                    str(cpp), "-o", str(binary)], check=True, timeout=30)
    subprocess.run([str(binary)], check=True, timeout=30)
