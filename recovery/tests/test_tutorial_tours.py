"""Actual-body tutorial controls; service doubles do not prove game acceptance."""

from pathlib import Path
import shutil
import subprocess

import pytest
from current_game_storage_fixture import current_game_storage_fixture


ROOT = Path(__file__).resolve().parents[2]


def body(signature):
    source = (ROOT / "src/tutwin.cpp").read_text()
    start = source.index(signature)
    return source[start:source.index("\n}", start) + 2]


def execute(tmp_path, harness, candidate, label, compiler=None):
    compiler = compiler or shutil.which("g++")
    if not compiler:
        pytest.skip("host C++ compiler unavailable")
    unit = tmp_path / (label + ".cpp")
    unit.write_text(harness.replace("BODY", candidate))
    executable = tmp_path / label
    subprocess.run([compiler, "-std=c++17", str(unit), "-o", str(executable)],
                   check=True, capture_output=True, text=True, timeout=30)
    return subprocess.run([str(executable)], capture_output=True, text=True,
                          timeout=10).returncode


HOST_COMPILERS = tuple(path for name in ("g++", "clang++")
                       if (path := shutil.which(name)) is not None)


@pytest.mark.parametrize("compiler", HOST_COMPILERS)
def test_drone_tour_passes_complete_base_name(tmp_path, compiler):
    harness = r'''
#include <cassert>
#include <cstring>
struct RECT {int left,top,right,bottom;};
struct GraphicWin { int identity; };
struct Prefix { int value; };
struct MapWin : Prefix, public virtual GraphicWin {};
MapWin primary_map;
struct Base {char prefix[0x13];char name_string[25];int faction_id_current;};
Base base; Base* BaseCurrent=&base; Base Bases[2]; int BaseCurrentCount=2;
struct Console {int faction_id_;} ConsoleGlobal;
struct Button {bool is_visible(){return false;}};
struct Window {
 bool visible=true;
 RECT population_rect_,garrison_rect_,rect12_,flash_rect_a_,rect13_,rect11_,expenses_help_rects_[6];
 Button flatButtons_[18];
 bool is_visible(){return visible;} void zoom(int,int){visible=true;}
 void focus(){} void client_to_screen(int*,int*){}
 void draw_energy_alloc(int){} void draw_expenses(int){} void draw_farm(int){}
} TutWinBaseWindow, MainInterfaceGlobal;
MapWin* MapWinTable[1];int parsed,calls;
void parse_says(int slot,char* name,int a,int b){
 assert(slot==0&&a==-1&&b==-1);assert(name==base.name_string);
 assert(!std::strcmp(name,"Complete base name"));++parsed;
}
struct TutWin {
 void tour6();
 int tut_win(GraphicWin* owner,const char*,int,int,int,int,int,int){
  assert(owner==static_cast<GraphicWin*>(MapWinTable[0]));++calls;return 0;}
};
bool tut_callout(TutWin&,Window&,RECT&,const char*){return false;}
BODY
int main(){GameDataStateStorage.fields_.base_current_count_=2;TutWin w;
 for(int nonnull=0;nonnull<2;++nonnull){
  std::strcpy(base.name_string,"Complete base name");
  MapWinTable[0]=nonnull?&primary_map:0;parsed=0;calls=0;
  w.tour6();assert(parsed==1&&calls==1);
 }}
'''
    candidate = body("void TutWin::tour6() {")
    harness = current_game_storage_fixture() + '\n' + harness.replace(
        'int BaseCurrentCount=2;', '').replace(
        'int main(){',
        'int main(){GameDataStateStorage.fields_.base_current_count_=2;', 1)
    assert execute(tmp_path, harness, candidate, "name", compiler) == 0
    damaged = candidate.replace("BaseCurrent->name_string,",
                                "BaseCurrent->name_string + 0x13,")
    assert damaged != candidate
    assert execute(tmp_path, harness, damaged, "name_damage", compiler) != 0
    assert "BaseCurrent->name_string +" not in body("void TutWin::tour1() {")


