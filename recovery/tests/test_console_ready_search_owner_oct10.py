"""Actual selector and independent original-pass oracle; not game runtime proof."""
from dataclasses import fields
from pathlib import Path
import subprocess

import pytest

from match_recovery.source import read_file, read_text
from test_console_typed_owner_members_oct10 import run_control
from test_win_zorder_pointer_flow_oct10 import extract

ROOT = Path(__file__).resolve().parents[2]
BASE = "3cde305ff1cec450230cd8518e096630e32c1df5"


def control():
    body = extract((ROOT / "src/console.cpp").read_text(),
                   "int Console::ready_search(int veh_id)")
    assert "reinterpret_cast" not in body
    return r'''
#include <cassert>
#include <cstdint>
#include <cstring>
#include <cstdlib>
#include <vector>
struct Veh {int16_t x,y;uint32_t state;int8_t order;unsigned char faction_id;} Vehs[8];
struct Console {
 int faction_id_,cursor_history_index_,cursor_history_x_[32],cursor_history_y_[32];
 int focus_vehicle_,selected_vehicle_;unsigned field_23D24_;
 int ready_search(int veh_id);
} ConsoleGlobal;
struct Storage {struct {int vehicle_current_count_;} fields_;} GameDataStateStorage;
static int IsMultiplayerNet,MapIsFlat,MapLongitudeBounds,change_mode;
static uint32_t MapLongitude;
static bool ready[8];
static std::vector<int> visits;
static int veh_ready(int index){
 assert(index>=0 && index<8);visits.push_back(index);
 if(visits.size()==1 && change_mode)GameDataStateStorage.fields_.vehicle_current_count_=change_mode==1?3:1;
 return ready[index];
}
BODY
static int original(Console &owner,int input){
 int x,y;
 if(input<0 || Vehs[input].faction_id!=ConsoleGlobal.faction_id_){
  input=-1;x=ConsoleGlobal.cursor_history_x_[ConsoleGlobal.cursor_history_index_];
  y=ConsoleGlobal.cursor_history_y_[ConsoleGlobal.cursor_history_index_];
 }else {x=Vehs[input].x;y=Vehs[input].y;}
 int best=9999,winner=-1,pass=IsMultiplayerNet?-1:0;
 for(;pass<3;++pass){
  int count=GameDataStateStorage.fields_.vehicle_current_count_;
  if(IsMultiplayerNet && pass==0 && owner.focus_vehicle_>=0 && owner.focus_vehicle_<count &&
     Vehs[owner.focus_vehicle_].faction_id==ConsoleGlobal.faction_id_ && veh_ready(owner.focus_vehicle_)){
   winner=owner.focus_vehicle_;owner.focus_vehicle_=-1;break;
  }
  for(int index=0;index<GameDataStateStorage.fields_.vehicle_current_count_;++index){
   if(!veh_ready(index))continue;
   const uint32_t state=Vehs[index].state;
   if(pass<0 && !((state>>8)&2) && (state&0x01000200)!=0x01000200 &&
      !((state>>8)&0x40) && !Vehs[index].order)continue;
   if(state&2)continue;
   int horizontal=abs(x-Vehs[index].x);
   if(!(MapIsFlat&1) && horizontal>int(MapLongitude))horizontal=MapLongitudeBounds-horizontal;
   horizontal=abs(horizontal);int vertical=abs(y-Vehs[index].y);
   int larger=horizontal>vertical?horizontal:vertical;
   int smaller=horizontal<vertical?horizontal:vertical;
   int score=larger-((((horizontal+vertical)>>1)-smaller+1)>>1);
   if(score<best){best=score;winner=index;}
   else if(score==best && index==owner.selected_vehicle_)winner=index;
  }
  if(winner>=0)break;
  if(pass>=0){
   count=GameDataStateStorage.fields_.vehicle_current_count_;
   for(int index=0;index<count;++index)
    if(pass!=0 || index!=owner.selected_vehicle_ || input<0)Vehs[index].state&=~2u;
  }
 }
 owner.field_23D24_=IsMultiplayerNet && pass<0?1:0;
 return winner;
}
static void check(Console owner,int input){
 Veh before[8];std::memcpy(before,Vehs,sizeof before);int count=GameDataStateStorage.fields_.vehicle_current_count_;
 Console expected=owner;visits.clear();int wanted=original(expected,input);
 const std::vector<int> expected_visits=visits;Veh after[8];std::memcpy(after,Vehs,sizeof after);
 int final_count=GameDataStateStorage.fields_.vehicle_current_count_;
 std::memcpy(Vehs,before,sizeof before);GameDataStateStorage.fields_.vehicle_current_count_=count;visits.clear();
 int found=owner.ready_search(input);
 assert(found==wanted && visits==expected_visits && owner.focus_vehicle_==expected.focus_vehicle_);
 assert(owner.field_23D24_==expected.field_23D24_ && GameDataStateStorage.fields_.vehicle_current_count_==final_count);
 for(int i=0;i<8;++i)assert(Vehs[i].state==after[i].state);
}
static Console reset(){
 std::memset(&ConsoleGlobal,0,sizeof ConsoleGlobal);ConsoleGlobal.faction_id_=3;
 ConsoleGlobal.cursor_history_index_=2;ConsoleGlobal.cursor_history_x_[2]=0;ConsoleGlobal.cursor_history_y_[2]=4;
 for(int i=0;i<8;++i){Vehs[i]={int16_t(i*2),4,0,0,3};ready[i]=true;}
 GameDataStateStorage.fields_.vehicle_current_count_=3;change_mode=0;
 MapIsFlat=1;MapLongitude=50;MapLongitudeBounds=100;IsMultiplayerNet=0;
 Console owner{};owner.faction_id_=7;owner.cursor_history_x_[0]=40;owner.cursor_history_y_[0]=40;
 owner.focus_vehicle_=-1;owner.selected_vehicle_=-1;owner.field_23D24_=99;return owner;
}
int main(){
 const uint32_t bits[]={0,2,0x200,0x4000,0x1000000,0x1000200,0x1000202,0x4200,0xffffffff};
 for(int net=0;net<2;++net)for(int flat=0;flat<2;++flat)for(uint32_t state:bits)
  for(int order: {-1,0,1,24})for(int input: {-1,0,1}){
   Console d=reset();IsMultiplayerNet=net;MapIsFlat=flat;
   Vehs[0].x=98;Vehs[0].state=state;Vehs[0].order=order;
   Vehs[1].x=-2;Vehs[1].y=-4;Vehs[1].state=2;Vehs[2].state=0x1000000;
   d.selected_vehicle_=1;check(d,input);
  }
 // Explicit equal-distance tie, focus priority, all-blocked retry and no-ready exit.
 for(int selected: {-1,0,1,2}){Console d=reset();Vehs[0].x=-2;Vehs[1].x=2;Vehs[2].x=30;d.selected_vehicle_=selected;check(d,-1);}
 for(int input: {-1,0})for(int focus: {-1,0,1,2,8}){
  Console d=reset();IsMultiplayerNet=1;d.focus_vehicle_=focus;Vehs[0].state=2;Vehs[1].state=2;Vehs[2].state=2;
  d.selected_vehicle_=0;check(d,input);
 }
 for(int missing=0;missing<4;++missing){Console d=reset();for(int i=0;i<missing && i<3;++i)ready[i]=false;check(d,-1);}
 for(int count: {0,1,3}){Console d=reset();GameDataStateStorage.fields_.vehicle_current_count_=count;check(d,-1);}
 for(int mutation: {1,2}){
  Console d=reset();change_mode=mutation;GameDataStateStorage.fields_.vehicle_current_count_=mutation==1?1:3;
  Vehs[0].x=30;Vehs[1].x=20;Vehs[2].x=0;check(d,-1);
 }
 Console other=reset();Vehs[0].faction_id=7;check(other,0);
}
'''.replace("BODY", body)


