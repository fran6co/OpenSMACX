#include <assert.h>
#include <string.h>
#include "enemy_pb_host_fixture.h"

Veh Vehs[32];
Base Bases[32];
PlayerData PlayersData[8];
VehPrototype VehPrototypes[32];
RulesBasic Rules;
GameDataStateWindow GameDataStateStorage = {};
uint32_t &LockEnableMask = GameDataStateStorage.fields_.lock_masks_.enable_mask_;
uint32_t &GameRules = GameDataStateStorage.fields_.game_rules_;
int &BaseCurrentCount = GameDataStateStorage.fields_.base_current_count_;
int &VehCurrentCount = GameDataStateStorage.fields_.vehicle_current_count_;
int &TurnCurrentNum = GameDataStateStorage.fields_.turn_current_num_;
int (&CouncilResolutionCount)[11] = GameDataStateStorage.fields_.council_resolution_count_;
int MapIsFlat, MapLongitude, MapLongitudeBounds, MapLatitudeBounds;
int RadiusOffsetX[16], RadiusOffsetY[16], VehMoveDeltaX[8], VehMoveDeltaY[8];
int RadiusRange[4];
int mock_at_climax_calls, mock_lift_calls, mock_drop_calls;
int mock_drop_x, mock_drop_y, mock_stack_value;
int mock_veh_at[16][16];
int mock_base_at[16][16];
Map MockMap[16][16];
int SecretProject[MaxSecretProjectNum];

// PRODUCT_BODY: enemy_planet_buster

static void reset_fixture() {
    memset(Vehs, 0, sizeof(Vehs));
    memset(Bases, 0, sizeof(Bases));
    memset(PlayersData, 0, sizeof(PlayersData));
    memset(VehPrototypes, 0, sizeof(VehPrototypes));
    memset(CouncilResolutionCount, 0, sizeof(CouncilResolutionCount));
    memset(SecretProject, 0xff, sizeof(SecretProject));
    memset(mock_veh_at, 0xff, sizeof(mock_veh_at));
    memset(mock_base_at, 0xff, sizeof(mock_base_at));
    memset(MockMap, 0, sizeof(MockMap));
    memset(RadiusOffsetX, 0, sizeof(RadiusOffsetX));
    memset(RadiusOffsetY, 0, sizeof(RadiusOffsetY));
    memset(VehMoveDeltaX, 0, sizeof(VehMoveDeltaX));
    memset(VehMoveDeltaY, 0, sizeof(VehMoveDeltaY));
    memset(RadiusRange, 0, sizeof(RadiusRange));
    LockEnableMask = 0;
    GameRules = 0;
    BaseCurrentCount = VehCurrentCount = TurnCurrentNum = 0;
    MapIsFlat = 1;
    MapLongitude = 16;
    MapLongitudeBounds = MapLatitudeBounds = 16;
    Rules.move_rate_roads = 1;
    mock_at_climax_calls = mock_lift_calls = mock_drop_calls = 0;
    mock_drop_x = mock_drop_y = -1;
    mock_stack_value = 0;
}

static void setup_vehicle() {
    Vehs[0].faction_id = 0;
    Vehs[0].proto_id = 0;
    Vehs[0].x = 1;
    Vehs[0].y = 1;
    Vehs[0].visibility = 0x55;
    PlayersData[0].ranking = 1;
    PlayersData[0].ai_fight = 0;
    VehCurrentCount = 1;
    VehPrototypes[0].cost = 100;
}

static void test_lazy_climax_query() {
    reset_fixture(); setup_vehicle();
    BaseCurrentCount = 1;
    Bases[0].faction_id_current = 1;
    Bases[0].x = 2; Bases[0].y = 1;
    PlayersData[0].diplo_treaties[1] = 0x02; // low bit, no qualifying C20 bit
    enemy_planet_buster(0);
    assert(mock_at_climax_calls == 0);

    reset_fixture(); setup_vehicle();
    BaseCurrentCount = 1;
    Bases[0].faction_id_current = 1;
    Bases[0].x = 2; Bases[0].y = 1;
    PlayersData[0].diplo_treaties[1] = 0x22; // low treaty shape + revenge bit
    enemy_planet_buster(0);
    assert(mock_at_climax_calls == 2); // once per pass; false shape rejects before shared gate
}

static void test_last_reachable_launch_base_and_callback_mutation() {
    reset_fixture(); setup_vehicle();
    PlayersData[0].flags = PFLAG_COMMIT_ATROCIT_WANTONLY;
    BaseCurrentCount = 3;
    for (int i = 0; i < BaseCurrentCount; ++i) {
        Bases[i].faction_id_current = 0;
        Bases[i].faction_id_former = 0;
        Bases[i].x = static_cast<short>(2 + i);
        Bases[i].y = 1;
    }
    RadiusRange[0] = 1;
    Vehs[1].faction_id = 1;
    VehCurrentCount = 2;
    PlayersData[1].ranking = 5;
    mock_stack_value = 1;
    mock_veh_at[1][2] = 1; // positive enemy-stack pressure for base zero
    const int result = enemy_planet_buster(0);
    assert(result == -1);
    assert(mock_lift_calls == 1 && mock_drop_calls == 1);
    assert(mock_drop_x == Bases[2].x && mock_drop_y == Bases[2].y);
    assert(Vehs[0].x == Bases[2].x && Vehs[0].y == Bases[2].y);
    assert(Vehs[0].visibility == 0); // body clears visibility after mutating drop callback
    assert(mock_at_climax_calls == 12); // two common-label call sites per admitted base/pass
}

