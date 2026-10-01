"""Host controls over the genuine Console bodies in the remaining owner wave."""
from pathlib import Path
import shutil
import subprocess

import pytest


ROOT = Path(__file__).resolve().parents[2]


def body_for(signature: str) -> str:
    source = (ROOT / "src/console.cpp").read_text()
    start = source.index(signature)
    opening = source.index("{", start)
    depth = 0
    for end in range(opening, len(source)):
        depth += (source[end] == "{") - (source[end] == "}")
        if depth == 0:
            return source[start : end + 1]
    raise AssertionError(f"unterminated production body: {signature}")


GOVERNOR_HARNESS = r'''
#define __cdecl
#include <cassert>
#include <cstdint>
#include <cstring>
#include <cstdio>
#include <string>
#include <vector>

std::vector<std::string> events;
struct Base { uint8_t faction_id_current=0; uint32_t governor=0; } Bases[8];
struct PlayerData { uint32_t base_governor_adv=0; } PlayersData[8];
struct Player { int is_leader_female=0, noun_gender=0, is_noun_plural=0;
  char title_leader[24]{}, name_leader[24]{}, noun_faction[24]{}; } Players[8];
int BaseCurrentCount=0, ConsoleControlTurnActive=0, GenderDefault=0;
int PluralityDefault=0, Directive=0, HumanMask=0xff, LockResult=0;
int SetBaseCalls=0, GovernorCalls=0, UnlockCalls=0;
char StringTemp[2048];
int __cdecl x_pop_short(const char *label,int flags,int (*callback)()) {
  assert(std::strcmp(label,"GOVERNORCONTROL")==0 && flags==0x40 && !callback);
  events.push_back("choice"); return Directive;
}
bool is_human(int faction) { return (HumanMask & (1<<faction)) != 0; }
int diplo_lock(int code) { assert(code==0x32); events.push_back("lock"); return LockResult; }
void diplo_unlock() { ++UnlockCalls; events.push_back("unlock"); }
int parse_says(int,const char *text,int,int) { events.push_back(std::string("parse:")+text); return 0; }
char *label_get(int offset) {
  static char a[]="AUTO", b[]="ECON", c[]="PSYCH", d[]="TECH", e[]="WEALTH", f[]="ADVANCED";
  switch(offset) { case 0xAF8/4:return a; case 0x824/4:return b;
    case 0x828/4:return c; case 0x82C/4:return d; case 0x830/4:return e;
    case 0xAFC/4:return f; default: assert(false); return a; }
}
struct Popup { void pop(const char *label,int timeout,int flags,const char *text) {
  assert(std::strcmp(label,"GOVERNORCONTROLDONE")==0 && timeout==5000 && flags==0 && !text);
  events.push_back("popup"); } } PopupObject;
Popup *NetMsgPopup=&PopupObject;
void set_base(int id);
struct BaseWin { int governor(int value) { assert(value==1); ++GovernorCalls; events.push_back("governor"); return 0; } } TutWinBaseWindow;
int __cdecl diplo_lock(int);
class Console { public:
  int faction_id_=1; uint32_t field_23BE8_=1;
  void governor_directives();
};
Console ConsoleGlobal, Receiver;
Base *BaseCurrent=&Bases[0];
void set_base(int id) { ++SetBaseCalls; BaseCurrent=&Bases[id]; events.push_back("base:"+std::to_string(id)); }
'''


def test_governor_directives_preserves_faction_scoping_and_lock_order(tmp_path):
    compiler = shutil.which("g++")
    if not compiler:
        pytest.skip("host C++ compiler unavailable")
    harness = GOVERNOR_HARNESS + body_for("void Console::governor_directives()") + r'''
static void reset() {
  events.clear(); for(auto &base:Bases) base=Base{};
  for(auto &data:PlayersData) data=PlayerData{};
  BaseCurrentCount=4; ConsoleControlTurnActive=0; Directive=2; LockResult=0;
  SetBaseCalls=GovernorCalls=UnlockCalls=0; StringTemp[0]=0;
  ConsoleGlobal.faction_id_=1; ConsoleGlobal.field_23BE8_=1;
  Receiver.faction_id_=3; Receiver.field_23BE8_=1; BaseCurrent=&Bases[2];
  Bases[0].faction_id_current=1; Bases[1].faction_id_current=3;
  Bases[2].faction_id_current=2; Bases[3].faction_id_current=1;
  Bases[1].governor=0x81234567; Bases[2].governor=0x01234567;
  Bases[3].governor=0x01000001;
}
int main() {
  reset(); Receiver.governor_directives();
  assert(Bases[0].governor==0 && Bases[3].governor==0x01000001);
  assert(Bases[1].governor==0x81234567 && Bases[2].governor==0x82234567);
  assert(std::strcmp(StringTemp,"PSYCHPSYCH")==0);
  assert((events==std::vector<std::string>{"choice","lock","parse:PSYCHPSYCH","popup","unlock"}));
  reset(); Directive=5; PlayersData[3].base_governor_adv=0x87654321;
  Receiver.governor_directives();
  assert(SetBaseCalls==1 && GovernorCalls==1 && Bases[0].governor==0x87654321 &&
         Bases[3].governor==0x01000001);
  assert(events[1]=="base:0" && events[2]=="governor" && events[3]=="lock");
  reset(); Directive=5; LockResult=1;
  Receiver.governor_directives();
  assert(Bases[0].governor==0 && Bases[3].governor==0x01000001 && UnlockCalls==0);
  reset(); ConsoleControlTurnActive=1;
  Receiver.governor_directives();
  assert(events.empty());
}
'''
    unit = tmp_path / "console_governor.cpp"
    unit.write_text(harness)
    executable = tmp_path / "console_governor"
    subprocess.run([compiler, "-std=c++17", "-Wall", "-Wextra", "-Werror",
                    str(unit), "-o", str(executable)], check=True,
                   capture_output=True, text=True)
    subprocess.run([str(executable)], check=True, capture_output=True, text=True)


