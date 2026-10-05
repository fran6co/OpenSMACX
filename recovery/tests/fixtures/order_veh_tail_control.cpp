#include "order_veh_host_surface.h"
/* PRODUCT_HELPERS */

Veh Vehs[16]; VehPrototype VehPrototypes[512]; RulesChassis Chassis[16];
RulesWeapon Weapon[32]; Map MapTiles[4096]; Console ConsoleGlobal;
NetDaemon NetDaemonNet; NetMsg NetMsgPrimary; RulesType Rules;
PlayerData PlayersData[8]; Player Players[8]; Base Bases[16];
RulesFacility Facility[128];
int GenderDefault,PluralityDefault;
uint32_t GamePreferences,GameMorePreferences,GameState,LockEnableMask;
int MapLongitude=64,IsMultiplayerNet,ConsoleControlTurnActive;
int MapIsFlat=1,MapLongitudeBounds=128,MapLatitudeBounds=64;
int VehMoveDeltaX[9],VehMoveDeltaY[9];
static int goal_calls,goal_x,goal_y,goal_faction,monolith_calls;
static int speed_result=4,home_calls,skip_calls,zoc_result;
static int parse_calls,spot_calls;
static int veh_at_calls;
static int rand_step;
Tutorial TutorialWindow; FX g_FX;
uint32_t VehOrderDelayCounter;
static bool mutate_spent,mutate_after_animation;
static int tutorial_calls,tiles_calls;
static const char *tutorial_text;
extern "C" int rand() throw() { return rand_step++==0 ? 2 : 3; }
int has_tech(...) { return 0; }
void bitmask(int id,int *byte_offset,int *mask) { *byte_offset=id/8; *mask=1<<(id&7); }
Map *map_loc(int x,int y) { return &MapTiles[MapLongitude*y+(x>>1)]; }
int veh_cargo(int) { return 1; }
int veh_at(...) { ++veh_at_calls; return -1; }
int parse_says(int,const char*,int,int) { ++parse_calls; return 0; }
void spot_stack(int vehicle,int faction) { assert(vehicle==0&&faction==2); ++spot_calls; }
void add_goal(int faction,int type,int priority,int x,int y,int base) {
 assert(type==6&&priority==2&&base==-1);
 ++goal_calls; goal_x=x; goal_y=y; goal_faction=faction;
}
void monolith(int vehicle) { assert(vehicle==0); ++monolith_calls; }
int goody_box(...) { return 0; }
int speed(int vehicle,int mode) { assert(vehicle>=0&&vehicle<=1&&mode==0); if(mutate_spent) Vehs[vehicle].moves_expended=99; return speed_result; }
int action_home(int vehicle,int mode) {
 assert(vehicle==0&&mode==0); ++home_calls;
 Vehs[0].terraforming_turns=9;
 return 0;
}
int zoc_veh(int,int,int) { return zoc_result; }
void veh_skip(int vehicle) { assert(vehicle==0); ++skip_calls; }
void veh_demote(...) {}
void draw_tile(...) {}
void draw_tiles(...) { ++tiles_calls; if(mutate_after_animation) { Vehs[0].proto_id=2; VehPrototypes[2].plan=PLAN_TERRAFORMING; map_loc(6,4)->bit=0; } }
int Tutorial::tut_map(const char *text,int x,int y,int pixel,Sprite *picture,int zoom,int forwarded_x,int forwarded_y) {
 assert(x==6&&y==4&&picture==0&&zoom==0);
 if(strcmp(text,"FRACTIONALMOVE")==0) assert(pixel==0&&forwarded_x==2&&forwarded_y==4);
 else assert(strcmp(text,"SLOWFUNGUS")==0&&pixel==-1&&forwarded_x==-1&&forwarded_y==-1);
 ++tutorial_calls; tutorial_text=text; return 0;
}
int random(int,int maximum) { assert(maximum>0); return maximum-1; }
int tut_check(...) { return 1; }
int GetAsyncKeyState(int) { return 0; }
KeyState ConsoleEditKeyStateSlot() { return GetAsyncKeyState; }
int stack_veh(...) { return 0; }
void veh_scoot(...) {}
void stack_put(...) {}
void spot_loc(...) {}
void synch_veh(...) {}
void boom(...) {}
void kill(...) {}

