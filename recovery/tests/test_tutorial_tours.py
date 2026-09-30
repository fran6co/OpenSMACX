"""Actual-body tutorial controls; service doubles do not prove game acceptance."""

from pathlib import Path
import shutil
import subprocess

import pytest


ROOT = Path(__file__).resolve().parents[2]


def body(signature):
    source = (ROOT / "src/tutwin.cpp").read_text()
    start = source.index(signature)
    return source[start:source.index("\n}", start) + 2]


def execute(tmp_path, harness, candidate, label):
    compiler = shutil.which("g++")
    if not compiler:
        pytest.skip("host C++ compiler unavailable")
    unit = tmp_path / (label + ".cpp")
    unit.write_text(harness.replace("BODY", candidate))
    executable = tmp_path / label
    subprocess.run([compiler, "-std=c++17", str(unit), "-o", str(executable)],
                   check=True, capture_output=True, text=True)
    return subprocess.run([str(executable)], capture_output=True, text=True).returncode


def test_drone_tour_passes_complete_base_name(tmp_path):
    harness = r'''
#include <cassert>
#include <cstring>
struct RECT {int left,top,right,bottom;};
struct GraphicWin {};
struct MapWin: GraphicWin {};
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
 int tut_win(GraphicWin*,const char*,int,int,int,int,int,int){++calls;return 0;}
};
bool tut_callout(TutWin&,Window&,RECT&,const char*){return false;}
BODY
int main(){std::strcpy(base.name_string,"Complete base name");TutWin w;
 w.tour6();assert(parsed==1&&calls==1);}
'''
    candidate = body("void TutWin::tour6() {")
    assert execute(tmp_path, harness, candidate, "name") == 0
    damaged = candidate.replace("BaseCurrent->name_string,",
                                "BaseCurrent->name_string + 0x13,")
    assert damaged != candidate
    assert execute(tmp_path, harness, damaged, "name_damage") != 0
    assert "BaseCurrent->name_string +" not in body("void TutWin::tour1() {")


def test_design_tour_routes_and_every_cancellation(tmp_path):
    # Expected rectangle offsets and labels come from original 4BAF20..4BB373,
    # not the candidate's declarations. This controls dispatch, not ABI layout.
    harness = r'''
#include <cassert>
#include <cstring>
struct RECT {int left,top,right,bottom;};
struct GraphicWin{};struct MapWin:GraphicWin{};MapWin*MapWinTable[1];
struct Console{int faction_id_;}ConsoleGlobal;
struct Window {
 bool visible;int opens,closes;
 RECT chassis_panel_rect_{0xAAC},weapon_panel_rect_{0xABC},armor_panel_rect_{0xACC},
 reactor_panel_rect_{0xADC},special_one_panel_rect_{0xAEC},field_B2C_rect_{0xB2C},
 field_B0C_rect_{0xB0C},field_CBC_rect_{0xCBC},caption_rect_{0xA6C},
 lower_panel_rect_{0xB8C},field_B5C_rect_{0xB5C},lower_panel_extra_rect_{0xB9C},
 field_BAC_rect_{0xBAC},field_BBC_rect_{0xBBC},field_B4C_rect_{0xB4C},
 lower_panel_end_rect_{0xBCC};
 int field_B3C_=0xB3C,field_B40_=0,field_B44_=0,field_B48_=0;
 int is_visible(){return visible;}
 void exec(int,int n){assert(n==-2);++opens;}
 void shut_that_badboy_down(){++closes;}
}TutWinDesWindow;
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
 int tut_win(GraphicWin*,const char*s,int x,int,int,int flag,int,int){
  assert(flag==2);return observe(x,kind,s);}
 int do_des(RECT*r,const char*s,int flag){assert(flag==2);return observe(r->left,3,s);}
};
BODY
int main(){for(int visible=0;visible<2;++visible)for(stop=-1;stop<17;++stop){
 count=0;TutWinDesWindow.visible=visible;
 TutWinDesWindow.opens=TutWinDesWindow.closes=0;TutWin w;w.tour7();
 assert(count==(stop<0?17:stop+1));assert(TutWinDesWindow.opens==!visible);
 assert(TutWinDesWindow.closes==!visible);}}
'''
    candidate = body("void TutWin::tour7() {")
    assert execute(tmp_path, harness, candidate, "routes") == 0
    damaged = candidate.replace('"DESTOURRENAME"', '"DESTOURRETIRE"')
    assert damaged != candidate
    assert execute(tmp_path, harness, damaged, "routes_damage") != 0
