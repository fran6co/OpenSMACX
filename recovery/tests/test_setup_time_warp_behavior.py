"""Host controls for actual translated time_warp phases."""
from pathlib import Path
import shutil
import subprocess

import pytest


ROOT = Path(__file__).resolve().parents[2]
SIGNATURE = "void __cdecl time_warp() {"
HELPER_SIGNATURE = "static int time_warp_find_prototype(int faction_id, int plan) {"


def source_body() -> str:
    source = (ROOT / "src/game.cpp").read_text()
    start = source.index(SIGNATURE)
    opening = source.index("{", start)
    depth = 0
    for pos in range(opening, len(source)):
        if source[pos] == "{":
            depth += 1
        elif source[pos] == "}":
            depth -= 1
            if depth == 0:
                return source[start : pos + 1]
    raise AssertionError("unterminated time_warp body")


def source_helper() -> str:
    source = (ROOT / "src/game.cpp").read_text()
    start = source.index(HELPER_SIGNATURE)
    opening = source.index("{", start)
    depth = 0
    for pos in range(opening, len(source)):
        if source[pos] == "{":
            depth += 1
        elif source[pos] == "}":
            depth -= 1
            if depth == 0:
                return source[start : pos + 1]
    raise AssertionError("unterminated time_warp prototype helper")


