"""Host controls for the actual balance initializer body."""
from pathlib import Path
import re
import shutil
import subprocess

import pytest


ROOT = Path(__file__).resolve().parents[2]
SIGNATURE = "void __cdecl balance() {"
CONTINENT_PLACEHOLDER = "CONTINENT_DECLARATION"


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
    raise AssertionError("unterminated balance body")


def continent_declaration() -> str:
    header = (ROOT / "src/map.h").read_text()
    match = re.search(r"\bstruct\s+Continent\s*\{", header)
    assert match, "Continent declaration missing from src/map.h"
    opening = header.index("{", match.start())
    depth = 0
    for end in range(opening, len(header)):
        depth += (header[end] == "{") - (header[end] == "}")
        if depth == 0:
            assert header[end + 1] == ";"
            declaration = header[match.start():end + 2]
            assert "int tile_count;" in declaration
            return declaration
    raise AssertionError("unterminated Continent declaration")


HARNESS = r"""
#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#define __cdecl
#include "game_data_state_host.h"
GameDataStateWindow GameDataStateStorage = {};
enum { MaxPlayerNum=8, BIT_RIVER=0x80, RFLAG_AQUATIC=0x100 };
struct Map { uint8_t climate, contour, val2, region, visibility, val3, unk_1, territory;
             uint32_t bit, bit2, bit_visible[7]; };
struct Player { int rule_flags, faction_bonus_count, faction_bonus_id[8],
                    faction_bonus_val1[8], faction_bonus_val2[8]; };
struct Veh { int16_t x,y; uint32_t state; uint16_t flags; int16_t proto_id,unk_1;
             uint8_t faction_id,year_end_lurking,dmg_incurred; int8_t order;
             uint8_t waypoint_count,patrol_current_point; int16_t waypoint_x[4],waypoint_y[4];
             uint8_t morale,terraforming_turns,order_auto_type,visibility,moves_expended;
             int8_t facing; uint8_t unk_6,move_to_ai_type,probe_action,probe_sabotage_id;
             int16_t home_base_id,next_veh_id_stack,prev_veh_id_stack; };
struct VehPrototype { char name[32]; uint32_t ability_flags; uint8_t chassis_id,weapon_id,
  armor_id,reactor_id,carry_capacity,cost,plan; int8_t unk_1; uint8_t obsolete_factions;
  int8_t combat_factions,icon_offset,padding; uint16_t flags; int16_t preq_tech; };
CONTINENT_DECLARATION
Player Players[8]; Veh Vehs[2049]; VehPrototype VehPrototypes[128];
Continent Continents[129]; Map tiles[5000]; Map *MapTiles=tiles;
int MapLongitudeBounds=100, MapLatitudeBounds=100, MapIsFlat=1;
uint32_t MapLongitude=50, MapArea=5000, LockEnableMask=0;
int DiffLevelCurrent=0;
int RadiusOffsetX[289]={0}, RadiusOffsetY[289]={0};
int find_result[8], find_calls[64], find_count, init_calls, init_faction[16];
int bit_calls, bit_x[64], bit_y[64], bit_mask[64];
int goody_result;
Map *map_loc(int x,int y) { return &MapTiles[(x>>1)+y*(int)MapLongitude]; }
int veh_find(int,int,int faction,int) { find_calls[find_count++]=faction; return find_result[faction]; }
int veh_init(int,int faction,int x,int y) { (void)x;(void)y; init_faction[init_calls++]=faction; return 2048; }
int goody_at(int,int) { return goody_result; }
void bit_set(int x,int y,int bit,int) { bit_x[bit_calls]=x; bit_y[bit_calls]=y; bit_mask[bit_calls++]=bit; }
int rand_values[64], rand_count, rand_index;
int rand(void) { return rand_index<rand_count ? rand_values[rand_index++] : 0; }
BODY
int main() {
  for (int i=0;i<8;++i) find_result[i]=-1;
  balance();
  assert(find_count==7 && find_calls[0]==1 && find_calls[6]==7);
  assert(init_calls==0 && bit_calls==0);

  find_count=init_calls=bit_calls=rand_index=rand_count=0;
  for (int i=0;i<2049;++i) Vehs[i]=Veh();
  for (int i=0;i<7;++i) find_result[i+1]=-1;
  find_result[1]=10; find_result[2]=11;
  Vehs[10].x=30; Vehs[10].y=30; Vehs[10].faction_id=1;
  Vehs[11].x=60; Vehs[11].y=60; Vehs[11].faction_id=2;
  for (int i=0;i<5000;++i) tiles[i]=Map();
  map_loc(30,30)->region=1; map_loc(60,60)->region=2;
  Continents[1].tile_count=32; Continents[1].open_terrain=512;
  Continents[2].tile_count=225; Continents[2].open_terrain=0;
  goody_result=1;
  rand_values[0]=0; rand_values[1]=15; rand_count=2;
  balance();
  assert(find_count==7 && rand_index==2);
  assert(init_calls==0); // Low difficulty halves the deficit; no defender is made.
  assert(bit_calls>0);
  for (int i=0;i<bit_calls;++i) {
    assert(bit_x[i]>=0 && bit_x[i]<MapLongitudeBounds);
    assert(bit_y[i]>=0 && bit_y[i]<MapLatitudeBounds);
    assert(bit_mask[i]==0x04000000 || bit_mask[i]==0x08000000);
  }
  assert(bit_calls < 10); // The deficit loop mutates its own bound on selected placements.

  // At higher difficulty, a large deficit consumes the defender decision.
  // A faction carrying the original 0x100 rule bit skips veh_init, while an
  // otherwise identical faction creates a defender and links no fabricated
  // unit state from the return slot.
  find_count=init_calls=bit_calls=rand_index=0;
  GameDataStateStorage.fields_.diff_level_current_=4;
  Players[1].rule_flags=0x100;
  balance();
  assert(init_calls==0);
  find_count=init_calls=bit_calls=rand_index=0;
  Players[1].rule_flags=0;
  balance();
  assert(init_calls>0 && init_faction[0]==1 && Vehs[2048].next_veh_id_stack==-1);

  // The first formula changes at tile_count 32 -> 33 while the /64 component
  // remains one. Distinct open_terrain values make a wrong first read visible.
  memset(Players,0,sizeof(Players)); memset(Vehs,0,sizeof(Vehs));
  memset(Continents,0,sizeof(Continents)); memset(MapTiles,0,sizeof(tiles));
  GameDataStateStorage.fields_.diff_level_current_=0;
  GameDataStateStorage.fields_.lock_masks_.enable_mask_=0;
  for(int i=0;i<8;++i)find_result[i]=-1;
  find_result[1]=10; find_result[2]=11;
  Vehs[10].x=30; Vehs[10].y=30; Vehs[11].x=60; Vehs[11].y=60;
  map_loc(30,30)->region=1; map_loc(60,60)->region=2;
  Continents[1].tile_count=32; Continents[2].tile_count=33;
  Continents[1].open_terrain=0; Continents[2].open_terrain=0;
  find_count=init_calls=bit_calls=rand_index=rand_count=0; goody_result=0;
  balance();
  assert(bit_calls==1);

  // The second formula changes at tile_count 0 -> 1 while /32 stays one.
  // Matching open_terrain values distinguish the wrong second read.
  memset(Players,0,sizeof(Players)); memset(Vehs,0,sizeof(Vehs));
  memset(Continents,0,sizeof(Continents)); memset(MapTiles,0,sizeof(tiles));
  GameDataStateStorage.fields_.diff_level_current_=0;
  GameDataStateStorage.fields_.lock_masks_.enable_mask_=0;
  for(int i=0;i<8;++i)find_result[i]=-1;
  find_result[1]=10; find_result[2]=11;
  Vehs[10].x=30; Vehs[10].y=30; Vehs[11].x=60; Vehs[11].y=60;
  map_loc(30,30)->region=1; map_loc(60,60)->region=2;
  Continents[1].tile_count=0; Continents[2].tile_count=1;
  Continents[1].open_terrain=0; Continents[2].open_terrain=0;
  find_count=init_calls=bit_calls=rand_index=rand_count=0;
  balance();
  assert(bit_calls==1);

  // Negative signed input covers the lower clamp in each formula. The two
  // wrong-field mutations collapse the correct three-placement deficit.
  memset(Players,0,sizeof(Players)); memset(Vehs,0,sizeof(Vehs));
  memset(Continents,0,sizeof(Continents)); memset(MapTiles,0,sizeof(tiles));
  GameDataStateStorage.fields_.diff_level_current_=0;
  GameDataStateStorage.fields_.lock_masks_.enable_mask_=0;
  for(int i=0;i<8;++i)find_result[i]=-1;
  find_result[1]=10; find_result[2]=11;
  Vehs[10].x=30; Vehs[10].y=30; Vehs[11].x=60; Vehs[11].y=60;
  map_loc(30,30)->region=1; map_loc(60,60)->region=2;
  Continents[1].tile_count=-128; Continents[2].tile_count=128;
  Continents[1].open_terrain=128; Continents[2].open_terrain=0;
  find_count=init_calls=bit_calls=rand_index=rand_count=0;
  balance();
  assert(bit_calls==2);
}
"""