@pytest.mark.parametrize("compiler", HOST_COMPILERS)
def test_design_tour_routes_and_every_cancellation(tmp_path, compiler):
    # Expected rectangle offsets and labels come from original 4BAF20..4BB373,
    # not the candidate's declarations. This controls dispatch, not ABI layout.
    harness = r'''
#include <cassert>
#include <cstring>
struct RECT {int left,top,right,bottom;};
struct GraphicWin{int identity;};struct Prefix{int value;};
struct MapWin:Prefix,public virtual GraphicWin{};
MapWin PrimaryMap,ReplacementMap;MapWin*MapWinTable[1]={&PrimaryMap};
struct Console{int faction_id_;}ConsoleGlobal;
struct DesignWin {
 enum ComponentPanelIndex { component_panel_chassis, component_panel_weapon,
  component_panel_armor, component_panel_reactor, component_panel_special_one,
  component_panel_special_two };
 enum LowerPanelActionIndex { lower_panel_done, lower_panel_rename,
  lower_panel_obsolete, lower_panel_upgrade, lower_panel_cancel };
 bool visible;int opens,closes;
 RECT component_panel_rects_[6],field_B2C_rect_{0xB2C},
 field_B0C_rect_{0xB0C},field_CBC_rect_{0xCBC},caption_rect_{0xA6C},
 lower_panel_rows_[5],field_B5C_rect_{0xB5C},field_B4C_rect_{0xB4C};
 int field_B3C_=0xB3C,field_B40_=0,field_B44_=0,field_B48_=0;
 DesignWin() : visible(true),opens(0),closes(0) {
  component_panel_rects_[0].left=0xAAC; component_panel_rects_[1].left=0xABC;
  component_panel_rects_[2].left=0xACC; component_panel_rects_[3].left=0xADC;
  component_panel_rects_[4].left=0xAEC; component_panel_rects_[5].left=0xAFC;
  for(int i=0;i<5;++i) lower_panel_rows_[i].left=0xB8C+i*0x10;
 }
 int is_visible(){return visible;}
 void exec(int,int n){assert(n==-2);++opens;}
 void shut_that_badboy_down(){++closes;}
}TutWinDesWindow;
using Window = DesignWin;
const char* labels[]={"DESTOURCHAS","DESTOURWEAP","DESTOURARM","DESTOURREACTOR",
 "DESTOURABILITY","DESTOURCOST","DESTOURSTATS","DESTOURBAR","DESTOURARROWS",
 "DESTOURPRESETS","DESTOURDONE","DESTOURAPPLY","DESTOURRENAME","DESTOUROBS",
 "DESTOURUPGRADE","DESTOURRETIRE","DESTOURCANCEL"};
int rects[]={0xAAC,0xABC,0xACC,0xADC,0xAEC,0xB2C,0xB3C,0xB0C,0xCBC,0xA6C,
 0xB8C,0xB5C,0xB9C,0xBAC,0xBBC,0xB4C,0xBCC};
int count,stop,kind;
int observe(int id,int transform,const char* label){
 assert(count<17);assert(id==rects[count]);assert(!std::strcmp(label,labels[count]));
 int wanted=count>=12?3:count>=7&&count<=9?2:1;
 assert(transform==wanted);return count++!=stop;
}
struct TutWin {
 void tour7();
 void des_rect(RECT*r,int*x,int*y){*x=r->left;*y=0;kind=1;}
 void iface_rect(RECT*r,int*x,int*y){*x=r->left;*y=0;kind=2;}
 int tut_win(GraphicWin*owner,const char*s,int x,int,int,int flag,int,int){
  assert(owner==static_cast<GraphicWin*>(MapWinTable[0]));
  assert(flag==2);int result=observe(x,kind,s);
  MapWinTable[0]=MapWinTable[0]==&PrimaryMap?&ReplacementMap:&PrimaryMap;
  return result;}
 int do_des(RECT*r,const char*s,int flag){assert(flag==2);return observe(r->left,3,s);}
};
BODY
int main(){for(int visible=0;visible<2;++visible)for(stop=-1;stop<17;++stop){
 MapWinTable[0]=&PrimaryMap;
 count=0;TutWinDesWindow.visible=visible;
 TutWinDesWindow.opens=TutWinDesWindow.closes=0;TutWin w;w.tour7();
 assert(count==(stop<0?17:stop+1));assert(TutWinDesWindow.opens==!visible);
 assert(TutWinDesWindow.closes==!visible);}
 stop=-1;count=0;TutWinDesWindow.visible=1;MapWinTable[0]=0;
 TutWin w;w.tour7();assert(count==17);}
'''
    candidate = body("void TutWin::tour7() {")
    assert execute(tmp_path, harness, candidate, "routes", compiler) == 0
    damaged = candidate.replace('"DESTOURRENAME"', '"DESTOURRETIRE"')
    assert damaged != candidate
    assert execute(tmp_path, harness, damaged, "routes_damage", compiler) != 0


