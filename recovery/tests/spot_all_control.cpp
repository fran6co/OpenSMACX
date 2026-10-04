#include "spot_all_control.h"

typedef char GameState_is_native_32_bit_owner[
    sizeof(GameState) == sizeof(uint32_t) ? 1 : -1];

Veh Vehs[32];
Map TestMap[32];
Base TestBases[8];
PlayerData PlayersData[8];
Player Players[8];
Console ConsoleGlobal;
uint32_t GameState;
int TurnCurrentNum;
uint32_t LockEnableMask;
uint32_t UnkBitfield1;
int TestMapWidth;
int TestMapHeight;
int TestMapSyncCount;
int TestWakeCount;
int TestWakeFaction;
int TestWakeSource;
int TestWantWakeCount;
int TestWantWakeArgs[16][3];
int TestEventOrder;
int TestDrawOrder;
int TestInterludeOrder;
int TestEncounterOrder;
int TestEncounterCount;
int TestEncounterFaction[16];
int TestEncounterArgs[16][6];
int TestGoalCount;
int TestGoalArgs[64][6];
int TestMultiplayer;
int MapLongitudeBounds;
int MapLatitudeBounds;
int MapLongitude;
int MapIsFlat;
int TestAbilityMask[512];
TestVehPrototype TestVehPrototypes[512];
TestRulesChassis TestRulesChassisTable[16];
TestRulesWeapon TestRulesWeaponTable[26];
int TestSensorPresent;
int TestSensorAt[32];
int TestUseSensorGrid;
int TestTerritoryOwner;
int TestSensorChecks;
int TestTerritoryChecks;
int TestVisibilityScanAllowed;
int TestSpotLocationCount;
int TestSpotLocationX[64];
int TestSpotLocationY[64];
int TestSpotLocationFaction[64];
int TestBaseAt[32];
int TestVehicleAt[32];
int TestStackCheckResult;
int TestStackCheckCount;
int TestSpotBaseCount;
int TestSpotBaseArgs[16][2];
int TestObservedVehicleId;
uint32_t TestObservedVehicleStateAtBaseReveal;
int TestRadiusDrawCount;
int TestRadiusDrawArgs[8][4];
int TestMainMapRedrawCount;
int TestInterludeCount;
int TestInterludeArgs[4][4];
MapWin *MapWinTable[1];
WorldWin WorldClimateWorldWin;
int TestWorldGraphicRedrawCount;
int TestWorldOnRedrawCount;
const int TestRadiusOffsetX[289] = {
     0,  1,  2,  1,  0, -1, -2, -1,  0,  2,  2, -2, -2,  1,  3,  3,
     1, -1, -3, -3, -1,  4, -4,  0,  0
};
const int TestRadiusOffsetY[289] = {
     0, -1,  0,  1,  2,  1,  0, -1, -2, -2,  2,  2, -2, -3, -1,  1,
     3,  3,  1, -1, -3,  0,  0,  4, -4
};
const int TestVehMoveDeltaX[9] = { 1, 2, 1, 0, -1, -2, -1, 0, 0 };
const int TestVehMoveDeltaY[9] = { -1, 0, 1, 2, 1, 0, -1, -2, 0 };
static int WakeOrder;
static int SyncOrder;
static int TerrainOrder;

int veh_top(int vehicle_id) {
    if (vehicle_id < 0) return -1;
    int top = vehicle_id;
    for (int16_t prev = Vehs[top].prev_veh_id_stack; prev >= 0;
         prev = Vehs[prev].prev_veh_id_stack) {
        top = prev;
    }
    return top;
}

int base_at(int x, int y) {
    if (!on_map(x, y)) return -1;
    return TestBaseAt[y * TestMapWidth + x];
}

int base_find(int x, int y) {
    const int base_id = base_at(x, y);
    return base_id >= 0 ? base_id : -1;
}

int veh_at(int x, int y) {
    if (!on_map(x, y)) return -1;
    return TestVehicleAt[y * TestMapWidth + x];
}

int stack_fix(int vehicle_id) {
    return vehicle_id < 0 ? -1 : veh_top(vehicle_id);
}

int stack_check(int, int type, int condition_one,
                int condition_two, int condition_three) {
    ++TestStackCheckCount;
    if (type != 1 || condition_one != -1 || condition_two != -1 ||
        condition_three != -1) return 0;
    return TestStackCheckResult;
}

BOOL on_map(int x, int y) {
    return x >= 0 && x < TestMapWidth && y >= 0 && y < TestMapHeight;
}

Map *map_loc(int x, int y) {
    return &TestMap[y * TestMapWidth + x];
}

void synch_bit(int, int, int) {
    ++TestMapSyncCount;
    SyncOrder = ++TestEventOrder;
}

void wake_stack(int) {
    ++TestWakeCount;
    WakeOrder = ++TestEventOrder;
}

BOOL want_to_wake(int faction_id, int vehicle_id, int spotted_vehicle_id) {
    const int index = TestWantWakeCount++;
    TestWakeFaction = faction_id;
    TestWakeSource = spotted_vehicle_id;
    TestWantWakeArgs[index][0] = faction_id;
    TestWantWakeArgs[index][1] = vehicle_id;
    TestWantWakeArgs[index][2] = spotted_vehicle_id;
    return 1;
}

void encounter(int source_faction_id, int target_faction_id, int x, int y,
               int event_flag_a, int event_flag_b) {
    TestEncounterOrder = ++TestEventOrder;
    const int index = TestEncounterCount;
    TestEncounterFaction[TestEncounterCount++] =
        source_faction_id * 8 + target_faction_id;
    TestEncounterArgs[index][0] = source_faction_id;
    TestEncounterArgs[index][1] = target_faction_id;
    TestEncounterArgs[index][2] = x;
    TestEncounterArgs[index][3] = y;
    TestEncounterArgs[index][4] = event_flag_a;
    TestEncounterArgs[index][5] = event_flag_b;
}

BOOL has_abil(int prototype_id, int ability_id) {
    return (TestAbilityMask[prototype_id] & ability_id) != 0;
}

BOOL is_sensor(int x, int y) {
    ++TestSensorChecks;
    if (TestUseSensorGrid && on_map(x, y))
        return TestSensorAt[y * TestMapWidth + x] != 0;
    return TestSensorPresent;
}

int whose_territory(int, int, int, int *, int) {
    ++TestTerritoryChecks;
    return TestTerritoryOwner;
}

void bitmask(int input, int *offset, int *mask) {
    *offset = input / 8;
    *mask = 1 << (input & 7);
}

int get_proto_triad(int prototype_id) {
    return TestRulesChassisTable[
        TestVehPrototypes[prototype_id].chassis_id].triad;
}

void spot_loc(int x, int y, int faction_id) {
    spot_tile(x, y, faction_id);
    const int base_id = base_at(x, y);
    if (base_id >= 0) {
        spot_base(base_id, faction_id);
    } else {
        const int top = veh_top(veh_at(x, y));
        const uint8_t mask = (uint8_t)(1u << faction_id);
        for (int id = top; id >= 0; id = Vehs[id].next_veh_id_stack) {
            Vehs[id].visibility |= mask;
            if (faction_id != 0)
                Vehs[id].flags &= (uint16_t)~(VFLAG_LURKER | VFLAG_INVISIBLE);
        }
    }
}

void spot_tile(int x, int y, int faction_id) {
    if (on_map(x, y)) {
        Map *tile = map_loc(x, y);
        const unsigned int mask = 1u << faction_id;
        const int index = TestSpotLocationCount++;
        TestSpotLocationX[index] = x;
        TestSpotLocationY[index] = y;
        TestSpotLocationFaction[index] = faction_id;
        if (faction_id == ConsoleGlobal.faction_id_ &&
            (tile->visibility & mask) == 0 &&
            (GameState & STATE_OMNISCIENT_VIEW) == 0 &&
            (PlayersData[faction_id].flags & PFLAG_MAP_REVEALED) == 0) {
            tile->bit2 |= 0x400000u;
            UnkBitfield1 |= 1u;
        }
        tile->visibility |= (uint8_t)mask;
        synch_bit(x, y, faction_id);
    }
}

void spot_stack(int vehicle_id, int faction_id) {
    if (vehicle_id >= 0 && on_map(Vehs[vehicle_id].x, Vehs[vehicle_id].y)) {
        spot_tile(Vehs[vehicle_id].x, Vehs[vehicle_id].y, faction_id);
    }
    const int top = veh_top(vehicle_id);
    const uint8_t mask = (uint8_t)(1u << faction_id);
    for (int id = top; id >= 0; id = Vehs[id].next_veh_id_stack) {
        Vehs[id].visibility |= mask;
        if (faction_id != 0)
            Vehs[id].flags &= (uint16_t)~(VFLAG_LURKER | VFLAG_INVISIBLE);
    }
}

void spot_base(int base_id, int faction_id) {
    const int index = TestSpotBaseCount++;
    if (TestObservedVehicleId >= 0)
        TestObservedVehicleStateAtBaseReveal =
            Vehs[TestObservedVehicleId].state;
    TestSpotBaseArgs[index][0] = base_id;
    TestSpotBaseArgs[index][1] = faction_id;
    Base &base = TestBases[base_id];
    const uint8_t faction_mask = (uint8_t)(1u << faction_id);
    base.visibility |= faction_mask;
    base.faction_pop_size_intel[faction_id] = (uint8_t)base.population_size;
    if (!on_map(base.x, base.y)) return;
    Map *tile = map_loc(base.x, base.y);
    if (faction_id == ConsoleGlobal.faction_id_ &&
        (tile->visibility & faction_mask) == 0 &&
        (GameState & STATE_OMNISCIENT_VIEW) == 0 &&
        (PlayersData[faction_id].flags & PFLAG_MAP_REVEALED) == 0) {
        tile->bit2 |= 0x400000u;
        UnkBitfield1 |= 1u;
    }
    tile->visibility |= faction_mask;
    synch_bit(base.x, base.y, faction_id);
}

void MapWin::on_redraw() {
    ++TestMainMapRedrawCount;
}

void GraphicWin::redraw() {
    ++TestWorldGraphicRedrawCount;
}

void WorldWin::on_redraw() {
    ++TestWorldOnRedrawCount;
}

void draw_radius(int x, int y, int radius, int draw_type) {
    TestDrawOrder = ++TestEventOrder;
    const int index = TestRadiusDrawCount++;
    TestRadiusDrawArgs[index][0] = x;
    TestRadiusDrawArgs[index][1] = y;
    TestRadiusDrawArgs[index][2] = radius;
    TestRadiusDrawArgs[index][3] = draw_type;
}

void interlude(int id, char *, int mode, int ending) {
    TestInterludeOrder = ++TestEventOrder;
    const int index = TestInterludeCount++;
    TestInterludeArgs[index][0] = id;
    TestInterludeArgs[index][1] = mode;
    TestInterludeArgs[index][2] = ending;
    TestInterludeArgs[index][3] = 0;
}

void add_goal(int faction_id, int subgoal, int goal_type,
              int x, int y, int expiry) {
    const int index = TestGoalCount++;
    TestGoalArgs[index][0] = faction_id;
    TestGoalArgs[index][1] = subgoal;
    TestGoalArgs[index][2] = goal_type;
    TestGoalArgs[index][3] = x;
    TestGoalArgs[index][4] = y;
    TestGoalArgs[index][5] = expiry;
}

// @@ACTUAL_SPOT_ALL_OWNER@@

