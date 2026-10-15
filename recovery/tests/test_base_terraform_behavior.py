"""Actual-source controls for the translated base_terraform planner."""

from pathlib import Path
import shutil
import subprocess
import re

import pytest


ROOT = Path(__file__).resolve().parents[2]


def mask_declarations():
    header = (ROOT / "src/game_data_state_window.h").read_text(encoding="utf-8")
    match = re.search(
        r"struct GameDataLockMaskBytes\s*\{.*?\};\s*union GameDataLockMaskView\s*\{.*?\};",
        header, re.DOTALL,
    )
    assert match, "production lock-mask byte/dword declarations missing"
    return match.group(0)


def body():
    source = (ROOT / "src/base.cpp").read_text()
    signature = "void __cdecl base_terraform(int force_improve) {"
    start = source.index(signature)
    opening = source.index("{", start)
    depth = 0
    for end in range(opening, len(source)):
        depth += (source[end] == "{") - (source[end] == "}")
        if depth == 0:
            return source[start:end + 1]
    raise AssertionError("unterminated base_terraform body")


HARNESS = r"""
#include <cassert>
#include <cstdint>
#include <cstdlib>
#define __cdecl
typedef int BOOL;
enum { TRUE=1, FALSE=0, ALT_BIT_SHORE_LINE=0x60,
       ALT_BIT_OCEAN_SHELF=0x40, BSTATE_UNK_1000=0x1000,
       BSTATE_UNK_2000=0x2000, BSTATE_UNK_4000=0x4000,
       AI_GOAL_TERRAFORM_WATER=25, AI_GOAL_TERRAFORM_LAND=9,
       AI_GOAL_SENSOR_ARRAY=121, AI_GOAL_LANDING_SITE=41,
       AI_GOAL_CONDENSER=73, AI_GOAL_THERMAL_BOREHOLE=105,
       TERRA_CONDENSER=12, TERRA_THERMAL_BORE=14,
       RFLAG_AQUATIC=0x100,
       TERRA_MAGTUBE=6, TERRA_REMOVE_FUNGUS=10,
       BIT_FUNGUS=0x20, BIT_SOLAR_TIDAL=0x40,
       BIT_MINE=0x10, BIT_FARM=0x8000, BIT_FOREST=0x200000,
       BIT_CONDENSER=0x400000, BIT_ECH_MIRROR=0x800000,
       BIT_THERMAL_BORE=0x1000000, BIT_MONOLITH=0x2000,
       BIT_VEH_IN_TILE=0x2, BIT_ROAD=0x4, MaxPlayerNum=8 };
struct Base {
  int16_t x,y;
  uint8_t faction_id_current,faction_id_former;
  int8_t population_size;
  uint8_t nerve_staple_turns_left, unk_1, visibility;
  char name_string[25];
  int16_t unk_x,unk_y;
  uint32_t state,event,governor;
  int nutrients_accumulated,minerals_accumulated,production_id_last,eco_damage;
  uint32_t queue_size;
  int queue_production_id[10];
  int resource_sqr_radius,specialist_total,specialist_unk_1;
  uint32_t spec[2]; uint8_t facilities_built[12];
  int mineral_surplus_final,minerals_accumulated_2,unk_6,unk_7,unk_8,unk_9;
  int nutrient_intake_1,mineral_intake_1,energy_intake_1,unused_intake_1;
  int nutrient_intake_2,mineral_intake_2,energy_intake_2,unused_intake_2;
  int nutrient_surplus,mineral_surplus,energy_surplus,unused_surplus;
  int nutrient_inefficiency,mineral_inefficiency,energy_inefficiency,unused_inefficiency;
};
struct Map { uint8_t climate,contour,val2,region,visibility,val3,unk_1; int8_t territory;
             uint32_t bit,bit2,bit_visible[7]; };
struct PlayerData { uint32_t diplo_treaties[32]; int ai_growth,ai_wealth,ai_power,ai_fight;
                    int tech_fungus_nutrient,tech_fungus_mineral,tech_fungus_energy,
                    net_random_event; };
struct Player { uint32_t rule_flags; };
struct ConsoleGlobalType { int faction_id_; } ConsoleGlobal;
struct RulesBasic { int tech_three_nutrients_sqr,tech_build_roads_fungus,
                    tech_improve_fungus_sqr; } Rules;
struct RulesTerraforming { int name,name_sea,preq_tech,preq_tech_sea; uint32_t bit,bit_incompatible; int rate,shortcuts; };
struct RulesAbility { int name,preq_tech; } Ability[29];
struct Continent { uint32_t tile_count,open_terrain,unk_3,pods,unk_5; uint8_t sea_coasts[8]; };
Base Bases[512], *BaseCurrent; int BaseIDCurrentSelected;
// @@MASK_TYPES@@
struct HostGameDataFields {
  int base_current_count_;
  GameDataLockMaskView lock_masks_;
  uint32_t game_more_preferences_;
  uint32_t game_preferences_;
  int turn_current_num_;
};
struct HostGameDataStorage { HostGameDataFields fields_; };
HostGameDataStorage GameDataStateStorage{};
int &BaseCurrentCount = GameDataStateStorage.fields_.base_current_count_;
uint32_t &LockEnableMask = GameDataStateStorage.fields_.lock_masks_.enable_mask_;
uint32_t &GameMorePreferences = GameDataStateStorage.fields_.game_more_preferences_;
uint32_t &GamePreferences = GameDataStateStorage.fields_.game_preferences_;
int &TurnCurrentNum = GameDataStateStorage.fields_.turn_current_num_;
PlayerData PlayersData[32]; Player Players[32]; Map Tiles[64*32]; Continent Continents[128];
RulesTerraforming Terraforming[20];
int MapLongitudeBounds=64,MapLatitudeBounds=64; uint32_t MapLongitude=32; int MapIsFlat=1;
uint32_t GameState; int BaseSquareEnergy;
int ExpansionEnabled;
uint32_t timeGetTime() { return 0; }
uint32_t MapCloudCover;
const int RadiusOffsetX[21]={0,1,2,1,0,-1,-2,-1,0,2,2,-2,-2,1,3,3,1,-1,-3,-3,-1};
const int RadiusOffsetY[21]={0,-1,0,1,2,1,0,-1,-2,2,-2,2,-2,3,3,1,-1,-3,-3,-1,-3};
const int RadiusRange[9]={0,1,9,21,37,57,81,109,141};
const int VehMoveDeltaX[9]={1,2,1,0,-1,-2,-1,0,0};
const int VehMoveDeltaY[9]={0,0,1,2,1,0,-1,-2,0};
Map *map_loc(int x,int y) { return &Tiles[y*32+(x>>1)]; }
int territory_calls,sensor_calls,goal_calls,action_goal_calls,landing_goal_calls,condenser_goal_calls,
    thermal_goal_calls,thermal_goal_x,thermal_goal_y;
int action_goal_x,action_goal_y,action_goal_type;
int whose_territory(int faction,int,int,int*,BOOL) { ++territory_calls; return faction; }
int sensor_value=1;
int is_sensor(int,int) { ++sensor_calls; return sensor_value; }
int vector_dist(int x,int y) { return (x+y)>>1; }
int radius_move(int,int,int,int,int) { return -1; }
int bonus_value;
int bonus_at(int,int,int) { return bonus_value; }
int tech_value;
int has_tech(int,int) { return tech_value; }
void bitmask(int input,int *offset,int *mask) {
  *offset=input/8; *mask=1<<(input&7);
}
int zoc_value=-1;
int zoc_move(int,int,int) { return zoc_value; }
int crop_value;
int crop_yield(int,int,int,int,int) { return crop_value; }
int terraform_allowed=1,terraform_result=4,goal_type,goal_priority,goal_x,goal_y,goal_base;
int can_terraform(int,int,int,int,int) { return terraform_allowed ? terraform_result : 0; }
int site_terrain_available;
int thermal_available=1,condenser_available=1,landing_available=1;
int terrain_avail(int id,int,int) {
  if (!site_terrain_available) return 0;
  if (id==TERRA_THERMAL_BORE) return thermal_available;
  if (id==TERRA_CONDENSER) return condenser_available;
  if (id==16) return landing_available;
  return 1;
}
void add_goal(int,int type,int priority,int x,int y,int base) {
  ++goal_calls;
  if (type==AI_GOAL_TERRAFORM_LAND || type==AI_GOAL_TERRAFORM_WATER ||
      type==AI_GOAL_SENSOR_ARRAY) {
    ++action_goal_calls; action_goal_x=x; action_goal_y=y; action_goal_type=type;
  }
  if (type==AI_GOAL_LANDING_SITE) ++landing_goal_calls;
  if (type==AI_GOAL_CONDENSER) ++condenser_goal_calls;
  if (type==AI_GOAL_THERMAL_BOREHOLE) {
    ++thermal_goal_calls; thermal_goal_x=x; thermal_goal_y=y;
  }
  goal_type=type;goal_priority=priority;goal_x=x;goal_y=y;goal_base=base;
}
int base_connect(int) { return 0; }
"""


