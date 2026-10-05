// Bounded host control declaration surface, never application source.
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <cassert>
#include <cstdlib>
#define __cdecl
#define FALSE 0
typedef char *LPSTR;
typedef const char *LPCSTR;
enum { ORDER_NONE=0, ORDER_MOVE_TO=24, ORDER_SENTRY_BOARD=1,
 PLAN_NAVAL_TRANSPORT=7, PLAN_COLONIZATION=8, PLAN_TERRAFORMING=9,
 PLAN_SUPPLY_CONVOY=10, PLAN_INFO_WARFARE=11, PLAN_ALIEN_ARTIFACT=12,
 TRIAD_LAND=0, TRIAD_SEA=1, TRIAD_AIR=2, ABL_AMPHIBIOUS=8,
 ABL_CARRIER=128, BIT_BASE_IN_TILE=1, BIT_AIRBASE=0x40000,
 BIT_BUNKER=0x800, BIT_FUNGUS=32, BIT_MONOLITH=8192,
 STATE_OMNISCIENT_VIEW=128, VSTATE_UNK_4=4, VSTATE_UNK_8=8, RFLAG_ALIEN=128,
 VK_SHIFT=16, VK_CONTROL=17, MaxPlayerNum=8 };
struct Veh {
 int16_t x,y; uint32_t state; uint16_t flags; int16_t proto_id;
 uint16_t unk_1; uint8_t faction_id,year_end_lurking,dmg_incurred;
 int8_t order; uint8_t waypoint_count,patrol_current_point;
 int16_t waypoint_x[4],waypoint_y[4];
 uint8_t morale,terraforming_turns,order_auto_type,visibility,moves_expended;
 int8_t facing; uint8_t unk_6,move_to_ai_type,probe_action,probe_sabotage_id;
 int16_t home_base_id,next_veh_id_stack,prev_veh_id_stack;
};
struct VehPrototype { char veh_name[32]; uint32_t ability_flags;
 uint8_t chassis_id,weapon_id,armor_id,reactor_id,carry_capacity,cost,plan;
 uint16_t flags; };
struct Effect { int planet; };
struct PlayerData { uint32_t diplo_treaties[32],flags; int diff_level,planet_ecology;
 Effect soc_effect_active,soc_effect_base; uint8_t proto_id_active[512];
 uint16_t unk_77[128],region_total_combat_vehs[128]; };
struct Player { char *adj_name_faction,*noun_faction,*title_leader,*name_leader;
 uint8_t is_leader_female,noun_gender,is_noun_plural; uint32_t rule_flags;
 int faction_bonus_count,faction_bonus_id[16],faction_bonus_val1[16]; };
struct RulesChassis { uint8_t triad,range; char *offsv_1_name; };
struct RulesWeapon { int8_t offense_rating; int mode; };
struct RulesAbility { char *name; };
struct Base { int faction_id_current,population_size,x,y,minerals_accumulated,
 minerals_accumulated_2,production_id_last,eco_damage; uint32_t state;
 uint8_t facilities_built[12]; int queue_production_id[10],visibility;
 char *name_string; };
struct RulesFacility { char *name; int cost,preq_tech; };
struct Map { uint8_t climate,region,val2,visibility; uint32_t bit; };
struct GraphicWin {};
struct Sprite { int init(...) { return 1; } };
struct Buffer { static int get_pcx_dimensions(...) { return 1; } };
struct Dialogs { void item(...) {} };
static int PopupConstructions,PopupDestructions;
struct Popup { Sprite picture; Dialogs rows;
 Popup() { ++PopupConstructions; } ~Popup() { ++PopupDestructions; }
 Sprite &owned_picture() { return picture; }
 Dialogs &dialogs() { return rows; } int exec(...) { return 0; }
 void start(...) {} void set_caller_picture(...) {} };
struct Console { int faction_id_,selected_vehicle_,field_23BE4_; uint32_t field_23BF0_;
 uint32_t &vehicle_order_ui_state() { return field_23BF0_; }
 void focus(...) {} void cursor_next(...) {} };
