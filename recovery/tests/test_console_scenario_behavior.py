"""Host controls compile the actual Console scenario-editor method bodies.

These fixtures cover the tech toggle and artifact-unit placement seams. They
check ordering and effects against service doubles; they are not native-layout,
gameplay, UI, or whole-recovery acceptance evidence.
"""

from pathlib import Path
import shutil
import subprocess

import pytest


ROOT = None
for parent in Path(__file__).resolve().parents:
    if (parent / "src/console.cpp").is_file():
        ROOT = parent
        break
if ROOT is None:
    raise RuntimeError("cannot locate actual OpenSMACX checkout")


def test_actual_console_capture_release_and_end_turn(tmp_path):
    """Own-source branch controls, not a native-layout or game proof."""
    compiler = shutil.which("g++")
    if not compiler:
        pytest.skip("host C++ compiler unavailable")
    source = (ROOT / "src/console.cpp").read_text()
    handlers = "\n".join(_body(source, signature) for signature in (
        "void Console::on_nc_left_up", "void Console::end_my_turn"))
    harness = r'''
#include <cassert>
#include <cstdint>
#include <vector>
struct Rect { int left, top, right, bottom; };
struct Update { int x,y,w,h; };
std::vector<Update> updates;
std::vector<int> events, genders, pluralities;
struct Win { virtual void on_lose_mouse_capture() { events.push_back(9); } };
struct GraphicWin : Win {
  void update(int x,int y,int w,int h,GraphicWin*) { updates.push_back(Update{x,y,w,h}); }
};
struct MapWin : GraphicWin {};
struct Sprite { int w=20,h=20; int width() const { return w; } int height() const { return h; } };
struct Console : MapWin {
  void on_nc_left_up(int,int); void end_my_turn();
  void set_move(int n) { assert(n==1); events.push_back(5); }
  int edit_lock() { return locked; }
  int ready_search(int) { return next_ready; }
  int locked=0, next_ready=-1, faction_id_=1, cursor_active_=0;
  uint32_t field_23D80_=100,field_23D84_=200,field_23D88_=300,field_23D8C_=400;
  Rect client_rect_{0,10,500,410},outer_rect_{0,0,500,420};
  int border_thickness_=4,bottom_border_thickness_=2,caption_height_=20;
  struct { struct { int biWidth=640,biHeight=-480; } bmiHeader; } dib_;
  Sprite sprite_a_,sprite_b_;
  uint32_t field_23BE4_=7,field_23BE8_=1;
  int selected_vehicle_=12,focus_vehicle_=-1;
} ConsoleGlobal;
Win *WinPointerOwner3=0,*WinPointerOwner4=0;
struct GameDataStateFields {
  uint32_t game_state_=2; int turn_current_num_=30;
  uint32_t net_turn_flags_=0; int net_turn_faction_=2;
  uint8_t game_tech_achieved_[89]={};
  struct { uint32_t enable_mask_=0; } lock_masks_;
};
struct GameDataStateFixture { GameDataStateFields fields_; } GameDataStateStorage;
uint32_t &GameState=GameDataStateStorage.fields_.game_state_;
uint32_t &NetTurnFlags=GameDataStateStorage.fields_.net_turn_flags_;
int &NetTurnFaction=GameDataStateStorage.fields_.net_turn_faction_;
int &TurnCurrentNum=GameDataStateStorage.fields_.turn_current_num_;
uint32_t ConsoleDragLineX=20,ConsoleDragLineY=30;
int ConsoleControlTurnActive=0,IsMultiplayerNet=0;
int GenderDefault=0,PluralityDefault=0,ConsoleFactionsWaitingMask=0;
struct InfoWin { int field_A8_=9,redraw_phase=7;
  void clear_pending_timer() { field_A8_=0; events.push_back(1); }
} ConsoleInfoWin;
struct BaseWin { void cancel_zoom_wait() { events.push_back(0); } } TutWinBaseWindow;
struct TutWin { void reset() { events.push_back(2); } } tutorial;
TutWin *TutWinGlobal=&tutorial;
struct StatusWin { void reset() { events.push_back(3); } } ConsoleStatusWin;
struct FX { void play(int id) { assert(id==15); events.push_back(4); } } g_FX;
struct Player { int is_leader_female=2,noun_gender=3,is_noun_plural=1;
  char title_leader[16]="title",name_leader[16]="name",noun_faction[16]="faction";
} Players[8];
struct NetDaemon { int who_2_pid(int n) { assert(n==2); return 11; }
  char *get_player_name(int n) { assert(n==11); return Players[2].name_leader; }
} NetDaemonNet;
const char *popup_caption=0;
struct NetMsg { void pop(const char *s,int n,int m,const char*) {
  assert(n==5000 && m==0); popup_caption=s;
} } popup;
NetMsg &NetMsgPrimary = popup;
int parse_says(int,char*,int,int) { genders.push_back(GenderDefault); pluralities.push_back(PluralityDefault); return 0; }
int message_calls=0;
int message_data(int code,int,int turn,int,int,int) { assert(code==0x2302 && turn==30); ++message_calls; return 0; }
int ready=0,pop_result=1,pop_calls=0;
int veh_ready(int) { return ready; }
int X_pop(const char *caption,int(*)()) { assert(caption[0]=='R'); ++pop_calls; return pop_result; }
''' + handlers + r'''
int main() {
  Console window;
  WinPointerOwner3=&window; WinPointerOwner4=&window;
  window.field_23D88_=0xFFFFFFFFu;
  window.on_nc_left_up(70,50);
  assert(!WinPointerOwner3 && !WinPointerOwner4 && events.size()==1 && events[0]==9);
  assert(window.field_23D88_==72 && window.field_23D8C_==400);
  assert(updates.size()==2 && updates[0].x==0 && updates[0].y==30 && updates[0].w==640 && updates[0].h==1);
  assert(updates[1].x==20 && updates[1].y==0 && updates[1].w==1 && updates[1].h==480);
  window.field_23D88_=window.field_23D8C_=0xFFFFFFFFu;
  window.on_nc_left_up(70,50);
  assert(window.field_23D88_==0xFFFFFFFFu && window.field_23D8C_==72);
  window.field_23D80_=0xFFFFFFFFu; window.field_23D84_=100;
  window.field_23D88_=window.field_23D8C_=0xFFFFFFFFu;
  window.on_nc_left_up(70,50);
  assert(window.field_23D80_==66 && window.field_23D84_==100);
  assert(window.field_23D88_==0xFFFFFFFFu && window.field_23D8C_==0xFFFFFFFFu);
  events.clear(); window.end_my_turn();
  assert(events==std::vector<int>({0,1,2,3,4}));
  assert(ConsoleInfoWin.field_A8_==0 && ConsoleInfoWin.redraw_phase==7);
  assert(window.field_23BE4_==0 && window.field_23BE8_==0 && window.focus_vehicle_==12);
  GameState=0; ConsoleGlobal.cursor_active_=1; window.field_23BE8_=1;
  ready=1; pop_result=0; window.end_my_turn();
  assert(pop_calls==1 && window.field_23BE8_==1);
  ready=0; ConsoleGlobal.next_ready=13; window.end_my_turn();
  assert(pop_calls==2 && window.field_23BE8_==1);
  ConsoleGlobal.next_ready=-1; window.end_my_turn();
  assert(pop_calls==2 && window.field_23BE8_==0 && (GameState&2));
  window.field_23BE8_=1; window.locked=1; popup_caption=0; window.end_my_turn();
  assert(popup_caption && popup_caption[0]=='P' && window.field_23BE8_==1);
  window.locked=0; IsMultiplayerNet=1; NetTurnFlags=0x10;
  genders.clear(); pluralities.clear(); window.end_my_turn();
  assert(popup_caption[0]=='S' && window.field_23BE8_==1);
  assert(genders==std::vector<int>({2,2,3,3}) && pluralities==std::vector<int>({0,0,1,1}));
  NetTurnFlags=0; ConsoleFactionsWaitingMask=2; GameState=2;
  window.end_my_turn(); assert(popup_caption[0]=='A' && message_calls==1 && window.field_23BE8_==1);
  ConsoleFactionsWaitingMask=255; window.end_my_turn();
  assert(window.field_23BE8_==0 && message_calls==1);
}
'''
    fixture = tmp_path / "console_capture_turn.cpp"
    fixture.write_text(harness)
    executable = tmp_path / "console_capture_turn"
    subprocess.run([compiler, "-std=c++11", str(fixture), "-o", str(executable)], check=True)
    subprocess.run([str(executable)], check=True)


