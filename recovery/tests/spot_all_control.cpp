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
int TestSpotTileOrder;
int TestContactRevealOrder;
int TestWantWakeOrder;
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
int TestAbilityCallCount;
int TestAbilityCallId[64];
TestVehPrototype TestVehPrototypes[512];
TestRulesChassis TestRulesChassisTable[16];
TestRulesWeapon TestRulesWeaponTable[26];
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
int TestWorldGraphicRedrawOrder;
int TestWorldOnRedrawCount;
int TestSensorPresent;
int mutate_source_faction_on_wake;

struct TestLockMasks { uint32_t enable_mask_; };
struct TestGameFields {
    uint32_t game_more_preferences_;
    uint32_t game_state_;
    int turn_current_num_;
    TestLockMasks lock_masks_;
};
struct TestGameDataState { TestGameFields fields_; };
TestGameDataState GameDataStateStorage;

const int TestRadiusOffsetX[289] = {
     0,  1,  2,  1,  0, -1, -2, -1,  0,  2,  2, -2, -2,  1,  3,  3,
     1, -1, -3, -3, -1,  4, -4,  0,  0,
     3, // Test-only perimeter witness; not evidence for production radius data.
     // Distinct off-map sentinels keep fixture padding 26..48 from aliasing
     // the source or another valid tile in wide-radius scans.
     100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111,
     112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122
};
const int TestRadiusOffsetY[289] = {
     0, -1,  0,  1,  2,  1,  0, -1, -2, -2,  2,  2, -2, -2, -3, -1,
     3,  3,  1, -1, -3,  0,  0,  4, -4,
     3,
     100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100,
     100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100
};
const int TestVehMoveDeltaX[9] = { 1, 2, 1, 0, -1, -2, -1, 0, 0 };
const int TestVehMoveDeltaY[9] = { -1, 0, 1, 2, 1, 0, -1, -2, 0 };

static int WakeOrder;
static int SyncOrder;

int veh_top(int vehicle_id) {
    if (vehicle_id < 0) return -1;
    int top = vehicle_id;
    for (int16_t previous = Vehs[top].prev_veh_id_stack; previous >= 0;
         previous = Vehs[previous].prev_veh_id_stack)
        top = previous;
    return top;
}

int base_at(int x, int y) {
    if (!on_map(x, y)) return -1;
    return TestBaseAt[y * TestMapWidth + x];
}

int base_find(int x, int y) { return base_at(x, y); }

int veh_at(int x, int y) {
    if (!on_map(x, y)) return -1;
    return TestVehicleAt[y * TestMapWidth + x];
}

int stack_fix(int vehicle_id) { return vehicle_id < 0 ? -1 : veh_top(vehicle_id); }

int stack_check(int, int type, int, int, int) {
    ++TestStackCheckCount;
    return type == 1 ? TestStackCheckResult : 0;
}

BOOL on_map(int x, int y) {
    return x >= 0 && x < TestMapWidth && y >= 0 && y < TestMapHeight;
}

Map *map_loc(int x, int y) { return &TestMap[y * TestMapWidth + x]; }

void synch_bit(int, int, int) {
    ++TestMapSyncCount;
    SyncOrder = ++TestEventOrder;
}

void wake_stack(int vehicle_id) {
    ++TestWakeCount;
    TestWakeFaction = -1;
    TestWakeSource = vehicle_id;
    const int order = ++TestEventOrder;
    if (TestWakeCount == 1) WakeOrder = order;
}

BOOL want_to_wake(int faction_id, int vehicle_id, int spotted_vehicle_id) {
    const int index = TestWantWakeCount++;
    const int order = ++TestEventOrder;
    if (index == 0) TestWantWakeOrder = order;
    TestWakeFaction = faction_id;
    TestWakeSource = spotted_vehicle_id;
    TestWantWakeArgs[index][0] = faction_id;
    TestWantWakeArgs[index][1] = vehicle_id;
    TestWantWakeArgs[index][2] = spotted_vehicle_id;
    if (mutate_source_faction_on_wake) Vehs[0].faction_id = 7;
    return 1;
}