def test_assemble_filters_faction_and_applies_selected_action(tmp_path):
    compiler = shutil.which("g++")
    if not compiler:
        pytest.skip("host C++ compiler unavailable")
    harness = r'''
#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>
    #define __cdecl
    std::vector<std::string> events;
    const int TRIAD_LAND=0,TRIAD_SEA=1,TRIAD_AIR=2;
    struct Vehicle { int16_t x=0,y=0; uint32_t state=0; uint16_t flags=0,proto_id=0,unk=0;
  uint8_t faction_id=0,year_end_lurking=0,dmg_incurred=0; int8_t order=0;
  uint8_t waypoint_count=0,patrol_current_point=0; int16_t waypoint_x[4]{},waypoint_y[4]{}; };
struct Prototype { char name[32]{}; uint32_t ability_flags=0; uint8_t chassis_id=0,weapon_id=0,
  armor_id=0,reactor_id=0,carry_capacity=0,cost=0,plan=0; int8_t unk=0; };
using Veh=Vehicle; using VehPrototype=Prototype;
struct RulesChassis { int triad=0; } Chassis[32];
Vehicle Vehs[16]; Prototype VehPrototypes[32]; int VehCurrentCount=0;
int32_t *const ConsoleGroupCount=&VehCurrentCount;
int PopupWriteCheckState=0, ConsoleGroupAction=1;
int MapIsFlat=1; uint32_t MapLongitude=100,MapLongitudeBounds=200;
int PrefValue=0, PopupChoice=0, LastPrefsPut=-1, DrawCalls=0, GroupCalls=0;
int __cdecl prefs_get(const char *key,int fallback,int use_default) {
  assert(std::strcmp(key,"Vehicle Groups")==0 && fallback==7 && use_default==0);
  return PrefValue;
}
void __cdecl prefs_put(const char *key,int value,int toggle) {
  assert(std::strcmp(key,"Vehicle Groups")==0 && toggle==0); LastPrefsPut=value;
  events.push_back("prefs");
}
int __cdecl x_pop_short(const char *label,int flags,int (*callback)()) {
  assert(std::strcmp(label,"ASSEMBLEGROUP")==0 && flags==0x41 && !callback);
  events.push_back("popup"); return PopupChoice;
}
void __cdecl draw_map(int value) { assert(value==1); ++DrawCalls; events.push_back("draw"); }
struct Console { uint8_t prior_group_word[0x23D1C]{}; int field_23D1C_=0;
  int faction_id_=1, cursor_history_index_=0;
  int cursor_history_x_[32]{},cursor_history_y_[32]{};
  void clear_group(); void group(int id); void assemble(); } ConsoleGlobal;
void Console::group(int id) { ++GroupCalls; events.push_back("group:"+std::to_string(id)); }
'''
    harness += body_for("void Console::clear_group()")
    harness += body_for("void Console::assemble()")
    harness += r'''
static void reset() {
  events.clear(); for(auto &v:Vehs) v=Vehicle{}; for(auto &p:VehPrototypes) p=Prototype{};
  for(auto &c:Chassis) c=RulesChassis{};
  VehCurrentCount=3; PrefValue=1; PopupChoice=0; PopupWriteCheckState=0;
  ConsoleGroupAction=1; MapIsFlat=1; DrawCalls=GroupCalls=0; LastPrefsPut=-1;
  ConsoleGlobal.faction_id_=1; ConsoleGlobal.field_23D1C_=0;
  ConsoleGlobal.cursor_history_index_=0; ConsoleGlobal.cursor_history_x_[0]=10;
  ConsoleGlobal.cursor_history_y_[0]=10;
  Vehs[0].faction_id=1; Vehs[0].x=10; Vehs[0].y=10; Vehs[0].proto_id=1;
  Vehs[1].faction_id=2; Vehs[1].x=10; Vehs[1].y=10; Vehs[1].proto_id=2;
  Vehs[2].faction_id=1; Vehs[2].x=16; Vehs[2].y=10; Vehs[2].proto_id=3;
}
int main() {
  reset(); ConsoleGlobal.assemble();
  assert(GroupCalls==1 && events[1]=="group:0");
  assert(Vehs[0].order==0x18 && Vehs[0].waypoint_x[0]==10 && Vehs[0].waypoint_y[0]==10);
  assert((Vehs[0].state&0x08000000u)!=0 && Vehs[1].state==0 && Vehs[2].state==0);
  assert(ConsoleGlobal.field_23D1C_==1 && DrawCalls==1 && LastPrefsPut==1);
  reset(); PrefValue=0; ConsoleGroupAction=2;
  Vehs[0].state=Vehs[1].state=Vehs[2].state=0x400;
  Vehs[0].order=Vehs[1].order=Vehs[2].order=9;
  ConsoleGlobal.assemble();
  assert(Vehs[0].order==0 && Vehs[2].order==0 && Vehs[1].order==9);
  assert((Vehs[0].state&0x08000000u)!=0 && (Vehs[1].state&0x08000000u)==0 &&
         (Vehs[2].state&0x08000000u)!=0);
  reset(); PrefValue=8; VehPrototypes[1].chassis_id=5; Chassis[5].triad=0;
  VehPrototypes[3].chassis_id=6; Chassis[6].triad=1;
  ConsoleGlobal.assemble();
  assert((Vehs[0].state&0x08000000u)==0 && (Vehs[2].state&0x08000000u)!=0);
  reset(); PopupChoice=-1; ConsoleGlobal.assemble();
  assert(events.size()==1 && events[0]=="popup" && GroupCalls==0 && DrawCalls==0);
}
'''
    unit = tmp_path / "console_assemble.cpp"
    unit.write_text(harness)
    executable = tmp_path / "console_assemble"
    subprocess.run([compiler, "-std=c++17", "-Wall", "-Wextra", "-Werror",
                    str(unit), "-o", str(executable)], check=True,
                   capture_output=True, text=True)
    subprocess.run([str(executable)], check=True, capture_output=True, text=True)


