"""Actual-source controls for Console::hold's live vehicle id."""
from pathlib import Path
import shutil
import subprocess

import pytest


ROOT = next(parent for parent in Path(__file__).resolve().parents if (parent / ".git").exists())
SOURCE = ROOT / "src/console_commands.cpp"
SIGNATURE = "void Console::hold(int veh_id, int waypoint_y) {"


def _definition(text, signature):
    start = text.index(signature)
    brace = text.index("{", start)
    depth = 0
    for end in range(brace, len(text)):
        depth += (text[end] == "{") - (text[end] == "}")
        if depth == 0:
            return text[start:end + 1]
    raise AssertionError("unterminated source definition: " + signature)


def _actual_hold():
    body = _definition(SOURCE.read_text(), SIGNATURE)
    assert "NetDaemonNet.lock_veh(&veh_id, 0, -1, -1, 0)" in body
    assert "lock_vehicle" not in body
    return body.replace("void Console::hold", "void hold", 1)


def _old_copied_local_mutant(body):
    original = "if (NetDaemonNet.lock_veh(&veh_id, 0, -1, -1, 0) != 0) {"
    mutant = (
        "int lock_vehicle = veh_id;\n"
        "    if (NetDaemonNet.lock_veh(&lock_vehicle, 0, -1, -1, 0) != 0) {"
    )
    assert body.count(original) == 1
    return body.replace(original, mutant, 1)


def _old_terrain_inequality_mutant(body):
    original = "(tile.val2 & 0x0F) >= 8) {"
    mutant = "(tile.val2 & 0x0F) < 8) {"
    assert body.count(original) == 2
    return body.replace(original, mutant, 1)