static void clear_state() {
    for (int i = 0; i != 32; ++i) {
        Vehs[i] = Veh();
        Vehs[i].prev_veh_id_stack = -1;
        Vehs[i].next_veh_id_stack = -1;
        TestMap[i] = Map();
    }
    for (int f = 0; f != 8; ++f) {
        PlayersData[f].flags = 0;
        PlayersData[f].x_target = -1;
        PlayersData[f].y_target = -1;
        Players[f].rule_flags = 0;
        for (int other = 0; other != 8; ++other)
            PlayersData[f].diplo_treaties[other] = 0;
    }
    for (int weapon = 0; weapon != 26; ++weapon)
        TestRulesWeaponTable[weapon].offense_rating = 0;
    ConsoleGlobal.faction_id_ = 1;
    GameState = 0;
    LockEnableMask = 0;
    UnkBitfield1 = 0;
    TestMapWidth = 4;
    TestMapHeight = 4;
    TestMapSyncCount = 0;
    TestWakeCount = 0;
    TestWakeFaction = -1;
    TestWakeSource = -1;
    TestStackCheckResult = 0;
    TestStackCheckCount = 0;
    TestWantWakeCount = 0;
    TestEventOrder = 0;
    TestDrawOrder = 0;
    TestInterludeOrder = 0;
    TestEncounterOrder = 0;
    TestEncounterCount = 0;
    TestGoalCount = 0;
    TestMultiplayer = 1;
    MapLongitudeBounds = 4;
    MapLatitudeBounds = 4;
    MapLongitude = 2;
    MapIsFlat = 1;
    for (int proto = 0; proto != 512; ++proto) {
        TestAbilityMask[proto] = 0;
        TestVehPrototypes[proto].chassis_id = 0;
        TestVehPrototypes[proto].weapon_id = 0;
        TestVehPrototypes[proto].plan = 0;
    }
    for (int chassis = 0; chassis != 16; ++chassis)
        TestRulesChassisTable[chassis].triad = 0;
    TestSensorPresent = 0;
    TestUseSensorGrid = 0;
    TestTerritoryOwner = -1;
    TestSensorChecks = 0;
    TestTerritoryChecks = 0;
    TestVisibilityScanAllowed = 1;
    TestSpotLocationCount = 0;
    TestSpotBaseCount = 0;
    TestObservedVehicleId = -1;
    TestObservedVehicleStateAtBaseReveal = 0;
    for (int i = 0; i != 32; ++i) {
        TestBaseAt[i] = -1;
        TestVehicleAt[i] = -1;
        TestSensorAt[i] = 0;
    }
    for (int i = 0; i != 8; ++i) TestBases[i] = Base();
    TurnCurrentNum = 0;
    TestRadiusDrawCount = 0;
    TestMainMapRedrawCount = 0;
    TestWorldGraphicRedrawCount = 0;
    TestWorldOnRedrawCount = 0;
    TestInterludeCount = 0;
    MapWinTable[0] = 0;
    WakeOrder = 0;
    SyncOrder = 0;
    TerrainOrder = 0;
}

static int check_stack_visibility_and_owner_exceptions() {
    clear_state();
    Vehs[4].prev_veh_id_stack = -1;
    Vehs[4].next_veh_id_stack = 3;
    Vehs[3].prev_veh_id_stack = 4;
    Vehs[3].next_veh_id_stack = 2;
    Vehs[2].prev_veh_id_stack = 3;
    Vehs[2].next_veh_id_stack = -1;
    Vehs[4].flags = VFLAG_LURKER | VFLAG_INVISIBLE;
    Vehs[4].x = 0;
    Vehs[4].y = 0;
    Vehs[4].faction_id = 1;
    Vehs[3].flags = VFLAG_LURKER | VFLAG_INVISIBLE;
    Vehs[3].x = 1;
    Vehs[3].y = 0;
    Vehs[3].faction_id = 2;
    Vehs[2].flags = VFLAG_LURKER | VFLAG_INVISIBLE;
    Vehs[2].x = 2;
    Vehs[2].y = 0;
    Vehs[2].faction_id = 3;

    mark_stack_seen_by_faction(3, 1);
    if (Vehs[4].visibility != 2 || Vehs[3].visibility != 2 ||
        Vehs[2].visibility != 2) return 1;
    if ((Vehs[4].flags & (VFLAG_LURKER | VFLAG_INVISIBLE)) != 0 ||
        (Vehs[3].flags & (VFLAG_LURKER | VFLAG_INVISIBLE)) != 0 ||
        (Vehs[2].flags & (VFLAG_LURKER | VFLAG_INVISIBLE)) != 0) return 2;
    if (TestSpotLocationCount != 0) return 5;

    Vehs[4].flags = VFLAG_LURKER | VFLAG_INVISIBLE;
    Vehs[3].flags = VFLAG_LURKER | VFLAG_INVISIBLE;
    Vehs[2].flags = VFLAG_LURKER | VFLAG_INVISIBLE;
    mark_stack_seen_by_faction(3, 0);
    if (Vehs[4].visibility != 3 || Vehs[3].visibility != 3 ||
        Vehs[2].visibility != 3) return 3;
    if ((Vehs[4].flags & (VFLAG_LURKER | VFLAG_INVISIBLE)) == 0 ||
        (Vehs[3].flags & (VFLAG_LURKER | VFLAG_INVISIBLE)) == 0 ||
        (Vehs[2].flags & (VFLAG_LURKER | VFLAG_INVISIBLE)) == 0) return 4;
    return 0;
}

static int check_owner_tile_visibility_and_discovery_guards() {
    clear_state();
    spot_tile(1, 2, 3);
    if (map_loc(1, 2)->visibility != 8 || TestMapSyncCount != 1) return 1;
    if (TestMapSyncCount != 0) {
        const uint32_t value = map_loc(1, 2)->bit_visible[3];
        if (value != 0) return 2; // The fixture synch hook records, it does not mutate.
    }

    spot_tile(0, 0, 1);
    if (map_loc(0, 0)->bit2 != 0x400000u || UnkBitfield1 != 1) return 3;
    spot_tile(0, 0, 1);
    if (UnkBitfield1 != 1) return 4;
    GameState = STATE_OMNISCIENT_VIEW;
    map_loc(0, 1)->visibility = 0;
    spot_tile(0, 1, 1);
    if (map_loc(0, 1)->bit2 != 0 || UnkBitfield1 != 1) return 5;
    GameState = 0;
    PlayersData[1].flags = PFLAG_MAP_REVEALED;
    spot_tile(0, 2, 1);
    if (map_loc(0, 2)->bit2 != 0 || UnkBitfield1 != 1) return 6;
    PlayersData[1].flags = 0;
    spot_tile(0, 3, 2);
    if (map_loc(0, 3)->bit2 != 0 || UnkBitfield1 != 1) return 7;
    return 0;
}

static int check_effect_order_and_wake_inputs() {
    clear_state();
    Vehs[1].prev_veh_id_stack = -1;
    Vehs[1].next_veh_id_stack = 2;
    Vehs[1].state = VSTATE_UNK_200 | VSTATE_UNK_2000000;
    Vehs[1].patrol_current_point = 0;
    Vehs[1].waypoint_count = 1;
    Vehs[1].order = 0x18;
    Vehs[2].prev_veh_id_stack = 1;
    Vehs[2].next_veh_id_stack = -1;
    Vehs[2].state = VSTATE_UNK_200 | VSTATE_UNK_2000000;
    Vehs[2].patrol_current_point = 0;
    Vehs[2].waypoint_count = 1;
    Vehs[2].order = 0x18;
    TestVehicleAt[1 * TestMapWidth + 2] = 1;
    SpotAllSourceContext reveal_source;
    reveal_source.faction_id = 1;
    reveal_source.special_terrain_mode = 0;
    reveal_source.source_low_terrain = 1;
    reveal_source.vehicle_triad = 1;

    apply_spotted_location_effects(reveal_source, 1, 2, 0, 1, 0, 2, 1);
    if (TestWakeCount != 0 || TestWakeFaction != -1 || TestWakeSource != -1)
        return 1;
    if ((Vehs[1].visibility & 4) == 0) return 2;
    if ((Vehs[1].state & 0x2000200u) != 0 || Vehs[1].order != 0) return 3;
    if ((Vehs[2].state & 0x2000200u) != 0 || Vehs[2].order != 0 ||
        (Vehs[2].visibility & 4u) == 0) return 10;
    if (TestMapSyncCount != 1 || map_loc(2, 1)->visibility != 4) return 4;
    if (SyncOrder == 0) return 5;

    clear_state();
    Vehs[1].proto_id = 20;
    Vehs[1].faction_id = 2;
    Vehs[1].prev_veh_id_stack = -1;
    Vehs[1].next_veh_id_stack = -1;
    TestAbilityMask[20] = 2;
    TestRulesChassisTable[0].triad = 1;
    PlayersData[1].diplo_treaties[2] = 0;
    map_loc(1, 1)->climate = 0x60;
    map_loc(1, 1)->territory = 2;
    SpotAllSourceContext source;
    source.faction_id = 1;
    source.special_terrain_mode = 0;
    source.source_low_terrain = 1;
    source.vehicle_triad = 1;
    wake_contact_sensor_stack(source, 1, 2, 1);
    if (TestWakeCount != 1 || WakeOrder == 0) return 6;
    PlayersData[1].diplo_treaties[2] = 1;
    wake_contact_sensor_stack(source, 1, 2, 1);
    if (TestWakeCount != 1) return 7;
    source.special_terrain_mode = 1;
    PlayersData[1].diplo_treaties[2] = 0;
    wake_contact_sensor_stack(source, 1, 2, 1);
    if (TestWakeCount != 1) return 8;

    source.special_terrain_mode = 0;
    if (may_query_wake_policy(source, 5, 2, 1)) return 10;
    LockEnableMask = 1u << 2;
    if (!may_query_wake_policy(source, 5, 2, 1)) return 11;
    if (may_query_wake_policy(source, 5, 2, 0)) return 18;
    PlayersData[1].diplo_treaties[2] = 0;
    LockEnableMask = 0;
    if (!should_use_target_stack_wake_fallback(source, 5, 2, 1)) return 12;
    if (should_use_target_stack_wake_fallback(source, 5, 2, 0))
        return 27;
    LockEnableMask = (1u << 1) | (1u << 2);
    if (!may_query_wake_policy(source, 5, 2, 1)) return 13;
    if (should_use_target_stack_wake_fallback(source, 5, 2, 0)) return 14;
    if (!may_query_sensor_owner_wake_policy(source, 5, 2, 30, 1)) return 16;
    PlayersData[1].diplo_treaties[2] = 1;
    if (!may_query_sensor_owner_wake_policy(source, 5, 2, 30, 1)) return 17;
    PlayersData[1].diplo_treaties[2] = 0;
    LockEnableMask = 0;
    source.source_low_terrain = 0;
    if (!should_use_target_stack_wake_fallback(source, 5, 2, 0)) return 15;
    if (should_use_source_vehicle_wake_fallback(source, 5, 2, 1)) return 19;
    if (!should_use_target_stack_wake_fallback(source, 5, 2, 1)) return 20;
    source.source_low_terrain = 1;
    if (!should_use_source_vehicle_wake_fallback(source, 5, 2, 1)) return 21;
    source.source_low_terrain = 0;
    source.vehicle_triad = 2;
    LockEnableMask = 1u << 1;
    if (!may_query_sensor_owner_wake_policy(source, 5, 2, 30, 1)) return 22;
    LockEnableMask = 0;
    if (!should_use_source_vehicle_wake_fallback(source, 5, 2, 0))
        return 26;
    if (!should_use_source_vehicle_wake_fallback(source, 5, 2, 1)) return 23;
    source.special_terrain_mode = 1;
    LockEnableMask = 1u << 1;
    if (!may_query_sensor_owner_wake_policy(source, 5, 2, 30, 1)) return 24;
    source.vehicle_triad = 1;
    if (may_query_sensor_owner_wake_policy(source, 5, 2, 30, 1)) return 25;

    clear_state();
    apply_spotted_location_effects(source, -1, 1, 0, 0, 0, 1, 0);
    if (TestWakeCount != 0 || TestMapSyncCount != 1) return 9;
    return 0;
}

