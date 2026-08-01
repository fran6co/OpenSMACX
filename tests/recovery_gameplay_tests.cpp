/*
 * OpenSMACX - an open source clone of Sid Meier's Alpha Centauri.
 * Copyright (C) 2013-2021 Brendan Casey
 *
 * OpenSMACX is free software: you can redistribute it and / or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * OpenSMACX is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with OpenSMACX. If not, see <http://www.gnu.org/licenses/>.
 */

/*
 * Direct-source tests for the gameplay core.
 *
 * This binary exists because `recovery-leaf-tests` cannot host these. That
 * suite defines file-scope stand-ins for map.cpp and general.cpp symbols -
 * a scriptable `mem_get`, call-recording `lock_map`/`unlock_map`, `xrange`,
 * `purge_spaces` - which its own fixtures assert against. Linking the real
 * translation units there would collide with every one of those observers.
 *
 * So the split is by link closure, not by subject: this target links the real
 * gameplay tree (map, veh, base, faction, alpha, general, technology, path,
 * probe, council, scenario, terraforming, game) and the support classes it
 * pulls in, and therefore has no stand-ins to collide with. That closure is
 * self-satisfying: the only symbols it does not resolve internally are Win32
 * imports, which is why gdi32 and winmm are linked, and the deferred oracle
 * entry point, which is stubbed below.
 *
 * Free functions in these files were previously deferred as "no home in a
 * linked file". That is what this target removes.
 */

#include "../src/stdafx.h"
#include "../src/game.h"
#include "../src/veh.h"
#include "../src/map.h"
#include "../src/faction.h"
#include "../src/alpha.h"
#include "../src/base.h"
#include "../src/general.h"
#include "../src/technology.h"
#include "../src/temp.h"
#include "recovery_fixtures.h"

using recovery_fixtures::expect;
using recovery_fixtures::failure_count;

/*
 * The real deferred oracle phase runs inside the hybrid process, after the
 * executable's CRT is up, and drives proprietary fixtures. Linking it here
 * would drag in every oracle suite; the scenario call site is exercised by the
 * hybrid smoke gate instead.
 */
void run_deferred_oracles() { }

namespace {

// Rebind a global pointer seam for the duration of a check and put it back,
// so one test cannot leak state into the next.
template <typename T>
class ScopedSeam {
 public:
    ScopedSeam(T **seam, T *replacement) : seam_(seam), saved_(*seam) {
        *seam_ = replacement;
    }
    ~ScopedSeam() { *seam_ = saved_; }

 private:
    T **seam_;
    T *saved_;
};

void test_not_my_turn() {
    BOOL is_net = 0;
    uint8_t flags = 0;
    int net_faction = 0;
    int local_faction = 0;

    ScopedSeam<BOOL> net_seam(&IsMultiplayerNet, &is_net);
    ScopedSeam<uint8_t> flag_seam(&NetTurnFlags, &flags);
    ScopedSeam<int> turn_seam(&NetTurnFaction, &net_faction);
    ScopedSeam<int> local_seam(&LocalFaction, &local_faction);

    // Not a net game: neither the flag nor the faction mismatch matters, and
    // the answer is "my turn" regardless.
    is_net = 0;
    flags = 0xFF;
    net_faction = 3;
    local_faction = 5;
    expect(not_my_turn() == false);

    // A net game that is not currently passing the turn: still my turn, even
    // with the factions disagreeing. Only bit 4 arms the comparison.
    is_net = 1;
    flags = 0xEF;  // every bit but 4
    expect(not_my_turn() == false);

    // Armed, and the active faction is someone else.
    flags = 0x10;
    net_faction = 3;
    local_faction = 5;
    expect(not_my_turn() == true);

    // Armed, and the active faction is the local one.
    net_faction = 5;
    expect(not_my_turn() == false);

    // The comparison is an inequality on the faction identity, so it fires for
    // a lower id just as it does for a higher one, and zero is an ordinary id
    // rather than a sentinel.
    net_faction = 0;
    local_faction = 1;
    expect(not_my_turn() == true);
    net_faction = 0;
    local_faction = 0;
    expect(not_my_turn() == false);
    net_faction = 7;
    local_faction = 0;
    expect(not_my_turn() == true);

    // A truthy net flag other than 1 still counts: the original tests the
    // dword for nonzero, it does not compare it against TRUE.
    is_net = 0x1234;
    net_faction = 3;
    local_faction = 5;
    expect(not_my_turn() == true);

    // Every other flag bit set alongside bit 4 leaves it armed.
    flags = 0xFF;
    expect(not_my_turn() == true);
}


/*
 * A self-contained world for stack_veh.
 *
 * Every global stack_veh reaches is a pointer, so the whole fixture is
 * rebindable through ScopedSeam and the fixed 0x00952828-family addresses are
 * never touched. PlayerData is ~8 KB per faction, so the world is static
 * rather than a local.
 *
 * Prototype ids stay below MaxVehProtoFactionNum (64) on purpose: has_abil()
 * then computes faction_id 0 and returns before it reads Players or Ability, so
 * those two do not need seams. Weapon[0].offense_rating is non-negative so
 * veh_cargo() returns carry_capacity verbatim rather than the morale path.
 */
struct StackVehWorld {
    Veh vehs[8];
    VehPrototype protos[8];
    RulesChassis chassis[4];
    RulesWeapon weapons[4];
    Map tiles[64];
    PlayerData players_data[MaxPlayerNum];
    Map *tiles_ptr;
    uint8_t factions_status;
    uint32_t map_longitude;
    int map_longitude_bounds;
    int map_latitude_bounds;
    BOOL is_net;
    // Only the mode != 0 paths reach these: they are read and written by
    // veh_lift/veh_drop, which mode 0 never calls. veh_current_count bounds
    // veh_at's scan, so it must cover every unit in `vehs` or a unit dropped
    // off-map would be invisible to the next drop and the stacks at (-2,-2)
    // would come out wrong.
    int veh_current_count;
    int veh_drop_lift_veh_id;
    int veh_lift_x;
    int veh_lift_y;
};

StackVehWorld g_stack_world;

// Chassis ids used by the fixtures.
const uint8_t CHASSIS_LAND = 0;
const uint8_t CHASSIS_SEA = 1;
const uint8_t CHASSIS_AIR = 2;

// Prototype ids used by the fixtures.
const int PROTO_SEA_TRANSPORT = 1;  // carry_capacity 4, no ABL_CARRIER
const int PROTO_LAND_GRUNT = 2;     // carry_capacity 0, TRIAD_LAND
const int PROTO_AIR_GRUNT = 3;      // carry_capacity 0, TRIAD_AIR, no ABL_CARRIER
const int PROTO_AIR_CARRIER = 4;    // carry_capacity 0, TRIAD_AIR, ABL_CARRIER
const int PROTO_SEA_SKIFF = 5;      // carry_capacity 1, so the cargo counter reaches 1
const int PROTO_LAND_COLONY = 6;    // carry_capacity 0, TRIAD_LAND, plan == PLAN_COLONIZATION

void reset_stack_world() {
    std::memset(&g_stack_world, 0, sizeof(g_stack_world));
    g_stack_world.tiles_ptr = g_stack_world.tiles;
    // 8x8 map, so a tile index is y * 4 + (x >> 1) and 64 tiles cover it.
    g_stack_world.map_longitude = 4;
    g_stack_world.map_longitude_bounds = 8;
    g_stack_world.map_latitude_bounds = 8;
    g_stack_world.is_net = 0;  // keeps stack_fix() out of the picture
    // 8 covers every slot in `vehs`; see the note on the field.
    g_stack_world.veh_current_count = 8;

    g_stack_world.chassis[CHASSIS_LAND].triad = TRIAD_LAND;
    g_stack_world.chassis[CHASSIS_SEA].triad = TRIAD_SEA;
    g_stack_world.chassis[CHASSIS_AIR].triad = TRIAD_AIR;
    g_stack_world.weapons[0].offense_rating = 1;

    g_stack_world.protos[PROTO_SEA_TRANSPORT].chassis_id = CHASSIS_SEA;
    g_stack_world.protos[PROTO_SEA_TRANSPORT].carry_capacity = 4;
    g_stack_world.protos[PROTO_SEA_TRANSPORT].plan = PLAN_NAVAL_TRANSPORT;
    g_stack_world.protos[PROTO_LAND_GRUNT].chassis_id = CHASSIS_LAND;
    g_stack_world.protos[PROTO_LAND_GRUNT].plan = PLAN_COMBAT;
    g_stack_world.protos[PROTO_AIR_GRUNT].chassis_id = CHASSIS_AIR;
    g_stack_world.protos[PROTO_AIR_GRUNT].plan = PLAN_COMBAT;
    g_stack_world.protos[PROTO_AIR_CARRIER].chassis_id = CHASSIS_AIR;
    g_stack_world.protos[PROTO_AIR_CARRIER].ability_flags = ABL_CARRIER;
    g_stack_world.protos[PROTO_SEA_SKIFF].chassis_id = CHASSIS_SEA;
    g_stack_world.protos[PROTO_SEA_SKIFF].carry_capacity = 1;
    g_stack_world.protos[PROTO_SEA_SKIFF].plan = PLAN_NAVAL_TRANSPORT;
    g_stack_world.protos[PROTO_LAND_COLONY].chassis_id = CHASSIS_LAND;
    g_stack_world.protos[PROTO_LAND_COLONY].plan = PLAN_COLONIZATION;

    for (int k = 0; k < 8; k++) {
        Veh &veh = g_stack_world.vehs[k];
        veh.x = -1;
        veh.y = -1;
        veh.next_veh_id_stack = -1;
        veh.prev_veh_id_stack = -1;
        veh.order = ORDER_NONE;
        veh.waypoint_x[0] = -1;
    }
}

// Link ids[0..count) into one stack, top first.
void link_stack(const int *ids, int count) {
    for (int k = 0; k < count; k++) {
        g_stack_world.vehs[ids[k]].prev_veh_id_stack =
            (int16_t)(k ? ids[k - 1] : -1);
        g_stack_world.vehs[ids[k]].next_veh_id_stack =
            (int16_t)(k + 1 < count ? ids[k + 1] : -1);
    }
}

void test_stack_veh_boarding() {
    ScopedSeam<Veh> vehs_seam(&Vehs, g_stack_world.vehs);
    ScopedSeam<VehPrototype> protos_seam(&VehPrototypes, g_stack_world.protos);
    ScopedSeam<RulesChassis> chassis_seam(&Chassis, g_stack_world.chassis);
    ScopedSeam<RulesWeapon> weapon_seam(&Weapon, g_stack_world.weapons);
    ScopedSeam<Map *> tiles_seam(&MapTiles, &g_stack_world.tiles_ptr);
    ScopedSeam<PlayerData> players_seam(&PlayersData, g_stack_world.players_data);
    ScopedSeam<uint8_t> status_seam(&FactionsStatus, &g_stack_world.factions_status);
    ScopedSeam<uint32_t> longitude_seam(&MapLongitude, &g_stack_world.map_longitude);
    ScopedSeam<int> lon_bounds_seam(&MapLongitudeBounds, &g_stack_world.map_longitude_bounds);
    ScopedSeam<int> lat_bounds_seam(&MapLatitudeBounds, &g_stack_world.map_latitude_bounds);
    ScopedSeam<BOOL> net_seam(&IsMultiplayerNet, &g_stack_world.is_net);
    // Reached only through veh_lift/veh_drop, i.e. only on the mode != 0 paths.
    ScopedSeam<int> count_seam(&VehCurrentCount, &g_stack_world.veh_current_count);
    ScopedSeam<int> drop_id_seam(&VehDropLiftVehID, &g_stack_world.veh_drop_lift_veh_id);
    ScopedSeam<int> lift_x_seam(&VehLiftX, &g_stack_world.veh_lift_x);
    ScopedSeam<int> lift_y_seam(&VehLiftY, &g_stack_world.veh_lift_y);

    // ---------------------------------------------------------------
    // A negative unit id returns before any global is read: 0 for mode 0,
    // the id itself for every non-zero mode. `neg eax; sbb eax, eax; and
    // eax, esi` is a mode-is-nonzero test, not a comparison against 1.
    // ---------------------------------------------------------------
    reset_stack_world();
    expect(stack_veh(-1, 0) == 0);
    expect(stack_veh(-1, 1) == -1);
    expect(stack_veh(-7, 5) == -7);
    expect(stack_veh(-7, -3) == -7);

    // ---------------------------------------------------------------
    // A unit that is neither a transport (veh_cargo) nor a carrier
    // (ABL_CARRIER) returns 0 in mode 0 without touching the stack: the
    // early exit precedes the pre-pass that clears VSTATE_UNK_1.
    // ---------------------------------------------------------------
    reset_stack_world();
    g_stack_world.vehs[0].proto_id = PROTO_LAND_GRUNT;
    g_stack_world.vehs[0].faction_id = 5;
    g_stack_world.vehs[0].state = VSTATE_UNK_1;
    g_stack_world.vehs[1].proto_id = PROTO_LAND_GRUNT;
    g_stack_world.vehs[1].faction_id = 5;
    g_stack_world.vehs[1].state = VSTATE_UNK_1;
    {
        const int ids[] = {0, 1};
        link_stack(ids, 2);
    }
    expect(stack_veh(0, 0) == 0);
    expect(g_stack_world.vehs[0].state == VSTATE_UNK_1);
    expect(g_stack_world.vehs[1].state == VSTATE_UNK_1);
    expect(g_stack_world.vehs[1].waypoint_x[0] == -1);
    expect(g_stack_world.vehs[1].order == ORDER_NONE);

    // ---------------------------------------------------------------
    // The ordinary case. A sea transport with capacity 4 carrying two land
    // units of its own faction, everything off the map, and the faction's
    // bit CLEAR in FactionsStatus so the second rule set applies: an
    // off-map unit fails on_map(), so the PLAN_DEFENSIVE veto cannot fire
    // and the unit boards.
    //
    // The subject is the head of its own stack and is examined first: it is
    // TRIAD_SEA (not LAND) and carrier_left is 0, so it never boards itself.
    // ---------------------------------------------------------------
    reset_stack_world();
    g_stack_world.factions_status = 0;
    g_stack_world.vehs[0].proto_id = PROTO_SEA_TRANSPORT;
    g_stack_world.vehs[0].faction_id = 5;
    g_stack_world.vehs[1].proto_id = PROTO_LAND_GRUNT;
    g_stack_world.vehs[1].faction_id = 5;
    g_stack_world.vehs[2].proto_id = PROTO_LAND_GRUNT;
    g_stack_world.vehs[2].faction_id = 5;
    {
        const int ids[] = {0, 1, 2};
        link_stack(ids, 3);
    }
    // veh_cargo(subject) is 4 and non-zero, so the return is the cargo
    // counter: 4 minus the two units that boarded.
    expect(stack_veh(0, 0) == 2);
    // sleep() rewrote the order and both waypoint words; the transport id
    // then overwrote sleep()'s -1 in waypoint_x[0].
    expect(g_stack_world.vehs[1].order == ORDER_SENTRY_BOARD);
    expect(g_stack_world.vehs[1].waypoint_x[0] == 0);
    expect(g_stack_world.vehs[1].waypoint_y[0] == 0);
    expect(g_stack_world.vehs[1].state == VSTATE_UNK_1);
    expect(g_stack_world.vehs[2].order == ORDER_SENTRY_BOARD);
    expect(g_stack_world.vehs[2].waypoint_x[0] == 0);
    expect(g_stack_world.vehs[2].state == VSTATE_UNK_1);
    // The subject is not boarded onto itself, and mode 0 moves nothing.
    expect(g_stack_world.vehs[0].order == ORDER_NONE);
    expect(g_stack_world.vehs[0].waypoint_x[0] == -1);
    expect(g_stack_world.vehs[0].x == -1);
    expect(g_stack_world.vehs[0].next_veh_id_stack == 1);
    expect(g_stack_world.vehs[1].next_veh_id_stack == 2);

    // ---------------------------------------------------------------
    // Called again on the state the first call left behind, each unit must
    // still be counted exactly ONCE. Pass 0 now matches (order is
    // SENTRY_BOARD and waypoint_x[0] is this transport) and boards them;
    // pass 1 would accept them too, and only the VSTATE_UNK_1 flag written
    // by pass 0 stops the second charge. Without that guard the answer is
    // 0 rather than 2.
    //
    // It also proves the pre-pass: VSTATE_UNK_1 survived from the previous
    // call and had to be cleared before pass 0 could board anything.
    // ---------------------------------------------------------------
    expect(stack_veh(0, 0) == 2);
    expect(g_stack_world.vehs[1].state == VSTATE_UNK_1);
    expect(g_stack_world.vehs[2].state == VSTATE_UNK_1);

    // ---------------------------------------------------------------
    // Foreign units in the same stack are skipped on faction identity
    // alone, before any eligibility or rule-set work.
    // ---------------------------------------------------------------
    reset_stack_world();
    g_stack_world.factions_status = 0;
    g_stack_world.vehs[0].proto_id = PROTO_SEA_TRANSPORT;
    g_stack_world.vehs[0].faction_id = 5;
    g_stack_world.vehs[1].proto_id = PROTO_LAND_GRUNT;
    g_stack_world.vehs[1].faction_id = 6;  // different faction
    g_stack_world.vehs[2].proto_id = PROTO_LAND_GRUNT;
    g_stack_world.vehs[2].faction_id = 5;
    {
        const int ids[] = {0, 1, 2};
        link_stack(ids, 3);
    }
    expect(stack_veh(0, 0) == 3);  // only unit 2 boarded
    expect(g_stack_world.vehs[1].order == ORDER_NONE);
    expect(g_stack_world.vehs[1].waypoint_x[0] == -1);
    expect(g_stack_world.vehs[2].waypoint_x[0] == 0);

    // ---------------------------------------------------------------
    // Faction 0 replaces the cargo counter with the literal 99, so the
    // transport's own capacity stops limiting it.
    // ---------------------------------------------------------------
    reset_stack_world();
    g_stack_world.factions_status = 0;
    g_stack_world.vehs[0].proto_id = PROTO_SEA_TRANSPORT;
    g_stack_world.vehs[0].faction_id = 0;
    g_stack_world.vehs[1].proto_id = PROTO_LAND_GRUNT;
    g_stack_world.vehs[1].faction_id = 0;
    g_stack_world.vehs[2].proto_id = PROTO_LAND_GRUNT;
    g_stack_world.vehs[2].faction_id = 0;
    {
        const int ids[] = {0, 1, 2};
        link_stack(ids, 3);
    }
    expect(stack_veh(0, 0) == 97);

    // ---------------------------------------------------------------
    // The FactionsStatus rule set. With the unit faction's bit SET, a
    // sentry/board unit boards only while waypoint_x[0] is still negative -
    // i.e. while it is not already recorded as riding some other transport.
    // ---------------------------------------------------------------
    reset_stack_world();
    g_stack_world.factions_status = (uint8_t)(1 << 5);
    g_stack_world.vehs[0].proto_id = PROTO_SEA_TRANSPORT;
    g_stack_world.vehs[0].faction_id = 5;
    g_stack_world.vehs[1].proto_id = PROTO_LAND_GRUNT;
    g_stack_world.vehs[1].faction_id = 5;
    g_stack_world.vehs[1].order = ORDER_SENTRY_BOARD;
    g_stack_world.vehs[1].waypoint_x[0] = -1;
    g_stack_world.vehs[2].proto_id = PROTO_LAND_GRUNT;
    g_stack_world.vehs[2].faction_id = 5;
    g_stack_world.vehs[2].order = ORDER_SENTRY_BOARD;
    g_stack_world.vehs[2].waypoint_x[0] = 3;  // already riding unit 3
    {
        const int ids[] = {0, 1, 2};
        link_stack(ids, 3);
    }
    expect(stack_veh(0, 0) == 3);  // only unit 1 boarded
    expect(g_stack_world.vehs[1].waypoint_x[0] == 0);
    expect(g_stack_world.vehs[2].waypoint_x[0] == 3);
    expect(g_stack_world.vehs[2].state == 0);

    // ---------------------------------------------------------------
    // With the bit set, a unit that is NOT sentry/board falls through to
    // the open-ocean rule, which is about the SUBJECT's tile, not the
    // unit's. Off the map, is_open_ocean is false and nothing boards.
    // ---------------------------------------------------------------
    reset_stack_world();
    g_stack_world.factions_status = (uint8_t)(1 << 5);
    g_stack_world.vehs[0].proto_id = PROTO_SEA_TRANSPORT;
    g_stack_world.vehs[0].faction_id = 5;
    g_stack_world.vehs[1].proto_id = PROTO_LAND_GRUNT;
    g_stack_world.vehs[1].faction_id = 5;
    g_stack_world.vehs[1].order = ORDER_HOLD;
    {
        const int ids[] = {0, 1};
        link_stack(ids, 2);
    }
    expect(stack_veh(0, 0) == 4);  // nothing boarded
    expect(g_stack_world.vehs[1].order == ORDER_HOLD);

    // Put the transport on an ocean tile with no base and the same unit
    // boards. tile index = y * MapLongitude + (x >> 1) = 1 * 4 + 1 = 5;
    // climate 0 means altitude 0, which is below ALT_BIT_SHORE_LINE.
    g_stack_world.vehs[0].x = 2;
    g_stack_world.vehs[0].y = 1;
    g_stack_world.tiles[5].climate = 0;
    g_stack_world.tiles[5].bit = 0;
    expect(stack_veh(0, 0) == 3);
    expect(g_stack_world.vehs[1].order == ORDER_SENTRY_BOARD);
    expect(g_stack_world.vehs[1].waypoint_x[0] == 0);

    // Raise that tile above the shore line and the same call boards nothing:
    // is_open_ocean is the only thing that changed.
    reset_stack_world();
    g_stack_world.factions_status = (uint8_t)(1 << 5);
    g_stack_world.vehs[0].proto_id = PROTO_SEA_TRANSPORT;
    g_stack_world.vehs[0].faction_id = 5;
    g_stack_world.vehs[0].x = 2;
    g_stack_world.vehs[0].y = 1;
    g_stack_world.vehs[1].proto_id = PROTO_LAND_GRUNT;
    g_stack_world.vehs[1].faction_id = 5;
    g_stack_world.vehs[1].order = ORDER_HOLD;
    {
        const int ids[] = {0, 1};
        link_stack(ids, 2);
    }
    g_stack_world.tiles[5].climate = (uint8_t)ALT_BIT_SHORE_LINE;
    expect(stack_veh(0, 0) == 4);
    expect(g_stack_world.vehs[1].order == ORDER_HOLD);

    // An ocean tile carrying an owned base is not open ocean either, which
    // is the second half of the same predicate.
    g_stack_world.tiles[5].climate = 0;
    g_stack_world.tiles[5].bit = BIT_BASE_IN_TILE;
    g_stack_world.tiles[5].val2 = 4;  // owner 4, below MaxPlayerNum
    expect(stack_veh(0, 0) == 4);
    expect(g_stack_world.vehs[1].order == ORDER_HOLD);
    // Owner 0xF is "unoccupied", so the base does not veto and it boards.
    g_stack_world.tiles[5].val2 = 0xF;
    expect(stack_veh(0, 0) == 3);
    expect(g_stack_world.vehs[1].order == ORDER_SENTRY_BOARD);

    // ---------------------------------------------------------------
    // The carrier half. A pure carrier has veh_cargo() == 0, so the cargo
    // counter starts at 0 and the carrier counter at max(0, 1) == 1; the
    // return then comes from the carrier counter instead. Its eligibility
    // rule is the mirror image: TRIAD_AIR units that are not carriers
    // themselves.
    //
    // A TRIAD_AIR subject also arms the pass-1 tile gate, which reads the
    // subject's tile with no on_map() guard.
    // ---------------------------------------------------------------
    reset_stack_world();
    g_stack_world.factions_status = 0;
    g_stack_world.vehs[0].proto_id = PROTO_AIR_CARRIER;
    g_stack_world.vehs[0].faction_id = 5;
    g_stack_world.vehs[0].x = 2;
    g_stack_world.vehs[0].y = 1;
    g_stack_world.vehs[1].proto_id = PROTO_AIR_GRUNT;
    g_stack_world.vehs[1].faction_id = 5;
    g_stack_world.vehs[2].proto_id = PROTO_AIR_GRUNT;
    g_stack_world.vehs[2].faction_id = 5;
    g_stack_world.vehs[3].proto_id = PROTO_AIR_CARRIER;  // a carrier cannot board
    g_stack_world.vehs[3].faction_id = 5;
    {
        const int ids[] = {0, 1, 2, 3};
        link_stack(ids, 4);
    }
    // No base and no airbase under an air subject: the gate rejects every
    // candidate and the carrier counter comes back untouched.
    g_stack_world.tiles[5].climate = 0;
    g_stack_world.tiles[5].bit = 0;
    expect(stack_veh(0, 0) == 1);
    expect(g_stack_world.vehs[1].order == ORDER_NONE);

    // An airbase opens the gate. Capacity is 1, so exactly one air unit
    // boards and the loop then stops on both counters being zero - unit 2
    // is left alone even though it is equally eligible.
    g_stack_world.tiles[5].bit = BIT_AIRBASE;
    expect(stack_veh(0, 0) == 0);
    expect(g_stack_world.vehs[1].order == ORDER_SENTRY_BOARD);
    expect(g_stack_world.vehs[1].waypoint_x[0] == 0);
    expect(g_stack_world.vehs[2].order == ORDER_NONE);
    expect(g_stack_world.vehs[3].order == ORDER_NONE);

    // An owned base opens it as well, but only for owners 1..7: the air
    // gate is the one owner test in this function that rejects owner 0.
    reset_stack_world();
    g_stack_world.factions_status = 0;
    g_stack_world.vehs[0].proto_id = PROTO_AIR_CARRIER;
    g_stack_world.vehs[0].faction_id = 5;
    g_stack_world.vehs[0].x = 2;
    g_stack_world.vehs[0].y = 1;
    g_stack_world.vehs[1].proto_id = PROTO_AIR_GRUNT;
    g_stack_world.vehs[1].faction_id = 5;
    {
        const int ids[] = {0, 1};
        link_stack(ids, 2);
    }
    g_stack_world.tiles[5].bit = BIT_BASE_IN_TILE;
    g_stack_world.tiles[5].val2 = 0;  // owner 0 - rejected
    expect(stack_veh(0, 0) == 1);
    expect(g_stack_world.vehs[1].order == ORDER_NONE);
    g_stack_world.tiles[5].val2 = 1;  // owner 1 - accepted
    expect(stack_veh(0, 0) == 0);
    expect(g_stack_world.vehs[1].order == ORDER_SENTRY_BOARD);

    // ---------------------------------------------------------------
    // The air gate's upper bound. Owner 8 is MaxPlayerNum itself, so the
    // base does NOT open the gate; only owners 1..7 do. Without an airbase
    // the candidate is rejected and the carrier counter comes back whole.
    // ---------------------------------------------------------------
    reset_stack_world();
    g_stack_world.factions_status = 0;
    g_stack_world.vehs[0].proto_id = PROTO_AIR_CARRIER;
    g_stack_world.vehs[0].faction_id = 5;
    g_stack_world.vehs[0].x = 2;
    g_stack_world.vehs[0].y = 1;
    g_stack_world.vehs[1].proto_id = PROTO_AIR_GRUNT;
    g_stack_world.vehs[1].faction_id = 5;
    {
        const int ids[] = {0, 1};
        link_stack(ids, 2);
    }
    g_stack_world.tiles[5].bit = BIT_BASE_IN_TILE;
    g_stack_world.tiles[5].val2 = 8;  // owner 8 - not below MaxPlayerNum
    expect(stack_veh(0, 0) == 1);
    expect(g_stack_world.vehs[1].order == ORDER_NONE);

    // ---------------------------------------------------------------
    // The same bound in is_open_ocean, which is a different owner test on a
    // different tile read. An ocean tile whose base records owner 8 is still
    // open ocean, so the sentry rule set lets a held unit board.
    // ---------------------------------------------------------------
    reset_stack_world();
    g_stack_world.factions_status = (uint8_t)(1 << 5);
    g_stack_world.vehs[0].proto_id = PROTO_SEA_TRANSPORT;
    g_stack_world.vehs[0].faction_id = 5;
    g_stack_world.vehs[0].x = 2;
    g_stack_world.vehs[0].y = 1;
    g_stack_world.vehs[1].proto_id = PROTO_LAND_GRUNT;
    g_stack_world.vehs[1].faction_id = 5;
    g_stack_world.vehs[1].order = ORDER_HOLD;
    {
        const int ids[] = {0, 1};
        link_stack(ids, 2);
    }
    g_stack_world.tiles[5].climate = 0;  // altitude 0: ocean
    g_stack_world.tiles[5].bit = BIT_BASE_IN_TILE;
    g_stack_world.tiles[5].val2 = 8;
    expect(stack_veh(0, 0) == 3);
    expect(g_stack_world.vehs[1].order == ORDER_SENTRY_BOARD);

    // ---------------------------------------------------------------
    // The pass-0 rule set on its own. The faction's bit is SET, so pass 1
    // would reject a unit whose waypoint_x[0] already names a transport;
    // pass 0 accepts it precisely because that transport is this one. The
    // unit therefore boards only if pass 0 runs and only if pass 0 tests
    // the order for EQUALITY with sentry/board.
    // ---------------------------------------------------------------
    reset_stack_world();
    g_stack_world.factions_status = (uint8_t)(1 << 5);
    g_stack_world.vehs[0].proto_id = PROTO_SEA_TRANSPORT;
    g_stack_world.vehs[0].faction_id = 5;
    g_stack_world.vehs[1].proto_id = PROTO_LAND_GRUNT;
    g_stack_world.vehs[1].faction_id = 5;
    g_stack_world.vehs[1].order = ORDER_SENTRY_BOARD;
    g_stack_world.vehs[1].waypoint_x[0] = 0;  // already riding THIS transport
    {
        const int ids[] = {0, 1};
        link_stack(ids, 2);
    }
    expect(stack_veh(0, 0) == 3);
    expect(g_stack_world.vehs[1].state == VSTATE_UNK_1);
    expect(g_stack_world.vehs[1].waypoint_x[0] == 0);

    // ---------------------------------------------------------------
    // The mirror of that, with the transport id shifted off zero so the two
    // meanings of waypoint_x[0] cannot be confused. Unit 2 rides transport
    // 0, and the subject is transport 1: pass 0 rejects it because the ids
    // differ, and pass 1 rejects it because the waypoint is not negative.
    // The bound is STRICT - a waypoint of exactly 0 is a transport id, not
    // the "riding nothing" sentinel.
    // ---------------------------------------------------------------
    reset_stack_world();
    g_stack_world.factions_status = (uint8_t)(1 << 5);
    g_stack_world.vehs[1].proto_id = PROTO_SEA_TRANSPORT;
    g_stack_world.vehs[1].faction_id = 5;
    g_stack_world.vehs[2].proto_id = PROTO_LAND_GRUNT;
    g_stack_world.vehs[2].faction_id = 5;
    g_stack_world.vehs[2].order = ORDER_SENTRY_BOARD;
    g_stack_world.vehs[2].waypoint_x[0] = 0;
    {
        const int ids[] = {1, 2};
        link_stack(ids, 2);
    }
    expect(stack_veh(1, 0) == 4);  // nothing boarded
    expect(g_stack_world.vehs[2].waypoint_x[0] == 0);
    expect(g_stack_world.vehs[2].state == 0);

    // ---------------------------------------------------------------
    // With the faction's bit CLEAR the sentry rule set does not apply, and a
    // held unit falls to `accept = is_open_ocean` instead. Off the map that
    // is false, so it must NOT board - the assignment is the only thing
    // standing between it and the `accept = true` the branch started with.
    // ---------------------------------------------------------------
    reset_stack_world();
    g_stack_world.factions_status = 0;
    g_stack_world.vehs[0].proto_id = PROTO_SEA_TRANSPORT;
    g_stack_world.vehs[0].faction_id = 5;
    g_stack_world.vehs[1].proto_id = PROTO_LAND_GRUNT;
    g_stack_world.vehs[1].faction_id = 5;
    g_stack_world.vehs[1].order = ORDER_HOLD;
    {
        const int ids[] = {0, 1};
        link_stack(ids, 2);
    }
    expect(stack_veh(0, 0) == 4);
    expect(g_stack_world.vehs[1].order == ORDER_HOLD);
    expect(g_stack_world.vehs[1].state == 0);

    // Put the transport on open ocean and the same unit boards, which is the
    // other half of that assignment.
    g_stack_world.vehs[0].x = 2;
    g_stack_world.vehs[0].y = 1;
    g_stack_world.tiles[5].climate = 0;
    g_stack_world.tiles[5].bit = 0;
    expect(stack_veh(0, 0) == 3);
    expect(g_stack_world.vehs[1].order == ORDER_SENTRY_BOARD);

    // ---------------------------------------------------------------
    // The VSTATE_UNK_40000 rule set, which is the one branch that reads the
    // CANDIDATE's tile rather than the subject's, and reads it unguarded.
    //
    //   unit 1  at (2,1) -> tile 5, region 3, plan NAVAL_TRANSPORT -> boards
    //   unit 2  at (4,1) -> tile 6, region 4, plan DEFENSIVE       -> rejected
    //   unit 3  at (2,1) but without VSTATE_UNK_20000              -> rejected
    //
    // Halving x is load bearing: tile index is y * MapLongitude + (x >> 1),
    // so unit 1 reads tile 5 and not tile 6.
    // ---------------------------------------------------------------
    reset_stack_world();
    g_stack_world.factions_status = 0;
    g_stack_world.vehs[0].proto_id = PROTO_SEA_TRANSPORT;
    g_stack_world.vehs[0].faction_id = 5;
    g_stack_world.vehs[1].proto_id = PROTO_LAND_GRUNT;
    g_stack_world.vehs[1].faction_id = 5;
    g_stack_world.vehs[1].state = VSTATE_UNK_40000 | VSTATE_UNK_20000;
    g_stack_world.vehs[1].x = 2;
    g_stack_world.vehs[1].y = 1;
    g_stack_world.vehs[2].proto_id = PROTO_LAND_GRUNT;
    g_stack_world.vehs[2].faction_id = 5;
    g_stack_world.vehs[2].state = VSTATE_UNK_40000 | VSTATE_UNK_20000;
    g_stack_world.vehs[2].x = 4;
    g_stack_world.vehs[2].y = 1;
    g_stack_world.vehs[3].proto_id = PROTO_LAND_GRUNT;
    g_stack_world.vehs[3].faction_id = 5;
    g_stack_world.vehs[3].state = VSTATE_UNK_40000;  // no VSTATE_UNK_20000
    g_stack_world.vehs[3].x = 2;
    g_stack_world.vehs[3].y = 1;
    {
        const int ids[] = {0, 1, 2, 3};
        link_stack(ids, 4);
    }
    g_stack_world.tiles[5].region = 3;
    g_stack_world.tiles[6].region = 4;
    g_stack_world.players_data[5].region_base_plan[3] = PLAN_NAVAL_TRANSPORT;
    g_stack_world.players_data[5].region_base_plan[4] = PLAN_DEFENSIVE;
    expect(stack_veh(0, 0) == 3);  // only unit 1 boarded
    expect(g_stack_world.vehs[1].order == ORDER_SENTRY_BOARD);
    expect(g_stack_world.vehs[1].waypoint_x[0] == 0);
    expect(g_stack_world.vehs[2].order == ORDER_NONE);
    expect(g_stack_world.vehs[3].order == ORDER_NONE);

    // ---------------------------------------------------------------
    // The PLAN_DEFENSIVE veto, and its plan bound. Off open ocean, a unit
    // standing in a region the faction plans to defend is held back - but
    // only while its own plan is BELOW PLAN_COLONIZATION. A colony pod sits
    // exactly on that bound and is therefore let through, which is what
    // makes the comparison strict rather than inclusive.
    // ---------------------------------------------------------------
    reset_stack_world();
    g_stack_world.factions_status = 0;
    g_stack_world.vehs[0].proto_id = PROTO_SEA_TRANSPORT;
    g_stack_world.vehs[0].faction_id = 5;  // off the map: not open ocean
    g_stack_world.vehs[1].proto_id = PROTO_LAND_GRUNT;  // PLAN_COMBAT, below the bound
    g_stack_world.vehs[1].faction_id = 5;
    g_stack_world.vehs[1].x = 2;
    g_stack_world.vehs[1].y = 1;
    g_stack_world.vehs[2].proto_id = PROTO_LAND_COLONY;  // PLAN_COLONIZATION, on the bound
    g_stack_world.vehs[2].faction_id = 5;
    g_stack_world.vehs[2].x = 2;
    g_stack_world.vehs[2].y = 1;
    {
        const int ids[] = {0, 1, 2};
        link_stack(ids, 3);
    }
    g_stack_world.tiles[5].region = 3;
    g_stack_world.players_data[5].region_base_plan[3] = PLAN_DEFENSIVE;
    expect(stack_veh(0, 0) == 3);  // only the colony pod boarded
    expect(g_stack_world.vehs[1].order == ORDER_NONE);
    expect(g_stack_world.vehs[2].order == ORDER_SENTRY_BOARD);
    expect(g_stack_world.vehs[2].waypoint_x[0] == 0);

    // ---------------------------------------------------------------
    // The two counters are independent, and a transport's carrier counter
    // starts at ZERO. The stack deliberately puts an air unit both BEFORE
    // and AFTER the land unit that boards:
    //
    //   unit 1 (air)  - rejected because carrier_left is 0 from the start
    //   unit 2 (land) - boards on the cargo counter
    //   unit 3 (air)  - rejected because boarding unit 2 must not have
    //                   driven carrier_left below zero
    // ---------------------------------------------------------------
    reset_stack_world();
    g_stack_world.factions_status = 0;
    g_stack_world.vehs[0].proto_id = PROTO_SEA_TRANSPORT;  // no ABL_CARRIER
    g_stack_world.vehs[0].faction_id = 5;
    g_stack_world.vehs[1].proto_id = PROTO_AIR_GRUNT;
    g_stack_world.vehs[1].faction_id = 5;
    g_stack_world.vehs[2].proto_id = PROTO_LAND_GRUNT;
    g_stack_world.vehs[2].faction_id = 5;
    g_stack_world.vehs[3].proto_id = PROTO_AIR_GRUNT;
    g_stack_world.vehs[3].faction_id = 5;
    {
        const int ids[] = {0, 1, 2, 3};
        link_stack(ids, 4);
    }
    expect(stack_veh(0, 0) == 3);
    expect(g_stack_world.vehs[1].order == ORDER_NONE);
    expect(g_stack_world.vehs[2].order == ORDER_SENTRY_BOARD);
    expect(g_stack_world.vehs[3].order == ORDER_NONE);

    // ---------------------------------------------------------------
    // The cargo counter's own decrement, watched at the point where it runs
    // out. A capacity-1 transport must take exactly one of two identical
    // land units; the second is stopped by the counter, not by any rule.
    // ---------------------------------------------------------------
    reset_stack_world();
    g_stack_world.factions_status = 0;
    g_stack_world.vehs[0].proto_id = PROTO_SEA_SKIFF;  // carry_capacity 1
    g_stack_world.vehs[0].faction_id = 5;
    g_stack_world.vehs[1].proto_id = PROTO_LAND_GRUNT;
    g_stack_world.vehs[1].faction_id = 5;
    g_stack_world.vehs[2].proto_id = PROTO_LAND_GRUNT;
    g_stack_world.vehs[2].faction_id = 5;
    {
        const int ids[] = {0, 1, 2};
        link_stack(ids, 3);
    }
    expect(stack_veh(0, 0) == 0);
    expect(g_stack_world.vehs[1].order == ORDER_SENTRY_BOARD);
    expect(g_stack_world.vehs[2].order == ORDER_NONE);

    // ---------------------------------------------------------------
    // And the cargo counter must NOT be decremented past zero. A pure
    // carrier starts with cargo_left 0 and carrier_left 1; the air unit it
    // takes runs off the carrier counter alone. If the cargo counter went
    // to -1 the loop would neither stop nor treat the land unit behind it as
    // ineligible, and the land unit would board a carrier.
    // ---------------------------------------------------------------
    reset_stack_world();
    g_stack_world.factions_status = 0;
    g_stack_world.vehs[0].proto_id = PROTO_AIR_CARRIER;
    g_stack_world.vehs[0].faction_id = 5;
    g_stack_world.vehs[0].x = 2;
    g_stack_world.vehs[0].y = 1;
    g_stack_world.vehs[1].proto_id = PROTO_AIR_GRUNT;
    g_stack_world.vehs[1].faction_id = 5;
    g_stack_world.vehs[2].proto_id = PROTO_LAND_GRUNT;
    g_stack_world.vehs[2].faction_id = 5;
    {
        const int ids[] = {0, 1, 2};
        link_stack(ids, 3);
    }
    g_stack_world.tiles[5].bit = BIT_AIRBASE;
    expect(stack_veh(0, 0) == 0);
    expect(g_stack_world.vehs[1].order == ORDER_SENTRY_BOARD);
    expect(g_stack_world.vehs[2].order == ORDER_NONE);

    // ===============================================================
    // From here on, mode != 0. These paths move units: they call
    // veh_lift/veh_drop, which rewrite x, y and both stack links, so the
    // off-map holding square (-2, -2) is an observable and the assertions
    // below read it directly.
    // ===============================================================

    // ---------------------------------------------------------------
    // A subject that is neither a transport nor a carrier still gets lifted
    // to the holding square in any non-zero mode, and the call returns the
    // unit id. Unit 3 rather than unit 0 so the id cannot be confused with
    // the 0 the mode-0 arm of the same expression returns.
    // ---------------------------------------------------------------
    reset_stack_world();
    g_stack_world.vehs[3].proto_id = PROTO_LAND_GRUNT;
    g_stack_world.vehs[3].faction_id = 5;
    g_stack_world.vehs[4].proto_id = PROTO_LAND_GRUNT;
    g_stack_world.vehs[4].faction_id = 5;
    {
        const int ids[] = {3, 4};
        link_stack(ids, 2);
    }
    expect(stack_veh(3, 1) == 3);
    expect(g_stack_world.vehs[3].x == -2);
    expect(g_stack_world.vehs[3].y == -2);
    // veh_lift unlinked it from the stack on the way out; nothing else moved.
    expect(g_stack_world.vehs[3].next_veh_id_stack == -1);
    expect(g_stack_world.vehs[4].prev_veh_id_stack == -1);
    expect(g_stack_world.vehs[4].x == -1);
    expect(g_stack_world.vehs[4].order == ORDER_NONE);

    // ---------------------------------------------------------------
    // Mode 1, and the sort that precedes it. The transport is deliberately
    // NOT at the top of its stack, and the mode != 0 walk starts at the
    // SUBJECT rather than at the stack top - so without stack_sort the
    // transport's next link is -1 and the grunt above it is never seen.
    //
    // The grunt also carries VSTATE_UNK_10000, which the boarding body
    // clears after the lift.
    // ---------------------------------------------------------------
    reset_stack_world();
    g_stack_world.factions_status = 0;
    g_stack_world.vehs[0].proto_id = PROTO_SEA_TRANSPORT;
    g_stack_world.vehs[0].faction_id = 5;
    g_stack_world.vehs[0].x = 2;
    g_stack_world.vehs[0].y = 1;
    g_stack_world.vehs[1].proto_id = PROTO_LAND_GRUNT;
    g_stack_world.vehs[1].faction_id = 5;
    g_stack_world.vehs[1].x = 2;
    g_stack_world.vehs[1].y = 1;
    g_stack_world.vehs[1].state = VSTATE_UNK_10000;
    {
        const int ids[] = {1, 0};  // grunt on top, transport underneath
        link_stack(ids, 2);
    }
    g_stack_world.tiles[5].climate = (uint8_t)ALT_BIT_SHORE_LINE;  // land, not open ocean
    g_stack_world.tiles[5].bit = BIT_VEH_IN_TILE;
    expect(stack_veh(0, 1) == 0);
    expect(g_stack_world.vehs[1].order == ORDER_SENTRY_BOARD);
    expect(g_stack_world.vehs[1].waypoint_x[0] == 0);
    expect(g_stack_world.vehs[1].waypoint_y[0] == 0);
    expect(g_stack_world.vehs[1].x == -2);
    expect(g_stack_world.vehs[1].y == -2);
    expect(g_stack_world.vehs[1].state == 0);  // VSTATE_UNK_10000 cleared after the lift
    // The subject is lifted last, by the return expression itself.
    expect(g_stack_world.vehs[0].x == -2);
    expect(g_stack_world.vehs[0].y == -2);

    // ---------------------------------------------------------------
    // Mode 1 does NOT take the owned-base short circuit: that is mode 2 and
    // above. The subject sits on a base owned by faction 4, which would end
    // the call immediately if the bound were `mode > 0` or `mode >= 1`.
    // Instead both passes run: unit 1 on pass 0 (already recorded as riding
    // this transport) and unit 2 on pass 1.
    // ---------------------------------------------------------------
    reset_stack_world();
    g_stack_world.factions_status = 0;
    g_stack_world.vehs[0].proto_id = PROTO_SEA_TRANSPORT;
    g_stack_world.vehs[0].faction_id = 5;
    g_stack_world.vehs[0].x = 2;
    g_stack_world.vehs[0].y = 1;
    g_stack_world.vehs[1].proto_id = PROTO_LAND_GRUNT;
    g_stack_world.vehs[1].faction_id = 5;
    g_stack_world.vehs[1].x = 2;
    g_stack_world.vehs[1].y = 1;
    g_stack_world.vehs[1].order = ORDER_SENTRY_BOARD;
    g_stack_world.vehs[1].waypoint_x[0] = 0;
    g_stack_world.vehs[2].proto_id = PROTO_LAND_GRUNT;
    g_stack_world.vehs[2].faction_id = 5;
    g_stack_world.vehs[2].x = 2;
    g_stack_world.vehs[2].y = 1;
    {
        const int ids[] = {0, 1, 2};
        link_stack(ids, 3);
    }
    g_stack_world.tiles[5].climate = (uint8_t)ALT_BIT_SHORE_LINE;
    g_stack_world.tiles[5].bit = BIT_BASE_IN_TILE | BIT_VEH_IN_TILE;
    g_stack_world.tiles[5].val2 = 4;  // owner 4, below MaxPlayerNum
    expect(stack_veh(0, 1) == 0);
    expect(g_stack_world.vehs[1].x == -2);  // boarded on pass 0
    expect(g_stack_world.vehs[2].x == -2);  // boarded on pass 1
    expect(g_stack_world.vehs[2].order == ORDER_SENTRY_BOARD);
    expect(g_stack_world.vehs[2].waypoint_x[0] == 0);

    // ---------------------------------------------------------------
    // Mode 2 on the same board: now the short circuit DOES fire. It runs
    // before stack_sort and before either pass, so the unit that mode 1
    // boarded above is left exactly where it stood, and only the subject
    // moves to the holding square.
    //
    // The tile it reads is y * MapLongitude + (x >> 1) = 1 * 4 + 1 = 5.
    // Tile 6 - the one an unhalved x would reach - is left empty, so
    // dropping the shift turns the short circuit off.
    // ---------------------------------------------------------------
    reset_stack_world();
    g_stack_world.factions_status = 0;
    g_stack_world.vehs[0].proto_id = PROTO_SEA_TRANSPORT;
    g_stack_world.vehs[0].faction_id = 5;
    g_stack_world.vehs[0].x = 2;
    g_stack_world.vehs[0].y = 1;
    g_stack_world.vehs[1].proto_id = PROTO_LAND_GRUNT;
    g_stack_world.vehs[1].faction_id = 5;
    g_stack_world.vehs[1].x = 2;
    g_stack_world.vehs[1].y = 1;
    g_stack_world.vehs[1].order = ORDER_SENTRY_BOARD;
    g_stack_world.vehs[1].waypoint_x[0] = 0;
    {
        const int ids[] = {0, 1};
        link_stack(ids, 2);
    }
    g_stack_world.tiles[5].climate = (uint8_t)ALT_BIT_SHORE_LINE;
    g_stack_world.tiles[5].bit = BIT_BASE_IN_TILE | BIT_VEH_IN_TILE;
    g_stack_world.tiles[5].val2 = 4;
    expect(stack_veh(0, 2) == 0);
    expect(g_stack_world.vehs[0].x == -2);
    expect(g_stack_world.vehs[0].y == -2);
    expect(g_stack_world.vehs[1].x == 2);
    expect(g_stack_world.vehs[1].y == 1);
    expect(g_stack_world.vehs[1].order == ORDER_SENTRY_BOARD);
    expect(g_stack_world.vehs[1].waypoint_x[0] == 0);

    // ---------------------------------------------------------------
    // Mode 2 with an UNOCCUPIED base (owner 0xF). The nibble is not below
    // MaxPlayerNum, so the short circuit does not fire and the call goes on
    // to sort and run its single pass. Unit 1 boards there; unit 2, which
    // only the general pass would take, does NOT - mode 2 runs pass 0 alone.
    // ---------------------------------------------------------------
    reset_stack_world();
    g_stack_world.factions_status = 0;
    g_stack_world.vehs[0].proto_id = PROTO_SEA_TRANSPORT;
    g_stack_world.vehs[0].faction_id = 5;
    g_stack_world.vehs[0].x = 2;
    g_stack_world.vehs[0].y = 1;
    g_stack_world.vehs[1].proto_id = PROTO_LAND_GRUNT;
    g_stack_world.vehs[1].faction_id = 5;
    g_stack_world.vehs[1].x = 2;
    g_stack_world.vehs[1].y = 1;
    g_stack_world.vehs[1].order = ORDER_SENTRY_BOARD;
    g_stack_world.vehs[1].waypoint_x[0] = 0;
    g_stack_world.vehs[2].proto_id = PROTO_LAND_GRUNT;
    g_stack_world.vehs[2].faction_id = 5;
    g_stack_world.vehs[2].x = 2;
    g_stack_world.vehs[2].y = 1;
    {
        const int ids[] = {0, 1, 2};
        link_stack(ids, 3);
    }
    g_stack_world.tiles[5].climate = (uint8_t)ALT_BIT_SHORE_LINE;
    g_stack_world.tiles[5].bit = BIT_BASE_IN_TILE | BIT_VEH_IN_TILE;
    g_stack_world.tiles[5].val2 = 0xF;  // unoccupied
    expect(stack_veh(0, 2) == 0);
    expect(g_stack_world.vehs[1].x == -2);
    expect(g_stack_world.vehs[2].x == 2);
    expect(g_stack_world.vehs[2].order == ORDER_NONE);

    // Owner 8 is MaxPlayerNum itself and behaves the same way: the bound is
    // strict, so the short circuit still does not fire.
    reset_stack_world();
    g_stack_world.factions_status = 0;
    g_stack_world.vehs[0].proto_id = PROTO_SEA_TRANSPORT;
    g_stack_world.vehs[0].faction_id = 5;
    g_stack_world.vehs[0].x = 2;
    g_stack_world.vehs[0].y = 1;
    g_stack_world.vehs[1].proto_id = PROTO_LAND_GRUNT;
    g_stack_world.vehs[1].faction_id = 5;
    g_stack_world.vehs[1].x = 2;
    g_stack_world.vehs[1].y = 1;
    g_stack_world.vehs[1].order = ORDER_SENTRY_BOARD;
    g_stack_world.vehs[1].waypoint_x[0] = 0;
    {
        const int ids[] = {0, 1};
        link_stack(ids, 2);
    }
    g_stack_world.tiles[5].climate = (uint8_t)ALT_BIT_SHORE_LINE;
    g_stack_world.tiles[5].bit = BIT_BASE_IN_TILE | BIT_VEH_IN_TILE;
    g_stack_world.tiles[5].val2 = 8;
    expect(stack_veh(0, 2) == 0);
    expect(g_stack_world.vehs[1].x == -2);
}


/*
 * A self-contained world for action_home().
 *
 * Every global the function and its callees reach is a pointer, so ScopedSeam
 * gives the whole world local storage and no fixed 0x0095xxxx address is ever
 * dereferenced. The map is 16 wide by 8 tall, so a tile index is
 * (x >> 1) + y * 8 and 64 tiles cover it exactly.
 *
 * The subject's prototype id stays below MaxVehProtoFactionNum (64) and its
 * weapon's offense_rating is negative on purpose. That pair is what keeps
 * speed() cheap and exact: has_abil() computes faction 0 and returns before it
 * reads Players or Ability; morale_veh() takes the "Basic Psi Veh" early return
 * before it reads Players, Technology or the social engineering block; and the
 * elite-morale bonus in speed() is disabled by the same offense_rating test, so
 * a unit's speed is exactly its chassis speed times move_rate_roads. Every
 * distance threshold below is therefore an exact integer, not an approximation.
 *
 * SecretProject is filled with -1 so has_project() is false for every project,
 * which is what keeps the Maritime Control Center and Cloudbase Academy bonuses
 * out of speed() and speed_proto().
 *
 * PlayerData is ~8 KB each, so the world is static rather than a local.
 */
struct ActionHomeWorld {
    Veh vehs[8];
    VehPrototype protos[8];
    RulesChassis chassis[4];
    RulesWeapon weapons[2];
    Base bases[4];
    Map tiles[64];
    PlayerData players[16];
    BaseSecretProject projects;
    RulesBasic rules;
    uint8_t factions_status[4];
    Map *tiles_ptr;
    int base_count;
    int veh_count;
    int lon_bounds;
    int lat_bounds;
    uint32_t longitude;
    BOOL is_flat;
    uint32_t game_state;
};

ActionHomeWorld g_home_world;

// Chassis ids used by the action_home fixtures.
const uint8_t HOME_CH_LAND = 0;
const uint8_t HOME_CH_SEA = 1;
const uint8_t HOME_CH_AIR = 2;

// Prototype ids used by the action_home fixtures.
const int HOME_P_LAND = 1;        // TRIAD_LAND, no abilities
const int HOME_P_SEA = 2;         // TRIAD_SEA
const int HOME_P_AIR = 3;         // TRIAD_AIR, PLAN_COMBAT - neither carrier nor convoy
const int HOME_P_CARRIER = 4;     // TRIAD_AIR, ABL_CARRIER
const int HOME_P_AIRCONVOY = 5;   // TRIAD_AIR, PLAN_SUPPLY_CONVOY
const int HOME_P_LANDCONVOY = 6;  // TRIAD_LAND, PLAN_SUPPLY_CONVOY

// The map is 16 wide, so map_loc()'s index is (x >> 1) + y * MapLongitude.
int home_tile(int x, int y) { return (x >> 1) + y * 8; }

void home_reset() {
    std::memset(&g_home_world, 0, sizeof(g_home_world));
    std::memset(&g_home_world.projects, 0xFF, sizeof(g_home_world.projects));
    g_home_world.tiles_ptr = g_home_world.tiles;
    g_home_world.lon_bounds = 16;
    g_home_world.lat_bounds = 8;
    g_home_world.longitude = 8;  // map.h: "halve of MapLongitudeBounds"
    g_home_world.is_flat = 1;    // keeps xrange() and x_dist() free of wrap-around
    g_home_world.rules.move_rate_roads = 1;  // so max_dist is reach, undivided
    g_home_world.veh_count = 1;
    g_home_world.base_count = 0;
    g_home_world.factions_status[0] = 0x02;  // faction 1 is a human player

    // offense_rating < 0 is load-bearing; see the note on the struct.
    g_home_world.weapons[0].offense_rating = -1;
    g_home_world.weapons[0].mode = 0;  // not WPN_MODE_TRANSPORT

    g_home_world.chassis[HOME_CH_LAND].triad = TRIAD_LAND;
    g_home_world.chassis[HOME_CH_LAND].speed = 2;
    g_home_world.chassis[HOME_CH_SEA].triad = TRIAD_SEA;
    g_home_world.chassis[HOME_CH_SEA].speed = 2;
    // An air chassis gets reactor_id * 2 added by speed_proto(), so a base speed
    // of 0 with a fission reactor lands on the same 2 as the other two triads.
    g_home_world.chassis[HOME_CH_AIR].triad = TRIAD_AIR;
    g_home_world.chassis[HOME_CH_AIR].speed = 0;

    for (int k = 1; k < 8; k++) {
        g_home_world.protos[k].reactor_id = 1;
        g_home_world.protos[k].plan = PLAN_COMBAT;
    }
    g_home_world.protos[HOME_P_LAND].chassis_id = HOME_CH_LAND;
    g_home_world.protos[HOME_P_SEA].chassis_id = HOME_CH_SEA;
    g_home_world.protos[HOME_P_AIR].chassis_id = HOME_CH_AIR;
    g_home_world.protos[HOME_P_CARRIER].chassis_id = HOME_CH_AIR;
    g_home_world.protos[HOME_P_CARRIER].ability_flags = ABL_CARRIER;
    g_home_world.protos[HOME_P_AIRCONVOY].chassis_id = HOME_CH_AIR;
    g_home_world.protos[HOME_P_AIRCONVOY].plan = PLAN_SUPPLY_CONVOY;
    g_home_world.protos[HOME_P_LANDCONVOY].chassis_id = HOME_CH_LAND;
    g_home_world.protos[HOME_P_LANDCONVOY].plan = PLAN_SUPPLY_CONVOY;

    for (int k = 0; k < 8; k++) {
        Veh &veh = g_home_world.vehs[k];
        veh.x = -1;
        veh.y = -1;
        veh.order = ORDER_NONE;
    }
    Veh &subject = g_home_world.vehs[0];
    subject.x = 2;
    subject.y = 2;
    subject.faction_id = 1;
    subject.proto_id = HOME_P_LAND;
    // A value that is neither ORDER_NONE nor ORDER_MOVE_TO, so "the orders were
    // left alone" and "the orders were cleared" are distinguishable outcomes.
    subject.order = ORDER_SENTRY_BOARD;
}

// Raise every tile to the shore line, so is_ocean() is false map-wide and
// base_on_sea() can never connect two tiles.
void home_dry_land() {
    for (int k = 0; k < 64; k++) {
        g_home_world.tiles[k].climate = (uint8_t)ALT_BIT_SHORE_LINE;
    }
}

// Report which assertion failed. expect() only counts, and a sweep that has to
// bisect 200 assertions by hand is a sweep that does not get run.
#define HOME_CHECK(cond)                                                      \
    do {                                                                      \
        const bool home_ok = (cond);                                          \
        if (!home_ok) {                                                       \
            std::fprintf(stderr, "action_home: line %d: %s\n", __LINE__,      \
                         #cond);                                              \
        }                                                                     \
        expect(home_ok);                                                      \
    } while (0)

void test_action_home() {
    ScopedSeam<Veh> vehs_seam(&Vehs, g_home_world.vehs);
    ScopedSeam<VehPrototype> protos_seam(&VehPrototypes, g_home_world.protos);
    ScopedSeam<RulesChassis> chassis_seam(&Chassis, g_home_world.chassis);
    ScopedSeam<RulesWeapon> weapon_seam(&Weapon, g_home_world.weapons);
    ScopedSeam<Base> bases_seam(&Bases, g_home_world.bases);
    ScopedSeam<int> base_count_seam(&BaseCurrentCount, &g_home_world.base_count);
    ScopedSeam<int> veh_count_seam(&VehCurrentCount, &g_home_world.veh_count);
    ScopedSeam<PlayerData> players_seam(&PlayersData, g_home_world.players);
    ScopedSeam<uint8_t> status_seam(&FactionsStatus, g_home_world.factions_status);
    ScopedSeam<Map *> tiles_seam(&MapTiles, &g_home_world.tiles_ptr);
    ScopedSeam<int> lon_seam(&MapLongitudeBounds, &g_home_world.lon_bounds);
    ScopedSeam<int> lat_seam(&MapLatitudeBounds, &g_home_world.lat_bounds);
    ScopedSeam<uint32_t> longitude_seam(&MapLongitude, &g_home_world.longitude);
    ScopedSeam<BOOL> flat_seam(&MapIsFlat, &g_home_world.is_flat);
    ScopedSeam<RulesBasic> rules_seam(&Rules, &g_home_world.rules);
    ScopedSeam<uint32_t> state_seam(&GameState, &g_home_world.game_state);
    ScopedSeam<BaseSecretProject> projects_seam(&SecretProject, &g_home_world.projects);

    Veh &veh = g_home_world.vehs[0];
    Base &base = g_home_world.bases[0];

    // ---- nothing anywhere -------------------------------------------------
    // best stays (-1,-1), which fails the on_map() guard. Nothing is written.
    // This is also what pins the (-1,-1) initialisers: (0,0) is on the map, so
    // a zero-initialised best would resolve and issue a move order.
    home_reset();
    HOME_CHECK(action_home(0, 0) == 0);
    HOME_CHECK(veh.order == ORDER_SENTRY_BOARD);
    HOME_CHECK(veh.waypoint_x[0] == 0);
    HOME_CHECK(veh.waypoint_y[0] == 0);
    HOME_CHECK(veh.state == 0);

    // ---- a friendly base, out of reach ------------------------------------
    // dist == (x_dist(2,6) + abs(2-4)) >> 1 == (4 + 2) >> 1 == 3.
    home_reset();
    g_home_world.base_count = 1;
    base.x = 6;
    base.y = 4;
    base.faction_id_current = 1;
    HOME_CHECK(action_home(0, 0) == 1);
    HOME_CHECK(veh.order == ORDER_MOVE_TO);
    HOME_CHECK(veh.waypoint_x[0] == 6);
    HOME_CHECK(veh.waypoint_y[0] == 4);
    HOME_CHECK(veh.moves_expended == 0);  // a range 0 chassis never reaches veh_skip

    // The y half of the distance is a separate term from the x half: moving the
    // base to the same column leaves a strictly smaller distance, and the
    // waypoint is written from the base's own coordinates, not the unit's.
    home_reset();
    g_home_world.base_count = 1;
    base.x = 2;
    base.y = 6;
    base.faction_id_current = 1;
    HOME_CHECK(action_home(0, 0) == 1);
    HOME_CHECK(veh.waypoint_x[0] == 2);
    HOME_CHECK(veh.waypoint_y[0] == 6);

    // ---- standing on the base ---------------------------------------------
    // A human keeps the rest of the turn; the AI spends it through veh_skip().
    home_reset();
    g_home_world.base_count = 1;
    base.x = 2;
    base.y = 2;
    base.faction_id_current = 1;
    veh.moves_expended = 7;
    HOME_CHECK(action_home(0, 0) == 1);
    HOME_CHECK(veh.order == ORDER_NONE);
    HOME_CHECK(veh.waypoint_x[0] == 0);
    HOME_CHECK(veh.waypoint_y[0] == 0);
    HOME_CHECK(veh.moves_expended == 7);

    home_reset();
    g_home_world.base_count = 1;
    g_home_world.factions_status[0] = 0;  // faction 1 is the AI now
    base.x = 2;
    base.y = 2;
    base.faction_id_current = 1;
    veh.moves_expended = 7;
    HOME_CHECK(action_home(0, 0) == 1);
    HOME_CHECK(veh.order == ORDER_NONE);
    HOME_CHECK(veh.moves_expended == 2);  // veh_skip() ran: speed() is 2

    // ---- the pact bit ------------------------------------------------------
    home_reset();
    g_home_world.base_count = 1;
    base.x = 6;
    base.y = 4;
    base.faction_id_current = 2;
    HOME_CHECK(action_home(0, 0) == 0);
    HOME_CHECK(veh.order == ORDER_SENTRY_BOARD);
    g_home_world.players[1].diplo_treaties[2] = 0x2;  // a treaty, but not a pact
    HOME_CHECK(action_home(0, 0) == 0);
    g_home_world.players[1].diplo_treaties[2] = DTREATY_PACT;
    HOME_CHECK(action_home(0, 0) == 1);
    HOME_CHECK(veh.order == ORDER_MOVE_TO);
    HOME_CHECK(veh.waypoint_x[0] == 6);
    HOME_CHECK(veh.waypoint_y[0] == 4);
    // The treaty is read from the SUBJECT's row against the BASE's faction, so
    // the transposed lookup is a different cell.
    home_reset();
    g_home_world.base_count = 1;
    base.x = 6;
    base.y = 4;
    base.faction_id_current = 2;
    g_home_world.players[2].diplo_treaties[1] = DTREATY_PACT;
    HOME_CHECK(action_home(0, 0) == 0);

    // ---- flags == 2 rejects a base carrying BSTATE_UNK_200000 --------------
    home_reset();
    g_home_world.base_count = 1;
    base.x = 6;
    base.y = 4;
    base.faction_id_current = 1;
    base.state = BSTATE_UNK_200000;
    HOME_CHECK(action_home(0, 2) == 0);
    HOME_CHECK(veh.order == ORDER_SENTRY_BOARD);
    HOME_CHECK(action_home(0, 0) == 1);  // same base, a different mode
    HOME_CHECK(veh.order == ORDER_MOVE_TO);
    // The test is equality against 2, not a bit test: 3 is also odd and also
    // has bit 1 set, and it must NOT reject.
    home_reset();
    g_home_world.base_count = 1;
    g_home_world.chassis[HOME_CH_LAND].range = 2;  // clears the endurance gate
    base.x = 6;
    base.y = 4;
    base.faction_id_current = 1;
    base.state = BSTATE_UNK_200000;
    HOME_CHECK(action_home(0, 3) == 1);
    HOME_CHECK(veh.order == ORDER_MOVE_TO);
    HOME_CHECK(veh.waypoint_x[0] == 6);
    // ... and with the bit clear, mode 2 accepts.
    home_reset();
    g_home_world.base_count = 1;
    base.x = 6;
    base.y = 4;
    base.faction_id_current = 1;
    HOME_CHECK(action_home(0, 2) == 1);
    HOME_CHECK(veh.order == ORDER_MOVE_TO);

    // ---- TRIAD_LAND needs a land route ------------------------------------
    home_reset();
    g_home_world.base_count = 1;
    base.x = 6;
    base.y = 4;
    base.faction_id_current = 1;
    g_home_world.tiles[home_tile(6, 4)].region = 9;  // the unit's tile is still 0
    HOME_CHECK(action_home(0, 0) == 0);
    // veh_region is read from the unit's own tile, so matching there matches.
    g_home_world.tiles[home_tile(2, 2)].region = 9;
    HOME_CHECK(action_home(0, 0) == 1);
    HOME_CHECK(veh.waypoint_x[0] == 6);
    HOME_CHECK(veh.waypoint_y[0] == 4);

    // ---- TRIAD_SEA ---------------------------------------------------------
    // A base on the unit's own tile is accepted without consulting base_on_sea.
    // Raising every tile to the shore line makes is_ocean() false everywhere,
    // so base_on_sea() cannot succeed and the same-tile short circuit is the
    // only way in.
    home_reset();
    veh.proto_id = HOME_P_SEA;
    home_dry_land();
    g_home_world.base_count = 1;
    base.x = 2;
    base.y = 2;
    base.faction_id_current = 1;
    HOME_CHECK(action_home(0, 0) == 1);
    HOME_CHECK(veh.order == ORDER_NONE);

    // Sharing only the column is not sharing the tile, and neither is sharing
    // only the row: both halves of the coordinate test are required.
    home_reset();
    veh.proto_id = HOME_P_SEA;
    home_dry_land();
    g_home_world.base_count = 1;
    base.x = 2;
    base.y = 6;
    base.faction_id_current = 1;
    HOME_CHECK(action_home(0, 0) == 0);
    HOME_CHECK(veh.order == ORDER_SENTRY_BOARD);
    home_reset();
    veh.proto_id = HOME_P_SEA;
    home_dry_land();
    g_home_world.base_count = 1;
    base.x = 6;
    base.y = 2;
    base.faction_id_current = 1;
    HOME_CHECK(action_home(0, 0) == 0);
    HOME_CHECK(veh.order == ORDER_SENTRY_BOARD);
    // An all-ocean, single-region map connects them, and base_on_sea says yes.
    home_reset();
    veh.proto_id = HOME_P_SEA;
    g_home_world.base_count = 1;
    base.x = 6;
    base.y = 4;
    base.faction_id_current = 1;
    HOME_CHECK(action_home(0, 0) == 1);
    HOME_CHECK(veh.waypoint_x[0] == 6);
    HOME_CHECK(veh.waypoint_y[0] == 4);
    // A sea unit is not held to the land unit's region test: the base tile
    // itself being another region changes nothing while the water connects.
    home_reset();
    veh.proto_id = HOME_P_SEA;
    g_home_world.base_count = 1;
    base.x = 6;
    base.y = 4;
    base.faction_id_current = 1;
    g_home_world.tiles[home_tile(6, 4)].region = 9;
    HOME_CHECK(action_home(0, 0) == 1);

    // ---- the base loop keeps the strict minimum, and honours the count -----
    home_reset();
    g_home_world.base_count = 2;
    g_home_world.bases[0].x = 6;
    g_home_world.bases[0].y = 4;
    g_home_world.bases[0].faction_id_current = 1;
    g_home_world.bases[1].x = 4;
    g_home_world.bases[1].y = 2;
    g_home_world.bases[1].faction_id_current = 1;
    HOME_CHECK(action_home(0, 0) == 1);
    HOME_CHECK(veh.waypoint_x[0] == 4);   // dist 1 beats dist 3
    HOME_CHECK(veh.waypoint_y[0] == 2);
    // Equal distances keep the FIRST base: the comparison is strict.
    home_reset();
    g_home_world.base_count = 2;
    g_home_world.bases[0].x = 2;
    g_home_world.bases[0].y = 4;
    g_home_world.bases[0].faction_id_current = 1;
    g_home_world.bases[1].x = 4;
    g_home_world.bases[1].y = 2;
    g_home_world.bases[1].faction_id_current = 1;
    HOME_CHECK(action_home(0, 0) == 1);
    HOME_CHECK(veh.waypoint_x[0] == 2);
    HOME_CHECK(veh.waypoint_y[0] == 4);
    // The same pair with the count at 1: base 1 is out of the scan entirely.
    home_reset();
    g_home_world.base_count = 1;
    g_home_world.bases[0].x = 6;
    g_home_world.bases[0].y = 4;
    g_home_world.bases[0].faction_id_current = 1;
    g_home_world.bases[1].x = 2;
    g_home_world.bases[1].y = 2;
    g_home_world.bases[1].faction_id_current = 1;
    HOME_CHECK(action_home(0, 0) == 1);
    HOME_CHECK(veh.order == ORDER_MOVE_TO);  // not the "standing on it" answer
    HOME_CHECK(veh.waypoint_x[0] == 6);

    // ---- the base loop's on-alert shortcut ---------------------------------
    // It reaches the finish WITHOUT passing the endurance gate: same range 0
    // chassis and same odd flags value that the next block shows returning 0.
    home_reset();
    g_home_world.base_count = 1;
    base.x = 2;
    base.y = 2;
    base.faction_id_current = 1;
    veh.state = VSTATE_UNK_200;
    veh.order_auto_type = ORDERA_ON_ALERT;
    veh.waypoint_x[1] = 2;
    veh.waypoint_y[1] = 2;
    HOME_CHECK(action_home(0, 1) == 1);
    HOME_CHECK(veh.order == ORDER_NONE);
    // Drop any one of the five predicates and the shortcut stops firing, so the
    // gate runs after all and refuses. Each is restored before the next.
    veh.order_auto_type = 0;
    HOME_CHECK(action_home(0, 1) == 0);
    veh.order_auto_type = ORDERA_ON_ALERT;
    veh.state = 0;
    HOME_CHECK(action_home(0, 1) == 0);
    veh.state = VSTATE_UNK_200;
    veh.waypoint_x[1] = 7;
    HOME_CHECK(action_home(0, 1) == 0);
    veh.waypoint_x[1] = 2;
    veh.waypoint_y[1] = 7;
    HOME_CHECK(action_home(0, 1) == 0);
    veh.waypoint_y[1] = 2;
    HOME_CHECK(action_home(0, 1) == 1);  // and restored, it fires again

    // dist <= max_dist is the fifth predicate. max_dist is 0 for a range 0
    // chassis, so a base one step away is too far for the shortcut.
    home_reset();
    g_home_world.base_count = 1;
    base.x = 4;
    base.y = 2;
    base.faction_id_current = 1;
    veh.state = VSTATE_UNK_200;
    veh.order_auto_type = ORDERA_ON_ALERT;
    veh.waypoint_x[1] = 4;
    veh.waypoint_y[1] = 2;
    HOME_CHECK(action_home(0, 1) == 0);
    HOME_CHECK(veh.order == ORDER_SENTRY_BOARD);

    // The shortcut WRITES best_dist, unlike the radius shortcut below. With a
    // range 3 chassis max_dist is 6, so "comfortably within reach" fires and
    // the answer is 0; had best_dist been left at 9999 the answer would be 1
    // with the orders cleared.
    home_reset();
    g_home_world.base_count = 1;
    g_home_world.chassis[HOME_CH_LAND].range = 3;
    base.x = 2;
    base.y = 2;
    base.faction_id_current = 1;
    veh.state = VSTATE_UNK_200;
    veh.order_auto_type = ORDERA_ON_ALERT;
    veh.waypoint_x[1] = 2;
    veh.waypoint_y[1] = 2;
    HOME_CHECK(action_home(0, 1) == 0);
    HOME_CHECK(veh.order == ORDER_SENTRY_BOARD);

    // It also stops the scan: a nearer base after it must not win.
    home_reset();
    g_home_world.base_count = 2;
    g_home_world.chassis[HOME_CH_LAND].range = 2;  // max_dist 4
    g_home_world.bases[0].x = 6;
    g_home_world.bases[0].y = 4;
    g_home_world.bases[0].faction_id_current = 1;
    g_home_world.bases[1].x = 2;
    g_home_world.bases[1].y = 2;
    g_home_world.bases[1].faction_id_current = 1;
    veh.state = VSTATE_UNK_200;
    veh.order_auto_type = ORDERA_ON_ALERT;
    veh.waypoint_x[1] = 6;
    veh.waypoint_y[1] = 4;
    HOME_CHECK(action_home(0, 0) == 1);
    HOME_CHECK(veh.order == ORDER_MOVE_TO);
    HOME_CHECK(veh.waypoint_x[0] == 6);
    HOME_CHECK(veh.waypoint_y[0] == 4);

    // ---- the endurance gate ------------------------------------------------
    // An odd flags value, and flags == 4, are answered only by a chassis that
    // has endurance. The destination is never even looked at.
    home_reset();
    g_home_world.base_count = 1;
    base.x = 6;
    base.y = 4;
    base.faction_id_current = 1;
    HOME_CHECK(action_home(0, 1) == 0);
    HOME_CHECK(veh.order == ORDER_SENTRY_BOARD);
    HOME_CHECK(action_home(0, 4) == 0);
    HOME_CHECK(veh.order == ORDER_SENTRY_BOARD);
    HOME_CHECK(action_home(0, 2) == 1);  // even and not 4: the gate is skipped
    HOME_CHECK(veh.order == ORDER_MOVE_TO);

    // ---- flags == 4 is a reachability question -----------------------------
    // A range 2 chassis at speed 2 reaches 2 * (2 - 0 - 1) + 2 == 4.
    home_reset();
    g_home_world.base_count = 1;
    g_home_world.chassis[HOME_CH_LAND].range = 2;
    base.x = 10;
    base.y = 2;
    base.faction_id_current = 1;
    HOME_CHECK(action_home(0, 4) == 1);  // dist 4, exactly max_dist
    HOME_CHECK(veh.order == ORDER_MOVE_TO);
    HOME_CHECK(veh.waypoint_x[0] == 10);
    home_reset();
    g_home_world.base_count = 1;
    g_home_world.chassis[HOME_CH_LAND].range = 2;
    base.x = 10;
    base.y = 4;
    base.faction_id_current = 1;
    HOME_CHECK(action_home(0, 4) == 0);  // dist 5, one past max_dist
    HOME_CHECK(veh.order == ORDER_SENTRY_BOARD);

    // ---- an odd flags value is a "do I need to go?" question ---------------
    // max_dist 6 from a range 3 chassis. best_dist + 1 <= max_dist - 1 is the
    // "comfortably within reach, nothing to do" test, and both boundaries are
    // checked here: dist 4 gives 5 <= 5 and refuses, dist 5 gives 6 <= 5 and
    // proceeds.
    home_reset();
    g_home_world.base_count = 1;
    g_home_world.chassis[HOME_CH_LAND].range = 3;
    base.x = 10;
    base.y = 2;
    base.faction_id_current = 1;
    HOME_CHECK(action_home(0, 1) == 0);
    HOME_CHECK(veh.order == ORDER_SENTRY_BOARD);
    home_reset();
    g_home_world.base_count = 1;
    g_home_world.chassis[HOME_CH_LAND].range = 3;
    base.x = 10;
    base.y = 4;
    base.faction_id_current = 1;
    HOME_CHECK(action_home(0, 1) == 1);
    HOME_CHECK(veh.order == ORDER_MOVE_TO);
    HOME_CHECK(veh.waypoint_x[0] == 10);
    HOME_CHECK(veh.waypoint_y[0] == 4);

    // ---- a negative flags value is a pure query ----------------------------
    // The comparison is SIGNED. Read unsigned it would fall through and rewrite
    // the orders. -2 is even and not 4, so the endurance gate lets it past.
    home_reset();
    g_home_world.base_count = 1;
    base.x = 6;
    base.y = 4;
    base.faction_id_current = 1;
    HOME_CHECK(action_home(0, -2) == 1);
    HOME_CHECK(veh.order == ORDER_SENTRY_BOARD);
    HOME_CHECK(veh.waypoint_x[0] == 0);
    HOME_CHECK(veh.waypoint_y[0] == 0);
    // -1 is odd, so it must clear the endurance gate first.
    home_reset();
    g_home_world.base_count = 1;
    base.x = 6;
    base.y = 4;
    base.faction_id_current = 1;
    HOME_CHECK(action_home(0, -1) == 0);  // range 0: refused at the gate
    g_home_world.chassis[HOME_CH_LAND].range = 2;
    HOME_CHECK(action_home(0, -1) == 1);
    HOME_CHECK(veh.order == ORDER_SENTRY_BOARD);
    HOME_CHECK(veh.waypoint_x[0] == 0);

    // ---- the out-of-reach tail --------------------------------------------
    // A human-controlled endurance unit whose destination is past max_dist gets
    // the order it was just given taken back, and exactly four state bits are
    // scrubbed. Every other bit of a fully set state survives.
    home_reset();
    g_home_world.base_count = 1;
    g_home_world.chassis[HOME_CH_LAND].range = 2;  // max_dist 4
    base.x = 10;
    base.y = 4;                                    // dist 5
    base.faction_id_current = 1;
    veh.state = 0xFFFFFFFFU;
    veh.order_auto_type = 0;  // ORDERA_ON_ALERT would take the shortcut instead
    HOME_CHECK(action_home(0, 0) == 1);
    HOME_CHECK(veh.waypoint_x[0] == 10);  // the waypoint is still written
    HOME_CHECK(veh.waypoint_y[0] == 4);
    HOME_CHECK(veh.order == ORDER_NONE);
    HOME_CHECK(veh.state == 0xFCFFBDFFU);
    // The AI is not given that courtesy.
    home_reset();
    g_home_world.base_count = 1;
    g_home_world.chassis[HOME_CH_LAND].range = 2;
    g_home_world.factions_status[0] = 0;
    base.x = 10;
    base.y = 4;
    base.faction_id_current = 1;
    veh.state = 0xFFFFFFFFU;
    veh.order_auto_type = 0;
    HOME_CHECK(action_home(0, 0) == 1);
    HOME_CHECK(veh.order == ORDER_MOVE_TO);
    HOME_CHECK(veh.state == 0xFFFFFFFFU);
    // Neither is a chassis without endurance.
    home_reset();
    g_home_world.base_count = 1;
    base.x = 10;
    base.y = 4;
    base.faction_id_current = 1;
    veh.state = 0xFFFFFFFFU;
    veh.order_auto_type = 0;
    HOME_CHECK(action_home(0, 0) == 1);
    HOME_CHECK(veh.order == ORDER_MOVE_TO);
    HOME_CHECK(veh.state == 0xFFFFFFFFU);
    // Nor a destination that IS within reach: dist 4, max_dist 4.
    home_reset();
    g_home_world.base_count = 1;
    g_home_world.chassis[HOME_CH_LAND].range = 2;
    base.x = 10;
    base.y = 2;
    base.faction_id_current = 1;
    veh.state = 0xFFFFFFFFU;
    veh.order_auto_type = 0;
    HOME_CHECK(action_home(0, 0) == 1);
    HOME_CHECK(veh.order == ORDER_MOVE_TO);
    HOME_CHECK(veh.state == 0xFFFFFFFFU);

    // ---- reach arithmetic --------------------------------------------------
    // moves_expended is subtracted, and the result is clamped at 0 rather than
    // going negative: 5 expended out of a speed of 2 leaves 0, not -3.
    home_reset();
    g_home_world.base_count = 1;
    g_home_world.chassis[HOME_CH_LAND].range = 2;
    base.x = 6;
    base.y = 4;                                    // dist 3
    base.faction_id_current = 1;
    HOME_CHECK(action_home(0, 4) == 1);            // max_dist 4
    veh.order = ORDER_SENTRY_BOARD;
    veh.moves_expended = 5;
    HOME_CHECK(action_home(0, 4) == 0);            // max_dist 2 now
    HOME_CHECK(veh.order == ORDER_SENTRY_BOARD);

    // terraforming_turns is subtracted from the chassis range before it is
    // multiplied out: range 3 with 1 turn spent reaches 4, not 6.
    home_reset();
    g_home_world.base_count = 1;
    g_home_world.chassis[HOME_CH_LAND].range = 3;
    base.x = 10;
    base.y = 4;                                    // dist 5
    base.faction_id_current = 1;
    HOME_CHECK(action_home(0, 4) == 1);            // max_dist 6
    veh.order = ORDER_SENTRY_BOARD;
    veh.terraforming_turns = 1;
    HOME_CHECK(action_home(0, 4) == 0);            // max_dist 4
    HOME_CHECK(veh.order == ORDER_SENTRY_BOARD);

    // A range 1 chassis, and ONLY a range 1 chassis, additionally trades its
    // remaining hitpoints for distance: proto_power is 10 for a fission
    // reactor, so it buys 2 * ((10 - 1) / 1) == 18 on top of the 2 it had.
    home_reset();
    g_home_world.base_count = 1;
    g_home_world.chassis[HOME_CH_LAND].range = 1;
    base.x = 10;
    base.y = 4;                                    // dist 5
    base.faction_id_current = 1;
    HOME_CHECK(action_home(0, 4) == 1);            // max_dist 20
    home_reset();
    g_home_world.base_count = 1;
    g_home_world.chassis[HOME_CH_LAND].range = 2;
    base.x = 10;
    base.y = 4;
    base.faction_id_current = 1;
    HOME_CHECK(action_home(0, 4) == 0);            // max_dist 4, no hitpoint term
    HOME_CHECK(veh.order == ORDER_SENTRY_BOARD);

    // The hitpoint term is clamped at 9999 from above and 0 from below, and the
    // -1 before the reactor division is real. A fully destroyed unit (10 damage
    // against 10 hitpoints) gives (0 - 1) / 1 == -1, which SUBTRACTS a move:
    // reach becomes 0 and a base one step away is out of reach. Clamping the
    // hitpoints to 1 instead would give 0 and leave it reachable.
    home_reset();
    g_home_world.base_count = 1;
    g_home_world.chassis[HOME_CH_LAND].range = 1;
    base.x = 4;
    base.y = 2;                                    // dist 1
    base.faction_id_current = 1;
    veh.dmg_incurred = 10;                         // speed() drops to 1
    HOME_CHECK(action_home(0, 4) == 0);
    HOME_CHECK(veh.order == ORDER_SENTRY_BOARD);
    veh.dmg_incurred = 9;                          // one hitpoint left
    HOME_CHECK(action_home(0, 4) == 1);
    HOME_CHECK(veh.order == ORDER_MOVE_TO);

    // The reactor is the divisor, not a constant: a fusion reactor doubles both
    // the hitpoints and the divisor, so the quotient is unchanged at full
    // health but not once the unit is damaged. 27 damage against a fusion
    // reactor's 30 hitpoints leaves 3, and (3 - 1) / 3 is 0 - the whole
    // hitpoint term vanishes and max_dist is 1 rather than 2.
    home_reset();
    g_home_world.base_count = 1;
    g_home_world.chassis[HOME_CH_LAND].range = 1;
    g_home_world.protos[HOME_P_LAND].reactor_id = 3;
    base.x = 6;
    base.y = 2;                                    // dist 2
    base.faction_id_current = 1;
    veh.dmg_incurred = 27;
    HOME_CHECK(action_home(0, 4) == 0);
    HOME_CHECK(veh.order == ORDER_SENTRY_BOARD);
    base.x = 4;                                    // dist 1
    HOME_CHECK(action_home(0, 4) == 1);
    HOME_CHECK(veh.order == ORDER_MOVE_TO);

    // ---- TRIAD_AIR: carriers and air supply convoys ------------------------
    home_reset();
    veh.proto_id = HOME_P_AIR;
    g_home_world.veh_count = 2;
    g_home_world.vehs[1].x = 4;
    g_home_world.vehs[1].y = 2;
    g_home_world.vehs[1].faction_id = 1;
    g_home_world.vehs[1].proto_id = HOME_P_AIR;
    HOME_CHECK(action_home(0, 0) == 0);  // neither a carrier nor a convoy
    HOME_CHECK(veh.order == ORDER_SENTRY_BOARD);
    g_home_world.vehs[1].proto_id = HOME_P_CARRIER;
    HOME_CHECK(action_home(0, 0) == 1);
    HOME_CHECK(veh.order == ORDER_MOVE_TO);
    HOME_CHECK(veh.waypoint_x[0] == 4);
    HOME_CHECK(veh.waypoint_y[0] == 2);
    // The other half of the predicate: an AIR supply convoy also qualifies, a
    // LAND one does not.
    home_reset();
    veh.proto_id = HOME_P_AIR;
    g_home_world.veh_count = 2;
    g_home_world.vehs[1].x = 4;
    g_home_world.vehs[1].y = 2;
    g_home_world.vehs[1].faction_id = 1;
    g_home_world.vehs[1].proto_id = HOME_P_AIRCONVOY;
    HOME_CHECK(action_home(0, 0) == 1);
    HOME_CHECK(veh.waypoint_x[0] == 4);
    home_reset();
    veh.proto_id = HOME_P_AIR;
    g_home_world.veh_count = 2;
    g_home_world.vehs[1].x = 4;
    g_home_world.vehs[1].y = 2;
    g_home_world.vehs[1].faction_id = 1;
    g_home_world.vehs[1].proto_id = HOME_P_LANDCONVOY;
    HOME_CHECK(action_home(0, 0) == 0);
    // Somebody else's carrier is not a home.
    home_reset();
    veh.proto_id = HOME_P_AIR;
    g_home_world.veh_count = 2;
    g_home_world.vehs[1].x = 4;
    g_home_world.vehs[1].y = 2;
    g_home_world.vehs[1].faction_id = 2;
    g_home_world.vehs[1].proto_id = HOME_P_CARRIER;
    HOME_CHECK(action_home(0, 0) == 0);
    // And the whole scan belongs to the air triad: a land unit ignores it.
    home_reset();
    g_home_world.veh_count = 2;
    g_home_world.vehs[1].x = 4;
    g_home_world.vehs[1].y = 2;
    g_home_world.vehs[1].faction_id = 1;
    g_home_world.vehs[1].proto_id = HOME_P_CARRIER;
    HOME_CHECK(action_home(0, 0) == 0);

    // The subject skips itself. Unit 0 is a carrier standing on its own tile,
    // and if it counted the answer would be "you are already home".
    home_reset();
    veh.proto_id = HOME_P_CARRIER;
    g_home_world.veh_count = 1;
    HOME_CHECK(action_home(0, 0) == 0);
    HOME_CHECK(veh.order == ORDER_SENTRY_BOARD);

    // The unit scan honours *VehCurrentCount, and keeps the strict minimum
    // against whatever the base loop already found.
    home_reset();
    veh.proto_id = HOME_P_AIR;
    g_home_world.veh_count = 2;
    g_home_world.vehs[1].x = 6;
    g_home_world.vehs[1].y = 2;
    g_home_world.vehs[1].faction_id = 1;
    g_home_world.vehs[1].proto_id = HOME_P_CARRIER;
    g_home_world.vehs[2].x = 2;
    g_home_world.vehs[2].y = 2;
    g_home_world.vehs[2].faction_id = 1;
    g_home_world.vehs[2].proto_id = HOME_P_CARRIER;
    HOME_CHECK(action_home(0, 0) == 1);
    HOME_CHECK(veh.order == ORDER_MOVE_TO);  // unit 2 is past the count
    HOME_CHECK(veh.waypoint_x[0] == 6);
    // A base at the same distance as the carrier wins, because the unit loop's
    // comparison is strict too.
    home_reset();
    veh.proto_id = HOME_P_AIR;
    g_home_world.base_count = 1;
    base.x = 2;
    base.y = 4;                                    // dist 1
    base.faction_id_current = 1;
    g_home_world.veh_count = 2;
    g_home_world.vehs[1].x = 4;
    g_home_world.vehs[1].y = 2;                    // also dist 1
    g_home_world.vehs[1].faction_id = 1;
    g_home_world.vehs[1].proto_id = HOME_P_CARRIER;
    HOME_CHECK(action_home(0, 0) == 1);
    HOME_CHECK(veh.waypoint_x[0] == 2);
    HOME_CHECK(veh.waypoint_y[0] == 4);
    // A nearer carrier does displace it.
    g_home_world.vehs[1].x = 2;
    g_home_world.vehs[1].y = 2;                    // dist 0
    veh.order = ORDER_SENTRY_BOARD;
    HOME_CHECK(action_home(0, 0) == 1);
    HOME_CHECK(veh.order == ORDER_NONE);           // standing on it

    // ---- the AI's distance bias on a carrier -------------------------------
    // Under an odd flags value the AI adds the carrier's own speed in road
    // moves to the distance. Both runs are identical apart from who is driving:
    // the human sees dist 2 and answers "comfortably within reach, nothing to
    // do", the AI sees 2 + 2 == 4 and issues the move.
    home_reset();
    veh.proto_id = HOME_P_AIR;
    g_home_world.chassis[HOME_CH_AIR].range = 2;   // max_dist 4
    g_home_world.veh_count = 2;
    g_home_world.vehs[1].x = 6;
    g_home_world.vehs[1].y = 2;                    // dist 2
    g_home_world.vehs[1].faction_id = 1;
    g_home_world.vehs[1].proto_id = HOME_P_CARRIER;
    HOME_CHECK(action_home(0, 1) == 0);
    HOME_CHECK(veh.order == ORDER_SENTRY_BOARD);
    g_home_world.factions_status[0] = 0;           // the AI now
    HOME_CHECK(action_home(0, 1) == 1);
    HOME_CHECK(veh.order == ORDER_MOVE_TO);
    HOME_CHECK(veh.waypoint_x[0] == 6);
    // With an even flags value that is not 4 the whole block is skipped, so the
    // AI sees the unbiased 2 as well.
    home_reset();
    veh.proto_id = HOME_P_AIR;
    g_home_world.chassis[HOME_CH_AIR].range = 2;
    g_home_world.factions_status[0] = 0;
    g_home_world.veh_count = 2;
    g_home_world.vehs[1].x = 6;
    g_home_world.vehs[1].y = 2;
    g_home_world.vehs[1].faction_id = 1;
    g_home_world.vehs[1].proto_id = HOME_P_CARRIER;
    HOME_CHECK(action_home(0, 2) == 1);
    HOME_CHECK(veh.order == ORDER_MOVE_TO);
    // flags == 4 enters the same block: 2 + 2 == 4 is still within max_dist 4,
    // but the biased distance is what the tail compares.
    home_reset();
    veh.proto_id = HOME_P_AIR;
    g_home_world.chassis[HOME_CH_AIR].range = 2;
    g_home_world.factions_status[0] = 0;
    g_home_world.veh_count = 2;
    g_home_world.vehs[1].x = 8;
    g_home_world.vehs[1].y = 2;                    // dist 3, biased to 5
    g_home_world.vehs[1].faction_id = 1;
    g_home_world.vehs[1].proto_id = HOME_P_CARRIER;
    HOME_CHECK(action_home(0, 4) == 0);            // 5 > 4
    HOME_CHECK(veh.order == ORDER_SENTRY_BOARD);
    g_home_world.factions_status[0] = 0x02;        // the human sees 3
    HOME_CHECK(action_home(0, 4) == 1);

    // ---- an air supply convoy must be reachable this turn ------------------
    // A range 2 air chassis with a base speed of 6 moves 8, so max_dist is 16
    // and the convoy at dist 4 is comfortably inside it. What rejects it is the
    // separate raw-distance test against what is LEFT of this turn.
    home_reset();
    veh.proto_id = HOME_P_AIR;
    g_home_world.chassis[HOME_CH_AIR].range = 2;
    g_home_world.chassis[HOME_CH_AIR].speed = 6;
    g_home_world.veh_count = 2;
    g_home_world.vehs[1].x = 10;
    g_home_world.vehs[1].y = 2;                    // dist 4
    g_home_world.vehs[1].faction_id = 1;
    g_home_world.vehs[1].proto_id = HOME_P_AIRCONVOY;
    HOME_CHECK(action_home(0, 4) == 1);
    HOME_CHECK(veh.order == ORDER_MOVE_TO);
    HOME_CHECK(veh.waypoint_x[0] == 10);
    veh.order = ORDER_SENTRY_BOARD;
    veh.moves_expended = 5;                        // 3 road moves left, dist 4
    HOME_CHECK(action_home(0, 4) == 0);
    HOME_CHECK(veh.order == ORDER_SENTRY_BOARD);
    // The test belongs to convoys alone: a carrier at the same spot with the
    // same moves spent is still accepted.
    home_reset();
    veh.proto_id = HOME_P_AIR;
    g_home_world.chassis[HOME_CH_AIR].range = 2;
    g_home_world.chassis[HOME_CH_AIR].speed = 6;
    g_home_world.veh_count = 2;
    g_home_world.vehs[1].x = 10;
    g_home_world.vehs[1].y = 2;
    g_home_world.vehs[1].faction_id = 1;
    g_home_world.vehs[1].proto_id = HOME_P_CARRIER;
    veh.moves_expended = 5;
    HOME_CHECK(action_home(0, 4) == 1);
    HOME_CHECK(veh.order == ORDER_MOVE_TO);
    // And it is skipped entirely when the flags do not ask for it.
    home_reset();
    veh.proto_id = HOME_P_AIR;
    g_home_world.chassis[HOME_CH_AIR].speed = 6;
    g_home_world.veh_count = 2;
    g_home_world.vehs[1].x = 10;
    g_home_world.vehs[1].y = 2;
    g_home_world.vehs[1].faction_id = 1;
    g_home_world.vehs[1].proto_id = HOME_P_AIRCONVOY;
    veh.moves_expended = 5;
    HOME_CHECK(action_home(0, 0) == 1);
    HOME_CHECK(veh.waypoint_x[0] == 10);

    // ---- the airbase radius scan -------------------------------------------
    // RadiusOffsetX/Y[2] is (+2, 0), so tile (4,2) is the third step.
    home_reset();
    g_home_world.tiles[home_tile(4, 2)].bit = BIT_AIRBASE;
    HOME_CHECK(action_home(0, 0) == 1);
    HOME_CHECK(veh.order == ORDER_MOVE_TO);
    HOME_CHECK(veh.waypoint_x[0] == 4);
    HOME_CHECK(veh.waypoint_y[0] == 2);
    // Without the airbase bit the tile is just a tile.
    home_reset();
    g_home_world.tiles[home_tile(4, 2)].bit = BIT_VEH_IN_TILE;
    HOME_CHECK(action_home(0, 0) == 0);
    HOME_CHECK(veh.order == ORDER_SENTRY_BOARD);

    // Somebody else's unit parked on the airbase needs a pact. The occupier is
    // the low nibble of val2, and the check is skipped entirely once the nibble
    // reaches MaxPlayerNum - 0xF is how an unoccupied tile is spelled.
    home_reset();
    g_home_world.tiles[home_tile(4, 2)].bit = BIT_AIRBASE | BIT_VEH_IN_TILE;
    g_home_world.tiles[home_tile(4, 2)].val2 = 7;
    HOME_CHECK(action_home(0, 0) == 0);
    g_home_world.players[1].diplo_treaties[7] = DTREATY_PACT;
    HOME_CHECK(action_home(0, 0) == 1);
    HOME_CHECK(veh.waypoint_x[0] == 4);
    home_reset();
    g_home_world.tiles[home_tile(4, 2)].bit = BIT_AIRBASE | BIT_VEH_IN_TILE;
    g_home_world.tiles[home_tile(4, 2)].val2 = 8;  // out of the faction range
    HOME_CHECK(action_home(0, 0) == 1);
    home_reset();
    g_home_world.tiles[home_tile(4, 2)].bit = BIT_AIRBASE | BIT_VEH_IN_TILE;
    g_home_world.tiles[home_tile(4, 2)].val2 = 0xF;  // unoccupied
    HOME_CHECK(action_home(0, 0) == 1);
    home_reset();
    g_home_world.tiles[home_tile(4, 2)].bit = BIT_AIRBASE | BIT_VEH_IN_TILE;
    g_home_world.tiles[home_tile(4, 2)].val2 = 1;  // our own unit
    HOME_CHECK(action_home(0, 0) == 1);
    // The nibble is a nibble: the high half of val2 is the site, not an owner.
    home_reset();
    g_home_world.tiles[home_tile(4, 2)].bit = BIT_AIRBASE | BIT_VEH_IN_TILE;
    g_home_world.tiles[home_tile(4, 2)].val2 = 0x71;  // site 7, owner 1
    HOME_CHECK(action_home(0, 0) == 1);
    // ... and only when there is a unit in the tile at all.
    home_reset();
    g_home_world.tiles[home_tile(4, 2)].bit = BIT_AIRBASE;
    g_home_world.tiles[home_tile(4, 2)].val2 = 7;
    HOME_CHECK(action_home(0, 0) == 1);

    // The AI additionally demands the airbase sit in its own territory AND that
    // the tile's owner nibble name it. They are separate reads of separate
    // fields, so neither alone is enough.
    home_reset();
    g_home_world.factions_status[0] = 0;
    g_home_world.tiles[home_tile(4, 2)].bit = BIT_AIRBASE;
    HOME_CHECK(action_home(0, 0) == 0);            // territory unclaimed
    g_home_world.tiles[home_tile(4, 2)].territory = 2;
    HOME_CHECK(action_home(0, 0) == 0);            // and claimed by someone else
    g_home_world.tiles[home_tile(4, 2)].territory = 1;
    HOME_CHECK(action_home(0, 0) == 0);            // ours, but the owner is not
    g_home_world.tiles[home_tile(4, 2)].val2 = 1;
    HOME_CHECK(action_home(0, 0) == 1);
    HOME_CHECK(veh.waypoint_x[0] == 4);
    HOME_CHECK(veh.waypoint_y[0] == 2);
    // A human is held to neither.
    home_reset();
    g_home_world.tiles[home_tile(4, 2)].bit = BIT_AIRBASE;
    g_home_world.tiles[home_tile(4, 2)].territory = 2;
    HOME_CHECK(action_home(0, 0) == 1);

    // owner_at() reports the raw nibble, and a nibble at or past MaxPlayerNum
    // is remapped to -1 before the comparison. The remap is only observable
    // from a faction id of 8, where the raw nibble would otherwise match.
    home_reset();
    veh.faction_id = 8;
    g_home_world.tiles[home_tile(4, 2)].bit = BIT_AIRBASE;
    g_home_world.tiles[home_tile(4, 2)].territory = 8;
    g_home_world.tiles[home_tile(4, 2)].val2 = 8;
    HOME_CHECK(action_home(0, 0) == 0);
    HOME_CHECK(veh.order == ORDER_SENTRY_BOARD);

    // ---- the radius width -------------------------------------------------
    // The scan is RadiusRange[best_dist] wide, saturating at index 8. A base at
    // dist 1 narrows it to the 9 innermost tiles, and the airbase at step 21 -
    // offset (+4, 0) - falls outside; with no base at all, best_dist is 9999,
    // the index saturates and the same airbase is found.
    home_reset();
    g_home_world.base_count = 1;
    base.x = 2;
    base.y = 4;                                    // dist 1
    base.faction_id_current = 1;
    g_home_world.tiles[home_tile(6, 2)].bit = BIT_AIRBASE;
    HOME_CHECK(action_home(0, 0) == 1);
    HOME_CHECK(veh.waypoint_x[0] == 2);
    HOME_CHECK(veh.waypoint_y[0] == 4);
    home_reset();
    g_home_world.tiles[home_tile(6, 2)].bit = BIT_AIRBASE;
    HOME_CHECK(action_home(0, 0) == 1);
    HOME_CHECK(veh.waypoint_x[0] == 6);
    HOME_CHECK(veh.waypoint_y[0] == 2);

    // The radius scan keeps the strict minimum, in scan order. Steps 2 and 4
    // are (+2,0) and (0,+2), both one unit away; the earlier one wins.
    home_reset();
    g_home_world.tiles[home_tile(4, 2)].bit = BIT_AIRBASE;
    g_home_world.tiles[home_tile(2, 4)].bit = BIT_AIRBASE;
    HOME_CHECK(action_home(0, 0) == 1);
    HOME_CHECK(veh.waypoint_x[0] == 4);
    HOME_CHECK(veh.waypoint_y[0] == 2);
    // A strictly nearer one later in the scan does displace it: step 21 is
    // (+4,0) at distance 2, step 0 is the unit's own tile at distance 0.
    home_reset();
    g_home_world.tiles[home_tile(6, 2)].bit = BIT_AIRBASE;
    g_home_world.tiles[home_tile(2, 2)].bit = BIT_AIRBASE;
    HOME_CHECK(action_home(0, 0) == 1);
    HOME_CHECK(veh.order == ORDER_NONE);           // standing on it

    // ---- the radius scan's on-alert shortcut -------------------------------
    // It bypasses the endurance gate, exactly as the base loop's does: a range
    // 0 chassis and an odd flags value would otherwise be refused.
    home_reset();
    g_home_world.tiles[home_tile(2, 2)].bit = BIT_AIRBASE;
    veh.state = VSTATE_UNK_200;
    veh.order_auto_type = ORDERA_ON_ALERT;
    veh.waypoint_x[1] = 2;
    veh.waypoint_y[1] = 2;
    HOME_CHECK(action_home(0, 1) == 1);
    HOME_CHECK(veh.order == ORDER_NONE);
    // Each predicate in turn, restored afterwards.
    veh.order = ORDER_SENTRY_BOARD;
    veh.order_auto_type = 0;
    HOME_CHECK(action_home(0, 1) == 0);
    veh.order_auto_type = ORDERA_ON_ALERT;
    veh.state = 0;
    HOME_CHECK(action_home(0, 1) == 0);
    veh.state = VSTATE_UNK_200;
    veh.waypoint_x[1] = 7;
    HOME_CHECK(action_home(0, 1) == 0);
    veh.waypoint_x[1] = 2;
    veh.waypoint_y[1] = 7;
    HOME_CHECK(action_home(0, 1) == 0);
    veh.waypoint_y[1] = 2;
    // A damaged unit does not get the shortcut - this one predicate has no
    // counterpart in the base loop.
    veh.dmg_incurred = 1;
    HOME_CHECK(action_home(0, 1) == 0);
    veh.dmg_incurred = 0;
    HOME_CHECK(action_home(0, 1) == 1);
    HOME_CHECK(veh.order == ORDER_NONE);

    // Unlike the base loop's shortcut, this one does NOT write best_dist. With
    // flags == 4 the tail asks best_dist > max_dist, and 9999 > 0 refuses;
    // had the distance been recorded, 0 > 0 would have let it through.
    home_reset();
    g_home_world.tiles[home_tile(2, 2)].bit = BIT_AIRBASE;
    veh.state = VSTATE_UNK_200;
    veh.order_auto_type = ORDERA_ON_ALERT;
    veh.waypoint_x[1] = 2;
    veh.waypoint_y[1] = 2;
    HOME_CHECK(action_home(0, 4) == 0);
    HOME_CHECK(veh.order == ORDER_SENTRY_BOARD);

    // The same, seen from the other side: with an endurance chassis the
    // untouched 9999 drives the out-of-reach tail, so the move order the
    // function just wrote is taken straight back off again.
    home_reset();
    g_home_world.chassis[HOME_CH_LAND].range = 2;  // max_dist 4
    g_home_world.tiles[home_tile(4, 2)].bit = BIT_AIRBASE;
    veh.state = VSTATE_UNK_200;
    veh.order_auto_type = ORDERA_ON_ALERT;
    veh.waypoint_x[1] = 4;
    veh.waypoint_y[1] = 2;                         // dist 1 <= max_dist 4
    HOME_CHECK(action_home(0, 1) == 1);
    HOME_CHECK(veh.waypoint_x[0] == 4);
    HOME_CHECK(veh.waypoint_y[0] == 2);
    HOME_CHECK(veh.order == ORDER_NONE);
    HOME_CHECK(veh.state == 0);                    // VSTATE_UNK_200 was scrubbed

    // ---- the subject is not always unit 0 ---------------------------------
    // The unit scan starts at unit 0 and excludes the subject by identity, not
    // by position: here the carrier IS unit 0 and the subject is unit 1.
    home_reset();
    g_home_world.veh_count = 2;
    g_home_world.vehs[0].x = 4;
    g_home_world.vehs[0].y = 2;
    g_home_world.vehs[0].faction_id = 1;
    g_home_world.vehs[0].proto_id = HOME_P_CARRIER;
    g_home_world.vehs[0].order = ORDER_NONE;
    g_home_world.vehs[1].x = 2;
    g_home_world.vehs[1].y = 2;
    g_home_world.vehs[1].faction_id = 1;
    g_home_world.vehs[1].proto_id = HOME_P_AIR;
    g_home_world.vehs[1].order = ORDER_SENTRY_BOARD;
    HOME_CHECK(action_home(1, 0) == 1);
    HOME_CHECK(g_home_world.vehs[1].order == ORDER_MOVE_TO);
    HOME_CHECK(g_home_world.vehs[1].waypoint_x[0] == 4);
    HOME_CHECK(g_home_world.vehs[1].waypoint_y[0] == 2);
    HOME_CHECK(g_home_world.vehs[0].order == ORDER_NONE);  // the carrier is untouched
    // ... and the subject still does not count as its own carrier.
    home_reset();
    g_home_world.veh_count = 2;
    g_home_world.vehs[0].x = 6;
    g_home_world.vehs[0].y = 2;
    g_home_world.vehs[0].faction_id = 1;
    g_home_world.vehs[0].proto_id = HOME_P_AIR;
    g_home_world.vehs[1].x = 2;
    g_home_world.vehs[1].y = 2;
    g_home_world.vehs[1].faction_id = 1;
    g_home_world.vehs[1].proto_id = HOME_P_CARRIER;
    g_home_world.vehs[1].order = ORDER_SENTRY_BOARD;
    HOME_CHECK(action_home(1, 0) == 0);
    HOME_CHECK(g_home_world.vehs[1].order == ORDER_SENTRY_BOARD);

    // ---- the convoy reachability test, at its boundaries -------------------
    // A range 2 air chassis with base speed 6 and a fission reactor moves 8
    // road moves a turn, so these three runs differ only in how much of the
    // turn is already spent. The convoy sits 4 units away in all of them.
    // 5 moves left, distance 4: reachable.
    home_reset();
    veh.proto_id = HOME_P_AIR;
    g_home_world.chassis[HOME_CH_AIR].range = 2;
    g_home_world.chassis[HOME_CH_AIR].speed = 6;
    g_home_world.veh_count = 2;
    g_home_world.vehs[1].x = 10;
    g_home_world.vehs[1].y = 2;
    g_home_world.vehs[1].faction_id = 1;
    g_home_world.vehs[1].proto_id = HOME_P_AIRCONVOY;
    veh.moves_expended = 3;
    HOME_CHECK(action_home(0, 4) == 1);
    HOME_CHECK(veh.waypoint_x[0] == 10);
    // Exactly 4 moves left against distance 4 is still reachable: the test is
    // strictly greater-than, so equality passes.
    home_reset();
    veh.proto_id = HOME_P_AIR;
    g_home_world.chassis[HOME_CH_AIR].range = 2;
    g_home_world.chassis[HOME_CH_AIR].speed = 6;
    g_home_world.veh_count = 2;
    g_home_world.vehs[1].x = 10;
    g_home_world.vehs[1].y = 2;
    g_home_world.vehs[1].faction_id = 1;
    g_home_world.vehs[1].proto_id = HOME_P_AIRCONVOY;
    veh.moves_expended = 4;
    HOME_CHECK(action_home(0, 4) == 1);
    HOME_CHECK(veh.waypoint_x[0] == 10);
    // A turn that is over leaves 0 road moves, not 1, so even the adjacent
    // convoy is out of reach.
    home_reset();
    veh.proto_id = HOME_P_AIR;
    g_home_world.chassis[HOME_CH_AIR].range = 2;
    g_home_world.chassis[HOME_CH_AIR].speed = 6;
    g_home_world.veh_count = 2;
    g_home_world.vehs[1].x = 4;
    g_home_world.vehs[1].y = 2;                    // dist 1
    g_home_world.vehs[1].faction_id = 1;
    g_home_world.vehs[1].proto_id = HOME_P_AIRCONVOY;
    veh.moves_expended = 10;                       // more than a whole turn
    HOME_CHECK(action_home(0, 4) == 0);
    HOME_CHECK(veh.order == ORDER_SENTRY_BOARD);

    // ---- the radius width is read, not assumed -----------------------------
    // A base at distance 1 narrows the scan to RadiusRange[1] == 9 tiles, and
    // step 21 - offset (+4,0) - is outside it. The unit is on alert for exactly
    // that tile, so a scan that did not narrow would take the shortcut there
    // and answer with a different destination.
    home_reset();
    g_home_world.chassis[HOME_CH_LAND].range = 2;  // max_dist 4
    g_home_world.base_count = 1;
    base.x = 2;
    base.y = 4;                                    // dist 1
    base.faction_id_current = 1;
    g_home_world.tiles[home_tile(6, 2)].bit = BIT_AIRBASE;
    veh.state = VSTATE_UNK_200;
    veh.order_auto_type = ORDERA_ON_ALERT;
    veh.waypoint_x[1] = 6;
    veh.waypoint_y[1] = 2;
    HOME_CHECK(action_home(0, 0) == 1);
    HOME_CHECK(veh.waypoint_x[0] == 2);
    HOME_CHECK(veh.waypoint_y[0] == 4);
    // The narrowest scan of all is one tile wide, and it really is one tile:
    // step 1 - offset (+1,-1) - is not walked.
    home_reset();
    g_home_world.chassis[HOME_CH_LAND].range = 2;
    g_home_world.base_count = 1;
    base.x = 2;
    base.y = 2;                                    // dist 0 -> RadiusRange[0] == 1
    base.faction_id_current = 1;
    g_home_world.tiles[home_tile(3, 1)].bit = BIT_AIRBASE;
    veh.state = VSTATE_UNK_200;
    veh.order_auto_type = ORDERA_ON_ALERT;
    veh.waypoint_x[1] = 3;
    veh.waypoint_y[1] = 1;
    HOME_CHECK(action_home(0, 0) == 1);
    HOME_CHECK(veh.order == ORDER_NONE);           // standing on the base

    // ---- the radius distance is halved -------------------------------------
    // A chassis of speed 1 with one road move already spent reaches exactly 1.
    // The airbase on step 2 is (2 + 0) >> 1 == 1 away and the unit is on alert
    // for it, so the shortcut fires and the earlier, nearer airbase on step 0
    // is discarded. Left unhalved the distance would be 2, the shortcut would
    // not fire, and step 0 would stand.
    home_reset();
    g_home_world.chassis[HOME_CH_LAND].speed = 1;
    g_home_world.chassis[HOME_CH_LAND].range = 2;
    veh.moves_expended = 1;                        // max_dist 1
    g_home_world.tiles[home_tile(2, 2)].bit = BIT_AIRBASE;
    g_home_world.tiles[home_tile(4, 2)].bit = BIT_AIRBASE;
    veh.state = VSTATE_UNK_200;
    veh.order_auto_type = ORDERA_ON_ALERT;
    veh.waypoint_x[1] = 4;
    veh.waypoint_y[1] = 2;
    HOME_CHECK(action_home(0, 0) == 1);
    HOME_CHECK(veh.order == ORDER_MOVE_TO);
    HOME_CHECK(veh.waypoint_x[0] == 4);
    HOME_CHECK(veh.waypoint_y[0] == 2);

    // The shortcut also stops the scan: step 0 is the unit's own tile, so an
    // airbase there would win on distance if the scan had continued.
    home_reset();
    g_home_world.chassis[HOME_CH_LAND].range = 2;
    g_home_world.tiles[home_tile(2, 2)].bit = BIT_AIRBASE;
    g_home_world.tiles[home_tile(4, 2)].bit = BIT_AIRBASE;
    veh.state = VSTATE_UNK_200;
    veh.order_auto_type = ORDERA_ON_ALERT;
    veh.waypoint_x[1] = 2;
    veh.waypoint_y[1] = 2;
    HOME_CHECK(action_home(0, 0) == 1);
    HOME_CHECK(veh.order == ORDER_NONE);           // step 0 fires it immediately
}

#undef HOME_CHECK


/*
 * A self-contained world for crop_yield / mine_yield / energy_yield.
 *
 * The three share a skeleton but not a helper - the original inlines the whole
 * thing three times - so they share this fixture instead. Every global they and
 * their three callees (bonus_at, bitmask, has_tech) reach is a pointer, so
 * ScopedSeam gives the world local storage and no fixed address is touched.
 *
 * Three things about the fixture are load-bearing:
 *
 *  - `Bases` is seamed to &bases[1], NOT &bases[0]. Four sites in these bodies
 *    index Bases with an unguarded base_id, and every caller in the image
 *    (StatusWin::draw_status, enemy_move, Console::terraform, base_terraform)
 *    passes -1. The original reads one Base below the array there; bases[0] is
 *    that entry, so the fixture can assert what -1 actually does instead of
 *    crashing.
 *  - ResourceInfo is filled so that entry i field f holds 3*i + f + 1. Every one
 *    of the 27 (row, column) pairs is therefore a different number, and any
 *    wrong index or wrong resource column shows up as a wrong total rather than
 *    as a coincidence.
 *  - MapRandSeed is 0, which makes bonus_at() return 0 for any tile without
 *    BIT_RSC_BONUS. With the bit set the pseudo-random path is skipped too, and
 *    the answer is decided by BIT_NUTRIENT_RSC / BIT_MINERAL_RSC /
 *    BIT_ENERGY_RSC alone - so a bonus is exact rather than positional.
 */
struct YieldWorld {
    Map tiles[64];
    Base bases[6];
    PlayerData players_data[9];
    Player players[9];
    RulesResourceinfo resource_info[MaxResourceInfoNum];
    RulesBasic rules;
    BaseSecretProject projects;
    RulesTechnology technology[MaxTechnologyNum];
    uint8_t tech_achieved[MaxTechnologyNum];
    Map *tiles_ptr;
    Base *base_current;
    uint32_t longitude;
    int lon_bounds;
    int lat_bounds;
    BOOL is_flat;
    uint32_t map_rand_seed;
    uint32_t game_rules;
    BOOL expansion;
    int dust_cloud;
    int restricted;
    int base_square_energy;
    int governor;
    int energy_event;
    int energy_event_selector;
};

YieldWorld g_yield_world;

// Technology ids the fixture hands to the four Rules gates, one each so a gate
// reading the wrong Rules field is visible.
const int YTECH_THREE_NUTRIENTS = 1;
const int YTECH_THREE_MINERALS = 2;
const int YTECH_THREE_ENERGY = 3;
const int YTECH_MINING_PLATFORM = 4;

// The subject faction in every case below. has_tech() refuses faction 0
// outright, so 1 is the smallest usable id.
const int YFACTION = 1;

// Land is spelled as the shore-line altitude; anything below it is ocean.
const uint8_t YALT_LAND = 0x60;      // alt 3
const uint8_t YALT_SHELF = 0x40;     // alt 2, ocean shelf
const uint8_t YALT_DEEP = 0x20;      // alt 1, ocean

// The map is 16 wide, so a tile index is (x >> 1) + y * MapLongitude.
Map &ytile(int x, int y) { return g_yield_world.tiles[(x >> 1) + y * 8]; }

void yield_give_fac(Base &base, uint32_t facility) {
    int offset;
    int mask;
    bitmask(facility, &offset, &mask);
    base.facilities_built[offset] |= (uint8_t)mask;
}

void yield_reset() {
    std::memset(&g_yield_world, 0, sizeof(g_yield_world));
    std::memset(&g_yield_world.projects, 0xFF, sizeof(g_yield_world.projects));
    g_yield_world.tiles_ptr = g_yield_world.tiles;
    g_yield_world.base_current = &g_yield_world.bases[1];  // == Bases[0]
    g_yield_world.longitude = 8;
    g_yield_world.lon_bounds = 16;
    g_yield_world.lat_bounds = 8;
    g_yield_world.is_flat = 1;
    g_yield_world.map_rand_seed = 0;
    g_yield_world.governor = -1;  // never the subject faction unless a case says so
    for (int k = 0; k < 64; k++) {
        g_yield_world.tiles[k].climate = YALT_LAND;
    }
    for (int i = 0; i < MaxResourceInfoNum; i++) {
        g_yield_world.resource_info[i].nutrients = 3 * i + 1;
        g_yield_world.resource_info[i].minerals = 3 * i + 2;
        g_yield_world.resource_info[i].energy = 3 * i + 3;
    }
    g_yield_world.rules.tech_three_nutrients_sqr = YTECH_THREE_NUTRIENTS;
    g_yield_world.rules.tech_three_minerals_sqr = YTECH_THREE_MINERALS;
    g_yield_world.rules.tech_three_energy_sqr = YTECH_THREE_ENERGY;
    g_yield_world.rules.tech_mining_platform_bonus = YTECH_MINING_PLATFORM;
    g_yield_world.rules.tgl_nutrient_effect_with_mine = -1;
    g_yield_world.rules.limit_mineral_mine_sans_road = 2;
    // The subject faction holds the three "three per square" technologies by
    // default, so the clip stays out of the way except where a case revokes it.
    g_yield_world.tech_achieved[YTECH_THREE_NUTRIENTS] = 1 << YFACTION;
    g_yield_world.tech_achieved[YTECH_THREE_MINERALS] = 1 << YFACTION;
    g_yield_world.tech_achieved[YTECH_THREE_ENERGY] = 1 << YFACTION;
    g_yield_world.players_data[YFACTION].tech_fungus_nutrient = 4;
    g_yield_world.players_data[YFACTION].tech_fungus_mineral = 5;
    g_yield_world.players_data[YFACTION].tech_fungus_energy = 6;
    g_yield_world.players_data[YFACTION].current_num_bases = 3;
}

// Mark a tile as carrying a base owned by `owner`.
void yield_put_base_in_tile(Map &tile, int owner) {
    tile.bit |= BIT_BASE_IN_TILE;
    tile.val2 = (uint8_t)((tile.val2 & 0xF0) | (owner & 0xF));
}

// Resource bonus selection, exact rather than positional; see the struct note.
const uint32_t YBONUS_NUTRIENT = BIT_RSC_BONUS | BIT_NUTRIENT_RSC;
const uint32_t YBONUS_MINERAL = BIT_RSC_BONUS | BIT_NUTRIENT_RSC | BIT_MINERAL_RSC;
const uint32_t YBONUS_ENERGY = BIT_RSC_BONUS | BIT_NUTRIENT_RSC | BIT_ENERGY_RSC;

#define YCHECK(cond)                                                          \
    do {                                                                      \
        const bool yield_ok = (cond);                                         \
        if (!yield_ok) {                                                      \
            std::fprintf(stderr, "terrain_yield: line %d: %s\n", __LINE__,    \
                         #cond);                                              \
        }                                                                     \
        expect(yield_ok);                                                     \
    } while (0)

class YieldSeams {
 public:
    YieldSeams()
        : tiles_(&MapTiles, &g_yield_world.tiles_ptr),
          longitude_(&MapLongitude, &g_yield_world.longitude),
          lon_(&MapLongitudeBounds, &g_yield_world.lon_bounds),
          lat_(&MapLatitudeBounds, &g_yield_world.lat_bounds),
          flat_(&MapIsFlat, &g_yield_world.is_flat),
          seed_(&MapRandSeed, &g_yield_world.map_rand_seed),
          rules_gate_(&GameRules, &g_yield_world.game_rules),
          bases_(&Bases, &g_yield_world.bases[1]),
          base_current_(&BaseCurrent, &g_yield_world.base_current),
          projects_(&SecretProject, &g_yield_world.projects),
          resource_(&ResourceInfo, g_yield_world.resource_info),
          rules_(&Rules, &g_yield_world.rules),
          players_data_(&PlayersData, g_yield_world.players_data),
          players_(&Players, g_yield_world.players),
          technology_(&Technology, g_yield_world.technology),
          achieved_(&GameTechAchieved, g_yield_world.tech_achieved),
          expansion_(&ExpansionEnabled, &g_yield_world.expansion),
          dust_(&DustCloudDuration, &g_yield_world.dust_cloud),
          restricted_(&TileYieldRestricted, &g_yield_world.restricted),
          base_energy_(&BaseSquareEnergy, &g_yield_world.base_square_energy),
          governor_(&GovernorFaction, &g_yield_world.governor),
          energy_event_(&GlobalEnergyEventState, &g_yield_world.energy_event),
          selector_(&UnkGlobal0093A934, &g_yield_world.energy_event_selector) { }

 private:
    ScopedSeam<Map *> tiles_;
    ScopedSeam<uint32_t> longitude_;
    ScopedSeam<int> lon_;
    ScopedSeam<int> lat_;
    ScopedSeam<BOOL> flat_;
    ScopedSeam<uint32_t> seed_;
    ScopedSeam<uint32_t> rules_gate_;
    ScopedSeam<Base> bases_;
    ScopedSeam<Base *> base_current_;
    ScopedSeam<BaseSecretProject> projects_;
    ScopedSeam<RulesResourceinfo> resource_;
    ScopedSeam<RulesBasic> rules_;
    ScopedSeam<PlayerData> players_data_;
    ScopedSeam<Player> players_;
    ScopedSeam<RulesTechnology> technology_;
    ScopedSeam<uint8_t> achieved_;
    ScopedSeam<BOOL> expansion_;
    ScopedSeam<int> dust_;
    ScopedSeam<int> restricted_;
    ScopedSeam<int> base_energy_;
    ScopedSeam<int> governor_;
    ScopedSeam<int> energy_event_;
    ScopedSeam<int> selector_;
};

void test_crop_yield() {
    YieldSeams seams;
    const int X = 2;
    const int Y = 2;

    // ---- plain land: the rainfall level, and the rockiness that erases it ---
    yield_reset();
    YCHECK(crop_yield(YFACTION, 0, X, Y, 0) == 0);   // arid
    ytile(X, Y).climate = YALT_LAND | 0x08;          // moist
    YCHECK(crop_yield(YFACTION, 0, X, Y, 0) == 1);
    ytile(X, Y).climate = YALT_LAND | 0x10;          // rainy
    YCHECK(crop_yield(YFACTION, 0, X, Y, 0) == 2);
    ytile(X, Y).val3 = 0x40;                         // rolling: still counted
    YCHECK(crop_yield(YFACTION, 0, X, Y, 0) == 2);
    ytile(X, Y).val3 = 0x80;                         // rocky: erased
    YCHECK(crop_yield(YFACTION, 0, X, Y, 0) == 0);

    // The tile really is read from (x >> 1) + y * MapLongitude: the neighbour
    // sharing the index is the same square, the one in the next row is not.
    yield_reset();
    ytile(X, Y).climate = YALT_LAND | 0x10;
    YCHECK(crop_yield(YFACTION, 0, 3, Y, 0) == 2);   // (3,2) shares the index
    YCHECK(crop_yield(YFACTION, 0, 4, Y, 0) == 0);   // (4,2) does not
    YCHECK(crop_yield(YFACTION, 0, X, 3, 0) == 0);

    // ---- the nutrient bonus is bonus id 1, not 2 and not 3 -----------------
    yield_reset();
    ytile(X, Y).climate = YALT_LAND | 0x10;
    ytile(X, Y).bit = YBONUS_NUTRIENT;
    YCHECK(crop_yield(YFACTION, 0, X, Y, 0) == 2 + 7);
    ytile(X, Y).bit = YBONUS_MINERAL;
    YCHECK(crop_yield(YFACTION, 0, X, Y, 0) == 2);
    ytile(X, Y).bit = YBONUS_ENERGY;
    YCHECK(crop_yield(YFACTION, 0, X, Y, 0) == 2);

    // ---- the Jungle landmark counts on land, Freshwater Sea on ocean -------
    yield_reset();
    ytile(X, Y).climate = YALT_LAND | 0x10;
    ytile(X, Y).bit2 = BIT2_JUNGLE;
    YCHECK(crop_yield(YFACTION, 0, X, Y, 0) == 3);
    ytile(X, Y).bit2 = BIT2_JUNGLE | BIT2_UNK_80000000;  // suppressed
    YCHECK(crop_yield(YFACTION, 0, X, Y, 0) == 2);
    ytile(X, Y).bit2 = BIT2_FRESH;                       // wrong terrain for it
    YCHECK(crop_yield(YFACTION, 0, X, Y, 0) == 2);
    // On the shelf the pair swaps over. OCEAN_SQ.nutrients is 1.
    yield_reset();
    ytile(X, Y).climate = YALT_SHELF;
    ytile(X, Y).bit2 = BIT2_FRESH;
    YCHECK(crop_yield(YFACTION, 0, X, Y, 0) == 1 + 1);
    ytile(X, Y).bit2 = BIT2_JUNGLE;
    YCHECK(crop_yield(YFACTION, 0, X, Y, 0) == 1);
    // BIT2_FRESH is tested WITHOUT the 0x80000000 suppressor, unlike Jungle.
    ytile(X, Y).bit2 = BIT2_FRESH | BIT2_UNK_80000000;
    YCHECK(crop_yield(YFACTION, 0, X, Y, 0) == 1 + 1);

    // ---- farm and mine on plain land ---------------------------------------
    yield_reset();
    ytile(X, Y).climate = YALT_LAND | 0x10;
    ytile(X, Y).bit = BIT_FARM;
    YCHECK(crop_yield(YFACTION, 0, X, Y, 0) == 2 + 16);  // IMPROVED_LAND
    ytile(X, Y).val3 = 0x40;                             // rolling still farms
    YCHECK(crop_yield(YFACTION, 0, X, Y, 0) == 2 + 16);
    ytile(X, Y).val3 = 0x80;                             // rocky does not
    YCHECK(crop_yield(YFACTION, 0, X, Y, 0) == 0);
    yield_reset();
    ytile(X, Y).climate = YALT_LAND | 0x10;
    ytile(X, Y).bit = BIT_MINE;
    YCHECK(crop_yield(YFACTION, 0, X, Y, 0) == 1);       // 2 > 1, so -1 applies
    ytile(X, Y).climate = YALT_LAND | 0x08;              // moist: 1, not > 1
    YCHECK(crop_yield(YFACTION, 0, X, Y, 0) == 1);

    // ---- soil enricher and condenser are each a half, applied in turn ------
    yield_reset();
    ytile(X, Y).climate = YALT_LAND | 0x10;
    ytile(X, Y).bit = BIT_SOIL_ENRICHER;
    YCHECK(crop_yield(YFACTION, 0, X, Y, 0) == 3);
    ytile(X, Y).bit = BIT_CONDENSER;
    YCHECK(crop_yield(YFACTION, 0, X, Y, 0) == 3);
    ytile(X, Y).bit = BIT_SOIL_ENRICHER | BIT_CONDENSER;
    YCHECK(crop_yield(YFACTION, 0, X, Y, 0) == 4);       // 2 -> 3 -> 4

    // ---- the clip to two, and its three suppressors ------------------------
    // Without the technology a square is held to 2 and the overflow is
    // published; with it, or with a resource bonus, or under a condenser, it is
    // not. The condenser suppressor is crop_yield's alone.
    yield_reset();
    g_yield_world.tech_achieved[YTECH_THREE_NUTRIENTS] = 0;
    ytile(X, Y).climate = YALT_LAND | 0x10;
    ytile(X, Y).bit = BIT_SOIL_ENRICHER;                 // 2 -> 3
    YCHECK(crop_yield(YFACTION, 0, X, Y, 0) == 2);
    YCHECK(g_yield_world.restricted == 1);
    g_yield_world.restricted = 0;
    ytile(X, Y).bit = BIT_CONDENSER;                     // also 3, but suppressed
    YCHECK(crop_yield(YFACTION, 0, X, Y, 0) == 3);
    YCHECK(g_yield_world.restricted == 0);
    g_yield_world.tech_achieved[YTECH_THREE_NUTRIENTS] = 1 << YFACTION;
    ytile(X, Y).bit = BIT_SOIL_ENRICHER;
    YCHECK(crop_yield(YFACTION, 0, X, Y, 0) == 3);
    YCHECK(g_yield_world.restricted == 0);
    // A different faction does not hold the technology.
    YCHECK(crop_yield(2, 0, X, Y, 0) == 2);
    YCHECK(g_yield_world.restricted == 1);
    // The gate is Rules->tech_three_nutrients_sqr, not the mineral or energy one.
    yield_reset();
    g_yield_world.tech_achieved[YTECH_THREE_NUTRIENTS] = 0;
    g_yield_world.rules.tech_three_nutrients_sqr = YTECH_THREE_MINERALS;
    ytile(X, Y).climate = YALT_LAND | 0x10;
    ytile(X, Y).bit = BIT_SOIL_ENRICHER;
    YCHECK(crop_yield(YFACTION, 0, X, Y, 0) == 3);
    // The published overflow accumulates rather than being assigned.
    yield_reset();
    g_yield_world.tech_achieved[YTECH_THREE_NUTRIENTS] = 0;
    g_yield_world.restricted = 100;
    ytile(X, Y).climate = YALT_LAND | 0x10;
    ytile(X, Y).bit = BIT_SOIL_ENRICHER;
    YCHECK(crop_yield(YFACTION, 0, X, Y, 0) == 2);
    YCHECK(g_yield_world.restricted == 101);

    // ---- forest -------------------------------------------------------------
    yield_reset();
    ytile(X, Y).bit = BIT_FOREST;
    YCHECK(crop_yield(YFACTION, 0, X, Y, 0) == 10);      // FOREST_SQ.nutrients
    ytile(X, Y).bit = BIT_FOREST | YBONUS_NUTRIENT;
    YCHECK(crop_yield(YFACTION, 0, X, Y, 0) == 10 + 7);
    yield_reset();
    ytile(X, Y).bit = BIT_FOREST;
    ytile(X, Y).bit2 = BIT2_JUNGLE;
    YCHECK(crop_yield(YFACTION, 0, X, Y, 0) == 11);
    yield_reset();
    ytile(X, Y).bit = BIT_FOREST;
    yield_give_fac(g_yield_world.bases[1], FAC_HYBRID_FOREST);
    YCHECK(crop_yield(YFACTION, 0, X, Y, 0) == 11);
    yield_give_fac(g_yield_world.bases[1], FAC_TREE_FARM);
    YCHECK(crop_yield(YFACTION, 0, X, Y, 0) == 12);
    YCHECK(crop_yield(YFACTION, -1, X, Y, 0) == 10);     // no base, no facilities

    // ---- ocean --------------------------------------------------------------
    yield_reset();
    ytile(X, Y).climate = YALT_SHELF;
    YCHECK(crop_yield(YFACTION, 0, X, Y, 0) == 1);       // OCEAN_SQ.nutrients
    ytile(X, Y).bit = BIT_FARM;                          // kelp farm
    YCHECK(crop_yield(YFACTION, 0, X, Y, 0) == 1 + 19);  // IMPROVED_SEA
    yield_give_fac(g_yield_world.bases[1], FAC_AQUAFARM);
    YCHECK(crop_yield(YFACTION, 0, X, Y, 0) == 1 + 19 + 1);
    // Deep ocean is outside the improvement block unless SMACX is enabled.
    yield_reset();
    ytile(X, Y).climate = YALT_DEEP;
    ytile(X, Y).bit = BIT_FARM;
    YCHECK(crop_yield(YFACTION, 0, X, Y, 0) == 1);
    g_yield_world.expansion = 1;
    YCHECK(crop_yield(YFACTION, 0, X, Y, 0) == 1 + 19);
    // A sea mine takes its nutrient back, and only above 1.
    yield_reset();
    ytile(X, Y).climate = YALT_SHELF;
    ytile(X, Y).bit = BIT_MINE | BIT_FARM;
    YCHECK(crop_yield(YFACTION, 0, X, Y, 0) == 1 + 19 - 1);
    ytile(X, Y).bit = BIT_MINE;
    YCHECK(crop_yield(YFACTION, 0, X, Y, 0) == 1);       // 1 is not > 1
    // The aquafarm read is NOT guarded against base_id -1: it reads the entry
    // below the array, which the fixture provides.
    yield_reset();
    ytile(X, Y).climate = YALT_SHELF;
    ytile(X, Y).bit = BIT_FARM;
    yield_give_fac(g_yield_world.bases[0], FAC_AQUAFARM);
    YCHECK(crop_yield(YFACTION, -1, X, Y, 0) == 1 + 19 + 1);

    // ---- a base square ------------------------------------------------------
    yield_reset();
    yield_put_base_in_tile(ytile(X, Y), 3);
    YCHECK(crop_yield(YFACTION, 0, X, Y, 0) == 4);       // BASE_SQ.nutrients
    ytile(X, Y).bit |= YBONUS_NUTRIENT;
    YCHECK(crop_yield(YFACTION, 0, X, Y, 0) == 8);       // doubled, not increased
    yield_reset();
    yield_put_base_in_tile(ytile(X, Y), 3);
    ytile(X, Y).bit2 = BIT2_JUNGLE;
    YCHECK(crop_yield(YFACTION, 0, X, Y, 0) == 5);
    yield_reset();
    yield_put_base_in_tile(ytile(X, Y), 3);
    yield_give_fac(g_yield_world.bases[1], FAC_RECYCLING_TANKS);
    YCHECK(crop_yield(YFACTION, 0, X, Y, 0) == 4 + 13);
    yield_reset();
    yield_put_base_in_tile(ytile(X, Y), 3);
    yield_give_fac(g_yield_world.bases[1], FAC_PRESSURE_DOME);
    YCHECK(crop_yield(YFACTION, 0, X, Y, 0) == 4 + 13);
    // An owner nibble of MaxPlayerNum or above is the "unoccupied" marker and
    // does not make it a base square.
    yield_reset();
    yield_put_base_in_tile(ytile(X, Y), 0xF);
    ytile(X, Y).climate = YALT_LAND | 0x10;
    YCHECK(crop_yield(YFACTION, 0, X, Y, 0) == 2);
    yield_reset();
    yield_put_base_in_tile(ytile(X, Y), 7);
    YCHECK(crop_yield(YFACTION, 0, X, Y, 0) == 4);
    yield_reset();
    yield_put_base_in_tile(ytile(X, Y), 8);
    ytile(X, Y).climate = YALT_LAND | 0x10;
    YCHECK(crop_yield(YFACTION, 0, X, Y, 0) == 2);
    // Without BIT_BASE_IN_TILE the owner nibble means nothing.
    yield_reset();
    ytile(X, Y).val2 = 3;
    ytile(X, Y).climate = YALT_LAND | 0x10;
    YCHECK(crop_yield(YFACTION, 0, X, Y, 0) == 2);

    // ---- a thermal borehole tests the VALUE, not the bonus flag ------------
    // This is the asymmetry with mine_yield's parallel arm and it is literal:
    // 0x004E6FCB is `test esi, esi` on the borehole nutrients just loaded.
    yield_reset();
    ytile(X, Y).bit = BIT_THERMAL_BORE;
    YCHECK(crop_yield(YFACTION, 0, X, Y, 0) == 25 + 7);  // no bonus, still added
    g_yield_world.resource_info[RSCINFO_BOREHOLE_SQ].nutrients = 0;
    ytile(X, Y).bit = BIT_THERMAL_BORE | YBONUS_NUTRIENT;
    YCHECK(crop_yield(YFACTION, 0, X, Y, 0) == 0);       // bonus, nothing added
    // The borehole arm is tested BEFORE the monolith arm here; mine_yield and
    // energy_yield order the two the other way round.
    yield_reset();
    ytile(X, Y).bit = BIT_THERMAL_BORE | BIT_MONOLITH;
    YCHECK(crop_yield(YFACTION, 0, X, Y, 0) == 25 + 7);
    // Neither arm reaches the clip: 32 survives with no technology at all.
    yield_reset();
    g_yield_world.tech_achieved[YTECH_THREE_NUTRIENTS] = 0;
    ytile(X, Y).bit = BIT_THERMAL_BORE;
    YCHECK(crop_yield(YFACTION, 0, X, Y, 0) == 32);
    YCHECK(g_yield_world.restricted == 0);

    // ---- a monolith ---------------------------------------------------------
    yield_reset();
    ytile(X, Y).bit = BIT_MONOLITH;
    YCHECK(crop_yield(YFACTION, 0, X, Y, 0) == 22);      // MONOLITH.nutrients
    ytile(X, Y).bit = BIT_MONOLITH | YBONUS_NUTRIENT;
    YCHECK(crop_yield(YFACTION, 0, X, Y, 0) == 22 + 7);  // added, not doubled
    // Manifold Harmonics needs a positive PLANET rating as well as the project.
    yield_reset();
    ytile(X, Y).bit = BIT_MONOLITH;
    g_yield_world.projects.manifold_harmonics = 0;       // held by Bases[0]
    g_yield_world.bases[1].faction_id_current = YFACTION;
    YCHECK(crop_yield(YFACTION, 0, X, Y, 0) == 22);      // planet 0 is not > 0
    g_yield_world.players_data[YFACTION].soc_effect_pending.planet = 1;
    YCHECK(crop_yield(YFACTION, 0, X, Y, 0) == 23);
    g_yield_world.bases[1].faction_id_current = 2;       // somebody else built it
    YCHECK(crop_yield(YFACTION, 0, X, Y, 0) == 22);
    g_yield_world.bases[1].faction_id_current = YFACTION;
    g_yield_world.projects.manifold_harmonics = -1;      // nobody built it
    YCHECK(crop_yield(YFACTION, 0, X, Y, 0) == 22);

    // ---- fungus -------------------------------------------------------------
    yield_reset();
    ytile(X, Y).bit = BIT_FUNGUS;
    YCHECK(crop_yield(YFACTION, 0, X, Y, 0) == 4);       // tech_fungus_nutrient
    YCHECK(crop_yield(YFACTION, 0, X, Y, 1) == 0);       // improved: arm skipped
    // A negative PLANET rating is subtracted, floored at -3; a positive one is
    // not added.
    g_yield_world.players_data[YFACTION].soc_effect_pending.planet = -2;
    YCHECK(crop_yield(YFACTION, 0, X, Y, 0) == 2);
    g_yield_world.players_data[YFACTION].soc_effect_pending.planet = -9;
    YCHECK(crop_yield(YFACTION, 0, X, Y, 0) == 1);
    g_yield_world.players_data[YFACTION].soc_effect_pending.planet = 5;
    YCHECK(crop_yield(YFACTION, 0, X, Y, 0) == 4);
    // ... and the sum is clamped at zero from below.
    g_yield_world.players_data[YFACTION].soc_effect_pending.planet = -3;
    g_yield_world.players_data[YFACTION].tech_fungus_nutrient = 1;
    YCHECK(crop_yield(YFACTION, 0, X, Y, 0) == 0);
    // Manifold Harmonics adds one on top, and only above PLANET 0.
    yield_reset();
    ytile(X, Y).bit = BIT_FUNGUS;
    g_yield_world.projects.manifold_harmonics = 0;
    g_yield_world.bases[1].faction_id_current = YFACTION;
    YCHECK(crop_yield(YFACTION, 0, X, Y, 0) == 4);
    g_yield_world.players_data[YFACTION].soc_effect_pending.planet = 2;
    YCHECK(crop_yield(YFACTION, 0, X, Y, 0) == 5);
    // Deep ocean fungus is below the shelf and falls through to the ocean arm.
    yield_reset();
    ytile(X, Y).bit = BIT_FUNGUS;
    ytile(X, Y).climate = YALT_SHELF;
    YCHECK(crop_yield(YFACTION, 0, X, Y, 0) == 4);
    ytile(X, Y).climate = YALT_DEEP;
    YCHECK(crop_yield(YFACTION, 0, X, Y, 0) == 1);       // OCEAN_SQ.nutrients

    // Faction 0 owns bases too, and its owner nibble is zero: "no base here" is
    // spelled -1, not 0.
    yield_reset();
    yield_put_base_in_tile(ytile(X, Y), 0);
    YCHECK(crop_yield(YFACTION, 0, X, Y, 0) == 4);

    // ---- the PLANET clamp and the Manifold threshold at exactly 1 ----------
    // planet_mod is 0 for any rating above 0, so a rating of 1 subtracts
    // nothing; the Manifold bonus, by contrast, does fire at 1.
    yield_reset();
    ytile(X, Y).bit = BIT_FUNGUS;
    g_yield_world.players_data[YFACTION].soc_effect_pending.planet = 1;
    YCHECK(crop_yield(YFACTION, 0, X, Y, 0) == 4);
    g_yield_world.projects.manifold_harmonics = 0;
    g_yield_world.bases[1].faction_id_current = YFACTION;
    YCHECK(crop_yield(YFACTION, 0, X, Y, 0) == 5);

    // ---- the ocean arm takes the resource bonus too -------------------------
    yield_reset();
    ytile(X, Y).climate = YALT_SHELF;
    ytile(X, Y).bit = YBONUS_NUTRIENT;
    YCHECK(crop_yield(YFACTION, 0, X, Y, 0) == 1 + 7);

    // ---- plain land floors at zero before the improvements are added -------
    // A negative bonus is the only way a square's terrain value goes below
    // zero, and the floor is applied after the bonus and the landmark.
    yield_reset();
    g_yield_world.resource_info[RSCINFO_BONUS_SQ].nutrients = -5;
    ytile(X, Y).bit = YBONUS_NUTRIENT;
    YCHECK(crop_yield(YFACTION, 0, X, Y, 0) == 0);
    ytile(X, Y).bit2 = BIT2_JUNGLE;
    YCHECK(crop_yield(YFACTION, 0, X, Y, 0) == 0);
    // With a farm on top the floored zero is what gets improved.
    ytile(X, Y).bit = YBONUS_NUTRIENT | BIT_FARM;
    ytile(X, Y).bit2 = 0;
    YCHECK(crop_yield(YFACTION, 0, X, Y, 0) == 16);

    // ---- the clip is strictly above two -------------------------------------
    yield_reset();
    g_yield_world.tech_achieved[YTECH_THREE_NUTRIENTS] = 0;
    ytile(X, Y).climate = YALT_LAND | 0x08;              // moist: one nutrient
    YCHECK(crop_yield(YFACTION, 0, X, Y, 0) == 1);
    YCHECK(g_yield_world.restricted == 0);
    ytile(X, Y).climate = YALT_LAND | 0x10;              // rainy: two
    YCHECK(crop_yield(YFACTION, 0, X, Y, 0) == 2);
    YCHECK(g_yield_world.restricted == 0);

    // ---- the base events, and the base square's early return ---------------
    yield_reset();
    ytile(X, Y).climate = YALT_LAND | 0x10;
    g_yield_world.bases[1].event = BEVENT_BUMPER;
    YCHECK(crop_yield(YFACTION, 0, X, Y, 0) == 3);
    g_yield_world.bases[1].event = BEVENT_FAMINE;
    YCHECK(crop_yield(YFACTION, 0, X, Y, 0) == 1);
    g_yield_world.bases[1].event = BEVENT_BUMPER | BEVENT_FAMINE;
    YCHECK(crop_yield(YFACTION, 0, X, Y, 0) == 2);
    ytile(X, Y).climate = YALT_LAND;                     // arid: nothing to take
    g_yield_world.bases[1].event = BEVENT_FAMINE;
    YCHECK(crop_yield(YFACTION, 0, X, Y, 0) == 0);
    YCHECK(crop_yield(YFACTION, -1, X, Y, 0) == 0);      // no base, no events
    // A base square with base_id -1 returns before the event tail, so an event
    // on the entry below the array cannot reach it.
    yield_reset();
    yield_put_base_in_tile(ytile(X, Y), 3);
    g_yield_world.bases[0].event = BEVENT_BUMPER;
    YCHECK(crop_yield(YFACTION, -1, X, Y, 0) == 4);
}

void test_mine_yield() {
    YieldSeams seams;
    const int X = 2;
    const int Y = 2;

    // ---- plain land: rockiness alone ---------------------------------------
    yield_reset();
    YCHECK(mine_yield(YFACTION, 0, X, Y, 0) == 0);
    ytile(X, Y).val3 = 0x40;                             // rolling
    YCHECK(mine_yield(YFACTION, 0, X, Y, 0) == 1);
    ytile(X, Y).val3 = 0x80;                             // rocky: still just one
    YCHECK(mine_yield(YFACTION, 0, X, Y, 0) == 1);

    // ---- the mineral bonus is bonus id 2 ------------------------------------
    yield_reset();
    ytile(X, Y).bit = YBONUS_MINERAL;
    YCHECK(mine_yield(YFACTION, 0, X, Y, 0) == 8);       // BONUS_SQ.minerals
    ytile(X, Y).bit = YBONUS_NUTRIENT;
    YCHECK(mine_yield(YFACTION, 0, X, Y, 0) == 0);
    ytile(X, Y).bit = YBONUS_ENERGY;
    YCHECK(mine_yield(YFACTION, 0, X, Y, 0) == 0);

    // ---- the mineral landmark set ------------------------------------------
    yield_reset();
    ytile(X, Y).bit2 = BIT2_VOLCANO;
    YCHECK(mine_yield(YFACTION, 0, X, Y, 0) == 1);
    ytile(X, Y).bit2 = BIT2_CRATER;
    YCHECK(mine_yield(YFACTION, 0, X, Y, 0) == 1);
    ytile(X, Y).bit2 = BIT2_FOSSIL;
    YCHECK(mine_yield(YFACTION, 0, X, Y, 0) == 1);
    ytile(X, Y).bit2 = BIT2_CANYON;
    YCHECK(mine_yield(YFACTION, 0, X, Y, 0) == 1);
    ytile(X, Y).bit2 = BIT2_JUNGLE;                      // a crop landmark
    YCHECK(mine_yield(YFACTION, 0, X, Y, 0) == 0);
    // The landmark sequence code caps Volcano and Crater at 9 and Fossil at 6;
    // Canyon has no cap at all.
    ytile(X, Y).bit2 = BIT2_VOLCANO | 0x09000000u;
    YCHECK(mine_yield(YFACTION, 0, X, Y, 0) == 0);
    ytile(X, Y).bit2 = BIT2_VOLCANO | 0x08000000u;
    YCHECK(mine_yield(YFACTION, 0, X, Y, 0) == 1);
    ytile(X, Y).bit2 = BIT2_CRATER | 0x09000000u;
    YCHECK(mine_yield(YFACTION, 0, X, Y, 0) == 0);
    ytile(X, Y).bit2 = BIT2_FOSSIL | 0x06000000u;
    YCHECK(mine_yield(YFACTION, 0, X, Y, 0) == 0);
    ytile(X, Y).bit2 = BIT2_FOSSIL | 0x05000000u;
    YCHECK(mine_yield(YFACTION, 0, X, Y, 0) == 1);
    ytile(X, Y).bit2 = BIT2_CANYON | 0x7F000000u;
    YCHECK(mine_yield(YFACTION, 0, X, Y, 0) == 1);
    // Every one of the four is suppressed by bit 31, which also makes the code
    // negative - so the caps cannot be what lets it through.
    ytile(X, Y).bit2 = BIT2_VOLCANO | BIT2_UNK_80000000;
    YCHECK(mine_yield(YFACTION, 0, X, Y, 0) == 0);
    ytile(X, Y).bit2 = BIT2_CRATER | BIT2_UNK_80000000;
    YCHECK(mine_yield(YFACTION, 0, X, Y, 0) == 0);
    ytile(X, Y).bit2 = BIT2_FOSSIL | BIT2_UNK_80000000;
    YCHECK(mine_yield(YFACTION, 0, X, Y, 0) == 0);
    ytile(X, Y).bit2 = BIT2_CANYON | BIT2_UNK_80000000;
    YCHECK(mine_yield(YFACTION, 0, X, Y, 0) == 0);

    // ---- a mine, a borehole, or the "assume improved" preview ---------------
    // rockiness is added once as terrain and once as the mine's own extra, and
    // a flat square still earns one.
    yield_reset();
    g_yield_world.rules.limit_mineral_mine_sans_road = 99;  // out of the way
    ytile(X, Y).bit = BIT_MINE;
    YCHECK(mine_yield(YFACTION, 0, X, Y, 0) == 1);       // flat: 0 + max(0,1)
    ytile(X, Y).val3 = 0x40;
    YCHECK(mine_yield(YFACTION, 0, X, Y, 0) == 2);       // rolling: 1 + 1
    ytile(X, Y).val3 = 0x80;
    YCHECK(mine_yield(YFACTION, 0, X, Y, 0) == 4);       // rocky: 2 + 2
    g_yield_world.tech_achieved[YTECH_THREE_MINERALS] = 0;
    YCHECK(mine_yield(YFACTION, 0, X, Y, 0) == 2);       // ... and then clipped
    YCHECK(g_yield_world.restricted == 2);
    // The preview flag takes the same arm as a real mine.
    yield_reset();
    g_yield_world.rules.limit_mineral_mine_sans_road = 99;
    g_yield_world.tech_achieved[YTECH_THREE_MINERALS] = 1 << YFACTION;
    ytile(X, Y).val3 = 0x80;
    YCHECK(mine_yield(YFACTION, 0, X, Y, 1) == 4);       // assume_improved
    YCHECK(mine_yield(YFACTION, 0, X, Y, 0) == 1);       // rocky, unimproved
    // A bonus or a landmark adds one to the mine's own extra.
    yield_reset();
    g_yield_world.rules.limit_mineral_mine_sans_road = 99;
    g_yield_world.tech_achieved[YTECH_THREE_MINERALS] = 1 << YFACTION;
    ytile(X, Y).bit = BIT_MINE;
    ytile(X, Y).bit2 = BIT2_CANYON;
    YCHECK(mine_yield(YFACTION, 0, X, Y, 0) == 1 + 0 + 2);  // landmark twice over

    // ---- the roadless mine limit -------------------------------------------
    yield_reset();
    g_yield_world.tech_achieved[YTECH_THREE_MINERALS] = 1 << YFACTION;
    ytile(X, Y).bit = BIT_MINE;
    ytile(X, Y).val3 = 0x80;                             // rocky: extra would be 2
    YCHECK(mine_yield(YFACTION, 0, X, Y, 0) == 4);       // limit 2, extra 2: no clip
    YCHECK(g_yield_world.restricted == 0);
    g_yield_world.rules.limit_mineral_mine_sans_road = 1;
    YCHECK(mine_yield(YFACTION, 0, X, Y, 0) == 3);
    YCHECK(g_yield_world.restricted == 1);
    g_yield_world.restricted = 0;
    ytile(X, Y).bit = BIT_MINE | BIT_ROAD;               // a road lifts the limit
    YCHECK(mine_yield(YFACTION, 0, X, Y, 0) == 4);
    YCHECK(g_yield_world.restricted == 0);
    ytile(X, Y).bit = BIT_MINE;                          // ... so does the preview
    YCHECK(mine_yield(YFACTION, 0, X, Y, 1) == 4);
    YCHECK(g_yield_world.restricted == 0);

    // ---- forest -------------------------------------------------------------
    yield_reset();
    ytile(X, Y).bit = BIT_FOREST;
    YCHECK(mine_yield(YFACTION, 0, X, Y, 0) == 11);      // FOREST_SQ.minerals
    ytile(X, Y).bit = BIT_FOREST | BIT_MINE;             // forest is tested first
    YCHECK(mine_yield(YFACTION, 0, X, Y, 0) == 11);
    // The prefix survives the forest arm, unlike the monolith and borehole arms.
    yield_reset();
    ytile(X, Y).bit = BIT_FOREST | YBONUS_MINERAL;
    YCHECK(mine_yield(YFACTION, 0, X, Y, 0) == 8 + 11);

    // ---- ocean --------------------------------------------------------------
    yield_reset();
    ytile(X, Y).climate = YALT_SHELF;
    YCHECK(mine_yield(YFACTION, 0, X, Y, 0) == 2);       // OCEAN_SQ.minerals
    // The aquatic bonus keys off the faction owning BASE_ID, not the caller's.
    g_yield_world.players[5].rule_flags = RFLAG_AQUATIC;
    g_yield_world.bases[1].faction_id_current = 5;
    YCHECK(mine_yield(YFACTION, 0, X, Y, 0) == 3);
    g_yield_world.players[5].rule_flags = 0;
    g_yield_world.players[YFACTION].rule_flags = RFLAG_AQUATIC;
    YCHECK(mine_yield(YFACTION, 0, X, Y, 0) == 2);
    // ... and only on the shelf.
    g_yield_world.players[5].rule_flags = RFLAG_AQUATIC;
    ytile(X, Y).climate = YALT_DEEP;
    YCHECK(mine_yield(YFACTION, 0, X, Y, 0) == 2);
    // It reads Bases[base_id] unguarded, so -1 reads the entry below the array.
    yield_reset();
    ytile(X, Y).climate = YALT_SHELF;
    g_yield_world.bases[0].faction_id_current = 5;
    g_yield_world.players[5].rule_flags = RFLAG_AQUATIC;
    YCHECK(mine_yield(YFACTION, -1, X, Y, 0) == 3);
    // A sea mine, the mining platform technology, and the subsea trunkline.
    yield_reset();
    g_yield_world.tech_achieved[YTECH_THREE_MINERALS] = 1 << YFACTION;
    ytile(X, Y).climate = YALT_SHELF;
    ytile(X, Y).bit = BIT_MINE;
    YCHECK(mine_yield(YFACTION, 0, X, Y, 0) == 2 + 20);  // IMPROVED_SEA
    YCHECK(mine_yield(YFACTION, 0, X, Y, 1) == 2 + 20);  // preview does the same
    g_yield_world.tech_achieved[YTECH_MINING_PLATFORM] = 1 << YFACTION;
    YCHECK(mine_yield(YFACTION, 0, X, Y, 0) == 2 + 20 + 1);
    yield_give_fac(g_yield_world.bases[1], FAC_SUBSEA_TRUNKLINE);
    YCHECK(mine_yield(YFACTION, 0, X, Y, 0) == 2 + 20 + 2);
    // Deep ocean needs SMACX for the improvement block.
    yield_reset();
    g_yield_world.tech_achieved[YTECH_THREE_MINERALS] = 1 << YFACTION;
    ytile(X, Y).climate = YALT_DEEP;
    ytile(X, Y).bit = BIT_MINE;
    YCHECK(mine_yield(YFACTION, 0, X, Y, 0) == 2);
    g_yield_world.expansion = 1;
    YCHECK(mine_yield(YFACTION, 0, X, Y, 0) == 2 + 20);

    // ---- a base square ------------------------------------------------------
    yield_reset();
    yield_put_base_in_tile(ytile(X, Y), 3);
    YCHECK(mine_yield(YFACTION, 0, X, Y, 0) == 5);       // BASE_SQ.minerals
    ytile(X, Y).bit |= YBONUS_MINERAL;                   // the prefix survives
    YCHECK(mine_yield(YFACTION, 0, X, Y, 0) == 5 + 8);
    yield_reset();
    yield_put_base_in_tile(ytile(X, Y), 3);
    yield_give_fac(g_yield_world.bases[1], FAC_RECYCLING_TANKS);
    YCHECK(mine_yield(YFACTION, 0, X, Y, 0) == 5 + 14);
    yield_reset();
    yield_put_base_in_tile(ytile(X, Y), 3);
    yield_give_fac(g_yield_world.bases[1], FAC_PRESSURE_DOME);
    YCHECK(mine_yield(YFACTION, 0, X, Y, 0) == 5 + 14);
    // A base square with base_id -1 returns before the event tail.
    yield_reset();
    yield_put_base_in_tile(ytile(X, Y), 3);
    g_yield_world.bases[0].event = BEVENT_INDUSTRY;
    YCHECK(mine_yield(YFACTION, -1, X, Y, 0) == 5);
    // Nor does the base square ever reach the clip.
    yield_reset();
    g_yield_world.tech_achieved[YTECH_THREE_MINERALS] = 0;
    yield_put_base_in_tile(ytile(X, Y), 3);
    yield_give_fac(g_yield_world.bases[1], FAC_RECYCLING_TANKS);
    YCHECK(mine_yield(YFACTION, 0, X, Y, 0) == 19);
    YCHECK(g_yield_world.restricted == 0);

    // ---- a monolith DISCARDS the bonus and landmark prefix ------------------
    yield_reset();
    ytile(X, Y).bit = BIT_MONOLITH;
    YCHECK(mine_yield(YFACTION, 0, X, Y, 0) == 23);      // MONOLITH.minerals
    ytile(X, Y).bit = BIT_MONOLITH | YBONUS_MINERAL;
    ytile(X, Y).bit2 = BIT2_CANYON;                      // landmark: discarded
    YCHECK(mine_yield(YFACTION, 0, X, Y, 0) == 23 + 8);
    // The monolith arm is tested BEFORE the borehole arm here; crop_yield
    // orders the two the other way round.
    yield_reset();
    ytile(X, Y).bit = BIT_MONOLITH | BIT_THERMAL_BORE;
    YCHECK(mine_yield(YFACTION, 0, X, Y, 0) == 23);
    // Manifold Harmonics needs PLANET above 1 here, not above 0.
    yield_reset();
    ytile(X, Y).bit = BIT_MONOLITH;
    g_yield_world.projects.manifold_harmonics = 0;
    g_yield_world.bases[1].faction_id_current = YFACTION;
    g_yield_world.players_data[YFACTION].soc_effect_pending.planet = 1;
    YCHECK(mine_yield(YFACTION, 0, X, Y, 0) == 23);
    g_yield_world.players_data[YFACTION].soc_effect_pending.planet = 2;
    YCHECK(mine_yield(YFACTION, 0, X, Y, 0) == 24);
    // Neither the monolith nor the fungus arm reaches the clip.
    yield_reset();
    g_yield_world.tech_achieved[YTECH_THREE_MINERALS] = 0;
    ytile(X, Y).bit = BIT_MONOLITH;
    YCHECK(mine_yield(YFACTION, 0, X, Y, 0) == 23);
    YCHECK(g_yield_world.restricted == 0);

    // ---- a thermal borehole tests the FLAG, not the value -------------------
    // The mirror image of crop_yield's arm, and the reason neither was
    // normalised.
    yield_reset();
    ytile(X, Y).bit = BIT_THERMAL_BORE;
    YCHECK(mine_yield(YFACTION, 0, X, Y, 0) == 26);      // no bonus, nothing added
    ytile(X, Y).bit = BIT_THERMAL_BORE | YBONUS_MINERAL;
    YCHECK(mine_yield(YFACTION, 0, X, Y, 0) == 26 + 8);
    // ... and unlike crop_yield's, it DOES reach the clip.
    yield_reset();
    g_yield_world.tech_achieved[YTECH_THREE_MINERALS] = 0;
    ytile(X, Y).bit = BIT_THERMAL_BORE;
    YCHECK(mine_yield(YFACTION, 0, X, Y, 0) == 2);
    YCHECK(g_yield_world.restricted == 24);
    // The landmark prefix is discarded by this arm too.
    yield_reset();
    ytile(X, Y).bit = BIT_THERMAL_BORE;
    ytile(X, Y).bit2 = BIT2_CANYON;
    YCHECK(mine_yield(YFACTION, 0, X, Y, 0) == 26);

    // ---- fungus -------------------------------------------------------------
    yield_reset();
    ytile(X, Y).bit = BIT_FUNGUS;
    YCHECK(mine_yield(YFACTION, 0, X, Y, 0) == 5);       // tech_fungus_mineral
    // There is NO assume_improved suppressor here; crop_yield has one.
    YCHECK(mine_yield(YFACTION, 0, X, Y, 1) == 5);
    g_yield_world.players_data[YFACTION].soc_effect_pending.planet = -2;
    YCHECK(mine_yield(YFACTION, 0, X, Y, 0) == 3);
    g_yield_world.players_data[YFACTION].soc_effect_pending.planet = -9;
    YCHECK(mine_yield(YFACTION, 0, X, Y, 0) == 2);
    yield_reset();
    ytile(X, Y).bit = BIT_FUNGUS;
    ytile(X, Y).climate = YALT_DEEP;                     // below the shelf
    YCHECK(mine_yield(YFACTION, 0, X, Y, 0) == 2);       // ocean arm instead
    yield_reset();
    ytile(X, Y).bit = BIT_FUNGUS;
    g_yield_world.projects.manifold_harmonics = 0;
    g_yield_world.bases[1].faction_id_current = YFACTION;
    g_yield_world.players_data[YFACTION].soc_effect_pending.planet = 1;
    YCHECK(mine_yield(YFACTION, 0, X, Y, 0) == 5);       // 1 is not > 1
    g_yield_world.players_data[YFACTION].soc_effect_pending.planet = 2;
    YCHECK(mine_yield(YFACTION, 0, X, Y, 0) == 6);

    // ---- the clip, and its two suppressors ----------------------------------
    // Mineral has no condenser suppressor - only the technology and the bonus.
    yield_reset();
    g_yield_world.tech_achieved[YTECH_THREE_MINERALS] = 0;
    ytile(X, Y).bit = BIT_FOREST;                        // 11 minerals
    YCHECK(mine_yield(YFACTION, 0, X, Y, 0) == 2);
    YCHECK(g_yield_world.restricted == 9);
    g_yield_world.restricted = 0;
    ytile(X, Y).bit = BIT_FOREST | BIT_CONDENSER;
    YCHECK(mine_yield(YFACTION, 0, X, Y, 0) == 2);       // no condenser reprieve
    g_yield_world.restricted = 0;
    ytile(X, Y).bit = BIT_FOREST | YBONUS_MINERAL;
    YCHECK(mine_yield(YFACTION, 0, X, Y, 0) == 8 + 11);
    YCHECK(g_yield_world.restricted == 0);
    // The gate is Rules->tech_three_minerals_sqr.
    yield_reset();
    g_yield_world.tech_achieved[YTECH_THREE_MINERALS] = 0;
    g_yield_world.rules.tech_three_minerals_sqr = YTECH_THREE_NUTRIENTS;
    ytile(X, Y).bit = BIT_FOREST;
    YCHECK(mine_yield(YFACTION, 0, X, Y, 0) == 11);

    // Faction 0's base square is a base square.
    yield_reset();
    yield_put_base_in_tile(ytile(X, Y), 0);
    YCHECK(mine_yield(YFACTION, 0, X, Y, 0) == 5);

    // The fungus arm reaches down to the ocean shelf exactly, and its sum is
    // floored at zero.
    yield_reset();
    ytile(X, Y).bit = BIT_FUNGUS;
    ytile(X, Y).climate = YALT_SHELF;
    YCHECK(mine_yield(YFACTION, 0, X, Y, 0) == 5);
    yield_reset();
    ytile(X, Y).bit = BIT_FUNGUS;
    g_yield_world.players_data[YFACTION].tech_fungus_mineral = 1;
    g_yield_world.players_data[YFACTION].soc_effect_pending.planet = -3;
    YCHECK(mine_yield(YFACTION, 0, X, Y, 0) == 0);

    // The ocean arm reaches the clip, like the borehole and land arms.
    yield_reset();
    g_yield_world.tech_achieved[YTECH_THREE_MINERALS] = 0;
    ytile(X, Y).climate = YALT_SHELF;
    ytile(X, Y).bit = BIT_MINE;
    YCHECK(mine_yield(YFACTION, 0, X, Y, 0) == 2);
    YCHECK(g_yield_world.restricted == 20);

    // The clip is strictly above two.
    yield_reset();
    g_yield_world.tech_achieved[YTECH_THREE_MINERALS] = 0;
    ytile(X, Y).val3 = 0x40;                             // one mineral
    YCHECK(mine_yield(YFACTION, 0, X, Y, 0) == 1);
    YCHECK(g_yield_world.restricted == 0);
    ytile(X, Y).bit = BIT_MINE;                          // two: 1 terrain + 1 extra
    YCHECK(mine_yield(YFACTION, 0, X, Y, 0) == 2);
    YCHECK(g_yield_world.restricted == 0);

    // ---- the base events ----------------------------------------------------
    yield_reset();
    ytile(X, Y).val3 = 0x40;                             // one mineral
    g_yield_world.bases[1].event = BEVENT_INDUSTRY;
    YCHECK(mine_yield(YFACTION, 0, X, Y, 0) == 2);
    g_yield_world.bases[1].event = BEVENT_BUST;
    YCHECK(mine_yield(YFACTION, 0, X, Y, 0) == 0);
    g_yield_world.bases[1].event = BEVENT_INDUSTRY | BEVENT_BUST;
    YCHECK(mine_yield(YFACTION, 0, X, Y, 0) == 1);
    ytile(X, Y).val3 = 0;                                // nothing to take
    g_yield_world.bases[1].event = BEVENT_BUST;
    YCHECK(mine_yield(YFACTION, 0, X, Y, 0) == 0);
    YCHECK(mine_yield(YFACTION, -1, X, Y, 0) == 0);
}

void test_energy_yield() {
    YieldSeams seams;
    const int X = 2;
    const int Y = 2;

    // ---- plain land: the altitude above level two --------------------------
    // Nothing collects it without a solar collector, an echelon mirror, a
    // borehole, or the preview flag.
    yield_reset();
    ytile(X, Y).climate = 0xA0;                          // alt 5
    YCHECK(energy_yield(YFACTION, 0, X, Y, 0) == 0);
    YCHECK(energy_yield(YFACTION, 0, X, Y, 1) == 3);     // preview
    ytile(X, Y).bit = BIT_SOLAR_TIDAL;
    YCHECK(energy_yield(YFACTION, 0, X, Y, 0) == 3);
    ytile(X, Y).bit = BIT_ECH_MIRROR;
    YCHECK(energy_yield(YFACTION, 0, X, Y, 0) == 3);
    ytile(X, Y).bit = BIT_THERMAL_BORE;                  // its own arm, in fact
    YCHECK(energy_yield(YFACTION, 0, X, Y, 0) == 27);    // BOREHOLE.energy
    // The altitude term floors at zero rather than going negative.
    yield_reset();
    ytile(X, Y).climate = YALT_LAND;                     // alt 3
    ytile(X, Y).bit = BIT_SOLAR_TIDAL;
    YCHECK(energy_yield(YFACTION, 0, X, Y, 0) == 1);
    ytile(X, Y).climate = 0x60 | 0x1F;                   // still alt 3
    YCHECK(energy_yield(YFACTION, 0, X, Y, 0) == 1);
    ytile(X, Y).climate = 0xE0;                          // alt 7
    YCHECK(energy_yield(YFACTION, 0, X, Y, 0) == 5);

    // ---- echelon mirrors reflect onto a solar collector ---------------------
    // Each neighbour that carries a mirror AND whose territory names the
    // CURRENT base's faction adds one. RadiusBaseX/Y walks the eight tiles
    // around (x, y).
    yield_reset();
    ytile(X, Y).climate = YALT_LAND;                     // one solar unit
    ytile(X, Y).bit = BIT_SOLAR_TIDAL;
    g_yield_world.bases[1].faction_id_current = 4;
    ytile(4, 2).bit = BIT_ECH_MIRROR;                    // (2,2)+(2,0)
    ytile(4, 2).territory = 4;
    YCHECK(energy_yield(YFACTION, 0, X, Y, 0) == 2);
    ytile(4, 2).territory = 5;                           // somebody else's
    YCHECK(energy_yield(YFACTION, 0, X, Y, 0) == 1);
    ytile(4, 2).territory = 4;
    ytile(4, 2).bit = 0;                                 // no mirror
    YCHECK(energy_yield(YFACTION, 0, X, Y, 0) == 1);
    // Two of them, and a third outside the ring that must not count.
    ytile(4, 2).bit = BIT_ECH_MIRROR;
    ytile(2, 4).bit = BIT_ECH_MIRROR;                    // (2,2)+(0,2)
    ytile(2, 4).territory = 4;
    ytile(8, 2).bit = BIT_ECH_MIRROR;                    // four columns away
    ytile(8, 2).territory = 4;
    YCHECK(energy_yield(YFACTION, 0, X, Y, 0) == 3);
    // The scan needs the collector: a mirror alone reflects nothing.
    ytile(X, Y).bit = BIT_ECH_MIRROR;
    YCHECK(energy_yield(YFACTION, 0, X, Y, 0) == 1);
    // Off-map neighbours are skipped rather than read.
    yield_reset();
    ytile(0, 0).climate = YALT_LAND;
    ytile(0, 0).bit = BIT_SOLAR_TIDAL;
    g_yield_world.bases[1].faction_id_current = 4;
    ytile(2, 0).bit = BIT_ECH_MIRROR;
    ytile(2, 0).territory = 4;
    YCHECK(energy_yield(YFACTION, 0, 0, 0, 0) == 2);

    // ---- forest -------------------------------------------------------------
    yield_reset();
    ytile(X, Y).bit = BIT_FOREST;
    YCHECK(energy_yield(YFACTION, 0, X, Y, 0) == 12);    // FOREST_SQ.energy
    yield_give_fac(g_yield_world.bases[1], FAC_HYBRID_FOREST);
    YCHECK(energy_yield(YFACTION, 0, X, Y, 0) == 13);
    YCHECK(energy_yield(YFACTION, -1, X, Y, 0) == 12);   // guarded, unlike crop's

    // ---- ocean --------------------------------------------------------------
    yield_reset();
    ytile(X, Y).climate = YALT_SHELF;
    YCHECK(energy_yield(YFACTION, 0, X, Y, 0) == 3);     // OCEAN_SQ.energy
    ytile(X, Y).bit = BIT_SOLAR_TIDAL;                   // tidal harness
    YCHECK(energy_yield(YFACTION, 0, X, Y, 0) == 3 + 21);
    yield_give_fac(g_yield_world.bases[1], FAC_THERMOCLINE_TRANSDUCER);
    YCHECK(energy_yield(YFACTION, 0, X, Y, 0) == 3 + 21 + 1);
    yield_reset();
    ytile(X, Y).climate = YALT_SHELF;
    YCHECK(energy_yield(YFACTION, 0, X, Y, 1) == 3 + 21);  // preview
    // Deep ocean produces nothing at all without SMACX - not even the base
    // ocean energy, unlike the crop and mineral arms.
    yield_reset();
    ytile(X, Y).climate = YALT_DEEP;
    ytile(X, Y).bit = BIT_SOLAR_TIDAL;
    YCHECK(energy_yield(YFACTION, 0, X, Y, 0) == 0);
    g_yield_world.expansion = 1;
    YCHECK(energy_yield(YFACTION, 0, X, Y, 0) == 3 + 21);
    // Sea fungus - as forest on an ocean tile - costs one.
    yield_reset();
    ytile(X, Y).climate = YALT_SHELF;
    ytile(X, Y).bit = BIT_FOREST;
    YCHECK(energy_yield(YFACTION, 0, X, Y, 0) == 2);
    // The transducer read is unguarded for base_id -1.
    yield_reset();
    ytile(X, Y).climate = YALT_SHELF;
    ytile(X, Y).bit = BIT_SOLAR_TIDAL;
    yield_give_fac(g_yield_world.bases[0], FAC_THERMOCLINE_TRANSDUCER);
    YCHECK(energy_yield(YFACTION, -1, X, Y, 0) == 3 + 21 + 1);

    // ---- a monolith ---------------------------------------------------------
    yield_reset();
    ytile(X, Y).bit = BIT_MONOLITH;
    YCHECK(energy_yield(YFACTION, 0, X, Y, 0) == 24);    // MONOLITH.energy
    // Manifold Harmonics gives one at PLANET 0 and another at PLANET 3.
    g_yield_world.projects.manifold_harmonics = 0;
    g_yield_world.bases[1].faction_id_current = YFACTION;
    YCHECK(energy_yield(YFACTION, 0, X, Y, 0) == 25);
    g_yield_world.players_data[YFACTION].soc_effect_pending.planet = -1;
    YCHECK(energy_yield(YFACTION, 0, X, Y, 0) == 24);
    g_yield_world.players_data[YFACTION].soc_effect_pending.planet = 2;
    YCHECK(energy_yield(YFACTION, 0, X, Y, 0) == 25);
    g_yield_world.players_data[YFACTION].soc_effect_pending.planet = 3;
    YCHECK(energy_yield(YFACTION, 0, X, Y, 0) == 26);
    // The monolith arm is tested before the borehole arm.
    yield_reset();
    ytile(X, Y).bit = BIT_MONOLITH | BIT_THERMAL_BORE;
    YCHECK(energy_yield(YFACTION, 0, X, Y, 0) == 24);

    // ---- fungus skips the whole shared tail --------------------------------
    yield_reset();
    ytile(X, Y).bit = BIT_FUNGUS;
    YCHECK(energy_yield(YFACTION, 0, X, Y, 0) == 6);     // tech_fungus_energy
    // Like mine_yield and unlike crop_yield, this arm has no preview
    // suppressor: 0x004E7A2F tests only BIT_FUNGUS and the altitude.
    YCHECK(energy_yield(YFACTION, 0, X, Y, 1) == 6);
    // A river, a resource bonus, a landmark and the Merchant Exchange all reach
    // every other arm and none of them reaches this one.
    yield_reset();
    ytile(X, Y).bit = BIT_FUNGUS | BIT_RIVER | YBONUS_ENERGY;
    ytile(X, Y).bit2 = BIT2_URANIUM;
    g_yield_world.projects.merchant_exchange = 0;
    g_yield_world.players_data[YFACTION].soc_effect_pending.economy = 4;
    YCHECK(energy_yield(YFACTION, 0, X, Y, 0) == 6);
    // The identical square without the fungus collects all four.
    ytile(X, Y).bit = BIT_RIVER | YBONUS_ENERGY | BIT_SOLAR_TIDAL;
    YCHECK(energy_yield(YFACTION, 0, X, Y, 0) == 1 + 1 + 9 + 1 + 1 + 1);
    // Manifold Harmonics still applies inside the fungus arm.
    yield_reset();
    ytile(X, Y).bit = BIT_FUNGUS;
    g_yield_world.projects.manifold_harmonics = 0;
    g_yield_world.bases[1].faction_id_current = YFACTION;
    YCHECK(energy_yield(YFACTION, 0, X, Y, 0) == 7);
    g_yield_world.players_data[YFACTION].soc_effect_pending.planet = 3;
    YCHECK(energy_yield(YFACTION, 0, X, Y, 0) == 8);
    // The base events, the dust cloud and the tripling are AFTER the skip, so
    // they still apply to a fungus square.
    yield_reset();
    ytile(X, Y).bit = BIT_FUNGUS;
    g_yield_world.bases[1].event = BEVENT_HEAT_WAVE;
    YCHECK(energy_yield(YFACTION, 0, X, Y, 0) == 7);

    // ---- the shared tail ----------------------------------------------------
    yield_reset();
    ytile(X, Y).bit = BIT_FOREST | BIT_RIVER;
    YCHECK(energy_yield(YFACTION, 0, X, Y, 0) == 13);
    ytile(X, Y).climate = YALT_SHELF;                    // a river at sea: no
    YCHECK(energy_yield(YFACTION, 0, X, Y, 0) == 2);
    yield_reset();
    ytile(X, Y).bit = BIT_FOREST | YBONUS_ENERGY;
    YCHECK(energy_yield(YFACTION, 0, X, Y, 0) == 12 + 9);
    ytile(X, Y).bit = BIT_FOREST | YBONUS_MINERAL;       // wrong bonus id
    YCHECK(energy_yield(YFACTION, 0, X, Y, 0) == 12);
    // The energy landmark set: Volcano (capped at code 9), Uranium, Geothermal
    // and Ridge, with Crater, Fossil, Canyon and Jungle belonging elsewhere.
    yield_reset();
    ytile(X, Y).bit = BIT_FOREST;
    ytile(X, Y).bit2 = BIT2_VOLCANO;
    YCHECK(energy_yield(YFACTION, 0, X, Y, 0) == 13);
    ytile(X, Y).bit2 = BIT2_VOLCANO | 0x09000000u;
    YCHECK(energy_yield(YFACTION, 0, X, Y, 0) == 12);
    ytile(X, Y).bit2 = BIT2_URANIUM;
    YCHECK(energy_yield(YFACTION, 0, X, Y, 0) == 13);
    ytile(X, Y).bit2 = BIT2_URANIUM | 0x7F000000u;       // uncapped
    YCHECK(energy_yield(YFACTION, 0, X, Y, 0) == 13);
    ytile(X, Y).bit2 = BIT2_GEOTHERMAL;
    YCHECK(energy_yield(YFACTION, 0, X, Y, 0) == 13);
    ytile(X, Y).bit2 = BIT2_RIDGE;
    YCHECK(energy_yield(YFACTION, 0, X, Y, 0) == 13);
    ytile(X, Y).bit2 = BIT2_CRATER;
    YCHECK(energy_yield(YFACTION, 0, X, Y, 0) == 12);
    ytile(X, Y).bit2 = BIT2_FOSSIL;
    YCHECK(energy_yield(YFACTION, 0, X, Y, 0) == 12);
    ytile(X, Y).bit2 = BIT2_CANYON;
    YCHECK(energy_yield(YFACTION, 0, X, Y, 0) == 12);
    ytile(X, Y).bit2 = BIT2_URANIUM | BIT2_UNK_80000000;
    YCHECK(energy_yield(YFACTION, 0, X, Y, 0) == 12);
    ytile(X, Y).bit2 = BIT2_GEOTHERMAL | BIT2_UNK_80000000;
    YCHECK(energy_yield(YFACTION, 0, X, Y, 0) == 12);
    ytile(X, Y).bit2 = BIT2_RIDGE | BIT2_UNK_80000000;
    YCHECK(energy_yield(YFACTION, 0, X, Y, 0) == 12);
    // The Merchant Exchange pays the base that holds it, and only that base.
    yield_reset();
    ytile(X, Y).bit = BIT_FOREST;
    g_yield_world.projects.merchant_exchange = 0;
    YCHECK(energy_yield(YFACTION, 0, X, Y, 0) == 13);
    YCHECK(energy_yield(YFACTION, 1, X, Y, 0) == 12);
    YCHECK(energy_yield(YFACTION, -1, X, Y, 0) == 12);
    // ECONOMY of two or better pays one more, and a golden age counts toward it.
    yield_reset();
    ytile(X, Y).bit = BIT_FOREST;
    g_yield_world.players_data[YFACTION].soc_effect_pending.economy = 1;
    YCHECK(energy_yield(YFACTION, 0, X, Y, 0) == 12);
    g_yield_world.bases[1].state = BSTATE_GOLDEN_AGE_ACTIVE;
    YCHECK(energy_yield(YFACTION, 0, X, Y, 0) == 13);
    g_yield_world.bases[1].state = 0;
    g_yield_world.players_data[YFACTION].soc_effect_pending.economy = 2;
    YCHECK(energy_yield(YFACTION, 0, X, Y, 0) == 13);

    // ---- the clip to two ----------------------------------------------------
    yield_reset();
    g_yield_world.tech_achieved[YTECH_THREE_ENERGY] = 0;
    ytile(X, Y).bit = BIT_FOREST;
    YCHECK(energy_yield(YFACTION, 0, X, Y, 0) == 2);
    YCHECK(g_yield_world.restricted == 10);
    g_yield_world.restricted = 0;
    ytile(X, Y).bit = BIT_FOREST | YBONUS_ENERGY;        // a bonus suppresses it
    YCHECK(energy_yield(YFACTION, 0, X, Y, 0) == 12 + 9);
    YCHECK(g_yield_world.restricted == 0);
    ytile(X, Y).bit = BIT_FOREST | BIT_CONDENSER;        // a condenser does not
    YCHECK(energy_yield(YFACTION, 0, X, Y, 0) == 2);
    // The gate is Rules->tech_three_energy_sqr.
    yield_reset();
    g_yield_world.tech_achieved[YTECH_THREE_ENERGY] = 0;
    g_yield_world.rules.tech_three_energy_sqr = YTECH_THREE_NUTRIENTS;
    ytile(X, Y).bit = BIT_FOREST;
    YCHECK(energy_yield(YFACTION, 0, X, Y, 0) == 12);

    // ---- a base square publishes instead of clipping ------------------------
    yield_reset();
    g_yield_world.tech_achieved[YTECH_THREE_ENERGY] = 0;
    yield_put_base_in_tile(ytile(X, Y), 3);
    YCHECK(energy_yield(YFACTION, 0, X, Y, 0) == 6);     // BASE_SQ.energy, unclipped
    YCHECK(g_yield_world.base_square_energy == 6);
    YCHECK(g_yield_world.restricted == 0);
    // The publication is clamped to 0..99 while the return is not clamped
    // afterwards - the base event tail still runs on the clamped value.
    yield_reset();
    yield_put_base_in_tile(ytile(X, Y), 3);
    g_yield_world.resource_info[RSCINFO_BASE_SQ].energy = 500;
    YCHECK(energy_yield(YFACTION, 0, X, Y, 0) == 99);
    YCHECK(g_yield_world.base_square_energy == 99);
    yield_reset();
    yield_put_base_in_tile(ytile(X, Y), 3);
    g_yield_world.resource_info[RSCINFO_BASE_SQ].energy = -5;
    YCHECK(energy_yield(YFACTION, 0, X, Y, 0) == 0);
    YCHECK(g_yield_world.base_square_energy == 0);
    // Headquarters is read BEFORE the base_id sign check, so -1 reads the entry
    // below the array and the point is granted anyway.
    yield_reset();
    yield_put_base_in_tile(ytile(X, Y), 3);
    yield_give_fac(g_yield_world.bases[0], FAC_HEADQUARTERS);
    YCHECK(energy_yield(YFACTION, -1, X, Y, 0) == 7);
    yield_reset();
    yield_put_base_in_tile(ytile(X, Y), 3);
    yield_give_fac(g_yield_world.bases[1], FAC_HEADQUARTERS);
    YCHECK(energy_yield(YFACTION, 0, X, Y, 0) == 7);
    // Recycling Tanks and the Pressure Dome, and only with a real base id.
    yield_reset();
    yield_put_base_in_tile(ytile(X, Y), 3);
    yield_give_fac(g_yield_world.bases[1], FAC_RECYCLING_TANKS);
    YCHECK(energy_yield(YFACTION, 0, X, Y, 0) == 6 + 15);
    yield_reset();
    yield_put_base_in_tile(ytile(X, Y), 3);
    yield_give_fac(g_yield_world.bases[1], FAC_PRESSURE_DOME);
    YCHECK(energy_yield(YFACTION, 0, X, Y, 0) == 6 + 15);
    yield_reset();
    yield_put_base_in_tile(ytile(X, Y), 3);
    yield_give_fac(g_yield_world.bases[0], FAC_RECYCLING_TANKS);
    YCHECK(energy_yield(YFACTION, -1, X, Y, 0) == 6);
    // The elected Planetary Governor earns one more on its own base square.
    yield_reset();
    yield_put_base_in_tile(ytile(X, Y), 3);
    g_yield_world.governor = YFACTION;
    YCHECK(energy_yield(YFACTION, 0, X, Y, 0) == 7);
    YCHECK(energy_yield(2, 0, X, Y, 0) == 6);

    // ---- the ECONOMY curve on a base square ---------------------------------
    // Positive: 2 and 3 and 4 map to 0, 2 and 4, and everything above 4 is held
    // at 4. Below 2 nothing is added.
    yield_reset();
    yield_put_base_in_tile(ytile(X, Y), 3);
    for (int economy = 0; economy <= 6; economy++) {
        g_yield_world.players_data[YFACTION].soc_effect_pending.economy = economy;
        int transformed = economy;
        if (economy > 1) {
            transformed = ((economy > 4) ? 4 : economy) * 2 - 4;
        }
        // The shared tail adds its own step on the RAW rating, not the
        // transformed one.
        int expected = 6 + transformed + ((economy >= 2) ? 1 : 0);
        YCHECK(energy_yield(YFACTION, 0, X, Y, 0) == expected);
    }
    // Negative: it is pulled back toward zero once, unless the base has a
    // Headquarters, is not worse than -1, and is not the faction's only base.
    yield_reset();
    yield_put_base_in_tile(ytile(X, Y), 3);
    g_yield_world.players_data[YFACTION].soc_effect_pending.economy = -1;
    YCHECK(energy_yield(YFACTION, 0, X, Y, 0) == 6 + 0);      // no HQ: -1 -> 0
    yield_give_fac(g_yield_world.bases[1], FAC_HEADQUARTERS);
    YCHECK(energy_yield(YFACTION, 0, X, Y, 0) == 6 + 1 - 1);  // HQ holds it at -1
    g_yield_world.players_data[YFACTION].current_num_bases = 1;
    YCHECK(energy_yield(YFACTION, 0, X, Y, 0) == 6 + 1 + 0);  // the only base
    g_yield_world.players_data[YFACTION].current_num_bases = 3;
    g_yield_world.players_data[YFACTION].soc_effect_pending.economy = -2;
    YCHECK(energy_yield(YFACTION, 0, X, Y, 0) == 6 + 1 - 1);  // worse than -1
    // A golden age counts toward the curve's input as well as the tail's.
    yield_reset();
    yield_put_base_in_tile(ytile(X, Y), 3);
    g_yield_world.players_data[YFACTION].soc_effect_pending.economy = 1;
    g_yield_world.bases[1].state = BSTATE_GOLDEN_AGE_ACTIVE;
    YCHECK(energy_yield(YFACTION, 0, X, Y, 0) == 6 + 0 + 1);

    // ---- the landmark code is the TOP BYTE of bit2 -------------------------
    // Shifted by anything less it would carry the landmark bits themselves into
    // the comparison and put every Volcano over the cap.
    yield_reset();
    ytile(X, Y).bit = BIT_FOREST;
    ytile(X, Y).bit2 = BIT2_VOLCANO | 0x01000000u;
    YCHECK(energy_yield(YFACTION, 0, X, Y, 0) == 13);

    // Faction 0's base square is a base square here too.
    yield_reset();
    yield_put_base_in_tile(ytile(X, Y), 0);
    YCHECK(energy_yield(YFACTION, 0, X, Y, 0) == 6);

    // ---- the fungus arm's altitude floor and PLANET arithmetic -------------
    yield_reset();
    ytile(X, Y).bit = BIT_FUNGUS;
    ytile(X, Y).climate = YALT_SHELF;                    // exactly the shelf
    YCHECK(energy_yield(YFACTION, 0, X, Y, 0) == 6);
    yield_reset();
    ytile(X, Y).bit = BIT_FUNGUS;
    g_yield_world.players_data[YFACTION].soc_effect_pending.planet = -2;
    YCHECK(energy_yield(YFACTION, 0, X, Y, 0) == 4);
    g_yield_world.players_data[YFACTION].soc_effect_pending.planet = -9;
    YCHECK(energy_yield(YFACTION, 0, X, Y, 0) == 3);     // floored at -3
    g_yield_world.players_data[YFACTION].soc_effect_pending.planet = 1;
    YCHECK(energy_yield(YFACTION, 0, X, Y, 0) == 6);     // above 0 subtracts nothing
    g_yield_world.players_data[YFACTION].tech_fungus_energy = 1;
    g_yield_world.players_data[YFACTION].soc_effect_pending.planet = -3;
    YCHECK(energy_yield(YFACTION, 0, X, Y, 0) == 0);     // and the sum floors at 0

    // ---- the clip is strictly above two -------------------------------------
    yield_reset();
    g_yield_world.tech_achieved[YTECH_THREE_ENERGY] = 0;
    ytile(X, Y).climate = YALT_LAND;
    ytile(X, Y).bit = BIT_SOLAR_TIDAL;                   // one energy
    YCHECK(energy_yield(YFACTION, 0, X, Y, 0) == 1);
    YCHECK(g_yield_world.restricted == 0);
    ytile(X, Y).climate = 0x80;                          // alt 4: two energy
    YCHECK(energy_yield(YFACTION, 0, X, Y, 0) == 2);
    YCHECK(g_yield_world.restricted == 0);

    // ---- the echelon mirror ring, tile by tile -----------------------------
    // The ring is RadiusBaseX/Y[0..7]. Step 8 is the collector's own tile and
    // must NOT be walked, and step 0 must be.
    yield_reset();
    ytile(X, Y).climate = YALT_LAND;
    ytile(X, Y).bit = BIT_SOLAR_TIDAL | BIT_ECH_MIRROR;
    ytile(X, Y).territory = 4;
    g_yield_world.bases[1].faction_id_current = 4;
    YCHECK(energy_yield(YFACTION, 0, X, Y, 0) == 1);     // it does not see itself
    yield_reset();
    ytile(X, Y).climate = YALT_LAND;
    ytile(X, Y).bit = BIT_SOLAR_TIDAL;
    g_yield_world.bases[1].faction_id_current = 4;
    ytile(3, 1).bit = BIT_ECH_MIRROR;                    // step 0 is (+1,-1)
    ytile(3, 1).territory = 4;
    YCHECK(energy_yield(YFACTION, 0, X, Y, 0) == 2);
    // Column zero is on the map, so a mirror there counts.
    yield_reset();
    ytile(X, Y).climate = YALT_LAND;
    ytile(X, Y).bit = BIT_SOLAR_TIDAL;
    g_yield_world.bases[1].faction_id_current = 4;
    ytile(0, 2).bit = BIT_ECH_MIRROR;                    // step 5 is (-2,0)
    ytile(0, 2).territory = 4;
    YCHECK(energy_yield(YFACTION, 0, X, Y, 0) == 2);

    // Column zero counts on a ROUND map too: the wrap is applied only to a
    // NEGATIVE column, so a neighbour that lands exactly on zero is left where
    // it is rather than pushed off the right-hand edge.
    yield_reset();
    g_yield_world.is_flat = 0;
    ytile(X, Y).climate = YALT_LAND;
    ytile(X, Y).bit = BIT_SOLAR_TIDAL;
    g_yield_world.bases[1].faction_id_current = 4;
    ytile(0, 2).bit = BIT_ECH_MIRROR;                    // step 5 is (-2,0)
    ytile(0, 2).territory = 4;
    YCHECK(energy_yield(YFACTION, 0, X, Y, 0) == 2);

    // On a flat map a neighbour off the left edge is skipped, NOT wrapped
    // round to the far column.
    yield_reset();
    ytile(0, 2).climate = YALT_LAND;
    ytile(0, 2).bit = BIT_SOLAR_TIDAL;
    g_yield_world.bases[1].faction_id_current = 4;
    ytile(14, 2).bit = BIT_ECH_MIRROR;                   // where (-2,0) would land
    ytile(14, 2).territory = 4;
    YCHECK(energy_yield(YFACTION, 0, 0, 2, 0) == 1);
    // On a round map it is wrapped, from both edges.
    g_yield_world.is_flat = 0;
    YCHECK(energy_yield(YFACTION, 0, 0, 2, 0) == 2);
    yield_reset();
    g_yield_world.is_flat = 0;
    ytile(14, 2).climate = YALT_LAND;
    ytile(14, 2).bit = BIT_SOLAR_TIDAL;
    g_yield_world.bases[1].faction_id_current = 4;
    ytile(0, 2).bit = BIT_ECH_MIRROR;                    // 14 + 2 == 16 wraps to 0
    ytile(0, 2).territory = 4;
    YCHECK(energy_yield(YFACTION, 0, 14, 2, 0) == 2);
    // The flat test is `MapIsFlat & 1`, so an even flag is round.
    yield_reset();
    g_yield_world.is_flat = 2;
    ytile(0, 2).climate = YALT_LAND;
    ytile(0, 2).bit = BIT_SOLAR_TIDAL;
    g_yield_world.bases[1].faction_id_current = 4;
    ytile(14, 2).bit = BIT_ECH_MIRROR;
    ytile(14, 2).territory = 4;
    YCHECK(energy_yield(YFACTION, 0, 0, 2, 0) == 2);

    // The two bounds are exclusive. Both are shrunk below the fixture's tile
    // array here, so the tile one step past each edge exists and can be shown
    // NOT to be consulted.
    yield_reset();
    g_yield_world.lat_bounds = 6;
    ytile(X, 4).climate = YALT_LAND;
    ytile(X, 4).bit = BIT_SOLAR_TIDAL;
    g_yield_world.bases[1].faction_id_current = 4;
    ytile(X, 6).bit = BIT_ECH_MIRROR;                    // step 3 is (0,+2)
    ytile(X, 6).territory = 4;
    YCHECK(energy_yield(YFACTION, 0, X, 4, 0) == 1);
    g_yield_world.lat_bounds = 8;
    YCHECK(energy_yield(YFACTION, 0, X, 4, 0) == 2);
    yield_reset();
    g_yield_world.lon_bounds = 12;
    ytile(10, 2).climate = YALT_LAND;
    ytile(10, 2).bit = BIT_SOLAR_TIDAL;
    g_yield_world.bases[1].faction_id_current = 4;
    ytile(12, 2).bit = BIT_ECH_MIRROR;                   // step 1 is (+2,0)
    ytile(12, 2).territory = 4;
    YCHECK(energy_yield(YFACTION, 0, 10, 2, 0) == 1);
    g_yield_world.lon_bounds = 16;
    YCHECK(energy_yield(YFACTION, 0, 10, 2, 0) == 2);

    // ---- the base events, the dust cloud and the tripling ------------------
    yield_reset();
    ytile(X, Y).bit = BIT_FOREST;
    g_yield_world.bases[1].event = BEVENT_HEAT_WAVE;
    YCHECK(energy_yield(YFACTION, 0, X, Y, 0) == 13);
    g_yield_world.bases[1].event = BEVENT_CLOUD_COVER;
    YCHECK(energy_yield(YFACTION, 0, X, Y, 0) == 11);
    g_yield_world.bases[1].event = BEVENT_HEAT_WAVE | BEVENT_CLOUD_COVER;
    YCHECK(energy_yield(YFACTION, 0, X, Y, 0) == 12);
    yield_reset();
    g_yield_world.bases[1].event = BEVENT_CLOUD_COVER;
    YCHECK(energy_yield(YFACTION, 0, X, Y, 0) == 0);     // nothing to take
    yield_reset();
    ytile(X, Y).bit = BIT_FOREST;
    g_yield_world.dust_cloud = 1;
    YCHECK(energy_yield(YFACTION, 0, X, Y, 0) == 11);
    yield_reset();
    g_yield_world.dust_cloud = 1;
    YCHECK(energy_yield(YFACTION, 0, X, Y, 0) == 0);
    // The tripling consults bit 1 of the state when the selector is set, and
    // bit 0 when it is clear.
    yield_reset();
    ytile(X, Y).bit = BIT_FOREST;
    g_yield_world.energy_event = 1;
    YCHECK(energy_yield(YFACTION, 0, X, Y, 0) == 36);
    g_yield_world.energy_event = 2;
    YCHECK(energy_yield(YFACTION, 0, X, Y, 0) == 12);
    g_yield_world.energy_event_selector = 1;
    YCHECK(energy_yield(YFACTION, 0, X, Y, 0) == 36);
    g_yield_world.energy_event = 1;
    YCHECK(energy_yield(YFACTION, 0, X, Y, 0) == 12);
    g_yield_world.energy_event = 3;
    YCHECK(energy_yield(YFACTION, 0, X, Y, 0) == 36);
    g_yield_world.energy_event = 0;
    YCHECK(energy_yield(YFACTION, 0, X, Y, 0) == 12);
    // The tripling comes last: it multiplies what the dust cloud left.
    yield_reset();
    ytile(X, Y).bit = BIT_FOREST;
    g_yield_world.dust_cloud = 1;
    g_yield_world.energy_event = 1;
    YCHECK(energy_yield(YFACTION, 0, X, Y, 0) == 33);
}

#undef YCHECK

/*
 * A self-contained world for base_support.
 *
 * Everything the body and its five callees - crop_yield, mine_yield,
 * energy_yield, has_abil, whose_territory - reach is a pointer global, so
 * ScopedSeam gives the world local storage and no fixed address is touched.
 *
 * Four things about the fixture are load-bearing:
 *
 *  - `Bases` is seamed to &bases[1] and `BaseCurrent` to the same entry, so
 *    that has_fac_built(FAC_BROOD_PIT, *BaseIDCurrentSelected) and the yield
 *    functions all see one base and bases[0] absorbs a -1 index.
 *  - The two convoy accumulators are 256 entries, not 4. The body indexes them
 *    with an unchecked `order_auto_type`, and one of its arms - a supply convoy
 *    whose auto type is above RSC_PSI - is only reachable with an index past
 *    the end of the real four-entry arrays.
 *  - Weapon 0 is unarmed, weapon 1 conventional and weapon 2 psi, because three
 *    separate decisions turn on the SIGN of the offense rating and a fourth on
 *    it being non-zero.
 *  - The harvest tile (4, 4) is deliberately not the base tile (2, 2): a convoy
 *    standing on a base tile takes the other branch.
 */
const int SFACTION = 1;
const int SUP_PROTO_COUNT = 96;

const int SUP_WPN_NONE = 0;   // offense 0
const int SUP_WPN_GUN = 1;    // offense 4
const int SUP_WPN_PSI = 2;    // offense -1
const int SUP_WPN_WEAK = 3;   // offense 1, the boundary of "offense > 0"

const int SUP_CH_LAND = 0;
const int SUP_CH_SEA = 1;
const int SUP_CH_AIR = 2;

struct SupportWorld {
    Map tiles[64];
    Base bases[6];
    PlayerData players_data[9];
    Player players[9];
    Veh vehs[16];
    VehPrototype protos[SUP_PROTO_COUNT];
    RulesWeapon weapons[8];
    RulesChassis chassis[4];
    RulesResourceinfo resource_info[MaxResourceInfoNum];
    RulesBasic rules;
    BaseSecretProject projects;
    RulesTechnology technology[MaxTechnologyNum];
    uint8_t tech_achieved[MaxTechnologyNum];
    uint32_t convoy_to[256];
    uint32_t convoy_from[256];
    uint32_t pacifism_count;
    uint32_t maint_count;
    uint32_t forces_supported;
    uint32_t maint_cost;
    uint32_t upkeep_stage;
    Map *tiles_ptr;
    Base *base_current;
    int base_id_selected;
    int veh_count;
    uint32_t longitude;
    int lon_bounds;
    int lat_bounds;
    BOOL is_flat;
    uint32_t map_rand_seed;
    uint32_t game_rules;
    uint32_t game_state;
    BOOL expansion;
    int dust_cloud;
    int restricted;
    int base_square_energy;
    int governor;
    int energy_event;
    int energy_event_selector;
};

SupportWorld g_support_world;

const int STECH_THREE_NUTRIENTS = 1;
const int STECH_THREE_MINERALS = 2;
const int STECH_THREE_ENERGY = 3;
const int STECH_MINING_PLATFORM = 4;

Map &stile(int x, int y) { return g_support_world.tiles[(x >> 1) + y * 8]; }

void support_give_fac(Base &base, uint32_t facility) {
    int offset;
    int mask;
    bitmask(facility, &offset, &mask);
    base.facilities_built[offset] |= (uint8_t)mask;
}

void support_reset() {
    std::memset(&g_support_world, 0, sizeof(g_support_world));
    std::memset(&g_support_world.projects, 0xFF, sizeof(g_support_world.projects));
    g_support_world.tiles_ptr = g_support_world.tiles;
    g_support_world.base_current = &g_support_world.bases[1];  // == Bases[0]
    g_support_world.base_id_selected = 0;
    g_support_world.longitude = 8;
    g_support_world.lon_bounds = 16;
    g_support_world.lat_bounds = 8;
    g_support_world.is_flat = 1;
    g_support_world.map_rand_seed = 0;
    g_support_world.governor = -1;
    for (int k = 0; k < 64; k++) {
        g_support_world.tiles[k].climate = 0x60;  // alt 3, land
        g_support_world.tiles[k].territory = -1;  // unclaimed
    }
    for (int i = 0; i < MaxResourceInfoNum; i++) {
        g_support_world.resource_info[i].nutrients = 3 * i + 1;
        g_support_world.resource_info[i].minerals = 3 * i + 2;
        g_support_world.resource_info[i].energy = 3 * i + 3;
    }
    g_support_world.rules.tech_three_nutrients_sqr = STECH_THREE_NUTRIENTS;
    g_support_world.rules.tech_three_minerals_sqr = STECH_THREE_MINERALS;
    g_support_world.rules.tech_three_energy_sqr = STECH_THREE_ENERGY;
    g_support_world.rules.tech_mining_platform_bonus = STECH_MINING_PLATFORM;
    g_support_world.rules.tgl_nutrient_effect_with_mine = -1;
    g_support_world.rules.limit_mineral_mine_sans_road = 2;
    g_support_world.tech_achieved[STECH_THREE_NUTRIENTS] = 1 << SFACTION;
    g_support_world.tech_achieved[STECH_THREE_MINERALS] = 1 << SFACTION;
    g_support_world.tech_achieved[STECH_THREE_ENERGY] = 1 << SFACTION;
    g_support_world.bases[1].faction_id_current = SFACTION;
    g_support_world.bases[1].x = 2;
    g_support_world.bases[1].y = 2;
    g_support_world.bases[1].population_size = 1;
    g_support_world.weapons[SUP_WPN_NONE].offense_rating = 0;
    g_support_world.weapons[SUP_WPN_GUN].offense_rating = 4;
    g_support_world.weapons[SUP_WPN_PSI].offense_rating = -1;
    g_support_world.weapons[SUP_WPN_WEAK].offense_rating = 1;
    g_support_world.chassis[SUP_CH_LAND].triad = TRIAD_LAND;
    g_support_world.chassis[SUP_CH_SEA].triad = TRIAD_SEA;
    g_support_world.chassis[SUP_CH_AIR].triad = TRIAD_AIR;
}

// A prototype with a plan, a weapon and a chassis; everything else stays zero.
VehPrototype &sup_proto(int proto_id, uint32_t plan, int weapon_id, int chassis_id) {
    VehPrototype &proto = g_support_world.protos[proto_id];
    proto.plan = (uint8_t)plan;
    proto.weapon_id = (uint8_t)weapon_id;
    proto.chassis_id = (uint8_t)chassis_id;
    return proto;
}

// A unit of the subject faction, homed at the current base, at (x, y).
Veh &sup_veh(int proto_id, int x, int y) {
    Veh &veh = g_support_world.vehs[g_support_world.veh_count++];
    veh.faction_id = (uint8_t)SFACTION;
    veh.proto_id = (int16_t)proto_id;
    veh.x = (int16_t)x;
    veh.y = (int16_t)y;
    veh.home_base_id = 0;
    return veh;
}

void sup_put_base_in_tile(Map &tile, int owner) {
    tile.bit |= BIT_BASE_IN_TILE;
    tile.val2 = (uint8_t)((tile.val2 & 0xF0) | (owner & 0xF));
}

#define SCHECK(cond)                                                          \
    do {                                                                      \
        const bool support_ok = (cond);                                       \
        if (!support_ok) {                                                    \
            std::fprintf(stderr, "base_support: line %d: %s\n", __LINE__,     \
                         #cond);                                              \
        }                                                                     \
        expect(support_ok);                                                   \
    } while (0)

class SupportSeams {
 public:
    SupportSeams()
        : tiles_(&MapTiles, &g_support_world.tiles_ptr),
          longitude_(&MapLongitude, &g_support_world.longitude),
          lon_(&MapLongitudeBounds, &g_support_world.lon_bounds),
          lat_(&MapLatitudeBounds, &g_support_world.lat_bounds),
          flat_(&MapIsFlat, &g_support_world.is_flat),
          seed_(&MapRandSeed, &g_support_world.map_rand_seed),
          rules_gate_(&GameRules, &g_support_world.game_rules),
          state_(&GameState, &g_support_world.game_state),
          bases_(&Bases, &g_support_world.bases[1]),
          base_current_(&BaseCurrent, &g_support_world.base_current),
          base_selected_(&BaseIDCurrentSelected, &g_support_world.base_id_selected),
          upkeep_(&BaseUpkeepStage, &g_support_world.upkeep_stage),
          convoy_to_(&BaseCurrentConvoyTo, g_support_world.convoy_to),
          convoy_from_(&BaseCurrentConvoyFrom, g_support_world.convoy_from),
          pacifism_(&BaseCurrentVehPacifismCount, &g_support_world.pacifism_count),
          maint_count_(&BaseCurrentForcesMaintCount, &g_support_world.maint_count),
          supported_(&BaseCurrentForcesSupported, &g_support_world.forces_supported),
          maint_cost_(&BaseCurrentForcesMaintCost, &g_support_world.maint_cost),
          projects_(&SecretProject, &g_support_world.projects),
          resource_(&ResourceInfo, g_support_world.resource_info),
          rules_(&Rules, &g_support_world.rules),
          players_data_(&PlayersData, g_support_world.players_data),
          players_(&Players, g_support_world.players),
          technology_(&Technology, g_support_world.technology),
          achieved_(&GameTechAchieved, g_support_world.tech_achieved),
          expansion_(&ExpansionEnabled, &g_support_world.expansion),
          dust_(&DustCloudDuration, &g_support_world.dust_cloud),
          restricted_(&TileYieldRestricted, &g_support_world.restricted),
          base_energy_(&BaseSquareEnergy, &g_support_world.base_square_energy),
          governor_(&GovernorFaction, &g_support_world.governor),
          energy_event_(&GlobalEnergyEventState, &g_support_world.energy_event),
          selector_(&UnkGlobal0093A934, &g_support_world.energy_event_selector),
          vehs_(&Vehs, g_support_world.vehs),
          protos_(&VehPrototypes, g_support_world.protos),
          veh_count_(&VehCurrentCount, &g_support_world.veh_count),
          weapons_(&Weapon, g_support_world.weapons),
          chassis_(&Chassis, g_support_world.chassis) { }

 private:
    ScopedSeam<Map *> tiles_;
    ScopedSeam<uint32_t> longitude_;
    ScopedSeam<int> lon_;
    ScopedSeam<int> lat_;
    ScopedSeam<BOOL> flat_;
    ScopedSeam<uint32_t> seed_;
    ScopedSeam<uint32_t> rules_gate_;
    ScopedSeam<uint32_t> state_;
    ScopedSeam<Base> bases_;
    ScopedSeam<Base *> base_current_;
    ScopedSeam<int> base_selected_;
    ScopedSeam<uint32_t> upkeep_;
    ScopedSeam<uint32_t> convoy_to_;
    ScopedSeam<uint32_t> convoy_from_;
    ScopedSeam<uint32_t> pacifism_;
    ScopedSeam<uint32_t> maint_count_;
    ScopedSeam<uint32_t> supported_;
    ScopedSeam<uint32_t> maint_cost_;
    ScopedSeam<BaseSecretProject> projects_;
    ScopedSeam<RulesResourceinfo> resource_;
    ScopedSeam<RulesBasic> rules_;
    ScopedSeam<PlayerData> players_data_;
    ScopedSeam<Player> players_;
    ScopedSeam<RulesTechnology> technology_;
    ScopedSeam<uint8_t> achieved_;
    ScopedSeam<BOOL> expansion_;
    ScopedSeam<int> dust_;
    ScopedSeam<int> restricted_;
    ScopedSeam<int> base_energy_;
    ScopedSeam<int> governor_;
    ScopedSeam<int> energy_event_;
    ScopedSeam<int> selector_;
    ScopedSeam<Veh> vehs_;
    ScopedSeam<VehPrototype> protos_;
    ScopedSeam<int> veh_count_;
    ScopedSeam<RulesWeapon> weapons_;
    ScopedSeam<RulesChassis> chassis_;
};

/*
 * Give the harvest tile (4, 4) a yield of 2 nutrients, 1 mineral and 3 energy,
 * three different numbers so that the convoy switch cannot pick the wrong one
 * and still agree. The three assertions here are the fixture's own check.
 */
void support_arm_harvest_tile() {
    stile(4, 4).climate = 0xA0 | 0x10;   // alt 5, rainy
    stile(4, 4).val3 = 0x40;             // rolling
    stile(4, 4).bit = BIT_SOLAR_TIDAL;
    SCHECK(crop_yield(SFACTION, 0, 4, 4, false) == 2);
    SCHECK(mine_yield(SFACTION, 0, 4, 4, false) == 1);
    SCHECK(energy_yield(SFACTION, 0, 4, 4, false) == 3);
}

void test_base_support_convoys() {
    SupportSeams seams;

    // ---- the accumulators are cleared, not accumulated ---------------------
    support_reset();
    for (int i = 0; i < 4; i++) {
        g_support_world.convoy_to[i] = 11 + i;
        g_support_world.convoy_from[i] = 21 + i;
    }
    g_support_world.convoy_to[4] = 99;      // one past the four that are cleared
    g_support_world.convoy_from[4] = 98;
    g_support_world.pacifism_count = 5;
    g_support_world.maint_count = 6;
    g_support_world.forces_supported = 7;
    g_support_world.maint_cost = 8;
    base_support();
    for (int i = 0; i < 4; i++) {
        SCHECK(g_support_world.convoy_to[i] == 0);
        SCHECK(g_support_world.convoy_from[i] == 0);
    }
    SCHECK(g_support_world.convoy_to[4] == 99);
    SCHECK(g_support_world.convoy_from[4] == 98);
    SCHECK(g_support_world.pacifism_count == 0);
    SCHECK(g_support_world.maint_count == 0);
    SCHECK(g_support_world.forces_supported == 0);
    SCHECK(g_support_world.maint_cost == 0);

    // ---- the faction of the CURRENT BASE selects the units -----------------
    support_reset();
    support_arm_harvest_tile();
    sup_proto(20, PLAN_SUPPLY_CONVOY, SUP_WPN_NONE, SUP_CH_LAND);
    {
        Veh &veh = sup_veh(20, 4, 4);
        veh.order = ORDER_CONVOY;
        veh.order_auto_type = RSC_NUTRIENTS;
        veh.faction_id = SFACTION + 1;      // someone else's convoy
    }
    base_support();
    SCHECK(g_support_world.convoy_to[RSC_NUTRIENTS] == 0);
    g_support_world.vehs[0].faction_id = SFACTION;
    base_support();
    SCHECK(g_support_world.convoy_to[RSC_NUTRIENTS] == 2);

    // ---- one arm of the switch per resource, and the two that yield zero ---
    support_reset();
    support_arm_harvest_tile();
    sup_proto(20, PLAN_SUPPLY_CONVOY, SUP_WPN_NONE, SUP_CH_LAND);
    {
        Veh &veh = sup_veh(20, 4, 4);
        veh.order = ORDER_CONVOY;
        veh.order_auto_type = RSC_MINERALS;
    }
    base_support();
    SCHECK(g_support_world.convoy_to[RSC_MINERALS] == 1);
    SCHECK(g_support_world.convoy_to[RSC_NUTRIENTS] == 0);
    g_support_world.vehs[0].order_auto_type = RSC_ENERGY;
    base_support();
    SCHECK(g_support_world.convoy_to[RSC_ENERGY] == 3);
    // RSC_PSI yields nothing AND clears the yield-restriction publication.
    g_support_world.vehs[0].order_auto_type = RSC_PSI;
    g_support_world.restricted = 77;
    base_support();
    SCHECK(g_support_world.convoy_to[RSC_PSI] == 0);
    SCHECK(g_support_world.restricted == 0);
    // Above RSC_PSI there is no arm at all: zero is added, and it is added to
    // the entry the auto type names, past the end of the real array.
    g_support_world.vehs[0].order_auto_type = 4;
    g_support_world.convoy_to[4] = 40;
    g_support_world.restricted = 77;
    base_support();
    SCHECK(g_support_world.convoy_to[4] == 40);
    SCHECK(g_support_world.restricted == 77);   // only RSC_PSI clears it

    // ---- what stops a convoy of ours from contributing ---------------------
    support_reset();
    support_arm_harvest_tile();
    sup_proto(20, PLAN_SUPPLY_CONVOY, SUP_WPN_NONE, SUP_CH_LAND);
    sup_proto(21, PLAN_TERRAFORMING, SUP_WPN_NONE, SUP_CH_LAND);
    {
        Veh &veh = sup_veh(20, 4, 4);
        veh.order = ORDER_MOVE_TO;          // not ORDER_CONVOY
        veh.order_auto_type = RSC_NUTRIENTS;
    }
    base_support();
    SCHECK(g_support_world.convoy_to[RSC_NUTRIENTS] == 0);
    g_support_world.vehs[0].order = ORDER_CONVOY;
    g_support_world.vehs[0].proto_id = 21;  // not a supply convoy plan
    base_support();
    SCHECK(g_support_world.convoy_to[RSC_NUTRIENTS] == 0);
    g_support_world.vehs[0].proto_id = 20;
    base_support();
    SCHECK(g_support_world.convoy_to[RSC_NUTRIENTS] == 2);
    // A base standing in the harvest tile suppresses the yield and instead
    // charges the square as a convoy OUT of this base.
    sup_put_base_in_tile(stile(4, 4), 3);
    base_support();
    SCHECK(g_support_world.convoy_to[RSC_NUTRIENTS] == 0);
    SCHECK(g_support_world.convoy_from[RSC_NUTRIENTS] == 1);
    // Owner 0xF is the "unoccupied" marker and does not count as a base.
    sup_put_base_in_tile(stile(4, 4), 0xF);
    base_support();
    SCHECK(g_support_world.convoy_to[RSC_NUTRIENTS] == 2);
    SCHECK(g_support_world.convoy_from[RSC_NUTRIENTS] == 0);

    // ---- a convoy homed elsewhere counts one, and only on the base tile ----
    support_reset();
    support_arm_harvest_tile();
    sup_proto(20, PLAN_SUPPLY_CONVOY, SUP_WPN_NONE, SUP_CH_LAND);
    sup_proto(21, PLAN_TERRAFORMING, SUP_WPN_NONE, SUP_CH_LAND);
    {
        Veh &veh = sup_veh(20, 2, 2);       // the current base's own tile
        veh.order = ORDER_CONVOY;
        veh.order_auto_type = RSC_ENERGY;
        veh.home_base_id = 3;               // homed somewhere else
    }
    base_support();
    SCHECK(g_support_world.convoy_to[RSC_ENERGY] == 1);   // one, not a yield
    SCHECK(g_support_world.convoy_from[RSC_ENERGY] == 0);
    // Each of the four guards, one at a time.
    g_support_world.vehs[0].x = 4;
    base_support();
    SCHECK(g_support_world.convoy_to[RSC_ENERGY] == 0);
    g_support_world.vehs[0].x = 2;
    g_support_world.vehs[0].y = 4;
    base_support();
    SCHECK(g_support_world.convoy_to[RSC_ENERGY] == 0);
    g_support_world.vehs[0].y = 2;
    g_support_world.vehs[0].order = ORDER_HOLD;
    base_support();
    SCHECK(g_support_world.convoy_to[RSC_ENERGY] == 0);
    g_support_world.vehs[0].order = ORDER_CONVOY;
    g_support_world.vehs[0].proto_id = 21;
    base_support();
    SCHECK(g_support_world.convoy_to[RSC_ENERGY] == 0);
    g_support_world.vehs[0].proto_id = 20;
    g_support_world.vehs[0].home_base_id = -1;   // "no home" is not "elsewhere"
    base_support();
    SCHECK(g_support_world.convoy_to[RSC_ENERGY] == 0);
    g_support_world.vehs[0].home_base_id = 3;
    base_support();
    SCHECK(g_support_world.convoy_to[RSC_ENERGY] == 1);
    // A unit homed elsewhere never reaches the support or pacifism sections.
    SCHECK(g_support_world.forces_supported == 0);
    SCHECK(g_support_world.pacifism_count == 0);

    // ---- convoy OUT is counted for our own convoy in any base -------------
    support_reset();
    support_arm_harvest_tile();
    sup_proto(20, PLAN_SUPPLY_CONVOY, SUP_WPN_NONE, SUP_CH_LAND);
    {
        Veh &veh = sup_veh(20, 6, 6);
        veh.order = ORDER_CONVOY;
        veh.order_auto_type = RSC_MINERALS;
    }
    base_support();
    SCHECK(g_support_world.convoy_from[RSC_MINERALS] == 0);
    sup_put_base_in_tile(stile(6, 6), 2);
    base_support();
    SCHECK(g_support_world.convoy_from[RSC_MINERALS] == 1);
    g_support_world.vehs[0].order = ORDER_HOLD;
    base_support();
    SCHECK(g_support_world.convoy_from[RSC_MINERALS] == 0);

    // ---- the loop stops at the count, not one past it ----------------------
    support_reset();
    sup_proto(20, PLAN_DEFENSIVE, SUP_WPN_GUN, SUP_CH_LAND);
    sup_veh(20, 6, 6);
    {
        // A unit one entry past the count, complete in every other way.
        Veh &beyond = g_support_world.vehs[g_support_world.veh_count];
        beyond.faction_id = (uint8_t)SFACTION;
        beyond.proto_id = 20;
        beyond.x = 6;
        beyond.y = 6;
        beyond.home_base_id = 0;
    }
    base_support();
    SCHECK(g_support_world.forces_supported == 1);

    // ---- home base zero is a real base id, not "no home" -------------------
    // The guard is on the home base id being non-negative, and base 0 is only
    // ever "another base" when the selected base is not base 0.
    support_reset();
    support_arm_harvest_tile();
    sup_proto(20, PLAN_SUPPLY_CONVOY, SUP_WPN_NONE, SUP_CH_LAND);
    g_support_world.base_id_selected = 1;
    g_support_world.base_current = &g_support_world.bases[2];   // == Bases[1]
    g_support_world.bases[2].faction_id_current = (uint8_t)SFACTION;
    g_support_world.bases[2].x = 2;
    g_support_world.bases[2].y = 2;
    {
        Veh &veh = sup_veh(20, 2, 2);
        veh.order = ORDER_CONVOY;
        veh.order_auto_type = RSC_ENERGY;
        veh.home_base_id = 0;
    }
    base_support();
    SCHECK(g_support_world.convoy_to[RSC_ENERGY] == 1);

    // ---- faction zero owns a base like any other faction -------------------
    // yield_tile_owner returns the owner, and zero is an owner; only the 0xF
    // marker and the missing bit mean "no base".
    support_reset();
    support_arm_harvest_tile();
    sup_proto(20, PLAN_SUPPLY_CONVOY, SUP_WPN_NONE, SUP_CH_LAND);
    {
        Veh &veh = sup_veh(20, 4, 4);
        veh.order = ORDER_CONVOY;
        veh.order_auto_type = RSC_NUTRIENTS;
    }
    sup_put_base_in_tile(stile(4, 4), 0);
    base_support();
    SCHECK(g_support_world.convoy_to[RSC_NUTRIENTS] == 0);
    SCHECK(g_support_world.convoy_from[RSC_NUTRIENTS] == 1);

    // ---- the arm above RSC_PSI adds zero, after an arm that added two ------
    support_reset();
    support_arm_harvest_tile();
    sup_proto(20, PLAN_SUPPLY_CONVOY, SUP_WPN_NONE, SUP_CH_LAND);
    {
        Veh &first = sup_veh(20, 4, 4);
        first.order = ORDER_CONVOY;
        first.order_auto_type = RSC_NUTRIENTS;
        Veh &second = sup_veh(20, 4, 4);
        second.order = ORDER_CONVOY;
        second.order_auto_type = 4;
    }
    g_support_world.convoy_to[4] = 40;
    base_support();
    SCHECK(g_support_world.convoy_to[RSC_NUTRIENTS] == 2);
    SCHECK(g_support_world.convoy_to[4] == 40);

    // ---- a convoy is never supported and never costs maintenance ----------
    support_reset();
    sup_proto(20, PLAN_SUPPLY_CONVOY, SUP_WPN_GUN, SUP_CH_LAND);
    sup_proto(21, PLAN_TERRAFORMING, SUP_WPN_GUN, SUP_CH_LAND);
    sup_veh(20, 6, 6);
    base_support();
    SCHECK(g_support_world.forces_supported == 0);
    g_support_world.vehs[0].proto_id = 21;   // plan 9 is the last supported one
    base_support();
    SCHECK(g_support_world.forces_supported == 1);
}

void test_base_support_maintenance() {
    SupportSeams seams;

    // ---- the free allowance, and the cost of the unit past it --------------
    support_reset();
    sup_proto(20, PLAN_DEFENSIVE, SUP_WPN_GUN, SUP_CH_LAND);
    for (int i = 0; i < 3; i++) {
        sup_veh(20, 6, 6);
    }
    g_support_world.players_data[SFACTION].soc_effect_pending.support = 0;  // 2 free
    base_support();
    SCHECK(g_support_world.forces_supported == 3);
    SCHECK(g_support_world.maint_count == 1);
    SCHECK(g_support_world.maint_cost == 1);
    SCHECK(!(g_support_world.vehs[0].state & VSTATE_REQUIRES_SUPPORT));
    SCHECK(!(g_support_world.vehs[1].state & VSTATE_REQUIRES_SUPPORT));
    SCHECK((g_support_world.vehs[2].state & VSTATE_REQUIRES_SUPPORT) != 0);
    // The whole free_support table, one rating at a time, against three units.
    const int free_by_support[8] = { 0, 0, 1, 1, 2, 3, 4, 4 };
    for (int support = -4; support <= 3; support++) {
        support_reset();
        sup_proto(20, PLAN_DEFENSIVE, SUP_WPN_GUN, SUP_CH_LAND);
        for (int i = 0; i < 3; i++) {
            sup_veh(20, 6, 6);
        }
        g_support_world.players_data[SFACTION].soc_effect_pending.support = support;
        base_support();
        int free_units = free_by_support[support + 4];
        int charged = (3 > free_units) ? 3 - free_units : 0;
        SCHECK((int)g_support_world.maint_count == charged);
        SCHECK((int)g_support_world.maint_cost == charged * ((support <= -4) ? 2 : 1));
    }
    // At +3 alone the allowance follows the population when that is above four.
    support_reset();
    sup_proto(20, PLAN_DEFENSIVE, SUP_WPN_GUN, SUP_CH_LAND);
    for (int i = 0; i < 6; i++) {
        sup_veh(20, 6, 6);
    }
    g_support_world.players_data[SFACTION].soc_effect_pending.support = 3;
    g_support_world.bases[1].population_size = 6;
    base_support();
    SCHECK(g_support_world.maint_count == 0);
    g_support_world.bases[1].population_size = 4;   // not above four: still four
    base_support();
    SCHECK(g_support_world.maint_count == 2);
    g_support_world.bases[1].population_size = 5;
    base_support();
    SCHECK(g_support_world.maint_count == 1);
    // ... and nowhere else. At +2 the allowance is four whatever the population.
    g_support_world.players_data[SFACTION].soc_effect_pending.support = 2;
    g_support_world.bases[1].population_size = 6;
    base_support();
    SCHECK(g_support_world.maint_count == 2);

    // ---- a clean reactor is supported by nobody ----------------------------
    support_reset();
    sup_proto(20, PLAN_DEFENSIVE, SUP_WPN_GUN, SUP_CH_LAND).ability_flags
        = ABL_CLEAN_REACTOR;
    sup_veh(20, 6, 6);
    g_support_world.players_data[SFACTION].soc_effect_pending.support = -4;
    g_support_world.upkeep_stage = 1;
    base_support();
    SCHECK(g_support_world.forces_supported == 0);
    SCHECK(g_support_world.maint_count == 0);
    SCHECK(g_support_world.maint_cost == 0);
    SCHECK(g_support_world.players_data[SFACTION].unk_38[0] == 0);
    g_support_world.protos[20].ability_flags = ABL_TRANCE;   // a different one
    base_support();
    SCHECK(g_support_world.forces_supported == 1);
    SCHECK(g_support_world.maint_cost == 2);

    // ---- native life in fungus is supported for free -----------------------
    support_reset();
    sup_proto(20, PLAN_DEFENSIVE, SUP_WPN_PSI, SUP_CH_LAND);
    sup_veh(20, 6, 6);
    stile(6, 6).bit = BIT_FUNGUS;
    stile(6, 6).climate = 0x40;             // ocean shelf: exactly the threshold
    base_support();
    SCHECK(g_support_world.forces_supported == 0);
    stile(6, 6).climate = 0x20;             // one level lower
    base_support();
    SCHECK(g_support_world.forces_supported == 1);
    stile(6, 6).climate = 0x40;
    stile(6, 6).bit = 0;                    // no fungus
    base_support();
    SCHECK(g_support_world.forces_supported == 1);
    stile(6, 6).bit = BIT_FUNGUS;
    g_support_world.protos[20].weapon_id = SUP_WPN_GUN;   // positive offense
    base_support();
    SCHECK(g_support_world.forces_supported == 1);
    // Prototype 15 takes the exemption whatever its weapon.
    support_reset();
    sup_proto(15, PLAN_DEFENSIVE, SUP_WPN_GUN, SUP_CH_LAND);
    sup_veh(15, 6, 6);
    stile(6, 6).bit = BIT_FUNGUS;
    stile(6, 6).climate = 0x40;
    base_support();
    SCHECK(g_support_world.forces_supported == 0);
    // A custom prototype does not, whatever its weapon.
    support_reset();
    sup_proto(64, PLAN_DEFENSIVE, SUP_WPN_PSI, SUP_CH_LAND);
    sup_veh(64, 6, 6);
    stile(6, 6).bit = BIT_FUNGUS;
    stile(6, 6).climate = 0x40;
    base_support();
    SCHECK(g_support_world.forces_supported == 1);

    // An offense rating of exactly one is above zero, and is supported.
    support_reset();
    sup_proto(20, PLAN_DEFENSIVE, SUP_WPN_NONE, SUP_CH_LAND);
    sup_veh(20, 6, 6);
    stile(6, 6).bit = BIT_FUNGUS;
    stile(6, 6).climate = 0x40;
    base_support();
    SCHECK(g_support_world.forces_supported == 1);   // offense zero is not psi

    // ---- the social-engineering preview table ------------------------------
    // One unit, so the entry is charged wherever the allowance is zero, which
    // is the -4 and -3 ratings alone.
    support_reset();
    sup_proto(20, PLAN_DEFENSIVE, SUP_WPN_GUN, SUP_CH_LAND);
    sup_veh(20, 6, 6);
    // A mineral intake well above the cost, so the amplification below is out
    // of the way here; with the default intake of zero everything triples.
    g_support_world.bases[1].mineral_intake_2 = 10;
    base_support();
    for (int i = 0; i < 8; i++) {
        SCHECK(g_support_world.players_data[SFACTION].unk_38[i] == 0);  // stage 0
    }
    g_support_world.upkeep_stage = 1;
    base_support();
    SCHECK(g_support_world.players_data[SFACTION].unk_38[0] == 2);   // -4 costs two
    SCHECK(g_support_world.players_data[SFACTION].unk_38[1] == 1);
    for (int i = 2; i < 8; i++) {
        SCHECK(g_support_world.players_data[SFACTION].unk_38[i] == 0);
    }
    // It accumulates rather than being assigned.
    base_support();
    SCHECK(g_support_world.players_data[SFACTION].unk_38[0] == 4);
    SCHECK(g_support_world.players_data[SFACTION].unk_38[1] == 2);
    // The marginal cost doubles above half the base's mineral intake and
    // triples once it reaches the whole of it. Cost one against intake:
    //   intake 4 -> half 2, 1 <= 2      -> 1
    //   intake 2 -> half 1, 1 <= 1      -> 1
    //   intake 1 -> 1 >= 1              -> 1 + (1 - 0) * 2 = 3
    //   intake 0 -> 1 >= 0              -> 1 + (1 - 0) * 2 = 3
    //   intake 3 -> half 1, 1 <= 1      -> 1
    // and cost two against intake 3: half 1, 2 < 3 and 2 > 1 -> 2 * 2 - 1 = 3.
    const int intakes[5] = { 4, 2, 1, 0, 3 };
    const int expected[5] = { 1, 1, 3, 3, 1 };
    for (int i = 0; i < 5; i++) {
        support_reset();
        sup_proto(20, PLAN_DEFENSIVE, SUP_WPN_GUN, SUP_CH_LAND);
        sup_veh(20, 6, 6);
        g_support_world.upkeep_stage = 1;
        g_support_world.bases[1].mineral_intake_2 = intakes[i];
        base_support();
        SCHECK((int)g_support_world.players_data[SFACTION].unk_38[1] == expected[i]);
    }
    support_reset();
    sup_proto(20, PLAN_DEFENSIVE, SUP_WPN_GUN, SUP_CH_LAND);
    sup_veh(20, 6, 6);
    g_support_world.upkeep_stage = 1;
    g_support_world.bases[1].mineral_intake_2 = 3;
    base_support();
    SCHECK(g_support_world.players_data[SFACTION].unk_38[0] == 3);   // cost two
    // Stage 2 is not stage 1.
    support_reset();
    sup_proto(20, PLAN_DEFENSIVE, SUP_WPN_GUN, SUP_CH_LAND);
    sup_veh(20, 6, 6);
    g_support_world.upkeep_stage = 2;
    base_support();
    SCHECK(g_support_world.players_data[SFACTION].unk_38[0] == 0);

    // Every one of the eight ratings is charged, and each is charged once per
    // unit that is past ITS allowance rather than past the active one. Five
    // units against free_support { 0, 0, 1, 1, 2, 3, 4, 4 } charge the k-th
    // unit at rating i whenever k > free_support[i]; rating -4 costs two.
    support_reset();
    sup_proto(20, PLAN_DEFENSIVE, SUP_WPN_GUN, SUP_CH_LAND);
    for (int i = 0; i < 5; i++) {
        sup_veh(20, 6, 6);
    }
    g_support_world.upkeep_stage = 1;
    g_support_world.bases[1].mineral_intake_2 = 10;
    base_support();
    {
        const int charged[8] = { 10, 5, 4, 4, 3, 2, 1, 1 };
        for (int i = 0; i < 8; i++) {
            SCHECK((int)g_support_world.players_data[SFACTION].unk_38[i] == charged[i]);
        }
    }

    // ---- the AI's offensive tally ------------------------------------------
    // Plans 0 and 1 count twice, 3 and 5 not at all, everything else once.
    const uint32_t plans[8] = { PLAN_OFFENSIVE, PLAN_COMBAT, PLAN_DEFENSIVE,
                                PLAN_RECONNAISANCE, PLAN_AIR_SUPERIORITY,
                                PLAN_PLANET_BUSTER, PLAN_NAVAL_TRANSPORT,
                                PLAN_TERRAFORMING };
    const int tally[8] = { 2, 2, 1, 0, 1, 0, 1, 1 };
    for (int i = 0; i < 8; i++) {
        support_reset();
        sup_proto(20, plans[i], SUP_WPN_GUN, SUP_CH_LAND);
        sup_veh(20, 6, 6);
        base_support();
        SCHECK((int)g_support_world.players_data[SFACTION].unk_48 == tally[i]);
    }
    // An offense rating of one is still above zero.
    support_reset();
    sup_proto(20, PLAN_DEFENSIVE, SUP_WPN_WEAK, SUP_CH_LAND);
    sup_veh(20, 6, 6);
    base_support();
    SCHECK(g_support_world.players_data[SFACTION].unk_48 == 1);
    // An unarmed unit is never counted, whatever its plan.
    support_reset();
    sup_proto(20, PLAN_OFFENSIVE, SUP_WPN_NONE, SUP_CH_LAND);
    sup_veh(20, 6, 6);
    base_support();
    SCHECK(g_support_world.players_data[SFACTION].unk_48 == 0);
    // A psi weapon counts on a custom prototype and not on a predefined one.
    support_reset();
    sup_proto(63, PLAN_OFFENSIVE, SUP_WPN_PSI, SUP_CH_LAND);
    sup_veh(63, 6, 6);
    base_support();
    SCHECK(g_support_world.players_data[SFACTION].unk_48 == 0);
    support_reset();
    sup_proto(64, PLAN_OFFENSIVE, SUP_WPN_PSI, SUP_CH_LAND);
    sup_veh(64, 6, 6);
    base_support();
    SCHECK(g_support_world.players_data[SFACTION].unk_48 == 2);
    // The tally accumulates across bases; it is not this function's to clear.
    base_support();
    SCHECK(g_support_world.players_data[SFACTION].unk_48 == 4);

    // ---- the three state bits are cleared before anything sets them --------
    support_reset();
    sup_proto(20, PLAN_DEFENSIVE, SUP_WPN_NONE, SUP_CH_LAND);
    {
        Veh &veh = sup_veh(20, 6, 6);
        veh.state = VSTATE_REQUIRES_SUPPORT | VSTATE_PACIFISM_DRONE
            | VSTATE_PACIFISM_FREE_SKIP | VSTATE_EXPLORE;
    }
    g_support_world.players_data[SFACTION].soc_effect_pending.support = 3;
    base_support();
    SCHECK(g_support_world.vehs[0].state == VSTATE_EXPLORE);
}

void test_base_support_pacifism() {
    SupportSeams seams;

    // ---- only armed units, and only away from home -------------------------
    support_reset();
    sup_proto(20, PLAN_DEFENSIVE, SUP_WPN_GUN, SUP_CH_LAND);
    sup_veh(20, 6, 6);
    stile(6, 6).territory = -1;                 // nobody's
    base_support();
    SCHECK(g_support_world.pacifism_count == 1);
    stile(6, 6).territory = (int8_t)SFACTION;   // ours
    base_support();
    SCHECK(g_support_world.pacifism_count == 0);
    stile(6, 6).territory = 4;                  // someone else's
    g_support_world.game_state = STATE_OMNISCIENT_VIEW;
    base_support();
    SCHECK(g_support_world.pacifism_count == 1);
    // A base in the tile stands in for our own territory.
    support_reset();
    sup_proto(20, PLAN_DEFENSIVE, SUP_WPN_GUN, SUP_CH_LAND);
    sup_veh(20, 6, 6);
    stile(6, 6).territory = 4;
    g_support_world.game_state = STATE_OMNISCIENT_VIEW;
    sup_put_base_in_tile(stile(6, 6), 4);
    base_support();
    SCHECK(g_support_world.pacifism_count == 0);
    // Faction zero's base counts as a base here too.
    support_reset();
    sup_proto(20, PLAN_DEFENSIVE, SUP_WPN_GUN, SUP_CH_LAND);
    sup_veh(20, 6, 6);
    stile(6, 6).territory = 4;
    g_support_world.game_state = STATE_OMNISCIENT_VIEW;
    sup_put_base_in_tile(stile(6, 6), 0);
    base_support();
    SCHECK(g_support_world.pacifism_count == 0);
    // An unarmed unit abroad is never a pacifism drone.
    support_reset();
    sup_proto(20, PLAN_DEFENSIVE, SUP_WPN_NONE, SUP_CH_LAND);
    sup_veh(20, 6, 6);
    base_support();
    SCHECK(g_support_world.pacifism_count == 0);

    // ---- air units count at home, unless they are interceptors -------------
    support_reset();
    sup_proto(20, PLAN_DEFENSIVE, SUP_WPN_GUN, SUP_CH_AIR);
    sup_veh(20, 6, 6);
    stile(6, 6).territory = (int8_t)SFACTION;
    base_support();
    SCHECK(g_support_world.pacifism_count == 1);
    g_support_world.protos[20].plan = PLAN_AIR_SUPERIORITY;
    base_support();
    SCHECK(g_support_world.pacifism_count == 0);
    g_support_world.protos[20].plan = PLAN_DEFENSIVE;
    g_support_world.protos[20].chassis_id = SUP_CH_SEA;
    base_support();
    SCHECK(g_support_world.pacifism_count == 0);
    // Abroad, an interceptor counts like anything else.
    g_support_world.protos[20].chassis_id = SUP_CH_AIR;
    g_support_world.protos[20].plan = PLAN_AIR_SUPERIORITY;
    stile(6, 6).territory = -1;
    base_support();
    SCHECK(g_support_world.pacifism_count == 1);

    // ---- the POLICE rating decides which bit the unit gets -----------------
    support_reset();
    sup_proto(20, PLAN_DEFENSIVE, SUP_WPN_GUN, SUP_CH_LAND);
    sup_veh(20, 6, 6);
    sup_veh(20, 6, 6);
    g_support_world.players_data[SFACTION].soc_effect_pending.police = -2;
    base_support();
    SCHECK(g_support_world.pacifism_count == 2);
    SCHECK((g_support_world.vehs[0].state & 0x600000) == 0);
    SCHECK((g_support_world.vehs[1].state & 0x600000) == 0);
    g_support_world.players_data[SFACTION].soc_effect_pending.police = -3;
    base_support();
    SCHECK((g_support_world.vehs[0].state & 0x600000) == VSTATE_PACIFISM_FREE_SKIP);
    SCHECK((g_support_world.vehs[1].state & 0x600000) == VSTATE_PACIFISM_DRONE);
    g_support_world.players_data[SFACTION].soc_effect_pending.police = -4;
    base_support();
    SCHECK((g_support_world.vehs[0].state & 0x600000) == VSTATE_PACIFISM_DRONE);
    SCHECK((g_support_world.vehs[1].state & 0x600000) == VSTATE_PACIFISM_DRONE);
    // A Brood Pit in the CURRENT base lifts the rating by two.
    support_give_fac(g_support_world.bases[1], FAC_BROOD_PIT);
    base_support();
    SCHECK((g_support_world.vehs[0].state & 0x600000) == 0);
    SCHECK((g_support_world.vehs[1].state & 0x600000) == 0);
    g_support_world.players_data[SFACTION].soc_effect_pending.police = -5;
    base_support();
    SCHECK((g_support_world.vehs[0].state & 0x600000) == VSTATE_PACIFISM_FREE_SKIP);
    SCHECK((g_support_world.vehs[1].state & 0x600000) == VSTATE_PACIFISM_DRONE);
    // It is the Brood Pit and not a neighbouring facility id.
    support_reset();
    sup_proto(20, PLAN_DEFENSIVE, SUP_WPN_GUN, SUP_CH_LAND);
    sup_veh(20, 6, 6);
    g_support_world.players_data[SFACTION].soc_effect_pending.police = -4;
    support_give_fac(g_support_world.bases[1], FAC_COVERT_OPS_CENTER);
    support_give_fac(g_support_world.bases[1], FAC_AQUAFARM);
    base_support();
    SCHECK((g_support_world.vehs[0].state & 0x600000) == VSTATE_PACIFISM_DRONE);

    // ---- a clean reactor skips support but still causes pacifism -----------
    support_reset();
    sup_proto(20, PLAN_DEFENSIVE, SUP_WPN_GUN, SUP_CH_LAND).ability_flags
        = ABL_CLEAN_REACTOR;
    sup_veh(20, 6, 6);
    g_support_world.players_data[SFACTION].soc_effect_pending.police = -4;
    base_support();
    SCHECK(g_support_world.forces_supported == 0);
    SCHECK(g_support_world.pacifism_count == 1);
    SCHECK((g_support_world.vehs[0].state & 0x600000) == VSTATE_PACIFISM_DRONE);
}

#undef SCHECK

/*
 * A self-contained world for world_site.
 *
 * The map is 32 columns by 16 rows - MapLongitude 16, because two adjacent x
 * share a tile index - and the subject site is (16, 8), far enough from every
 * edge that all forty-nine radius offsets land in bounds. Checked while
 * writing: the twenty-one inner offsets from (16, 8) address twenty-one
 * DISTINCT tile indices, so a case can move one radius tile without moving
 * another.
 *
 * Two of the fixture's settings exist to make the two callees decidable rather
 * than pseudo-random:
 *
 *  - MapRandSeed is 0, so bonus_at() answers on BIT_RSC_BONUS alone and is
 *    non-zero exactly when the tile carries it and sits at or above the ocean
 *    shelf.
 *  - GameRules carries RULES_NO_UNITY_SCATTERING, which takes goody_at()
 *    straight to its bitfield answer instead of its position hash. Without it
 *    goody_at() returns 2 on a scattering of land tiles and `special` picks up
 *    a term no case asked for.
 */
struct SiteWorld {
    Map tiles[256];
    Continent continents[MaxContinentNum];
    Map *tiles_ptr;
    uint32_t longitude;
    int lon_bounds;
    int lat_bounds;
    BOOL is_flat;
    uint32_t map_rand_seed;
    uint32_t game_rules;
    int turn;
};

SiteWorld g_site_world;

const int SITE_X = 16;
const int SITE_Y = 8;

// Plain land: altitude three, arid, flat, nothing built.
const uint8_t SALT_LAND = 0x60;
const uint8_t SALT_SHELF = 0x40;
const uint8_t SALT_DEEP = 0x20;

Map &stile2(int x, int y) { return g_site_world.tiles[(x >> 1) + y * 16]; }

// The tile the i-th radius offset from the subject site addresses.
Map &sring(int i) {
    return stile2(SITE_X + RadiusOffsetX[i], SITE_Y + RadiusOffsetY[i]);
}

void site_reset() {
    std::memset(&g_site_world, 0, sizeof(g_site_world));
    g_site_world.tiles_ptr = g_site_world.tiles;
    g_site_world.longitude = 16;
    g_site_world.lon_bounds = 32;
    g_site_world.lat_bounds = 16;
    g_site_world.is_flat = 1;
    g_site_world.map_rand_seed = 0;
    g_site_world.game_rules = RULES_NO_UNITY_SCATTERING;
    g_site_world.turn = 0;
    for (int k = 0; k < 256; k++) {
        g_site_world.tiles[k].climate = SALT_LAND;
    }
}

// The tile the i-th radius offset from an arbitrary site addresses.
Map &sringat(int cx, int cy, int i) {
    return stile2(cx + RadiusOffsetX[i], cy + RadiusOffsetY[i]);
}

// Twenty of the twenty-one inner tiles carry a river, which is +5 to the score
// and leaves room for a case to push it either way.
void site_arm_rivers_at(int cx, int cy) {
    for (int i = 1; i < 21; i++) {
        sringat(cx, cy, i).bit |= BIT_RIVER;
    }
}

void site_arm_rivers() { site_arm_rivers_at(SITE_X, SITE_Y); }

#define SCHECK2(cond)                                                         \
    do {                                                                      \
        const bool site_ok = (cond);                                          \
        if (!site_ok) {                                                       \
            std::fprintf(stderr, "world_site: line %d: %s\n", __LINE__,       \
                         #cond);                                              \
        }                                                                     \
        expect(site_ok);                                                      \
    } while (0)

class SiteSeams {
 public:
    SiteSeams()
        : tiles_(&MapTiles, &g_site_world.tiles_ptr),
          longitude_(&MapLongitude, &g_site_world.longitude),
          lon_(&MapLongitudeBounds, &g_site_world.lon_bounds),
          lat_(&MapLatitudeBounds, &g_site_world.lat_bounds),
          flat_(&MapIsFlat, &g_site_world.is_flat),
          seed_(&MapRandSeed, &g_site_world.map_rand_seed),
          rules_(&GameRules, &g_site_world.game_rules),
          continents_(&Continents, g_site_world.continents),
          turn_(&TurnCurrentNum, &g_site_world.turn) { }

 private:
    ScopedSeam<Map *> tiles_;
    ScopedSeam<uint32_t> longitude_;
    ScopedSeam<int> lon_;
    ScopedSeam<int> lat_;
    ScopedSeam<BOOL> flat_;
    ScopedSeam<uint32_t> seed_;
    ScopedSeam<uint32_t> rules_;
    ScopedSeam<Continent> continents_;
    ScopedSeam<int> turn_;
};

void test_world_site_terrain() {
    SiteSeams seams;

    // ---- the floor, and the river term that lifts it off it ----------------
    // Twenty-one plain land tiles score nothing at all; the constant 6 in the
    // rich-terrain term is the whole of it, and the clamp holds the answer at
    // one rather than zero.
    site_reset();
    SCHECK2(world_site(SITE_X, SITE_Y, false) == 1);
    // Rivers are counted a quarter each, truncating.
    for (int n = 1; n <= 8; n++) {
        site_reset();
        for (int i = 0; i < n; i++) {
            sring(i).bit |= BIT_RIVER;
        }
        SCHECK2(world_site(SITE_X, SITE_Y, false) == 1 + n / 4);
    }
    // A tile already inside somebody's base radius is not counted at all.
    site_reset();
    for (int i = 0; i < 8; i++) {
        sring(i).bit |= BIT_RIVER;
    }
    SCHECK2(world_site(SITE_X, SITE_Y, false) == 3);
    sring(3).bit |= BIT_BASE_RADIUS;
    SCHECK2(world_site(SITE_X, SITE_Y, false) == 2);

    // ---- resource bonuses, and the centre tile's sign inversion ------------
    site_reset();
    site_arm_rivers();
    SCHECK2(world_site(SITE_X, SITE_Y, false) == 6);
    sring(1).bit |= BIT_RSC_BONUS;
    SCHECK2(world_site(SITE_X, SITE_Y, false) == 8);   // +2 away from centre
    site_reset();
    site_arm_rivers();
    sring(0).bit |= BIT_RSC_BONUS;
    SCHECK2(world_site(SITE_X, SITE_Y, false) == 5);   // -1 on the centre
    // A bonus under fungus is not seen: fungus skips the whole block.
    site_reset();
    site_arm_rivers();
    sring(1).bit |= BIT_RSC_BONUS | BIT_FUNGUS;
    SCHECK2(world_site(SITE_X, SITE_Y, false) == 6);
    // A tile already in a base radius still contributes its bonus - the skip
    // happens after the bonus block, not before it.
    site_reset();
    site_arm_rivers();
    sring(1).bit |= BIT_RSC_BONUS | BIT_BASE_RADIUS;
    SCHECK2(world_site(SITE_X, SITE_Y, false) == 8 - 1);  // bonus in, river out

    // ---- the four landmarks, each with its own weight ----------------------
    struct LandmarkCase {
        uint32_t bit2;
        int away;
        int centre;
    };
    const LandmarkCase landmarks[4] = {
        { BIT2_VOLCANO, 1, -1 },
        { BIT2_CRATER, 2, -1 },
        { BIT2_URANIUM, 1, -3 },
        { BIT2_JUNGLE, 2, 2 },       // the only one that does not invert
    };
    for (int k = 0; k < 4; k++) {
        site_reset();
        site_arm_rivers();
        sring(1).bit2 = landmarks[k].bit2;
        SCHECK2(world_site(SITE_X, SITE_Y, false) == 6 + landmarks[k].away);
        site_reset();
        site_arm_rivers();
        sring(0).bit2 = landmarks[k].bit2;
        SCHECK2(world_site(SITE_X, SITE_Y, false) == 6 + landmarks[k].centre);
    }
    // Volcano and Crater are gated on the landmark's tile sequence code, which
    // lives in the top byte; Uranium and Jungle are not.
    const uint32_t gated[2] = { BIT2_VOLCANO, BIT2_CRATER };
    const int gated_away[2] = { 1, 2 };
    for (int k = 0; k < 2; k++) {
        site_reset();
        site_arm_rivers();
        sring(1).bit2 = gated[k] | 0x08000000u;
        SCHECK2(world_site(SITE_X, SITE_Y, false) == 6 + gated_away[k]);
        site_reset();
        site_arm_rivers();
        sring(1).bit2 = gated[k] | 0x09000000u;
        SCHECK2(world_site(SITE_X, SITE_Y, false) == 6);
    }
    site_reset();
    site_arm_rivers();
    sring(1).bit2 = BIT2_URANIUM | 0x09000000u;
    SCHECK2(world_site(SITE_X, SITE_Y, false) == 7);
    // BIT2_UNK_80000000 suppresses all four.
    for (int k = 0; k < 4; k++) {
        site_reset();
        site_arm_rivers();
        sring(1).bit2 = landmarks[k].bit2 | BIT2_UNK_80000000;
        SCHECK2(world_site(SITE_X, SITE_Y, false) == 6);
    }

    // ---- supply pods count, and not on the centre tile ---------------------
    site_reset();
    site_arm_rivers();
    sring(1).bit |= BIT_UNK_4000000;
    SCHECK2(world_site(SITE_X, SITE_Y, false) == 7);
    site_reset();
    site_arm_rivers();
    sring(0).bit |= BIT_UNK_4000000;
    SCHECK2(world_site(SITE_X, SITE_Y, false) == 6);

    // ---- rich terrain: forest, monolith, and the monolith's second term ----
    site_reset();
    site_arm_rivers();
    sring(1).bit |= BIT_FOREST;
    SCHECK2(world_site(SITE_X, SITE_Y, false) == 7);   // (1*3+6)/4 == 2, was 1
    site_reset();
    site_arm_rivers();
    sring(1).bit |= BIT_MONOLITH;
    SCHECK2(world_site(SITE_X, SITE_Y, false) == 7);
    // A monolith without a forest scores a second time once the tile is high
    // enough or wet enough. Altitude five gives an elevation of three.
    site_reset();
    site_arm_rivers();
    sring(1).bit |= BIT_MONOLITH;
    sring(1).climate = 0xA0;
    SCHECK2(world_site(SITE_X, SITE_Y, false) == 8);   // (2*3+6)/4 == 3
    site_reset();
    site_arm_rivers();
    sring(1).bit |= BIT_MONOLITH | BIT_FOREST;         // the forest cancels it
    sring(1).climate = 0xA0;
    SCHECK2(world_site(SITE_X, SITE_Y, false) == 7);
    // Rainfall reaches the same second term without the altitude.
    site_reset();
    site_arm_rivers();
    sring(1).bit |= BIT_MONOLITH;
    sring(1).climate = (uint8_t)(SALT_LAND | 0x10);    // rainfall two
    SCHECK2(world_site(SITE_X, SITE_Y, false) == 8);
    // and a volcano zeroes the rainfall before that test sees it.
    site_reset();
    site_arm_rivers();
    sring(1).bit |= BIT_MONOLITH;
    sring(1).climate = (uint8_t)(SALT_LAND | 0x10);
    sring(1).bit2 = BIT2_VOLCANO | 0x09000000u;        // gated out of `special`
    SCHECK2(world_site(SITE_X, SITE_Y, false) == 7);
    // Rockiness two zeroes it too.
    site_reset();
    site_arm_rivers();
    sring(1).bit |= BIT_MONOLITH;
    sring(1).climate = (uint8_t)(SALT_LAND | 0x10);
    sring(1).val3 = 0x80;
    SCHECK2(world_site(SITE_X, SITE_Y, false) == 7);
    // On the centre tile both terms invert, and the second one costs two.
    site_reset();
    site_arm_rivers();
    sring(0).bit |= BIT_MONOLITH;
    sring(0).climate = 0xA0;
    // rich == -3, so (rich*3+6)/4 == 0 where the floor is 1; but a monolith on
    // the centre tile is refused outright.
    SCHECK2(world_site(SITE_X, SITE_Y, false) == 0);

    // ---- fair terrain: rolling, rainy and under fungus ----------------------
    // Land is never below elevation one, so this arm is reachable only with
    // fungus on the tile.
    for (int n = 1; n <= 5; n++) {
        site_reset();
        site_arm_rivers();
        for (int i = 1; i <= n; i++) {
            sring(i).climate = (uint8_t)(SALT_LAND | 0x08);
            sring(i).val3 = 0x40;
            sring(i).bit |= BIT_FUNGUS;
        }
        SCHECK2(world_site(SITE_X, SITE_Y, false) == 6 + (n + 1) / 3);
    }
    // Without the fungus the same tile is RICH instead, not fair.
    site_reset();
    site_arm_rivers();
    sring(1).climate = (uint8_t)(SALT_LAND | 0x08);
    sring(1).val3 = 0x40;
    SCHECK2(world_site(SITE_X, SITE_Y, false) == 7);
    // Without the rolling rockiness it is neither.
    site_reset();
    site_arm_rivers();
    sring(1).climate = (uint8_t)(SALT_LAND | 0x08);
    sring(1).bit |= BIT_FUNGUS;
    SCHECK2(world_site(SITE_X, SITE_Y, false) == 6);
    // Nor without the rain.
    site_reset();
    site_arm_rivers();
    sring(1).val3 = 0x40;
    sring(1).bit |= BIT_FUNGUS;
    SCHECK2(world_site(SITE_X, SITE_Y, false) == 6);

    // A volcano zeroes the rainfall before the fair-terrain arm tests it, so
    // two tiles that would otherwise be fair are worth nothing.
    site_reset();
    site_arm_rivers();
    for (int i = 1; i <= 2; i++) {
        sring(i).climate = (uint8_t)(SALT_LAND | 0x08);
        sring(i).val3 = 0x40;
        sring(i).bit |= BIT_FUNGUS;
        sring(i).bit2 = BIT2_VOLCANO | 0x09000000u;
    }
    SCHECK2(world_site(SITE_X, SITE_Y, false) == 6);

    // A forest on the centre tile costs, where anywhere else it pays.
    site_reset();
    site_arm_rivers();
    sring(0).bit |= BIT_FOREST;
    SCHECK2(world_site(SITE_X, SITE_Y, false) == 5);

    // The monolith's second term needs rainfall ABOVE one, not merely present.
    site_reset();
    site_arm_rivers();
    sring(1).bit |= BIT_MONOLITH;
    sring(1).climate = (uint8_t)(SALT_LAND | 0x08);    // rainfall one
    SCHECK2(world_site(SITE_X, SITE_Y, false) == 7);

    // ---- rocky tiles are a quarter each, and not on the centre -------------
    site_reset();
    site_arm_rivers();
    for (int i = 1; i <= 4; i++) {
        sring(i).val3 = 0x80;
    }
    SCHECK2(world_site(SITE_X, SITE_Y, false) == 5);
    site_reset();
    site_arm_rivers();
    for (int i = 1; i <= 3; i++) {
        sring(i).val3 = 0x80;
    }
    SCHECK2(world_site(SITE_X, SITE_Y, false) == 6);
    site_reset();
    site_arm_rivers();
    for (int i = 1; i <= 3; i++) {
        sring(i).val3 = 0x80;
    }
    sring(0).val3 = 0x40;   // rolling on the centre is not rocky and is allowed
    SCHECK2(world_site(SITE_X, SITE_Y, false) == 6);

    // ---- fungus is only charged after turn 150 -----------------------------
    site_reset();
    site_arm_rivers();
    for (int i = 1; i <= 3; i++) {
        sring(i).bit |= BIT_FUNGUS;
    }
    g_site_world.turn = 150;
    SCHECK2(world_site(SITE_X, SITE_Y, false) == 6);
    g_site_world.turn = 151;
    SCHECK2(world_site(SITE_X, SITE_Y, false) == 5);
    for (int i = 4; i <= 5; i++) {
        sring(i).bit |= BIT_FUNGUS;
    }
    SCHECK2(world_site(SITE_X, SITE_Y, false) == 5);   // five thirds is one
    sring(6).bit |= BIT_FUNGUS;
    SCHECK2(world_site(SITE_X, SITE_Y, false) == 4);
}

void test_world_site_score() {
    SiteSeams seams;

    // ---- ocean in the radius, and which way it counts ----------------------
    site_reset();
    site_arm_rivers();
    for (int i = 1; i <= 4; i++) {
        sring(i).climate = SALT_SHELF;
    }
    // Four ocean tiles stop being river tiles as well: the ocean arm of the
    // loop leaves before the river count.
    SCHECK2(world_site(SITE_X, SITE_Y, false) == 3);    // 16/4 + 1 - 4/2
    SCHECK2(world_site(SITE_X, SITE_Y, true) == 7);     // ... + 4/2 instead
    // Below the shelf each tile counts twice: once as ocean, once as deep.
    site_reset();
    site_arm_rivers();
    for (int i = 1; i <= 4; i++) {
        sring(i).climate = SALT_DEEP;
    }
    SCHECK2(world_site(SITE_X, SITE_Y, false) == 1);    // 4 + 1 - (2 + 2)
    SCHECK2(world_site(SITE_X, SITE_Y, true) == 9);
    // An ocean tile with a bonus at or above the shelf still scores; below it
    // bonus_at answers zero.
    site_reset();
    site_arm_rivers();
    sring(1).climate = SALT_SHELF;
    sring(1).bit |= BIT_RSC_BONUS;
    SCHECK2(world_site(SITE_X, SITE_Y, false) == 6);    // 19/4 + 1 + 1 - 0
    site_reset();
    site_arm_rivers();
    sring(1).climate = SALT_DEEP;
    sring(1).bit |= BIT_RSC_BONUS;
    SCHECK2(world_site(SITE_X, SITE_Y, false) == 5);    // 4 + 1 + 1 - 1
    // Fungus at or above the shelf suppresses the ocean bonus.
    site_reset();
    site_arm_rivers();
    sring(1).climate = SALT_SHELF;
    sring(1).bit |= BIT_RSC_BONUS | BIT_FUNGUS;
    SCHECK2(world_site(SITE_X, SITE_Y, false) == 5);
    // The centre tile's ocean never scores a bonus at all.
    site_reset();
    site_arm_rivers();
    sring(0).climate = SALT_SHELF;
    sring(0).bit |= BIT_RSC_BONUS;
    SCHECK2(world_site(SITE_X, SITE_Y, true) == 6);     // 5 + 1 + 0 + 0

    // ---- a big continent, decided by the nine innermost tiles only ---------
    site_reset();
    site_arm_rivers();
    sring(1).climate = SALT_SHELF;
    sring(1).region = 5;
    g_site_world.continents[5].tile_count = 50;
    SCHECK2(world_site(SITE_X, SITE_Y, false) == 5 + 4);   // 19/4 + 1 + 4 - 0
    g_site_world.continents[5].tile_count = 49;
    SCHECK2(world_site(SITE_X, SITE_Y, false) == 5);       // 19/4 + 1 - 0
    // Ninth offset in, tenth out.
    site_reset();
    site_arm_rivers();
    sring(8).climate = SALT_SHELF;
    sring(8).region = 5;
    g_site_world.continents[5].tile_count = 50;
    SCHECK2(world_site(SITE_X, SITE_Y, false) == 9);
    site_reset();
    site_arm_rivers();
    sring(9).climate = SALT_SHELF;
    sring(9).region = 5;
    g_site_world.continents[5].tile_count = 50;
    SCHECK2(world_site(SITE_X, SITE_Y, false) == 5);
    // On an ocean site the same continent costs two instead of paying four.
    site_reset();
    site_arm_rivers();
    sring(1).climate = SALT_SHELF;
    sring(1).region = 5;
    g_site_world.continents[5].tile_count = 50;
    SCHECK2(world_site(SITE_X, SITE_Y, true) == 5 - 2);    // and (1-1)/2 == 0

    // ---- the last radius tile's two flags are worth four -------------------
    site_reset();
    site_arm_rivers();
    sring(20).bit |= BIT_UNK_4000;
    SCHECK2(world_site(SITE_X, SITE_Y, false) == 10);
    site_reset();
    site_arm_rivers();
    sring(20).bit |= BIT_UNK_40000000;
    SCHECK2(world_site(SITE_X, SITE_Y, false) == 10);
    // It really is the LAST tile of the radius and not any other.
    site_reset();
    site_arm_rivers();
    sring(19).bit |= BIT_UNK_4000;
    SCHECK2(world_site(SITE_X, SITE_Y, false) == 6);

    // ---- crowding, counted over the outer ring -----------------------------
    site_reset();
    site_arm_rivers();
    sring(25).bit |= BIT_BASE_IN_TILE;
    SCHECK2(world_site(SITE_X, SITE_Y, false) == 5);    // one, at single weight
    site_reset();
    site_arm_rivers();
    sring(21).bit |= BIT_BASE_IN_TILE;
    SCHECK2(world_site(SITE_X, SITE_Y, false) == 4);    // the first four count two
    site_reset();
    site_arm_rivers();
    sring(24).bit |= BIT_BASE_IN_TILE;
    SCHECK2(world_site(SITE_X, SITE_Y, false) == 4);
    // Ocean on both the site and the neighbouring base makes it four.
    site_reset();
    for (int i = 1; i < 21; i++) {
        sring(i).bit |= BIT_RIVER;
    }
    sring(0).climate = SALT_SHELF;
    sring(21).climate = SALT_SHELF;
    sring(21).bit |= BIT_BASE_IN_TILE;
    // 20 rivers -> 5, +1, ocean count 1 -> water 0; an ocean site pays 4 x 1.
    SCHECK2(world_site(SITE_X, SITE_Y, true) == 2);
    // A land site is the wrong element for an ocean tile, and four is the
    // crowding at which that becomes a refusal.
    SCHECK2(world_site(SITE_X, SITE_Y, false) == 0);
    // Three is not four.
    site_reset();
    for (int i = 1; i < 21; i++) {
        sring(i).bit |= BIT_RIVER;
    }
    sring(0).climate = SALT_SHELF;
    sring(21).bit |= BIT_BASE_IN_TILE;   // land neighbour: two, not four
    SCHECK2(world_site(SITE_X, SITE_Y, false) == 2);    // 5 + 1 - 2*2

    // ---- the outer ring is 21 to 48, and nothing else ----------------------
    site_reset();
    site_arm_rivers();
    sring(1).bit |= BIT_BASE_IN_TILE;      // inside the inner ring, not counted
    SCHECK2(world_site(SITE_X, SITE_Y, false) == 6);
    site_reset();
    site_arm_rivers();
    sring(49).bit |= BIT_BASE_IN_TILE;     // one past the outer ring
    SCHECK2(world_site(SITE_X, SITE_Y, false) == 6);
    sring(48).bit |= BIT_BASE_IN_TILE;     // the last one that is counted
    SCHECK2(world_site(SITE_X, SITE_Y, false) == 5);

    // ---- both loops take a coordinate of zero and refuse one of bounds -----
    // Offset 47 is (0, -6) and offset 48 is (-6, 0), so a site six from an edge
    // puts exactly one outer-ring tile on the boundary.
    site_reset();
    site_arm_rivers_at(SITE_X, 6);
    sringat(SITE_X, 6, 47).bit |= BIT_BASE_IN_TILE;    // y == 0, counted
    SCHECK2(world_site(SITE_X, 6, false) == 5);
    site_reset();
    site_arm_rivers_at(6, SITE_Y);
    sringat(6, SITE_Y, 48).bit |= BIT_BASE_IN_TILE;    // x == 0, counted
    SCHECK2(world_site(6, SITE_Y, false) == 5);
    site_reset();
    g_site_world.lat_bounds = 14;
    site_arm_rivers();
    sring(45).bit |= BIT_BASE_IN_TILE;                 // y == bound, refused
    SCHECK2(world_site(SITE_X, SITE_Y, false) == 6);
    site_reset();
    g_site_world.lon_bounds = 30;
    site_arm_rivers_at(24, SITE_Y);
    sringat(24, SITE_Y, 46).bit |= BIT_BASE_IN_TILE;   // x == bound, refused
    SCHECK2(world_site(24, SITE_Y, false) == 6);

    // The inner ring's own four bounds, with offsets 13 and 20 at dy -3, 16 and
    // 17 at dy +3, 18 and 19 at dx -3, and 14 and 15 at dx +3. Each pair is
    // worth four to `special` when it is in bounds and nothing when it is not.
    site_reset();
    site_arm_rivers_at(SITE_X, 3);
    sringat(SITE_X, 3, 13).bit |= BIT_RSC_BONUS;       // y == 0
    sringat(SITE_X, 3, 20).bit |= BIT_RSC_BONUS;
    SCHECK2(world_site(SITE_X, 3, false) == 10);
    site_reset();
    site_arm_rivers_at(3, SITE_Y);
    sringat(3, SITE_Y, 18).bit |= BIT_RSC_BONUS;       // x == 0
    sringat(3, SITE_Y, 19).bit |= BIT_RSC_BONUS;
    SCHECK2(world_site(3, SITE_Y, false) == 10);
    site_reset();
    g_site_world.lat_bounds = 14;
    site_arm_rivers_at(SITE_X, 11);
    sringat(SITE_X, 11, 16).bit |= BIT_RSC_BONUS;      // y == bound
    sringat(SITE_X, 11, 17).bit |= BIT_RSC_BONUS;
    SCHECK2(world_site(SITE_X, 11, false) == 5);
    site_reset();
    g_site_world.lon_bounds = 30;
    site_arm_rivers_at(27, SITE_Y);
    sringat(27, SITE_Y, 14).bit |= BIT_RSC_BONUS;      // x == bound
    sringat(27, SITE_Y, 15).bit |= BIT_RSC_BONUS;
    SCHECK2(world_site(27, SITE_Y, false) == 5);

    // The doubling's third term needs BOTH the site and the neighbouring base
    // to be ocean; the site's own altitude is read for it.
    site_reset();
    site_arm_rivers();
    sring(21).climate = SALT_SHELF;
    sring(21).bit |= BIT_BASE_IN_TILE;
    SCHECK2(world_site(SITE_X, SITE_Y, false) == 4);   // two, not four

    // ---- the four refusals --------------------------------------------------
    site_reset();
    site_arm_rivers();
    sring(0).bit |= BIT_FUNGUS;
    SCHECK2(world_site(SITE_X, SITE_Y, false) == 0);    // fungus above the shelf
    site_reset();
    site_arm_rivers();
    sring(0).climate = SALT_DEEP;                       // below it, allowed
    sring(0).bit |= BIT_FUNGUS;
    SCHECK2(world_site(SITE_X, SITE_Y, true) == 6);
    site_reset();
    site_arm_rivers();
    sring(0).climate = SALT_SHELF;                      // exactly the shelf
    sring(0).bit |= BIT_FUNGUS;
    SCHECK2(world_site(SITE_X, SITE_Y, true) == 0);
    site_reset();
    site_arm_rivers();
    sring(0).bit |= BIT_MONOLITH;
    SCHECK2(world_site(SITE_X, SITE_Y, false) == 0);
    site_reset();
    site_arm_rivers();
    sring(0).val3 = 0x80;                               // rocky land
    SCHECK2(world_site(SITE_X, SITE_Y, false) == 0);
    site_reset();
    site_arm_rivers();
    sring(0).val3 = 0x40;                               // rolling land is fine
    SCHECK2(world_site(SITE_X, SITE_Y, false) == 6);
    site_reset();
    site_arm_rivers();
    sring(0).val3 = 0x80;
    sring(0).climate = SALT_SHELF;                      // rocky ocean is fine
    SCHECK2(world_site(SITE_X, SITE_Y, true) == 6);

    // A score of exactly zero is lifted to one rather than refused.
    site_reset();
    sring(25).bit |= BIT_BASE_IN_TILE;
    SCHECK2(world_site(SITE_X, SITE_Y, false) == 1);

    // ---- both clamps --------------------------------------------------------
    site_reset();
    for (int i = 1; i < 21; i++) {
        sring(i).bit |= BIT_RSC_BONUS;                  // special == 40
    }
    SCHECK2(world_site(SITE_X, SITE_Y, false) == 15);
    site_reset();
    for (int i = 1; i <= 12; i++) {
        sring(i).climate = SALT_DEEP;                   // water == 12
    }
    SCHECK2(world_site(SITE_X, SITE_Y, false) == 1);

    // ---- the x wrap, and the low bit of MapIsFlat that gates it ------------
    // From x == 0 the nine radius offsets with a negative x fall off a flat
    // map and wrap around a round one.
    site_reset();
    for (int k = 0; k < 256; k++) {
        g_site_world.tiles[k].bit = BIT_RIVER;
    }
    SCHECK2(world_site(0, SITE_Y, false) == 4);         // 12 rivers -> 3, +1
    g_site_world.is_flat = 0;
    SCHECK2(world_site(0, SITE_Y, false) == 6);         // 21 rivers -> 5, +1
    // Only the low bit is consulted, which xrange() does not do.
    g_site_world.is_flat = 2;
    SCHECK2(world_site(0, SITE_Y, false) == 6);
    g_site_world.is_flat = 3;
    SCHECK2(world_site(0, SITE_Y, false) == 4);
}

#undef SCHECK2

/*
 * A self-contained world for num_objectives.
 *
 * Eleven scenario victory conditions add into one total and each is gated by
 * its own bit, so almost every case here sets exactly one bit of GameRules or
 * GameState and asserts one number. The banked count the total STARTS from -
 * PlayerData.unk_101 - is left at zero except where a case names it, which
 * makes every other number a delta from nothing.
 *
 * The map is eight columns by four rows, MapLongitude 4, which packs to four
 * Map entries per row and sixteen in total; the two map walks step one entry
 * per tile without resetting between rows, so the fixture has to agree with
 * that packing exactly, and it does.
 *
 * Technology is left zeroed, which makes has_tech() answer on GameTechAchieved
 * alone for ids 0 to 87 and refuse id 88 as 'Transcendent Thought'.
 */
struct ObjWorld {
    Map tiles[32];
    Base bases[6];
    Veh vehs[8];
    PlayerData players_data[9];
    RulesTechnology technology[MaxTechnologyNum];
    uint8_t tech_achieved[MaxTechnologyNum];
    BaseSecretProject projects;
    uint8_t faction_status[4];
    Map *tiles_ptr;
    int veh_count;
    int base_count;
    uint32_t longitude;
    int lon_bounds;
    int lat_bounds;
    BOOL is_flat;
    uint32_t game_rules;
    uint32_t game_state;
    uint32_t scn_vict_facility;
};

ObjWorld g_obj_world;

const int OFACTION = 1;

Map &otile(int x, int y) { return g_obj_world.tiles[(x >> 1) + y * 4]; }

void obj_reset() {
    std::memset(&g_obj_world, 0, sizeof(g_obj_world));
    std::memset(&g_obj_world.projects, 0xFF, sizeof(g_obj_world.projects));
    g_obj_world.tiles_ptr = g_obj_world.tiles;
    g_obj_world.longitude = 4;
    g_obj_world.lon_bounds = 8;
    g_obj_world.lat_bounds = 4;
    g_obj_world.is_flat = 1;
    g_obj_world.faction_status[1] = 0xFF;   // every faction alive
    g_obj_world.scn_vict_facility = FAC_RECYCLING_TANKS;
}

// A base of `owner` at (x, y), with the map bit base_at() insists on.
Base &obj_base(int base_id, int owner, int x, int y) {
    Base &base = g_obj_world.bases[base_id];
    base.faction_id_current = (uint8_t)owner;
    base.x = (int16_t)x;
    base.y = (int16_t)y;
    otile(x, y).bit |= BIT_BASE_IN_TILE;
    if (base_id >= g_obj_world.base_count) {
        g_obj_world.base_count = base_id + 1;
    }
    return base;
}

// An objective-flagged unit of the subject faction at (x, y).
Veh &obj_veh(int x, int y) {
    Veh &veh = g_obj_world.vehs[g_obj_world.veh_count++];
    veh.faction_id = (uint8_t)OFACTION;
    veh.flags = VFLAG_IS_OBJECTIVE;
    veh.x = (int16_t)x;
    veh.y = (int16_t)y;
    return veh;
}

void obj_give_fac(Base &base, uint32_t facility) {
    int offset;
    int mask;
    bitmask(facility, &offset, &mask);
    base.facilities_built[offset] |= (uint8_t)mask;
}

#define OCHECK(cond)                                                          \
    do {                                                                      \
        const bool obj_ok = (cond);                                           \
        if (!obj_ok) {                                                        \
            std::fprintf(stderr, "num_objectives: line %d: %s\n", __LINE__,   \
                         #cond);                                              \
        }                                                                     \
        expect(obj_ok);                                                       \
    } while (0)

class ObjSeams {
 public:
    ObjSeams()
        : tiles_(&MapTiles, &g_obj_world.tiles_ptr),
          longitude_(&MapLongitude, &g_obj_world.longitude),
          lon_(&MapLongitudeBounds, &g_obj_world.lon_bounds),
          lat_(&MapLatitudeBounds, &g_obj_world.lat_bounds),
          flat_(&MapIsFlat, &g_obj_world.is_flat),
          rules_(&GameRules, &g_obj_world.game_rules),
          state_(&GameState, &g_obj_world.game_state),
          bases_(&Bases, g_obj_world.bases),
          base_count_(&BaseCurrentCount, &g_obj_world.base_count),
          vehs_(&Vehs, g_obj_world.vehs),
          veh_count_(&VehCurrentCount, &g_obj_world.veh_count),
          players_data_(&PlayersData, g_obj_world.players_data),
          technology_(&Technology, g_obj_world.technology),
          achieved_(&GameTechAchieved, g_obj_world.tech_achieved),
          projects_(&SecretProject, &g_obj_world.projects),
          status_(&FactionsStatus, g_obj_world.faction_status),
          facility_(&ScnVictFacilityObj, &g_obj_world.scn_vict_facility) { }

 private:
    ScopedSeam<Map *> tiles_;
    ScopedSeam<uint32_t> longitude_;
    ScopedSeam<int> lon_;
    ScopedSeam<int> lat_;
    ScopedSeam<BOOL> flat_;
    ScopedSeam<uint32_t> rules_;
    ScopedSeam<uint32_t> state_;
    ScopedSeam<Base> bases_;
    ScopedSeam<int> base_count_;
    ScopedSeam<Veh> vehs_;
    ScopedSeam<int> veh_count_;
    ScopedSeam<PlayerData> players_data_;
    ScopedSeam<RulesTechnology> technology_;
    ScopedSeam<uint8_t> achieved_;
    ScopedSeam<BaseSecretProject> projects_;
    ScopedSeam<uint8_t> status_;
    ScopedSeam<uint32_t> facility_;
};

void test_num_objectives_units() {
    ObjSeams seams;

    // ---- an eliminated faction scores nothing, whatever it holds -----------
    obj_reset();
    g_obj_world.players_data[OFACTION].unk_101 = 7;
    OCHECK(num_objectives(OFACTION, false) == 7);
    g_obj_world.faction_status[1] = (uint8_t)~(1 << OFACTION);
    OCHECK(num_objectives(OFACTION, false) == 0);
    // It is entry ONE of the status pair - entry zero says human, not alive.
    obj_reset();
    g_obj_world.players_data[OFACTION].unk_101 = 7;
    g_obj_world.faction_status[0] = 0;
    OCHECK(num_objectives(OFACTION, false) == 7);

    // ---- with neither unit rule set, an objective unit counts by itself ----
    obj_reset();
    obj_veh(0, 0);
    obj_veh(2, 0);
    OCHECK(num_objectives(OFACTION, false) == 2);
    g_obj_world.vehs[1].flags = 0;                    // not an objective unit
    OCHECK(num_objectives(OFACTION, false) == 1);
    g_obj_world.vehs[1].flags = VFLAG_IS_OBJECTIVE;
    g_obj_world.vehs[1].faction_id = OFACTION + 1;    // not our unit
    OCHECK(num_objectives(OFACTION, false) == 1);
    // The flag is 0x20 and not a neighbouring one.
    g_obj_world.vehs[1].faction_id = OFACTION;
    g_obj_world.vehs[1].flags = VFLAG_LURKER;
    OCHECK(num_objectives(OFACTION, false) == 1);

    // A unit one past the count is not walked.
    obj_reset();
    obj_veh(0, 0);
    {
        Veh &beyond = g_obj_world.vehs[g_obj_world.veh_count];
        beyond.faction_id = (uint8_t)OFACTION;
        beyond.flags = VFLAG_IS_OBJECTIVE;
    }
    OCHECK(num_objectives(OFACTION, false) == 1);

    // ---- reaching a friendly HQ base ---------------------------------------
    obj_reset();
    g_obj_world.game_rules = RULES_SCN_VICT_OBJ_UNITS_REACH_FRIEND_HQ_BASE;
    obj_base(0, OFACTION, 0, 0);
    obj_veh(0, 0);
    OCHECK(num_objectives(OFACTION, false) == 0);     // no headquarters
    obj_give_fac(g_obj_world.bases[0], FAC_HEADQUARTERS);
    OCHECK(num_objectives(OFACTION, false) == 1);
    // A neighbouring facility id is not the headquarters.
    obj_reset();
    g_obj_world.game_rules = RULES_SCN_VICT_OBJ_UNITS_REACH_FRIEND_HQ_BASE;
    obj_base(0, OFACTION, 0, 0);
    obj_veh(0, 0);
    obj_give_fac(g_obj_world.bases[0], FAC_CHILDREN_CRECHE);
    OCHECK(num_objectives(OFACTION, false) == 0);
    // Off a base entirely.
    obj_reset();
    g_obj_world.game_rules = RULES_SCN_VICT_OBJ_UNITS_REACH_FRIEND_HQ_BASE;
    obj_base(0, OFACTION, 0, 0);
    obj_give_fac(g_obj_world.bases[0], FAC_HEADQUARTERS);
    obj_veh(2, 0);
    OCHECK(num_objectives(OFACTION, false) == 0);

    // ---- somebody else's base, and the pact that redeems it ----------------
    obj_reset();
    g_obj_world.game_rules = RULES_SCN_VICT_OBJ_UNITS_REACH_FRIEND_HQ_BASE;
    obj_base(0, OFACTION + 1, 0, 0);
    obj_give_fac(g_obj_world.bases[0], FAC_HEADQUARTERS);
    obj_veh(0, 0);
    OCHECK(num_objectives(OFACTION, false) == 0);
    g_obj_world.players_data[OFACTION].diplo_treaties[OFACTION + 1] = DTREATY_PACT;
    OCHECK(num_objectives(OFACTION, false) == 0);     // no cooperative victory
    g_obj_world.game_rules |= RULES_VICTORY_COOPERATIVE;
    OCHECK(num_objectives(OFACTION, false) == 1);
    // A treaty is not a pact.
    g_obj_world.players_data[OFACTION].diplo_treaties[OFACTION + 1] = DTREATY_TREATY;
    OCHECK(num_objectives(OFACTION, false) == 0);
    // The treaty row is the SUBJECT faction's, indexed by the base's owner.
    g_obj_world.players_data[OFACTION].diplo_treaties[OFACTION + 1] = 0;
    g_obj_world.players_data[OFACTION + 1].diplo_treaties[OFACTION] = DTREATY_PACT;
    OCHECK(num_objectives(OFACTION, false) == 0);

    // ---- reaching an OBJECTIVE base, which outranks the headquarters rule --
    obj_reset();
    g_obj_world.game_rules = RULES_SCN_VICT_OBJ_UNITS_REACH_FRIEND_OBJ_BASE
        | RULES_SCN_VICT_OBJ_UNITS_REACH_FRIEND_HQ_BASE;
    obj_base(0, OFACTION, 0, 0);
    obj_give_fac(g_obj_world.bases[0], FAC_HEADQUARTERS);
    obj_veh(0, 0);
    OCHECK(num_objectives(OFACTION, false) == 0);     // a headquarters is not enough
    // Flagging the base counts twice: once for the unit that reached it and
    // once for the base itself in the loop below.
    g_obj_world.bases[0].event = BEVENT_OBJECTIVE;
    OCHECK(num_objectives(OFACTION, false) == 2);
    // ... and is_objective's other three routes reach it too.
    obj_reset();
    g_obj_world.game_rules = RULES_SCN_VICT_OBJ_UNITS_REACH_FRIEND_OBJ_BASE;
    obj_base(0, OFACTION, 0, 0);
    obj_veh(0, 0);
    OCHECK(num_objectives(OFACTION, false) == 0);
    g_obj_world.game_state = STATE_SCN_VICT_BASE_FACIL_COUNT_OBJ;
    obj_give_fac(g_obj_world.bases[0], FAC_RECYCLING_TANKS);
    // The facility route also turns on the base-facility count below, which
    // adds one of its own.
    OCHECK(num_objectives(OFACTION, false) == 2);
}

void test_num_objectives_totals() {
    ObjSeams seams;

    // ---- bases, their population, and which of them are objectives ---------
    obj_reset();
    obj_base(0, OFACTION, 0, 0).population_size = 3;
    obj_base(1, OFACTION, 2, 0).population_size = 5;
    obj_base(2, OFACTION + 1, 4, 0).population_size = 9;
    OCHECK(num_objectives(OFACTION, false) == 0);
    g_obj_world.game_state = STATE_SCN_VICT_POPULATION_COUNT_OBJ;
    OCHECK(num_objectives(OFACTION, false) == 8);     // not the other faction's
    g_obj_world.bases[0].event = BEVENT_OBJECTIVE;
    OCHECK(num_objectives(OFACTION, false) == 9);
    g_obj_world.game_rules = RULES_SCN_VICT_ALL_BASE_COUNT_OBJ;
    OCHECK(num_objectives(OFACTION, false) == 10);    // both of ours now
    g_obj_world.game_state = 0;
    OCHECK(num_objectives(OFACTION, false) == 2);

    // A base one past the count is not walked either.
    obj_reset();
    g_obj_world.game_rules = RULES_SCN_VICT_ALL_BASE_COUNT_OBJ;
    obj_base(0, OFACTION, 0, 0);
    {
        Base &beyond = g_obj_world.bases[g_obj_world.base_count];
        beyond.faction_id_current = (uint8_t)OFACTION;
        beyond.event = BEVENT_OBJECTIVE;
    }
    OCHECK(num_objectives(OFACTION, false) == 1);

    // ---- technologies, and the field added after them ----------------------
    obj_reset();
    g_obj_world.tech_achieved[3] = 1 << OFACTION;
    g_obj_world.tech_achieved[4] = 1 << OFACTION;
    g_obj_world.tech_achieved[5] = 1 << (OFACTION + 1);
    OCHECK(num_objectives(OFACTION, false) == 0);
    g_obj_world.game_state = STATE_SCN_VICT_TECH_COUNT_OBJ;
    OCHECK(num_objectives(OFACTION, false) == 2);
    g_obj_world.tech_achieved[0] = 1 << OFACTION;     // the scan starts at zero
    OCHECK(num_objectives(OFACTION, false) == 3);
    g_obj_world.tech_achieved[0] = 0;
    g_obj_world.players_data[OFACTION].theory_of_everything = 6;
    OCHECK(num_objectives(OFACTION, false) == 8);
    // The last technology id is 'Transcendent Thought' and has_tech refuses it.
    g_obj_world.tech_achieved[MaxTechnologyNum - 1] = 1 << OFACTION;
    OCHECK(num_objectives(OFACTION, false) == 8);
    g_obj_world.tech_achieved[MaxTechnologyNum - 2] = 1 << OFACTION;
    OCHECK(num_objectives(OFACTION, false) == 9);

    // ---- energy credits -----------------------------------------------------
    obj_reset();
    g_obj_world.players_data[OFACTION].energy_reserves = 42;
    OCHECK(num_objectives(OFACTION, false) == 0);
    g_obj_world.game_state = STATE_SCN_VICT_CREDITS_COUNT_OBJ;
    OCHECK(num_objectives(OFACTION, false) == 42);

    // ---- bases holding the scenario's nominated facility -------------------
    obj_reset();
    obj_base(0, OFACTION, 0, 0);
    obj_base(1, OFACTION, 2, 0);
    obj_base(2, OFACTION + 1, 4, 0);
    obj_give_fac(g_obj_world.bases[0], FAC_RECYCLING_TANKS);
    obj_give_fac(g_obj_world.bases[2], FAC_RECYCLING_TANKS);
    OCHECK(num_objectives(OFACTION, false) == 0);
    g_obj_world.game_state = STATE_SCN_VICT_BASE_FACIL_COUNT_OBJ;
    OCHECK(num_objectives(OFACTION, false) == 1);     // ours only
    obj_give_fac(g_obj_world.bases[1], FAC_RECYCLING_TANKS);
    OCHECK(num_objectives(OFACTION, false) == 2);
    // A base one past the count is not walked.
    {
        Base &beyond = g_obj_world.bases[g_obj_world.base_count];
        beyond.faction_id_current = (uint8_t)OFACTION;
        obj_give_fac(beyond, FAC_RECYCLING_TANKS);
    }
    OCHECK(num_objectives(OFACTION, false) == 2);
    // A facility id at or above the repeating range is refused rather than
    // indexed - and refused even when the base really does hold it.
    obj_give_fac(g_obj_world.bases[0], FacilityRepStart);
    g_obj_world.scn_vict_facility = FacilityRepStart;
    OCHECK(num_objectives(OFACTION, false) == 0);
    g_obj_world.scn_vict_facility = FacilityRepStart - 1;
    OCHECK(num_objectives(OFACTION, false) == 0);     // nobody has that one

    // ---- terrain enhancements over owned territory -------------------------
    obj_reset();
    g_obj_world.tiles[0].territory = OFACTION;
    g_obj_world.tiles[0].bit = BIT_ROAD | BIT_MINE | BIT_FARM;
    g_obj_world.tiles[1].territory = OFACTION;
    g_obj_world.tiles[1].bit = BIT_RIVER | BIT_BASE_IN_TILE;   // neither counts
    g_obj_world.tiles[2].territory = OFACTION + 1;
    g_obj_world.tiles[2].bit = BIT_ROAD | BIT_MINE;
    OCHECK(num_objectives(OFACTION, false) == 0);
    g_obj_world.game_state = STATE_SCN_VICT_TERRAIN_ENH_COUNT_OBJ;
    OCHECK(num_objectives(OFACTION, false) == 3);
    // All twelve bits of the mask, one at a time, against three that are not.
    const uint32_t counted[12] = {
        BIT_ROAD, BIT_MAGTUBE, BIT_MINE, BIT_SOLAR_TIDAL, BIT_BUNKER, BIT_FARM,
        BIT_AIRBASE, BIT_FOREST, BIT_CONDENSER, BIT_ECH_MIRROR,
        BIT_THERMAL_BORE, BIT_SENSOR_ARRAY };
    for (int k = 0; k < 12; k++) {
        obj_reset();
        g_obj_world.game_state = STATE_SCN_VICT_TERRAIN_ENH_COUNT_OBJ;
        g_obj_world.tiles[0].territory = OFACTION;
        g_obj_world.tiles[0].bit = counted[k];
        OCHECK(num_objectives(OFACTION, false) == 1);
    }
    const uint32_t ignored[3] = { BIT_BASE_IN_TILE, BIT_SOIL_ENRICHER,
                                  BIT_SUPPLY_POD };
    for (int k = 0; k < 3; k++) {
        obj_reset();
        g_obj_world.game_state = STATE_SCN_VICT_TERRAIN_ENH_COUNT_OBJ;
        g_obj_world.tiles[0].territory = OFACTION;
        g_obj_world.tiles[0].bit = ignored[k];
        OCHECK(num_objectives(OFACTION, false) == 0);
    }
    // The walk is one Map per tile across all four rows, sixteen in total.
    obj_reset();
    g_obj_world.game_state = STATE_SCN_VICT_TERRAIN_ENH_COUNT_OBJ;
    for (int k = 0; k < 16; k++) {
        g_obj_world.tiles[k].territory = OFACTION;
        g_obj_world.tiles[k].bit = BIT_ROAD;
    }
    g_obj_world.tiles[16].territory = OFACTION;       // one past the last row
    g_obj_world.tiles[16].bit = BIT_ROAD;
    OCHECK(num_objectives(OFACTION, false) == 16);
    // Odd rows start at x == 1, so an ODD width visits one tile fewer on them.
    g_obj_world.lon_bounds = 7;
    OCHECK(num_objectives(OFACTION, false) == 14);

    // ---- plain territory ----------------------------------------------------
    obj_reset();
    for (int k = 0; k < 16; k++) {
        g_obj_world.tiles[k].territory = (int8_t)((k < 5 || k == 15) ? OFACTION : -1);
    }
    g_obj_world.tiles[16].territory = OFACTION;
    OCHECK(num_objectives(OFACTION, false) == 0);
    g_obj_world.game_state = STATE_SCN_VICT_TERRITORY_COUNT_OBJ;
    // Six: the first row's four, one in the second, and the very last tile,
    // which is only reached because the walk starts at row zero.
    OCHECK(num_objectives(OFACTION, false) == 6);
    // The same odd-width asymmetry as the enhancement walk.
    for (int k = 0; k < 16; k++) {
        g_obj_world.tiles[k].territory = (int8_t)OFACTION;
    }
    OCHECK(num_objectives(OFACTION, false) == 16);
    g_obj_world.lon_bounds = 7;
    OCHECK(num_objectives(OFACTION, false) == 14);

    // ---- secret projects the faction owns -----------------------------------
    obj_reset();
    obj_base(0, OFACTION, 0, 0);
    obj_base(1, OFACTION + 1, 2, 0);
    g_obj_world.projects.human_genome_project = 0;
    g_obj_world.projects.command_nexus = 1;
    g_obj_world.projects.weather_paradigm = 0;
    OCHECK(num_objectives(OFACTION, false) == 0);
    g_obj_world.game_rules = RULES_SCN_VICT_SP_COUNT_OBJ;
    OCHECK(num_objectives(OFACTION, false) == 2);
    // An unbuilt project is -1 and is not indexed.
    g_obj_world.projects.human_genome_project = SP_Unbuilt;
    g_obj_world.projects.weather_paradigm = SP_Destroyed;
    OCHECK(num_objectives(OFACTION, false) == 0);

    // ---- the one-deep recursion over pact partners --------------------------
    obj_reset();
    g_obj_world.players_data[OFACTION].unk_101 = 1;
    g_obj_world.players_data[2].unk_101 = 4;
    g_obj_world.players_data[3].unk_101 = 8;
    g_obj_world.players_data[OFACTION].diplo_treaties[2] = DTREATY_PACT;
    g_obj_world.players_data[OFACTION].diplo_treaties[3] = DTREATY_PACT;
    OCHECK(num_objectives(OFACTION, true) == 1);      // no cooperative victory
    g_obj_world.game_rules = RULES_VICTORY_COOPERATIVE;
    OCHECK(num_objectives(OFACTION, true) == 13);
    OCHECK(num_objectives(OFACTION, false) == 1);     // the flag gates it
    // A partner's own partners are NOT added: the recursion passes false.
    g_obj_world.players_data[2].diplo_treaties[3] = DTREATY_PACT;
    OCHECK(num_objectives(OFACTION, true) == 13);
    // A dead partner contributes nothing, and the faction never counts itself.
    g_obj_world.faction_status[1] = (uint8_t)~(1 << 3);
    OCHECK(num_objectives(OFACTION, true) == 5);
    obj_reset();
    g_obj_world.game_rules = RULES_VICTORY_COOPERATIVE;
    g_obj_world.players_data[OFACTION].unk_101 = 1;
    g_obj_world.players_data[OFACTION].diplo_treaties[OFACTION] = DTREATY_PACT;
    OCHECK(num_objectives(OFACTION, true) == 1);
    // Faction zero is outside the loop, which starts at one.
    obj_reset();
    g_obj_world.game_rules = RULES_VICTORY_COOPERATIVE;
    g_obj_world.players_data[0].unk_101 = 9;
    g_obj_world.players_data[OFACTION].diplo_treaties[0] = DTREATY_PACT;
    OCHECK(num_objectives(OFACTION, true) == 0);
    // ... and faction seven is inside it.
    obj_reset();
    g_obj_world.game_rules = RULES_VICTORY_COOPERATIVE;
    g_obj_world.players_data[7].unk_101 = 9;
    g_obj_world.players_data[OFACTION].diplo_treaties[7] = DTREATY_PACT;
    OCHECK(num_objectives(OFACTION, true) == 9);
}

#undef OCHECK

/*
 * A self-contained world for the spot_*() family.
 *
 * The map is 16 columns by 8 rows - MapLongitude 8, because two adjacent x
 * share one tile index - so the 64 live tiles fit inside an array a case can
 * assert over in full.
 *
 * MapTiles is aimed at the MIDDLE third of a 192-entry array, which is what
 * makes the bounds tests mean something. spot_tile() computes its tile index
 * before it would fault, so a mutant that drops the on_map() guard writes at
 * index -6 for y == -1 and index 66 for y == 8; both land inside the guard
 * thirds, where spot_world_untouched() sees them, instead of outside the
 * object where the behaviour would be undefined and the suite might still
 * pass.
 */
struct SpotWorld {
    Map tiles[192];
    Base bases[8];
    Veh vehs[16];
    PlayerData players_data[8];
    Map *tiles_ptr;
    uint32_t longitude;
    int lon_bounds;
    int lat_bounds;
    int local_faction;
    uint32_t game_state;
    uint32_t dirty;
    int base_count;
    int veh_count;
};

SpotWorld g_spot_world;

const int SPOT_LIVE = 64;  // index of tile (0, 0) inside SpotWorld::tiles

// The tile the live map addresses for (x, y).
Map &spot_at(int x, int y) {
    return g_spot_world.tiles[SPOT_LIVE + (x >> 1) + y * 8];
}

void spot_reset() {
    std::memset(&g_spot_world, 0, sizeof(g_spot_world));
    g_spot_world.tiles_ptr = &g_spot_world.tiles[SPOT_LIVE];
    g_spot_world.longitude = 8;
    g_spot_world.lon_bounds = 16;
    g_spot_world.lat_bounds = 8;
    g_spot_world.local_faction = 3;
    for (int i = 0; i < 16; i++) {
        g_spot_world.vehs[i].next_veh_id_stack = -1;
        g_spot_world.vehs[i].prev_veh_id_stack = -1;
    }
}

// True when no byte of any of the 192 map entries has been written.
bool spot_map_untouched() {
    const uint8_t *raw = reinterpret_cast<const uint8_t *>(g_spot_world.tiles);
    for (size_t i = 0; i < sizeof(g_spot_world.tiles); i++) {
        if (raw[i] != 0) {
            return false;
        }
    }
    return true;
}

// True when the only map entry written is the one the live map holds for
// (x, y). Catches a stride or sign error that lands on a neighbour.
bool spot_only_tile_written(int x, int y) {
    const Map *subject = &spot_at(x, y);
    for (int i = 0; i < 192; i++) {
        const Map *entry = &g_spot_world.tiles[i];
        if (entry == subject) {
            continue;
        }
        const uint8_t *raw = reinterpret_cast<const uint8_t *>(entry);
        for (size_t k = 0; k < sizeof(Map); k++) {
            if (raw[k] != 0) {
                return false;
            }
        }
    }
    return true;
}

#define PCHECK(cond)                                                          \
    do {                                                                      \
        const bool spot_ok = (cond);                                          \
        if (!spot_ok) {                                                       \
            std::fprintf(stderr, "spot: line %d: %s\n", __LINE__, #cond);     \
        }                                                                     \
        expect(spot_ok);                                                      \
    } while (0)

class SpotSeams {
 public:
    SpotSeams()
        : tiles_(&MapTiles, &g_spot_world.tiles_ptr),
          longitude_(&MapLongitude, &g_spot_world.longitude),
          lon_(&MapLongitudeBounds, &g_spot_world.lon_bounds),
          lat_(&MapLatitudeBounds, &g_spot_world.lat_bounds),
          local_(&LocalFaction, &g_spot_world.local_faction),
          state_(&GameState, &g_spot_world.game_state),
          players_(&PlayersData, g_spot_world.players_data),
          dirty_(&UnkBitfield1, &g_spot_world.dirty),
          bases_(&Bases, g_spot_world.bases),
          vehs_(&Vehs, g_spot_world.vehs),
          base_count_(&BaseCurrentCount, &g_spot_world.base_count),
          veh_count_(&VehCurrentCount, &g_spot_world.veh_count) { }

 private:
    ScopedSeam<Map *> tiles_;
    ScopedSeam<uint32_t> longitude_;
    ScopedSeam<int> lon_;
    ScopedSeam<int> lat_;
    ScopedSeam<int> local_;
    ScopedSeam<uint32_t> state_;
    ScopedSeam<PlayerData> players_;
    ScopedSeam<uint32_t> dirty_;
    ScopedSeam<Base> bases_;
    ScopedSeam<Veh> vehs_;
    ScopedSeam<int> base_count_;
    ScopedSeam<int> veh_count_;
};

void test_spot_tile() {
    SpotSeams seams;

    // The strides the original encodes as `imul 0x134` and `imul 0x34`. Every
    // indexed assertion below is only as good as these two.
    spot_reset();
    PCHECK(sizeof(Map) == 44);
    PCHECK(sizeof(Base) == 0x134);
    PCHECK(sizeof(Veh) == 0x34);

    // ---- the bounds guard, one edge at a time ------------------------------
    // Each of these four would write inside the 192-entry array if the guard
    // were not there, so a survivor here is a real miss and not a crash.
    const int off_map[4][2] = { {-1, 2}, {16, 2}, {4, -1}, {4, 8} };
    for (int c = 0; c < 4; c++) {
        spot_reset();
        spot_tile(off_map[c][0], off_map[c][1], 3);
        PCHECK(spot_map_untouched());
        PCHECK(g_spot_world.dirty == 0);
    }
    // ... and the four coordinates just inside those edges are all revealed.
    const int on_edge[4][2] = { {0, 2}, {15, 2}, {4, 0}, {4, 7} };
    for (int c = 0; c < 4; c++) {
        spot_reset();
        spot_tile(on_edge[c][0], on_edge[c][1], 3);
        PCHECK(spot_at(on_edge[c][0], on_edge[c][1]).visibility == 0x08);
        PCHECK(spot_only_tile_written(on_edge[c][0], on_edge[c][1]));
    }

    // ---- the plain reveal, by a faction that is not the local one ----------
    spot_reset();
    spot_at(6, 3).bit = BIT_FUNGUS | BIT_RIVER;
    spot_at(6, 3).bit2 = BIT2_MESA;
    spot_at(6, 3).visibility = 0x02;
    spot_tile(6, 3, 5);
    PCHECK(spot_at(6, 3).visibility == 0x22);           // 0x02 kept, 1 << 5 added
    PCHECK(spot_at(6, 3).bit2 == BIT2_MESA);            // no dirty bit: not local
    PCHECK(g_spot_world.dirty == 0);
    PCHECK(spot_at(6, 3).bit_visible[4] == (BIT_FUNGUS | BIT_RIVER));  // synch_bit
    PCHECK(spot_at(6, 3).bit_visible[3] == 0);
    PCHECK(spot_only_tile_written(6, 3));

    // Faction zero gets the visibility bit but no remembered terrain: the
    // faction-1 index in synch_bit() is why the whole call is skipped.
    spot_reset();
    spot_at(6, 3).bit = BIT_FUNGUS;
    spot_tile(6, 3, 0);
    PCHECK(spot_at(6, 3).visibility == 0x01);
    for (int k = 0; k < 7; k++) {
        PCHECK(spot_at(6, 3).bit_visible[k] == 0);
    }

    // ---- two adjacent x share one tile, an odd x is not a different tile ---
    spot_reset();
    spot_tile(6, 3, 1);
    PCHECK(spot_at(6, 3).visibility == 0x02);
    spot_tile(7, 3, 2);
    PCHECK(spot_at(6, 3).visibility == 0x06);
    PCHECK(spot_only_tile_written(6, 3));
    // The row stride is MapLongitude, not MapLongitudeBounds: (0, 1) is the
    // eighth entry, not the sixteenth.
    spot_reset();
    spot_tile(0, 1, 1);
    PCHECK(g_spot_world.tiles[SPOT_LIVE + 8].visibility == 0x02);
    PCHECK(spot_only_tile_written(0, 1));

    // ---- the local faction's first-sight repaint hint ----------------------
    spot_reset();
    spot_at(6, 3).bit2 = BIT2_MESA;
    g_spot_world.dirty = 0x80;
    spot_tile(6, 3, 3);                                  // 3 is the local faction
    PCHECK(spot_at(6, 3).bit2 == (BIT2_MESA | 0x400000)); // ORed in, not assigned
    PCHECK(g_spot_world.dirty == 0x81);                   // ORed in, not assigned
    PCHECK(spot_at(6, 3).visibility == 0x08);

    // Already visible to the local faction: the reveal still happens, the hint
    // does not.
    spot_reset();
    spot_at(6, 3).visibility = 0x08;
    spot_tile(6, 3, 3);
    PCHECK(spot_at(6, 3).visibility == 0x08);
    PCHECK(spot_at(6, 3).bit2 == 0);
    PCHECK(g_spot_world.dirty == 0);
    // The test is against the SUBJECT faction's bit, not any other faction's.
    spot_reset();
    spot_at(6, 3).visibility = 0x04;
    spot_tile(6, 3, 3);
    PCHECK(spot_at(6, 3).bit2 == 0x400000);
    PCHECK(g_spot_world.dirty == 1);

    // Not the local faction: no hint even on first sight.
    spot_reset();
    spot_tile(6, 3, 4);
    PCHECK(spot_at(6, 3).bit2 == 0);
    PCHECK(g_spot_world.dirty == 0);

    // Omniscient view suppresses the hint; a different GameState bit does not.
    spot_reset();
    g_spot_world.game_state = STATE_OMNISCIENT_VIEW;
    spot_tile(6, 3, 3);
    PCHECK(spot_at(6, 3).bit2 == 0);
    PCHECK(g_spot_world.dirty == 0);
    PCHECK(spot_at(6, 3).visibility == 0x08);
    spot_reset();
    g_spot_world.game_state = STATE_DEBUG_MODE;
    spot_tile(6, 3, 3);
    PCHECK(spot_at(6, 3).bit2 == 0x400000);

    // A revealed map suppresses the hint, and the flag is read from the
    // SUBJECT faction's row; a different faction's flag does not suppress it.
    spot_reset();
    g_spot_world.players_data[3].flags = PFLAG_MAP_REVEALED;
    spot_tile(6, 3, 3);
    PCHECK(spot_at(6, 3).bit2 == 0);
    PCHECK(g_spot_world.dirty == 0);
    PCHECK(spot_at(6, 3).visibility == 0x08);
    spot_reset();
    g_spot_world.players_data[2].flags = PFLAG_MAP_REVEALED;
    g_spot_world.players_data[4].flags = PFLAG_MAP_REVEALED;
    spot_tile(6, 3, 3);
    PCHECK(spot_at(6, 3).bit2 == 0x400000);
    // A different PlayerData flag does not suppress it either.
    spot_reset();
    g_spot_world.players_data[3].flags = PFLAG_GENETIC_PLAGUE_INTRO;
    spot_tile(6, 3, 3);
    PCHECK(spot_at(6, 3).bit2 == 0x400000);
}

void test_spot_base() {
    SpotSeams seams;

    // The ordinary sighting: visibility bit, remembered population, and the
    // tile the base stands on.
    spot_reset();
    g_spot_world.bases[2].x = 6;
    g_spot_world.bases[2].y = 3;
    g_spot_world.bases[2].population_size = 9;
    g_spot_world.bases[2].visibility = 0x02;
    spot_at(6, 3).bit = BIT_BASE_IN_TILE;
    spot_base(2, 5);
    PCHECK(g_spot_world.bases[2].visibility == 0x22);
    PCHECK(g_spot_world.bases[2].faction_pop_size_intel[5] == 9);
    PCHECK(spot_at(6, 3).visibility == 0x20);
    PCHECK(spot_at(6, 3).bit_visible[4] == BIT_BASE_IN_TILE);
    PCHECK(spot_only_tile_written(6, 3));
    // Only that faction's intel slot is written.
    for (int k = 0; k < 8; k++) {
        PCHECK(k == 5 || g_spot_world.bases[2].faction_pop_size_intel[k] == 0);
    }
    // And no neighbouring base row is touched.
    PCHECK(g_spot_world.bases[1].visibility == 0);
    PCHECK(g_spot_world.bases[3].visibility == 0);

    // The remembered size is the size at the moment of sighting, and it is
    // copied on EVERY sighting, not only the first.
    spot_reset();
    g_spot_world.bases[2].x = 6;
    g_spot_world.bases[2].y = 3;
    g_spot_world.bases[2].population_size = 4;
    spot_base(2, 5);
    PCHECK(g_spot_world.bases[2].faction_pop_size_intel[5] == 4);
    g_spot_world.bases[2].population_size = 7;
    spot_base(2, 5);
    PCHECK(g_spot_world.bases[2].faction_pop_size_intel[5] == 7);

    // A destroyed base carries a negative size; the copy is a byte move and
    // does not interpret it.
    spot_reset();
    g_spot_world.bases[2].x = 6;
    g_spot_world.bases[2].y = 3;
    g_spot_world.bases[2].population_size = -3;
    spot_base(2, 1);
    PCHECK(g_spot_world.bases[2].faction_pop_size_intel[1] == 0xFD);

    // A base off the map: the base fields are still written, the tile is not.
    spot_reset();
    g_spot_world.bases[2].x = -1;
    g_spot_world.bases[2].y = -1;
    g_spot_world.bases[2].population_size = 6;
    spot_base(2, 5);
    PCHECK(g_spot_world.bases[2].visibility == 0x20);
    PCHECK(g_spot_world.bases[2].faction_pop_size_intel[5] == 6);
    PCHECK(spot_map_untouched());

    // The base id indexes with the real 0x134 stride: base 7 is the last row
    // of the fixture and base 0 the first.
    spot_reset();
    g_spot_world.bases[7].x = 2;
    g_spot_world.bases[7].y = 5;
    g_spot_world.bases[7].population_size = 2;
    spot_base(7, 1);
    PCHECK(g_spot_world.bases[7].faction_pop_size_intel[1] == 2);
    PCHECK(spot_at(2, 5).visibility == 0x02);
    PCHECK(g_spot_world.bases[6].visibility == 0);

    // The local faction's first sight of a base still raises the repaint hint,
    // because the tile reveal is the same call.
    spot_reset();
    g_spot_world.bases[0].x = 6;
    g_spot_world.bases[0].y = 3;
    spot_base(0, 3);
    PCHECK(spot_at(6, 3).bit2 == 0x400000);
    PCHECK(g_spot_world.dirty == 1);
}

// Chain ids[0..n-1] into one stack, in order, and leave the ends terminated.
void spot_link(const int *ids, int n) {
    for (int i = 0; i < n; i++) {
        g_spot_world.vehs[ids[i]].prev_veh_id_stack =
            (int16_t)(i == 0 ? -1 : ids[i - 1]);
        g_spot_world.vehs[ids[i]].next_veh_id_stack =
            (int16_t)(i == n - 1 ? -1 : ids[i + 1]);
    }
}

void test_spot_stack() {
    SpotSeams seams;

    // A negative id returns before anything is read.
    spot_reset();
    spot_stack(-1, 3);
    PCHECK(spot_map_untouched());
    spot_stack(-9, 3);
    PCHECK(spot_map_untouched());

    // A lone unit: its tile is revealed and its own visibility bit set.
    spot_reset();
    g_spot_world.vehs[4].x = 6;
    g_spot_world.vehs[4].y = 3;
    g_spot_world.vehs[4].visibility = 0x02;
    spot_at(6, 3).bit = BIT_VEH_IN_TILE;
    spot_stack(4, 5);
    PCHECK(g_spot_world.vehs[4].visibility == 0x22);
    PCHECK(spot_at(6, 3).visibility == 0x20);
    PCHECK(spot_at(6, 3).bit_visible[4] == BIT_VEH_IN_TILE);
    PCHECK(spot_only_tile_written(6, 3));
    PCHECK(g_spot_world.vehs[3].visibility == 0);
    PCHECK(g_spot_world.vehs[5].visibility == 0);

    // A stack of three entered at the MIDDLE unit. The climb up
    // prev_veh_id_stack is what makes the other two visible; the coordinates
    // used for the tile are the NAMED unit's, not the top one's.
    spot_reset();
    const int chain[3] = {9, 4, 12};
    spot_link(chain, 3);
    g_spot_world.vehs[9].x = 0;
    g_spot_world.vehs[9].y = 0;
    g_spot_world.vehs[4].x = 6;
    g_spot_world.vehs[4].y = 3;
    g_spot_world.vehs[12].x = 2;
    g_spot_world.vehs[12].y = 5;
    spot_stack(4, 5);
    PCHECK(g_spot_world.vehs[9].visibility == 0x20);
    PCHECK(g_spot_world.vehs[4].visibility == 0x20);
    PCHECK(g_spot_world.vehs[12].visibility == 0x20);
    PCHECK(spot_only_tile_written(6, 3));

    // Entered at the bottom: the descent alone reaches nothing above, so the
    // climb has to have happened.
    spot_reset();
    spot_link(chain, 3);
    spot_stack(12, 1);
    PCHECK(g_spot_world.vehs[9].visibility == 0x02);
    PCHECK(g_spot_world.vehs[4].visibility == 0x02);
    PCHECK(g_spot_world.vehs[12].visibility == 0x02);
    // Entered at the top: the descent covers the whole stack.
    spot_reset();
    spot_link(chain, 3);
    spot_stack(9, 1);
    PCHECK(g_spot_world.vehs[9].visibility == 0x02);
    PCHECK(g_spot_world.vehs[4].visibility == 0x02);
    PCHECK(g_spot_world.vehs[12].visibility == 0x02);
    // A unit outside the stack keeps its own state.
    PCHECK(g_spot_world.vehs[0].visibility == 0);
    PCHECK(g_spot_world.vehs[11].visibility == 0);

    // The climb stops at a NEGATIVE prev, not at a false one: a stack whose
    // top is unit zero must still be climbed all the way, which `> 0` or
    // `>= 1` would not do.
    spot_reset();
    const int from_zero[3] = {0, 4, 12};
    spot_link(from_zero, 3);
    spot_stack(12, 1);
    PCHECK(g_spot_world.vehs[0].visibility == 0x02);
    PCHECK(g_spot_world.vehs[4].visibility == 0x02);
    PCHECK(g_spot_world.vehs[12].visibility == 0x02);
    // ... and the descent likewise runs to a negative next, through unit zero
    // sitting in the middle of a stack.
    spot_reset();
    const int through_zero[3] = {4, 0, 12};
    spot_link(through_zero, 3);
    spot_stack(4, 1);
    PCHECK(g_spot_world.vehs[4].visibility == 0x02);
    PCHECK(g_spot_world.vehs[0].visibility == 0x02);
    PCHECK(g_spot_world.vehs[12].visibility == 0x02);

    // Lurking and invisibility are cleared for every unit in the stack, and
    // only those two bits.
    spot_reset();
    spot_link(chain, 3);
    for (int i = 0; i < 3; i++) {
        g_spot_world.vehs[chain[i]].flags =
            VFLAG_LURKER | VFLAG_INVISIBLE | VFLAG_IS_OBJECTIVE
            | VFLAG_START_RAND_FUNGUS;
    }
    spot_stack(4, 1);
    for (int i = 0; i < 3; i++) {
        PCHECK(g_spot_world.vehs[chain[i]].flags
               == (VFLAG_IS_OBJECTIVE | VFLAG_START_RAND_FUNGUS));
    }
    // Faction zero leaves them alone - and still sets visibility.
    spot_reset();
    spot_link(chain, 3);
    for (int i = 0; i < 3; i++) {
        g_spot_world.vehs[chain[i]].flags = VFLAG_LURKER | VFLAG_INVISIBLE;
    }
    spot_stack(4, 0);
    for (int i = 0; i < 3; i++) {
        PCHECK(g_spot_world.vehs[chain[i]].flags
               == (VFLAG_LURKER | VFLAG_INVISIBLE));
        PCHECK(g_spot_world.vehs[chain[i]].visibility == 0x01);
    }

    // A unit off the map: the tile is left alone, the stack is still marked.
    spot_reset();
    spot_link(chain, 3);
    g_spot_world.vehs[4].x = -1;
    g_spot_world.vehs[4].y = 9;
    spot_stack(4, 1);
    PCHECK(spot_map_untouched());
    PCHECK(g_spot_world.vehs[9].visibility == 0x02);
    PCHECK(g_spot_world.vehs[12].visibility == 0x02);

    // The 0x34 stride, at both ends of the fixture.
    spot_reset();
    g_spot_world.vehs[15].x = 2;
    g_spot_world.vehs[15].y = 5;
    spot_stack(15, 1);
    PCHECK(g_spot_world.vehs[15].visibility == 0x02);
    PCHECK(g_spot_world.vehs[14].visibility == 0);
    PCHECK(spot_at(2, 5).visibility == 0x02);
    spot_reset();
    spot_stack(0, 1);
    PCHECK(g_spot_world.vehs[0].visibility == 0x02);
    PCHECK(g_spot_world.vehs[1].visibility == 0);
    PCHECK(spot_at(0, 0).visibility == 0x02);
}

/*
 * spot_loc reaches base_at and veh_at, and both have an error path that logs
 * and rebuilds the world's occupancy bits when a tile claims an occupant it
 * cannot find. Every case below therefore sets BIT_BASE_IN_TILE and
 * BIT_VEH_IN_TILE only on a tile that really does hold one, and keeps the two
 * counts in step with the fixture, so the suite measures spot_loc rather than
 * the recovery routines.
 */
void test_spot_loc() {
    SpotSeams seams;

    // ---- an empty tile: the tile is revealed and nothing else is -----------
    spot_reset();
    spot_at(6, 3).bit = BIT_RIVER;
    spot_loc(6, 3, 5);
    PCHECK(spot_at(6, 3).visibility == 0x20);
    PCHECK(spot_at(6, 3).bit_visible[4] == BIT_RIVER);
    PCHECK(spot_only_tile_written(6, 3));
    PCHECK(g_spot_world.bases[0].visibility == 0);
    PCHECK(g_spot_world.vehs[0].visibility == 0);

    // ---- off the map: no tile, no base, no unit ----------------------------
    const int off_map[4][2] = { {-1, 3}, {16, 3}, {6, -1}, {6, 8} };
    for (int c = 0; c < 4; c++) {
        spot_reset();
        spot_loc(off_map[c][0], off_map[c][1], 5);
        PCHECK(spot_map_untouched());
        PCHECK(g_spot_world.bases[0].visibility == 0);
        PCHECK(g_spot_world.vehs[0].visibility == 0);
    }

    // ---- a base on the tile ------------------------------------------------
    spot_reset();
    g_spot_world.base_count = 3;
    g_spot_world.bases[0].x = 2;
    g_spot_world.bases[0].y = 5;
    g_spot_world.bases[1].x = 6;
    g_spot_world.bases[1].y = 3;
    g_spot_world.bases[1].population_size = 8;
    g_spot_world.bases[2].x = 0;
    g_spot_world.bases[2].y = 6;
    spot_at(6, 3).bit = BIT_BASE_IN_TILE;
    spot_loc(6, 3, 5);
    PCHECK(g_spot_world.bases[1].visibility == 0x20);
    PCHECK(g_spot_world.bases[1].faction_pop_size_intel[5] == 8);
    PCHECK(spot_at(6, 3).visibility == 0x20);
    PCHECK(spot_only_tile_written(6, 3));
    // The bases either side of it are not the one that was found.
    PCHECK(g_spot_world.bases[0].visibility == 0);
    PCHECK(g_spot_world.bases[2].visibility == 0);

    // ---- a base WINS: the unit stack under it is left alone ----------------
    // This is the arm that returns. Without it the units would be spotted too.
    spot_reset();
    g_spot_world.base_count = 1;
    g_spot_world.bases[0].x = 6;
    g_spot_world.bases[0].y = 3;
    g_spot_world.bases[0].population_size = 4;
    g_spot_world.veh_count = 3;
    for (int i = 0; i < 3; i++) {
        g_spot_world.vehs[i].x = 6;
        g_spot_world.vehs[i].y = 3;
    }
    {
        const int chain[3] = {0, 1, 2};
        spot_link(chain, 3);
    }
    spot_at(6, 3).bit = BIT_BASE_IN_TILE | BIT_VEH_IN_TILE;
    spot_loc(6, 3, 5);
    PCHECK(g_spot_world.bases[0].faction_pop_size_intel[5] == 4);
    PCHECK(g_spot_world.vehs[0].visibility == 0);
    PCHECK(g_spot_world.vehs[1].visibility == 0);
    PCHECK(g_spot_world.vehs[2].visibility == 0);

    // ---- a unit stack, no base ---------------------------------------------
    // Entered at the middle unit, so the whole stack is only reached through
    // spot_stack's climb; veh_at hands over the top of the stack.
    spot_reset();
    g_spot_world.veh_count = 12;
    const int stack[3] = {11, 7, 2};
    spot_link(stack, 3);
    for (int i = 0; i < 3; i++) {
        g_spot_world.vehs[stack[i]].x = 6;
        g_spot_world.vehs[stack[i]].y = 3;
        g_spot_world.vehs[stack[i]].flags = VFLAG_LURKER | VFLAG_INVISIBLE;
    }
    spot_at(6, 3).bit = BIT_VEH_IN_TILE;
    spot_loc(6, 3, 5);
    PCHECK(g_spot_world.vehs[11].visibility == 0x20);
    PCHECK(g_spot_world.vehs[7].visibility == 0x20);
    PCHECK(g_spot_world.vehs[2].visibility == 0x20);
    for (int i = 0; i < 3; i++) {
        PCHECK(g_spot_world.vehs[stack[i]].flags == 0);
    }
    PCHECK(spot_at(6, 3).visibility == 0x20);
    PCHECK(spot_only_tile_written(6, 3));
    // No base was touched, and nothing outside the stack was.
    PCHECK(g_spot_world.bases[0].visibility == 0);
    PCHECK(g_spot_world.vehs[0].visibility == 0);
    PCHECK(g_spot_world.vehs[3].visibility == 0);

    // A tile whose occupancy bit is clear reveals only the tile, even with the
    // units sitting on it: base_at and veh_at both answer -1.
    spot_reset();
    g_spot_world.veh_count = 12;
    spot_link(stack, 3);
    for (int i = 0; i < 3; i++) {
        g_spot_world.vehs[stack[i]].x = 6;
        g_spot_world.vehs[stack[i]].y = 3;
    }
    spot_loc(6, 3, 5);
    PCHECK(spot_at(6, 3).visibility == 0x20);
    PCHECK(g_spot_world.vehs[2].visibility == 0);
    PCHECK(g_spot_world.vehs[7].visibility == 0);
    PCHECK(g_spot_world.vehs[11].visibility == 0);

    // ---- the local faction's repaint hint travels through spot_loc ---------
    spot_reset();
    spot_loc(6, 3, 3);
    PCHECK(spot_at(6, 3).bit2 == 0x400000);
    PCHECK(g_spot_world.dirty == 1);
    spot_reset();
    spot_loc(6, 3, 4);
    PCHECK(spot_at(6, 3).bit2 == 0);
    PCHECK(g_spot_world.dirty == 0);
}

#undef PCHECK

/*
 * A self-contained world for reset_territory.
 *
 * 16 columns by 8 rows - MapLongitude 8 - so the linear walk visits exactly 64
 * tiles and every one of them can be asserted over. The live map is again the
 * middle third of a 192-entry array, so a walk that runs past its last row
 * lands somewhere the assertions can see.
 *
 * Three fixture settings exist to make the callees decidable:
 *
 *  - GameRules carries RULES_NO_UNITY_SCATTERING, which takes goody_at()
 *    straight to its bitfield answer instead of its position hash, so a pod is
 *    exactly BIT_UNK_4000000 and nothing else is.
 *  - Every tile starts with val2 = 0x10, i.e. a stored site score of 1. That
 *    keeps world_site() out of the cases that are not about it; the cases that
 *    are clear the nibble deliberately.
 *  - territory_max_dist_base is 2 rather than the default 8, so one base claims
 *    nine tiles of a 64-tile map instead of most of it.
 */
struct TerrWorld {
    Map tiles[192];
    Base bases[8];
    // Nine faction rows and 129 continents: the last of each exists only to be
    // poisoned, so a loop bound that runs one too far leaves a mark instead of
    // writing over whatever the fixture happens to store next.
    PlayerData players_data[9];
    Continent continents[MaxContinentNum + 1];
    RulesBasic rules;
    Map *tiles_ptr;
    uint32_t longitude;
    int lon_bounds;
    int lat_bounds;
    BOOL is_flat;
    uint32_t map_rand_seed;
    uint32_t game_rules;
    uint32_t game_state;
    int diff_level;
    int base_count;
    int base_find_dist;
    uint8_t faction_status[2];
    uint32_t dirty;
    int turn;
};

TerrWorld g_terr_world;

const int TERR_LIVE = 64;  // index of tile (0, 0) inside TerrWorld::tiles

Map &terr_at(int x, int y) {
    return g_terr_world.tiles[TERR_LIVE + (x >> 1) + y * 8];
}

void terr_reset() {
    std::memset(&g_terr_world, 0, sizeof(g_terr_world));
    g_terr_world.tiles_ptr = &g_terr_world.tiles[TERR_LIVE];
    g_terr_world.longitude = 8;
    g_terr_world.lon_bounds = 16;
    g_terr_world.lat_bounds = 8;
    g_terr_world.is_flat = 1;
    g_terr_world.game_rules = RULES_NO_UNITY_SCATTERING;
    g_terr_world.diff_level = 1;
    g_terr_world.rules.territory_max_dist_base = 2;
    for (int y = 0; y < 8; y++) {
        for (int x = y & 1; x < 16; x += 2) {
            Map &tile = terr_at(x, y);
            tile.climate = 0x60;      // altitude 3, arid: dry land
            tile.territory = -1;      // unclaimed, so a no-change run is quiet
            tile.val2 = 0x10;         // stored site score of 1
        }
    }
}

// Put a base of the given faction on the map, wired so base_at() finds it
// rather than taking its bit-error path.
void terr_base(int base_id, int faction_id, int x, int y) {
    g_terr_world.bases[base_id].x = (int16_t)x;
    g_terr_world.bases[base_id].y = (int16_t)y;
    g_terr_world.bases[base_id].faction_id_current = (uint8_t)faction_id;
    terr_at(x, y).bit |= BIT_BASE_IN_TILE;
    if (g_terr_world.base_count <= base_id) {
        g_terr_world.base_count = base_id + 1;
    }
}

const uint16_t TERR_POISON16 = 0x7777;
const uint8_t TERR_POISON8 = 0x77;

/*
 * Fill every array the clear is supposed to reach with a sentinel, and every
 * array it is NOT supposed to reach - the one below unk_78, the one above
 * unk_83, the row past the last faction, the entry past the last region - with
 * a different one. Without this the clear is unobservable: the fixture starts
 * at zero, so a run that skips it agrees with a run that performs it.
 */
void terr_poison_tallies() {
    for (int f = 0; f < 9; f++) {
        PlayerData &player = g_terr_world.players_data[f];
        for (int r = 0; r < MaxContinentNum; r++) {
            player.unk_78[r] = TERR_POISON16;
            player.unk_79[r] = TERR_POISON16;
            player.unk_80[r] = TERR_POISON16;
            player.unk_81[r] = TERR_POISON16;
            player.unk_82[r] = TERR_POISON8;
            player.unk_83[r] = TERR_POISON8;
            player.unk_77[r] = 0x6666;          // immediately below unk_78
            player.region_base_plan[r] = 0x55;  // immediately above unk_83
        }
    }
    for (int r = 0; r <= MaxContinentNum; r++) {
        g_terr_world.continents[r].unk_3 = 9;
        g_terr_world.continents[r].pods = 9;
        g_terr_world.continents[r].tile_count = 0x1234;
    }
}

int terr_owned_count(int faction_id) {
    int total = 0;
    for (int y = 0; y < 8; y++) {
        for (int x = y & 1; x < 16; x += 2) {
            if (terr_at(x, y).territory == (int8_t)faction_id) {
                total++;
            }
        }
    }
    return total;
}

#define TCHECK(cond)                                                          \
    do {                                                                      \
        const bool terr_ok = (cond);                                          \
        if (!terr_ok) {                                                       \
            std::fprintf(stderr, "reset_territory: line %d: %s\n", __LINE__,  \
                         #cond);                                              \
        }                                                                     \
        expect(terr_ok);                                                      \
    } while (0)

class TerrSeams {
 public:
    TerrSeams()
        : tiles_(&MapTiles, &g_terr_world.tiles_ptr),
          longitude_(&MapLongitude, &g_terr_world.longitude),
          lon_(&MapLongitudeBounds, &g_terr_world.lon_bounds),
          lat_(&MapLatitudeBounds, &g_terr_world.lat_bounds),
          flat_(&MapIsFlat, &g_terr_world.is_flat),
          seed_(&MapRandSeed, &g_terr_world.map_rand_seed),
          rules_flags_(&GameRules, &g_terr_world.game_rules),
          state_(&GameState, &g_terr_world.game_state),
          diff_(&DiffLevelCurrent, &g_terr_world.diff_level),
          players_(&PlayersData, g_terr_world.players_data),
          continents_(&Continents, g_terr_world.continents),
          bases_(&Bases, g_terr_world.bases),
          base_count_(&BaseCurrentCount, &g_terr_world.base_count),
          base_dist_(&BaseFindDist, &g_terr_world.base_find_dist),
          status_(&FactionsStatus, g_terr_world.faction_status),
          dirty_(&UnkBitfield1, &g_terr_world.dirty),
          rules_(&Rules, &g_terr_world.rules),
          turn_(&TurnCurrentNum, &g_terr_world.turn) { }

 private:
    ScopedSeam<Map *> tiles_;
    ScopedSeam<uint32_t> longitude_;
    ScopedSeam<int> lon_;
    ScopedSeam<int> lat_;
    ScopedSeam<BOOL> flat_;
    ScopedSeam<uint32_t> seed_;
    ScopedSeam<uint32_t> rules_flags_;
    ScopedSeam<uint32_t> state_;
    ScopedSeam<int> diff_;
    ScopedSeam<PlayerData> players_;
    ScopedSeam<Continent> continents_;
    ScopedSeam<Base> bases_;
    ScopedSeam<int> base_count_;
    ScopedSeam<int> base_dist_;
    ScopedSeam<uint8_t> status_;
    ScopedSeam<uint32_t> dirty_;
    ScopedSeam<RulesBasic> rules_;
    ScopedSeam<int> turn_;
};

void test_reset_territory_ownership() {
    TerrSeams seams;

    // ---- an empty world: every tile unclaimed, and nothing reported dirty ---
    terr_reset();
    reset_territory();
    TCHECK(terr_owned_count(-1) == 64);
    TCHECK(g_terr_world.dirty == 0);
    TCHECK(g_terr_world.players_data[0].unk_78[0] == 64);
    TCHECK(g_terr_world.players_data[1].unk_78[0] == 0);
    // Site 1 everywhere, no pods.
    TCHECK(g_terr_world.continents[0].unk_3 == 1);
    TCHECK(g_terr_world.continents[0].pods == 0);

    // ---- one base claims by proximity --------------------------------------
    // territory_max_dist_base is 2, so the nine tiles at vector_dist 0 or 1
    // from (4, 2) are claimed and nothing else is.
    terr_reset();
    terr_base(0, 2, 4, 2);
    reset_territory();
    TCHECK(terr_owned_count(2) == 9);
    TCHECK(terr_owned_count(-1) == 55);
    TCHECK(terr_at(4, 2).territory == 2);
    TCHECK(terr_at(3, 1).territory == 2);
    TCHECK(terr_at(6, 2).territory == 2);
    TCHECK(terr_at(4, 4).territory == 2);
    TCHECK(terr_at(6, 4).territory == -1);   // vector_dist exactly 2
    TCHECK(terr_at(8, 2).territory == -1);
    TCHECK(g_terr_world.players_data[2].unk_78[0] == 9);
    TCHECK(g_terr_world.players_data[0].unk_78[0] == 55);
    // Nine tiles changed owner, so the repaint hint is up.
    TCHECK(g_terr_world.dirty == 1);
    TCHECK(terr_at(4, 2).bit2 == 0x400000);
    TCHECK(terr_at(8, 2).bit2 == 0);
    // Running it again over the answer it just wrote changes nothing and
    // reports nothing dirty: the hint is on the CHANGE, not on the claim.
    g_terr_world.dirty = 0;
    terr_at(4, 2).bit2 = 0;
    reset_territory();
    TCHECK(terr_owned_count(2) == 9);
    TCHECK(g_terr_world.dirty == 0);
    TCHECK(terr_at(4, 2).bit2 == 0);
    // The tallies are rebuilt from scratch each time, not accumulated.
    TCHECK(g_terr_world.players_data[2].unk_78[0] == 9);

    // ---- an ocean tile halves the distance ---------------------------------
    // (6, 4) is at vector_dist 2 from (4, 2) and has no base in its own ring,
    // which is the only place the two rules can be told apart: every tile at
    // vector_dist 1 from a base is also in that base's ring.
    terr_reset();
    g_terr_world.rules.territory_max_dist_base = 4;
    terr_base(0, 2, 4, 2);
    reset_territory();
    TCHECK(terr_at(6, 4).territory == 2);    // land: 2 is inside 4
    terr_reset();
    g_terr_world.rules.territory_max_dist_base = 4;
    terr_base(0, 2, 4, 2);
    terr_at(6, 4).climate = 0x40;            // ALT_OCEAN_SHELF: ocean
    reset_territory();
    TCHECK(terr_at(6, 4).territory == -1);   // ocean: 2 is not inside 4 >> 1

    // ---- and an adjacent base overrules the distance entirely --------------
    // The last RadiusBase entry is (0, 0), so an ocean tile with a base
    // standing on it claims it through the ring even when the halved distance
    // has already refused it: territory_max_dist_base 1 halves to 0 and
    // `dist < 0` is false at distance 0.
    terr_reset();
    g_terr_world.rules.territory_max_dist_base = 1;
    terr_base(0, 5, 7, 3);
    terr_at(7, 3).climate = 0x40;
    reset_territory();
    TCHECK(terr_at(7, 3).territory == 5);
    // The same holds one tile out, at RadiusBase offset (-1, 1).
    terr_reset();
    g_terr_world.rules.territory_max_dist_base = 1;
    terr_base(0, 5, 7, 3);
    terr_at(6, 4).climate = 0x40;
    reset_territory();
    TCHECK(terr_at(6, 4).territory == 5);
    // On LAND the ring is not consulted at all, and vector_dist 1 is not
    // inside a distance of 1.
    terr_reset();
    g_terr_world.rules.territory_max_dist_base = 1;
    terr_base(0, 5, 7, 3);
    reset_territory();
    TCHECK(terr_at(7, 3).territory == 5);    // the base's own tile, distance 0
    TCHECK(terr_at(6, 4).territory == -1);
    // The ring overrules a claim proximity already made, rather than only
    // filling in an unclaimed tile.
    terr_reset();
    terr_base(0, 2, 4, 2);
    terr_base(1, 5, 5, 1);                   // adjacent to (4, 2) itself
    terr_at(4, 2).climate = 0x40;
    reset_territory();
    TCHECK(terr_at(4, 2).territory == 5);

    // ---- a base of faction ZERO claims like any other ----------------------
    // The ownership base_find passes -1 as its faction_id_2, which excludes
    // nothing. A zero there would exclude exactly the faction-zero bases.
    terr_reset();
    terr_base(0, 0, 4, 2);
    reset_territory();
    TCHECK(terr_at(4, 2).territory == 0);
    TCHECK(terr_at(3, 1).territory == 0);
    TCHECK(terr_at(8, 2).territory == -1);

    // ---- the six tallies are CLEARED, not accumulated into -----------------
    terr_reset();
    terr_poison_tallies();
    // A ninth faction row that would tally every tile if the loops reached it.
    g_terr_world.players_data[8].flags = PFLAG_MAP_REVEALED;
    terr_base(0, 2, 4, 2);
    reset_territory();
    // Exactly the answer a clean world gives.
    TCHECK(g_terr_world.players_data[2].unk_78[0] == 9);
    TCHECK(g_terr_world.players_data[0].unk_78[0] == 55);
    TCHECK(g_terr_world.players_data[0].unk_78[1] == 0);
    TCHECK(g_terr_world.players_data[0].unk_79[0] == 0);
    TCHECK(g_terr_world.players_data[7].unk_79[3] == 0);
    TCHECK(g_terr_world.players_data[0].unk_80[0] == 0);
    TCHECK(g_terr_world.players_data[0].unk_81[0] == 0);
    TCHECK(g_terr_world.players_data[0].unk_82[0] == 0);
    TCHECK(g_terr_world.players_data[0].unk_83[0] == 0);
    TCHECK(g_terr_world.players_data[2].unk_82[0] == 1);
    TCHECK(g_terr_world.continents[0].unk_3 == 1);
    TCHECK(g_terr_world.continents[0].pods == 0);
    TCHECK(g_terr_world.continents[3].unk_3 == 0);
    TCHECK(g_terr_world.continents[3].pods == 0);
    // The arrays either side of the six are not in the clear.
    TCHECK(g_terr_world.players_data[0].unk_77[0] == 0x6666);
    TCHECK(g_terr_world.players_data[0].region_base_plan[0] == 0x55);
    TCHECK(g_terr_world.players_data[7].region_base_plan[127] == 0x55);
    TCHECK(g_terr_world.continents[3].tile_count == 0x1234);
    // Nor is the row past the last faction, nor the entry past the last
    // region: every one of the four loops stops one short of them.
    TCHECK(g_terr_world.players_data[8].unk_78[0] == TERR_POISON16);
    TCHECK(g_terr_world.players_data[8].unk_79[0] == TERR_POISON16);
    TCHECK(g_terr_world.players_data[8].unk_80[127] == TERR_POISON16);
    TCHECK(g_terr_world.players_data[8].unk_82[127] == TERR_POISON8);
    TCHECK(g_terr_world.continents[MaxContinentNum].unk_3 == 9);
    TCHECK(g_terr_world.continents[MaxContinentNum].pods == 9);

    // ---- the walk's shape --------------------------------------------------
    // One Map entry per tile visited, never reset between rows. On an odd
    // width an odd row visits one tile fewer, so the walk covers 60 rather
    // than 64 and the last four entries are never reached.
    terr_reset();
    g_terr_world.lon_bounds = 15;
    reset_territory();
    TCHECK(g_terr_world.players_data[0].unk_78[0] == 60);
    // A zero-height map walks nothing at all.
    terr_reset();
    g_terr_world.lat_bounds = 0;
    reset_territory();
    TCHECK(g_terr_world.players_data[0].unk_78[0] == 0);
    TCHECK(g_terr_world.continents[0].unk_3 == 0);
}

void test_reset_territory_tallies() {
    TerrSeams seams;

    // ---- the visibility tally, and the three ways into it ------------------
    // Faction 2 sees four tiles.
    terr_reset();
    terr_at(0, 0).visibility = 0x04;
    terr_at(2, 0).visibility = 0x04;
    terr_at(4, 0).visibility = 0x0C;
    terr_at(6, 0).visibility = 0x04;
    reset_territory();
    TCHECK(g_terr_world.players_data[2].unk_79[0] == 4);
    TCHECK(g_terr_world.players_data[3].unk_79[0] == 1);
    TCHECK(g_terr_world.players_data[1].unk_79[0] == 0);
    // Faction zero is outside the loop, which starts at one.
    terr_reset();
    for (int y = 0; y < 8; y++) {
        for (int x = y & 1; x < 16; x += 2) {
            terr_at(x, y).visibility = 0xFF;
        }
    }
    reset_territory();
    TCHECK(g_terr_world.players_data[0].unk_79[0] == 0);
    TCHECK(g_terr_world.players_data[1].unk_79[0] == 64);
    TCHECK(g_terr_world.players_data[7].unk_79[0] == 64);
    // A revealed map counts every tile without any visibility bit.
    terr_reset();
    g_terr_world.players_data[3].flags = PFLAG_MAP_REVEALED;
    reset_territory();
    TCHECK(g_terr_world.players_data[3].unk_79[0] == 64);
    TCHECK(g_terr_world.players_data[2].unk_79[0] == 0);
    // The objectives strategy SUBSTITUTES a different question: it counts
    // BIT_SUPPLY_REMOVE tiles and stops counting visible ones.
    terr_reset();
    g_terr_world.players_data[3].flags =
        PFLAG_STRAT_SEARCH_OBJECTIVES | PFLAG_MAP_REVEALED;
    terr_at(0, 0).bit = BIT_SUPPLY_REMOVE;
    terr_at(2, 0).bit = BIT_SUPPLY_REMOVE;
    for (int y = 0; y < 8; y++) {
        for (int x = y & 1; x < 16; x += 2) {
            terr_at(x, y).visibility = 0xFF;
        }
    }
    reset_territory();
    TCHECK(g_terr_world.players_data[3].unk_79[0] == 2);
    TCHECK(g_terr_world.players_data[2].unk_79[0] == 64);

    // ---- the quality tallies -----------------------------------------------
    // Wet, not rocky, no fungus. Everything else in the fixture is arid.
    terr_reset();
    terr_at(0, 0).climate = 0x60 | RAINFALL_MOIST;
    terr_at(2, 0).climate = 0x60 | RAINFALL_RAINY;
    terr_at(4, 0).climate = 0x60 | RAINFALL_MOIST;
    terr_at(4, 0).val3 = ROCKINESS_ROCKY << 6;         // rocky: not counted
    terr_at(6, 0).climate = 0x60 | RAINFALL_MOIST;
    terr_at(6, 0).val3 = ROCKINESS_ROLLING << 6;       // rolling: counted
    terr_at(8, 0).climate = 0x60 | RAINFALL_MOIST;
    terr_at(8, 0).bit = BIT_FUNGUS;                    // fungus: not counted
    terr_at(10, 0).climate = 0x60 | RAINFALL_MOIST;
    terr_at(10, 0).bit = BIT_BASE_RADIUS;              // counted, and again
    reset_territory();
    TCHECK(g_terr_world.players_data[0].unk_80[0] == 4);
    TCHECK(g_terr_world.players_data[0].unk_81[0] == 1);
    // They follow the owner, not faction zero.
    terr_reset();
    terr_base(0, 2, 4, 2);
    terr_at(4, 2).climate = 0x60 | RAINFALL_MOIST;
    terr_at(4, 2).bit |= BIT_BASE_RADIUS;
    reset_territory();
    TCHECK(g_terr_world.players_data[2].unk_80[0] == 1);
    TCHECK(g_terr_world.players_data[2].unk_81[0] == 1);
    TCHECK(g_terr_world.players_data[0].unk_80[0] == 0);

    // ---- everything is indexed by the tile's own region --------------------
    terr_reset();
    terr_at(0, 0).region = 7;
    terr_at(2, 0).region = 7;
    terr_at(0, 0).climate = 0x60 | RAINFALL_MOIST;
    reset_territory();
    TCHECK(g_terr_world.players_data[0].unk_78[7] == 2);
    TCHECK(g_terr_world.players_data[0].unk_78[0] == 62);
    TCHECK(g_terr_world.players_data[0].unk_80[7] == 1);
    TCHECK(g_terr_world.players_data[0].unk_80[0] == 0);
    TCHECK(g_terr_world.continents[7].unk_3 == 1);

    // ---- the site score ----------------------------------------------------
    // A tile already inside a base radius is not scored at all, and its zero
    // is what the region maximum sees.
    terr_reset();
    for (int y = 0; y < 8; y++) {
        for (int x = y & 1; x < 16; x += 2) {
            terr_at(x, y).bit = BIT_BASE_RADIUS;
        }
    }
    reset_territory();
    TCHECK(g_terr_world.continents[0].unk_3 == 0);
    // The stored score is used when there is one; the maximum over the region
    // is what lands in the Continent.
    terr_reset();
    terr_at(0, 0).val2 = 0x70;
    terr_at(2, 0).val2 = 0x30;
    reset_territory();
    TCHECK(g_terr_world.continents[0].unk_3 == 7);
    TCHECK(terr_at(0, 0).val2 == 0x70);            // untouched, not rescored
    // A stored score of zero is computed and CACHED with site_set, so the
    // nibble is written back.
    terr_reset();
    terr_at(0, 0).val2 = 0x00;
    reset_territory();
    TCHECK((terr_at(0, 0).val2 >> 4) != 0);
    TCHECK(g_terr_world.continents[0].unk_3 == (terr_at(0, 0).val2 >> 4u));
    TCHECK((terr_at(0, 0).val2 & 0x0F) == 0);      // low nibble preserved
    // The owner's per-region best is the maximum of its OWN tiles only.
    terr_reset();
    terr_base(0, 2, 4, 2);
    terr_at(4, 2).val2 = 0x50;
    terr_at(0, 0).val2 = 0xF0;                     // unclaimed, and higher
    reset_territory();
    TCHECK(g_terr_world.players_data[2].unk_82[0] == 5);
    TCHECK(g_terr_world.continents[0].unk_3 == 15);
    // Faction zero never gets one, even though unclaimed tiles tally under it.
    TCHECK(g_terr_world.players_data[0].unk_82[0] == 0);
}

void test_reset_territory_sites() {
    TerrSeams seams;

    // ---- supply pods are counted per region and per owner ------------------
    terr_reset();
    terr_at(0, 0).bit = BIT_UNK_4000000;
    terr_at(2, 0).bit = BIT_UNK_4000000;
    terr_at(4, 0).bit = BIT_UNK_4000000 | BIT_SUPPLY_REMOVE;  // already opened
    reset_territory();
    TCHECK(g_terr_world.continents[0].pods == 2);
    TCHECK(g_terr_world.players_data[0].unk_83[0] == 2);

    // ---- an owned pod asks its owner for a site ----------------------------
    // The owner must be able to see the tile, or be an AI above difficulty 3.
    terr_reset();
    terr_base(0, 2, 4, 2);
    terr_at(4, 2).bit |= BIT_UNK_4000000;
    reset_territory();
    TCHECK(g_terr_world.players_data[2].unk_83[0] == 1);
    TCHECK(!at_site(2, 3, 4, 2));                  // invisible, easy AI: no
    terr_reset();
    terr_base(0, 2, 4, 2);
    terr_at(4, 2).bit |= BIT_UNK_4000000;
    terr_at(4, 2).visibility = 0x04;
    reset_territory();
    TCHECK(at_site(2, 3, 4, 2));
    TCHECK(g_terr_world.players_data[2].sites[0].priority == 1);
    TCHECK(!at_site(1, 3, 4, 2));                  // and only for the owner
    // An unseen tile still reaches the site list for an AI above difficulty 3.
    terr_reset();
    terr_base(0, 2, 4, 2);
    terr_at(4, 2).bit |= BIT_UNK_4000000;
    g_terr_world.diff_level = 4;
    reset_territory();
    TCHECK(at_site(2, 3, 4, 2));
    // ... but not for a human faction, whatever the difficulty.
    terr_reset();
    terr_base(0, 2, 4, 2);
    terr_at(4, 2).bit |= BIT_UNK_4000000;
    g_terr_world.diff_level = 4;
    g_terr_world.faction_status[0] = 0x04;         // faction 2 is human
    reset_territory();
    TCHECK(!at_site(2, 3, 4, 2));
    // ... and difficulty 3 is not ABOVE 3, which is where the edge is.
    terr_reset();
    terr_base(0, 2, 4, 2);
    terr_at(4, 2).bit |= BIT_UNK_4000000;
    g_terr_world.diff_level = 3;
    reset_territory();
    TCHECK(!at_site(2, 3, 4, 2));

    // ---- an unclaimed pod asks every faction with a base in the region -----
    // Faction 2's base at (2, 0) is vector_dist 4 from the pod at (8, 0):
    // that is at least territory_max_dist_base of 2 and at most twice it,
    // which is the window that arm requires. Faction 3 has no base in the
    // region and is not asked at all.
    terr_reset();
    terr_base(0, 2, 2, 0);
    g_terr_world.players_data[2].region_total_bases[0] = 1;
    g_terr_world.players_data[3].region_total_bases[0] = 0;
    terr_at(8, 0).bit = BIT_UNK_4000000;
    terr_at(8, 0).visibility = 0xFF;
    reset_territory();
    TCHECK(terr_at(8, 0).territory == -1);         // genuinely unclaimed
    TCHECK(at_site(2, 3, 8, 0));
    TCHECK(!at_site(3, 3, 8, 0));
    // Without the region base count the faction is skipped outright.
    terr_reset();
    terr_base(0, 2, 2, 0);
    terr_at(8, 0).bit = BIT_UNK_4000000;
    terr_at(8, 0).visibility = 0xFF;
    reset_territory();
    TCHECK(!at_site(2, 3, 8, 0));
    // Nor is an unseen tile offered on an easy difficulty; 3 is not above 3.
    terr_reset();
    terr_base(0, 2, 2, 0);
    g_terr_world.players_data[2].region_total_bases[0] = 1;
    terr_at(8, 0).bit = BIT_UNK_4000000;
    g_terr_world.diff_level = 3;
    reset_territory();
    TCHECK(!at_site(2, 3, 8, 0));
    g_terr_world.diff_level = 4;
    reset_territory();
    TCHECK(at_site(2, 3, 8, 0));

    // ---- the loop's own two ends -------------------------------------------
    // It starts at faction ONE. Faction zero holds a base in the region and
    // can see the tile, and is still never asked.
    terr_reset();
    terr_base(0, 0, 2, 0);
    g_terr_world.players_data[0].region_total_bases[0] = 1;
    terr_at(8, 0).bit = BIT_UNK_4000000;
    terr_at(8, 0).visibility = 0xFF;
    reset_territory();
    TCHECK(terr_at(8, 0).territory == -1);
    TCHECK(!at_site(0, 3, 8, 0));
    // ... and it stops at faction SEVEN. The synthetic ninth row - which the
    // game has no such faction for - is given a base in the region and a base
    // of its own inside the window, so reaching it would leave a mark.
    terr_reset();
    terr_base(0, 2, 2, 0);
    terr_base(1, 8, 14, 0);
    g_terr_world.players_data[2].region_total_bases[0] = 1;
    g_terr_world.players_data[8].region_total_bases[0] = 1;
    g_terr_world.diff_level = 4;
    terr_at(8, 0).bit = BIT_UNK_4000000;
    reset_territory();
    TCHECK(at_site(2, 3, 8, 0));
    TCHECK(g_terr_world.players_data[8].sites[0].type == 0);

    // ---- the inner base_find asks for THIS faction's bases only ------------
    // A faction-zero base one tile from the pod is nearer than faction two's,
    // and would take BaseFindDist below the window if it were allowed to
    // match. It also owns the tile, and owner zero still takes the
    // every-faction branch.
    terr_reset();
    terr_base(0, 2, 2, 0);
    terr_base(1, 0, 6, 0);
    g_terr_world.players_data[2].region_total_bases[0] = 1;
    terr_at(8, 0).bit = BIT_UNK_4000000;
    terr_at(8, 0).visibility = 0xFF;
    reset_territory();
    TCHECK(terr_at(8, 0).territory == 0);
    TCHECK(at_site(2, 3, 8, 0));

    // ---- the window's lower edge is inclusive ------------------------------
    // A base at exactly territory_max_dist_base still asks for the site; the
    // tile it is that far from is not one it has claimed.
    terr_reset();
    terr_base(0, 2, 6, 2);                         // vector_dist 2 from (8, 0)
    g_terr_world.players_data[2].region_total_bases[0] = 1;
    terr_at(8, 0).bit = BIT_UNK_4000000;
    terr_at(8, 0).visibility = 0xFF;
    reset_territory();
    TCHECK(terr_at(8, 0).territory == -1);
    TCHECK(at_site(2, 3, 8, 0));
    // Too close: a base within territory_max_dist_base of the pod would have
    // claimed the region already, so no site is requested.
    terr_reset();
    terr_base(0, 2, 6, 0);                         // vector_dist 1 from (8, 0)
    g_terr_world.players_data[2].region_total_bases[0] = 1;
    terr_at(8, 0).bit = BIT_UNK_4000000;
    terr_at(8, 0).visibility = 0xFF;
    reset_territory();
    TCHECK(terr_at(8, 0).territory == 2);
    // Too far: beyond twice the distance and it is somebody else's problem.
    terr_reset();
    terr_base(0, 2, 0, 0);
    g_terr_world.players_data[2].region_total_bases[0] = 1;
    terr_at(14, 6).bit = BIT_UNK_4000000;
    terr_at(14, 6).visibility = 0xFF;
    reset_territory();
    TCHECK(!at_site(2, 3, 14, 6));
    // Already holding a site here: not asked again. The pre-installed site
    // carries priority 0, which add_site would raise to 1 if it ever ran, so
    // the assertion can tell the guard from its absence.
    terr_reset();
    terr_base(0, 2, 2, 0);
    g_terr_world.players_data[2].region_total_bases[0] = 1;
    terr_at(8, 0).bit = BIT_UNK_4000000;
    terr_at(8, 0).visibility = 0xFF;
    g_terr_world.players_data[2].sites[0].type = 3;
    g_terr_world.players_data[2].sites[0].priority = 0;
    g_terr_world.players_data[2].sites[0].x = 8;
    g_terr_world.players_data[2].sites[0].y = 0;
    reset_territory();
    TCHECK(g_terr_world.players_data[2].sites[0].priority == 0);  // untouched
}

#undef TCHECK

/*
 * A self-contained world for territory().
 *
 * Same 16-by-8 map as reset_territory's, for the same reason: MapLongitude 8
 * with the live tiles in the middle third of a 192-entry array, so a walk that
 * runs off the end lands somewhere the assertions can see.
 *
 * The scene tres_scene() builds is the one every case starts from: faction 1
 * owns every tile through its base at (4, 2), and one ordinary combat unit of
 * faction 2 stands at (6, 2), two tiles away. That unit is counted, which is
 * what makes a case that expects zero evidence rather than an empty world.
 *
 * Three fixture settings exist to keep the callees decidable:
 *
 *  - Every tile is dry land (climate 0x60) in region 2, so region_at() and the
 *    altitude gate answer the same way everywhere until a case changes one.
 *  - SunspotDuration starts at 1. Sunspots down is a once-per-unit grace pass
 *    that suppresses the weighting, so leaving them up is what lets the
 *    ordinary weight be asserted at all.
 *  - The five prototypes cover exactly the plans the filter distinguishes.
 */
struct TrespassWorld {
    Map tiles[192];
    Base bases[8];
    Veh vehs[16];
    VehPrototype protos[8];
    // Nine faction rows: the ninth exists only to be poisoned, so a scan that
    // runs one faction too far leaves a mark instead of reading whatever the
    // fixture happens to store next.
    PlayerData players_data[9];
    Map *tiles_ptr;
    uint32_t longitude;
    int lon_bounds;
    int lat_bounds;
    BOOL is_flat;
    int base_count;
    int base_find_dist;
    int veh_count;
    int sunspots;
    int friction;
    uint32_t game_state;
};

TrespassWorld g_tres_world;

const int TRES_LIVE = 64;  // index of tile (0, 0) inside TrespassWorld::tiles

Map &tres_at(int x, int y) {
    return g_tres_world.tiles[TRES_LIVE + (x >> 1) + y * 8];
}

void tres_reset() {
    std::memset(&g_tres_world, 0, sizeof(g_tres_world));
    g_tres_world.tiles_ptr = &g_tres_world.tiles[TRES_LIVE];
    g_tres_world.longitude = 8;
    g_tres_world.lon_bounds = 16;
    g_tres_world.lat_bounds = 8;
    g_tres_world.is_flat = 1;
    g_tres_world.sunspots = 1;
    for (int y = 0; y < 8; y++) {
        for (int x = y & 1; x < 16; x += 2) {
            Map &tile = tres_at(x, y);
            tile.climate = 0x60;  // altitude 3, arid: dry land
            tile.region = 2;
            tile.territory = -1;
        }
    }
    g_tres_world.protos[0].plan = PLAN_OFFENSIVE;      // 0, well under the cut
    g_tres_world.protos[1].plan = PLAN_COLONIZATION;   // 8, the last one allowed
    g_tres_world.protos[2].plan = PLAN_TERRAFORMING;   // 9, the first refused
    g_tres_world.protos[3].plan = PLAN_SUPPLY_CONVOY;  // 10, allowed while convoying
    g_tres_world.protos[4].plan = PLAN_INFO_WARFARE;   // 11
}

void tres_claim(int faction_id) {
    for (int y = 0; y < 8; y++) {
        for (int x = y & 1; x < 16; x += 2) {
            tres_at(x, y).territory = (int8_t)faction_id;
        }
    }
}

void tres_base(int base_id, int faction_id, int x, int y) {
    g_tres_world.bases[base_id].x = (int16_t)x;
    g_tres_world.bases[base_id].y = (int16_t)y;
    g_tres_world.bases[base_id].faction_id_current = (uint8_t)faction_id;
    if (g_tres_world.base_count <= base_id) {
        g_tres_world.base_count = base_id + 1;
    }
}

void tres_veh(int veh_id, int faction_id, int proto_id, int x, int y) {
    Veh &veh = g_tres_world.vehs[veh_id];
    veh.x = (int16_t)x;
    veh.y = (int16_t)y;
    veh.faction_id = (uint8_t)faction_id;
    veh.proto_id = (int16_t)proto_id;
    if (g_tres_world.veh_count <= veh_id) {
        g_tres_world.veh_count = veh_id + 1;
    }
}

void tres_scene() {
    tres_reset();
    tres_claim(1);
    tres_base(0, 1, 4, 2);
    tres_veh(0, 2, 0, 6, 2);
}

// Faction other_id is at vendetta with both 1 and 2 and holds a base in the
// named region, which is the whole of the shared-war exemption.
void tres_share(int other_id, int region) {
    g_tres_world.players_data[1].diplo_treaties[other_id] |= DTREATY_VENDETTA;
    g_tres_world.players_data[2].diplo_treaties[other_id] |= DTREATY_VENDETTA;
    g_tres_world.players_data[other_id].region_total_bases[region] = 1;
}

#define RCHECK(cond)                                                          \
    do {                                                                      \
        const bool tres_ok = (cond);                                          \
        if (!tres_ok) {                                                       \
            std::fprintf(stderr, "territory: line %d: %s\n", __LINE__,        \
                         #cond);                                              \
        }                                                                     \
        expect(tres_ok);                                                      \
    } while (0)

class TresSeams {
 public:
    TresSeams()
        : tiles_(&MapTiles, &g_tres_world.tiles_ptr),
          longitude_(&MapLongitude, &g_tres_world.longitude),
          lon_(&MapLongitudeBounds, &g_tres_world.lon_bounds),
          lat_(&MapLatitudeBounds, &g_tres_world.lat_bounds),
          flat_(&MapIsFlat, &g_tres_world.is_flat),
          state_(&GameState, &g_tres_world.game_state),
          players_(&PlayersData, g_tres_world.players_data),
          bases_(&Bases, g_tres_world.bases),
          base_count_(&BaseCurrentCount, &g_tres_world.base_count),
          base_dist_(&BaseFindDist, &g_tres_world.base_find_dist),
          vehs_(&Vehs, g_tres_world.vehs),
          veh_count_(&VehCurrentCount, &g_tres_world.veh_count),
          protos_(&VehPrototypes, g_tres_world.protos),
          sunspots_(&SunspotDuration, &g_tres_world.sunspots),
          friction_(&DiploFriction, &g_tres_world.friction) { }

 private:
    ScopedSeam<Map *> tiles_;
    ScopedSeam<uint32_t> longitude_;
    ScopedSeam<int> lon_;
    ScopedSeam<int> lat_;
    ScopedSeam<BOOL> flat_;
    ScopedSeam<uint32_t> state_;
    ScopedSeam<PlayerData> players_;
    ScopedSeam<Base> bases_;
    ScopedSeam<int> base_count_;
    ScopedSeam<int> base_dist_;
    ScopedSeam<Veh> vehs_;
    ScopedSeam<int> veh_count_;
    ScopedSeam<VehPrototype> protos_;
    ScopedSeam<int> sunspots_;
    ScopedSeam<int> friction_;
};

void test_territory_gates() {
    TresSeams seams;
    int base_id = 0;
    int count = 0;

    // ---- both outputs are cleared before anything can refuse --------------
    // A pact answers zero, and it answers it before the unit walk: the stale
    // mark on the unit is still there afterwards.
    tres_scene();
    g_tres_world.players_data[1].diplo_treaties[2] = DTREATY_PACT;
    g_tres_world.vehs[0].state = VSTATE_UNK_400;
    base_id = 0x5A5A;
    count = 0x5A5A;
    RCHECK(territory(1, 2, 1, &base_id, &count) == 0);
    RCHECK(base_id == -1);
    RCHECK(count == 0);
    RCHECK(g_tres_world.vehs[0].state == VSTATE_UNK_400);
    RCHECK((g_tres_world.players_data[1].flags & PFLAG_UNK_10000) == 0);

    // A vendetta is the other half of the same gate.
    tres_scene();
    g_tres_world.players_data[1].diplo_treaties[2] = DTREATY_VENDETTA;
    base_id = 0x5A5A;
    count = 0x5A5A;
    RCHECK(territory(1, 2, 1, &base_id, &count) == 0);
    RCHECK(base_id == -1);
    RCHECK(count == 0);

    // ---- the ordinary trespass, which every zero above is measured against -
    tres_scene();
    base_id = 0x5A5A;
    count = 0x5A5A;
    RCHECK(territory(1, 2, 1, &base_id, &count) == 2);
    RCHECK(base_id == 0);
    RCHECK(count == 0);  // the unit does not carry VFLAG bit 0
    RCHECK((g_tres_world.players_data[1].flags & PFLAG_UNK_10000) != 0);

    // The gate is those two bits and no others: a truce passes through it.
    tres_scene();
    g_tres_world.players_data[1].diplo_treaties[2] = DTREATY_TRUCE;
    RCHECK(territory(1, 2, 1, &base_id, &count) == 2);

    // The treaty read is the owner's view of the trespasser. The reverse
    // direction carries the pact here and is ignored.
    tres_scene();
    g_tres_world.players_data[2].diplo_treaties[1] = DTREATY_PACT | DTREATY_VENDETTA;
    RCHECK(territory(1, 2, 1, &base_id, &count) == 2);

    // ---- DTREATY_UNK_200 refuses unless bit 1 of flags overrides it -------
    tres_scene();
    g_tres_world.players_data[1].diplo_treaties[2] = DTREATY_UNK_200;
    RCHECK(territory(1, 2, 1, &base_id, &count) == 0);
    tres_scene();
    g_tres_world.players_data[1].diplo_treaties[2] = DTREATY_UNK_200;
    RCHECK(territory(1, 2, 3, &base_id, &count) == 2);
    // Bit 1 alone: the override without the weighting the other bit arms.
    tres_scene();
    g_tres_world.players_data[1].diplo_treaties[2] = DTREATY_UNK_200;
    RCHECK(territory(1, 2, 2, &base_id, &count) == 1);
    // A neighbouring treaty bit is not that one.
    tres_scene();
    g_tres_world.players_data[1].diplo_treaties[2] = DTREATY_UNK_100;
    RCHECK(territory(1, 2, 1, &base_id, &count) == 2);

    // ---- both outputs are optional ----------------------------------------
    tres_scene();
    RCHECK(territory(1, 2, 1, NULL, NULL) == 2);
    tres_scene();
    g_tres_world.vehs[0].flags = 1;
    RCHECK(territory(1, 2, 1, NULL, NULL) == 2);
    RCHECK(g_tres_world.vehs[0].flags == 3);

    // ---- the flag is published only when there is something to publish ----
    tres_scene();
    g_tres_world.players_data[1].flags = PFLAG_BEEN_ELECTED_GOVERNOR;
    RCHECK(territory(1, 2, 1, &base_id, &count) == 2);
    RCHECK(g_tres_world.players_data[1].flags
           == (PFLAG_BEEN_ELECTED_GOVERNOR | PFLAG_UNK_10000));
    // Nothing found: the flag stays exactly as it was.
    tres_scene();
    g_tres_world.vehs[0].faction_id = 3;
    g_tres_world.players_data[1].flags = PFLAG_BEEN_ELECTED_GOVERNOR;
    RCHECK(territory(1, 2, 1, &base_id, &count) == 0);
    RCHECK(g_tres_world.players_data[1].flags == PFLAG_BEEN_ELECTED_GOVERNOR);
}

void test_territory_shared_war() {
    TresSeams seams;
    int base_id = 0;
    int count = 0;

    // ---- a common enemy with a base in the region exempts the trespass ----
    tres_scene();
    tres_share(3, 2);
    RCHECK(territory(1, 2, 1, &base_id, &count) == 0);
    RCHECK((g_tres_world.vehs[0].state & VSTATE_UNK_400) == 0);
    RCHECK(base_id == -1);

    // Its bases are somewhere else: the exemption is per region, not global.
    tres_scene();
    tres_share(3, 3);
    RCHECK(territory(1, 2, 1, &base_id, &count) == 2);

    // The region read is the unit's tile, not a constant.
    tres_scene();
    tres_share(3, 3);
    tres_at(6, 2).region = 3;
    RCHECK(territory(1, 2, 1, &base_id, &count) == 0);

    // ---- the scan runs 1 through 7 ----------------------------------------
    // Faction zero is the native life faction and is never the third party.
    tres_scene();
    tres_share(0, 2);
    RCHECK(territory(1, 2, 1, &base_id, &count) == 2);
    // Faction seven is.
    tres_scene();
    tres_share(7, 2);
    RCHECK(territory(1, 2, 1, &base_id, &count) == 0);
    // One row past the last faction, wired so a scan that reached it would
    // exempt: diplo_treaties[8] is diplo_agenda[0], and the ninth PlayerData
    // row carries the base.
    tres_scene();
    g_tres_world.players_data[1].diplo_agenda[0] = DTREATY_VENDETTA;
    g_tres_world.players_data[2].diplo_agenda[0] = DTREATY_VENDETTA;
    g_tres_world.players_data[8].region_total_bases[2] = 1;
    RCHECK(territory(1, 2, 1, &base_id, &count) == 2);

    // ---- both sides must hold the vendetta --------------------------------
    tres_scene();
    g_tres_world.players_data[1].diplo_treaties[3] = DTREATY_VENDETTA;
    g_tres_world.players_data[3].region_total_bases[2] = 1;
    RCHECK(territory(1, 2, 1, &base_id, &count) == 2);
    tres_scene();
    g_tres_world.players_data[2].diplo_treaties[3] = DTREATY_VENDETTA;
    g_tres_world.players_data[3].region_total_bases[2] = 1;
    RCHECK(territory(1, 2, 1, &base_id, &count) == 2);
    // And it is a vendetta, not any treaty bit.
    tres_scene();
    g_tres_world.players_data[1].diplo_treaties[3] = DTREATY_TRUCE;
    g_tres_world.players_data[2].diplo_treaties[3] = DTREATY_TRUCE;
    g_tres_world.players_data[3].region_total_bases[2] = 1;
    RCHECK(territory(1, 2, 1, &base_id, &count) == 2);

    // ---- neither faction is its own third party ---------------------------
    // The owner at vendetta with itself, with bases in the region, and the
    // trespasser agreeing: still not an exemption.
    tres_scene();
    g_tres_world.players_data[1].diplo_treaties[1] = DTREATY_VENDETTA;
    g_tres_world.players_data[2].diplo_treaties[1] = DTREATY_VENDETTA;
    g_tres_world.players_data[1].region_total_bases[2] = 1;
    RCHECK(territory(1, 2, 1, &base_id, &count) == 2);

    // ---- two ways to switch the whole exemption off -----------------------
    // A trespasser with five blemishes has earned the complaint.
    tres_scene();
    tres_share(3, 2);
    g_tres_world.players_data[2].integrity_blemishes = 4;
    RCHECK(territory(1, 2, 1, &base_id, &count) == 0);
    tres_scene();
    tres_share(3, 2);
    g_tres_world.players_data[2].integrity_blemishes = 5;
    RCHECK(territory(1, 2, 1, &base_id, &count) == 2);
    tres_scene();
    tres_share(3, 2);
    g_tres_world.players_data[2].integrity_blemishes = 6;
    RCHECK(territory(1, 2, 1, &base_id, &count) == 2);
    // The blemishes are the trespasser's, not the owner's.
    tres_scene();
    tres_share(3, 2);
    g_tres_world.players_data[1].integrity_blemishes = 9;
    RCHECK(territory(1, 2, 1, &base_id, &count) == 0);
    // An owner who wants revenge does not grant it either.
    tres_scene();
    tres_share(3, 2);
    g_tres_world.players_data[1].diplo_treaties[2] |= DTREATY_WANT_REVENGE;
    RCHECK(territory(1, 2, 1, &base_id, &count) == 2);

    // ---- the exemption is decided per region, for all 64 land regions -----
    // Two units, one in an exempt region and one not.
    tres_scene();
    tres_share(3, 2);
    tres_at(8, 2).region = 3;
    tres_base(1, 1, 8, 2);  // whose_territory needs a base in region 3 as well
    tres_veh(1, 2, 0, 8, 2);
    RCHECK(territory(1, 2, 1, &base_id, &count) == 2);
    RCHECK((g_tres_world.vehs[0].state & VSTATE_UNK_400) == 0);
    RCHECK((g_tres_world.vehs[1].state & VSTATE_UNK_400) != 0);
    // The last land region is inside the sweep in both directions: exempt
    // when the common enemy is there, and counted when it is not. Whichever
    // way a bound that stopped one region short went - reading a slot the
    // clear never wrote, whatever happened to be in it - one of these two
    // fails.
    tres_scene();
    tres_at(6, 2).region = (uint8_t)(MaxRegionLandNum - 1);
    tres_at(4, 2).region = (uint8_t)(MaxRegionLandNum - 1);
    tres_share(3, MaxRegionLandNum - 1);
    RCHECK(territory(1, 2, 1, &base_id, &count) == 0);
    tres_scene();
    tres_at(6, 2).region = (uint8_t)(MaxRegionLandNum - 1);
    tres_at(4, 2).region = (uint8_t)(MaxRegionLandNum - 1);
    tres_share(3, 2);
    RCHECK(territory(1, 2, 1, &base_id, &count) == 2);
    // Region zero the same way, for a sweep that started one region late.
    tres_scene();
    tres_at(6, 2).region = 0;
    tres_at(4, 2).region = 0;
    tres_share(3, 0);
    RCHECK(territory(1, 2, 1, &base_id, &count) == 0);
    tres_scene();
    tres_at(6, 2).region = 0;
    tres_at(4, 2).region = 0;
    tres_share(3, 2);
    RCHECK(territory(1, 2, 1, &base_id, &count) == 2);
}

void test_territory_unit_filter() {
    TresSeams seams;
    int base_id = 0;
    int count = 0;

    // ---- every unit is unmarked, counted or not ---------------------------
    // Four units: one counted, one refused by its plan, one belonging to a
    // third faction, one counted. All four lose VSTATE_UNK_400; the two that
    // are counted get it straight back.
    tres_scene();
    tres_veh(1, 2, 2, 8, 2);   // plan 9
    tres_veh(2, 3, 0, 10, 2);  // not the trespasser
    tres_veh(3, 2, 0, 12, 2);
    for (int i = 0; i < 4; i++) {
        g_tres_world.vehs[i].state = 0xFFFFFFFFu;
    }
    RCHECK(territory(1, 2, 0, &base_id, &count) == 2);
    RCHECK(g_tres_world.vehs[0].state == 0xFFFFFFFFu);
    RCHECK(g_tres_world.vehs[1].state == 0xFFFFFBFFu);
    RCHECK(g_tres_world.vehs[2].state == 0xFFFFFBFFu);
    RCHECK(g_tres_world.vehs[3].state == 0xFFFFFFFFu);
    // The unit past the end of the roster is never touched.
    RCHECK(g_tres_world.vehs[4].state == 0);

    // ---- the roster ends at VehCurrentCount --------------------------------
    // Unit 1 is wired to qualify in every way, and left out of the count. A
    // walk that ran one unit too far would count it and clear its mark.
    tres_scene();
    g_tres_world.vehs[1].x = 8;
    g_tres_world.vehs[1].y = 2;
    g_tres_world.vehs[1].faction_id = 2;
    g_tres_world.vehs[1].proto_id = 0;
    g_tres_world.vehs[1].state = 0xFFFFFFFFu;
    RCHECK(g_tres_world.veh_count == 1);
    RCHECK(territory(1, 2, 1, &base_id, &count) == 2);
    RCHECK(g_tres_world.vehs[1].state == 0xFFFFFFFFu);

    // ---- the plan filter ---------------------------------------------------
    {
        struct PlanCase {
            int proto_id;
            int order;
            int expected;
        };
        const PlanCase plan_cases[] = {
            {0, ORDER_NONE, 2},    // plan 0
            {1, ORDER_NONE, 2},    // plan 8, the last one allowed
            {2, ORDER_NONE, 0},    // plan 9, the first refused
            {2, ORDER_CONVOY, 0},  // and convoying does not rescue it
            {3, ORDER_CONVOY, 2},  // plan 10 while convoying
            {3, ORDER_HOLD, 0},    // plan 10 otherwise
            {3, ORDER_NONE, 0},
            {4, ORDER_CONVOY, 0},  // plan 11
        };
        for (unsigned i = 0; i < sizeof(plan_cases) / sizeof(plan_cases[0]); i++) {
            tres_scene();
            g_tres_world.vehs[0].proto_id = (int16_t)plan_cases[i].proto_id;
            g_tres_world.vehs[0].order = (int8_t)plan_cases[i].order;
            RCHECK(territory(1, 2, 1, &base_id, &count) == plan_cases[i].expected);
        }
    }

    // ---- only land, and only the altitude bits of climate ------------------
    tres_scene();
    tres_at(6, 2).climate = (ALT_SHORE_LINE << 5);
    RCHECK(territory(1, 2, 1, &base_id, &count) == 2);
    tres_scene();
    tres_at(6, 2).climate = (ALT_SHORE_LINE << 5) | 0x1F;  // rainfall and heat
    RCHECK(territory(1, 2, 1, &base_id, &count) == 2);
    tres_scene();
    tres_at(6, 2).climate = ALT_BIT_OCEAN_SHELF;
    RCHECK(territory(1, 2, 1, &base_id, &count) == 0);
    tres_scene();
    tres_at(6, 2).climate = ALT_BIT_OCEAN_SHELF | 0x1F;
    RCHECK(territory(1, 2, 1, &base_id, &count) == 0);
    tres_scene();
    tres_at(6, 2).climate = ALT_BIT_3_LEVELS_ABOVE_SEA;
    RCHECK(territory(1, 2, 1, &base_id, &count) == 2);

    // ---- the tile has to be the owner's ------------------------------------
    tres_scene();
    tres_at(6, 2).territory = 3;
    RCHECK(territory(1, 2, 1, &base_id, &count) == 0);
    tres_scene();
    tres_at(6, 2).territory = -1;
    RCHECK(territory(1, 2, 1, &base_id, &count) == 0);
    tres_scene();
    tres_at(6, 2).territory = 0;
    RCHECK(territory(1, 2, 1, &base_id, &count) == 0);
    // The owner's own units are not trespassing on it.
    tres_scene();
    g_tres_world.vehs[0].faction_id = 1;
    RCHECK(territory(1, 2, 1, &base_id, &count) == 0);

    // ---- a claimed tile with no base in its region yields no base id -------
    tres_scene();
    tres_at(4, 2).region = 5;  // the base's own tile leaves region 2
    base_id = 0x5A5A;
    RCHECK(territory(1, 2, 1, &base_id, &count) == 0);
    RCHECK(base_id == -1);
    RCHECK((g_tres_world.vehs[0].state & VSTATE_UNK_400) == 0);

    // ---- the last qualifying unit owns the reported base -------------------
    tres_scene();
    tres_base(1, 1, 14, 6);
    tres_veh(0, 2, 0, 2, 2);
    tres_veh(1, 2, 0, 14, 6);
    RCHECK(territory(1, 2, 0, &base_id, &count) == 2);
    RCHECK(base_id == 1);
    // Same two units, met in the other order.
    tres_scene();
    tres_base(1, 1, 14, 6);
    tres_veh(0, 2, 0, 14, 6);
    tres_veh(1, 2, 0, 2, 2);
    RCHECK(territory(1, 2, 0, &base_id, &count) == 2);
    RCHECK(base_id == 0);
}

void test_territory_weight() {
    TresSeams seams;
    int base_id = 0;
    int count = 0;

    // ---- improved terrain is worth a second point --------------------------
    {
        const uint32_t improved[] = {
            BIT_MINE, BIT_SOLAR_TIDAL, BIT_BUNKER, BIT_FARM,
            BIT_FOREST, BIT_CONDENSER, BIT_THERMAL_BORE, BIT_SENSOR_ARRAY,
        };
        for (unsigned i = 0; i < sizeof(improved) / sizeof(improved[0]); i++) {
            tres_scene();
            tres_at(6, 2).bit = improved[i];
            RCHECK(territory(1, 2, 1, &base_id, &count) == 3);
        }
        const uint32_t plain[] = {
            BIT_ROAD, BIT_MAGTUBE, BIT_AIRBASE, BIT_SOIL_ENRICHER, BIT_ECH_MIRROR,
        };
        for (unsigned i = 0; i < sizeof(plain) / sizeof(plain[0]); i++) {
            tres_scene();
            tres_at(6, 2).bit = plain[i];
            RCHECK(territory(1, 2, 1, &base_id, &count) == 2);
        }
    }

    // ---- bit 0 of flags arms everything after the raw count ----------------
    tres_scene();
    g_tres_world.vehs[0].flags = 1;
    count = 0x5A5A;
    RCHECK(territory(1, 2, 0, &base_id, &count) == 1);
    RCHECK(count == 0);
    RCHECK(g_tres_world.vehs[0].flags == 1);
    RCHECK((g_tres_world.vehs[0].state & VSTATE_UNK_400) != 0);
    RCHECK((g_tres_world.vehs[0].state & VSTATE_UNK_800) == 0);
    tres_scene();
    g_tres_world.vehs[0].flags = 1;
    RCHECK(territory(1, 2, 1, &base_id, &count) == 2);
    RCHECK(count == 1);
    RCHECK(g_tres_world.vehs[0].flags == 3);

    // ---- count_out counts only the units already carrying VFLAG bit 0 ------
    tres_scene();
    g_tres_world.vehs[0].flags = 0x8000;
    RCHECK(territory(1, 2, 1, &base_id, &count) == 2);
    RCHECK(count == 0);
    RCHECK(g_tres_world.vehs[0].flags == 0x8003);
    tres_scene();
    g_tres_world.vehs[0].flags = 1;
    tres_veh(1, 2, 0, 2, 2);
    g_tres_world.vehs[1].flags = 1;
    tres_veh(2, 2, 0, 8, 2);
    RCHECK(territory(1, 2, 1, &base_id, &count) == 6);
    RCHECK(count == 2);

    // ---- distance: (dy + dx) & ~1 against 4 --------------------------------
    // Two tiles away, and four, are inside; six is not.
    tres_scene();
    tres_veh(0, 2, 0, 8, 2);  // dx 4
    RCHECK(territory(1, 2, 1, &base_id, &count) == 2);
    tres_scene();
    tres_veh(0, 2, 0, 10, 2);  // dx 6
    RCHECK(territory(1, 2, 1, &base_id, &count) == 1);
    // The y term is an absolute value in both directions.
    tres_scene();
    tres_veh(0, 2, 0, 4, 6);  // dy 4, base below the unit
    RCHECK(territory(1, 2, 1, &base_id, &count) == 2);
    tres_scene();
    tres_base(0, 1, 4, 6);
    tres_veh(0, 2, 0, 4, 2);  // dy 4, base above the unit
    RCHECK(territory(1, 2, 1, &base_id, &count) == 2);
    tres_scene();
    tres_base(0, 1, 4, 0);
    tres_veh(0, 2, 0, 4, 6);  // dy 6
    RCHECK(territory(1, 2, 1, &base_id, &count) == 1);
    // The x term likewise.
    tres_scene();
    tres_base(0, 1, 8, 2);
    tres_veh(0, 2, 0, 4, 2);  // dx -4
    RCHECK(territory(1, 2, 1, &base_id, &count) == 2);
    // An odd sum is rounded down before the comparison. A base is never
    // stored off the two-column grid in a real game, so this is the only way
    // to reach an odd distance at all: (9, 2) and (4, 2) are the same tile
    // apart as (8, 2) and (4, 2), which is what keeps base_find on it.
    tres_scene();
    tres_base(0, 1, 9, 2);
    tres_veh(0, 2, 0, 4, 2);  // dx 5, rounded to 4
    RCHECK(territory(1, 2, 1, &base_id, &count) == 2);
    tres_scene();
    tres_base(0, 1, 11, 2);
    tres_veh(0, 2, 0, 4, 2);  // dx 7, rounded to 6
    RCHECK(territory(1, 2, 1, &base_id, &count) == 1);

    // ---- a round map wraps the x term, on bit 0 of MapIsFlat ---------------
    tres_scene();
    tres_base(0, 1, 2, 2);
    tres_veh(0, 2, 0, 14, 2);  // dx 12, or 4 the short way
    RCHECK(territory(1, 2, 1, &base_id, &count) == 1);
    tres_scene();
    tres_base(0, 1, 2, 2);
    tres_veh(0, 2, 0, 14, 2);
    g_tres_world.is_flat = 0;
    RCHECK(territory(1, 2, 1, &base_id, &count) == 2);
    // Bit 0 is the whole test: an even value is a round map here, exactly as
    // it is in reset_territory's own wrap and unlike x_dist's.
    tres_scene();
    tres_base(0, 1, 2, 2);
    tres_veh(0, 2, 0, 14, 2);
    g_tres_world.is_flat = 2;
    RCHECK(territory(1, 2, 1, &base_id, &count) == 2);
    tres_scene();
    tres_base(0, 1, 2, 2);
    tres_veh(0, 2, 0, 14, 2);
    g_tres_world.is_flat = 3;
    RCHECK(territory(1, 2, 1, &base_id, &count) == 1);
    // The wrap arms strictly above MapLongitude. At exactly MapLongitude the
    // long way stands, and one tile further the short way wins.
    tres_scene();
    tres_base(0, 1, 2, 2);
    tres_veh(0, 2, 0, 10, 2);  // dx 8
    g_tres_world.is_flat = 0;
    g_tres_world.lon_bounds = 12;
    RCHECK(territory(1, 2, 1, &base_id, &count) == 1);
    tres_scene();
    tres_base(0, 1, 2, 2);
    tres_veh(0, 2, 0, 12, 2);  // dx 10, or 2 the short way
    g_tres_world.is_flat = 0;
    g_tres_world.lon_bounds = 12;
    RCHECK(territory(1, 2, 1, &base_id, &count) == 2);

    // ---- three grievances and a friction threshold override the distance ---
    {
        const uint32_t grievance[] = {
            DTREATY_WANT_REVENGE, DTREATY_SHALL_BETRAY, DTREATY_UNK_800,
        };
        for (unsigned i = 0; i < sizeof(grievance) / sizeof(grievance[0]); i++) {
            tres_scene();
            tres_veh(0, 2, 0, 10, 2);  // dx 6: outside
            g_tres_world.players_data[1].diplo_treaties[2] = grievance[i];
            RCHECK(territory(1, 2, 1, &base_id, &count) == 2);
        }
        // A neighbouring bit is not one of the three.
        tres_scene();
        tres_veh(0, 2, 0, 10, 2);
        g_tres_world.players_data[1].diplo_treaties[2] = DTREATY_UNK_100;
        RCHECK(territory(1, 2, 1, &base_id, &count) == 1);
    }
    tres_scene();
    tres_veh(0, 2, 0, 10, 2);
    g_tres_world.friction = 12;
    RCHECK(territory(1, 2, 1, &base_id, &count) == 1);
    tres_scene();
    tres_veh(0, 2, 0, 10, 2);
    g_tres_world.friction = 13;
    RCHECK(territory(1, 2, 1, &base_id, &count) == 2);
    // A unit inside the distance is counted whatever the friction is.
    tres_scene();
    g_tres_world.friction = 12;
    RCHECK(territory(1, 2, 1, &base_id, &count) == 2);

    // ---- sunspots buy each unit one silent pass ----------------------------
    tres_scene();
    g_tres_world.sunspots = 0;
    RCHECK(territory(1, 2, 1, &base_id, &count) == 1);
    RCHECK((g_tres_world.vehs[0].state & VSTATE_UNK_800) != 0);
    // Only once: the grace flag is honoured from then on.
    RCHECK(territory(1, 2, 1, &base_id, &count) == 2);
    tres_scene();
    g_tres_world.sunspots = -1;
    RCHECK(territory(1, 2, 1, &base_id, &count) == 1);
    // Sunspots up, and the flag is never set at all.
    tres_scene();
    g_tres_world.sunspots = 1;
    RCHECK(territory(1, 2, 1, &base_id, &count) == 2);
    RCHECK((g_tres_world.vehs[0].state & VSTATE_UNK_800) == 0);
    // A unit that already carries the flag is weighted even while sunspots
    // are down.
    tres_scene();
    g_tres_world.sunspots = 0;
    g_tres_world.vehs[0].state = VSTATE_UNK_800;
    RCHECK(territory(1, 2, 1, &base_id, &count) == 2);

    // ---- a treaty partner's trespass is worth five, not one ----------------
    tres_scene();
    g_tres_world.players_data[1].diplo_treaties[2] = DTREATY_TREATY;
    RCHECK(territory(1, 2, 1, &base_id, &count) == 6);
    tres_scene();
    g_tres_world.players_data[1].diplo_treaties[2] = DTREATY_TREATY;
    tres_at(6, 2).bit = BIT_MINE;
    RCHECK(territory(1, 2, 1, &base_id, &count) == 7);
    // The five is not paid on the grace pass either.
    tres_scene();
    g_tres_world.players_data[1].diplo_treaties[2] = DTREATY_TREATY;
    g_tres_world.sunspots = 0;
    RCHECK(territory(1, 2, 1, &base_id, &count) == 1);
}

#undef RCHECK

}  // namespace

int main() {
    test_not_my_turn();
    test_stack_veh_boarding();
    test_action_home();
    test_crop_yield();
    test_mine_yield();
    test_energy_yield();
    test_base_support_convoys();
    test_base_support_maintenance();
    test_base_support_pacifism();
    test_world_site_terrain();
    test_world_site_score();
    test_num_objectives_units();
    test_num_objectives_totals();
    test_spot_tile();
    test_spot_base();
    test_spot_stack();
    test_spot_loc();
    test_reset_territory_ownership();
    test_reset_territory_tallies();
    test_reset_territory_sites();
    test_territory_gates();
    test_territory_shared_war();
    test_territory_unit_filter();
    test_territory_weight();
    if (failure_count() != 0) {
        std::fprintf(stderr, "recovery-gameplay-tests: %d failure(s)\n",
                     failure_count());
    }
    return failure_count() == 0 ? 0 : 1;
}