def _body(source: str, signature: str) -> str:
    start = source.index(signature)
    opening = source.index("{", start)
    depth = 0
    for end in range(opening, len(source)):
        depth += (source[end] == "{") - (source[end] == "}")
        if depth == 0:
            return source[start:end + 1]
    raise AssertionError(f"unterminated production body: {signature}")


def test_editor_veh_artifact_path_uses_typed_tile_and_vehicle_calls(tmp_path):
    compiler = shutil.which("g++")
    if not compiler:
        pytest.skip("host C++ compiler unavailable")
    source = (ROOT / "src/console.cpp").read_text()
    body = _body(source, "void Console::editor_veh()")
    harness = r'''
#include <cassert>
#include <cstdint>
#include <cstring>
#include <string>
#include <vector>
std::vector<std::string> events;
struct Dialogs { void item(char*, int id) { events.push_back("item:"+std::to_string(id)); } };
struct Popup {
  Dialogs list;
  Popup() { events.push_back("construct"); }
  ~Popup() { events.push_back("destroy"); }
  int start(char*, const char*, int, char*, int, void*) { events.push_back("start"); return 0; }
  int start(char*, const char*, int, char*, int) { events.push_back("start"); return 0; }
  Dialogs& dialogs() { return list; }
  void set_value(int) { events.push_back("select"); }
  int exec(int, void*) { events.push_back("exec"); return 19; }
  unsigned input_poll_state() const { return 0; }
  int init(int,int) { return 0; }
};
struct GraphicWin { void redraw() {} };
struct Console : GraphicWin {
  int cursor_history_index_=0, cursor_history_x_[32]={}, cursor_history_y_[32]={};
  int faction_id_=1, selected_vehicle_=-1, cursor_active_=0;
  void auto_undo(); void set_move(int); void editor_veh();
} ConsoleGlobal;
struct VehPrototype { char veh_name[32]; uint16_t flags; uint8_t chassis_id; };
struct Vehicle { int16_t x=0,y=0,proto_id=19; uint32_t state=0; uint16_t flags=0;
  uint8_t faction_id=1,morale=0; };
VehPrototype VehPrototypes[512]; Vehicle Vehs[16];
struct RulesMorale { int name; int life; } Morale[7];
struct Player { int noun_gender=0,is_noun_plural=0; char noun_faction[16]="Faction"; } Players[8];
struct Table { char* get(int) { return const_cast<char*>("Morale"); } } StringTable;
char PopupStartCaption[256], StringTemp[1024], StrEditorWhoListTitle[32];
int PopupModalState=0, GenderDefault=0, PluralityDefault=0;
int ScenarioVehicleFaction=1, ScenarioVehicleMorale=2, ScenarioVehicleMode=0,
    ScenarioPreviousPrototype=19;
struct GameDataLockMaskFixture { uint32_t enable_mask_=0; } GameDataLockMask;
struct GameDataStateFields { GameDataLockMaskFixture lock_masks_; } ;
struct GameDataStateFixture { GameDataStateFields fields_; } GameDataStateStorage;
uint32_t &LockEnableMask=GameDataStateStorage.fields_.lock_masks_.enable_mask_;
const int MaxVehProtoNum=512, MaxPlayerNum=8, MaxMoraleNum=7;
void auto_undo() { events.push_back("undo"); }
void Console::auto_undo() { ::auto_undo(); }
void Console::set_move(int) { events.push_back("move"); }
int veh_avail(int,int,int) { return 1; }
int veh_init(int proto,int faction,int x,int y) {
  assert(proto==19 && faction==0 && x==27 && y==31);
  events.push_back("veh_init"); Vehs[2].proto_id=proto; Vehs[2].faction_id=faction; return 2;
}
void bit_set(int x,int y,int bit,int set) {
  assert(x==27 && y==31);
  if(bit==0x20) { assert(set==1); events.push_back("bit:fungus"); }
  else { assert(bit==0x28805C && set==0); events.push_back("bit:artifact-clear"); }
}
void synch_bit(int x,int y,int faction) {
  assert(x==27 && y==31 && faction>=1 && faction<=7);
  events.push_back("sync:"+std::to_string(faction));
}
void draw_tile(int x,int y,int mode) { assert(x==27&&y==31&&mode==1); events.push_back("draw"); }
void draw_tile_fixup(int x,int y,int flag,int mode) {
  assert(x==27&&y==31&&flag==2&&mode==2); events.push_back("fixup");
}
'''+body+r'''
int main() {
  ConsoleGlobal.faction_id_=0;
  ConsoleGlobal.cursor_history_index_=3;
  ConsoleGlobal.cursor_history_x_[3]=27; ConsoleGlobal.cursor_history_y_[3]=31;
  VehPrototypes[19].flags=3; std::strcpy(VehPrototypes[19].veh_name,"Artifact");
  events.clear(); ConsoleGlobal.editor_veh();
  const std::vector<std::string> expected={"construct","undo","start","item:19",
    "select","exec","bit:fungus","bit:artifact-clear","sync:1","sync:2",
    "sync:3","sync:4","sync:5","sync:6","sync:7","draw","veh_init",
    "fixup","destroy"};
  assert(events==expected);
  assert(ScenarioVehicleFaction==0 && Vehs[2].morale==2);
  assert(ConsoleGlobal.selected_vehicle_==2 && (static_cast<unsigned char*>(
    static_cast<void*>(&LockEnableMask))[1]&1)==1);
}
'''
    unit = tmp_path / "scenario_veh.cpp"
    unit.write_text(harness)
    executable = tmp_path / "scenario_veh"
    subprocess.run([compiler, "-std=c++17", "-Wall", "-Wextra", "-Werror",
                    str(unit), "-o", str(executable)], check=True,
                   capture_output=True, text=True)
    subprocess.run([str(executable)], check=True, capture_output=True, text=True)