static int check_encounter_notification_order_and_gates() {
    clear_state();
    report_spot_encounters(4, 1 | (1 << 1) | (1 << 4) | (1 << 6),
                           1, 11, 20, 30, 40);
    if (TestEncounterCount != 3) return 1;
    if (TestEncounterFaction[0] != 32 ||
        TestEncounterFaction[1] != 32 + 1 ||
        TestEncounterFaction[2] != 32 + 6) return 2;
    if (TestEncounterArgs[0][2] != 11 || TestEncounterArgs[0][3] != 20 ||
        TestEncounterArgs[0][4] != 30 || TestEncounterArgs[0][5] != 40 ||
        TestEncounterArgs[2][2] != 11 || TestEncounterArgs[2][3] != 20 ||
        TestEncounterArgs[2][4] != 0 ||
        TestEncounterArgs[2][5] != 0) return 3;
    clear_state();
    report_spot_encounters(4, 1 | (1 << 1) | (1 << 6),
                           0, 11, 20, 30, 40);
    if (TestEncounterCount != 1 || TestEncounterFaction[0] != 32) return 3;
    clear_state();
    report_spot_encounters(0, 1, 1, 11, 20, 30, 40);
    if (TestEncounterCount != 1 || TestEncounterFaction[0] != 0) return 4;
    return 0;
}

static int check_source_policy_branches() {
    clear_state();
    Vehs[3].x = 1;
    Vehs[3].y = 1;
    Vehs[3].faction_id = 2;
    Vehs[3].proto_id = 20;
    TestMultiplayer = 0;
    TestAbilityMask[20] = 2;
    SpotAllSourceContext source = inspect_spot_all_source(3, 0);
    if (source.effective_toggle != 1 || source.offset_limit != 49 ||
        source.source_controls_sensor_radius != 1) return 1;
    if (TestSensorChecks != 0 || TestTerritoryChecks != 0) return 2;

    clear_state();
    Vehs[3].x = 1;
    Vehs[3].y = 1;
    Vehs[3].faction_id = 2;
    Vehs[3].proto_id = 18;
    map_loc(1, 1)->climate = 0x60;
    map_loc(1, 1)->bit = 0x20u;
    source = inspect_spot_all_source(3, 1);
    if (source.special_terrain_mode != 2) return 31;
    if (source.offset_limit != 25) return 32;

    Vehs[3].proto_id = 20;
    source = inspect_spot_all_source(3, 1);
    if (source.special_terrain_mode != 2)
        return 100 + source.special_terrain_mode;
    Vehs[3].proto_id = 0x13;
    source = inspect_spot_all_source(3, 1);
    if (source.special_terrain_mode != 0) return 8;

    clear_state();
    Vehs[3].x = 1;
    Vehs[3].y = 1;
    Vehs[3].faction_id = 2;
    Vehs[3].proto_id = 20;
    TestSensorPresent = 1;
    TestTerritoryOwner = 2;
    source = inspect_spot_all_source(3, 1);
    if (source.source_controls_sensor_radius != 1 || source.offset_limit != 49 ||
        TestSensorChecks != 1 || TestTerritoryChecks != 1) return 4;
    TestTerritoryOwner = 3;
    source = inspect_spot_all_source(3, 1);
    if (source.source_controls_sensor_radius != 0 || source.offset_limit != 25)
        return 5;

    clear_state();
    Vehs[3].proto_id = 20;
    TestAbilityMask[20] = 0x40;
    source = inspect_spot_all_source(3, 1);
    if (source.special_terrain_mode != 1) return 6;
    return 0;
}

static int check_visibility_scan_geometry_and_gate() {
    clear_state();
    Vehs[3].x = 0;
    Vehs[3].y = 1;
    Vehs[3].faction_id = 1;
    Vehs[3].proto_id = 20;
    TestAbilityMask[20] = 0x40;
    MapIsFlat = 0;
    SpotAllSourceContext source = inspect_spot_all_source(3, 1);
    if (source.visibility_offset_limit != 9) return 1;
    (void)scan_optical_location_radius(source);
    if (TestSpotLocationCount != 0) return 2;

    clear_state();
    Vehs[3].faction_id = 2;
    Vehs[3].proto_id = 20;
    source = inspect_spot_all_source(3, 1);
    (void)scan_optical_location_radius(source);
    if (TestSpotLocationCount != 0) return 4;
    TestVisibilityScanAllowed = 1;
    ConsoleGlobal.faction_id_ = 2;
    source = inspect_spot_all_source(3, 1);
    TestVisibilityScanAllowed = 0;
    (void)scan_optical_location_radius(source);
    if (TestSpotLocationCount != 0) return 5;

    clear_state();
    Vehs[3].x = 1;
    Vehs[3].y = 1;
    Vehs[3].faction_id = 1;
    Vehs[3].proto_id = 20;
    source = inspect_spot_all_source(3, 1);
    const int sensor_checks_before_omniscient_scan = TestSensorChecks;
    GameState = STATE_OMNISCIENT_VIEW;
    (void)scan_optical_location_radius(source);
    if (TestSensorChecks != sensor_checks_before_omniscient_scan) return 16;

    clear_state();
    Vehs[3].x = 0;
    Vehs[3].y = 1;
    Vehs[3].faction_id = 1;
    Vehs[3].proto_id = 20;
    MapIsFlat = 0;
    source = inspect_spot_all_source(3, 1);
    map_loc(3, 0)->bit = 0x1234u;
    (void)scan_terrain_visibility_radius(source);
    if ((map_loc(3, 0)->visibility & 2) == 0) return 17;
    if (map_loc(3, 0)->bit_visible[0] != 0x1234u) return 7;
    if ((map_loc(3, 0)->bit2 & 0x400000u) == 0 || UnkBitfield1 != 1) return 8;

    clear_state();
    Vehs[3].x = 0;
    Vehs[3].y = 1;
    Vehs[3].faction_id = 2;
    Vehs[3].proto_id = 20;
    MapIsFlat = 0;
    source = inspect_spot_all_source(3, 1);
    map_loc(3, 0)->bit = 0x5678u;
    (void)scan_terrain_visibility_radius(source);
    if ((map_loc(3, 0)->visibility & 4) == 0 ||
        map_loc(3, 0)->bit_visible[1] != 0x5678u ||
        map_loc(3, 0)->bit2 != 0 || UnkBitfield1 != 0) return 9;

    clear_state();
    ConsoleGlobal.faction_id_ = 0;
    Vehs[3].x = 0;
    Vehs[3].y = 1;
    Vehs[3].faction_id = 0;
    Vehs[3].proto_id = 20;
    source = inspect_spot_all_source(3, 1);
    map_loc(0, 1)->bit2 = 0x2000u;
    map_loc(0, 1)->bit = 0x1234u;
    (void)scan_terrain_visibility_radius(source);
    if ((map_loc(0, 1)->bit2 & 0x400000u) == 0 ||
        UnkBitfield1 != 1 || map_loc(0, 1)->bit_visible[0] != 0)
        return 18;

    clear_state();
    Vehs[3].x = 1;
    Vehs[3].y = 1;
    Vehs[3].faction_id = 1;
    Vehs[3].proto_id = 20;
    ConsoleGlobal.faction_id_ = 1;
    PlayersData[1].flags = PFLAG_MAP_REVEALED;
    source = inspect_spot_all_source(3, 1);
    map_loc(1, 1)->bit = 0x1234u;
    (void)scan_terrain_visibility_radius(source);
    if ((map_loc(1, 1)->bit2 & 0x400000u) != 0 || UnkBitfield1 != 0)
        return 19;

    clear_state();
    Vehs[3].x = 1;
    Vehs[3].y = 1;
    Vehs[3].faction_id = 1;
    Vehs[3].proto_id = 20;
    ConsoleGlobal.faction_id_ = 1;
    PlayersData[1].diplo_treaties[0] = DTREATY_UNK_200;
    source = inspect_spot_all_source(3, 1);
    map_loc(1, 1)->bit = 0x1234u;
    (void)scan_terrain_visibility_radius(source);
    if ((map_loc(1, 1)->bit2 & 0x400000u) == 0 || UnkBitfield1 != 1 ||
        source.early_radius_tier != 2 ||
        (map_loc(1, 1)->visibility & 2u) == 0 ||
        map_loc(1, 1)->bit_visible[0] != 0x1234u)
        return 20;

    clear_state();
    Vehs[3].x = 1;
    Vehs[3].y = 1;
    Vehs[3].faction_id = 1;
    Vehs[3].proto_id = 20;
    source = inspect_spot_all_source(3, 1);
    for (int y = 0; y != TestMapHeight; ++y) {
        for (int x = 0; x != TestMapWidth; ++x) {
            map_loc(x, y)->visibility = 2;
        }
    }
    map_loc(2, 0)->visibility = 0;
    map_loc(3, 1)->visibility = 0;
    map_loc(2, 2)->visibility = 0;
    if (count_hidden_neighbor_tiles(1, 1, 2) != 4) return 10;
    map_loc(1, 3)->visibility = 0;
    if (count_hidden_neighbor_tiles(1, 1, 2) != 6)
        return 30 + count_hidden_neighbor_tiles(1, 1, 2);
    for (int y = 0; y != TestMapHeight; ++y) {
        for (int x = 0; x != TestMapWidth; ++x) {
            map_loc(x, y)->visibility = 2;
        }
    }
    (void)scan_adjacent_sensor_perimeter(source);
    if (TestSpotLocationCount != 0 || source.early_radius_tier != 0) return 12;
    TestMapWidth = 5;
    TestMapHeight = 5;
    MapLongitudeBounds = 5;
    MapLatitudeBounds = 5;
    for (int y = 0; y != TestMapHeight; ++y) {
        for (int x = 0; x != TestMapWidth; ++x) {
            map_loc(x, y)->visibility = 2;
        }
    }
    map_loc(4, 2)->visibility = 0;
    if (scan_adjacent_sensor_perimeter(source) < 1 ||
        source.early_radius_tier != 2) return 21;
    clear_state();
    Vehs[3].x = 1;
    Vehs[3].y = 1;
    Vehs[3].faction_id = 1;
    Vehs[3].proto_id = 20;
    source = inspect_spot_all_source(3, 1);
    for (int y = 0; y != TestMapHeight; ++y) {
        for (int x = 0; x != TestMapWidth; ++x) {
            map_loc(x, y)->visibility = 2;
        }
    }
    map_loc(2, 0)->visibility = 0;
    map_loc(3, 1)->visibility = 0;
    map_loc(2, 2)->visibility = 0;
    map_loc(1, 3)->visibility = 0;
    map_loc(0, 2)->visibility = 0;
    map_loc(0, 0)->visibility = 0;
    (void)scan_adjacent_sensor_perimeter(source);
    if (TestSpotLocationCount != 0) return 13;
    return 0;
}

static int check_visibility_scan_source_triad_terrain_filter() {
    clear_state();
    TestMapWidth = 5;
    TestMapHeight = 5;
    MapLongitudeBounds = 5;
    MapLatitudeBounds = 5;
    MapLongitude = 2;
    Vehs[3].x = 2;
    Vehs[3].y = 2;
    Vehs[3].faction_id = 1;
    Vehs[3].proto_id = 20;
    TestAbilityMask[20] = 2;
    TestRulesChassisTable[0].triad = 0;
    SpotAllSourceContext source = inspect_spot_all_source(3, 1);
    scan_terrain_visibility_radius(source);
    if ((map_loc(4, 0)->visibility & 2u) != 0) return 1;

    clear_state();
    TestMapWidth = 5;
    TestMapHeight = 5;
    MapLongitudeBounds = 5;
    MapLatitudeBounds = 5;
    MapLongitude = 2;
    Vehs[3].x = 2;
    Vehs[3].y = 2;
    Vehs[3].faction_id = 1;
    Vehs[3].proto_id = 20;
    TestAbilityMask[20] = 2;
    TestRulesChassisTable[0].triad = 1;
    source = inspect_spot_all_source(3, 1);
    scan_terrain_visibility_radius(source);
    if ((map_loc(4, 0)->visibility & 2u) == 0) return 2;

    clear_state();
    TestMapWidth = 5;
    TestMapHeight = 5;
    MapLongitudeBounds = 5;
    MapLatitudeBounds = 5;
    MapLongitude = 2;
    Vehs[3].x = 2;
    Vehs[3].y = 2;
    Vehs[3].faction_id = 1;
    Vehs[3].proto_id = 20;
    TestAbilityMask[20] = 2;
    TestRulesChassisTable[0].triad = 2;
    map_loc(4, 0)->climate = 0x60u;
    source = inspect_spot_all_source(3, 1);
    scan_terrain_visibility_radius(source);
    if ((map_loc(4, 0)->visibility & 2u) == 0) return 3;
    return 0;
}

