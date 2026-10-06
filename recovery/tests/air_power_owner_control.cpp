#include <assert.h>
#include <stdint.h>
#include <stdlib.h>

#define __cdecl

enum {
    PLAN_OFFENSIVE = 0, PLAN_AIR_SUPERIORITY = 4,
    PLAN_NAVAL_TRANSPORT = 7, PLAN_COLONIZATION = 8,
    ORDER_TERRAFORM_DOWN = 21, ORDER_MOVE_TO = 24,
    ORDERA_BOMBING_RUN = 10, ORDERA_ON_ALERT = 11,
    ORDERA_AUTOMATE_AIR_DEFENSE = 12, TRIAD_LAND = 0,
    TRIAD_SEA = 1, TRIAD_AIR = 2,
    VSTATE_UNK_200 = 0x200, BIT_BASE_IN_TILE = 1
};
struct Veh {
    int16_t x, y; uint32_t state; uint16_t flags; int16_t proto_id;
    uint8_t faction_id, pad0, dmg_incurred; int8_t order;
    uint8_t pad1; int8_t waypoint_pad[1];
    int16_t waypoint_x[4], waypoint_y[4];
    uint8_t pad2[20]; uint8_t order_auto_type, visibility;
    uint8_t moves_expended;
    int move_to_ai_type; uint8_t terraforming_turns; int facing;
};
struct VehPrototype { int plan, chassis_id, armor_id, cost, flags; };
struct RulesChassis { int triad, range, missile; };
struct Map { uint8_t climate, contour, val2, region, visibility, val3, unk1;
             int8_t territory; uint32_t bit; };
struct Base {
    int x, y, faction_id_current, population_size;
    uint32_t event, state; int visibility;
};
struct Goal { int type, priority, x, y, base_id; };
struct SocialEffect {
    enum EffectIndex { ECONOMY, EFFICIENCY, SUPPORT, TALENT, MORALE, POLICE,
                       GROWTH, PLANET, PROBE, INDUSTRY, RESEARCH };
    int values_[11];
};
struct PlayerData {
    uint32_t flags, diplo_treaties[8];
    uint8_t region_base_plan[128], region_total_bases[128];
    int base_id_atk_target, unk_67, unk_105;
    int ranking; uint8_t proto_id_active[512];
    Goal goals[75]; SocialEffect soc_effect_pending;
};
struct GameRules { int move_rate_roads; };
struct AbilityEntry { int preq_tech; };

static Veh Vehs[8];
static VehPrototype VehPrototypes[8];
static RulesChassis Chassis[8];
static Map Tiles[256];
static PlayerData PlayersData[8];
static Base Bases[8];
static AbilityEntry Ability[32];
static int VehCurrentCount, BaseFindDist, BaseCurrentCount;
static int MapLongitudeBounds = 16, MapLatitudeBounds = 16;
static uint32_t MapLongitude = 8;
static int MapIsFlat, base_find_result, territory_result;
static GameRules Rules = { 3 };
static int RadiusOffsetX[289], RadiusOffsetY[289];
static int VehMoveDeltaX[9] = { 1, 2, 1, 0, -1, -2, -1, 0, 0 };
static int VehMoveDeltaY[9] = { -1, 0, 1, 2, 1, 0, -1, -2, 0 };
static int stack_80, stack_4, stack_count, stack_air_defenders;
static int stack_bombing, stub_has_tech, offense_result;
static int mutate_plan_on_offense;
static int stub_speed, action_home_sets_order;
static int odds_result, battle_calls, odds_calls, stack_calls;
static int home_calls, vehicle_at_calls, stack_fix_calls, offense_calls;
static uint32_t LockEnableMask;
static int IsMultiplayerNet;
static int base_at_calls;
static int veh_lift_calls, veh_drop_calls, action_tectonic_calls, action_fungal_calls;

