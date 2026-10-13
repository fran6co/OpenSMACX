"""Bounded host controls for the actual BaseWin::zoom tutorial branches.

The harness extracts and executes the current production method. It substitutes
only its external game/UI services, and checks observed calls, arguments,
rectangles and state changes. It is not a native ABI, target compiler, whole
game or complete runtime proof.
"""
from __future__ import annotations

from pathlib import Path
import re
import shutil
import subprocess

import pytest

from match_recovery.includes import COMMENTS_OR_STRINGS


ROOT = next(parent for parent in Path(__file__).resolve().parents
            if (parent / "src/basewin.cpp").is_file())
SOURCE = ROOT / "src/basewin.cpp"
INSERTION = "/* ACTUAL_BASEWIN_ZOOM */"


def _definition(source: str, signature: str) -> str:
    masked = COMMENTS_OR_STRINGS.sub(
        lambda match: re.sub(r"[^\n]", " ", match[0]), source)
    assert masked.count(signature) == 1, (signature, masked.count(signature))
    start = masked.index(signature)
    opening = masked.index("{", start + len(signature))
    depth = 0
    for index in range(opening, len(masked)):
        if masked[index] == "{":
            depth += 1
        elif masked[index] == "}":
            depth -= 1
            if depth == 0:
                return source[start:index + 1]
    raise AssertionError(f"unterminated definition: {signature}")