static int check_sensor_contact_goals_and_stack_order() {
    clear_state();
    Vehs[3].x = 1;
    Vehs[3].y = 1;
    Vehs[3].faction_id = 1;
    Vehs[3].proto_id = 20;
    TestRulesChassisTable[0].triad = 0;
    Vehs[3].prev_veh_id_stack = -1;
    Vehs[3].next_veh_id_stack = -1;
    Vehs[3].flags = VFLAG_LURKER;
    TestVehicleAt[1 * TestMapWidth + 1] = 3;
    map_loc(1, 1)->territory = 2;
    TestSensorPresent = 1;
    SpotAllSourceContext source = inspect_spot_all_source(3, 1);
    if (!is_sensor(1, 1) || map_loc(1, 1)->territory != 2) return 10;
    int encountered_factions = 0;
    const int contacts = scan_enemy_sensor_contacts(source, 3, 1, 0, 1,
                                                    &encountered_factions);
    if (contacts != 1) return 1;
    if (encountered_factions != 0) return 9;
    if (TestGoalCount != 2) return 6;
    if (TestGoalArgs[0][0] != 2 || TestGoalArgs[0][1] != 0 ||
        TestGoalArgs[0][2] != 2 || TestGoalArgs[0][3] != 1 ||
        TestGoalArgs[0][4] != 1 || TestGoalArgs[1][1] != 2 ||
        TestGoalArgs[1][2] != 1) return 2;
    if (TestSpotLocationCount != 1 || TestSpotLocationFaction[0] != 2 ||
        (Vehs[3].visibility & 4) == 0 ||
        (Vehs[3].flags & (VFLAG_LURKER | VFLAG_INVISIBLE)) != 0) return 3;

    clear_state();
    Vehs[3].x = 1;
    Vehs[3].y = 1;
    Vehs[3].faction_id = 1;
    Vehs[3].proto_id = 20;
    map_loc(1, 1)->territory = 2;
    map_loc(1, 1)->bit = 1;
    map_loc(1, 1)->val2 = 2;
    TestSensorPresent = 1;
    source = inspect_spot_all_source(3, 1);
    if (scan_enemy_sensor_contacts(source, 3, 1, 0, 1, 0) != 0 ||
        TestGoalCount != 0 || TestSpotLocationCount != 0) return 14;

    clear_state();
    Vehs[3].x = 1;
    Vehs[3].y = 1;
    Vehs[3].faction_id = 1;
    Vehs[3].proto_id = 20;
    map_loc(1, 1)->territory = 2;
    PlayersData[1].diplo_treaties[2] = 1;
    TestSensorPresent = 1;
    source = inspect_spot_all_source(3, 0);
    if (scan_enemy_sensor_contacts(source, 3, 1, 0, 1, 0) != 1 ||
        TestGoalCount != 0) return 4;

    clear_state();
    Vehs[3].x = 1;
    Vehs[3].y = 1;
    Vehs[3].faction_id = 1;
    Vehs[3].proto_id = 20;
    TestAbilityMask[20] = 0x40;
    map_loc(1, 1)->territory = 2;
    TestSensorPresent = 1;
    source = inspect_spot_all_source(3, 1);
    if (scan_enemy_sensor_contacts(source, 3, 1, 0, 1, 0) != 0 ||
        TestGoalCount != 0) return 5;

    clear_state();
    Vehs[3].x = 1;
    Vehs[3].y = 1;
    Vehs[3].faction_id = 1;
    Vehs[3].proto_id = 20;
    Vehs[3].prev_veh_id_stack = -1;
    Vehs[3].next_veh_id_stack = -1;
    Vehs[6].x = 2;
    Vehs[6].y = 0;
    Vehs[6].faction_id = 3;
    Vehs[6].proto_id = 30;
    Vehs[6].prev_veh_id_stack = -1;
    Vehs[6].next_veh_id_stack = -1;
    Vehs[6].flags = VFLAG_LURKER | VFLAG_INVISIBLE;
    Vehs[6].state = VSTATE_UNK_200 | VSTATE_UNK_1000000;
    Vehs[6].order = 0x18;
    TestBaseAt[2] = 7;
    TestVehicleAt[2] = 6;
    TestBases[7].x = 2;
    TestBases[7].y = 0;
    TestBases[7].population_size = 6;
    TestAbilityMask[30] = 0x44;
    TestVehPrototypes[20].plan = 0x0B;
    map_loc(2, 0)->territory = 2;
    TestSensorPresent = 1;
    LockEnableMask = (1u << 1) | (1u << 2);
    TestObservedVehicleId = 6;
    source = inspect_spot_all_source(3, 1);
    encountered_factions = 0;
    const int detailed_contacts = scan_enemy_sensor_contacts(
        source, 3, 1, 1, 2, &encountered_factions);
    if (detailed_contacts != 1) return 20 + detailed_contacts;
    if (TestSpotBaseCount != 0) return 11;
    if ((TestBases[7].visibility & 2) == 0 ||
        TestBases[7].faction_pop_size_intel[1] != 6 ||
        (map_loc(2, 0)->visibility & 2) == 0 ||
        (map_loc(2, 0)->bit2 & 0x400000u) == 0 ||
        source.early_radius_tier != 2) return 15;
    if (TestWakeCount != 1 || (Vehs[6].visibility & 2) == 0 ||
        (Vehs[6].flags & (VFLAG_LURKER | VFLAG_INVISIBLE)) != 0 ||
        Vehs[6].order != 0) return 12;
    if (TestWantWakeCount != 1 || TestWantWakeArgs[0][0] != 3 ||
        TestWantWakeArgs[0][1] != 3 || TestWantWakeArgs[0][2] != 6 ||
        TestWakeFaction != 3 || TestWakeSource != 6) return 17;
    if (encountered_factions != ((1 << 0) | (1 << 3)))
        return 13;
    if (TestGoalCount != 5) return 101;
    if (TestGoalArgs[0][0] != 2 ||
        TestGoalArgs[0][1] != AI_GOAL_ATTACK ||
        TestGoalArgs[0][3] != 2 || TestGoalArgs[0][4] != 0) return 102;
    if (TestGoalArgs[1][0] != 2 ||
        TestGoalArgs[1][1] != AI_GOAL_DEFEND) return 103;
    if (TestGoalArgs[2][0] != 1 ||
        TestGoalArgs[2][1] != AI_GOAL_UNK_1 ||
        TestGoalArgs[2][2] != 1) return 104;
    if (TestGoalArgs[3][0] != 1 || TestGoalArgs[3][1] != AI_GOAL_ATTACK ||
        TestGoalArgs[3][2] != 3) return 105;
    if (TestGoalArgs[4][0] != 1 ||
        TestGoalArgs[4][1] != AI_GOAL_ATTACK ||
        TestGoalArgs[4][2] != 2 || TestGoalArgs[4][3] != 2 ||
        TestGoalArgs[4][4] != 0 || TestGoalArgs[4][5] != -1) return 107;

    clear_state();
    TestMapWidth = 5;
    TestMapHeight = 5;
    MapLongitudeBounds = 5;
    MapLatitudeBounds = 5;
    MapLongitude = 2;
    Vehs[3].x = 2;
    Vehs[3].y = 2;
    Vehs[3].faction_id = 1;
    Vehs[3].proto_id = 20;
    Vehs[3].prev_veh_id_stack = -1;
    Vehs[3].next_veh_id_stack = -1;
    Vehs[6].x = 2;
    Vehs[6].y = 0;
    Vehs[6].faction_id = 3;
    Vehs[6].proto_id = 30;
    Vehs[6].prev_veh_id_stack = -1;
    Vehs[6].next_veh_id_stack = -1;
    TestVehicleAt[2] = 6;
    TestVehPrototypes[20].plan = 0x0B;
    PlayersData[1].diplo_treaties[2] = 1;
    map_loc(2, 0)->territory = 2;
    TestSensorPresent = 1;
    source = inspect_spot_all_source(3, 1);
    scan_enemy_sensor_contacts(source, 3, 1, 1, 2, 0);
    if (TestGoalCount != 0) return 106;

    clear_state();
    Vehs[3].x = 1;
    Vehs[3].y = 1;
    Vehs[3].faction_id = 1;
    Vehs[3].proto_id = 20;
    TestBaseAt[2] = 7;
    TestBases[7].x = 2;
    TestBases[7].y = 0;
    TestBases[7].faction_id_current = 0;
    TestBases[7].visibility = 2;
    TestBases[7].population_size = 6;
    TestBases[7].faction_pop_size_intel[1] = 4;
    map_loc(2, 0)->territory = 2;
    TestSensorPresent = 1;
    source = inspect_spot_all_source(3, 1);
    if (scan_enemy_sensor_contacts(source, 3, 1, 1, 2, 0) != 1 ||
        source.early_radius_tier != 1 ||
        TestBases[7].faction_pop_size_intel[1] != 6)
        return 30 + source.early_radius_tier;

    clear_state();
    add_sensor_priority_goal(2, 3, 4, 5);
    add_sensor_faction_goal(2, 0, 6, 7);
    add_sensor_faction_goal(2, 5, 8, 9);
    add_sensor_facility_goals(3, 10, 11);
    if (TestGoalCount != 6) return 7;
    if (TestGoalArgs[0][1] != AI_GOAL_UNK_1 ||
        TestGoalArgs[0][2] != 3 || TestGoalArgs[1][2] != 1 ||
        TestGoalArgs[2][2] != 3 || TestGoalArgs[3][1] != AI_GOAL_ATTACK ||
        TestGoalArgs[4][1] != AI_GOAL_DEFEND ||
        TestGoalArgs[5][1] != AI_GOAL_SCOUT) return 8;
    return 0;
}

static int check_source_owned_contact_base_continues_unit_history() {
    clear_state();
    Vehs[3].x = 1;
    Vehs[3].y = 1;
    Vehs[3].faction_id = 1;
    Vehs[3].proto_id = 20;
    Vehs[3].prev_veh_id_stack = -1;
    Vehs[3].next_veh_id_stack = -1;
    Vehs[6].x = 2;
    Vehs[6].y = 0;
    Vehs[6].faction_id = 3;
    Vehs[6].proto_id = 30;
    Vehs[6].prev_veh_id_stack = -1;
    Vehs[6].next_veh_id_stack = -1;
    TestBaseAt[2] = 0;
    TestBases[0].x = 2;
    TestBases[0].y = 0;
    TestBases[0].faction_id_current = 1;
    TestVehicleAt[2] = 6;
    map_loc(2, 0)->territory = 2;
    TestSensorPresent = 1;

    SpotAllSourceContext source = inspect_spot_all_source(3, 1);
    int encounter_mask = 0;
    if (scan_enemy_sensor_contacts(source, 3, 1, 1, 2,
                                   &encounter_mask) != 1) return 1;
    if (TestSpotBaseCount != 0 || TestWantWakeCount != 0) return 2;
    if ((Vehs[6].visibility & 2u) == 0 ||
        (Vehs[6].flags & (VFLAG_LURKER | VFLAG_INVISIBLE)) != 0) return 3;
    if (TestGoalCount != 5 || TestSpotLocationCount < 2 ||
        TestSpotLocationX[0] != 1 || TestSpotLocationY[0] != 1) return 4;
    if (encounter_mask != (1 << 3)) return 5;
    return 0;
}

