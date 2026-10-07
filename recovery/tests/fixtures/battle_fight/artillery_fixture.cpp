#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#define __cdecl
enum { PLAN_ALIEN_ARTIFACT=12, PLAN_TERRAFORMING=9,
       TRIAD_LAND=0, TRIAD_SEA=1, TRIAD_AIR=2,
       ABL_AIR_SUPERIORITY=0x20, BSTATE_UNK_200000=0x200000,
       BEVENT_UNK_100=0x100, VSTATE_UNK_4=4, VSTATE_UNK_8=8,
       VSTATE_UNK_200=0x200, VSTATE_EXPLORE=0x4000,
       VSTATE_UNK_1000000=0x1000000, VSTATE_UNK_2000000=0x2000000,
       STATE_OMNISCIENT_VIEW=0x80 };
struct RulesBasic { uint32_t artillery_dmg_num,artillery_dmg_denom,
    max_pct_dmg_art_vs_unit_base_bnkr,max_pct_dmg_art_vs_units_open,
    max_pct_dmg_art_vs_units_sea; };
struct Veh { int16_t proto_id; uint32_t state; uint16_t flags;
    uint8_t dmg_incurred,visibility,terraforming_turns; int8_t order;
    int16_t next_veh_id_stack,prev_veh_id_stack; };
struct VehPrototype { uint8_t chassis_id,reactor_id,plan; uint32_t ability_flags; };
struct RulesChassis { uint8_t triad; };
struct Map { uint32_t bit; };
struct PlayerData { uint32_t diplo_treaties[8]; int diplo_unk3[8],diplo_unk4[8]; };
struct Base { uint32_t state,event; char name_string[25]; };
struct TestConsole { int faction_id_; int focus(int,int,int) { return 1; } };
struct TestBattleWin { void set_iface_mode() {} void clear() {} void stop_timer() {} void pulse_timer() {} };
struct TestStatusWin { int draws,field_15D4_,stores; TestStatusWin():draws(0),field_15D4_(0),stores(0){}
    void draw(int,int,int,int,int){++draws;}
    /* ACTUAL_STATUS_SETTER */ };
static int status_flushes;
static RulesBasic Rules; static Veh Vehs[8]; static VehPrototype VehPrototypes[8];
static RulesChassis Chassis[8]; static Map TestTile; static PlayerData PlayersData[8];
static Base Bases[8]; static TestConsole ConsoleGlobal; static TestBattleWin g_BattleWin;
static TestStatusWin ConsoleStatusWin;
struct TestGameDataStateFields {
    uint32_t game_state_, game_more_preferences_, game_preferences_;
    struct { uint32_t enable_mask_; } lock_masks_;
};
struct TestGameDataState { TestGameDataStateFields fields_; };
static TestGameDataState GameDataStateStorage;
static int IsMultiplayerNet,ConsoleGroupAction,VehDrawSelection689F28,VehDrawSelection689F2C;
static char *ScriptPopupFile=0; static int target_stack_root,draw_count,goal_calls,compute_calls,skip_id,fixups;
static int range(int v,int lo,int hi){return v<lo?lo:v>hi?hi:v;}
static Map *map_loc(int,int){return &TestTile;}
static int stack_fix(int id){return id;}
static int veh_at(int,int){return target_stack_root;}
static void stack_veh(int,int){}
static void stack_kill(int id){Vehs[id].dmg_incurred=0;}
static void draw_tile_fixup(int,int,int,int){++fixups;}
static void base_reset(int,int){}
static int has_abil(int p,int a){return (VehPrototypes[p].ability_flags&static_cast<uint32_t>(a))!=0;}
static void add_goal(int,int,int,int,int,int){++goal_calls;}
static void veh_skip(int id){skip_id=id;}
static void battle_compute(int,int,int *a,int *d,int){++compute_calls;*a=10;*d=1;}
static void draw_tile(int,int,int){++draw_count;}
static void flush_input(){assert(ConsoleStatusWin.field_15D4_==1);assert(ConsoleStatusWin.draws==ConsoleStatusWin.stores*2+1);++ConsoleStatusWin.stores;++status_flushes;}
static int GetAsyncKeyState(int){return 0;}
static int popp(const char*,const char*,int,const char*,int){return 1;}
static void prefs_save(int){}
static void clock_wait(int){}
static int boom(int,int,int){return 0;}
static int atrocity_calls;
void atrocity(int,int,int,int){++atrocity_calls;}
static int battle_fight_artillery_cleanup(int,int,int,int,int);
