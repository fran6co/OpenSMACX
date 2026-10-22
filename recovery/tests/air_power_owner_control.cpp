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
struct LockMasks { uint32_t enable_mask_; };
struct GameDataFields {
    int base_current_count_, vehicle_current_count_;
    LockMasks lock_masks_;
};
struct GameDataState { GameDataFields fields_; };

static Veh Vehs[8];
static VehPrototype VehPrototypes[512];
static RulesChassis Chassis[8];
static Map Tiles[256];
static PlayerData PlayersData[8];
static Base Bases[8];
static AbilityEntry Ability[32];
static GameDataState GameDataStateStorage;
static int VehCurrentCount, BaseFindDist, BaseCurrentCount;
static int MapLongitudeBounds = 16, MapLatitudeBounds = 16;
static int MapLongitude = 8;
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
static int speed_calls, rand_calls, rand_value;
static int dropped_vehicle, dropped_x, dropped_y, callback_order[1024], callback_count;
static int action_target_x, action_target_y;

/* Trace IDs: speed 1, battle 2, odds 3, base_find 4, home 5, lift 6, drop 7,
   tectonic 8, fungal 9, rand 10, veh_at 11, stack_fix 12, base_at 13,
   offense_proto 14. */
static void note_callback(int value) {
    assert(callback_count < 1024);
    callback_order[callback_count++] = value;
}
static void assert_event_subsequence(const int *expected, int count) {
    int expected_index = 0;
    int event_index;
    for (event_index = 0; event_index < callback_count && expected_index < count;
         ++event_index) {
        if (callback_order[event_index] == expected[expected_index])
            ++expected_index;
    }
    assert(expected_index == count);
}
static Map *map_loc(int x, int y) { return &Tiles[y * MapLongitudeBounds + x]; }
static int veh_at(int x, int y) {
    ++vehicle_at_calls; note_callback(11);
    for (int i = 0; i < VehCurrentCount; ++i)
        if (Vehs[i].x == x && Vehs[i].y == y) return i;
    return -1;
}
static int stack_fix(int id) { ++stack_fix_calls; note_callback(12); return id; }
static int speed(int, int) { ++speed_calls; note_callback(1); return stub_speed; }
static int stack_check(int, int type, int item, int, int) {
    ++stack_calls;
    if (type == 6) return stack_80;
    if (type == 2) return item == 9 ? stack_bombing : stack_4;
    if (type == 1) return stack_count;
    if (type == 0x13) return stack_air_defenders;
    return 0;
}
static int battle_fight(int, int, int, int, int, int, int) {
    ++battle_calls; note_callback(2); return 91;
}
static int compute_odds(int, int, int, int, int) {
    ++odds_calls; note_callback(3); return odds_result;
}
static int base_find(int, int, int) {
    BaseFindDist = 1; note_callback(4); return base_find_result;
}
static int base_find(int, int) {
    BaseFindDist = 1; note_callback(4); return base_find_result;
}
static int whose_territory(int, int, int, int *, int) { return territory_result; }
static int action_home(int vehicle_id, int) {
    ++home_calls; note_callback(5);
    if (action_home_sets_order) Vehs[vehicle_id].order = ORDER_MOVE_TO;
    return 0;
}
static int offense_proto(int, int, int) {
    ++offense_calls;
    note_callback(14);
    if (mutate_plan_on_offense)
        VehPrototypes[0].plan = PLAN_COLONIZATION;
    return offense_result;
}
static int abil_index(int ability_id) { return ability_id; }
static int has_tech(int, int) { return stub_has_tech; }
static int has_abil(int, int) { return 0; }
static int base_at(int, int) { ++base_at_calls; note_callback(13); return -1; }
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
static int veh_lift(int vehicle_id) { ++veh_lift_calls; note_callback(6); return vehicle_id + 40; }
static int veh_drop(int id, int x, int y) {
    ++veh_drop_calls; note_callback(7);
    dropped_vehicle = id; dropped_x = x; dropped_y = y; return 0;
}
static void action_tectonic(int, int x, int y) {
    ++action_tectonic_calls; note_callback(8); action_target_x=x; action_target_y=y;
}
static void action_fungal(int, int x, int y) {
    ++action_fungal_calls; note_callback(9); action_target_x=x; action_target_y=y;
}