def test_turn_hints_uses_typed_faction_tutorial_and_vehicle_state(tmp_path):
    compiler = shutil.which("g++")
    if not compiler:
        pytest.skip("host C++ compiler unavailable")
    harness = r'''
#include <cassert>
#include <cstdint>
#include <cstring>
#include <string>
#include <vector>
#define __cdecl
std::vector<std::string> events;
const uint32_t PREF_BSC_TUTORIAL_MSGS=0x20;
uint32_t GamePreferences=PREF_BSC_TUTORIAL_MSGS; int IsMultiplayerNet=0,TurnCurrentNum=1;
int GenderDefault=0,PluralityDefault=0,BaseFound=0,TutMask=0,PopupCalls=0;
const int MaxPlayerNum=8,DTREATY_COMMLINK=8,DTREATY_VENDETTA=0x10;
char *TutorialPopupCaption=const_cast<char*>("TUTOR");
struct Base { int16_t x=0,y=0; uint8_t faction_id_current=0; char name_string[25]{}; } Bases[8];
int BaseCurrentCount=8;
struct Veh { int16_t x=0,y=0; uint32_t state=0; uint16_t flags=0,proto_id=0,unk=0;
  uint8_t faction_id=0; } Vehs[8];
struct VehPrototype { char veh_name[32]{}; } VehPrototypes[8];
struct Player { int is_leader_female=0,noun_gender=0,is_noun_plural=0;
  char title_leader[24]{},name_leader[24]{},noun_faction[24]{}; } Players[8];
struct PlayerData { int current_num_bases=0,pop_total=0,last_turn_new_base=0;
  uint8_t proto_id_active[512]{},proto_id_queue[512]{}; uint32_t diplo_treaties[8]{}; } PlayersData[8];
struct TutWin { bool blocked=false; int stages=0,maps=0;
  bool tutorial_blocked() const { return blocked; }
  void set_tour_stage(int value) { assert(value==0); ++stages; events.push_back("stage"); }
  int tut_map(const char *text,int x,int y,int pixel,void *sprite,int forwarded,int ax,int ay) {
    assert(sprite==nullptr && ax==-1 && ay==-1); ++maps;
    events.push_back(std::string("map:")+text+":"+std::to_string(x)+":"+std::to_string(y)+":"+
      std::to_string(pixel)+":"+std::to_string(forwarded)); return 0;
  }
} Tutorial; TutWin *TutWinGlobal=&Tutorial;
int __cdecl tut_check(int mask) { events.push_back("check:"+std::to_string(mask)); return (TutMask&mask)!=0; }
int __cdecl base_find(int x,int y,int faction) { assert(faction==1); events.push_back("find:"+std::to_string(x)+":"+std::to_string(y)); return BaseFound; }
int __cdecl parse_says(int slot,const char *text,int,int) { events.push_back(std::string("parse:")+std::to_string(slot)+":"+text); return 0; }
int __cdecl x_pop_caption_label(char *caption,const char *label,int (*callback)()) {
  assert(std::strcmp(caption,"TUTOR")==0 && std::strcmp(label,"VENDETTA")==0 && !callback);
  ++PopupCalls; events.push_back("popup"); return 0;
}
struct Console { int faction_id_=1,selected_vehicle_=0,cursor_history_index_=0;
  int cursor_history_x_[32]{},cursor_history_y_[32]{}; int focus_calls=0;
  int focus(int x,int y,int faction) { assert(faction==1); ++focus_calls; events.push_back("focus:"+std::to_string(x)+":"+std::to_string(y)); return 0; }
  void turn_hints(); } ConsoleGlobal;
'''
    harness += body_for("void Console::turn_hints()")
    harness += r'''
static void reset() {
  events.clear(); for(auto &b:Bases) b=Base{}; for(auto &p:Players) p=Player{};
  for(auto &p:PlayersData) p=PlayerData{};
  for(auto &v:Vehs) v=Veh{};
  for(auto &v:VehPrototypes) v=VehPrototype{};
  GamePreferences=PREF_BSC_TUTORIAL_MSGS; IsMultiplayerNet=0; TurnCurrentNum=1;
  GenderDefault=PluralityDefault=BaseFound=TutMask=PopupCalls=0;
  Tutorial.blocked=false; Tutorial.stages=Tutorial.maps=0; ConsoleGlobal.focus_calls=0;
  ConsoleGlobal.faction_id_=1; ConsoleGlobal.selected_vehicle_=0;
  ConsoleGlobal.cursor_history_index_=2; ConsoleGlobal.cursor_history_x_[2]=12;
  ConsoleGlobal.cursor_history_y_[2]=14; std::strcpy(Bases[0].name_string,"Alpha");
  std::strcpy(VehPrototypes[0].veh_name,"Scout");
}
int main() {
  reset(); TutMask=1; ConsoleGlobal.turn_hints();
  assert(Tutorial.stages==1 && Tutorial.maps==1 &&
         events[2]=="parse:0:Alpha" && events[3]=="parse:1:Scout" &&
         events[4]=="stage" && events[5]=="map:LANDFALL:12:14:-1:256");
  reset(); TurnCurrentNum=20; TutMask=0x100000; BaseFound=2;
  PlayersData[1].current_num_bases=1; PlayersData[1].pop_total=2;
  Bases[2].x=30; Bases[2].y=32; std::strcpy(Bases[2].name_string,"Beta");
  ConsoleGlobal.turn_hints();
  assert(ConsoleGlobal.focus_calls==1 && PlayersData[1].last_turn_new_base==20 &&
         events.back()=="map:SECONDBASE:30:32:-4:0");
  reset(); Tutorial.blocked=true; ConsoleGlobal.turn_hints(); assert(events.empty());
  reset(); TurnCurrentNum=30; TutMask=0x80; PlayersData[1].diplo_treaties[4]=0x18;
  std::strcpy(Players[4].title_leader,"Leader"); std::strcpy(Players[4].name_leader,"Morgan");
  std::strcpy(Players[4].noun_faction,"Faction");
  ConsoleGlobal.turn_hints();
  assert(PopupCalls==1 && GenderDefault==0 && PluralityDefault==0 && events.back()=="popup");
}
'''
    unit = tmp_path / "console_turn_hints.cpp"
    unit.write_text(harness)
    executable = tmp_path / "console_turn_hints"
    subprocess.run([compiler, "-std=c++17", "-Wall", "-Wextra", "-Werror",
                    str(unit), "-o", str(executable)], check=True,
                   capture_output=True, text=True)
    subprocess.run([str(executable)], check=True, capture_output=True, text=True)