@pytest.mark.parametrize("compiler", HOST_COMPILERS)
def test_social_tour5_owner_routes_lifetime_and_conversion_damage(tmp_path, compiler):
    harness = r'''
#include <cassert>
#include <cstdio>
#include <cstring>
#include <vector>
struct RECT {int left,top,right,bottom;};
struct GraphicWin {int identity;};
struct Prefix {int value;};
struct MapWin : Prefix, public virtual GraphicWin {};
MapWin FirstMap, SecondMap;
MapWin *MapWinTable[1];
std::vector<int> events;
int stop=-1,count=0,ok_calls=0;
char StringTemp[64];
const char *labels[]={"SOCTOURMAIN","SOCTOURFACTION","SOCTOUROVERVIEW",
 "SOCTOUREFFECTS","SOCTOUR0","SOCTOUR1","SOCTOUR2","SOCTOUR3",
 "SOCTOUR4","SOCTOUR5","SOCTOUR6","SOCTOUR7","SOCTOUR8","SOCTOUR9",
 "SOCTOURECON","SOCTOURPSYCH","SOCTOURLABS","SOCTOURINFO",
 "SOCTOURCANCEL","SOCTOURRESET"};
int rect_ids[]={10,20,30,40,100,101,102,103,104,105,106,107,108,109,
 200,210,220,230,240,250};
RECT rect_for(int id){RECT r={id,id*2,id+1,id*2+1};return r;}
struct SocWin {
 bool visible;
 int init_calls,show_calls,hide_calls,release_calls,close_calls;
 RECT categoryHeaders_[2],tooltipFactionPower_,tooltipEffects_,tutRect1_;
 RECT tutRects2_[10],tooltipEconomy_,tooltipPsych_,tooltipLabs_,tutRect14_;
 RECT tutRect11_,tutRect12_,tutRect13_;
 SocWin() : visible(true),init_calls(0),show_calls(0),hide_calls(0),
  release_calls(0),close_calls(0) {
  categoryHeaders_[1]=rect_for(10);tooltipFactionPower_=rect_for(20);
  tooltipEffects_=rect_for(30);tutRect1_=rect_for(40);
  for(int i=0;i<10;++i)tutRects2_[i]=rect_for(100+i);
  tooltipEconomy_=rect_for(200);tooltipPsych_=rect_for(210);
  tooltipLabs_=rect_for(220);tutRect14_=rect_for(230);
  tutRect11_=rect_for(240);tutRect12_=rect_for(250);
  tutRect13_=rect_for(260);
 }
 int is_visible(){return visible;}
 void init(int faction){assert(faction==-5);++init_calls;events.push_back(1);}
 void show(int mode){assert(mode==0);++show_calls;events.push_back(2);}
 void hide(){++hide_calls;events.push_back(3);}
 void release_iface_mode(){++release_calls;events.push_back(4);}
 void close(){++close_calls;events.push_back(5);}
} TutWinSocWindow;
struct Console {int faction_id_;} ConsoleGlobal={5};
void soc_rect(RECT*r,int*x,int*y){*x=r->left+5;*y=r->top+7;}
void iface_rect(RECT*r,int*x,int*y){*x=r->left+50;*y=r->top+70;}
void say_num(int value){char digits[16];std::sprintf(digits,"%d",value);std::strcat(StringTemp,digits);}
struct TutWin {
 void tour5();
 int tut_win(GraphicWin*owner,const char*text,int x,int y,void*sprite,
             int flag,int tail_x,int tail_y){
  assert(count<20);int index=count;
  assert(owner==static_cast<GraphicWin*>(MapWinTable[0]));
  assert(!std::strcmp(text,labels[index]));assert(flag==2&&sprite==0);
  assert(tail_x==-1&&tail_y==-1);
  assert(x==rect_ids[index]+(index==2||index>=14&&index<=17?50:5));
  assert(y==rect_ids[index]*2+(index==2||index>=14&&index<=17?70:7));
  events.push_back(10+index);++count;
  MapWinTable[0]=MapWinTable[0]==&FirstMap?&SecondMap:&FirstMap;
  return index==stop?0:1;
 }
 int do_soc(RECT*r,const char*text,int flag){
  assert(r->left==260&&r->top==520);assert(!std::strcmp(text,"SOCTOUROK"));
  assert(flag==2);++ok_calls;events.push_back(90);return 0;
 }
};
BODY
static void reset_case(bool visible,int first_null){
 TutWinSocWindow.visible=visible;TutWinSocWindow.init_calls=0;
 TutWinSocWindow.show_calls=0;TutWinSocWindow.hide_calls=0;
 TutWinSocWindow.release_calls=0;TutWinSocWindow.close_calls=0;
 events.clear();count=0;ok_calls=0;MapWinTable[0]=first_null?0:&FirstMap;
}
static void check_case(bool visible,int first_null){
 std::vector<int> expected;
 if(!visible){expected.push_back(1);expected.push_back(2);}
 int last=stop<0?19:stop;
 for(int i=0;i<=last;++i)expected.push_back(10+i);
 if(stop<0)expected.push_back(90);
 if(!visible){expected.push_back(3);expected.push_back(4);expected.push_back(5);}
 assert(events==expected);
 assert(count==(stop<0?20:stop+1));assert(ok_calls==(stop<0?1:0));
 assert(TutWinSocWindow.init_calls==!visible);
 assert(TutWinSocWindow.show_calls==!visible);
 assert(TutWinSocWindow.hide_calls==!visible);
 assert(TutWinSocWindow.release_calls==!visible);
 assert(TutWinSocWindow.close_calls==!visible);
 (void)first_null;
}
int main(){
 assert(static_cast<void*>(&FirstMap)!=static_cast<void*>(static_cast<GraphicWin*>(&FirstMap)));
 TutWin tutorial;
 for(int visible=0;visible<2;++visible)for(int first_null=0;first_null<2;++first_null)
  for(stop=-1;stop<20;++stop){
   reset_case(visible,first_null);tutorial.tour5();check_case(visible,first_null);
  }
 return 0;
}
'''
    candidate = body("void TutWin::tour5() {")
    assert execute(tmp_path, harness, candidate, "tour5", compiler) == 0

    direct = "tut_win(MapWinTable[0],"
    old = "tut_win(MapWinTable[0] ? static_cast<GraphicWin *>(MapWinTable[0]) : 0,"
    if direct in candidate:
        damaged = candidate.replace(
            direct, "tut_win(reinterpret_cast<GraphicWin *>(MapWinTable[0]),", 1
        )
    else:
        assert old in candidate
        damaged = candidate.replace(
            old, "tut_win(reinterpret_cast<GraphicWin *>(MapWinTable[0]),", 1
        )
    assert damaged != candidate
    assert execute(tmp_path, harness, damaged, "tour5_conversion_damage", compiler) != 0
