// Actual-source regression fixture. Synthetic service doubles and host layout are not native/game proof.
#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cctype>
#include <stdint.h>
#define __cdecl






typedef int int4;
typedef unsigned int uint4;
typedef short int2;
typedef unsigned short uint2;
typedef char int1;
typedef unsigned char uint1;
typedef struct SocialCategory { uint4 models_[4]; };
typedef struct SocialEffect { int4 economy; int4 efficiency; int4 support; int4 talent; int4 morale; int4 police; int4 growth; int4 planet; int4 probe; int4 industry; int4 research; };
typedef struct Goal { int2 type; int2 priority; int4 x; int4 y; int4 base_id; };
typedef struct Player { int4 is_leader_female; int1 filename[24]; int1 search_key[24]; int1 name_leader[24]; int1 title_leader[24]; int1 adj_leader[128]; int1 adj_insult_leader[128]; int1 adj_faction[128]; int1 adj_insult_faction[128]; uint1 padding1[128]; int1 noun_faction[24]; int4 noun_gender; int4 is_noun_plural; int1 adj_name_faction[128]; int1 formal_name_faction[40]; int1 insult_leader[24]; int1 desc_name_faction[24]; int1 assistant_name[24]; int1 scientist_name[24]; int1 assistant_city[24]; uint1 padding2[176]; int4 rule_tech_selected; int4 rule_morale; int4 rule_research; int4 rule_drone; int4 rule_talent; int4 rule_energy; int4 rule_interest; int4 rule_population; int4 rule_hurry; int4 rule_techcost; int4 rule_psi; int4 rule_sharetech; int4 rule_commerce; int4 rule_flags; int4 faction_bonus_count; int4 faction_bonus_id[8]; int4 faction_bonus_val1[8]; int4 faction_bonus_val2[8]; int4 ai_fight; int4 ai_growth; int4 ai_tech; int4 ai_wealth; int4 ai_power; int4 soc_ideology_category; int4 soc_anti_ideology_category; int4 soc_ideology_model; int4 soc_anti_ideology_model; int4 soc_ideology_effect; int4 soc_anti_ideology_effect; };
typedef struct PlayerData { uint4 flags; uint4 ranking; int4 diff_level; uint4 base_name_offset; uint4 base_sea_name_offset; int4 last_turn_new_base; uint4 diplo_treaties[8]; int4 diplo_agenda[8]; uint4 diplo_friction[8]; int4 diplo_spoke[8]; int4 diplo_merc[8]; int1 diplo_patience[8]; int4 sanction_turns; int4 loan_balance[8]; int4 loan_payment[8]; int4 unk_12[8]; int4 integrity_blemishes; int4 global_reputation; int4 diplo_unk1[8]; int4 diplo_wrongs[8]; int4 diplo_betrayed[8]; int4 diplo_unk3[8]; int4 diplo_unk4[8]; int4 traded_maps; int4 base_governor_adv; int4 atrocities; int4 major_atrocities; uint4 mind_control_total; uint4 diplo_mind_control[8]; uint4 stolen_data_count[8]; int4 energy_reserves; uint4 hurry_cost_total; SocialCategory soc_category_pending; SocialCategory soc_category_active; int4 soc_upheaval_cost_paid; SocialEffect soc_effect_pending; SocialEffect soc_effect_active; SocialEffect soc_effect_temp; SocialEffect soc_effect_base; int4 unk_13; uint4 maint_cost_total; int4 tech_commerce_bonus; int4 turn_commerce_income; int4 unk_17; int4 unk_18; int4 tech_fungus_nutrient; int4 tech_fungus_mineral; int4 tech_fungus_energy; int4 tech_fungus_unk; int4 se_alloc_psych; int4 se_alloc_labs; uint4 tech_pact_shared_goals[12]; int4 tech_ranking; int4 unk_26; uint4 sat_odp_deployed; int4 theory_of_everything; int1 tech_trade_source[92]; int4 tech_accumulated; int4 tech_id_researching; int4 tech_cost; int4 earned_techs_saved; int4 net_random_event; int4 ai_fight; int4 ai_growth; int4 ai_tech; int4 ai_wealth; int4 ai_power; int4 x_target; int4 y_target; int4 unk_28; int4 council_call_turn; int4 unk_29[11]; int4 unk_30[11]; uint1 facility_announced[9]; uint1 announcement_reserved[3]; int4 planet_ecology; int4 base_id_atk_target; int4 unk_37; int1 saved_queue_name[8][24]; int4 saved_queue_size[8]; int4 saved_queue_items[8][10]; int4 unk_38[8]; int4 unk_39[8][9]; int4 unk_46[9]; int4 unk_47; int4 unk_48; int4 unk_49; uint4 nutrient_surplus_total; int4 labs_total; uint4 satellites_[4]; int4 best_weapon_value; int4 best_psi_offense; int4 best_psi_defense; int4 best_armor_value; int4 best_land_speed; int4 enemy_best_weapon_value; int4 enemy_best_armor_value; int4 enemy_best_land_speed; int4 enemy_best_psi_offense; int4 enemy_best_psi_defense; int4 unk_64; int4 unk_65; int4 unk_66; int4 unk_67; int4 unk_68; int1 unk_69[4]; uint1 proto_id_active[512]; uint1 proto_id_queue[512]; int2 proto_id_lost[512]; int4 total_mil_veh; uint4 current_num_bases; int4 mil_strength_1; int4 mil_strength_2; int4 pop_total; int4 unk_70; int4 planet_busters; int4 unk_71; int4 unk_72; uint2 region_total_combat_vehs[128]; uint1 region_total_bases[128]; uint1 region_total_offensive_vehs[128]; uint2 region_force_rating[128]; uint2 unk_77[128]; uint2 unk_78[128]; uint2 unk_79[128]; uint2 unk_80[128]; uint2 unk_81[128]; uint1 unk_82[128]; uint1 unk_83[128]; uint1 region_base_plan[128]; Goal goals[75]; Goal sites[25]; int4 unk_93; int4 unk_94; int4 unk_95; int4 unk_96; int4 unk_97; uint4 tech_achieved; int4 time_bonus_count; int4 unk_99; uint4 secret_project_intel[8]; int4 corner_market_turn; int4 corner_market_active; int4 unk_101; int4 unk_102; int4 prototype_history_cursor; uint4 flags_ext; int4 unk_105; int4 unk_106; int4 unk_107; int4 unk_108; int4 unk_109; int4 score_items_[9]; };
typedef struct Base { int2 x; int2 y; uint1 faction_id_current; uint1 faction_id_former; int1 population_size; uint1 assimilation_turns_left; uint1 nerve_staple_turns_left; uint1 unk_1; uint1 visibility; uint1 faction_pop_size_intel[8]; int1 name_string[25]; int2 unk_x; int2 unk_y; uint4 state; uint4 event; uint4 governor; int4 nutrients_accumulated; int4 minerals_accumulated; int4 production_id_last; int4 eco_damage; uint4 queue_size; int4 queue_production_id[10]; int4 resource_sqr_radius; int4 specialist_total; int4 specialist_unk_1; uint4 spec[2]; uint1 facilities_built[12]; int4 mineral_surplus_final; int4 minerals_accumulated_2; int4 unk_6; int4 unk_7; int4 unk_8; int4 unk_9; int4 nutrient_intake_1; int4 mineral_intake_1; int4 energy_intake_1; int4 unused_intake_1; int4 nutrient_intake_2; int4 mineral_intake_2; int4 energy_intake_2; int4 unused_intake_2; int4 nutrient_surplus; int4 mineral_surplus; int4 energy_surplus; int4 unused_surplus; int4 nutrient_inefficiency; int4 mineral_inefficiency; int4 energy_inefficiency; int4 unused_inefficiency; int4 nutrient_consumption; int4 mineral_consumption; int4 energy_consumption; int4 unused_consumption; int4 economy_total; int4 psych_total; int4 labs_total; int4 unk_10; int2 auto_forward_land_base_id; int2 auto_forward_sea_base_id; int2 auto_forward_air_base_id; int2 padding_0; int4 talent_total; int4 drone_total; int4 super_drone_total; int4 random_event_turns; int4 nerve_staple_count; int4 unk_11; int4 location; };
typedef struct RulesFacility { int1 * name; int1 * effect; uint4 pad; int4 cost; uint4 maint; int4 preq_tech; int4 free_tech; int4 sp_ai_fight; int4 sp_ai_growth; int4 sp_ai_tech; int4 sp_ai_wealth; int4 sp_ai_power; };
typedef struct Map { uint1 climate; uint1 contour; uint1 val2; uint1 region; uint1 visibility; uint1 val3; uint1 unk_1; int1 territory; uint4 bit; uint4 bit2; uint4 bit_visible[7]; };
typedef struct Veh { int2 x; int2 y; uint4 state; uint2 flags; int2 proto_id; uint2 unk_1; uint1 faction_id; uint1 year_end_lurking; uint1 dmg_incurred; int1 order; uint1 waypoint_count; uint1 patrol_current_point; int2 waypoint_x[4]; int2 waypoint_y[4]; uint1 morale; uint1 terraforming_turns; uint1 order_auto_type; uint1 visibility; uint1 moves_expended; int1 facing; uint1 unk_6; uint1 move_to_ai_type; uint1 probe_action; uint1 probe_sabotage_id; int2 home_base_id; int2 next_veh_id_stack; int2 prev_veh_id_stack; };
typedef struct VehPrototype { int1 veh_name[32]; uint4 ability_flags; uint1 chassis_id; uint1 weapon_id; uint1 armor_id; uint1 reactor_id; uint1 carry_capacity; uint1 cost; uint1 plan; int1 unk_1; uint1 obsolete_factions; int1 combat_factions; int1 icon_offset; int1 padding; uint2 flags; int2 preq_tech; };
typedef struct RulesChassis { int1 * offsv_1_name; int1 * offsv_2_name; int1 * offsv_lrg_name; int1 * defsv_1_name; int1 * defsv_2_name; int1 * defsv_lrg_name; uint4 offsv_1_gender; uint4 offsv_2_gender; uint4 offsv_lrg_gender; uint4 defsv_1_gender; uint4 defsv_2_gender; uint4 defsv_lrg_gender; int4 offsv_1_plural; int4 offsv_2_plural; int4 offsv_lrg_plural; int4 defsv_1_plural; int4 defsv_2_plural; int4 defsv_lrg_plural; uint1 speed; uint1 triad; uint1 range; uint1 cargo; uint1 cost; uint1 missile; uint1 sprite_flag_x_off[8]; uint1 sprite_flag_y_off[8]; uint1 sprite_unk1_x_off[8]; uint1 sprite_unk1_y_off[8]; uint1 sprite_unk2_x_off[8]; uint1 sprite_unk2_y_off[8]; uint1 sprite_unk3_x_off[8]; uint1 sprite_unk3_y_off[8]; int2 preq_tech; };
typedef struct RulesWeapon { int1 * name; int1 * name_short; int1 offense_rating; int1 icon; uint1 mode; uint1 cost; int2 preq_tech; int2 padding; };
typedef struct RulesArmor { int1 * name; int1 * name_short; int1 defense_rating; uint1 mode; uint1 cost; uint1 padding1; int2 preq_tech; int2 padding2; };
typedef struct RulesBasic { uint4 move_rate_roads; uint4 nutrient_req_citizen; uint4 max_airdrop_sans_orb_insert; uint4 artillery_max_rng; uint4 artillery_dmg_num; uint4 artillery_dmg_denom; uint4 nutrient_cost_multi; uint4 mineral_cost_multi; uint4 tech_discovery_rate_pct_std; uint4 limit_mineral_mine_sans_road; int4 tgl_nutrient_effect_with_mine; uint4 min_base_size_specialists; uint4 drones_genejack_factory; uint4 pop_limit_sans_hab_complex; uint4 pop_limit_sans_hab_dome; int4 tech_improve_fungus_sqr; int4 tech_ease_fungus_movement; int4 tech_build_roads_fungus; int4 tech_two_spec_abilities; int4 tech_three_nutrients_sqr; int4 tech_three_minerals_sqr; int4 tech_three_energy_sqr; uint4 extra_pct_cost_proto_land; uint4 extra_pct_cost_proto_sea; uint4 extra_pct_cost_proto_air; uint4 psi_combat_ratio_atk[3]; uint4 psi_combat_ratio_def[3]; uint4 player_start_energy_reserve; int4 combat_pct_base_def; int4 combat_pct_atk_road; int4 combat_pct_atk_higher_elev; int4 combat_pen_pct_atk_lwr_elev; int4 tech_orb_insert_sans_spc_elev; uint4 min_turns_councils; uint4 minerals_harvesting_forest; uint4 territory_max_dist_base; uint4 turns_corner_gbl_energy_mrkt; int4 tech_mining_platform_bonus; int4 tech_economic_victory; int4 combat_pen_pct_atk_airdrop; int4 combat_pct_fanatic_atk_bonus; int4 combat_pct_land_gun_vs_ship_art; int4 combat_pct_art_bonus_lvl_alt; int4 combat_pct_mobile_open_ground; int4 combat_pct_def_vs_mobile_rough; int4 combat_pct_trance_def_vs_psi; int4 combat_pct_emp_song_atk_vs_psi; int4 combat_pct_infantry_vs_base; int4 combat_pen_pct_air_supr_vs_grnd; int4 combat_pct_air_supr_vs_air; int4 combat_pen_pct_non_cbt_def_vs_cbt; int4 combat_pct_com_jam_def_vs_mobl; int4 combat_pct_bonus_vs_ship_port; int4 combat_pct_aaa_bonus_vs_air; int4 combat_pct_def_range_sensor; int4 combat_pct_psi_atk_bonus_planet; uint4 retool_strictness; uint4 retool_pct_pen_prod_chg; uint4 retool_exemption; int4 tgl_probe_can_steal_tech; int4 tgl_human_always_contact_net; int4 tgl_humans_always_contact_pbem; uint4 max_pct_dmg_art_vs_unit_base_bnkr; uint4 max_pct_dmg_art_vs_units_open; uint4 max_pct_dmg_art_vs_units_sea; uint4 freq_global_warming_num; uint4 freq_global_warming_denom; uint4 normal_starting_year; uint4 normal_end_year_low_three_diff; uint4 normal_end_year_high_three_diff; int4 tgl_oblit_base_atrocity; uint4 size_base_subspace_gen; uint4 subspace_generators_needed; };
typedef struct RulesWorldbuilder { uint4 land_base; uint4 land_mod; uint4 continent_base; uint4 continent_mod; uint4 hills_base; uint4 hills_mod; uint4 plateau_base; uint4 plateau_mod; uint4 rivers_base; uint4 rivers_rain_mod; uint4 solar_energy; uint4 thermal_band; uint4 thermal_deviance; uint4 global_warming; uint4 sea_level_rises; uint4 cloudmass_peaks; uint4 cloudmass_hills; uint4 rainfall_coeff; int4 deep_water; int4 shelf; int4 plains; int4 beach; uint4 hills; int4 peaks; uint4 fungus; uint4 cont_size_ratio1; uint4 cont_size_ratio2; uint4 cont_size_ratio3; uint4 cont_size_ratio4; uint4 cont_size_ratio5; uint4 islands; };
typedef struct RulesResourceinfo { int4 nutrients; int4 minerals; int4 energy; int4 unused; };
typedef struct Continent { uint4 tile_count; uint4 open_terrain; uint4 unk_3; uint4 pods; uint4 unk_5; uint1 sea_coasts[8]; };
struct RulesReactor { int1 *name; int1 *name_short; int2 preq_tech; uint2 power; };
/*ACTUAL_PROJECT_OWNER*/
BaseSecretProject SecretProject;
RulesReactor Reactor[4];
uint4 RankingFactionIDUnk1;
int4 ClimateSeaLevelDelta;
uint4 AltNatural[11];
uint1 GameTechAchieved[89];
int LastSetBaseId, SetBaseCalls, ComputeCalls, VehAvailCalls, FacilityAvailCalls;
int StubAvailVehicle=-1, StubAvailFacility=-1;
int4 BaseIDCurrentSelected;
uint4 MapLongitude;
uint4 BaseCurrentForcesSupported;
Base *BaseCurrent;
int4 IsMultiplayerNet;
RulesBasic Rules;
RulesWorldbuilder WorldBuilder;
RulesResourceinfo ResourceInfo[9];
int4 MapLongitudeBounds;
int4 MapLatitudeBounds;
int4 MapSeaLevel;
uint4 MapAreaSqRoot;
int4 MapIsFlat;
uint4 MapOceanCoverage;
uint4 MapCloudCover;
Map *MapTiles;
RulesChassis Chassis[19];
RulesWeapon Weapon[26];
RulesArmor Armor[14];
Player Players[8];
PlayerData PlayersData[8];
Base Bases[512];
RulesFacility Facility[134];
uint4 GameRules;
uint4 GameState;
int4 DiffLevelCurrent;
int4 VehCurrentCount;
int4 BaseCurrentCount;
int4 TurnCurrentNum;
uint4 LockEnableMask;
uint4 FactionRankings[8];
int4 GovernorFaction;
VehPrototype VehPrototypes[512];
Continent Continents[128];
Veh Vehs[2049];
int4 RadiusOffsetX[289];
int4 RadiusOffsetY[289];
int1 StringTemp[1032];
void base_compute(int4 a0) { assert(a0==1 && SetBaseCalls==1 && ComputeCalls==0); ++ComputeCalls; }
void set_base(int4 a0) { LastSetBaseId=a0; ++SetBaseCalls; BaseIDCurrentSelected=a0; BaseCurrent=&Bases[a0]; }
int4 cost_factor(int4 a0,int4 a1,int4 a2) { return 10; }
int4 base_lose_minerals(int4 a0,int4 a1) { return 0; }
int4 morale_mod(int4 a0,int4 a1,int4 a2) { return 0; }
int4 worm_mod(int4 a0,int4 a1) { return 0; }
int4 fac_maint(int4 a0,int4 a1) { return 0; }
int4 has_project(int4 a0,int4 a1) { return 0; }
uint1 has_fac_built(int4 a0) { return 0; }
int4 base_project(int4 a0) { return SecretProject[a0]; }
int4 garrison_check(int4 a0) { return 0; }
int4 defensive_check(int4 a0) { return 0; }
int4 facility_avail(int4 a0,int4 a1,int4 a2,int4 a3) { ++FacilityAvailCalls; return a0==StubAvailFacility; }
int4 base_claimed(int4 a0,int4 a1,int4 a2,int4 a3,int4 a4,int4 a5) { return 0; }
int4 has_treaty(int4 a0,int4 a1,int4 a2) { return 0; }
int4 aah_ooga(int4 a0,int4 a1) { return 0; }
int4 climactic_battle() { return 0; }
int4 at_climax(int4 a0) { return 0; }
int4 guard_check(int4 a0,int4 a1) { return 0; }
int4 at_goal(int4 a0,int4 a1,int4 a2,int4 a3) { return 0; }
int4 generators(int4 a0,int4 * a1) { return 0; }
int4 range(int4 a0,int4 a1,int4 a2) { return a0 < a1 ? a1 : (a0 > a2 ? a2 : a0); }
int4 parse_num(int4 a0,int4 a1) { return 0; }
int4 bit_count(int4 a0) { return 0; }
void bitmask(int4 a0,int4 * a1,int4 * a2) { *a1=a0 >> 3; *a2=1 << (a0 & 7); }
int4 on_map(int4 a0,int4 a1) { return 0; }
int4 xrange(int4 a0) { return 0; }
int4 whose_territory(int4 a0,int4 a1,int4 a2,int4 * a3,int4 a4) { return -1; }
int4 sea_coasts(int4 a0) { return 0; }
int4 base_coast(int4 a0) { return 0; }
int4 transport_base(int4 a0) { return 0; }
int4 naval_base(int4 a0) { return 0; }
int4 alt_at(int4 a0,int4 a1) { return 0; }
int4 alt_detail_at(int4 a0,int4 a1) { return 0; }
int4 region_at(int4 a0,int4 a1) { return 0; }
int4 bit_at(int4 a0,int4 a1) { return 0; }
int4 is_coast(int4 a0,int4 a1,int4 a2) { return 0; }
int4 is_ocean(int4 a0,int4 a1) { return 0; }
int4 veh_who(int4 a0,int4 a1) { return 0; }
void clear() { StringTemp[0]=0; }
void says(const int1 * a0) { std::strcpy(StringTemp,a0); }
uint1 has_tech(int4 a0,int4 a1) { return 0; }
int4 planet_buster2(int4 a0) { return 0; }
int4 arm_strat(int4 a0,int4 a1) { return 0; }
int4 weap_strat(int4 a0,int4 a1) { return 0; }
int4 weap_val(int4 a0,int4 a1) { return 0; }
int4 arm_val(int4 a0,int4 a1) { return 0; }
int4 best_reactor(int4 a0) { return 1; }
int4 stack_fix(int4 a0) { return 0; }
int4 stack_check(int4 a0,int4 a1,int4 a2,int4 a3,int4 a4) { return 0; }
int4 veh_avail(int4 a0,int4 a1,int4 a2) { ++VehAvailCalls; return a0==StubAvailVehicle; }
int4 veh_at(int4 a0,int4 a1) { return 0; }
int4 has_abil(int4 a0,int4 a1) { return 0; }
int4 speed_proto(int4 a0) { return 0; }
int4 veh_cargo(int4 a0) { return 0; }
int4 veh_cost(int4 a0,int4 a1,int4 * a2) { return VehPrototypes[a0].cost; }
char *CharUpperA(char *s) { for (char *p=s;*p;++p) *p=static_cast<char>(std::toupper(static_cast<unsigned char>(*p))); return s; }
/*ACTUAL_BASE_BUILD*/
static Map host_tiles[64];
static void setup() {
  std::memset(Bases,0,sizeof(Bases)); std::memset(PlayersData,0,sizeof(PlayersData));
  std::memset(Players,0,sizeof(Players)); std::memset(VehPrototypes,0,sizeof(VehPrototypes));
  std::memset(host_tiles,0,sizeof(host_tiles));
  MapTiles=host_tiles; MapLongitude=4; MapLongitudeBounds=8; MapLatitudeBounds=8;
  MapAreaSqRoot=8; BaseCurrentCount=1; VehCurrentCount=0; BaseCurrentForcesSupported=0;
  Bases[0].x=2; Bases[0].y=2; Bases[0].faction_id_current=1; Bases[0].population_size=5;
  Bases[0].queue_production_id[0]=0; Bases[0].mineral_surplus=10; Bases[0].nutrient_surplus=5;
  Bases[0].mineral_intake_2=20; Bases[0].energy_intake_2=10;
  PlayersData[1].current_num_bases=1; PlayersData[1].region_total_bases[0]=1;
  PlayersData[1].energy_reserves=100; PlayersData[1].total_mil_veh=0;
  Rules.move_rate_roads=1;
  SetBaseCalls=ComputeCalls=VehAvailCalls=FacilityAvailCalls=0;
  for(int i=0;i<64;++i) SecretProject[i]=-1;
  for(int i=0;i<512;++i) { VehPrototypes[i].reactor_id=1; VehPrototypes[i].cost=1; }
  for(int i=0;i<134;++i) { Facility[i].cost=10; Facility[i].preq_tech=-1; }
}
int main() {
  setup(); int score=-123, second=-234, third=-345;
  int result=base_build(0,&second,&third,&score);
  std::printf("result=%d secondary=%d tertiary=%d score=%d calls=%d/%d/%d/%d\n",result,second,third,score,SetBaseCalls,ComputeCalls,VehAvailCalls,FacilityAvailCalls);
  assert(result==-69 && score==999999);
  assert(LastSetBaseId==0 && SetBaseCalls==1 && ComputeCalls==1);
  setup(); result=base_build(-1,0,0,&score);
  assert(LastSetBaseId==0 && SetBaseCalls==1 && ComputeCalls==1);
  assert(result==-69 && score==999999);
  std::printf("negative-mode result=%d score=%d\n",result,score);
  assert(second==99999 && third==99999);
  const int expected_results[]={-1,-2,-3,-4,-6,-9,-10,-27,-69,-68};
  const int expected_scores[]={21844,43688,87380,16384,76456,109224,0,27306,999999,54612};
  const int choices[]={1,2,3,4,6,9,10,27,28,68};
  for(unsigned i=0;i<sizeof(choices)/sizeof(choices[0]);++i) {
    setup(); StubAvailFacility=choices[i];
    result=base_build(0,&second,&third,&score);
    std::printf("facility=%d result=%d score=%d\n",choices[i],result,score);
    assert(result==expected_results[i] && score==expected_scores[i]);
    assert(SetBaseCalls==1 && ComputeCalls==1);
  }
  setup(); StubAvailFacility=-1; StubAvailVehicle=0;
  result=base_build(0,&second,&third,&score);
  assert(result==0 && score==36864);
  std::printf("prototype=0 result=%d score=%d\n",result,score);
}