@pytest.mark.parametrize("compiler", ("g++", "clang++"))
@pytest.mark.parametrize("damage", (None,
    ("innerIdx < GameDataStateStorage.fields_.vehicle_current_count_", "innerIdx < vehCount"),
    ("vehicle.state & 0x200", "vehicle.state & 0x2"),
    ("vehicle.state & 0x4000", "vehicle.state & 0x40"),
    ("vehicle.order != 0", "vehicle.order == 0"),
    ("if (vehicle.state & 2)", "if (vehicle.state & 4)"),
    ("int vehX = vehicle.x;", "int vehX = vehicle.y;"),
    ("innerIdx == selected_vehicle_", "innerIdx != selected_vehicle_"),
    ("Vehs[clearIdx].state &= 0xfffffffd;", "Vehs[clearIdx].state &= 0xffffffff;")))
def test_actual_selector_and_damaging_controls(tmp_path, compiler, damage):
    run_control(tmp_path, compiler, control(), damage)


def test_console_original_identities_claims_and_history_remain():
    path = ROOT / "src/console.cpp"
    before = subprocess.check_output(["git", "show", f"{BASE}:src/console.cpp"], cwd=ROOT).decode()
    old = {r.address:r for r in read_text(before,path)}
    new = {r.address:r for r in read_file(path)}
    assert set(old)==set(new)
    for address,row in old.items():
        for field in fields(row):
            if field.name not in {"line","region","origin"}:
                assert getattr(row,field.name)==getattr(new[address],field.name)
        assert [(l.kind,l.key,l.prose,tuple(t for _,t in l.lines)) for l in row.origin]==[
            (l.kind,l.key,l.prose,tuple(t for _,t in l.lines)) for l in new[address].origin]
