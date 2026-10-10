"""Actual prototype declarations/scans with bounded slot-selection controls."""
from dataclasses import fields
from pathlib import Path
import subprocess

import pytest

from match_recovery.source import read_file, read_text
from test_console_typed_owner_members_oct10 import run_control
from test_win_zorder_pointer_flow_oct10 import extract

ROOT = Path(__file__).resolve().parents[2]
BASE = "8b14f137db5e11e9344ebd948241028983d8c047"


def control():
    body = extract((ROOT / "src/designwin.cpp").read_text(), "int DesignWin::design_it()")
    first_start = body.index("int own = factionID_ * 0x40;")
    first_end = body.index("} while (checked < 0x40);", first_start) + len("} while (checked < 0x40);")
    second_start = body.index("int search = factionID_ * 0x40;")
    second_end = body.index("} while (looked < 0x40);", second_start) + len("} while (looked < 0x40);")
    first, second = body[first_start:first_end], body[second_start:second_end]
    assert "reinterpret_cast" not in first + second
    prototype = extract((ROOT / "src/veh.h").read_text(), "struct VehPrototype") + ";"
    return r'''
#include <cassert>
#include <cstdint>
#include <cstddef>
#include <cstring>
#include <initializer_list>
PROTOTYPE
static_assert(sizeof(VehPrototype)==0x34,"original record stride");
static_assert(offsetof(VehPrototype,unk_1)==0x2b,"category offset");
static_assert(offsetof(VehPrototype,obsolete_factions)==0x2c,"obsolete mask offset");
static_assert(offsetof(VehPrototype,flags)==0x30,"flags offset");
static VehPrototype VehPrototypes[512];
struct Player {uint8_t proto_id_active[512],proto_id_queue[512];} PlayersData[8];
static const int PROTO_ACTIVE=1;
static int first_scan(int factionID_,int protoID_){
FIRST
 return protoID_;
}
struct Selection {int slot,replaced;};
static Selection second_scan(int factionID_,int protoID_){
SECOND
settled:
 return {protoID_,replaced};
}
static int first_original(int faction,int original){
 int last=original;unsigned bit=1u<<faction;
 for(int id=faction*64;id<faction*64+64;++id){
  if((VehPrototypes[id].obsolete_factions&bit) && !PlayersData[faction].proto_id_active[id] &&
     !PlayersData[faction].proto_id_queue[id])return id;
  if(!(VehPrototypes[id].flags&1))last=id;
 }
 return last;
}
static Selection second_original(int faction,int original){
 Selection result{original,0};int8_t category=VehPrototypes[original].unk_1;unsigned bit=1u<<faction;
 for(int id=faction*64;id<faction*64+64;++id){
  if(!(VehPrototypes[id].flags&1))return {id,result.replaced};
  if(!(VehPrototypes[id].obsolete_factions&bit) || PlayersData[faction].proto_id_active[id])continue;
  if(!PlayersData[faction].proto_id_queue[id])result={id,1};
  else if(!result.replaced && VehPrototypes[id].unk_1==category)result.slot=id;
 }
 return result;
}
static void check(int faction,int initial){
 assert(first_scan(faction,initial)==first_original(faction,initial));
 Selection a=second_scan(faction,initial),b=second_original(faction,initial);
 assert(a.slot==b.slot && a.replaced==b.replaced);
}
static void reset(){
 std::memset(VehPrototypes,0,sizeof VehPrototypes);std::memset(PlayersData,0,sizeof PlayersData);
 for(auto &row:VehPrototypes){row.flags=0x8101;row.unk_1=-1;}
}
static unsigned random_state=73;
static unsigned next(){random_state=1664525u*random_state+1013904223u;return random_state;}
int main(){
 for(int faction=0;faction<8;++faction){
  int first=faction*64,initial=17;
  reset();check(faction,initial);
  // First scan retains the last inactive slot; second takes the first.
  VehPrototypes[first+2].flags=0x8100;VehPrototypes[first+63].flags=0x8100;
  check(faction,initial);assert(first_scan(faction,initial)==first+63 && second_scan(faction,initial).slot==first+2);
  // First obsolete/free row wins before later inactive candidates.
  reset();VehPrototypes[first+1].obsolete_factions=1u<<faction;VehPrototypes[first+63].flags=0;
  check(faction,initial);assert(first_scan(faction,initial)==first+1);
  // Final slot is included; unsigned bit7 and signed categories survive.
  reset();VehPrototypes[first+63].obsolete_factions=1u<<faction;check(faction,initial);
  assert(first_scan(faction,initial)==first+63 && second_scan(faction,initial).replaced==1);
  // Unqueued replacements outrank later queued category matches.
  reset();for(int offset: {1,2,63})VehPrototypes[first+offset].obsolete_factions=1u<<faction;
  PlayersData[faction].proto_id_queue[first+1]=1;PlayersData[faction].proto_id_queue[first+63]=1;
  check(faction,initial);assert(second_scan(faction,initial).slot==first+2);
  // An active row must not be replaced even if its obsolete bit is set.
  PlayersData[faction].proto_id_active[first+2]=1;check(faction,initial);
  for(int trial=0;trial<120;++trial){
   reset();VehPrototypes[initial].unk_1=static_cast<int8_t>(next()>>16);
   for(int id=first;id<first+64;++id){
    unsigned r=next();VehPrototypes[id].flags=(r&1)|0x8100;
    VehPrototypes[id].obsolete_factions=static_cast<uint8_t>(r>>8);
    VehPrototypes[id].unk_1=static_cast<int8_t>(r>>16);
    PlayersData[faction].proto_id_active[id]=(r>>24)&1;
    PlayersData[faction].proto_id_queue[id]=(r>>25)&1;
   }
   check(faction,initial);
  }
 }
}
'''.replace("PROTOTYPE", prototype).replace("FIRST", first).replace("SECOND", second)