static int check_base_owner_encounter_mask_predicates() {
    clear_state();
    SpotAllSourceContext source;
    source.faction_id = 1;
    source.prototype_id = 20;
    source.vehicle_triad = 0;
    source.special_terrain_mode = 0;
    source.source_low_terrain = 0;
    Base base;
    base.faction_id_current = 3;
    int encounter_mask = 0;
    PlayersData[1].diplo_treaties[3] = 8u;
    record_foreign_base_encounter_fact(source, base, &encounter_mask);
    if (encounter_mask != (1 << 3)) return 1;

    encounter_mask = 0;
    source.source_low_terrain = 1;
    record_foreign_base_encounter_fact(source, base, &encounter_mask);
    if (encounter_mask != 0) return 2;
    PlayersData[1].diplo_treaties[3] = 0;
    record_foreign_base_encounter_fact(source, base, &encounter_mask);
    if (encounter_mask != (1 << 3)) return 3;

    encounter_mask = 0;
    source.vehicle_triad = 2;
    TestVehPrototypes[20].chassis_id = 0;
    TestRulesChassisTable[0].triad = 2;
    TestRulesChassisTable[0].range = 1;
    record_foreign_base_encounter_fact(source, base, &encounter_mask);
    if (encounter_mask != 0) return 4;
    TestRulesChassisTable[0].range = 0;
    record_foreign_base_encounter_fact(source, base, &encounter_mask);
    if (encounter_mask != (1 << 3)) return 5;
    return 0;
}

static int check_expanded_contacts_do_not_publish_event_or_encounter_bits() {
    clear_state();
    TestMapWidth = 5;
    TestMapHeight = 5;
    MapLongitudeBounds = 5;
    MapLatitudeBounds = 5;
    Vehs[3].x = 2;
    Vehs[3].y = 2;
    Vehs[3].faction_id = 1;
    Vehs[3].proto_id = 20;
    Vehs[3].prev_veh_id_stack = -1;
    Vehs[3].next_veh_id_stack = -1;
    Vehs[6].x = 4;
    Vehs[6].y = 0;
    Vehs[6].faction_id = 3;
    Vehs[6].proto_id = 0x13;
    Vehs[6].prev_veh_id_stack = -1;
    Vehs[6].next_veh_id_stack = -1;
    TestVehicleAt[4] = 6;
    TestBaseAt[4] = 0;
    TestBases[0].x = 4;
    TestBases[0].y = 0;
    TestBases[0].faction_id_current = 0;
    map_loc(4, 0)->territory = 2;
    TestSensorPresent = 1;
    SpotAllSourceContext source = inspect_spot_all_source(3, 1);
    int encounter_mask = 0;
    if (scan_enemy_sensor_contacts(source, 3, 1, 9, 10,
                                   &encounter_mask) != 1) return 1;
    if (encounter_mask != 0 || source.neutral_event_flag_a != 0 ||
        source.neutral_event_flag_b != 0) return 2;

    clear_state();
    TestMapWidth = 5;
    TestMapHeight = 5;
    MapLongitudeBounds = 5;
    MapLatitudeBounds = 5;
    MapLongitude = 2;
    Vehs[3].x = 2;
    Vehs[3].y = 2;
    Vehs[3].faction_id = 1;
    Vehs[3].proto_id = 20;
    Vehs[3].prev_veh_id_stack = -1;
    Vehs[3].next_veh_id_stack = -1;
    TestAbilityMask[20] = 2;
    TestBaseAt[4] = 0;
    TestBases[0].x = 4;
    TestBases[0].y = 0;
    TestBases[0].faction_id_current = 3;
    TestBases[0].population_size = 6;
    map_loc(4, 0)->territory = 2;
    map_loc(4, 0)->climate = 0x60u;
    TestSensorPresent = 1;
    source = inspect_spot_all_source(3, 1);
    if (scan_enemy_sensor_contacts(source, 3, 1, 9, 10, 0) != 1 ||
        source.early_radius_tier != 3 ||
        TestBases[0].faction_pop_size_intel[1] != 6) return 3;

    clear_state();
    TestMapWidth = 5;
    TestMapHeight = 5;
    MapLongitudeBounds = 5;
    MapLatitudeBounds = 5;
    MapLongitude = 2;
    Vehs[3].x = 2;
    Vehs[3].y = 2;
    Vehs[3].faction_id = 1;
    Vehs[3].proto_id = 20;
    TestAbilityMask[20] = 2;
    TestBaseAt[4] = 0;
    TestBases[0].x = 4;
    TestBases[0].y = 0;
    TestBases[0].faction_id_current = 3;
    TestBases[0].visibility = 2;
    TestBases[0].population_size = 6;
    TestBases[0].faction_pop_size_intel[1] = 4;
    map_loc(4, 0)->territory = 2;
    map_loc(4, 0)->climate = 0x60u;
    PlayersData[1].diplo_treaties[0] = DTREATY_UNK_200;
    TestSensorPresent = 1;
    source = inspect_spot_all_source(3, 1);
    if (scan_enemy_sensor_contacts(source, 3, 1, 9, 10, 0) != 1 ||
        source.early_radius_tier != 2 ||
        TestBases[0].faction_pop_size_intel[1] != 6 ||
        (map_loc(4, 0)->visibility & 2) == 0 ||
        (map_loc(4, 0)->bit2 & 0x400000u) == 0) return 4;
    return 0;
}

static int check_event_arguments_are_not_encounter_mask_membership() {
    clear_state();
    TestMapWidth = 5;
    TestMapHeight = 5;
    MapLongitudeBounds = 5;
    MapLatitudeBounds = 5;
    Vehs[3].x = 2;
    Vehs[3].y = 2;
    Vehs[3].faction_id = 1;
    Vehs[3].proto_id = 20;
    Vehs[3].prev_veh_id_stack = -1;
    Vehs[3].next_veh_id_stack = -1;
    Vehs[6].x = 3;
    Vehs[6].y = 1;
    Vehs[6].faction_id = 3;
    Vehs[6].proto_id = 0x13;
    Vehs[6].prev_veh_id_stack = -1;
    Vehs[6].next_veh_id_stack = -1;
    TestVehicleAt[8] = 6;
    map_loc(3, 1)->territory = 2;
    TestSensorPresent = 1;
    SpotAllSourceContext source = inspect_spot_all_source(3, 1);
    int encounter_mask = 0;
    if (scan_enemy_sensor_contacts(source, 3, 1, 1, 2,
                                   &encounter_mask) != 1) return 1;
    if (encounter_mask != (1 << 3) || source.neutral_event_flag_a != 1 ||
        source.neutral_event_flag_b != 0) return 2;

    clear_state();
    TestMapWidth = 5;
    TestMapHeight = 5;
    MapLongitudeBounds = 5;
    MapLatitudeBounds = 5;
    Vehs[3].x = 2;
    Vehs[3].y = 2;
    Vehs[3].faction_id = 1;
    Vehs[3].proto_id = 20;
    Vehs[3].prev_veh_id_stack = -1;
    Vehs[3].next_veh_id_stack = -1;
    Vehs[6].x = 3;
    Vehs[6].y = 1;
    Vehs[6].faction_id = 3;
    Vehs[6].proto_id = 0x40;
    Vehs[6].prev_veh_id_stack = -1;
    Vehs[6].next_veh_id_stack = -1;
    TestVehicleAt[8] = 6;
    map_loc(3, 1)->territory = 2;
    TestSensorPresent = 1;
    source = inspect_spot_all_source(3, 1);
    encounter_mask = 0;
    if (scan_enemy_sensor_contacts(source, 3, 1, 1, 2,
                                   &encounter_mask) != 1) return 3;
    if (encounter_mask != (1 << 3) || source.neutral_event_flag_a != 0 ||
        source.neutral_event_flag_b != 0) return 4;
    return 0;
}

static int check_expanded_source_plan_goal_tuple_without_contact_stack() {
    clear_state();
    TestMapWidth = 7;
    TestMapHeight = 5;
    MapLongitudeBounds = 7;
    MapLatitudeBounds = 5;
    MapLongitude = 3;
    Vehs[3].x = 2;
    Vehs[3].y = 2;
    Vehs[3].faction_id = 1;
    Vehs[3].proto_id = 20;
    Vehs[3].prev_veh_id_stack = -1;
    Vehs[3].next_veh_id_stack = -1;
    TestVehPrototypes[20].plan = 0x0B;
    map_loc(4, 0)->territory = 2;
    TestSensorPresent = 1;

    SpotAllSourceContext source = inspect_spot_all_source(3, 1);
    if (scan_enemy_sensor_contacts(source, 3, 1, 9, 10, 0) != 1)
        return 1;
    if (TestGoalCount != 5) return 2;
    if (TestGoalArgs[0][0] != 2 ||
        TestGoalArgs[0][1] != AI_GOAL_ATTACK ||
        TestGoalArgs[1][0] != 2 ||
        TestGoalArgs[1][1] != AI_GOAL_DEFEND) return 3;
    if (TestGoalArgs[2][0] != 2 || TestGoalArgs[2][1] != AI_GOAL_ATTACK ||
        TestGoalArgs[2][2] != 2 || TestGoalArgs[2][3] != 2 ||
        TestGoalArgs[2][4] != 2 || TestGoalArgs[3][0] != 2 ||
        TestGoalArgs[3][1] != AI_GOAL_DEFEND ||
        TestGoalArgs[4][1] != AI_GOAL_SCOUT) return 4;

    clear_state();
    TestMapWidth = 7;
    TestMapHeight = 5;
    MapLongitudeBounds = 7;
    MapLatitudeBounds = 5;
    MapLongitude = 3;
    Vehs[3].x = 2;
    Vehs[3].y = 2;
    Vehs[3].faction_id = 1;
    Vehs[3].proto_id = 20;
    Vehs[3].prev_veh_id_stack = -1;
    Vehs[3].next_veh_id_stack = -1;
    TestVehPrototypes[20].plan = 0x0B;
    PlayersData[1].diplo_treaties[2] = 1u;
    map_loc(4, 0)->territory = 2;
    TestSensorPresent = 1;
    source = inspect_spot_all_source(3, 1);
    scan_enemy_sensor_contacts(source, 3, 1, 9, 10, 0);
    if (TestGoalCount != 0) return 5;

    clear_state();
    TestMapWidth = 7;
    TestMapHeight = 5;
    MapLongitudeBounds = 7;
    MapLatitudeBounds = 5;
    MapLongitude = 3;
    Vehs[3].x = 2;
    Vehs[3].y = 2;
    Vehs[3].faction_id = 1;
    Vehs[3].proto_id = 20;
    Vehs[3].prev_veh_id_stack = -1;
    Vehs[3].next_veh_id_stack = -1;
    map_loc(4, 0)->territory = 2;
    TestSensorPresent = 1;
    source = inspect_spot_all_source(3, 1);
    scan_enemy_sensor_contacts(source, 3, 1, 9, 10, 0);
    if (TestGoalCount != 2 || TestGoalArgs[0][1] != AI_GOAL_ATTACK ||
        TestGoalArgs[1][1] != AI_GOAL_DEFEND) return 6;
    return 0;
}

