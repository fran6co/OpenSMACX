"""Recovered parent hooks execute through the actual declared virtual interface."""
from pathlib import Path
import re
import shutil
import subprocess

import pytest

from test_listbox_rendering_actual_source import extract_function

ROOT = Path(__file__).resolve().parents[2]


def declarations(owner):
    text = (ROOT / f"src/{owner.lower()}.h").read_text()
    return "\n".join(re.findall(
        r"int on_dialog_(?:item_)?back_draw\([^;]+\) override;", text))


def control(damage=None):
    win = (ROOT / "src/win.h").read_text()
    defaults = "\n".join(re.findall(
        r"virtual int on_dialog_(?:item_)?back_draw\([^\n]+\}", win))
    assert defaults.count("virtual int") == 2
    for owner, count in (("Popup", 2), ("MainInterface", 2), ("Datalink", 2), ("PickWin", 1)):
        assert declarations(owner).count("override;") == count
    bodies = []
    for owner in ("MainInterface", "Datalink"):
        source = (ROOT / f"src/{owner.lower()}.cpp").read_text()
        for method in ("on_dialog_back_draw", "on_dialog_item_back_draw"):
            match = re.search(r"int " + owner + "::" + method + r"\(", source)
            bodies.append(extract_function(source, match.group(0)))
    body = "\n".join(bodies)
    if damage:
        old, new = damage
        assert old in body
        body = body.replace(old, new, 1)
    return r'''
#include <cassert>
struct RECT { int left, top, right, bottom; };
struct GraphicWin;
struct Win { DEFAULTS };
struct Style { void *mask_; } InterfaceBoxStyles[32];
struct GraphicWin : Win {
    RECT clip_, outer_; int tiles, colour, x,y,sx,sy,w,h; void *mask;
    GraphicWin():clip_{1,2,21,32},outer_{50,60,90,100},tiles(0),colour(-1){}
    RECT clip_rect(){return clip_;} const RECT &outer_rect(){return outer_;}
    void tile(void *m,int xx,int yy,int ssx,int ssy,int ww,int hh){
        ++tiles;mask=m;x=xx;y=yy;sx=ssx;sy=ssy;w=ww;h=hh;
    }
    void set_text_color(int c,int b,int d,int e){assert(b==-1 && d==1 && e==1);colour=c;}
};
struct SubInterface {
    int back_calls=0,item_calls=0; GraphicWin *last=nullptr; RECT *area=nullptr; int id=0,state=0;
    int on_iface_dialog_back_draw(GraphicWin *v){++back_calls;last=v;return 139;}
    int on_iface_dialog_item_back_draw(GraphicWin *v,int i,int s,RECT *a){
        ++item_calls;last=v;id=i;state=s;area=a;return 149;
    }
};
unsigned int WorldClimateSkipTerrainClear;
int ActiveIfaceIdStorage, IfaceIdToSlotStorage[4]; SubInterface *InterfaceRegistry[4];
struct MainInterface : Win { MAIN_DECLARATIONS };
struct Datalink : Win { int field_29DC_=0; DATALINK_DECLARATIONS };
ACTUAL_BODIES
int main(){
    Win base; Win *parent=&base; GraphicWin window; RECT area={3,4,13,24};
    assert(parent->on_dialog_back_draw(&window)==0);
    assert(parent->on_dialog_item_back_draw(&window,17,2,&area)==0);
    MainInterface main; parent=&main; SubInterface iface;
    ActiveIfaceIdStorage=2;IfaceIdToSlotStorage[2]=3;InterfaceRegistry[3]=&iface;
    assert(parent->on_dialog_back_draw(&window)==139 && iface.last==&window);
    assert(parent->on_dialog_item_back_draw(&window,17,2,&area)==149);
    assert(iface.last==&window && iface.id==17 && iface.state==2 && iface.area==&area);
    WorldClimateSkipTerrainClear=1;
    assert(parent->on_dialog_back_draw(&window)==0);
    assert(parent->on_dialog_item_back_draw(&window,17,2,&area)==0);
    assert(iface.back_calls==1 && iface.item_calls==1);
    WorldClimateSkipTerrainClear=0;InterfaceRegistry[3]=nullptr;
    assert(parent->on_dialog_back_draw(&window)==0);
    assert(parent->on_dialog_item_back_draw(&window,17,2,&area)==0);
    InterfaceRegistry[3]=&iface;WorldClimateSkipTerrainClear=2;
    assert(parent->on_dialog_back_draw(&window)==139 && iface.back_calls==2);
    Datalink data;parent=&data;int masks[32];
    for(int i=0;i<32;++i)InterfaceBoxStyles[i].mask_=&masks[i];
    assert(parent->on_dialog_back_draw(&window)==1);
    assert(window.mask==&masks[0] && window.x==50 && window.y==60);
    assert(window.sx==0 && window.sy==0 && window.w==20 && window.h==30);
    for(int mode=0;mode<3;++mode){
        assert(parent->on_dialog_item_back_draw(&window,10500,mode,&area)==1);
        assert(window.mask==&masks[mode==2?28:mode==1?27:0]);
        assert(window.x==53 && window.y==64 && window.sx==3 && window.sy==4);
        assert(window.w==10 && window.h==20 && window.colour==-1);
    }
    data.field_29DC_=12;
    assert(parent->on_dialog_item_back_draw(&window,10500,2,&area)==1);
    assert(window.colour==155 && window.tiles==5);
}
'''.replace("DEFAULTS", defaults).replace("MAIN_DECLARATIONS", declarations("MainInterface")).replace("DATALINK_DECLARATIONS", declarations("Datalink")).replace("ACTUAL_BODIES", body)


@pytest.mark.parametrize("compiler", ("g++", "clang++"))
@pytest.mark.parametrize("damage", (None,
    ("WorldClimateSkipTerrainClear != 1u", "WorldClimateSkipTerrainClear == 1u"),
    ("on_iface_dialog_item_back_draw(a1, a2, a3, a4)", "on_iface_dialog_item_back_draw(a1, a3, a2, a4)"),
    ("outer.top + rect->top", "outer.left + rect->top")))
def test_actual_parent_background_handlers(tmp_path, compiler, damage):
    executable = shutil.which(compiler)
    if executable is None:
        pytest.skip(compiler + " unavailable")
    source = tmp_path / "background.cpp"
    source.write_text(control(damage))
    binary = tmp_path / "background"
    subprocess.run([executable, "-std=c++11", str(source), "-o", str(binary)], check=True, capture_output=True)
    result = subprocess.run([str(binary)], capture_output=True)
    assert (result.returncode == 0) == (damage is None)
