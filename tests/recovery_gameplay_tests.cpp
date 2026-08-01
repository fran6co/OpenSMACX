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
#include "../src/terraforming.h"
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

/*
 * Self-registration, so adding a test touches ONE place instead of two.
 *
 * main() used to call all 56 cases by name. That list was the only part of
 * this file every recovery had to edit, and it is the one merge conflict where
 * a careless resolution is SILENT: drop a line and the test simply stops
 * running, while everything that remains still passes. Parallel recoveries in
 * separate worktrees all collide here.
 *
 * Order is preserved exactly. Registration happens during static
 * initialisation, which within a single translation unit runs in declaration
 * order, and each GAMEPLAY_CASE sits immediately after its function - so the
 * registry order is the declaration order, which was already identical to the
 * old call order (verified: 56 declared, 56 called, same sequence).
 */
namespace {

using GameplayCase = void (*)();

std::vector<std::pair<const char *, GameplayCase>> &gameplay_cases() {
    static std::vector<std::pair<const char *, GameplayCase>> cases;
    return cases;
}

struct GameplayRegistrar {
    GameplayRegistrar(const char *name, GameplayCase run) {
        gameplay_cases().emplace_back(name, run);
    }
};

}  // namespace

#define GAMEPLAY_CASE(fn) \
    namespace { const GameplayRegistrar registrar_##fn(#fn, fn); }

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
GAMEPLAY_CASE(test_not_my_turn);


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
GAMEPLAY_CASE(test_stack_veh_boarding);


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
GAMEPLAY_CASE(test_action_home);

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
GAMEPLAY_CASE(test_crop_yield);

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
GAMEPLAY_CASE(test_mine_yield);

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
GAMEPLAY_CASE(test_energy_yield);

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
GAMEPLAY_CASE(test_base_support_convoys);

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
GAMEPLAY_CASE(test_base_support_maintenance);

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
GAMEPLAY_CASE(test_base_support_pacifism);

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
GAMEPLAY_CASE(test_world_site_terrain);

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
GAMEPLAY_CASE(test_world_site_score);

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
GAMEPLAY_CASE(test_num_objectives_units);

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
GAMEPLAY_CASE(test_num_objectives_totals);

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
GAMEPLAY_CASE(test_spot_tile);

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
GAMEPLAY_CASE(test_spot_base);

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
GAMEPLAY_CASE(test_spot_stack);

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
GAMEPLAY_CASE(test_spot_loc);

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
GAMEPLAY_CASE(test_reset_territory_ownership);

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
GAMEPLAY_CASE(test_reset_territory_tallies);

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
GAMEPLAY_CASE(test_reset_territory_sites);

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
GAMEPLAY_CASE(test_territory_gates);

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
GAMEPLAY_CASE(test_territory_shared_war);

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
GAMEPLAY_CASE(test_territory_unit_filter);

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
GAMEPLAY_CASE(test_territory_weight);

#undef RCHECK

/*
 * A self-contained world for rankings().
 *
 * rankings() reaches five other recovered functions - num_objectives,
 * has_tech, great_satan, climactic_battle, reputation and set_treaty - so the
 * fixture has to make all of them decidable, not just the subject. Three
 * settings do that and are what every case starts from:
 *
 *  - The secret projects are all SP_Unbuilt (-1). That is both what the score
 *    loop wants and what keeps ascending() false, which is one of the three
 *    arms of climactic_battle().
 *  - No faction is human and VehCurrentCount is zero, so climactic_battle()
 *    and num_objectives() both answer without touching anything else. Cases
 *    that want a human or an objective say so.
 *  - RULES_VICTORY_DIPLOMATIC is clear, so aah_ooga() returns zero
 *    immediately, and current_num_bases is zero, so great_beelzebub() - and
 *    therefore great_satan() - is false. That leaves target selection
 *    entirely to the climactic-battle override, which a case can drive.
 *
 * Objective points are 1 and the two objective thresholds are 9000, which is
 * the "no scenario objective victory" configuration; the scenario branch is
 * reached by lowering one of them.
 */
struct RankWorld {
    // Nine faction rows, 65 secret-project slots and one spare history entry:
    // each last element exists only to be poisoned, so a loop that runs one
    // too far leaves a mark instead of writing over the next field.
    PlayerData players_data[9];
    Base bases[8];
    VehPrototype protos[MaxVehProtoNum + 1];
    RulesWeapon weapons[32];
    RulesTechnology technology[MaxTechnologyNum + 1];
    RulesFacility facility[128];
    int projects[MaxSecretProjectNum + 1];
    uint8_t tech_achieved[MaxTechnologyNum + 1];
    uint32_t rankings_unk[MaxPlayerNum + 1];
    uint32_t faction_rankings[MaxPlayerNum + 1];
    uint16_t history[MaxRankingHistoryTurns * MaxPlayerNum + MaxPlayerNum];
    Veh vehs[4];
    uint8_t faction_status[2];
    uint32_t rank_unk1;
    uint32_t rank_unk2;
    uint32_t game_state;
    uint32_t game_rules;
    int turn;
    int veh_count;
    int base_count;
    int base_find_dist;
    int diff_level_current;
    uint32_t obj_req_victory;
    uint32_t obj_sudden_death;
    uint32_t obj_achieve_pts;
};

RankWorld g_rank_world;

const uint16_t RANK_POISON16 = 0x7777;

void rank_reset() {
    std::memset(&g_rank_world, 0, sizeof(g_rank_world));
    for (int i = 0; i <= MaxSecretProjectNum; i++) {
        g_rank_world.projects[i] = SP_Unbuilt;
    }
    g_rank_world.obj_req_victory = 9000;
    g_rank_world.obj_sudden_death = 9000;
    g_rank_world.obj_achieve_pts = 1;
    g_rank_world.turn = 3;
}

// Every history slot the archive can reach, plus the one past it.
void rank_poison_history() {
    for (int i = 0; i < MaxRankingHistoryTurns * MaxPlayerNum + MaxPlayerNum; i++) {
        g_rank_world.history[i] = RANK_POISON16;
    }
}

void rank_alive(int faction_id) {
    g_rank_world.faction_status[1] |= (uint8_t)(1 << faction_id);
}

void rank_human(int faction_id) {
    g_rank_world.faction_status[0] |= (uint8_t)(1 << faction_id);
}

// A prototype the offense scan will consider, with the named weapon.
void rank_proto(int proto_id, int weapon_id, int offense) {
    g_rank_world.protos[proto_id].flags = PROTO_ACTIVE | PROTO_TYPED_COMPLETE;
    g_rank_world.protos[proto_id].weapon_id = (uint8_t)weapon_id;
    g_rank_world.weapons[weapon_id].offense_rating = (int8_t)offense;
}

#define KCHECK(cond)                                                          \
    do {                                                                      \
        const bool rank_ok = (cond);                                          \
        if (!rank_ok) {                                                       \
            std::fprintf(stderr, "rankings: line %d: %s\n", __LINE__,         \
                         #cond);                                              \
        }                                                                     \
        expect(rank_ok);                                                      \
    } while (0)

class RankSeams {
 public:
    RankSeams()
        : players_(&PlayersData, g_rank_world.players_data),
          bases_(&Bases, g_rank_world.bases),
          protos_(&VehPrototypes, g_rank_world.protos),
          weapons_(&Weapon, g_rank_world.weapons),
          technology_(&Technology, g_rank_world.technology),
          facility_(&Facility, g_rank_world.facility),
          projects_(&SecretProject,
                    reinterpret_cast<BaseSecretProject *>(g_rank_world.projects)),
          achieved_(&GameTechAchieved, g_rank_world.tech_achieved),
          unk_(&FactionRankingsUnk, g_rank_world.rankings_unk),
          ranks_(&FactionRankings, g_rank_world.faction_rankings),
          history_(&FactionRankingHistory, g_rank_world.history),
          vehs_(&Vehs, g_rank_world.vehs),
          veh_count_(&VehCurrentCount, &g_rank_world.veh_count),
          base_count_(&BaseCurrentCount, &g_rank_world.base_count),
          base_dist_(&BaseFindDist, &g_rank_world.base_find_dist),
          status_(&FactionsStatus, g_rank_world.faction_status),
          unk1_(&RankingFactionIDUnk1, &g_rank_world.rank_unk1),
          unk2_(&RankingFactionIDUnk2, &g_rank_world.rank_unk2),
          state_(&GameState, &g_rank_world.game_state),
          rules_(&GameRules, &g_rank_world.game_rules),
          turn_(&TurnCurrentNum, &g_rank_world.turn),
          diff_(&DiffLevelCurrent, &g_rank_world.diff_level_current),
          req_(&ObjectiveReqVictory, &g_rank_world.obj_req_victory),
          sudden_(&ObjectivesSuddenDeathVictory, &g_rank_world.obj_sudden_death),
          pts_(&ObjectiveAchievePts, &g_rank_world.obj_achieve_pts) { }

 private:
    ScopedSeam<PlayerData> players_;
    ScopedSeam<Base> bases_;
    ScopedSeam<VehPrototype> protos_;
    ScopedSeam<RulesWeapon> weapons_;
    ScopedSeam<RulesTechnology> technology_;
    ScopedSeam<RulesFacility> facility_;
    ScopedSeam<BaseSecretProject> projects_;
    ScopedSeam<uint8_t> achieved_;
    ScopedSeam<uint32_t> unk_;
    ScopedSeam<uint32_t> ranks_;
    ScopedSeam<uint16_t> history_;
    ScopedSeam<Veh> vehs_;
    ScopedSeam<int> veh_count_;
    ScopedSeam<int> base_count_;
    ScopedSeam<int> base_dist_;
    ScopedSeam<uint8_t> status_;
    ScopedSeam<uint32_t> unk1_;
    ScopedSeam<uint32_t> unk2_;
    ScopedSeam<uint32_t> state_;
    ScopedSeam<uint32_t> rules_;
    ScopedSeam<int> turn_;
    ScopedSeam<int> diff_;
    ScopedSeam<uint32_t> req_;
    ScopedSeam<uint32_t> sudden_;
    ScopedSeam<uint32_t> pts_;
};

void test_rankings_score() {
    RankSeams seams;

    // ---- faction zero is scored by nobody but still has its rank cleared ---
    rank_reset();
    for (int f = 0; f <= MaxPlayerNum; f++) {
        g_rank_world.players_data[f].ranking = 0x5A;
    }
    rankings(1);
    for (int f = 0; f < MaxPlayerNum; f++) {
        KCHECK(g_rank_world.players_data[f].ranking != 0x5A);
    }
    KCHECK(g_rank_world.players_data[0].ranking == 0);
    KCHECK(g_rank_world.players_data[MaxPlayerNum].ranking == 0x5A);
    // Without the flag not one of them is touched.
    rank_reset();
    for (int f = 0; f <= MaxPlayerNum; f++) {
        g_rank_world.players_data[f].ranking = 0x5A;
    }
    rankings(0);
    for (int f = 0; f <= MaxPlayerNum; f++) {
        KCHECK(g_rank_world.players_data[f].ranking == 0x5A);
    }

    // ---- an empty world scores nothing, and faction zero is never scored ---
    rank_reset();
    g_rank_world.rankings_unk[0] = 0x5A5A;
    g_rank_world.players_data[0].pop_total = 1000;
    rankings(0);
    KCHECK(g_rank_world.rankings_unk[0] == 0x5A5A);  // untouched
    for (int f = 1; f < MaxPlayerNum; f++) {
        KCHECK(g_rank_world.rankings_unk[f] == 0);   // not alive
    }
    // The row past the last faction is not scored either.
    KCHECK(g_rank_world.rankings_unk[MaxPlayerNum] == 0);

    // ---- population and the theory of everything, times four ---------------
    rank_reset();
    rank_alive(1);
    g_rank_world.players_data[1].pop_total = 7;
    g_rank_world.players_data[1].theory_of_everything = 3;
    rankings(0);
    KCHECK(g_rank_world.rankings_unk[1] == 40);
    // A faction that is not alive scores zero however big it is.
    rank_reset();
    g_rank_world.players_data[1].pop_total = 7;
    rankings(0);
    KCHECK(g_rank_world.rankings_unk[1] == 0);

    // ---- the technology term ------------------------------------------------
    // has_tech refuses tech 88, so 0 through 87 are the reachable ones.
    rank_reset();
    rank_alive(1);
    g_rank_world.technology[5].growth_value = 1;
    g_rank_world.technology[5].tech_value = 2;
    g_rank_world.technology[5].wealth_value = 4;
    g_rank_world.technology[5].power_value = 8;
    g_rank_world.tech_achieved[5] = (uint8_t)(1 << 1);
    rankings(0);
    KCHECK(g_rank_world.rankings_unk[1] == 15);
    // Technology zero is inside the sweep.
    rank_reset();
    rank_alive(1);
    g_rank_world.technology[0].tech_value = 6;
    g_rank_world.tech_achieved[0] = (uint8_t)(1 << 1);
    rankings(0);
    KCHECK(g_rank_world.rankings_unk[1] == 6);
    // Not held: not counted.
    rank_reset();
    rank_alive(1);
    g_rank_world.technology[5].growth_value = 1;
    rankings(0);
    KCHECK(g_rank_world.rankings_unk[1] == 0);
    // Held by another faction: not counted for this one.
    rank_reset();
    rank_alive(1);
    g_rank_world.technology[5].growth_value = 16;
    g_rank_world.tech_achieved[5] = (uint8_t)(1 << 2);
    rankings(0);
    KCHECK(g_rank_world.rankings_unk[1] == 0);
    // The last technology has_tech will answer for is 87, and the loop reaches
    // it; the row past the table is never read.
    rank_reset();
    rank_alive(1);
    g_rank_world.technology[MaxTechnologyNum - 2].power_value = 32;
    g_rank_world.tech_achieved[MaxTechnologyNum - 2] = (uint8_t)(1 << 1);
    g_rank_world.technology[MaxTechnologyNum].power_value = 64;
    g_rank_world.tech_achieved[MaxTechnologyNum] = 0xFF;
    rankings(0);
    KCHECK(g_rank_world.rankings_unk[1] == 32);

    // ---- ten per secret project standing in one of the faction's bases -----
    rank_reset();
    rank_alive(1);
    g_rank_world.bases[2].faction_id_current = 1;
    g_rank_world.bases[3].faction_id_current = 4;
    g_rank_world.projects[0] = 2;
    g_rank_world.projects[1] = 3;
    g_rank_world.projects[MaxSecretProjectNum - 1] = 2;
    rankings(0);
    KCHECK(g_rank_world.rankings_unk[1] == 20);
    // Base zero is a base like any other: the guard is on the project slot
    // being filled at all, and zero is a filled slot.
    rank_reset();
    rank_alive(1);
    g_rank_world.bases[0].faction_id_current = 1;
    g_rank_world.projects[4] = 0;
    rankings(0);
    KCHECK(g_rank_world.rankings_unk[1] == 10);
    // One project slot past the end, wired to score: it must not be read.
    rank_reset();
    rank_alive(1);
    g_rank_world.bases[2].faction_id_current = 1;
    g_rank_world.projects[MaxSecretProjectNum] = 2;
    rankings(0);
    KCHECK(g_rank_world.rankings_unk[1] == 0);
    // An unbuilt project is not a base id.
    rank_reset();
    rank_alive(1);
    g_rank_world.bases[0].faction_id_current = 1;
    rankings(0);
    KCHECK(g_rank_world.rankings_unk[1] == 0);

    // ---- objectives times their point value --------------------------------
    rank_reset();
    rank_alive(1);
    g_rank_world.players_data[1].unk_101 = 3;
    g_rank_world.obj_achieve_pts = 7;
    rankings(0);
    KCHECK(g_rank_world.rankings_unk[1] == 21);

    // ---- the scenario branch: ten per objective and nothing else -----------
    rank_reset();
    rank_alive(1);
    g_rank_world.players_data[1].unk_101 = 3;
    g_rank_world.players_data[1].pop_total = 100;
    g_rank_world.obj_req_victory = 8999;
    g_rank_world.obj_achieve_pts = 0;
    rankings(0);
    KCHECK(g_rank_world.rankings_unk[1] == 30);
    // The sudden-death threshold is the other half of the same test.
    rank_reset();
    rank_alive(1);
    g_rank_world.players_data[1].unk_101 = 3;
    g_rank_world.players_data[1].pop_total = 100;
    g_rank_world.obj_sudden_death = 8999;
    g_rank_world.obj_achieve_pts = 0;
    rankings(0);
    KCHECK(g_rank_world.rankings_unk[1] == 30);
    // Exactly 9000 is not below it.
    rank_reset();
    rank_alive(1);
    g_rank_world.players_data[1].unk_101 = 3;
    g_rank_world.players_data[1].pop_total = 100;
    g_rank_world.obj_achieve_pts = 0;
    rankings(0);
    KCHECK(g_rank_world.rankings_unk[1] == 400);
    // A point value in play takes the ordinary branch even below threshold.
    rank_reset();
    rank_alive(1);
    g_rank_world.players_data[1].unk_101 = 3;
    g_rank_world.players_data[1].pop_total = 100;
    g_rank_world.obj_req_victory = 8999;
    g_rank_world.obj_achieve_pts = 2;
    rankings(0);
    KCHECK(g_rank_world.rankings_unk[1] == 406);
}
GAMEPLAY_CASE(test_rankings_score);

void test_rankings_prototypes() {
    RankSeams seams;

    // ---- cost times active count, scaled by the weapon -----------------------
    // A faction slot (>= 64) needs no technology at all.
    rank_reset();
    rank_alive(1);
    g_rank_world.protos[70].cost = 5;
    g_rank_world.protos[70].weapon_id = 3;
    g_rank_world.weapons[3].offense_rating = 0;
    g_rank_world.players_data[1].proto_id_active[70] = 8;
    rankings(0);
    KCHECK(g_rank_world.rankings_unk[1] == 10);            // 40 / 4
    // A negative rating halves it instead.
    rank_reset();
    rank_alive(1);
    g_rank_world.protos[70].cost = 5;
    g_rank_world.protos[70].weapon_id = 3;
    g_rank_world.weapons[3].offense_rating = -1;
    g_rank_world.players_data[1].proto_id_active[70] = 8;
    rankings(0);
    KCHECK(g_rank_world.rankings_unk[1] == 20);            // 40 / 2
    // 99 or better takes the whole of it.
    rank_reset();
    rank_alive(1);
    g_rank_world.protos[70].cost = 5;
    g_rank_world.protos[70].weapon_id = 3;
    g_rank_world.weapons[3].offense_rating = 99;
    g_rank_world.players_data[1].proto_id_active[70] = 8;
    rankings(0);
    KCHECK(g_rank_world.rankings_unk[1] == 40);
    // Below 99 it is scaled by the best weapon in play, which is this one, so
    // the ratio is exactly 1.
    rank_reset();
    rank_alive(1);
    g_rank_world.protos[70].cost = 5;
    g_rank_world.protos[70].weapon_id = 3;
    g_rank_world.weapons[3].offense_rating = 40;
    g_rank_world.players_data[1].proto_id_active[70] = 8;
    rank_proto(80, 4, 40);   // the offense scan needs an ACTIVE, TYPED prototype
    rankings(0);
    KCHECK(g_rank_world.rankings_unk[1] == 40);
    // Half the best weapon rounds the ratio to one as well; a third rounds to
    // zero, which is what makes the division observable.
    rank_reset();
    rank_alive(1);
    g_rank_world.protos[70].cost = 5;
    g_rank_world.protos[70].weapon_id = 3;
    g_rank_world.weapons[3].offense_rating = 30;
    g_rank_world.players_data[1].proto_id_active[70] = 8;
    rank_proto(80, 4, 90);
    rankings(0);
    KCHECK(g_rank_world.rankings_unk[1] == 0);             // 30 / 90 == 0
    rank_reset();
    rank_alive(1);
    g_rank_world.protos[70].cost = 5;
    g_rank_world.protos[70].weapon_id = 3;
    g_rank_world.weapons[3].offense_rating = 90;
    g_rank_world.players_data[1].proto_id_active[70] = 8;
    rank_proto(80, 4, 30);
    rankings(0);
    KCHECK(g_rank_world.rankings_unk[1] == 120);           // (90 / 30) * 40

    // ---- the count is skipped above 250 -------------------------------------
    rank_reset();
    rank_alive(1);
    g_rank_world.protos[70].cost = 1;
    g_rank_world.protos[70].weapon_id = 3;
    g_rank_world.players_data[1].proto_id_active[70] = 250;
    rankings(0);
    KCHECK(g_rank_world.rankings_unk[1] == 62);            // 250 / 4
    rank_reset();
    rank_alive(1);
    g_rank_world.protos[70].cost = 1;
    g_rank_world.protos[70].weapon_id = 3;
    g_rank_world.players_data[1].proto_id_active[70] = 251;
    rankings(0);
    KCHECK(g_rank_world.rankings_unk[1] == 0);

    // ---- a predefined slot needs its prerequisite technology ---------------
    rank_reset();
    rank_alive(1);
    g_rank_world.protos[63].cost = 4;
    g_rank_world.protos[63].preq_tech = 5;
    g_rank_world.players_data[1].proto_id_active[63] = 4;
    rankings(0);
    KCHECK(g_rank_world.rankings_unk[1] == 0);
    rank_reset();
    rank_alive(1);
    g_rank_world.protos[63].cost = 4;
    g_rank_world.protos[63].preq_tech = 5;
    g_rank_world.tech_achieved[5] = (uint8_t)(1 << 1);
    g_rank_world.players_data[1].proto_id_active[63] = 4;
    rankings(0);
    KCHECK(g_rank_world.rankings_unk[1] == 4);             // 16 / 4
    // Slot 64 is the first that skips the check entirely.
    rank_reset();
    rank_alive(1);
    g_rank_world.protos[64].cost = 4;
    g_rank_world.protos[64].preq_tech = 5;
    g_rank_world.players_data[1].proto_id_active[64] = 4;
    rankings(0);
    KCHECK(g_rank_world.rankings_unk[1] == 4);
    // Slot zero is scored like the rest.
    rank_reset();
    rank_alive(1);
    g_rank_world.protos[0].cost = 4;
    g_rank_world.protos[0].preq_tech = TechNone;   // has_tech answers yes
    g_rank_world.players_data[1].proto_id_active[0] = 4;
    rankings(0);
    KCHECK(g_rank_world.rankings_unk[1] == 4);
    // The slot past the last prototype is never read, and neither is the
    // proto_id_active entry past the last - which is proto_id_queue[0], wired
    // here so a walk one slot too long would score it.
    rank_reset();
    rank_alive(1);
    g_rank_world.protos[MaxVehProtoNum].cost = 100;
    g_rank_world.protos[MaxVehProtoNum].preq_tech = TechNone;
    g_rank_world.players_data[1].proto_id_queue[0] = 4;
    rankings(0);
    KCHECK(g_rank_world.rankings_unk[1] == 0);
    // The last real slot is reached.
    rank_reset();
    rank_alive(1);
    g_rank_world.protos[MaxVehProtoNum - 1].cost = 4;
    g_rank_world.protos[MaxVehProtoNum - 1].weapon_id = 3;
    g_rank_world.players_data[1].proto_id_active[MaxVehProtoNum - 1] = 4;
    rankings(0);
    KCHECK(g_rank_world.rankings_unk[1] == 4);

    // ---- the best-weapon scan --------------------------------------------
    // It only looks at faction slots 64 through 511, and only at prototypes
    // that are both active and typed.
    rank_reset();
    rank_alive(1);
    g_rank_world.protos[70].cost = 5;
    g_rank_world.protos[70].weapon_id = 3;
    g_rank_world.weapons[3].offense_rating = 30;
    g_rank_world.players_data[1].proto_id_active[70] = 8;
    rank_proto(63, 4, 90);          // below the scan's first slot
    rankings(0);
    KCHECK(g_rank_world.rankings_unk[1] == 1200);          // best stayed 1
    rank_reset();
    rank_alive(1);
    g_rank_world.protos[70].cost = 5;
    g_rank_world.protos[70].weapon_id = 3;
    g_rank_world.weapons[3].offense_rating = 30;
    g_rank_world.players_data[1].proto_id_active[70] = 8;
    rank_proto(64, 4, 90);          // the scan's first slot
    rankings(0);
    KCHECK(g_rank_world.rankings_unk[1] == 0);
    rank_reset();
    rank_alive(1);
    g_rank_world.protos[70].cost = 5;
    g_rank_world.protos[70].weapon_id = 3;
    g_rank_world.weapons[3].offense_rating = 30;
    g_rank_world.players_data[1].proto_id_active[70] = 8;
    rank_proto(MaxVehProtoNum - 1, 4, 90);   // the scan's last slot
    rankings(0);
    KCHECK(g_rank_world.rankings_unk[1] == 0);
    // And the slot past it, wired the same way, is out of the scan's reach.
    rank_reset();
    rank_alive(1);
    g_rank_world.protos[70].cost = 5;
    g_rank_world.protos[70].weapon_id = 3;
    g_rank_world.weapons[3].offense_rating = 30;
    g_rank_world.players_data[1].proto_id_active[70] = 8;
    rank_proto(MaxVehProtoNum, 4, 90);
    rankings(0);
    KCHECK(g_rank_world.rankings_unk[1] == 1200);          // best stayed 1
    // Retired, or never typed: not scanned.
    rank_reset();
    rank_alive(1);
    g_rank_world.protos[70].cost = 5;
    g_rank_world.protos[70].weapon_id = 3;
    g_rank_world.weapons[3].offense_rating = 30;
    g_rank_world.players_data[1].proto_id_active[70] = 8;
    rank_proto(80, 4, 90);
    g_rank_world.protos[80].flags = PROTO_TYPED_COMPLETE;
    rankings(0);
    KCHECK(g_rank_world.rankings_unk[1] == 1200);
    rank_reset();
    rank_alive(1);
    g_rank_world.protos[70].cost = 5;
    g_rank_world.protos[70].weapon_id = 3;
    g_rank_world.weapons[3].offense_rating = 30;
    g_rank_world.players_data[1].proto_id_active[70] = 8;
    rank_proto(80, 4, 90);
    g_rank_world.protos[80].flags = PROTO_ACTIVE;
    rankings(0);
    KCHECK(g_rank_world.rankings_unk[1] == 1200);
    // A rating of 99 or more is not a candidate for "best" either.
    rank_reset();
    rank_alive(1);
    g_rank_world.protos[70].cost = 5;
    g_rank_world.protos[70].weapon_id = 3;
    g_rank_world.weapons[3].offense_rating = 30;
    g_rank_world.players_data[1].proto_id_active[70] = 8;
    rank_proto(80, 4, 99);
    rankings(0);
    KCHECK(g_rank_world.rankings_unk[1] == 1200);
    rank_reset();
    rank_alive(1);
    g_rank_world.protos[70].cost = 5;
    g_rank_world.protos[70].weapon_id = 3;
    g_rank_world.weapons[3].offense_rating = 30;
    g_rank_world.players_data[1].proto_id_active[70] = 8;
    rank_proto(80, 4, 98);
    rankings(0);
    KCHECK(g_rank_world.rankings_unk[1] == 0);             // 30 / 98
}
GAMEPLAY_CASE(test_rankings_prototypes);

void test_rankings_history() {
    RankSeams seams;

    // ---- the archive is indexed faction + turn * 8 --------------------------
    rank_reset();
    rank_poison_history();
    rank_alive(3);
    g_rank_world.players_data[3].pop_total = 6;
    g_rank_world.turn = 2;
    rankings(0);
    KCHECK(g_rank_world.history[3 + 2 * MaxPlayerNum] == 24);
    // Every other faction of that turn is written too - with zero, which the
    // poison can tell from "not written".
    KCHECK(g_rank_world.history[1 + 2 * MaxPlayerNum] == 0);
    KCHECK(g_rank_world.history[7 + 2 * MaxPlayerNum] == 0);
    // Faction zero's slot is not.
    KCHECK(g_rank_world.history[0 + 2 * MaxPlayerNum] == RANK_POISON16);
    // Neither is the previous turn's row.
    KCHECK(g_rank_world.history[3 + 1 * MaxPlayerNum] == RANK_POISON16);

    // ---- sixteen bits, and it wraps ----------------------------------------
    rank_reset();
    rank_poison_history();
    rank_alive(1);
    g_rank_world.players_data[1].pop_total = 16400;   // * 4 == 65600
    g_rank_world.turn = 0;
    rankings(0);
    KCHECK(g_rank_world.rankings_unk[1] == 65600);
    KCHECK(g_rank_world.history[1] == 64);

    // ---- the last archived turn is 999 -------------------------------------
    rank_reset();
    rank_poison_history();
    rank_alive(1);
    g_rank_world.players_data[1].pop_total = 1;
    g_rank_world.turn = MaxRankingHistoryTurns - 1;
    rankings(0);
    KCHECK(g_rank_world.history[1 + (MaxRankingHistoryTurns - 1) * MaxPlayerNum] == 4);
    rank_reset();
    rank_poison_history();
    rank_alive(1);
    g_rank_world.players_data[1].pop_total = 1;
    g_rank_world.turn = MaxRankingHistoryTurns;
    rankings(0);
    KCHECK(g_rank_world.rankings_unk[1] == 4);
    for (int i = 0; i < MaxRankingHistoryTurns * MaxPlayerNum + MaxPlayerNum; i++) {
        if (g_rank_world.history[i] != RANK_POISON16) {
            KCHECK(false);
            break;
        }
    }
}
GAMEPLAY_CASE(test_rankings_history);

void test_rankings_publication() {
    RankSeams seams;

    // ---- without the flag nothing is published -----------------------------
    rank_reset();
    for (int f = 1; f < MaxPlayerNum; f++) {
        rank_alive(f);
        g_rank_world.players_data[f].ranking = 0x5A;
        g_rank_world.players_data[f].pop_total = f;
    }
    g_rank_world.faction_rankings[3] = 0x5A;
    rankings(0);
    KCHECK(g_rank_world.players_data[1].ranking == 0x5A);
    KCHECK(g_rank_world.faction_rankings[3] == 0x5A);
    KCHECK(g_rank_world.rankings_unk[7] == 28);   // scores survive, unranked

    // ---- with it, ranks run 7 down to 1 in score order ---------------------
    rank_reset();
    for (int f = 1; f < MaxPlayerNum; f++) {
        rank_alive(f);
        g_rank_world.players_data[f].pop_total = f;   // 1 is worst, 7 is best
    }
    rankings(1);
    for (int f = 1; f < MaxPlayerNum; f++) {
        KCHECK(g_rank_world.players_data[f].ranking == (uint32_t)f);
        KCHECK(g_rank_world.faction_rankings[f] == (uint32_t)f);
    }
    KCHECK(g_rank_world.players_data[0].ranking == 0);
    // Every ranked score is struck out.
    for (int f = 1; f < MaxPlayerNum; f++) {
        KCHECK(g_rank_world.rankings_unk[f] == 0xFFFFFFFFu);
    }
    // Reversed, the order reverses with it.
    rank_reset();
    for (int f = 1; f < MaxPlayerNum; f++) {
        rank_alive(f);
        g_rank_world.players_data[f].pop_total = MaxPlayerNum - f;
    }
    rankings(1);
    for (int f = 1; f < MaxPlayerNum; f++) {
        KCHECK(g_rank_world.players_data[f].ranking == (uint32_t)(MaxPlayerNum - f));
    }

    // ---- ties go to the FIRST faction holding the score --------------------
    // The search keeps a candidate only on a strictly greater score, so an
    // all-equal field is ranked in faction order, best first.
    rank_reset();
    for (int f = 1; f < MaxPlayerNum; f++) {
        rank_alive(f);
        g_rank_world.players_data[f].pop_total = 5;
    }
    rankings(1);
    KCHECK(g_rank_world.players_data[1].ranking == 7);
    KCHECK(g_rank_world.players_data[7].ranking == 1);

    // ---- a field where nothing beats -1 awards the rank to faction zero ----
    // No faction is alive, so every score is zero... which still beats -1.
    // Strike them all out by hand instead: after seven passes the eighth
    // candidate is faction zero.
    rank_reset();
    rank_alive(1);
    g_rank_world.players_data[1].pop_total = 1;
    rankings(1);
    KCHECK(g_rank_world.players_data[1].ranking == 7);
    // Faction 2 scored 0, which still beats -1, so it takes rank 6, and so on
    // down; faction 7 takes rank 1 and nothing is left for faction 0.
    KCHECK(g_rank_world.players_data[2].ranking == 6);
    KCHECK(g_rank_world.players_data[7].ranking == 1);
    KCHECK(g_rank_world.players_data[0].ranking == 0);

    // ---- FactionRankings is filled only for the living ---------------------
    rank_reset();
    rank_alive(2);
    rank_alive(5);
    g_rank_world.players_data[2].pop_total = 9;
    g_rank_world.players_data[5].pop_total = 4;
    g_rank_world.faction_rankings[0] = 0x5A;
    rankings(1);
    KCHECK(g_rank_world.players_data[2].ranking == 7);
    KCHECK(g_rank_world.players_data[5].ranking == 6);
    KCHECK(g_rank_world.faction_rankings[7] == 2);
    KCHECK(g_rank_world.faction_rankings[6] == 5);
    KCHECK(g_rank_world.faction_rankings[0] == 0);   // cleared, then not refilled
    KCHECK(g_rank_world.faction_rankings[MaxPlayerNum] == 0);

    // ---- the highest and lowest ranked humans ------------------------------
    rank_reset();
    for (int f = 1; f < MaxPlayerNum; f++) {
        rank_alive(f);
        g_rank_world.players_data[f].pop_total = f;
    }
    rank_human(2);
    rank_human(6);
    g_rank_world.rank_unk1 = 0x5A;
    g_rank_world.rank_unk2 = 0x5A;
    rankings(1);
    KCHECK(g_rank_world.rank_unk1 == 6);   // rank 6, the highest-ranked human
    KCHECK(g_rank_world.rank_unk2 == 2);   // rank 2, the lowest
    // No human at all leaves both at zero rather than at their old values.
    rank_reset();
    for (int f = 1; f < MaxPlayerNum; f++) {
        rank_alive(f);
        g_rank_world.players_data[f].pop_total = f;
    }
    g_rank_world.rank_unk1 = 0x5A;
    g_rank_world.rank_unk2 = 0x5A;
    rankings(1);
    KCHECK(g_rank_world.rank_unk1 == 0);
    KCHECK(g_rank_world.rank_unk2 == 0);
    // A human at rank 7 is found by the descending search, and one at rank 0
    // - faction zero's slot, which no living faction claims - by the
    // ascending one.
    rank_reset();
    for (int f = 1; f < MaxPlayerNum; f++) {
        rank_alive(f);
        g_rank_world.players_data[f].pop_total = f;
    }
    rank_human(7);
    rank_human(0);
    rankings(1);
    KCHECK(g_rank_world.rank_unk1 == 7);
    KCHECK(g_rank_world.rank_unk2 == 0);

    // ---- the ranking pass runs exactly seven times -------------------------
    // An eighth pass would find nothing left to beat -1, award rank 0 to
    // faction zero and strike its score out, so faction zero's slot answers
    // for the loop count.
    rank_reset();
    for (int f = 1; f < MaxPlayerNum; f++) {
        rank_alive(f);
        g_rank_world.players_data[f].pop_total = f;
    }
    g_rank_world.rankings_unk[0] = 0x5A5A;
    rankings(1);
    KCHECK(g_rank_world.rankings_unk[0] == 0x5A5A);
    KCHECK(g_rank_world.players_data[0].ranking == 0);

    // ---- a field where every score is negative -----------------------------
    // One technology worth -1 puts all seven factions below the -1 the search
    // starts from, so no pass finds a candidate and every rank goes to the
    // faction the search starts on.
    rank_reset();
    g_rank_world.technology[5].growth_value = -1;
    for (int f = 1; f < MaxPlayerNum; f++) {
        rank_alive(f);
        g_rank_world.tech_achieved[5] |= (uint8_t)(1 << f);
    }
    rankings(1);
    for (int f = 1; f < MaxPlayerNum; f++) {
        KCHECK((int)g_rank_world.rankings_unk[f] == -1);
        KCHECK(g_rank_world.players_data[f].ranking == 0);
    }
    KCHECK(g_rank_world.players_data[0].ranking == 1);   // the last pass wins

    // ---- the search itself stops at faction seven --------------------------
    // The row past the last faction is given the best score in the world.
    rank_reset();
    for (int f = 1; f < MaxPlayerNum; f++) {
        rank_alive(f);
        g_rank_world.players_data[f].pop_total = f;
    }
    g_rank_world.rankings_unk[MaxPlayerNum] = 9999;
    rankings(1);
    KCHECK(g_rank_world.rankings_unk[MaxPlayerNum] == 9999);
    KCHECK(g_rank_world.players_data[MaxPlayerNum].ranking == 0);
    KCHECK(g_rank_world.players_data[7].ranking == 7);

    // ---- the rank-to-faction table has exactly eight slots -----------------
    // Slot eight carries a human faction id: neither the clear that precedes
    // the table nor either of the two searches over it may reach it.
    rank_reset();
    for (int f = 1; f < MaxPlayerNum; f++) {
        rank_alive(f);
        g_rank_world.players_data[f].pop_total = f;
    }
    rank_human(3);
    g_rank_world.faction_rankings[MaxPlayerNum] = 3;
    g_rank_world.rank_unk1 = 0x5A;
    g_rank_world.rank_unk2 = 0x5A;
    rankings(1);
    KCHECK(g_rank_world.faction_rankings[MaxPlayerNum] == 3);   // not cleared
    KCHECK(g_rank_world.rank_unk1 == 3);   // found at rank 3, not at slot 8
    KCHECK(g_rank_world.rank_unk2 == 3);
    // Rank zero is inside the descending search. The all-negative field above
    // leaves every faction unranked, so the table's slot zero is the one that
    // holds the last living faction, and a human there must still be found.
    rank_reset();
    g_rank_world.technology[5].growth_value = -1;
    for (int f = 1; f < MaxPlayerNum; f++) {
        rank_alive(f);
        g_rank_world.tech_achieved[5] |= (uint8_t)(1 << f);
    }
    rank_human(7);
    g_rank_world.rank_unk1 = 0x5A;
    rankings(1);
    KCHECK(g_rank_world.faction_rankings[0] == 7);
    KCHECK(g_rank_world.rank_unk1 == 7);

    // Faction 5 is human but not alive, so it never reaches the table and
    // both searches must come up empty - unless one of them runs one slot too
    // far and finds the id parked in slot eight.
    rank_reset();
    for (int f = 1; f < MaxPlayerNum; f++) {
        if (f != 5) {
            rank_alive(f);
        }
        g_rank_world.players_data[f].pop_total = f;
    }
    rank_human(5);
    g_rank_world.faction_rankings[MaxPlayerNum] = 5;
    g_rank_world.rank_unk1 = 0x5A;
    g_rank_world.rank_unk2 = 0x5A;
    rankings(1);
    KCHECK(g_rank_world.faction_rankings[MaxPlayerNum] == 5);
    KCHECK(g_rank_world.rank_unk1 == 0);
    KCHECK(g_rank_world.rank_unk2 == 0);
}
GAMEPLAY_CASE(test_rankings_publication);


// Pick a seed whose first rnd(bound) lands inside [lo, hi), so a case can be
// written against a known roll without depending on which generator the C
// runtime ships. Returns 0 if no seed in range produced one, which the caller
// asserts on rather than silently proceeding.
unsigned rank_seed_rolling(int bound, int lo, int hi) {
    for (unsigned seed = 1; seed < 5000; seed++) {
        std::srand(seed);
        int roll = std::rand() % bound;
        if (roll >= lo && roll < hi) {
            return seed;
        }
    }
    return 0;
}

/*
 * The great_satan route to a target, which is the only one that does not also
 * make climactic_battle() true - and therefore the only one under which the
 * climactic +4 is absent and the other two urge terms can be seen on their
 * own.
 *
 * great_beelzebub wants a human faction sitting at rank 7 with more bases than
 * max(4, (turn + 25) / 50) and either difficulty above DLVL_SPECIALIST or
 * intense rivalry; great_satan then wants the turn past 100 and, with intense
 * rivalry in force, compares 4 * FactionRankingsUnk[FactionRankings[7]]
 * against 5 * FactionRankingsUnk[FactionRankings[6]] - both of which the
 * ranking pass has already struck out to -1, so the unsigned product makes the
 * comparison true.
 */
void rank_satan_scene(int target_id) {
    rank_reset();
    for (int f = 1; f < MaxPlayerNum; f++) {
        rank_alive(f);
        g_rank_world.players_data[f].pop_total = f;
    }
    g_rank_world.players_data[target_id].pop_total = 99;   // ranks 7th
    rank_human(target_id);
    g_rank_world.players_data[target_id].current_num_bases = 20;
    g_rank_world.game_rules = RULES_INTENSE_RIVALRY;
    g_rank_world.turn = 200;
}

void test_rankings_betrayal() {
    RankSeams seams;

    // ---- no target clears the state bit and stops --------------------------
    rank_reset();
    rank_alive(1);
    g_rank_world.game_state = STATE_UNK_200 | STATE_DEBUG_MODE;
    rankings(1);
    KCHECK(g_rank_world.game_state == STATE_DEBUG_MODE);

    // A climactic battle with no qualifying faction is still no target.
    rank_reset();
    for (int f = 1; f < MaxPlayerNum; f++) {
        rank_alive(f);
    }
    rank_human(3);
    g_rank_world.players_data[3].corner_market_turn = 99;   // climactic
    g_rank_world.players_data[3].diff_level = DLVL_LIBRARIAN;
    g_rank_world.game_state = STATE_UNK_200;
    rankings(1);
    KCHECK(g_rank_world.game_state == 0);

    // ---- difficulty four makes the human the target ------------------------
    rank_reset();
    for (int f = 1; f < MaxPlayerNum; f++) {
        rank_alive(f);
    }
    rank_human(3);
    g_rank_world.players_data[3].corner_market_turn = 99;
    g_rank_world.players_data[3].diff_level = DLVL_THINKER;
    rankings(1);
    KCHECK(g_rank_world.game_state == STATE_UNK_200);
    // So does intense rivalry at any difficulty.
    rank_reset();
    for (int f = 1; f < MaxPlayerNum; f++) {
        rank_alive(f);
    }
    rank_human(3);
    g_rank_world.players_data[3].corner_market_turn = 99;
    g_rank_world.game_rules = RULES_INTENSE_RIVALRY;
    rankings(1);
    KCHECK(g_rank_world.game_state == STATE_UNK_200);

    // ---- the last qualifying faction wins the target -----------------------
    // Two humans qualify; the one with the higher id is the target, which the
    // betrayal it provokes reports.
    rank_reset();
    for (int f = 1; f < MaxPlayerNum; f++) {
        rank_alive(f);
    }
    rank_human(2);
    rank_human(5);
    g_rank_world.players_data[2].corner_market_turn = 99;
    g_rank_world.players_data[2].diff_level = DLVL_THINKER;
    g_rank_world.players_data[5].diff_level = DLVL_THINKER;
    // Faction 4 is an AI in a treaty with everyone, and hates both.
    g_rank_world.players_data[4].diplo_treaties[2] = DTREATY_TREATY;
    g_rank_world.players_data[4].diplo_treaties[5] = DTREATY_TREATY;
    g_rank_world.players_data[4].ai_fight = 1;             // bound == 256
    g_rank_world.players_data[2].integrity_blemishes = 99;
    g_rank_world.players_data[5].integrity_blemishes = 99;
    rankings(1);
    KCHECK(g_rank_world.players_data[4].diplo_treaties[5] & DTREATY_SHALL_BETRAY);
    KCHECK(!(g_rank_world.players_data[4].diplo_treaties[2] & DTREATY_SHALL_BETRAY));

    // ---- the roll ------------------------------------------------------------
    // reputation 99 times difficulty 4 is 396, which beats every value
    // rnd(256) can return, so this betrayal is certain.
    rank_reset();
    for (int f = 1; f < MaxPlayerNum; f++) {
        rank_alive(f);
    }
    rank_human(5);
    g_rank_world.players_data[5].corner_market_turn = 99;
    g_rank_world.players_data[5].diff_level = DLVL_THINKER;
    g_rank_world.players_data[5].integrity_blemishes = 99;
    g_rank_world.players_data[4].diplo_treaties[5] = DTREATY_TREATY;
    g_rank_world.players_data[4].ai_fight = 1;
    rankings(1);
    KCHECK(g_rank_world.players_data[4].diplo_treaties[5]
           == (uint32_t)(DTREATY_TREATY | DTREATY_SHALL_BETRAY));
    // Reputation zero and no bonus leaves the urge at zero, which no roll can
    // fall below.
    rank_reset();
    for (int f = 1; f < MaxPlayerNum; f++) {
        rank_alive(f);
    }
    rank_human(5);
    g_rank_world.players_data[5].corner_market_turn = 99;
    g_rank_world.players_data[5].diff_level = DLVL_THINKER;
    g_rank_world.players_data[4].diplo_treaties[5] = DTREATY_TREATY;
    g_rank_world.players_data[4].ai_fight = 1;
    rankings(1);
    KCHECK(g_rank_world.players_data[4].diplo_treaties[5] == DTREATY_TREATY);
    // Reputation is the target's blemishes less what it has given this
    // faction, so a gift of 99 cancels it.
    rank_reset();
    for (int f = 1; f < MaxPlayerNum; f++) {
        rank_alive(f);
    }
    rank_human(5);
    g_rank_world.players_data[5].corner_market_turn = 99;
    g_rank_world.players_data[5].diff_level = DLVL_THINKER;
    g_rank_world.players_data[5].integrity_blemishes = 99;
    g_rank_world.players_data[5].diplo_unk1[4] = 99;
    g_rank_world.players_data[4].diplo_treaties[5] = DTREATY_TREATY;
    g_rank_world.players_data[4].ai_fight = 1;
    rankings(1);
    KCHECK(g_rank_world.players_data[4].diplo_treaties[5] == DTREATY_TREATY);
    // Wanting revenge is worth 4, and difficulty 4 multiplies it to 16 - not
    // enough against rnd(256), but enough against rnd(0), which is what an
    // ai_fight of 2 and no vendettas produces.
    rank_reset();
    for (int f = 1; f < MaxPlayerNum; f++) {
        rank_alive(f);
    }
    rank_human(5);
    g_rank_world.players_data[5].corner_market_turn = 99;
    g_rank_world.players_data[5].diff_level = DLVL_THINKER;
    g_rank_world.players_data[5].diplo_treaties[4] = DTREATY_WANT_REVENGE;
    g_rank_world.players_data[4].diplo_treaties[5] = DTREATY_TREATY;
    g_rank_world.players_data[4].ai_fight = 2;
    rankings(1);
    KCHECK(g_rank_world.players_data[4].diplo_treaties[5]
           == (uint32_t)(DTREATY_TREATY | DTREATY_SHALL_BETRAY));
    // Shameless betrayal of humans is the other 4.
    rank_reset();
    for (int f = 1; f < MaxPlayerNum; f++) {
        rank_alive(f);
    }
    rank_human(5);
    g_rank_world.players_data[5].corner_market_turn = 99;
    g_rank_world.players_data[5].diff_level = DLVL_THINKER;
    g_rank_world.players_data[4].flags_ext = PFLAGEXT_SHAMELESS_BETRAY_HUMANS;
    g_rank_world.players_data[4].diplo_treaties[5] = DTREATY_TREATY;
    g_rank_world.players_data[4].ai_fight = 2;
    rankings(1);
    KCHECK(g_rank_world.players_data[4].diplo_treaties[5]
           == (uint32_t)(DTREATY_TREATY | DTREATY_SHALL_BETRAY));
    // A climactic battle is a third 4, and it is always in force here, so a
    // bare treaty with an ai_fight of 2 already betrays.
    rank_reset();
    for (int f = 1; f < MaxPlayerNum; f++) {
        rank_alive(f);
    }
    rank_human(5);
    g_rank_world.players_data[5].corner_market_turn = 99;
    g_rank_world.players_data[5].diff_level = DLVL_THINKER;
    g_rank_world.players_data[4].diplo_treaties[5] = DTREATY_TREATY;
    g_rank_world.players_data[4].ai_fight = 2;
    rankings(1);
    KCHECK(g_rank_world.players_data[4].diplo_treaties[5]
           == (uint32_t)(DTREATY_TREATY | DTREATY_SHALL_BETRAY));
    // Every vendetta against the target widens the roll by 256, and two of
    // them put it back out of reach.
    rank_reset();
    for (int f = 1; f < MaxPlayerNum; f++) {
        rank_alive(f);
    }
    rank_human(5);
    g_rank_world.players_data[5].corner_market_turn = 99;
    g_rank_world.players_data[5].diff_level = DLVL_THINKER;
    g_rank_world.players_data[4].diplo_treaties[5] = DTREATY_TREATY;
    g_rank_world.players_data[4].ai_fight = 2;
    g_rank_world.players_data[6].diplo_treaties[5] = DTREATY_VENDETTA;
    g_rank_world.players_data[7].diplo_treaties[5] = DTREATY_VENDETTA;
    rankings(1);
    // urge is 16 against rnd(512): not certain, so assert only that the
    // vendetta count reached the roll at all by widening it - the AI's own
    // vendetta bit is untouched either way.
    KCHECK(!(g_rank_world.players_data[6].diplo_treaties[5] & DTREATY_SHALL_BETRAY));

    // ---- intense rivalry replaces the difficulty weight with five ----------
    // Difficulty 0 would make the urge zero; five keeps it at 20.
    rank_reset();
    for (int f = 1; f < MaxPlayerNum; f++) {
        rank_alive(f);
    }
    rank_human(5);
    g_rank_world.players_data[5].corner_market_turn = 99;
    g_rank_world.game_rules = RULES_INTENSE_RIVALRY;
    g_rank_world.players_data[4].diplo_treaties[5] = DTREATY_TREATY;
    g_rank_world.players_data[4].ai_fight = 2;
    rankings(1);
    KCHECK(g_rank_world.players_data[4].diplo_treaties[5]
           == (uint32_t)(DTREATY_TREATY | DTREATY_SHALL_BETRAY));

    // ---- who is eligible to betray -----------------------------------------
    {
        // ai_fight is set per case so that (vendetta_count - ai_fight + 2)
        // stays zero and the roll is rnd(0), which is always 0 and therefore
        // always below the urge of 16 this scene produces.
        //
        // DTREATY_SHALL_BETRAY is deliberately absent from this table. It is
        // the one rejection this output cannot see: the only thing a betrayal
        // does is OR that same bit in, so a faction that already carries it
        // reads identically whether the roll ran or not. The 0xC01 rejection
        // is covered by its other two bits instead.
        struct TreatyCase {
            uint32_t treaty;
            int ai_fight;
            bool betrays;
        };
        const TreatyCase treaty_cases[] = {
            {DTREATY_TREATY, 2, true},
            {DTREATY_TRUCE, 2, true},
            {DTREATY_PACT, 2, false},                            // rejected by 0xC01
            {DTREATY_PACT | DTREATY_TREATY, 2, false},           // and it rejects the pair
            {DTREATY_COMMLINK, 2, false},                        // none of the three
            {0, 2, false},
            {DTREATY_TREATY | DTREATY_UNK_800, 2, false},
            {DTREATY_TREATY | DTREATY_UNK_4000000, 2, false},
            {DTREATY_TRUCE | DTREATY_VENDETTA, 3, true},         // vendetta is no bar
        };
        for (unsigned i = 0; i < sizeof(treaty_cases) / sizeof(treaty_cases[0]); i++) {
            rank_reset();
            for (int f = 1; f < MaxPlayerNum; f++) {
                rank_alive(f);
            }
            rank_human(5);
            g_rank_world.players_data[5].corner_market_turn = 99;
            g_rank_world.players_data[5].diff_level = DLVL_THINKER;
            g_rank_world.players_data[4].diplo_treaties[5] = treaty_cases[i].treaty;
            g_rank_world.players_data[4].ai_fight = treaty_cases[i].ai_fight;
            rankings(1);
            bool betrayed = (g_rank_world.players_data[4].diplo_treaties[5]
                             & DTREATY_SHALL_BETRAY) != 0;
            KCHECK(betrayed == treaty_cases[i].betrays);
        }
    }

    // A human never betrays.
    rank_reset();
    for (int f = 1; f < MaxPlayerNum; f++) {
        rank_alive(f);
    }
    rank_human(5);
    rank_human(4);
    g_rank_world.players_data[5].corner_market_turn = 99;
    g_rank_world.players_data[5].diff_level = DLVL_THINKER;
    g_rank_world.players_data[4].diplo_treaties[5] = DTREATY_TREATY;
    g_rank_world.players_data[4].ai_fight = 2;
    rankings(1);
    KCHECK(g_rank_world.players_data[4].diplo_treaties[5] == DTREATY_TREATY);

    // ---- neither faction zero nor the row past the last is a betrayer ------
    rank_reset();
    for (int f = 1; f < MaxPlayerNum; f++) {
        rank_alive(f);
    }
    rank_human(5);
    g_rank_world.players_data[5].corner_market_turn = 99;
    g_rank_world.players_data[5].diff_level = DLVL_THINKER;
    g_rank_world.players_data[0].diplo_treaties[5] = DTREATY_TREATY;
    g_rank_world.players_data[0].ai_fight = 2;
    g_rank_world.players_data[MaxPlayerNum].diplo_treaties[5] = DTREATY_TREATY;
    g_rank_world.players_data[MaxPlayerNum].ai_fight = 2;
    rankings(1);
    KCHECK(g_rank_world.players_data[0].diplo_treaties[5] == DTREATY_TREATY);
    KCHECK(g_rank_world.players_data[MaxPlayerNum].diplo_treaties[5] == DTREATY_TREATY);

    // ---- the vendetta tally decides how wide the roll is -------------------
    // Three vendettas against the target and an ai_fight of 2 make the roll
    // rnd(768); the urge here is 16, and a seed is chosen so the roll lands
    // above it. A tally that missed any of the three would narrow the roll,
    // and a tally that missed all of them would collapse it to rnd(0), which
    // is always zero and always below 16.
    {
        unsigned seed = rank_seed_rolling(768, 20, 700);
        KCHECK(seed != 0);
        rank_reset();
        for (int f = 1; f < MaxPlayerNum; f++) {
            rank_alive(f);
        }
        rank_human(5);
        g_rank_world.players_data[5].corner_market_turn = 99;
        g_rank_world.players_data[5].diff_level = DLVL_THINKER;
        g_rank_world.players_data[4].diplo_treaties[5] = DTREATY_TREATY;
        g_rank_world.players_data[4].ai_fight = 2;
        g_rank_world.players_data[1].diplo_treaties[5] = DTREATY_VENDETTA;
        g_rank_world.players_data[2].diplo_treaties[5] = DTREATY_VENDETTA;
        g_rank_world.players_data[3].diplo_treaties[5] = DTREATY_VENDETTA;
        std::srand(seed);
        rankings(1);
        KCHECK(g_rank_world.players_data[4].diplo_treaties[5] == DTREATY_TREATY);
    }
    // Faction zero's and the guard row's vendettas are not part of the tally:
    // counting either would widen the roll from rnd(0) and lose the betrayal.
    rank_reset();
    for (int f = 1; f < MaxPlayerNum; f++) {
        rank_alive(f);
    }
    rank_human(5);
    g_rank_world.players_data[5].corner_market_turn = 99;
    g_rank_world.players_data[5].diff_level = DLVL_THINKER;
    g_rank_world.players_data[4].diplo_treaties[5] = DTREATY_TREATY;
    g_rank_world.players_data[4].ai_fight = 2;
    g_rank_world.players_data[0].diplo_treaties[5] = DTREATY_VENDETTA;
    g_rank_world.players_data[MaxPlayerNum].diplo_treaties[5] = DTREATY_VENDETTA;
    std::srand(rank_seed_rolling(256, 20, 250));
    rankings(1);
    KCHECK(g_rank_world.players_data[4].diplo_treaties[5]
           == (uint32_t)(DTREATY_TREATY | DTREATY_SHALL_BETRAY));

    // ---- the great_satan route -----------------------------------------------
    // No climactic battle here, so the urge starts at zero and rnd(0) is zero:
    // the comparison is strict, and nothing at all provokes a betrayal.
    rank_satan_scene(6);
    g_rank_world.players_data[4].diplo_treaties[6] = DTREATY_TREATY;
    g_rank_world.players_data[4].ai_fight = 2;
    rankings(1);
    KCHECK(g_rank_world.game_state == STATE_UNK_200);   // there IS a target
    KCHECK(g_rank_world.players_data[4].diplo_treaties[6] == DTREATY_TREATY);
    // The target really is the top-ranked faction: nobody else's treaties are
    // consulted.
    rank_satan_scene(6);
    g_rank_world.players_data[4].diplo_treaties[6] = DTREATY_TREATY;
    g_rank_world.players_data[4].diplo_treaties[2] = DTREATY_TREATY;
    g_rank_world.players_data[4].ai_fight = 2;
    g_rank_world.players_data[4].flags_ext = PFLAGEXT_SHAMELESS_BETRAY_HUMANS;
    rankings(1);
    KCHECK(g_rank_world.players_data[4].diplo_treaties[6]
           == (uint32_t)(DTREATY_TREATY | DTREATY_SHALL_BETRAY));
    KCHECK(g_rank_world.players_data[4].diplo_treaties[2] == DTREATY_TREATY);
    // Wanting revenge on its own, with no climactic battle to add to it.
    rank_satan_scene(6);
    g_rank_world.players_data[4].diplo_treaties[6] = DTREATY_TREATY;
    g_rank_world.players_data[4].ai_fight = 2;
    g_rank_world.players_data[6].diplo_treaties[4] = DTREATY_WANT_REVENGE;
    rankings(1);
    KCHECK(g_rank_world.players_data[4].diplo_treaties[6]
           == (uint32_t)(DTREATY_TREATY | DTREATY_SHALL_BETRAY));
    // And the weight the three terms are multiplied by: intense rivalry makes
    // it five, so an urge of 4 becomes 20 and clears a roll seeded below it,
    // where the unmultiplied 4 would not.
    {
        unsigned seed = rank_seed_rolling(256, 5, 19);
        KCHECK(seed != 0);
        rank_satan_scene(6);
        g_rank_world.players_data[4].diplo_treaties[6] = DTREATY_TREATY;
        g_rank_world.players_data[4].ai_fight = 1;      // rnd(256)
        g_rank_world.players_data[4].flags_ext = PFLAGEXT_SHAMELESS_BETRAY_HUMANS;
        std::srand(seed);
        rankings(1);
        KCHECK(g_rank_world.players_data[4].diplo_treaties[6]
               == (uint32_t)(DTREATY_TREATY | DTREATY_SHALL_BETRAY));
    }

    // ---- the climactic override beats great_satan, and only from faction 1 -
    // A great_satan target is in place; faction zero is human and at
    // difficulty 4, and a climactic battle is on. The override scan starts at
    // faction 1, so faction zero cannot claim the target - and if it did, a
    // target of zero would clear the state bit instead of setting it.
    // Intense rivalry is dropped for this one so the great_satan target does
    // not also qualify for the override; RULES_VICTORY_CONQUEST and a turn
    // past 250 keep great_satan true without it.
    rank_satan_scene(6);
    g_rank_world.game_rules = RULES_VICTORY_CONQUEST;
    g_rank_world.players_data[6].diff_level = DLVL_TALENT;
    g_rank_world.turn = 300;
    rank_human(0);
    g_rank_world.players_data[0].diff_level = DLVL_THINKER;
    g_rank_world.players_data[6].corner_market_turn = 999;   // climactic
    g_rank_world.players_data[4].diplo_treaties[6] = DTREATY_TREATY;
    g_rank_world.players_data[4].ai_fight = 2;
    rankings(1);
    KCHECK(g_rank_world.game_state == STATE_UNK_200);
    // Faction 6 is still the target, which its betrayal reports; an override
    // that had reached faction zero would have cleared the bit instead.
    KCHECK(g_rank_world.players_data[4].diplo_treaties[6]
           == (uint32_t)(DTREATY_TREATY | DTREATY_SHALL_BETRAY));
}
GAMEPLAY_CASE(test_rankings_betrayal);

#undef KCHECK

/*
 * A self-contained world for valid_patrol().
 *
 * The same 16-by-8 map the territory and reset_territory fixtures use, and
 * for the same reason. Everything is dry land in region 2 until a case says
 * otherwise, and the three prototypes carry one chassis each so a case picks
 * its triad by picking a unit.
 *
 * valid_patrol reaches base_at, base_on_sea, port_to_port and speed, and the
 * last of those has a closure of its own - speed_proto, morale_veh,
 * has_project. Two settings keep it decidable: every unit's home base is -1,
 * which is what stops morale_veh walking into has_fac_built, and the secret
 * projects are all SP_Unbuilt so has_project answers no. What speed() then
 * returns is a fact about the fixture rather than about valid_patrol, so the
 * air cases read it back and build the expected reach from it; a perturbed
 * reach formula in the recovery still fails against that, because the test's
 * copy of the formula is independent of the recovery's.
 */
struct PatrolWorld {
    Map tiles[192];
    Base bases[8];
    Veh vehs[4];
    VehPrototype protos[128];
    RulesChassis chassis[9];
    RulesWeapon weapons[8];
    RulesBasic rules;
    Player players[8];
    PlayerData players_data[8];
    int projects[MaxSecretProjectNum];
    Continent continents[MaxContinentNum];
    Map *tiles_ptr;
    uint32_t longitude;
    int lon_bounds;
    int lat_bounds;
    BOOL is_flat;
    int base_count;
    int veh_count;
    int base_find_dist;
};

PatrolWorld g_patrol_world;

const int PATROL_LIVE = 64;

Map &patrol_at(int x, int y) {
    return g_patrol_world.tiles[PATROL_LIVE + (x >> 1) + y * 8];
}

void patrol_reset() {
    std::memset(&g_patrol_world, 0, sizeof(g_patrol_world));
    g_patrol_world.tiles_ptr = &g_patrol_world.tiles[PATROL_LIVE];
    g_patrol_world.longitude = 8;
    g_patrol_world.lon_bounds = 16;
    g_patrol_world.lat_bounds = 8;
    g_patrol_world.is_flat = 1;
    g_patrol_world.rules.move_rate_roads = 1;
    for (int i = 0; i < MaxSecretProjectNum; i++) {
        g_patrol_world.projects[i] = SP_Unbuilt;
    }
    // EVERY tile of the array, not just the live window: a coordinate one past
    // the map still lands inside the array, and giving it the same region as
    // the live map means the bounds test is the only thing that can refuse it.
    for (int i = 0; i < 192; i++) {
        g_patrol_world.tiles[i].climate = 0x60;   // altitude 3: dry land
        g_patrol_world.tiles[i].region = 2;
    }
    // One chassis per triad; the air chassis has a range, the others do not.
    g_patrol_world.chassis[0].triad = TRIAD_LAND;
    g_patrol_world.chassis[0].speed = 1;
    g_patrol_world.chassis[1].triad = TRIAD_SEA;
    g_patrol_world.chassis[1].speed = 4;
    g_patrol_world.chassis[2].triad = TRIAD_AIR;
    g_patrol_world.chassis[2].speed = 1;   // small, so a reach fits on the map
    g_patrol_world.chassis[2].range = 8;
    for (int i = 0; i < 3; i++) {
        g_patrol_world.protos[MaxVehProtoFactionNum + i].chassis_id = (uint8_t)i;
    }
}

// Unit 0, of the given triad, at the given tile. Home base -1 keeps morale_veh
// out of the base facilities.
void patrol_veh(int triad, int x, int y) {
    Veh &veh = g_patrol_world.vehs[0];
    veh.x = (int16_t)x;
    veh.y = (int16_t)y;
    veh.faction_id = 1;
    veh.proto_id = (int16_t)(MaxVehProtoFactionNum + triad);
    veh.home_base_id = -1;
    g_patrol_world.veh_count = 1;
}

void patrol_base(int base_id, int x, int y) {
    g_patrol_world.bases[base_id].x = (int16_t)x;
    g_patrol_world.bases[base_id].y = (int16_t)y;
    g_patrol_world.bases[base_id].faction_id_current = 1;
    patrol_at(x, y).bit |= BIT_BASE_IN_TILE;
    if (g_patrol_world.base_count <= base_id) {
        g_patrol_world.base_count = base_id + 1;
    }
}

void patrol_ocean(int x, int y, int region) {
    patrol_at(x, y).climate = 0x40;   // below ALT_SHORE_LINE
    patrol_at(x, y).region = (uint8_t)region;
}

#define VCHECK(cond)                                                          \
    do {                                                                      \
        const bool patrol_ok = (cond);                                        \
        if (!patrol_ok) {                                                     \
            std::fprintf(stderr, "valid_patrol: line %d: %s\n", __LINE__,     \
                         #cond);                                              \
        }                                                                     \
        expect(patrol_ok);                                                    \
    } while (0)

class PatrolSeams {
 public:
    PatrolSeams()
        : tiles_(&MapTiles, &g_patrol_world.tiles_ptr),
          longitude_(&MapLongitude, &g_patrol_world.longitude),
          lon_(&MapLongitudeBounds, &g_patrol_world.lon_bounds),
          lat_(&MapLatitudeBounds, &g_patrol_world.lat_bounds),
          flat_(&MapIsFlat, &g_patrol_world.is_flat),
          continents_(&Continents, g_patrol_world.continents),
          bases_(&Bases, g_patrol_world.bases),
          base_count_(&BaseCurrentCount, &g_patrol_world.base_count),
          base_dist_(&BaseFindDist, &g_patrol_world.base_find_dist),
          vehs_(&Vehs, g_patrol_world.vehs),
          veh_count_(&VehCurrentCount, &g_patrol_world.veh_count),
          protos_(&VehPrototypes, g_patrol_world.protos),
          chassis_(&Chassis, g_patrol_world.chassis),
          weapons_(&Weapon, g_patrol_world.weapons),
          rules_(&Rules, &g_patrol_world.rules),
          players_(&Players, g_patrol_world.players),
          players_data_(&PlayersData, g_patrol_world.players_data),
          projects_(&SecretProject,
                    reinterpret_cast<BaseSecretProject *>(g_patrol_world.projects)) { }

 private:
    ScopedSeam<Map *> tiles_;
    ScopedSeam<uint32_t> longitude_;
    ScopedSeam<int> lon_;
    ScopedSeam<int> lat_;
    ScopedSeam<BOOL> flat_;
    ScopedSeam<Continent> continents_;
    ScopedSeam<Base> bases_;
    ScopedSeam<int> base_count_;
    ScopedSeam<int> base_dist_;
    ScopedSeam<Veh> vehs_;
    ScopedSeam<int> veh_count_;
    ScopedSeam<VehPrototype> protos_;
    ScopedSeam<RulesChassis> chassis_;
    ScopedSeam<RulesWeapon> weapons_;
    ScopedSeam<RulesBasic> rules_;
    ScopedSeam<Player> players_;
    ScopedSeam<PlayerData> players_data_;
    ScopedSeam<BaseSecretProject> projects_;
};

void test_valid_patrol_bounds() {
    PatrolSeams seams;

    // ---- the tile the unit is already on is never a waypoint ---------------
    patrol_reset();
    patrol_veh(TRIAD_LAND, 6, 2);
    VCHECK(!valid_patrol(0, 6, 2));
    // Either coordinate differing is enough.
    VCHECK(valid_patrol(0, 8, 2));
    VCHECK(valid_patrol(0, 6, 4));

    // ---- the four bounds ----------------------------------------------------
    patrol_reset();
    patrol_veh(TRIAD_LAND, 6, 2);
    VCHECK(!valid_patrol(0, 6, -1));
    VCHECK(valid_patrol(0, 6, 0));
    VCHECK(!valid_patrol(0, 6, 8));
    VCHECK(valid_patrol(0, 7, 7));
    VCHECK(!valid_patrol(0, -1, 2));
    VCHECK(valid_patrol(0, 0, 2));
    VCHECK(!valid_patrol(0, 16, 2));
    VCHECK(valid_patrol(0, 14, 2));

    // ---- land compares regions ----------------------------------------------
    patrol_reset();
    patrol_veh(TRIAD_LAND, 6, 2);
    patrol_at(10, 2).region = 3;
    VCHECK(!valid_patrol(0, 10, 2));
    VCHECK(valid_patrol(0, 12, 2));
    // It is the unit's own tile on the other side of the comparison.
    patrol_reset();
    patrol_veh(TRIAD_LAND, 6, 2);
    patrol_at(6, 2).region = 3;
    patrol_at(10, 2).region = 3;
    VCHECK(valid_patrol(0, 10, 2));
    VCHECK(!valid_patrol(0, 12, 2));

    // ---- a triad the table does not know is always allowed -----------------
    patrol_reset();
    patrol_veh(TRIAD_LAND, 6, 2);
    g_patrol_world.chassis[0].triad = 3;
    patrol_at(10, 2).region = 3;
    VCHECK(valid_patrol(0, 10, 2));
}
GAMEPLAY_CASE(test_valid_patrol_bounds);

void test_valid_patrol_sea() {
    PatrolSeams seams;

    // ---- open water at both ends falls back to the region comparison -------
    patrol_reset();
    patrol_veh(TRIAD_SEA, 6, 2);
    patrol_at(10, 2).region = 3;
    VCHECK(!valid_patrol(0, 10, 2));
    VCHECK(valid_patrol(0, 12, 2));

    // ---- two ports go through port_to_port ---------------------------------
    // Both bases sit next to the same ocean region, so the water joins them.
    patrol_reset();
    patrol_veh(TRIAD_SEA, 4, 2);
    patrol_base(0, 4, 2);
    patrol_base(1, 12, 2);
    patrol_ocean(5, 1, 9);
    patrol_ocean(13, 1, 9);
    VCHECK(valid_patrol(0, 12, 2));
    // Different oceans: no route.
    patrol_reset();
    patrol_veh(TRIAD_SEA, 4, 2);
    patrol_base(0, 4, 2);
    patrol_base(1, 12, 2);
    patrol_ocean(5, 1, 9);
    patrol_ocean(13, 1, 10);
    VCHECK(!valid_patrol(0, 12, 2));
    // Landlocked at one end: no route either.
    patrol_reset();
    patrol_veh(TRIAD_SEA, 4, 2);
    patrol_base(0, 4, 2);
    patrol_base(1, 12, 2);
    patrol_ocean(5, 1, 9);
    VCHECK(!valid_patrol(0, 12, 2));

    // ---- a port and open water go through base_on_sea ----------------------
    // The unit is in a base; the destination is a water tile of region 9,
    // which that base touches.
    patrol_reset();
    patrol_veh(TRIAD_SEA, 4, 2);
    patrol_base(0, 4, 2);
    patrol_ocean(5, 1, 9);
    patrol_ocean(12, 2, 9);
    VCHECK(valid_patrol(0, 12, 2));
    // A destination in a water region the base does not touch.
    patrol_reset();
    patrol_veh(TRIAD_SEA, 4, 2);
    patrol_base(0, 4, 2);
    patrol_ocean(5, 1, 9);
    patrol_ocean(12, 2, 10);
    VCHECK(!valid_patrol(0, 12, 2));
    // Base id zero is still a base on BOTH sides of the port test. With the
    // unit in base 1 and base 0 at the destination, the two ports are joined
    // by ocean region 9 - while the base_on_sea fallback the other arm would
    // take asks about the destination TILE's region, which is land, and
    // fails. Only a test that admits id zero answers yes.
    patrol_reset();
    patrol_veh(TRIAD_SEA, 4, 2);
    patrol_base(0, 12, 2);
    patrol_base(1, 4, 2);
    patrol_ocean(5, 1, 9);
    patrol_ocean(13, 1, 9);
    VCHECK(valid_patrol(0, 12, 2));

    // Base id zero at the destination is still a base: the sea branch tests
    // it against zero with >=, unlike the air branch's landing-place test.
    patrol_reset();
    patrol_veh(TRIAD_SEA, 12, 2);
    patrol_ocean(12, 2, 9);
    patrol_base(0, 4, 2);
    patrol_at(4, 2).region = 3;      // so the region fallback would disagree
    patrol_ocean(5, 1, 9);
    VCHECK(valid_patrol(0, 4, 2));   // through base_on_sea, which succeeds
    patrol_reset();
    patrol_veh(TRIAD_SEA, 12, 2);
    patrol_ocean(12, 2, 9);
    patrol_base(0, 4, 2);
    patrol_at(4, 2).region = 3;
    patrol_ocean(5, 1, 10);
    VCHECK(!valid_patrol(0, 4, 2));  // base_on_sea fails; no region fallback

    // The mirror image: the unit is in open water and the destination is the
    // port, so the base is asked about the UNIT's region.
    patrol_reset();
    patrol_veh(TRIAD_SEA, 12, 2);
    patrol_ocean(12, 2, 9);
    patrol_base(0, 4, 2);
    patrol_ocean(5, 1, 9);
    VCHECK(valid_patrol(0, 4, 2));
    patrol_reset();
    patrol_veh(TRIAD_SEA, 12, 2);
    patrol_ocean(12, 2, 10);
    patrol_base(0, 4, 2);
    patrol_ocean(5, 1, 9);
    VCHECK(!valid_patrol(0, 4, 2));
}
GAMEPLAY_CASE(test_valid_patrol_sea);

// Burn the turn's remaining movement to nothing and leave exactly `steps`
// range steps of flight time, so the reachable distance is speed * steps
// divided by move_rate_roads - which the fixture keeps at 1.
void patrol_air_reach(int steps) {
    g_patrol_world.vehs[0].moves_expended = 255;
    g_patrol_world.vehs[0].terraforming_turns =
        (uint8_t)(g_patrol_world.chassis[2].range - 1 - steps);
}

void test_valid_patrol_air() {
    PatrolSeams seams;

    // ---- a chassis with no range is unlimited ------------------------------
    patrol_reset();
    patrol_veh(TRIAD_AIR, 6, 2);
    g_patrol_world.chassis[2].range = 0;
    patrol_at(14, 6).region = 3;
    VCHECK(valid_patrol(0, 14, 6));

    // ---- with both ends a landing place, the whole reach is available ------
    {
        patrol_reset();
        patrol_veh(TRIAD_AIR, 4, 2);
        patrol_at(4, 2).bit |= BIT_AIRBASE;
        patrol_air_reach(1);
        int reach = (int)speed(0, false);
        int x_ok = 4 + 2 * reach;
        int x_bad = 4 + 2 * (reach + 1);
        VCHECK(reach >= 1 && x_bad <= 14);
        patrol_at(x_ok, 2).bit |= BIT_AIRBASE;
        patrol_at(x_bad, 2).bit |= BIT_AIRBASE;
        VCHECK(valid_patrol(0, x_ok, 2));
        VCHECK(!valid_patrol(0, x_bad, 2));
    }

    // ---- half the reach when an end is NOT a landing place -----------------
    // "Landing place" is base_at() != 0 or an airbase, and base_at() answers
    // -1 on an empty tile - so empty ground is a landing place and the ONE
    // thing that is not is a tile holding base id zero without an airbase.
    {
        patrol_reset();
        patrol_veh(TRIAD_AIR, 4, 2);
        patrol_base(0, 4, 2);           // base id zero, under the unit
        patrol_air_reach(2);
        int reach = (int)speed(0, false) * 2;
        int half = reach / 2;
        VCHECK(half >= 1 && 4 + 2 * (half + 1) <= 14);
        VCHECK(valid_patrol(0, 4 + 2 * half, 2));
        VCHECK(!valid_patrol(0, 4 + 2 * (half + 1), 2));
        // An airbase on the same tile restores the whole reach.
        patrol_at(4, 2).bit |= BIT_AIRBASE;
        VCHECK(valid_patrol(0, 4 + 2 * (half + 1), 2));
    }
    // Base id one under the unit is a landing place, so the whole reach
    // applies without any airbase - which is what makes the case above a
    // statement about the id and not about bases in general.
    {
        patrol_reset();
        patrol_veh(TRIAD_AIR, 4, 2);
        patrol_base(0, 12, 6);          // occupies id zero somewhere else
        patrol_base(1, 4, 2);
        patrol_air_reach(2);
        int reach = (int)speed(0, false) * 2;
        int half = reach / 2;
        VCHECK(valid_patrol(0, 4 + 2 * (half + 1), 2));
    }
    // And the far end: base id zero there, with the unit on open ground.
    {
        patrol_reset();
        patrol_veh(TRIAD_AIR, 4, 2);
        patrol_air_reach(2);
        int reach = (int)speed(0, false) * 2;
        int half = reach / 2;
        int x_far = 4 + 2 * (half + 1);
        VCHECK(valid_patrol(0, x_far, 2));   // empty ground: a landing place
        patrol_base(0, x_far, 2);
        VCHECK(!valid_patrol(0, x_far, 2));
        patrol_at(x_far, 2).bit |= BIT_AIRBASE;
        VCHECK(valid_patrol(0, x_far, 2));
    }

    // ---- the round-map wrap, on bit 0 of MapIsFlat -------------------------
    // Fourteen columns apart is distance 7 the long way and 1 the short way.
    {
        patrol_reset();
        patrol_veh(TRIAD_AIR, 0, 2);
        patrol_at(0, 2).bit |= BIT_AIRBASE;
        patrol_at(14, 2).bit |= BIT_AIRBASE;
        patrol_air_reach(1);
        int reach = (int)speed(0, false);
        VCHECK(reach >= 1 && reach < 7);
        VCHECK(!valid_patrol(0, 14, 2));
        g_patrol_world.is_flat = 0;
        VCHECK(valid_patrol(0, 14, 2));
        g_patrol_world.is_flat = 2;         // even: still a round map
        VCHECK(valid_patrol(0, 14, 2));
        g_patrol_world.is_flat = 3;         // odd: flat again
        VCHECK(!valid_patrol(0, 14, 2));
    }

    // ---- flight time already burned shortens the reach ---------------------
    {
        patrol_reset();
        patrol_veh(TRIAD_AIR, 4, 2);
        patrol_at(4, 2).bit |= BIT_AIRBASE;
        patrol_at(6, 2).bit |= BIT_AIRBASE;
        patrol_air_reach(1);
        VCHECK(valid_patrol(0, 6, 2));      // distance 1, one step left
        patrol_air_reach(0);
        VCHECK(!valid_patrol(0, 6, 2));     // no steps left, reach 0
    }

    // ---- the turn's remaining movement is added ----------------------------
    {
        patrol_reset();
        patrol_veh(TRIAD_AIR, 4, 2);
        patrol_air_reach(0);                         // no flight time left
        g_patrol_world.vehs[0].moves_expended = 0;   // but a full turn to spend
        int sp = (int)speed(0, false);
        VCHECK(sp >= 1 && 4 + 2 * (sp + 1) <= 14);
        VCHECK(valid_patrol(0, 4 + 2 * sp, 2));
        // One tile further is out of reach - unless the movement had been
        // clamped up to 999 on its way in.
        VCHECK(!valid_patrol(0, 4 + 2 * (sp + 1), 2));
    }

    // ---- and clamped at 999 ------------------------------------------------
    // move_rate_roads of 200 with a chassis speed of 99 makes the turn's
    // movement 19800, which the clamp cuts to 999 - a reach of 4 rather than
    // 99. Only the clamp keeps the far tile out of range.
    {
        patrol_reset();
        g_patrol_world.rules.move_rate_roads = 200;
        g_patrol_world.chassis[2].speed = 99;
        patrol_veh(TRIAD_AIR, 4, 2);
        patrol_air_reach(0);
        g_patrol_world.vehs[0].moves_expended = 0;
        VCHECK((int)speed(0, false) >= 19800);
        VCHECK(valid_patrol(0, 12, 2));    // distance 4, and the reach is 4
        VCHECK(!valid_patrol(0, 14, 2));   // distance 5
    }

    // ---- the wrap arms strictly above MapLongitude -------------------------
    // Twelve columns of bounds against a MapLongitude of 8: a delta of exactly
    // 8 is not wrapped, and the long way is distance 4 while the short way
    // would be 2.
    {
        patrol_reset();
        patrol_veh(TRIAD_AIR, 0, 2);
        g_patrol_world.is_flat = 0;
        g_patrol_world.lon_bounds = 12;
        int sp = (int)speed(0, false);
        VCHECK(sp >= 1 && sp <= 2);
        patrol_air_reach(2 / sp);
        int reach = sp * (2 / sp);
        VCHECK(reach >= 2 && reach <= 3);
        VCHECK(!valid_patrol(0, 8, 2));    // delta 8: not wrapped, distance 4
        VCHECK(valid_patrol(0, 10, 2));    // delta 10: wrapped to 2, distance 1
    }
}
GAMEPLAY_CASE(test_valid_patrol_air);

#undef VCHECK

/*
 * scan_prototypes (0x0053A4A0).
 *
 * The function publishes nothing but two prototype ids, so every assertion
 * here is about WHICH prototype wins. That bounds what any fixture can see:
 * a change that scales every score by the same factor cannot be observed at
 * all, which is why the plan term's shift width is recorded as an equivalence
 * in the Verification note rather than pinned here.
 *
 * Two prototype bands are under test - the viewer's 64 slots and the owner's
 * 64 - and every slot outside them holds a live PLAN_OFFENSIVE prototype, the
 * best-scoring plan there is. A scan that runs one slot too far in either
 * direction therefore lands on something that takes the answer away from the
 * prototype the case set up, instead of on a zeroed slot that could not.
 */
struct ScanWorld {
    VehPrototype protos[320];
    RulesChassis chassis[4];
    RulesWeapon weapons[8];
    RulesArmor armor[8];
    RulesBasic rules;
    PlayerData players_data[8];
    BOOL expansion;
    int best_trade;
    int best_mention;
};

ScanWorld g_scan_world;

// faction_id: the yardstick. Its prototypes cancel the owner's.
const int SCAN_VIEWER = 1;
// faction_id_with: the band actually scanned and scored.
const int SCAN_OWNER = 3;

const int SCAN_TOTAL_PROTOS = 320;

int scan_id(int faction, int slot) {
    return faction * MaxVehProtoFactionNum + slot;
}

VehPrototype &scan_at(int index) {
    return g_scan_world.protos[index];
}

VehPrototype &scan_owned(int slot) {
    return g_scan_world.protos[scan_id(SCAN_OWNER, slot)];
}

VehPrototype &scan_rival(int slot) {
    return g_scan_world.protos[scan_id(SCAN_VIEWER, slot)];
}

void scan_reset() {
    std::memset(&g_scan_world, 0, sizeof(g_scan_world));
    g_scan_world.expansion = 1;   // keep arm_strat/weap_strat off their psi paths

    // Chassis 0 and 1 are the same triad at different speeds; chassis 2 is a
    // different triad at chassis 0's speed, so a triad mismatch can be set up
    // without also changing the score.
    g_scan_world.chassis[0].triad = TRIAD_LAND;
    g_scan_world.chassis[0].speed = 0;
    g_scan_world.chassis[1].triad = TRIAD_LAND;
    g_scan_world.chassis[1].speed = 4;
    g_scan_world.chassis[2].triad = TRIAD_SEA;
    g_scan_world.chassis[2].speed = 0;
    g_scan_world.chassis[3].triad = TRIAD_AIR;
    g_scan_world.chassis[3].speed = 0;

    // Weapon k is read two ways: Weapon[k].offense_rating scores it, and
    // Armor[k].defense_rating is what arm_strat answers for it, because the
    // original passes the weapon id to arm_strat.
    g_scan_world.weapons[0].offense_rating = 0;
    g_scan_world.weapons[1].offense_rating = 2;
    g_scan_world.weapons[2].offense_rating = 3;
    g_scan_world.weapons[3].offense_rating = 4;
    g_scan_world.weapons[4].offense_rating = 2;
    g_scan_world.armor[0].defense_rating = 0;
    g_scan_world.armor[1].defense_rating = 2;
    g_scan_world.armor[2].defense_rating = 2;
    g_scan_world.armor[3].defense_rating = 3;
    g_scan_world.armor[4].defense_rating = 1;
    g_scan_world.armor[5].defense_rating = 100;   // only reachable through armor_id

    for (int i = 0; i < SCAN_TOTAL_PROTOS; i++) {
        int faction = i / MaxVehProtoFactionNum;
        if (faction == SCAN_VIEWER || faction == SCAN_OWNER) {
            continue;
        }
        VehPrototype &guard = g_scan_world.protos[i];
        guard.flags = PROTO_ACTIVE | PROTO_TYPED_COMPLETE;
        guard.plan = PLAN_OFFENSIVE;   // outscores every case's own candidates
        guard.chassis_id = 3;          // a triad no case uses
        guard.weapon_id = 0;
        guard.armor_id = 0;
        guard.reactor_id = 0;
    }
}

// A live, unremarkable prototype of the owning faction.
void scan_live(int slot, int plan) {
    VehPrototype &proto = scan_owned(slot);
    proto.flags = PROTO_ACTIVE | PROTO_TYPED_COMPLETE;
    proto.plan = (uint8_t)plan;
    proto.chassis_id = 0;
    proto.weapon_id = 1;
    proto.armor_id = 1;
    proto.reactor_id = 1;
}

// A live prototype of the viewer, shaped to match the owner's slot so it can
// be pushed into or out of cancelling it one field at a time.
void scan_live_rival(int slot, int plan) {
    VehPrototype &rival = scan_rival(slot);
    rival.flags = PROTO_ACTIVE | PROTO_TYPED_COMPLETE;
    rival.plan = (uint8_t)plan;
    rival.chassis_id = 0;
    rival.weapon_id = 1;
    rival.armor_id = 1;
    rival.reactor_id = 1;
}

void scan_run() {
    // Poison rather than clear: the entry stores are the only thing that can
    // put -1 in these, and a zeroed pair would look like a plausible result.
    g_scan_world.best_trade = 0x5A5A5A;
    g_scan_world.best_mention = 0x5A5A5A;
    scan_prototypes(SCAN_VIEWER, SCAN_OWNER);
}

#define SCHECK(cond)                                                          \
    do {                                                                      \
        const bool scan_ok = (cond);                                          \
        if (!scan_ok) {                                                       \
            std::fprintf(stderr, "scan_prototypes: line %d: %s\n", __LINE__,  \
                         #cond);                                              \
        }                                                                     \
        expect(scan_ok);                                                      \
    } while (0)

class ScanSeams {
 public:
    ScanSeams()
        : protos_(&VehPrototypes, g_scan_world.protos),
          chassis_(&Chassis, g_scan_world.chassis),
          weapons_(&Weapon, g_scan_world.weapons),
          armor_(&Armor, g_scan_world.armor),
          rules_(&Rules, &g_scan_world.rules),
          players_data_(&PlayersData, g_scan_world.players_data),
          expansion_(&ExpansionEnabled, &g_scan_world.expansion),
          trade_(&BestProtoForTrade, &g_scan_world.best_trade),
          mention_(&BestProtoToMention, &g_scan_world.best_mention) { }

 private:
    ScopedSeam<VehPrototype> protos_;
    ScopedSeam<RulesChassis> chassis_;
    ScopedSeam<RulesWeapon> weapons_;
    ScopedSeam<RulesArmor> armor_;
    ScopedSeam<RulesBasic> rules_;
    ScopedSeam<PlayerData> players_data_;
    ScopedSeam<BOOL> expansion_;
    ScopedSeam<int> trade_;
    ScopedSeam<int> mention_;
};

void test_scan_prototypes_selection() {
    ScanSeams seams;

    // ---- nothing qualifies, and both answers are cleared -------------------
    // The whole owner band is retired, so only the entry stores can move the
    // poison.
    scan_reset();
    scan_run();
    SCHECK(g_scan_world.best_trade == -1);
    SCHECK(g_scan_world.best_mention == -1);

    // ---- one live prototype takes both answers -----------------------------
    scan_reset();
    scan_live(5, PLAN_COMBAT);
    scan_run();
    SCHECK(g_scan_world.best_trade == scan_id(SCAN_OWNER, 5));
    SCHECK(g_scan_world.best_mention == scan_id(SCAN_OWNER, 5));

    // ---- both prototype flags are required, separately ---------------------
    scan_reset();
    scan_live(5, PLAN_COMBAT);
    scan_owned(5).flags = PROTO_ACTIVE;
    scan_run();
    SCHECK(g_scan_world.best_trade == -1);
    scan_owned(5).flags = PROTO_TYPED_COMPLETE;
    scan_run();
    SCHECK(g_scan_world.best_trade == -1);
    scan_owned(5).flags = PROTO_ACTIVE | PROTO_TYPED_COMPLETE;
    scan_run();
    SCHECK(g_scan_world.best_trade == scan_id(SCAN_OWNER, 5));
    // An unrelated flag does not disqualify it.
    scan_owned(5).flags = PROTO_ACTIVE | PROTO_TYPED_COMPLETE | PROTO_CUSTOM_NAME_SET;
    scan_run();
    SCHECK(g_scan_world.best_trade == scan_id(SCAN_OWNER, 5));

    // ---- obsolescence is read against the OWNING faction's bit -------------
    scan_reset();
    scan_live(5, PLAN_COMBAT);
    scan_owned(5).obsolete_factions = (uint8_t)(1 << SCAN_OWNER);
    scan_run();
    SCHECK(g_scan_world.best_trade == -1);
    // Every other faction's bit set, the owner's clear: still scanned.
    scan_owned(5).obsolete_factions = (uint8_t)~(1 << SCAN_OWNER);
    scan_run();
    SCHECK(g_scan_world.best_trade == scan_id(SCAN_OWNER, 5));
    // Including the viewer's own bit, which governs the rival loop instead.
    scan_owned(5).obsolete_factions = (uint8_t)(1 << SCAN_VIEWER);
    scan_run();
    SCHECK(g_scan_world.best_trade == scan_id(SCAN_OWNER, 5));

    // ---- the plan term outranks everything else ----------------------------
    // Slot 1 is the best possible unit on a worse plan; slot 2 is the worst
    // possible unit on a better one, and slot 2 still wins.
    scan_reset();
    scan_live(1, PLAN_COMBAT);
    scan_owned(1).weapon_id = 3;    // offense 4
    scan_owned(1).armor_id = 3;     // defense 3
    scan_owned(1).chassis_id = 1;   // speed 4
    scan_live(2, PLAN_OFFENSIVE);
    scan_owned(2).weapon_id = 0;
    scan_owned(2).armor_id = 0;
    scan_owned(2).chassis_id = 0;
    scan_run();
    SCHECK(g_scan_world.best_trade == scan_id(SCAN_OWNER, 2));

    // ---- weapon counts three, armour counts four ---------------------------
    // Same plan, so the tie-break terms decide. Slot 1 is offense 4 and no
    // armour: 12. Slot 2 is armour 3 and no weapon: also 12, and the tie goes
    // to the lower slot.
    scan_reset();
    scan_live(1, PLAN_COMBAT);
    scan_owned(1).weapon_id = 3;    // offense 4 -> 12
    scan_owned(1).armor_id = 0;
    scan_live(2, PLAN_COMBAT);
    scan_owned(2).weapon_id = 0;
    scan_owned(2).armor_id = 3;     // defense 3 -> 12
    scan_run();
    SCHECK(g_scan_world.best_trade == scan_id(SCAN_OWNER, 1));
    // One more point of armour is worth four and breaks it.
    scan_owned(2).armor_id = 4;
    g_scan_world.armor[4].defense_rating = 4;   // 16
    scan_run();
    SCHECK(g_scan_world.best_trade == scan_id(SCAN_OWNER, 2));

    // ---- chassis speed counts one ------------------------------------------
    scan_reset();
    scan_live(1, PLAN_COMBAT);
    scan_owned(1).weapon_id = 1;    // offense 2 -> 6
    scan_owned(1).armor_id = 0;
    scan_owned(1).chassis_id = 0;   // speed 0
    scan_live(2, PLAN_COMBAT);
    scan_owned(2).weapon_id = 0;
    scan_owned(2).armor_id = 0;
    scan_owned(2).chassis_id = 1;   // speed 4
    scan_run();
    SCHECK(g_scan_world.best_trade == scan_id(SCAN_OWNER, 1));
    g_scan_world.chassis[1].speed = 7;
    scan_run();
    SCHECK(g_scan_world.best_trade == scan_id(SCAN_OWNER, 2));

    // ---- ties go to the lower slot, in both orders -------------------------
    scan_reset();
    scan_live(9, PLAN_COMBAT);
    scan_live(11, PLAN_COMBAT);
    scan_run();
    SCHECK(g_scan_world.best_trade == scan_id(SCAN_OWNER, 9));
    SCHECK(g_scan_world.best_mention == scan_id(SCAN_OWNER, 9));

    // ---- a score of zero never wins ----------------------------------------
    // PLAN_NAVAL_TRANSPORT is exactly seven, so the plan term vanishes and a
    // prototype with no weapon, no armour and no speed scores nothing.
    scan_reset();
    scan_live(5, PLAN_NAVAL_TRANSPORT);
    scan_owned(5).weapon_id = 0;
    scan_owned(5).armor_id = 0;
    scan_owned(5).chassis_id = 0;
    scan_run();
    SCHECK(g_scan_world.best_trade == -1);
    // One point of speed is enough.
    g_scan_world.chassis[0].speed = 1;
    scan_run();
    SCHECK(g_scan_world.best_trade == scan_id(SCAN_OWNER, 5));

    // ---- the plan term is seven minus the plan ------------------------------
    // One plan past PLAN_NAVAL_TRANSPORT the term goes negative, and a whole
    // plan step is far more than any weapon, armour or speed can repay.
    scan_reset();
    scan_live(5, PLAN_COLONIZATION);
    scan_owned(5).weapon_id = 3;
    scan_owned(5).armor_id = 3;
    scan_owned(5).chassis_id = 1;
    scan_run();
    SCHECK(g_scan_world.best_trade == -1);
}
GAMEPLAY_CASE(test_scan_prototypes_selection);

void test_scan_prototypes_domination() {
    ScanSeams seams;

    // A cancelled prototype keeps a sixteenth of its score for the trade
    // answer and loses the mention answer outright, so best_mention going to
    // -1 while best_trade stays put is the signal that a rival cancelled it.
    // ---- a matching rival with a better reactor cancels --------------------
    scan_reset();
    scan_live(5, PLAN_COMBAT);
    scan_live_rival(2, PLAN_COMBAT);
    scan_run();
    SCHECK(g_scan_world.best_trade == scan_id(SCAN_OWNER, 5));
    SCHECK(g_scan_world.best_mention == scan_id(SCAN_OWNER, 5));
    scan_rival(2).reactor_id = 2;
    scan_run();
    SCHECK(g_scan_world.best_trade == scan_id(SCAN_OWNER, 5));
    SCHECK(g_scan_world.best_mention == -1);
    // A worse reactor does not.
    scan_rival(2).reactor_id = 0;
    scan_run();
    SCHECK(g_scan_world.best_mention == scan_id(SCAN_OWNER, 5));

    // ---- and a faster chassis cancels --------------------------------------
    scan_reset();
    scan_live(5, PLAN_COMBAT);
    scan_live_rival(2, PLAN_COMBAT);
    scan_rival(2).chassis_id = 1;   // same triad, speed 4 against speed 0
    scan_run();
    SCHECK(g_scan_world.best_mention == -1);
    // The other way round it does not, even though the speeds still differ.
    scan_reset();
    scan_live(5, PLAN_COMBAT);
    scan_owned(5).chassis_id = 1;
    scan_live_rival(2, PLAN_COMBAT);
    scan_run();
    SCHECK(g_scan_world.best_mention == scan_id(SCAN_OWNER, 5));

    // ---- a rival only counts if it is live, current, same plan, same triad -
    scan_reset();
    scan_live(5, PLAN_COMBAT);
    scan_live_rival(2, PLAN_COMBAT);
    scan_rival(2).reactor_id = 2;   // would cancel
    scan_rival(2).flags = PROTO_ACTIVE;
    scan_run();
    SCHECK(g_scan_world.best_mention == scan_id(SCAN_OWNER, 5));
    scan_rival(2).flags = PROTO_TYPED_COMPLETE;
    scan_run();
    SCHECK(g_scan_world.best_mention == scan_id(SCAN_OWNER, 5));
    scan_rival(2).flags = PROTO_ACTIVE | PROTO_TYPED_COMPLETE;
    scan_run();
    SCHECK(g_scan_world.best_mention == -1);
    // Obsolete to the VIEWER, whose band this is.
    scan_rival(2).obsolete_factions = (uint8_t)(1 << SCAN_VIEWER);
    scan_run();
    SCHECK(g_scan_world.best_mention == scan_id(SCAN_OWNER, 5));
    // Obsolete to the owner instead: that is the other loop's bit and does
    // nothing here.
    scan_rival(2).obsolete_factions = (uint8_t)~(1 << SCAN_VIEWER);
    scan_run();
    SCHECK(g_scan_world.best_mention == -1);
    scan_rival(2).obsolete_factions = 0;
    // A different plan.
    scan_rival(2).plan = PLAN_DEFENSIVE;
    scan_run();
    SCHECK(g_scan_world.best_mention == scan_id(SCAN_OWNER, 5));
    scan_rival(2).plan = PLAN_COMBAT;
    // A different triad at the same speed.
    scan_rival(2).chassis_id = 2;
    scan_run();
    SCHECK(g_scan_world.best_mention == scan_id(SCAN_OWNER, 5));
    scan_rival(2).chassis_id = 0;
    scan_run();
    SCHECK(g_scan_world.best_mention == -1);

    // ---- the weapon-and-armour clause --------------------------------------
    // Rival weapon 2 (offense 3, and Armor[2] answers 2) against owner weapon
    // 1 (offense 2, and Armor[1] answers 2): strictly better on one, equal on
    // the other, so it cancels.
    scan_reset();
    scan_live(5, PLAN_COMBAT);
    scan_live_rival(2, PLAN_COMBAT);
    scan_owned(5).weapon_id = 1;
    scan_rival(2).weapon_id = 2;
    scan_run();
    SCHECK(g_scan_world.best_mention == -1);
    // Equal on both cancels nothing: the clause needs a strict improvement.
    scan_rival(2).weapon_id = 1;
    scan_run();
    SCHECK(g_scan_world.best_mention == scan_id(SCAN_OWNER, 5));
    // Better weapon, worse armour: Armor[4] answers 1 against Armor[1]'s 2.
    scan_rival(2).weapon_id = 4;
    scan_run();
    SCHECK(g_scan_world.best_mention == scan_id(SCAN_OWNER, 5));
    // A slower rival is excused the whole clause even when strictly better.
    scan_reset();
    scan_live(5, PLAN_COMBAT);
    scan_owned(5).chassis_id = 1;   // speed 4
    scan_owned(5).weapon_id = 1;
    scan_live_rival(2, PLAN_COMBAT);
    scan_rival(2).chassis_id = 0;   // speed 0
    scan_rival(2).weapon_id = 2;
    scan_run();
    SCHECK(g_scan_world.best_mention == scan_id(SCAN_OWNER, 5));

    // ---- arm_strat is handed the WEAPON id, both sides ---------------------
    // A rival that is better armoured and identically armed changes nothing,
    // because the armour field never reaches arm_strat.
    scan_reset();
    scan_live(5, PLAN_COMBAT);
    scan_owned(5).weapon_id = 1;
    scan_owned(5).armor_id = 0;     // defense 0
    scan_live_rival(2, PLAN_COMBAT);
    scan_rival(2).weapon_id = 1;
    scan_rival(2).armor_id = 5;     // defense 100, and unreachable from here
    scan_run();
    SCHECK(g_scan_world.best_mention == scan_id(SCAN_OWNER, 5));
    // The mirror image: identical armour ids, and the rival's WEAPON id alone
    // moves the armour half of the comparison.
    scan_reset();
    scan_live(5, PLAN_COMBAT);
    scan_owned(5).weapon_id = 4;    // offense 2, Armor[4] answers 1
    scan_owned(5).armor_id = 1;
    scan_live_rival(2, PLAN_COMBAT);
    scan_rival(2).weapon_id = 1;    // offense 2, Armor[1] answers 2
    scan_rival(2).armor_id = 1;
    scan_run();
    SCHECK(g_scan_world.best_mention == -1);

    // ---- a cancelled prototype keeps a sixteenth of its score --------------
    // Both slots are PLAN_NAVAL_TRANSPORT, so the plan term is zero and the
    // shift works on numbers small enough to see. Slot 1 scores 1 and is not
    // cancelled; slot 11 scores 20 and is, which is 1 after the shift - a tie
    // that the lower slot keeps.
    scan_reset();
    g_scan_world.chassis[0].speed = 1;
    g_scan_world.chassis[1].speed = 20;
    g_scan_world.chassis[1].triad = TRIAD_SEA;   // no rival shares this triad
    scan_live(1, PLAN_NAVAL_TRANSPORT);
    scan_owned(1).weapon_id = 0;
    scan_owned(1).armor_id = 0;
    scan_owned(1).chassis_id = 0;   // speed 1
    scan_live(11, PLAN_NAVAL_TRANSPORT);
    scan_owned(11).weapon_id = 0;
    scan_owned(11).armor_id = 0;
    scan_owned(11).chassis_id = 1;  // speed 20
    scan_live_rival(2, PLAN_NAVAL_TRANSPORT);
    scan_rival(2).chassis_id = 1;   // TRIAD_SEA, speed 20: cancels slot 11 only
    scan_rival(2).reactor_id = 2;
    scan_run();
    SCHECK(g_scan_world.best_trade == scan_id(SCAN_OWNER, 1));
    // Nineteen shifts to 1 as well; twenty-one is where the tie would break if
    // the shift were smaller.
    g_scan_world.chassis[1].speed = 32;
    scan_run();
    SCHECK(g_scan_world.best_trade == scan_id(SCAN_OWNER, 11));
    // And a cancelled prototype scoring less than sixteen is worth nothing.
    scan_reset();
    g_scan_world.chassis[0].speed = 15;
    scan_live(11, PLAN_NAVAL_TRANSPORT);
    scan_owned(11).weapon_id = 0;
    scan_owned(11).armor_id = 0;
    scan_live_rival(2, PLAN_NAVAL_TRANSPORT);
    scan_rival(2).reactor_id = 2;
    scan_run();
    SCHECK(g_scan_world.best_trade == -1);
    g_scan_world.chassis[0].speed = 16;
    scan_run();
    SCHECK(g_scan_world.best_trade == scan_id(SCAN_OWNER, 11));
}
GAMEPLAY_CASE(test_scan_prototypes_domination);

void test_scan_prototypes_mention() {
    ScanSeams seams;

    PlayerData &viewer = g_scan_world.players_data[SCAN_VIEWER];
    PlayerData &owner = g_scan_world.players_data[SCAN_OWNER];

    // ---- the already-mentioned flags, and how a pact changes them ----------
    scan_reset();
    scan_live(5, PLAN_COMBAT);
    scan_owned(5).flags |= PROTO_UNK_10;
    scan_run();
    SCHECK(g_scan_world.best_trade == scan_id(SCAN_OWNER, 5));
    SCHECK(g_scan_world.best_mention == -1);
    // Under a pact only the second flag disqualifies.
    viewer.diplo_treaties[SCAN_OWNER] = DTREATY_PACT;
    scan_run();
    SCHECK(g_scan_world.best_mention == scan_id(SCAN_OWNER, 5));
    scan_owned(5).flags |= PROTO_UNK_20;
    scan_run();
    SCHECK(g_scan_world.best_mention == -1);
    // Without the pact, either flag is enough.
    viewer.diplo_treaties[SCAN_OWNER] = 0;
    scan_owned(5).flags = PROTO_ACTIVE | PROTO_TYPED_COMPLETE | PROTO_UNK_20;
    scan_run();
    SCHECK(g_scan_world.best_mention == -1);
    scan_owned(5).flags = PROTO_ACTIVE | PROTO_TYPED_COMPLETE;
    scan_run();
    SCHECK(g_scan_world.best_mention == scan_id(SCAN_OWNER, 5));

    // ---- the treaty is read one way round only -----------------------------
    scan_reset();
    scan_live(5, PLAN_COMBAT);
    scan_owned(5).flags |= PROTO_UNK_10;
    owner.diplo_treaties[SCAN_VIEWER] = DTREATY_PACT;   // the reverse entry
    scan_run();
    SCHECK(g_scan_world.best_mention == -1);
    owner.diplo_treaties[SCAN_VIEWER] = 0;
    // A different faction's slot in the right row is not it either.
    viewer.diplo_treaties[SCAN_VIEWER] = DTREATY_PACT;
    scan_run();
    SCHECK(g_scan_world.best_mention == -1);
    viewer.diplo_treaties[SCAN_VIEWER] = 0;
    viewer.diplo_treaties[SCAN_OWNER] = DTREATY_PACT;
    scan_run();
    SCHECK(g_scan_world.best_mention == scan_id(SCAN_OWNER, 5));

    // ---- which plans can be shown off --------------------------------------
    // PLAN_RECONNAISANCE is refused in the middle of an otherwise allowed run,
    // and everything past PLAN_NAVAL_SUPERIORITY is refused as well. Each case
    // still takes the trade answer, which is what separates the two gates.
    // The run stops at PLAN_NAVAL_TRANSPORT because one plan further the score
    // itself goes negative and the trade answer disappears for a reason that
    // has nothing to do with this gate.
    for (int plan = PLAN_OFFENSIVE; plan <= PLAN_NAVAL_TRANSPORT; plan++) {
        scan_reset();
        g_scan_world.chassis[0].speed = 3;
        scan_live(5, plan);
        scan_run();
        const bool mentionable = plan <= PLAN_NAVAL_SUPERIORITY
            && plan != PLAN_RECONNAISANCE;
        SCHECK(g_scan_world.best_trade == scan_id(SCAN_OWNER, 5));
        SCHECK(g_scan_world.best_mention
               == (mentionable ? scan_id(SCAN_OWNER, 5) : -1));
    }

    // ---- a cancelled prototype is worth nothing, except in a vendetta ------
    scan_reset();
    scan_live(5, PLAN_DEFENSIVE);
    scan_live_rival(2, PLAN_DEFENSIVE);
    scan_rival(2).reactor_id = 2;
    scan_run();
    SCHECK(g_scan_world.best_trade == scan_id(SCAN_OWNER, 5));
    SCHECK(g_scan_world.best_mention == -1);
    viewer.diplo_treaties[SCAN_OWNER] = DTREATY_VENDETTA;
    scan_run();
    SCHECK(g_scan_world.best_mention == scan_id(SCAN_OWNER, 5));
    // The vendetta only rescues PLAN_DEFENSIVE.
    scan_owned(5).plan = PLAN_COMBAT;
    scan_rival(2).plan = PLAN_COMBAT;
    scan_run();
    SCHECK(g_scan_world.best_mention == -1);
    // And it is the vendetta bit specifically, not any treaty at all.
    scan_owned(5).plan = PLAN_DEFENSIVE;
    scan_rival(2).plan = PLAN_DEFENSIVE;
    viewer.diplo_treaties[SCAN_OWNER] = DTREATY_TREATY | DTREATY_TRUCE;
    scan_run();
    SCHECK(g_scan_world.best_mention == -1);

    // ---- the mention answer scores the full value, not the trade value -----
    // Slot 1 is a cancelled PLAN_DEFENSIVE unit in a vendetta, worth a
    // sixteenth of five plan steps - 20480 - and slot 3 is an intact
    // PLAN_NAVAL_SUPERIORITY unit worth one whole step, 65550. Slot 3 is
    // therefore ahead on both answers, but only just: had the mention
    // comparison reused slot 3's already-shifted trade value of 4096 it would
    // have handed the mention to slot 1 while leaving the trade alone.
    scan_reset();
    viewer.diplo_treaties[SCAN_OWNER] = DTREATY_VENDETTA;
    scan_live(1, PLAN_DEFENSIVE);
    scan_live_rival(2, PLAN_DEFENSIVE);
    scan_rival(2).reactor_id = 2;
    scan_live(3, PLAN_NAVAL_SUPERIORITY);
    scan_run();
    SCHECK(g_scan_world.best_trade == scan_id(SCAN_OWNER, 3));
    SCHECK(g_scan_world.best_mention == scan_id(SCAN_OWNER, 3));
    // Take slot 3 away and the vendetta-rescued slot 1 is what is left.
    scan_owned(3).flags = 0;
    scan_run();
    SCHECK(g_scan_world.best_trade == scan_id(SCAN_OWNER, 1));
    SCHECK(g_scan_world.best_mention == scan_id(SCAN_OWNER, 1));

    // ---- the two answers are independent -----------------------------------
    scan_reset();
    scan_live(1, PLAN_OFFENSIVE);
    scan_owned(1).flags |= PROTO_UNK_10;   // best score, never mentioned
    scan_live(2, PLAN_COMBAT);
    scan_run();
    SCHECK(g_scan_world.best_trade == scan_id(SCAN_OWNER, 1));
    SCHECK(g_scan_world.best_mention == scan_id(SCAN_OWNER, 2));
}
GAMEPLAY_CASE(test_scan_prototypes_mention);

void test_scan_prototypes_bounds() {
    ScanSeams seams;

    // ---- the scanned band is exactly the owner's 64 slots ------------------
    scan_reset();
    scan_live(63, PLAN_COMBAT);
    scan_run();
    SCHECK(g_scan_world.best_trade == scan_id(SCAN_OWNER, 63));
    scan_reset();
    scan_live(0, PLAN_COMBAT);
    scan_run();
    SCHECK(g_scan_world.best_trade == scan_id(SCAN_OWNER, 0));
    // The slots on either side hold PLAN_OFFENSIVE prototypes that would win
    // outright if the loop reached them, and it does not.
    scan_reset();
    scan_live(5, PLAN_COMBAT);
    SCHECK(scan_at(scan_id(SCAN_OWNER, -1)).plan == PLAN_OFFENSIVE);
    SCHECK(scan_at(scan_id(SCAN_OWNER, MaxVehProtoFactionNum)).plan == PLAN_OFFENSIVE);
    scan_run();
    SCHECK(g_scan_world.best_trade == scan_id(SCAN_OWNER, 5));

    // ---- the rival band is exactly the viewer's 64 slots -------------------
    // The one prototype that can cancel slot 5 is placed at the last rival
    // slot, then one past each end of the band.
    scan_reset();
    scan_live(5, PLAN_COMBAT);
    scan_live_rival(63, PLAN_COMBAT);
    scan_rival(63).reactor_id = 2;
    scan_run();
    SCHECK(g_scan_world.best_mention == -1);

    scan_reset();
    scan_live(5, PLAN_COMBAT);
    scan_live_rival(0, PLAN_COMBAT);
    scan_rival(0).reactor_id = 2;
    scan_run();
    SCHECK(g_scan_world.best_mention == -1);

    scan_reset();
    scan_live(5, PLAN_COMBAT);
    {
        VehPrototype &past = scan_at(scan_id(SCAN_VIEWER, MaxVehProtoFactionNum));
        past.flags = PROTO_ACTIVE | PROTO_TYPED_COMPLETE;
        past.plan = PLAN_COMBAT;
        past.chassis_id = 0;
        past.weapon_id = 1;
        past.armor_id = 1;
        past.reactor_id = 2;
    }
    scan_run();
    SCHECK(g_scan_world.best_mention == scan_id(SCAN_OWNER, 5));

    scan_reset();
    scan_live(5, PLAN_COMBAT);
    {
        VehPrototype &before = scan_at(scan_id(SCAN_VIEWER, -1));
        before.flags = PROTO_ACTIVE | PROTO_TYPED_COMPLETE;
        before.plan = PLAN_COMBAT;
        before.chassis_id = 0;
        before.weapon_id = 1;
        before.armor_id = 1;
        before.reactor_id = 2;
    }
    scan_run();
    SCHECK(g_scan_world.best_mention == scan_id(SCAN_OWNER, 5));
}
GAMEPLAY_CASE(test_scan_prototypes_bounds);

#undef SCHECK

/*
 * set_course (0x00564890).
 *
 * The whole observable is the order the unit ends up carrying, so every case
 * here poisons the order first and then asks either "which tile was it sent
 * to" or "was it left alone". The map is the same 16x8 window the patrol
 * fixtures use: tiles are addressed by game coordinates and only tiles whose
 * x and y share a parity are real, exactly as the packed map intends.
 */
struct CourseWorld {
    Map tiles[192];
    Base bases[8];
    Veh vehs[4];
    VehPrototype protos[128];
    Continent continents[MaxContinentNum];
    Map *tiles_ptr;
    uint32_t longitude;
    int lon_bounds;
    int lat_bounds;
    BOOL is_flat;
    int base_count;
    int veh_count;
    int base_find_dist;
};

CourseWorld g_course_world;

const int COURSE_LIVE = 64;
const int COURSE_TYPE = 5;

Map &course_at(int x, int y) {
    return g_course_world.tiles[COURSE_LIVE + (x >> 1) + y * 8];
}

void course_land(int x, int y, int region) {
    course_at(x, y).climate = 0x80;   // one level above sea
    course_at(x, y).region = (uint8_t)region;
}

void course_water(int x, int y, int region) {
    course_at(x, y).climate = 0x40;   // below ALT_BIT_SHORE_LINE
    course_at(x, y).region = (uint8_t)region;
}

void course_reset() {
    std::memset(&g_course_world, 0, sizeof(g_course_world));
    g_course_world.tiles_ptr = &g_course_world.tiles[COURSE_LIVE];
    g_course_world.longitude = 8;
    g_course_world.lon_bounds = 16;
    g_course_world.lat_bounds = 8;
    g_course_world.is_flat = 1;
    // Every tile of the array, live window or not, is dry land of region 2.
    // A candidate one row off the map therefore fails only the bounds test.
    for (int i = 0; i < 192; i++) {
        g_course_world.tiles[i].climate = 0x80;
        g_course_world.tiles[i].region = 2;
    }
    Veh &veh = g_course_world.vehs[0];
    veh.x = 0;
    veh.y = 0;
    veh.faction_id = 1;
    veh.proto_id = 0;
    veh.home_base_id = -1;
    g_course_world.veh_count = 1;
    g_course_world.protos[0].plan = PLAN_COMBAT;
}

void course_run(int x, int y) {
    Veh &veh = g_course_world.vehs[0];
    veh.order = ORDER_HOLD;
    veh.move_to_ai_type = 0x77;
    veh.waypoint_x[0] = -999;
    veh.waypoint_y[0] = -999;
    set_course(0, (char)COURSE_TYPE, x, y);
}

bool course_ordered(int x, int y) {
    const Veh &veh = g_course_world.vehs[0];
    return veh.order == ORDER_MOVE_TO && veh.move_to_ai_type == COURSE_TYPE
        && veh.waypoint_x[0] == x && veh.waypoint_y[0] == y;
}

bool course_idle() {
    return g_course_world.vehs[0].order == ORDER_HOLD;
}

#define CCHECK(cond)                                                          \
    do {                                                                      \
        const bool course_ok = (cond);                                        \
        if (!course_ok) {                                                     \
            std::fprintf(stderr, "set_course: line %d: %s\n", __LINE__,       \
                         #cond);                                              \
        }                                                                     \
        expect(course_ok);                                                    \
    } while (0)

class CourseSeams {
 public:
    CourseSeams()
        : tiles_(&MapTiles, &g_course_world.tiles_ptr),
          longitude_(&MapLongitude, &g_course_world.longitude),
          lon_(&MapLongitudeBounds, &g_course_world.lon_bounds),
          lat_(&MapLatitudeBounds, &g_course_world.lat_bounds),
          flat_(&MapIsFlat, &g_course_world.is_flat),
          continents_(&Continents, g_course_world.continents),
          bases_(&Bases, g_course_world.bases),
          base_count_(&BaseCurrentCount, &g_course_world.base_count),
          base_dist_(&BaseFindDist, &g_course_world.base_find_dist),
          vehs_(&Vehs, g_course_world.vehs),
          veh_count_(&VehCurrentCount, &g_course_world.veh_count),
          protos_(&VehPrototypes, g_course_world.protos) { }

 private:
    ScopedSeam<Map *> tiles_;
    ScopedSeam<uint32_t> longitude_;
    ScopedSeam<int> lon_;
    ScopedSeam<int> lat_;
    ScopedSeam<BOOL> flat_;
    ScopedSeam<Continent> continents_;
    ScopedSeam<Base> bases_;
    ScopedSeam<int> base_count_;
    ScopedSeam<int> base_dist_;
    ScopedSeam<Veh> vehs_;
    ScopedSeam<int> veh_count_;
    ScopedSeam<VehPrototype> protos_;
};

void test_set_course_direct() {
    CourseSeams seams;

    // ---- the direct answer needs the same region AND an afloat unit --------
    // Destination (10,4) is land of the unit's own region 5, so the region
    // halves of the two branches agree and only the unit's own altitude
    // decides. On land it searches and finds the anchorage at (12,4); afloat
    // it hands the destination straight to go_to.
    course_reset();
    course_land(0, 0, 5);
    course_land(10, 4, 5);
    course_water(12, 4, 5);
    course_run(10, 4);
    CCHECK(course_ordered(12, 4));
    course_water(0, 0, 5);
    course_run(10, 4);
    CCHECK(course_ordered(10, 4));
    // ALT_BIT_SHORE_LINE itself is already ashore.
    course_at(0, 0).climate = 0x60;
    course_run(10, 4);
    CCHECK(course_ordered(12, 4));

    // ---- and a differing region searches however afloat the unit is --------
    course_reset();
    course_water(0, 0, 5);
    course_water(12, 4, 5);
    course_run(10, 4);   // (10,4) is land of region 2
    CCHECK(course_ordered(12, 4));

    // ---- nothing qualifies, and the unit is left exactly as it was ---------
    course_reset();
    course_water(0, 0, 5);
    course_run(10, 4);
    CCHECK(course_idle());
    CCHECK(g_course_world.vehs[0].waypoint_x[0] == -999);
    CCHECK(g_course_world.vehs[0].waypoint_y[0] == -999);
}
GAMEPLAY_CASE(test_set_course_direct);

void test_set_course_anchorage() {
    CourseSeams seams;

    // ---- an anchorage must be water, and in the unit's own region ----------
    course_reset();
    course_water(0, 0, 5);
    course_water(12, 4, 7);   // water, but a region the unit cannot be in
    course_run(10, 4);
    CCHECK(course_idle());
    course_water(12, 4, 5);
    course_run(10, 4);
    CCHECK(course_ordered(12, 4));
    course_land(12, 4, 5);    // right region, wrong element
    course_run(10, 4);
    CCHECK(course_idle());

    // ---- ties keep the earlier candidate, and only a strict gain moves it --
    // (12,4) is radius offset 2 and (8,4) is offset 6, and with the map
    // otherwise solid both front eight tiles of region 2.
    course_reset();
    course_water(0, 0, 5);
    course_water(12, 4, 5);
    course_water(8, 4, 5);
    course_run(10, 4);
    CCHECK(course_ordered(12, 4));
    // Take one neighbour away from the earlier candidate and the later one
    // overtakes it.
    course_water(14, 4, 9);   // not region 5, so not itself a candidate
    course_run(10, 4);
    CCHECK(course_ordered(8, 4));
    // Put it back and the tie returns.
    course_land(14, 4, 2);
    course_run(10, 4);
    CCHECK(course_ordered(12, 4));

    // ---- a neighbour must be land, of the DESTINATION's region, unbased ----
    course_reset();
    course_water(0, 0, 5);
    course_water(12, 4, 5);
    course_water(8, 4, 5);
    // Land, but the anchorage's own region rather than the destination's.
    course_land(14, 4, 5);
    course_run(10, 4);
    CCHECK(course_ordered(8, 4));
    course_land(14, 4, 2);
    // A base owned by a faction takes the neighbour out of the count.
    course_at(14, 4).bit |= BIT_BASE_IN_TILE;
    course_at(14, 4).val2 = 1;
    course_run(10, 4);
    CCHECK(course_ordered(8, 4));
    // An unoccupied owner nibble is not a base as far as base_who is
    // concerned, and the neighbour counts again.
    course_at(14, 4).val2 = 0xF;
    course_run(10, 4);
    CCHECK(course_ordered(12, 4));

    // ---- the bar the first candidate has to clear --------------------------
    // (11,1) is radius offset 13 and touches none of the destination's own
    // tile, so with the destination given a region of its own it fronts
    // nothing at all. A land destination refuses a frontage of zero; a water
    // destination accepts it, which is the only way a unit can be sent to open
    // sea.
    course_reset();
    course_water(0, 0, 5);
    course_water(11, 1, 5);
    course_land(10, 4, 9);
    course_run(10, 4);
    CCHECK(course_idle());
    course_water(10, 4, 9);
    course_run(10, 4);
    CCHECK(course_ordered(11, 1));
    // One tile of the destination's region next to the anchorage lifts it over
    // the land destination's bar as well.
    course_land(10, 4, 9);
    course_land(12, 2, 9);   // a neighbour of (11,1)
    course_run(10, 4);
    CCHECK(course_ordered(11, 1));
}
GAMEPLAY_CASE(test_set_course_anchorage);

void test_set_course_base_and_wrap() {
    CourseSeams seams;

    // ---- a unit inside a base asks base_on_sea instead ----------------------
    // The anchorage is region 7, which is not the unit's region 2, so the only
    // thing that can admit it is the base's own coastline.
    course_reset();
    course_land(0, 0, 2);
    course_at(0, 0).bit |= BIT_BASE_IN_TILE;
    g_course_world.bases[0].x = 0;
    g_course_world.bases[0].y = 0;
    g_course_world.bases[0].faction_id_current = 1;
    g_course_world.base_count = 1;
    course_water(12, 4, 7);
    course_run(10, 4);
    CCHECK(course_idle());
    // Give the base a stretch of region 7 water beside it and the same
    // anchorage is accepted.
    course_water(2, 0, 7);
    course_run(10, 4);
    CCHECK(course_ordered(12, 4));
    // A different sea beside the base does not admit a region 7 anchorage.
    course_water(2, 0, 8);
    course_run(10, 4);
    CCHECK(course_idle());
    // Without the base the unit's own region decides again, and region 2 is
    // not region 7.
    course_at(0, 0).bit &= ~(uint32_t)BIT_BASE_IN_TILE;
    g_course_world.base_count = 0;
    course_water(2, 0, 7);
    course_run(10, 4);
    CCHECK(course_idle());

    // ---- x wraps on bit zero of MapIsFlat, not on the whole word -----------
    // Two is the value that separates the two readings: xrange() would call it
    // flat and leave x alone, and this reads bit zero and wraps.
    course_reset();
    g_course_world.is_flat = 2;
    course_water(0, 0, 5);
    course_water(0, 4, 5);
    course_run(14, 4);   // radius offset 2 puts a candidate at x = 16
    CCHECK(course_ordered(0, 4));
    // Genuinely flat, and the same candidate is off the map instead.
    g_course_world.is_flat = 1;
    course_run(14, 4);
    CCHECK(course_idle());

    // ---- the candidate's own bounds ----------------------------------------
    // Radius offset 16 is (1,3), which from (10,4) puts the only anchorage on
    // the last row of the map. Taking that row away is what refuses it, and
    // nothing else about the tile changes.
    course_reset();
    course_water(0, 0, 5);
    course_water(11, 7, 5);
    course_run(10, 4);
    CCHECK(course_ordered(11, 7));
    g_course_world.lat_bounds = 7;
    course_run(10, 4);
    CCHECK(course_idle());

    // Radius offset 14 is (3,-1), three columns east of the destination.
    course_reset();
    course_water(0, 0, 5);
    course_water(13, 3, 5);
    course_run(10, 4);
    CCHECK(course_ordered(13, 3));
    g_course_world.lon_bounds = 13;
    course_run(10, 4);
    CCHECK(course_idle());

    // And the low side. Radius offset 1 is (1,-1): from row zero it is off the
    // top of the map, and from row two it is an ordinary anchorage.
    course_reset();
    course_water(0, 0, 5);
    course_water(11, -1, 5);
    course_run(10, 0);
    CCHECK(course_idle());
    course_water(11, 1, 5);
    course_run(10, 2);
    CCHECK(course_ordered(11, 1));
}
GAMEPLAY_CASE(test_set_course_base_and_wrap);

void test_set_course_edges() {
    CourseSeams seams;

    // ---- ALT_BIT_SHORE_LINE itself is land, on all three readings ----------
    // The destination: at exactly the shore line the bar is zero and a
    // frontage of zero is refused; one step lower it is water, the bar is
    // minus one, and the same anchorage is accepted.
    course_reset();
    course_water(0, 0, 5);
    course_water(11, 1, 5);   // radius offset 13, touching none of (10,4)
    course_land(10, 4, 9);
    course_at(10, 4).climate = 0x60;
    course_run(10, 4);
    CCHECK(course_idle());
    course_at(10, 4).climate = 0x40;
    course_run(10, 4);
    CCHECK(course_ordered(11, 1));

    // The anchorage: at exactly the shore line it is ashore and unusable.
    course_reset();
    course_water(0, 0, 5);
    course_water(12, 4, 5);
    course_at(12, 4).climate = 0x60;
    course_run(10, 4);
    CCHECK(course_idle());
    course_at(12, 4).climate = 0x40;
    course_run(10, 4);
    CCHECK(course_ordered(12, 4));

    // A neighbour: at exactly the shore line it is land and it counts.
    course_reset();
    course_water(0, 0, 5);
    course_water(12, 4, 5);
    course_water(8, 4, 5);
    course_at(14, 4).climate = 0x60;
    course_run(10, 4);
    CCHECK(course_ordered(12, 4));   // eight each, and the earlier one keeps it
    course_at(14, 4).climate = 0x40;
    course_run(10, 4);
    CCHECK(course_ordered(8, 4));    // seven against eight

    // ---- faction zero owns bases too ---------------------------------------
    // base_who answers 0 there, which is still a base. Only the 0xF nibble
    // means nobody.
    course_reset();
    course_water(0, 0, 5);
    course_water(12, 4, 5);
    course_water(8, 4, 5);
    course_at(14, 4).bit |= BIT_BASE_IN_TILE;
    course_at(14, 4).val2 = 0;
    course_run(10, 4);
    CCHECK(course_ordered(8, 4));

    // ---- the first radius offset is the destination's own tile -------------
    // A unit ashore in the destination's region cannot take the direct branch,
    // and the destination itself - offset zero - is the anchorage it finds.
    course_reset();
    course_land(0, 0, 5);
    course_water(10, 4, 5);
    course_run(10, 4);
    CCHECK(course_ordered(10, 4));

    // ---- and the twenty-first is outside the radius ------------------------
    // Radius offset 21 would be (4,0). Four columns east is unreachable; two
    // columns east is offset 2 and is reached.
    course_reset();
    course_water(0, 0, 5);
    course_water(14, 4, 5);
    course_run(10, 4);
    CCHECK(course_idle());
    course_land(14, 4, 2);
    course_water(12, 4, 5);
    course_run(10, 4);
    CCHECK(course_ordered(12, 4));

    // ---- the first neighbour offset counts ---------------------------------
    // Every neighbour of (11,1) except (12,0) - RadiusBase entry zero - is put
    // in a region the destination does not share, so the whole frontage rests
    // on that one tile.
    course_reset();
    course_water(0, 0, 5);
    course_water(11, 1, 5);
    course_land(13, 1, 3);
    course_land(12, 2, 3);
    course_land(11, 3, 3);
    course_land(10, 2, 3);
    course_land(9, 1, 3);
    course_land(10, 0, 3);
    course_run(10, 4);
    CCHECK(course_ordered(11, 1));
    course_land(12, 0, 3);
    course_run(10, 4);
    CCHECK(course_idle());
}
GAMEPLAY_CASE(test_set_course_edges);

#undef CCHECK

/*
 * good_sensor (0x00564EB0).
 *
 * The subject tile is always (8,4) and the faction is always 1. Every tile of
 * the map starts as the faction's own arid land, which is the shape the
 * terrain tail says yes to, so a case only has to say what it changes.
 */
struct SensorWorld {
    Map tiles[192];
    Base bases[8];
    RulesTechnology technology[MaxTechnologyNum];
    uint8_t tech_achieved[MaxTechnologyNum];
    RulesBasic rules;
    PlayerData players_data[8];
    Map *tiles_ptr;
    uint32_t longitude;
    int lon_bounds;
    int lat_bounds;
    BOOL is_flat;
    uint32_t rand_seed;
    uint32_t game_state;
    uint32_t game_rules;
    int base_count;
    int base_find_dist;
};

SensorWorld g_sensor_world;

const int SENSOR_LIVE = 64;
const int SENSOR_FACTION = 1;
const int SENSOR_X = 8;
const int SENSOR_Y = 4;
const int SENSOR_FUNGUS_TECH = 5;

Map &sensor_at(int x, int y) {
    return g_sensor_world.tiles[SENSOR_LIVE + (x >> 1) + y * 8];
}

void sensor_reset() {
    std::memset(&g_sensor_world, 0, sizeof(g_sensor_world));
    g_sensor_world.tiles_ptr = &g_sensor_world.tiles[SENSOR_LIVE];
    g_sensor_world.longitude = 8;
    g_sensor_world.lon_bounds = 16;
    g_sensor_world.lat_bounds = 8;
    g_sensor_world.is_flat = 1;
    g_sensor_world.rand_seed = 0;   // keeps bonus_at off its random path
    g_sensor_world.rules.tech_improve_fungus_sqr = SENSOR_FUNGUS_TECH;
    for (int i = 0; i < 192; i++) {
        // Dry land, one level above sea, owned by the faction under test.
        g_sensor_world.tiles[i].climate = 0x80;
        g_sensor_world.tiles[i].region = 2;
        g_sensor_world.tiles[i].territory = (int8_t)SENSOR_FACTION;
    }
}

// The cheapest of the three reasons to want a sensor: one tile of the inner
// ring belongs to nobody. (10,4) is radius offset 2.
void sensor_give_reason() {
    sensor_at(10, 4).territory = 0;
}

// A base of the faction under test, with the map bit base_at insists on.
void sensor_base(int base_id, int x, int y, int faction_id) {
    g_sensor_world.bases[base_id].x = (int16_t)x;
    g_sensor_world.bases[base_id].y = (int16_t)y;
    g_sensor_world.bases[base_id].faction_id_current = (uint8_t)faction_id;
    sensor_at(x, y).bit |= BIT_BASE_IN_TILE;
    sensor_at(x, y).val2 = (uint8_t)faction_id;
    if (g_sensor_world.base_count <= base_id) {
        g_sensor_world.base_count = base_id + 1;
    }
}

void sensor_grant_fungus_tech() {
    g_sensor_world.tech_achieved[SENSOR_FUNGUS_TECH] = (uint8_t)(1 << SENSOR_FACTION);
}

#define NCHECK(cond)                                                          \
    do {                                                                      \
        const bool sensor_ok = (cond);                                        \
        if (!sensor_ok) {                                                     \
            std::fprintf(stderr, "good_sensor: line %d: %s\n", __LINE__,      \
                         #cond);                                              \
        }                                                                     \
        expect(sensor_ok);                                                    \
    } while (0)

class SensorSeams {
 public:
    SensorSeams()
        : tiles_(&MapTiles, &g_sensor_world.tiles_ptr),
          longitude_(&MapLongitude, &g_sensor_world.longitude),
          lon_(&MapLongitudeBounds, &g_sensor_world.lon_bounds),
          lat_(&MapLatitudeBounds, &g_sensor_world.lat_bounds),
          flat_(&MapIsFlat, &g_sensor_world.is_flat),
          seed_(&MapRandSeed, &g_sensor_world.rand_seed),
          state_(&GameState, &g_sensor_world.game_state),
          rules_bits_(&GameRules, &g_sensor_world.game_rules),
          bases_(&Bases, g_sensor_world.bases),
          base_count_(&BaseCurrentCount, &g_sensor_world.base_count),
          base_dist_(&BaseFindDist, &g_sensor_world.base_find_dist),
          rules_(&Rules, &g_sensor_world.rules),
          players_data_(&PlayersData, g_sensor_world.players_data),
          technology_(&Technology, g_sensor_world.technology),
          achieved_(&GameTechAchieved, g_sensor_world.tech_achieved) { }

 private:
    ScopedSeam<Map *> tiles_;
    ScopedSeam<uint32_t> longitude_;
    ScopedSeam<int> lon_;
    ScopedSeam<int> lat_;
    ScopedSeam<BOOL> flat_;
    ScopedSeam<uint32_t> seed_;
    ScopedSeam<uint32_t> state_;
    ScopedSeam<uint32_t> rules_bits_;
    ScopedSeam<Base> bases_;
    ScopedSeam<int> base_count_;
    ScopedSeam<int> base_dist_;
    ScopedSeam<RulesBasic> rules_;
    ScopedSeam<PlayerData> players_data_;
    ScopedSeam<RulesTechnology> technology_;
    ScopedSeam<uint8_t> achieved_;
};

void test_good_sensor_gates() {
    SensorSeams seams;

    // ---- the tile has to be the faction's own territory --------------------
    sensor_reset();
    sensor_give_reason();
    NCHECK(good_sensor(SENSOR_FACTION, SENSOR_X, SENSOR_Y));
    sensor_at(SENSOR_X, SENSOR_Y).territory = 0;      // unowned
    NCHECK(!good_sensor(SENSOR_FACTION, SENSOR_X, SENSOR_Y));
    sensor_at(SENSOR_X, SENSOR_Y).territory = 2;      // someone else's
    NCHECK(!good_sensor(SENSOR_FACTION, SENSOR_X, SENSOR_Y));
    sensor_at(SENSOR_X, SENSOR_Y).territory = -1;
    NCHECK(!good_sensor(SENSOR_FACTION, SENSOR_X, SENSOR_Y));

    // ---- fungus above the shelf needs the fungus technology ----------------
    sensor_reset();
    sensor_give_reason();
    sensor_at(SENSOR_X, SENSOR_Y).bit |= BIT_FUNGUS;
    sensor_at(SENSOR_X, SENSOR_Y).climate = 0x80;     // altitude well above 0x40
    NCHECK(!good_sensor(SENSOR_FACTION, SENSOR_X, SENSOR_Y));
    sensor_grant_fungus_tech();
    NCHECK(good_sensor(SENSOR_FACTION, SENSOR_X, SENSOR_Y));
    // Below the shelf the technology is not asked for at all.
    sensor_reset();
    sensor_give_reason();
    sensor_at(SENSOR_X, SENSOR_Y).bit |= BIT_FUNGUS;
    sensor_at(SENSOR_X, SENSOR_Y).climate = 0x20;
    NCHECK(good_sensor(SENSOR_FACTION, SENSOR_X, SENSOR_Y));
    sensor_at(SENSOR_X, SENSOR_Y).climate = 0x40;     // exactly the shelf
    NCHECK(!good_sensor(SENSOR_FACTION, SENSOR_X, SENSOR_Y));
    // And without the fungus bit the altitude is irrelevant.
    sensor_at(SENSOR_X, SENSOR_Y).bit = 0;
    NCHECK(good_sensor(SENSOR_FACTION, SENSOR_X, SENSOR_Y));
}
GAMEPLAY_CASE(test_good_sensor_gates);

void test_good_sensor_reasons() {
    SensorSeams seams;

    // ---- with no reason at all the answer is no ----------------------------
    // Every tile is the faction's own and there is no base anywhere.
    sensor_reset();
    NCHECK(!good_sensor(SENSOR_FACTION, SENSOR_X, SENSOR_Y));

    // ---- reason 1: a tile of the inner ring is not ours --------------------
    sensor_reset();
    sensor_at(10, 4).territory = 0;
    NCHECK(good_sensor(SENSOR_FACTION, SENSOR_X, SENSOR_Y));
    // The subject's own tile is radius offset zero and is ours by definition,
    // so it never supplies the reason.
    sensor_reset();
    sensor_at(6, 4).territory = 0;   // radius offset 6
    NCHECK(good_sensor(SENSOR_FACTION, SENSOR_X, SENSOR_Y));
    // Outside the inner nine it does not count.
    sensor_reset();
    sensor_at(10, 2).territory = 0;   // radius offset 9
    NCHECK(!good_sensor(SENSOR_FACTION, SENSOR_X, SENSOR_Y));
    sensor_at(8, 2).territory = 0;    // radius offset 8
    NCHECK(good_sensor(SENSOR_FACTION, SENSOR_X, SENSOR_Y));

    // ---- reasons 2 and 4: our base in the inner ring, and its own cover ----
    sensor_reset();
    sensor_base(0, 10, 4, SENSOR_FACTION);
    NCHECK(good_sensor(SENSOR_FACTION, SENSOR_X, SENSOR_Y));
    // A base of another faction supplies nothing.
    sensor_reset();
    sensor_base(0, 10, 4, 2);
    NCHECK(!good_sensor(SENSOR_FACTION, SENSOR_X, SENSOR_Y));
    // Outside the inner nine it supplies nothing either.
    sensor_reset();
    sensor_base(0, 10, 2, SENSOR_FACTION);
    NCHECK(!good_sensor(SENSOR_FACTION, SENSOR_X, SENSOR_Y));

    // ---- the base's own radius is searched for a sensor --------------------
    // (14,4) is radius offset 21 from the base at (10,4) and six columns from
    // the subject, so it is inside the base's search and outside the subject's.
    sensor_reset();
    sensor_base(0, 10, 4, SENSOR_FACTION);
    sensor_at(14, 4).bit |= BIT_SENSOR_ARRAY;
    NCHECK(good_sensor(SENSOR_FACTION, SENSOR_X, SENSOR_Y));   // reason 2 alone
    // That sensor only counts as cover while it stands on our territory.
    sensor_at(14, 4).territory = 0;
    NCHECK(good_sensor(SENSOR_FACTION, SENSOR_X, SENSOR_Y));

    // ---- a covered base of another faction still supplies nothing ----------
    sensor_reset();
    sensor_base(0, 10, 4, 2);
    sensor_at(14, 4).bit |= BIT_SENSOR_ARRAY;
    NCHECK(!good_sensor(SENSOR_FACTION, SENSOR_X, SENSOR_Y));
}
GAMEPLAY_CASE(test_good_sensor_reasons);

void test_good_sensor_existing() {
    SensorSeams seams;

    // ---- an existing friendly sensor anywhere in the radius refuses --------
    sensor_reset();
    sensor_give_reason();
    NCHECK(good_sensor(SENSOR_FACTION, SENSOR_X, SENSOR_Y));
    sensor_at(SENSOR_X, SENSOR_Y).bit |= BIT_SENSOR_ARRAY;   // offset 0
    NCHECK(!good_sensor(SENSOR_FACTION, SENSOR_X, SENSOR_Y));

    // The search runs the whole 25, not just the nine the reasons use.
    sensor_reset();
    sensor_give_reason();
    sensor_at(12, 4).bit |= BIT_SENSOR_ARRAY;   // radius offset 21
    NCHECK(!good_sensor(SENSOR_FACTION, SENSOR_X, SENSOR_Y));
    sensor_reset();
    sensor_give_reason();
    sensor_at(8, 0).bit |= BIT_SENSOR_ARRAY;    // radius offset 24, the last
    NCHECK(!good_sensor(SENSOR_FACTION, SENSOR_X, SENSOR_Y));

    // A sensor on someone else's territory does not refuse.
    sensor_reset();
    sensor_give_reason();
    sensor_at(12, 4).bit |= BIT_SENSOR_ARRAY;
    sensor_at(12, 4).territory = 2;
    NCHECK(good_sensor(SENSOR_FACTION, SENSOR_X, SENSOR_Y));

    // Neither does one off the map: (8,-4) is radius offset 24 from (8,0).
    sensor_reset();
    sensor_at(10, 0).territory = 0;   // the reason, in the inner ring
    sensor_at(8, -4).bit |= BIT_SENSOR_ARRAY;
    sensor_at(8, -4).territory = (int8_t)SENSOR_FACTION;
    NCHECK(good_sensor(SENSOR_FACTION, 8, 0));

    // Nor one at radius offset 25, which is outside the search. (1,-5) is the
    // first offset past the twenty-five, and from row six it is on the map.
    sensor_reset();
    sensor_at(10, 6).territory = 0;   // the reason
    sensor_at(9, 1).bit |= BIT_SENSOR_ARRAY;
    NCHECK(good_sensor(SENSOR_FACTION, 8, 6));
    // Offset 24 from the same tile is (0,-4), and that one is inside it.
    sensor_at(8, 2).bit |= BIT_SENSOR_ARRAY;
    NCHECK(!good_sensor(SENSOR_FACTION, 8, 6));
}
GAMEPLAY_CASE(test_good_sensor_existing);

void test_good_sensor_terrain() {
    SensorSeams seams;

    // ---- a resource bonus refuses ------------------------------------------
    sensor_reset();
    sensor_give_reason();
    sensor_at(SENSOR_X, SENSOR_Y).bit |= BIT_RSC_BONUS;
    NCHECK(!good_sensor(SENSOR_FACTION, SENSOR_X, SENSOR_Y));

    // ---- a base on the tile itself refuses ---------------------------------
    sensor_reset();
    sensor_give_reason();
    sensor_base(0, SENSOR_X, SENSOR_Y, SENSOR_FACTION);
    NCHECK(!good_sensor(SENSOR_FACTION, SENSOR_X, SENSOR_Y));
    // The 0xF owner nibble is nobody, and base_who lets it through.
    sensor_at(SENSOR_X, SENSOR_Y).val2 = 0xF;
    NCHECK(good_sensor(SENSOR_FACTION, SENSOR_X, SENSOR_Y));
    // Faction zero owns bases too, and base_who answering 0 is still a base.
    // The reason has to come from the border here, because a base belonging to
    // somebody else supplies none.
    sensor_reset();
    sensor_give_reason();
    sensor_base(0, SENSOR_X, SENSOR_Y, 0);
    NCHECK(!good_sensor(SENSOR_FACTION, SENSOR_X, SENSOR_Y));

    // ---- the five improvements that would be displaced ---------------------
    const uint32_t displaced[] = {
        BIT_MONOLITH, BIT_CONDENSER, BIT_THERMAL_BORE, BIT_MINE, BIT_SOLAR_TIDAL,
    };
    for (int i = 0; i < 5; i++) {
        sensor_reset();
        sensor_give_reason();
        NCHECK(good_sensor(SENSOR_FACTION, SENSOR_X, SENSOR_Y));
        sensor_at(SENSOR_X, SENSOR_Y).bit |= displaced[i];
        NCHECK(!good_sensor(SENSOR_FACTION, SENSOR_X, SENSOR_Y));
    }
    // A neighbouring improvement is not this tile's problem.
    sensor_reset();
    sensor_give_reason();
    sensor_at(10, 4).bit |= BIT_MONOLITH;
    NCHECK(good_sensor(SENSOR_FACTION, SENSOR_X, SENSOR_Y));

    // ---- rocky says yes even when the terrain is otherwise good ------------
    sensor_reset();
    sensor_give_reason();
    sensor_at(SENSOR_X, SENSOR_Y).climate = (uint8_t)(0x80 | RAINFALL_RAINY);
    NCHECK(!good_sensor(SENSOR_FACTION, SENSOR_X, SENSOR_Y));   // moist, no fungus
    sensor_at(SENSOR_X, SENSOR_Y).val3 = 0x80;                  // rocky
    NCHECK(good_sensor(SENSOR_FACTION, SENSOR_X, SENSOR_Y));
    sensor_at(SENSOR_X, SENSOR_Y).val3 = TERRAIN_BIT_ROLLING;   // not rocky enough
    NCHECK(!good_sensor(SENSOR_FACTION, SENSOR_X, SENSOR_Y));
    sensor_at(SENSOR_X, SENSOR_Y).val3 = 0xC0;
    NCHECK(good_sensor(SENSOR_FACTION, SENSOR_X, SENSOR_Y));
    // The other six bits of the field are not part of the answer.
    sensor_at(SENSOR_X, SENSOR_Y).val3 = 0x3F;
    NCHECK(!good_sensor(SENSOR_FACTION, SENSOR_X, SENSOR_Y));

    // ---- arid says yes, and either rainfall bit takes that away ------------
    sensor_reset();
    sensor_give_reason();
    NCHECK(good_sensor(SENSOR_FACTION, SENSOR_X, SENSOR_Y));    // 0x80, arid
    sensor_at(SENSOR_X, SENSOR_Y).climate = (uint8_t)(0x80 | RAINFALL_MOIST);
    NCHECK(!good_sensor(SENSOR_FACTION, SENSOR_X, SENSOR_Y));
    sensor_at(SENSOR_X, SENSOR_Y).climate = (uint8_t)(0x80 | RAINFALL_RAINY);
    NCHECK(!good_sensor(SENSOR_FACTION, SENSOR_X, SENSOR_Y));

    // ---- and fungus rescues a moist tile, above the shelf only -------------
    sensor_reset();
    sensor_give_reason();
    sensor_grant_fungus_tech();
    sensor_at(SENSOR_X, SENSOR_Y).climate = (uint8_t)(0x80 | RAINFALL_MOIST);
    sensor_at(SENSOR_X, SENSOR_Y).bit |= BIT_FUNGUS;
    NCHECK(good_sensor(SENSOR_FACTION, SENSOR_X, SENSOR_Y));
    sensor_at(SENSOR_X, SENSOR_Y).climate = (uint8_t)(0x20 | RAINFALL_MOIST);
    NCHECK(!good_sensor(SENSOR_FACTION, SENSOR_X, SENSOR_Y));
    sensor_at(SENSOR_X, SENSOR_Y).climate = (uint8_t)(0x40 | RAINFALL_MOIST);
    NCHECK(good_sensor(SENSOR_FACTION, SENSOR_X, SENSOR_Y));
}
GAMEPLAY_CASE(test_good_sensor_terrain);

#undef NCHECK

/*
 * alt_get_ocean_detail (0x00462190).
 *
 * The map is 16 columns by 8 rows, which is 8 tiles per row once the x
 * coordinate is halved, and the subject tile is (8,4) - far enough from every
 * edge that a case has to opt in to the wrap. The eight neighbours of (8,4) in
 * RadiusBase order are listed below, because every direction assertion here is
 * really a claim about which of those eight was read.
 *
 * The live 8x8 window sits at tiles[64], so tiles[0..63] and tiles[128..191]
 * are poison: land at contour 200, which clamps to 79. Neither value can
 * appear legitimately, tiles[128] is exactly one past the last valid index,
 * and any bounds check that stops holding therefore changes an answer instead
 * of reading a convenient zero.
 */
struct OceanWorld {
    Map tiles[192];
    Map *tiles_ptr;
    uint32_t longitude;
    int lon_bounds;
    int lat_bounds;
    BOOL is_flat;
};

OceanWorld g_ocean_world;

const int OCEAN_LIVE = 64;

// The eight neighbours of (8,4), in RadiusBaseX/RadiusBaseY order.
const int OCEAN_NB_X[8] = { 9, 10, 9, 8, 7, 6, 7, 8 };
const int OCEAN_NB_Y[8] = { 3,  4, 5, 6, 5, 4, 3, 2 };

Map &ocean_at(int x, int y) {
    return g_ocean_world.tiles[OCEAN_LIVE + (x >> 1) + y * 8];
}

void ocean_reset() {
    std::memset(&g_ocean_world, 0, sizeof(g_ocean_world));
    for (int i = 0; i < 192; i++) {
        g_ocean_world.tiles[i].contour = 200;
        g_ocean_world.tiles[i].climate = ALT_BIT_3_LEVELS_ABOVE_SEA;
    }
    for (int i = OCEAN_LIVE; i < OCEAN_LIVE + 64; i++) {
        g_ocean_world.tiles[i].contour = 4;
        g_ocean_world.tiles[i].climate = ALT_BIT_OCEAN_TRENCH;
    }
    g_ocean_world.tiles_ptr = &g_ocean_world.tiles[OCEAN_LIVE];
    g_ocean_world.longitude = 8;
    g_ocean_world.lon_bounds = 16;
    g_ocean_world.lat_bounds = 8;
    g_ocean_world.is_flat = 1;
}

// Every live tile above the shore line, for the branch that looks for water.
void ocean_reset_land() {
    ocean_reset();
    for (int i = OCEAN_LIVE; i < OCEAN_LIVE + 64; i++) {
        g_ocean_world.tiles[i].climate = ALT_BIT_1_LEVEL_ABOVE_SEA;
    }
}

void ocean_land(int x, int y) {
    ocean_at(x, y).climate = ALT_BIT_1_LEVEL_ABOVE_SEA;
}

void ocean_water(int x, int y) {
    ocean_at(x, y).climate = ALT_BIT_OCEAN_TRENCH;
}

#define OCHECK(expr, want)                                                    \
    do {                                                                      \
        const int ocean_got = (expr);                                         \
        const int ocean_want = (want);                                        \
        if (ocean_got != ocean_want) {                                        \
            std::fprintf(stderr,                                              \
                         "alt_get_ocean_detail: line %d: %s = %d, want %d\n", \
                         __LINE__, #expr, ocean_got, ocean_want);             \
        }                                                                     \
        expect(ocean_got == ocean_want);                                      \
    } while (0)

class OceanSeams {
 public:
    OceanSeams()
        : tiles_(&MapTiles, &g_ocean_world.tiles_ptr),
          longitude_(&MapLongitude, &g_ocean_world.longitude),
          lon_(&MapLongitudeBounds, &g_ocean_world.lon_bounds),
          lat_(&MapLatitudeBounds, &g_ocean_world.lat_bounds),
          flat_(&MapIsFlat, &g_ocean_world.is_flat) { }

 private:
    ScopedSeam<Map *> tiles_;
    ScopedSeam<uint32_t> longitude_;
    ScopedSeam<int> lon_;
    ScopedSeam<int> lat_;
    ScopedSeam<BOOL> flat_;
};

void test_alt_ocean_bounds_and_centre() {
    OceanSeams seams;

    // ---- the four arms of the entry guard, in the order it tests them -----
    ocean_reset();
    ocean_at(8, 4).contour = 50;
    OCHECK(alt_get_ocean_detail(8, 4, 0, 0), 50);
    OCHECK(alt_get_ocean_detail(8, -1, 0, 0), 0);
    OCHECK(alt_get_ocean_detail(8, 8, 0, 0), 0);
    OCHECK(alt_get_ocean_detail(-1, 4, 0, 0), 0);
    OCHECK(alt_get_ocean_detail(16, 4, 0, 0), 0);
    // The last tile inside each bound still answers, so the guard is refusing
    // for the right reason rather than refusing everything.
    ocean_at(15, 7).contour = 33;
    OCHECK(alt_get_ocean_detail(15, 7, 0, 0), 33);
    OCHECK(alt_get_ocean_detail(0, 0, 0, 0), 4);

    // ---- point 0 is the tile's own contour, clamped only at the top -------
    ocean_at(8, 4).contour = 79;
    OCHECK(alt_get_ocean_detail(8, 4, 0, 0), 79);
    ocean_at(8, 4).contour = 80;
    OCHECK(alt_get_ocean_detail(8, 4, 0, 0), 79);
    ocean_at(8, 4).contour = 200;
    OCHECK(alt_get_ocean_detail(8, 4, 0, 0), 79);
    ocean_at(8, 4).contour = 0;
    OCHECK(alt_get_ocean_detail(8, 4, 0, 0), 0);

    // ---- and no vertex and no coast can change it ------------------------
    ocean_reset();
    ocean_at(8, 4).contour = 50;
    for (int i = 0; i < 8; i++) {
        ocean_land(OCEAN_NB_X[i], OCEAN_NB_Y[i]);
    }
    for (int vertex = 0; vertex < 4; vertex++) {
        OCHECK(alt_get_ocean_detail(8, 4, vertex, 0), 50);
    }
    // The same with the subject on land and every neighbour water, which is
    // the other side of the branch the scan takes.
    ocean_reset_land();
    ocean_at(8, 4).contour = 50;
    for (int i = 0; i < 8; i++) {
        ocean_water(OCEAN_NB_X[i], OCEAN_NB_Y[i]);
    }
    for (int vertex = 0; vertex < 4; vertex++) {
        OCHECK(alt_get_ocean_detail(8, 4, vertex, 0), 50);
    }
}
GAMEPLAY_CASE(test_alt_ocean_bounds_and_centre);

void test_alt_ocean_edge_midpoints() {
    OceanSeams seams;

    // Which neighbour each (vertex, point) pair averages against. Getting any
    // one of these wrong is the failure this table exists to catch.
    struct EdgeCase {
        int vertex;
        int point;
        int x;
        int y;
    };
    static const EdgeCase edges[] = {
        { 0, 1, 7, 5 }, { 1, 1, 7, 3 }, { 2, 1, 9, 3 }, { 3, 1, 9, 5 },
        { 0, 3, 7, 3 }, { 1, 3, 9, 3 }, { 2, 3, 9, 5 }, { 3, 3, 7, 5 },
    };
    for (int c = 0; c < 8; c++) {
        const EdgeCase &e = edges[c];
        ocean_reset();
        // Every neighbour reads zero except the one that should be used, so a
        // misdirected read halves the subject alone and shows up as 20.
        for (int i = 0; i < 8; i++) {
            ocean_at(OCEAN_NB_X[i], OCEAN_NB_Y[i]).contour = 0;
        }
        ocean_at(8, 4).contour = 40;
        ocean_at(e.x, e.y).contour = 20;
        OCHECK(alt_get_ocean_detail(8, 4, e.vertex, e.point), 30);
    }

    // ---- both terms are clamped before the mean --------------------------
    ocean_reset();
    ocean_at(8, 4).contour = 40;
    ocean_at(7, 5).contour = 200;
    OCHECK(alt_get_ocean_detail(8, 4, 0, 1), 59);
    ocean_reset();
    ocean_at(8, 4).contour = 200;
    ocean_at(7, 5).contour = 0;
    OCHECK(alt_get_ocean_detail(8, 4, 0, 1), 39);

    // ---- the mean truncates rather than rounds ---------------------------
    ocean_reset();
    ocean_at(8, 4).contour = 41;
    ocean_at(7, 5).contour = 20;
    OCHECK(alt_get_ocean_detail(8, 4, 0, 1), 30);

    // ---- an off-map neighbour is dropped, and the subject is still halved -
    ocean_reset();
    ocean_at(8, 0).contour = 41;
    OCHECK(alt_get_ocean_detail(8, 0, 1, 1), 20);
    ocean_at(8, 0).contour = 200;
    OCHECK(alt_get_ocean_detail(8, 0, 1, 1), 39);

    // ---- the round map wraps in both directions --------------------------
    ocean_reset();
    g_ocean_world.is_flat = 0;
    ocean_at(0, 0).contour = 30;
    ocean_at(15, 1).contour = 50;   // (0,0) + (-1,+1) wraps to (15,1)
    OCHECK(alt_get_ocean_detail(0, 0, 0, 1), 40);
    g_ocean_world.is_flat = 1;
    OCHECK(alt_get_ocean_detail(0, 0, 0, 1), 15);

    ocean_reset();
    g_ocean_world.is_flat = 0;
    ocean_at(15, 3).contour = 20;
    ocean_at(0, 2).contour = 60;    // (15,3) + (+1,-1) wraps to (0,2)
    OCHECK(alt_get_ocean_detail(15, 3, 2, 1), 40);
    g_ocean_world.is_flat = 1;
    OCHECK(alt_get_ocean_detail(15, 3, 2, 1), 10);
}
GAMEPLAY_CASE(test_alt_ocean_edge_midpoints);

void test_alt_ocean_corner() {
    OceanSeams seams;

    // ---- each vertex averages its own four tiles -------------------------
    ocean_reset();
    ocean_at(8, 4).contour = 32;
    ocean_at(9, 3).contour = 8;
    ocean_at(10, 4).contour = 12;
    ocean_at(9, 5).contour = 16;
    ocean_at(8, 6).contour = 20;
    ocean_at(7, 5).contour = 24;
    ocean_at(6, 4).contour = 28;
    ocean_at(7, 3).contour = 36;
    ocean_at(8, 2).contour = 40;
    OCHECK(alt_get_ocean_detail(8, 4, 0, 2), 30);   // (6,4) (7,3) (8,4) (7,5)
    OCHECK(alt_get_ocean_detail(8, 4, 1, 2), 29);   // (7,3) (8,2) (9,3) (8,4)
    OCHECK(alt_get_ocean_detail(8, 4, 2, 2), 17);   // (8,4) (9,3) (10,4) (9,5)
    OCHECK(alt_get_ocean_detail(8, 4, 3, 2), 23);   // (7,5) (8,4) (9,5) (8,6)

    // ---- every term is clamped, and the mean truncates -------------------
    ocean_reset();
    ocean_at(6, 4).contour = 200;
    ocean_at(7, 3).contour = 200;
    ocean_at(8, 4).contour = 200;
    ocean_at(7, 5).contour = 200;
    OCHECK(alt_get_ocean_detail(8, 4, 0, 2), 79);   // 4 * 79 >> 2
    // One clamped term among three zeros, so the clamp is visible through the
    // mean instead of being masked by the clamp on the return.
    ocean_at(6, 4).contour = 0;
    ocean_at(7, 3).contour = 0;
    ocean_at(8, 4).contour = 0;
    ocean_at(7, 5).contour = 200;
    OCHECK(alt_get_ocean_detail(8, 4, 0, 2), 19);   // 79 >> 2, not 200 >> 2
    ocean_at(6, 4).contour = 10;
    ocean_at(7, 3).contour = 10;
    ocean_at(8, 4).contour = 10;
    ocean_at(7, 5).contour = 11;
    OCHECK(alt_get_ocean_detail(8, 4, 0, 2), 10);   // 41 >> 2
    // A sum three short of a multiple of four, so the accumulator starting
    // anywhere but zero shows.
    ocean_at(7, 5).contour = 13;
    OCHECK(alt_get_ocean_detail(8, 4, 0, 2), 10);   // 43 >> 2, and 44 >> 2 is 11

    // ---- the relief bias comes from [vertex][1] and [vertex][3] only -----
    ocean_reset();
    ocean_at(6, 4).contour = 10;
    ocean_at(7, 3).contour = 20;
    ocean_at(8, 4).contour = 30;
    ocean_at(7, 5).contour = 40;
    OCHECK(alt_get_ocean_detail(8, 4, 0, 2), 25);
    ocean_land(7, 3);
    OCHECK(alt_get_ocean_detail(8, 4, 0, 2), 32);   // +7
    ocean_land(7, 5);
    OCHECK(alt_get_ocean_detail(8, 4, 0, 2), 29);   // +7 -3
    ocean_water(7, 3);
    OCHECK(alt_get_ocean_detail(8, 4, 0, 2), 22);   // -3
    ocean_water(7, 5);
    // The other two tiles of the corner carry no bias at all.
    ocean_land(6, 4);
    ocean_land(8, 4);
    OCHECK(alt_get_ocean_detail(8, 4, 0, 2), 25);

    // ---- the altitude test is the top three bits, and takes 0x60 exactly --
    ocean_reset();
    ocean_at(6, 4).contour = 10;
    ocean_at(7, 3).contour = 20;
    ocean_at(8, 4).contour = 30;
    ocean_at(7, 5).contour = 40;
    ocean_at(7, 3).climate = 0x5F;                  // masks to 0x40
    OCHECK(alt_get_ocean_detail(8, 4, 0, 2), 25);
    ocean_at(7, 3).climate = 0x7F;                  // masks to ALT_BIT_SHORE_LINE
    OCHECK(alt_get_ocean_detail(8, 4, 0, 2), 32);
    ocean_at(7, 3).climate = ALT_BIT_OCEAN_SHELF;   // one step below
    OCHECK(alt_get_ocean_detail(8, 4, 0, 2), 25);
    ocean_at(7, 3).climate = ALT_BIT_SHORE_LINE;
    OCHECK(alt_get_ocean_detail(8, 4, 0, 2), 32);
    // The tile below the corner takes the same boundary for its -3.
    ocean_at(7, 3).climate = ALT_BIT_OCEAN_TRENCH;
    ocean_at(7, 5).climate = ALT_BIT_OCEAN_SHELF;
    OCHECK(alt_get_ocean_detail(8, 4, 0, 2), 25);
    ocean_at(7, 5).climate = ALT_BIT_SHORE_LINE;
    OCHECK(alt_get_ocean_detail(8, 4, 0, 2), 22);

    // ---- corner 1, where the two shaded tiles do not alias in x ----------
    // At corner 0 from (8,4) the column-0 and column-1 tiles both halve to
    // the same index, so a bias reading the wrong column is invisible. At
    // corner 1 they do not: [1][1] is (8,2) and [1][0] would be (7,2),
    // [1][3] is (8,4) and [1][0] would be (7,4).
    ocean_reset();
    ocean_at(7, 3).contour = 10;
    ocean_at(8, 2).contour = 20;
    ocean_at(9, 3).contour = 30;
    ocean_at(8, 4).contour = 40;
    OCHECK(alt_get_ocean_detail(8, 4, 1, 2), 25);
    ocean_land(8, 2);                               // the tile above corner 1
    OCHECK(alt_get_ocean_detail(8, 4, 1, 2), 32);
    ocean_land(8, 4);                               // the tile below it
    OCHECK(alt_get_ocean_detail(8, 4, 1, 2), 29);
    ocean_water(8, 2);
    OCHECK(alt_get_ocean_detail(8, 4, 1, 2), 22);

    // ---- the bias can take the mean outside 0..79, and it is clamped -----
    ocean_reset();
    ocean_at(6, 4).contour = 79;
    ocean_at(7, 3).contour = 79;
    ocean_at(8, 4).contour = 79;
    ocean_at(7, 5).contour = 79;
    OCHECK(alt_get_ocean_detail(8, 4, 0, 2), 79);
    ocean_land(7, 3);
    OCHECK(alt_get_ocean_detail(8, 4, 0, 2), 79);   // 86 clamped
    ocean_reset();
    ocean_at(6, 4).contour = 12;
    ocean_at(7, 3).contour = 12;
    ocean_at(8, 4).contour = 12;
    ocean_at(7, 5).contour = 12;
    OCHECK(alt_get_ocean_detail(8, 4, 0, 2), 12);
    ocean_land(7, 5);
    OCHECK(alt_get_ocean_detail(8, 4, 0, 2), 9);    // -3 is visible
    ocean_at(6, 4).contour = 0;
    ocean_at(7, 3).contour = 0;
    ocean_at(8, 4).contour = 0;
    ocean_at(7, 5).contour = 0;
    OCHECK(alt_get_ocean_detail(8, 4, 0, 2), 0);    // -3 clamped up

    // ---- off-map corner tiles are skipped, and so are their biases -------
    ocean_reset();
    ocean_at(0, 0).contour = 40;
    OCHECK(alt_get_ocean_detail(0, 0, 0, 2), 10);   // one term of four
    ocean_land(15, 1);
    OCHECK(alt_get_ocean_detail(0, 0, 0, 2), 10);   // still off the flat map

    // ---- the round map wraps the corner sample too -----------------------
    ocean_reset();
    g_ocean_world.is_flat = 0;
    ocean_at(14, 0).contour = 8;    // (0,0) + (-2, 0)
    ocean_at(0, 0).contour = 12;
    ocean_at(15, 1).contour = 20;   // (0,0) + (-1,+1)
    OCHECK(alt_get_ocean_detail(0, 0, 0, 2), 10);   // (15,-1) is still off it
    ocean_land(15, 1);
    OCHECK(alt_get_ocean_detail(0, 0, 0, 2), 7);    // and now it biases

    // ---- an even point that is not 2 takes the mean without the bias -----
    ocean_reset();
    ocean_at(6, 4).contour = 10;
    ocean_at(7, 3).contour = 20;
    ocean_at(8, 4).contour = 30;
    ocean_at(7, 5).contour = 40;
    ocean_land(7, 3);
    OCHECK(alt_get_ocean_detail(8, 4, 0, 2), 32);
    OCHECK(alt_get_ocean_detail(8, 4, 0, 4), 25);
}
GAMEPLAY_CASE(test_alt_ocean_corner);

void test_alt_ocean_shoreline() {
    OceanSeams seams;

    struct CoastCase {
        int vertex;
        int point;
        int x;
        int y;
        int water_side;
        int land_side;
    };
    // The tile each (vertex, point) pair scans, and the two fixed details it
    // answers with - one for a water tile finding land, one for a land tile
    // finding water. The two are always the other one of 57 and 67.
    static const CoastCase coast[] = {
        { 0, 1, 7, 5, 57, 67 },
        { 1, 1, 7, 3, 67, 57 },
        { 2, 1, 9, 3, 67, 57 },
        { 3, 1, 9, 5, 57, 67 },
        { 0, 3, 7, 3, 67, 57 },
        { 1, 3, 9, 3, 67, 57 },
        { 2, 3, 9, 5, 57, 67 },
        { 3, 3, 7, 5, 57, 67 },
    };
    for (int c = 0; c < 8; c++) {
        const CoastCase &k = coast[c];
        ocean_reset();
        ocean_land(k.x, k.y);
        OCHECK(alt_get_ocean_detail(8, 4, k.vertex, k.point), k.water_side);
        ocean_reset_land();
        ocean_water(k.x, k.y);
        OCHECK(alt_get_ocean_detail(8, 4, k.vertex, k.point), k.land_side);
    }

    // ---- the middle step of the scan can never answer --------------------
    ocean_reset();
    ocean_at(8, 4).contour = 40;
    ocean_at(7, 5).contour = 20;
    ocean_at(7, 3).contour = 60;
    ocean_land(6, 4);                 // vertex 0, scan step 1
    OCHECK(alt_get_ocean_detail(8, 4, 0, 1), 30);
    OCHECK(alt_get_ocean_detail(8, 4, 0, 3), 50);
    // ---- and neither can the step the other point owns -------------------
    ocean_reset();
    ocean_at(8, 4).contour = 40;
    ocean_at(7, 5).contour = 20;
    ocean_land(7, 3);                 // vertex 0, scan step 2, which is point 3's
    OCHECK(alt_get_ocean_detail(8, 4, 0, 1), 30);
    ocean_reset();
    ocean_at(8, 4).contour = 40;
    ocean_at(7, 3).contour = 60;
    ocean_land(7, 5);                 // vertex 0, scan step 0, which is point 1's
    OCHECK(alt_get_ocean_detail(8, 4, 0, 3), 50);

    // ---- the corner never enters the scan --------------------------------
    ocean_reset();
    ocean_at(6, 4).contour = 10;
    ocean_at(7, 3).contour = 20;
    ocean_at(8, 4).contour = 30;
    ocean_at(7, 5).contour = 40;
    ocean_land(6, 4);
    OCHECK(alt_get_ocean_detail(8, 4, 0, 2), 25);

    // ---- the coast tile itself is tested at ALT_BIT_SHORE_LINE exactly ---
    ocean_reset();
    ocean_at(7, 5).climate = ALT_BIT_OCEAN_SHELF;   // one step below, no coast
    ocean_at(8, 4).contour = 40;
    ocean_at(7, 5).contour = 20;
    OCHECK(alt_get_ocean_detail(8, 4, 0, 1), 30);
    ocean_at(7, 5).climate = ALT_BIT_SHORE_LINE;    // at it, and it is coast
    OCHECK(alt_get_ocean_detail(8, 4, 0, 1), 57);
    // And the land branch draws the same line: a neighbour exactly at the
    // shore line is still land, so it is not the water this branch wants.
    ocean_reset_land();
    ocean_at(7, 5).climate = ALT_BIT_SHORE_LINE;
    OCHECK(alt_get_ocean_detail(8, 4, 0, 1), 4);
    ocean_at(7, 5).climate = ALT_BIT_OCEAN_SHELF;   // now it is water
    OCHECK(alt_get_ocean_detail(8, 4, 0, 1), 67);

    // ---- the subject's own altitude picks the branch, at 0x60 exactly ----
    ocean_reset();
    ocean_land(7, 5);
    ocean_at(8, 4).climate = ALT_BIT_OCEAN_SHELF;   // 0x40, still water
    OCHECK(alt_get_ocean_detail(8, 4, 0, 1), 57);
    ocean_at(8, 4).climate = ALT_BIT_SHORE_LINE;    // 0x60, now land
    // The land branch wants water at step 0 and finds land, so it falls
    // through to the mean of two tiles that both read 4.
    OCHECK(alt_get_ocean_detail(8, 4, 0, 1), 4);

    // ---- a vertex outside 0..3 answers 60, and the compare is unsigned ---
    ocean_reset();
    ocean_land(7, 5);                 // vertex 4 scans direction 4, the same tile
    OCHECK(alt_get_ocean_detail(8, 4, 4, 1), 60);
    ocean_reset();
    ocean_land(9, 5);                 // vertex -1 scans direction 2
    OCHECK(alt_get_ocean_detail(8, 4, -1, 1), 60);

    // ---- the scan wraps on a round map -----------------------------------
    ocean_reset();
    g_ocean_world.is_flat = 0;
    ocean_land(15, 1);                // (0,0) + (-1,+1) wraps to (15,1)
    OCHECK(alt_get_ocean_detail(0, 0, 0, 1), 57);
    g_ocean_world.is_flat = 1;
    OCHECK(alt_get_ocean_detail(0, 0, 0, 1), 2);   // 4 >> 1, no neighbour at all
}
GAMEPLAY_CASE(test_alt_ocean_shoreline);

#undef OCHECK

// =====================================================================================

/*
 * compute_odds (0x00565F20).
 *
 * The whole world is rebindable through ScopedSeam, so the fixed 0x00952828 /
 * 0x009AB868 / 0x0096C9E0 addresses are never touched.
 *
 * TWO THINGS ABOUT THE SHAPE OF THIS FIXTURE ARE LOAD-BEARING.
 *
 * The Veh array carries an entry BELOW index zero. veh_at() answers -1 for a
 * tile with no unit and the original then reads Vehs[-1].faction_id without
 * checking, so `Vehs` is seamed at vehs[1] and vehs[0] is where that read
 * lands. Without the extra slot the read would go off the front of the struct
 * and the bug could not be exercised at all, let alone poisoned.
 *
 * Nothing is memset to a usable value and left there. Every table is POISONED
 * first and then only the entries under test are written, because a zero
 * fixture cannot tell `region_base_plan[region]` from `region_base_plan[0]`,
 * nor `PlayersData[1]` from `PlayersData[0]`. Rows 0 and 2 of PlayersData exist
 * only to be poisoned, so a wrong 8396-byte stride reads a wrong answer instead
 * of a plausible zero; prototype 128 exists only to be poisoned, so the
 * drop-pod scan's `i < MaxVehProtoFactionNum` bound is a real bound; and the
 * Veh, Map and VehPrototype guards one past the last valid index are asserted
 * unchanged by the purity check at the end.
 *
 * compute_odds writes nothing, anywhere. The last section proves that with a
 * whole-fixture memcmp rather than by spot-checking fields.
 */

const int ODDS_VEH_COUNT = 12;
const int ODDS_PROTO_COUNT = 129;   // 0..127 live, 128 is the guard one past faction 1
const int ODDS_TILE_COUNT = 65;     // 0..63 live, 64 is the guard
const int ODDS_LONGITUDE = 8;
const int ODDS_LON_BOUNDS = 16;
const int ODDS_LAT_BOUNDS = 8;

const int ODDS_FACTION = 1;         // the attacking faction; PlayersData[0] and [2] are poison
const int ODDS_ENEMY = 2;           // the defender's faction
const int ODDS_STRANGER = 3;        // a third party, never ours

const int ODDS_DEF_X = 8;
const int ODDS_DEF_Y = 4;

const int16_t ODDS_POISON16 = 0x7777;
const uint8_t ODDS_POISON8 = 0x77;

// Unit ids.
const int VEH_ATK = 1;              // never placed on the map; only its prototype is read
const int VEH_DEF = 2;
const int VEH_STACK_A = 3;
const int VEH_STACK_B = 4;
const int VEH_NEIGH_A = 5;
const int VEH_NEIGH_B = 6;
const int VEH_NEIGH_C = 7;
const int VEH_NEIGH_D = 8;
const int VEH_NEIGH_E = 9;

// Chassis ids.
const uint8_t CHS_LAND = 0;         // TRIAD_LAND, not a missile
const uint8_t CHS_SEA = 1;          // TRIAD_SEA,  not a missile
const uint8_t CHS_AIR = 2;          // TRIAD_AIR,  not a missile
const uint8_t CHS_MSL = 3;          // TRIAD_AIR,  missile
const int ODDS_CHASSIS_COUNT = 5;   // [4] is the guard

// Weapon ids.
const uint8_t WPN_UNARMED = 0;      // offense_rating 0, so get_offense_rating() answers 0
const uint8_t WPN_ARMED = 1;        // offense_rating 4
const int ODDS_WEAPON_COUNT = 3;    // [2] is the guard

// Prototype ids. 0..63 belong to faction 0, 64..127 to faction 1, 128 is the guard.
const int P_DEF = 2;                // cost 20, PLAN_COMBAT, armed, land
const int P_STACK = 3;              // cost 12, PLAN_COMBAT, armed, land
const int P_ARTIFACT = 4;           // cost  5, PLAN_ALIEN_ARTIFACT, UNARMED
const int P_TERRA = 5;              // cost  5, PLAN_TERRAFORMING, UNARMED
const int P_TRANSPORT = 6;          // cost  8, PLAN_NAVAL_TRANSPORT - the >= 7 rule
const int P_DEF_AIR = 7;            // cost 20, PLAN_COMBAT, armed, AIR
const int P_NEIGH_LAND = 8;         // cost 10, PLAN_COMBAT, armed, land
const int P_NEIGH_SEA = 9;          // cost 10, PLAN_COMBAT, armed, sea
const int P_DROP_LOW = 63;          // faction 0's last slot - one BELOW the scan
const int P_ATK_LAND = 64;          // cost 10, PLAN_DEFENSIVE, land
const int P_ATK_SEA = 65;           // cost 10, PLAN_DEFENSIVE, sea
const int P_ATK_AIR = 66;           // cost 10, PLAN_DEFENSIVE, air
const int P_ATK_MSL = 67;           // cost 10, PLAN_DEFENSIVE, missile
const int P_ATK_AS = 68;            // cost 10, PLAN_AIR_SUPERIORITY, land
const int P_ATK_AIROFF = 69;        // cost 10, PLAN_OFFENSIVE, air
const int P_ATK_MSLOFF = 70;        // cost 10, PLAN_OFFENSIVE, missile
const int P_DROP = 71;              // the drop-pod design the scan is meant to find
const int P_DROP_HIGH = 127;        // faction 1's last slot - the last one scanned
const int P_DROP_GUARD = 128;       // one PAST the scan

struct OddsWorld {
    // Index 0 is Vehs[-1]; 1..ODDS_VEH_COUNT are the live ids; the last is the
    // guard one past the end. See the note above.
    Veh vehs[1 + ODDS_VEH_COUNT + 1];
    VehPrototype protos[ODDS_PROTO_COUNT];
    RulesChassis chassis[ODDS_CHASSIS_COUNT];
    RulesWeapon weapons[ODDS_WEAPON_COUNT];
    Map tiles[ODDS_TILE_COUNT];
    Base bases[2];
    Player players[3];
    PlayerData players_data[3];   // [0] and [2] exist only to be poisoned
    Map *tiles_ptr;
    uint32_t longitude;
    int lon_bounds;
    int lat_bounds;
    BOOL is_flat;
    int veh_count;
    int base_count;
    BOOL expansion;
    // veh_at()'s diagnostic path is never taken by these fixtures - every tile
    // that carries BIT_VEH_IN_TILE holds a unit inside VehCurrentCount. The
    // three seams exist so that a fixture mistake writes here instead of into
    // an unmapped fixed address.
    BOOL veh_bit_error;
    uint32_t game_state;
    BOOL is_net;
};

OddsWorld g_odds_world;

// veh_id -1 is the underflow slot the veh_at bug reads; ODDS_VEH_COUNT is the guard.
Veh &odds_veh(int veh_id) {
    return g_odds_world.vehs[1 + veh_id];
}

Map &odds_tile(int x, int y) {
    return g_odds_world.tiles[(x >> 1) + y * ODDS_LONGITUDE];
}

PlayerData &odds_player(int faction_id) {
    return g_odds_world.players_data[faction_id];
}

void odds_reset() {
    std::memset(&g_odds_world, 0, sizeof(g_odds_world));
    g_odds_world.tiles_ptr = g_odds_world.tiles;
    g_odds_world.longitude = ODDS_LONGITUDE;
    g_odds_world.lon_bounds = ODDS_LON_BOUNDS;
    g_odds_world.lat_bounds = ODDS_LAT_BOUNDS;
    g_odds_world.is_flat = 1;
    g_odds_world.veh_count = ODDS_VEH_COUNT;   // the guard slot is outside the roster
    g_odds_world.base_count = 0;               // no base until a case builds one
    g_odds_world.expansion = 0;                // keeps is_alien_faction() out of has_abil

    // ---- units: every slot off the map, unstacked, and owned by nobody we ----
    // ---- ever ask about. The underflow slot and the guard get the same. ------
    for (int i = -1; i <= ODDS_VEH_COUNT; i++) {
        Veh &veh = odds_veh(i);
        veh.x = ODDS_POISON16;
        veh.y = ODDS_POISON16;
        veh.proto_id = (int16_t)P_DEF;
        veh.faction_id = (uint8_t)ODDS_STRANGER;
        veh.order = (int8_t)ODDS_POISON8;
        veh.flags = (uint16_t)0;
        veh.prev_veh_id_stack = -1;   // a zero here makes veh_top() spin forever
        veh.next_veh_id_stack = -1;
    }

    // ---- prototypes: poison first, then only the ones under test ------------
    // chassis_id and weapon_id stay INSIDE their tables even when poisoned, so a
    // misindexed read lands on the poison entry rather than off the end of the
    // fixture. cost and plan carry 0x77, which is >= PLAN_NAVAL_TRANSPORT and
    // matches neither PLAN_ALIEN_ARTIFACT nor PLAN_TERRAFORMING.
    for (int i = 0; i < ODDS_PROTO_COUNT; i++) {
        VehPrototype &proto = g_odds_world.protos[i];
        proto.chassis_id = CHS_MSL;
        proto.weapon_id = WPN_ARMED;
        proto.cost = ODDS_POISON8;
        proto.plan = ODDS_POISON8;
        proto.flags = 0;
        proto.ability_flags = 0;
    }
    g_odds_world.chassis[CHS_LAND].triad = TRIAD_LAND;
    g_odds_world.chassis[CHS_SEA].triad = TRIAD_SEA;
    g_odds_world.chassis[CHS_AIR].triad = TRIAD_AIR;
    g_odds_world.chassis[CHS_MSL].triad = TRIAD_AIR;
    g_odds_world.chassis[CHS_MSL].missile = 1;
    g_odds_world.chassis[4].triad = ODDS_POISON8;     // the guard
    g_odds_world.chassis[4].missile = 1;
    g_odds_world.weapons[WPN_UNARMED].offense_rating = 0;
    g_odds_world.weapons[WPN_ARMED].offense_rating = 4;
    g_odds_world.weapons[2].offense_rating = (int8_t)0x77;   // the guard

    struct Spec { int id; uint8_t chassis; uint8_t weapon; uint8_t cost; uint8_t plan; };
    static const Spec specs[] = {
        { P_DEF,        CHS_LAND, WPN_ARMED,   20, PLAN_COMBAT },
        { P_STACK,      CHS_LAND, WPN_ARMED,   12, PLAN_COMBAT },
        { P_ARTIFACT,   CHS_LAND, WPN_UNARMED,  5, PLAN_ALIEN_ARTIFACT },
        { P_TERRA,      CHS_LAND, WPN_UNARMED,  5, PLAN_TERRAFORMING },
        { P_TRANSPORT,  CHS_SEA,  WPN_UNARMED,  8, PLAN_NAVAL_TRANSPORT },
        { P_DEF_AIR,    CHS_AIR,  WPN_ARMED,   20, PLAN_COMBAT },
        { P_NEIGH_LAND, CHS_LAND, WPN_ARMED,   10, PLAN_COMBAT },
        { P_NEIGH_SEA,  CHS_SEA,  WPN_ARMED,   10, PLAN_COMBAT },
        { P_ATK_LAND,   CHS_LAND, WPN_ARMED,   10, PLAN_DEFENSIVE },
        { P_ATK_SEA,    CHS_SEA,  WPN_ARMED,   10, PLAN_DEFENSIVE },
        { P_ATK_AIR,    CHS_AIR,  WPN_ARMED,   10, PLAN_DEFENSIVE },
        { P_ATK_MSL,    CHS_MSL,  WPN_ARMED,   10, PLAN_DEFENSIVE },
        { P_ATK_AS,     CHS_LAND, WPN_ARMED,   10, PLAN_AIR_SUPERIORITY },
        { P_ATK_AIROFF, CHS_AIR,  WPN_ARMED,   10, PLAN_OFFENSIVE },
        { P_ATK_MSLOFF, CHS_MSL,  WPN_ARMED,   10, PLAN_OFFENSIVE },
    };
    for (int i = 0; i < (int)(sizeof(specs) / sizeof(specs[0])); i++) {
        VehPrototype &proto = g_odds_world.protos[specs[i].id];
        proto.chassis_id = specs[i].chassis;
        proto.weapon_id = specs[i].weapon;
        proto.cost = specs[i].cost;
        proto.plan = specs[i].plan;
    }

    // ---- the map: dry land one level above sea, all one region --------------
    for (int i = 0; i < ODDS_TILE_COUNT; i++) {
        g_odds_world.tiles[i].climate = 0x80;   // altitude 4, not ocean
        g_odds_world.tiles[i].region = 3;
        g_odds_world.tiles[i].bit = 0;
    }
    g_odds_world.tiles[ODDS_TILE_COUNT - 1].region = ODDS_POISON8;   // the guard tile
    g_odds_world.tiles[ODDS_TILE_COUNT - 1].climate = 0x20;

    // ---- bases: none, and nowhere near a real coordinate --------------------
    for (int i = 0; i < 2; i++) {
        g_odds_world.bases[i].x = -9999;
        g_odds_world.bases[i].y = -9999;
    }

    // ---- factions: rows 0 and 2 are poison, row 1 is the one under test -----
    for (int f = 0; f < 3; f++) {
        PlayerData &player = odds_player(f);
        const bool live = (f == ODDS_FACTION);
        player.flags = live ? 0u : 0x80000u;
        for (int r = 0; r < 128; r++) {
            player.region_base_plan[r] = ODDS_POISON8;
        }
        // Region 3 is the one every live tile carries. A wrong faction stride
        // reads 2 here and turns the "our ground" bonus off, or turns the
        // two-thirds tail on, either of which the assertions below catch.
        player.region_base_plan[3] = live ? 0 : 2;
        for (int p = 0; p < MaxVehProtoNum; p++) {
            player.proto_id_active[p] = live ? 0 : 0xFF;
        }
        for (int g = 0; g < MaxGoalsNum; g++) {
            player.goals[g].type = -1;
            player.goals[g].x = -9999;
            player.goals[g].y = -9999;
        }
    }
}

// Put a unit on the map and light the tile bit veh_at() insists on.
void odds_place(int veh_id, int proto_id, int faction_id, int x, int y) {
    Veh &veh = odds_veh(veh_id);
    veh.x = (int16_t)x;
    veh.y = (int16_t)y;
    veh.proto_id = (int16_t)proto_id;
    veh.faction_id = (uint8_t)faction_id;
    veh.order = ORDER_NONE;
    veh.flags = 0;
    odds_tile(x, y).bit |= BIT_VEH_IN_TILE;
}

// Link ids[0..count) into one stack, top first.
void odds_link(const int *ids, int count) {
    for (int k = 0; k < count; k++) {
        odds_veh(ids[k]).prev_veh_id_stack = (int16_t)(k ? ids[k - 1] : -1);
        odds_veh(ids[k]).next_veh_id_stack = (int16_t)(k + 1 < count ? ids[k + 1] : -1);
    }
}

// The attacker is never on the map; only its prototype is ever read.
void odds_attacker(int proto_id) {
    odds_veh(VEH_ATK).proto_id = (int16_t)proto_id;
    odds_veh(VEH_ATK).faction_id = (uint8_t)ODDS_FACTION;
}

// A lone defender of the enemy faction at the standard tile.
void odds_defender(int proto_id) {
    odds_place(VEH_DEF, proto_id, ODDS_ENEMY, ODDS_DEF_X, ODDS_DEF_Y);
}

void odds_goal(int slot, int type, int x, int y) {
    Goal &goal = odds_player(ODDS_FACTION).goals[slot];
    goal.type = (int16_t)type;
    goal.x = x;
    goal.y = y;
}

void odds_make_ocean(int x, int y) {
    odds_tile(x, y).climate = 0x20;   // altitude 1, below ALT_BIT_SHORE_LINE
}

// A design this faction could actually drop out of the sky.
void odds_drop_design(int proto_id) {
    g_odds_world.protos[proto_id].flags = PROTO_ACTIVE | PROTO_TYPED_COMPLETE;
    g_odds_world.protos[proto_id].ability_flags = ABL_DROP_POD;
    g_odds_world.protos[proto_id].chassis_id = CHS_AIR;
    g_odds_world.protos[proto_id].weapon_id = WPN_ARMED;
    g_odds_world.protos[proto_id].cost = 10;
    g_odds_world.protos[proto_id].plan = PLAN_COMBAT;
    odds_player(ODDS_FACTION).proto_id_active[proto_id] = 1;
}

#define OCHECK(cond)                                                          \
    do {                                                                      \
        const bool odds_ok = (cond);                                          \
        if (!odds_ok) {                                                       \
            std::fprintf(stderr, "compute_odds: line %d: %s\n", __LINE__,     \
                         #cond);                                              \
        }                                                                     \
        expect(odds_ok);                                                      \
    } while (0)

class OddsSeams {
 public:
    OddsSeams()
        : vehs_(&Vehs, &g_odds_world.vehs[1]),
          protos_(&VehPrototypes, g_odds_world.protos),
          chassis_(&Chassis, g_odds_world.chassis),
          weapons_(&Weapon, g_odds_world.weapons),
          tiles_(&MapTiles, &g_odds_world.tiles_ptr),
          longitude_(&MapLongitude, &g_odds_world.longitude),
          lon_(&MapLongitudeBounds, &g_odds_world.lon_bounds),
          lat_(&MapLatitudeBounds, &g_odds_world.lat_bounds),
          flat_(&MapIsFlat, &g_odds_world.is_flat),
          players_data_(&PlayersData, g_odds_world.players_data),
          players_(&Players, g_odds_world.players),
          bases_(&Bases, g_odds_world.bases),
          base_count_(&BaseCurrentCount, &g_odds_world.base_count),
          veh_count_(&VehCurrentCount, &g_odds_world.veh_count),
          expansion_(&ExpansionEnabled, &g_odds_world.expansion),
          bit_error_(&VehBitError, &g_odds_world.veh_bit_error),
          state_(&GameState, &g_odds_world.game_state),
          net_(&IsMultiplayerNet, &g_odds_world.is_net) { }

 private:
    ScopedSeam<Veh> vehs_;
    ScopedSeam<VehPrototype> protos_;
    ScopedSeam<RulesChassis> chassis_;
    ScopedSeam<RulesWeapon> weapons_;
    ScopedSeam<Map *> tiles_;
    ScopedSeam<uint32_t> longitude_;
    ScopedSeam<int> lon_;
    ScopedSeam<int> lat_;
    ScopedSeam<BOOL> flat_;
    ScopedSeam<PlayerData> players_data_;
    ScopedSeam<Player> players_;
    ScopedSeam<Base> bases_;
    ScopedSeam<int> base_count_;
    ScopedSeam<int> veh_count_;
    ScopedSeam<BOOL> expansion_;
    ScopedSeam<BOOL> bit_error_;
    ScopedSeam<uint32_t> state_;
    ScopedSeam<BOOL> net_;
};

/*
 * The cost basis: the one-fifth blend out in the field, the Alien Artifact
 * half, the flat cost on a base tile, and the only real division in the body.
 */
void odds_basis() {
    // A lone 20-cost defender, a 10-cost attacker, no base and no goal.
    // ((20 - 20) / 5 + 20) * 7 = 140, / 10 = 14.
    odds_reset();
    odds_attacker(P_ATK_LAND);
    odds_defender(P_DEF);
    OCHECK(compute_odds(7, ODDS_FACTION, VEH_ATK, VEH_DEF, -1) == 14);

    // A second unit in the stack counts at one FIFTH, and the fifth truncates:
    // ((32 - 20) / 5 + 20) * 7 = 154, / 10 = 15. A /4 would give 23*7/10 = 16.
    odds_reset();
    odds_attacker(P_ATK_LAND);
    odds_defender(P_DEF);
    odds_place(VEH_STACK_A, P_STACK, ODDS_ENEMY, ODDS_DEF_X, ODDS_DEF_Y);
    { const int ids[] = { VEH_DEF, VEH_STACK_A }; odds_link(ids, 2); }
    OCHECK(compute_odds(7, ODDS_FACTION, VEH_ATK, VEH_DEF, -1) == 15);

    // Three: ((44 - 20) / 5 + 20) * 7 = 168, / 10 = 16.
    odds_place(VEH_STACK_B, P_STACK, ODDS_ENEMY, ODDS_DEF_X, ODDS_DEF_Y);
    { const int ids[] = { VEH_DEF, VEH_STACK_A, VEH_STACK_B }; odds_link(ids, 3); }
    OCHECK(compute_odds(7, ODDS_FACTION, VEH_ATK, VEH_DEF, -1) == 16);

    // The fifth's sign correction is real code, not dead: a stack whose prev
    // and next chains disagree totals LESS than the defender's own cost.
    // veh_top(DEF) walks up to STACK_A, whose next is -1, so the sum is 12.
    // (12 - 20) / 5 truncates toward zero to -1, not down to -2:
    // (-1 + 20) * 7 = 133, / 10 = 13.  Floor division would give 12.
    odds_reset();
    odds_attacker(P_ATK_LAND);
    odds_defender(P_DEF);
    odds_place(VEH_STACK_A, P_STACK, ODDS_ENEMY, ODDS_DEF_X, ODDS_DEF_Y);
    odds_veh(VEH_DEF).prev_veh_id_stack = (int16_t)VEH_STACK_A;
    odds_veh(VEH_STACK_A).prev_veh_id_stack = -1;
    odds_veh(VEH_STACK_A).next_veh_id_stack = -1;
    OCHECK(compute_odds(7, ODDS_FACTION, VEH_ATK, VEH_DEF, -1) == 13);

    // An Alien Artifact anywhere in the stack adds half, BEFORE the division by
    // the attacker's cost: ((25 - 20) / 5 + 20) = 21, * 7 = 147, * 3 / 2 = 220,
    // / 10 = 22. The 3/2 truncates - 441 / 2 is 220, not 221.
    odds_reset();
    odds_attacker(P_ATK_LAND);
    odds_defender(P_DEF);
    odds_place(VEH_STACK_A, P_ARTIFACT, ODDS_ENEMY, ODDS_DEF_X, ODDS_DEF_Y);
    { const int ids[] = { VEH_DEF, VEH_STACK_A }; odds_link(ids, 2); }
    OCHECK(compute_odds(7, ODDS_FACTION, VEH_ATK, VEH_DEF, -1) == 22);

    // Any other non-combat plan is not an artifact: the same 21 * 7 = 147 now
    // goes straight to the division, 147 / 10 = 14.
    g_odds_world.protos[P_ARTIFACT].plan = PLAN_TERRAFORMING;
    OCHECK(compute_odds(7, ODDS_FACTION, VEH_ATK, VEH_DEF, -1) == 14);
    g_odds_world.protos[P_ARTIFACT].plan = PLAN_ALIEN_ARTIFACT;
    OCHECK(compute_odds(7, ODDS_FACTION, VEH_ATK, VEH_DEF, -1) == 22);

    // ---- a base tile ignores the stack entirely -----------------------------
    // Same two-unit stack, but base_id 0: 20 * 7 = 140, / 10 = 14, where the
    // field branch answered 15. The tile is ocean so the domain bonus below
    // cannot fire, and the stack is two armed units so the garrison rule
    // cannot either.
    odds_reset();
    odds_attacker(P_ATK_LAND);
    odds_defender(P_DEF);
    odds_place(VEH_STACK_A, P_STACK, ODDS_ENEMY, ODDS_DEF_X, ODDS_DEF_Y);
    { const int ids[] = { VEH_DEF, VEH_STACK_A }; odds_link(ids, 2); }
    odds_make_ocean(ODDS_DEF_X, ODDS_DEF_Y);
    OCHECK(compute_odds(7, ODDS_FACTION, VEH_ATK, VEH_DEF, 0) == 14);
    OCHECK(compute_odds(7, ODDS_FACTION, VEH_ATK, VEH_DEF, -1) == 15);

    // ---- the division by the attacker's cost truncates ----------------------
    odds_reset();
    odds_attacker(P_ATK_LAND);
    odds_defender(P_DEF);
    g_odds_world.protos[P_ATK_LAND].cost = 3;      // 140 / 3 = 46, not 47
    OCHECK(compute_odds(7, ODDS_FACTION, VEH_ATK, VEH_DEF, -1) == 46);
    g_odds_world.protos[P_ATK_LAND].cost = 1;
    OCHECK(compute_odds(7, ODDS_FACTION, VEH_ATK, VEH_DEF, -1) == 140);
    g_odds_world.protos[P_ATK_LAND].cost = 200;    // 140 / 200 = 0
    OCHECK(compute_odds(7, ODDS_FACTION, VEH_ATK, VEH_DEF, -1) == 0);

    // ---- base_id strictly below -1 asks base_at for the answer --------------
    // No base bit on the tile: base_at answers -1 and the field branch runs.
    odds_reset();
    odds_attacker(P_ATK_LAND);
    odds_defender(P_DEF);
    odds_place(VEH_STACK_A, P_STACK, ODDS_ENEMY, ODDS_DEF_X, ODDS_DEF_Y);
    { const int ids[] = { VEH_DEF, VEH_STACK_A }; odds_link(ids, 2); }
    odds_make_ocean(ODDS_DEF_X, ODDS_DEF_Y);
    OCHECK(compute_odds(7, ODDS_FACTION, VEH_ATK, VEH_DEF, -2) == 15);

    // Now give the tile a real base. base_at answers 0 and the base branch runs.
    g_odds_world.bases[0].x = (int16_t)ODDS_DEF_X;
    g_odds_world.bases[0].y = (int16_t)ODDS_DEF_Y;
    g_odds_world.bases[0].faction_id_current = (uint8_t)ODDS_ENEMY;
    g_odds_world.base_count = 1;
    odds_tile(ODDS_DEF_X, ODDS_DEF_Y).bit |= BIT_BASE_IN_TILE;
    OCHECK(compute_odds(7, ODDS_FACTION, VEH_ATK, VEH_DEF, -2) == 14);

    // -1 asserts there is no base and is NOT a lookup request, even with the
    // base sitting right there.
    OCHECK(compute_odds(7, ODDS_FACTION, VEH_ATK, VEH_DEF, -1) == 15);
    // Anything below -1 is, though.
    OCHECK(compute_odds(7, ODDS_FACTION, VEH_ATK, VEH_DEF, -37) == 14);
}

/*
 * The multipliers that run before the triad fork: the non-combat defender, the
 * "our ground" three halves, the goal gate, the domain match, the thin
 * garrison, and the objective flag.
 */
void odds_multipliers() {
    // ---- a defender whose plan is PLAN_NAVAL_TRANSPORT or later doubles -----
    // 8 * 7 = 56, / 10 = 5, then doubled to 10.
    odds_reset();
    odds_attacker(P_ATK_LAND);
    odds_defender(P_TRANSPORT);
    OCHECK(compute_odds(7, ODDS_FACTION, VEH_ATK, VEH_DEF, -1) == 10);
    g_odds_world.protos[P_TRANSPORT].plan = PLAN_NAVAL_SUPERIORITY;   // 6, one below
    OCHECK(compute_odds(7, ODDS_FACTION, VEH_ATK, VEH_DEF, -1) == 5);
    g_odds_world.protos[P_TRANSPORT].plan = PLAN_COLONIZATION;        // 8, above
    OCHECK(compute_odds(7, ODDS_FACTION, VEH_ATK, VEH_DEF, -1) == 10);

    // ---- an offensive or combat LAND attacker on an unplanned region --------
    // Base value 15; 15 * 3 / 2 truncates to 22, not 23.
    odds_reset();
    odds_attacker(P_ATK_LAND);
    odds_defender(P_DEF);
    odds_place(VEH_STACK_A, P_STACK, ODDS_ENEMY, ODDS_DEF_X, ODDS_DEF_Y);
    { const int ids[] = { VEH_DEF, VEH_STACK_A }; odds_link(ids, 2); }
    g_odds_world.protos[P_ATK_LAND].plan = PLAN_COMBAT;
    OCHECK(compute_odds(7, ODDS_FACTION, VEH_ATK, VEH_DEF, -1) == 22);
    g_odds_world.protos[P_ATK_LAND].plan = PLAN_OFFENSIVE;            // the other side
    OCHECK(compute_odds(7, ODDS_FACTION, VEH_ATK, VEH_DEF, -1) == 22);
    g_odds_world.protos[P_ATK_LAND].plan = PLAN_DEFENSIVE;            // one past
    OCHECK(compute_odds(7, ODDS_FACTION, VEH_ATK, VEH_DEF, -1) == 15);

    // Any region plan at all takes it away, and the plan is read from THIS
    // faction's row - rows 0 and 2 both carry 2 for region 3.
    g_odds_world.protos[P_ATK_LAND].plan = PLAN_COMBAT;
    odds_player(ODDS_FACTION).region_base_plan[3] = 1;
    OCHECK(compute_odds(7, ODDS_FACTION, VEH_ATK, VEH_DEF, -1) == 15);
    odds_player(ODDS_FACTION).region_base_plan[3] = 0;
    OCHECK(compute_odds(7, ODDS_FACTION, VEH_ATK, VEH_DEF, -1) == 22);
    // The region comes from the DEFENDER's tile, not from index zero.
    odds_tile(ODDS_DEF_X, ODDS_DEF_Y).region = 9;
    odds_player(ODDS_FACTION).region_base_plan[9] = 4;
    OCHECK(compute_odds(7, ODDS_FACTION, VEH_ATK, VEH_DEF, -1) == 15);
    odds_tile(ODDS_DEF_X, ODDS_DEF_Y).region = 3;
    OCHECK(compute_odds(7, ODDS_FACTION, VEH_ATK, VEH_DEF, -1) == 22);

    // And a sea attacker never gets it.
    g_odds_world.protos[P_ATK_LAND].chassis_id = CHS_SEA;
    OCHECK(compute_odds(7, ODDS_FACTION, VEH_ATK, VEH_DEF, -1) == 15);

    // ---- the goal gate ------------------------------------------------------
    // Off the map's declared goals, neither the domain bonus nor the garrison
    // rule is reachable for a field target.
    odds_reset();
    odds_attacker(P_ATK_LAND);
    odds_defender(P_DEF);
    OCHECK(compute_odds(7, ODDS_FACTION, VEH_ATK, VEH_DEF, -1) == 14);
    odds_goal(0, AI_GOAL_ATTACK, ODDS_DEF_X, ODDS_DEF_Y);
    OCHECK(compute_odds(7, ODDS_FACTION, VEH_ATK, VEH_DEF, -1) == 28);   // land on land
    odds_goal(0, AI_GOAL_DEFEND, ODDS_DEF_X, ODDS_DEF_Y);                // wrong type
    OCHECK(compute_odds(7, ODDS_FACTION, VEH_ATK, VEH_DEF, -1) == 14);
    odds_goal(0, AI_GOAL_ATTACK, ODDS_DEF_X + 1, ODDS_DEF_Y);            // wrong tile
    OCHECK(compute_odds(7, ODDS_FACTION, VEH_ATK, VEH_DEF, -1) == 14);
    odds_goal(MaxGoalsNum - 1, AI_GOAL_ATTACK, ODDS_DEF_X, ODDS_DEF_Y);  // the last slot
    OCHECK(compute_odds(7, ODDS_FACTION, VEH_ATK, VEH_DEF, -1) == 28);

    // ---- the domain match ---------------------------------------------------
    odds_reset();
    odds_attacker(P_ATK_LAND);
    odds_defender(P_DEF);
    odds_goal(0, AI_GOAL_ATTACK, ODDS_DEF_X, ODDS_DEF_Y);
    OCHECK(compute_odds(7, ODDS_FACTION, VEH_ATK, VEH_DEF, -1) == 28);   // land / land
    odds_make_ocean(ODDS_DEF_X, ODDS_DEF_Y);
    OCHECK(compute_odds(7, ODDS_FACTION, VEH_ATK, VEH_DEF, -1) == 14);   // land / water
    odds_attacker(P_ATK_SEA);
    OCHECK(compute_odds(7, ODDS_FACTION, VEH_ATK, VEH_DEF, -1) == 28);   // sea  / water
    odds_tile(ODDS_DEF_X, ODDS_DEF_Y).climate = 0x80;
    OCHECK(compute_odds(7, ODDS_FACTION, VEH_ATK, VEH_DEF, -1) == 14);   // sea  / land
    // ALT_BIT_SHORE_LINE itself is land.
    odds_tile(ODDS_DEF_X, ODDS_DEF_Y).climate = 0x60;
    OCHECK(compute_odds(7, ODDS_FACTION, VEH_ATK, VEH_DEF, -1) == 14);
    odds_tile(ODDS_DEF_X, ODDS_DEF_Y).climate = 0x40;                    // one below
    OCHECK(compute_odds(7, ODDS_FACTION, VEH_ATK, VEH_DEF, -1) == 28);
    // Air is excluded from the bonus even when the domains "match".
    odds_reset();
    odds_attacker(P_ATK_AIR);
    odds_defender(P_DEF);
    odds_goal(0, AI_GOAL_ATTACK, ODDS_DEF_X, ODDS_DEF_Y);
    OCHECK(compute_odds(7, ODDS_FACTION, VEH_ATK, VEH_DEF, -1) == 14);

    // ---- the thin garrison, which the domain bonus returns before ----------
    // Ocean tile with a land attacker, so the domain never matches and the
    // else branch is reached. One armed defender: 1 - 0 <= 1, doubled.
    odds_reset();
    odds_attacker(P_ATK_LAND);
    odds_defender(P_DEF);
    odds_make_ocean(ODDS_DEF_X, ODDS_DEF_Y);
    OCHECK(compute_odds(7, ODDS_FACTION, VEH_ATK, VEH_DEF, 0) == 28);
    // Two armed defenders: 2 - 0 > 1, not doubled.
    odds_place(VEH_STACK_A, P_STACK, ODDS_ENEMY, ODDS_DEF_X, ODDS_DEF_Y);
    { const int ids[] = { VEH_DEF, VEH_STACK_A }; odds_link(ids, 2); }
    OCHECK(compute_odds(7, ODDS_FACTION, VEH_ATK, VEH_DEF, 0) == 14);
    // Two units, one of them unarmed: 2 - 1 <= 1, doubled again. This is the
    // only pair that moves the type-19 count without moving the type-1 count.
    g_odds_world.protos[P_STACK].weapon_id = WPN_UNARMED;
    OCHECK(compute_odds(7, ODDS_FACTION, VEH_ATK, VEH_DEF, 0) == 28);
    // Three, two of them unarmed: 3 - 2 <= 1.
    odds_place(VEH_STACK_B, P_STACK, ODDS_ENEMY, ODDS_DEF_X, ODDS_DEF_Y);
    { const int ids[] = { VEH_DEF, VEH_STACK_A, VEH_STACK_B }; odds_link(ids, 3); }
    OCHECK(compute_odds(7, ODDS_FACTION, VEH_ATK, VEH_DEF, 0) == 28);
    // Three, one unarmed: 3 - 1 > 1.
    g_odds_world.protos[P_STACK].weapon_id = WPN_ARMED;
    odds_veh(VEH_STACK_B).proto_id = (int16_t)P_ARTIFACT;   // unarmed
    OCHECK(compute_odds(7, ODDS_FACTION, VEH_ATK, VEH_DEF, 0) == 14);
    // The garrison rule needs the base. Declaring the tile an attack goal
    // opens the same gate, but the field branch reaches only the domain test -
    // which a land attacker over water cannot pass - so the thin garrison here
    // buys nothing. The field basis returns as well: the stack is now
    // 20 + 12 + 5 = 37, ((37 - 20) / 5 + 20) = 23, * 7 = 161, and the artifact
    // in it takes that to 161 * 3 / 2 = 241, / 10 = 24.
    odds_goal(0, AI_GOAL_ATTACK, ODDS_DEF_X, ODDS_DEF_Y);
    OCHECK(compute_odds(7, ODDS_FACTION, VEH_ATK, VEH_DEF, -1) == 24);
}

/*
 * The objective flag, kept separate because it is the only x4 in the body.
 */
void odds_objective() {
    odds_reset();
    odds_attacker(P_ATK_LAND);
    odds_defender(P_DEF);
    OCHECK(compute_odds(7, ODDS_FACTION, VEH_ATK, VEH_DEF, -1) == 14);
    odds_veh(VEH_DEF).flags = VFLAG_IS_OBJECTIVE;
    OCHECK(compute_odds(7, ODDS_FACTION, VEH_ATK, VEH_DEF, -1) == 56);   // x4, not x2
    odds_veh(VEH_DEF).flags = VFLAG_LURKER;                              // a neighbouring bit
    OCHECK(compute_odds(7, ODDS_FACTION, VEH_ATK, VEH_DEF, -1) == 14);
    odds_veh(VEH_DEF).flags = VFLAG_PROBE_PACT_OPERATIONS;
    OCHECK(compute_odds(7, ODDS_FACTION, VEH_ATK, VEH_DEF, -1) == 14);
    odds_veh(VEH_DEF).flags = (uint16_t)(VFLAG_IS_OBJECTIVE | VFLAG_LURKER | VFLAG_INVISIBLE);
    OCHECK(compute_odds(7, ODDS_FACTION, VEH_ATK, VEH_DEF, -1) == 56);
    // It is the defender's flag, not the attacker's.
    odds_veh(VEH_DEF).flags = 0;
    odds_veh(VEH_ATK).flags = VFLAG_IS_OBJECTIVE;
    OCHECK(compute_odds(7, ODDS_FACTION, VEH_ATK, VEH_DEF, -1) == 14);
}

/*
 * The air branch: the missile over open water, the terraformer block, the
 * eight-neighbour scan, the tenth-of-active threshold and the drop-pod scan.
 */
void odds_air() {
    // ---- a missile aimed at open water away from a base --------------------
    // ((32 - 20) / 5 + 20) * 7 = 154, / 10 = 15, doubled to 30.
    odds_reset();
    odds_attacker(P_ATK_MSL);
    odds_defender(P_DEF);
    odds_place(VEH_STACK_A, P_STACK, ODDS_ENEMY, ODDS_DEF_X, ODDS_DEF_Y);
    { const int ids[] = { VEH_DEF, VEH_STACK_A }; odds_link(ids, 2); }
    odds_make_ocean(ODDS_DEF_X, ODDS_DEF_Y);
    OCHECK(compute_odds(7, ODDS_FACTION, VEH_ATK, VEH_DEF, -1) == 30);
    // Not a missile: no bonus.
    odds_attacker(P_ATK_AIR);
    OCHECK(compute_odds(7, ODDS_FACTION, VEH_ATK, VEH_DEF, -1) == 15);
    // A missile over land: no bonus.
    odds_attacker(P_ATK_MSL);
    odds_tile(ODDS_DEF_X, ODDS_DEF_Y).climate = 0x80;
    OCHECK(compute_odds(7, ODDS_FACTION, VEH_ATK, VEH_DEF, -1) == 15);
    // A missile over water WITH a base: no bonus. The base branch also drops
    // the stack from the basis, so 20 * 7 / 10 = 14, and the garrison is two
    // armed units so the earlier rule cannot fire either.
    odds_make_ocean(ODDS_DEF_X, ODDS_DEF_Y);
    OCHECK(compute_odds(7, ODDS_FACTION, VEH_ATK, VEH_DEF, 0) == 14);

    // ---- the terraformer block ---------------------------------------------
    // ((25 - 20) / 5 + 20) * 7 = 147, / 10 = 14, then x2 for not being a
    // missile = 28.
    odds_reset();
    odds_attacker(P_ATK_AIR);
    odds_defender(P_DEF);
    odds_place(VEH_STACK_A, P_TERRA, ODDS_ENEMY, ODDS_DEF_X, ODDS_DEF_Y);
    { const int ids[] = { VEH_DEF, VEH_STACK_A }; odds_link(ids, 2); }
    OCHECK(compute_odds(7, ODDS_FACTION, VEH_ATK, VEH_DEF, -1) == 28);
    // No terraformer in the stack and the whole block is skipped: 14.
    g_odds_world.protos[P_TERRA].plan = PLAN_SUPPLY_CONVOY;
    OCHECK(compute_odds(7, ODDS_FACTION, VEH_ATK, VEH_DEF, -1) == 14);
    g_odds_world.protos[P_TERRA].plan = PLAN_TERRAFORMING;
    // A missile does NOT get the not-a-missile half of it: back to 14, and the
    // open-water half cannot fire on land.
    odds_attacker(P_ATK_MSL);
    OCHECK(compute_odds(7, ODDS_FACTION, VEH_ATK, VEH_DEF, -1) == 14);
    // Over water the missile gets the earlier open-water bonus instead: 28.
    odds_make_ocean(ODDS_DEF_X, ODDS_DEF_Y);
    OCHECK(compute_odds(7, ODDS_FACTION, VEH_ATK, VEH_DEF, -1) == 28);

    // The unnamed 0x80000 player flag, over water only, and read from THIS
    // faction's row - rows 0 and 2 both carry the bit.
    odds_reset();
    odds_attacker(P_ATK_AIR);
    odds_defender(P_DEF);
    odds_place(VEH_STACK_A, P_TERRA, ODDS_ENEMY, ODDS_DEF_X, ODDS_DEF_Y);
    { const int ids[] = { VEH_DEF, VEH_STACK_A }; odds_link(ids, 2); }
    odds_player(ODDS_FACTION).flags = 0x80000;
    OCHECK(compute_odds(7, ODDS_FACTION, VEH_ATK, VEH_DEF, -1) == 28);   // land, no bonus
    odds_make_ocean(ODDS_DEF_X, ODDS_DEF_Y);
    OCHECK(compute_odds(7, ODDS_FACTION, VEH_ATK, VEH_DEF, -1) == 56);   // water + flag
    odds_player(ODDS_FACTION).flags = 0x40000;                           // a neighbouring bit
    OCHECK(compute_odds(7, ODDS_FACTION, VEH_ATK, VEH_DEF, -1) == 28);
    odds_player(ODDS_FACTION).flags = 0xFFFFFFFFu;
    OCHECK(compute_odds(7, ODDS_FACTION, VEH_ATK, VEH_DEF, -1) == 56);

    // The two terraform orders, on the DEFENDER, each double once. They are two
    // independent tests in the original and a unit can only hold one value, so
    // the two doublings can never both apply.
    odds_reset();
    odds_attacker(P_ATK_AIR);
    odds_defender(P_DEF);
    odds_place(VEH_STACK_A, P_TERRA, ODDS_ENEMY, ODDS_DEF_X, ODDS_DEF_Y);
    { const int ids[] = { VEH_DEF, VEH_STACK_A }; odds_link(ids, 2); }
    OCHECK(compute_odds(7, ODDS_FACTION, VEH_ATK, VEH_DEF, -1) == 28);
    odds_veh(VEH_DEF).order = ORDER_DRILL_AQUIFIER;      // 19, one below
    OCHECK(compute_odds(7, ODDS_FACTION, VEH_ATK, VEH_DEF, -1) == 28);
    odds_veh(VEH_DEF).order = ORDER_TERRAFORM_UP;        // 20
    OCHECK(compute_odds(7, ODDS_FACTION, VEH_ATK, VEH_DEF, -1) == 56);
    odds_veh(VEH_DEF).order = ORDER_TERRAFORM_DOWN;      // 21
    OCHECK(compute_odds(7, ODDS_FACTION, VEH_ATK, VEH_DEF, -1) == 56);
    odds_veh(VEH_DEF).order = ORDER_TERRAFORM_LEVEL;     // 22, one above
    OCHECK(compute_odds(7, ODDS_FACTION, VEH_ATK, VEH_DEF, -1) == 28);
    // It is read from the defender, and only inside the terraformer block.
    odds_veh(VEH_DEF).order = ORDER_TERRAFORM_UP;
    g_odds_world.protos[P_TERRA].plan = PLAN_SUPPLY_CONVOY;
    OCHECK(compute_odds(7, ODDS_FACTION, VEH_ATK, VEH_DEF, -1) == 14);
}

/*
 * The air branch's eight-neighbour scan and the drop-pod scan behind it.
 *
 * The standing setup: an air attacker, a base on the defender's tile, and a
 * two-unit armed garrison so the earlier thin-garrison rule stays off. The
 * basis is 20 * 7 / 10 = 14 in every case here.
 */
void odds_air_neighbours() {
    // No friendly neighbour: 14.
    odds_reset();
    odds_attacker(P_ATK_AIR);
    odds_defender(P_DEF);
    odds_place(VEH_STACK_A, P_STACK, ODDS_ENEMY, ODDS_DEF_X, ODDS_DEF_Y);
    { const int ids[] = { VEH_DEF, VEH_STACK_A }; odds_link(ids, 2); }
    OCHECK(compute_odds(7, ODDS_FACTION, VEH_ATK, VEH_DEF, 0) == 14);

    // A friendly LAND unit next door doubles it, once.
    odds_place(VEH_NEIGH_A, P_NEIGH_LAND, ODDS_FACTION, ODDS_DEF_X + 2, ODDS_DEF_Y);
    OCHECK(compute_odds(7, ODDS_FACTION, VEH_ATK, VEH_DEF, 0) == 28);
    // Two of them still only double once - the loop breaks on the first.
    odds_place(VEH_NEIGH_B, P_NEIGH_LAND, ODDS_FACTION, ODDS_DEF_X - 1, ODDS_DEF_Y - 1);
    OCHECK(compute_odds(7, ODDS_FACTION, VEH_ATK, VEH_DEF, 0) == 28);

    // A friendly SEA unit is not land, and does not.
    odds_reset();
    odds_attacker(P_ATK_AIR);
    odds_defender(P_DEF);
    odds_place(VEH_STACK_A, P_STACK, ODDS_ENEMY, ODDS_DEF_X, ODDS_DEF_Y);
    { const int ids[] = { VEH_DEF, VEH_STACK_A }; odds_link(ids, 2); }
    odds_place(VEH_NEIGH_A, P_NEIGH_SEA, ODDS_FACTION, ODDS_DEF_X + 2, ODDS_DEF_Y);
    OCHECK(compute_odds(7, ODDS_FACTION, VEH_ATK, VEH_DEF, 0) == 14);
    // A LAND unit belonging to somebody else does not either.
    odds_veh(VEH_NEIGH_A).proto_id = (int16_t)P_NEIGH_LAND;
    odds_veh(VEH_NEIGH_A).faction_id = (uint8_t)ODDS_ENEMY;
    OCHECK(compute_odds(7, ODDS_FACTION, VEH_ATK, VEH_DEF, 0) == 14);
    odds_veh(VEH_NEIGH_A).faction_id = (uint8_t)ODDS_FACTION;
    OCHECK(compute_odds(7, ODDS_FACTION, VEH_ATK, VEH_DEF, 0) == 28);
    // Two tiles away is not a neighbour: (12,4) is outside RadiusBase.
    odds_reset();
    odds_attacker(P_ATK_AIR);
    odds_defender(P_DEF);
    odds_place(VEH_STACK_A, P_STACK, ODDS_ENEMY, ODDS_DEF_X, ODDS_DEF_Y);
    { const int ids[] = { VEH_DEF, VEH_STACK_A }; odds_link(ids, 2); }
    odds_place(VEH_NEIGH_A, P_NEIGH_LAND, ODDS_FACTION, ODDS_DEF_X + 4, ODDS_DEF_Y);
    OCHECK(compute_odds(7, ODDS_FACTION, VEH_ATK, VEH_DEF, 0) == 14);
    // Nor is the defender's own tile - RadiusBase entry 8 is (0,0) and the loop
    // stops at 8, so it is never read.
    //
    // Moving the unit by hand leaves BIT_VEH_IN_TILE behind on (12,4), which is
    // deliberate and safe here because (12,4) is not one of the eight. Keep it
    // that way: veh_at() answers a tile that carries the bit and holds no unit
    // by falling through to log_say() and rebuild_base_bits(), which would pass
    // quietly while writing diagnostics rather than fail. Clear the bit if any
    // later edit brings that tile inside the radius.
    odds_veh(VEH_NEIGH_A).x = (int16_t)ODDS_DEF_X;
    odds_veh(VEH_NEIGH_A).y = (int16_t)ODDS_DEF_Y;
    OCHECK(compute_odds(7, ODDS_FACTION, VEH_ATK, VEH_DEF, 0) == 14);

    // ---- the unchecked veh_at() answer of -1 --------------------------------
    // BUG, reproduced not corrected: veh_at answers -1 for an empty tile and
    // the original reads Vehs[-1].faction_id from it. The slot below vehs[0]
    // exists so that read lands inside the fixture; it is poisoned with a
    // faction that is not ours, so the loop skips.
    odds_reset();
    odds_attacker(P_ATK_AIR);
    odds_defender(P_DEF);
    odds_place(VEH_STACK_A, P_STACK, ODDS_ENEMY, ODDS_DEF_X, ODDS_DEF_Y);
    { const int ids[] = { VEH_DEF, VEH_STACK_A }; odds_link(ids, 2); }
    OCHECK(odds_veh(-1).faction_id == (uint8_t)ODDS_STRANGER);
    OCHECK(compute_odds(7, ODDS_FACTION, VEH_ATK, VEH_DEF, 0) == 14);
    // Make the underflow slot answer OUR faction and the comparison now passes
    // for every empty neighbour, so stack_check(-1, ...) really is called. It
    // answers zero because veh_top(-1) is -1, which is why the defect is quiet
    // rather than fatal - and why only a fixture that can see the read at all
    // can say so.
    odds_veh(-1).faction_id = (uint8_t)ODDS_FACTION;
    OCHECK(compute_odds(7, ODDS_FACTION, VEH_ATK, VEH_DEF, 0) == 14);
    OCHECK(odds_veh(-1).faction_id == (uint8_t)ODDS_FACTION);   // and nothing wrote to it

    // ---- the tenth-of-active threshold, and the drop-pod scan --------------
    // Garrison is 2. proto_id_active[attacker] / 10 + 1 must be at least 2, so
    // 9 gives 1 and refuses while 10 gives 2 and admits.
    odds_reset();
    odds_attacker(P_ATK_AIR);
    odds_defender(P_DEF);
    odds_place(VEH_STACK_A, P_STACK, ODDS_ENEMY, ODDS_DEF_X, ODDS_DEF_Y);
    { const int ids[] = { VEH_DEF, VEH_STACK_A }; odds_link(ids, 2); }
    odds_place(VEH_NEIGH_A, P_NEIGH_LAND, ODDS_FACTION, ODDS_DEF_X + 2, ODDS_DEF_Y);
    odds_drop_design(P_DROP);
    odds_player(ODDS_FACTION).proto_id_active[P_ATK_AIR] = 9;
    OCHECK(compute_odds(7, ODDS_FACTION, VEH_ATK, VEH_DEF, 0) == 28);    // scan refused
    odds_player(ODDS_FACTION).proto_id_active[P_ATK_AIR] = 10;
    OCHECK(compute_odds(7, ODDS_FACTION, VEH_ATK, VEH_DEF, 0) == 56);    // scan admitted
    odds_player(ODDS_FACTION).proto_id_active[P_ATK_AIR] = 19;           // still 1 + 1
    OCHECK(compute_odds(7, ODDS_FACTION, VEH_ATK, VEH_DEF, 0) == 56);
    // A one-unit garrison passes the threshold with a zero count, but it also
    // trips the thin-garrison rule, so the value is 14 x2 x2 x2 = 112.
    odds_reset();
    odds_attacker(P_ATK_AIR);
    odds_defender(P_DEF);
    odds_make_ocean(ODDS_DEF_X, ODDS_DEF_Y);   // keeps the domain bonus off
    odds_place(VEH_NEIGH_A, P_NEIGH_LAND, ODDS_FACTION, ODDS_DEF_X + 2, ODDS_DEF_Y);
    odds_drop_design(P_DROP);
    OCHECK(compute_odds(7, ODDS_FACTION, VEH_ATK, VEH_DEF, 0) == 112);

    // ---- what disqualifies a drop-pod design -------------------------------
    odds_reset();
    odds_attacker(P_ATK_AIR);
    odds_defender(P_DEF);
    odds_place(VEH_STACK_A, P_STACK, ODDS_ENEMY, ODDS_DEF_X, ODDS_DEF_Y);
    { const int ids[] = { VEH_DEF, VEH_STACK_A }; odds_link(ids, 2); }
    odds_place(VEH_NEIGH_A, P_NEIGH_LAND, ODDS_FACTION, ODDS_DEF_X + 2, ODDS_DEF_Y);
    odds_player(ODDS_FACTION).proto_id_active[P_ATK_AIR] = 10;
    odds_drop_design(P_DROP);
    OCHECK(compute_odds(7, ODDS_FACTION, VEH_ATK, VEH_DEF, 0) == 56);
    g_odds_world.protos[P_DROP].flags = PROTO_TYPED_COMPLETE;          // retired
    OCHECK(compute_odds(7, ODDS_FACTION, VEH_ATK, VEH_DEF, 0) == 28);
    g_odds_world.protos[P_DROP].flags = PROTO_ACTIVE;                  // not typed
    OCHECK(compute_odds(7, ODDS_FACTION, VEH_ATK, VEH_DEF, 0) == 28);
    g_odds_world.protos[P_DROP].flags = PROTO_ACTIVE | PROTO_TYPED_COMPLETE;
    OCHECK(compute_odds(7, ODDS_FACTION, VEH_ATK, VEH_DEF, 0) == 56);
    odds_player(ODDS_FACTION).proto_id_active[P_DROP] = 0;             // none in service
    OCHECK(compute_odds(7, ODDS_FACTION, VEH_ATK, VEH_DEF, 0) == 28);
    odds_player(ODDS_FACTION).proto_id_active[P_DROP] = 1;
    g_odds_world.protos[P_DROP].ability_flags = ABL_AIR_SUPERIORITY;   // wrong ability
    OCHECK(compute_odds(7, ODDS_FACTION, VEH_ATK, VEH_DEF, 0) == 28);
    g_odds_world.protos[P_DROP].ability_flags = ABL_DROP_POD | ABL_AAA;
    OCHECK(compute_odds(7, ODDS_FACTION, VEH_ATK, VEH_DEF, 0) == 56);

    // ---- the scan's bounds, which is what the guard prototype is for -------
    odds_reset();
    odds_attacker(P_ATK_AIR);
    odds_defender(P_DEF);
    odds_place(VEH_STACK_A, P_STACK, ODDS_ENEMY, ODDS_DEF_X, ODDS_DEF_Y);
    { const int ids[] = { VEH_DEF, VEH_STACK_A }; odds_link(ids, 2); }
    odds_place(VEH_NEIGH_A, P_NEIGH_LAND, ODDS_FACTION, ODDS_DEF_X + 2, ODDS_DEF_Y);
    odds_player(ODDS_FACTION).proto_id_active[P_ATK_AIR] = 10;
    odds_drop_design(P_DROP_HIGH);            // 127, the last slot faction 1 owns
    OCHECK(compute_odds(7, ODDS_FACTION, VEH_ATK, VEH_DEF, 0) == 56);
    g_odds_world.protos[P_DROP_HIGH].flags = 0;
    odds_drop_design(P_DROP_GUARD);           // 128, one past the sixty-four
    OCHECK(compute_odds(7, ODDS_FACTION, VEH_ATK, VEH_DEF, 0) == 28);
    g_odds_world.protos[P_DROP_GUARD].flags = 0;
    odds_drop_design(P_DROP_LOW);             // 63, one below - faction 0's slot
    OCHECK(compute_odds(7, ODDS_FACTION, VEH_ATK, VEH_DEF, 0) == 28);
    g_odds_world.protos[P_DROP_LOW].flags = 0;
    odds_drop_design(P_ATK_LAND);             // 64, faction 1's first
    OCHECK(compute_odds(7, ODDS_FACTION, VEH_ATK, VEH_DEF, 0) == 56);

    // A missile attacker doubles a second time for the same drop-pod design.
    odds_reset();
    odds_attacker(P_ATK_MSL);
    odds_defender(P_DEF);
    odds_place(VEH_STACK_A, P_STACK, ODDS_ENEMY, ODDS_DEF_X, ODDS_DEF_Y);
    { const int ids[] = { VEH_DEF, VEH_STACK_A }; odds_link(ids, 2); }
    odds_place(VEH_NEIGH_A, P_NEIGH_LAND, ODDS_FACTION, ODDS_DEF_X + 2, ODDS_DEF_Y);
    odds_player(ODDS_FACTION).proto_id_active[P_ATK_MSL] = 10;
    odds_drop_design(P_DROP);
    OCHECK(compute_odds(7, ODDS_FACTION, VEH_ATK, VEH_DEF, 0) == 112);

    // None of this happens off a base tile.
    OCHECK(compute_odds(7, ODDS_FACTION, VEH_ATK, VEH_DEF, -1) == 15);
}

/*
 * The non-air branch: the accumulated neighbour count, the divide-by-garrison
 * doubling loop, the zero-defender guard and the million clamp.
 *
 * The standing setup puts the defender on an OCEAN base tile with a land
 * attacker, so the domain bonus never matches and the two-unit garrison keeps
 * the thin-garrison rule off. The basis is 20 * 7 / 10 = 14 throughout.
 */
void odds_ground() {
    odds_reset();
    odds_attacker(P_ATK_LAND);
    odds_defender(P_DEF);
    odds_place(VEH_STACK_A, P_STACK, ODDS_ENEMY, ODDS_DEF_X, ODDS_DEF_Y);
    { const int ids[] = { VEH_DEF, VEH_STACK_A }; odds_link(ids, 2); }
    odds_make_ocean(ODDS_DEF_X, ODDS_DEF_Y);
    OCHECK(compute_odds(7, ODDS_FACTION, VEH_ATK, VEH_DEF, 0) == 14);   // no neighbours

    // Two land units next door against a two-unit garrison: 2 > 2 is false.
    odds_place(VEH_NEIGH_A, P_NEIGH_LAND, ODDS_FACTION, ODDS_DEF_X + 2, ODDS_DEF_Y);
    odds_place(VEH_NEIGH_B, P_NEIGH_LAND, ODDS_FACTION, ODDS_DEF_X - 1, ODDS_DEF_Y - 1);
    OCHECK(compute_odds(7, ODDS_FACTION, VEH_ATK, VEH_DEF, 0) == 14);

    // Three: 3 > 2 doubles once, then 1 > 2 stops.
    odds_place(VEH_NEIGH_C, P_NEIGH_LAND, ODDS_FACTION, ODDS_DEF_X + 1, ODDS_DEF_Y + 1);
    OCHECK(compute_odds(7, ODDS_FACTION, VEH_ATK, VEH_DEF, 0) == 28);

    // Five: 5 > 2 doubles, 3 > 2 doubles, 1 > 2 stops. Two of them share a
    // tile, which proves the count is accumulated across the whole stack and
    // across all eight neighbours rather than stopping at the first.
    odds_place(VEH_NEIGH_D, P_NEIGH_LAND, ODDS_FACTION, ODDS_DEF_X + 2, ODDS_DEF_Y);
    odds_place(VEH_NEIGH_E, P_NEIGH_LAND, ODDS_FACTION, ODDS_DEF_X + 2, ODDS_DEF_Y);
    { const int ids[] = { VEH_NEIGH_A, VEH_NEIGH_D, VEH_NEIGH_E }; odds_link(ids, 3); }
    OCHECK(compute_odds(7, ODDS_FACTION, VEH_ATK, VEH_DEF, 0) == 56);

    // Sea units next door contribute nothing to a LAND count.
    g_odds_world.protos[P_NEIGH_LAND].chassis_id = CHS_SEA;
    OCHECK(compute_odds(7, ODDS_FACTION, VEH_ATK, VEH_DEF, 0) == 14);
    g_odds_world.protos[P_NEIGH_LAND].chassis_id = CHS_LAND;

    // ---- a garrison of zero skips the loop entirely ------------------------
    // Both defenders get a plan outside {DEFENSIVE, RECONNAISANCE, COMBAT} and
    // below PLAN_NAVAL_TRANSPORT, so nothing else moves. Without the guard the
    // subtraction would never terminate.
    g_odds_world.protos[P_DEF].plan = PLAN_AIR_SUPERIORITY;
    g_odds_world.protos[P_STACK].plan = PLAN_AIR_SUPERIORITY;
    OCHECK(compute_odds(7, ODDS_FACTION, VEH_ATK, VEH_DEF, 0) == 14);
    // PLAN_RECONNAISANCE is in the set and brings the loop back.
    g_odds_world.protos[P_DEF].plan = PLAN_RECONNAISANCE;
    g_odds_world.protos[P_STACK].plan = PLAN_DEFENSIVE;
    OCHECK(compute_odds(7, ODDS_FACTION, VEH_ATK, VEH_DEF, 0) == 56);

    // ---- the million clamp, at its exact boundary --------------------------
    // odds 500000 gives 20 * 500000 / 10 = 1000000 exactly, which is NOT below
    // the clamp, so the loop refuses to run even with five attackers.
    g_odds_world.protos[P_DEF].plan = PLAN_COMBAT;
    g_odds_world.protos[P_STACK].plan = PLAN_COMBAT;
    OCHECK(compute_odds(500000, ODDS_FACTION, VEH_ATK, VEH_DEF, 0) == 1000000);
    // One less: 20 * 499999 / 10 = 999998 is below it, so the loop doubles
    // exactly once and the next test stops it.
    OCHECK(compute_odds(499999, ODDS_FACTION, VEH_ATK, VEH_DEF, 0) == 1999996);

    // ---- the ground branch needs the base ----------------------------------
    // Field tile, same five neighbours, and none of it runs. The basis returns
    // to the field blend: ((32 - 20) / 5 + 20) * 7 / 10 = 15.
    OCHECK(compute_odds(7, ODDS_FACTION, VEH_ATK, VEH_DEF, -1) == 15);
}

/*
 * The tail: the air-superiority halving and the offensive-air thirds.
 */
void odds_tail() {
    // ---- an air-superiority attacker ---------------------------------------
    // Basis 15, halved to 7 - and 15 / 2 truncates, so 7 rather than 8.
    odds_reset();
    odds_attacker(P_ATK_AS);
    odds_defender(P_DEF);
    odds_place(VEH_STACK_A, P_STACK, ODDS_ENEMY, ODDS_DEF_X, ODDS_DEF_Y);
    { const int ids[] = { VEH_DEF, VEH_STACK_A }; odds_link(ids, 2); }
    OCHECK(compute_odds(7, ODDS_FACTION, VEH_ATK, VEH_DEF, -1) == 7);
    // An airborne target is what it is for, and keeps its full value.
    odds_veh(VEH_DEF).proto_id = (int16_t)P_DEF_AIR;
    OCHECK(compute_odds(7, ODDS_FACTION, VEH_ATK, VEH_DEF, -1) == 15);
    // So does an unarmed one - there is nothing there to intercept.
    odds_veh(VEH_DEF).proto_id = (int16_t)P_DEF;
    g_odds_world.protos[P_DEF].weapon_id = WPN_UNARMED;
    OCHECK(compute_odds(7, ODDS_FACTION, VEH_ATK, VEH_DEF, -1) == 15);
    g_odds_world.protos[P_DEF].weapon_id = WPN_ARMED;
    OCHECK(compute_odds(7, ODDS_FACTION, VEH_ATK, VEH_DEF, -1) == 7);
    // The two tests are on the DEFENDER's chassis and weapon, not the
    // attacker's - the attacker here is a land unit with no air chassis at all.
    OCHECK(g_odds_world.protos[P_ATK_AS].chassis_id == CHS_LAND);

    // ---- a plain offensive air unit ----------------------------------------
    // Basis 14; a third of it is 4, and two thirds is 9. Neither is a rounding
    // of the other, so the divisor and the numerator are both pinned.
    odds_reset();
    odds_attacker(P_ATK_AIROFF);
    odds_defender(P_DEF);
    OCHECK(compute_odds(7, ODDS_FACTION, VEH_ATK, VEH_DEF, -1) == 4);
    odds_player(ODDS_FACTION).region_base_plan[3] = 2;
    OCHECK(compute_odds(7, ODDS_FACTION, VEH_ATK, VEH_DEF, -1) == 9);
    odds_player(ODDS_FACTION).region_base_plan[3] = 1;     // one below
    OCHECK(compute_odds(7, ODDS_FACTION, VEH_ATK, VEH_DEF, -1) == 4);
    odds_player(ODDS_FACTION).region_base_plan[3] = 3;     // one above
    OCHECK(compute_odds(7, ODDS_FACTION, VEH_ATK, VEH_DEF, -1) == 4);
    odds_player(ODDS_FACTION).region_base_plan[3] = 0;

    // A missile is exempt.
    odds_attacker(P_ATK_MSLOFF);
    OCHECK(compute_odds(7, ODDS_FACTION, VEH_ATK, VEH_DEF, -1) == 14);
    // So is any air unit whose plan is not PLAN_OFFENSIVE.
    odds_attacker(P_ATK_AIR);                              // PLAN_DEFENSIVE
    OCHECK(compute_odds(7, ODDS_FACTION, VEH_ATK, VEH_DEF, -1) == 14);
    g_odds_world.protos[P_ATK_AIR].plan = PLAN_COMBAT;     // still not zero
    OCHECK(compute_odds(7, ODDS_FACTION, VEH_ATK, VEH_DEF, -1) == 14);
    // And so is a PLAN_OFFENSIVE unit that is not airborne. This one also
    // collects the "our ground" three halves on the way past, 14 -> 21.
    odds_attacker(P_ATK_LAND);
    g_odds_world.protos[P_ATK_LAND].plan = PLAN_OFFENSIVE;
    OCHECK(compute_odds(7, ODDS_FACTION, VEH_ATK, VEH_DEF, -1) == 21);

    // The air-superiority test comes FIRST: an air-superiority attacker that is
    // also airborne and not a missile still takes the halving, never a third.
    odds_reset();
    odds_attacker(P_ATK_AS);
    g_odds_world.protos[P_ATK_AS].chassis_id = CHS_AIR;
    odds_defender(P_DEF);
    OCHECK(compute_odds(7, ODDS_FACTION, VEH_ATK, VEH_DEF, -1) == 7);
}

/*
 * The round-map wrap, both ways, and the fixture guards.
 */
void odds_wrap_and_guards() {
    // ---- the negative wrap --------------------------------------------------
    // The defender sits at x 0. RadiusBase entry 5 is (-2, 0), which is off the
    // left edge on a flat map and 14 on a round one. The friendly land stack is
    // at 14, so the neighbour bonus fires only when the map is round.
    // Entry 4 is (-1, 1) -> 15 on a round map, and that tile stays empty, so it
    // cannot supply the bonus first.
    odds_reset();
    odds_attacker(P_ATK_AIR);
    odds_place(VEH_DEF, P_DEF, ODDS_ENEMY, 0, 4);
    odds_place(VEH_STACK_A, P_STACK, ODDS_ENEMY, 0, 4);
    { const int ids[] = { VEH_DEF, VEH_STACK_A }; odds_link(ids, 2); }
    odds_place(VEH_NEIGH_A, P_NEIGH_LAND, ODDS_FACTION, 14, 4);
    g_odds_world.is_flat = 1;
    OCHECK(compute_odds(7, ODDS_FACTION, VEH_ATK, VEH_DEF, 0) == 14);
    g_odds_world.is_flat = 0;
    OCHECK(compute_odds(7, ODDS_FACTION, VEH_ATK, VEH_DEF, 0) == 28);
    // Only bit zero of MapIsFlat is read, exactly as course_xrange does it.
    g_odds_world.is_flat = 2;
    OCHECK(compute_odds(7, ODDS_FACTION, VEH_ATK, VEH_DEF, 0) == 28);
    g_odds_world.is_flat = 3;
    OCHECK(compute_odds(7, ODDS_FACTION, VEH_ATK, VEH_DEF, 0) == 14);

    // ---- the positive wrap --------------------------------------------------
    // The defender at x 14; entry 1 is (2, 0), which is 16 - off the right edge
    // on a flat map and 0 on a round one.
    odds_reset();
    odds_attacker(P_ATK_AIR);
    odds_place(VEH_DEF, P_DEF, ODDS_ENEMY, 14, 4);
    odds_place(VEH_STACK_A, P_STACK, ODDS_ENEMY, 14, 4);
    { const int ids[] = { VEH_DEF, VEH_STACK_A }; odds_link(ids, 2); }
    odds_place(VEH_NEIGH_A, P_NEIGH_LAND, ODDS_FACTION, 0, 4);
    g_odds_world.is_flat = 1;
    OCHECK(compute_odds(7, ODDS_FACTION, VEH_ATK, VEH_DEF, 0) == 14);
    g_odds_world.is_flat = 0;
    OCHECK(compute_odds(7, ODDS_FACTION, VEH_ATK, VEH_DEF, 0) == 28);

    // ---- the top and bottom edges are never wrapped ------------------------
    // A defender on row 0: entry 0 is (1, -1) and entry 7 is (0, -2), both off
    // the map whatever MapIsFlat says.
    odds_reset();
    odds_attacker(P_ATK_AIR);
    odds_place(VEH_DEF, P_DEF, ODDS_ENEMY, 8, 0);
    odds_place(VEH_STACK_A, P_STACK, ODDS_ENEMY, 8, 0);
    { const int ids[] = { VEH_DEF, VEH_STACK_A }; odds_link(ids, 2); }
    odds_place(VEH_NEIGH_A, P_NEIGH_LAND, ODDS_FACTION, 8, ODDS_LAT_BOUNDS - 2);
    g_odds_world.is_flat = 0;
    OCHECK(compute_odds(7, ODDS_FACTION, VEH_ATK, VEH_DEF, 0) == 14);
    // But a real neighbour on row 2 still counts.
    odds_place(VEH_NEIGH_B, P_NEIGH_LAND, ODDS_FACTION, 8, 2);
    OCHECK(compute_odds(7, ODDS_FACTION, VEH_ATK, VEH_DEF, 0) == 28);

    // ---- the faction stride -------------------------------------------------
    // Every value the body reads out of PlayersData is read at 8396 bytes per
    // faction. Rows 0 and 2 carry the opposite of every one of them, so a wrong
    // stride cannot answer correctly by accident. Asking for faction 0 or 2
    // with the same units therefore gives a DIFFERENT answer, and that is the
    // whole point of them being poisoned rather than zeroed.
    odds_reset();
    odds_attacker(P_ATK_LAND);
    g_odds_world.protos[P_ATK_LAND].plan = PLAN_COMBAT;   // arms the region rule
    odds_defender(P_DEF);
    OCHECK(compute_odds(7, ODDS_FACTION, VEH_ATK, VEH_DEF, -1) == 21);   // plan 0 -> x3/2
    OCHECK(compute_odds(7, 0, VEH_ATK, VEH_DEF, -1) == 14);              // plan 2 -> no
    OCHECK(compute_odds(7, 2, VEH_ATK, VEH_DEF, -1) == 14);

    // ---- compute_odds writes nothing, anywhere -----------------------------
    // A whole-fixture comparison rather than a spot check: this covers the
    // underflow slot, the guard unit one past the roster, the guard prototype,
    // the guard tile and the two poison faction rows in one assertion.
    odds_reset();
    odds_attacker(P_ATK_AIR);
    odds_defender(P_DEF);
    odds_place(VEH_STACK_A, P_TERRA, ODDS_ENEMY, ODDS_DEF_X, ODDS_DEF_Y);
    { const int ids[] = { VEH_DEF, VEH_STACK_A }; odds_link(ids, 2); }
    odds_place(VEH_NEIGH_A, P_NEIGH_LAND, ODDS_FACTION, ODDS_DEF_X + 2, ODDS_DEF_Y);
    odds_veh(VEH_DEF).order = ORDER_TERRAFORM_UP;
    odds_player(ODDS_FACTION).proto_id_active[P_ATK_AIR] = 10;
    odds_drop_design(P_DROP);
    odds_goal(0, AI_GOAL_ATTACK, ODDS_DEF_X, ODDS_DEF_Y);
    {
        static OddsWorld before;
        std::memcpy(&before, &g_odds_world, sizeof(OddsWorld));
        const int answer = compute_odds(7, ODDS_FACTION, VEH_ATK, VEH_DEF, 0);
        OCHECK(std::memcmp(&before, &g_odds_world, sizeof(OddsWorld)) == 0);
        // The one case that stacks five of the doublings at once:
        //   20 * 7 / 10                                          =  14
        //   x2  garrison of 2 units, one of them unarmed         =  28
        //   x2  a terraformer in the stack, attacker not missile =  56
        //   x2  the defender's ORDER_TERRAFORM_UP                = 112
        //   x2  a friendly land stack next door                  = 224
        //   x2  the faction owns an active drop-pod design       = 448
        OCHECK(answer == 448);
    }
    // And the guards still hold their poison, stated separately so a failure
    // names the slot rather than the whole struct.
    OCHECK(odds_veh(ODDS_VEH_COUNT).x == ODDS_POISON16);
    OCHECK(odds_veh(ODDS_VEH_COUNT).faction_id == (uint8_t)ODDS_STRANGER);
    OCHECK(g_odds_world.protos[P_DROP_GUARD].cost == ODDS_POISON8);
    OCHECK(g_odds_world.tiles[ODDS_TILE_COUNT - 1].region == ODDS_POISON8);
    OCHECK(g_odds_world.chassis[4].triad == ODDS_POISON8);
    OCHECK(g_odds_world.weapons[2].offense_rating == (int8_t)0x77);
    OCHECK(odds_player(0).region_base_plan[3] == 2);
    OCHECK(odds_player(2).region_base_plan[3] == 2);
}

/*
 * The ends of the two eight-step neighbour loops, and the census the drop-pod
 * gate shares with the thin-garrison rule.
 *
 * The first sweep left four mutants of these alive because no case reached
 * them: none put a friendly unit on RadiusBase entry 0, which is (+1,-1), and
 * none made the noncombat census answer anything but zero.
 */
void odds_loop_edges() {
    // ---- the air loop reads RadiusBase entry 0 -----------------------------
    odds_reset();
    odds_attacker(P_ATK_AIR);
    odds_defender(P_DEF);
    odds_place(VEH_STACK_A, P_STACK, ODDS_ENEMY, ODDS_DEF_X, ODDS_DEF_Y);
    { const int ids[] = { VEH_DEF, VEH_STACK_A }; odds_link(ids, 2); }
    OCHECK(compute_odds(7, ODDS_FACTION, VEH_ATK, VEH_DEF, 0) == 14);
    odds_place(VEH_NEIGH_A, P_NEIGH_LAND, ODDS_FACTION, ODDS_DEF_X + 1, ODDS_DEF_Y - 1);
    OCHECK(compute_odds(7, ODDS_FACTION, VEH_ATK, VEH_DEF, 0) == 28);

    // ---- and so does the ground loop ---------------------------------------
    // Three of them stacked on that one tile, because the ground rule counts
    // rather than breaks and the garrison it divides by is two.
    odds_reset();
    odds_attacker(P_ATK_LAND);
    odds_defender(P_DEF);
    odds_place(VEH_STACK_A, P_STACK, ODDS_ENEMY, ODDS_DEF_X, ODDS_DEF_Y);
    { const int ids[] = { VEH_DEF, VEH_STACK_A }; odds_link(ids, 2); }
    odds_make_ocean(ODDS_DEF_X, ODDS_DEF_Y);
    OCHECK(compute_odds(7, ODDS_FACTION, VEH_ATK, VEH_DEF, 0) == 14);
    odds_place(VEH_NEIGH_A, P_NEIGH_LAND, ODDS_FACTION, ODDS_DEF_X + 1, ODDS_DEF_Y - 1);
    odds_place(VEH_NEIGH_B, P_NEIGH_LAND, ODDS_FACTION, ODDS_DEF_X + 1, ODDS_DEF_Y - 1);
    odds_place(VEH_NEIGH_C, P_NEIGH_LAND, ODDS_FACTION, ODDS_DEF_X + 1, ODDS_DEF_Y - 1);
    { const int ids[] = { VEH_NEIGH_A, VEH_NEIGH_B, VEH_NEIGH_C }; odds_link(ids, 3); }
    OCHECK(compute_odds(7, ODDS_FACTION, VEH_ATK, VEH_DEF, 0) == 28);

    // ---- the census the two garrison rules share ---------------------------
    // One armed defender and two unarmed passengers. Three minus two is one,
    // which is the thin-garrison threshold and also the drop-pod threshold
    // when the faction has none of the attacker's prototype in service, so a
    // noncombat count that answers zero loses BOTH doublings at once.
    odds_reset();
    odds_attacker(P_ATK_AIR);
    odds_defender(P_DEF);
    odds_place(VEH_STACK_A, P_ARTIFACT, ODDS_ENEMY, ODDS_DEF_X, ODDS_DEF_Y);
    odds_place(VEH_STACK_B, P_ARTIFACT, ODDS_ENEMY, ODDS_DEF_X, ODDS_DEF_Y);
    { const int ids[] = { VEH_DEF, VEH_STACK_A, VEH_STACK_B }; odds_link(ids, 3); }
    odds_drop_design(P_DROP);
    OCHECK(compute_odds(7, ODDS_FACTION, VEH_ATK, VEH_DEF, 0) == 56);
}

void test_compute_odds() {
    OddsSeams seams;
    odds_basis();
    odds_multipliers();
    odds_objective();
    odds_air();
    odds_air_neighbours();
    odds_ground();
    odds_tail();
    odds_wrap_and_guards();
    odds_loop_edges();
}
GAMEPLAY_CASE(test_compute_odds);

#undef OCHECK

/*
 * can_terraform (0x00565320).
 *
 * The subject tile is always (8,4) and the faction is always 1. There are two
 * base worlds, because the body splits on altitude and shares almost nothing
 * across the split: ct_reset() lays down arid flat land one level above the
 * shore line, ct_make_ocean() rewrites the same map as ocean shelf.
 *
 * FIVE THINGS ABOUT THE FIXTURE ARE LOAD-BEARING.
 *
 *  - `Bases` is seamed to &bases[1], NOT &bases[0], exactly as YieldWorld does
 *    it. can_terraform guards every Bases[] read with base_id >= 0, but it
 *    calls crop_yield(faction, -1, x, y, 0), and crop_yield's aquafarm read is
 *    unguarded. bases[0] is the Bases[-1] entry and is left poisoned.
 *
 *  - EVERY Terraforming[] entry carries a DIFFERENT prerequisite technology,
 *    40 + 2*i on land and 41 + 2*i at sea, and the faction starts holding NONE
 *    of them. A wrong index or a swapped land/sea field therefore asks about a
 *    technology the faction does not have and the answer changes, instead of
 *    coinciding with the right one. Index MaxTerrainNum - one past the last
 *    valid action - is the GUARD: its two technologies are the only ones the
 *    faction always holds, so a read one entry past the end turns a refusal
 *    into an order.
 *
 *  - The tiles OUTSIDE the 8x8 live window are poisoned with every bit set and
 *    an ocean-shelf altitude, so a dropped on_map() guard in the base-radius
 *    census counts improvements that are not there.
 *
 *  - Every Base except the live one is poisoned with all three planning bits,
 *    all facilities and a nonsense surplus; bases[9] is the guard one past the
 *    last id any case uses, and is asserted untouched.
 *
 *  - MapRandSeed is 0, which keeps bonus_at() off its pseudo-random path, so a
 *    resource bonus is decided by BIT_NUTRIENT_RSC / BIT_MINERAL_RSC /
 *    BIT_ENERGY_RSC alone and is exact rather than positional.
 */
struct CtWorld {
    Map tiles[192];
    Base bases[10];
    PlayerData players_data[9];
    Player players[9];
    RulesResourceinfo resource_info[MaxResourceInfoNum];
    RulesTerraforming terraforming[MaxTerrainNum + 1];
    RulesBasic rules;
    BaseSecretProject projects;
    RulesTechnology technology[MaxTechnologyNum];
    uint8_t tech_achieved[MaxTechnologyNum];
    uint8_t factions_status[2];
    Map *tiles_ptr;
    Base *base_current;
    uint32_t longitude;
    int lon_bounds;
    int lat_bounds;
    BOOL is_flat;
    uint32_t map_rand_seed;
    uint32_t game_rules;
    uint32_t game_state;
    uint32_t preferences;
    uint32_t more_preferences;
    int turn_current;
    int local_faction;
    BOOL expansion;
    int dust_cloud;
    int restricted;
    int base_square_energy;
    int governor;
    int energy_event;
    int energy_event_selector;
};

CtWorld g_ct_world;

const int CT_LIVE = 64;          // the live window starts here; 64 tiles follow
const int CT_FACTION = 1;        // has_tech() refuses faction 0, so 1 is the floor
const int CT_X = 8;
const int CT_Y = 4;
const int CT_POISON = 0x77;
const int CT_TECH_THREE_NUTRIENTS = 5;

// Terraforming[i]'s two prerequisites. Distinct per entry and per field.
int ct_preq(int terraform_id) { return 40 + 2 * terraform_id; }
int ct_preq_sea(int terraform_id) { return 41 + 2 * terraform_id; }

const uint8_t CT_ALT_LAND = 0x60;    // ALT_BIT_SHORE_LINE, alt level 3, elev 1
const uint8_t CT_ALT_SHELF = 0x40;   // ALT_BIT_OCEAN_SHELF
const uint8_t CT_ALT_DEEP = 0x20;    // ALT_BIT_OCEAN

// Position-independent resource bonuses; see the fixture note.
const uint32_t CT_BONUS_NUTRIENT = BIT_RSC_BONUS | BIT_NUTRIENT_RSC;
const uint32_t CT_BONUS_MINERAL = BIT_RSC_BONUS | BIT_NUTRIENT_RSC | BIT_MINERAL_RSC;
const uint32_t CT_BONUS_ENERGY = BIT_RSC_BONUS | BIT_NUTRIENT_RSC | BIT_ENERGY_RSC;

Map &ct_at(int x, int y) {
    return g_ct_world.tiles[CT_LIVE + (x >> 1) + y * 8];
}

Map &ct_subject() { return ct_at(CT_X, CT_Y); }

Base &ct_base(int base_id) { return g_ct_world.bases[base_id + 1]; }

void ct_grant(int tech_id) {
    g_ct_world.tech_achieved[tech_id] = (uint8_t)(1 << CT_FACTION);
}

void ct_give_fac(int base_id, uint32_t facility) {
    int offset;
    int mask;
    bitmask(facility, &offset, &mask);
    ct_base(base_id).facilities_built[offset] |= (uint8_t)mask;
}

void ct_make_human() {
    g_ct_world.factions_status[0] = (uint8_t)(1 << CT_FACTION);
}

void ct_reset() {
    std::memset(&g_ct_world, 0, sizeof(g_ct_world));
    std::memset(&g_ct_world.projects, 0xFF, sizeof(g_ct_world.projects));
    g_ct_world.tiles_ptr = &g_ct_world.tiles[CT_LIVE];
    g_ct_world.base_current = &g_ct_world.bases[1];
    g_ct_world.longitude = 8;
    g_ct_world.lon_bounds = 16;
    g_ct_world.lat_bounds = 8;
    g_ct_world.is_flat = 1;
    g_ct_world.map_rand_seed = 0;
    g_ct_world.game_state = STATE_OMNISCIENT_VIEW;  // whose_territory reports the real owner
    g_ct_world.turn_current = 100;                  // past the < 50 plant bonus
    g_ct_world.local_faction = CT_FACTION;
    g_ct_world.governor = -1;

    // Poison the whole map, then lay the live window down over it. An off-map
    // read lands on a tile with every improvement on it.
    for (int k = 0; k < 192; k++) {
        g_ct_world.tiles[k].climate = CT_ALT_SHELF;
        g_ct_world.tiles[k].bit = 0xFFFFFFFF;
        g_ct_world.tiles[k].territory = (int8_t)0x7;
    }
    for (int k = 0; k < 64; k++) {
        Map &tile = g_ct_world.tiles[CT_LIVE + k];
        tile.climate = CT_ALT_LAND;   // arid, flat, one level above the shore
        tile.bit = 0;
        tile.bit2 = 0;
        tile.val2 = 0;
        tile.val3 = 0;
        tile.territory = (int8_t)CT_FACTION;
    }

    // Every base poisoned; the live one (id 0) then cleared.
    for (int b = 0; b < 10; b++) {
        Base &base = g_ct_world.bases[b];
        base.x = (int16_t)0x777;
        base.y = (int16_t)0x777;
        base.state = 0xFFFFFFFF;
        base.eco_damage = CT_POISON;
        base.nutrient_surplus = CT_POISON;
        std::memset(base.facilities_built, 0xFF, sizeof(base.facilities_built));
    }
    Base &live = ct_base(0);
    live.x = (int16_t)CT_X;
    live.y = (int16_t)CT_Y;
    live.state = 0;
    live.eco_damage = 0;
    live.nutrient_surplus = 0;
    std::memset(live.facilities_built, 0, sizeof(live.facilities_built));

    // Distinct prerequisites, none of them held. The guard one past the end is
    // the only entry whose technologies the faction always has.
    for (int i = 0; i <= MaxTerrainNum; i++) {
        g_ct_world.terraforming[i].preq_tech = ct_preq(i);
        g_ct_world.terraforming[i].preq_tech_sea = ct_preq_sea(i);
    }
    ct_grant(ct_preq(MaxTerrainNum));
    ct_grant(ct_preq_sea(MaxTerrainNum));

    // Poison every resource row, then pin the three the subject and crop_yield
    // actually read so the comparisons below stay legible.
    for (int i = 0; i < MaxResourceInfoNum; i++) {
        g_ct_world.resource_info[i].nutrients = 40 + 3 * i;
        g_ct_world.resource_info[i].minerals = 41 + 3 * i;
        g_ct_world.resource_info[i].energy = 42 + 3 * i;
    }
    g_ct_world.resource_info[RSCINFO_FOREST_SQ].nutrients = 2;
    g_ct_world.resource_info[RSCINFO_FOREST_SQ].minerals = 3;
    g_ct_world.resource_info[RSCINFO_FOREST_SQ].energy = 4;   // forest_value = 9
    g_ct_world.resource_info[RSCINFO_IMPROVED_LAND].nutrients = 1;
    g_ct_world.resource_info[RSCINFO_BONUS_SQ].nutrients = 2;

    g_ct_world.rules.tgl_nutrient_effect_with_mine = -1;   // the alphax.txt default
    g_ct_world.rules.tech_three_nutrients_sqr = CT_TECH_THREE_NUTRIENTS;

    // fungus_value = 3 by default; poisoned rather than zero so that a dropped
    // term in the three-way sum is visible.
    g_ct_world.players_data[CT_FACTION].tech_fungus_nutrient = 1;
    g_ct_world.players_data[CT_FACTION].tech_fungus_mineral = 1;
    g_ct_world.players_data[CT_FACTION].tech_fungus_energy = 1;

    // The guard element one past the last faction id. A stride error in the
    // PlayersData arithmetic reads this and every answer moves.
    g_ct_world.players_data[8].tech_fungus_nutrient = 0x7777;
    g_ct_world.players_data[8].tech_fungus_mineral = 0x7777;
    g_ct_world.players_data[8].tech_fungus_energy = 0x7777;
    g_ct_world.players_data[8].ai_growth = 1;
    g_ct_world.players_data[8].ai_fight = -1;
    for (int t = 0; t < 8; t++) {
        g_ct_world.players_data[8].diplo_treaties[t] = 0x77777777;
    }
}

// Turn the live window into ocean shelf.
void ct_make_ocean() {
    for (int k = 0; k < 64; k++) {
        g_ct_world.tiles[CT_LIVE + k].climate = CT_ALT_SHELF;
    }
}

void ct_set_forest_value(int nutrients, int minerals, int energy) {
    g_ct_world.resource_info[RSCINFO_FOREST_SQ].nutrients = nutrients;
    g_ct_world.resource_info[RSCINFO_FOREST_SQ].minerals = minerals;
    g_ct_world.resource_info[RSCINFO_FOREST_SQ].energy = energy;
}

void ct_set_fungus_value(int total) {
    g_ct_world.players_data[CT_FACTION].tech_fungus_nutrient = total;
    g_ct_world.players_data[CT_FACTION].tech_fungus_mineral = 0;
    g_ct_world.players_data[CT_FACTION].tech_fungus_energy = 0;
}

#define CTCHECK(cond)                                                         \
    do {                                                                      \
        const bool ct_ok = (cond);                                            \
        if (!ct_ok) {                                                         \
            std::fprintf(stderr, "can_terraform: line %d: %s\n", __LINE__,    \
                         #cond);                                              \
        }                                                                     \
        expect(ct_ok);                                                        \
    } while (0)

// Every poison guard, checked at the end of each case group. can_terraform
// writes nothing, so any change here is a stray store.
#define CT_GUARDS()                                                                   \
    do {                                                                              \
        CTCHECK(g_ct_world.terraforming[MaxTerrainNum].preq_tech                      \
                == ct_preq(MaxTerrainNum));                                           \
        CTCHECK(g_ct_world.terraforming[MaxTerrainNum].preq_tech_sea                  \
                == ct_preq_sea(MaxTerrainNum));                                       \
        CTCHECK(g_ct_world.players_data[8].tech_fungus_nutrient == 0x7777);           \
        CTCHECK(g_ct_world.players_data[8].diplo_treaties[7] == 0x77777777u);         \
        CTCHECK(g_ct_world.bases[9].nutrient_surplus == CT_POISON);                   \
        CTCHECK(g_ct_world.bases[9].state == 0xFFFFFFFFu);                            \
        CTCHECK(g_ct_world.tiles[0].bit == 0xFFFFFFFFu);                              \
        CTCHECK(g_ct_world.tiles[191].bit == 0xFFFFFFFFu);                            \
    } while (0)

class CtSeams {
 public:
    CtSeams()
        : tiles_(&MapTiles, &g_ct_world.tiles_ptr),
          longitude_(&MapLongitude, &g_ct_world.longitude),
          lon_(&MapLongitudeBounds, &g_ct_world.lon_bounds),
          lat_(&MapLatitudeBounds, &g_ct_world.lat_bounds),
          flat_(&MapIsFlat, &g_ct_world.is_flat),
          seed_(&MapRandSeed, &g_ct_world.map_rand_seed),
          game_rules_(&GameRules, &g_ct_world.game_rules),
          game_state_(&GameState, &g_ct_world.game_state),
          prefs_(&GamePreferences, &g_ct_world.preferences),
          more_prefs_(&GameMorePreferences, &g_ct_world.more_preferences),
          turn_(&TurnCurrentNum, &g_ct_world.turn_current),
          local_(&LocalFaction, &g_ct_world.local_faction),
          status_(&FactionsStatus, g_ct_world.factions_status),
          bases_(&Bases, &g_ct_world.bases[1]),
          base_current_(&BaseCurrent, &g_ct_world.base_current),
          projects_(&SecretProject, &g_ct_world.projects),
          resource_(&ResourceInfo, g_ct_world.resource_info),
          terraforming_(&Terraforming, g_ct_world.terraforming),
          rules_(&Rules, &g_ct_world.rules),
          players_data_(&PlayersData, g_ct_world.players_data),
          players_(&Players, g_ct_world.players),
          technology_(&Technology, g_ct_world.technology),
          achieved_(&GameTechAchieved, g_ct_world.tech_achieved),
          expansion_(&ExpansionEnabled, &g_ct_world.expansion),
          dust_(&DustCloudDuration, &g_ct_world.dust_cloud),
          restricted_(&TileYieldRestricted, &g_ct_world.restricted),
          base_energy_(&BaseSquareEnergy, &g_ct_world.base_square_energy),
          governor_(&GovernorFaction, &g_ct_world.governor),
          energy_event_(&GlobalEnergyEventState, &g_ct_world.energy_event),
          selector_(&UnkGlobal0093A934, &g_ct_world.energy_event_selector) { }

 private:
    ScopedSeam<Map *> tiles_;
    ScopedSeam<uint32_t> longitude_;
    ScopedSeam<int> lon_;
    ScopedSeam<int> lat_;
    ScopedSeam<BOOL> flat_;
    ScopedSeam<uint32_t> seed_;
    ScopedSeam<uint32_t> game_rules_;
    ScopedSeam<uint32_t> game_state_;
    ScopedSeam<uint32_t> prefs_;
    ScopedSeam<uint32_t> more_prefs_;
    ScopedSeam<int> turn_;
    ScopedSeam<int> local_;
    ScopedSeam<uint8_t> status_;
    ScopedSeam<Base> bases_;
    ScopedSeam<Base *> base_current_;
    ScopedSeam<BaseSecretProject> projects_;
    ScopedSeam<RulesResourceinfo> resource_;
    ScopedSeam<RulesTerraforming> terraforming_;
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

/*
 * The fixture's own arithmetic, pinned before anything depends on it. If
 * crop_yield() or bonus_at() ever move, these fail first and the rest of the
 * file stops lying about why.
 */
void test_can_terraform_gates() {
    CtSeams seams;

    // ---- fixture self-check -------------------------------------------------
    ct_reset();
    CTCHECK(crop_yield(CT_FACTION, -1, CT_X, CT_Y, 0) == 0);      // arid flat land
    CTCHECK(bonus_at(CT_X, CT_Y, 0) == 0);                        // no bonus bit, seed 0
    ct_subject().climate = (uint8_t)(CT_ALT_LAND | RAINFALL_MOIST);
    CTCHECK(crop_yield(CT_FACTION, -1, CT_X, CT_Y, 0) == 1);
    ct_subject().climate = (uint8_t)(CT_ALT_LAND | RAINFALL_RAINY);
    CTCHECK(crop_yield(CT_FACTION, -1, CT_X, CT_Y, 0) == 2);
    ct_reset();
    ct_subject().bit = CT_BONUS_NUTRIENT;
    CTCHECK(bonus_at(CT_X, CT_Y, 0) == 1);
    ct_subject().bit = CT_BONUS_MINERAL;
    CTCHECK(bonus_at(CT_X, CT_Y, 0) == 2);
    ct_subject().bit = CT_BONUS_ENERGY;
    CTCHECK(bonus_at(CT_X, CT_Y, 0) == 3);

    // ---- a base in the tile refuses, for owners 0..7 only -------------------
    // The three-nutrients technology is granted so that the refusal is
    // OBSERVABLE. BIT_BASE_IN_TILE with an owner in range also sends crop_yield
    // down its base-square arm, which returns ResourceInfo[RSCINFO_BASE_SQ]
    // rather than the terrain's own nutrients - 43 here. Without the technology
    // that large crop fails the farm gate on its own, so a body that forgot to
    // refuse would answer ORDER_NONE anyway and every mutant of the guard would
    // look equivalent. With it, not refusing means ORDER_FARM.
    ct_reset();
    ct_grant(ct_preq(TERRA_FARM));
    ct_grant(CT_TECH_THREE_NUTRIENTS);
    CTCHECK(crop_yield(CT_FACTION, -1, CT_X, CT_Y, 0) == 0);
    CTCHECK(can_terraform(CT_FACTION, CT_X, CT_Y, 0, -1) == ORDER_FARM);
    ct_subject().bit |= BIT_BASE_IN_TILE;
    CTCHECK(crop_yield(CT_FACTION, -1, CT_X, CT_Y, 0) == 43);  // the base-square arm
    ct_subject().val2 = 0;                       // owner 0 is still an owner
    CTCHECK(can_terraform(CT_FACTION, CT_X, CT_Y, 0, -1) == ORDER_NONE);
    ct_subject().val2 = 7;
    CTCHECK(can_terraform(CT_FACTION, CT_X, CT_Y, 0, -1) == ORDER_NONE);
    ct_subject().val2 = 8;                       // 8 is out of the 0..7 range
    CTCHECK(can_terraform(CT_FACTION, CT_X, CT_Y, 0, -1) == ORDER_FARM);
    ct_subject().val2 = 0xF;                     // unoccupied
    CTCHECK(can_terraform(CT_FACTION, CT_X, CT_Y, 0, -1) == ORDER_FARM);
    ct_subject().val2 = 0xF0;                    // only the low nibble is the owner, so 0
    CTCHECK(can_terraform(CT_FACTION, CT_X, CT_Y, 0, -1) == ORDER_NONE);
    // And without the bit the nibble is not consulted at all.
    ct_subject().bit = 0;
    ct_subject().val2 = 1;
    CTCHECK(can_terraform(CT_FACTION, CT_X, CT_Y, 0, -1) == ORDER_FARM);

    // ---- foreign territory needs a pact -------------------------------------
    ct_reset();
    ct_grant(ct_preq(TERRA_FARM));
    ct_subject().territory = 2;
    CTCHECK(can_terraform(CT_FACTION, CT_X, CT_Y, 0, -1) == ORDER_NONE);
    g_ct_world.players_data[CT_FACTION].diplo_treaties[2] = DTREATY_PACT;
    CTCHECK(can_terraform(CT_FACTION, CT_X, CT_Y, 0, -1) == ORDER_FARM);
    g_ct_world.players_data[CT_FACTION].diplo_treaties[2] = DTREATY_TREATY;  // not a pact
    CTCHECK(can_terraform(CT_FACTION, CT_X, CT_Y, 0, -1) == ORDER_NONE);
    // whose_territory answers -1 for an unowned tile, and -1 is not "foreign".
    ct_subject().territory = 0;
    CTCHECK(can_terraform(CT_FACTION, CT_X, CT_Y, 0, -1) == ORDER_FARM);
    ct_subject().territory = -1;
    CTCHECK(can_terraform(CT_FACTION, CT_X, CT_Y, 0, -1) == ORDER_FARM);
    // Our own territory never asks about treaties.
    ct_subject().territory = (int8_t)CT_FACTION;
    g_ct_world.players_data[CT_FACTION].diplo_treaties[CT_FACTION] = 0;
    CTCHECK(can_terraform(CT_FACTION, CT_X, CT_Y, 0, -1) == ORDER_FARM);

    // ---- a monolith refuses --------------------------------------------------
    ct_reset();
    ct_grant(ct_preq(TERRA_FARM));
    ct_subject().bit |= BIT_MONOLITH;
    CTCHECK(can_terraform(CT_FACTION, CT_X, CT_Y, 0, -1) == ORDER_NONE);

    // ---- an existing forest worth more than the terrain refuses --------------
    ct_reset();
    ct_grant(ct_preq(TERRA_FARM));
    ct_subject().bit |= BIT_FOREST;
    CTCHECK(can_terraform(CT_FACTION, CT_X, CT_Y, 0, -1) == ORDER_NONE);  // value 2 <= 9
    ct_set_forest_value(0, 0, 0);
    CTCHECK(can_terraform(CT_FACTION, CT_X, CT_Y, 0, -1) == ORDER_FARM);  // value 2 > 0

    CT_GUARDS();
}
GAMEPLAY_CASE(test_can_terraform_gates);

/*
 * Water. Deep water is only ever raised, the shelf is farmed, mined or
 * harnessed, and fungus on the shelf is removed.
 */
void test_can_terraform_sea() {
    CtSeams seams;

    // ---- below the shelf, only ORDER_TERRAFORM_UP ---------------------------
    ct_reset();
    ct_make_ocean();
    ct_subject().climate = CT_ALT_DEEP;
    ct_grant(ct_preq_sea(TERRA_FARM));            // irrelevant down here
    CTCHECK(can_terraform(CT_FACTION, CT_X, CT_Y, 0, -1) == ORDER_NONE);
    ct_grant(ct_preq_sea(TERRA_RAISE_LAND));
    CTCHECK(can_terraform(CT_FACTION, CT_X, CT_Y, 0, -1) == ORDER_TERRAFORM_UP);
    // Scenario rules can forbid it outright, through terrain_avail.
    g_ct_world.game_rules = RULES_SCN_NO_TERRAFORMING;
    CTCHECK(can_terraform(CT_FACTION, CT_X, CT_Y, 0, -1) == ORDER_NONE);
    g_ct_world.game_rules = 0;
    // Fungus in deep water is not "fungus": the altitude test is on the tile.
    ct_subject().bit |= BIT_FUNGUS;
    ct_grant(ct_preq_sea(TERRA_REMOVE_FUNGUS));
    CTCHECK(can_terraform(CT_FACTION, CT_X, CT_Y, 0, -1) == ORDER_TERRAFORM_UP);

    // ---- fungus on the shelf ------------------------------------------------
    ct_reset();
    ct_make_ocean();
    ct_subject().bit |= BIT_FUNGUS;
    CTCHECK(can_terraform(CT_FACTION, CT_X, CT_Y, 0, -1) == ORDER_NONE);  // no technology
    ct_grant(ct_preq_sea(TERRA_REMOVE_FUNGUS));
    CTCHECK(can_terraform(CT_FACTION, CT_X, CT_Y, 0, -1) == ORDER_REMOVE_FUNGUS);
    // The land prerequisite is NOT the one consulted - see the bug note.
    ct_reset();
    ct_make_ocean();
    ct_subject().bit |= BIT_FUNGUS;
    ct_grant(ct_preq(TERRA_REMOVE_FUNGUS));
    CTCHECK(can_terraform(CT_FACTION, CT_X, CT_Y, 0, -1) == ORDER_NONE);
    // Fungus worth four or more is left alone.
    ct_reset();
    ct_make_ocean();
    ct_subject().bit |= BIT_FUNGUS;
    ct_grant(ct_preq_sea(TERRA_REMOVE_FUNGUS));
    ct_set_fungus_value(3);
    CTCHECK(can_terraform(CT_FACTION, CT_X, CT_Y, 0, -1) == ORDER_REMOVE_FUNGUS);
    ct_set_fungus_value(4);
    CTCHECK(can_terraform(CT_FACTION, CT_X, CT_Y, 0, -1) == ORDER_NONE);
    // A human has to have asked for automatic fungus removal.
    ct_set_fungus_value(3);
    ct_make_human();
    CTCHECK(can_terraform(CT_FACTION, CT_X, CT_Y, 0, -1) == ORDER_NONE);
    g_ct_world.more_preferences = MPREF_AUTO_FORMER_REMOVE_FUNGUS;
    CTCHECK(can_terraform(CT_FACTION, CT_X, CT_Y, 0, -1) == ORDER_REMOVE_FUNGUS);

    // ---- a thermal borehole on the shelf refuses before anything else -------
    ct_reset();
    ct_make_ocean();
    ct_grant(ct_preq_sea(TERRA_FARM));
    CTCHECK(can_terraform(CT_FACTION, CT_X, CT_Y, 0, -1) == ORDER_FARM);
    ct_subject().bit |= BIT_THERMAL_BORE;
    CTCHECK(can_terraform(CT_FACTION, CT_X, CT_Y, 0, -1) == ORDER_NONE);
    // An echelon mirror is "special" but is not the bit that refuses here.
    ct_subject().bit = BIT_ECH_MIRROR;
    CTCHECK(can_terraform(CT_FACTION, CT_X, CT_Y, 0, -1) == ORDER_FARM);

    // ---- the kelp farm and its enricher upgrade -----------------------------
    ct_reset();
    ct_make_ocean();
    ct_subject().bit |= BIT_FARM;
    ct_grant(ct_preq_sea(TERRA_FARM));            // already farmed, so this does nothing
    CTCHECK(can_terraform(CT_FACTION, CT_X, CT_Y, 0, -1) == ORDER_NONE);
    ct_grant(ct_preq_sea(TERRA_SOIL_ENR));
    CTCHECK(can_terraform(CT_FACTION, CT_X, CT_Y, 0, -1) == ORDER_FARM);
    ct_subject().bit |= BIT_SOIL_ENRICHER;        // already upgraded
    CTCHECK(can_terraform(CT_FACTION, CT_X, CT_Y, 0, -1) == ORDER_NONE);

    // ---- the two base planning bits, and the nutrient-effect rule -----------
    // No 0x2000: the mining platform is only reached by the tail, which wants a
    // nutrient surplus above one.
    ct_reset();
    ct_make_ocean();
    ct_grant(ct_preq_sea(TERRA_MINE));
    ct_base(0).state = BSTATE_UNK_4000;
    ct_base(0).nutrient_surplus = 0;
    CTCHECK(can_terraform(CT_FACTION, CT_X, CT_Y, 0, 0) == ORDER_NONE);
    ct_base(0).state = BSTATE_UNK_4000 | BSTATE_UNK_2000;
    CTCHECK(can_terraform(CT_FACTION, CT_X, CT_Y, 0, 0) == ORDER_MINE);

    // 0x1000 picks the tidal harness ahead of everything the tail would do.
    ct_reset();
    ct_make_ocean();
    ct_grant(ct_preq_sea(TERRA_MINE));
    ct_grant(ct_preq_sea(TERRA_SOLAR_TIDAL));
    g_ct_world.rules.tgl_nutrient_effect_with_mine = 0;   // suppresses the third rule
    ct_base(0).state = 0;
    ct_base(0).nutrient_surplus = 2;
    CTCHECK(can_terraform(CT_FACTION, CT_X, CT_Y, 0, 0) == ORDER_MINE);
    ct_base(0).state = BSTATE_UNK_1000;
    CTCHECK(can_terraform(CT_FACTION, CT_X, CT_Y, 0, 0) == ORDER_SOLAR_COLLECTOR);

    // The third rule: a hungry base with the negative nutrient effect harnesses.
    ct_reset();
    ct_make_ocean();
    ct_grant(ct_preq_sea(TERRA_SOLAR_TIDAL));
    ct_base(0).state = 0;
    ct_base(0).nutrient_surplus = 0;              // below two
    CTCHECK(can_terraform(CT_FACTION, CT_X, CT_Y, 0, 0) == ORDER_SOLAR_COLLECTOR);
    g_ct_world.rules.tgl_nutrient_effect_with_mine = 0;
    ct_base(0).nutrient_surplus = 0;
    // Now the third rule is off; the census still says harness, because the
    // seeded mine count is 0 and the missing platform technology adds one.
    CTCHECK(can_terraform(CT_FACTION, CT_X, CT_Y, 0, 0) == ORDER_SOLAR_COLLECTOR);

    // ---- the base-radius census, and both MapIsFlat states ------------------
    // The base sits at x == 0. Three of its radius offsets are negative, and on
    // a round map they wrap onto column 14/15 where the harnesses are; on a flat
    // map they fall off the edge and are not counted.
    ct_reset();
    ct_make_ocean();
    ct_grant(ct_preq_sea(TERRA_SOLAR_TIDAL));     // deliberately NOT the platform
    ct_base(0).x = 0;
    ct_base(0).y = 4;
    ct_base(0).state = 0;
    ct_base(0).nutrient_surplus = 2;              // above one, so the third rule is off
    ct_at(14, 4).bit |= BIT_SOLAR_TIDAL;          // radius offset 6  (-2, 0)
    ct_at(14, 6).bit |= BIT_SOLAR_TIDAL;          // radius offset 11 (-2, +2)
    ct_at(14, 2).bit |= BIT_SOLAR_TIDAL;          // radius offset 12 (-2, -2)
    g_ct_world.is_flat = 1;
    CTCHECK(can_terraform(CT_FACTION, CT_X, CT_Y, 0, 0) == ORDER_SOLAR_COLLECTOR);
    g_ct_world.is_flat = 0;                       // round: the three now count
    CTCHECK(can_terraform(CT_FACTION, CT_X, CT_Y, 0, 0) == ORDER_NONE);
    // Only the low bit of MapIsFlat is read.
    g_ct_world.is_flat = 2;
    CTCHECK(can_terraform(CT_FACTION, CT_X, CT_Y, 0, 0) == ORDER_NONE);
    g_ct_world.is_flat = 3;
    CTCHECK(can_terraform(CT_FACTION, CT_X, CT_Y, 0, 0) == ORDER_SOLAR_COLLECTOR);
    // The census only counts water. Raising those three above the shore line
    // takes them out of it again.
    g_ct_world.is_flat = 0;
    ct_at(14, 4).climate = CT_ALT_LAND;
    ct_at(14, 6).climate = CT_ALT_LAND;
    ct_at(14, 2).climate = CT_ALT_LAND;
    CTCHECK(can_terraform(CT_FACTION, CT_X, CT_Y, 0, 0) == ORDER_SOLAR_COLLECTOR);

    // ---- the tail mine, and what stops it -----------------------------------
    ct_reset();
    ct_make_ocean();
    ct_grant(ct_preq_sea(TERRA_MINE));
    ct_base(0).state = 0;
    ct_base(0).nutrient_surplus = 2;
    CTCHECK(can_terraform(CT_FACTION, CT_X, CT_Y, 0, 0) == ORDER_MINE);
    ct_base(0).nutrient_surplus = 1;              // not above one
    CTCHECK(can_terraform(CT_FACTION, CT_X, CT_Y, 0, 0) == ORDER_NONE);
    ct_base(0).nutrient_surplus = 2;
    ct_base(0).state = BSTATE_UNK_4000;
    CTCHECK(can_terraform(CT_FACTION, CT_X, CT_Y, 0, 0) == ORDER_NONE);
    ct_base(0).state = 0;
    g_ct_world.rules.tgl_nutrient_effect_with_mine = 0;   // the surplus stops mattering
    ct_base(0).nutrient_surplus = 0;
    CTCHECK(can_terraform(CT_FACTION, CT_X, CT_Y, 0, 0) == ORDER_MINE);
    // With no base at all the surplus branch cannot fire, so only the rule can.
    CTCHECK(can_terraform(CT_FACTION, CT_X, CT_Y, 0, -1) == ORDER_MINE);
    g_ct_world.rules.tgl_nutrient_effect_with_mine = -1;
    CTCHECK(can_terraform(CT_FACTION, CT_X, CT_Y, 0, -1) == ORDER_NONE);

    // ---- an improvement already on the tile skips the whole block -----------
    ct_reset();
    ct_make_ocean();
    ct_grant(ct_preq_sea(TERRA_MINE));
    g_ct_world.rules.tgl_nutrient_effect_with_mine = 0;
    CTCHECK(can_terraform(CT_FACTION, CT_X, CT_Y, 0, -1) == ORDER_MINE);
    ct_subject().bit = BIT_MINE;
    CTCHECK(can_terraform(CT_FACTION, CT_X, CT_Y, 0, -1) == ORDER_NONE);
    ct_subject().bit = BIT_SOLAR_TIDAL;
    CTCHECK(can_terraform(CT_FACTION, CT_X, CT_Y, 0, -1) == ORDER_NONE);
    ct_subject().bit = BIT_CONDENSER;             // "special"
    CTCHECK(can_terraform(CT_FACTION, CT_X, CT_Y, 0, -1) == ORDER_NONE);
    ct_subject().bit = 0;
    ct_subject().bit2 = BIT2_VOLCANO;             // the volcano's own tile is special
    CTCHECK(can_terraform(CT_FACTION, CT_X, CT_Y, 0, -1) == ORDER_NONE);
    ct_subject().bit2 = BIT2_VOLCANO | BIT2_UNK_80000000;
    CTCHECK(can_terraform(CT_FACTION, CT_X, CT_Y, 0, -1) == ORDER_MINE);
    ct_subject().bit2 = BIT2_VOLCANO | 0x01000000u;   // a nonzero landmark code
    CTCHECK(can_terraform(CT_FACTION, CT_X, CT_Y, 0, -1) == ORDER_MINE);

    // ---- the three tests of the hungry-base rule, one at a time -------------
    // The census behind it has to be made to REFUSE first, or it answers
    // harness on its own and the rule above it cannot be seen.
    // (10,4) is radius offset 2 from a base at (8,4) and (6,4) is offset 6;
    // no other offset in 0..20 lands on either tile at row 4.
    ct_reset();
    ct_make_ocean();
    ct_grant(ct_preq_sea(TERRA_SOLAR_TIDAL));     // deliberately NOT the platform
    ct_base(0).state = 0;
    ct_base(0).nutrient_surplus = 0;              // below two
    ct_at(10, 4).bit |= BIT_SOLAR_TIDAL;
    ct_at(6, 4).bit |= BIT_SOLAR_TIDAL;           // census: 1 + 1 > 2 is false
    CTCHECK(can_terraform(CT_FACTION, CT_X, CT_Y, 0, 0) == ORDER_SOLAR_COLLECTOR);
    ct_base(0).nutrient_surplus = 2;              // no longer hungry
    CTCHECK(can_terraform(CT_FACTION, CT_X, CT_Y, 0, 0) == ORDER_NONE);

    // The negative nutrient effect is the third of the three, and it is asked
    // for strictly - zero does not satisfy it.
    ct_reset();
    ct_make_ocean();
    ct_grant(ct_preq_sea(TERRA_SOLAR_TIDAL));
    g_ct_world.rules.tgl_nutrient_effect_with_mine = 0;
    ct_base(0).state = BSTATE_UNK_4000;           // the first of the three, so it holds
    ct_base(0).nutrient_surplus = 0;
    ct_at(10, 4).bit |= BIT_SOLAR_TIDAL;          // census: 0 + 1 > 1 is false
    CTCHECK(can_terraform(CT_FACTION, CT_X, CT_Y, 0, 0) == ORDER_NONE);
    g_ct_world.rules.tgl_nutrient_effect_with_mine = -1;
    CTCHECK(can_terraform(CT_FACTION, CT_X, CT_Y, 0, 0) == ORDER_SOLAR_COLLECTOR);

    // ---- the census seed, and the mines it counts ---------------------------
    // The seeded one mine is the whole margin here: without it the tally ties
    // at zero and the tail builds a platform instead.
    ct_reset();
    ct_make_ocean();
    ct_grant(ct_preq_sea(TERRA_MINE));
    ct_grant(ct_preq_sea(TERRA_SOLAR_TIDAL));
    ct_base(0).state = 0;
    ct_base(0).nutrient_surplus = 2;
    CTCHECK(can_terraform(CT_FACTION, CT_X, CT_Y, 0, 0) == ORDER_SOLAR_COLLECTOR);
    // With the effect non-negative the seed is zero, and one counted mine puts
    // the tally back over the line.
    g_ct_world.rules.tgl_nutrient_effect_with_mine = 0;
    CTCHECK(can_terraform(CT_FACTION, CT_X, CT_Y, 0, 0) == ORDER_MINE);
    ct_at(10, 4).bit |= BIT_MINE;
    CTCHECK(can_terraform(CT_FACTION, CT_X, CT_Y, 0, 0) == ORDER_SOLAR_COLLECTOR);

    // ---- the census bounds, at both ends ------------------------------------
    // The base moves to column zero so the subject tile is outside its radius
    // and cannot pollute the tally. Offset 0 is the base's own tile (2,4),
    // offset 2 is (4,4) and offset 21 - the first one PAST the twenty-one - is
    // (6,4). Each of the three is reachable by exactly one offset at row 4.
    ct_reset();
    ct_make_ocean();
    ct_grant(ct_preq_sea(TERRA_SOLAR_TIDAL));
    ct_base(0).x = 2;
    ct_base(0).y = 4;
    ct_base(0).state = 0;
    ct_base(0).nutrient_surplus = 2;
    ct_at(2, 4).bit |= BIT_SOLAR_TIDAL;           // offset 0
    ct_at(4, 4).bit |= BIT_SOLAR_TIDAL;           // offset 2
    CTCHECK(can_terraform(CT_FACTION, CT_X, CT_Y, 0, 0) == ORDER_NONE);   // 2 > 2 false
    ct_at(2, 4).bit &= ~(uint32_t)BIT_SOLAR_TIDAL;
    CTCHECK(can_terraform(CT_FACTION, CT_X, CT_Y, 0, 0) == ORDER_SOLAR_COLLECTOR);
    ct_at(6, 4).bit |= BIT_SOLAR_TIDAL;           // offset 21, outside the loop
    CTCHECK(can_terraform(CT_FACTION, CT_X, CT_Y, 0, 0) == ORDER_SOLAR_COLLECTOR);

    CT_GUARDS();
}
GAMEPLAY_CASE(test_can_terraform_sea);

/*
 * Land, fungus. The eco-damage-adjusted terrain value decides whether the
 * fungus is worth more than what is under it.
 */
void test_can_terraform_land_fungus() {
    CtSeams seams;

    // ---- no base: only the technology and the preference matter -------------
    ct_reset();
    ct_subject().bit |= BIT_FUNGUS;
    CTCHECK(can_terraform(CT_FACTION, CT_X, CT_Y, 0, -1) == ORDER_NONE);
    ct_grant(ct_preq_sea(TERRA_REMOVE_FUNGUS));   // the SEA field, on land - see the bug note
    CTCHECK(can_terraform(CT_FACTION, CT_X, CT_Y, 0, -1) == ORDER_REMOVE_FUNGUS);
    ct_reset();
    ct_subject().bit |= BIT_FUNGUS;
    ct_grant(ct_preq(TERRA_REMOVE_FUNGUS));       // the land field is never read
    CTCHECK(can_terraform(CT_FACTION, CT_X, CT_Y, 0, -1) == ORDER_NONE);

    // ---- a base weighs eco damage against the terrain -----------------------
    // value is elev 1 + rainfall 0 + rockiness 0 + 1 == 2, fungus_value is 3.
    ct_reset();
    ct_subject().bit |= BIT_FUNGUS;
    ct_grant(ct_preq_sea(TERRA_REMOVE_FUNGUS));
    ct_base(0).eco_damage = 0;
    CTCHECK(can_terraform(CT_FACTION, CT_X, CT_Y, 0, 0) == ORDER_NONE);   // 0/2 + 2 <= 3
    ct_base(0).eco_damage = 3;
    CTCHECK(can_terraform(CT_FACTION, CT_X, CT_Y, 0, 0) == ORDER_NONE);   // 3/2 + 2 == 3
    ct_base(0).eco_damage = 4;
    CTCHECK(can_terraform(CT_FACTION, CT_X, CT_Y, 0, 0) == ORDER_REMOVE_FUNGUS);  // 2 + 2 > 3
    // The division truncates towards zero, so 5 is still 2.
    ct_base(0).eco_damage = 5;
    CTCHECK(can_terraform(CT_FACTION, CT_X, CT_Y, 0, 0) == ORDER_REMOVE_FUNGUS);
    // A cheaper fungus is removed with no eco damage at all.
    ct_base(0).eco_damage = 0;
    ct_set_fungus_value(1);
    CTCHECK(can_terraform(CT_FACTION, CT_X, CT_Y, 0, 0) == ORDER_REMOVE_FUNGUS);  // 0 + 2 > 1
    ct_set_fungus_value(2);
    CTCHECK(can_terraform(CT_FACTION, CT_X, CT_Y, 0, 0) == ORDER_NONE);           // 2 <= 2

    // ---- the human preference gate ------------------------------------------
    ct_reset();
    ct_subject().bit |= BIT_FUNGUS;
    ct_grant(ct_preq_sea(TERRA_REMOVE_FUNGUS));
    ct_make_human();
    CTCHECK(can_terraform(CT_FACTION, CT_X, CT_Y, 0, -1) == ORDER_NONE);
    g_ct_world.more_preferences = MPREF_AUTO_FORMER_REMOVE_FUNGUS;
    CTCHECK(can_terraform(CT_FACTION, CT_X, CT_Y, 0, -1) == ORDER_REMOVE_FUNGUS);
    // A different bit of the same word is not the one that is read.
    g_ct_world.more_preferences = MPREF_AUTO_FORMER_BUILD_SENSORS;
    CTCHECK(can_terraform(CT_FACTION, CT_X, CT_Y, 0, -1) == ORDER_NONE);
    // Another faction's bit does not make this one human.
    g_ct_world.factions_status[0] = (uint8_t)(1 << (CT_FACTION + 1));
    g_ct_world.more_preferences = 0;
    CTCHECK(can_terraform(CT_FACTION, CT_X, CT_Y, 0, -1) == ORDER_REMOVE_FUNGUS);

    CT_GUARDS();
}
GAMEPLAY_CASE(test_can_terraform_land_fungus);

/*
 * Land, the improvement cascade: solar, mine, farm, enricher, and what
 * force_improve changes about all four.
 */
void test_can_terraform_land_orders() {
    CtSeams seams;

    // ---- the farm, and the crop gate in front of it -------------------------
    ct_reset();
    CTCHECK(can_terraform(CT_FACTION, CT_X, CT_Y, 0, -1) == ORDER_NONE);
    ct_grant(ct_preq(TERRA_FARM));
    CTCHECK(can_terraform(CT_FACTION, CT_X, CT_Y, 0, -1) == ORDER_FARM);
    // Rocky ground is never farmed.
    ct_subject().val3 = 0x80;                     // ROCKINESS_ROCKY
    CTCHECK(can_terraform(CT_FACTION, CT_X, CT_Y, 0, -1) == ORDER_NONE);
    ct_subject().val3 = 0x40;                     // ROCKINESS_ROLLING is fine
    CTCHECK(can_terraform(CT_FACTION, CT_X, CT_Y, 0, -1) == ORDER_FARM);
    ct_subject().val3 = 0;
    // A thermal borehole, an echelon mirror or the volcano tile each refuse.
    ct_subject().bit = BIT_THERMAL_BORE;
    CTCHECK(can_terraform(CT_FACTION, CT_X, CT_Y, 0, -1) == ORDER_NONE);
    ct_subject().bit = BIT_ECH_MIRROR;
    CTCHECK(can_terraform(CT_FACTION, CT_X, CT_Y, 0, -1) == ORDER_NONE);
    ct_subject().bit = 0;
    ct_subject().bit2 = BIT2_VOLCANO;
    CTCHECK(can_terraform(CT_FACTION, CT_X, CT_Y, 0, -1) == ORDER_NONE);
    ct_subject().bit2 = BIT2_VOLCANO | BIT2_UNK_80000000;
    CTCHECK(can_terraform(CT_FACTION, CT_X, CT_Y, 0, -1) == ORDER_FARM);
    ct_subject().bit2 = 0;
    // Two crop already, without the "three nutrients" technology and without a
    // nutrient bonus, and the farm is not worth building.
    ct_subject().climate = (uint8_t)(CT_ALT_LAND | RAINFALL_RAINY);   // crop 2
    CTCHECK(can_terraform(CT_FACTION, CT_X, CT_Y, 0, -1) == ORDER_NONE);
    ct_grant(CT_TECH_THREE_NUTRIENTS);
    CTCHECK(can_terraform(CT_FACTION, CT_X, CT_Y, 0, -1) == ORDER_FARM);
    // A nutrient bonus opens the same gate without the technology.
    ct_reset();
    ct_grant(ct_preq(TERRA_FARM));
    ct_subject().climate = (uint8_t)(CT_ALT_LAND | RAINFALL_RAINY);
    ct_subject().bit = CT_BONUS_MINERAL;          // bonus 2, not 1
    CTCHECK(can_terraform(CT_FACTION, CT_X, CT_Y, 0, -1) == ORDER_NONE);
    ct_subject().bit = CT_BONUS_NUTRIENT;         // bonus 1 opens it
    CTCHECK(can_terraform(CT_FACTION, CT_X, CT_Y, 0, -1) == ORDER_FARM);

    // ---- rainfall only decides when an order has already been chosen --------
    // With nothing chosen the farm is forced regardless of rainfall, so the
    // `rainfall < 2` arm needs a solar collector standing in front of it.
    // force_improve supplies one, and the nutrient bonus keeps the crop above
    // the "build a farm instead" threshold independently of the rainfall.
    ct_reset();
    ct_grant(ct_preq(TERRA_FARM));
    ct_grant(ct_preq(TERRA_SOLAR_TIDAL));
    ct_subject().bit = CT_BONUS_NUTRIENT;         // bonus 1, crop == rainfall + 2
    ct_subject().climate = (uint8_t)(CT_ALT_LAND | RAINFALL_RAINY);
    CTCHECK(can_terraform(CT_FACTION, CT_X, CT_Y, 1, -1) == ORDER_SOLAR_COLLECTOR);
    ct_subject().climate = CT_ALT_LAND;           // arid; crop is still 2
    CTCHECK(can_terraform(CT_FACTION, CT_X, CT_Y, 1, -1) == ORDER_FARM);

    // ---- the mine ------------------------------------------------------------
    ct_reset();
    ct_grant(ct_preq(TERRA_MINE));
    CTCHECK(can_terraform(CT_FACTION, CT_X, CT_Y, 0, -1) == ORDER_NONE);
    ct_subject().val3 = 0x80;                     // rocky
    CTCHECK(can_terraform(CT_FACTION, CT_X, CT_Y, 0, -1) == ORDER_MINE);
    ct_reset();
    ct_grant(ct_preq(TERRA_MINE));
    ct_subject().bit = CT_BONUS_MINERAL;          // bonus 2
    CTCHECK(can_terraform(CT_FACTION, CT_X, CT_Y, 0, -1) == ORDER_MINE);
    ct_reset();
    ct_grant(ct_preq(TERRA_MINE));
    ct_subject().bit = CT_BONUS_ENERGY;           // bonus 3, through the rolling rule
    CTCHECK(can_terraform(CT_FACTION, CT_X, CT_Y, 0, -1) == ORDER_MINE);
    ct_reset();
    ct_grant(ct_preq(TERRA_MINE));
    ct_subject().bit = CT_BONUS_NUTRIENT;         // bonus 1 is not a reason to mine
    CTCHECK(can_terraform(CT_FACTION, CT_X, CT_Y, 0, -1) == ORDER_NONE);
    // Without the technology, rock is not enough.
    ct_reset();
    ct_subject().val3 = 0x80;
    CTCHECK(can_terraform(CT_FACTION, CT_X, CT_Y, 0, -1) == ORDER_NONE);

    // ---- the solar collector -------------------------------------------------
    // elev is 1 and rockiness flat, so the ordinary rule needs neither.
    ct_reset();
    ct_grant(ct_preq(TERRA_SOLAR_TIDAL));
    ct_subject().climate = (uint8_t)(CT_ALT_LAND | RAINFALL_MOIST);   // crop 1
    CTCHECK(can_terraform(CT_FACTION, CT_X, CT_Y, 0, -1) == ORDER_NONE);
    ct_subject().val3 = 0x40;                     // rolling
    CTCHECK(can_terraform(CT_FACTION, CT_X, CT_Y, 0, -1) == ORDER_SOLAR_COLLECTOR);
    ct_subject().val3 = 0;
    ct_subject().climate = (uint8_t)(0x80 | RAINFALL_MOIST);          // elev 2
    CTCHECK(can_terraform(CT_FACTION, CT_X, CT_Y, 0, -1) == ORDER_SOLAR_COLLECTOR);
    // Rolling but no crop at all, and the rule fails again.
    ct_reset();
    ct_grant(ct_preq(TERRA_SOLAR_TIDAL));
    ct_subject().val3 = 0x40;
    CTCHECK(can_terraform(CT_FACTION, CT_X, CT_Y, 0, -1) == ORDER_NONE);   // crop 0 < 1
    // force_improve takes the marginal answer.
    CTCHECK(can_terraform(CT_FACTION, CT_X, CT_Y, 1, -1) == ORDER_SOLAR_COLLECTOR);
    // It is not a licence to ignore the technology.
    ct_reset();
    CTCHECK(can_terraform(CT_FACTION, CT_X, CT_Y, 1, -1) == ORDER_NONE);

    // ---- force_improve on an existing farm ----------------------------------
    // has_farm plus force_improve plus the solar technology says harness it.
    ct_reset();
    ct_grant(ct_preq(TERRA_SOLAR_TIDAL));
    ct_subject().bit = BIT_FARM;
    CTCHECK(can_terraform(CT_FACTION, CT_X, CT_Y, 0, -1) == ORDER_NONE);
    CTCHECK(can_terraform(CT_FACTION, CT_X, CT_Y, 1, -1) == ORDER_SOLAR_COLLECTOR);
    // With the mine technology instead, the same shape says mine it.
    ct_reset();
    ct_grant(ct_preq(TERRA_MINE));
    ct_subject().bit = BIT_FARM;
    CTCHECK(can_terraform(CT_FACTION, CT_X, CT_Y, 0, -1) == ORDER_NONE);
    CTCHECK(can_terraform(CT_FACTION, CT_X, CT_Y, 1, -1) == ORDER_MINE);

    // ---- the soil enricher ---------------------------------------------------
    // A crop of one is below the gate, and only force_improve gets past it.
    ct_reset();
    ct_grant(ct_preq(TERRA_SOIL_ENR));
    ct_subject().bit = BIT_FARM;                  // arid, crop 0 + 1 == 1
    CTCHECK(can_terraform(CT_FACTION, CT_X, CT_Y, 0, -1) == ORDER_NONE);
    CTCHECK(can_terraform(CT_FACTION, CT_X, CT_Y, 1, -1) == ORDER_FARM);
    // Two crop is above it, and no force is needed.
    ct_subject().climate = (uint8_t)(CT_ALT_LAND | RAINFALL_RAINY);   // 2 + 1, clipped to 2
    CTCHECK(can_terraform(CT_FACTION, CT_X, CT_Y, 0, -1) == ORDER_FARM);
    // Already enriched, so there is nothing to build.
    ct_subject().bit = BIT_FARM | BIT_SOIL_ENRICHER;
    CTCHECK(can_terraform(CT_FACTION, CT_X, CT_Y, 0, -1) == ORDER_NONE);
    // And nothing to enrich without a farm.
    ct_subject().bit = 0;
    CTCHECK(can_terraform(CT_FACTION, CT_X, CT_Y, 0, -1) == ORDER_NONE);
    // Without the technology the whole block is skipped.
    ct_reset();
    ct_subject().bit = BIT_FARM;
    ct_subject().climate = (uint8_t)(CT_ALT_LAND | RAINFALL_RAINY);
    CTCHECK(can_terraform(CT_FACTION, CT_X, CT_Y, 1, -1) == ORDER_NONE);

    // ---- the base planning bits on land -------------------------------------
    // 0x2000 plus the mine technology, with crop enough to want one.
    ct_reset();
    ct_grant(ct_preq(TERRA_MINE));
    ct_subject().climate = (uint8_t)(CT_ALT_LAND | RAINFALL_RAINY);   // crop 2
    ct_base(0).state = 0;
    CTCHECK(can_terraform(CT_FACTION, CT_X, CT_Y, 0, 0) == ORDER_NONE);
    ct_base(0).state = BSTATE_UNK_2000;
    CTCHECK(can_terraform(CT_FACTION, CT_X, CT_Y, 0, 0) == ORDER_MINE);
    // One crop is not enough for either half of that rule.
    ct_subject().climate = (uint8_t)(CT_ALT_LAND | RAINFALL_MOIST);   // crop 1
    CTCHECK(can_terraform(CT_FACTION, CT_X, CT_Y, 0, 0) == ORDER_NONE);
    // 0x1000 plus the solar technology.
    ct_reset();
    ct_grant(ct_preq(TERRA_SOLAR_TIDAL));
    ct_subject().climate = (uint8_t)(CT_ALT_LAND | RAINFALL_MOIST);   // crop 1
    ct_base(0).state = 0;
    CTCHECK(can_terraform(CT_FACTION, CT_X, CT_Y, 0, 0) == ORDER_NONE);
    ct_base(0).state = BSTATE_UNK_1000;
    CTCHECK(can_terraform(CT_FACTION, CT_X, CT_Y, 0, 0) == ORDER_SOLAR_COLLECTOR);
    // A base id of -1 never reads a Base, so the same case answers nothing.
    CTCHECK(can_terraform(CT_FACTION, CT_X, CT_Y, 0, -1) == ORDER_NONE);

    CT_GUARDS();
}
GAMEPLAY_CASE(test_can_terraform_land_orders);

/*
 * Land, the two closing questions: plant a forest, or plant fungus. Both are
 * decided by plant_value against a yardstick, and plant_value is where the base
 * planning bits, the tree farms and the turn number all land.
 */
void test_can_terraform_land_plant() {
    CtSeams seams;

    // ---- the forest, and forest_value ---------------------------------------
    // plant_value is elev 1 + rainfall 0 + rockiness 0 + 1 == 2.
    ct_reset();
    ct_grant(ct_preq(TERRA_FOREST));
    CTCHECK(can_terraform(CT_FACTION, CT_X, CT_Y, 0, -1) == ORDER_PLANT_FOREST);  // 2 < 9
    ct_set_forest_value(2, 0, 0);
    CTCHECK(can_terraform(CT_FACTION, CT_X, CT_Y, 0, -1) == ORDER_NONE);          // 2 < 2
    ct_set_forest_value(3, 0, 0);
    CTCHECK(can_terraform(CT_FACTION, CT_X, CT_Y, 0, -1) == ORDER_PLANT_FOREST);  // 2 < 3
    // Rock, an existing mine and an existing forest each refuse.
    ct_reset();
    ct_grant(ct_preq(TERRA_FOREST));
    ct_subject().val3 = 0x80;
    CTCHECK(can_terraform(CT_FACTION, CT_X, CT_Y, 0, -1) == ORDER_NONE);
    ct_subject().val3 = 0x40;                     // rolling is allowed
    CTCHECK(can_terraform(CT_FACTION, CT_X, CT_Y, 0, -1) == ORDER_PLANT_FOREST);
    ct_subject().val3 = 0;
    ct_subject().bit = BIT_MINE;
    CTCHECK(can_terraform(CT_FACTION, CT_X, CT_Y, 0, -1) == ORDER_NONE);
    ct_subject().bit = BIT_CONDENSER;             // "special"
    CTCHECK(can_terraform(CT_FACTION, CT_X, CT_Y, 0, -1) == ORDER_NONE);
    ct_subject().bit = 0;
    ct_subject().bit2 = BIT2_VOLCANO;
    CTCHECK(can_terraform(CT_FACTION, CT_X, CT_Y, 0, -1) == ORDER_NONE);
    ct_subject().bit2 = 0;
    // A tile that already has a farm is never forested: the guard behind
    // has_farm asks for elev == 0, which cannot happen above the shore line.
    ct_subject().bit = BIT_FARM;
    CTCHECK(can_terraform(CT_FACTION, CT_X, CT_Y, 0, -1) == ORDER_NONE);

    // ---- the human preference, and the local faction ------------------------
    ct_reset();
    ct_grant(ct_preq(TERRA_FOREST));
    ct_make_human();
    CTCHECK(can_terraform(CT_FACTION, CT_X, CT_Y, 0, -1) == ORDER_NONE);
    g_ct_world.preferences = PREF_AUTO_FORMER_PLANT_FORESTS;
    CTCHECK(can_terraform(CT_FACTION, CT_X, CT_Y, 0, -1) == ORDER_PLANT_FOREST);
    g_ct_world.local_faction = CT_FACTION + 1;    // somebody else is at the keyboard
    CTCHECK(can_terraform(CT_FACTION, CT_X, CT_Y, 0, -1) == ORDER_NONE);
    g_ct_world.local_faction = CT_FACTION;
    g_ct_world.preferences = PREF_AUTO_FORMER_BUILD_ADV;   // the neighbouring bit
    CTCHECK(can_terraform(CT_FACTION, CT_X, CT_Y, 0, -1) == ORDER_NONE);

    // ---- the two forest facilities move forest_value ------------------------
    ct_reset();
    ct_grant(ct_preq(TERRA_FOREST));
    ct_set_forest_value(1, 0, 0);
    CTCHECK(can_terraform(CT_FACTION, CT_X, CT_Y, 0, 0) == ORDER_NONE);           // 2 < 1
    ct_give_fac(0, FAC_TREE_FARM);
    CTCHECK(can_terraform(CT_FACTION, CT_X, CT_Y, 0, 0) == ORDER_NONE);           // 2 < 2
    ct_give_fac(0, FAC_HYBRID_FOREST);
    CTCHECK(can_terraform(CT_FACTION, CT_X, CT_Y, 0, 0) == ORDER_PLANT_FOREST);   // 2 < 4
    // The hybrid forest is worth two on its own.
    ct_reset();
    ct_grant(ct_preq(TERRA_FOREST));
    ct_set_forest_value(1, 0, 0);
    ct_give_fac(0, FAC_HYBRID_FOREST);
    CTCHECK(can_terraform(CT_FACTION, CT_X, CT_Y, 0, 0) == ORDER_PLANT_FOREST);   // 2 < 3
    // And neither counts without a base.
    CTCHECK(can_terraform(CT_FACTION, CT_X, CT_Y, 0, -1) == ORDER_NONE);

    // ---- the growth-only AI plan is worth one more forest -------------------
    ct_reset();
    ct_grant(ct_preq(TERRA_FOREST));
    ct_set_forest_value(2, 0, 0);
    CTCHECK(can_terraform(CT_FACTION, CT_X, CT_Y, 0, -1) == ORDER_NONE);          // 2 < 2
    g_ct_world.players_data[CT_FACTION].ai_growth = 1;
    g_ct_world.players_data[CT_FACTION].ai_fight = -1;
    CTCHECK(can_terraform(CT_FACTION, CT_X, CT_Y, 0, -1) == ORDER_PLANT_FOREST);  // 2 < 3
    // Any of the other three plans, or a non-negative ai_fight, cancels it.
    g_ct_world.players_data[CT_FACTION].ai_power = 1;
    CTCHECK(can_terraform(CT_FACTION, CT_X, CT_Y, 0, -1) == ORDER_NONE);
    g_ct_world.players_data[CT_FACTION].ai_power = 0;
    g_ct_world.players_data[CT_FACTION].ai_wealth = 1;
    CTCHECK(can_terraform(CT_FACTION, CT_X, CT_Y, 0, -1) == ORDER_NONE);
    g_ct_world.players_data[CT_FACTION].ai_wealth = 0;
    g_ct_world.players_data[CT_FACTION].ai_tech = 1;
    CTCHECK(can_terraform(CT_FACTION, CT_X, CT_Y, 0, -1) == ORDER_NONE);
    g_ct_world.players_data[CT_FACTION].ai_tech = 0;
    g_ct_world.players_data[CT_FACTION].ai_fight = 0;
    CTCHECK(can_terraform(CT_FACTION, CT_X, CT_Y, 0, -1) == ORDER_NONE);
    g_ct_world.players_data[CT_FACTION].ai_fight = -1;
    CTCHECK(can_terraform(CT_FACTION, CT_X, CT_Y, 0, -1) == ORDER_PLANT_FOREST);
    // A human never gets it either.
    ct_make_human();
    g_ct_world.preferences = PREF_AUTO_FORMER_PLANT_FORESTS;
    CTCHECK(can_terraform(CT_FACTION, CT_X, CT_Y, 0, -1) == ORDER_NONE);

    // ---- the base planning bits inside plant_value --------------------------
    // 0x4000 with no rainfall takes one off.
    ct_reset();
    ct_grant(ct_preq(TERRA_FOREST));
    ct_set_forest_value(2, 0, 0);
    ct_base(0).state = 0;
    CTCHECK(can_terraform(CT_FACTION, CT_X, CT_Y, 0, 0) == ORDER_NONE);           // 2 < 2
    ct_base(0).state = BSTATE_UNK_4000;
    CTCHECK(can_terraform(CT_FACTION, CT_X, CT_Y, 0, 0) == ORDER_PLANT_FOREST);   // 1 < 2
    // 0x2000 takes one off flat ground.
    ct_base(0).state = BSTATE_UNK_2000;
    CTCHECK(can_terraform(CT_FACTION, CT_X, CT_Y, 0, 0) == ORDER_PLANT_FOREST);   // 1 < 2
    ct_subject().val3 = 0x40;                     // rolling, so nothing comes off
    CTCHECK(can_terraform(CT_FACTION, CT_X, CT_Y, 0, 0) == ORDER_NONE);           // 3 < 2
    ct_subject().val3 = 0;
    // 0x1000 adds one two levels up.
    ct_reset();
    ct_grant(ct_preq(TERRA_FOREST));
    ct_set_forest_value(4, 0, 0);
    ct_subject().climate = 0x80;                  // elev 2, plant_value 3
    ct_base(0).state = 0;
    CTCHECK(can_terraform(CT_FACTION, CT_X, CT_Y, 0, 0) == ORDER_PLANT_FOREST);   // 3 < 4
    ct_base(0).state = BSTATE_UNK_1000;
    CTCHECK(can_terraform(CT_FACTION, CT_X, CT_Y, 0, 0) == ORDER_NONE);           // 4 < 4
    // Neither planning bit set, and an existing harness adds one instead.
    ct_reset();
    ct_grant(ct_preq(TERRA_FOREST));
    ct_set_forest_value(3, 0, 0);
    ct_base(0).state = 0;
    CTCHECK(can_terraform(CT_FACTION, CT_X, CT_Y, 0, 0) == ORDER_PLANT_FOREST);   // 2 < 3
    ct_subject().bit = BIT_SOLAR_TIDAL;
    CTCHECK(can_terraform(CT_FACTION, CT_X, CT_Y, 0, 0) == ORDER_NONE);           // 3 < 3
    ct_base(0).state = BSTATE_UNK_2000;           // 0x6000 present, so no harness bonus
    CTCHECK(can_terraform(CT_FACTION, CT_X, CT_Y, 0, 0) == ORDER_PLANT_FOREST);   // 1 < 3

    // ---- the early-turn bonus on rolling, rainy ground ----------------------
    ct_reset();
    ct_grant(ct_preq(TERRA_FOREST));
    ct_subject().climate = (uint8_t)(CT_ALT_LAND | RAINFALL_MOIST);
    ct_subject().val3 = 0x40;                     // rolling; plant_value 1+1+1+1 == 4
    ct_set_forest_value(5, 0, 0);
    g_ct_world.turn_current = 100;
    CTCHECK(can_terraform(CT_FACTION, CT_X, CT_Y, 0, -1) == ORDER_PLANT_FOREST);  // 4 < 5
    g_ct_world.turn_current = 49;
    CTCHECK(can_terraform(CT_FACTION, CT_X, CT_Y, 0, -1) == ORDER_NONE);          // 5 < 5
    g_ct_world.turn_current = 50;                 // the boundary is exclusive
    CTCHECK(can_terraform(CT_FACTION, CT_X, CT_Y, 0, -1) == ORDER_PLANT_FOREST);
    // Arid ground does not get it whatever the turn.
    g_ct_world.turn_current = 10;
    ct_subject().climate = CT_ALT_LAND;
    ct_set_forest_value(4, 0, 0);                 // plant_value 1+0+1+1 == 3
    CTCHECK(can_terraform(CT_FACTION, CT_X, CT_Y, 0, -1) == ORDER_PLANT_FOREST);  // 3 < 4

    // ---- planting fungus, which can override a forest -----------------------
    ct_reset();
    ct_set_fungus_value(3);
    CTCHECK(can_terraform(CT_FACTION, CT_X, CT_Y, 0, -1) == ORDER_NONE);   // no technology
    ct_grant(ct_preq(TERRA_PLANT_FUNGUS));
    CTCHECK(can_terraform(CT_FACTION, CT_X, CT_Y, 0, -1) == ORDER_NONE);   // 3 < 3 is false
    ct_set_fungus_value(4);
    CTCHECK(can_terraform(CT_FACTION, CT_X, CT_Y, 0, -1) == ORDER_PLANT_FUNGUS);  // 3 < 4
    // A human is never told to plant fungus.
    ct_make_human();
    CTCHECK(can_terraform(CT_FACTION, CT_X, CT_Y, 0, -1) == ORDER_NONE);
    g_ct_world.factions_status[0] = 0;
    // A special tile is not planted either, and the increment does not happen.
    ct_subject().bit = BIT_CONDENSER;
    CTCHECK(can_terraform(CT_FACTION, CT_X, CT_Y, 0, -1) == ORDER_NONE);
    ct_subject().bit = 0;
    // Rocky ground costs one more, or two more when it is already mined.
    ct_reset();
    ct_grant(ct_preq(TERRA_PLANT_FUNGUS));
    ct_subject().val3 = 0x80;                     // rocky; plant_value 1+0+2+1 == 4
    ct_set_fungus_value(7);
    CTCHECK(can_terraform(CT_FACTION, CT_X, CT_Y, 0, -1) == ORDER_PLANT_FUNGUS);  // 4+1+1 < 7
    ct_subject().bit = BIT_MINE;                  // the rocky surcharge doubles
    CTCHECK(can_terraform(CT_FACTION, CT_X, CT_Y, 0, -1) == ORDER_NONE);          // 4+2+1 == 7
    ct_set_fungus_value(8);
    CTCHECK(can_terraform(CT_FACTION, CT_X, CT_Y, 0, -1) == ORDER_PLANT_FUNGUS);  // 7 < 8
    // Fungus wins over a forest the same terrain would otherwise get.
    ct_reset();
    ct_grant(ct_preq(TERRA_FOREST));
    ct_grant(ct_preq(TERRA_PLANT_FUNGUS));
    ct_set_forest_value(9, 0, 0);
    ct_set_fungus_value(3);
    CTCHECK(can_terraform(CT_FACTION, CT_X, CT_Y, 0, -1) == ORDER_PLANT_FOREST);
    ct_set_fungus_value(4);
    CTCHECK(can_terraform(CT_FACTION, CT_X, CT_Y, 0, -1) == ORDER_PLANT_FUNGUS);
    // And over an order the cascade already chose.
    ct_reset();
    ct_grant(ct_preq(TERRA_FARM));
    ct_grant(ct_preq(TERRA_PLANT_FUNGUS));
    CTCHECK(can_terraform(CT_FACTION, CT_X, CT_Y, 0, -1) == ORDER_FARM);
    ct_set_fungus_value(4);
    CTCHECK(can_terraform(CT_FACTION, CT_X, CT_Y, 0, -1) == ORDER_PLANT_FUNGUS);

    CT_GUARDS();
}
GAMEPLAY_CASE(test_can_terraform_land_plant);

/*
 * The land order cascade, one decision at a time.
 *
 * Everything here exists to make ONE comparison decide the answer, which the
 * broader cases in test_can_terraform_land_orders() cannot do because several
 * rules agree on ORDER_MINE and shadow each other. Two shadowing facts are
 * worth stating once, because most of the setup below is arranged around them:
 *
 *  - `if (bonus == 2 || rockiness == ROCKINESS_ROCKY) order = ORDER_MINE;` runs
 *    FIRST inside the mine block, so a mineral bonus or rocky ground puts
 *    ORDER_MINE on the board before any of the flag logic executes, and every
 *    later write in that block also writes ORDER_MINE. Nothing after it can be
 *    seen while either holds.
 *  - `rolling_rule` and `farm_rule` are not answers. `rolling_rule` is visible
 *    only through `if (bonus == 3) order = ORDER_MINE`, and `farm_rule` only
 *    through the force_improve-on-a-farm rule. A case that wants to see either
 *    flag has to supply the thing that reads it.
 */
void test_can_terraform_cascade() {
    CtSeams seams;

    // ---- rolling_rule, seen through the energy bonus ------------------------
    // Energy bonus, flat, one crop: the worked total is two, so the else arm
    // sets neither flag and nothing answers.
    ct_reset();
    ct_grant(ct_preq(TERRA_MINE));
    ct_subject().bit = CT_BONUS_ENERGY;
    ct_subject().climate = (uint8_t)(CT_ALT_LAND | RAINFALL_MOIST);
    CTCHECK(crop_yield(CT_FACTION, -1, CT_X, CT_Y, 0) == 1);
    CTCHECK(can_terraform(CT_FACTION, CT_X, CT_Y, 0, -1) == ORDER_NONE);
    // A non-negative nutrient effect takes the rolling arm instead, and the
    // energy bonus then answers. This is the only reader of rolling_rule.
    g_ct_world.rules.tgl_nutrient_effect_with_mine = 0;
    CTCHECK(can_terraform(CT_FACTION, CT_X, CT_Y, 0, -1) == ORDER_MINE);
    // Arid instead of moist drops the worked total to one, which reaches the
    // rolling arm the other way.
    g_ct_world.rules.tgl_nutrient_effect_with_mine = -1;
    ct_subject().climate = CT_ALT_LAND;
    CTCHECK(can_terraform(CT_FACTION, CT_X, CT_Y, 0, -1) == ORDER_MINE);

    // ---- farm_rule, seen through force_improve on a farm --------------------
    // Two worked is one short of the threshold, so no flag and no answer.
    ct_reset();
    ct_grant(ct_preq(TERRA_MINE));
    ct_subject().bit = BIT_FARM;
    ct_subject().climate = (uint8_t)(CT_ALT_LAND | RAINFALL_MOIST);
    CTCHECK(crop_yield(CT_FACTION, -1, CT_X, CT_Y, 0) == 2);
    CTCHECK(can_terraform(CT_FACTION, CT_X, CT_Y, 1, -1) == ORDER_NONE);
    // Three worked clears it. The technology is needed only to stop crop_yield
    // clipping the third nutrient away.
    ct_grant(CT_TECH_THREE_NUTRIENTS);
    ct_subject().climate = (uint8_t)(CT_ALT_LAND | RAINFALL_RAINY);
    CTCHECK(crop_yield(CT_FACTION, -1, CT_X, CT_Y, 0) == 3);
    CTCHECK(can_terraform(CT_FACTION, CT_X, CT_Y, 1, -1) == ORDER_MINE);
    // And without the force there is nothing to read the flag with.
    CTCHECK(can_terraform(CT_FACTION, CT_X, CT_Y, 0, -1) == ORDER_NONE);

    // ---- the solar rule's crop threshold, which a farm raises ---------------
    ct_reset();
    ct_grant(ct_preq(TERRA_SOLAR_TIDAL));
    ct_subject().bit = BIT_FARM;
    ct_subject().val3 = 0x40;                     // rolling, so the rule is in play
    CTCHECK(crop_yield(CT_FACTION, -1, CT_X, CT_Y, 0) == 1);
    CTCHECK(can_terraform(CT_FACTION, CT_X, CT_Y, 0, -1) == ORDER_NONE);   // 1 < 2
    ct_subject().climate = (uint8_t)(CT_ALT_LAND | RAINFALL_MOIST);
    CTCHECK(crop_yield(CT_FACTION, -1, CT_X, CT_Y, 0) == 2);
    CTCHECK(can_terraform(CT_FACTION, CT_X, CT_Y, 0, -1) == ORDER_SOLAR_COLLECTOR);

    // ---- the base's harness rule, both halves of its disjunction ------------
    // No farm, no crop, one level up: neither half holds.
    ct_reset();
    ct_grant(ct_preq(TERRA_SOLAR_TIDAL));
    ct_base(0).state = BSTATE_UNK_1000;
    CTCHECK(can_terraform(CT_FACTION, CT_X, CT_Y, 0, 0) == ORDER_NONE);
    ct_subject().climate = (uint8_t)(CT_ALT_LAND | RAINFALL_MOIST);   // one crop
    CTCHECK(can_terraform(CT_FACTION, CT_X, CT_Y, 0, 0) == ORDER_SOLAR_COLLECTOR);
    ct_subject().climate = 0x80;                  // two levels up, no crop
    CTCHECK(can_terraform(CT_FACTION, CT_X, CT_Y, 0, 0) == ORDER_SOLAR_COLLECTOR);
    // A farm raises the crop half of it out of reach again.
    ct_reset();
    ct_grant(ct_preq(TERRA_SOLAR_TIDAL));
    ct_base(0).state = BSTATE_UNK_1000;
    ct_subject().bit = BIT_FARM;                  // one crop, threshold two
    CTCHECK(can_terraform(CT_FACTION, CT_X, CT_Y, 0, 0) == ORDER_NONE);

    // ---- the base's mine rule: the crop test, isolated ----------------------
    // A nutrient bonus lifts the crop to two while leaving the tile arid, which
    // is what keeps `wants_mine` false and the crop test alone on the board.
    ct_reset();
    ct_grant(ct_preq(TERRA_MINE));
    ct_base(0).state = BSTATE_UNK_2000;
    ct_subject().bit = CT_BONUS_NUTRIENT;
    CTCHECK(crop_yield(CT_FACTION, -1, CT_X, CT_Y, 0) == 2);
    CTCHECK(can_terraform(CT_FACTION, CT_X, CT_Y, 0, 0) == ORDER_MINE);
    // A farm raises that threshold from two to three.
    ct_reset();
    ct_grant(ct_preq(TERRA_MINE));
    ct_base(0).state = BSTATE_UNK_2000;
    ct_subject().bit = BIT_FARM;                  // one crop, threshold three
    CTCHECK(can_terraform(CT_FACTION, CT_X, CT_Y, 0, 0) == ORDER_NONE);

    // ---- the base's mine rule: wants_mine, term by term ---------------------
    // Arid flat with a farm and no bonus: every term of wants_mine is false.
    // Turning any one of them on answers, and the mineral-bonus term cannot be
    // tested here because that bonus answers two statements earlier.
    ct_reset();
    ct_grant(ct_preq(TERRA_MINE));
    ct_base(0).state = BSTATE_UNK_2000;
    ct_subject().bit = BIT_FARM;
    CTCHECK(can_terraform(CT_FACTION, CT_X, CT_Y, 0, 0) == ORDER_NONE);
    ct_subject().climate = (uint8_t)(CT_ALT_LAND | RAINFALL_MOIST);   // rainfall term
    CTCHECK(can_terraform(CT_FACTION, CT_X, CT_Y, 0, 0) == ORDER_MINE);
    // Two levels up takes the elevation clause of that term away again.
    ct_subject().climate = (uint8_t)(0x80 | RAINFALL_MOIST);
    CTCHECK(can_terraform(CT_FACTION, CT_X, CT_Y, 0, 0) == ORDER_NONE);

    // ---- value, and the forest it is weighed against ------------------------
    // A forested tile that also carries a farm: the enricher bonus to `value`
    // is the single point that keeps it off the "not worth touching" answer.
    ct_reset();
    ct_grant(ct_preq(TERRA_SOIL_ENR));
    ct_set_forest_value(2, 0, 0);
    ct_subject().bit = BIT_FARM | BIT_FOREST;
    CTCHECK(can_terraform(CT_FACTION, CT_X, CT_Y, 0, -1) == ORDER_FARM);   // value 3 > 2
    // The weighing is inclusive: equal is not worth touching.
    ct_reset();
    ct_grant(ct_preq(TERRA_FARM));
    ct_grant(CT_TECH_THREE_NUTRIENTS);
    ct_set_forest_value(2, 0, 0);
    ct_subject().bit = BIT_FOREST;
    CTCHECK(can_terraform(CT_FACTION, CT_X, CT_Y, 0, -1) == ORDER_NONE);   // value 2 == 2

    // ---- crop_yield is asked about NO base ----------------------------------
    // Base zero carries a bumper harvest. Asking crop_yield about it instead of
    // about nobody would add one nutrient and close the farm gate.
    ct_reset();
    ct_grant(ct_preq(TERRA_FARM));
    ct_base(0).event = BEVENT_BUMPER;
    ct_subject().climate = (uint8_t)(CT_ALT_LAND | RAINFALL_MOIST);
    CTCHECK(crop_yield(CT_FACTION, -1, CT_X, CT_Y, 0) == 1);
    CTCHECK(crop_yield(CT_FACTION, 0, CT_X, CT_Y, 0) == 2);
    CTCHECK(can_terraform(CT_FACTION, CT_X, CT_Y, 0, -1) == ORDER_FARM);

    // ---- the farm block's three-way choice ----------------------------------
    // A solar collector is standing, the base wants a farm, and it is rainy, so
    // the closing `rainfall < 2` cannot rescue the answer. Only the base's own
    // planning bit turns the collector into a farm here.
    ct_reset();
    ct_grant(ct_preq(TERRA_FARM));
    ct_grant(ct_preq(TERRA_SOLAR_TIDAL));
    ct_grant(CT_TECH_THREE_NUTRIENTS);
    ct_subject().climate = (uint8_t)(CT_ALT_LAND | RAINFALL_RAINY);
    ct_base(0).state = BSTATE_UNK_4000;
    CTCHECK(can_terraform(CT_FACTION, CT_X, CT_Y, 1, 0) == ORDER_FARM);
    ct_base(0).state = 0;
    CTCHECK(can_terraform(CT_FACTION, CT_X, CT_Y, 1, 0) == ORDER_SOLAR_COLLECTOR);
    // Without the base the same tile keeps the collector, and a forest tile -
    // whose crop is the forest yield rather than the rainfall - lets the crop
    // test be moved on its own.
    ct_reset();
    ct_grant(ct_preq(TERRA_FARM));
    ct_grant(ct_preq(TERRA_SOLAR_TIDAL));
    ct_set_forest_value(1, 0, 0);
    ct_subject().bit = BIT_FOREST;
    ct_subject().climate = (uint8_t)(CT_ALT_LAND | RAINFALL_RAINY);
    CTCHECK(crop_yield(CT_FACTION, -1, CT_X, CT_Y, 0) == 1);
    CTCHECK(can_terraform(CT_FACTION, CT_X, CT_Y, 1, -1) == ORDER_FARM);   // crop 1 < 2
    ct_set_forest_value(2, 0, 0);
    ct_grant(CT_TECH_THREE_NUTRIENTS);
    CTCHECK(crop_yield(CT_FACTION, -1, CT_X, CT_Y, 0) == 2);
    CTCHECK(can_terraform(CT_FACTION, CT_X, CT_Y, 1, -1) == ORDER_SOLAR_COLLECTOR);

    // ---- the enricher's gate, and the base bit behind it --------------------
    // A collector is already chosen, so the fallback at the bottom of the
    // enricher block cannot fire and only the base's planning bit can enrich.
    ct_reset();
    ct_grant(ct_preq(TERRA_SOIL_ENR));
    ct_grant(ct_preq(TERRA_SOLAR_TIDAL));
    ct_grant(CT_TECH_THREE_NUTRIENTS);
    ct_subject().bit = BIT_FARM;
    ct_subject().climate = (uint8_t)(CT_ALT_LAND | RAINFALL_RAINY);
    CTCHECK(crop_yield(CT_FACTION, -1, CT_X, CT_Y, 0) == 3);
    ct_base(0).state = BSTATE_UNK_4000;
    CTCHECK(can_terraform(CT_FACTION, CT_X, CT_Y, 1, 0) == ORDER_FARM);
    ct_base(0).state = 0;
    CTCHECK(can_terraform(CT_FACTION, CT_X, CT_Y, 1, 0) == ORDER_SOLAR_COLLECTOR);
    // One crop is below the gate entirely, so the base bit is never reached.
    ct_reset();
    ct_grant(ct_preq(TERRA_SOIL_ENR));
    ct_grant(ct_preq(TERRA_SOLAR_TIDAL));
    ct_subject().bit = BIT_FARM;                  // arid, one crop
    ct_base(0).state = BSTATE_UNK_4000;
    CTCHECK(can_terraform(CT_FACTION, CT_X, CT_Y, 1, 0) == ORDER_SOLAR_COLLECTOR);
    // Exactly two crop is inside the gate only with the three-nutrients rule.
    ct_reset();
    ct_grant(ct_preq(TERRA_SOIL_ENR));
    ct_grant(ct_preq(TERRA_SOLAR_TIDAL));
    ct_subject().bit = BIT_FARM;
    ct_subject().climate = (uint8_t)(CT_ALT_LAND | RAINFALL_MOIST);
    CTCHECK(crop_yield(CT_FACTION, -1, CT_X, CT_Y, 0) == 2);
    ct_base(0).state = BSTATE_UNK_4000;
    CTCHECK(can_terraform(CT_FACTION, CT_X, CT_Y, 1, 0) == ORDER_SOLAR_COLLECTOR);
    ct_grant(CT_TECH_THREE_NUTRIENTS);
    CTCHECK(can_terraform(CT_FACTION, CT_X, CT_Y, 1, 0) == ORDER_FARM);

    CT_GUARDS();
}
GAMEPLAY_CASE(test_can_terraform_cascade);

/*
 * plant_value, the score the two planting answers are weighed against. Every
 * case here moves it by exactly one and puts the yardstick on the boundary, so
 * the answer flips on the single adjustment under test.
 */
void test_can_terraform_scoring() {
    CtSeams seams;

    // ---- the enricher bonus, visible only through fungus --------------------
    // A farmed tile can never be forested, so the forest yardstick is no use
    // here and the fungus one has to carry it.
    ct_reset();
    ct_grant(ct_preq(TERRA_SOIL_ENR));
    ct_grant(ct_preq(TERRA_PLANT_FUNGUS));
    ct_subject().bit = BIT_FARM;
    ct_set_fungus_value(4);
    CTCHECK(can_terraform(CT_FACTION, CT_X, CT_Y, 0, -1) == ORDER_NONE);       // 4 < 4
    ct_set_fungus_value(5);
    CTCHECK(can_terraform(CT_FACTION, CT_X, CT_Y, 0, -1) == ORDER_PLANT_FUNGUS);

    // ---- the rainfall adjustment behind BSTATE_UNK_4000 ---------------------
    // Moist is neither wet enough to add nor dry enough to subtract.
    ct_reset();
    ct_grant(ct_preq(TERRA_FOREST));
    ct_set_forest_value(4, 0, 0);
    ct_base(0).state = BSTATE_UNK_4000;
    ct_subject().climate = (uint8_t)(CT_ALT_LAND | RAINFALL_MOIST);
    CTCHECK(can_terraform(CT_FACTION, CT_X, CT_Y, 0, 0) == ORDER_PLANT_FOREST);   // 3 < 4
    // Rainy adds one.
    ct_set_forest_value(5, 0, 0);
    ct_subject().climate = (uint8_t)(CT_ALT_LAND | RAINFALL_RAINY);
    CTCHECK(can_terraform(CT_FACTION, CT_X, CT_Y, 0, 0) == ORDER_NONE);           // 5 < 5

    // ---- the elevation adjustment behind BSTATE_UNK_1000 --------------------
    // One level up is neither high enough to add nor zero, which cannot happen.
    ct_reset();
    ct_grant(ct_preq(TERRA_FOREST));
    ct_set_forest_value(3, 0, 0);
    ct_base(0).state = BSTATE_UNK_1000;
    CTCHECK(can_terraform(CT_FACTION, CT_X, CT_Y, 0, 0) == ORDER_PLANT_FOREST);   // 2 < 3
    ct_set_forest_value(2, 0, 0);
    CTCHECK(can_terraform(CT_FACTION, CT_X, CT_Y, 0, 0) == ORDER_NONE);           // 2 < 2

    // ---- the rocky surcharge on planting fungus -----------------------------
    ct_reset();
    ct_grant(ct_preq(TERRA_PLANT_FUNGUS));
    ct_subject().val3 = 0x80;                     // rocky, plant_value four
    ct_set_fungus_value(6);
    CTCHECK(can_terraform(CT_FACTION, CT_X, CT_Y, 0, -1) == ORDER_NONE);          // 6 < 6
    ct_set_fungus_value(7);
    CTCHECK(can_terraform(CT_FACTION, CT_X, CT_Y, 0, -1) == ORDER_PLANT_FUNGUS);  // 6 < 7

    CT_GUARDS();
}
GAMEPLAY_CASE(test_can_terraform_scoring);

#undef CT_GUARDS
#undef CTCHECK

/*
 * repair_phase (0x00526030).
 *
 * The turn-start pass that resets per-unit state, heals damaged units and
 * redraws the tiles they stand on. Everything it reaches is a pointer global,
 * so the whole world is rebindable and no fixed address is ever touched.
 *
 * FIVE THINGS ABOUT THIS FIXTURE ARE LOAD-BEARING.
 *
 *  - draw_tile is a RECORDING STAND-IN defined at the bottom of this file,
 *    not the real body from src/mapwin.cpp. Linking mapwin.cpp here would drag
 *    the MapWin/Console object graph into this target for a function whose own
 *    slot walk is already covered by recovery-leaf-tests. The stand-in is what
 *    makes the second pass observable at all: without it the only evidence
 *    that pass runs is that it does not crash.
 *
 *  - do_all_draws IS the real body from src/temp.cpp. It writes *MsgStatus and
 *    calls do_video/check_net/do_net, so all four are seamed - MsgStatus at a
 *    fixed address would fault, and the three function pointers still point at
 *    the original image. Counting the three calls is how "do_all_draws ran
 *    exactly once, at the end" is asserted.
 *
 *  - The MAP WINDOW starts at tiles[RP_LIVE]; everything before it is poisoned
 *    with every terrain bit set and a base owner of 0, so an off-by-one in the
 *    map index turns a bare tile into a sheltered, fungus-covered base tile.
 *
 *  - PROTOTYPE IDS ARE NOT INTERCHANGEABLE. 16, 17, 18 and 19 are the Unity
 *    Ogres and the Fungal Tower, which the body special-cases by id, so no
 *    other case may borrow them. RP_PROTO_PSI_CUSTOM is deliberately at 64 -
 *    one past MaxVehProtoFactionNum - so the two `proto_id < 64` guards fail
 *    for a prototype that is otherwise identical to RP_PROTO_PSI.
 *
 *  - Guard rows one past every live index (a unit, a prototype, a base and a
 *    faction) are poisoned and asserted untouched after each group, so a
 *    stride error in any of the four index computations is visible.
 */
struct RepairWorld {
    Veh vehs[16];
    VehPrototype protos[68];
    RulesChassis chassis[4];
    RulesWeapon weapons[4];
    Map tiles[128];
    Base bases[6];
    BaseSecretProject projects;
    PlayerData players_data[MaxPlayerNum + 1];
    uint8_t factions_status[2];
    Map *tiles_ptr;
    uint32_t longitude;
    int lon_bounds;
    int lat_bounds;
    int veh_current_count;
    int base_current_count;
    int turn_current;
    int local_faction;
    uint32_t game_state;
    uint32_t msg_status;
};

RepairWorld g_rp_world;

// draw_tile's recorder. The stand-in itself is defined after this namespace
// closes, because game.cpp needs it at external linkage.
struct RepairDrawCall {
    int x;
    int y;
    int draw_type;
};
std::vector<RepairDrawCall> g_rp_draw_calls;

// do_draw's three seams, counted rather than ignored so that "do_all_draws was
// reached" is an assertion instead of an assumption.
int g_rp_do_video_calls;
int g_rp_check_net_calls;
int g_rp_do_net_calls;

void *rp_do_video() {
    g_rp_do_video_calls++;
    return NULL;
}

void *rp_check_net() {
    g_rp_check_net_calls++;
    return NULL;
}

void *rp_do_net() {
    g_rp_do_net_calls++;
    return NULL;
}

const int RP_LIVE = 64;              // the live map window starts here, 8x8 tiles
const int RP_FACTION = 1;            // territory 0 reads as "unowned", so 1 is the floor
const int RP_OTHER = 3;
const int RP_X = 8;                  // the subject tile
const int RP_Y = 4;
const int RP_BASE_X = 2;             // the base tile
const int RP_BASE_Y = 2;
const uint8_t RP_POISON = 0x77;

const uint8_t RP_ALT_LAND = 0x80;    // ALT_BIT_1_LEVEL_ABOVE_SEA: not ocean, fungus-eligible
const uint8_t RP_ALT_SHORE = 0x60;   // ALT_BIT_SHORE_LINE: the exact edge of "is ocean"
const uint8_t RP_ALT_SHELF = 0x40;   // ALT_BIT_OCEAN_SHELF: ocean, still fungus-eligible
const uint8_t RP_ALT_DEEP = 0x20;    // ALT_BIT_OCEAN: below the fungus-regeneration floor

const uint8_t RP_CHASSIS_LAND = 0;
const uint8_t RP_CHASSIS_SEA = 1;
const uint8_t RP_CHASSIS_AIR = 2;
const uint8_t RP_CHASSIS_ODD = 3;    // triad 7: no branch names it, so it is the default arm

const uint8_t RP_WPN_CONVENTIONAL = 0;   // offense 4
const uint8_t RP_WPN_PSI = 1;            // offense -1
const uint8_t RP_WPN_ZERO = 2;           // offense 0, ie. not negative

const int RP_PROTO_LAND = 0;
const int RP_PROTO_SEA = 1;
const int RP_PROTO_AIR = 2;
const int RP_PROTO_PSI = 3;
const int RP_PROTO_TRANSPORT = 4;
const int RP_PROTO_REPAIR = 5;
const int RP_PROTO_ODD_TRIAD = 6;
const int RP_PROTO_ZERO_OFFENSE = 7;
const int RP_PROTO_PSI_CUSTOM = 64;      // one past MaxVehProtoFactionNum
const int RP_PROTO_GUARD = 67;

const int RP_VEH_GUARD = 15;
const int RP_BASE_GUARD = 5;

Map &rp_at(int x, int y) {
    return g_rp_world.tiles[RP_LIVE + (x >> 1) + y * 8];
}

Veh &rp_veh(int veh_id) {
    return g_rp_world.vehs[veh_id];
}

void rp_build_project(int project_id, int base_id) {
    (&g_rp_world.projects.human_genome_project)[project_id] = base_id;
}

void rp_give_fac(int base_id, uint32_t facility_id) {
    int offset;
    int mask;
    bitmask(facility_id, &offset, &mask);
    g_rp_world.bases[base_id].facilities_built[offset] |= (uint8_t)mask;
}

void rp_reset() {
    std::memset(&g_rp_world, 0, sizeof(g_rp_world));
    // -1 in every slot: no secret project is built anywhere.
    std::memset(&g_rp_world.projects, 0xFF, sizeof(g_rp_world.projects));
    g_rp_draw_calls.clear();
    g_rp_do_video_calls = 0;
    g_rp_check_net_calls = 0;
    g_rp_do_net_calls = 0;

    g_rp_world.tiles_ptr = &g_rp_world.tiles[RP_LIVE];
    g_rp_world.longitude = 8;
    g_rp_world.lon_bounds = 16;
    g_rp_world.lat_bounds = 8;
    g_rp_world.local_faction = RP_FACTION;
    // Omniscient view keeps whose_territory off its treaty path, so the answer
    // is the tile's own territory byte and nothing else.
    g_rp_world.game_state = STATE_OMNISCIENT_VIEW;
    g_rp_world.factions_status[0] = 0;   // nobody is human unless a case says so
    g_rp_world.msg_status = RP_POISON;

    for (int k = 0; k < RP_LIVE; k++) {
        g_rp_world.tiles[k].climate = RP_ALT_LAND;
        g_rp_world.tiles[k].bit = 0xFFFFFFFF;
        g_rp_world.tiles[k].val2 = 0;            // a base owned by faction 0
        g_rp_world.tiles[k].territory = (int8_t)RP_FACTION;
    }
    for (int k = RP_LIVE; k < 128; k++) {
        Map &tile = g_rp_world.tiles[k];
        tile.climate = RP_ALT_LAND;
        tile.bit = 0;
        tile.val2 = 0x0F;                        // 15: no owner
        tile.territory = -1;                     // unclaimed
    }

    g_rp_world.chassis[RP_CHASSIS_LAND].triad = TRIAD_LAND;
    g_rp_world.chassis[RP_CHASSIS_SEA].triad = TRIAD_SEA;
    g_rp_world.chassis[RP_CHASSIS_AIR].triad = TRIAD_AIR;
    g_rp_world.chassis[RP_CHASSIS_ODD].triad = 7;

    g_rp_world.weapons[RP_WPN_CONVENTIONAL].offense_rating = 4;
    g_rp_world.weapons[RP_WPN_PSI].offense_rating = -1;
    g_rp_world.weapons[RP_WPN_ZERO].offense_rating = 0;
    g_rp_world.weapons[3].offense_rating = 0x7F;

    for (int i = 0; i < RP_PROTO_GUARD; i++) {
        g_rp_world.protos[i].chassis_id = RP_CHASSIS_LAND;
        g_rp_world.protos[i].weapon_id = RP_WPN_CONVENTIONAL;
        g_rp_world.protos[i].reactor_id = 1;
    }
    g_rp_world.protos[RP_PROTO_SEA].chassis_id = RP_CHASSIS_SEA;
    g_rp_world.protos[RP_PROTO_AIR].chassis_id = RP_CHASSIS_AIR;
    g_rp_world.protos[RP_PROTO_PSI].weapon_id = RP_WPN_PSI;
    g_rp_world.protos[RP_PROTO_PSI_CUSTOM].weapon_id = RP_WPN_PSI;
    g_rp_world.protos[RP_PROTO_TRANSPORT].carry_capacity = 4;
    g_rp_world.protos[RP_PROTO_REPAIR].ability_flags = ABL_REPAIR;
    g_rp_world.protos[RP_PROTO_ODD_TRIAD].chassis_id = RP_CHASSIS_ODD;
    g_rp_world.protos[RP_PROTO_ZERO_OFFENSE].weapon_id = RP_WPN_ZERO;

    // Guard rows, one past every live index.
    g_rp_world.protos[RP_PROTO_GUARD].reactor_id = RP_POISON;
    g_rp_world.protos[RP_PROTO_GUARD].chassis_id = RP_CHASSIS_ODD;
    g_rp_world.players_data[MaxPlayerNum].sat_odp_deployed = 0x7777;
    g_rp_world.bases[RP_BASE_GUARD].state = 0xFFFFFFFF;
    g_rp_world.bases[RP_BASE_GUARD].x = (int16_t)RP_X;
    g_rp_world.bases[RP_BASE_GUARD].y = (int16_t)RP_Y;
    std::memset(g_rp_world.bases[RP_BASE_GUARD].facilities_built, 0xFF,
                sizeof(g_rp_world.bases[RP_BASE_GUARD].facilities_built));

    for (int i = 0; i < 16; i++) {
        Veh &veh = g_rp_world.vehs[i];
        veh.faction_id = (uint8_t)RP_OTHER;
        veh.proto_id = RP_PROTO_LAND;
        veh.next_veh_id_stack = -1;
        veh.prev_veh_id_stack = -1;
    }
    Veh &guard = g_rp_world.vehs[RP_VEH_GUARD];
    guard.faction_id = (uint8_t)RP_FACTION;   // ours, and still must not be touched
    guard.dmg_incurred = RP_POISON;
    guard.state = 0xFFFFFFFF;
    guard.moves_expended = RP_POISON;
    guard.unk_6 = RP_POISON;
}

// Run one repair phase with the per-call observers cleared, so RP_GUARDS can
// assert that the draw pump ran exactly once for THIS call rather than for the
// whole case.
void rp_run(int faction_id) {
    g_rp_draw_calls.clear();
    g_rp_do_video_calls = 0;
    g_rp_check_net_calls = 0;
    g_rp_do_net_calls = 0;
    g_rp_world.msg_status = RP_POISON;
    repair_phase(faction_id);
}

// Put one of our units on the map. Leaves it undamaged, orderless and unstacked.
void rp_place(int veh_id, int proto_id, int x, int y) {
    Veh &veh = g_rp_world.vehs[veh_id];
    veh.faction_id = (uint8_t)RP_FACTION;
    veh.proto_id = (int16_t)proto_id;
    veh.x = (int16_t)x;
    veh.y = (int16_t)y;
    veh.state = 0;
    veh.flags = 0;
    veh.order = ORDER_NONE;
    veh.dmg_incurred = 0;
    veh.visibility = 0;
    veh.next_veh_id_stack = -1;
    veh.prev_veh_id_stack = -1;
    if (g_rp_world.veh_current_count <= veh_id) {
        g_rp_world.veh_current_count = veh_id + 1;
    }
}

void rp_make_base(int base_id, int x, int y, int faction_id) {
    Base &base = g_rp_world.bases[base_id];
    base.x = (int16_t)x;
    base.y = (int16_t)y;
    base.faction_id_current = (uint8_t)faction_id;
    base.state = 0;
    Map &tile = rp_at(x, y);
    tile.bit |= BIT_BASE_IN_TILE;
    tile.val2 = (uint8_t)((tile.val2 & 0xF0) | (faction_id & 0xF));
    if (g_rp_world.base_current_count <= base_id) {
        g_rp_world.base_current_count = base_id + 1;
    }
}

class RepairSeams {
 public:
    RepairSeams()
        : vehs_(&Vehs, g_rp_world.vehs),
          protos_(&VehPrototypes, g_rp_world.protos),
          chassis_(&Chassis, g_rp_world.chassis),
          weapons_(&Weapon, g_rp_world.weapons),
          tiles_(&MapTiles, &g_rp_world.tiles_ptr),
          longitude_(&MapLongitude, &g_rp_world.longitude),
          lon_(&MapLongitudeBounds, &g_rp_world.lon_bounds),
          lat_(&MapLatitudeBounds, &g_rp_world.lat_bounds),
          bases_(&Bases, g_rp_world.bases),
          base_count_(&BaseCurrentCount, &g_rp_world.base_current_count),
          projects_(&SecretProject, &g_rp_world.projects),
          players_data_(&PlayersData, g_rp_world.players_data),
          status_(&FactionsStatus, g_rp_world.factions_status),
          veh_count_(&VehCurrentCount, &g_rp_world.veh_current_count),
          turn_(&TurnCurrentNum, &g_rp_world.turn_current),
          local_(&LocalFaction, &g_rp_world.local_faction),
          state_(&GameState, &g_rp_world.game_state),
          msg_(&MsgStatus, &g_rp_world.msg_status),
          video_(&do_video, rp_do_video),
          net_check_(&check_net, rp_check_net),
          net_(&do_net, rp_do_net) { }

 private:
    ScopedSeam<Veh> vehs_;
    ScopedSeam<VehPrototype> protos_;
    ScopedSeam<RulesChassis> chassis_;
    ScopedSeam<RulesWeapon> weapons_;
    ScopedSeam<Map *> tiles_;
    ScopedSeam<uint32_t> longitude_;
    ScopedSeam<int> lon_;
    ScopedSeam<int> lat_;
    ScopedSeam<Base> bases_;
    ScopedSeam<int> base_count_;
    ScopedSeam<BaseSecretProject> projects_;
    ScopedSeam<PlayerData> players_data_;
    ScopedSeam<uint8_t> status_;
    ScopedSeam<int> veh_count_;
    ScopedSeam<int> turn_;
    ScopedSeam<int> local_;
    ScopedSeam<uint32_t> state_;
    ScopedSeam<uint32_t> msg_;
    ScopedSeam<func_msg> video_;
    ScopedSeam<func_msg> net_check_;
    ScopedSeam<func_msg> net_;
};

#define RPCHECK(cond)                                                         \
    do {                                                                      \
        const bool rp_ok = (cond);                                            \
        if (!rp_ok) {                                                         \
            std::fprintf(stderr, "repair_phase: line %d: %s\n", __LINE__,     \
                         #cond);                                              \
        }                                                                     \
        expect(rp_ok);                                                        \
    } while (0)

// The four guard rows, plus the two facts that hold for every call: the draw
// pump always runs to completion, and it runs exactly once.
#define RP_GUARDS()                                                                       \
    do {                                                                                  \
        RPCHECK(g_rp_world.players_data[MaxPlayerNum].sat_odp_deployed == 0x7777u);       \
        RPCHECK(g_rp_world.protos[RP_PROTO_GUARD].reactor_id == RP_POISON);               \
        RPCHECK(g_rp_world.bases[RP_BASE_GUARD].state == 0xFFFFFFFFu);                    \
        RPCHECK(g_rp_world.vehs[RP_VEH_GUARD].dmg_incurred == RP_POISON);                 \
        RPCHECK(g_rp_world.vehs[RP_VEH_GUARD].state == 0xFFFFFFFFu);                      \
        RPCHECK(g_rp_world.vehs[RP_VEH_GUARD].moves_expended == RP_POISON);               \
        RPCHECK(g_rp_world.msg_status == 0);                                              \
        RPCHECK(g_rp_do_video_calls == 1);                                                \
        RPCHECK(g_rp_check_net_calls == 1);                                               \
        RPCHECK(g_rp_do_net_calls == 1);                                                  \
    } while (0)

void test_repair_phase_turn_reset() {
    RepairSeams seams;

    // ---- the ODP counter, and only this faction's --------------------------
    rp_reset();
    g_rp_world.players_data[0].sat_odp_deployed = 6;
    g_rp_world.players_data[RP_FACTION].sat_odp_deployed = 5;
    g_rp_world.players_data[RP_OTHER].sat_odp_deployed = 7;
    rp_run(RP_FACTION);
    RPCHECK(g_rp_world.players_data[RP_FACTION].sat_odp_deployed == 0);
    RPCHECK(g_rp_world.players_data[0].sat_odp_deployed == 6);
    RPCHECK(g_rp_world.players_data[RP_OTHER].sat_odp_deployed == 7);
    RP_GUARDS();

    // A different faction id lands on a different row, which is the only check
    // the 8396-byte stride gets.
    rp_reset();
    g_rp_world.players_data[RP_FACTION].sat_odp_deployed = 5;
    g_rp_world.players_data[RP_OTHER].sat_odp_deployed = 7;
    rp_run(RP_OTHER);
    RPCHECK(g_rp_world.players_data[RP_OTHER].sat_odp_deployed == 0);
    RPCHECK(g_rp_world.players_data[RP_FACTION].sat_odp_deployed == 5);
    RP_GUARDS();

    // ---- the unconditional per-unit resets, and the faction filter ---------
    rp_reset();
    rp_place(0, RP_PROTO_LAND, RP_X, RP_Y);
    rp_place(1, RP_PROTO_LAND, RP_X, RP_Y);
    rp_veh(1).faction_id = (uint8_t)RP_OTHER;
    for (int i = 0; i < 2; i++) {
        rp_veh(i).unk_6 = 9;
        rp_veh(i).moves_expended = 9;
        rp_veh(i).state = 0xFFFFFFFF;
        rp_veh(i).flags = 0xFFFF;
    }
    g_rp_world.turn_current = 1;   // (1 + 0) & 3 != 0, so no ageing this turn
    rp_run(RP_FACTION);
    RPCHECK(rp_veh(0).unk_6 == 0);
    RPCHECK(rp_veh(0).moves_expended == 0);
    // Exactly three state bits go, and no others.
    RPCHECK(rp_veh(0).state == (0xFFFFFFFFu
                                & ~(uint32_t)(VSTATE_UNK_2 | VSTATE_UNK_2000 | VSTATE_CRAWLING)));
    RPCHECK(rp_veh(0).flags == (uint16_t)(0xFFFF & ~VFLAG_UNK_1000));
    // The other faction's unit is not touched at all.
    RPCHECK(rp_veh(1).unk_6 == 9);
    RPCHECK(rp_veh(1).moves_expended == 9);
    RPCHECK(rp_veh(1).state == 0xFFFFFFFFu);
    RPCHECK(rp_veh(1).flags == 0xFFFF);
    RP_GUARDS();

    // ---- one unit in four ages, chosen by (turn + unit id) -----------------
    for (int turn = 0; turn < 4; turn++) {
        rp_reset();
        for (int i = 0; i < 4; i++) {
            rp_place(i, RP_PROTO_LAND, RP_X, RP_Y);
            rp_veh(i).state = VSTATE_UNK_800;
        }
        g_rp_world.turn_current = turn;
        rp_run(RP_FACTION);
        for (int i = 0; i < 4; i++) {
            const bool aged = ((turn + i) & 3) == 0;
            RPCHECK(rp_veh(i).state == (aged ? 0u : (uint32_t)VSTATE_UNK_800));
        }
        RP_GUARDS();
    }

    // A negative turn counter still selects by the low two bits rather than by
    // sign: -1 + 1 is 0.
    rp_reset();
    rp_place(0, RP_PROTO_LAND, RP_X, RP_Y);
    rp_place(1, RP_PROTO_LAND, RP_X, RP_Y);
    rp_veh(0).state = VSTATE_UNK_800;
    rp_veh(1).state = VSTATE_UNK_800;
    g_rp_world.turn_current = -1;
    rp_run(RP_FACTION);
    RPCHECK(rp_veh(0).state == (uint32_t)VSTATE_UNK_800);
    RPCHECK(rp_veh(1).state == 0);
    RP_GUARDS();

    // ---- the two-step flag countdown ---------------------------------------
    // Bit 2 first, bit 1 only once bit 2 is already clear, and nothing else in
    // the word moves.
    rp_reset();
    rp_place(0, RP_PROTO_LAND, RP_X, RP_Y);
    rp_veh(0).flags = (uint16_t)(VFLAG_UNK_1 | VFLAG_UNK_2 | VFLAG_LURKER);
    rp_run(RP_FACTION);
    RPCHECK(rp_veh(0).flags == (uint16_t)(VFLAG_UNK_1 | VFLAG_LURKER));
    rp_run(RP_FACTION);
    RPCHECK(rp_veh(0).flags == (uint16_t)VFLAG_LURKER);
    rp_run(RP_FACTION);
    RPCHECK(rp_veh(0).flags == (uint16_t)VFLAG_LURKER);
    RP_GUARDS();

    // Bit 2 alone: bit 1 stays clear rather than being set.
    rp_reset();
    rp_place(0, RP_PROTO_LAND, RP_X, RP_Y);
    rp_veh(0).flags = (uint16_t)VFLAG_UNK_2;
    rp_run(RP_FACTION);
    RPCHECK(rp_veh(0).flags == 0);
    RP_GUARDS();

    // Bit 1 alone goes on the first ageing turn, because bit 2 is already down.
    rp_reset();
    rp_place(0, RP_PROTO_LAND, RP_X, RP_Y);
    rp_veh(0).flags = (uint16_t)VFLAG_UNK_1;
    rp_run(RP_FACTION);
    RPCHECK(rp_veh(0).flags == 0);
    RP_GUARDS();

    // The 0x1000 clear happens every turn, not only on the ageing one, and it
    // happens before the countdown reads the word.
    rp_reset();
    rp_place(0, RP_PROTO_LAND, RP_X, RP_Y);
    rp_veh(0).flags = (uint16_t)(VFLAG_UNK_1000 | VFLAG_UNK_2);
    g_rp_world.turn_current = 1;   // not an ageing turn for unit 0
    rp_run(RP_FACTION);
    RPCHECK(rp_veh(0).flags == (uint16_t)VFLAG_UNK_2);
    RP_GUARDS();

    // ---- the Hold / Sentry countdown ---------------------------------------
    rp_reset();
    rp_place(0, RP_PROTO_LAND, RP_X, RP_Y);
    rp_place(1, RP_PROTO_LAND, RP_X, RP_Y);
    rp_place(2, RP_PROTO_LAND, RP_X, RP_Y);
    rp_place(3, RP_PROTO_LAND, RP_X, RP_Y);
    rp_veh(0).order = ORDER_SENTRY_BOARD;
    rp_veh(0).waypoint_y[0] = 2;
    rp_veh(1).order = ORDER_HOLD;
    rp_veh(1).waypoint_y[0] = 1;
    rp_veh(2).order = ORDER_CONVOY;      // any other order is left alone
    rp_veh(2).waypoint_y[0] = 5;
    rp_veh(3).order = ORDER_HOLD;
    rp_veh(3).waypoint_y[0] = 0;         // plain Hold never expires
    rp_run(RP_FACTION);
    RPCHECK(rp_veh(0).waypoint_y[0] == 1);
    RPCHECK(rp_veh(0).order == ORDER_SENTRY_BOARD);
    RPCHECK(rp_veh(1).waypoint_y[0] == 0);
    RPCHECK(rp_veh(1).order == ORDER_NONE);
    RPCHECK(rp_veh(2).waypoint_y[0] == 5);
    RPCHECK(rp_veh(2).order == ORDER_CONVOY);
    RPCHECK(rp_veh(3).waypoint_y[0] == 0);
    RPCHECK(rp_veh(3).order == ORDER_HOLD);
    // The countdown is on the first waypoint's y, not its x.
    RPCHECK(rp_veh(0).waypoint_x[0] == 0);
    RP_GUARDS();

    // A negative countdown is decremented too - the test is against zero, not
    // against a sign - so it walks away from expiry rather than towards it.
    rp_reset();
    rp_place(0, RP_PROTO_LAND, RP_X, RP_Y);
    rp_veh(0).order = ORDER_HOLD;
    rp_veh(0).waypoint_y[0] = -1;
    rp_run(RP_FACTION);
    RPCHECK(rp_veh(0).waypoint_y[0] == -2);
    RPCHECK(rp_veh(0).order == ORDER_HOLD);
    RP_GUARDS();

    // ---- the loop bound ----------------------------------------------------
    // Unit 1 is ours and on the map, but past the count, so nothing reaches it.
    rp_reset();
    rp_place(0, RP_PROTO_LAND, RP_X, RP_Y);
    rp_place(1, RP_PROTO_LAND, RP_X, RP_Y);
    rp_veh(0).moves_expended = 9;
    rp_veh(1).moves_expended = 9;
    g_rp_world.veh_current_count = 1;
    rp_run(RP_FACTION);
    RPCHECK(rp_veh(0).moves_expended == 0);
    RPCHECK(rp_veh(1).moves_expended == 9);
    RPCHECK(g_rp_draw_calls.size() == 1);
    RP_GUARDS();

    // A zero count runs neither pass but still pumps the draws.
    rp_reset();
    rp_place(0, RP_PROTO_LAND, RP_X, RP_Y);
    rp_veh(0).moves_expended = 9;
    g_rp_world.veh_current_count = 0;
    rp_run(RP_FACTION);
    RPCHECK(rp_veh(0).moves_expended == 9);
    RPCHECK(g_rp_draw_calls.empty());
    RP_GUARDS();
}
GAMEPLAY_CASE(test_repair_phase_turn_reset);

void test_repair_phase_shelter() {
    RepairSeams seams;

    // Every case here leaves the unit undamaged, so the body stops at the
    // damage test and the shelter decision is the only thing observed.
    struct Shelter {
        int proto_id;
        uint32_t tile_bit;
        uint8_t owner;
        bool cleared;
        const char *why;
    };
    const Shelter cases[] = {
        { RP_PROTO_LAND, 0,               0x0F, false, "bare land tile" },
        { RP_PROTO_LAND, BIT_BASE_IN_TILE, 3,   true,  "somebody's base" },
        { RP_PROTO_LAND, BIT_BASE_IN_TILE, 0,   true,  "faction 0's base" },
        { RP_PROTO_LAND, BIT_BASE_IN_TILE, 7,   true,  "the last valid owner" },
        { RP_PROTO_LAND, BIT_BASE_IN_TILE, 8,   false, "owner 8 is out of range" },
        { RP_PROTO_LAND, BIT_BASE_IN_TILE, 0x0F, false, "the unowned nibble" },
        { RP_PROTO_LAND, BIT_BUNKER,      0x0F, true,  "a land unit in a bunker" },
        { RP_PROTO_SEA,  BIT_BUNKER,      0x0F, false, "a sea unit in a bunker" },
        { RP_PROTO_AIR,  BIT_BUNKER,      0x0F, false, "an air unit in a bunker" },
        { RP_PROTO_AIR,  BIT_AIRBASE,     0x0F, true,  "an air unit on an airbase" },
        { RP_PROTO_LAND, BIT_AIRBASE,     0x0F, false, "a land unit on an airbase" },
        { RP_PROTO_SEA,  BIT_AIRBASE,     0x0F, false, "a sea unit on an airbase" },
        { RP_PROTO_ODD_TRIAD, BIT_BUNKER, 0x0F, false, "triad 7 shelters nowhere" },
        { RP_PROTO_ODD_TRIAD, BIT_AIRBASE, 0x0F, false, "triad 7 shelters nowhere" },
    };
    for (size_t i = 0; i < sizeof(cases) / sizeof(cases[0]); i++) {
        rp_reset();
        rp_place(0, cases[i].proto_id, RP_X, RP_Y);
        rp_veh(0).state = VSTATE_UNK_8 | VSTATE_UNK_10000;
        rp_at(RP_X, RP_Y).bit = cases[i].tile_bit;
        rp_at(RP_X, RP_Y).val2 = cases[i].owner;
        rp_run(RP_FACTION);
        const uint32_t expected = cases[i].cleared
            ? (uint32_t)VSTATE_UNK_10000
            : (uint32_t)(VSTATE_UNK_8 | VSTATE_UNK_10000);
        if (rp_veh(0).state != expected) {
            std::fprintf(stderr, "repair_phase shelter: %s\n", cases[i].why);
        }
        RPCHECK(rp_veh(0).state == expected);
        RP_GUARDS();
    }

    // Without the bit there is no shelter decision to make, and a bunker tile
    // does not set it.
    rp_reset();
    rp_place(0, RP_PROTO_LAND, RP_X, RP_Y);
    rp_veh(0).state = VSTATE_UNK_10000;
    rp_at(RP_X, RP_Y).bit = BIT_BUNKER;
    rp_run(RP_FACTION);
    RPCHECK(rp_veh(0).state == (uint32_t)VSTATE_UNK_10000);
    RP_GUARDS();

    // The tile read is the unit's own, not tile zero: the same unit one tile
    // away from the bunker is not sheltered.
    rp_reset();
    rp_place(0, RP_PROTO_LAND, RP_X, RP_Y);
    rp_veh(0).state = VSTATE_UNK_8;
    rp_at(RP_X, RP_Y + 1).bit = BIT_BUNKER;
    rp_at(RP_X + 2, RP_Y).bit = BIT_BUNKER;
    rp_run(RP_FACTION);
    RPCHECK(rp_veh(0).state == (uint32_t)VSTATE_UNK_8);
    rp_at(RP_X, RP_Y).bit = BIT_BUNKER;
    rp_veh(0).state = VSTATE_UNK_8;
    rp_run(RP_FACTION);
    RPCHECK(rp_veh(0).state == 0);
    RP_GUARDS();
}
GAMEPLAY_CASE(test_repair_phase_shelter);

void test_repair_phase_skips() {
    RepairSeams seams;

    // ---- the three Unity Ogres never repair --------------------------------
    for (int proto_id = BSC_BATTLE_OGRE_MK1; proto_id <= BSC_BATTLE_OGRE_MK3; proto_id++) {
        rp_reset();
        rp_place(0, proto_id, RP_X, RP_Y);
        rp_veh(0).dmg_incurred = 100;
        rp_veh(0).moves_expended = 9;
        rp_run(RP_FACTION);
        RPCHECK(rp_veh(0).dmg_incurred == 100);
        RPCHECK(rp_veh(0).moves_expended == 0);   // the resets above still ran
        RP_GUARDS();
    }

    // The prototype either side of the Ogre block does repair, so the three
    // comparisons are bounded rather than a range.
    for (int proto_id = 15; proto_id <= 20; proto_id++) {
        if (proto_id == BSC_FUNGAL_TOWER) {
            continue;   // covered separately below
        }
        rp_reset();
        rp_place(0, proto_id, RP_X, RP_Y);
        rp_veh(0).dmg_incurred = 100;
        rp_run(RP_FACTION);
        const bool is_ogre = proto_id >= BSC_BATTLE_OGRE_MK1 && proto_id <= BSC_BATTLE_OGRE_MK3;
        RPCHECK(rp_veh(0).dmg_incurred == (is_ogre ? 100 : 99));
        RP_GUARDS();
    }

    // ---- VSTATE_UNK_4 blocks repair, except for a Fungal Tower -------------
    rp_reset();
    rp_place(0, RP_PROTO_LAND, RP_X, RP_Y);
    rp_veh(0).dmg_incurred = 100;
    rp_veh(0).state = VSTATE_UNK_4;
    rp_run(RP_FACTION);
    RPCHECK(rp_veh(0).dmg_incurred == 100);
    RP_GUARDS();

    rp_reset();
    rp_place(0, BSC_FUNGAL_TOWER, RP_X, RP_Y);
    rp_veh(0).dmg_incurred = 100;
    rp_veh(0).state = VSTATE_UNK_4;
    rp_run(RP_FACTION);
    RPCHECK(rp_veh(0).dmg_incurred == 99);
    RP_GUARDS();

    // ---- an undamaged unit stops before the rate is ever computed ----------
    rp_reset();
    rp_place(0, RP_PROTO_LAND, RP_X, RP_Y);
    rp_veh(0).dmg_incurred = 0;
    rp_run(RP_FACTION);
    RPCHECK(rp_veh(0).dmg_incurred == 0);
    RP_GUARDS();

    // ---- the Fungal Tower loses its owner ----------------------------------
    // It also skips the whole rate section, so a tower on its own fungus in its
    // own territory still repairs at rate 1 rather than at 2 + 1.
    rp_reset();
    rp_place(0, BSC_FUNGAL_TOWER, RP_X, RP_Y);
    rp_place(1, RP_PROTO_PSI, RP_X, RP_Y);
    rp_veh(0).dmg_incurred = 100;
    rp_veh(1).dmg_incurred = 100;
    rp_at(RP_X, RP_Y).bit = BIT_FUNGUS;
    rp_at(RP_X, RP_Y).territory = (int8_t)RP_FACTION;
    rp_run(RP_FACTION);
    RPCHECK(rp_veh(0).faction_id == 0);
    RPCHECK(rp_veh(0).dmg_incurred == 99);    // rate 1
    RPCHECK(rp_veh(1).faction_id == (uint8_t)RP_FACTION);
    RPCHECK(rp_veh(1).dmg_incurred == 97);    // rate 2 in fungus, +1 for territory
    // The tower is retired before the redraw pass filters on the faction byte,
    // so only the native-life unit is drawn. See the bug note on the recovery.
    RPCHECK(g_rp_draw_calls.size() == 1);
    RP_GUARDS();
}
GAMEPLAY_CASE(test_repair_phase_skips);

void test_repair_phase_rate() {
    RepairSeams seams;

    // Every case in this group is a unit in the open with reactor 1 and 100
    // damage, so the damage removed IS the repair rate.
    struct Rate {
        int proto_id;
        uint32_t tile_bit;
        uint8_t climate;
        int territory;
        int removed;
        const char *why;
    };
    const Rate cases[] = {
        { RP_PROTO_LAND, 0,           RP_ALT_LAND,  -1, 1, "bare tile" },
        { RP_PROTO_LAND, 0,           RP_ALT_LAND,   1, 2, "own territory" },
        { RP_PROTO_LAND, 0,           RP_ALT_LAND,   2, 1, "someone else's territory" },
        { RP_PROTO_LAND, BIT_BUNKER,  RP_ALT_LAND,  -1, 2, "land unit in a bunker" },
        { RP_PROTO_LAND, BIT_BUNKER,  RP_ALT_LAND,   1, 3, "bunker and territory stack" },
        { RP_PROTO_LAND, BIT_AIRBASE, RP_ALT_LAND,  -1, 1, "land unit on an airbase" },
        { RP_PROTO_AIR,  BIT_AIRBASE, RP_ALT_LAND,  -1, 2, "air unit on an airbase" },
        { RP_PROTO_AIR,  BIT_BUNKER,  RP_ALT_LAND,  -1, 1, "air unit in a bunker" },
        { RP_PROTO_SEA,  BIT_BUNKER,  RP_ALT_SHELF, -1, 1, "sea unit in a bunker" },
        { RP_PROTO_SEA,  BIT_AIRBASE, RP_ALT_SHELF, -1, 1, "sea unit on an airbase" },
        { RP_PROTO_PSI,  BIT_FUNGUS,  RP_ALT_LAND,  -1, 2, "native life in land fungus" },
        { RP_PROTO_PSI,  BIT_FUNGUS,  RP_ALT_SHELF, -1, 2, "native life in shelf fungus" },
        { RP_PROTO_PSI,  BIT_FUNGUS,  RP_ALT_DEEP,  -1, 1, "fungus too deep to regenerate" },
        { RP_PROTO_PSI,  0,           RP_ALT_LAND,  -1, 1, "native life, no fungus" },
        { RP_PROTO_LAND, BIT_FUNGUS,  RP_ALT_LAND,  -1, 1, "a conventional unit in fungus" },
        { RP_PROTO_ZERO_OFFENSE, BIT_FUNGUS, RP_ALT_LAND, -1, 1, "offense 0 is not psi" },
        { RP_PROTO_PSI_CUSTOM, BIT_FUNGUS, RP_ALT_LAND, -1, 1, "prototype 64 is not predefined" },
        { RP_PROTO_PSI,  BIT_FUNGUS,  RP_ALT_LAND,   1, 3, "the fungus rate is a floor, not a term" },
    };
    for (size_t i = 0; i < sizeof(cases) / sizeof(cases[0]); i++) {
        rp_reset();
        rp_place(0, cases[i].proto_id, RP_X, RP_Y);
        rp_veh(0).dmg_incurred = 100;
        rp_at(RP_X, RP_Y).bit = cases[i].tile_bit;
        rp_at(RP_X, RP_Y).climate = cases[i].climate;
        rp_at(RP_X, RP_Y).territory = (int8_t)cases[i].territory;
        rp_run(RP_FACTION);
        if (rp_veh(0).dmg_incurred != (uint8_t)(100 - cases[i].removed)) {
            std::fprintf(stderr, "repair_phase rate: %s\n", cases[i].why);
        }
        RPCHECK(rp_veh(0).dmg_incurred == (uint8_t)(100 - cases[i].removed));
        RP_GUARDS();
    }

    // Territory zero reads as unowned, so faction 0 can never claim the bonus
    // and neither can anyone standing on it.
    rp_reset();
    rp_place(0, RP_PROTO_LAND, RP_X, RP_Y);
    rp_veh(0).faction_id = 0;
    rp_veh(0).dmg_incurred = 100;
    rp_at(RP_X, RP_Y).territory = 0;
    rp_run(0);
    RPCHECK(rp_veh(0).dmg_incurred == 99);
    RP_GUARDS();

    // The reactor is the multiplier on the whole rate, not an addend.
    for (int reactor = 0; reactor <= 4; reactor++) {
        rp_reset();
        rp_place(0, RP_PROTO_LAND, RP_X, RP_Y);
        rp_veh(0).dmg_incurred = 100;
        rp_at(RP_X, RP_Y).bit = BIT_BUNKER;
        rp_at(RP_X, RP_Y).territory = (int8_t)RP_FACTION;    // rate 3
        g_rp_world.protos[RP_PROTO_LAND].reactor_id = (uint8_t)reactor;
        rp_run(RP_FACTION);
        RPCHECK(rp_veh(0).dmg_incurred == (uint8_t)(100 - 3 * reactor));
        RP_GUARDS();
    }
}
GAMEPLAY_CASE(test_repair_phase_rate);

void test_repair_phase_base() {
    RepairSeams seams;

    // A base adds one, and rioting takes it away again.
    rp_reset();
    rp_make_base(0, RP_BASE_X, RP_BASE_Y, RP_FACTION);
    rp_place(0, RP_PROTO_LAND, RP_BASE_X, RP_BASE_Y);
    rp_veh(0).dmg_incurred = 100;
    rp_run(RP_FACTION);
    RPCHECK(rp_veh(0).dmg_incurred == 98);   // 1 + 1
    RP_GUARDS();

    rp_reset();
    rp_make_base(0, RP_BASE_X, RP_BASE_Y, RP_FACTION);
    g_rp_world.bases[0].state = BSTATE_DRONE_RIOTS_ACTIVE;
    rp_place(0, RP_PROTO_LAND, RP_BASE_X, RP_BASE_Y);
    rp_veh(0).dmg_incurred = 100;
    rp_run(RP_FACTION);
    RPCHECK(rp_veh(0).dmg_incurred == 99);   // the base contributes nothing
    RP_GUARDS();

    // The base found is the one on the unit's tile, not base zero.
    rp_reset();
    rp_make_base(0, RP_BASE_X, RP_BASE_Y, RP_FACTION);
    rp_make_base(1, RP_X, RP_Y, RP_FACTION);
    rp_give_fac(1, FAC_COMMAND_CENTER);
    rp_place(0, RP_PROTO_LAND, RP_X, RP_Y);
    rp_veh(0).dmg_incurred = 100;
    rp_run(RP_FACTION);
    RPCHECK(rp_veh(0).dmg_incurred == 0);
    RP_GUARDS();

    // ---- the triad's own repair facility, and its secret project -----------
    struct Facility {
        int proto_id;
        uint32_t facility_id;
        int project_id;
        bool heals;
        const char *why;
    };
    const Facility cases[] = {
        { RP_PROTO_LAND, FAC_COMMAND_CENTER,    -1, true,  "land: Command Center" },
        { RP_PROTO_LAND, FAC_NAVAL_YARD,        -1, false, "land: Naval Yard is the wrong one" },
        { RP_PROTO_LAND, FAC_AEROSPACE_COMPLEX, -1, false, "land: Aerospace is the wrong one" },
        { RP_PROTO_SEA,  FAC_NAVAL_YARD,        -1, true,  "sea: Naval Yard" },
        { RP_PROTO_SEA,  FAC_COMMAND_CENTER,    -1, false, "sea: Command Center is wrong" },
        { RP_PROTO_AIR,  FAC_AEROSPACE_COMPLEX, -1, true,  "air: Aerospace Complex" },
        { RP_PROTO_AIR,  FAC_NAVAL_YARD,        -1, false, "air: Naval Yard is wrong" },
        { RP_PROTO_ODD_TRIAD, FAC_COMMAND_CENTER, -1, true, "triad 7 falls to Command Center" },
        { RP_PROTO_LAND, 0, SP_COMMAND_NEXUS,           true,  "land: Command Nexus" },
        { RP_PROTO_LAND, 0, SP_MARITIME_CONTROL_CENTER, false, "land: the wrong project" },
        { RP_PROTO_SEA,  0, SP_MARITIME_CONTROL_CENTER, true,  "sea: Maritime Control Center" },
        { RP_PROTO_SEA,  0, SP_COMMAND_NEXUS,           false, "sea: the wrong project" },
        { RP_PROTO_AIR,  0, SP_CLOUDBASE_ACADEMY,       true,  "air: Cloudbase Academy" },
        { RP_PROTO_AIR,  0, SP_COMMAND_NEXUS,           false, "air: the wrong project" },
        { RP_PROTO_ODD_TRIAD, 0, SP_COMMAND_NEXUS,      true,  "triad 7 falls to Command Nexus" },
    };
    for (size_t i = 0; i < sizeof(cases) / sizeof(cases[0]); i++) {
        rp_reset();
        rp_make_base(0, RP_BASE_X, RP_BASE_Y, RP_FACTION);
        if (cases[i].facility_id) {
            rp_give_fac(0, cases[i].facility_id);
        }
        if (cases[i].project_id >= 0) {
            rp_build_project(cases[i].project_id, 0);
        }
        rp_place(0, cases[i].proto_id, RP_BASE_X, RP_BASE_Y);
        rp_veh(0).dmg_incurred = 100;
        rp_run(RP_FACTION);
        const uint8_t expected = cases[i].heals ? 0 : 98;
        if (rp_veh(0).dmg_incurred != expected) {
            std::fprintf(stderr, "repair_phase base: %s\n", cases[i].why);
        }
        RPCHECK(rp_veh(0).dmg_incurred == expected);
        RP_GUARDS();
    }

    // The project has to be held by the unit's faction, not merely built.
    rp_reset();
    rp_make_base(0, RP_BASE_X, RP_BASE_Y, RP_FACTION);
    rp_make_base(1, RP_X, RP_Y, RP_OTHER);
    rp_build_project(SP_COMMAND_NEXUS, 1);
    rp_place(0, RP_PROTO_LAND, RP_BASE_X, RP_BASE_Y);
    rp_veh(0).dmg_incurred = 100;
    rp_run(RP_FACTION);
    RPCHECK(rp_veh(0).dmg_incurred == 98);
    RP_GUARDS();

    // ---- native life is healed by the BASE OWNER's lifecycle bonus ---------
    // The base's own Centauri Preserve is enough on its own.
    rp_reset();
    rp_make_base(0, RP_BASE_X, RP_BASE_Y, RP_OTHER);
    rp_give_fac(0, FAC_CENTAURI_PRESERVE);
    rp_place(0, RP_PROTO_PSI, RP_BASE_X, RP_BASE_Y);
    rp_veh(0).dmg_incurred = 100;
    rp_run(RP_FACTION);
    RPCHECK(rp_veh(0).dmg_incurred == 0);
    RP_GUARDS();

    // ...and the faction breed_mod is asked about is the BASE OWNER, not the
    // unit's owner. Only breed_mod's has_project() terms depend on the faction
    // at all - its facility terms are faction-blind - so a project is the only
    // fixture that can tell the two arguments apart. Pholus Mutagen sits in a
    // base belonging to the third faction, so the unit's own faction does not
    // hold it, and native life of faction 1 is still healed outright.
    rp_reset();
    rp_make_base(0, RP_BASE_X, RP_BASE_Y, RP_OTHER);
    rp_build_project(SP_PHOLUS_MUTAGEN, 0);
    rp_place(0, RP_PROTO_PSI, RP_BASE_X, RP_BASE_Y);
    rp_veh(0).dmg_incurred = 100;
    rp_run(RP_FACTION);
    RPCHECK(rp_veh(0).dmg_incurred == 0);
    RP_GUARDS();

    // The same project in a base the third faction does NOT own buys nothing,
    // which is what makes the case above an assertion about the argument
    // rather than about the project existing.
    rp_reset();
    rp_make_base(0, RP_BASE_X, RP_BASE_Y, RP_OTHER);
    rp_make_base(1, RP_X, RP_Y, RP_FACTION);
    rp_build_project(SP_PHOLUS_MUTAGEN, 1);
    rp_place(0, RP_PROTO_PSI, RP_BASE_X, RP_BASE_Y);
    rp_veh(0).dmg_incurred = 100;
    rp_run(RP_FACTION);
    RPCHECK(rp_veh(0).dmg_incurred == 98);
    RP_GUARDS();

    // Without the bonus the base is worth its ordinary single point, and the
    // Command Center it does have is not consulted for native life.
    rp_reset();
    rp_make_base(0, RP_BASE_X, RP_BASE_Y, RP_OTHER);
    rp_give_fac(0, FAC_COMMAND_CENTER);
    rp_place(0, RP_PROTO_PSI, RP_BASE_X, RP_BASE_Y);
    rp_veh(0).dmg_incurred = 100;
    rp_run(RP_FACTION);
    RPCHECK(rp_veh(0).dmg_incurred == 98);
    RP_GUARDS();

    // ---- which prototypes count as native life -----------------------------
    // A base carrying a lifecycle bonus and NO repair facility heals native
    // life outright and nobody else, so each of the two guards is asked over a
    // prototype differing from the native one in exactly the way it tests.
    struct Native {
        int proto_id;
        bool is_native;
        const char *why;
    };
    const Native natives[] = {
        { RP_PROTO_PSI,          true,  "offense -1 and predefined" },
        { RP_PROTO_ZERO_OFFENSE, false, "offense 0 is not negative" },
        { RP_PROTO_LAND,         false, "offense 4 is not negative" },
        { RP_PROTO_PSI_CUSTOM,   false, "prototype 64 is one past predefined" },
    };
    for (size_t i = 0; i < sizeof(natives) / sizeof(natives[0]); i++) {
        rp_reset();
        rp_make_base(0, RP_BASE_X, RP_BASE_Y, RP_OTHER);
        rp_give_fac(0, FAC_CENTAURI_PRESERVE);
        rp_place(0, natives[i].proto_id, RP_BASE_X, RP_BASE_Y);
        rp_veh(0).dmg_incurred = 100;
        rp_run(RP_FACTION);
        const uint8_t expected = natives[i].is_native ? 0 : 98;
        if (rp_veh(0).dmg_incurred != expected) {
            std::fprintf(stderr, "repair_phase native: %s\n", natives[i].why);
        }
        RPCHECK(rp_veh(0).dmg_incurred == expected);
        RP_GUARDS();
    }

    // The same four in a base with the land repair facility and no lifecycle
    // bonus, where every answer is the other way round.
    for (size_t i = 0; i < sizeof(natives) / sizeof(natives[0]); i++) {
        rp_reset();
        rp_make_base(0, RP_BASE_X, RP_BASE_Y, RP_OTHER);
        rp_give_fac(0, FAC_COMMAND_CENTER);
        rp_place(0, natives[i].proto_id, RP_BASE_X, RP_BASE_Y);
        rp_veh(0).dmg_incurred = 100;
        rp_run(RP_FACTION);
        RPCHECK(rp_veh(0).dmg_incurred == (natives[i].is_native ? 98 : 0));
        RP_GUARDS();
    }
}
GAMEPLAY_CASE(test_repair_phase_base);

void test_repair_phase_stack_and_projects() {
    RepairSeams seams;

    // ---- a land transport with a repair unit aboard doubles the rate -------
    // At sea, so the transport is genuinely carrying rather than escorting.
    rp_reset();
    rp_place(0, RP_PROTO_TRANSPORT, RP_X, RP_Y);
    rp_place(1, RP_PROTO_REPAIR, RP_X, RP_Y);
    rp_at(RP_X, RP_Y).climate = RP_ALT_SHELF;
    rp_veh(0).next_veh_id_stack = 1;
    rp_veh(1).prev_veh_id_stack = 0;
    rp_veh(0).dmg_incurred = 100;
    rp_run(RP_FACTION);
    RPCHECK(rp_veh(0).dmg_incurred == 98);   // rate 1, doubled
    RP_GUARDS();

    // The same stack without the ability heals at the plain rate, and the
    // minimum-damage floor comes back with it.
    rp_reset();
    rp_place(0, RP_PROTO_TRANSPORT, RP_X, RP_Y);
    rp_place(1, RP_PROTO_LAND, RP_X, RP_Y);
    rp_at(RP_X, RP_Y).climate = RP_ALT_SHELF;
    rp_veh(0).next_veh_id_stack = 1;
    rp_veh(1).prev_veh_id_stack = 0;
    rp_veh(0).dmg_incurred = 100;
    rp_run(RP_FACTION);
    RPCHECK(rp_veh(0).dmg_incurred == 99);
    RP_GUARDS();

    // The transport does not count itself: a repair-capable transport alone in
    // its stack gets nothing.
    rp_reset();
    rp_place(0, RP_PROTO_TRANSPORT, RP_X, RP_Y);
    g_rp_world.protos[RP_PROTO_TRANSPORT].ability_flags = ABL_REPAIR;
    rp_at(RP_X, RP_Y).climate = RP_ALT_SHELF;
    rp_veh(0).dmg_incurred = 100;
    rp_run(RP_FACTION);
    RPCHECK(rp_veh(0).dmg_incurred == 99);
    RP_GUARDS();

    // The walk starts at the TOP of the stack, so a repair unit above the
    // transport is found as readily as one below it.
    rp_reset();
    rp_place(0, RP_PROTO_REPAIR, RP_X, RP_Y);
    rp_place(1, RP_PROTO_TRANSPORT, RP_X, RP_Y);
    rp_at(RP_X, RP_Y).climate = RP_ALT_SHELF;
    rp_veh(0).next_veh_id_stack = 1;
    rp_veh(1).prev_veh_id_stack = 0;
    rp_veh(1).dmg_incurred = 100;
    rp_run(RP_FACTION);
    RPCHECK(rp_veh(1).dmg_incurred == 98);
    RP_GUARDS();

    // ---- the three gates on the doubling -----------------------------------
    // On land, and not under Sentry/Board: no doubling.
    rp_reset();
    rp_place(0, RP_PROTO_TRANSPORT, RP_X, RP_Y);
    rp_place(1, RP_PROTO_REPAIR, RP_X, RP_Y);
    rp_veh(0).next_veh_id_stack = 1;
    rp_veh(1).prev_veh_id_stack = 0;
    rp_veh(0).dmg_incurred = 100;
    rp_run(RP_FACTION);
    RPCHECK(rp_veh(0).dmg_incurred == 99);
    RP_GUARDS();

    // On land, under Sentry/Board: doubling.
    rp_reset();
    rp_place(0, RP_PROTO_TRANSPORT, RP_X, RP_Y);
    rp_place(1, RP_PROTO_REPAIR, RP_X, RP_Y);
    rp_veh(0).next_veh_id_stack = 1;
    rp_veh(1).prev_veh_id_stack = 0;
    rp_veh(0).order = ORDER_SENTRY_BOARD;
    rp_veh(0).dmg_incurred = 100;
    rp_run(RP_FACTION);
    RPCHECK(rp_veh(0).dmg_incurred == 98);
    RP_GUARDS();

    // Carrying nothing: no doubling, even at sea with a repair unit stacked.
    rp_reset();
    rp_place(0, RP_PROTO_LAND, RP_X, RP_Y);
    rp_place(1, RP_PROTO_REPAIR, RP_X, RP_Y);
    rp_at(RP_X, RP_Y).climate = RP_ALT_SHELF;
    rp_veh(0).next_veh_id_stack = 1;
    rp_veh(1).prev_veh_id_stack = 0;
    rp_veh(0).dmg_incurred = 100;
    rp_run(RP_FACTION);
    RPCHECK(rp_veh(0).dmg_incurred == 99);
    RP_GUARDS();

    // The sea gate is the shore line exactly: a shore-line tile is ashore, so
    // the transport is escorting rather than carrying and nothing doubles.
    rp_reset();
    rp_place(0, RP_PROTO_TRANSPORT, RP_X, RP_Y);
    rp_place(1, RP_PROTO_REPAIR, RP_X, RP_Y);
    rp_at(RP_X, RP_Y).climate = RP_ALT_SHORE;
    rp_veh(0).next_veh_id_stack = 1;
    rp_veh(1).prev_veh_id_stack = 0;
    rp_veh(0).dmg_incurred = 100;
    rp_run(RP_FACTION);
    RPCHECK(rp_veh(0).dmg_incurred == 99);
    RP_GUARDS();

    // One altitude step below it, the same stack doubles.
    rp_reset();
    rp_place(0, RP_PROTO_TRANSPORT, RP_X, RP_Y);
    rp_place(1, RP_PROTO_REPAIR, RP_X, RP_Y);
    rp_at(RP_X, RP_Y).climate = RP_ALT_SHELF;
    rp_veh(0).next_veh_id_stack = 1;
    rp_veh(1).prev_veh_id_stack = 0;
    rp_veh(0).dmg_incurred = 100;
    rp_run(RP_FACTION);
    RPCHECK(rp_veh(0).dmg_incurred == 98);
    RP_GUARDS();

    // A sea transport is not a land transport.
    rp_reset();
    rp_place(0, RP_PROTO_TRANSPORT, RP_X, RP_Y);
    g_rp_world.protos[RP_PROTO_TRANSPORT].chassis_id = RP_CHASSIS_SEA;
    rp_place(1, RP_PROTO_REPAIR, RP_X, RP_Y);
    rp_at(RP_X, RP_Y).climate = RP_ALT_SHELF;
    rp_veh(0).next_veh_id_stack = 1;
    rp_veh(1).prev_veh_id_stack = 0;
    rp_veh(0).dmg_incurred = 100;
    rp_run(RP_FACTION);
    RPCHECK(rp_veh(0).dmg_incurred == 99);
    RP_GUARDS();

    // ---- the Nano Factory heals everything, everywhere ---------------------
    rp_reset();
    rp_make_base(1, RP_BASE_X, RP_BASE_Y, RP_FACTION);
    rp_build_project(SP_NANO_FACTORY, 1);
    rp_place(0, RP_PROTO_LAND, RP_X, RP_Y);      // in the open, nowhere near it
    rp_veh(0).dmg_incurred = 100;
    rp_run(RP_FACTION);
    RPCHECK(rp_veh(0).dmg_incurred == 0);
    RP_GUARDS();

    // Held by somebody else, it does nothing - and the field floor is back.
    rp_reset();
    rp_make_base(1, RP_BASE_X, RP_BASE_Y, RP_OTHER);
    rp_build_project(SP_NANO_FACTORY, 1);
    rp_place(0, RP_PROTO_LAND, RP_X, RP_Y);
    rp_veh(0).dmg_incurred = 100;
    rp_run(RP_FACTION);
    RPCHECK(rp_veh(0).dmg_incurred == 99);
    RP_GUARDS();

    // ---- the Xenoempathy Dome: one more reactor, and no floor --------------
    // Reactor 2 in fungus. Without the Dome the floor is 4 and the unit stops
    // there; with it the floor is gone and a third reactor's worth comes off.
    rp_reset();
    rp_place(0, RP_PROTO_LAND, RP_X, RP_Y);
    g_rp_world.protos[RP_PROTO_LAND].reactor_id = 2;
    rp_at(RP_X, RP_Y).bit = BIT_FUNGUS;
    rp_veh(0).dmg_incurred = 5;
    rp_run(RP_FACTION);
    RPCHECK(rp_veh(0).dmg_incurred == 4);    // range(3, 4, 999)
    RP_GUARDS();

    rp_reset();
    rp_make_base(1, RP_BASE_X, RP_BASE_Y, RP_FACTION);
    rp_build_project(SP_XENOEMPATYH_DOME, 1);
    rp_place(0, RP_PROTO_LAND, RP_X, RP_Y);
    g_rp_world.protos[RP_PROTO_LAND].reactor_id = 2;
    rp_at(RP_X, RP_Y).bit = BIT_FUNGUS;
    rp_veh(0).dmg_incurred = 5;
    rp_run(RP_FACTION);
    RPCHECK(rp_veh(0).dmg_incurred == 1);    // 5 - (2 * 1 + 2), floor 0
    RP_GUARDS();

    // The Dome only applies in fungus.
    rp_reset();
    rp_make_base(1, RP_BASE_X, RP_BASE_Y, RP_FACTION);
    rp_build_project(SP_XENOEMPATYH_DOME, 1);
    rp_place(0, RP_PROTO_LAND, RP_X, RP_Y);
    g_rp_world.protos[RP_PROTO_LAND].reactor_id = 2;
    rp_veh(0).dmg_incurred = 5;
    rp_run(RP_FACTION);
    RPCHECK(rp_veh(0).dmg_incurred == 4);
    RP_GUARDS();

    // ...and not in fungus too deep for it.
    rp_reset();
    rp_make_base(1, RP_BASE_X, RP_BASE_Y, RP_FACTION);
    rp_build_project(SP_XENOEMPATYH_DOME, 1);
    rp_place(0, RP_PROTO_LAND, RP_X, RP_Y);
    g_rp_world.protos[RP_PROTO_LAND].reactor_id = 2;
    rp_at(RP_X, RP_Y).bit = BIT_FUNGUS;
    rp_at(RP_X, RP_Y).climate = RP_ALT_DEEP;
    rp_veh(0).dmg_incurred = 5;
    rp_run(RP_FACTION);
    RPCHECK(rp_veh(0).dmg_incurred == 4);
    RP_GUARDS();
}
GAMEPLAY_CASE(test_repair_phase_stack_and_projects);

void test_repair_phase_damage_math() {
    RepairSeams seams;

    // ---- the minimum damage a unit in the field keeps ----------------------
    // Reactor 2 gives a floor of 4, and the unit walks down to it and stops.
    rp_reset();
    rp_place(0, RP_PROTO_LAND, RP_X, RP_Y);
    g_rp_world.protos[RP_PROTO_LAND].reactor_id = 2;
    rp_veh(0).dmg_incurred = 10;
    rp_run(RP_FACTION);
    RPCHECK(rp_veh(0).dmg_incurred == 8);
    rp_run(RP_FACTION);
    RPCHECK(rp_veh(0).dmg_incurred == 6);
    rp_run(RP_FACTION);
    RPCHECK(rp_veh(0).dmg_incurred == 4);
    rp_run(RP_FACTION);
    RPCHECK(rp_veh(0).dmg_incurred == 4);
    RP_GUARDS();

    // Below the floor already: repairing must never ADD damage. Without the
    // final `if (repaired >= damage)` this unit would come out at 4.
    rp_reset();
    rp_place(0, RP_PROTO_LAND, RP_X, RP_Y);
    g_rp_world.protos[RP_PROTO_LAND].reactor_id = 2;
    rp_veh(0).dmg_incurred = 3;
    rp_run(RP_FACTION);
    RPCHECK(rp_veh(0).dmg_incurred == 3);
    RP_GUARDS();

    // The floor is lifted by each of the four exemptions in turn.
    rp_reset();
    rp_place(0, RP_PROTO_LAND, RP_X, RP_Y);
    rp_veh(0).faction_id = 0;                    // faction 0 has no floor
    g_rp_world.protos[RP_PROTO_LAND].reactor_id = 2;
    rp_veh(0).dmg_incurred = 5;
    rp_run(0);
    RPCHECK(rp_veh(0).dmg_incurred == 3);
    RP_GUARDS();

    rp_reset();
    rp_make_base(0, RP_BASE_X, RP_BASE_Y, RP_FACTION);
    g_rp_world.bases[0].state = BSTATE_DRONE_RIOTS_ACTIVE;   // no base bonus, but still a base
    rp_place(0, RP_PROTO_LAND, RP_BASE_X, RP_BASE_Y);
    g_rp_world.protos[RP_PROTO_LAND].reactor_id = 2;
    rp_veh(0).dmg_incurred = 5;
    rp_run(RP_FACTION);
    RPCHECK(rp_veh(0).dmg_incurred == 3);
    RP_GUARDS();

    rp_reset();
    rp_place(0, RP_PROTO_TRANSPORT, RP_X, RP_Y);
    rp_place(1, RP_PROTO_REPAIR, RP_X, RP_Y);
    rp_at(RP_X, RP_Y).climate = RP_ALT_SHELF;
    rp_veh(0).next_veh_id_stack = 1;
    rp_veh(1).prev_veh_id_stack = 0;
    g_rp_world.protos[RP_PROTO_TRANSPORT].reactor_id = 2;
    rp_veh(0).dmg_incurred = 5;
    rp_run(RP_FACTION);
    RPCHECK(rp_veh(0).dmg_incurred == 1);        // rate 2, reactor 2, no floor
    RP_GUARDS();

    rp_reset();
    rp_place(0, RP_PROTO_PSI, RP_X, RP_Y);
    rp_at(RP_X, RP_Y).bit = BIT_FUNGUS;
    g_rp_world.protos[RP_PROTO_PSI].reactor_id = 2;
    rp_veh(0).dmg_incurred = 5;
    rp_run(RP_FACTION);
    RPCHECK(rp_veh(0).dmg_incurred == 1);        // native life in fungus keeps no floor
    RP_GUARDS();

    // Reactor 0 makes both the removal and the floor zero, so nothing moves.
    rp_reset();
    rp_place(0, RP_PROTO_LAND, RP_X, RP_Y);
    g_rp_world.protos[RP_PROTO_LAND].reactor_id = 0;
    rp_veh(0).dmg_incurred = 7;
    rp_run(RP_FACTION);
    RPCHECK(rp_veh(0).dmg_incurred == 7);
    RP_GUARDS();

    // ---- the exact edges of the two floor exemptions -----------------------
    // Reactor 2 against 4 damage: the floor is 4 and the unit cannot move at
    // all unless an exemption lifts it, and it reaches zero only if that
    // exemption is a real zero.
    struct Floor {
        int proto_id;
        uint32_t tile_bit;
        uint8_t climate;
        bool dome;
        uint8_t expected;
        const char *why;
    };
    const Floor floors[] = {
        { RP_PROTO_LAND, BIT_FUNGUS, RP_ALT_LAND,  false, 4, "no exemption at all" },
        { RP_PROTO_PSI,  BIT_FUNGUS, RP_ALT_LAND,  false, 0, "native life in land fungus" },
        { RP_PROTO_PSI,  BIT_FUNGUS, RP_ALT_SHELF, false, 0, "the shelf is shallow enough" },
        { RP_PROTO_PSI,  BIT_FUNGUS, RP_ALT_DEEP,  false, 4, "below the shelf it is not" },
        { RP_PROTO_PSI,  0,          RP_ALT_LAND,  false, 4, "native life needs the fungus" },
        { RP_PROTO_ZERO_OFFENSE, BIT_FUNGUS, RP_ALT_LAND, false, 4, "offense 0 is not native" },
        { RP_PROTO_PSI_CUSTOM,   BIT_FUNGUS, RP_ALT_LAND, false, 4, "prototype 64 is not native" },
        { RP_PROTO_LAND, BIT_FUNGUS, RP_ALT_LAND,  true,  0, "the Dome, in land fungus" },
        { RP_PROTO_LAND, BIT_FUNGUS, RP_ALT_SHELF, true,  0, "the Dome, on the shelf" },
        { RP_PROTO_LAND, BIT_FUNGUS, RP_ALT_DEEP,  true,  4, "the Dome, too deep" },
        { RP_PROTO_LAND, 0,          RP_ALT_LAND,  true,  4, "the Dome needs the fungus" },
    };
    for (size_t i = 0; i < sizeof(floors) / sizeof(floors[0]); i++) {
        rp_reset();
        if (floors[i].dome) {
            rp_make_base(1, RP_BASE_X, RP_BASE_Y, RP_FACTION);
            rp_build_project(SP_XENOEMPATYH_DOME, 1);
        }
        rp_place(0, floors[i].proto_id, RP_X, RP_Y);
        g_rp_world.protos[floors[i].proto_id].reactor_id = 2;
        rp_at(RP_X, RP_Y).bit = floors[i].tile_bit;
        rp_at(RP_X, RP_Y).climate = floors[i].climate;
        rp_veh(0).dmg_incurred = 4;
        rp_run(RP_FACTION);
        if (rp_veh(0).dmg_incurred != floors[i].expected) {
            std::fprintf(stderr, "repair_phase floor: %s\n", floors[i].why);
        }
        RPCHECK(rp_veh(0).dmg_incurred == floors[i].expected);
        RP_GUARDS();
    }

    // ---- waking a human player's sentry ------------------------------------
    struct Wake {
        int proto_id;
        uint8_t climate;
        bool human;
        int order;
        uint8_t damage;
        bool woken;
        const char *why;
    };
    const Wake cases[] = {
        { RP_PROTO_LAND, RP_ALT_LAND,  true,  ORDER_SENTRY_BOARD, 3, true,  "land unit ashore" },
        { RP_PROTO_LAND, RP_ALT_SHELF, true,  ORDER_SENTRY_BOARD, 3, false, "land unit aboard" },
        { RP_PROTO_LAND, RP_ALT_SHORE, true,  ORDER_SENTRY_BOARD, 3, true,  "the shore line is ashore" },
        { RP_PROTO_SEA,  RP_ALT_SHELF, true,  ORDER_SENTRY_BOARD, 3, true,  "sea unit at sea" },
        { RP_PROTO_AIR,  RP_ALT_SHELF, true,  ORDER_SENTRY_BOARD, 3, true,  "air unit at sea" },
        { RP_PROTO_LAND, RP_ALT_LAND,  false, ORDER_SENTRY_BOARD, 3, false, "an AI is not woken" },
        { RP_PROTO_LAND, RP_ALT_LAND,  true,  ORDER_HOLD,         3, false, "Hold is not Sentry" },
        { RP_PROTO_LAND, RP_ALT_LAND,  true,  ORDER_SENTRY_BOARD, 2, false, "already at the floor" },
        { RP_PROTO_LAND, RP_ALT_LAND,  true,  ORDER_SENTRY_BOARD, 9, false, "still above the floor" },
    };
    for (size_t i = 0; i < sizeof(cases) / sizeof(cases[0]); i++) {
        rp_reset();
        rp_place(0, cases[i].proto_id, RP_X, RP_Y);
        rp_at(RP_X, RP_Y).climate = cases[i].climate;
        g_rp_world.factions_status[0] = cases[i].human ? (uint8_t)(1 << RP_FACTION) : 0;
        rp_veh(0).order = (int8_t)cases[i].order;
        rp_veh(0).waypoint_y[0] = 0;
        rp_veh(0).dmg_incurred = cases[i].damage;
        rp_run(RP_FACTION);
        const int expected = cases[i].woken ? ORDER_NONE : cases[i].order;
        if (rp_veh(0).order != expected) {
            std::fprintf(stderr, "repair_phase wake: %s\n", cases[i].why);
        }
        RPCHECK(rp_veh(0).order == expected);
        RP_GUARDS();
    }

    // The "damage actually changed" test is separate from the floor test: a
    // unit already resting on the floor is not woken, but one that reaches it
    // this turn is.
    rp_reset();
    rp_place(0, RP_PROTO_LAND, RP_X, RP_Y);
    g_rp_world.factions_status[0] = (uint8_t)(1 << RP_FACTION);
    rp_veh(0).order = ORDER_SENTRY_BOARD;
    rp_veh(0).dmg_incurred = 3;
    rp_run(RP_FACTION);
    RPCHECK(rp_veh(0).dmg_incurred == 2);
    RPCHECK(rp_veh(0).order == ORDER_NONE);
    RP_GUARDS();
}
GAMEPLAY_CASE(test_repair_phase_damage_math);

void test_repair_phase_redraw() {
    RepairSeams seams;

    // ---- what the second pass draws, and with which arguments --------------
    rp_reset();
    rp_place(0, RP_PROTO_LAND, RP_X, RP_Y);
    rp_place(1, RP_PROTO_LAND, 4, 6);
    rp_place(2, RP_PROTO_LAND, 4, 6);
    rp_veh(2).faction_id = (uint8_t)RP_OTHER;
    rp_run(RP_FACTION);
    RPCHECK(g_rp_draw_calls.size() == 2);
    if (g_rp_draw_calls.size() == 2) {
        RPCHECK(g_rp_draw_calls[0].x == RP_X);
        RPCHECK(g_rp_draw_calls[0].y == RP_Y);
        RPCHECK(g_rp_draw_calls[0].draw_type == -1);
        RPCHECK(g_rp_draw_calls[1].x == 4);
        RPCHECK(g_rp_draw_calls[1].y == 6);
        RPCHECK(g_rp_draw_calls[1].draw_type == -1);
    }
    RP_GUARDS();

    // A unit standing in somebody's base is not drawn; the base's own draw
    // covers the tile.
    rp_reset();
    rp_place(0, RP_PROTO_LAND, RP_X, RP_Y);
    rp_at(RP_X, RP_Y).bit = BIT_BASE_IN_TILE;
    rp_at(RP_X, RP_Y).val2 = RP_OTHER;
    rp_run(RP_FACTION);
    RPCHECK(g_rp_draw_calls.empty());
    RP_GUARDS();

    // The base-tile skip needs BOTH the bit and an owner in range.
    rp_reset();
    rp_place(0, RP_PROTO_LAND, RP_X, RP_Y);
    rp_at(RP_X, RP_Y).val2 = RP_OTHER;   // an owner nibble without the bit
    rp_run(RP_FACTION);
    RPCHECK(g_rp_draw_calls.size() == 1);
    RP_GUARDS();

    rp_reset();
    rp_place(0, RP_PROTO_LAND, RP_X, RP_Y);
    rp_at(RP_X, RP_Y).bit = BIT_BASE_IN_TILE;
    rp_at(RP_X, RP_Y).val2 = 0x0F;       // the bit without an owner in range
    rp_run(RP_FACTION);
    RPCHECK(g_rp_draw_calls.size() == 1);
    RP_GUARDS();

    // A base owned by faction 0 is still a base: the owner test in the redraw
    // pass is a RANGE, and its lower half is not a truth test.
    rp_reset();
    rp_place(0, RP_PROTO_LAND, RP_X, RP_Y);
    rp_at(RP_X, RP_Y).bit = BIT_BASE_IN_TILE;
    rp_at(RP_X, RP_Y).val2 = 0;
    rp_run(RP_FACTION);
    RPCHECK(g_rp_draw_calls.empty());
    RP_GUARDS();

    // Owner 8 is one past the last faction, so that tile IS drawn.
    rp_reset();
    rp_place(0, RP_PROTO_LAND, RP_X, RP_Y);
    rp_at(RP_X, RP_Y).bit = BIT_BASE_IN_TILE;
    rp_at(RP_X, RP_Y).val2 = 8;
    rp_run(RP_FACTION);
    RPCHECK(g_rp_draw_calls.size() == 1);
    RP_GUARDS();

    // ---- the local player's view ------------------------------------------
    // Somebody else's turn: the tile is only redrawn if the local player can
    // see the unit.
    rp_reset();
    rp_place(0, RP_PROTO_LAND, RP_X, RP_Y);
    g_rp_world.local_faction = RP_OTHER;
    rp_run(RP_FACTION);
    RPCHECK(g_rp_draw_calls.empty());
    RP_GUARDS();

    rp_reset();
    rp_place(0, RP_PROTO_LAND, RP_X, RP_Y);
    g_rp_world.local_faction = RP_OTHER;
    rp_veh(0).visibility = (uint8_t)(1 << RP_OTHER);
    rp_run(RP_FACTION);
    RPCHECK(g_rp_draw_calls.size() == 1);
    RP_GUARDS();

    // A visibility bit for the wrong faction is not the local player's.
    rp_reset();
    rp_place(0, RP_PROTO_LAND, RP_X, RP_Y);
    g_rp_world.local_faction = RP_OTHER;
    rp_veh(0).visibility = (uint8_t)~(1 << RP_OTHER);
    rp_run(RP_FACTION);
    RPCHECK(g_rp_draw_calls.empty());
    RP_GUARDS();

    // The draw pass runs even when the repair pass changed nothing, and the
    // pump runs exactly once regardless of how many tiles were drawn.
    rp_reset();
    rp_place(0, RP_PROTO_LAND, RP_X, RP_Y);
    rp_place(1, RP_PROTO_LAND, 6, 2);
    rp_place(2, RP_PROTO_LAND, 10, 0);
    rp_run(RP_FACTION);
    RPCHECK(g_rp_draw_calls.size() == 3);
    RP_GUARDS();
}
GAMEPLAY_CASE(test_repair_phase_redraw);

#undef RP_GUARDS
#undef RPCHECK

/*
 * alien_base (0x005665D0): which base a native life form heads for.
 *
 * The subject is deliberately unit 3 rather than unit 0, so an implementation
 * that read Vehs[0] instead of Vehs[veh_id] would be caught rather than
 * accidentally right. Unit 5 is loaded with the opposite of everything the
 * subject wants for the same reason.
 *
 * Everything the body reaches is real: region_at, x_dist, vector_dist,
 * base_on_sea, veh_at and stack_check all run against this world, so the
 * fixture has to satisfy them. Three consequences are worth stating because
 * they are easy to get wrong and silent when wrong:
 *
 *   - every unit's next/prev stack links start at -1. Zeroed links make
 *     veh_top() walk from unit 0 to unit 0 forever.
 *   - BIT_VEH_IN_TILE is set only where a unit actually stands. veh_at()
 *     falls through to log_say() for a tile that claims a unit and has none.
 *   - the map is 16x8 with MapLongitude 8, so a tile index is (x >> 1) + y * 8
 *     and MapTiles is aimed at the middle of a 192-entry array.
 */
struct AlienWorld {
    Map tiles[192];
    Base bases[8];
    Veh vehs[8];
    VehPrototype protos[16];
    PlayerData players_data[MaxPlayerNum];
    Map *tiles_ptr;
    uint32_t longitude;
    int lon_bounds;
    int lat_bounds;
    BOOL is_flat;
    int base_count;
    int base_find_dist;
    int veh_count;
};

AlienWorld g_alien_world;
AlienWorld g_alien_saved;

const int AB_LIVE = 64;                 // MapTiles aims here
const int AB_VEH = 3;                   // the subject, deliberately not zero
const int AB_DECOY_VEH = 5;             // holds the opposite of every subject field
const int AB_X = 0;                     // where the subject stands
const int AB_Y = 0;
const int AB_REGION = 2;                // the region the subject stands in
const int AB_BASE_X = 14;               // the reference base
const int AB_BASE_Y = 6;
const int AB_FACTION = 1;
const int AB_DIST_SENTINEL = 0x5A5A5A5A;

Map &ab_at(int x, int y) {
    return g_alien_world.tiles[AB_LIVE + (x >> 1) + y * 8];
}

Base &ab_base(int base_id) {
    return g_alien_world.bases[base_id];
}

void ab_reset() {
    std::memset(&g_alien_world, 0, sizeof(g_alien_world));
    g_alien_world.tiles_ptr = &g_alien_world.tiles[AB_LIVE];
    g_alien_world.longitude = 8;
    g_alien_world.lon_bounds = 16;
    g_alien_world.lat_bounds = 8;
    g_alien_world.is_flat = 1;          // no x wrapping unless a test asks for it
    g_alien_world.base_find_dist = AB_DIST_SENTINEL;
    g_alien_world.veh_count = 8;
    for (int i = 0; i < 192; i++) {
        g_alien_world.tiles[i].climate = 0x80;   // dry land, well above the shore line
        g_alien_world.tiles[i].region = (uint8_t)AB_REGION;
    }
    for (int i = 0; i < 8; i++) {
        Veh &veh = g_alien_world.vehs[i];
        veh.x = -1;
        veh.y = -1;
        veh.next_veh_id_stack = -1;
        veh.prev_veh_id_stack = -1;
        veh.home_base_id = -1;
        veh.proto_id = (int16_t)BSC_MIND_WORMS;
    }
    // The decoy wants everything the subject does not.
    g_alien_world.vehs[AB_DECOY_VEH].proto_id = (int16_t)BSC_SEALURK;
    g_alien_world.vehs[AB_DECOY_VEH].home_base_id = 0;
    g_alien_world.vehs[AB_DECOY_VEH].order_auto_type = (uint8_t)AB_FACTION;
    g_alien_world.protos[BSC_ALIEN_ARTIFACT].plan = (uint8_t)PLAN_ALIEN_ARTIFACT;
    g_alien_world.protos[BSC_MIND_WORMS].plan = (uint8_t)PLAN_COMBAT;
    g_alien_world.protos[BSC_SEALURK].plan = (uint8_t)PLAN_COMBAT;
    g_alien_world.protos[BSC_SPORE_LAUNCHER].plan = (uint8_t)PLAN_COMBAT;
}

// A base of `faction` at (x, y). Intakes stay zero, so the divisor is 32.
void ab_place_base(int base_id, int x, int y, int faction_id) {
    Base &base = ab_base(base_id);
    base.x = (int16_t)x;
    base.y = (int16_t)y;
    base.faction_id_current = (uint8_t)faction_id;
    if (g_alien_world.base_count <= base_id) {
        g_alien_world.base_count = base_id + 1;
    }
}

// A unit standing on a tile, with the map bit veh_at() insists on.
//
// The faction is deliberately NOT zero. stack_check's type 2 reads cond2 as
// "only units of this faction", and the -1 the original passes means "any".
// With a zero-faction unit in the stack, replacing that -1 with 0 counts the
// unit anyway and the mutation is invisible; with faction 3 it is not.
void ab_place_unit(int veh_id, int x, int y, int proto_id) {
    Veh &veh = g_alien_world.vehs[veh_id];
    veh.x = (int16_t)x;
    veh.y = (int16_t)y;
    veh.proto_id = (int16_t)proto_id;
    veh.faction_id = 3;
    ab_at(x, y).bit |= BIT_VEH_IN_TILE;
}

void ab_snapshot() {
    std::memcpy(&g_alien_saved, &g_alien_world, sizeof(g_alien_world));
}

// Everything except the one published output has to come back untouched.
bool ab_only_dist_changed() {
    AlienWorld now;
    std::memcpy(&now, &g_alien_world, sizeof(now));
    now.base_find_dist = g_alien_saved.base_find_dist;
    return std::memcmp(&now, &g_alien_saved, sizeof(now)) == 0;
}

#define ABCHECK(cond)                                                         \
    do {                                                                      \
        const bool alien_ok = (cond);                                         \
        if (!alien_ok) {                                                      \
            std::fprintf(stderr, "alien_base: line %d: %s\n", __LINE__,       \
                         #cond);                                              \
        }                                                                     \
        expect(alien_ok);                                                     \
    } while (0)

class AlienSeams {
 public:
    AlienSeams()
        : tiles_(&MapTiles, &g_alien_world.tiles_ptr),
          longitude_(&MapLongitude, &g_alien_world.longitude),
          lon_(&MapLongitudeBounds, &g_alien_world.lon_bounds),
          lat_(&MapLatitudeBounds, &g_alien_world.lat_bounds),
          flat_(&MapIsFlat, &g_alien_world.is_flat),
          bases_(&Bases, g_alien_world.bases),
          base_count_(&BaseCurrentCount, &g_alien_world.base_count),
          base_dist_(&BaseFindDist, &g_alien_world.base_find_dist),
          vehs_(&Vehs, g_alien_world.vehs),
          protos_(&VehPrototypes, g_alien_world.protos),
          veh_count_(&VehCurrentCount, &g_alien_world.veh_count),
          players_data_(&PlayersData, g_alien_world.players_data) { }

 private:
    ScopedSeam<Map *> tiles_;
    ScopedSeam<uint32_t> longitude_;
    ScopedSeam<int> lon_;
    ScopedSeam<int> lat_;
    ScopedSeam<BOOL> flat_;
    ScopedSeam<Base> bases_;
    ScopedSeam<int> base_count_;
    ScopedSeam<int> base_dist_;
    ScopedSeam<Veh> vehs_;
    ScopedSeam<VehPrototype> protos_;
    ScopedSeam<int> veh_count_;
    ScopedSeam<PlayerData> players_data_;
};

void test_alien_base_reachability() {
    AlienSeams seams;

    // ---- no bases at all -----------------------------------------------
    // The 9999 seed is published on the way IN, so a caller that reads
    // BaseFindDist after a failed search sees 9999 and not the sentinel.
    ab_reset();
    ABCHECK(alien_base(AB_VEH, AB_X, AB_Y) == -1);
    ABCHECK(g_alien_world.base_find_dist == 9999);

    // ---- one base in the unit's own region ------------------------------
    ab_reset();
    ab_place_base(0, AB_BASE_X, AB_BASE_Y, AB_FACTION);
    ab_snapshot();
    ABCHECK(alien_base(AB_VEH, AB_X, AB_Y) == 0);
    ABCHECK(g_alien_world.base_find_dist == 12);
    ABCHECK(ab_only_dist_changed());

    // ---- a base in another region is unreachable ------------------------
    ab_reset();
    ab_place_base(0, AB_BASE_X, AB_BASE_Y, AB_FACTION);
    ab_at(AB_BASE_X, AB_BASE_Y).region = 3;
    ABCHECK(alien_base(AB_VEH, AB_X, AB_Y) == -1);
    ABCHECK(g_alien_world.base_find_dist == 9999);

    // ---- the bound is the land/water split, and it is exactly 64 --------
    // 63 is still land, so the region still has to match; 64 and above is a
    // unit already at sea, which reaches everything.
    ab_at(AB_X, AB_Y).region = 63;
    ABCHECK(alien_base(AB_VEH, AB_X, AB_Y) == -1);
    ab_at(AB_X, AB_Y).region = 64;
    ABCHECK(alien_base(AB_VEH, AB_X, AB_Y) == 0);
    ab_at(AB_X, AB_Y).region = 65;
    ABCHECK(alien_base(AB_VEH, AB_X, AB_Y) == 0);

    // ---- a Sealurk may also take a base that touches its own water ------
    // (12,6) is one of the eight tiles base_on_sea() looks at from (14,6).
    ab_reset();
    ab_place_base(0, AB_BASE_X, AB_BASE_Y, AB_FACTION);
    ab_at(AB_BASE_X, AB_BASE_Y).region = 3;
    ab_at(12, 6).climate = 0x20;                     // below the shore line
    ab_at(12, 6).region = (uint8_t)AB_REGION;
    ABCHECK(alien_base(AB_VEH, AB_X, AB_Y) == -1);   // a worm still cannot
    g_alien_world.vehs[AB_VEH].proto_id = (int16_t)BSC_SEALURK;
    ABCHECK(alien_base(AB_VEH, AB_X, AB_Y) == 0);
    ABCHECK(g_alien_world.base_find_dist == 12);
    // Neighbouring prototype ids are not Sealurks.
    g_alien_world.vehs[AB_VEH].proto_id = (int16_t)BSC_UNITY_FOIL;      // 13
    ABCHECK(alien_base(AB_VEH, AB_X, AB_Y) == -1);
    g_alien_world.vehs[AB_VEH].proto_id = (int16_t)BSC_SPORE_LAUNCHER;  // 15
    ABCHECK(alien_base(AB_VEH, AB_X, AB_Y) == -1);

    // ---- and only where the water really is the unit's own region -------
    g_alien_world.vehs[AB_VEH].proto_id = (int16_t)BSC_SEALURK;
    ab_at(12, 6).region = 5;
    ABCHECK(alien_base(AB_VEH, AB_X, AB_Y) == -1);
    ab_at(12, 6).region = (uint8_t)AB_REGION;
    ab_at(12, 6).climate = 0x80;                     // dry land again
    ABCHECK(alien_base(AB_VEH, AB_X, AB_Y) == -1);
}
GAMEPLAY_CASE(test_alien_base_reachability);

void test_alien_base_scoring() {
    AlienSeams seams;

    // ---- the divisor is the base's own mineral and energy intake --------
    // Driven to 2 rather than left near 32, because a small divisor is what
    // makes each of the three terms individually visible in the quotient.
    ab_reset();
    ab_place_base(0, AB_BASE_X, AB_BASE_Y, AB_FACTION);
    ab_base(0).mineral_intake_2 = -20;
    ab_base(0).energy_intake_2 = -10;
    ABCHECK(alien_base(AB_VEH, AB_X, AB_Y) == 0);
    ABCHECK(g_alien_world.base_find_dist == 192);    // 12 * 32 / 2
    ab_base(0).mineral_intake_2 = 0;
    ABCHECK(alien_base(AB_VEH, AB_X, AB_Y) == 0);
    ABCHECK(g_alien_world.base_find_dist == 17);     // 384 / 22
    ab_base(0).mineral_intake_2 = -20;
    ab_base(0).energy_intake_2 = 0;
    ABCHECK(alien_base(AB_VEH, AB_X, AB_Y) == 0);
    ABCHECK(g_alien_world.base_find_dist == 32);     // 384 / 12
    // A productive base pulls from further: bigger intake, smaller cost.
    ab_base(0).mineral_intake_2 = 16;
    ab_base(0).energy_intake_2 = 0;
    ABCHECK(alien_base(AB_VEH, AB_X, AB_Y) == 0);
    ABCHECK(g_alien_world.base_find_dist == 8);      // 384 / 48

    // ---- an Alien Artifact in the base halves the cost ------------------
    ab_reset();
    ab_place_base(0, AB_BASE_X, AB_BASE_Y, AB_FACTION);
    ab_place_unit(1, AB_BASE_X, AB_BASE_Y, BSC_ALIEN_ARTIFACT);
    ABCHECK(alien_base(AB_VEH, AB_X, AB_Y) == 0);
    ABCHECK(g_alien_world.base_find_dist == 6);
    // It is the PLAN that counts, not the prototype id.
    g_alien_world.protos[BSC_ALIEN_ARTIFACT].plan = (uint8_t)PLAN_COMBAT;
    ABCHECK(alien_base(AB_VEH, AB_X, AB_Y) == 0);
    ABCHECK(g_alien_world.base_find_dist == 12);
    // And the tile it has to be standing on is the BASE's, not the unit's.
    ab_reset();
    ab_place_base(0, AB_BASE_X, AB_BASE_Y, AB_FACTION);
    ab_place_unit(1, AB_X, AB_Y, BSC_ALIEN_ARTIFACT);
    ABCHECK(alien_base(AB_VEH, AB_X, AB_Y) == 0);
    ABCHECK(g_alien_world.base_find_dist == 12);

    // ---- the unit's own home base halves it -----------------------------
    // Two bases, so the halving has to be applied to the one whose id the
    // unit actually names rather than to whichever base is being looked at.
    ab_reset();
    ab_place_base(0, AB_BASE_X, AB_BASE_Y, AB_FACTION);
    ab_place_base(1, 2, 6, AB_FACTION);
    ab_base(1).mineral_intake_2 = -22;                // 5 * 32 / 10 = 16
    g_alien_world.vehs[AB_VEH].home_base_id = -1;
    ABCHECK(alien_base(AB_VEH, AB_X, AB_Y) == 0);
    ABCHECK(g_alien_world.base_find_dist == 12);
    g_alien_world.vehs[AB_VEH].home_base_id = 0;
    ABCHECK(alien_base(AB_VEH, AB_X, AB_Y) == 0);
    ABCHECK(g_alien_world.base_find_dist == 6);
    g_alien_world.vehs[AB_VEH].home_base_id = 1;
    ABCHECK(alien_base(AB_VEH, AB_X, AB_Y) == 1);
    ABCHECK(g_alien_world.base_find_dist == 8);       // 16 halved beats 12

    // ---- both halvings compose ------------------------------------------
    ab_reset();
    ab_place_base(0, AB_BASE_X, AB_BASE_Y, AB_FACTION);
    ab_place_unit(1, AB_BASE_X, AB_BASE_Y, BSC_ALIEN_ARTIFACT);
    g_alien_world.vehs[AB_VEH].home_base_id = 0;
    ABCHECK(alien_base(AB_VEH, AB_X, AB_Y) == 0);
    ABCHECK(g_alien_world.base_find_dist == 3);

    // ---- a green faction with a clean base is left alone ----------------
    // All three conditions have to hold before the cost doubles.
    ab_reset();
    ab_place_base(0, AB_BASE_X, AB_BASE_Y, AB_FACTION);
    g_alien_world.players_data[AB_FACTION].soc_effect_active.planet = 1;
    ABCHECK(alien_base(AB_VEH, AB_X, AB_Y) == 0);
    ABCHECK(g_alien_world.base_find_dist == 24);
    // A Planet rating of zero or below is not green enough.
    g_alien_world.players_data[AB_FACTION].soc_effect_active.planet = 0;
    ABCHECK(alien_base(AB_VEH, AB_X, AB_Y) == 0);
    ABCHECK(g_alien_world.base_find_dist == 12);
    g_alien_world.players_data[AB_FACTION].soc_effect_active.planet = -1;
    ABCHECK(alien_base(AB_VEH, AB_X, AB_Y) == 0);
    ABCHECK(g_alien_world.base_find_dist == 12);
    // Eco damage forfeits it.
    g_alien_world.players_data[AB_FACTION].soc_effect_active.planet = 1;
    ab_base(0).eco_damage = 1;
    ABCHECK(alien_base(AB_VEH, AB_X, AB_Y) == 0);
    ABCHECK(g_alien_world.base_find_dist == 12);
    ab_base(0).eco_damage = 0;
    // So does the unit's own Veh+0x26 matching the base's owner.
    g_alien_world.vehs[AB_VEH].order_auto_type = (uint8_t)AB_FACTION;
    ABCHECK(alien_base(AB_VEH, AB_X, AB_Y) == 0);
    ABCHECK(g_alien_world.base_find_dist == 12);
    g_alien_world.vehs[AB_VEH].order_auto_type = (uint8_t)(AB_FACTION + 1);
    ABCHECK(alien_base(AB_VEH, AB_X, AB_Y) == 0);
    ABCHECK(g_alien_world.base_find_dist == 24);
    // The Planet rating read is the BASE OWNER's, not the subject's or 0's.
    ab_reset();
    ab_place_base(0, AB_BASE_X, AB_BASE_Y, 4);
    g_alien_world.players_data[AB_FACTION].soc_effect_active.planet = 1;
    ABCHECK(alien_base(AB_VEH, AB_X, AB_Y) == 0);
    ABCHECK(g_alien_world.base_find_dist == 12);
    g_alien_world.players_data[4].soc_effect_active.planet = 1;
    ABCHECK(alien_base(AB_VEH, AB_X, AB_Y) == 0);
    ABCHECK(g_alien_world.base_find_dist == 24);
    // ... and it is soc_effect_active, not the pending or base copies.
    ab_reset();
    ab_place_base(0, AB_BASE_X, AB_BASE_Y, AB_FACTION);
    g_alien_world.players_data[AB_FACTION].soc_effect_pending.planet = 1;
    g_alien_world.players_data[AB_FACTION].soc_effect_temp.planet = 1;
    g_alien_world.players_data[AB_FACTION].soc_effect_base.planet = 1;
    ABCHECK(alien_base(AB_VEH, AB_X, AB_Y) == 0);
    ABCHECK(g_alien_world.base_find_dist == 12);
}
GAMEPLAY_CASE(test_alien_base_scoring);

void test_alien_base_selection() {
    AlienSeams seams;

    // ---- equal costs: the LAST base wins --------------------------------
    ab_reset();
    ab_place_base(0, AB_BASE_X, AB_BASE_Y, AB_FACTION);
    ab_place_base(1, AB_BASE_X, AB_BASE_Y, AB_FACTION);
    ABCHECK(alien_base(AB_VEH, AB_X, AB_Y) == 1);
    ABCHECK(g_alien_world.base_find_dist == 12);
    // A worse second base leaves the first standing, and the published
    // distance is the WINNER's rather than the last one looked at.
    ab_base(1).mineral_intake_2 = -8;                 // 384 / 24 = 16
    ABCHECK(alien_base(AB_VEH, AB_X, AB_Y) == 0);
    ABCHECK(g_alien_world.base_find_dist == 12);
    ab_base(1).mineral_intake_2 = 16;                 // 384 / 48 = 8
    ABCHECK(alien_base(AB_VEH, AB_X, AB_Y) == 1);
    ABCHECK(g_alien_world.base_find_dist == 8);

    // ---- 9999 is a real ceiling, not just a seed ------------------------
    // A round map with a very wide wrap is the only way to drive the cost
    // that high on a map this size: x_dist answers MapLongitudeBounds minus
    // the separation once the separation passes MapLongitude.
    ab_reset();
    g_alien_world.is_flat = 0;
    g_alien_world.lon_bounds = 13342;                 // x_dist -> 13332
    ab_place_base(0, 10, 0, AB_FACTION);
    ABCHECK(alien_base(AB_VEH, AB_X, AB_Y) == 0);     // exactly 9999, taken
    ABCHECK(g_alien_world.base_find_dist == 9999);
    g_alien_world.lon_bounds = 13344;                 // x_dist -> 13334
    ABCHECK(alien_base(AB_VEH, AB_X, AB_Y) == -1);    // 10000, refused
    ABCHECK(g_alien_world.base_find_dist == 9999);
    // Flat again and the wrap does not happen at all, so the base is close.
    g_alien_world.is_flat = 1;
    ABCHECK(alien_base(AB_VEH, AB_X, AB_Y) == 0);
    ABCHECK(g_alien_world.base_find_dist == 7);       // vector_dist(10, 0)

    // ---- the unit read is the one that was asked for --------------------
    // Unit 5 carries a Sealurk prototype, home base 0 and an order_auto_type
    // equal to the base owner; none of it may leak into unit 3's answer.
    ab_reset();
    ab_place_base(0, AB_BASE_X, AB_BASE_Y, AB_FACTION);
    ab_at(AB_BASE_X, AB_BASE_Y).region = 3;
    g_alien_world.players_data[AB_FACTION].soc_effect_active.planet = 1;
    ABCHECK(alien_base(AB_VEH, AB_X, AB_Y) == -1);
    ABCHECK(alien_base(AB_DECOY_VEH, AB_X, AB_Y) == -1);   // no water either
    ab_at(AB_BASE_X, AB_BASE_Y).region = (uint8_t)AB_REGION;
    ab_snapshot();
    ABCHECK(alien_base(AB_VEH, AB_X, AB_Y) == 0);
    ABCHECK(g_alien_world.base_find_dist == 24);           // doubled, not halved
    ABCHECK(ab_only_dist_changed());
    ABCHECK(alien_base(AB_DECOY_VEH, AB_X, AB_Y) == 0);
    ABCHECK(g_alien_world.base_find_dist == 6);            // halved, not doubled
}
GAMEPLAY_CASE(test_alien_base_selection);

#undef ABCHECK

/*
 * suggest_plan (0x0054ACC0): the base two factions are told to attack together.
 *
 * battle_plans (0x0054B1C0) is the only caller, and it asks twice with the
 * arguments swapped, so the fixtures do the same rather than assuming one side
 * is the player. Everything the body reaches is real - region_at, x_dist, both
 * vector_dist overloads and both base_find overloads run against this world -
 * so the numbers below are hand-derived from the disassembly and then asserted,
 * not read back out of a run.
 *
 * Three things about the world are easy to get wrong and silent when wrong:
 *
 *   - x_target and base_id_atk_target must start at -1, not at the zero a
 *     memset leaves. Zero is a real tile and base 0 is a real base, and the
 *     body branches on both being negative.
 *   - the map is 16x8 with MapLongitude 8, so a tile index is (x >> 1) + y * 8
 *     and MapTiles is aimed at the middle of a 192-entry array. Two adjacent
 *     even/odd x share one tile, and therefore one region.
 *   - the candidate has to be VISIBLE to the first faction. Bases[i].visibility
 *     is a faction bitfield, and a zeroed one hides every base from everyone.
 */
struct PlanWorld {
    Map tiles[192];
    Base bases[8];
    PlayerData players_data[MaxPlayerNum];
    Map *tiles_ptr;
    uint32_t longitude;
    int lon_bounds;
    int lat_bounds;
    BOOL is_flat;
    int base_count;
    int base_find_dist;
    int popup_faction;
};

PlanWorld g_plan_world;
PlanWorld g_plan_saved;

const int SP_LIVE = 64;                 // MapTiles aims here
const int SP_REGION = 2;                // the region almost everything is in
const int SP_REGION_FAR = 7;            // the pocket used for the penalty tests
const int SP_PLANNER = 1;               // the faction whose plan is drawn up
const int SP_PARTNER = 2;               // the other side of the conversation
const int SP_ENEMY = 3;                 // owns the bases worth attacking
const int SP_NEUTRAL = 0;               // at war with nobody, deliberately id 0
const int SP_DIST_SENTINEL = 0x5A5A5A5A;
const int SP_POPUP_SENTINEL = 0x3C3C3C3C;
const unsigned SP_SEEN_BY_BOTH = (1u << SP_PLANNER) | (1u << SP_PARTNER);

Map &sp_at(int x, int y) {
    return g_plan_world.tiles[SP_LIVE + (x >> 1) + y * 8];
}

void sp_reset() {
    std::memset(&g_plan_world, 0, sizeof(g_plan_world));
    g_plan_world.tiles_ptr = &g_plan_world.tiles[SP_LIVE];
    g_plan_world.longitude = 8;
    g_plan_world.lon_bounds = 16;
    g_plan_world.lat_bounds = 8;
    g_plan_world.is_flat = 1;           // no x wrapping unless a test asks
    g_plan_world.base_find_dist = SP_DIST_SENTINEL;
    g_plan_world.popup_faction = SP_POPUP_SENTINEL;
    for (int i = 0; i < 192; i++) {
        g_plan_world.tiles[i].region = (uint8_t)SP_REGION;
    }
    for (int f = 0; f < MaxPlayerNum; f++) {
        g_plan_world.players_data[f].x_target = -1;
        g_plan_world.players_data[f].y_target = -1;
        g_plan_world.players_data[f].base_id_atk_target = -1;
    }
}

void sp_place_base(int base_id, int x, int y, int faction_id, unsigned visibility) {
    Base &base = g_plan_world.bases[base_id];
    base.x = (int16_t)x;
    base.y = (int16_t)y;
    base.faction_id_current = (uint8_t)faction_id;
    base.visibility = (uint8_t)visibility;
    if (g_plan_world.base_count <= base_id) {
        g_plan_world.base_count = base_id + 1;
    }
}

void sp_declare_war(int faction_id, int against) {
    g_plan_world.players_data[faction_id].diplo_treaties[against] |= DTREATY_VENDETTA;
}

void sp_set_war_aim(int faction_id, int x, int y) {
    g_plan_world.players_data[faction_id].x_target = x;
    g_plan_world.players_data[faction_id].y_target = y;
}

/*
 * The default two-sided war. One enemy base both factions can see, one base
 * each for the planner and the partner, one region, no war aim:
 *
 *   base 0  enemy   (8, 4)   planner reach 5, partner reach 3, cost 8
 *   base 1  planner (2, 0)
 *   base 2  partner (12, 6)
 */
void sp_standard_world() {
    sp_reset();
    sp_place_base(0, 8, 4, SP_ENEMY, SP_SEEN_BY_BOTH);
    sp_place_base(1, 2, 0, SP_PLANNER, 1u << SP_PLANNER);
    sp_place_base(2, 12, 6, SP_PARTNER, 1u << SP_PARTNER);
    sp_declare_war(SP_PLANNER, SP_ENEMY);
    sp_declare_war(SP_PARTNER, SP_ENEMY);
}

void sp_snapshot() {
    std::memcpy(&g_plan_saved, &g_plan_world, sizeof(g_plan_world));
}

// BaseFindDist and the popup faction are the two published outputs. Everything
// else has to come back untouched.
bool sp_only_published_changed() {
    PlanWorld now;
    std::memcpy(&now, &g_plan_world, sizeof(now));
    now.base_find_dist = g_plan_saved.base_find_dist;
    now.popup_faction = g_plan_saved.popup_faction;
    return std::memcmp(&now, &g_plan_saved, sizeof(now)) == 0;
}

#define SPCHECK(cond)                                                         \
    do {                                                                      \
        const bool plan_ok = (cond);                                          \
        if (!plan_ok) {                                                       \
            std::fprintf(stderr, "suggest_plan: line %d: %s\n", __LINE__,     \
                         #cond);                                              \
        }                                                                     \
        expect(plan_ok);                                                      \
    } while (0)

class PlanSeams {
 public:
    PlanSeams()
        : tiles_(&MapTiles, &g_plan_world.tiles_ptr),
          longitude_(&MapLongitude, &g_plan_world.longitude),
          lon_(&MapLongitudeBounds, &g_plan_world.lon_bounds),
          lat_(&MapLatitudeBounds, &g_plan_world.lat_bounds),
          flat_(&MapIsFlat, &g_plan_world.is_flat),
          bases_(&Bases, g_plan_world.bases),
          base_count_(&BaseCurrentCount, &g_plan_world.base_count),
          base_dist_(&BaseFindDist, &g_plan_world.base_find_dist),
          players_data_(&PlayersData, g_plan_world.players_data),
          popup_(&PopupDialogFactionID, &g_plan_world.popup_faction) { }

 private:
    ScopedSeam<Map *> tiles_;
    ScopedSeam<uint32_t> longitude_;
    ScopedSeam<int> lon_;
    ScopedSeam<int> lat_;
    ScopedSeam<BOOL> flat_;
    ScopedSeam<Base> bases_;
    ScopedSeam<int> base_count_;
    ScopedSeam<int> base_dist_;
    ScopedSeam<PlayerData> players_data_;
    ScopedSeam<int> popup_;
};

void test_suggest_plan_eligibility() {
    PlanSeams seams;

    // ---- nothing to suggest, and the popup faction is published anyway ----
    // The write happens before the base count is even read, which is what
    // makes it safe for battle_plans to open a popup on a -1 answer.
    sp_standard_world();
    g_plan_world.base_count = 0;
    sp_snapshot();
    SPCHECK(suggest_plan(SP_PLANNER, SP_PARTNER) == -1);
    SPCHECK(g_plan_world.popup_faction == SP_PARTNER);
    SPCHECK(g_plan_world.base_find_dist == SP_DIST_SENTINEL);   // no search ran
    SPCHECK(sp_only_published_changed());

    // It is the SECOND argument that is published, both ways round.
    sp_standard_world();
    suggest_plan(SP_PARTNER, SP_PLANNER);
    SPCHECK(g_plan_world.popup_faction == SP_PLANNER);

    // ---- the ordinary answer ---------------------------------------------
    sp_standard_world();
    sp_snapshot();
    SPCHECK(suggest_plan(SP_PLANNER, SP_PARTNER) == 0);
    SPCHECK(g_plan_world.base_find_dist == 3);        // partner reach, published
    SPCHECK(g_plan_world.popup_faction == SP_PARTNER);
    SPCHECK(sp_only_published_changed());

    // ---- both factions have to be at vendetta already --------------------
    // Neither gate reaches a search, so BaseFindDist is still the sentinel:
    // that is how the fixture tells "skipped" from "searched and found none".
    sp_standard_world();
    g_plan_world.players_data[SP_PLANNER].diplo_treaties[SP_ENEMY] = 0;
    sp_snapshot();
    SPCHECK(suggest_plan(SP_PLANNER, SP_PARTNER) == -1);
    SPCHECK(g_plan_world.base_find_dist == SP_DIST_SENTINEL);
    SPCHECK(sp_only_published_changed());

    sp_standard_world();
    g_plan_world.players_data[SP_PARTNER].diplo_treaties[SP_ENEMY] = 0;
    SPCHECK(suggest_plan(SP_PLANNER, SP_PARTNER) == -1);
    SPCHECK(g_plan_world.base_find_dist == SP_DIST_SENTINEL);

    // Any other treaty bit is not a vendetta.
    sp_standard_world();
    g_plan_world.players_data[SP_PLANNER].diplo_treaties[SP_ENEMY] =
        DTREATY_TREATY | DTREATY_COMMLINK | DTREATY_WANT_REVENGE;
    SPCHECK(suggest_plan(SP_PLANNER, SP_PARTNER) == -1);

    // ---- only the first faction's sight matters --------------------------
    sp_standard_world();
    g_plan_world.bases[0].visibility = (uint8_t)(1u << SP_PARTNER);
    SPCHECK(suggest_plan(SP_PLANNER, SP_PARTNER) == -1);
    SPCHECK(g_plan_world.base_find_dist == SP_DIST_SENTINEL);

    sp_standard_world();
    g_plan_world.bases[0].visibility = (uint8_t)(1u << SP_PLANNER);
    SPCHECK(suggest_plan(SP_PLANNER, SP_PARTNER) == 0);

    // ---- neither party's own base is ever the suggestion -----------------
    sp_standard_world();
    g_plan_world.bases[0].faction_id_current = (uint8_t)SP_PLANNER;
    SPCHECK(suggest_plan(SP_PLANNER, SP_PARTNER) == -1);

    sp_standard_world();
    g_plan_world.bases[0].faction_id_current = (uint8_t)SP_PARTNER;
    SPCHECK(suggest_plan(SP_PLANNER, SP_PARTNER) == -1);
}
GAMEPLAY_CASE(test_suggest_plan_eligibility);

/*
 * Three candidates whose two reach terms are deliberately traded off against
 * each other, so that changing how the terms are weighted changes the answer:
 *
 *   base 0  enemy (8, 4)    planner 5, partner 3
 *   base 3  enemy (2, 2)    planner 1, partner 8
 *   base 4  enemy (12, 4)   planner 8, partner 1
 */
void sp_traded_world() {
    sp_standard_world();
    sp_place_base(3, 2, 2, SP_ENEMY, SP_SEEN_BY_BOTH);
    sp_place_base(4, 12, 4, SP_ENEMY, SP_SEEN_BY_BOTH);
}

void test_suggest_plan_reach_weighting() {
    PlanSeams seams;

    // ---- no war aim: the two reaches count once each ---------------------
    //   base 0  5 + 3 = 8      base 3  1 + 8 = 9      base 4  8 + 1 = 9
    sp_traded_world();
    sp_snapshot();
    SPCHECK(suggest_plan(SP_PLANNER, SP_PARTNER) == 0);
    SPCHECK(g_plan_world.base_find_dist == 1);        // base 4 was examined last
    SPCHECK(sp_only_published_changed());

    // ---- a war aim doubles the FIRST faction's reach, not the partner's --
    // The aim sits at (5, 3), which is two tiles from base 0 and two from
    // base 3, so the distance term is equal for those two and only the
    // doubling can decide between them:
    //   base 0  4 + 5*2 + 3 = 17     base 3  4 + 1*2 + 8 = 14
    //   base 4  10 + 8*2 + 1 = 27
    sp_traded_world();
    sp_set_war_aim(SP_PLANNER, 5, 3);
    SPCHECK(suggest_plan(SP_PLANNER, SP_PARTNER) == 3);
    SPCHECK(g_plan_world.base_find_dist == 1);

    // The partner's own aim is never read.
    sp_traded_world();
    sp_set_war_aim(SP_PARTNER, 5, 3);
    SPCHECK(suggest_plan(SP_PLANNER, SP_PARTNER) == 0);

    // ---- and the distance to the aim is worth twice the tile count -------
    // With the aim on base 0's own tile the distance term is the only thing
    // separating it from base 3, which is nearer its own faction:
    //   base 0  0 + 10 + 3 = 13     base 3  10 + 2 + 8 = 20
    //   base 4  6 + 16 + 1 = 23
    sp_traded_world();
    sp_set_war_aim(SP_PLANNER, 8, 4);
    SPCHECK(suggest_plan(SP_PLANNER, SP_PARTNER) == 0);
    SPCHECK(g_plan_world.base_find_dist == 1);
}
GAMEPLAY_CASE(test_suggest_plan_reach_weighting);

/*
 * A pocket of a second region along y == 4, x < 8, holding one candidate and
 * optionally a base for each faction. Everything else stays in region 2.
 *
 *   base 0  enemy   (14, 0)  region 2   planner 9, partner 5, cost 14
 *   base 1  planner (2, 0)   region 2
 *   base 2  partner (12, 6)  region 2
 *   base 3  enemy   (4, 4)   region 7   the candidate under test
 *   base 4  planner (0, 4)   region 7   present only when planner_has_reach
 *   base 5  partner (2, 4)   region 7   present only when partner_has_reach
 */
void sp_pocket_world(bool planner_has_reach, bool partner_has_reach) {
    sp_reset();
    for (int x = 0; x < 8; x += 2) {
        sp_at(x, 4).region = (uint8_t)SP_REGION_FAR;
    }
    sp_place_base(0, 14, 0, SP_ENEMY, SP_SEEN_BY_BOTH);
    sp_place_base(1, 2, 0, SP_PLANNER, 1u << SP_PLANNER);
    sp_place_base(2, 12, 6, SP_PARTNER, 1u << SP_PARTNER);
    sp_place_base(3, 4, 4, SP_ENEMY, SP_SEEN_BY_BOTH);
    if (planner_has_reach) {
        sp_place_base(4, 0, 4, SP_PLANNER, 1u << SP_PLANNER);
    }
    if (partner_has_reach) {
        sp_place_base(5, 2, 4, SP_PARTNER, 1u << SP_PARTNER);
    }
    sp_declare_war(SP_PLANNER, SP_ENEMY);
    sp_declare_war(SP_PARTNER, SP_ENEMY);
}

void test_suggest_plan_penalties() {
    PlanSeams seams;

    // ---- control: a base both factions can reach in its own region -------
    //   base 0  9 + 5 = 14        base 3  3 + 1 = 4
    sp_pocket_world(true, true);
    sp_snapshot();
    SPCHECK(suggest_plan(SP_PLANNER, SP_PARTNER) == 3);
    SPCHECK(g_plan_world.base_find_dist == 1);
    SPCHECK(sp_only_published_changed());

    // ---- the planner owns nothing in that region: 1024 -------------------
    // The search is redone without the region restriction, finds the base at
    // (2, 0) three tiles away, and the answer costs 1024 more than that.
    //   base 0  14        base 3  (3 + 1024) + 1 = 1028
    sp_pocket_world(false, true);
    SPCHECK(suggest_plan(SP_PLANNER, SP_PARTNER) == 0);
    SPCHECK(g_plan_world.base_find_dist == 1);

    // ---- the partner owns nothing there: 256, a quarter of the price -----
    //   base 0  14        base 3  3 + (6 + 256) = 265
    sp_pocket_world(true, false);
    SPCHECK(suggest_plan(SP_PLANNER, SP_PARTNER) == 0);
    SPCHECK(g_plan_world.base_find_dist == 262);

    // ---- a base outside the war aim's region: 512 ------------------------
    // The aim is at (12, 0), one tile from base 0 and seven from base 3, so
    // without the region penalty base 3 would still be the cheaper answer:
    //   base 0  2 + 0 + 9*2 + 5 = 25
    //   base 3  14 + 512 + 3*2 + 1 = 533   (21 without the penalty)
    sp_pocket_world(true, true);
    sp_set_war_aim(SP_PLANNER, 12, 0);
    SPCHECK(suggest_plan(SP_PLANNER, SP_PARTNER) == 0);
    SPCHECK(g_plan_world.base_find_dist == 1);
}
GAMEPLAY_CASE(test_suggest_plan_penalties);

void test_suggest_plan_unreachable_ceiling() {
    PlanSeams seams;

    // A base neither faction can reach at all is not suggested, because its
    // cost passes the 9999 the comparison starts at. Both searches fail, so
    // both keep base_find's own 9999 seed and add their penalty on top:
    //   (9999 + 1024) + (9999 + 256) = 21278
    sp_reset();
    sp_place_base(0, 8, 4, SP_ENEMY, SP_SEEN_BY_BOTH);
    sp_declare_war(SP_PLANNER, SP_ENEMY);
    sp_declare_war(SP_PARTNER, SP_ENEMY);
    sp_snapshot();
    SPCHECK(suggest_plan(SP_PLANNER, SP_PARTNER) == -1);
    SPCHECK(g_plan_world.base_find_dist == 10255);
    SPCHECK(sp_only_published_changed());

    // ...unless it is the base the planner has already declared for, which is
    // returned without ever being compared against anything.
    sp_reset();
    sp_place_base(0, 8, 4, SP_ENEMY, SP_SEEN_BY_BOTH);
    sp_declare_war(SP_PLANNER, SP_ENEMY);
    sp_declare_war(SP_PARTNER, SP_ENEMY);
    g_plan_world.players_data[SP_PLANNER].base_id_atk_target = 0;
    SPCHECK(suggest_plan(SP_PLANNER, SP_PARTNER) == 0);
}
GAMEPLAY_CASE(test_suggest_plan_unreachable_ceiling);

void test_suggest_plan_declared_target() {
    PlanSeams seams;

    // base 0 costs 8 and comes first; base 3 costs 9.
    sp_standard_world();
    sp_place_base(3, 2, 2, SP_ENEMY, SP_SEEN_BY_BOTH);
    sp_snapshot();
    SPCHECK(suggest_plan(SP_PLANNER, SP_PARTNER) == 0);
    SPCHECK(g_plan_world.base_find_dist == 8);        // base 3 was examined last
    SPCHECK(sp_only_published_changed());

    // A declared target wins even though it is dearer and later.
    sp_standard_world();
    sp_place_base(3, 2, 2, SP_ENEMY, SP_SEEN_BY_BOTH);
    g_plan_world.players_data[SP_PLANNER].base_id_atk_target = 3;
    SPCHECK(suggest_plan(SP_PLANNER, SP_PARTNER) == 3);

    // It is the FIRST faction's declaration. The partner's is never read.
    sp_standard_world();
    sp_place_base(3, 2, 2, SP_ENEMY, SP_SEEN_BY_BOTH);
    g_plan_world.players_data[SP_PARTNER].base_id_atk_target = 3;
    SPCHECK(suggest_plan(SP_PLANNER, SP_PARTNER) == 0);

    // The return is immediate: base 3 is never examined, so the published
    // distance is base 0's partner reach and not base 3's.
    sp_standard_world();
    sp_place_base(3, 2, 2, SP_ENEMY, SP_SEEN_BY_BOTH);
    g_plan_world.players_data[SP_PLANNER].base_id_atk_target = 0;
    SPCHECK(suggest_plan(SP_PLANNER, SP_PARTNER) == 0);
    SPCHECK(g_plan_world.base_find_dist == 3);

    // A declaration the gates reject is not honoured: base 1 belongs to the
    // planner, so the loop skips it before the comparison can fire.
    sp_standard_world();
    sp_place_base(3, 2, 2, SP_ENEMY, SP_SEEN_BY_BOTH);
    g_plan_world.players_data[SP_PLANNER].base_id_atk_target = 1;
    SPCHECK(suggest_plan(SP_PLANNER, SP_PARTNER) == 0);

    // ...and neither is one the planner cannot see.
    sp_standard_world();
    sp_place_base(3, 2, 2, SP_ENEMY, 1u << SP_PARTNER);
    g_plan_world.players_data[SP_PLANNER].base_id_atk_target = 3;
    SPCHECK(suggest_plan(SP_PLANNER, SP_PARTNER) == 0);
}
GAMEPLAY_CASE(test_suggest_plan_declared_target);

/*
 * Both searches are told "this faction only" and "no visibility filter" by the
 * four -1 arguments the original pushes. Two worlds make those four arguments
 * load-bearing, because in an ordinary world dropping either is invisible.
 *
 * sp_neutral_world puts a base belonging to a faction that is party to nothing
 * next to the second candidate: relaxing "this faction only" to "faction 0 as
 * well" then makes that candidate the cheaper one.
 *
 *   base 0  enemy   (8, 4)   planner 5, partner 3, cost 8
 *   base 1  planner (2, 0)
 *   base 2  partner (12, 6)
 *   base 3  enemy   (6, 6)   planner 5, partner 4, cost 9
 *   base 4  neutral (4, 6)   one tile from base 3, three from base 0
 */
void sp_neutral_world() {
    sp_standard_world();
    sp_place_base(3, 6, 6, SP_ENEMY, SP_SEEN_BY_BOTH);
    sp_place_base(4, 4, 6, SP_NEUTRAL, 0xFFu);
}

/*
 * sp_hidden_reach_world gives each faction two bases: a near one that no
 * faction 0 can see and a far one that it can. Relaxing "no visibility filter"
 * to "faction 0 must see it" drops the near base out of both searches, and the
 * near bases are placed so that costs the two candidates different amounts.
 *
 *   base 0  enemy   (8, 4)   planner 1 (near) / 7 (far), partner 1 / 6, cost 2
 *   base 1  planner (8, 2)   near, visible to faction 1 only
 *   base 2  partner (8, 6)   near, visible to faction 2 only
 *   base 3  enemy   (2, 2)   planner 4 (near) / 2 (far), partner 5 / 3, cost 5
 *   base 4  planner (0, 0)   far, visible to faction 0 as well
 *   base 5  partner (0, 6)   far, visible to faction 0 as well
 */
void sp_hidden_reach_world() {
    sp_reset();
    sp_place_base(0, 8, 4, SP_ENEMY, SP_SEEN_BY_BOTH);
    sp_place_base(1, 8, 2, SP_PLANNER, 1u << SP_PLANNER);
    sp_place_base(2, 8, 6, SP_PARTNER, 1u << SP_PARTNER);
    sp_place_base(3, 2, 2, SP_ENEMY, SP_SEEN_BY_BOTH);
    sp_place_base(4, 0, 0, SP_PLANNER, (1u << SP_PLANNER) | (1u << SP_NEUTRAL));
    sp_place_base(5, 0, 6, SP_PARTNER, (1u << SP_PARTNER) | (1u << SP_NEUTRAL));
    sp_declare_war(SP_PLANNER, SP_ENEMY);
    sp_declare_war(SP_PARTNER, SP_ENEMY);
}

void test_suggest_plan_search_scope() {
    PlanSeams seams;

    // A faction that is party to nothing owns bases too, and neither search
    // may count them.
    sp_neutral_world();
    sp_snapshot();
    SPCHECK(suggest_plan(SP_PLANNER, SP_PARTNER) == 0);
    SPCHECK(g_plan_world.base_find_dist == 4);        // base 3's partner reach
    SPCHECK(sp_only_published_changed());

    // The neutral base is not a candidate either: nobody is at vendetta with
    // its owner, so it never reaches a cost at all.
    SPCHECK(g_plan_world.players_data[SP_PLANNER].diplo_treaties[SP_NEUTRAL] == 0);

    // Neither search may drop a base merely because faction 0 cannot see it.
    sp_hidden_reach_world();
    sp_snapshot();
    SPCHECK(suggest_plan(SP_PLANNER, SP_PARTNER) == 0);
    SPCHECK(g_plan_world.base_find_dist == 3);        // base 3's partner reach
    SPCHECK(sp_only_published_changed());
}
GAMEPLAY_CASE(test_suggest_plan_search_scope);

void test_suggest_plan_tie_keeps_the_first() {
    PlanSeams seams;

    // Two candidates that cost exactly the same. The comparison is strict, so
    // the earlier base id stays.
    //   base 0  (8, 4)  5 + 3 = 8        base 3  (4, 0)  1 + 7 = 8
    sp_standard_world();
    sp_place_base(3, 4, 0, SP_ENEMY, SP_SEEN_BY_BOTH);
    sp_snapshot();
    SPCHECK(suggest_plan(SP_PLANNER, SP_PARTNER) == 0);
    SPCHECK(g_plan_world.base_find_dist == 7);
    SPCHECK(sp_only_published_changed());
}
GAMEPLAY_CASE(test_suggest_plan_tie_keeps_the_first);

/*
 * A war aim standing on column zero. `x_target >= 0` is the "is there an aim"
 * test, and column zero is a real tile, so a body that asked for a positive
 * x_target would treat this faction as having no aim at all.
 *
 * Both candidates sit outside the aim tile's region, so the 512 lands on both
 * and cancels; what separates them is the doubling and the distance term, and
 * the two are only one point apart:
 *
 *   base 0  enemy   (4, 0)  region 2   2*3 + 512 + 5*2 + 1 = 529
 *   base 1  planner (11, 0) region 2   base 0's reach, 5
 *   base 2  partner (4, 2)  region 2   base 0's reach, 1
 *   base 3  enemy   (0, 2)  region 7   2*1 + 512 + 1*2 + 12 = 528  <- wins
 *   base 4  planner (0, 4)  region 7   base 3's reach, 1
 *   base 5  partner (15, 6) region 7   base 3's reach, 12
 *
 * The regions are what keep the four reaches independent: each search is
 * restricted to the candidate's own region, so base 0 cannot see base 3's
 * neighbours and vice versa.
 */
void test_suggest_plan_zero_war_aim() {
    PlanSeams seams;

    sp_reset();
    sp_at(0, 0).region = 9;                     // the aim tile, its own region
    sp_at(0, 2).region = (uint8_t)SP_REGION_FAR;
    sp_at(0, 4).region = (uint8_t)SP_REGION_FAR;
    sp_at(15, 6).region = (uint8_t)SP_REGION_FAR;
    sp_place_base(0, 4, 0, SP_ENEMY, SP_SEEN_BY_BOTH);
    sp_place_base(1, 11, 0, SP_PLANNER, 1u << SP_PLANNER);
    sp_place_base(2, 4, 2, SP_PARTNER, 1u << SP_PARTNER);
    sp_place_base(3, 0, 2, SP_ENEMY, SP_SEEN_BY_BOTH);
    sp_place_base(4, 0, 4, SP_PLANNER, 1u << SP_PLANNER);
    sp_place_base(5, 15, 6, SP_PARTNER, 1u << SP_PARTNER);
    sp_declare_war(SP_PLANNER, SP_ENEMY);
    sp_declare_war(SP_PARTNER, SP_ENEMY);
    sp_set_war_aim(SP_PLANNER, 0, 0);
    sp_snapshot();
    SPCHECK(suggest_plan(SP_PLANNER, SP_PARTNER) == 3);
    SPCHECK(g_plan_world.base_find_dist == 12);
    SPCHECK(sp_only_published_changed());
}
GAMEPLAY_CASE(test_suggest_plan_zero_war_aim);

/*
 * base_find answers with a base id, and base id 0 is a real base. A search
 * that landed on it is a success, so the test for failure has to be strictly
 * negative. These two worlds put the successful answer AT id 0 for one
 * candidate and elsewhere for the other, so treating zero as a failure
 * penalises exactly one of them.
 */
void test_suggest_plan_base_zero_is_found() {
    PlanSeams seams;

    // The planner's nearest base is base 0 for the near candidate and base 4
    // for the far one.
    //   base 0  planner (12, 2)     base 1  partner (12, 6)
    //   base 2  enemy   (12, 4)  reach 1 (via base 0) + 1 = 2   <- wins
    //   base 3  enemy   (2, 2)   reach 1 (via base 4) + 8 = 9
    //   base 4  planner (2, 0)
    sp_reset();
    sp_place_base(0, 12, 2, SP_PLANNER, 1u << SP_PLANNER);
    sp_place_base(1, 12, 6, SP_PARTNER, 1u << SP_PARTNER);
    sp_place_base(2, 12, 4, SP_ENEMY, SP_SEEN_BY_BOTH);
    sp_place_base(3, 2, 2, SP_ENEMY, SP_SEEN_BY_BOTH);
    sp_place_base(4, 2, 0, SP_PLANNER, 1u << SP_PLANNER);
    sp_declare_war(SP_PLANNER, SP_ENEMY);
    sp_declare_war(SP_PARTNER, SP_ENEMY);
    sp_snapshot();
    SPCHECK(suggest_plan(SP_PLANNER, SP_PARTNER) == 2);
    SPCHECK(g_plan_world.base_find_dist == 8);
    SPCHECK(sp_only_published_changed());

    // The same shape with the partner owning base 0.
    sp_reset();
    sp_place_base(0, 12, 2, SP_PARTNER, 1u << SP_PARTNER);
    sp_place_base(1, 12, 6, SP_PLANNER, 1u << SP_PLANNER);
    sp_place_base(2, 12, 4, SP_ENEMY, SP_SEEN_BY_BOTH);
    sp_place_base(3, 2, 2, SP_ENEMY, SP_SEEN_BY_BOTH);
    sp_place_base(4, 2, 0, SP_PARTNER, 1u << SP_PARTNER);
    sp_declare_war(SP_PLANNER, SP_ENEMY);
    sp_declare_war(SP_PARTNER, SP_ENEMY);
    sp_snapshot();
    SPCHECK(suggest_plan(SP_PLANNER, SP_PARTNER) == 2);
    SPCHECK(g_plan_world.base_find_dist == 1);
    SPCHECK(sp_only_published_changed());
}
GAMEPLAY_CASE(test_suggest_plan_base_zero_is_found);

void test_suggest_plan_unrestricted_retry() {
    PlanSeams seams;

    // One candidate, in a region the planner owns nothing in. The retry
    // without the region restriction is what keeps its cost under the ceiling
    // at all: base_find's own 9999 seed plus 1024 would put it out of reach.
    //   base 3  (3 + 1024) + 1 = 1028, against a ceiling of 9999
    sp_pocket_world(false, true);
    g_plan_world.bases[0].visibility = 0;       // leave base 3 the only candidate
    sp_snapshot();
    SPCHECK(suggest_plan(SP_PLANNER, SP_PARTNER) == 3);
    SPCHECK(g_plan_world.base_find_dist == 1);
    SPCHECK(sp_only_published_changed());
}
GAMEPLAY_CASE(test_suggest_plan_unrestricted_retry);

/*
 * The ceiling is exactly 9999 and the comparison against it is strict.
 *
 * No 16x8 world can produce a cost anywhere near 9999 - the reachable costs
 * are either under about 1300 or over 10000 - so this one wraps instead. With
 * a round map one tile wide, x_dist answers MapLongitudeBounds minus the
 * separation, which puts the planner's only base an arbitrary distance away
 * while every coordinate stays small enough to index a real tile.
 *
 *   bounds 13332  ->  x_dist 13330  ->  reach 9997  ->  cost 9998, suggested
 *   bounds 13333  ->  x_dist 13331  ->  reach 9998  ->  cost 9999, refused
 */
void sp_wrapped_ceiling_world(int longitude_bounds) {
    sp_reset();
    g_plan_world.is_flat = 0;                   // round map: x_dist wraps
    g_plan_world.longitude = 1;
    g_plan_world.lon_bounds = longitude_bounds;
    sp_place_base(0, 0, 0, SP_ENEMY, SP_SEEN_BY_BOTH);
    sp_place_base(1, 2, 0, SP_PLANNER, 1u << SP_PLANNER);
    sp_place_base(2, 1, 0, SP_PARTNER, 1u << SP_PARTNER);
    sp_declare_war(SP_PLANNER, SP_ENEMY);
    sp_declare_war(SP_PARTNER, SP_ENEMY);
}

void test_suggest_plan_cost_ceiling_boundary() {
    PlanSeams seams;

    sp_wrapped_ceiling_world(13332);
    sp_snapshot();
    SPCHECK(suggest_plan(SP_PLANNER, SP_PARTNER) == 0);      // 9998 < 9999
    SPCHECK(g_plan_world.base_find_dist == 1);
    SPCHECK(sp_only_published_changed());

    sp_wrapped_ceiling_world(13333);
    sp_snapshot();
    SPCHECK(suggest_plan(SP_PLANNER, SP_PARTNER) == -1);     // 9999 is not
    SPCHECK(g_plan_world.base_find_dist == 1);
    SPCHECK(sp_only_published_changed());
}
GAMEPLAY_CASE(test_suggest_plan_cost_ceiling_boundary);

/*
 * Every constant in the cost, pinned to its exact value.
 *
 * The mutation harness only ever rewrites a literal to zero, so killing all of
 * its mutants shows each term is PRESENT, not that it has the right value:
 * measured, writing 511 for the region penalty passed the whole suite. This
 * pair fixes that for all of them at once by putting the total on the ceiling
 * with every term switched on - distance, region penalty, doubled planner
 * reach with its unreachable surcharge, and the partner's surcharge:
 *
 *   2*1 + 512 + (3588 + 1024)*2 + (dist + 256)
 *
 * The two worlds differ only in the partner's base, one tile further out, and
 * that one tile is what moves the total across the boundary:
 *
 *   partner at (0, 6)   reach 4   total 9998   suggested
 *   partner at (0, 7)   reach 5   total 9999   refused
 *
 * Adding one to any constant now refuses the first world and taking one away
 * suggests the second, so neither direction can pass unnoticed. The round
 * one-tile-wide map is again what makes a five-figure reach reachable while
 * every coordinate still indexes a real tile.
 */
void sp_pinned_constants_world(int partner_y) {
    sp_reset();
    g_plan_world.is_flat = 0;
    g_plan_world.longitude = 1;
    g_plan_world.lon_bounds = 4786;
    // With MapLongitude 1 a tile index is (x >> 1) + y, so sp_at() does not
    // apply here and the four tiles that matter are addressed directly.
    g_plan_world.tiles[SP_LIVE + 2].region = 9;             // (0, 2), the aim
    g_plan_world.tiles[SP_LIVE + 1].region = 5;             // (2, 0), planner
    g_plan_world.tiles[SP_LIVE + partner_y].region = 5;     // (0, partner_y)
    sp_place_base(0, 0, 0, SP_ENEMY, SP_SEEN_BY_BOTH);
    sp_place_base(1, 2, 0, SP_PLANNER, 1u << SP_PLANNER);
    sp_place_base(2, 0, partner_y, SP_PARTNER, 1u << SP_PARTNER);
    sp_declare_war(SP_PLANNER, SP_ENEMY);
    sp_declare_war(SP_PARTNER, SP_ENEMY);
    sp_set_war_aim(SP_PLANNER, 0, 2);
}

void test_suggest_plan_constants_are_exact() {
    PlanSeams seams;

    sp_pinned_constants_world(6);
    sp_snapshot();
    SPCHECK(suggest_plan(SP_PLANNER, SP_PARTNER) == 0);      // 9998
    SPCHECK(g_plan_world.base_find_dist == 260);             // 4 + 256
    SPCHECK(sp_only_published_changed());

    sp_pinned_constants_world(7);
    sp_snapshot();
    SPCHECK(suggest_plan(SP_PLANNER, SP_PARTNER) == -1);     // 9999
    SPCHECK(g_plan_world.base_find_dist == 261);             // 5 + 256
    SPCHECK(sp_only_published_changed());
}
GAMEPLAY_CASE(test_suggest_plan_constants_are_exact);

#undef SPCHECK

}  // namespace

/*
 * The recording stand-in for draw_tile, which src/mapwin.h declares and
 * repair_phase's second pass calls.
 *
 * src/mapwin.cpp is deliberately NOT linked into this target. Its body walks
 * the eight-slot MapWin table and dispatches through the still-original
 * MapWin::draw_radius, so linking it would pull the MapWin/Console object
 * graph in here for no verification gain - that slot walk is already covered
 * by recovery-leaf-tests. Recording the arguments instead is what makes the
 * second pass observable at all.
 *
 * Defined outside the anonymous namespace because game.cpp has to link
 * against it; the recorder it appends to is declared with the repair_phase
 * fixtures above.
 */
void __cdecl draw_tile(int x_coord, int y_coord, int draw_type) {
    RepairDrawCall call;
    call.x = x_coord;
    call.y = y_coord;
    call.draw_type = draw_type;
    g_rp_draw_calls.push_back(call);
}


int main() {
    for (const auto &entry : gameplay_cases()) {
        entry.second();
    }
    if (failure_count() != 0) {
        std::fprintf(stderr, "recovery-gameplay-tests: %d failure(s)\n",
                     failure_count());
    }
    // Printed on every run so a refactor of this file can be proved neutral by
    // comparing two runs, rather than by a number in a commit message that
    // nobody can reproduce.
    std::fprintf(stderr, "recovery-gameplay-tests: %lld assertions evaluated\n",
                 recovery_fixtures::expect_evaluations());
    return failure_count() == 0 ? 0 : 1;
}