def test_sentry_checks_landing_transport_and_network_order(tmp_path):
    compiler = shutil.which("g++")
    if not compiler:
        pytest.skip("host C++ compiler unavailable")
    harness = r'''
#include <cassert>
#include <cstdint>
#include <cstring>
#include <string>
#include <vector>
#define __cdecl
std::vector<std::string> events;
const int TRIAD_LAND=0,TRIAD_SEA=1,TRIAD_AIR=2,MaxPlayerNum=8,RFLAG_ALIEN=0x80;
const uint32_t BIT_BASE_IN_TILE=1,BIT_AIRBASE=2;
char *ScriptPopupFile=const_cast<char*>("SCRIPT"); int PopupAnswer=0,StackResult=0,DrawCalls=0;
struct Veh { int16_t x=0,y=0; uint32_t state=0; uint16_t flags=0,proto_id=0,unk=0;
  uint8_t faction_id=0; int16_t prev_veh_id_stack=-1,next_veh_id_stack=-1,waypoint_x[4]{},waypoint_y[4]{}; } Vehs[8];
struct VehPrototype { uint8_t chassis_id=0; int plan=0; } VehPrototypes[8];
struct ChassisRecord { uint8_t triad=0; } Chassis[8];
struct Player { uint32_t rule_flags=0; } Players[8];
struct Map { uint32_t bit=0; uint8_t val2=0; } Tile;
Map *map_loc(int x,int y) { assert(x==4&&y==6); return &Tile; }
int stack_check(int id,int mode,int flags,int x,int y) {
  assert(id==0 && mode==6 && flags==0x80 && x==-1 && y==-1); return StackResult;
}
struct Network { int lock_result=0,lock_calls=0,unlock_calls=0;
  int lock_veh(int*,int mode,int x,int y,int flags) { assert(mode==0&&x==-1&&y==-1&&flags==0); ++lock_calls; events.push_back("lock"); return lock_result; }
  void await_synch() { events.push_back("await"); } void unlock_veh() { ++unlock_calls; events.push_back("unlock"); }
} NetDaemonNet;
int __cdecl popp(const char *file,const char *label,int value,const char *picture,void *callback) {
  assert(std::strcmp(file,"SCRIPT")==0 && value==0 && !callback);
  events.push_back(std::string("popup:")+label+":"+picture); return PopupAnswer;
}
void __cdecl sleep(int id) { assert(id==0); events.push_back("sleep"); }
void __cdecl synch_veh(int id) { assert(id==0); events.push_back("sync"); }
void __cdecl draw_tile(int x,int y,int mode) { assert(x==4&&y==6&&mode==1); ++DrawCalls; events.push_back("draw"); }
struct Console { int faction_id_=1,field_23BE4_=7; void sentry(int,int); } ConsoleGlobal;
'''
    harness += body_for("void Console::sentry(int vehicle_id, int parameter)")
    harness += r'''
static void reset() {
  events.clear(); for(auto &v:Vehs) v=Veh{}; for(auto &p:VehPrototypes) p=VehPrototype{};
  for(auto &c:Chassis) c=ChassisRecord{};
  for(auto &p:Players) p=Player{};
  Tile=Map{}; PopupAnswer=0; StackResult=0; DrawCalls=0;
  NetDaemonNet.lock_result=NetDaemonNet.lock_calls=NetDaemonNet.unlock_calls=0;
  ConsoleGlobal.faction_id_=1; ConsoleGlobal.field_23BE4_=7;
  Vehs[0].x=4; Vehs[0].y=6; Vehs[0].proto_id=0;
}
int main() {
  reset(); Chassis[0].triad=TRIAD_AIR; ConsoleGlobal.sentry(0,9);
  assert(NetDaemonNet.lock_calls==0 && events.empty());
  reset(); Chassis[0].triad=TRIAD_LAND; Vehs[0].next_veh_id_stack=1;
  Vehs[1].prev_veh_id_stack=0; Vehs[1].proto_id=1;
  Chassis[1].triad=TRIAD_AIR; VehPrototypes[1].chassis_id=1;
  VehPrototypes[1].plan=7; Players[1].rule_flags=RFLAG_ALIEN;
  ConsoleGlobal.sentry(0,9);
  assert(NetDaemonNet.lock_calls==0 && events.size()==1 &&
         events[0]=="popup:AIRLOAD:alair_sm.pcx");
  events.clear(); PopupAnswer=1; ConsoleGlobal.sentry(0,22);
  assert((events==std::vector<std::string>{"popup:AIRLOAD:alair_sm.pcx","lock","sleep","sync","await","unlock","draw"}));
  assert(Vehs[0].waypoint_y[0]==22 && Vehs[0].waypoint_x[2]==0 &&
         ConsoleGlobal.field_23BE4_==0 && DrawCalls==1);
  reset(); Chassis[0].triad=TRIAD_AIR; NetDaemonNet.lock_result=1;
  Tile.bit=BIT_BASE_IN_TILE; Tile.val2=0;
  ConsoleGlobal.sentry(0,1);
  assert(NetDaemonNet.lock_calls==1 && events==std::vector<std::string>{"lock"} &&
         ConsoleGlobal.field_23BE4_==7);
}
'''
    unit = tmp_path / "console_sentry.cpp"
    unit.write_text(harness)
    executable = tmp_path / "console_sentry"
    subprocess.run([compiler, "-std=c++17", "-Wall", "-Wextra", "-Werror",
                    str(unit), "-o", str(executable)], check=True,
                   capture_output=True, text=True)
    subprocess.run([str(executable)], check=True, capture_output=True, text=True)


