#ifndef ALIEN_HOST_FIXTURE_H
#define ALIEN_HOST_FIXTURE_H
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define __cdecl
struct SocialEffect {
    enum EffectIndex { ECONOMY, EFFICIENCY, SUPPORT, TALENT, MORALE, POLICE,
                       GROWTH, PLANET, PROBE, INDUSTRY, RESEARCH };
    int values_[11];
};
typedef SocialEffect SocialPlanet;
struct GoalHost { int16_t type,priority; int x,y,base_id; };
struct PlayerDataHost {
    int diff_level;
    uint32_t diplo_treaties[8];
    int diplo_agenda[8];
    SocialPlanet soc_effect_active;
    uint32_t current_num_bases;
    int x_target,y_target;
    int region_force_rating[128];
    uint8_t region_base_plan[128];
    GoalHost sites[25];
};
typedef PlayerDataHost PlayerData;
struct Veh {
    int16_t x, y;
    uint32_t state;
    uint16_t flags;
    int16_t proto_id, unk_1;
    uint8_t faction_id, year_end_lurking, dmg_incurred;
    int8_t order;
    uint8_t waypoint_count, patrol_current_point;
    int16_t waypoint_x[4], waypoint_y[4];
    uint8_t morale, terraforming_turns, order_auto_type, visibility, moves_expended;
    int8_t facing;
    uint8_t unk_6, move_to_ai_type, probe_action, probe_sabotage_id;
    int16_t home_base_id, next_veh_id_stack, prev_veh_id_stack;
};
struct Base { int16_t x,y; uint8_t faction_id_current,faction_id_former; int8_t population_size; uint8_t pad[2]; uint8_t unk,visibility; uint8_t intel[8]; char name_string[25]; };
struct Map { uint8_t climate,contour,val2,region,visibility,val3,unk; int8_t territory; uint32_t bit,bit2,bit_visible[7]; };
struct RulesBasicHost { int move_rate_roads; int artillery_max_rng; };
struct RulesChassis { char pad0[0x49]; uint8_t triad; char pad1[3]; uint8_t missile; };
struct VehPrototype { char name[32]; uint32_t abilities; uint8_t chassis_id,weapon_id,armor_id,reactor_id,carry,cost,plan; int8_t unk_1; };
struct RulesWeapon { char pad[8]; int8_t offense_rating; };
struct RulesTerraforming { int preq_tech; };
enum { TERRA_MAGTUBE = 6 };
extern RulesTerraforming Terraforming[20];
struct ConsoleHost { int faction_id_; int focus(int,int,int); };
struct NetMsgHost { void pop(const char *, int, int, const char *); };
struct PathHost { int move(int,int); };

extern Veh Vehs[128];
extern Base Bases[128];
extern Map TestMap[64][64];
extern PlayerDataHost PlayersData[128];
extern VehPrototype VehPrototypes[128];
extern RulesChassis Chassis[32];
extern RulesWeapon Weapon[32];
extern RulesBasicHost Rules;
extern ConsoleHost ConsoleGlobal;
extern NetMsgHost NetMsgPrimary;
extern PathHost Paths;
extern int MapLongitudeBounds,MapLatitudeBounds,MapLongitude,MapIsFlat;
struct GameDataLockMaskView { uint32_t enable_mask_; };
struct GameDataStateWindowFields {
    int turn_current_num_, diff_level_current_, base_current_count_, vehicle_current_count_;
    uint32_t game_rules_; GameDataLockMaskView lock_masks_;
};
struct GameDataStateWindow { GameDataStateWindowFields fields_; };
extern GameDataStateWindow GameDataStateStorage;
extern int BaseFindDist, IsMultiplayerNet;
extern int &TurnCurrentNum, &DiffLevelCurrent;
extern uint32_t &LockEnableMask, &GameRules;
extern int &VehCurrentCount, &BaseCurrentCount;
extern int VehMoveDeltaX[9],VehMoveDeltaY[9],RadiusOffsetX[289],RadiusOffsetY[289];
extern int RadiusBaseX[9],RadiusBaseY[9];
extern int events[64],event_count;
extern int test_alien_base,test_zoc,test_cargo,test_stack_check,test_can_arty;
extern int test_path_move,test_veh_at,test_stack_value,test_morale,test_rand_value;
extern int test_stack_call_count,test_stack_args[16][5];
extern int test_garrison_base_calls,test_defensive_base_calls;
extern int test_garrison_required,test_defensive_required;
extern int test_add_goal_calls,test_add_goal_args[6];
extern int test_has_abil_result,test_has_abil_proto,test_has_abil_mask;
extern int test_non_input_calls;
extern int test_zoc_move_args[3],test_zoc_veh_args[3];
extern int test_zoc_move_result,test_zoc_veh_result;
extern int test_zoc_veh_override,test_zoc_veh_override_x,test_zoc_veh_override_y;
extern int test_base_find_count,test_base_find_args[2][6];
extern int test_base_find_dist_values[2];
extern int test_territory_args[5];
extern int test_zoc_sea_result,test_zoc_sea_calls,test_zoc_sea_args[3];
extern int test_has_tech_result,test_has_tech_tech,test_has_tech_faction;
extern int test_base_at_result,test_base_at_args[2],test_base_at_calls;
extern const char *test_parse_says_text;
extern int test_parse_says_calls;