static int check_stack_history_skip_uses_existing_stack_owner() {
    clear_state();
    SpotAllSourceContext source;
    source.faction_id = 1;
    source.source_controls_sensor_radius = 0;
    Vehs[6].proto_id = 30;
    Vehs[6].x = 0;
    Vehs[6].y = 1;
    PlayersData[1].diplo_treaties[2] = 8u;
    TestAbilityMask[30] = 0x40;
    TestStackCheckResult = 1;
    if (!should_skip_contact_history_for_target_stack(source, 6, 2) ||
        TestStackCheckCount != 1) return 1;
    source.source_controls_sensor_radius = 1;
    if (should_skip_contact_history_for_target_stack(source, 6, 2) ||
        TestStackCheckCount != 1) return 2;
    source.source_controls_sensor_radius = 0;
    PlayersData[1].diplo_treaties[2] = 0;
    if (should_skip_contact_history_for_target_stack(source, 6, 2)) return 3;
    PlayersData[1].diplo_treaties[2] = 8u;
    TestStackCheckResult = 0;
    if (should_skip_contact_history_for_target_stack(source, 6, 2) ||
        TestStackCheckCount != 3) return 4;
    TestAbilityMask[30] = 0;
    map_loc(0, 1)->bit = 0x20u;
    map_loc(0, 1)->climate = 0x60u;
    if (should_skip_contact_history_for_target_stack(source, 6, 2) ||
        TestStackCheckCount != 4) return 5;
    return 0;
}

static int check_sensor_reveal_does_not_spot_source_base() {
    clear_state();
    Vehs[3].x = 1;
    Vehs[3].y = 1;
    Vehs[3].faction_id = 1;
    Vehs[3].proto_id = 20;
    Vehs[3].prev_veh_id_stack = -1;
    Vehs[3].next_veh_id_stack = -1;
    TestBaseAt[1 * TestMapWidth + 1] = 0;
    TestBases[0].x = 1;
    TestBases[0].y = 1;
    map_loc(3, 1)->territory = 2;
    TestSensorPresent = 1;
    SpotAllSourceContext source = inspect_spot_all_source(3, 1);
    int encounter_mask = 0;
    scan_enemy_sensor_contacts(source, 3, 1, 2, 3, &encounter_mask);
    if (TestSpotBaseCount != 0 || TestSpotLocationCount != 1 ||
        TestSpotLocationX[0] != 1 || TestSpotLocationY[0] != 1 ||
        TestSpotLocationFaction[0] != 2) return 1;
    if ((Vehs[3].visibility & 4u) == 0) return 2;
    return 0;
}

static int check_source_stack_owner_refresh_paths() {
    clear_state();
    Vehs[3].x = 1;
    Vehs[3].y = 1;
    Vehs[3].faction_id = 1;
    Vehs[3].proto_id = 20;
    Vehs[3].prev_veh_id_stack = -1;
    Vehs[3].next_veh_id_stack = 4;
    Vehs[4].x = 2;
    Vehs[4].y = 1;
    Vehs[4].faction_id = 2;
    Vehs[4].proto_id = 21;
    Vehs[4].prev_veh_id_stack = 3;
    Vehs[4].next_veh_id_stack = -1;
    Vehs[3].flags = VFLAG_LURKER;
    Vehs[4].flags = VFLAG_INVISIBLE;
    SpotAllSourceContext source = inspect_spot_all_source(3, 1);
    refresh_non_source_stack_owners(source, 3);
    if (TestSpotLocationCount != 0)
        return 1;
    if ((Vehs[3].visibility & 4u) != 0 ||
        (Vehs[4].visibility & 4u) != 0 ||
        (Vehs[3].flags & VFLAG_LURKER) == 0 ||
        (Vehs[4].flags & VFLAG_INVISIBLE) == 0) return 2;

    clear_state();
    Vehs[3].x = 1;
    Vehs[3].y = 1;
    Vehs[3].faction_id = 1;
    Vehs[3].prev_veh_id_stack = -1;
    Vehs[3].next_veh_id_stack = 4;
    Vehs[3].flags = VFLAG_LURKER;
    Vehs[4].x = 2;
    Vehs[4].y = 1;
    Vehs[4].faction_id = 2;
    Vehs[4].prev_veh_id_stack = 3;
    Vehs[4].next_veh_id_stack = 5;
    Vehs[4].flags = VFLAG_INVISIBLE;
    Vehs[5].x = 3;
    Vehs[5].y = 1;
    Vehs[5].faction_id = 3;
    Vehs[5].prev_veh_id_stack = 4;
    Vehs[5].next_veh_id_stack = -1;
    Vehs[5].flags = VFLAG_LURKER | VFLAG_INVISIBLE;
    source = inspect_spot_all_source(5, 1);
    refresh_non_source_stack_owners(source, 5);
    if (TestSpotLocationCount != 1 || TestSpotLocationX[0] != 1 ||
        TestSpotLocationY[0] != 1 || TestSpotLocationFaction[0] != 1)
        return 4;
    if ((Vehs[3].visibility & 0x02u) == 0 ||
        (Vehs[4].visibility & 0x02u) == 0 ||
        (Vehs[5].visibility & 0x02u) == 0 ||
        (Vehs[3].flags & (VFLAG_LURKER | VFLAG_INVISIBLE)) != 0 ||
        (Vehs[4].flags & VFLAG_INVISIBLE) != 0 ||
        (Vehs[5].flags & (VFLAG_LURKER | VFLAG_INVISIBLE)) != 0)
        return 5;

    clear_state();
    Vehs[3].x = 1;
    Vehs[3].y = 1;
    Vehs[3].faction_id = 0;
    Vehs[3].proto_id = 20;
    Vehs[3].visibility = 0xFFu;
    map_loc(1, 1)->visibility = 0x04u;
    spot_all_effect_candidate(3, 1);
    if (Vehs[3].visibility != 0x04u) return 3;
    return 0;
}

static int check_contact_base_owner_source_reveal() {
    clear_state();
    Vehs[3].x = 1;
    Vehs[3].y = 1;
    Vehs[3].faction_id = 1;
    Vehs[3].proto_id = 20;
    Vehs[3].prev_veh_id_stack = -1;
    Vehs[3].next_veh_id_stack = -1;
    SpotAllSourceContext source = inspect_spot_all_source(3, 1);
    reveal_source_to_contact_vehicle_owner(source, 3, 0);
    if (TestSpotLocationCount != 1 || TestSpotLocationFaction[0] != 0 ||
        (Vehs[3].visibility & 1u) == 0) return 1;

    clear_state();
    Vehs[3].x = 1;
    Vehs[3].y = 1;
    Vehs[3].faction_id = 1;
    Vehs[3].proto_id = 20;
    source = inspect_spot_all_source(3, 1);
    source.special_terrain_mode = 1;
    PlayersData[1].diplo_treaties[0] = 8u;
    reveal_source_to_contact_vehicle_owner(source, 3, 0);
    if (TestSpotLocationCount != 1 || TestSpotLocationFaction[0] != 0 ||
        (Vehs[3].visibility & 1u) == 0) return 2;
    source.x = -1;
    reveal_source_to_contact_vehicle_owner(source, 3, 0);
    if (TestSpotLocationCount != 1 || (Vehs[3].visibility & 1u) == 0)
        return 3;
    return 0;
}

static int check_perimeter_hidden_neighbor_thresholds() {
    clear_state();
    SpotAllSourceContext source;
    source.source_controls_sensor_radius = 0;
    Map &tile = *map_loc(1, 1);
    tile.climate = 0x60u;
    if (perimeter_hidden_neighbor_limit(source, tile) != 6) return 1;
    tile.climate = 0x40u;
    if (perimeter_hidden_neighbor_limit(source, tile) != 4) return 2;
    tile.climate = 0x60u;
    source.source_controls_sensor_radius = 1;
    if (perimeter_hidden_neighbor_limit(source, tile) != 4) return 3;
    return 0;
}

static int check_full_candidate_pass_order_and_encounter_publication() {
    clear_state();
    // Put all nine first-ring cells inside the fixture map so the source's
    // strict `count > 8` interlude gate can be exercised without counting
    // out-of-map radius entries.
    TestMapWidth = 5;
    TestMapHeight = 5;
    MapLongitudeBounds = 5;
    MapLatitudeBounds = 5;
    MapLongitude = 2;
    Vehs[3].x = 2;
    Vehs[3].y = 2;
    Vehs[3].faction_id = 1;
    Vehs[3].proto_id = 20;
    Vehs[3].prev_veh_id_stack = -1;
    Vehs[3].next_veh_id_stack = -1;
    map_loc(2, 2)->territory = 2;
    TestSensorPresent = 1;
    TurnCurrentNum = 1;
    MapWinTable[0] = 0;
    Players[1].rule_flags = 0x80u;
    for (int i = 0; i != 25; ++i) {
        TestMap[i].bit2 = 0x2000u;
        TestMap[i].visibility = 2;
        TestMap[i].bit_visible[0] = 4;
    }
    spot_all_effect_candidate(3, 1);

    // Optical locations precede the visibility-radius writes; selected stack
    // reveal precedes the enemy sensor's second reveal and final encounter.
    if (TestSpotLocationCount != 1 || TestEncounterCount != 0) return 1;
    if (TestSpotLocationFaction[0] != 2) return 2;
    if (TestEncounterCount != 0) return 3;
    if (TestGoalCount != 2 || TestGoalArgs[0][0] != 2 ||
        TestGoalArgs[0][1] != AI_GOAL_ATTACK ||
        TestGoalArgs[1][1] != AI_GOAL_DEFEND) return 4;
    if (TestRadiusDrawCount != 1) return 26;
    if (TestRadiusDrawArgs[0][0] != 2 || TestRadiusDrawArgs[0][1] != 2)
        return 27;
    if (TestRadiusDrawArgs[0][2] != 2 || TestRadiusDrawArgs[0][3] != 2)
        return 28;
    if (TestWorldGraphicRedrawCount != 1 || TestWorldOnRedrawCount != 0 ||
        TestMainMapRedrawCount != 0) return 29;
    if (TestInterludeCount != 1 || TestInterludeArgs[0][0] != 0x29 ||
        TestInterludeArgs[0][1] != 1 || TestInterludeArgs[0][2] != 0)
        return 30;

    clear_state();
    Vehs[3].x = 1;
    Vehs[3].y = 1;
    Vehs[3].faction_id = 1;
    Vehs[3].proto_id = 20;
    Vehs[3].prev_veh_id_stack = -1;
    Vehs[3].next_veh_id_stack = -1;
    TestAbilityMask[20] = 2;
    TurnCurrentNum = 1;
    for (int i = 0; i != 16; ++i) TestMap[i].visibility = 2;
    spot_all_effect_candidate(3, 1);
    if (TestRadiusDrawCount != 1 || TestRadiusDrawArgs[0][2] != 3)
        return 7;

    clear_state();
    Vehs[3].x = 1;
    Vehs[3].y = 1;
    Vehs[3].faction_id = 1;
    Vehs[3].proto_id = 20;
    Vehs[3].prev_veh_id_stack = -1;
    Vehs[3].next_veh_id_stack = -1;
    map_loc(1, 1)->territory = 2;
    TestSensorPresent = 1;
    TestMultiplayer = 0;
    spot_all_effect_candidate(3, 0);
    if (TestEncounterCount != 0) return 51;
    if (TestGoalCount != 2) return 52;

    clear_state();
    TestMapWidth = 5;
    TestMapHeight = 5;
    MapLongitudeBounds = 5;
    MapLatitudeBounds = 5;
    MapLongitude = 2;
    Vehs[3].x = 2;
    Vehs[3].y = 2;
    Vehs[3].faction_id = 1;
    Vehs[3].proto_id = 20;
    Vehs[3].prev_veh_id_stack = -1;
    Vehs[3].next_veh_id_stack = -1;
    map_loc(3, 1)->territory = 2;
    TestBaseAt[8] = 7;
    TestBases[7].faction_id_current = 0;
    TestBases[7].x = 3;
    TestBases[7].y = 1;
    TestSensorPresent = 1;
    TurnCurrentNum = 1;
    for (int i = 0; i != 25; ++i) TestMap[i].bit2 = 0x2000u;
    Players[1].rule_flags = 0x80u;
    spot_all_effect_candidate(3, 1);
    if (TestEncounterCount != 1 || TestEncounterFaction[0] != 8)
        return 53;
    if (TestDrawOrder == 0) return 54;
    if (TestInterludeOrder <= TestDrawOrder) return 55;
    if (TestEncounterOrder <= TestInterludeOrder) return 56;

    clear_state();
    TestMapWidth = 5;
    TestMapHeight = 5;
    MapLongitudeBounds = 5;
    MapLatitudeBounds = 5;
    MapLongitude = 2;
    Vehs[3].x = 2;
    Vehs[3].y = 2;
    Vehs[3].faction_id = 1;
    Vehs[3].proto_id = 20;
    Vehs[3].prev_veh_id_stack = -1;
    Vehs[3].next_veh_id_stack = -1;
    map_loc(3, 1)->territory = 2;
    TestBaseAt[8] = 7;
    TestBases[7].faction_id_current = 0;
    TestBases[7].x = 3;
    TestBases[7].y = 1;
    TestSensorPresent = 1;
    for (int i = 0; i != 25; ++i) TestMap[i].bit2 = 0x2000u;
    spot_all_effect_candidate(3, 1);
    if (TestRadiusDrawCount != 0 || TestInterludeCount != 0 ||
        TestEncounterCount != 0) return 57;
    return 0;
}