HARNESS = r"""
#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#define __cdecl
typedef int BOOL;
enum { TRUE=1, FALSE=0 };
enum { MaxPlayerNum=8, FAC_RECYCLING_TANKS=3, FAC_RECREATION_COMMONS=6,
       FAC_ENERGY_BANK=7, FAC_COMMAND_CENTER=27, FAC_NETWORK_NODE=8,
       FAC_PERIMETER_DEFENSE=4, BSC_TRANSPORT_FOIL=3, BSC_UNITY_FOIL=13,
       BSC_UNITY_ROVER=11, BSC_MIND_WORMS=8, SP_Unbuilt=-1,
       RFLAG_AQUATIC=0x100, TRIAD_LAND=0, TRIAD_SEA=1 };
struct RulesBasic { int pad[17]; int tech_build_roads_fungus; } Rules;
struct Map { uint8_t climate,contour,val2,region,visibility,val3,unk_1,territory;
             uint32_t bit,bit2,bit_visible[7]; };
struct Base { int16_t x,y; uint8_t faction_id_current,faction_id_former; int8_t population_size;
              uint8_t padding[0x35]; int nutrients_accumulated; uint8_t pad3[0x38];
              int resource_sqr_radius; };
struct Player { int rule_talent,rule_drone,rule_population,rule_flags; char filename[24]; };
struct PlayerData { int ai_wealth,ai_power,tech_id_researching,tech_accumulated;
                    uint32_t tech_achieved; int ai_growth,ai_fight,net_random_event; };
struct VehPrototype { int16_t preq_tech; uint16_t flags; uint8_t obsolete_factions;
                     uint8_t pad[0x21]; uint8_t plan; uint8_t chassis_id; } VehPrototypes[512];
struct Veh { int16_t x,y; uint32_t state; uint16_t flags; int16_t proto_id; uint16_t unk;
             uint8_t faction_id,year; uint8_t damage; int8_t order; uint8_t waypoint_count;
             uint8_t patrol; int16_t waypoint_x[4],waypoint_y[4]; uint8_t morale; } Vehs[2049];
struct Chassis { uint8_t pad[0x4d]; uint8_t triad; } Chassis[64];
Player Players[8]; PlayerData PlayersData[8]; Base Bases[512]; Map Tiles[64*64];
struct BaseSecretProject { int human_genome_project_,command_nexus_,weather_paradigm_,merchant_exchange_,
  empath_guild_,citizens_defense_force_,virtual_world_,planetary_transit_system_;
 int &human_genome_project() { return human_genome_project_; }
 const int &human_genome_project() const { return human_genome_project_; }

 int &command_nexus() { return command_nexus_; }
 const int &command_nexus() const { return command_nexus_; }

 int &weather_paradigm() { return weather_paradigm_; }
 const int &weather_paradigm() const { return weather_paradigm_; }

 int &merchant_exchange() { return merchant_exchange_; }
 const int &merchant_exchange() const { return merchant_exchange_; }

 int &empath_guild() { return empath_guild_; }
 const int &empath_guild() const { return empath_guild_; }

 int &citizens_defense_force() { return citizens_defense_force_; }
 const int &citizens_defense_force() const { return citizens_defense_force_; }

 int &virtual_world() { return virtual_world_; }
 const int &virtual_world() const { return virtual_world_; }

 int &planetary_transit_system() { return planetary_transit_system_; }
 const int &planetary_transit_system() const { return planetary_transit_system_; }
} SecretProject;
uint32_t LockEnableMask; int AiResearchBatchActive, ExpansionEnabled;
int BaseCurrentCount, VehCurrentCount, MapLongitudeBounds=64, MapLatitudeBounds=64;
uint32_t MapLongitude=32;
int MapIsFlat=1, TurnCurrentNum;
int RadiusOffsetX[289], RadiusOffsetY[289], VehMoveDeltaX[9], VehMoveDeltaY[9];
int path_find_calls, path_mode_seen=-1, path_tech_seen=-1;
struct Path { int find(int,int,int,int,int,int,int mode,int) {
  ++path_find_calls; path_mode_seen=mode; path_tech_seen=Rules.tech_build_roads_fungus; return -1;
} } Paths;
Map *map_loc(int x,int y) { return &Tiles[y*32+(x>>1)]; }
int base_by_faction[8];
int base_find(int x,int y,int faction) {
  assert(x==0 && y==0); return base_by_faction[faction];
}
int base_find(int,int) { return 0; }
int advances, advanced_factions[64], selected_tech[64], research_calls;
int tech_ai(int faction) { assert(AiResearchBatchActive==1); selected_tech[research_calls]=100+research_calls;
  advanced_factions[research_calls++]=faction; return 100+research_calls-1; }
int tech_advance(int faction) { assert(faction>0 && faction<8);
  assert(PlayersData[faction].tech_id_researching==selected_tech[advances]);
  ++advances; return 0;
}
int designed[8], design_count;
void consider_designs(uint32_t faction) { assert(AiResearchBatchActive==0);
  assert(PlayersData[faction].tech_id_researching==-1); designed[design_count++]=faction; }
int facility_id[64], facility_base[64], facility_value[64], facility_count;
void set_fac(int facility,int base,int value) { facility_id[facility_count]=facility;
  facility_base[facility_count]=base; facility_value[facility_count++]=value; }
int has_tech_result, has_tech_id, has_tech_faction, has_tech_count;
int has_tech_ids[64], has_tech_factions[64];
int has_tech(int tech,int faction) { has_tech_id=tech; has_tech_faction=faction;
  has_tech_ids[has_tech_count]=tech; has_tech_factions[has_tech_count++]=faction;
  return has_tech_result; }
int vehicle_prototype[16], vehicle_faction[16], vehicle_x[16], vehicle_y[16], vehicle_count;
int veh_init(int proto,int faction,int x,int y) { vehicle_prototype[vehicle_count]=proto;
  vehicle_faction[vehicle_count]=faction; vehicle_x[vehicle_count]=x; vehicle_y[vehicle_count]=y;
  return vehicle_count++; }
int goodie_result, goody_calls, territory_resets, enemy_strategy_calls, set_base_calls;
int compute_calls, terraform_calls, reset_base_calls, synch_calls, site_calls, coast_result;
int world_site_calls, world_site_arg, world_site_result;
int lift_calls, drop_calls, dropped_vehicle=-1, dropped_x=-1, dropped_y=-1;
int goody_at(int,int) { ++goody_calls; return goodie_result; }
void reset_territory() { ++territory_resets; }
void enemy_strategy(int faction) { assert(AiResearchBatchActive==1); assert(faction==1); ++enemy_strategy_calls; }
void set_base(int id) { assert(id>=0); ++set_base_calls; }
void base_compute(int force) { assert(force==1); ++compute_calls; }
void base_terraform(int force) { assert(force==1); ++terraform_calls; }
void base_reset(int id,int force) { assert(id>=0 && force==0); ++reset_base_calls; }
void synch_bit(int,int,int) { ++synch_calls; }
int base_init(int faction,int x,int y) { assert(faction==1); int id=BaseCurrentCount++;
  Bases[id].x=x; Bases[id].y=y; Bases[id].faction_id_current=faction;
  map_loc(x,y)->val3=0xC0; return id; }
int world_site(int,int,int ocean) { ++world_site_calls; world_site_arg=ocean; return world_site_result; }
void site_set(int,int,int) { ++site_calls; }
int is_coast(int,int,int) { return coast_result; }
int veh_lift(int id) { ++lift_calls; return id; }
int veh_drop(int id,int x,int y) { ++drop_calls; dropped_vehicle=id; dropped_x=x; dropped_y=y; return id; }
void sleep(int) {}
int rand_values[64], rand_count, rand_index;
int rand(void) { return rand_index<rand_count ? rand_values[rand_index++] : 0; }
HELPER
BODY
int main() {
  for(int i=0;i<8;++i) base_by_faction[i]=-1;
  rand_index=0; rand_count=2; rand_values[0]=0; rand_values[1]=0;
  SecretProject.human_genome_project()=SecretProject.command_nexus()=
    SecretProject.weather_paradigm()=SecretProject.merchant_exchange()=
    SecretProject.empath_guild()=SecretProject.citizens_defense_force()=
    SecretProject.virtual_world()=SecretProject.planetary_transit_system()=-1;
  LockEnableMask=(1u<<(1+8)) | (1u<<(3+8));
  base_by_faction[1]=4; // faction 3 is alive but has no base
  Players[1].rule_talent=1; Players[1].rule_drone=0; Players[1].rule_population=0;
  PlayersData[1].ai_wealth=1; PlayersData[1].ai_power=0;
  PlayersData[1].ai_growth=1; PlayersData[1].tech_achieved=3;
  Bases[4].x=14; Bases[4].y=22;
  time_warp();
  assert(advances==8 && research_calls==8);
  for(int i=0;i<8;++i) assert(advanced_factions[i]==1);
  assert(AiResearchBatchActive==0 && PlayersData[1].tech_id_researching==-1);
  assert(PlayersData[1].tech_accumulated==6);
  assert(Bases[4].population_size==6 && Bases[4].nutrients_accumulated==30);
  assert(design_count==1 && designed[0]==1);
  assert(facility_count==4);
  assert(facility_id[0]==FAC_RECYCLING_TANKS && facility_id[1]==FAC_RECREATION_COMMONS);
  assert(facility_id[2]==FAC_ENERGY_BANK && facility_id[3]==FAC_NETWORK_NODE);
  assert(SecretProject.human_genome_project()==4);
  assert(TurnCurrentNum==30);
  for(int i=0;i<facility_count;++i) assert(facility_base[i]==4);

  advances=research_calls=design_count=facility_count=0;
  vehicle_count=0; rand_index=0; rand_count=2; rand_values[0]=0; rand_values[1]=1;
  ExpansionEnabled=1; has_tech_count=0;
  Players[1].rule_talent=0; Players[1].rule_drone=0; Players[1].rule_population=0;
  Players[1].rule_flags=0;
  PlayersData[1].ai_wealth=0; PlayersData[1].ai_power=0;
  PlayersData[1].ai_growth=0; PlayersData[1].ai_fight=1;
  Players[1].rule_flags=0x180;
  VehPrototypes[BSC_UNITY_FOIL].preq_tech=37;
  VehPrototypes[6].flags=1; VehPrototypes[6].plan=2; VehPrototypes[6].preq_tech=29;
  has_tech_result=1;
  time_warp();
  assert(vehicle_count==4 && vehicle_prototype[0]==BSC_TRANSPORT_FOIL &&
         vehicle_faction[0]==1 && vehicle_x[0]==14 && vehicle_y[0]==22);
  assert(vehicle_prototype[1]==6 && vehicle_prototype[2]==6 && vehicle_prototype[3]==4);
  assert((VehPrototypes[6].flags & 4) != 0);
  assert(Vehs[1].waypoint_count==2 && Vehs[1].morale==1 && Vehs[2].waypoint_count==2);
  assert(has_tech_id==29 && has_tech_faction==1 && has_tech_count==6);
  assert(has_tech_ids[0]==37 && has_tech_ids[1]==29 && has_tech_ids[2]==37 &&
         has_tech_ids[3]==29 && has_tech_ids[4]==29 && has_tech_ids[5]==29);
  for(int i=0;i<has_tech_count;++i) assert(has_tech_factions[i]==1);
  assert(facility_count==3 && facility_id[2]==FAC_PERIMETER_DEFENSE &&
         facility_base[2]==1 && facility_value[2]==4);
  assert(SecretProject.command_nexus()==4);

  vehicle_count=0; has_tech_count=0; facility_count=0;
  rand_index=0; rand_count=2; rand_values[0]=0; rand_values[1]=2;
  PlayersData[1].ai_fight=0; PlayersData[1].ai_power=0;
  PlayersData[1].net_random_event=1;
  VehPrototypes[7].flags=1; VehPrototypes[7].plan=0; VehPrototypes[7].preq_tech=31;
  time_warp();
  assert(vehicle_count==5);
  assert(vehicle_prototype[0]==6 && vehicle_prototype[1]==6 && vehicle_prototype[2]==4 &&
         vehicle_prototype[3]==7 && vehicle_prototype[4]==7);
  assert(Vehs[3].morale==1);
  assert(SecretProject.weather_paradigm()==4);

  rand_index=0; rand_count=4;
  rand_values[0]=0; rand_values[1]=4; rand_values[2]=5; rand_values[3]=6;
  std::strcpy(Players[1].filename,"HIVE");
  time_warp();
  assert(SecretProject.empath_guild()==-1 && SecretProject.citizens_defense_force()==-1 &&
         SecretProject.virtual_world()==4);

  // A single land candidate exercises site scoring, base creation, native
  // territory/base recomputation, path dispatch, and the final turn store.
  std::memset(Tiles,0,sizeof(Tiles)); std::memset(Bases,0,sizeof(Bases));
  std::memset(RadiusOffsetX,0,sizeof(RadiusOffsetX));
  std::memset(RadiusOffsetY,0,sizeof(RadiusOffsetY));
  std::memset(VehMoveDeltaX,0,sizeof(VehMoveDeltaX));
  std::memset(VehMoveDeltaY,0,sizeof(VehMoveDeltaY));
  BaseCurrentCount=1; VehCurrentCount=0; vehicle_count=0; facility_count=0;
  path_find_calls=territory_resets=0;
  path_mode_seen=path_tech_seen=-1; Rules.tech_build_roads_fungus=73; has_tech_result=0;
  enemy_strategy_calls=set_base_calls=compute_calls=terraform_calls=reset_base_calls=0;
  synch_calls=goody_calls=site_calls=0; goodie_result=1; coast_result=0;
  std::memset(&Players[1],0,sizeof(Players[1]));
  std::memset(&PlayersData[1],0,sizeof(PlayersData[1]));
  SecretProject.human_genome_project()=SecretProject.command_nexus()=
    SecretProject.weather_paradigm()=SecretProject.merchant_exchange()=
    SecretProject.empath_guild()=SecretProject.citizens_defense_force()=
    SecretProject.virtual_world()=SecretProject.planetary_transit_system()=-1;
  PlayersData[1].ai_wealth=0; PlayersData[1].ai_power=0;
  PlayersData[1].tech_achieved=1;
  Bases[0].x=14; Bases[0].y=22; Bases[0].faction_id_current=1;
  base_by_faction[1]=0; LockEnableMask=(1u<<(1+8));
  for(int y=0;y<64;++y) for(int x=0;x<32;++x) Tiles[y*32+x].climate=0;
  map_loc(14,22)->climate=0;  // center is ocean and cannot be reselected
  map_loc(16,22)->climate=0x60;
  map_loc(16,22)->region=0;
  RadiusOffsetX[25]=2; RadiusOffsetY[25]=0;
  rand_index=0; rand_count=2; rand_values[0]=0; rand_values[1]=0;
  time_warp();
  assert(BaseCurrentCount==2 && Bases[1].x==16 && Bases[1].y==22);
  assert(Bases[0].population_size==5 && Bases[1].population_size==5);
  assert(territory_resets==1 && enemy_strategy_calls==1);
  assert(set_base_calls==2 && compute_calls==8 && terraform_calls==6 && reset_base_calls==2);
  assert(facility_count==6 && facility_id[0]==FAC_RECYCLING_TANKS &&
         facility_id[1]==FAC_RECREATION_COMMONS && facility_id[2]==FAC_RECREATION_COMMONS &&
         facility_id[3]==FAC_RECREATION_COMMONS && facility_id[4]==FAC_RECYCLING_TANKS &&
         facility_id[5]==FAC_COMMAND_CENTER);
  assert(vehicle_count==7 && vehicle_prototype[5]==0 && vehicle_prototype[6]==7);
  assert(path_find_calls==2 && synch_calls==162);
  assert(path_mode_seen==4 && path_tech_seen==73);
  assert(goody_calls==42 && (map_loc(16,22)->bit & 0x00100400u)!=0);
  assert(TurnCurrentNum==30);

  // Aquatic faction scoring requests ocean-site evaluation and stores its
  // return before selecting an expansion site.
  BaseCurrentCount=1; VehCurrentCount=0; vehicle_count=0;
  std::memset(Tiles,0,sizeof(Tiles)); std::memset(Bases,0,sizeof(Bases));
  std::memset(RadiusOffsetX,0,sizeof(RadiusOffsetX));
  std::memset(RadiusOffsetY,0,sizeof(RadiusOffsetY));
  RadiusOffsetX[25]=2;
  Bases[0].x=14; Bases[0].y=22; Bases[0].faction_id_current=1;
  base_by_faction[1]=0; LockEnableMask=(1u<<(1+8));
  std::memset(&Players[1],0,sizeof(Players[1])); Players[1].rule_flags=RFLAG_AQUATIC;
  std::memset(&PlayersData[1],0,sizeof(PlayersData[1]));
  SecretProject.human_genome_project()=SecretProject.command_nexus()=
    SecretProject.weather_paradigm()=SecretProject.merchant_exchange()=
    SecretProject.empath_guild()=SecretProject.citizens_defense_force()=
    SecretProject.virtual_world()=SecretProject.planetary_transit_system()=-1;
  has_tech_result=0; world_site_calls=site_calls=0; world_site_result=5;
  rand_index=0; rand_count=2; rand_values[0]=0; rand_values[1]=0;
  time_warp();
  assert(BaseCurrentCount>1 && world_site_calls>0 && world_site_arg==1 && site_calls>0);

  // Native-life relocation tests native plan/chassis/terrain gates and the
  // strongest adjacent-site preference before the lift/drop pair.
  BaseCurrentCount=1; VehCurrentCount=1; vehicle_count=0;
  SecretProject.human_genome_project()=SecretProject.command_nexus()=
    SecretProject.weather_paradigm()=SecretProject.merchant_exchange()=
    SecretProject.empath_guild()=SecretProject.citizens_defense_force()=
    SecretProject.virtual_world()=SecretProject.planetary_transit_system()=-1;
  Bases[0].x=14; Bases[0].y=22; Bases[0].faction_id_current=1;
  Players[1].rule_flags=0;
  std::memset(Tiles,0,sizeof(Tiles));
  map_loc(14,22)->val3=0xC0;
  map_loc(16,22)->climate=0x60;
  map_loc(16,22)->val3=0x40;
  map_loc(16,22)->bit=4;
  Vehs[0].x=14; Vehs[0].y=22; Vehs[0].faction_id=1; Vehs[0].proto_id=30;
  VehPrototypes[30].plan=9; VehPrototypes[30].chassis_id=2; Chassis[2].triad=TRIAD_LAND;
  VehMoveDeltaX[0]=2; VehMoveDeltaY[0]=0;
  drop_calls=lift_calls=0; dropped_vehicle=-1;
  rand_index=0; rand_count=2; rand_values[0]=0; rand_values[1]=1;
  time_warp();
  assert(drop_calls==1 && lift_calls==1 && dropped_vehicle==0);
  assert(dropped_x==16 && dropped_y==22);

  LockEnableMask=0; BaseCurrentCount=0;
  for(int profile=0;profile<3;++profile) {
    rand_index=0; rand_count=1; rand_values[0]=profile;
    time_warp();
    assert(TurnCurrentNum==(profile==0 ? 30 : (profile==1 ? 60 : 80)));
  }
}
"""


@pytest.mark.skipif(not shutil.which("g++"), reason="host C++ compiler unavailable")
def test_time_warp_actual_prefix(tmp_path: Path) -> None:
    program = HARNESS.replace("HELPER", source_helper()).replace("BODY", source_body())
    source = tmp_path / "time_warp.cpp"
    binary = tmp_path / "time_warp"
    source.write_text(program)
    subprocess.run(["g++", "-std=c++11", "-Wall", "-Wextra", "-Werror", str(source), "-o", str(binary)], check=True)
    subprocess.run([str(binary)], check=True)