def test_corner_market_parses_before_exits_and_reloads_after_callbacks(tmp_path):
    compiler = shutil.which("g++")
    if not compiler:
        pytest.skip("host C++ compiler unavailable")
    harness = r'''
#include <cassert>
#include <cstring>
#include <string>
#include <vector>
#define __cdecl
std::vector<std::string> events; int GameRules=4,TurnCurrentNum=30,BaseCurrentCount=1;
int IsMultiplayerNet=0,PopupAnswer=1,LockResult=0,Amount=40,UpdateTurn=44,UpdateActive=12;
const int FAC_HEADQUARTERS=1; char *ScriptPopupFile=const_cast<char*>("SCRIPT");
struct RuleSet { int tech_economic_victory=9,turns_corner_gbl_energy_mrkt=5; } Rules;
struct Base { int faction_id_current=0; } Bases[2];
struct PlayerData { int corner_market_turn=0,corner_market_active=0,energy_reserves=100; } PlayersData[8];
struct NetMsg { void pop(const char *label,int timeout,int flags,const char *text) {
  assert(timeout==5000&&flags==0&&!text); events.push_back(std::string("msg:")+label); }
} PopupObj; NetMsg *NetMsgPopup=&PopupObj;
int has_tech(int tech,int faction) { assert(tech==9&&faction==2); return 1; }
int has_fac_built(int facility,int base) { assert(facility==FAC_HEADQUARTERS); return base==0; }
int corner_market(int faction) { assert(faction==2); events.push_back("amount"); return Amount; }
int game_year(int turn) { events.push_back("year:"+std::to_string(turn)); return turn+1000; }
void parse_num(int slot,int value) { events.push_back("parse:"+std::to_string(slot)+":"+std::to_string(value)); }
int __cdecl popp(const char *file,const char *label,int value,const char *picture,void *cb) {
  assert(!std::strcmp(file,"SCRIPT")&&value==0&&
         !std::strcmp(picture,"econwin_sm.pcx")&&!cb);
  events.push_back(std::string("pop:")+label);
  if(!std::strcmp(label,"CORNERMARKET")) return PopupAnswer;
  return PopupAnswer;
}
int diplo_lock(int value) { assert(value==0x32); events.push_back("lock"); return LockResult; }
void diplo_unlock() { events.push_back("unlock"); }
void synch_leader(int faction) { assert(faction==2); events.push_back("sync");
  PlayersData[2].corner_market_turn=46; }
void message_data(int code,int a,int faction,int turn,int amount,int tail) {
  assert(code==0x244F&&a==0&&faction==2&&turn==46&&amount==40&&tail==0);
  events.push_back("message");
}
struct Console { int faction_id_=2; void update_data(int); void corner_the_market(); } ConsoleGlobal;
void Console::update_data(int value) { assert(value==0); events.push_back("update");
  PlayersData[2].corner_market_turn=UpdateTurn; PlayersData[2].corner_market_active=UpdateActive; }
'''
    harness += body_for("void Console::corner_the_market()")
    harness += r'''
static void reset() { events.clear(); GameRules=4; TurnCurrentNum=30; PopupAnswer=1; LockResult=0;
  Amount=40; UpdateTurn=44; UpdateActive=12; Rules.turns_corner_gbl_energy_mrkt=5;
  Bases[0].faction_id_current=2; PlayersData[2]=PlayerData{}; }
int main() {
  reset(); PlayersData[2].corner_market_turn=35; ConsoleGlobal.corner_the_market();
  assert((events==std::vector<std::string>{"amount","parse:0:40","year:35",
    "parse:1:1035","year:35","parse:0:1035","msg:CORNERALREADY"}));
  reset(); PlayersData[2].energy_reserves=2; ConsoleGlobal.corner_the_market();
  assert(events[0]=="amount" && events[1]=="parse:0:40" && events[2]=="year:35" &&
         events[3]=="parse:1:1035" && events.back()=="msg:CORNERNOTYET");
  reset(); ConsoleGlobal.corner_the_market();
  assert(PlayersData[2].corner_market_turn==44 && PlayersData[2].corner_market_active==12 &&
         PlayersData[2].energy_reserves==60);
  assert((events==std::vector<std::string>{"amount","parse:0:40","year:35",
    "parse:1:1035","pop:CORNERMARKET","lock","sync","message","unlock","update",
    "year:44","parse:0:1044","parse:1:12","pop:CORNERING"}));
}
'''
    unit = tmp_path / "console_corner_market.cpp"
    unit.write_text(harness)
    executable = tmp_path / "console_corner_market"
    subprocess.run([compiler, "-std=c++17", "-Wall", "-Wextra", "-Werror",
                    str(unit), "-o", str(executable)], check=True,
                   capture_output=True, text=True)
    subprocess.run([str(executable)], check=True, capture_output=True, text=True)


