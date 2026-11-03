"""Actual-source controls for Console::automate's borrowed live vehicle ID."""
from pathlib import Path
import shutil
import subprocess

import pytest


ROOT = next(parent for parent in Path(__file__).resolve().parents if (parent / ".git").exists())
SOURCE = ROOT / "src/console_commands.cpp"
SIGNATURE = "void Console::automate(int veh_id, int order) {"


def _definition(text, signature):
    start = text.index(signature)
    brace = text.index("{", start)
    depth = 0
    for end in range(brace, len(text)):
        depth += (text[end] == "{") - (text[end] == "}")
        if depth == 0:
            return text[start:end + 1]
    raise AssertionError("unterminated source definition: " + signature)


def _actual_automate():
    body = _definition(SOURCE.read_text(), SIGNATURE)
    assert "NetDaemonNet.lock_veh(&veh_id, 0, -1, -1, 0)" in body
    assert "Veh &locked_vehicle = Vehs[veh_id];" in body
    assert "Veh &after_home_vehicle = Vehs[veh_id];" in body
    assert "Veh &updated_vehicle = Vehs[veh_id];" in body
    return body.replace("void Console::automate", "void automate", 1)


def _actual_map_helpers():
    header = (ROOT / "src/map.h").read_text()
    faction = (ROOT / "src/faction.h").read_text()
    base_bit = next(line.strip().rstrip(",") for line in header.splitlines()
                    if line.strip().startswith("BIT_BASE_IN_TILE ="))
    player_count = next(line for line in faction.splitlines()
                        if line.startswith("static const int MaxPlayerNum ="))
    return "\n".join(("enum { " + base_bit + " };", player_count,
                       _definition(header, "inline Map *&map_tiles()"),
                       _definition(header, "inline Map *__cdecl map_loc("),
                       _definition(header, "MEASURED inline int __cdecl base_who(")))


def _old_prelock_reference_mutant(body):
    replacements = (
        ("Veh &locked_vehicle = Vehs[veh_id];", "Veh &locked_vehicle = vehicle;"),
        ("        const VehPrototype &locked_prototype =\n            VehPrototypes[locked_vehicle.proto_id];\n", ""),
        ("Chassis[locked_prototype.chassis_id];", "chassis;"),
        ("if (Vehs[veh_id].order == 0x18)", "if (vehicle.order == 0x18)"),
        ("Veh &after_home_vehicle = Vehs[veh_id];", "Veh &after_home_vehicle = vehicle;"),
        ("Vehs[veh_id].order = 0;", "vehicle.order = 0;"),
        ("Veh &updated_vehicle = Vehs[veh_id];", "Veh &updated_vehicle = vehicle;"),
    )
    for old, new in replacements:
        assert body.count(old) == 1, old
        body = body.replace(old, new, 1)
    return body


