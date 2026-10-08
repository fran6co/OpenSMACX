"""Host syntax/behavior controls for the tracked enemy_strategy source.

This extracts the actual candidate body at test time; the structs and services
below are deliberately small doubles, not target ABI/layout evidence.
"""
from __future__ import annotations

import os
import shutil
import subprocess
import tempfile
from pathlib import Path


ROOT = Path(os.environ.get("OPENSMACX_SOURCE_ROOT", Path(__file__).resolve().parents[2])).resolve()
BODY = ROOT / "src/faction_strategy.cpp"


STUBS = r'''
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#define __cdecl
static const int MaxPlayerNum=8, MapLongitudeBounds=32, MapLatitudeBounds=24;
static const int MapAbstractLongBounds=7, MapAbstractLatBounds=5;
static const int TRIAD_LAND=0, TRIAD_SEA=1, TRIAD_AIR=2;
static const int AI_GOAL_ATTACK=0, AI_GOAL_DEFEND=2, AI_GOAL_SCOUT=3;
static const int AI_GOAL_COLONIZE=8, AI_GOAL_TERRAFORM_LAND=9;
static const int AI_GOAL_PRIORITY_COMBAT=16, AI_GOAL_LANDING_SITE=41;
static const uint32_t BIT_BASE_IN_TILE=1u;

struct GameDataLockMaskView { uint32_t enable_mask_; };
struct GameDataStateWindowFields {
    uint32_t game_state_, game_preferences_; int diff_level_current_, turn_current_num_;
    int base_current_count_, vehicle_current_count_;
    GameDataLockMaskView lock_masks_; uint32_t faction_rankings_[8];
};
struct GameDataStateWindow { GameDataStateWindowFields fields_; };
static GameDataStateWindow GameDataStateStorage = {{0,0,0,40,0,0,{0},{0}}};
int &TurnCurrentNum = GameDataStateStorage.fields_.turn_current_num_;
int &BaseCurrentCount = GameDataStateStorage.fields_.base_current_count_;
int &VehCurrentCount = GameDataStateStorage.fields_.vehicle_current_count_;
int &DiffLevelCurrent = GameDataStateStorage.fields_.diff_level_current_;
uint32_t &GameState = GameDataStateStorage.fields_.game_state_;
uint32_t &LockEnableMask = GameDataStateStorage.fields_.lock_masks_.enable_mask_;
uint32_t &GamePreferences = GameDataStateStorage.fields_.game_preferences_;

int MapIsFlat=1, MapArea=1000;
struct Goal { int16_t type,priority; int x,y,base_id; };
struct PlayerData {
    int total_mil_veh; uint32_t current_num_bases; int mil_strength_1,mil_strength_2,pop_total;
    int unk_64,unk_65,unk_66,unk_68,unk_70,planet_busters,unk_71,unk_72;
    int unk_105,unk_106;
    uint16_t region_total_combat_vehs[128]; uint8_t region_total_bases[128];
    uint8_t region_total_offensive_vehs[128]; uint16_t region_force_rating[128];
    uint16_t unk_77[128],unk_78[128],unk_79[128],unk_80[128],unk_81[128];
    uint8_t unk_82[128],unk_83[128],region_base_plan[128];
    Goal goals[75],sites[25];
    int base_id_atk_target, energy_reserves,best_weapon_value,corner_market_turn;
    int ai_growth,ai_wealth,ai_fight;
    int x_target,y_target;
    uint32_t flags,diplo_treaties[8];
    uint8_t proto_id_active[512],proto_id_queue[512]; int16_t proto_id_lost[512];
};
struct Base {
    int16_t x,y; int8_t faction_id_current,faction_id_former;
    uint8_t visibility,population_size,nerve_staple_turns_left,queue_size;
    uint32_t state,event; int queue_production_id[16]; uint32_t facilities_built[8];
    int nutrient_surplus,mineral_surplus;
    int16_t unk_x,unk_y;
};
struct Map { uint8_t climate,contour,val2,region,visibility,val3,unk_1; int8_t territory; uint32_t bit,bit2; };
struct Veh {
    int16_t x,y; uint32_t state; uint16_t flags; int16_t proto_id,unk_1;
    uint8_t faction_id,year_end_lurking,dmg_incurred; int8_t order;
    uint8_t waypoint_count,patrol_current_point; int16_t waypoint_x[4],waypoint_y[4];
    uint8_t morale,terraforming_turns,order_auto_type,visibility,moves_expended;
    int8_t facing; uint8_t unk_6,move_to_ai_type,probe_action,probe_sabotage_id;
    int16_t home_base_id,next_veh_id_stack,prev_veh_id_stack;
};
struct VehPrototype { uint8_t chassis_id,weapon_id,armor_id,reactor_id,carry_capacity,cost; int8_t plan,unk_1; uint8_t obsolete_factions,combat_factions; int8_t icon_offset,padding; uint16_t flags; };
struct RulesChassis { uint8_t triad,missile; };
struct Continent { uint32_t tile_count,open_terrain,unk_3,pods,unk_5; uint8_t sea_coasts[8]; };
struct ConsoleType { int faction_id_; int upgrade(int) { return 1; } };
ConsoleType ConsoleGlobal;
PlayerData PlayersData[8]; Base Bases[128]; Map MapTiles[32*24]; Veh Vehs[128];
VehPrototype VehPrototypes[512]; RulesChassis Chassis[256]; Continent Continents[128];
uint32_t FactionRankings[8];
int goal_calls=0,wipe_calls=0,reset_calls=0;
int stack_check_result=1,veh_kill_calls=0,sleep_calls=0;
static inline Map *map_loc(int x,int y) { return &MapTiles[(x>>1)+y*16]; }
static const int VehMoveDeltaX[8]={0,1,1,1,0,-1,-1,-1};
static const int VehMoveDeltaY[8]={-1,-1,0,1,1,1,0,-1};
static int BaseFindDist=9999;
static inline int abstract_at(int x,int y) { return map_loc(x,y)->region; }
static inline int x_dist(int a,int b) { int d=abs(a-b); if (!(MapIsFlat&1) && d>16) d=32-d; return d; }
static inline int vector_dist(int x,int y) { x=abs(x); y=abs(y); int hi=x>y?x:y,lo=x<y?x:y; return hi-(((x+y)/2-lo+1)/2); }
static inline int base_at(int,int) { return -1; }
static inline int base_find(int,int) { return -1; }
static inline int base_find(int,int,int) { return -1; }
static inline int base_find(int,int,int,int,int,int) { return -1; }
static inline int garrison_check(int) { return 0; }
static inline int veh_at(int,int) { return -1; }
static inline int stack_fix(int x) { return x; }
static inline int bit_count(uint32_t x) { int n=0; while(x) { n += (x&1u)!=0; x >>= 1; } return n; }
static inline int stack_check(int,int,int,int,int) { return stack_check_result; }
static inline int is_coast(int,int,int) { return 0; }
static inline int terraform_cost(int,int,int) { return 0; }
static inline void base_reset(int,int) { ++reset_calls; }
static inline void bases_reset(int,int,int) { ++reset_calls; }
static inline void bitmask(int n,int *o,int *m) { *o=n/32; *m=1<<(n%32); }
static inline int is_objective(int) { return 0; }
static inline int want_monolith(int) { return 0; }
static inline int has_abil(int,int) { return 0; }
static inline int base_territory(int,int,int) { return -1; }
static inline int whose_territory(int,int,int,int,int) { return -1; }
static inline void del_site(int,int,int,int,int) {}
static inline int weap_val(int,int) { return 0; }
static inline int arm_val(int,int) { return 0; }
static inline int planet_buster(int) { return 0; }
static inline int veh_cargo(int) { return 0; }
static inline int weap_strat(int,int) { return 0; }
static inline int arm_strat(int,int) { return 0; }
static inline int morale_veh(int,int,int) { return 0; }
static inline int zoc_sea(int,int,int) { return 0; }
static inline int contiguous(int,int,int,int) { return 0; }
static inline int wants_to_attack(int,int,int) { return 0; }
static inline int great_satan(int,int) { return 0; }
static inline int world_site(int,int,int) { return 0; }
static inline void site_set(int,int,int) {}
static inline void add_goal(int,int,int,int,int,int) { ++goal_calls; }
static inline void wipe_goals(int) { ++wipe_calls; }
static inline void enemy_capabilities(int) {}
static inline void kill(int) {}
static inline void veh_kill(int) { ++veh_kill_calls; }
static inline void sleep(int) { ++sleep_calls; }
'''