struct LockEntry { uint32_t leading_words_[4]; };
struct NetDaemon { LockEntry lock_state_[2]; void await_synch() {} };
struct Tutorial { int tut_map(const char*,int,int,int,Sprite*,int,int,int); bool tutorial_blocked() { return false; } };
struct TutorialBase { void check_base(...) {} };
struct NetMsg { void pop(...) {} };
struct FX { void play(...) {} void engine_pitch(...) {} };
struct RulesType { int move_rate_roads; };
struct Projects { int xenoempathy_dome,universal_translator; };
extern Veh Vehs[16]; extern VehPrototype VehPrototypes[512];
extern PlayerData PlayersData[8]; extern Player Players[8];
extern RulesChassis Chassis[16]; extern RulesWeapon Weapon[32];
extern RulesAbility Ability[32]; extern Base Bases[16];
extern RulesFacility Facility[128]; extern Map MapTiles[4096];
extern Console ConsoleGlobal; extern NetDaemon NetDaemonNet;
extern Tutorial TutorialWindow; extern TutorialBase TutWinBaseWindow;
extern NetMsg NetMsgPrimary; extern FX g_FX; extern RulesType Rules;
extern Projects SecretProject;
extern char StringTemp[4096],ScriptPopupFile[64],PopupStartCaption[256],*TextBufferGetPtr;
extern uint32_t GamePreferences,GameMorePreferences,GameRules,GameState,LockEnableMask;
extern uint32_t VehOrderDelayCounter,MapNativeLifeForms;
extern int OrderVehTutorialNoticeCounter;
extern int ExpansionEnabled,ConsoleControlTurnActive,IsMultiplayerNet,TurnCurrentNum,
 BaseCurrentCount,BaseFindDist,MapLongitude,MapLatitudeBounds,MapLongitudeBounds,
 MapIsFlat,VehSelectionFollowLimit,*ConsoleGroupCount,GenderDefault,PluralityDefault;
extern int RadiusOffsetX[289],RadiusOffsetY[289],RadiusBaseX[9],RadiusBaseY[9],
 VehMoveDeltaX[9],VehMoveDeltaY[9];
int GetAsyncKeyState(int);
typedef int (*KeyState)(int);
KeyState ConsoleEditKeyStateSlot();
Map *map_loc(int,int);
int xrange(int); bool on_map(int,int);
char *text_get();
int random(int,int);
int speed(int,int);
int action_home(int,int);
int zoc_veh(int,int,int);
int veh_cargo(int);
void add_goal(int,int,int,int,int,int);
void monolith(int);
void veh_skip(int);
void spot_stack(int,int);
int break_treaty(int,int,int);
int popp(const char*,const char*,int,const char*,int(*)());
void bitmask(int,int*,int*);
int parse_say(int,int,int,int); int parse_says(int,const char*,int,int);
int parse_num(...),parse_string(...),X_text_open(...),X_pop(...),
 has_abil(...),stack_check(...),veh_at(...),stack_fix(...),base_at(...),base_find(...),
 goody_at(...),veh_cost(...),cost_factor(...),supply_options(...),is_sensor(...),
 whose_territory(...),abil_index(...),quick_zoc(...),zoc_move(...),probe(...),
 clock_wait(...),veh_init(...),battle_fight(...),tut_check(...),
 hex_cost(...),has_tech(...),is_objective(...),eliminate_player(...),
 can_arty(...),study_artifact(...),goody_box(...),
 morale_alien(...),zoc_any(...),log_say(...);
void draw_tile(...),synch_veh(...),spot_loc(...),
 draw_radius(...),veh_scoot(...),stack_put(...),draw_tiles(...),veh_kill(...),
 bit_set(...),owner_set(...),set_fac(...),kill(...),base_kill(...),draw_map(...),
 act_of_aggression(...),capture_base(...),veh_demote(...),spot_all(...),sleep(...),
 unspot_stack(...),boom(...),draw_tile_fixup(...);
int stack_veh(...);
