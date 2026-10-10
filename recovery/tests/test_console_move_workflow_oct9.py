"""Execute Console::set_move's actual body against event-traced host owners."""

from __future__ import annotations

import shutil
import subprocess
from pathlib import Path

import pytest


ROOT = Path(__file__).resolve().parents[2]


def _definition(source: str, signature: str) -> str:
    start = source.index(signature)
    opening = source.index("{", start)
    depth = 0
    for index in range(opening, len(source)):
        if source[index] == "{":
            depth += 1
        elif source[index] == "}":
            depth -= 1
            if depth == 0:
                return source[start : index + 1]
    raise AssertionError(f"unterminated definition: {signature}")


HARNESS = r'''
#include <cassert>
#include <cstdint>
#include <string>
#include <vector>
using uint32_t = std::uint32_t;
static std::vector<std::string> events;
static void event(const std::string &value) { events.push_back(value); }
static int not_turn = 0;
static int ready_selection = -1;
static int ambience_reselection = -1;
static int spot_reselection = -1;
static int spot_x = 700, spot_y = 800;
static int selection_after_info = -1, selection_after_status = -1;
static int selection_after_caption = -1, selection_after_focus = -1;
static int selection_after_menu = -1;
static int history_after_info = -1, history_after_status = -1;
static int history_after_caption = -1, history_after_menu = -1;
static int history_after_set_loc = -1;

struct LockStateEntry {
    struct { uint32_t flags = 0; } payload_;
    uint32_t trailing_words_[10]{};
};
struct NetDaemon {
    LockStateEntry channels_[2];
    struct Primary { uint32_t leading_words_[3]{}; uint32_t flags_{}; uint32_t trailing_words_[10]{}; } primary_state_;
    uint32_t &factions_waiting_mask() { return primary_state_.trailing_words_[7]; }
};
NetDaemon NetDaemonNet;
struct GameFields { uint32_t net_turn_flags_ = 0; };
struct GameData { GameFields fields_; };
GameData GameDataStateStorage;
int IsMultiplayerNet = 0;
int ConsoleControlTurnActive = 0;
uint32_t &ConsoleFactionsWaitingMask = NetDaemonNet.factions_waiting_mask();

struct ConsoleGlobalState {
    int cursor_active_ = 1;
    int cursor_history_index_ = 0;
    int cursor_history_x_[8]{};
    int cursor_history_y_[8]{};
};
ConsoleGlobalState ConsoleGlobal;
struct Veh { int x = 0, y = 0, faction_id = 0; uint8_t moves_expended = 0; };
Veh Vehs[16];
static int selectable[16]{};
static int selectable_calls[16]{};
struct Console;
static Console *active_console;

struct MapWin {
    void draw_map(int value) { event("draw_map:" + std::to_string(value)); }
    void draw_cursor() { event("map_cursor"); }
    void main_caption();
};
static MapWin map_window;
struct InfoWin { void change(int value); };
struct StatusWin {
    void redraw();
    void set_loc(int x, int y);
};
struct AlphaMenu { void check(int value); };
InfoWin ConsoleInfoWin;
StatusWin ConsoleStatusWin;

struct Console {
    uint32_t cursor_active_ = 0, field_23BF8_ = 0, field_23BFC_ = 0;
    uint32_t field_23BE4_ = 0, field_23BF4_ = 0, survey_mode_ = 0;
    int faction_id_ = 0, selected_vehicle_ = -1;
    void ready() {
        event("ready");
        selected_vehicle_ = ready_selection;
    }
    MapWin *console_map_win() { return &map_window; }
    void cursor_next(int x, int y) { event("next:" + std::to_string(x) + "," + std::to_string(y)); }
    void focus(int x, int y, int faction);
    AlphaMenu alpha_menu_;
    void set_move(int force_update);
};

void InfoWin::change(int value) {
    event("info:" + std::to_string(value));
    if (selection_after_info >= 0) active_console->selected_vehicle_ = selection_after_info;
    if (history_after_info >= 0) ConsoleGlobal.cursor_history_index_ = history_after_info;
}
void StatusWin::redraw() {
    event("status");
    if (selection_after_status >= 0) active_console->selected_vehicle_ = selection_after_status;
    if (history_after_status >= 0) ConsoleGlobal.cursor_history_index_ = history_after_status;
}
void MapWin::main_caption() {
    event("caption");
    if (selection_after_caption >= 0) active_console->selected_vehicle_ = selection_after_caption;
    if (history_after_caption >= 0) ConsoleGlobal.cursor_history_index_ = history_after_caption;
}
void StatusWin::set_loc(int x, int y) {
    event("loc:" + std::to_string(x) + "," + std::to_string(y));
    if (history_after_set_loc >= 0) ConsoleGlobal.cursor_history_index_ = history_after_set_loc;
}
void AlphaMenu::check(int value) {
    event("menu:" + std::to_string(value));
    if (selection_after_menu >= 0) active_console->selected_vehicle_ = selection_after_menu;
    if (history_after_menu >= 0) ConsoleGlobal.cursor_history_index_ = history_after_menu;
}
void Console::focus(int x, int y, int faction) {
    event("focus:" + std::to_string(x) + "," + std::to_string(y) + "," + std::to_string(faction));
    if (selection_after_focus >= 0) selected_vehicle_ = selection_after_focus;
}

int not_my_turn() { event("not_my_turn"); return not_turn; }
int veh_selectable(int id) {
    event("select:" + std::to_string(id));
    if (id < 0) return 0;
    ++selectable_calls[id];
    return selectable[id];
}
void draw_cursor() { event("draw_cursor"); }
int ambience(int value) { event("ambience:" + std::to_string(value)); return 0; }
void ambience_veh(int id) {
    event("ambience_veh:" + std::to_string(id));
    if (ambience_reselection >= 0) active_console->selected_vehicle_ = ambience_reselection;
}
void spot_all(int id, int toggle) {
    event("spot:" + std::to_string(id) + "," + std::to_string(toggle));
    const int visible_vehicle = spot_reselection >= 0 ? spot_reselection : id;
    if (spot_reselection >= 0) active_console->selected_vehicle_ = spot_reselection;
    Vehs[visible_vehicle].x = spot_x;
    Vehs[visible_vehicle].y = spot_y;
}

BODY

static void reset_state() {
    events.clear();
    IsMultiplayerNet = 0;
    ConsoleControlTurnActive = 0;
    ConsoleFactionsWaitingMask = 0;
    GameDataStateStorage.fields_.net_turn_flags_ = 0;
    NetDaemonNet.channels_[1].payload_.flags = 0;
    NetDaemonNet.primary_state_.trailing_words_[4] = 0;
    ConsoleGlobal.cursor_active_ = 1;
    ConsoleGlobal.cursor_history_index_ = 0;
    ConsoleGlobal.cursor_history_x_[0] = 5;
    ConsoleGlobal.cursor_history_y_[0] = 6;
    ready_selection = -1;
    ambience_reselection = -1;
    spot_reselection = -1;
    selection_after_info = selection_after_status = -1;
    selection_after_caption = selection_after_focus = -1;
    selection_after_menu = -1;
    history_after_info = history_after_status = -1;
    history_after_caption = history_after_menu = -1;
    history_after_set_loc = -1;
    not_turn = 0;
    for (int i = 0; i < 16; ++i) {
        selectable[i] = 0;
        selectable_calls[i] = 0;
        Vehs[i] = Veh{};
    }
}

static void check_blocked(Console &console) {
    console.set_move(0);
    assert(events.empty());
}

int main() {
    reset_state();
    Console console;
    active_console = &console;
    console.selected_vehicle_ = 1;
    selectable[1] = 1;
    selectable[2] = 1;
    Vehs[2].moves_expended = 0;
    ambience_reselection = 2;
    spot_reselection = 3;
    console.set_move(0);
    const std::vector<std::string> initial_path = {
        "select:1", "ambience_veh:1", "select:2", "spot:2,0", "map_cursor", "next:700,800"
    };
    assert(events == initial_path);

    reset_state();
    Console ready_console;
    active_console = &ready_console;
    ready_console.selected_vehicle_ = -1;
    ready_selection = 3;
    selectable[3] = 1;
    ready_console.set_move(0);
    const std::vector<std::string> ready_path = {
        "select:-1", "ready", "select:3", "spot:3,0", "map_cursor", "next:700,800"
    };
    assert(events == ready_path);

    reset_state();
    Console denied;
    active_console = &denied;
    IsMultiplayerNet = 1;
    ConsoleControlTurnActive = 1;
    check_blocked(denied);
    ConsoleControlTurnActive = 0;
    NetDaemonNet.channels_[1].payload_.flags = 1;
    check_blocked(denied);
    NetDaemonNet.channels_[1].payload_.flags = 0;
    denied.faction_id_ = 2;
    ConsoleFactionsWaitingMask = 1 << 2;
    check_blocked(denied);
    ConsoleFactionsWaitingMask = 0;
    NetDaemonNet.primary_state_.trailing_words_[4] = 1;
    check_blocked(denied);
    NetDaemonNet.primary_state_.trailing_words_[4] = 0;
    GameDataStateStorage.fields_.net_turn_flags_ = 0x10;
    not_turn = 1;
    denied.set_move(0);
    assert(events.size() == 1 && events[0] == "not_my_turn");

    reset_state();
    Console online;
    active_console = &online;
    IsMultiplayerNet = 1;
    online.selected_vehicle_ = 4;
    selectable[4] = 1;
    Vehs[4].moves_expended = 0;
    online.set_move(0);
    const std::vector<std::string> online_path = {
        "select:4", "ambience_veh:4", "select:4", "map_cursor", "next:0,0"
    };
    assert(events == online_path);

    reset_state();
    Console spent;
    active_console = &spent;
    spent.selected_vehicle_ = 5;
    selectable[5] = 1;
    Vehs[5].moves_expended = 1;
    spent.set_move(0);
    const std::vector<std::string> spent_path = {
        "select:5", "ambience_veh:5", "select:5", "map_cursor", "next:0,0"
    };
    assert(events == spent_path);

    reset_state();
    Console callback_path;
    active_console = &callback_path;
    callback_path.selected_vehicle_ = 1;
    selectable[1] = 1;
    Vehs[1].moves_expended = 1;
    ConsoleGlobal.cursor_history_x_[3] = 33;
    ConsoleGlobal.cursor_history_y_[3] = 34;
    Vehs[7].faction_id = 77;
    selection_after_info = 2;
    selection_after_status = 3;
    selection_after_caption = 7;
    selection_after_focus = 9;
    history_after_info = 1;
    history_after_status = 2;
    history_after_caption = 3;
    callback_path.set_move(1);
    const std::vector<std::string> callback_events = {
        "draw_cursor", "select:1", "ambience_veh:1", "select:1", "map_cursor",
        "next:0,0", "info:0", "status", "caption", "focus:33,34,77", "menu:9"
    };
    assert(events == callback_events);

    reset_state();
    Console empty_path;
    active_console = &empty_path;
    empty_path.selected_vehicle_ = -1;
    empty_path.faction_id_ = 9;
    ConsoleGlobal.cursor_history_x_[4] = 44;
    ConsoleGlobal.cursor_history_y_[4] = 45;
    ConsoleGlobal.cursor_history_x_[5] = 55;
    ConsoleGlobal.cursor_history_y_[5] = 56;
    selection_after_info = 1;
    selection_after_status = 2;
    selection_after_caption = 3;
    history_after_info = 1;
    history_after_status = 2;
    history_after_caption = 3;
    history_after_menu = 4;
    history_after_set_loc = 5;
    empty_path.set_move(0);
    const std::vector<std::string> empty_events = {
        "select:-1", "ready", "ambience:30", "draw_cursor", "info:0", "status",
        "caption", "menu:3", "loc:44,45", "focus:55,56,9"
    };
    assert(events == empty_events);
}
'''


