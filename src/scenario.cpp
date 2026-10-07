/*
 * OpenSMACX - an open source clone of Sid Meier's Alpha Centauri.
 * Copyright (C) 2013-2021 Brendan Casey
 *
 * OpenSMACX is free software: you can redistribute it and / or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 */
#include "stdafx.h"
#include "scenario.h"

#include "alpha.h"
#include "game.h"
#include "map.h"
#include "maininterface.h"

#include "veh.h"

#include <cerrno>
#include <climits>
#include <cstring>

extern "C" void __cdecl scenario_human_turn_ready(Console *self);

namespace {

// Historical finding (superseded): the 2026-09-05 absolute-reference
// scan missed MainInterface's array constructor because it searched for an
// immediate-address vptr store. Pinned constructor 0x0045EF10 builds the
// 42-element FlatButton member array at this+0xE00, stride 0xB4C; element
// 2 is 0x007B0CB8. Keep the same object's behavior through that owner.
static const uintptr_t TurnUpkeepCallerReturn = 0x0052768F;
// The turn-loop exit flag and the turn phase are named globals now - see
// game.h (ExitTurnLoop, ControlTurnPhase); the raw uintptr_t bindings that
// stood here were retired when control_game was homed into game.cpp on
// 2026-08-29 and both addresses gained real definitions.

// A namespaced plain enum rather than `enum class`, which VC6 does not have.
// It keeps every `ScenarioPhase::Finished` at the sixteen use sites spelt
// exactly as before; what is lost is the distinct type, so the enumerators
// convert to int again as they did in C++98.
namespace ScenarioPhase {
enum Value {
    Inactive,
    Loaded,
    AwaitingAdvance,
    Finished,
};
}  // namespace ScenarioPhase

struct ScenarioState {
    ScenarioPhase::Value phase;
    char save_path[1024];
    char result_path[1024];
    bool inspect_only;
    bool resolve_movement;
    bool advance_turn;
    int vehicle_id;
    int target_x;
    int target_y;
    int initial_turn;
    int initial_x;
    int initial_y;
    int movement_cost;
};

// `{0}` would be a `const int` initialising the leading enum member, which
// VC6 will not narrow; naming the enumerator says the same thing everywhere.
ScenarioState State = {ScenarioPhase::Inactive};

bool read_environment(LPCSTR name, char *output, DWORD size) {
    DWORD length = GetEnvironmentVariableA(name, output, size);
    return length > 0 && length < size;
}

bool parse_environment_int(LPCSTR name, int &value) {
    char input[32];
    if (!read_environment(name, input, sizeof(input))) {
        return false;
    }
    char *end = nullptr;
    errno = 0;
    long parsed = strtol(input, &end, 10);
    if (errno != 0 || end == input || *end != '\0' || parsed < INT_MIN || parsed > INT_MAX) {
        return false;
    }
    value = static_cast<int>(parsed);
    return true;
}

void write_bytes(const char *data, DWORD size) {
    HANDLE file = CreateFileA(State.result_path, GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS,
                              FILE_ATTRIBUTE_NORMAL, nullptr);
    if (file == INVALID_HANDLE_VALUE) {
        return;
    }
    DWORD written;
    WriteFile(file, data, size, &written, nullptr);
    FlushFileBuffers(file);
    CloseHandle(file);
}

void write_failure(const char *error) {
    char output[512];
    int size = snprintf(output, sizeof(output),
        "{\n  \"status\": \"failed\",\n  \"error\": \"%s\"\n}\n", error);
    if (size > 0 && static_cast<size_t>(size) < sizeof(output)) {
        write_bytes(output, static_cast<DWORD>(size));
    }
}

void write_progress(const char *phase) {
    char output[256];
    int size = snprintf(output, sizeof(output),
        "{\n  \"status\": \"running\",\n  \"phase\": \"%s\"\n}\n", phase);
    if (size > 0 && static_cast<size_t>(size) < sizeof(output)) {
        write_bytes(output, static_cast<DWORD>(size));
    }
}

void finish_failure(const char *error) {
    State.phase = ScenarioPhase::Finished;
    ExitTurnLoop = 1;
    write_failure(error);
}

bool configure_scenario() {
    if (!read_environment("OPENSMACX_SCENARIO_SAVE", State.save_path,
                          sizeof(State.save_path))) {
        return false;
    }
    State.phase = ScenarioPhase::Loaded;
    if (!read_environment("OPENSMACX_SCENARIO_RESULT", State.result_path,
                          sizeof(State.result_path))) {
        State.phase = ScenarioPhase::Inactive;
        return false;
    }

    char inspect[8];
    State.inspect_only = read_environment(
        "OPENSMACX_SCENARIO_INSPECT", inspect, sizeof(inspect));
    char resolve[8];
    State.resolve_movement = read_environment(
        "OPENSMACX_SCENARIO_RESOLVE", resolve, sizeof(resolve));
    char advance[8];
    State.advance_turn = read_environment(
        "OPENSMACX_SCENARIO_ADVANCE", advance, sizeof(advance));
    if (State.advance_turn) {
        State.resolve_movement = true;
    }
    if (!State.inspect_only
        && (!parse_environment_int("OPENSMACX_SCENARIO_VEHICLE", State.vehicle_id)
            || !parse_environment_int("OPENSMACX_SCENARIO_X", State.target_x)
            || !parse_environment_int("OPENSMACX_SCENARIO_Y", State.target_y))) {
        State.phase = ScenarioPhase::Finished;
        write_failure("invalid_command_environment");
    }
    return true;
}

void refresh_loaded_game() {
    for (int faction = 1; faction < 8; faction++) {
        load_faction_art(faction);
    }
    load_flags();
    GameDataStateStorage.fields_.game_state_ &= 0xF79FCF16;

    // Original slot 2 on this selected FlatButton view is its hide action.
    MainInterfaceGlobal.flat_button_[2].hide();
    prefs_use();
}

bool validate_loaded_state() {
    return !IsMultiplayerNet && !IsMultiplayerPBEM
        && GameDataStateStorage.fields_.vehicle_current_count_ > 0 && GameDataStateStorage.fields_.vehicle_current_count_ <= 2049;
}

void inspect_loaded_state() {
    int current_faction = ConsoleGlobal.faction_id_;
    int candidate = -1;
    for (int vehicle = 0; vehicle < GameDataStateStorage.fields_.vehicle_current_count_; vehicle++) {
        if (Vehs[vehicle].faction_id == current_faction && veh_moves(vehicle) > 0) {
            candidate = vehicle;
            break;
        }
    }
    if (candidate < 0) {
        finish_failure("no_movable_current_faction_vehicle");
        return;
    }

    char output[2048];
    int size = snprintf(output, sizeof(output),
        "{\n"
        "  \"status\": \"inspected\",\n"
        "  \"turn\": %d,\n"
        "  \"current_faction\": %d,\n"
        "  \"vehicle_count\": %d,\n"
        "  \"candidate\": {\"id\": %d, \"x\": %d, \"y\": %d, "
        "\"prototype\": %d, \"triad\": %u, \"moves\": %u},\n"
        "  \"adjacent\": [\n",
        GameDataStateStorage.fields_.turn_current_num_, current_faction, GameDataStateStorage.fields_.vehicle_current_count_, candidate,
        Vehs[candidate].x, Vehs[candidate].y, Vehs[candidate].proto_id,
        get_proto_triad(Vehs[candidate].proto_id), veh_moves(candidate));
    if (size <= 0 || static_cast<size_t>(size) >= sizeof(output)) {
        finish_failure("inspection_result_overflow");
        return;
    }
    size_t used = static_cast<size_t>(size);
    for (int index = 1; index < RadiusRange[1]; index++) {
        int x = xrange(Vehs[candidate].x + RadiusOffsetX[index]);
        int y = Vehs[candidate].y + RadiusOffsetY[index];
        int occupied = on_map(x, y) ? veh_at(x, y) : -2;
        int cost = on_map(x, y)
            ? hex_cost(Vehs[candidate].proto_id, Vehs[candidate].faction_id,
                       Vehs[candidate].x, Vehs[candidate].y, x, y, false)
            : -1;
        int added = snprintf(output + used, sizeof(output) - used,
            "    {\"x\": %d, \"y\": %d, \"occupied\": %d, \"cost\": %d, "
            "\"ocean\": %s, \"base\": %s}%s\n",
            x, y, occupied, cost,
            on_map(x, y) && is_ocean(x, y) ? "true" : "false",
            on_map(x, y) && (bit_at(x, y) & BIT_BASE_IN_TILE) ? "true" : "false",
            index + 1 < RadiusRange[1] ? "," : "");
        if (added <= 0 || static_cast<size_t>(added) >= sizeof(output) - used) {
            finish_failure("inspection_result_overflow");
            return;
        }
        used += static_cast<size_t>(added);
    }
    int added = snprintf(output + used, sizeof(output) - used, "  ]\n}\n");
    if (added <= 0 || static_cast<size_t>(added) >= sizeof(output) - used) {
        finish_failure("inspection_result_overflow");
        return;
    }
    used += static_cast<size_t>(added);
    State.phase = ScenarioPhase::Finished;
    ExitTurnLoop = 1;
    write_bytes(output, static_cast<DWORD>(used));
}

void request_end_turn(Console *self) {
    GameDataStateStorage.fields_.game_state_ |= STATE_UNK_2;
    ControlTurnPhase = 0;
    self->field_23BE4_ = 0;
    self->field_23BE8_ = 0;
    self->focus_vehicle_ = self->selected_vehicle_;
}

void execute_commands(Console *self) {
    if (State.vehicle_id < 0 || State.vehicle_id >= GameDataStateStorage.fields_.vehicle_current_count_) {
        finish_failure("vehicle_out_of_range");
        return;
    }
    int current_faction = ConsoleGlobal.faction_id_;
    if (Vehs[State.vehicle_id].faction_id != current_faction
        || veh_moves(State.vehicle_id) == 0) {
        finish_failure("vehicle_not_movable_by_current_faction");
        return;
    }
    bool adjacent = false;
    int movement_direction = -1;
    for (int index = 1; index < RadiusRange[1]; index++) {
        if (xrange(Vehs[State.vehicle_id].x + RadiusOffsetX[index]) == State.target_x
            && Vehs[State.vehicle_id].y + RadiusOffsetY[index] == State.target_y) {
            adjacent = true;
            movement_direction = index - 1;
            break;
        }
    }
    bool target_on_map = on_map(State.target_x, State.target_y);
    bool target_base = target_on_map
        && (bit_at(State.target_x, State.target_y) & BIT_BASE_IN_TILE);
    uint8_t triad = get_proto_triad(Vehs[State.vehicle_id].proto_id);
    bool incompatible_terrain = target_on_map && !target_base
        && ((triad == TRIAD_LAND && is_ocean(State.target_x, State.target_y))
            || (triad == TRIAD_SEA && !is_ocean(State.target_x, State.target_y)));
    if (!adjacent || !target_on_map
        || veh_at(State.target_x, State.target_y) != -1 || incompatible_terrain) {
        finish_failure("illegal_movement_target");
        return;
    }
    State.initial_turn = GameDataStateStorage.fields_.turn_current_num_;
    State.initial_x = Vehs[State.vehicle_id].x;
    State.initial_y = Vehs[State.vehicle_id].y;
    int initial_moves_expended = Vehs[State.vehicle_id].moves_expended;
    int expected_cost = hex_cost(
        Vehs[State.vehicle_id].proto_id, Vehs[State.vehicle_id].faction_id,
        State.initial_x, State.initial_y, State.target_x, State.target_y, false);
    bool singleton_stack = Vehs[State.vehicle_id].next_veh_id_stack == -1
        && Vehs[State.vehicle_id].prev_veh_id_stack == -1;
    bool source_occupied = (bit_at(State.initial_x, State.initial_y) & BIT_VEH_IN_TILE) != 0;
    if (State.resolve_movement && (!singleton_stack || !source_occupied)) {
        finish_failure("invalid_movement_fixture_stack");
        return;
    }
    if (State.advance_turn) {
        for (int vehicle = 0; vehicle < GameDataStateStorage.fields_.vehicle_current_count_; vehicle++) {
            if (get_triad(vehicle) == TRIAD_AIR) {
                finish_failure("turn_fixture_contains_air_vehicle");
                return;
            }
        }
    }

    write_progress("before_move_order");
    go_to(State.vehicle_id, 0, State.target_x, State.target_y);
    write_progress("after_move_order");
    bool movement_ordered = Vehs[State.vehicle_id].order == ORDER_MOVE_TO
        && Vehs[State.vehicle_id].waypoint_x[0] == State.target_x
        && Vehs[State.vehicle_id].waypoint_y[0] == State.target_y;
    if (State.resolve_movement && movement_ordered) {
        write_progress("before_move_resolution");
        action_go_to(State.vehicle_id);
        write_progress("after_move_resolution");
    }
    int movement_cost = static_cast<int>(Vehs[State.vehicle_id].moves_expended)
        - initial_moves_expended;
    State.movement_cost = movement_cost;
    bool order_cleared = Vehs[State.vehicle_id].order == ORDER_NONE;
    bool movement_resolved = State.resolve_movement
        && Vehs[State.vehicle_id].x == State.target_x
        && Vehs[State.vehicle_id].y == State.target_y
        && movement_cost == expected_cost
        && Vehs[State.vehicle_id].facing == movement_direction
        && order_cleared
        && veh_moves(State.vehicle_id) == 0
        && veh_at(State.target_x, State.target_y) == State.vehicle_id
        && (bit_at(State.target_x, State.target_y) & BIT_VEH_IN_TILE) != 0
        && (bit_at(State.initial_x, State.initial_y) & BIT_VEH_IN_TILE) == 0
        && Vehs[State.vehicle_id].next_veh_id_stack == -1
        && Vehs[State.vehicle_id].prev_veh_id_stack == -1;
    request_end_turn(self);
    bool end_turn_requested = (GameDataStateStorage.fields_.game_state_ & STATE_UNK_2) != 0
        && ControlTurnPhase == 0
        && self->field_23BE4_ == 0
        && self->field_23BE8_ == 0;
    bool passed = movement_ordered && end_turn_requested
        && (!State.resolve_movement || movement_resolved);
    if (passed && State.advance_turn) {
        State.phase = ScenarioPhase::AwaitingAdvance;
        write_progress("awaiting_turn_advance");
        return;
    }
    const char *status = passed ? "passed" : "failed";

    char output[1024];
    int size = snprintf(output, sizeof(output),
        "{\n"
        "  \"status\": \"%s\",\n"
        "  \"turn\": %d,\n"
        "  \"vehicle\": %d,\n"
        "  \"start\": [%d, %d],\n"
        "  \"target\": [%d, %d],\n"
        "  \"movement_ordered\": %s,\n"
        "  \"movement_resolved\": %s,\n"
        "  \"order_cleared\": %s,\n"
        "  \"movement_cost\": %d,\n"
        "  \"end_turn_requested\": %s\n"
        "}\n",
        status, State.initial_turn, State.vehicle_id,
        State.initial_x, State.initial_y, State.target_x, State.target_y,
        movement_ordered ? "true" : "false",
        movement_resolved ? "true" : "false",
        order_cleared ? "true" : "false", movement_cost,
        end_turn_requested ? "true" : "false");
    State.phase = ScenarioPhase::Finished;
    ExitTurnLoop = 1;
    if (size > 0 && static_cast<size_t>(size) < sizeof(output)) {
        write_bytes(output, static_cast<DWORD>(size));
    }
}

} // namespace