@pytest.mark.skipif(not shutil.which("g++"), reason="host C++ compiler unavailable")
def test_balance_actual_body(tmp_path: Path) -> None:
    program = HARNESS.replace("BODY", source_body()).replace(
        CONTINENT_PLACEHOLDER, continent_declaration())
    host_state = ROOT / "recovery/tests/vehicle_gameplay_six/game_data_state_host.h"
    (tmp_path / host_state.name).write_bytes(host_state.read_bytes())
    source = tmp_path / "balance.cpp"
    binary = tmp_path / "balance"
    source.write_text(program)
    subprocess.run(["g++", "-std=c++11", "-Wall", "-Wextra", "-Werror", str(source), "-o", str(binary)], check=True)
    subprocess.run([str(binary)], check=True)


def run_body(tmp_path: Path, body: str) -> subprocess.CompletedProcess:
    program = HARNESS.replace("BODY", body).replace(
        CONTINENT_PLACEHOLDER, continent_declaration())
    host_state = ROOT / "recovery/tests/vehicle_gameplay_six/game_data_state_host.h"
    (tmp_path / host_state.name).write_bytes(host_state.read_bytes())
    source = tmp_path / "balance.cpp"
    binary = tmp_path / "balance"
    source.write_text(program)
    subprocess.run(["g++", "-std=c++11", "-Wall", "-Wextra", "-Werror",
                    str(source), "-o", str(binary)], check=True)
    return subprocess.run([str(binary)], capture_output=True, text=True)


@pytest.mark.skipif(not shutil.which("g++"), reason="host C++ compiler unavailable")
@pytest.mark.parametrize(("before", "after"), [
    ("Continents[region].tile_count + 31", "Continents[region].open_terrain + 31"),
    ("Continents[region].tile_count + 63", "Continents[region].open_terrain + 63"),
])
def test_balance_wrong_continent_score_field_mutations_fail(tmp_path: Path,
                                                             before: str,
                                                             after: str) -> None:
    body = source_body()
    assert body.count(before) == 1
    result = run_body(tmp_path, body.replace(before, after, 1))
    assert result.returncode != 0
