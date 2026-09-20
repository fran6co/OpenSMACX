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
#include "temp.h"
#include "veh.h"

#include <cerrno>
#include <climits>
#include <cstring>

extern "C" void __cdecl scenario_human_turn_ready(Console *self);

namespace {

static const uintptr_t TopMenuAddress = 0x0058E360;
static const uintptr_t OpeningMovieAddress = 0x00403BE0;
static const uintptr_t LoadFlagsAddress = 0x00453F70;
// 0x007B0CB8 is an interface object, but it is NOT the MainInterface: the
// desktop MainInterface is MainInterfaceGlobal at 0x007AE820 (mapwin.h), and
// 0x7B0CB8 sits inside no part of it. Every image site that reaches it does
// the same three things - `mov eax, [0x7b0cb8]` for the vftable,
// `mov ecx, 0x7b0cb8` for the receiver, then a call through slot 1 or slot 2 -
// and the same sites first read ActiveInterfaceId (0x007D392C) and the
// id-to-slot map (0x007AE778) and skip the call when a slot is already taken,
// which is the interface-registry guard maininterface.cpp's own handlers use.
// So this is a second interface object under the same registry, one whose
// slot 1 takes a single int and whose slot 2 is nullary, and the class that
// owns it is still unidentified: no `mov dword ptr [0x7b0cb8], imm` store
// exists in .text, so its constructor installs the vftable through a
// register.
//
// THE ADDRESS IS UNREACHABLE TO ANY CONSTRUCTOR, measured 2026-09-05 by
// scanning every section of the shipped image for the four bytes B8 0C 7B 00.
// .text holds 32, and every one is one of the NINE dispatch sites (0x4D9A46,
// 0x4DF7DC, 0x4E109C, 0x4E115F, 0x5186CC, 0x51C296, 0x51CEAA, 0x5A9A81,
// 0x5AADFE) in the forms `b9 b8 0c 7b 00` (mov ecx, imm),
// `a1 b8 0c 7b 00` (mov eax, [imm]) and `8b 15 b8 0c 7b 00` (mov edx, [imm])
// - all READS. There is no `89 05`/`89 0d`/`89 15`/`a3` store to the dword and
// no `b8 b8 0c 7b 00` that would put the address in a register to store
// through, and _SELFMOD/.rdata/.data hold it not at all. 0x7B0CB8 is past
// .data's raw data (which ends at 0x6A8000), so it is zero-initialised BSS:
// the nine sites read a null vftable pointer. 0x51BC1B does `or dword ptr
// [0x9A64C0], 0xE0`, which is the guard - `test byte [0x9A64C0], 0x80` and
// `, 0x40` - opening, so the shipped game either never reaches one of these
// nine sites or faults there. There is therefore no class to model: do not
// name one from the two slots. Named for what the harness does with it rather
// than for a class the image has not given us.
static const uintptr_t InterfaceSingletonAddress = 0x007B0CB8;
static const uintptr_t ActionGoToAddress = 0x004CB310;
static const uintptr_t TurnUpkeepCallerReturn = 0x0052768F;
// The turn-loop exit flag and the turn phase are named globals now - see
// game.h (ExitTurnLoop, ControlTurnPhase); the raw uintptr_t bindings that
// stood here were retired when control_game was homed into game.cpp on
// 2026-08-29 and both addresses gained real definitions.

typedef int (__cdecl *TopMenuFunction)(int);
typedef void (__cdecl *OpeningMovieFunction)(char *);
typedef void (__cdecl *LoadFlagsFunction)();
typedef void (__fastcall *InterfaceRefreshFunction)(void *, void *);
typedef void (__cdecl *ActionGoToFunction)(int);

TopMenuFunction OriginalTopMenu = reinterpret_cast<TopMenuFunction>(TopMenuAddress);
OpeningMovieFunction OriginalOpeningMovie =
    reinterpret_cast<OpeningMovieFunction>(OpeningMovieAddress);
LoadFlagsFunction LoadFlags = reinterpret_cast<LoadFlagsFunction>(LoadFlagsAddress);
ActionGoToFunction ActionGoTo = reinterpret_cast<ActionGoToFunction>(ActionGoToAddress);

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
    LoadFlags();
    GameState &= 0xF79FCF16;

    void *object = reinterpret_cast<void *>(InterfaceSingletonAddress);
    uintptr_t *vtable = *reinterpret_cast<uintptr_t **>(object);
    // vftable slot 2 of the interface object - the image's
    // `mov eax,[0x7b0cb8] / mov ecx,0x7b0cb8 / call dword ptr [eax+8]`, the
    // same refresh the console's own guard dispatches when no other
    // interface holds the registry slot.
    InterfaceRefreshFunction refresh =
        reinterpret_cast<InterfaceRefreshFunction>(vtable[2]);
    refresh(object, nullptr);
    prefs_use();
}