static Map *map_loc(int x, int y) { return &Tiles[y * MapLongitudeBounds + x]; }
static int veh_at(int x, int y) {
    ++vehicle_at_calls;
    for (int i = 0; i < VehCurrentCount; ++i)
        if (Vehs[i].x == x && Vehs[i].y == y) return i;
    return -1;
}
static int stack_fix(int id) { ++stack_fix_calls; return id; }
static int speed(int, int) { return stub_speed; }
static int stack_check(int, int type, int item, int, int) {
    ++stack_calls;
    if (type == 6) return stack_80;
    if (type == 2) return item == 9 ? stack_bombing : stack_4;
    if (type == 1) return stack_count;
    if (type == 0x13) return stack_air_defenders;
    return 0;
}
static int battle_fight(int, int, int, int, int, int, int) {
    ++battle_calls; return 91;
}
static int compute_odds(int, int, int, int, int) {
    ++odds_calls; return odds_result;
}
static int base_find(int, int, int) {
    BaseFindDist = 1; return base_find_result;
}
static int base_find(int, int) {
    BaseFindDist = 1; return base_find_result;
}
static int whose_territory(int, int, int, int *, int) { return territory_result; }
static int action_home(int vehicle_id, int) {
    ++home_calls;
    if (action_home_sets_order) Vehs[vehicle_id].order = ORDER_MOVE_TO;
    return 0;
}
static int offense_proto(int, int, int) {
    ++offense_calls;
    if (mutate_plan_on_offense)
        VehPrototypes[0].plan = PLAN_COLONIZATION;
    return offense_result;
}
static int abil_index(int ability_id) { return ability_id; }
static int has_tech(int, int) { return stub_has_tech; }
static int has_abil(int, int) { return 0; }
static int base_at(int, int) { ++base_at_calls; return -1; }
static int bit_count(int value) {
    int count = 0;
    while (value != 0) { count += value & 1; value >>= 1; }
    return count;
}
static int xrange(int x) {
    if ((MapIsFlat & 1) == 0) {
        if (x < 0) x += MapLongitudeBounds;
        else if (x >= MapLongitudeBounds) x -= MapLongitudeBounds;
    }
    return x;
}
static int x_dist(int x1, int x2) {
    int dx = abs(x1 - x2);
    if ((MapIsFlat & 1) == 0 && static_cast<int>(MapLongitude) < dx)
        dx = MapLongitudeBounds - dx;
    return dx;
}
static int cursor_dist(int x1, int y1, int x2, int y2) {
    return (x_dist(x1, x2) + abs(y1 - y2)) >> 1;
}
static int veh_lift(int vehicle_id) { ++veh_lift_calls; return vehicle_id; }
static int veh_drop(int, int, int) { ++veh_drop_calls; return 0; }
static void action_tectonic(int, int, int) { ++action_tectonic_calls; }
static void action_fungal(int, int, int) { ++action_fungal_calls; }


// ACTUAL_SOURCE_OWNER

static void reset() {
    int waypoint;
    VehCurrentCount = 2; BaseCurrentCount = 1; BaseFindDist = 9999;
    base_find_result = 0; territory_result = 1;
    MapIsFlat = 1; stack_80 = stack_4 = stack_count = stack_air_defenders = 0;
    odds_result = 30; battle_calls = odds_calls = stack_calls = 0;
    home_calls = vehicle_at_calls = stack_fix_calls = offense_calls = 0;
    stub_speed = 6; action_home_sets_order = 0;
    stack_bombing = stub_has_tech = offense_result = 0;
    mutate_plan_on_offense = 0;
    LockEnableMask = 0; IsMultiplayerNet = 0;
    base_at_calls = 0;
    veh_lift_calls = veh_drop_calls = 0;
    action_tectonic_calls = action_fungal_calls = 0;
    for (int i = 0; i < 8; ++i) {
        Vehs[i].x = 5; Vehs[i].y = 5; Vehs[i].state = 0;
        Vehs[i].flags = 0; Vehs[i].proto_id = i; Vehs[i].faction_id = i;
        Vehs[i].order = 0; Vehs[i].order_auto_type = 0;
        Vehs[i].visibility = 0; Vehs[i].moves_expended = 0;
        Vehs[i].dmg_incurred = 0;
        Vehs[i].move_to_ai_type = 0; Vehs[i].terraforming_turns = 9;
        Vehs[i].facing = 0;
        for (waypoint = 0; waypoint < 4; ++waypoint) {
            Vehs[i].waypoint_x[waypoint] = 0;
            Vehs[i].waypoint_y[waypoint] = 0;
        }
        VehPrototypes[i].plan = PLAN_OFFENSIVE;
        VehPrototypes[i].chassis_id = i;
        VehPrototypes[i].cost = VehPrototypes[i].flags = 0;
        Chassis[i].triad = 0;
        Chassis[i].range = 0;
        Chassis[i].missile = 0;
        PlayersData[i].flags = 0; PlayersData[i].unk_67 = 0;
        PlayersData[i].unk_105 = 0;
        for (int f = 0; f < 8; ++f)
            PlayersData[i].diplo_treaties[f] = 0;
        for (int r = 0; r < 128; ++r) {
            PlayersData[i].region_base_plan[r] = 0;
            PlayersData[i].region_total_bases[r] = 1;
        }
    }
    for (int i = 0; i < 256; ++i) {
        Tiles[i].climate = 0; Tiles[i].val2 = 1; Tiles[i].region = 1;
        Tiles[i].visibility = 0; Tiles[i].bit = 0;
    }
    for (int i = 0; i < 289; ++i)
        RadiusOffsetX[i] = RadiusOffsetY[i] = 0;
    for (int i = 1; i <= 8; ++i) {
        RadiusOffsetX[i] = (i <= 2 || i >= 8) ? 1 : -1;
        RadiusOffsetY[i] = (i % 2) ? 0 : 1;
    }
    Ability[0x10].preq_tech = 0;
    Bases[0].x = Bases[0].y = 5; Bases[0].faction_id_current = 0;
}