def test_oblit_validates_ownership_then_confirms_and_dispatches(tmp_path):
    compiler = shutil.which("g++")
    if not compiler:
        pytest.skip("host C++ compiler unavailable")
    harness = r'''
#include <cassert>
#include <cstdint>
#include <cstring>
#include <string>
#include <vector>
#define __cdecl
std::vector<std::string> events; int TurnDenied=0,Ready=1,BaseId=0,Objective=0;
int LockResult=0,PopupAnswer=1,IsMultiplayerNet=0,AddGlobalResult=0,ActionCalls=0;
const uint32_t RFLAG_ALIEN=0x80; char *ScriptPopupFile=const_cast<char*>("SCRIPT");
struct Veh { int x=4,y=6,faction_id=2; } Vehs[4];
struct Base { int faction_id_current=2; char name_string[25]="Delta"; } Bases[4];
struct Player { uint32_t rule_flags=0; } Players[8];
struct RulesData { int tgl_oblit_base_atrocity=1; } Rules;
struct NetMsg { void pop(const char *label,int timeout,int flags,const char *text) {
  assert(timeout==5000&&flags==0&&!text); events.push_back(std::string("msg:")+label); }
} PopupObj; NetMsg *NetMsgPopup=&PopupObj;
int not_my_turn() { return TurnDenied; } int veh_ready(int id) { assert(id==0); return Ready; }
int base_at(int x,int y) { assert(x==4&&y==6); return BaseId; }
int is_objective(int id) { assert(id==0); return Objective; }
int __cdecl popp(const char *file,const char *label,int value,const char *picture,void *cb) {
  assert(!std::strcmp(file,"SCRIPT")&&value==0&&!cb&&
         !std::strcmp(picture,"colfoun_sm.pcx")); events.push_back(std::string("pop:")+label);
  return PopupAnswer;
}
int __cdecl parse_says(int slot,const char *text,int x,int y) {
  assert(slot==0&&x==-1&&y==-1&&!std::strcmp(text,"Delta")); events.push_back("parse"); return 0;
}
struct Network { int lock_veh(int*,int mode,int x,int y,int flags) {
  assert(mode==4&&x==-1&&y==-1&&flags==0); events.push_back("lock"); return LockResult; }
  int add_global() { events.push_back("add"); return AddGlobalResult; }
  void unlock_veh() { events.push_back("unlock"); }
  void await_exec(int value) { assert(value==1); events.push_back("await"); }
} NetDaemonNet;
void action_oblit(int vehicle,int base) { assert(vehicle==0&&base==0); ++ActionCalls; events.push_back("action"); }
void message_veh(int code,int vehicle,int base,int unused) {
  assert(code==0x2409&&vehicle==0&&base==0&&unused==0); events.push_back("message"); }
struct Console { void oblit(int); } ConsoleGlobal;
'''
    harness += body_for("void Console::oblit(int vehicle_id)")
    harness += r'''
int main() {
  ConsoleGlobal.oblit(0);
  assert((events==std::vector<std::string>{"lock","parse","pop:OBLIT","add","action"}));
  events.clear(); Objective=1; ConsoleGlobal.oblit(0);
  assert((events==std::vector<std::string>{"msg:OBLITNOTOBJECTIVE"}));
  events.clear(); Objective=0; IsMultiplayerNet=1; ConsoleGlobal.oblit(0);
  assert((events==std::vector<std::string>{"lock","parse","pop:OBLIT","add","message","await"}));
  events.clear(); IsMultiplayerNet=0; PopupAnswer=0; ConsoleGlobal.oblit(0);
  assert((events==std::vector<std::string>{"lock","parse","pop:OBLIT","unlock"}));
}
'''
    unit = tmp_path / "console_oblit.cpp"
    unit.write_text(harness)
    executable = tmp_path / "console_oblit"
    subprocess.run([compiler, "-std=c++17", "-Wall", "-Wextra", "-Werror",
                    str(unit), "-o", str(executable)], check=True,
                   capture_output=True, text=True)
    subprocess.run([str(executable)], check=True, capture_output=True, text=True)