int main() {
 // Actual candidate terrain-goal helper: non-MoveTo, averaged lowland,
 // averaged-highland destination fallback, and original Y-odd -> X increment.
 Vehs[0].order=ORDER_NONE;
 order_veh_move_to_terrain_goal(0,3,6,4);
 assert(goal_calls==1&&goal_x==6&&goal_y==4&&goal_faction==3);
 Vehs[0].order=ORDER_MOVE_TO; Vehs[0].waypoint_x[0]=10; Vehs[0].waypoint_y[0]=10;
 order_veh_move_to_terrain_goal(0,3,6,4);
 assert(goal_calls==2&&goal_x==9&&goal_y==7);
 MapTiles[64*7+(9>>1)].climate=0x60;
 order_veh_move_to_terrain_goal(0,3,6,4);
 assert(goal_calls==3&&goal_x==6&&goal_y==4);
 MapTiles[64*4+3].climate=0x60;
 order_veh_move_to_terrain_goal(0,3,6,4);
 assert(goal_calls==3);
 // Negative averages truncate toward zero; the native parity repair changes X.
 Vehs[0].waypoint_x[0]=-7; Vehs[0].waypoint_y[0]=0;
 order_veh_move_to_terrain_goal(0,3,6,4);
 assert(goal_calls==4&&goal_x==0&&goal_y==2);

 Map destination={0,0,0,0,BIT_MONOLITH};
 Weapon[0].offense_rating=1; Vehs[0].state=0x100;
 Vehs[0].dmg_incurred=0;
 order_veh_monolith_goody_corridor(0,-1,3,destination,TRIAD_LAND,false,0);
 assert(monolith_calls==0);
 Vehs[0].dmg_incurred=1; Vehs[0].morale=7;
 order_veh_monolith_goody_corridor(0,-1,3,destination,TRIAD_LAND,false,0);
 assert(monolith_calls==1);
 order_veh_monolith_goody_corridor(0,0,3,destination,TRIAD_AIR,false,0);
 assert(monolith_calls==1);

 // Fatigue increment must read the live byte AFTER action_home callback.
 Chassis[0].range=8; ConsoleGlobal.faction_id_=3; GamePreferences=0x10000;
 LockEnableMask=8; Vehs[0].terraforming_turns=3; Vehs[0].state=0;
 Vehs[0].order=ORDER_NONE;
 assert(!order_veh_599061_fatigue_and_damage(0,3,8,false));
 assert(home_calls==1&&Vehs[0].terraforming_turns==10);
 // Unlocked in-tile airbase reset skips/demotes rather than fuel continuation.
 LockEnableMask=0; Vehs[0].terraforming_turns=4;
 assert(order_veh_598e84_range_speed_gate(0,3,8,false,false,6,4)==OrderVeh598E84ContinueTo599151);
 assert(skip_calls==1&&Vehs[0].terraforming_turns==0);

 // Foreign-base entry uses actor->controller bit1 and controller->actor mask7.
 // The native controller==active branch intentionally does not spot the stack.
 int selector=0; NetDaemonNet.lock_state_[1].leading_words_[3]=2;
 ConsoleGlobal.faction_id_=2; Bases[0].faction_id_current=2;
 PlayersData[2].diplo_treaties[1]=1; PlayersData[1].diplo_treaties[2]=0;
 assert(order_veh_597b24_entry(0,1,2,0,2,3,selector)==OrderVeh597b24EntryContinueCombat);
 assert(selector==1&&parse_calls==3&&spot_calls==0);
 PlayersData[2].diplo_treaties[1]=0; PlayersData[1].diplo_treaties[2]=1;
 order_veh_597b24_entry(0,1,2,0,2,3,selector);
 assert(parse_calls==3&&spot_calls==0);
 PlayersData[1].diplo_treaties[3]=0;
 order_veh_597b24_entry(0,1,2,0,3,3,selector);
 assert(parse_calls==3&&spot_calls==1);

 // Sea recovery looks for one adjacent landing tile, then operates on the
 // mover's own passenger stack. It never queries a vehicle on the adjacent tile.
 memset(MapTiles,0,sizeof(MapTiles));
 Vehs[0].proto_id=1; VehPrototypes[1].chassis_id=1; Chassis[1].triad=TRIAD_SEA;
 Vehs[0].moves_expended=4; Vehs[0].order=ORDER_NONE;
 Vehs[0].prev_veh_id_stack=-1; Vehs[0].next_veh_id_stack=1;
 Vehs[1].prev_veh_id_stack=0; Vehs[1].next_veh_id_stack=-1;
 Vehs[1].moves_expended=0; Vehs[1].order=ORDER_SENTRY_BOARD;
 VehMoveDeltaX[0]=2; VehMoveDeltaY[0]=0;
 assert(!order_veh_598a84_sea_cargo_recovery(0,2,6,4,true,true));
 assert(Vehs[1].order==ORDER_SENTRY_BOARD&&veh_at_calls==0);
 map_loc(8,4)->climate=0x60;
 assert(order_veh_598a84_sea_cargo_recovery(0,2,6,4,true,true));
 assert(Vehs[1].order==ORDER_NONE&&ConsoleGlobal.selected_vehicle_==1&&veh_at_calls==0);

 // Facility validity resets each retry (59834F), including the actual local30.
 Players[0].faction_bonus_count=1; Players[0].faction_bonus_id[0]=2;
 Players[0].faction_bonus_val1[0]=2; Bases[0].facilities_built[0]=12;
 const OrderVehEventFacilityResult facility=order_veh_598338_pick_event_facility(0,6,4);
 assert(facility.facility_id==3&&facility.attempts==1&&facility.facility_valid&&facility.native_local_30);

 // 597714 low remaining enters the delay path even on nonfungus, visible tiles.
 memset(MapTiles,0,sizeof(MapTiles)); Vehs[0].proto_id=0;
 Vehs[0].order=ORDER_NONE; Vehs[0].state=0; Vehs[0].moves_expended=3;
 speed_result=4; Rules.move_rate_roads=3; ConsoleGlobal.faction_id_=1;
 NetDaemonNet.lock_state_[1].leading_words_[3]=1; ConsoleControlTurnActive=0;
 selector=0; const int skip_before=skip_calls;
 assert(order_veh_5976d6_terrain_delay(0,1,0,2,4,6,4,7,5,false,selector)==OrderVeh5976d6CommonCleanup);
 assert(tutorial_calls==1&&strcmp(tutorial_text,"FRACTIONALMOVE")==0&&skip_calls==skip_before&&selector==0);
 // First clamp must retain the byte loaded before a speed callback changes it.
 Vehs[0].moves_expended=1; mutate_spent=true;
 assert(order_veh_5976d6_clamp_remaining(0)==3&&Vehs[0].moves_expended==99);
 mutate_spent=false;
 // Animation redraw is draw_tiles; tutorial predicates reload terrain and proto.
 Vehs[0].moves_expended=3; Vehs[0].state=0; VehPrototypes[0].plan=0;
 map_loc(6,4)->bit=BIT_FUNGUS; map_loc(6,4)->climate=0x60;
 mutate_after_animation=true;
 assert(order_veh_5976d6_terrain_delay(0,1,0,2,4,6,4,-1,5,false,selector)==OrderVeh5976d6CommonCleanup);
 assert(tiles_calls==2&&tutorial_calls==2&&strcmp(tutorial_text,"FRACTIONALMOVE")==0);
 return 0;
}