@pytest.mark.parametrize("compiler", ("g++", "clang++"))
@pytest.mark.parametrize("damage", (None,
    ("checked < 0x40", "checked < 0x3f"),
    ("looked < 0x40", "looked < 0x3f"),
    ("const VehPrototype &candidate = VehPrototypes[own];", "const VehPrototype &candidate = VehPrototypes[own+1];"),
    ("const VehPrototype &candidate = VehPrototypes[search];", "const VehPrototype &candidate = VehPrototypes[search+1];"),
    ("proto_id_queue[own] == 0", "proto_id_queue[own] != 0"),
    ("proto_id_active[own] == 0", "proto_id_active[own] != 0"),
    ("replaced == 0 && candidate.unk_1", "replaced != 0 && candidate.unk_1"),
    ("if ((candidate.flags & PROTO_ACTIVE) == 0) {\n                protoID_ = search;",
     "if (candidate.flags == 0) {\n                protoID_ = search;")))
def test_actual_record_scans_and_damaging_controls(tmp_path, compiler, damage):
    run_control(tmp_path, compiler, control(), damage)


def test_designwin_identities_claims_and_seed_notes_remain():
    path = ROOT / "src/designwin.cpp"
    before = subprocess.check_output(["git", "show", f"{BASE}:src/designwin.cpp"], cwd=ROOT).decode()
    old = {r.address:r for r in read_text(before,path)}
    new = {r.address:r for r in read_file(path)}
    assert set(old)==set(new)
    for address,row in old.items():
        for field in fields(row):
            if field.name not in {"line","region","origin"}:
                assert getattr(row,field.name)==getattr(new[address],field.name)
        assert [(l.kind,l.key,l.prose,tuple(t for _,t in l.lines)) for l in row.origin]==[
            (l.kind,l.key,l.prose,tuple(t for _,t in l.lines)) for l in new[address].origin]