def test_give_vehicle_scopes_owner_receiver_and_dispatch(tmp_path):
    compiler = shutil.which("g++")
    if not compiler:
        pytest.skip("host C++ compiler unavailable")
    harness = r'''
#include <cassert>
#include <cstdint>
#include <cstring>
#include <string>
#include <vector>
#define __cdecl
std::vector<std::string> events; const int DTREATY_COMMLINK=8,DTREATY_VENDETTA=0x10;
const int TRIAD_LAND=0,TRIAD_SEA=1,TRIAD_AIR=2; const uint32_t RFLAG_ALIEN=0x80;
const uint32_t BIT_BASE_IN_TILE=1,BIT_AIRBASE=0x40000; int GenderDefault=0,PluralityDefault=0;
char *ScriptPopupFile=const_cast<char*>("SCRIPT"); int IsMultiplayerNet=0;
int Territory=4,PopupAnswer=1,GiveCalls=0,GiveRecipient=-1,MessageCalls=0;
extern int *ReceiverFactionSlot; int ReceiverAfterTerritory=-1,ReceiverAfterFirstParse=-1;
int PrototypeAfterFirstParse=-1,ReceiverAfterSecondParse=-1,PrototypeAfterSecondParse=-1;
struct Veh { int16_t x=4,y=6,proto_id=0; uint32_t state=0; uint16_t flags=0,unk=0;
  uint8_t faction_id=2; int8_t order=0; } Vehs[8];
struct VehPrototype { char veh_name[32]="Scout"; uint32_t ability_flags=0; uint8_t chassis_id=0,weapon_id=0;
  uint8_t armor_id=0,reactor_id=0,carry_capacity=0,cost=0,plan=0; int8_t unk=0; } VehPrototypes[8];
struct RulesChassis { int triad=0; } Chassis[8];
struct RulesWeapon { int8_t offense_rating=-1; } Weapon[8];
struct Player { int rule_flags=0,noun_gender=0,is_noun_plural=0; char noun_faction[24]="Faction"; } Players[8];
struct PlayerData { uint32_t diplo_treaties[8]{}; } PlayersData[8];
struct Map { uint32_t bit=1; uint8_t val2=3; } Tile;
Map *map_loc(int x,int y) { assert(x==4&&y==6); return &Tile; }
struct Popup { void pop(const char *label,int timeout,int flags,const char *text) {
  assert(timeout==5000&&flags==0&&!text); events.push_back(std::string("msg:")+label); }
} PopupObj; Popup *NetMsgPopup=&PopupObj;
struct Network { int locked=0,unlocked=0;
  int lock_veh(int *id,int mode,int x,int y,int flags) { assert(*id==0&&mode==4&&x==-1&&y==-1&&flags==0); events.push_back("lock"); return locked; }
  void unlock_veh() { ++unlocked; events.push_back("unlock"); }
  void await_exec(int value) { assert(value==1); events.push_back("await"); }
} NetDaemonNet;
int whose_territory(int faction,int x,int y,int *base,int *unused) {
  assert(faction==2&&x==4&&y==6&&!base&&!unused);
  if (ReceiverAfterTerritory>=0) *ReceiverFactionSlot=ReceiverAfterTerritory;
  return Territory;
}
void parse_says(int slot,const char *text,int x,int y) {
  assert(x==-1&&y==-1); events.push_back(std::string("parse:")+std::to_string(slot)+":"+text);
  if (slot==0&&ReceiverAfterFirstParse>=0) *ReceiverFactionSlot=ReceiverAfterFirstParse;
  if (slot==0&&PrototypeAfterFirstParse>=0) Vehs[0].proto_id=PrototypeAfterFirstParse;
  if (slot==1&&ReceiverAfterSecondParse>=0) *ReceiverFactionSlot=ReceiverAfterSecondParse;
  if (slot==1&&PrototypeAfterSecondParse>=0) Vehs[0].proto_id=PrototypeAfterSecondParse;
}
int __cdecl popp(const char *file,const char *label,int flags,const char *picture,void *callback) {
  assert(!std::strcmp(file,"SCRIPT")&&flags==0&&!callback);
  events.push_back(std::string("pop:")+label+":"+picture); return PopupAnswer;
}
void action_give(int id,int recipient) { assert(id==0); ++GiveCalls; GiveRecipient=recipient; events.push_back("give"); }
void message_veh(int op,int id,int recipient,int tail) {
  assert(op==0x2416&&id==0&&recipient==4&&tail==0); ++MessageCalls; events.push_back("message"); }
struct Console { int faction_id_; void give_veh(int); } ConsoleGlobal{2},Receiver{3};
int *ReceiverFactionSlot=&Receiver.faction_id_;
'''
    harness += body_for("void Console::give_veh(int vehicle_id)")
    harness += r'''
int main() {
  PlayersData[3].diplo_treaties[4]=DTREATY_COMMLINK;
  Receiver.give_veh(0);
  assert(GiveCalls==1&&GiveRecipient==4&&NetDaemonNet.unlocked==0);
  assert((events==std::vector<std::string>{"lock","parse:0:Faction","parse:1:Scout",
    "pop:CANGIVE2:native_sm.pcx","give"}));
  events.clear(); GiveCalls=0; Territory=2; PlayersData[3].diplo_treaties[2]=DTREATY_COMMLINK;
  PopupAnswer=2; Receiver.give_veh(0);
  assert(GiveCalls==1&&GiveRecipient==0);
  events.clear(); GiveCalls=0; Territory=4; IsMultiplayerNet=1; PopupAnswer=1;
  Receiver.give_veh(0);
  assert(MessageCalls==1&&GiveCalls==0&&events.back()=="await");
  events.clear(); IsMultiplayerNet=0; Territory=4; Weapon[0].offense_rating=1;
  PlayersData[3].diplo_treaties[4]=0; Receiver.give_veh(0);
  assert(GiveCalls==0&&NetDaemonNet.unlocked==1&&events.back()=="msg:CANTGIVE");
  events.clear(); Weapon[0].offense_rating=-1; Chassis[0].triad=TRIAD_AIR;
  Territory=4; PlayersData[3].diplo_treaties[4]=DTREATY_COMMLINK; Tile.bit=0;
  Receiver.give_veh(0);
  assert(GiveCalls==0&&NetDaemonNet.unlocked==2&&events.back()=="msg:CANTGIVEAIR");
  events.clear(); NetDaemonNet.unlocked=0; IsMultiplayerNet=0; Territory=4;
  Tile.bit=BIT_BASE_IN_TILE; Weapon[0].offense_rating=1; Vehs[0].proto_id=0;
  Receiver.faction_id_=3; ReceiverAfterTerritory=5;
  PlayersData[5].diplo_treaties[4]=DTREATY_COMMLINK;
  std::strcpy(VehPrototypes[5].veh_name,"ChangedScout");
  VehPrototypes[6].chassis_id=6; Chassis[6].triad=TRIAD_LAND;
  Players[7].rule_flags=RFLAG_ALIEN;
  PrototypeAfterFirstParse=5; ReceiverAfterSecondParse=7; PrototypeAfterSecondParse=6;
  GiveCalls=0; Receiver.give_veh(0);
  assert(events[1]=="parse:0:Faction" && events[2]=="parse:1:ChangedScout");
  assert(events[3]=="pop:CANGIVE:alunit_sm.pcx" && GiveCalls==1&&GiveRecipient==4);
}
'''
    unit = tmp_path / "console_give_vehicle.cpp"
    unit.write_text(harness)
    executable = tmp_path / "console_give_vehicle"
    subprocess.run([compiler, "-std=c++17", "-Wall", "-Wextra", "-Werror",
                    str(unit), "-o", str(executable)], check=True,
                   capture_output=True, text=True)
    subprocess.run([str(executable)], check=True, capture_output=True, text=True)