def extract_body(text: str) -> str:
    start = text.index("void __cdecl enemy_strategy(int faction_id)")
    opening = text.index("{", start)
    depth = 0
    for i in range(opening, len(text)):
        if text[i] == "{":
            depth += 1
        elif text[i] == "}":
            depth -= 1
            if depth == 0:
                return text[start:i + 1]
    raise AssertionError("unclosed enemy_strategy body")


def run_case(compiler: str) -> None:
    body = extract_body(BODY.read_text())
    source = STUBS + "\n" + body + r'''
int main() {
    for (int i=0;i<8;i++) PlayersData[i].base_id_atk_target=-1;
    PlayersData[1].total_mil_veh=9;
    PlayersData[1].region_total_bases[7]=12;
    PlayersData[1].region_total_offensive_vehs[7]=13;
    PlayersData[1].region_total_offensive_vehs[0]=15;
    PlayersData[1].proto_id_queue[11]=14;
    enemy_strategy(1);
    if (PlayersData[1].total_mil_veh != 0) return 1;
    if (PlayersData[1].region_total_bases[7] != 0
        || PlayersData[1].region_total_offensive_vehs[7] != 0) return 2;
    if (PlayersData[1].region_total_offensive_vehs[0] != 15) return 5;
    if (PlayersData[1].proto_id_queue[11] != 0) return 3;
    if (wipe_calls != 1) return 4;
    return 0;
}
'''
    with tempfile.TemporaryDirectory(prefix="enemy-strategy-host-") as temp:
        src = Path(temp) / "candidate.cpp"
        exe = Path(temp) / "candidate"
        src.write_text(source)
        subprocess.run([compiler,"-std=c++98","-Wall","-Wextra","-Werror",
                        "-fsanitize=undefined","-fno-sanitize-recover=all",
                        str(src),"-o",str(exe)],check=True,timeout=60)
        subprocess.run([str(exe)],check=True,timeout=30)


def test_gcc_actual_candidate_body() -> None:
    compiler = shutil.which("g++")
    if compiler is None:
        import pytest
        pytest.skip("g++ unavailable")
    run_case(compiler)


def test_clang_actual_candidate_body() -> None:
    compiler = shutil.which("clang++")
    if compiler is None:
        import pytest
        pytest.skip("clang++ unavailable")
    run_case(compiler)