def _harness(body):
    return r'''
#include <assert.h>
#include <stdint.h>
#define __cdecl
#define MEASURED

enum { TRIAD_LAND = 0, TRIAD_AIR = 2, TERRA_FARM = 0 };

struct Veh {
    int16_t x, y;
    uint32_t state;
    uint16_t flags;
    int16_t proto_id;
    uint16_t unknown;
    uint8_t faction_id, year_end_lurking, damage;
    int8_t order;
    uint8_t waypoint_count, patrol_current_point;
    int16_t waypoint_x[4], waypoint_y[4];
    uint8_t morale, terraforming_turns, order_auto_type, visibility;
    uint8_t moves_expended;
    int8_t facing;
    uint8_t unknown6, move_to_ai_type, probe_action, probe_sabotage_id;
    int16_t home_base_id, next_veh_id_stack, prev_veh_id_stack;
};
struct Map { uint8_t climate, contour, val2, region, visibility, val3, unknown; int8_t territory; uint32_t bit; };
struct RulesChassis { uint8_t triad, range; };
struct VehPrototype { uint8_t chassis_id, weapon_id, plan; };
struct RulesWeapon { int8_t offense_rating; };
struct TerraformAction { int prerequisite_tech[2]; };

Veh Vehs[4];
Map Tiles[512];
Map *MapTiles = Tiles;
RulesChassis Chassis[4];
VehPrototype VehPrototypes[4];
RulesWeapon Weapon[4];
TerraformAction Terraforming[16];
const int MapLongitude = 16;
int selected_vehicle = -1;
int tech_available = 1;

enum { EV_LOCK = 1, EV_HOME = 2, EV_SYNCH = 3, EV_AWAIT = 4,
       EV_UNLOCK = 5, EV_CURSOR = 6 };
int events[32], event_count;
int fail_lock, lock_update_to, home_update_to;
int lock_seen_id, home_seen_id, synch_id, cursor_mode;

struct Daemon {
    int *follow;
    int lock_veh(int *vehicle_id, int, int, int, int) {
        events[event_count++] = EV_LOCK;
        lock_seen_id = *vehicle_id;
        follow = vehicle_id;
        if (fail_lock) {
            follow = 0;
            return 1;
        }
        if (lock_update_to >= 0) *vehicle_id = lock_update_to;
        return 0;
    }
    void await_synch() { events[event_count++] = EV_AWAIT; }
    void unlock_veh() { events[event_count++] = EV_UNLOCK; follow = 0; }
} NetDaemonNet;

struct ConsoleState {
    void cursor_on(int mode, int) {
        events[event_count++] = EV_CURSOR;
        cursor_mode = mode;
    }
} ConsoleGlobal;

MAP_HELPERS
int has_tech(int, int) { return tech_available; }
void action_home(int vehicle_id, int) {
    events[event_count++] = EV_HOME;
    home_seen_id = vehicle_id;
    if (home_update_to >= 0 && NetDaemonNet.follow != 0) {
        *NetDaemonNet.follow = home_update_to;
        Vehs[home_update_to].order = 0x18;
        Vehs[home_update_to].waypoint_x[0] = 31;
        Vehs[home_update_to].waypoint_y[0] = 41;
        selected_vehicle = home_update_to;
    }
}
void synch_veh(int vehicle_id) {
    events[event_count++] = EV_SYNCH;
    synch_id = vehicle_id;
}

BODY

void reset_state() {
    for (int i = 0; i != 4; ++i) {
        Vehs[i] = Veh();
        Chassis[i] = RulesChassis();
        VehPrototypes[i] = VehPrototype();
        Weapon[i] = RulesWeapon();
    }
    for (int i = 0; i != 512; ++i) Tiles[i] = Map();
    for (int i = 0; i != 16; ++i) Terraforming[i] = TerraformAction();
    event_count = 0;
    fail_lock = 0;
    lock_update_to = -1;
    home_update_to = -1;
    lock_seen_id = home_seen_id = synch_id = -99;
    cursor_mode = -99;
    selected_vehicle = -1;
    tech_available = 1;
    NetDaemonNet.follow = 0;
}

void expect_events(const int *expected, int count) {
    assert(event_count == count);
    for (int i = 0; i != count; ++i) assert(events[i] == expected[i]);
}

void prelock_validation_and_lock_failure_are_early() {
    reset_state();
    Vehs[0].proto_id = 0;
    VehPrototypes[0].plan = 9;
    Chassis[0].triad = TRIAD_LAND;
    Vehs[0].x = 2; Vehs[0].y = 3;
    Tiles[MapLongitude * 3 + (2 >> 1)].climate = 0x20;
    lock_update_to = 1;
    automate(0, 1);
    assert(event_count == 0);
    assert(Vehs[0].order_auto_type == 0 && Vehs[1].order_auto_type == 0);

    reset_state();
    fail_lock = 1;
    const int failed[] = { EV_LOCK };
    automate(0, 3);
    expect_events(failed, 1);
    assert(lock_seen_id == 0 && synch_id == -99 && NetDaemonNet.follow == 0);
}

void unchanged_selection_preserves_common_write_order() {
    reset_state();
    Vehs[0].state = 0x01000003;
    const int expected[] = { EV_LOCK, EV_SYNCH, EV_AWAIT, EV_UNLOCK };
    automate(0, 3);
    expect_events(expected, 4);
    assert(Vehs[0].order_auto_type == 3 && Vehs[0].order == 0);
    assert(Vehs[0].state == ((0x01000003u & 0xFCFFFFFFu) | 0x200u));
    assert(Vehs[0].waypoint_count == 0 && synch_id == 0);
}

void lock_renumber_uses_selected_vehicle_and_its_chassis_range() {
    reset_state();
    lock_update_to = 1;
    home_update_to = -1;
    Vehs[0].proto_id = 0;
    VehPrototypes[0].chassis_id = 0;
    Chassis[0].range = 0;
    Vehs[1].x = 7; Vehs[1].y = 8; Vehs[1].proto_id = 1;
    VehPrototypes[1].chassis_id = 1;
    Chassis[1].range = 1;
    const int expected[] = { EV_LOCK, EV_HOME, EV_SYNCH, EV_AWAIT, EV_UNLOCK };
    automate(0, 0xB);
    expect_events(expected, 5);
    assert(home_seen_id == 1 && synch_id == 1);
    assert(Vehs[0].waypoint_x[1] == 0 && Vehs[0].waypoint_y[1] == 0);
    assert(Vehs[1].waypoint_x[1] == 7 && Vehs[1].waypoint_y[1] == 8);
    assert(Vehs[1].order_auto_type == 0xB && Vehs[1].order == 0);
}

void home_callback_renumber_uses_current_order18_vehicle() {
    reset_state();
    lock_update_to = 1;
    home_update_to = 2;
    Vehs[0].proto_id = 0;
    VehPrototypes[0].chassis_id = 0;
    Chassis[0].range = 0;
    Vehs[1].x = 7; Vehs[1].y = 8; Vehs[1].proto_id = 1;
    VehPrototypes[1].chassis_id = 1;
    Chassis[1].range = 1;
    Vehs[2].proto_id = 2;
    const int expected[] = { EV_LOCK, EV_HOME, EV_SYNCH, EV_AWAIT, EV_UNLOCK };
    automate(0, 0xB);
    expect_events(expected, 5);
    assert(home_seen_id == 1 && selected_vehicle == 2);
    assert(Vehs[1].waypoint_x[1] == 7 && Vehs[1].waypoint_y[1] == 8);
    assert(Vehs[2].waypoint_x[1] == 31 && Vehs[2].waypoint_y[1] == 41);
    assert(Vehs[2].order_auto_type == 0xB && Vehs[2].order == 0);
    assert(synch_id == 2);
}

void direct_air_order_keeps_prelock_path() {
    reset_state();
    Vehs[0].proto_id = 0;
    VehPrototypes[0].plan = 8;
    VehPrototypes[0].chassis_id = 0;
    VehPrototypes[0].weapon_id = 0;
    Chassis[0].triad = TRIAD_AIR;
    Weapon[0].offense_rating = 1;
    const int expected[] = { EV_CURSOR };
    automate(0, 10);
    expect_events(expected, 1);
    assert(cursor_mode == 0xB && Vehs[0].order == 0);
}

int main() {
    prelock_validation_and_lock_failure_are_early();
    unchanged_selection_preserves_common_write_order();
    lock_renumber_uses_selected_vehicle_and_its_chassis_range();
    home_callback_renumber_uses_current_order18_vehicle();
    direct_air_order_keeps_prelock_path();
    return 0;
}
'''.replace("BODY", body).replace("MAP_HELPERS", _actual_map_helpers())


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
def test_actual_automate_uses_live_id_across_lock_and_home_callback(tmp_path, compiler_name):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(compiler_name + " unavailable")
    built, exe = _compile_run(tmp_path, compiler, _harness(_actual_automate()), "actual_automate")
    assert built.returncode == 0, built.stderr
    ran = subprocess.run((str(exe),), capture_output=True, text=True, timeout=10)
    assert ran.returncode == 0, ran.stdout + ran.stderr


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
def test_actual_automate_old_prelock_reference_mutant_compiles_then_fails(tmp_path, compiler_name):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(compiler_name + " unavailable")
    mutant = _old_prelock_reference_mutant(_actual_automate())
    built, exe = _compile_run(tmp_path, compiler, _harness(mutant), "old_prelock_reference_mutant")
    assert built.returncode == 0, built.stderr
    ran = subprocess.run((str(exe),), capture_output=True, text=True, timeout=10)
    assert ran.returncode != 0