HARNESS = r'''#include <cassert>
#include <cstdint>
#include <cstring>
#include <string>
#include <vector>

struct RECT { int left, top, right, bottom; };
static const uint32_t BSTATE_UNK_10 = 0x10;
static std::vector<std::string> events;
static void event(const char *name) { events.push_back(name); }
static int position(const char *name) {
    for (int i = 0; i < static_cast<int>(events.size()); ++i)
        if (events[i] == name) return i;
    return -1;
}
static int occurrences(const char *name) {
    int count = 0;
    for (std::size_t i = 0; i < events.size(); ++i)
        if (events[i] == name) ++count;
    return count;
}

struct Base {
    uint8_t faction_id_current;
    int8_t population_size;
    uint32_t state;
    uint32_t governor;
    int queue_production_id[10];
    int resource_sqr_radius;
    int specialist_total;
    uint8_t facilities_built[16];
    int16_t x, y;
    char name_string[32];
    int nutrient_surplus;
    int mineral_surplus;
    int energy_surplus;
    int energy_inefficiency;
};
static Base Bases[8];
static Base *BaseCurrent;
struct GameDataStateWindowFields { int base_current_count_; };
struct GameDataStateWindow { GameDataStateWindowFields fields_; };
static GameDataStateWindow GameDataStateStorage;
static int BaseIDCurrentSelected;
static int BaseComputedID;
static uint32_t BaseCurrentForcesMaintCost;
static int mutate_radius;
static int base_compute_argument;

struct PlayerData { int current_num_bases; };
static PlayerData PlayersData[8];
struct ConsoleState { int faction_id_; };
static ConsoleState ConsoleGlobal;
static int IsMultiplayerNet;
static int ConsoleControlTurnActive;
static int ExitTurnLoop;
static int wait_hides_window;

struct Map { uint8_t climate; };
static Map map_tile;
static int map_x, map_y, map_calls;
static Map *map_loc(int x, int y) {
    ++map_calls; map_x = x; map_y = y; return &map_tile;
}

struct GraphicWin { virtual void redraw() { event("main.redraw"); } };
struct MapWin : GraphicWin {};
static MapWin primary_map;
static MapWin *MapWinTable[1];
static struct MainInterfaceType : GraphicWin {} MainInterfaceGlobal;

struct Win {
    static int visible;
    int is_visible() { event("visible"); return visible; }
};
int Win::visible;

struct TutWin {
    int stage;
    int reset() { event("tutorial.reset"); return 0; }
    void set_tour_stage(int value) {
        stage = value; event("tutorial.stage");
    }
    void base_rect(RECT *rect, int *x, int *y) {
        event("tutorial.base_rect");
        captured_rect = *rect;
        *x = rect->left + 7; *y = rect->top + 9;
    }
    int tut_win(GraphicWin *owner, const char *label, int x, int y,
                void *sprite, int mode, int tail_a, int tail_b) {
        event("tutorial.tut_win");
        callout_owner = owner; callout_label = label;
        callout_x = x; callout_y = y; callout_sprite = sprite;
        callout_mode = mode; callout_tail_a = tail_a; callout_tail_b = tail_b;
        return 1;
    }
    int do_base(RECT *rect, const char *label, int mode) {
        event("tutorial.do_base");
        captured_rect = *rect; callout_label = label; callout_mode = mode;
        return 1;
    }
    RECT captured_rect;
    GraphicWin *callout_owner;
    std::string callout_label;
    int callout_x, callout_y, callout_mode, callout_tail_a, callout_tail_b;
    void *callout_sprite;
};
static TutWin TutorialWindow;

struct NetDaemon {
    int lock_result;
    int lock_calls, unlock_calls, task_calls;
    int locked_base, lock_mode, lock_a, lock_b, unlocked_base;
    int wait_clears;
    int lock_base(int base, int mode, int a, int b) {
        event("net.lock"); ++lock_calls;
        locked_base = base; lock_mode = mode; lock_a = a; lock_b = b;
        return lock_result;
    }
    void unlock_base(int base) {
        event("net.unlock"); ++unlock_calls; unlocked_base = base;
    }
    void net_tasks();
};
static NetDaemon NetDaemonNet;

struct FX { void play(int id) { assert(id == 15); event("fx.play"); } } g_FX;
static void ambience(int id) { assert(id == 30); event("ambience"); }
static std::vector<int> tut_checks, tut_checks2;
static int check_2, check_10, check_20, check_200, check_40, check_400, check_80;
static int tut_check(int flag) {
    tut_checks.push_back(flag); event("tut_check"); return check_2;
}
static int tut_check2(int flag) {
    tut_checks2.push_back(flag); event("tut_check2");
    switch (flag) {
        case 0x10: return check_10;
        case 0x20: return check_20;
        case 0x200: return check_200;
        case 0x40: return check_40;
        case 0x400: return check_400;
        case 0x80: return check_80;
        default: return 0;
    }
}
static std::vector<std::string> parsed_names;
static void parse_says(int slot, const char *text, int x, int y) {
    assert(slot == 0 && x == -1 && y == -1);
    event("parse_says"); parsed_names.push_back(text);
}
static void bitmask(int input, int *offset, int *mask) {
    *offset = input / 8; *mask = 1 << (input & 7);
}

static int wait_task_calls;
class BaseWin;
static BaseWin *BaseWinWaitTarget;
static void wait_task();
class BaseWin : public Win {
public:
    void zoom(int base_id, int wait);
    void show(int mode) { assert(mode == 0); event("show"); visible = 1; }
    void redraw() { event("base.redraw"); }
    void draw_expenses(int mode) { assert(mode == 2); event("draw_expenses"); }
    int32_t field_40B00_;
    uint32_t focusedBaseID_;
    uint32_t field_40B10_, field_40B14_, field_40B18_;
    int zoom_wait_pending_;
    int displayed_production_id_;
    uint32_t field_1E670_;
    RECT rect1_, rect8_, expenses_help_rects_[6];
};

static void wait_task() {
    event("wait_task"); ++wait_task_calls;
    if (wait_hides_window) Win::visible = 0;
    if (wait_task_calls == NetDaemonNet.wait_clears)
        BaseWinWaitTarget->zoom_wait_pending_ = 0;
}

void NetDaemon::net_tasks() {
    event("net.tasks"); ++task_calls;
    if (wait_hides_window) Win::visible = 0;
    if (task_calls == wait_clears) BaseWinWaitTarget->zoom_wait_pending_ = 0;
}

static void set_base(int base_id) {
    event("set_base"); BaseIDCurrentSelected = base_id; BaseCurrent = &Bases[base_id];
}
static void base_compute(int arg) {
    event("base_compute"); base_compute_argument = arg;
    if (mutate_radius) ++Bases[BaseIDCurrentSelected].resource_sqr_radius;
}

/* ACTUAL_BASEWIN_ZOOM */

static void reset_case() {
    events.clear(); parsed_names.clear(); tut_checks.clear(); tut_checks2.clear();
    std::memset(Bases, 0, sizeof(Bases));
    std::memset(PlayersData, 0, sizeof(PlayersData));
    BaseCurrent = 0; BaseIDCurrentSelected = -1; BaseComputedID = 99;
    GameDataStateStorage.fields_.base_current_count_ = 4;
    BaseCurrentForcesMaintCost = 0; mutate_radius = 0;
    base_compute_argument = -1; ConsoleGlobal.faction_id_ = 1;
    IsMultiplayerNet = 0; ConsoleControlTurnActive = 0; ExitTurnLoop = 0;
    wait_hides_window = 0;
    Win::visible = 0; wait_task_calls = 0; NetDaemonNet = NetDaemon();
    NetDaemonNet.wait_clears = 1; TutorialWindow.stage = -1;
    TutorialWindow.callout_label.clear(); TutorialWindow.callout_owner = 0;
    TutorialWindow.callout_sprite = 0; TutorialWindow.callout_mode = -1;
    TutorialWindow.callout_tail_a = 7; TutorialWindow.callout_tail_b = 7;
    TutorialWindow.callout_x = TutorialWindow.callout_y = -1;
    TutorialWindow.captured_rect = RECT{0, 0, 0, 0};
    check_2 = check_10 = check_20 = check_200 = check_40 = check_400 = check_80 = 0;
    map_tile.climate = 0; map_x = map_y = map_calls = 0;
    MapWinTable[0] = &primary_map;
    Bases[3].faction_id_current = 1;
    Bases[3].population_size = 5;
    Bases[3].resource_sqr_radius = 4;
    Bases[3].queue_production_id[0] = 73;
    Bases[3].state = 0x10;
    Bases[3].x = 12; Bases[3].y = 34;
    std::strcpy(Bases[3].name_string, "TEST BASE");
    Bases[3].mineral_surplus = 8;
    Bases[3].energy_surplus = 4;
    Bases[3].energy_inefficiency = 4;
    for (int i = 0; i < 4; ++i) Bases[i].faction_id_current = 2;
    Bases[3].faction_id_current = 1;
    BaseWinWaitTarget = 0;
}

static BaseWin new_window() {
    BaseWin window;
    window.field_40B00_ = 21;
    window.focusedBaseID_ = 0;
    window.field_40B10_ = window.field_40B14_ = window.field_40B18_ = 8;
    window.zoom_wait_pending_ = 0;
    window.displayed_production_id_ = -1;
    window.field_1E670_ = 0x1234;
    window.rect8_ = RECT{11, 22, 33, 44};
    window.rect1_ = RECT{51, 62, 73, 84};
    for (int i = 0; i < 6; ++i)
        window.expenses_help_rects_[i] = RECT{100 + i * 10, 200 + i * 10,
                                               300 + i * 10, 400 + i * 10};
    return window;
}

static void run_zoom(BaseWin &window, int base_id, int wait) {
    BaseWinWaitTarget = &window;
    window.zoom(base_id, wait);
}

static void check_common(BaseWin &window, int visible_before) {
    assert(window.field_40B00_ == -1 && window.focusedBaseID_ == 3);
    assert(BaseComputedID == -1 && BaseIDCurrentSelected == 3);
    assert(BaseCurrent == &Bases[3] &&
           (BaseCurrent->state == 0 || BaseCurrent->state == 2));
    assert(window.zoom_wait_pending_ == 1 && window.field_40B10_ == 0);
    assert(window.field_40B14_ == 0 && window.field_40B18_ == 1);
    assert(window.displayed_production_id_ == 73 && window.field_1E670_ == 0);
    assert(base_compute_argument == 0);
    assert(position("tutorial.reset") == 0 && position("set_base") > 0);
    assert(position("set_base") < position("base_compute"));
    assert(position("base_compute") < position("fx.play"));
    assert(position("fx.play") < position("ambience"));
    assert(position("ambience") < position(visible_before ? "base.redraw" : "show"));
    assert(position("main.redraw") > position(visible_before ? "base.redraw" : "show"));
    assert(occurrences("main.redraw") == 1);
}

static void check_positioned(const char *label, int stage, bool null_owner) {
    reset_case();
    PlayersData[1].current_num_bases = label[0] == 'B' ? 1 : 2;
    if (stage == 1) {
        check_2 = 1;
        // Verify first-branch priority over every later tutorial family.
        Bases[3].state = 0x12; Bases[3].nutrient_surplus = -2;
        Bases[3].specialist_total = 2;
        check_10 = check_20 = check_200 = check_40 = 1;
    } else {
        Bases[3].state = 0x12; check_10 = 1;
        Bases[3].nutrient_surplus = -2; Bases[3].specialist_total = 2;
        check_20 = 1;
    }
    if (null_owner) MapWinTable[0] = 0;
    BaseWin window = new_window();
    run_zoom(window, 3, 0);
    check_common(window, 0);
    assert(TutorialWindow.stage == stage);
    assert(TutorialWindow.callout_label == label);
    assert(TutorialWindow.callout_owner == (null_owner ? 0 :
           static_cast<GraphicWin *>(&primary_map)));
    assert(TutorialWindow.callout_sprite == 0 && TutorialWindow.callout_mode == 0x100);
    assert(TutorialWindow.callout_tail_a == -1 && TutorialWindow.callout_tail_b == -1);
    assert(TutorialWindow.callout_x == 18 && TutorialWindow.callout_y == 31);
    assert(TutorialWindow.captured_rect.left == 11 &&
           TutorialWindow.captured_rect.top == 22 &&
           TutorialWindow.captured_rect.right == 33 &&
           TutorialWindow.captured_rect.bottom == 44);
    assert(parsed_names.size() == 1 && parsed_names[0] == "TEST BASE");
    assert(position("tutorial.stage") < position("parse_says"));
    assert(position("parse_says") < position("tutorial.base_rect"));
    assert(position("tutorial.base_rect") < position("tutorial.tut_win"));
    assert(occurrences("tutorial.do_base") == 0);
    if (stage == 1) {
        assert(tut_checks.size() == 1 && tut_checks[0] == 2);
        assert(tut_checks2.empty());
    } else {
        assert(tut_checks.empty());
        assert(tut_checks2.size() == 1 && tut_checks2[0] == 0x10);
    }
}

static void check_do_base(const char *label, RECT expected, int mode = 0) {
    assert(TutorialWindow.callout_label == label);
    assert(TutorialWindow.callout_mode == mode);
    assert(TutorialWindow.captured_rect.left == expected.left);
    assert(TutorialWindow.captured_rect.top == expected.top);
    assert(TutorialWindow.captured_rect.right == expected.right);
    assert(TutorialWindow.captured_rect.bottom == expected.bottom);
    assert(position("parse_says") < position("tutorial.do_base"));
    assert(occurrences("tutorial.tut_win") == 0);
    assert(parsed_names.size() == 1 && parsed_names[0] == "TEST BASE");
}

static void run_all_controls() {
    check_positioned("BASECONTROL", 1, true);
    check_positioned("DRONERIOTS", 6, false);

    reset_case();
    Bases[3].nutrient_surplus = -1; Bases[3].specialist_total = 1;
    check_20 = 1;
    BaseWin win_nutrient = new_window(); run_zoom(win_nutrient, 3, 0);
    check_common(win_nutrient, 0);
    assert(tut_checks2.size() == 1 && tut_checks2[0] == 0x20);
    check_do_base("LOWNUTRIENT", RECT{51, 62, 73, 84});

    reset_case();
    Bases[3].population_size = 4; Bases[3].mineral_surplus = 1;
    BaseCurrentForcesMaintCost = 1; check_200 = 1; check_40 = 1;
    BaseWin win_support = new_window(); run_zoom(win_support, 3, 0);
    check_common(win_support, 0);
    assert(tut_checks2.size() == 1 && tut_checks2[0] == 0x200);
    check_do_base("SUPPORT", win_support.expenses_help_rects_[2]);

    // Population four distinguishes the pinned signed population/2 threshold
    // from (population - 1)/2. This also checks the climate-specific label.
    reset_case();
    Bases[3].population_size = 4; Bases[3].mineral_surplus = 1;
    BaseCurrentForcesMaintCost = 1; check_40 = 1;
    map_tile.climate = 0x40;
    BaseWin win_low = new_window(); run_zoom(win_low, 3, 0);
    check_common(win_low, 0);
    assert(tut_checks2.size() == 2 && tut_checks2[0] == 0x200 &&
           tut_checks2[1] == 0x40);
    check_do_base("LOWMINERALS2", win_low.expenses_help_rects_[2]);
    assert(map_calls == 1 && map_x == 12 && map_y == 34);

    reset_case();
    Bases[3].population_size = 4; Bases[3].mineral_surplus = 1;
    BaseCurrentForcesMaintCost = 1; check_40 = 1;
    map_tile.climate = 0x60;
    BaseWin win_high = new_window(); run_zoom(win_high, 3, 0);
    check_common(win_high, 0);
    check_do_base("LOWMINERALS", win_high.expenses_help_rects_[2]);
    assert(map_calls == 1 && map_x == 12 && map_y == 34);

    reset_case();
    Bases[3].energy_inefficiency = 9; Bases[3].energy_surplus = 2;
    check_400 = 1; check_80 = 1;
    BaseWin win_hq = new_window(); run_zoom(win_hq, 3, 0);
    check_common(win_hq, 0);
    assert(tut_checks2.size() == 1 && tut_checks2[0] == 0x400);
    assert(position("draw_expenses") < position("tutorial.do_base"));
    check_do_base("INEFFICIENCYHQ", win_hq.expenses_help_rects_[0]);

    reset_case();
    Bases[3].energy_inefficiency = 9; Bases[3].energy_surplus = 2;
    Bases[1].faction_id_current = 1;
    Bases[1].facilities_built[0] = 1 << 3; // bitmask(base_id=3)
    check_400 = 1; check_80 = 1;
    BaseWin win_facility = new_window(); run_zoom(win_facility, 3, 0);
    check_common(win_facility, 0);
    assert(tut_checks2.size() == 1 && tut_checks2[0] == 0x80);
    assert(position("draw_expenses") < position("tutorial.do_base"));
    check_do_base("INEFFICIENCY", win_facility.expenses_help_rects_[0]);

    reset_case();
    Bases[3].energy_inefficiency = 9; Bases[3].energy_surplus = 2;
    check_400 = 0; check_80 = 1;
    BaseWin win_fallback = new_window(); run_zoom(win_fallback, 3, 0);
    check_common(win_fallback, 0);
    assert(tut_checks2.size() == 2 && tut_checks2[0] == 0x400 &&
           tut_checks2[1] == 0x80);
    check_do_base("INEFFICIENCY", win_fallback.expenses_help_rects_[0]);

    // A declined nutrient tutorial does not terminate the ordered chain.
    reset_case();
    Bases[3].nutrient_surplus = -1; Bases[3].specialist_total = 1;
    check_20 = 0; check_200 = 1;
    Bases[3].population_size = 4; Bases[3].mineral_surplus = 1;
    BaseCurrentForcesMaintCost = 1;
    BaseWin win_nutrient_fallback = new_window();
    run_zoom(win_nutrient_fallback, 3, 0);
    check_common(win_nutrient_fallback, 0);
    assert(tut_checks2.size() == 2 && tut_checks2[0] == 0x20 &&
           tut_checks2[1] == 0x200);
    check_do_base("SUPPORT", win_nutrient_fallback.expenses_help_rects_[2]);

    // A declined low-mineral tutorial continues into the inefficiency gate.
    reset_case();
    Bases[3].population_size = 4; Bases[3].mineral_surplus = 1;
    BaseCurrentForcesMaintCost = 1; check_40 = 0;
    Bases[3].energy_inefficiency = 9; Bases[3].energy_surplus = 2;
    check_400 = 1;
    BaseWin win_low_fallback = new_window(); run_zoom(win_low_fallback, 3, 0);
    check_common(win_low_fallback, 0);
    assert(tut_checks2.size() == 3 && tut_checks2[0] == 0x200 &&
           tut_checks2[1] == 0x40 && tut_checks2[2] == 0x400);
    check_do_base("INEFFICIENCYHQ", win_low_fallback.expenses_help_rects_[0]);

    // The source owner is unsigned storage, but the original comparison is
    // signed: a high-bit value fails support-cost >= 1 and meets cost < pop/2.
    reset_case();
    Bases[3].population_size = 4; Bases[3].mineral_surplus = 1;
    BaseCurrentForcesMaintCost = 0x80000000u; check_40 = 1;
    BaseWin win_signed_cost = new_window(); run_zoom(win_signed_cost, 3, 0);
    check_common(win_signed_cost, 0);
    assert(tut_checks2.size() == 1 && tut_checks2[0] == 0x40);
    check_do_base("LOWMINERALS2", win_signed_cost.expenses_help_rects_[2]);

    // Original selection/display state and multiplayer lock transaction.
    reset_case(); IsMultiplayerNet = 1; mutate_radius = 1;
    NetDaemonNet.lock_result = 0;
    BaseWin win_lock = new_window(); run_zoom(win_lock, 3, 0);
    check_common(win_lock, 0);
    assert(NetDaemonNet.lock_calls == 1 && NetDaemonNet.unlock_calls == 1);
    assert(NetDaemonNet.locked_base == 3 && NetDaemonNet.unlocked_base == 3);
    assert(NetDaemonNet.lock_mode == 0 && NetDaemonNet.lock_a == -1 &&
           NetDaemonNet.lock_b == -1);
    assert(position("net.lock") < position("net.unlock"));

    reset_case(); NetDaemonNet.lock_result = 1;
    IsMultiplayerNet = 1; mutate_radius = 1;
    BaseWin win_lock_refused = new_window(); run_zoom(win_lock_refused, 3, 0);
    assert(NetDaemonNet.lock_calls == 1 && NetDaemonNet.unlock_calls == 0);

    reset_case(); IsMultiplayerNet = 1; mutate_radius = 0;
    BaseWin win_unchanged_radius = new_window();
    run_zoom(win_unchanged_radius, 3, 0);
    assert(Bases[3].resource_sqr_radius == 4);
    assert(NetDaemonNet.lock_calls == 0 && NetDaemonNet.unlock_calls == 0);

    reset_case(); IsMultiplayerNet = 1; mutate_radius = 1;
    ConsoleControlTurnActive = 1;
    BaseWin win_controlled_turn = new_window();
    run_zoom(win_controlled_turn, 3, 0);
    assert(Bases[3].resource_sqr_radius == 5);
    assert(NetDaemonNet.lock_calls == 0 && NetDaemonNet.unlock_calls == 0);

    // Hidden-window show and both wait-service routes rejoin the same tail.
    reset_case();
    BaseWin win_wait = new_window(); run_zoom(win_wait, 3, 1);
    assert(position("show") < position("main.redraw"));
    assert(position("tutorial.do_base") >= 0 || position("tutorial.tut_win") >= 0 ||
           position("wait_task") >= 0);
    assert(wait_task_calls == 1 && win_wait.zoom_wait_pending_ == 0);
    assert(position("wait_task") > position("main.redraw"));

    reset_case();
    BaseWin win_wait_skipped = new_window(); run_zoom(win_wait_skipped, 3, 0);
    assert(wait_task_calls == 0 && NetDaemonNet.task_calls == 0);
    assert(win_wait_skipped.zoom_wait_pending_ == 1);

    reset_case(); Win::visible = 1; ExitTurnLoop = 1;
    BaseWin win_exit_wait = new_window(); run_zoom(win_exit_wait, 3, 1);
    assert(wait_task_calls == 0 && NetDaemonNet.task_calls == 0);
    assert(win_exit_wait.zoom_wait_pending_ == 1);

    reset_case(); Win::visible = 1; wait_hides_window = 1;
    NetDaemonNet.wait_clears = 0;
    BaseWin win_hidden_during_wait = new_window();
    run_zoom(win_hidden_during_wait, 3, 1);
    assert(wait_task_calls == 1 && win_hidden_during_wait.zoom_wait_pending_ == 1);
    assert(Win::visible == 0);

    reset_case(); IsMultiplayerNet = 1;
    BaseWin win_net_wait = new_window(); run_zoom(win_net_wait, 3, 1);
    assert(NetDaemonNet.task_calls == 1 && win_net_wait.zoom_wait_pending_ == 0);
    assert(position("net.tasks") > position("main.redraw"));

    reset_case(); Win::visible = 1;
    BaseWin win_visible = new_window(); run_zoom(win_visible, 3, 0);
    check_common(win_visible, 1);
    assert(position("base.redraw") >= 0 && position("show") == -1);
}

int main() { run_all_controls(); return 0; }
'''


