// Test-only API/storage doubles. No recovered algorithm is copied here.
// The test inserts current src/enemy.cpp and two slices extracted from it.
#ifndef ENEMY_FULL_BODY_HOST_FIXTURE_H
#define ENEMY_FULL_BODY_HOST_FIXTURE_H
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
typedef GoalHost Goal;
static const int MaxGoalsNum = 75;
struct PlayerDataHost {
    uint32_t flags;
    int diff_level;
    uint32_t ranking;
    uint32_t diplo_treaties[8];
    int diplo_agenda[8];
    uint32_t diplo_friction[8];
    int integrity_blemishes;
    int mil_strength_1;
    int tech_ranking;
    SocialPlanet soc_effect_active, soc_effect_pending;
    int ai_fight, energy_reserves, base_id_atk_target, corner_market_turn;
    int diplo_spoke[8];
    uint16_t unk_79[128];
    uint16_t unk_80[128];
    uint16_t unk_81[128];
    uint8_t unk_82[128];
    uint32_t current_num_bases;
    int x_target,y_target;
    uint16_t region_total_combat_vehs[128];
    uint8_t region_total_bases[128];
    uint8_t region_total_offensive_vehs[128];
    uint16_t region_force_rating[128];
    uint16_t unk_77[128];
    uint8_t unk_83[128];
    uint8_t region_base_plan[128];
    GoalHost goals[MaxGoalsNum];
    GoalHost sites[25];
    uint8_t proto_id_active[512];
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
struct Base {
    int16_t x,y; union { uint8_t faction_id_current; uint8_t faction_id; }; uint8_t faction_id_former;
    int8_t population_size; uint8_t pad[2]; union { uint8_t unk; uint8_t unk_1; }; uint8_t visibility;
    uint8_t intel[8]; char name_string[25]; int16_t unk_x,unk_y;
    uint32_t state,event,governor;
    int nutrients_accumulated,minerals_accumulated,production_id_last,eco_damage;
    uint32_t queue_size; int queue_production_id[10];
    int resource_sqr_radius,specialist_total,specialist_unk_1;
    uint32_t spec[2]; uint8_t facilities_built[12];
    int mineral_surplus_final,minerals_accumulated_2,unk_6,unk_7,unk_8,unk_9;
    int nutrient_intake_1,mineral_intake_1,energy_intake_1,unused_intake_1;
    int nutrient_intake_2,mineral_intake_2,energy_intake_2,unused_intake_2;
    int nutrient_surplus,mineral_surplus,energy_surplus,unused_surplus;
};
struct BaseSecretProject {
    int dream_twister_, neural_amplifier_, universal_translator_, xenoempathy_dome_;
    int &neural_amplifier() { return neural_amplifier_; }
    const int &neural_amplifier() const { return neural_amplifier_; }
    int &dream_twister() { return dream_twister_; }
    const int &dream_twister() const { return dream_twister_; }
    int &universal_translator() { return universal_translator_; }
    const int &universal_translator() const { return universal_translator_; }
    int &xenoempathy_dome() { return xenoempathy_dome_; }
    const int &xenoempathy_dome() const { return xenoempathy_dome_; }
};
struct GameDataLockMaskView { uint32_t enable_mask_; };
struct GameDataStateWindowFields {
    int base_current_count_, diff_level_current_, turn_current_num_;
    int vehicle_current_count_, sunspot_duration_;
    uint32_t game_more_preferences_, game_state_;
    GameDataLockMaskView lock_masks_;
    BaseSecretProject secret_project_;
};
struct GameDataStateWindow { GameDataStateWindowFields fields_; };
extern GameDataStateWindow GameDataStateStorage;

struct Map { uint8_t climate,contour,val2,region,visibility,val3,unk; int8_t territory; uint32_t bit,bit2,bit_visible[7]; };
struct RulesBasicHost {
    int move_rate_roads; int artillery_max_rng;
    int tech_build_roads_fungus; int tech_improve_fungus_sqr;
    int tech_ease_fungus_movement, combat_pct_atk_higher_elev, combat_pen_pct_atk_lwr_elev;
};
struct RulesChassis { char pad0[0x48]; uint8_t speed,triad,range; char pad1[1]; uint8_t missile; };
struct VehPrototype { char name[32]; union { uint32_t abilities; uint32_t ability_flags; }; uint8_t chassis_id,weapon_id,armor_id,reactor_id,carry_capacity,cost,plan; int8_t unk_1; };
struct RulesWeapon { char pad[8]; int8_t offense_rating; int8_t icon; uint8_t mode,cost; };
struct RulesTerraforming { int preq_tech; int preq_tech_sea; };
enum { TERRA_FARM = 0, TERRA_MINE = 2, TERRA_SOLAR_TIDAL = 3,
       TERRA_ROAD = 5, TERRA_MAGTUBE = 6, TERRA_REMOVE_FUNGUS = 10,
       DTREATY_VENDETTA = 0x10 };
enum { SP_NANO_FACTORY = 22 };
enum { FAC_NETWORK_NODE = 8 };
enum { BIT_FUNGUS = 0x20 };
enum { PFLAG_UNK_10000 = 0x00010000 };
extern RulesTerraforming Terraforming[20];
struct ConsoleHost { int faction_id_; int focus(int,int,int); };
struct NetDaemonHost { int add_lock(int,int,int); void await_exec(int);
                       void await_synch(); uint32_t unlock_veh(); };
extern NetDaemonHost NetDaemonNet;
struct NetMsgHost { void pop(const char *, int, int, const char *); };
struct PathHost { int move(int,int); int sensors(int,int *,int *);
                  int find(int,int,int,int,int,int,int,int); };

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
extern int BaseFindDist,IsMultiplayerNet;
extern int VehMoveDeltaX[9],VehMoveDeltaY[9],RadiusOffsetX[289],RadiusOffsetY[289];
extern int RadiusBaseX[9],RadiusBaseY[9];
extern const int RadiusRange[9];
extern int events[8192],event_count;
extern int test_alien_base,test_zoc,test_cargo,test_stack_check,test_can_arty;
extern int test_veh_cost_result,test_veh_cost_calls,test_veh_cost_args[3];
extern int test_path_move,test_path_move_calls,test_path_move_args[2];
extern int test_hex_cost_result,test_hex_cost_calls,test_hex_cost_args[7];
extern int test_base_who_result,test_base_who_calls,test_base_who_args[2];
extern int test_veh_at,test_veh_at_calls,test_veh_at_args[2];
extern int test_stack_value,test_morale,test_rand_value;
extern int test_rnd_calls,test_rnd_range;
extern char test_rnd_text[32];
extern int test_stack_call_count,test_stack_args[8192][5];
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
extern int test_territory_override;
extern int test_zoc_sea_result,test_zoc_sea_calls,test_zoc_sea_args[3];
extern int test_has_tech_result,test_has_tech_tech,test_has_tech_faction;
extern int test_can_convoy_result,test_can_convoy_calls,test_can_convoy_args[2];
extern int test_bonus_at_result,test_bonus_at_calls,test_bonus_at_args[3];
extern int test_resource_yield_result[3],test_resource_yield_calls,
           test_resource_yield_args[5];
extern int test_base_find_three_result,test_base_find_three_calls,
           test_base_find_three_args[3];
extern int test_base_on_sea_result,test_base_on_sea_calls,test_base_on_sea_args[2];
extern int test_base_at_result,test_base_at_args[2],test_base_at_calls;
extern const char *test_parse_says_text;
extern int test_parse_says_calls;
extern int test_good_sensor_result,test_good_sensor_args[3];
extern int test_path_sensors_result,test_path_sensors_calls,test_path_sensors_args[3];
extern int test_path_sensors_target[2];
extern int test_go_to_calls,test_go_to_args[4];
extern int test_action_home_calls,test_action_home_args[2];
extern int test_has_project_result,test_has_project_project,
           test_has_project_faction;
extern int test_get_there_result,test_get_there_calls,test_get_there_args[3];
extern int test_speed_proto_result,test_speed_proto_proto;
extern int test_path_find_result;
int anything_at(int,int);
int has_fac(int,int,int);
int study_artifact(int);
int rnd(int,char *);
int energy_yield(int,int,int,int,int);
extern int test_anything_at_result,test_anything_at_calls,
           test_anything_at_args[2];

inline Map *map_loc(int x,int y) { return &TestMap[y][x>>1]; }
inline int xrange(int x) { if (!(MapIsFlat & 1)) { if (x<0) x+=MapLongitudeBounds; else if (x>=MapLongitudeBounds) x-=MapLongitudeBounds; } return x; }
inline int on_map(int x,int y) {
    return x>=0 && x<MapLongitudeBounds && y>=0 && y<MapLatitudeBounds;
}
inline int speed(int,int) { return 3; }
inline void veh_skip(int id) { events[event_count++]=1; Vehs[id].moves_expended=3; }
inline void synch_veh(int id) { (void)id; events[event_count++]=2; }
extern int test_del_site_calls, test_del_site_args[5];
inline void del_site(int faction, int type, int x, int y, int proximity) {
    ++test_del_site_calls;
    test_del_site_args[0]=faction; test_del_site_args[1]=type;
    test_del_site_args[2]=x; test_del_site_args[3]=y;
    test_del_site_args[4]=proximity;
}
extern int test_stack_sort_calls, test_stack_sort_id;
extern int test_veh_top_calls, test_veh_top_id, test_veh_top_result;
inline void stack_sort_2(int id) { ++test_stack_sort_calls; test_stack_sort_id=id; }
inline int veh_top(int id) {
    ++test_veh_top_calls; test_veh_top_id=id; return test_veh_top_result;
}
extern int test_proto_power_result, test_proto_power_calls, test_proto_power_id;
inline int proto_power(int id) {
    ++test_proto_power_calls; test_proto_power_id=id;
    return test_proto_power_result;
}
inline int alien_base(int,int,int) { return test_alien_base; }
inline int zoc_any(int,int,int) { return test_zoc; }
inline void kill(int) { events[event_count++]=2; }
inline int veh_cost(int proto,int base,void *has_proto_cost) {
    ++test_veh_cost_calls;
    test_veh_cost_args[0]=proto; test_veh_cost_args[1]=base;
    test_veh_cost_args[2]=has_proto_cost != NULL;
    return test_veh_cost_result;
}
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
inline int veh_at(int x,int y) {
    ++test_veh_at_calls;
    test_veh_at_args[0]=x; test_veh_at_args[1]=y;
    return test_veh_at;
}
inline int has_treaty(int faction,int other,int treaty) {
    return PlayersData[faction].diplo_treaties[other] & treaty;
}
inline int has_project(int project,int faction) {
    test_has_project_project=project;
    test_has_project_faction=faction;
    return test_has_project_result;
}
inline int speed_proto(int prototype) {
    test_speed_proto_proto=prototype;
    return test_speed_proto_result;
}
inline int has_tech_call(int tech,int faction) {
    test_has_tech_tech=tech; test_has_tech_faction=faction;
    return test_has_tech_result;
}
inline int x_dist(int x1,int x2) {
    int distance=abs(x1-x2);
    if (!(MapIsFlat & 1) && distance>MapLongitude)
        distance=MapLongitudeBounds-distance;
    return distance;
}
inline int vector_dist(int x1,int y1,int x2,int y2) {
    const int dx=x_dist(x1,x2);
    const int dy=abs(y1-y2);
    int largest=dx;
    if (dx<=dy) largest=dy;
    int smallest=dx;
    if (dx>=dy) smallest=dy;
    return largest-((((dy+dx)>>1)-smallest+1)>>1);
}
inline int vector_dist(int dx,int dy) {
    dx=abs(dx); dy=abs(dy);
    int largest=dx;
    if (dx<=dy) largest=dy;
    int smallest=dx;
    if (dx>=dy) smallest=dy;
    return largest-((((dy+dx)>>1)-smallest+1)>>1);
}
inline int get_there(int vehicle,int x,int y) {
    ++test_get_there_calls;
    test_get_there_args[0]=vehicle;
    test_get_there_args[1]=x;
    test_get_there_args[2]=y;
    return test_get_there_result;
}
inline int base_on_sea(int base_id,int region) {
    ++test_base_on_sea_calls;
    test_base_on_sea_args[0]=base_id;
    test_base_on_sea_args[1]=region;
    return test_base_on_sea_result;
}
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
inline int base_find(int x,int y,int faction) {
    ++test_base_find_three_calls;
    test_base_find_three_args[0]=x; test_base_find_three_args[1]=y;
    test_base_find_three_args[2]=faction;
    return test_base_find_three_result;
}
inline int can_convoy(int x,int y) {
    ++test_can_convoy_calls;
    test_can_convoy_args[0]=x; test_can_convoy_args[1]=y;
    return test_can_convoy_result;
}
inline int bonus_at(int x,int y,int type) {
    ++test_bonus_at_calls;
    test_bonus_at_args[0]=x; test_bonus_at_args[1]=y;
    test_bonus_at_args[2]=type;
    return test_bonus_at_result;
}
inline int resource_yield(int type,int faction,int base,int x,int y) {
    ++test_resource_yield_calls;
    test_resource_yield_args[0]=type; test_resource_yield_args[1]=faction;
    test_resource_yield_args[2]=base; test_resource_yield_args[3]=x;
    test_resource_yield_args[4]=y;
    return test_resource_yield_result[type];
}
int want_monolith(int vehicle_id);
inline int whose_territory(int faction,int x,int y,int *base_id,int ignore_comm) {
    test_territory_args[0]=faction; test_territory_args[1]=x;
    test_territory_args[2]=y; test_territory_args[3]=base_id != 0;
    test_territory_args[4]=ignore_comm;
    return test_territory_override == -9999 ? faction : test_territory_override;
}
inline int zoc_sea(int x,int y,int faction) {
    ++test_zoc_sea_calls;
    test_zoc_sea_args[0]=x; test_zoc_sea_args[1]=y;
    test_zoc_sea_args[2]=faction;
    return test_zoc_sea_result;
}
inline int good_sensor(int faction,int x,int y) {
    test_good_sensor_args[0]=faction;
    test_good_sensor_args[1]=x;
    test_good_sensor_args[2]=y;
    return test_good_sensor_result;
}
inline void go_to(int id,int8_t order,int x,int y) {
    ++test_go_to_calls;
    test_go_to_args[0]=id; test_go_to_args[1]=order;
    test_go_to_args[2]=x; test_go_to_args[3]=y;
}
inline int action_home(int id,int flags) {
    ++test_action_home_calls;
    test_action_home_args[0]=id; test_action_home_args[1]=flags;
    return 0;
}
inline int bit_count(int bits) { int n=0; for(unsigned u=(unsigned)bits;u;u&=u-1)++n; return n; }
inline int battle_fight(int,int,int,int,int *) { events[event_count++]=7; return 0; }
inline void action_destroy(int,int,int,int) { events[event_count++]=4; }
inline void synch_bit(int,int,int) { events[event_count++]=5; }
inline void draw_tile(int,int,int) { events[event_count++]=6; }
inline void log_say(const char *,int,int,int) { events[event_count++]=8; }
void action_gate(int,int);
void message_veh(int,int,int,int);
inline int morale_alien(int,int) { return test_morale; }
inline void parse_says(int,const char *text,int,int) {
    test_parse_says_text=text; ++test_parse_says_calls;
    events[event_count++]=9;
}
inline int popp(const char *,const char *,int,const char *,int(*)()) { events[event_count++]=10; return 0; }
inline int ConsoleHost::focus(int,int,int) { events[event_count++]=11; return 0; }
inline void NetMsgHost::pop(const char *,int,int,const char *) { events[event_count++]=12; }
inline int PathHost::move(int vehicle,int faction) {
    ++test_path_move_calls;
    test_path_move_args[0]=vehicle; test_path_move_args[1]=faction;
    events[event_count++]=13; return test_path_move;
}
inline int PathHost::find(int,int,int,int,int,int,int,int) {
    return test_path_find_result;
}
inline int hex_cost(int proto,int faction,int source_x,int source_y,
                    int destination_x,int destination_y,int toggle) {
    ++test_hex_cost_calls;
    test_hex_cost_args[0]=proto; test_hex_cost_args[1]=faction;
    test_hex_cost_args[2]=source_x; test_hex_cost_args[3]=source_y;
    test_hex_cost_args[4]=destination_x; test_hex_cost_args[5]=destination_y;
    test_hex_cost_args[6]=toggle;
    return test_hex_cost_result;
}
inline int base_who(int x,int y) {
    ++test_base_who_calls;
    test_base_who_args[0]=x; test_base_who_args[1]=y;
    return test_base_who_result;
}
inline int PathHost::sensors(int faction,int *x,int *y) {
    ++test_path_sensors_calls;
    test_path_sensors_args[0]=faction;
    test_path_sensors_args[1]=*x; test_path_sensors_args[2]=*y;
    *x=test_path_sensors_target[0]; *y=test_path_sensors_target[1];
    return test_path_sensors_result;
}
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


struct Continent { unsigned tile_count,open_terrain; }; extern Continent Continents[128];
struct Player { unsigned rule_flags; }; extern Player Players[8];
extern unsigned GameState;
extern int DiploScratchFA60,DiploScratchFA3C,SunspotDuration,ExpansionEnabled;
int base_find(int,int);
int port_to_port(int,int);int port_to_coast(int,int);int sea_coast(int,int);
int is_port(int,int);int is_coast(int,int,int);
void assemble_passengers(int,int,int);void diplomacy_check(int,int,int);
int veh_who(int,int);int at_goal(int,int,int,int);int is_objective(int);
void set_course(int,char,int,int);
int defense_value(int,int,int,int,int);int reputation(int,int);
int compute_odds(int,int,int,int,int);int probe(int,int,int,int);
int is_known(int,int,int);int goody_at(int,int);
int message_data(int,int,int,int,int,int);int X_pop(const char*,int(*)());
int can_terraform(int,int,int,int,int);int is_sensor(int,int);
int terraform_cost(int,int,int);int terrain_avail(int,int,int);
int radius_move(int,int,int,int,int);
int coast_or_border(int,int,int,int,int);
int alien_move(int);
int enemy_planet_buster(int);
int air_power(int,int,int,int*,int*);
int base_territory(int,int,int);
int site_at(int,int,int);
int action_build(int,char*);
void message_base(int,int,char*,int);
int add_site(int,int,int,int,int);
int at_site(int,int,int,int);
void action_give(int,int);

#endif

#include <assert.h>
#include <stddef.h>
RulesTerraforming Terraforming[20];
NetDaemonHost NetDaemonNet;
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
GameDataStateWindow GameDataStateStorage;
int VehMoveDeltaX[9],VehMoveDeltaY[9],RadiusOffsetX[289],RadiusOffsetY[289];
int RadiusBaseX[9],RadiusBaseY[9];
const int RadiusRange[9]={1,9,25,49,81,121,169,225,289};
int events[8192],event_count;
int test_alien_base,test_zoc,test_cargo,test_stack_check,test_can_arty;
int test_veh_cost_result,test_veh_cost_calls,test_veh_cost_args[3];
int test_path_move,test_path_move_calls,test_path_move_args[2];
int test_hex_cost_result,test_hex_cost_calls,test_hex_cost_args[7];
int test_base_who_result,test_base_who_calls,test_base_who_args[2];
int test_veh_at,test_veh_at_calls,test_veh_at_args[2];
int test_stack_value,test_morale,test_rand_value;
int test_rnd_calls,test_rnd_range;
char test_rnd_text[32];
int test_stack_call_count,test_stack_args[8192][5];
int test_garrison_base_calls,test_defensive_base_calls;
int test_garrison_required,test_defensive_required;
int test_add_goal_calls,test_add_goal_args[6];
int test_has_abil_result,test_has_abil_proto,test_has_abil_mask;
int test_non_input_calls;
int test_zoc_move_args[3],test_zoc_veh_args[3];
int test_zoc_move_result,test_zoc_veh_result;
int test_zoc_veh_override,test_zoc_veh_override_x,test_zoc_veh_override_y;
int test_base_find_count,test_base_find_args[2][6];
int test_base_find_dist_values[2];
int test_territory_args[5];
int test_territory_override;
int test_zoc_sea_result,test_zoc_sea_calls,test_zoc_sea_args[3];
int test_has_tech_result,test_has_tech_tech,test_has_tech_faction;
int test_can_convoy_result,test_can_convoy_calls,test_can_convoy_args[2];
int test_bonus_at_result,test_bonus_at_calls,test_bonus_at_args[3];
int test_base_on_sea_result,test_base_on_sea_calls,test_base_on_sea_args[2];
int test_base_at_result,test_base_at_args[2],test_base_at_calls;
const char *test_parse_says_text;
int test_parse_says_calls;
int test_good_sensor_result,test_good_sensor_args[3];
int test_path_sensors_result,test_path_sensors_calls,test_path_sensors_args[3];
int test_path_sensors_target[2];
int test_go_to_calls,test_go_to_args[4];
int test_action_home_calls,test_action_home_args[2];
int test_get_there_result,test_get_there_calls,test_get_there_args[3];
int test_speed_proto_result,test_speed_proto_proto;
int test_path_find_result;
int test_del_site_calls, test_del_site_args[5];
int test_stack_sort_calls, test_stack_sort_id;
int test_veh_top_calls, test_veh_top_id, test_veh_top_result;
int test_proto_power_result, test_proto_power_calls, test_proto_power_id;
unsigned GameState;
int DiploScratchFA60,DiploScratchFA3C,SunspotDuration,ExpansionEnabled;


#define ENEMY_MOVE_HOST_TEST
#define SEA_SCORING_HOST
#define STRATEGIC_ACTIONS_HOST
#define TACTICAL_TAIL_HOST
#define ENEMY_SELECTED_HOST
#define ENEMY_NONSEA_HOST
Continent Continents[128]; Player Players[8];
static int test_air_result,test_site_score,test_build_calls;
int air_power(int,int,int,int*,int*) { return test_air_result; }
int enemy_planet_buster(int) { return 1; }
int base_territory(int,int,int) { return -1; }
int base_find(int,int) { BaseFindDist=0; return -1; }
int site_at(int,int,int) { return test_site_score; }
int is_port(int,int) { return 0; }
int is_coast(int,int,int) { return 1; }
int action_build(int,char*) { ++test_build_calls; return 0; }
int add_site(int,int,int,int,int) { return 0; }
int message_data(int,int,int,int,int,int) { return 0; }
void message_veh(int,int,int,int) {}
void message_base(int,int,char*,int) {}
void NetDaemonHost::await_exec(int) {}
int anything_at(int x,int y) { return TestMap[y][x].territory; }

// Bounded test doubles; this file is never included in the recovered owner.
int test_base_find_three_args[3],test_base_find_three_calls,test_base_find_three_result;
int test_has_project_faction,test_has_project_project,test_has_project_result;
int test_resource_yield_result[3],test_resource_yield_calls,test_resource_yield_args[5];
int reputation(int,int){return 0;}
int test_drop_range_result,test_drop_range_calls,test_airdrop_calls;
int drop_range(int){++test_drop_range_calls;return test_drop_range_result;}
int action_airdrop(int,int,int,int){++test_airdrop_calls;return 0;}
int X_pop(const char*,int(*)()){return 0;}
int study_artifact(int){return 0;}
int is_human(uint32_t){return 0;}
void action_gate(int,int){}
int radius_move(int,int,int,int,int){return -1;}
int compute_odds(int,int,int,int,int){return 0;}
int energy_yield(int,int,int,int,int){return 0;}
int is_objective(int){return 0;}
int port_to_port(int,int){return 0;}
int port_to_coast(int,int){return 0;}
int can_terraform(int,int,int,int,int){return 0;}
int defense_value(int,int,int,int,int){return 0;}
int terrain_avail(int,int,int){return 0;}
int want_monolith(int){return 0;}
int terraform_cost(int,int,int){return 0;}
int coast_or_border(int,int,int,int,int){return 0;}
void diplomacy_check(int,int,int){}
void assemble_passengers(int,int,int){}
int rnd(int,char*){return test_rand_value;}
int probe(int,int,int,int){return 0;}
int at_goal(int,int,int,int){return 0;}
int at_site(int,int,int,int){return 0;}
int has_fac(int,int,int){return 0;}
int veh_who(int,int){return -1;}
int goody_at(int,int){return 0;}
int is_known(int,int,int){return 1;}
int is_sensor(int,int){return 0;}
int sea_coast(int,int){return 0;}
int NetDaemonHost::add_lock(int,int,int){return 1;}
int alien_move(int){return 321;}
void action_give(int,int){}
void NetDaemonHost::await_synch(){}
uint32_t NetDaemonHost::unlock_veh(){return 0;}

/* ACTUAL_ENEMY_SOURCE */

static int run_late_scans(EnemyMoveFullFrame &f,int entry) {
    if (entry) goto label_56ef2f;
    goto label_56ed87;
/* ACTUAL_LATE_SCANS */
label_570002:return 2;
label_57000c:return 3;
label_57460a:return 4;
label_57707f:return 5;
label_578f46:return 6;
label_578b26:return 7;
label_578b29:return 8;
label_578b34:return 9;
}
static void reset_late(EnemyMoveFullFrame &f) {
    memset(&f,0,sizeof(f)); memset(Vehs,0,sizeof(Vehs));
    memset(PlayersData,0,sizeof(PlayersData)); memset(TestMap,0,sizeof(TestMap));
    memset(VehPrototypes,0,sizeof(VehPrototypes)); memset(Chassis,0,sizeof(Chassis));
    f.local_c=1; f.local_18=10;f.local_14=10;f.local_50=1;
    f.local_40=0;f.local_34=1;f.local_64=3;f.local_7c=3;
    f.local_80=-1; f.local_bc=0; f.local_48=0;
    Vehs[0].faction_id=1; Vehs[0].x=10;Vehs[0].y=10;
    Rules.move_rate_roads=3; Rules.artillery_max_rng=2;
    test_can_arty=0;test_site_score=0;test_air_result=-4;
    test_build_calls=0;test_go_to_calls=0;test_veh_at=-1;
    test_base_who_result=-1;test_base_on_sea_result=1;
    MapLongitudeBounds=64;MapLatitudeBounds=64;MapLongitude=32;
    for(int y=0;y<64;++y) for(int x=0;x<64;++x){
        TestMap[y][x].region=1;TestMap[y][x].territory=-1;
        TestMap[y][x].climate=0x60;
    }
}
static int late_scans_control_main() {
    EnemyMoveFullFrame f;
    reset_late(f);f.local_40=2;
    assert(run_late_scans(f,0)==5&&f.local_8c==1);
    reset_late(f);f.local_40=2;test_air_result=-3;
    Vehs[0].state=0x200;Vehs[0].order_auto_type=10;
    assert(run_late_scans(f,0)==0&&!(Vehs[0].state&0x200));
    reset_late(f);f.local_b0=1;
    assert(run_late_scans(f,1)==4);
    reset_late(f);f.local_50=8;test_site_score=9;
    // All radius entries are the center: strict winner keeps index0.
    assert(run_late_scans(f,1)==1&&test_build_calls==1);
    assert(Vehs[0].terraforming_turns==1&&f.local_24==10);
    reset_late(f);test_site_score=10;
    assert(run_late_scans(f,1)==2&&f.local_80==0&&f.local_24==10);
    reset_late(f);f.local_40=1;f.local_74=1;
    TestMap[10][10].climate=0x40;test_can_arty=1;test_veh_at=1;
    Vehs[1].faction_id=2;PlayersData[1].diplo_treaties[2]=0x10;
    test_stack_check=1;test_base_who_result=3;
    assert(run_late_scans(f,1)==2&&f.local_78==-1);
    return 0;
}

static int run_final_upstream(EnemyMoveFullFrame &f,int entry) {
    if(entry==1) goto label_56e6a9;
    if(entry==2) goto label_56e9aa;
    goto label_56e507;
/* ACTUAL_FINAL_UPSTREAM */
label_56ed87:return 11;
label_56ef2f:return 12;
label_57708a:return 13;
}

int main() {
    assert(late_scans_control_main()==0);
    EnemyMoveFullFrame f;
    reset_late(f);
    test_non_input_calls=0;test_base_find_count=0;
    Vehs[0].faction_id=0;
    assert(enemy_move(0)==321&&test_non_input_calls==1);

    for(int automatic=4;automatic<=5;++automatic) {
        reset_late(f);test_base_find_count=0;test_stack_call_count=0;
        test_zoc_veh_result=1;test_zoc_move_result=0;
        test_base_find_dist_values[0]=3;test_base_find_dist_values[1]=3;
        Vehs[0].order=6;Vehs[0].state=0x20200;
        Vehs[0].order_auto_type=automatic;
        const int before_terraform=test_has_tech_tech=-123;
        assert(enemy_move(0)==0);
        assert(Vehs[0].order==0&&Vehs[0].state==0x20000);
        assert(test_has_tech_tech==before_terraform);
    }

    reset_late(f);test_base_find_count=0;test_stack_call_count=0;
    test_zoc_veh_result=1;test_zoc_move_result=0;
    Vehs[0].order=6;Vehs[0].state=0x20200;Vehs[0].order_auto_type=1;
    map_loc(10,10)->climate=0x40; // Every zero-offset land neighbor fails terrain gate.
    assert(enemy_move(0)==0&&Vehs[0].order==0&&Vehs[0].state==0x200);

    reset_late(f);test_base_find_count=0;test_stack_call_count=0;
    Vehs[0].flags=0x440;GameDataStateStorage.fields_.lock_masks_.enable_mask_=0;
    assert(enemy_move(0)==0&&Vehs[0].moves_expended==3);

    reset_late(f);test_base_find_count=0;test_stack_call_count=0;
    Vehs[0].moves_expended=3;Vehs[0].order=24;
    assert(enemy_move(0)==0&&Vehs[0].order==24);

    reset_late(f);test_base_find_count=0;test_stack_call_count=0;
    test_base_find_dist_values[0]=3;test_base_find_dist_values[1]=3;
    test_zoc_veh_result=0;VehPrototypes[0].plan=8;
    assert(enemy_move(-1)==0); // Native negative-input selection request.

    reset_late(f);VehPrototypes[0].plan=12;
    assert(enemy_move_damage_priority(0,f)==4&&f.local_90==4);
    Vehs[0].proto_id=0x10;VehPrototypes[0x10].plan=12;
    assert(enemy_move_damage_priority(0,f)==0&&f.local_90==0);

    // Native DC6F writes damage(+10), not automatic order(+26).
    reset_late(f);f.local_90=3;f.local_84=0;f.local_7c=1;f.local_40=0;
    Weapon[0].offense_rating=1;map_loc(10,10)->climate=0x60;
    map_loc(10,10)->bit=0x2000;TestMap[10][10].territory=-1;
    Vehs[0].dmg_incurred=9;Vehs[0].order_auto_type=99;
    assert(enemy_move_upstream_56da90_probe(f)==ENEMY_MOVE_UPSTREAM_PROBE_JOIN_57708A);
    assert(Vehs[0].dmg_incurred==0&&Vehs[0].order_auto_type==99);
    assert(f.local_4==10&&f.local_8==10&&f.local_70==-1&&f.local_8c==1);

    // The DDD7 loop uses each stack member's damage and continues after a
    // suppressed member, leaving the terminal native -1 stack cursor.
    reset_late(f);f.local_90=3;f.local_74=1;f.local_40=0;f.local_7c=1;
    test_veh_top_result=1;test_proto_power_result=6;test_proto_power_calls=0;
    Vehs[1].proto_id=1;VehPrototypes[1].plan=7;Vehs[1].dmg_incurred=3;
    Vehs[1].next_veh_id_stack=2;Vehs[2].proto_id=1;
    Vehs[2].dmg_incurred=0;Vehs[2].next_veh_id_stack=-1;
    Vehs[0].order_auto_type=0;
    assert(enemy_move_upstream_56ddd7_stack_budget(f)==ENEMY_MOVE_UPSTREAM_DDD7_CONTINUE_56DEB8);
    assert(test_proto_power_calls==2&&f.local_1c==-1);

    // E52F tests moves_expended, not the distinct automatic-order byte.
    reset_late(f);f.local_64=1;f.local_7c=0;f.local_90=0;
    GameDataStateStorage.fields_.base_current_count_=0;test_has_abil_result=1;test_stack_check=1;
    Weapon[0].offense_rating=1;GameDataStateStorage.fields_.lock_masks_.enable_mask_=0;test_drop_range_calls=0;
    Vehs[0].moves_expended=1;Vehs[0].order_auto_type=0;
    assert(run_final_upstream(f,0)==11&&test_drop_range_calls==0);
    Vehs[0].moves_expended=0;Vehs[0].order_auto_type=99;
    assert(run_final_upstream(f,0)==11&&test_drop_range_calls==1);

    // A distant selected enemy must skip the own-base route immediately.
    reset_late(f);f.local_64=1;GameDataStateStorage.fields_.base_current_count_=2;
    memset(Bases,0,sizeof(Bases));test_has_tech_result=0;
    Bases[0].faction_id_current=2;Bases[0].x=30;Bases[0].y=10;
    Bases[1].faction_id_current=1;Bases[1].x=10;Bases[1].y=10;
    PlayersData[1].diplo_treaties[2]=0x10;
    test_drop_range_result=1;test_stack_call_count=0;test_stack_check=0;
    assert(run_final_upstream(f,1)==11&&test_go_to_calls==0);
    assert(test_stack_call_count==0&&f.local_24==0&&f.local_54==2);
    assert(f.local_58==30&&f.local_60==0);

    // E9AA uses region_base_plan and preserves native terminal scratch writes.
    reset_late(f);f.local_64=1;f.local_38=1;f.local_28=99999;
    GameDataStateStorage.fields_.base_current_count_=2;memset(Bases,0,sizeof(Bases));
    Bases[0].faction_id_current=2;Bases[0].population_size=4;
    Bases[0].x=10;Bases[0].y=10;Bases[1].faction_id_current=1;
    PlayersData[1].proto_id_active[0]=2;
    PlayersData[1].diplo_treaties[2]=0x10;
    PlayersData[2].region_base_plan[1]=7;
    test_territory_override=-1;test_stack_check=0;test_has_tech_result=0;
    test_rand_value=0;test_veh_at=-1;
    assert(run_final_upstream(f,2)==11);
    assert(f.local_20==12&&f.local_70==1&&f.local_2c==32);
    assert(f.local_1c==-1&&f.local_24==2);
    assert(f.base_cursor_48==reinterpret_cast<const short *>(&Bases[2].faction_id_current));
    return 0;
}