extern "C" int rand(void) {
    ++rand_calls; note_callback(10); return rand_value;
}


// ACTUAL_SOURCE_OWNER

static void reset() {
    int waypoint;
    VehCurrentCount = 2; BaseCurrentCount = 1; BaseFindDist = 9999;
    GameDataStateStorage.fields_.vehicle_current_count_ = VehCurrentCount;
    GameDataStateStorage.fields_.base_current_count_ = BaseCurrentCount;
    GameDataStateStorage.fields_.lock_masks_.enable_mask_ = 0;
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
    speed_calls = rand_calls = callback_count = 0;
    rand_value = 0; dropped_vehicle = dropped_x = dropped_y = -1;
    action_target_x = action_target_y = -1;
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
    int target_x = 5, target_y = 5;

    // Original early target mismatch is returned before speed/base callbacks.
    reset();
    Vehs[0].order = ORDER_MOVE_TO;
    Vehs[0].waypoint_x[0] = 4; Vehs[0].waypoint_y[0] = 5;
    assert(air_power(0, 1, 0, &target_x, &target_y) == -2);
    assert(speed_calls == 0 && base_at_calls == 0 && target_x == 5);
    reset();
    Vehs[0].order = ORDER_MOVE_TO;
    Vehs[0].waypoint_x[0] = 4; Vehs[0].waypoint_y[0] = 5;
    assert(air_power(0, 0, 1, &target_x, &target_y) == -1);
    assert(speed_calls == 0 && base_at_calls == 0);

    // Ordinary air-defense selection uses live candidate state and the mapped
    // combat/score path, then returns after committing the selected waypoint.
    reset();
    GameDataStateStorage.fields_.vehicle_current_count_ = 2;
    VehPrototypes[0].plan = PLAN_AIR_SUPERIORITY;
    Vehs[1].faction_id = 1; Vehs[1].visibility = 1; Vehs[1].x = 6;
    odds_result = 30;
    assert(air_power(0, 1, 0, &target_x, &target_y) == -2);
    assert(battle_calls == 1 && odds_calls == 1);
    { const int events[] = { 2, 3 }; assert_event_subsequence(events, 2); }
    assert(Vehs[0].order == ORDER_MOVE_TO && Vehs[0].move_to_ai_type == 0x61);
    assert(Vehs[0].waypoint_x[0] == 6 && Vehs[0].waypoint_y[0] == 5);
    assert(target_x == 5 && target_y == 5);

    // Equal-cost candidates preserve the source's <= tie rule (later ID wins).
    reset();
    GameDataStateStorage.fields_.vehicle_current_count_ = 3;
    VehPrototypes[0].plan = PLAN_AIR_SUPERIORITY;
    Vehs[1].faction_id = 1; Vehs[1].visibility = 1;
    Vehs[1].x = 6; Vehs[1].y = 6;
    Vehs[2].faction_id = 2; Vehs[2].visibility = 1;
    Vehs[2].x = 7; Vehs[2].y = 5;
    PlayersData[0].diplo_treaties[1] = PlayersData[0].diplo_treaties[2] = 0;
    assert(air_power(0, 1, 0, &target_x, &target_y) == -2);
    assert(Vehs[0].waypoint_x[0] == 7 && Vehs[0].waypoint_y[0] == 5);

    // At zero speed, an exact-zero distance still satisfies the mapped <= range test.
    reset();
    stub_speed = 0;
    VehPrototypes[0].plan = PLAN_AIR_SUPERIORITY;
    Vehs[1].faction_id = 1; Vehs[1].visibility = 1;
    assert(air_power(0, 1, 0, &target_x, &target_y) == -1);
    assert(Vehs[0].order == ORDER_MOVE_TO && Vehs[0].waypoint_x[0] == 5);

    // Range-one, undamaged aircraft gets the original extra search-speed read;
    // a distant visible defender is inside that extended radius only.
    reset();
    stub_speed = 12; Chassis[0].range = 1;
    VehPrototypes[0].plan = PLAN_AIR_SUPERIORITY;
    Vehs[0].terraforming_turns = 0;
    Vehs[1].faction_id = 1; Vehs[1].visibility = 1; Vehs[1].x = 11;
    assert(air_power(0, 1, 0, &target_x, &target_y) == -2);
    assert(speed_calls >= 3 && Vehs[0].waypoint_x[0] == 11);
    reset();
    stub_speed = 12; Chassis[0].range = 1;
    VehPrototypes[0].plan = PLAN_AIR_SUPERIORITY;
    Vehs[0].terraforming_turns = 0; Vehs[0].moves_expended = 3;
    Vehs[0].dmg_incurred = 1;
    Vehs[1].faction_id = 1; Vehs[1].visibility = 1; Vehs[1].x = 7;
    assert(air_power(0, 1, 0, &target_x, &target_y) == -2);
    assert(Vehs[0].waypoint_x[0] == 7 && speed_calls >= 2);
    reset();
    stub_speed = 12; Chassis[0].range = 2;
    VehPrototypes[0].plan = PLAN_AIR_SUPERIORITY;
    Vehs[0].terraforming_turns = 0; Vehs[0].moves_expended = 2;
    Vehs[1].faction_id = 1; Vehs[1].visibility = 1; Vehs[1].x = 11;
    assert(air_power(0, 1, 0, &target_x, &target_y) == -2);
    assert(Vehs[0].waypoint_x[0] == 11 && speed_calls >= 2);

    // Requested-territory relocation takes the first eligible offset; full-map
    // visibility suppresses offsets, so the native same-tile base fallback
    // consumes one random value and commits that base instead.
    reset();
    stub_speed = 2;
    Vehs[0].state = 0x4000;
    assert(air_power(0, 0, 0, &target_x, &target_y) == -2);
    assert(Vehs[0].order == ORDER_MOVE_TO && Vehs[0].move_to_ai_type == 0x61);
    assert(Vehs[0].waypoint_x[0] == 7 && Vehs[0].waypoint_y[0] == 3);
    assert(callback_count == 1 && callback_order[0] == 1);
    reset();
    Vehs[0].state = 0x4000;
    PlayersData[0].flags = 0x200;
    Bases[0].x = 7; Bases[0].y = 5;
    rand_value = 0;
    assert(air_power(0, 0, 0, &target_x, &target_y) == -2);
    assert(rand_calls == 1);
    { const int events[] = { 1, 10 }; assert_event_subsequence(events, 2); }
    assert(Vehs[0].order == ORDER_MOVE_TO && Vehs[0].move_to_ai_type == 0x63);
    assert(Vehs[0].waypoint_x[0] == Bases[0].x &&
           Vehs[0].waypoint_y[0] == Bases[0].y);
    reset();
    Vehs[0].state = 0x4000;
    PlayersData[0].flags = 0x200;
    GameDataStateStorage.fields_.base_current_count_ = 0;
    BaseCurrentCount = 0;
    assert(air_power(0, 0, 0, &target_x, &target_y) == -3);
    assert(Vehs[0].order != ORDER_MOVE_TO && rand_calls == 0);
    reset();
    VehPrototypes[0].plan = PLAN_COLONIZATION;
    Vehs[0].terraforming_turns = 7;
    Tiles[5 * MapLongitudeBounds + 5].climate = 0x60;
    PlayersData[0].region_base_plan[33] = 8;
    assert(air_power(0, 0, 0, &target_x, &target_y) == -2);
    assert(Vehs[0].order == ORDER_MOVE_TO && Vehs[0].move_to_ai_type == 0x61);
    assert(Vehs[0].terraforming_turns == 0 && rand_calls == 0);

    // Lock filtering requires both the faction lock bit and network mode.
    reset();
    Chassis[0].missile = 1; VehPrototypes[0].plan = 15;
    IsMultiplayerNet = 1;
    GameDataStateStorage.fields_.vehicle_current_count_ = 1;
    GameDataStateStorage.fields_.base_current_count_ = 0;
    BaseCurrentCount = 0;
    assert(air_power(0, 1, 0, &target_x, &target_y) == -3);
    assert(base_at_calls == 1);
    reset();
    Chassis[0].missile = 1; VehPrototypes[0].plan = 15;
    GameDataStateStorage.fields_.lock_masks_.enable_mask_ = 1;
    GameDataStateStorage.fields_.vehicle_current_count_ = 1;
    GameDataStateStorage.fields_.base_current_count_ = 0;
    BaseCurrentCount = 0;
    assert(air_power(0, 1, 0, &target_x, &target_y) == -3);
    assert(base_at_calls == 1);
    reset();
    Chassis[0].missile = 1; VehPrototypes[0].plan = 15;
    GameDataStateStorage.fields_.lock_masks_.enable_mask_ = 1;
    IsMultiplayerNet = 1;
    assert(air_power(0, 1, 0, &target_x, &target_y) == -3);
    assert(base_at_calls == 0);

    // The automatic bombing callback snapshots its waypoint/base selection,
    // bypasses ordinary battle callbacks, and preserves return/write order.
    reset();
    VehPrototypes[0].plan = PLAN_OFFENSIVE;
    Vehs[0].state = VSTATE_UNK_200;
    Vehs[0].order_auto_type = ORDERA_BOMBING_RUN;
    Vehs[0].waypoint_x[1] = 5; Vehs[0].waypoint_y[1] = 5;
    Bases[0].faction_id_current = 1;
    Bases[0].population_size = 3;
    Vehs[1].x = 5; Vehs[1].y = 5; Vehs[1].faction_id = 1;
    GameDataStateStorage.fields_.vehicle_current_count_ = 2;
    assert(air_power(0, 1, 0, &target_x, &target_y) == -2);
    assert(Vehs[0].order == ORDER_MOVE_TO && Vehs[0].waypoint_x[0] == 5);
    assert(vehicle_at_calls == 1 && stack_fix_calls == 1);
    assert(battle_calls == 0 && odds_calls == 0);
    { const int events[] = { 1, 11, 12 };
      assert_event_subsequence(events, 3); }

    // Missile plan 13 chooses a typed goal and preserves the original early
    // return before the later route/effect dispatch.
    reset();
    VehPrototypes[0].plan = 13; Chassis[0].missile = 1; stub_speed = 30;
    GameDataStateStorage.fields_.base_current_count_ = 0; BaseCurrentCount = 0;
    PlayersData[0].goals[0].type = 0x29;
    PlayersData[0].goals[0].priority = 5;
    PlayersData[0].goals[0].x = 6; PlayersData[0].goals[0].y = 5;
    Tiles[5 * MapLongitudeBounds + 6].climate = 0x60;
    assert(air_power(0, 1, 0, &target_x, &target_y) == -3);
    assert(Vehs[0].order == ORDER_MOVE_TO);
    assert(Vehs[0].waypoint_x[0] == 6 && Vehs[0].waypoint_y[0] == 5);
    assert(veh_lift_calls == 0 && veh_drop_calls == 0);
    assert(action_tectonic_calls == 0 && action_fungal_calls == 0);

    // Missile plan 14 selects a hostile base-radius tile and preserves the
    // separate fungal callback path.
    reset();
    VehPrototypes[0].plan = 14; Chassis[0].missile = 1; stub_speed = 30;
    GameDataStateStorage.fields_.base_current_count_ = 1;
    Bases[0].faction_id_current = 1;
    Bases[0].population_size = 3;
    PlayersData[0].diplo_treaties[1] = 0x10;
    PlayersData[1].soc_effect_pending.values_[SocialEffect::PLANET] = 1;
    PlayersData[0].region_total_bases[1] = 0;
    assert(air_power(0, 1, 0, &target_x, &target_y) == -5);
    assert(veh_drop_calls == 1 && dropped_vehicle == 40);
    assert(action_fungal_calls == 1 && action_tectonic_calls == 0);
    assert(action_target_x == 5 && action_target_y == 5);
    { const int events[] = { 6, 7, 9 }; assert_event_subsequence(events, 3); }

    // The general missile fallback chooses an enemy unit, advances to the
    // first valid neighboring tile, and uses the live veh_lift return value.
    reset();
    VehPrototypes[0].plan = 15; Chassis[0].missile = 1; stub_speed = 30;
    GameDataStateStorage.fields_.base_current_count_ = 0; BaseCurrentCount = 0;
    GameDataStateStorage.fields_.vehicle_current_count_ = 2;
    Vehs[1].faction_id = 1; Vehs[1].visibility = 1; Vehs[1].x = 6;
    PlayersData[0].diplo_treaties[1] = 0x10;
    stack_4 = 1;
    assert(air_power(0, 1, 0, &target_x, &target_y) == 4);
    assert(Vehs[0].order == 0 && Vehs[0].visibility == 0);
    assert(target_x == 7 && target_y == 4 && Vehs[0].facing == 0);
    assert(veh_drop_calls == 1 && dropped_vehicle == 40 &&
           dropped_x == 7 && dropped_y == 4);
    { const int events[] = { 2, 3, 6, 7 };
      assert_event_subsequence(events, 4); }

    // On-alert interception observes the callback's live plan mutation after
    // offense_proto returns and retains the original result formula.
    reset();
    VehPrototypes[0].plan = 6; Chassis[0].triad = TRIAD_AIR;
    Vehs[0].state = VSTATE_UNK_200; Vehs[0].order_auto_type = ORDERA_ON_ALERT;
    Vehs[0].terraforming_turns = 7;
    GameDataStateStorage.fields_.vehicle_current_count_ = 2;
    Vehs[1].faction_id = 1; Vehs[1].visibility = 0; Vehs[1].x = 6;
    Vehs[1].proto_id = 1; VehPrototypes[1].chassis_id = 1;
    Chassis[1].triad = TRIAD_LAND;
    PlayersData[1].diplo_treaties[0] = 0x10;
    Tiles[Vehs[1].y * MapLongitudeBounds + Vehs[1].x].bit = 1;
    offense_result = 10; mutate_plan_on_offense = 1;
    RadiusOffsetX[1] = 1; RadiusOffsetY[1] = 0;
    assert(air_power(0, 1, 1, &target_x, &target_y) == -1);
    assert(offense_calls == 2 && VehPrototypes[0].plan == PLAN_COLONIZATION);
    { const int events[] = { 11, 12, 14, 11, 12, 14 };
      assert_event_subsequence(events, 6); }
    assert(Vehs[0].order == ORDER_MOVE_TO && Vehs[0].move_to_ai_type == 0x4C);
    assert(Vehs[0].waypoint_x[0] == 6 && Vehs[0].terraforming_turns == 0);

    // A no-intercept alert with no waypoint change retains the original -4
    // outcome rather than manufacturing an order or callback.
    reset();
    VehPrototypes[0].plan = 6; Chassis[0].triad = TRIAD_AIR;
    Vehs[0].state = VSTATE_UNK_200; Vehs[0].order_auto_type = ORDERA_ON_ALERT;
    Vehs[0].waypoint_x[1] = Vehs[0].x;
    Vehs[0].waypoint_y[1] = Vehs[0].y;
    GameDataStateStorage.fields_.vehicle_current_count_ = 1;
    assert(air_power(0, 1, 1, &target_x, &target_y) == -4);
    assert(offense_calls == 0 && Vehs[0].order == 0);

    // A live action_home callback can set MOVE_TO and trigger the immediate
    // post-callback return; this is intentionally distinct from a snapshot.
    reset();
    Chassis[0].range = 1; Vehs[0].terraforming_turns = 1;
    action_home_sets_order = 1;
    assert(air_power(0, 1, 0, &target_x, &target_y) == -2);
    assert(home_calls == 1 && speed_calls == 2 && base_at_calls == 0);
    { const int events[] = { 1, 5 }; assert_event_subsequence(events, 2); }
    return 0;
}