def _translation_unit(source: str) -> str:
    assert HARNESS.count(INSERTION) == 1
    body = _definition(source, "void BaseWin::zoom(int base_id, int wait)")
    return HARNESS.replace(INSERTION, body, 1)


def _compile_and_run(source: str, compiler_name: str, tmp_path: Path,
                     stem: str) -> subprocess.CompletedProcess[str]:
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(f"{compiler_name} unavailable")
    cpp = tmp_path / f"{stem}.cpp"
    executable = tmp_path / stem
    cpp.write_text(_translation_unit(source), encoding="utf-8")
    built = subprocess.run(
        [compiler, "-std=c++11", "-Wall", "-Wextra", "-Werror",
         str(cpp), "-o", str(executable)],
        cwd=ROOT, capture_output=True, text=True, timeout=45,
    )
    assert built.returncode == 0, built.stdout + built.stderr
    return subprocess.run([str(executable)], cwd=ROOT, capture_output=True,
                          text=True, timeout=20)


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
def test_actual_zoom_tutorial_branches_and_rejoin(compiler_name: str,
                                                   tmp_path: Path) -> None:
    source = SOURCE.read_text(encoding="utf-8")
    result = _compile_and_run(source, compiler_name, tmp_path,
                              "basewin_zoom_tutorial_actual")
    assert result.returncode == 0, result.stdout + result.stderr


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
@pytest.mark.parametrize(("name", "old", "new"), (
    ("positioned_tail", '"BASECONTROL", x, y, nullptr, 0x100,\n                               -1, -1)',
     '"BASECONTROL", x, y, nullptr, 0x100,\n                               0, 0)'),
    ("wrong_tutorial_rect", 'TutorialWindow.do_base(&rect1_, "LOWNUTRIENT", 0);',
     'TutorialWindow.do_base(&rect8_, "LOWNUTRIENT", 0);'),
    ("wrong_population_half", '                    < population / 2',
     '                    < (population - 1) / 2'),
    ("wrong_altitude_label", '? "LOWMINERALS2" : "LOWMINERALS"',
     '? "LOWMINERALS" : "LOWMINERALS2"'),
    ("inefficiency_gate", 'tut_check2(0x400) != 0',
     'tut_check2(0x80) != 0'),
    ("support_cost_signed", 'static_cast<int32_t>(BaseCurrentForcesMaintCost) >= 1',
     'BaseCurrentForcesMaintCost >= 1'),
    ("wait_zero_guard", 'if (wait != 0) {', 'if (wait == 0) {'),
    ("lock_unchanged_radius", 'if (IsMultiplayerNet && !ConsoleControlTurnActive\n'
     '        && worked_tiles != Bases[focusedBaseID_].resource_sqr_radius',
     'if (IsMultiplayerNet && !ConsoleControlTurnActive\n'
     '        && worked_tiles == Bases[focusedBaseID_].resource_sqr_radius'),
))
def test_actual_zoom_mutants_are_observable(compiler_name: str, name: str,
                                            old: str, new: str,
                                            tmp_path: Path) -> None:
    source = SOURCE.read_text(encoding="utf-8")
    assert source.count(old) == 1, (name, source.count(old))
    mutant = source.replace(old, new, 1)
    result = _compile_and_run(mutant, compiler_name, tmp_path,
                              f"basewin_zoom_{name}")
    assert result.returncode != 0, f"{name} mutant escaped actual-body controls"
