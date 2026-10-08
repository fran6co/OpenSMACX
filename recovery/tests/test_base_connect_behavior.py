"""Actual-source controls for the recovered base connection planner.

These host controls exercise the translated owner body with explicit map,
base, path, and territory doubles. They are not full pathfinding or ABI proof.
"""

from pathlib import Path
import shutil
import subprocess

import pytest


ROOT = Path(__file__).resolve().parents[2]


def body():
    source = (ROOT / "src/base.cpp").read_text()
    signature = "int __cdecl base_connect(int priority) {"
    start = source.index(signature)
    opening = source.index("{", start)
    depth = 0
    for end in range(opening, len(source)):
        depth += (source[end] == "{") - (source[end] == "}")
        if depth == 0:
            return source[start:end + 1]
    raise AssertionError("unterminated base_connect body")


HARNESS = r"""
#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#define __cdecl
typedef int BOOL;
enum { TRUE=1, FALSE=0, FAC_HEADQUARTERS=1, AI_GOAL_TERRAFORM_LAND=9,
       DTREATY_PACT=1, ALT_BIT_SHORE_LINE=0x60,
       ALT_BIT_OCEAN_SHELF=0x40, BIT_BASE_IN_TILE=1, BIT_VEH_IN_TILE=2,
       BIT_ROAD=4, BIT_MAGTUBE=8, BIT_FUNGUS=0x20,
       MPREF_AUTO_FORMER_BUILD_SENSORS=0x20,
       TERRA_MAGTUBE=6, TERRA_REMOVE_FUNGUS=10 };
struct Map { uint8_t climate, contour, val2, region, visibility, val3, unk;
             int8_t territory;
             uint32_t bit, bit2, visible[7]; };
struct Base { int16_t x,y; uint8_t faction_id_current,faction_id_former;
              int8_t population_size; uint8_t pad[0x85];
              uint8_t facilities_built[12]; };
struct PlayerData { uint32_t diplo_treaties[8]; };
struct RulesBasic { int tech_build_roads_fungus; } Rules;
struct RulesTerraforming { int name,name_sea,preq_tech,preq_tech_sea; uint32_t bit,bit_incompatible; int rate,shortcuts; };
RulesTerraforming Terraforming[20];
Base Bases[512]; Base *BaseCurrent; int BaseIDCurrentSelected;
// Minimal host owner only; this does not model native offsets or allocation.
struct HostGameDataLockMasks { uint32_t enable_mask_; };
struct HostGameDataFields {
  int base_current_count_;
  HostGameDataLockMasks lock_masks_;
  uint32_t game_more_preferences_;
};
struct HostGameDataStorage { HostGameDataFields fields_; };
HostGameDataStorage GameDataStateStorage{};
int &BaseCurrentCount = GameDataStateStorage.fields_.base_current_count_;
uint32_t &LockEnableMask = GameDataStateStorage.fields_.lock_masks_.enable_mask_;
uint32_t &GameMorePreferences = GameDataStateStorage.fields_.game_more_preferences_;
PlayerData PlayersData[8];
Map Tiles[64*64];
int MapLongitudeBounds=64, MapLatitudeBounds=64, MapIsFlat=1;
uint32_t MapLongitude=32;
const int VehMoveDeltaX[8]={1,2,1,0,-1,-2,-1,0};
const int VehMoveDeltaY[8]={-1,0,1,2,1,0,-1,-2};
Map *map_loc(int x,int y) { return &Tiles[y*32+(x>>1)]; }
int territory_override=-2;
int whose_territory(int,int x,int y,int*,int) {
  const int natural=map_loc(x,y)->territory;
  return territory_override==-2 ? natural : territory_override;
}
int tech_available;
int has_tech(int,int) { return tech_available; }
int zoc_sea(int,int,int) { return 0; }
bool has_fac_built(int id,int base) { return id==FAC_HEADQUARTERS && Bases[base].facilities_built[0]!=0; }
int goals, goal_x=-1, goal_y=-1, goal_faction=-1, goal_type=-1, goal_priority=-1, goal_base=-1;
void add_goal(int faction,int type,int priority,int x,int y,int base) {
  ++goals; goal_x=x; goal_y=y; goal_faction=faction; goal_type=type;
  goal_priority=priority; goal_base=base;
}
int path_calls, sensor_calls, next_direction=-1, sensor_result=1, sensor_x=0, sensor_y=0;
struct Path {
  int find(int,int,int,int,int,int,int,int) { ++path_calls; return next_direction; }
  int sensors(int,int *x,int *y) { ++sensor_calls; *x=sensor_x; *y=sensor_y; return sensor_result; }
} Paths;
"""