def test_editor_tech_toggle_and_cancel_lifecycle(tmp_path):
    compiler = shutil.which("g++")
    if not compiler:
        pytest.skip("host C++ compiler unavailable")
    source = (ROOT / "src/console.cpp").read_text()
    body = _body(source, "void Console::editor_tech()")
    harness = r'''
#include <cassert>
#include <cstdint>
#include <cstring>
#include <string>
#include <vector>
std::vector<std::string> events; std::vector<int> results={0,-1}; int result_index=0;
struct GraphicWin { void redraw() { events.push_back("redraw"); } };
GraphicWin TutWinBaseWindow;
struct Dialogs { void item(char*,int id) { events.push_back("item:"+std::to_string(id)); } };
struct StringList { void sort(int key) { assert(key==0); events.push_back("sort"); } };
struct Popup { Dialogs list; StringList item_ids_;
  Popup(){events.push_back("construct");} ~Popup(){events.push_back("destroy");}
  int start(char*,const char*,int,char*,int){events.push_back("start");return 0;}
  void set_cancel_text(char*){events.push_back("cancel-text");}
  void set_ok_text(char*){events.push_back("ok-text");}
  Dialogs& dialogs(){return list;} void set_value(int){events.push_back("select");}
  int exec(int,void*){events.push_back("exec");return results[result_index++];}
  unsigned input_poll_state() const{return 0;}
};
struct Console:GraphicWin { int faction_id_=1; void editor_tech(); } ConsoleGlobal;
struct Player { int noun_gender=0,is_noun_plural=0; char noun_faction[16]="Faction"; } Players[8];
struct PlayerData { uint32_t tech_ranking=0; } PlayersData[8];
struct RulesTechnology { int preq_tech_1=-2,preq_tech_2=-2; } Technology[89];
struct GameDataStateFields { uint8_t game_tech_achieved_[89]={}; };
struct GameDataStateFixture { GameDataStateFields fields_; } GameDataStateStorage;
uint8_t (&GameTechAchieved)[89]=GameDataStateStorage.fields_.game_tech_achieved_;
char StringTemp[1024]; char PopupStartCaption[256];
int PopupModalState=0, ScenarioTechFaction=-1,ScenarioTechSelection=-1;
int GenderDefault=0,PluralityDefault=0; char StrEditorWhoListTitle[32];
const int MaxTechnologyNum=89,MaxPlayerNum=8,TechNone=-1;
char* label_get(int id){assert(id==270||id==1008||id==246||id==248);return const_cast<char*>("label");}
void auto_undo(){events.push_back("undo");}
void parse_says(int slot,char*,int,int){assert(slot==0);events.push_back("parse");}
void say_tech(char*out,int,int){std::strcpy(out,"TECH");events.push_back("say-tech");}
bool has_tech(int tech,int faction){return (GameTechAchieved[tech]&(1u<<faction))!=0;}
int tech_avail(int,int){return 0;}
void bitmask(int,int*,int*){events.push_back("bitmask");}
void tech_achieved(int,int,int,int){events.push_back("achieved");}
void see_map_check(){events.push_back("see-map");}
'''+body+r'''
int main(){
  ConsoleGlobal.faction_id_=1; Technology[0].preq_tech_1=-1; Technology[0].preq_tech_2=-1;
  GameTechAchieved[0]=2; results={0,-1}; result_index=0; events.clear();
  ConsoleGlobal.editor_tech();
  const std::vector<std::string> expected={"construct","undo","parse","start",
    "cancel-text","ok-text","say-tech","item:0","sort","exec","bitmask",
    "redraw","parse","start","cancel-text","ok-text","say-tech","item:0",
    "sort","select","exec","see-map","destroy"};
  assert(events==expected); assert(GameTechAchieved[0]==0);
  assert(ScenarioTechFaction==1 && ScenarioTechSelection==0);
  assert(PlayersData[1].tech_ranking==0 && PopupModalState==0);
}
'''
    unit = tmp_path / "scenario_tech.cpp"
    unit.write_text(harness)
    executable = tmp_path / "scenario_tech"
    subprocess.run([compiler, "-std=c++17", "-Wall", "-Wextra", "-Werror",
                    str(unit), "-o", str(executable)], check=True,
                   capture_output=True, text=True)
    subprocess.run([str(executable)], check=True, capture_output=True, text=True)


