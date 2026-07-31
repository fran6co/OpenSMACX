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

}  // namespace

int main() {
    test_not_my_turn();
    test_stack_veh_boarding();
    test_action_home();
    if (failure_count() != 0) {
        std::fprintf(stderr, "recovery-gameplay-tests: %d failure(s)\n",
                     failure_count());
    }
    return failure_count() == 0 ? 0 : 1;
}