void encounter(int source_faction_id, int target_faction_id, int x, int y,
               int event_flag_a, int event_flag_b) {
    TestEncounterOrder = ++TestEventOrder;
    const int index = TestEncounterCount++;
    TestEncounterFaction[index] = source_faction_id * 8 + target_faction_id;
    TestEncounterArgs[index][0] = source_faction_id;
    TestEncounterArgs[index][1] = target_faction_id;
    TestEncounterArgs[index][2] = x;
    TestEncounterArgs[index][3] = y;
    TestEncounterArgs[index][4] = event_flag_a;
    TestEncounterArgs[index][5] = event_flag_b;
}

BOOL has_abil(int prototype_id, int ability_id) {
    TestAbilityCallId[TestAbilityCallCount++] = ability_id;
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

void bitmask(int value, int *offset, int *mask) {
    *offset = value / 8;
    *mask = 1 << (value & 7);
}

int get_proto_triad(int prototype_id) {
    return TestRulesChassisTable[
        TestVehPrototypes[prototype_id].chassis_id].triad;
}

void spot_tile(int x, int y, int faction_id) {
    if (!on_map(x, y)) return;
    if (x == 3 && y == 1 && faction_id == 1)
        TestContactRevealOrder = ++TestEventOrder;
    else
        ++TestEventOrder;
    ++TestSpotLocationCount;
    const unsigned int faction_mask = 1u << faction_id;
    Map *tile = map_loc(x, y);
    if (faction_id == ConsoleGlobal.faction_id_ &&
        (tile->visibility & faction_mask) == 0 &&
        (GameDataStateStorage.fields_.game_state_ & STATE_OMNISCIENT_VIEW) == 0 &&
        (PlayersData[faction_id].flags & PFLAG_MAP_REVEALED) == 0) {
        tile->bit2 |= 0x400000u;
        UnkBitfield1 |= 1u;
    }
    tile->visibility |= (uint8_t)faction_mask;
    synch_bit(x, y, faction_id);
}

void spot_base(int base_id, int faction_id) {
    ++TestSpotBaseCount;
    TestSpotBaseArgs[TestSpotBaseCount - 1][0] = base_id;
    TestSpotBaseArgs[TestSpotBaseCount - 1][1] = faction_id;
    Base &base = TestBases[base_id];
    const uint8_t mask = (uint8_t)(1u << faction_id);
    base.visibility |= mask;
    base.faction_pop_size_intel[faction_id] = (uint8_t)base.population_size;
    if (on_map(base.x, base.y)) {
        map_loc(base.x, base.y)->visibility |= mask;
        synch_bit(base.x, base.y, faction_id);
    }
}

void MapWin::on_redraw() { ++TestMainMapRedrawCount; }
void GraphicWin::redraw() {
    ++TestWorldGraphicRedrawCount;
    TestWorldGraphicRedrawOrder = ++TestEventOrder;
}
void WorldWin::on_redraw() { ++TestWorldOnRedrawCount; }

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
    for (int i = 0; i < 32; ++i) {
        Vehs[i] = Veh();
        Vehs[i].prev_veh_id_stack = -1;
        Vehs[i].next_veh_id_stack = -1;
        TestMap[i] = Map();
        TestBaseAt[i] = -1;
        TestVehicleAt[i] = -1;
        TestSensorAt[i] = 0;
    }
    for (int faction = 0; faction < 8; ++faction) {
        PlayersData[faction] = PlayerData();
        Players[faction] = Player();
    }
    for (int proto = 0; proto < 512; ++proto) {
        TestAbilityMask[proto] = 0;
        TestVehPrototypes[proto] = TestVehPrototype();
    }
    TestAbilityCallCount = 0;
    for (int chassis = 0; chassis < 16; ++chassis)
        TestRulesChassisTable[chassis] = TestRulesChassis();
    for (int weapon = 0; weapon < 26; ++weapon)
        TestRulesWeaponTable[weapon] = TestRulesWeapon();
    for (int base = 0; base < 8; ++base) TestBases[base] = Base();
    ConsoleGlobal.faction_id_ = 1;
    GameState = 0;
    TurnCurrentNum = 0;
    LockEnableMask = 0;
    UnkBitfield1 = 0;
    TestMultiplayer = 1;
    TestVisibilityScanAllowed = 0;
    MapLongitudeBounds = TestMapWidth = 4;
    MapLatitudeBounds = TestMapHeight = 4;
    MapLongitude = 2;
    MapIsFlat = 1;
    GameDataStateStorage.fields_.game_more_preferences_ = 0;
    GameDataStateStorage.fields_.game_state_ = 0;
    GameDataStateStorage.fields_.turn_current_num_ = 0;
    GameDataStateStorage.fields_.lock_masks_.enable_mask_ = 0;
    TestMapSyncCount = TestWakeCount = 0;
    TestWakeFaction = TestWakeSource = -1;
    TestWantWakeCount = 0;
    TestEventOrder = TestDrawOrder = TestSpotTileOrder = 0;
    TestContactRevealOrder = TestWantWakeOrder = 0;
    TestInterludeOrder = TestEncounterOrder = 0;
    TestEncounterCount = TestGoalCount = 0;
    TestSensorChecks = TestTerritoryChecks = 0;
    TestUseSensorGrid = TestSensorPresent = 0;
    TestTerritoryOwner = -1;
    TestStackCheckCount = TestStackCheckResult = 0;
    TestSpotLocationCount = TestSpotBaseCount = 0;
    TestObservedVehicleId = -1;
    TestObservedVehicleStateAtBaseReveal = 0;
    TestRadiusDrawCount = TestMainMapRedrawCount = 0;
    TestInterludeCount = TestWorldGraphicRedrawCount = 0;
    TestWorldGraphicRedrawOrder = TestWorldOnRedrawCount = 0;
    MapWinTable[0] = 0;
    mutate_source_faction_on_wake = 0;
    WakeOrder = SyncOrder = 0;
}

