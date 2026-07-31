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

}  // namespace

int main() {
    test_not_my_turn();
    test_stack_veh_boarding();
    if (failure_count() != 0) {
        std::fprintf(stderr, "recovery-gameplay-tests: %d failure(s)\n",
                     failure_count());
    }
    return failure_count() == 0 ? 0 : 1;
}