def test_retire_game_handles_prompt_pbem_and_network_restart(tmp_path):
    compiler = shutil.which("g++")
    if not compiler:
        pytest.skip("host C++ compiler unavailable")
    harness = r'''
#include <cassert>
#include <cstdint>
#include <string>
#include <vector>
#define __cdecl
std::vector<std::string> events;
const int RULES_IRONMAN=0x400,STATE_IS_SCENARIO=0x10;
int GameRules=0,GameState=0,IsMultiplayerNet=0,IsMultiplayerPBEM=0;
uint32_t LockEnableMask=1; int ExitTurnLoop=0,GameRestartQueued=0,PromptResult=1;
int SavedFaction=0,QuayleFaction=-1,SaveCalls=0,AutoSaveCalls=0,KillAutoSaveCalls=0;
int ReportCalls=0,HallCalls=0,ReplayCalls=0;
int bit_count(unsigned int bits) { int count=0; while(bits) { count+=bits&1; bits>>=1; } return count; }
void auto_save() { ++AutoSaveCalls; events.push_back("auto_save"); }
void kill_auto_save() { ++KillAutoSaveCalls; events.push_back("kill_auto_save"); }
int save_game(int slot) { assert(slot==0); ++SaveCalls; events.push_back("save"); return 0; }
int X_pop(const char *label,const char *text) { assert(!text); events.push_back(std::string("prompt:")+label); return PromptResult; }
void report_score(int value) { assert(value==1); ++ReportCalls; events.push_back("score"); }
void quayle(int faction) { QuayleFaction=faction; events.push_back("quayle"); }
void hall_of_fame(int retire) { assert(retire==1); ++HallCalls; events.push_back("hall"); }
void show_replay() { ++ReplayCalls; events.push_back("replay"); }
void log_say(const char *message,int a,int b,int c) {
  assert(std::string(message)=="Retire**"&&a==0&&b==0&&c==0); events.push_back("log"); }
struct AlphaNet { void close() { events.push_back("close"); } };
struct Lock { void clear() { events.push_back("clear_locks"); } };
struct LockStateEntry { uint32_t dword_[15]{}; };
struct NetDaemon:AlphaNet { Lock locks_; LockStateEntry lock_state_[24]{};
  void hang_up() { events.push_back("hang_up"); } } NetDaemonNet;
struct Console { int faction_id_=3; void retire_game(); } ConsoleGlobal;
'''
    harness += body_for("inline void console_close_network(unsigned int final_channel_mask)")
    harness += body_for("void Console::retire_game()")
    harness += r'''
static void reset() { events.clear(); GameRules=GameState=0; IsMultiplayerNet=IsMultiplayerPBEM=0;
  LockEnableMask=1; ExitTurnLoop=GameRestartQueued=0; PromptResult=1; SaveCalls=AutoSaveCalls=0;
  KillAutoSaveCalls=ReportCalls=HallCalls=ReplayCalls=0; QuayleFaction=-1; }
int main() {
  reset(); GameRules=RULES_IRONMAN; PromptResult=0; ConsoleGlobal.retire_game();
  assert((events==std::vector<std::string>{"auto_save","prompt:REALLYRETIRE2"}));
  assert(GameRestartQueued==0&&KillAutoSaveCalls==0);
  reset(); GameRules=RULES_IRONMAN; IsMultiplayerPBEM=1; LockEnableMask=0xD;
  ConsoleGlobal.retire_game();
  assert((events==std::vector<std::string>{"auto_save","prompt:REALLYWIMPOUT","save",
    "kill_auto_save","score","quayle","hall","replay","log"}));
  assert(SaveCalls==1&&LockEnableMask==5&&QuayleFaction==3&&ExitTurnLoop==1&&GameRestartQueued==1);
  reset(); IsMultiplayerNet=1; GameState=STATE_IS_SCENARIO; LockEnableMask=1;
  ConsoleGlobal.retire_game();
  assert((events==std::vector<std::string>{"prompt:REALLYRETIRE","hang_up","clear_locks",
    "close","score","log"}));
  assert(!IsMultiplayerNet&&ExitTurnLoop==1&&GameRestartQueued==0&&HallCalls==0&&ReplayCalls==0);
  assert(NetDaemonNet.lock_state_[2].dword_[11]==0xFF);
}
'''
    unit = tmp_path / "console_retire_game.cpp"
    unit.write_text(harness)
    executable = tmp_path / "console_retire_game"
    subprocess.run([compiler, "-std=c++17", "-Wall", "-Wextra", "-Werror",
                    str(unit), "-o", str(executable)], check=True,
                   capture_output=True, text=True)
    subprocess.run([str(executable)], check=True, capture_output=True, text=True)