static void enable_surface_presentation() {
    GameDataStateStorage.fields_.turn_current_num_ = 1;
    GameDataStateStorage.fields_.game_more_preferences_ = 1;
    Players[1].rule_flags = 0x80;
    for (int i = 0; i < 32; ++i) TestMap[i].bit2 = 0x2000u;
}

static int check_source_ability_short_circuit() {
    clear_state();
    Vehs[0].faction_id = 1;
    Vehs[0].proto_id = 10;
    Vehs[0].x = 1;
    Vehs[0].y = 1;
    TestAbilityMask[10] = 0x40;
    TestUseSensorGrid = 1;
    spot_all(0, 0);
    if (TestAbilityCallCount != 2 || TestAbilityCallId[0] != 0x40 ||
        TestAbilityCallId[1] != 2) return 1;
    if (TestTerritoryChecks != 0) return 2;
    return 0;
}

static int check_optical_memory_and_outer_perimeter_threshold() {
    clear_state();
    enable_surface_presentation();
    Vehs[0].faction_id = 1;
    Vehs[0].proto_id = 10;
    Vehs[0].x = 0;
    Vehs[0].y = 0;
    TestAbilityMask[10] = 2;
    TestRulesChassisTable[0].triad = 0;
    TestMap[13].visibility = 2;
    TestMap[10].visibility = 2;
    TestMap[7].visibility = 2;

    // Index 25 is a deliberately test-only outer point (3,3). Its three
    // on-map neighbors are visible, so the perimeter admits this reveal.
    spot_all(0, 1);
    if ((TestMap[15].visibility & 2u) == 0) return 1;
    if ((TestMap[15].bit2 & 0x400000u) == 0 || UnkBitfield1 == 0) return 2;
    if (TestRadiusDrawCount != 1 || TestRadiusDrawArgs[0][0] != 0 ||
        TestRadiusDrawArgs[0][1] != 0 || TestRadiusDrawArgs[0][3] != 2 ||
        TestRadiusDrawArgs[0][2] == 0) return 3;
    if (TestWorldGraphicRedrawCount != 1) return 4;
    if (!(TestDrawOrder < TestWorldGraphicRedrawOrder)) return 5;

    clear_state();
    enable_surface_presentation();
    Vehs[0].faction_id = 1;
    Vehs[0].proto_id = 10;
    Vehs[0].x = 0;
    Vehs[0].y = 0;
    TestAbilityMask[10] = 2;
    // The two orthogonal unseen neighbors contribute exactly four hidden
    // units, so the native >= threshold refuses to reveal the outer tile.
    TestMap[13].visibility = 0;
    TestMap[10].visibility = 2;
    TestMap[7].visibility = 0;
    spot_all(0, 1);
    if ((TestMap[15].visibility & 2u) != 0) return 6;
    return 0;
}