static void test_no_winning_target_falls_back_to_launch_slot_zero() {
    reset_fixture(); setup_vehicle();
    PlayersData[0].flags = PFLAG_COMMIT_ATROCIT_WANTONLY;
    BaseCurrentCount = 1;
    Bases[0].faction_id_current = 0;
    Bases[0].faction_id_former = 0;
    Bases[0].x = 4; Bases[0].y = 4;
    RadiusRange[0] = 1;
    mock_veh_at[4][4] = -1; // center-without-vehicle rejects the only candidate
    assert(enemy_planet_buster(0) == -1);
    assert(mock_lift_calls == 1 && mock_drop_calls == 1);
    assert(mock_drop_x == Bases[0].x && mock_drop_y == Bases[0].y);
}

static void test_pass0_adjacent_reposition_callback_order() {
    reset_fixture(); setup_vehicle();
    PlayersData[0].flags = PFLAG_COMMIT_ATROCIT_WANTONLY;
    BaseCurrentCount = 1;
    Bases[0].faction_id_current = 1;
    Bases[0].x = 2; Bases[0].y = 1;
    PlayersData[0].diplo_treaties[1] = 0;
    PlayersData[0].ranking = 7;
    VehPrototypes[0].cost = 1;
    RadiusRange[0] = 1;
    VehMoveDeltaX[0] = 1; VehMoveDeltaY[0] = 0;
    Vehs[1].faction_id = 1;
    VehCurrentCount = 2;
    PlayersData[1].ranking = 1;
    mock_stack_value = 20;
    mock_veh_at[1][2] = 1;
    MockMap[1][3].bit = 0;
    const int result = enemy_planet_buster(0);
    assert(result == 4); // direction 0 XOR 4
    assert(mock_lift_calls == 1 && mock_drop_calls == 1);
    assert(mock_drop_x == 3 && mock_drop_y == 1);
    assert(Vehs[0].visibility == 0);
}

static void test_protected_city_exits_radius_scan() {
    reset_fixture(); setup_vehicle();
    PlayersData[0].flags = PFLAG_COMMIT_ATROCIT_WANTONLY;
    BaseCurrentCount = 2;
    Bases[0].faction_id_current = 1;
    Bases[0].x = 2; Bases[0].y = 1;
    Bases[1].faction_id_current = 0;
    Bases[1].x = 2; Bases[1].y = 1;
    mock_base_at[1][2] = 1; // protected friendly city at the candidate center
    mock_veh_at[1][2] = 1; // occupied center isolates the protected-city break
    RadiusRange[0] = 2;
    RadiusOffsetX[0] = 0; RadiusOffsetY[0] = 0;
    RadiusOffsetX[1] = 1; RadiusOffsetY[1] = 0;
    Vehs[1].faction_id = 1;
    VehCurrentCount = 2;
    PlayersData[1].ranking = 7;
    mock_stack_value = 3000; // later pressure would overcome -9999 without loop exit
    mock_veh_at[1][3] = 1;
    assert(enemy_planet_buster(0) == -1);
    assert(mock_lift_calls == 0 && mock_drop_calls == 0);
}

static void test_empty_center_exits_radius_scan() {
    reset_fixture(); setup_vehicle();
    PlayersData[0].flags = PFLAG_COMMIT_ATROCIT_WANTONLY;
    BaseCurrentCount = 2;
    Bases[0].faction_id_current = 1;
    Bases[0].x = 2; Bases[0].y = 1;
    Bases[1].faction_id_current = 0;
    Bases[1].x = 2; Bases[1].y = 1;
    RadiusRange[0] = 2;
    RadiusOffsetX[0] = 0; RadiusOffsetY[0] = 0;
    RadiusOffsetX[1] = 1; RadiusOffsetY[1] = 0;
    Vehs[1].faction_id = 1;
    VehCurrentCount = 2;
    PlayersData[1].ranking = 7;
    mock_stack_value = 3000;
    mock_veh_at[1][3] = 1; // later offset would dominate if scan continued
    assert(enemy_planet_buster(0) == -1);
    assert(mock_lift_calls == 0 && mock_drop_calls == 0);
}

int main() {
    test_lazy_climax_query();
    test_last_reachable_launch_base_and_callback_mutation();
    test_no_winning_target_falls_back_to_launch_slot_zero();
    test_pass0_adjacent_reposition_callback_order();
    test_protected_city_exits_radius_scan();
    test_empty_center_exits_radius_scan();
    return 0;
}