static int check_seed_neutral_event_prototype_classification() {
    clear_state();
    Vehs[3].x = 1;
    Vehs[3].y = 1;
    Vehs[3].faction_id = 1;
    Vehs[3].proto_id = 20;
    Vehs[3].prev_veh_id_stack = -1;
    Vehs[3].next_veh_id_stack = -1;
    Vehs[6].faction_id = 3;
    Vehs[6].proto_id = 0x13;
    Vehs[6].x = 2;
    Vehs[6].y = 0;
    Vehs[6].prev_veh_id_stack = -1;
    Vehs[6].next_veh_id_stack = -1;
    TestVehicleAt[2] = 6;
    map_loc(2, 0)->territory = 2;
    TestSensorPresent = 1;
    SpotAllSourceContext source = inspect_spot_all_source(3, 1);
    scan_enemy_sensor_contacts(source, 3, 1, 1, 2, 0);
    if (TestSpotLocationCount == 0) return 3;
    if (source.neutral_event_flag_a != 1) return 4;
    if (source.neutral_event_flag_b != 0) return 5;
    PlayersData[1].diplo_treaties[2] = 8;
    map_loc(2, 0)->climate = 0x60u;
    source.neutral_event_flag_a = 0;
    source.neutral_event_flag_b = 0;
    scan_enemy_sensor_contacts(source, 3, 1, 1, 2, 0);
    if (source.neutral_event_flag_a != 0 ||
        source.neutral_event_flag_b != 0) return 7;

    clear_state();
    Vehs[3].x = 1;
    Vehs[3].y = 1;
    Vehs[3].faction_id = 1;
    Vehs[3].proto_id = 20;
    Vehs[3].prev_veh_id_stack = -1;
    Vehs[3].next_veh_id_stack = -1;
    Vehs[6].faction_id = 3;
    Vehs[6].proto_id = 30;
    Vehs[6].prev_veh_id_stack = -1;
    Vehs[6].next_veh_id_stack = -1;
    TestVehPrototypes[30].weapon_id = 4;
    TestRulesWeaponTable[4].offense_rating = -1;
    TestVehicleAt[2] = 6;
    Vehs[6].x = 2;
    Vehs[6].y = 0;
    map_loc(2, 0)->territory = 2;
    map_loc(1, 1)->climate = 0x60u;
    TestSensorPresent = 1;
    source = inspect_spot_all_source(3, 1);
    scan_enemy_sensor_contacts(source, 3, 1, 1, 2, 0);
    if (source.neutral_event_flag_a != 0 ||
        source.neutral_event_flag_b != 1) return 2;
    if (source.early_radius_tier != 1) return 6;

    clear_state();
    Vehs[3].x = 1;
    Vehs[3].y = 1;
    Vehs[3].faction_id = 1;
    Vehs[3].proto_id = 20;
    map_loc(1, 1)->climate = 0x60u;
    PlayersData[1].diplo_treaties[2] = 8u;
    source = inspect_spot_all_source(3, 1);
    if (!collect_neutral_event_classification(source, 2, 7,
                                               0x13, 1) ||
        source.neutral_event_flag_a != 1) return 8;
    source.neutral_event_flag_a = 0;
    if (collect_neutral_event_classification(source, 2, -1,
                                              0x13, 1) ||
        source.neutral_event_flag_a != 0) return 9;
    return 0;
}

static int run_unseen_unit_tier_fixture(int detected_proto, uint16_t flags,
                                        unsigned char climate,
                                        uint32_t tile_bit,
                                        uint8_t source_visibility,
                                        uint8_t detected_visibility) {
    clear_state();
    TestMapWidth = 5;
    TestMapHeight = 5;
    MapLongitudeBounds = 5;
    MapLatitudeBounds = 5;
    MapLongitude = 2;
    Vehs[3].x = 2;
    Vehs[3].y = 2;
    Vehs[3].faction_id = 1;
    Vehs[3].proto_id = 20;
    Vehs[3].visibility = source_visibility;
    Vehs[3].prev_veh_id_stack = -1;
    Vehs[3].next_veh_id_stack = -1;
    Vehs[6].faction_id = 3;
    Vehs[6].proto_id = detected_proto;
    Vehs[6].x = 4;
    Vehs[6].y = 0;
    Vehs[6].flags = flags;
    Vehs[6].visibility = detected_visibility;
    Vehs[6].prev_veh_id_stack = -1;
    Vehs[6].next_veh_id_stack = -1;
    TestVehicleAt[4] = 6;
    map_loc(4, 0)->territory = 2;
    map_loc(4, 0)->climate = climate;
    map_loc(4, 0)->bit = tile_bit;
    TestSensorPresent = 1;
    SpotAllSourceContext source = inspect_spot_all_source(3, 1);
    scan_enemy_sensor_contacts(source, 3, 1, 9, 10, 0);
    return source.early_radius_tier;
}

static int check_unseen_unit_radius_tier_filters() {
    if (run_unseen_unit_tier_fixture(30, 0, 0, 0, 0, 0) != 2) return 1;
    if (run_unseen_unit_tier_fixture(30, VFLAG_INVISIBLE, 0, 0, 0, 0) != 0)
        return 2;
    if (run_unseen_unit_tier_fixture(30, 0, 0x60u, 0x20u, 0, 0) != 0)
        return 3;
    if (run_unseen_unit_tier_fixture(0x13, 0, 0x60u, 0x20u, 0, 0) != 2)
        return 4;
    if (run_unseen_unit_tier_fixture(30, 0, 0, 0, 2, 0) != 0)
        return 5;
    if (run_unseen_unit_tier_fixture(30, 0, 0, 0, 0, 2) != 2)
        return 6;
    return 0;
}

static int check_selected_sensor_target_cleanup() {
    clear_state();
    TestMapWidth = 5;
    TestMapHeight = 5;
    MapLongitudeBounds = 5;
    MapLatitudeBounds = 5;
    Vehs[3].x = 2;
    Vehs[3].y = 2;
    Vehs[3].faction_id = 1;
    Vehs[3].proto_id = 20;
    Vehs[3].prev_veh_id_stack = -1;
    Vehs[3].next_veh_id_stack = -1;
    Vehs[6].x = 4;
    Vehs[6].y = 0;
    Vehs[6].faction_id = 3;
    Vehs[6].proto_id = 30;
    Vehs[6].prev_veh_id_stack = -1;
    Vehs[6].next_veh_id_stack = -1;
    Vehs[6].state = 0x40000u;
    TestVehicleAt[4] = 6;
    map_loc(4, 0)->territory = 2;
    PlayersData[2].x_target = 4;
    PlayersData[2].y_target = 0;
    TestSensorPresent = 1;
    SpotAllSourceContext source = inspect_spot_all_source(3, 1);
    scan_enemy_sensor_contacts(source, 3, 1, 9, 10, 0);
    if (PlayersData[2].x_target != -1 || PlayersData[2].y_target != -1)
        return 3;
    if ((Vehs[6].state & 0x60000u) != 0x60000u) return 4;

    clear_state();
    Vehs[3].x = 2;
    Vehs[3].y = 2;
    Vehs[3].faction_id = 1;
    Vehs[3].proto_id = 20;
    TestRulesChassisTable[0].triad = 1;
    map_loc(4, 0)->territory = 2;
    PlayersData[2].x_target = 4;
    PlayersData[2].y_target = 0;
    source = inspect_spot_all_source(3, 1);
    clear_matching_sensor_target(source, 2, 4, 0);
    if (PlayersData[2].x_target != 4 || PlayersData[2].y_target != 0)
        return 2;

    clear_state();
    Vehs[3].x = 2;
    Vehs[3].y = 2;
    Vehs[3].faction_id = 1;
    Vehs[3].proto_id = 20;
    TestRulesChassisTable[0].triad = 0;
    TestVehPrototypes[20].plan = 1;
    PlayersData[2].x_target = 4;
    PlayersData[2].y_target = 0;
    source = inspect_spot_all_source(3, 1);
    clear_matching_sensor_target(source, 2, 4, 0);
    if (PlayersData[2].x_target != -1 || PlayersData[2].y_target != -1)
        return 5;

    clear_state();
    Vehs[3].x = 2;
    Vehs[3].y = 2;
    Vehs[3].faction_id = 1;
    Vehs[3].proto_id = 20;
    TestRulesChassisTable[0].triad = 0;
    map_loc(2, 2)->region = 1;
    PlayersData[2].x_target = 4;
    PlayersData[2].y_target = 0;
    source = inspect_spot_all_source(3, 1);
    clear_matching_sensor_target(source, 2, 4, 0);
    if (PlayersData[2].x_target != 4 || PlayersData[2].y_target != 0)
        return 6;
    return 0;
}

static int check_outer_scan_radius_tier_mapping() {
    int tier = 0;
    retain_outer_scan_radius_tier(tier, 0, 1, 1);
    if (tier != 1) return 1;
    retain_outer_scan_radius_tier(tier, 8, 0, 0);
    if (tier != 1) return 2;
    retain_outer_scan_radius_tier(tier, 9, 1, 1);
    if (tier != 2) return 3;
    retain_outer_scan_radius_tier(tier, 24, 0, 0);
    if (tier != 2) return 4;
    retain_outer_scan_radius_tier(tier, 25, 1, 1);
    if (tier != 3) return 5;
    retain_outer_scan_radius_tier(tier, 48, 0, 0);
    if (tier != 3) return 6;
    retain_outer_scan_radius_tier(tier, 8, 1, 0);
    retain_outer_scan_radius_tier(tier, 25, 0, 1);
    if (tier != 3) return 7;
    return 0;
}

static int check_known_inner_radius_tier_effect() {
    clear_state();
    Vehs[3].x = 1;
    Vehs[3].y = 1;
    Vehs[3].faction_id = 1;
    Vehs[3].proto_id = 20;
    for (int i = 0; i != TestMapWidth * TestMapHeight; ++i)
        TestMap[i].bit2 = 0x200000u;
    SpotAllSourceContext source = inspect_spot_all_source(3, 1);
    if (source.early_radius_tier != 0) return 1;
    scan_optical_location_radius(source);
    if (source.early_radius_tier != 1) return 2;

    clear_state();
    Vehs[3].x = 1;
    Vehs[3].y = 1;
    Vehs[3].faction_id = 1;
    Vehs[3].proto_id = 20;
    TestUseSensorGrid = 1;
    for (int i = 0; i != TestMapWidth * TestMapHeight; ++i)
        TestMap[i].bit2 = 0x200000u;
    source = inspect_spot_all_source(3, 1);
    const int source_sensor_checks = TestSensorChecks;
    scan_optical_location_radius(source);
    if (TestSensorChecks <= source_sensor_checks ||
        source.early_radius_tier != 1) return 3;
    return 0;
}