def _harness(body):
    return r'''
#include <assert.h>
#include <stdint.h>

enum { TRIAD_SEA = 1, TRIAD_AIR = 2, ORDER_HOLD = 2,
       PLAN_ALIEN_ARTIFACT = 12 };

struct Veh {
    int x, y, proto_id;
    uint8_t faction_id;
    int16_t waypoint_y[1];
    int8_t order;
};
struct Map { uint8_t climate; uint32_t bit; uint8_t val2; };
struct RulesChassis { int triad, range; };
struct VehPrototype { int chassis_id, plan; };

Veh Vehs[4];
Map Tiles[512];
RulesChassis Chassis[4];
VehPrototype VehPrototypes[4];
const int MapLongitude = 16;

enum { EV_LOCK = 1, EV_SYNCH = 2, EV_AWAIT = 3, EV_UNLOCK = 4,
       EV_CHECK = 5, EV_DRAW = 6, EV_ARTIFACT_ORDER = 7 };
int events[16], event_count;
int fail_lock, lock_update_to, await_update_to;
int synch_id, check_id, draw_x, draw_y, ordered_id;
int synch_seen_waypoint, sync_mutate_coordinates;

struct Daemon {
    int *follow;

    int lock_veh(int *vehicle_id, int, int, int, int) {
        events[event_count++] = EV_LOCK;
        follow = vehicle_id;
        if (fail_lock) {
            follow = 0;
            return 1;
        }
        if (lock_update_to >= 0) *vehicle_id = lock_update_to;
        return 0;
    }
    void await_synch() {
        events[event_count++] = EV_AWAIT;
        if (await_update_to >= 0 && follow != 0) *follow = await_update_to;
    }
    void unlock_veh() {
        events[event_count++] = EV_UNLOCK;
        follow = 0;
    }
    void order_veh(int vehicle_id, int, int) {
        events[event_count++] = EV_ARTIFACT_ORDER;
        ordered_id = vehicle_id;
    }
} NetDaemonNet;

struct TutorialWindow {
    void check_veh(int vehicle_id) {
        events[event_count++] = EV_CHECK;
        check_id = vehicle_id;
    }
} TutWinBaseWindow;

Map *map_tiles() { return Tiles; }

void synch_veh(int vehicle_id) {
    events[event_count++] = EV_SYNCH;
    synch_id = vehicle_id;
    synch_seen_waypoint = Vehs[vehicle_id].waypoint_y[0];
    if (sync_mutate_coordinates) {
        Vehs[vehicle_id].x = 111;
        Vehs[vehicle_id].y = 112;
    }
}

void draw_tile(int x, int y, int) {
    events[event_count++] = EV_DRAW;
    draw_x = x;
    draw_y = y;
}

BODY

void reset_state() {
    for (int i = 0; i != 4; ++i) {
        Vehs[i] = Veh();
        Chassis[i] = RulesChassis();
        VehPrototypes[i] = VehPrototype();
    }
    for (int i = 0; i != 512; ++i) Tiles[i] = Map();
    event_count = 0;
    fail_lock = 0;
    lock_update_to = -1;
    await_update_to = -1;
    synch_id = check_id = ordered_id = -99;
    draw_x = draw_y = -99;
    synch_seen_waypoint = -99;
    sync_mutate_coordinates = 0;
    NetDaemonNet.follow = 0;
}

void expect_prefix(const int *expected, int count) {
    assert(event_count == count);
    for (int i = 0; i != count; ++i) assert(events[i] == expected[i]);
}

void lock_failure_is_early() {
    reset_state();
    fail_lock = 1;
    const int events_expected[] = { EV_LOCK };
    hold(0, 77);
    expect_prefix(events_expected, 1);
    assert(Vehs[0].order == 0 && Vehs[0].waypoint_y[0] == 0);
    assert(synch_id == -99 && check_id == -99 && draw_x == -99);
}

void unchanged_selection_uses_expected_order() {
    reset_state();
    Vehs[0].x = 4; Vehs[0].y = 5; Vehs[0].proto_id = 0;
    Chassis[0].triad = 1;
    Tiles[16 * 5 + (4 >> 1)].climate = 0x60;
    const int events_expected[] = { EV_LOCK, EV_SYNCH, EV_AWAIT, EV_UNLOCK,
                                    EV_CHECK, EV_DRAW };
    hold(0, 77);
    expect_prefix(events_expected, 6);
    assert(Vehs[0].waypoint_y[0] == 77 && Vehs[0].order == ORDER_HOLD);
    assert(synch_id == 0 && synch_seen_waypoint == 77 && check_id == 0);
    assert(draw_x == 4 && draw_y == 5 && ordered_id == -99);
}

void lock_update_selects_live_vehicle_and_snapshots_coordinates() {
    reset_state();
    lock_update_to = 1;
    sync_mutate_coordinates = 1;
    Vehs[1].x = 7; Vehs[1].y = 8; Vehs[1].proto_id = 1;
    Vehs[1].faction_id = 4;
    Chassis[1].triad = 1;
    VehPrototypes[1].plan = PLAN_ALIEN_ARTIFACT;
    Tiles[16 * 8 + (7 >> 1)].climate = 0x60;
    Tiles[16 * 8 + (7 >> 1)].bit = 1;
    Tiles[16 * 8 + (7 >> 1)].val2 = 4;
    const int events_expected[] = { EV_LOCK, EV_SYNCH, EV_AWAIT, EV_UNLOCK,
                                    EV_CHECK, EV_DRAW, EV_ARTIFACT_ORDER };
    hold(0, 66);
    expect_prefix(events_expected, 7);
    assert(Vehs[1].waypoint_y[0] == 66 && Vehs[1].order == ORDER_HOLD);
    assert(synch_id == 1 && synch_seen_waypoint == 66 && check_id == 1);
    assert(draw_x == 7 && draw_y == 8);
    assert(ordered_id == 1);
}

void await_dispatch_update_is_used_afterward_but_draw_keeps_snapshot() {
    reset_state();
    await_update_to = 2;
    sync_mutate_coordinates = 1;
    Vehs[1].x = 9; Vehs[1].y = 10; Vehs[1].proto_id = 0;
    Chassis[0].triad = 1;
    Vehs[2].proto_id = 2; Vehs[2].faction_id = 3;
    VehPrototypes[2].plan = PLAN_ALIEN_ARTIFACT;
    Tiles[16 * 10 + (9 >> 1)].climate = 0x60;
    Tiles[16 * 10 + (9 >> 1)].bit = 1;
    Tiles[16 * 10 + (9 >> 1)].val2 = 3;
    const int events_expected[] = { EV_LOCK, EV_SYNCH, EV_AWAIT, EV_UNLOCK,
                                    EV_CHECK, EV_DRAW, EV_ARTIFACT_ORDER };
    hold(1, 55);
    expect_prefix(events_expected, 7);
    assert(Vehs[1].waypoint_y[0] == 55 && Vehs[1].order == ORDER_HOLD);
    assert(synch_id == 1 && synch_seen_waypoint == 55);
    assert(check_id == 2);
    assert(draw_x == 9 && draw_y == 10);
    assert(ordered_id == 2);
}

void low_climate_land_base_with_valid_owner_is_admitted() {
    reset_state();
    Vehs[0].x = 3; Vehs[0].y = 4; Vehs[0].proto_id = 0;
    Tiles[16 * 4 + (3 >> 1)].climate = 0x20;
    Tiles[16 * 4 + (3 >> 1)].bit = 1;
    Tiles[16 * 4 + (3 >> 1)].val2 = 3;
    const int events_expected[] = { EV_LOCK, EV_SYNCH, EV_AWAIT, EV_UNLOCK,
                                    EV_CHECK, EV_DRAW };
    hold(0, 33);
    expect_prefix(events_expected, 6);
    assert(Vehs[0].waypoint_y[0] == 33 && Vehs[0].order == ORDER_HOLD);
    assert(synch_id == 0 && check_id == 0 && draw_x == 3 && draw_y == 4);
}

void low_climate_land_without_base_is_rejected() {
    reset_state();
    Vehs[0].x = 3; Vehs[0].y = 4;
    Tiles[16 * 4 + (3 >> 1)].climate = 0x20;
    Tiles[16 * 4 + (3 >> 1)].bit = 0;
    const int events_expected[] = { EV_LOCK, EV_UNLOCK };
    hold(0, 33);
    expect_prefix(events_expected, 2);
    assert(Vehs[0].waypoint_y[0] == 0 && Vehs[0].order == 0);
}

void sea_unit_bypasses_land_base_gate() {
    reset_state();
    Vehs[0].x = 3; Vehs[0].y = 4;
    Chassis[0].triad = TRIAD_SEA;
    Tiles[16 * 4 + (3 >> 1)].climate = 0x20;
    Tiles[16 * 4 + (3 >> 1)].bit = 0;
    const int events_expected[] = { EV_LOCK, EV_SYNCH, EV_AWAIT, EV_UNLOCK,
                                    EV_CHECK, EV_DRAW };
    hold(0, 33);
    expect_prefix(events_expected, 6);
    assert(Vehs[0].waypoint_y[0] == 33 && Vehs[0].order == ORDER_HOLD);
}

void invalid_owner_nibble_boundary_is_rejected() {
    reset_state();
    Vehs[0].x = 3; Vehs[0].y = 4;
    Tiles[16 * 4 + (3 >> 1)].climate = 0x20;
    Tiles[16 * 4 + (3 >> 1)].bit = 1;
    Tiles[16 * 4 + (3 >> 1)].val2 = 9;
    const int events_expected[] = { EV_LOCK, EV_UNLOCK };
    hold(0, 33);
    expect_prefix(events_expected, 2);
    assert(Vehs[0].waypoint_y[0] == 0 && Vehs[0].order == 0);
    /* Nibble 9 is only the invalid-owner boundary control; no reachability claim. */
}

void air_range_requires_base_or_airbase_exception() {
    reset_state();
    Vehs[0].x = 3; Vehs[0].y = 4;
    Chassis[0].triad = TRIAD_AIR;
    Chassis[0].range = 1;
    Tiles[16 * 4 + (3 >> 1)].climate = 0x20;
    const int rejected[] = { EV_LOCK, EV_UNLOCK };
    hold(0, 33);
    expect_prefix(rejected, 2);

    reset_state();
    Vehs[0].x = 3; Vehs[0].y = 4;
    Chassis[0].triad = TRIAD_AIR;
    Chassis[0].range = 1;
    Tiles[16 * 4 + (3 >> 1)].climate = 0x20;
    Tiles[16 * 4 + (3 >> 1)].bit = 0x40000;
    const int admitted[] = { EV_LOCK, EV_SYNCH, EV_AWAIT, EV_UNLOCK,
                             EV_CHECK, EV_DRAW };
    hold(0, 33);
    expect_prefix(admitted, 6);
    assert(Vehs[0].waypoint_y[0] == 33 && Vehs[0].order == ORDER_HOLD);
}

void air_range_accepts_valid_base_and_range_zero_disables_air_gate() {
    reset_state();
    Vehs[0].x = 3; Vehs[0].y = 4;
    Chassis[0].triad = TRIAD_AIR;
    Chassis[0].range = 1;
    Tiles[16 * 4 + (3 >> 1)].climate = 0x20;
    Tiles[16 * 4 + (3 >> 1)].bit = 1;
    Tiles[16 * 4 + (3 >> 1)].val2 = 3;
    const int admitted[] = { EV_LOCK, EV_SYNCH, EV_AWAIT, EV_UNLOCK,
                             EV_CHECK, EV_DRAW };
    hold(0, 33);
    expect_prefix(admitted, 6);

    reset_state();
    Vehs[0].x = 3; Vehs[0].y = 4;
    Chassis[0].triad = TRIAD_AIR;
    Chassis[0].range = 0;
    Tiles[16 * 4 + (3 >> 1)].climate = 0x20;
    const int range_zero[] = { EV_LOCK, EV_SYNCH, EV_AWAIT, EV_UNLOCK,
                               EV_CHECK, EV_DRAW };
    hold(0, 33);
    expect_prefix(range_zero, 6);
}

// Sentinel width control; faction 255 is a malformed-state test, not a
// reachability assertion. All records used here are within the host tables.
void artifact_owner_comparison_keeps_full_width() {
    const int owners[] = {3, 3, 8, 15, 8};
    const int factions[] = {3, 4, 255, 255, 255};
    for (int i = 0; i < 5; ++i) {
        reset_state();
        const bool after_await = i == 4;
        lock_update_to = 1;
        await_update_to = after_await ? 2 : -1;
        Vehs[1].x = 9; Vehs[1].y = 10; Vehs[1].proto_id = 1;
        VehPrototypes[1].chassis_id = 1;
        Chassis[1].triad = TRIAD_SEA;
        const int selected = after_await ? 2 : 1;
        Vehs[selected].proto_id = selected;
        Vehs[selected].faction_id = static_cast<uint8_t>(factions[i]);
        VehPrototypes[selected].plan = PLAN_ALIEN_ARTIFACT;
        Tiles[16 * 10 + (9 >> 1)].bit = 1;
        Tiles[16 * 10 + (9 >> 1)].val2 = static_cast<uint8_t>(owners[i]);
        const int expected[] = {EV_LOCK, EV_SYNCH, EV_AWAIT, EV_UNLOCK,
                                EV_CHECK, EV_DRAW, EV_ARTIFACT_ORDER};
        hold(0, 55);
        expect_prefix(expected, i == 0 ? 7 : 6);
        assert(synch_id == 1 && check_id == selected);
        assert(draw_x == 9 && draw_y == 10);
        assert(ordered_id == (i == 0 ? selected : -99));
    }
}

int main() {
    artifact_owner_comparison_keeps_full_width();
    lock_failure_is_early();
    unchanged_selection_uses_expected_order();
    lock_update_selects_live_vehicle_and_snapshots_coordinates();
    await_dispatch_update_is_used_afterward_but_draw_keeps_snapshot();
    low_climate_land_base_with_valid_owner_is_admitted();
    low_climate_land_without_base_is_rejected();
    sea_unit_bypasses_land_base_gate();
    invalid_owner_nibble_boundary_is_rejected();
    air_range_requires_base_or_airbase_exception();
    air_range_accepts_valid_base_and_range_zero_disables_air_gate();
    return 0;
}
'''.replace("BODY", body)