inline Map *map_loc(int x,int y) { return &TestMap[y][x>>1]; }
inline int xrange(int x) { if (!(MapIsFlat & 1)) { if (x<0) x+=MapLongitudeBounds; else if (x>=MapLongitudeBounds) x-=MapLongitudeBounds; } return x; }
inline int speed(int,int) { return 3; }
inline void veh_skip(int id) { events[event_count++]=1; Vehs[id].moves_expended=3; }
inline int alien_base(int,int,int) { return test_alien_base; }
inline int zoc_any(int,int,int) { return test_zoc; }
inline void kill(int) { events[event_count++]=2; }
inline int veh_cargo(int) { return test_cargo; }
inline int stack_check(int vehicle,int type,int cond1,int cond2,int cond3) {
    const int index=test_stack_call_count++;
    test_stack_args[index][0]=vehicle; test_stack_args[index][1]=type;
    test_stack_args[index][2]=cond1; test_stack_args[index][3]=cond2;
    test_stack_args[index][4]=cond3;
    return test_stack_check;
}
inline int garrison_check(int base_id) {
    (void)base_id;
    ++test_garrison_base_calls;
    return test_garrison_required;
}
inline int defensive_check(int base_id) {
    (void)base_id;
    ++test_defensive_base_calls;
    return test_defensive_required;
}
inline void add_goal(int faction_id,int type,int priority,int x,int y,int base_id) {
    ++test_add_goal_calls;
    test_add_goal_args[0]=faction_id; test_add_goal_args[1]=type;
    test_add_goal_args[2]=priority; test_add_goal_args[3]=x;
    test_add_goal_args[4]=y; test_add_goal_args[5]=base_id;
}
inline void set_course(int id,char,int x,int y) { events[event_count++]=3; Vehs[id].order=0x18; Vehs[id].waypoint_x[0]=x; Vehs[id].waypoint_y[0]=y; }
inline int can_arty(int,int) { return test_can_arty; }
inline int stack_fix(int id) { return id; }
inline int base_at(int x,int y) {
    ++test_base_at_calls; test_base_at_args[0]=x; test_base_at_args[1]=y;
    return test_base_at_result;
}
inline int veh_at(int,int) { return test_veh_at; }
inline int zoc_veh(int x,int y,int faction) {
    test_zoc_veh_args[0]=x; test_zoc_veh_args[1]=y; test_zoc_veh_args[2]=faction;
    if (test_zoc_veh_override && x==test_zoc_veh_override_x &&
        y==test_zoc_veh_override_y) return 0;
    return test_zoc_veh_result;
}
inline int zoc_move(int x,int y,int faction) {
    test_zoc_move_args[0]=x; test_zoc_move_args[1]=y; test_zoc_move_args[2]=faction;
    return test_zoc_move_result;
}
inline int base_find(int x,int y,int faction,int region,int faction2,int faction3) {
    const int index=test_base_find_count++;
    int *args=test_base_find_args[index];
    args[0]=x; args[1]=y; args[2]=faction; args[3]=region;
    args[4]=faction2; args[5]=faction3;
    BaseFindDist=test_base_find_dist_values[index];
    return 20+index;
}
inline int whose_territory(int faction,int x,int y,int *base_id,int ignore_comm) {
    test_territory_args[0]=faction; test_territory_args[1]=x;
    test_territory_args[2]=y; test_territory_args[3]=base_id != 0;
    test_territory_args[4]=ignore_comm;
    return faction;
}
inline int zoc_sea(int x,int y,int faction) {
    ++test_zoc_sea_calls;
    test_zoc_sea_args[0]=x; test_zoc_sea_args[1]=y;
    test_zoc_sea_args[2]=faction;
    return test_zoc_sea_result;
}
inline int bit_count(int bits) { int n=0; for(unsigned u=(unsigned)bits;u;u&=u-1)++n; return n; }
inline int battle_fight(int,int,int,int,int *) { events[event_count++]=7; return 0; }
inline void action_destroy(int,int,int,int) { events[event_count++]=4; }
inline void synch_bit(int,int,int) { events[event_count++]=5; }
inline void draw_tile(int,int,int) { events[event_count++]=6; }
inline void log_say(const char *,int,int,int) { events[event_count++]=8; }
inline int morale_alien(int,int) { return test_morale; }
inline void parse_says(int,const char *text,int,int) {
    test_parse_says_text=text; ++test_parse_says_calls;
    events[event_count++]=9;
}
inline int popp(const char *,const char *,int,const char *,int(*)()) { events[event_count++]=10; return 0; }
inline int ConsoleHost::focus(int,int,int) { events[event_count++]=11; return 0; }
inline void NetMsgHost::pop(const char *,int,int,const char *) { events[event_count++]=12; }
inline int PathHost::move(int,int) { events[event_count++]=13; return test_path_move; }
inline int has_abil(int proto_id,int ability_id) {
    test_has_abil_proto=proto_id; test_has_abil_mask=ability_id;
    return test_has_abil_result;
}
inline int has_tech(int tech_id,int faction_id) {
    test_has_tech_tech=tech_id; test_has_tech_faction=faction_id;
    return test_has_tech_result;
}
inline int range(int value,int minimum,int maximum) {
    if (value<minimum) return minimum;
    if (value>maximum) return maximum;
    return value;
}
inline int veh_moves(int id) { return range(speed(id,0)-Vehs[id].moves_expended,0,999); }
inline int planet_buster(int) { return 0; }
inline void do_all_non_input() { ++test_non_input_calls; }

#endif