static int check_contact_wake_goals_snapshot_and_final_order() {
    clear_state();
    enable_surface_presentation();
    TestMapWidth = MapLongitudeBounds = 8;
    MapLongitude = 4;
    Vehs[0].faction_id = 1;
    Vehs[0].proto_id = 10;
    Vehs[0].x = 1;
    Vehs[0].y = 1;
    TestAbilityMask[10] = 2;
    TestRulesChassisTable[0].triad = 0;
    TestUseSensorGrid = 1;
    mutate_source_faction_on_wake = 1;
    GameDataStateStorage.fields_.lock_masks_.enable_mask_ =
        (1u << 0) | (1u << 2) | (1u << 3);
    PlayersData[1].diplo_treaties[4] = 4u;

    // Deliberately discover factions in 3,4,2 order; final notifications
    // must publish neutral first, then faction ids ascending.
    const int contact_x[3] = {3, 2, 1};
    const int contact_y[3] = {1, 2, 3};
    const int contact_vehicle[3] = {2, 3, 4};
    const int sensor_faction[3] = {3, 4, 2};
    const int contact_faction[3] = {3, 0, 2};
    for (int i = 0; i < 3; ++i) {
        const int tile_index = contact_y[i] * TestMapWidth + contact_x[i];
        TestSensorAt[tile_index] = 1;
        TestMap[tile_index].territory = (int8_t)sensor_faction[i];
        TestVehicleAt[tile_index] = contact_vehicle[i];
        Vehs[contact_vehicle[i]].faction_id = (uint8_t)contact_faction[i];
        Vehs[contact_vehicle[i]].proto_id = (int16_t)(20 + i);
        Vehs[contact_vehicle[i]].x = (int16_t)contact_x[i];
        Vehs[contact_vehicle[i]].y = (int16_t)contact_y[i];
        Vehs[contact_vehicle[i]].state = 0x02004200u;
        Vehs[contact_vehicle[i]].order = 0x18;
        Vehs[contact_vehicle[i]].waypoint_count = 1;
        TestAbilityMask[20 + i] = 0;
    }
    Vehs[3].proto_id = 0x13;
    TestUseSensorGrid = 1;

    spot_all(0, 1);
    if (TestWantWakeCount != 3 || TestWakeCount != 3) return 1;
    if (TestWantWakeArgs[0][0] != 1 || TestWantWakeArgs[0][1] != 2 ||
        TestWantWakeArgs[0][2] != 0) return 2;
    if (TestWantWakeArgs[1][0] != 1 || TestWantWakeArgs[1][1] != 3 ||
        TestWantWakeArgs[1][2] != 0 ||
        TestWantWakeArgs[2][0] != 1 || TestWantWakeArgs[2][1] != 4 ||
        TestWantWakeArgs[2][2] != 0) return 12;
    if (!(WakeOrder < TestWantWakeOrder &&
          TestWantWakeOrder < TestContactRevealOrder)) return 3;
    if (Vehs[2].state & 0x02004200u || Vehs[2].order != 0) return 4;
    if (TestGoalCount < 6 || TestGoalArgs[0][0] != 3 ||
        TestGoalArgs[0][1] != AI_GOAL_ATTACK ||
        TestGoalArgs[0][2] != 2 || TestGoalArgs[0][3] != 3 ||
        TestGoalArgs[0][4] != 1) return 5;
    for (int goal = 0; goal < TestGoalCount; ++goal) {
        if (TestGoalArgs[goal][0] == 4 && TestGoalArgs[goal][3] == 2 &&
            TestGoalArgs[goal][4] == 2) return 11;
    }
    if (TestEncounterCount != 3) return 6;
    if (TestEncounterFaction[0] != 8 || TestEncounterFaction[1] != 10 ||
        TestEncounterFaction[2] != 11) return 7;
    if (TestEncounterArgs[0][4] != 1) return 8;
    if (Vehs[0].faction_id != 7) return 9;
    if (!(TestDrawOrder < TestWorldGraphicRedrawOrder &&
          TestWorldGraphicRedrawOrder < TestInterludeOrder &&
          TestInterludeOrder < TestEncounterOrder)) return 10;
    if (TestInterludeCount != 1 || TestInterludeArgs[0][0] != 0x29 ||
        TestInterludeArgs[0][1] != 1 || TestInterludeArgs[0][2] != 0)
        return 13;
    return 0;
}