def test_base_connection_routes_and_sensor_fallback(tmp_path):
    compiler = shutil.which("g++")
    if compiler is None:
        pytest.skip("host g++ required")
    checks = r"""
    // Own adjacent-region base: choose the first valid route tile and keep
    // priority, faction, and selected-base arguments in the goal.
    BaseCurrent=&Bases[0]; BaseCurrentCount=2; BaseIDCurrentSelected=0;
    Bases[0].x=10; Bases[0].y=10; Bases[0].faction_id_current=1; Bases[0].population_size=2;
    Bases[1].x=14; Bases[1].y=10; Bases[1].faction_id_current=1; Bases[1].population_size=2;
    map_loc(10,10)->climate=0x60; map_loc(10,10)->region=3; map_loc(10,10)->territory=1;
    map_loc(14,10)->climate=0x60; map_loc(14,10)->region=3; map_loc(14,10)->territory=1;
    map_loc(12,10)->climate=0x60; map_loc(12,10)->region=3; map_loc(12,10)->territory=-1;
    next_direction=1; goals=path_calls=sensor_calls=0;
    assert(base_connect(2)==1 && goals==1 && goal_x==12 && goal_y==10);
    assert(goal_faction==1 && goal_type==AI_GOAL_TERRAFORM_LAND && goal_priority==2 && goal_base==0);
    assert(path_calls==1 && sensor_calls==0);

    // A road-only path square is blocked, but a magtube on a road is a valid
    // connection square once the faction has the magtube technology. The
    // native predicate permits that exception rather than rejecting every
    // road square before testing the magtube bit.
    map_loc(12,10)->bit=BIT_ROAD;
    tech_available=0; next_direction=1; goals=path_calls=sensor_calls=0;
    assert(base_connect(2)==0 && path_calls==2 && goals==0 && sensor_calls==1);
    map_loc(12,10)->bit=BIT_ROAD|BIT_MAGTUBE;
    tech_available=1; next_direction=1; goals=path_calls=sensor_calls=0;
    assert(base_connect(2)==1 && goals==1 && goal_x==12 && goal_y==10);
    tech_available=0; map_loc(12,10)->bit=0;

    // A human without the automatic-sensor preference does not issue a goal
    // when no path candidate exists and does not query sensors.
    BaseCurrentCount=1; LockEnableMask=2; GameMorePreferences=0;
    next_direction=-1; goals=path_calls=sensor_calls=0;
    assert(base_connect(4)==0 && goals==0 && path_calls==0 && sensor_calls==0);

    // The preference enables the same sensor fallback for the human.
    GameMorePreferences=MPREF_AUTO_FORMER_BUILD_SENSORS;
    sensor_result=0; sensor_x=18; sensor_y=22; goals=0;
    assert(base_connect(4)==1 && goals==1 && goal_x==18 && goal_y==22);
    assert(goal_faction==1 && goal_priority==4 && sensor_calls==1);

    // Foreign endpoint eligibility is pact-gated before pathfinding.
    BaseCurrentCount=2; LockEnableMask=0; GameMorePreferences=0;
    Bases[1].faction_id_current=2; Bases[1].x=14; Bases[1].y=10;
    PlayersData[1].diplo_treaties[2]=0; next_direction=1;
    path_calls=goals=sensor_calls=0;
    assert(base_connect(3)==1 && path_calls==0 && sensor_calls==1);
    PlayersData[1].diplo_treaties[2]=DTREATY_PACT;
    path_calls=goals=sensor_calls=0;
    assert(base_connect(3)==1 && path_calls==1 && goals==1);

    // A peer's population is compared with the destination's population, not
    // the source base's. The peer HQ remains eligible as a destination, while
    // it outranks the smaller non-HQ candidate and suppresses that route.
    BaseCurrentCount=3; Bases[1].faction_id_current=1; Bases[1].x=30;
    BaseCurrent->population_size=5; Bases[1].population_size=2;
    Bases[2].faction_id_current=1; Bases[2].x=40;
    Bases[2].y=10; Bases[2].population_size=3;
    Bases[2].facilities_built[0]=1;
    map_loc(30,10)->region=3; map_loc(40,10)->region=3;
    next_direction=-1; sensor_result=0;
    path_calls=goals=sensor_calls=0;
    assert(base_connect(3)==1 && path_calls==1 && sensor_calls==1);
    """
    source = HARNESS + "\n" + body() + "\nint main(){\n" + checks + "\nreturn 0;}\n"
    cpp = tmp_path / "base_connect.cpp"
    executable = tmp_path / "base_connect"
    cpp.write_text(source)
    compiled = subprocess.run([compiler, "-std=c++11", "-O1", "-fsanitize=undefined",
                               "-fno-sanitize-recover=undefined", str(cpp), "-o", str(executable)],
                              capture_output=True, text=True, timeout=30)
    assert compiled.returncode == 0, compiled.stderr
    result = subprocess.run([str(executable)], capture_output=True, text=True, timeout=10)
    assert result.returncode == 0, result.stderr