void __cdecl scenario_opening_movie(char *movie_name) {
    // The deferred-oracle phase used to be triggered here - this is the first
    // startup call site reached after the executable's CRT is running, which is
    // what it needed for real allocations. The runtime oracle route is retired
    // (docs/RETIRED_ROUTES.md); the hook point is recorded in case it returns.
    if (GetEnvironmentVariableA("OPENSMACX_SCENARIO_SAVE", nullptr, 0) == 0
        || GetEnvironmentVariableA("OPENSMACX_SCENARIO_RESULT", nullptr, 0) == 0) {
        amovie_project(movie_name);
    }
}

extern "C" void __cdecl scenario_human_turn_ready(Console *self) {
    if (State.phase == ScenarioPhase::Inactive
        || State.phase == ScenarioPhase::AwaitingAdvance) {
        return;
    }
    if (State.phase == ScenarioPhase::Finished) {
        ExitTurnLoop = 1;
        return;
    }
    if (!validate_loaded_state()) {
        finish_failure("invalid_loaded_state");
        return;
    }
    if (State.phase == ScenarioPhase::Loaded) {
        if (State.inspect_only) {
            inspect_loaded_state();
        }
        else {
            execute_commands(self);
        }
        return;
    }
}

extern "C" void __cdecl scenario_turn_advanced(uintptr_t caller_return) {
    if (State.phase != ScenarioPhase::AwaitingAdvance) {
        return;
    }
    int advanced_turn = GameDataStateStorage.fields_.turn_current_num_;
    uint32_t expected_year = game_year(advanced_turn);
    bool turn_advanced = caller_return == TurnUpkeepCallerReturn
        && advanced_turn == State.initial_turn + 1
        && GameDataStateStorage.fields_.mission_year_current_ == expected_year
        && Vehs[State.vehicle_id].x == State.target_x
        && Vehs[State.vehicle_id].y == State.target_y;
    if (!turn_advanced) {
        char failure[768];
        int failure_size = snprintf(failure, sizeof(failure),
            "{\n"
            "  \"status\": \"failed\",\n"
            "  \"error\": \"turn_advance_validation_failed\",\n"
            "  \"caller_return\": %u,\n"
            "  \"expected_caller_return\": %u,\n"
            "  \"initial_turn\": %d,\n"
            "  \"advanced_turn\": %d,\n"
            "  \"mission_year\": %u,\n"
            "  \"expected_year\": %u,\n"
            "  \"vehicle_position\": [%d, %d],\n"
            "  \"target\": [%d, %d]\n"
            "}\n",
            static_cast<unsigned int>(caller_return),
            static_cast<unsigned int>(TurnUpkeepCallerReturn),
            State.initial_turn, advanced_turn, GameDataStateStorage.fields_.mission_year_current_, expected_year,
            Vehs[State.vehicle_id].x, Vehs[State.vehicle_id].y,
            State.target_x, State.target_y);
        State.phase = ScenarioPhase::Finished;
        ExitTurnLoop = 1;
        if (failure_size > 0 && static_cast<size_t>(failure_size) < sizeof(failure)) {
            write_bytes(failure, static_cast<DWORD>(failure_size));
        }
        return;
    }

    char output[1024];
    int size = snprintf(output, sizeof(output),
        "{\n"
        "  \"status\": \"passed\",\n"
        "  \"turn\": %d,\n"
        "  \"initial_turn\": %d,\n"
        "  \"advanced_turn\": %d,\n"
        "  \"mission_year\": %u,\n"
        "  \"vehicle\": %d,\n"
        "  \"start\": [%d, %d],\n"
        "  \"target\": [%d, %d],\n"
        "  \"movement_ordered\": true,\n"
        "  \"movement_resolved\": true,\n"
        "  \"order_cleared\": true,\n"
        "  \"movement_cost\": %d,\n"
        "  \"end_turn_requested\": true,\n"
        "  \"turn_advanced\": true\n"
        "}\n",
        advanced_turn, State.initial_turn, advanced_turn, GameDataStateStorage.fields_.mission_year_current_,
        State.vehicle_id, State.initial_x, State.initial_y,
        State.target_x, State.target_y, State.movement_cost);
    State.phase = ScenarioPhase::Finished;
    ExitTurnLoop = 1;
    if (size > 0 && static_cast<size_t>(size) < sizeof(output)) {
        write_bytes(output, static_cast<DWORD>(size));
    }
}

int __cdecl scenario_top_menu(int mode) {
    if (!configure_scenario()) {
        return top_menu(mode);
    }
    if (State.phase == ScenarioPhase::Finished) {
        return 1;
    }
    write_progress("before_load");
    uint32_t result = load_daemon(State.save_path, false);
    if (result != 0) {
        State.phase = ScenarioPhase::Finished;
        write_failure("load_daemon_failed");
        return 1;
    }
    write_progress("after_load");
    refresh_loaded_game();
    write_progress("after_refresh");
    scenario_human_turn_ready(&ConsoleGlobal);
    return 0;
}