def test_actual_set_move_network_presentation_and_visibility_order(tmp_path: Path) -> None:
    compiler = shutil.which("g++") or shutil.which("clang++")
    if compiler is None:
        pytest.skip("host C++ compiler unavailable")
    source = (ROOT / "src/console.cpp").read_text()
    body = _definition(source, "void Console::set_move(int force_update)")
    fixture = HARNESS.replace("BODY", body)

    def build_and_run(label: str, contents: str, expect_success: bool = True) -> subprocess.CompletedProcess[bytes]:
        cpp_file = tmp_path / f"console_move_{label}.cpp"
        binary = tmp_path / f"console_move_{label}"
        cpp_file.write_text(contents)
        subprocess.run(
            [compiler, "-std=c++11", "-Wall", "-Wextra", "-Werror", str(cpp_file), "-o", str(binary)],
            check=True,
            timeout=30,
        )
        result = subprocess.run([str(binary)], capture_output=True, timeout=30)
        if expect_success:
            assert result.returncode == 0, result.stderr.decode(errors="replace")
        return result

    build_and_run("actual", fixture)
    for label, call in (("ambience", "ambience_veh(selected_vehicle_);"), ("spot", "spot_all(visible_vehicle_id, 0);")):
        mutant = fixture.replace(call, "/* omitted by damage control */")
        assert mutant != fixture
        damaged = build_and_run(f"missing_{label}", mutant, expect_success=False)
        assert damaged.returncode != 0, f"{label} omission escaped the event-trace control"
