#include <cassert>
#include <stdint.h>
#include <cstring>
#define __cdecl

enum { TRIAD_LAND = 0, TRIAD_SEA = 1, TRIAD_AIR = 2, PLAN_ALIEN_ARTIFACT = 12 };
enum { BIT_BASE_IN_TILE = 1 };
struct Veh { int16_t x,y; uint32_t state; uint16_t flags; int16_t proto_id; uint16_t unk_1; uint8_t faction_id,year_end_lurking,dmg_incurred; int8_t order; uint8_t waypoint_count,patrol_current_point; int16_t waypoint_x[4],waypoint_y[4]; uint8_t morale,terraforming_turns,order_auto_type,visibility,moves_expended; int8_t facing; uint8_t unk_6,move_to_ai_type,probe_action,probe_sabotage_id; int16_t home_base_id,next_veh_id_stack,prev_veh_id_stack; };
struct VehPrototype { char veh_name[32]; uint32_t ability_flags; uint8_t chassis_id,weapon_id,armor_id,reactor_id,carry_capacity,cost,plan; int8_t unk_1; uint8_t obsolete_factions; int8_t combat_factions,icon_offset,padding; uint16_t flags; int16_t preq_tech; };
struct RulesChassis { char bytes[0x49]; uint8_t triad,range; };
struct Map { uint8_t climate,contour; uint16_t val2; uint8_t region,visibility,val3,unk_1; uint32_t bit; };
struct GameDataStateWindow { struct Fields { int vehicle_current_count_; } fields_; } GameDataStateStorage;
struct Veh Vehs[8];
struct VehPrototype VehPrototypes[8];
int mutate_proto_on_focus = -1;
void mutate_focus_proto() { if (mutate_proto_on_focus >= 0) Vehs[0].proto_id = static_cast<int16_t>(mutate_proto_on_focus); }
struct Console { Console() : focus_calls(0),last_x(0),last_y(0),last_faction(0),faction_id_(0) {} int focus(int x,int y,int faction) { ++focus_calls; last_x=x; last_y=y; last_faction=faction; mutate_focus_proto(); return 0; } int focus_calls,last_x,last_y,last_faction,faction_id_; };
int &VehCurrentCount=GameDataStateStorage.fields_.vehicle_current_count_; RulesChassis Chassis[8]; Map tile; uint32_t MapIsFlat=1,MapLongitude=4,MapLongitudeBounds=8,MapLatitudeBounds=4; int speed_value=5; Console ConsoleGlobal; int kills=0,booms=0,draws=0,stacks=0;
Map *map_loc(int x,int y) { assert(x>=0&&x<8&&y>=0&&y<4); return &tile; }
int speed(int,bool) { return speed_value; }
int stack_check(int,int,int,int,int) { ++stacks; return 0; }
int boom(int,int,int) { ++booms; return 0; }
void kill(int) { ++kills; }
void draw_tile(int,int,int) { ++draws; }
#include "actual.inc"

static void reset() {
    std::memset(Vehs,0,sizeof(Vehs));
    std::memset(VehPrototypes,0,sizeof(VehPrototypes));
    std::memset(Chassis,0,sizeof(Chassis));
    std::memset(&tile,0,sizeof(tile));
    VehCurrentCount=1; ConsoleGlobal=Console();
    kills=booms=draws=stacks=0; speed_value=5; mutate_proto_on_focus=-1;
}
int main() {
    reset();
    Vehs[0].x=2; Vehs[0].y=1; Vehs[0].proto_id=0; Vehs[0].faction_id=0;
    VehPrototypes[0].chassis_id=0; VehPrototypes[0].reactor_id=2;
    Chassis[0].triad=TRIAD_AIR; Chassis[0].range=1;
    Vehs[0].moves_expended=5; Vehs[0].terraforming_turns=0;
    mash_planes();
    assert(stacks==0 && ConsoleGlobal.focus_calls==0 && Vehs[0].dmg_incurred==0);

    reset();
    Vehs[0].x=2; Vehs[0].y=1; Vehs[0].proto_id=0; Vehs[0].faction_id=0;
    Vehs[0].moves_expended=1; Vehs[0].terraforming_turns=77;
    VehPrototypes[0].chassis_id=0; VehPrototypes[0].reactor_id=2;
    Chassis[0].triad=TRIAD_AIR; Chassis[0].range=1;
    VehPrototypes[1].chassis_id=6; VehPrototypes[1].plan=PLAN_ALIEN_ARTIFACT;
    VehPrototypes[1].weapon_id=1; VehPrototypes[1].reactor_id=3;
    Chassis[6].range=1; Chassis[6].triad=TRIAD_LAND;
    mutate_proto_on_focus=1;
    mash_planes();
    assert(Vehs[0].proto_id==1 && Vehs[0].dmg_incurred==9);
    assert(Vehs[0].terraforming_turns==77 && kills==1);
    assert(ConsoleGlobal.focus_calls==1 && booms==1);

    reset();
    Vehs[0].x=2; Vehs[0].y=1; Vehs[0].proto_id=0; Vehs[0].faction_id=1;
    Vehs[0].moves_expended=1; Vehs[0].terraforming_turns=0;
    VehPrototypes[0].chassis_id=0; VehPrototypes[0].reactor_id=2;
    VehPrototypes[0].plan=PLAN_ALIEN_ARTIFACT; VehPrototypes[0].weapon_id=0;
    Chassis[0].triad=TRIAD_AIR; Chassis[0].range=1;
    mash_planes();
    assert(ConsoleGlobal.focus_calls==0 && booms==0);
    assert(Vehs[0].dmg_incurred==1 && kills==1);

    reset();
    Vehs[0].x=2; Vehs[0].y=1; Vehs[0].faction_id=1;
    Vehs[0].moves_expended=1; Vehs[0].dmg_incurred=3;
    Vehs[0].terraforming_turns=7;
    VehPrototypes[0].chassis_id=0; VehPrototypes[0].reactor_id=2;
    Chassis[0].triad=TRIAD_AIR; Chassis[0].range=1;
    mash_planes();
    assert(Vehs[0].dmg_incurred==20 && kills==1 && draws==0);
    assert(Vehs[0].terraforming_turns==7);

    reset();
    Vehs[0].x=2; Vehs[0].y=1; Vehs[0].faction_id=1;
    speed_value=1000;
    VehPrototypes[0].chassis_id=0;
    VehPrototypes[0].plan=PLAN_ALIEN_ARTIFACT;
    Chassis[0].triad=TRIAD_AIR; Chassis[0].range=1;
    mash_planes();
    assert(stacks==1 && Vehs[0].dmg_incurred==1 && kills==1);
}