def test_editor_natural_uses_selection_for_feature_and_switch_for_radius(tmp_path):
    compiler = shutil.which("g++")
    if not compiler:
        pytest.skip("host C++ compiler unavailable")
    source = (ROOT / "src/console.cpp").read_text()
    body = _body(source, "void Console::editor_natural(int place)")
    harness = r'''
#include <cassert>
#include <cstdint>
#include <cstring>
#include <string>
#include <vector>
std::vector<std::string> events;
template<class T,class S> T fake_cast(S value) {
  return static_cast<T>(reinterpret_cast<uintptr_t>(value));
}
#define reinterpret_cast fake_cast
struct Dialogs { void item(char*,int) { events.push_back("item"); } };
struct Popup {
  Dialogs list; Popup(){events.push_back("construct");} ~Popup(){events.push_back("destroy");}
  int start(char*,const char*,int,char*,int){events.push_back("start");return 0;}
  Dialogs& dialogs(){return list;}
  int exec(int,void*){events.push_back("exec");return selected_result;}
  static int selected_result;
};
int Popup::selected_result=0;
struct Console { int cursor_history_index_=0,cursor_history_x_[32]={},cursor_history_y_[32]={};
  void editor_natural(int); } ConsoleGlobal;
struct NaturalRec { char* name; } Natural[16];
struct Map { uint32_t bit2=0; uint8_t climate=0,region=0; } MapTiles[4096];
struct Landmark { char* name; int x=0,y=0; } MapLandmark[4];
struct Table { char* get(int){return const_cast<char*>("name");} } StringTable;
char PopupStartCaption[256];
const int MaxNaturalNum=16,LM_FRESH=7,BIT2_FRESH=0x10000000;
const int MapLatitudeBounds=64,MapLongitudeBounds=64;
uint32_t MapLongitude=64;
int RadiusRange[9]={1,9,25,49,81,121,169,225,289};
int RadiusOffsetX[289]={},RadiusOffsetY[289]={};
int xrange_calls=0;
int xrange(int x){++xrange_calls;return x;}
void auto_undo(){events.push_back("undo");}
void bit2_set(int x,int y,uint32_t bit,int set){
  assert(x==0&&y==0); events.push_back("feature:"+std::to_string(bit));
  if(set) MapTiles[0].bit2|=bit; else MapTiles[0].bit2&=~bit;
}
void code_set(int x,int y,int code){assert(x==0&&y==0&&code==0);events.push_back("code");}
void bit_set(int x,int y,int bit,int set){assert(x==0&&y==0&&set==0);events.push_back("extra:"+std::to_string(bit));}
int find_landmark(int,int,int){events.push_back("landmark");return -1;}
void kill_landmark(int,int){assert(false);}
void world_climate(){events.push_back("climate");}
void draw_map(int mode){assert(mode==1);events.push_back("draw-map");}
void world_crater(int,int){} void world_volcano(int,int,int){} void world_monsoon(int,int){}
void world_diamond(int,int){} void world_sargasso(int,int){} void world_ruin(int,int){}
void world_dune(int,int){} void world_fresh(int,int){} void world_mesa(int,int){}
void world_canyon(int,int){} void world_geothermal(int,int){} void world_ridge(int,int){}
void world_borehole(int,int){} void world_temple(int,int){} void world_unity(int,int){}
void world_fossil(int,int){}
'''+body+r'''
int main(){
  ConsoleGlobal.cursor_history_index_=2;
  for(int selected : {0,4,12}) {
    Popup::selected_result=selected; MapTiles[0].bit2=(1u<<selected);
    events.clear(); xrange_calls=0; ConsoleGlobal.editor_natural(0);
    const int radius=selected==0?3:selected==4?4:1;
    assert(xrange_calls==1+RadiusRange[radius]);
    assert((MapTiles[0].bit2&(1u<<selected))==0);
    assert(events.size()>=7 && events[0]=="construct" && events[1]=="undo");
    assert(events[events.size()-4]=="landmark");
    assert(events[events.size()-3]=="climate" && events[events.size()-2]=="draw-map");
    assert(events.back()=="destroy");
    int feature_clears=0, extras=0;
    for(const std::string& event:events){
      if(event=="feature:"+std::to_string(1u<<selected)) ++feature_clears;
      if(event.rfind("extra:",0)==0) ++extras;
    }
    assert(feature_clears==1);
    assert(extras==(selected==4||selected==12?1:0));
    if(selected==4) assert(events[events.size()-5]=="extra:32");
    if(selected==12) assert(events[events.size()-5]=="extra:17825792");
  }
}
'''
    unit = tmp_path / "scenario_natural.cpp"
    unit.write_text(harness)
    executable = tmp_path / "scenario_natural"
    subprocess.run([compiler, "-std=c++17", "-Wall", "-Wextra", "-Werror",
                    str(unit), "-o", str(executable)], check=True,
                   capture_output=True, text=True)
    subprocess.run([str(executable)], check=True, capture_output=True, text=True)