def test_base_priority_flags_and_native_radius_scan(tmp_path):
    compiler = shutil.which("g++")
    if compiler is None:
        pytest.skip("host g++ required")
    checks = r"""
    BaseCurrent=&Bases[0]; BaseCurrentCount=1; BaseIDCurrentSelected=0;
    BaseCurrent->x=20; BaseCurrent->y=20; BaseCurrent->faction_id_current=2;
    BaseCurrent->population_size=3; BaseCurrent->energy_intake_1=0;
    BaseCurrent->mineral_intake_1=0; BaseCurrent->nutrient_surplus=-1;
    BaseCurrent->state=BSTATE_UNK_1000|BSTATE_UNK_2000;
    for (int i=0;i<21;i++) {
      const int x=BaseCurrent->x+RadiusOffsetX[i];
      const int y=BaseCurrent->y+RadiusOffsetY[i];
      map_loc(x,y)->climate=ALT_BIT_SHORE_LINE;
      map_loc(x,y)->territory=2;
    }
    territory_calls=sensor_calls=goal_calls=0;
    base_terraform(0);
    // Native computes need-based bits before visiting its fixed 21-square
    // radius; negative nutrient surplus selects the third state bit.
    assert((BaseCurrent->state&BSTATE_UNK_1000)==0);
    assert((BaseCurrent->state&BSTATE_UNK_2000)!=0);
    assert((BaseCurrent->state&BSTATE_UNK_4000)!=0);
    assert(territory_calls==41 && sensor_calls==41);
    assert(goal_calls==0);

    // Human governor flags suppress AI defaults and retain only the selected
    // priority branch. With nutrient surplus restored, the threshold bit is
    // cleared on this invocation rather than sticky from the prior call.
    LockEnableMask=1u<<2; BaseCurrent->governor=0x04000000;
    BaseCurrent->nutrient_surplus=10;
    territory_calls=sensor_calls=0;
    base_terraform(0);
    assert((BaseCurrent->state&BSTATE_UNK_4000)==0);
    assert(territory_calls==41 && sensor_calls==41);

    // The separate expansion pass ranks eligible shelf squares, consults
    // continent size, and records a water-terraformation objective only for
    // a candidate in the source base's region.
    BaseCurrent->x=20;BaseCurrent->y=20;
    map_loc(20,20)->region=4;
    map_loc(21,19)->climate=ALT_BIT_OCEAN_SHELF;
    map_loc(21,19)->region=4;
    Continents[4].tile_count=100;
    terraform_allowed=1;bonus_value=0;goal_calls=0;
    base_terraform(0);
    assert(goal_calls==1 && goal_type==AI_GOAL_TERRAFORM_WATER);
    assert(goal_priority==2 && goal_x==21 && goal_y==19 && goal_base==0);

    // With productive land yield enabled, the native shared farm/solar score
    // selects the earliest eligible former square and emits its land goal.
    for (int i=0;i<21;i++) {
      const int x=BaseCurrent->x+RadiusOffsetX[i];
      const int y=BaseCurrent->y+RadiusOffsetY[i];
      map_loc(x,y)->region=4;
    }
    crop_value=3; goal_calls=0; terraform_allowed=1;
    base_terraform(0);
    assert(goal_calls==2 && goal_type==AI_GOAL_TERRAFORM_LAND);
    assert(goal_priority==2 && goal_x==22 && goal_y==20 && goal_base==0);

    sensor_value=1; Players[2].rule_flags=0; ExpansionEnabled=0;
    crop_value=3; goal_calls=0;
    base_terraform(1);
    assert((map_loc(22,20)->bit&(BIT_FARM|BIT_ROAD|BIT_MINE))==
           (BIT_FARM|BIT_ROAD|BIT_MINE));

    map_loc(22,20)->bit=0; terraform_result=6;
    base_terraform(1);
    assert((map_loc(22,20)->bit&(BIT_MINE|BIT_ROAD|BIT_FARM))==
           (BIT_MINE|BIT_ROAD|BIT_FARM));
    map_loc(22,20)->bit=0; terraform_result=7;
    base_terraform(1);
    assert((map_loc(22,20)->bit&(BIT_SOLAR_TIDAL|BIT_ROAD))==
           (BIT_SOLAR_TIDAL|BIT_ROAD));
    map_loc(22,20)->bit=0; terraform_result=8;
    base_terraform(1);
    assert((map_loc(22,20)->bit&BIT_FOREST)!=0);

    // The aquatic expansion-improvement selector seeds from the selected
    // base ID (not the current turn number). With time=0 and coordinates
    // summing to 42, base ID 2 yields choice 4; TurnCurrentNum 0 would
    // incorrectly select the forest case instead.
    map_loc(22,20)->bit=0; terraform_result=4;
    ExpansionEnabled=1; Players[2].rule_flags=RFLAG_AQUATIC;
    BaseIDCurrentSelected=2; TurnCurrentNum=0;
    base_terraform(1);
    assert((map_loc(22,20)->bit&(BIT_FARM|BIT_ROAD|BIT_MINE))==
           (BIT_FARM|BIT_ROAD|BIT_MINE));
    assert((map_loc(22,20)->bit&BIT_FOREST)==0);
    BaseIDCurrentSelected=0; ExpansionEnabled=0;

    // With no sensor coverage and no productive crop alternative, the native
    // local sensor score wins and dispatches the sensor-array goal.
    sensor_value=0; crop_value=0; Players[2].rule_flags=RFLAG_AQUATIC;
    LockEnableMask=0;
    goal_calls=0;
    base_terraform(1);
    assert(goal_calls==2 && goal_type==AI_GOAL_SENSOR_ARRAY);
    assert(goal_priority==2 && goal_x>=21 && goal_y>=19 && goal_base==0);

    // The original tests only LockEnableMask's low byte. A faction bit in
    // the next byte must therefore take the AI-priority path, not the human
    // governor path, even though the corresponding full-width bit is set.
    BaseCurrent->faction_id_current=8; BaseCurrent->population_size=3;
    PlayersData[8].ai_growth=4; PlayersData[8].ai_wealth=0;
    PlayersData[8].ai_power=0; BaseCurrent->governor=0x04000000;
    LockEnableMask=1u<<8; BaseCurrent->state=0;
    base_terraform(0);
    assert((BaseCurrent->state&BSTATE_UNK_1000)!=0);
    assert((BaseCurrent->state&BSTATE_UNK_2000)==0);

    BaseCurrent->faction_id_current=2; LockEnableMask=0;
    for (int i=0;i<21;i++) {
      const int x=BaseCurrent->x+RadiusOffsetX[i];
      const int y=BaseCurrent->y+RadiusOffsetY[i];
      map_loc(x,y)->climate=0x70;
    }
    map_loc(12,20)->climate=ALT_BIT_SHORE_LINE;
    base_terraform(0);
    assert(BaseCurrent->unk_x==12 && BaseCurrent->unk_y==20);

    sensor_value=1; crop_value=0; LockEnableMask=0;
    for (int i=0;i<21;i++) {
      const int x=BaseCurrent->x+RadiusOffsetX[i];
      const int y=BaseCurrent->y+RadiusOffsetY[i];
      map_loc(x,y)->climate=ALT_BIT_SHORE_LINE;
      map_loc(x,y)->bit=0;
      map_loc(x,y)->region=4;
    }
    for (int i=2;i<6;i++) {
      const int x=BaseCurrent->x+RadiusOffsetX[i];
      const int y=BaseCurrent->y+RadiusOffsetY[i];
      map_loc(x,y)->bit=BIT_FOREST;
    }
    site_terrain_available=1; thermal_available=0; goal_calls=0;
    landing_goal_calls=condenser_goal_calls=thermal_goal_calls=0;
    base_terraform(0);
    assert(landing_goal_calls==0 && condenser_goal_calls==1 &&
           thermal_goal_calls==0);
    assert(goal_type==AI_GOAL_CONDENSER);
    assert(goal_priority==1 && goal_x>=21 && goal_y>=19 && goal_base==0);

    // Native tail prioritizes an eligible thermal objective over condenser;
    // when both are unavailable it falls through to the landing candidate.
    TurnCurrentNum=201; thermal_available=1; goal_calls=0;
    condenser_goal_calls=thermal_goal_calls=landing_goal_calls=0;
    base_terraform(0);
    assert(thermal_goal_calls==1 && condenser_goal_calls==0 &&
           landing_goal_calls==0 && goal_type==AI_GOAL_THERMAL_BOREHOLE);
    thermal_available=0; condenser_available=0; goal_calls=0;
    thermal_goal_calls=condenser_goal_calls=landing_goal_calls=0;
    base_terraform(0);
    assert(landing_goal_calls==1 && condenser_goal_calls==0 &&
           thermal_goal_calls==0 && goal_type==AI_GOAL_LANDING_SITE);
    thermal_available=1;

    // Thermal scoring gives the rocky-2/rainfall-2 square its native 24-point
    // terrain bonus, exceeding a rocky-0/rainfall-0 competitor. This catches
    // reversing the signed-mask term in the original score expression.
    map_loc(22,20)->climate=0x70;
    map_loc(22,20)->val3=0x80;
    map_loc(21,19)->climate=0x60;
    map_loc(21,19)->val3=0;
    thermal_goal_calls=0;
    base_terraform(0);
    assert(thermal_goal_calls==1 && thermal_goal_x==22 && thermal_goal_y==20);

    // Two eligible altitude-level-5 tiles suppress landing ranking globally.
    thermal_available=0; condenser_available=0; bonus_value=0;
    for (int i=0;i<21;i++) {
      const int x=BaseCurrent->x+RadiusOffsetX[i];
      const int y=BaseCurrent->y+RadiusOffsetY[i];
      map_loc(x,y)->climate=ALT_BIT_SHORE_LINE;
      map_loc(x,y)->bit=0;
      map_loc(x,y)->region=4;
    }
    map_loc(BaseCurrent->x+RadiusOffsetX[1],BaseCurrent->y+RadiusOffsetY[1])->climate=0xA0;
    map_loc(BaseCurrent->x+RadiusOffsetX[2],BaseCurrent->y+RadiusOffsetY[2])->climate=0xA0;
    for (int i=2;i<6;i++)
      map_loc(BaseCurrent->x+RadiusOffsetX[i],BaseCurrent->y+RadiusOffsetY[i])->bit=BIT_FOREST;
    landing_goal_calls=0;
    base_terraform(0);
    assert(landing_goal_calls==0);
    bonus_value=0;

    // Native zoc_move result 1 is an unconditional rejection; it is not the
    // no-zoc sentinel. This ensures that edge case cannot produce a former goal.
    zoc_value=1; action_goal_calls=0;
    base_terraform(0);
    assert(action_goal_calls==0);

    // High-fungus action scoring retains the native 4-versus-1 factor:
    // rank equal to aggregate fungus technology uses one, a greater rank uses
    // four. Place the factor-four candidate second to distinguish the winner.
    zoc_value=-1; tech_value=1; BaseCurrent->eco_damage=100;
    PlayersData[2].tech_fungus_nutrient=2;
    for (int i=0;i<21;i++) {
      const int x=BaseCurrent->x+RadiusOffsetX[i];
      const int y=BaseCurrent->y+RadiusOffsetY[i];
      map_loc(x,y)->climate=ALT_BIT_SHORE_LINE;
      map_loc(x,y)->bit=BIT_MONOLITH;
      map_loc(x,y)->val3=0;
      map_loc(x,y)->region=4;
    }
    const int factor_one_x=BaseCurrent->x+RadiusOffsetX[1];
    const int factor_one_y=BaseCurrent->y+RadiusOffsetY[1];
    const int factor_four_x=BaseCurrent->x+RadiusOffsetX[2];
    const int factor_four_y=BaseCurrent->y+RadiusOffsetY[2];
    map_loc(factor_one_x,factor_one_y)->bit=BIT_FUNGUS;
    map_loc(factor_one_x,factor_one_y)->climate=0x80;
    map_loc(factor_four_x,factor_four_y)->bit=BIT_FUNGUS;
    map_loc(factor_four_x,factor_four_y)->val3=0x80;
    goal_calls=0; action_goal_calls=0;
    base_terraform(0);
    assert(action_goal_calls==1 && action_goal_type==AI_GOAL_TERRAFORM_LAND &&
           action_goal_x==factor_four_x && action_goal_y==factor_four_y);

    // Fungus below the four-tech aggregate threshold is not counted as a
    // special-land square merely because the fungus-removal technology is
    // available. The native aggregate then sees two open ordinary squares,
    // detects the shortage at population zero, and suppresses the condenser
    // objective. This distinguishes its tech-removal decrement from the
    // separate tech-total special-square count.
    tech_value=1; thermal_available=0; condenser_available=1;
    BaseCurrent->population_size=0; BaseCurrent->eco_damage=0;
    PlayersData[2].tech_fungus_nutrient=3;
    PlayersData[2].tech_fungus_mineral=0;
    PlayersData[2].tech_fungus_energy=0;
    for (int i=0;i<21;i++) {
      const int x=BaseCurrent->x+RadiusOffsetX[i];
      const int y=BaseCurrent->y+RadiusOffsetY[i];
      map_loc(x,y)->climate=ALT_BIT_SHORE_LINE;
      map_loc(x,y)->bit=BIT_MONOLITH;
      map_loc(x,y)->val3=0;
      map_loc(x,y)->region=4;
    }
    const int low_threshold_fungus_x=BaseCurrent->x+RadiusOffsetX[1];
    const int low_threshold_fungus_y=BaseCurrent->y+RadiusOffsetY[1];
    map_loc(low_threshold_fungus_x,low_threshold_fungus_y)->climate=0x80;
    map_loc(low_threshold_fungus_x,low_threshold_fungus_y)->bit=BIT_FUNGUS;
    for (int i=2;i<4;i++) {
      const int x=BaseCurrent->x+RadiusOffsetX[i];
      const int y=BaseCurrent->y+RadiusOffsetY[i];
      map_loc(x,y)->climate=0x90;
      map_loc(x,y)->bit=0;
    }
    condenser_goal_calls=thermal_goal_calls=landing_goal_calls=0;
    base_terraform(0);
    assert(condenser_goal_calls==0 && thermal_goal_calls==0 &&
           landing_goal_calls==0);

    // The native presence latches are set after the monolith test but before
    // the center and later region/ZOC/territory filters. An off-region
    // condenser therefore suppresses the local condenser objective.
    BaseCurrent->population_size=3; tech_value=0;
    PlayersData[2].tech_fungus_nutrient=0;
    thermal_available=0; condenser_available=1;
    for (int i=0;i<21;i++) {
      const int x=BaseCurrent->x+RadiusOffsetX[i];
      const int y=BaseCurrent->y+RadiusOffsetY[i];
      map_loc(x,y)->climate=ALT_BIT_SHORE_LINE;
      map_loc(x,y)->bit=BIT_MONOLITH;
      map_loc(x,y)->val3=0;
      map_loc(x,y)->region=4;
    }
    const int off_region_x=BaseCurrent->x+RadiusOffsetX[1];
    const int off_region_y=BaseCurrent->y+RadiusOffsetY[1];
    map_loc(off_region_x,off_region_y)->bit=BIT_CONDENSER;
    map_loc(off_region_x,off_region_y)->region=5;
    const int local_open_x=BaseCurrent->x+RadiusOffsetX[2];
    const int local_open_y=BaseCurrent->y+RadiusOffsetY[2];
    map_loc(local_open_x,local_open_y)->bit=0;
    condenser_goal_calls=thermal_goal_calls=landing_goal_calls=0;
    base_terraform(0);
    assert(condenser_goal_calls==0 && thermal_goal_calls==0);
    map_loc(off_region_x,off_region_y)->bit=0;
    map_loc(off_region_x,off_region_y)->region=4;
    map_loc(BaseCurrent->x,BaseCurrent->y)->bit=BIT_CONDENSER;
    condenser_goal_calls=0;
    base_terraform(0);
    assert(condenser_goal_calls==0);
    // The monolith rejection precedes the native center/presence checks.
    map_loc(BaseCurrent->x,BaseCurrent->y)->bit=BIT_CONDENSER|BIT_MONOLITH;
    condenser_goal_calls=0;
    base_terraform(0);
    assert(condenser_goal_calls==1);
    """
    cpp = tmp_path / "base_terraform.cpp"
    executable = tmp_path / "base_terraform"
    harness = HARNESS.replace("// @@MASK_TYPES@@", mask_declarations())
    cpp.write_text(harness + "\n" + body() + "\nint main(){\n" + checks + "\nreturn 0;}\n")
    compiled = subprocess.run([compiler, "-std=c++11", "-O1", "-fsanitize=undefined",
                               "-fno-sanitize-recover=undefined", str(cpp), "-o", str(executable)],
                              capture_output=True, text=True, timeout=30)
    assert compiled.returncode == 0, compiled.stderr
    result = subprocess.run([str(executable)], capture_output=True, text=True, timeout=10)
    assert result.returncode == 0, result.stderr
