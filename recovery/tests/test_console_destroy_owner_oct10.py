"""Actual destroy body with independent picker/dispatch controls, not runtime proof."""

from dataclasses import fields
from pathlib import Path
import subprocess

import pytest

from match_recovery.source.reader import read_file, read_text
from test_console_typed_owner_members_oct10 import run_control
from test_win_zorder_pointer_flow_oct10 import extract

ROOT = Path(__file__).resolve().parents[2]
BASE = "f9a3618dc90de7dd03989422f6a5312ecac5a57f"


def control():
    body = extract((ROOT / "src/console.cpp").read_text(), "void Console::destroy(int veh_id)")
    return r'''
#include <cassert>
#include <cstdint>
#include <cstring>
#include <vector>
#include <utility>
#define DTREATY_VENDETTA 0x10
static int stage, blocked_stage, notready_stage, lock_failure;
static int popup_count,exec_count,unlocks,awaits,action_count,double_count;
static int territory,refuse,chosen,action_bits,action_vehicle,netmsg_count;
static std::vector<std::pair<const char *,uint32_t> > options;
struct Veh {int x,y,faction_id,proto_id;} Vehs[2];
struct Prototype {int weapon_id,chassis_id;} VehPrototypes[2];
struct WeaponRow {int offense_rating,mode;} Weapon[2];
struct ChassisRow {int triad;} Chassis[2];
struct Map {unsigned char climate,val2;uint32_t bit;} tile;
static Map *map_loc(int x,int y){assert(x==18 && y==4);return &tile;}
struct TerraformRow {char *name,*name_sea;uint32_t bit;} Terraforming[19];
struct Player {uint32_t diplo_treaties[8];} PlayersData[8];
static int IsMultiplayerNet;
static const int WPN_MODE_TERRAFORMER=9;
static const char *PopupStartCaption="caption",*ConsoleStrSimulNo="turn",
 *ConsoleStrUnitMoved="ready",*ConsoleStrSimulNo2="turn2",*ConsoleStrUnitMoved2="ready2",
 *ConsoleStrOnlyCombat="combat",*ConsoleStrNotBase="base",*ConsoleStrNotInTriad="triad",
 *ConsoleStrDestroyBomb="bomb",*ConsoleStrDestroy="destroy",*ConsoleStrNotHere="empty";
static bool not_my_turn(){++stage;return stage==blocked_stage;}
static int veh_ready(int id){assert(id==0);return stage!=notready_stage;}
struct Message {void pop(const char *,int,int,int){++netmsg_count;}} NetMsgPrimary;
struct Dialogs;
static Dialogs *expected_dialog;
struct Dialogs {
 void item(const char *name,uint32_t bits){assert(this==expected_dialog);options.push_back({name,bits});}
};
struct Popup {
 Dialogs owned;
 Popup(){++popup_count;expected_dialog=&owned;}
 ~Popup(){--popup_count;}
 Dialogs &dialogs(){return owned;}
 void start(const char *caption,const char *title,int value,void *,int flags,void *){
  assert(caption==PopupStartCaption && value==-1 && flags==0x40);
  assert(title==(Chassis[0].triad==2 ? ConsoleStrDestroyBomb:ConsoleStrDestroy));
 }
 int exec(int a,int b){assert(a==0 && b==0);++exec_count;return chosen;}
};
struct Strings {const char *get(const char *s){return s;}} StringTable;
struct Net {
 int lock_veh(int *id,int a,int b,int c,int d){assert(*id==0 && a==0 && b==-1 && c==-1 && d==-1);return lock_failure;}
 void unlock_veh(){++unlocks;}
 void await_exec(int mode){assert(mode==0 || mode==1);++awaits;}
} NetDaemonNet;
static int whose_territory(int f,int x,int y,int *owner,int mode){
 assert(f==2 && x==18 && y==4 && owner==nullptr && mode==0);return territory;
}
static int break_treaty(int f,int other,int reason){assert(f==2 && other==3 && reason==7);return refuse;}
static void double_cross(int f,int other,int mode){assert(f==2 && other==3 && mode==-1);++double_count;}
static std::vector<int> messages;
static void message_data(int code,int a,int b,int f,int other,int mode){
 if(code==0x2440)assert(a==0 && b==0 && f==2 && other==3 && mode==-1);
 else assert(code==0x2214 && a==0 && b==0 && f==0 && other==0 && mode==0);
 messages.push_back(code);
}
static void message_veh(int code,int id,int bits,int mode){
 assert(code==0x2406 && id==0 && mode==0);action_vehicle=id;action_bits=bits;++action_count;
}
static void action_destroy(int id,int bits,int x,int y){
 assert(x==-1 && y==-1);action_vehicle=id;action_bits=bits;++action_count;
}
struct Console {int field_23BE4_;void destroy(int veh_id);};
BODY
static void reset(){
 stage=blocked_stage=notready_stage=lock_failure=0;
 popup_count=exec_count=unlocks=awaits=action_count=double_count=netmsg_count=0;
 action_bits=action_vehicle=-99;territory=-1;refuse=0;chosen=17;IsMultiplayerNet=0;
 options.clear();messages.clear();std::memset(PlayersData,0,sizeof PlayersData);
 Vehs[0]={18,4,2,0};Vehs[1]={7,8,3,1};VehPrototypes[0]={0,0};Weapon[0]={1,0};Chassis[0]={0};
 tile={0x60,8,0};
 for(int i=0;i<19;++i){Terraforming[i].name=(char *)"land";Terraforming[i].name_sea=(char *)"sea";Terraforming[i].bit=1u<<i;}
}
int main(){
 // Exhaust the captured tile flags and both label columns independently of
 // coordinate x=18: using x/2 as the source selector cannot pass this test.
 for(int ocean=0;ocean<2;++ocean)for(unsigned flags=0;flags<0x80000u;flags+=7919u){
  reset();tile.climate=ocean?0x20:0x60;tile.bit=flags;Chassis[0].triad=ocean?1:0;
  Console console{99};console.destroy(0);assert(popup_count==0);
  std::vector<std::pair<const char *,uint32_t> > expected;
  for(unsigned k=0;k<19;++k){
   const bool excluded=k==10 || k==11 || (k==0 && (flags&(1u<<19))) || (k==5 && (flags&(1u<<3)));
   if(!excluded && (flags&(1u<<k)))expected.push_back({ocean ? Terraforming[k].name_sea:Terraforming[k].name,1u<<k});
  }
  assert(options==expected);
  if(expected.empty()){assert(unlocks==1 && action_count==0 && console.field_23BE4_==99);}
  else {assert(unlocks==0 && action_count==1 && action_vehicle==0 && console.field_23BE4_==0);
   assert(exec_count==(expected.size()>=2));assert(action_bits==(expected.size()==1?0:17));}
 }
 for(unsigned kind: {0u,2u,4u,5u,18u}){
  reset();tile.bit=1u<<kind;Console single{99};single.destroy(0);
  assert(options.size()==1 && exec_count==0 && action_count==1 && action_bits==0 && unlocks==0);
 }
 // Explicit cases exercise both conditional skips, including the bit19 flag.
 for(unsigned flags: {1u<<19 | 1u, 1u<<3 | 1u<<5,1u<<11,1u<<10}){
  reset();tile.bit=flags;Console c{99};c.destroy(0);
  for(auto item:options)assert(item.second!=1u && item.second!=(1u<<5) && item.second!=(1u<<10) && item.second!=(1u<<11));
 }
 for(int network=0;network<2;++network)for(int actor=0;actor<2;++actor)for(int reverse=0;reverse<2;++reverse){
  reset();tile.bit=(1u<<2)|(1u<<4);territory=3;IsMultiplayerNet=network;
  PlayersData[2].diplo_treaties[3]=actor?0x10:0;PlayersData[3].diplo_treaties[2]=reverse?0x10:0;
  Console c{99};c.destroy(0);assert(c.field_23BE4_==0 && action_count==1 && action_bits==17);
  assert(double_count==(!network && !actor));assert(messages.size()==(network && !actor?2u:0u));
  assert(awaits==(network?(actor?1:2):0));assert(unlocks==0);
 }
 // Cancel/refused treaty must unlock and never publish an action/state clear.
 for(int cancel=0;cancel<2;++cancel){reset();tile.bit=(1u<<2)|(1u<<4);
  if(cancel)chosen=-1;else {territory=3;refuse=1;}
  Console c{99};c.destroy(0);assert(unlocks==1 && !action_count && c.field_23BE4_==99 && popup_count==0);
 }
 // Exact equality with -1 matters: do not silently normalize other negatives.
 reset();tile.bit=(1u<<2)|(1u<<4);chosen=-2;Console c{99};c.destroy(0);assert(action_bits==-2 && !unlocks);
 for(int at=1;at<=2;++at){reset();blocked_stage=at;Console b{99};b.destroy(0);assert(!action_count && unlocks==(at==2) && popup_count==0);}
 for(int at=1;at<=2;++at){reset();notready_stage=at;Console b{99};b.destroy(0);assert(!action_count && unlocks==(at==2) && popup_count==0);}
 reset();lock_failure=1;Console l{99};l.destroy(0);assert(!action_count && !unlocks && popup_count==0);
 reset();Weapon[0]={0,0};Console w{99};w.destroy(0);assert(unlocks==1 && !action_count);
 reset();tile.bit=1;tile.val2=7;Console base{99};base.destroy(0);assert(unlocks==1 && !action_count);
 reset();tile.climate=0x20;Console mismatch{99};mismatch.destroy(0);assert(unlocks==1 && !action_count);
}
'''.replace("BODY", body)


@pytest.mark.parametrize("compiler", ("g++", "clang++"))
@pytest.mark.parametrize("damage", (None,
    ("Dialogs &dialogs = popup.dialogs();", "Dialogs dialogs;"),
    ("const bool is_ocean = (tile->climate & 0xE0) < 0x60;", "const bool is_ocean = (vx >> 1) != 0;"),
    ("title, -1, nullptr", "title, tile_flags, nullptr"),
    ("int32_t result = 0;", "int32_t result = -1;"),
    ("if (result == -1) {\n        NetDaemonNet.unlock_veh();", "if (result == -1) {"),
    ("PlayersData[faction].diplo_treaties[territory]", "PlayersData[territory].diplo_treaties[faction]"),
    ("|| a == 11", "|| a == 12"),
    ("tile_flags & 0x80000", "tile_flags & 0x40000")))
def test_actual_destroy_owner_and_damage_controls(tmp_path, compiler, damage):
    run_control(tmp_path, compiler, control(), damage)


def test_all_console_identities_claims_and_history_remain():
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
    body=extract(path.read_text(),"void Console::destroy(int veh_id)")
    assert "reinterpret_cast" not in body and "x_half" not in body
    assert "Scroll scroll;" not in body and "FlatButton btn" not in body
