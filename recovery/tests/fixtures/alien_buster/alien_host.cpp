#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "alien_host_fixture.h"

Veh Vehs[128];
Base Bases[128];
Map TestMap[64][64];
PlayerDataHost PlayersData[128];
VehPrototype VehPrototypes[128];
RulesChassis Chassis[32];
RulesWeapon Weapon[32];
RulesBasicHost Rules;
ConsoleHost ConsoleGlobal;
NetMsgHost NetMsgPrimary;
PathHost Paths;
int MapLongitudeBounds,MapLatitudeBounds,MapLongitude,MapIsFlat;
int BaseFindDist,IsMultiplayerNet;
GameDataStateWindow GameDataStateStorage = {};
int &TurnCurrentNum = GameDataStateStorage.fields_.turn_current_num_;
int &DiffLevelCurrent = GameDataStateStorage.fields_.diff_level_current_;
uint32_t &LockEnableMask = GameDataStateStorage.fields_.lock_masks_.enable_mask_;
uint32_t &GameRules = GameDataStateStorage.fields_.game_rules_;
int &VehCurrentCount = GameDataStateStorage.fields_.vehicle_current_count_;
int &BaseCurrentCount = GameDataStateStorage.fields_.base_current_count_;
int VehMoveDeltaX[9]={1,2,1,0,-1,-2,-1,0,0};
int VehMoveDeltaY[9]={-1,0,1,2,1,0,-1,-2,0};
int RadiusOffsetX[289],RadiusOffsetY[289];
int RadiusBaseX[9]={1,2,1,0,-1,-2,-1,0,0};
int RadiusBaseY[9]={-1,0,1,2,1,0,-1,-2,0};
int events[64],event_count;
int test_zoc_move_args[3],test_zoc_veh_args[3];
int test_zoc_veh_override,test_zoc_veh_override_x,test_zoc_veh_override_y;
int test_base_find_count,test_base_find_args[2][6];
int test_base_find_dist_values[2];
int test_territory_args[5];
int test_alien_base,test_zoc,test_cargo,test_stack_check,test_can_arty;
int test_path_move,test_veh_at,test_stack_value,test_morale,test_rand_value;
int test_stack_call_count,test_stack_args[16][5];
int test_garrison_base_calls,test_defensive_base_calls;
int test_garrison_required,test_defensive_required;
const char *test_parse_says_text;
int test_parse_says_calls;

// PRODUCT_BODY: alien_move

static void reset() {
    memset(Vehs,0,sizeof(Vehs)); memset(Bases,0,sizeof(Bases));
    memset(TestMap,0,sizeof(TestMap)); memset(PlayersData,0,sizeof(PlayersData));
    memset(VehPrototypes,0,sizeof(VehPrototypes)); memset(Chassis,0,sizeof(Chassis));
    memset(Weapon,0,sizeof(Weapon)); memset(RadiusOffsetX,0,sizeof(RadiusOffsetX));
    memset(RadiusOffsetY,0,sizeof(RadiusOffsetY)); memset(events,0,sizeof(events));
    memset(test_stack_args,0,sizeof(test_stack_args));
    MapLongitudeBounds=MapLatitudeBounds=64; MapLongitude=32; MapIsFlat=1;
    BaseFindDist=0; TurnCurrentNum=0x40; DiffLevelCurrent=2; IsMultiplayerNet=0;
    LockEnableMask=GameRules=0; VehCurrentCount=BaseCurrentCount=1;
    test_alien_base=0; test_zoc=1; test_cargo=0; test_stack_check=0;
    test_can_arty=0; test_path_move=1; test_veh_at=-1; test_stack_value=0;
    test_morale=0; test_rand_value=0; event_count=0; ConsoleGlobal.faction_id_=0;
    test_stack_call_count=0; test_garrison_base_calls=0;
    test_base_find_count=0; memset(test_base_find_args,0,sizeof(test_base_find_args));
    test_base_find_dist_values[0]=70; test_base_find_dist_values[1]=71;
    test_defensive_base_calls=0; test_garrison_required=1;
    test_defensive_required=1;
    test_parse_says_text=0; test_parse_says_calls=0;
    test_add_goal_calls=0; memset(test_add_goal_args,0,sizeof(test_add_goal_args));
    test_zoc_move_result=41; test_zoc_veh_result=42;
    test_zoc_veh_override=0; test_zoc_veh_override_x=test_zoc_veh_override_y=-1;
    test_zoc_sea_result=0; test_zoc_sea_calls=0;
    Rules.move_rate_roads=3; Rules.artillery_max_rng=2;
    Vehs[0].x=10; Vehs[0].y=10; Vehs[0].faction_id=0; Vehs[0].proto_id=1;
    Vehs[0].moves_expended=0; Vehs[0].order=0; Vehs[0].terraforming_turns=0;
    Vehs[0].next_veh_id_stack=-1; Vehs[0].prev_veh_id_stack=-1;
    VehPrototypes[1].chassis_id=1; Chassis[1].triad=0;
    Bases[0].x=12; Bases[0].y=10; Bases[0].faction_id_current=0;
    TestMap[10][5].region=3; TestMap[10][6].region=3;
}