static int check_detected_base_history_and_encounter() {
    clear_state();
    GameDataStateStorage.fields_.turn_current_num_ = 1;
    Vehs[0].faction_id = 1;
    Vehs[0].proto_id = 10;
    Vehs[0].x = 1;
    Vehs[0].y = 1;
    TestAbilityMask[10] = 2;
    TestUseSensorGrid = 1;
    const int index = 2; // radius offset 1 is (+1,-1)
    TestSensorAt[index] = 1;
    TestMap[index].territory = 2;
    TestBaseAt[index] = 0;
    TestBases[0].x = 3;
    TestBases[0].y = 0;
    TestBases[0].faction_id_current = 2;
    TestBases[0].population_size = 4;

    spot_all(0, 1);
    if ((TestBases[0].visibility & 2u) == 0 ||
        TestBases[0].faction_pop_size_intel[1] != 4) return 1;
    if ((TestMap[3].visibility & 2u) == 0 ||
        (TestMap[3].bit2 & 0x400000u) == 0)
        return 2;
    if (TestEncounterCount != 1) return 3;
    if (TestEncounterArgs[0][0] != 1 || TestEncounterArgs[0][1] != 2 ||
        TestEncounterArgs[0][2] != 1 || TestEncounterArgs[0][3] != 1 ||
        TestEncounterArgs[0][4] != 0 || TestEncounterArgs[0][5] != 0)
        return 20;
    return 0;
}

static int check_source_wake_policy_callback() {
    clear_state();
    Vehs[0].faction_id = 1;
    Vehs[0].proto_id = 10;
    Vehs[0].x = 1;
    Vehs[0].y = 1;
    Vehs[0].state = 0x02004200u;
    Vehs[0].order = 0x18;
    Vehs[0].waypoint_count = 1;
    TestAbilityMask[10] = 2;
    TestUseSensorGrid = 1;
    GameDataStateStorage.fields_.lock_masks_.enable_mask_ = 1u << 1;
    GameDataStateStorage.fields_.lock_masks_.enable_mask_ |= 1u << 2;
    const int index = 2; // radius offset 1 is (+1,-1)
    TestSensorAt[index] = 1;
    TestMap[index].territory = 2;
    TestVehicleAt[index] = 2;
    Vehs[2].faction_id = 2;
    Vehs[2].proto_id = 20;
    Vehs[2].x = 2;
    Vehs[2].y = 0;
    TestWantWakeCount = 0;

    spot_all(0, 1);
    if (TestWantWakeCount != 2) return 1;
    // First callback considers waking the detected stack; the second is the
    // source-vehicle policy query, enabled specifically by faction 1's lock bit.
    if (TestWantWakeArgs[0][0] != 1 || TestWantWakeArgs[0][1] != 2 ||
        TestWantWakeArgs[0][2] != 0 ||
        TestWantWakeArgs[1][0] != 2 || TestWantWakeArgs[1][1] != 0 ||
        TestWantWakeArgs[1][2] != 2) return 2;
    if ((Vehs[0].state & 0x02000200u) != 0 || Vehs[0].order != 0)
        return 3;
    return 0;
}