def _compile_run(tmp_path, compiler, source, stem):
    cpp = tmp_path / (stem + ".cpp")
    exe = tmp_path / stem
    cpp.write_text(source)
    built = subprocess.run(
        (compiler, "-std=c++11", "-Wall", "-Wextra", "-Werror", str(cpp), "-o", str(exe)),
        capture_output=True, text=True, timeout=60,
    )
    return built, exe


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
def test_actual_hold_live_selection_and_callback_order(tmp_path, compiler_name):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(compiler_name + " unavailable")
    built, exe = _compile_run(tmp_path, compiler, _harness(_actual_hold()), "actual_hold")
    assert built.returncode == 0, built.stderr
    ran = subprocess.run((str(exe),), capture_output=True, text=True, timeout=10)
    assert ran.returncode == 0, ran.stdout + ran.stderr


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
def test_actual_hold_copied_local_mutant_compiles_then_fails(tmp_path, compiler_name):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(compiler_name + " unavailable")
    mutant = _old_copied_local_mutant(_actual_hold())
    built, exe = _compile_run(tmp_path, compiler, _harness(mutant), "copied_local_mutant")
    assert built.returncode == 0, built.stderr
    ran = subprocess.run((str(exe),), capture_output=True, text=True, timeout=10)
    assert ran.returncode != 0


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
def test_actual_hold_old_terrain_inequality_compiles_then_fails(tmp_path, compiler_name):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(compiler_name + " unavailable")
    mutant = _old_terrain_inequality_mutant(_actual_hold())
    built, exe = _compile_run(tmp_path, compiler, _harness(mutant), "old_terrain_inequality")
    assert built.returncode == 0, built.stderr
    ran = subprocess.run((str(exe),), capture_output=True, text=True, timeout=10)
    assert ran.returncode != 0


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
def test_actual_hold_old_narrowed_owner_compiles_then_fails(tmp_path, compiler_name):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(compiler_name + " unavailable")
    body = _actual_hold()
    original = "if (owner == Vehs[veh_id].faction_id)"
    assert body.count(original) == 1
    mutant = body.replace(original, "if (static_cast<uint8_t>(owner) == Vehs[veh_id].faction_id)", 1)
    built, exe = _compile_run(tmp_path, compiler, _harness(mutant), "old_narrowed_owner")
    assert built.returncode == 0, built.stderr
    ran = subprocess.run((str(exe),), capture_output=True, text=True, timeout=10)
    assert ran.returncode != 0
