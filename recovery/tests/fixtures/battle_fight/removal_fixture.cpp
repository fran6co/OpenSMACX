#include <assert.h>
#include <stdint.h>
struct Veh { int prev_veh_id_stack,next_veh_id_stack,proto_id,dmg_incurred,faction_id; };
struct VehPrototype { int plan,reactor_id; };
Veh Vehs[16]; VehPrototype VehPrototypes[16];
enum { PLAN_ALIEN_ARTIFACT=12 };
static int killed[16], killed_count;
static int range(int v,int lo,int hi) { return v<lo?lo:v>hi?hi:v; }
static int battle_kill(int id,int*,int*,int*,int,int) { killed[killed_count++]=id; return 0; }
static int veh_at(int,int) { return -1; }
static int stack_fix(int id) { return id; }
static int stack_check(int,int,int,int,int) { return 0; }
static void battle_kill_stack(int,int*,int*,int*,int,int) {}
enum BattleFightRemovalRouteV22 { BATTLE_FIGHT_REMOVAL_ROUTE_AC8B_V22=0 };