bool validate_loaded_state() {
    return !IsMultiplayerNet && !IsMultiplayerPBEM
        && VehCurrentCount > 0 && VehCurrentCount <= 2049;
}

void inspect_loaded_state() {
    int current_faction = ConsoleGlobal.faction_id_;
    int candidate = -1;
    for (int vehicle = 0; vehicle < VehCurrentCount; vehicle++) {
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
        TurnCurrentNum, current_faction, VehCurrentCount, candidate,
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
    GameState |= STATE_UNK_2;
    ControlTurnPhase = 0;
    self->field_23BE4_ = 0;
    self->field_23BE8_ = 0;
    self->focus_vehicle_ = self->selected_vehicle_;
}

void execute_commands(Console *self) {
    if (State.vehicle_id < 0 || State.vehicle_id >= VehCurrentCount) {
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
    State.initial_turn = TurnCurrentNum;
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
        for (int vehicle = 0; vehicle < VehCurrentCount; vehicle++) {
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
        ActionGoTo(State.vehicle_id);
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
        && Vehs[State.vehicle_id].unk_5 == movement_direction
        && order_cleared
        && veh_moves(State.vehicle_id) == 0
        && veh_at(State.target_x, State.target_y) == State.vehicle_id
        && (bit_at(State.target_x, State.target_y) & BIT_VEH_IN_TILE) != 0
        && (bit_at(State.initial_x, State.initial_y) & BIT_VEH_IN_TILE) == 0
        && Vehs[State.vehicle_id].next_veh_id_stack == -1
        && Vehs[State.vehicle_id].prev_veh_id_stack == -1;
    request_end_turn(self);
    bool end_turn_requested = (GameState & STATE_UNK_2) != 0
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
        OriginalOpeningMovie(movie_name);
    }
}

extern "C" {
volatile int ScenarioTrampolineAction = 0;
volatile int ScenarioTurnAdvanceAction = 0;
}

extern "C" void __cdecl scenario_human_turn_ready(Console *self) {
    ScenarioTrampolineAction = 0;
    if (State.phase == ScenarioPhase::Inactive
        || State.phase == ScenarioPhase::AwaitingAdvance) {
        return;
    }
    ScenarioTrampolineAction = 2;
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
    ScenarioTurnAdvanceAction = 0;
    if (State.phase != ScenarioPhase::AwaitingAdvance) {
        return;
    }
    ScenarioTurnAdvanceAction = 1;
    int advanced_turn = TurnCurrentNum;
    uint32_t expected_year = game_year(advanced_turn);
    bool turn_advanced = caller_return == TurnUpkeepCallerReturn
        && advanced_turn == State.initial_turn + 1
        && MissionYearCurrent == expected_year
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
            State.initial_turn, advanced_turn, MissionYearCurrent, expected_year,
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
        advanced_turn, State.initial_turn, advanced_turn, MissionYearCurrent,
        State.vehicle_id, State.initial_x, State.initial_y,
        State.target_x, State.target_y, State.movement_cost);
    State.phase = ScenarioPhase::Finished;
    ExitTurnLoop = 1;
    if (size > 0 && static_cast<size_t>(size) < sizeof(output)) {
        write_bytes(output, static_cast<DWORD>(size));
    }
}

__declspec(naked) void scenario_human_turn_trampoline() {
    __asm {
        mov dword ptr [esi + 023D24h], edi
        pushfd
        pushad
        push esi
        call scenario_human_turn_ready
        add esp, 4
        cmp ScenarioTrampolineAction, 0
        jne exit_human_turn
        popad
        popfd
        push 0051418Fh
        ret
    exit_human_turn:
        popad
        popfd
        push 005147B9h
        ret
    }
}

__declspec(naked) void scenario_turn_advance_trampoline() {
    __asm {
        pushfd
        pushad
        push dword ptr [ebp + 4]
        call scenario_turn_advanced
        add esp, 4
        cmp ScenarioTurnAdvanceAction, 0
        jne exit_upkeep
        popad
        popfd
        push 00525AF9h
        push 0046FB10h
        ret
    exit_upkeep:
        popad
        popfd
        mov dword ptr [ebp + 4], 005282CEh
        push 00526026h
        ret
    }
}

int __cdecl scenario_top_menu(int mode) {
    if (!configure_scenario()) {
        return OriginalTopMenu(mode);
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