static int check_nested_sensor_base_facility_probe() {
    clear_state();
    Vehs[3].x = 1;
    Vehs[3].y = 1;
    Vehs[3].faction_id = 1;
    Vehs[3].proto_id = 20;
    TestUseSensorGrid = 1;
    TestSensorAt[2 * TestMapWidth + 0] = 1;
    map_loc(0, 2)->territory = 2;
    TestBaseAt[1 * TestMapWidth + 1] = 0;
    TestBases[0].x = 1;
    TestBases[0].y = 1;
    TestBases[0].faction_id_current = 2;
    TestBases[0].facilities_built[5] = 0x02; // bitmask(0x29)

    SpotAllSourceContext source = inspect_spot_all_source(3, 1);
    if (!find_inner_sensor_match(source, 1, 1)) return 1;
    TestBases[0].facilities_built[5] = 0;
    if (find_inner_sensor_match(source, 1, 1)) return 2;
    TestBases[0].facilities_built[5] = 0x02;
    map_loc(0, 2)->territory = 1;
    if (!find_inner_sensor_match(source, 1, 1)) return 3;

    clear_state();
    Vehs[3].x = 1;
    Vehs[3].y = 1;
    Vehs[3].faction_id = 1;
    Vehs[3].proto_id = 20;
    TestSensorPresent = 1;
    TestBaseAt[1 * TestMapWidth + 2] = 0;
    TestBases[0].x = 2;
    TestBases[0].y = 1;
    TestBases[0].facilities_built[5] = 0x02;
    source = inspect_spot_all_source(3, 1);
    if (find_inner_sensor_match(source, 2, 1)) return 7;

    clear_state();
    Vehs[3].x = 1;
    Vehs[3].y = 1;
    Vehs[3].faction_id = 1;
    Vehs[3].proto_id = 20;
    Vehs[4].faction_id = 1;
    Vehs[4].proto_id = 21;
    Vehs[4].prev_veh_id_stack = -1;
    Vehs[4].next_veh_id_stack = -1;
    TestAbilityMask[21] = 2;
    TestVehPrototypes[21].chassis_id = 1;
    TestRulesChassisTable[1].triad = 1;
    PlayersData[2].diplo_treaties[1] = 1;
    TestVehicleAt[2 * TestMapWidth + 0] = 4;
    map_loc(0, 2)->bit = 2;
    map_loc(0, 2)->val2 = 2;
    source = inspect_spot_all_source(3, 1);
    if (!find_inner_sensor_match(source, 1, 1)) return 5;
    PlayersData[2].diplo_treaties[1] = 2;
    if (find_inner_sensor_match(source, 1, 1)) return 9;
    PlayersData[2].diplo_treaties[1] = 1;
    TestRulesChassisTable[1].triad = 0;
    if (find_inner_sensor_match(source, 1, 1)) return 6;

    // The native triad comparison uses the outer scan center's terrain,
    // which can differ from the selected source tile's terrain.
    clear_state();
    Vehs[3].x = 1;
    Vehs[3].y = 1;
    Vehs[3].faction_id = 1;
    Vehs[3].proto_id = 20;
    Vehs[4].faction_id = 1;
    Vehs[4].proto_id = 21;
    Vehs[4].prev_veh_id_stack = -1;
    Vehs[4].next_veh_id_stack = -1;
    TestAbilityMask[21] = 2;
    TestVehPrototypes[21].chassis_id = 1;
    TestRulesChassisTable[1].triad = 1;
    TestVehicleAt[2 * TestMapWidth] = 4;
    map_loc(0, 1)->climate = 0x60u;
    source = inspect_spot_all_source(3, 1);
    if (has_matching_faction_sensor_vehicle(source, 0, 2, 0, 1))
        return 8;
    if (!has_matching_faction_sensor_vehicle(source, 0, 2, 1, 1))
        return 8;

    clear_state();
    Vehs[3].x = 1;
    Vehs[3].y = 1;
    Vehs[3].faction_id = 1;
    Vehs[3].proto_id = 20;
    TestUseSensorGrid = 1;
    TestSensorAt[2 * TestMapWidth + 0] = 1;
    map_loc(0, 2)->territory = 2;
    map_loc(1, 1)->bit2 = 0x200000u;
    TestBaseAt[1 * TestMapWidth + 1] = 0;
    TestBases[0].x = 1;
    TestBases[0].y = 1;
    TestBases[0].facilities_built[5] = 0x02;
    source = inspect_spot_all_source(3, 1);
    scan_optical_location_radius(source);
    if (source.early_radius_tier != 1) return 4;
    return 0;
}

static int check_perimeter_map_and_base_visibility_write() {
    clear_state();
    SpotAllSourceContext source;
    source.faction_id = 1;
    source.faction_mask = 2;
    TestBaseAt[1 * TestMapWidth + 1] = 0;
    TestBases[0].x = 2;
    TestBases[0].y = 1;
    TestBases[0].population_size = 5;
    map_loc(1, 1)->bit = 0x1234u;
    map_loc(2, 1)->bit = 0x2345u;
    set_perimeter_tile_visibility(source, 1, 1);
    if ((map_loc(1, 1)->visibility & 2) == 0 ||
        map_loc(1, 1)->bit_visible[0] != 0x1234u ||
        (TestBases[0].visibility & 2) == 0 ||
        TestBases[0].faction_pop_size_intel[1] != 5 ||
        (map_loc(1, 1)->bit2 & 0x400000u) == 0 ||
        (map_loc(2, 1)->visibility & 2) == 0 ||
        map_loc(2, 1)->bit_visible[0] != 0x2345u ||
        (map_loc(2, 1)->bit2 & 0x400000u) == 0 ||
        TestMapSyncCount != 1) return 1;

    const uint32_t remembered_flags = map_loc(1, 1)->bit2;
    set_perimeter_tile_visibility(source, 1, 1);
    if (map_loc(1, 1)->bit2 != remembered_flags || TestMapSyncCount != 2)
        return 2;
    return 0;
}

static int run_first_body_plan_goal_case(int source_plan, int contact_plan,
                                         unsigned int treaty,
                                         int territory_owner) {
    clear_state();
    TestMapWidth = 5;
    TestMapHeight = 5;
    MapLongitudeBounds = 5;
    MapLatitudeBounds = 5;
    Vehs[3].x = 1;
    Vehs[3].y = 1;
    Vehs[3].faction_id = 1;
    Vehs[3].proto_id = 20;
    Vehs[3].prev_veh_id_stack = -1;
    Vehs[3].next_veh_id_stack = -1;
    Vehs[6].x = 2;
    Vehs[6].y = 0;
    Vehs[6].faction_id = 2;
    Vehs[6].proto_id = 30;
    Vehs[6].prev_veh_id_stack = -1;
    Vehs[6].next_veh_id_stack = -1;
    TestVehPrototypes[20].plan = (unsigned char)source_plan;
    TestVehPrototypes[30].plan = (unsigned char)contact_plan;
    TestVehicleAt[2] = 6;
    TestSensorPresent = 1;
    TestTerritoryOwner = territory_owner;
    map_loc(2, 0)->territory = 2;
    PlayersData[1].diplo_treaties[2] = treaty;
    SpotAllSourceContext source = inspect_spot_all_source(3, 1);
    scan_enemy_sensor_contacts(source, 3, 0, 1, 2, 0);
    return TestGoalCount;
}

static int check_first_body_plan_ge8_goal_arm() {
    if (run_first_body_plan_goal_case(8, 8, 2u | 4u, 2) != 3) return 1;
    if (TestGoalArgs[0][0] != 2 || TestGoalArgs[0][1] != AI_GOAL_DEFEND ||
        TestGoalArgs[0][2] != 2 || TestGoalArgs[0][3] != 2 ||
        TestGoalArgs[0][4] != 0 || TestGoalArgs[0][5] != -1) return 2;
    if (TestGoalArgs[1][0] != 2 || TestGoalArgs[1][1] != AI_GOAL_ATTACK ||
        TestGoalArgs[1][2] != 1 || TestGoalArgs[2][1] != AI_GOAL_SCOUT ||
        TestGoalArgs[2][2] != 1) return 3;
    if (run_first_body_plan_goal_case(7, 8, 0, 2) != 1) return 4;
    if (TestGoalArgs[0][0] != 1 || TestGoalArgs[0][1] != AI_GOAL_ATTACK ||
        TestGoalArgs[0][2] != 2) return 9;
    if (run_first_body_plan_goal_case(8, 7, 0, 2) != 1) return 5;
    if (run_first_body_plan_goal_case(8, 8, 1u, 2) != 0)
        return 6;
    if (run_first_body_plan_goal_case(8, 8, 0, 3) != 1) return 7;
    if (run_first_body_plan_goal_case(8, 8, 0, -1) != 1) return 8;
    return 0;
}

static int check_spot_all_map_dirty_owner_flag() {
    clear_state();
    SpotAllSourceContext source;
    source.faction_id = 1;
    ConsoleGlobal.faction_id_ = 1;
    PlayersData[1].flags = PFLAG_MAP_REVEALED;
    if (may_mark_spot_all_map_dirty(source) != 0) return 1;
    PlayersData[1].flags = PFLAG_UNK_80;
    if (may_mark_spot_all_map_dirty(source) != 1) return 2;
    return 0;
}

int main() {
    const int a = check_stack_visibility_and_owner_exceptions();
    const int b = check_owner_tile_visibility_and_discovery_guards();
    const int c = check_effect_order_and_wake_inputs();
    const int d = check_encounter_notification_order_and_gates();
    const int e = check_source_policy_branches();
    const int f = check_visibility_scan_geometry_and_gate();
    const int g = check_sensor_contact_goals_and_stack_order();
    const int h = check_full_candidate_pass_order_and_encounter_publication();
    const int i = check_outer_scan_radius_tier_mapping();
    const int j = check_known_inner_radius_tier_effect();
    const int k = check_nested_sensor_base_facility_probe();
    const int l = check_perimeter_map_and_base_visibility_write();
    const int m = check_seed_neutral_event_prototype_classification();
    const int n = check_unseen_unit_radius_tier_filters();
    const int o = check_selected_sensor_target_cleanup();
    const int p = check_sensor_reveal_does_not_spot_source_base();
    const int q = check_perimeter_hidden_neighbor_thresholds();
    const int r = check_source_stack_owner_refresh_paths();
    const int s = check_contact_base_owner_source_reveal();
    const int t = check_stack_history_skip_uses_existing_stack_owner();
    const int u = check_visibility_scan_source_triad_terrain_filter();
    const int v = check_source_owned_contact_base_continues_unit_history();
    const int w = check_base_owner_encounter_mask_predicates();
    const int x = check_expanded_contacts_do_not_publish_event_or_encounter_bits();
    const int y = check_event_arguments_are_not_encounter_mask_membership();
    const int z = check_expanded_source_plan_goal_tuple_without_contact_stack();
    const int aa = check_first_body_plan_ge8_goal_arm();
    const int ab = check_spot_all_map_dirty_owner_flag();
    if (a) return 10 + a;
    if (b) return 20 + b;
    if (c) return 30 + c;
    if (d) return 40 + d;
    if (e) return 50 + e;
    if (f) return 60 + f;
    if (g) return 70 + g;
    if (h) return 80 + h;
    if (i) return 90 + i;
    if (j) return 100 + j;
    if (k) return 110 + k;
    if (l) return 120 + l;
    if (m) return 130 + m;
    if (n) return 140 + n;
    if (o) return 150 + o;
    if (p) return 160 + p;
    if (q) return 170 + q;
    if (r) return 180 + r;
    if (s) return 190 + s;
    if (t) return 200 + t;
    if (u) return 210 + u;
    if (v) return 220 + v;
    if (w) return 230 + w;
    if (x) return 240 + x;
    if (y) return 250 + y;
    if (z) return 260 + z;
    if (aa) return 270 + aa;
    if (ab) return 280 + ab;
    return 0;
}