static void test_gates_and_no_base_kill() {
    reset(); Vehs[0].flags=0x400;
    assert(alien_move(0)==0 && event_count==1 && events[0]==1);
    reset(); test_alien_base=-1; test_zoc=0;
    assert(alien_move(0)==1 && event_count==1 && events[0]==2);
}

static void test_bug_for_bug_no_base_zoc_storage_alias() {
    reset();
    test_alien_base=-1; test_zoc=1; test_cargo=1; test_stack_check=1;
    // Native 16-bit x/y reads alias the low/high halves of this live field.
    PlayersData[7].sites[14].base_id=(10 << 16) | 12;
    TestMap[10][5].region=5;
    TestMap[10][6].region=6;
    Map &safe=TestMap[9][5];
    safe.region=6; safe.climate=0x60; safe.bit=0; safe.val2=0;
    assert(alien_move(0)==0);
    assert(BaseFindDist==99);
    assert(event_count>=4);
    assert(events[0]==1 && events[1]==11 && events[2]==6 && events[3]==10);

    reset();
    test_alien_base=-1; test_zoc=1; test_cargo=1; test_stack_check=1;
    PlayersData[7].sites[14].base_id=(10 << 16) | 14;
    TestMap[10][5].region=5;
    TestMap[10][7].region=7;
    TestMap[9][5].region=7;
    TestMap[9][5].climate=0x60;
    assert(alien_move(0)==0);
    assert(BaseFindDist==99 && event_count>=4);
    assert(events[1]==11 && events[2]==6 && events[3]==10);
}

static void test_safe_cargo_tile_callback_order() {
    reset(); test_cargo=1; test_stack_check=1;
    strcpy(Bases[0].name_string,"<T>");
    Map &tile=TestMap[9][5]; tile.bit=0; tile.climate=0x60; tile.region=3;
    assert(alien_move(0)==0);
    assert(event_count>=5);
    assert(events[0]==1); // movement skipped before local notifications
    assert(events[1]==11 && events[2]==6 && events[3]==9 && events[4]==10);
    assert(test_parse_says_calls==1);
    assert(test_parse_says_text==Bases[0].name_string);
}

static void test_path_owner_and_artillery_action() {
    reset(); PlayersData[0].current_num_bases=5;
    assert(alien_move(0)==0);
    assert(events[0]==13); // Paths::move selected after movement scoring

    reset(); test_can_arty=1; RadiusOffsetX[1]=2; RadiusOffsetY[1]=0;
    Map &target=TestMap[10][6]; target.climate=0x60; target.bit=0x80e88054u; target.val2=0;
    const int result=alien_move(0);
    if (result!=1) { fprintf(stderr,"arty result=%d events=%d order=%d\n",result,event_count,Vehs[0].order); }
    assert(result==1);
    int action_index=-1, sync_count=0, draw_index=-1;
    for(int i=0;i<event_count;++i) {
        if(events[i]==4) action_index=i;
        if(events[i]==5) ++sync_count;
        if(events[i]==6) draw_index=i;
    }
    assert(action_index>=0 && sync_count==7 && draw_index>action_index);
}

static void test_retreat_cleanup_and_selected_direction() {
    reset(); test_morale=0;
    assert(alien_move(0)==0);
    assert(Vehs[0].order==0 || Vehs[0].order==0x58);

    reset(); test_morale=1;
    TestMap[9][5].climate=0x60; TestMap[9][5].bit=0;
    assert(alien_move(0)==0);
    assert(Vehs[0].order==0x58);
    assert(Vehs[0].waypoint_x[0]==11 && Vehs[0].waypoint_y[0]==9);
}

int main() {
    test_gates_and_no_base_kill();
    test_bug_for_bug_no_base_zoc_storage_alias();
    test_safe_cargo_tile_callback_order();
    test_path_owner_and_artillery_action();
    test_retreat_cleanup_and_selected_direction();
}
