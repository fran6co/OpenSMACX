#include <assert.h>
#include <stdint.h>
#define __cdecl
enum { TRIAD_LAND = 0, TRIAD_SEA = 1, TRIAD_AIR = 2,
       PLAN_PLANET_BUSTER = 5, PLAN_ALIEN_ARTIFACT = 12,
       ABL_MARINE_DETACHMENT = 0x4000000, VSTATE_UNK_40 = 0x40 };
struct Veh { int proto_id, faction_id, order; uint32_t state;
    uint8_t visibility, dmg_incurred; };
struct VehPrototype { int chassis_id, weapon_id, plan, reactor_id, abilities; };
struct RulesChassis { int triad, missile; };
struct WeaponValue { int offense_rating; };
struct ArmorValue { int defense_rating; };
struct Map { uint8_t climate; uint32_t bit; };
struct TutWindow { int zoom_wait_pending_; };
struct ConsoleMock { int faction_id_; int focus(int, int, int) { return 1; } };
struct BattleMock { int set_mode, clears; void set_iface_mode() { ++set_mode; }
    void clear() { ++clears; } };
struct StatusMock { int draws, field_15D4_; /* ACTUAL_STATUS_SETTER */
    void draw(int,int,int,int,int) { ++draws; } };
struct RuleValues { int move_rate_roads; };
static Veh Vehs[4]; static VehPrototype VehPrototypes[80];
static RulesChassis Chassis[4]; static WeaponValue Weapon[4];
static ArmorValue Armor[4]; static Map target_map; static TutWindow TutWinBaseWindow;
static ConsoleMock ConsoleGlobal; static BattleMock g_BattleWin;
static StatusMock ConsoleStatusWin; static RuleValues Rules;
static uint32_t UnkBitfield1; static int IsMultiplayerNet;
static int VehDrawSelection689F28, VehDrawSelection689F2C;
static int planet_busting_calls, add_goal_calls, stack_veh_calls, stack_put_calls;
static int draw_calls, boom_calls, flush_calls, scoot_calls;
static int range(int v,int lo,int hi) { return v<lo?lo:v>hi?hi:v; }
static int proto_power(int id) { const VehPrototype &p=VehPrototypes[Vehs[id].proto_id];
    return p.plan==12?1:range(p.reactor_id,1,100)*10; }
static int speed(int id,int) { return id==0?1:3; }
static int has_abil(int proto,int ability) { return (VehPrototypes[proto].abilities&ability)!=0; }
static Map *map_loc(int,int) { return &target_map; }
static void add_goal(int,int,int,int,int,int) { ++add_goal_calls; }
static void draw_tile(int,int,int) { ++draw_calls; }
static void stack_veh(int,int) { ++stack_veh_calls; }
static void flush_input() { ++flush_calls; }
static short GetAsyncKeyState(int) { return 0; }
static void veh_scoot(int,int,int,int,int) { ++scoot_calls; }
static void stack_put(int,int,int) { ++stack_put_calls; }
static int stack_check(int,int,int,int,int) { return 1; }
static int boom(int,int,int) { ++boom_calls; return 0; }
int __cdecl planet_busting(int,int,int) { ++planet_busting_calls; return -37; }