int main() {
    int target_x = 5, target_y = 5, best_id, best_cost, best_distance;
    reset();
    Vehs[0].faction_id = 0; Vehs[1].faction_id = 1;
    Vehs[1].visibility = 1;
    Chassis[1].triad = 0;
    assert(air_power_ordinary_candidate_scan(0, &target_x, &target_y,
        6, best_id, best_cost, best_distance));
    assert(best_id == 1 && best_cost == 0 && best_distance == 0);
    assert(battle_calls == 1 && odds_calls == 1);

    reset();
    Vehs[0].faction_id = 0; Vehs[1].faction_id = 1;
    assert(!air_power_ordinary_candidate_scan(0, &target_x, &target_y,
        6, best_id, best_cost, best_distance));
    assert(battle_calls == 0 && best_id == -1);

    reset();
    Vehs[0].faction_id = 0; Vehs[1].faction_id = 1;
    Vehs[1].visibility = 1;
    Vehs[1].order = ORDER_TERRAFORM_DOWN;
    Vehs[1].x = 6;
    Bases[0].x = 6; base_find_result = 0;
    assert(air_power_ordinary_candidate_scan(0, &target_x, &target_y,
        6, best_id, best_cost, best_distance));
    assert(best_id == 1 && best_distance == 0);

    reset();
    VehCurrentCount = 3;
    Vehs[0].faction_id = 0;
    Vehs[1].faction_id = 1; Vehs[1].visibility = 1;
    Vehs[1].x = 6;
    Vehs[2].faction_id = 2; Vehs[2].visibility = 1;
    Vehs[2].x = 4;
    assert(air_power_ordinary_candidate_scan(0, &target_x, &target_y,
        6, best_id, best_cost, best_distance));
    assert(best_id == 2 && best_cost == 0 && best_distance == 0);

    reset();
    VehPrototypes[0].plan = PLAN_AIR_SUPERIORITY;
    Vehs[1].faction_id = 0; Vehs[1].x = 6;
    Vehs[1].moves_expended = 6;
    VehPrototypes[1].plan = PLAN_NAVAL_TRANSPORT;
    Chassis[1].triad = TRIAD_AIR;
    stack_count = 1;
    assert(air_power_ordinary_candidate_scan(0, &target_x, &target_y,
        6, best_id, best_cost, best_distance));
    assert(best_id == 1 && battle_calls == 0 && odds_calls == 0);

    reset();
    Vehs[0].faction_id = 0; Vehs[1].faction_id = 1;
    Vehs[1].visibility = 1;
    int live_cvar = -1, result = 999;
    assert(air_power_ordinary_post_loop(0, &target_x, &target_y,
        6, 6, false, live_cvar, result));
    assert(result == -1 && live_cvar == 0 && Vehs[0].order == 24);

    reset();
    Vehs[1].faction_id = 1; Vehs[1].visibility = 1;
    Vehs[1].x = 6;
    assert(air_power_ordinary_post_loop(0, &target_x, &target_y,
        6, 6, true, live_cvar, result));
    assert(result == -2 && Vehs[0].order == ORDER_MOVE_TO
           && Vehs[0].waypoint_x[0] == 6
           && Vehs[0].waypoint_y[0] == 5
           && target_x == 5 && target_y == 5);

    reset();
    VehPrototypes[0].plan = PLAN_AIR_SUPERIORITY;
    assert(air_power_ordinary_post_loop(0, &target_x, &target_y,
        6, 6, false, live_cvar, result));
    assert(result == -3 && live_cvar == 0 && home_calls == 0);

    reset();
    VehPrototypes[0].plan = PLAN_AIR_SUPERIORITY;
    assert(air_power_ordinary_post_loop(0, &target_x, &target_y,
        6, 6, true, live_cvar, result));
    assert(result == -2 && live_cvar == 0 && home_calls == 1);

    reset();
    Vehs[0].state = VSTATE_UNK_200;
    Vehs[0].order_auto_type = ORDERA_AUTOMATE_AIR_DEFENSE;
    Vehs[1].faction_id = 1; Vehs[1].visibility = 1; Vehs[1].x = 10;
    BaseCurrentCount = 1;
    Bases[0].x = 11; Bases[0].y = 5; Bases[0].faction_id_current = 0;
    assert(air_power_ordinary_post_loop(0, &target_x, &target_y,
        4, 0, false, live_cvar, result));
    assert(result == -2 && Vehs[0].order == ORDER_MOVE_TO
           && Vehs[0].move_to_ai_type == 0x61
           && Vehs[0].waypoint_x[0] == 11);

    reset();
    VehPrototypes[0].plan = PLAN_AIR_SUPERIORITY;
    Vehs[0].state = VSTATE_UNK_200;
    Vehs[0].order_auto_type = ORDERA_AUTOMATE_AIR_DEFENSE;
    Chassis[1].triad = TRIAD_AIR;
    Vehs[1].faction_id = 1; Vehs[1].visibility = 1;
    Vehs[1].x = 12;
    BaseCurrentCount = 2;
    Bases[0].x = 10; Bases[0].y = 5; Bases[0].faction_id_current = 0;
    Bases[1].x = 14; Bases[1].y = 5; Bases[1].faction_id_current = 0;
    assert(air_power_ordinary_post_loop(0, &target_x, &target_y,
        4, 2, true, live_cvar, result));
    assert(result == -2 && Vehs[0].waypoint_x[0] == 10);

    reset();
    VehPrototypes[0].plan = PLAN_AIR_SUPERIORITY;
    Vehs[0].state = VSTATE_UNK_200;
    Vehs[0].order_auto_type = ORDERA_AUTOMATE_AIR_DEFENSE;
    Chassis[1].triad = TRIAD_AIR;
    Vehs[1].faction_id = 1; Vehs[1].visibility = 1;
    Vehs[1].x = 11;
    BaseCurrentCount = 0;
    assert(air_power_ordinary_post_loop(0, &target_x, &target_y,
        4, 6, true, live_cvar, result));
    assert(result == -2 && Vehs[0].waypoint_x[0] == 11);

    reset();
    VehPrototypes[0].plan = PLAN_COLONIZATION;
    Vehs[0].state = VSTATE_UNK_200;
    Vehs[0].order_auto_type = ORDERA_AUTOMATE_AIR_DEFENSE;
    Vehs[1].faction_id = 1; Vehs[1].visibility = 1;
    assert(air_power_ordinary_post_loop(0, &target_x, &target_y,
        6, 6, false, live_cvar, result));
    assert(result == -1 && Vehs[0].terraforming_turns == 0);

    reset();
    VehPrototypes[0].plan = 6;
    Vehs[0].state = VSTATE_UNK_200;
    Vehs[0].order_auto_type = ORDERA_ON_ALERT;
    assert(!air_power_ordinary_post_loop(0, &target_x, &target_y,
        6, 6, true, live_cvar, result));
    assert(live_cvar == 6 && home_calls == 0);

    reset();
    VehPrototypes[0].plan = 1;
    assert(air_power_nonmissile_continuation(
        0, &target_x, &target_y, 6, 6, 1, false, false) == -3);

    reset();
    VehPrototypes[0].plan = 6;
    Vehs[0].state = VSTATE_UNK_200;
    Vehs[0].order_auto_type = ORDERA_ON_ALERT;
    Vehs[0].waypoint_x[1] = 5; Vehs[0].waypoint_y[1] = 5;
    assert(air_power_nonmissile_continuation(
        0, &target_x, &target_y, 6, 6, 1, true, false) == -4);
    assert(offense_calls == 0);

    reset();
    Chassis[0].triad = TRIAD_AIR;
    Vehs[1].faction_id = 1; Vehs[1].visibility = 1; Vehs[1].x = 6;
    PlayersData[1].diplo_treaties[0] = 0x10;
    offense_result = 5;
    mutate_plan_on_offense = 1;
    assert(air_power_on_alert_intercept(
        0, &target_x, &target_y, 6, 0, false) == -2);
    assert(VehPrototypes[0].plan == PLAN_COLONIZATION
           && Vehs[0].terraforming_turns == 0);

    reset();
    VehPrototypes[0].plan = PLAN_OFFENSIVE;
    Vehs[0].state = VSTATE_UNK_200;
    Vehs[0].order_auto_type = ORDERA_BOMBING_RUN;
    Vehs[1].faction_id = 1; Vehs[1].visibility = 1;
    BaseCurrentCount = 0;
    assert(air_power(0, 0, 0, &target_x, &target_y) == -3);
    assert(battle_calls == 0 && odds_calls == 0);

    reset();
    Vehs[0].state = 0x4000;
    assert(air_power_relocation(0, &target_x, &target_y, 4, result));
    assert(result == -2 && Vehs[0].order == ORDER_MOVE_TO
           && Vehs[0].move_to_ai_type == 0x61);

    reset();
    Vehs[0].state = 0x4000;
    PlayersData[0].flags = 0x200;
    BaseCurrentCount = 0;
    assert(air_power_relocation(0, &target_x, &target_y, 4, result));
    assert(result == -3 && Vehs[0].order != ORDER_MOVE_TO);

    reset();
    VehPrototypes[0].plan = PLAN_COLONIZATION;
    Tiles[target_y * MapLongitudeBounds + target_x].climate = 0x60;
    Tiles[target_y * MapLongitudeBounds + target_x].region = 1;
    PlayersData[0].region_base_plan[33] = 8;
    PlayersData[0].flags = 0x200;
    BaseCurrentCount = 0;
    assert(!air_power_relocation(0, &target_x, &target_y, 4, result));
    assert(Vehs[0].order != ORDER_MOVE_TO);

    reset();
    Tiles[target_y * MapLongitudeBounds + target_x].climate = 0x60;
    Tiles[target_y * MapLongitudeBounds + target_x].region = 1;
    PlayersData[0].region_base_plan[33] = 8;
    assert(air_power_relocation(0, &target_x, &target_y, 4, result));
    assert(result == -2 && Vehs[0].order == ORDER_MOVE_TO);

    reset();
    VehPrototypes[0].plan = PLAN_COLONIZATION;
    VehPrototypes[0].armor_id = 3;
    Vehs[0].state = 0x4000;
    assert(air_power_relocation(0, &target_x, &target_y, 4, result));
    assert(result == -2 && Vehs[0].terraforming_turns == 0);

    reset();
    int live_iVar10 = -1, live_iVar11 = -1, range_result = 999;
    int requested_region = -1;
    assert(!air_power_range_entry(0, &target_x, &target_y, false,
        requested_region, live_iVar10, live_iVar11, range_result));
    assert(live_iVar10 == 12 && live_iVar11 == 12);

    reset();
    stub_speed = 12; Chassis[0].range = 1;
    Vehs[0].moves_expended = 3;
    assert(!air_power_range_entry(0, &target_x, &target_y, false,
        requested_region, live_iVar10, live_iVar11, range_result));
    assert(live_iVar10 == 3 && live_iVar11 == 7);

    reset();
    Chassis[0].range = 1; Vehs[0].dmg_incurred = 1;
    assert(!air_power_range_entry(0, &target_x, &target_y, false,
        requested_region, live_iVar10, live_iVar11, range_result));
    assert(live_iVar10 == 2 && live_iVar11 == 2);

    reset();
    stub_speed = 12; Chassis[0].range = 3;
    Vehs[0].moves_expended = 5; Vehs[0].dmg_incurred = 1;
    assert(!air_power_range_entry(0, &target_x, &target_y, false,
        requested_region, live_iVar10, live_iVar11, range_result));
    assert(live_iVar10 == 6 && live_iVar11 == 6);

    reset();
    Vehs[0].order = ORDER_MOVE_TO;
    Vehs[0].waypoint_x[0] = 4; Vehs[0].waypoint_y[0] = 5;
    assert(air_power_range_entry(0, &target_x, &target_y, false,
        requested_region, live_iVar10, live_iVar11, range_result));
    assert(range_result == -2);

    reset();
    Vehs[0].order = ORDER_MOVE_TO;
    Vehs[0].waypoint_x[0] = 4; Vehs[0].waypoint_y[0] = 5;
    assert(air_power_range_entry(0, &target_x, &target_y, true,
        requested_region, live_iVar10, live_iVar11, range_result));
    assert(range_result == -1);

    reset();
    Chassis[0].range = 1; Vehs[0].terraforming_turns = 1;
    action_home_sets_order = 1;
    assert(air_power_range_entry(0, &target_x, &target_y, false,
        requested_region, live_iVar10, live_iVar11, range_result));
    assert(range_result == -2 && home_calls == 1);

    reset();
    requested_region = -1;
    Vehs[0].order = ORDER_MOVE_TO;
    Vehs[0].waypoint_x[0] = 4; Vehs[0].waypoint_y[0] = 5;
    assert(air_power_entry_relocation_dispatch(
        0, &target_x, &target_y, false, live_iVar10, live_iVar11,
        requested_region, range_result));
    assert(range_result == -2 && requested_region == -1);

    reset();
    Vehs[0].state = 0x4200;
    Vehs[0].order_auto_type = ORDERA_BOMBING_RUN;
    BaseCurrentCount = 0;
    assert(air_power_entry_relocation_dispatch(
        0, &target_x, &target_y, false, live_iVar10, live_iVar11,
        requested_region, range_result));
    assert(range_result == -3 && requested_region == 1);

    reset();
    Vehs[0].state = 0x4000;
    assert(air_power_entry_relocation_dispatch(
        0, &target_x, &target_y, false, live_iVar10, live_iVar11,
        requested_region, range_result));
    assert(range_result == -2 && Vehs[0].move_to_ai_type == 0x61);

    reset();
    VehPrototypes[0].plan = 6;
    assert(!air_power_entry_relocation_dispatch(
        0, &target_x, &target_y, false, live_iVar10, live_iVar11,
        requested_region, range_result));
    assert(live_iVar10 == 12 && live_iVar11 == 12
           && requested_region == 1);

    /* Exercise the full top-level live-local path: early returns stay ahead
       of missile dispatch; successful entry passes road-scaled ranges once. */
    reset();
    Vehs[0].order = ORDER_MOVE_TO;
    Vehs[0].waypoint_x[0] = 4; Vehs[0].waypoint_y[0] = 5;
    assert(air_power(
        0, 1, 0, &target_x, &target_y) == -2);
    assert(base_at_calls == 0);

    reset();
    Chassis[0].missile = 1;
    LockEnableMask = 1;
    IsMultiplayerNet = 1;
    assert(air_power(
        0, 1, 0, &target_x, &target_y) == -3);
    assert(base_at_calls == 0);

    reset();
    Chassis[0].missile = 1;
    assert(air_power(
        0, 1, 0, &target_x, &target_y) == -3);
    assert(base_at_calls == 2);

    reset();
    Chassis[0].missile = 1;
    IsMultiplayerNet = 1;
    assert(air_power(
        0, 1, 0, &target_x, &target_y) == -3);
    assert(base_at_calls == 2);

    reset();
    VehPrototypes[0].plan = PLAN_AIR_SUPERIORITY;
    assert(air_power(
        0, 0, 0, &target_x, &target_y) == -3);
    assert(home_calls == 0);

    reset();
    VehPrototypes[0].plan = PLAN_AIR_SUPERIORITY;
    assert(air_power(
        0, 1, 0, &target_x, &target_y) == -2);
    assert(home_calls == 1);

    reset();
    VehPrototypes[0].plan = 6;
    Vehs[0].state = VSTATE_UNK_200;
    Vehs[0].order_auto_type = ORDERA_ON_ALERT;
    assert(air_power(
        0, 1, 0, &target_x, &target_y) == -2);
    assert(Vehs[0].order == ORDER_MOVE_TO);
    assert(Vehs[0].move_to_ai_type == 0x6C);
    assert(Vehs[0].waypoint_x[0] == Vehs[0].waypoint_x[1]);
    assert(Vehs[0].waypoint_y[0] == Vehs[0].waypoint_y[1]);

    reset();
    Vehs[0].state = VSTATE_UNK_200;
    Vehs[0].order_auto_type = ORDERA_BOMBING_RUN;
    BaseCurrentCount = 0;
    assert(air_power(
        0, 1, 0, &target_x, &target_y) == -3);

    reset();
    assert(air_power(
        0, 0, 0, &target_x, &target_y) == -3);
    return 0;
}