static int check_sensor_target_cleanup() {
    clear_state();
    Vehs[0].faction_id = 1;
    Vehs[0].proto_id = 10;
    Vehs[0].x = 1;
    Vehs[0].y = 1;
    TestUseSensorGrid = 1;
    const int index = 3 * TestMapWidth + 3; // radius offset 10 is (+2,+2)
    TestSensorAt[index] = 1;
    TestMap[index].territory = 2;
    TestMap[index].region = 6;
    TestMap[1 * TestMapWidth + 1].region = 6;
    PlayersData[2].x_target = 3;
    PlayersData[2].y_target = 3;
    TestVehicleAt[index] = 2;
    Vehs[2].faction_id = 2;
    Vehs[2].state = 0x40000u;

    spot_all(0, 1);
    if (PlayersData[2].x_target != -1 || PlayersData[2].y_target != -1)
        return 1;
    if ((Vehs[2].state & 0x60000u) != 0x60000u) return 2;
    return 0;
}

static int check_offline_toggle_with_wrapped_longitude() {
    clear_state();
    GameDataStateStorage.fields_.turn_current_num_ = 1;
    TestMultiplayer = 0;
    TestMapWidth = MapLongitudeBounds = 8;
    MapLatitudeBounds = TestMapHeight = 4;
    MapIsFlat = 0;
    MapLongitude = 4;
    Vehs[0].faction_id = 1;
    Vehs[0].proto_id = 10;
    Vehs[0].x = 0;
    Vehs[0].y = 1;
    TestUseSensorGrid = 1;
    const int wrapped_index = 2 * TestMapWidth + 7; // source x=0 + (-1)
    TestSensorAt[wrapped_index] = 1;
    TestMap[wrapped_index].territory = 2;
    TestVehicleAt[wrapped_index] = 2;
    Vehs[2].faction_id = 2;
    Vehs[2].proto_id = 20;
    Vehs[2].x = 7;
    Vehs[2].y = 2;

    spot_all(0, 0); // offline play forces the effective toggle on
    if (TestEncounterCount != 1 || TestEncounterArgs[0][0] != 1 ||
        TestEncounterArgs[0][1] != 2 || TestEncounterArgs[0][2] != 0 ||
        TestEncounterArgs[0][3] != 1) return 1;
    if (TestGoalCount < 2 || TestGoalArgs[0][0] != 2 ||
        TestGoalArgs[0][3] != 7 || TestGoalArgs[0][4] != 2) return 2;
    if ((TestMap[wrapped_index].visibility & 2u) == 0) return 3;
    return 0;
}

int main() {
    const int abilities = check_source_ability_short_circuit();
    if (abilities) return abilities;
    const int optical = check_optical_memory_and_outer_perimeter_threshold();
    if (optical) return 10 + optical;
    const int contacts = check_contact_wake_goals_snapshot_and_final_order();
    if (contacts) return 30 + contacts;
    const int base = check_detected_base_history_and_encounter();
    if (base) return 50 + base;
    const int source_wake = check_source_wake_policy_callback();
    if (source_wake) return 60 + source_wake;
    const int target_cleanup = check_sensor_target_cleanup();
    if (target_cleanup) return 70 + target_cleanup;
    const int offline_wrap = check_offline_toggle_with_wrapped_longitude();
    if (offline_wrap) return 80 + offline_wrap;
    return 0;
}
