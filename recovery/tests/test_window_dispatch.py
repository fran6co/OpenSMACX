"""Recovered capture/update dispatch and bounded real capture reentrancy.

The relevant inheritance clauses, virtual declarations, default handlers,
MapWin fields and complete recovered bodies are extracted from product source.
Unrelated class members are omitted: this is a declaration projection, not an
original object-layout/vtable-index proof. Native table and compiler-generated
adjustor checks are separate. The actual MapWin capture override calls actual
go_reset, including one nested reset; graphics, timer, OS and sound boundaries
remain explicit doubles, not proof of complete game or unbounded reentrancy.
"""

from pathlib import Path
import os
import re
import shutil
import subprocess

import pytest


ROOT=Path(__file__).resolve().parents[2]


def braced(source,signature):
    start=source.index(signature);opening=source.index("{",start);depth=0
    for end in range(opening,len(source)):
        depth+=(source[end]=="{")-(source[end]=="}")
        if not depth:return source[start:end+1]
    raise AssertionError("unterminated actual definition")


def actual_source():
    headers={name:(ROOT/"src"/(name+".h")).read_text() for name in ("win","graphicwin","mapwin","planwin","console")}
    parts={}
    for name,cls in (("win","Win"),("graphicwin","GraphicWin"),("mapwin","MapWin"),("planwin","PlanWin"),("console","Console")):
        declaration,=re.findall(r"^class "+cls+r"\s*:[^{]+\{",headers[name],re.M)
        parts[cls.upper()+"_BASES"]=declaration
    parts["WIN_DEFAULTS"]="\n".join(braced(headers["win"],"virtual void "+method+"()")
                                           for method in ("on_lose_mouse_capture","on_update"))
    parts["GRAPHIC_DECL"],=re.findall(r"  virtual void on_update\(\);",headers["graphicwin"])
    parts["MAP_DECL"],=re.findall(r"  virtual void on_lose_mouse_capture\(\);",headers["mapwin"])
    parts["WIN_UPDATE_DECL"],=re.findall(r"  void __cdecl update\(\);",headers["win"])
    fields=[]
    for name in ("cursor_sprite_","cursor_resource_id_","field_1DD74_","field_1DD80_","field_1DD84_","field_1DD88_","field_1DD8C_","field_1DE0C_"):
        declaration,=re.findall(r"^  (?:Sprite \*|uint32_t )"+name+r";",headers["mapwin"],re.M);fields.append(declaration)
    parts["MAP_FIELDS"]="\n".join(fields)
    parts["SLOT_BOUND"],=re.findall(r"^static const size_t MapWinTableSlots = \d+;",headers["mapwin"],re.M)
    globals=[]
    for pattern in (r"^extern Win \*WinZOrderArray\[\d+\];",r"^extern int WinZOrderCount;"):
        declaration,=re.findall(pattern,headers["win"],re.M);globals.append(declaration.replace("extern ",""))
    parts["ZORDER_GLOBALS"]="\n".join(globals)
    for key,unit,signature in (("CAPTURE_BODY","mapwin","void MapWin::on_lose_mouse_capture()"),
                                ("SURFACE_BODY","graphicwin","void GraphicWin::on_update()"),
                                ("RESET_BODY","time","void __cdecl go_reset()"),
                                ("ZORDER_BODY","win","void __cdecl Win::update()")):
        parts[key]=braced((ROOT/"src"/(unit+".cpp")).read_text(),signature)
    return parts


HARNESS=r'''
#include <cstdint>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <vector>
#define __cdecl
void check(bool ok,int line){if(!ok){std::fprintf(stderr,"assertion line %d\n",line);std::exit(95);}}
#define require(x) check((x),__LINE__)
struct RECT {int left,top,right,bottom;};struct Sprite{};Sprite g_CURSOR_SPRITES[12];
struct AutoSound {virtual ~AutoSound(){}};struct Buffer {virtual ~Buffer(){}};
class GraphicWin;
WIN_BASES
public:
    WIN_DEFAULTS
    WIN_UPDATE_DECL
    virtual void hide();
    virtual void on_nc_paint(RECT*,int);
    virtual GraphicWin *get_graphic_win(){return nullptr;}
    int set_cursor(int);
    int id;
};
GRAPHICWIN_BASES
public:
    GRAPHIC_DECL
    virtual GraphicWin *get_graphic_win(){return this;}
    void update(GraphicWin *);
    int graphic_padding[5];
};
MAPWIN_BASES
public:
    MAP_DECL
    MAP_FIELDS
    int map_padding[7];
};
PLANWIN_BASES public:int plan_padding[11];};
CONSOLE_BASES public:int console_padding[13];};
SLOT_BOUND
MapWin *MapWinTable[MapWinTableSlots];MapWin maps[10];PlanWin plan;Console console;
Win plain;GraphicWin graphic,CoverWindow;
Win *WinPointerOwner3,*WinPointerOwner4;int MapWinSelectedSlot;
ZORDER_GLOBALS
struct Event {char kind;int id,selected;};std::vector<Event> seen;
int hide_count,expect_inner_owner,replace_on_nested_hide,replace_on_title,title_selected;
int replacement_id;
void go_reset();
void do_sound();
void note(char kind,int id){seen.push_back({kind,id,MapWinSelectedSlot});}
Win *map_base(int id){return static_cast<GraphicWin*>(&maps[id]);}
bool cleared(const MapWin&w){return w.field_1DD88_==1 && w.field_1DD80_==0 && w.field_1DD8C_==0 && w.field_1DD84_==0 && w.field_1DE0C_==0xffffffffu;}
void Win::hide(){
    require(this==static_cast<Win*>(&CoverWindow));note('H',-1);++hide_count;
    require(hide_count<=4); // Test fail-fast for damaged recursion, not a product guard.
    if(hide_count==2 && expect_inner_owner>=0){require(WinPointerOwner3==nullptr && cleared(maps[expect_inner_owner]));}
    if(hide_count==2 && replace_on_nested_hide>=0){MapWinTable[replace_on_nested_hide]=&maps[replacement_id];replace_on_nested_hide=-1;}
}
struct Timer{void close(){note('C',-1);}} GoTimer;
struct Status{void set_title(int value){require(value==0);note('T',-1);
    if(replace_on_title>=0){MapWinTable[replace_on_title]=&maps[replacement_id];replace_on_title=-1;}
    if(title_selected>=0){MapWinSelectedSlot=title_selected;title_selected=-1;}
}} ConsoleStatusWin;
int Win::set_cursor(int value){require(value==0x7f00);note('R',id);return 17;}
void GraphicWin::update(GraphicWin *target){require(target==nullptr);note('U',id);}
void Win::on_nc_paint(RECT *area,int flags){require(!area && flags==-1);note('N',id);}
CAPTURE_BODY
SURFACE_BODY
RESET_BODY
ZORDER_BODY
struct Track:GraphicWin {
    Win *after_paint,*after_query;GraphicWin *query_result;
    int query_count,paint_count;
    void on_nc_paint(RECT *area,int flags) override {
        Win::on_nc_paint(area,flags);
        if(after_paint)WinZOrderArray[0]=after_paint;
        if(paint_count!=-999)WinZOrderCount=paint_count;
    }
    GraphicWin *get_graphic_win() override {
        note('Q',id);if(after_query)WinZOrderArray[0]=after_query;
        if(query_count!=-999)WinZOrderCount=query_count;
        return query_result;
    }
} track[4];
int count_on_sound;
void do_sound(){note('S',-1);if(count_on_sound!=-999){WinZOrderCount=count_on_sound;count_on_sound=-999;}}
void reset(){
    seen.clear();hide_count=0;expect_inner_owner=replace_on_nested_hide=replace_on_title=title_selected=-1;replacement_id=9;
    for(int i=0;i<10;++i){maps[i].id=i;maps[i].cursor_sprite_=nullptr;maps[i].cursor_resource_id_=0x7f00;
        maps[i].field_1DD74_=maps[i].field_1DD80_=0;maps[i].field_1DD84_=84;maps[i].field_1DD88_=88;maps[i].field_1DD8C_=140;maps[i].field_1DE0C_=12;}
    for(size_t i=0;i<MapWinTableSlots;++i)MapWinTable[i]=nullptr;MapWinTable[0]=&maps[0];
    MapWinSelectedSlot=-1;WinPointerOwner3=WinPointerOwner4=nullptr;
    plain.id=20;graphic.id=21;plan.id=22;console.id=23;CoverWindow.id=24;
    WinZOrderCount=0;count_on_sound=-999;
    for(auto &entry:WinZOrderArray)entry=nullptr;
    for(int i=0;i<4;++i){track[i].id=30+i;track[i].after_paint=track[i].after_query=nullptr;track[i].query_result=&graphic;
        track[i].query_count=track[i].paint_count=-999;}
}
void expect(size_t i,char kind,int id){require(i<seen.size() && seen[i].kind==kind && seen[i].id==id);}
void inherited_dispatch(){
    reset();Win *p=&plain;p->on_lose_mouse_capture();p->on_update();require(seen.empty());
    Win *g=&graphic;g->on_lose_mouse_capture();require(seen.empty());g->on_update();require(seen.size()==1);expect(0,'U',21);
    Win *receivers[]={map_base(0),static_cast<GraphicWin*>(&plan),static_cast<GraphicWin*>(&console)};
    int ids[]={0,22,23};
    for(int i=0;i<3;++i){reset();receivers[i]->on_update();require(seen.size()==1);expect(0,'U',ids[i]);
        seen.clear();receivers[i]->on_lose_mouse_capture();require(seen.size()==2);expect(0,'H',-1);expect(1,'C',-1);}
}
void main_recursion(){
    reset();MapWinTable[2]=&maps[2];maps[2].field_1DD74_=maps[2].field_1DD80_=1;
    WinPointerOwner3=WinPointerOwner4=map_base(2);expect_inner_owner=2;go_reset();
    require(seen.size()==6 && hide_count==2 && WinPointerOwner3==nullptr && WinPointerOwner4==nullptr && cleared(maps[2]));
    expect(0,'H',-1);expect(1,'C',-1);expect(2,'H',-1);expect(3,'C',-1);expect(4,'U',2);expect(5,'T',-1);
    reset();MapWinTable[2]=&maps[2];maps[2].field_1DD74_=maps[2].field_1DD80_=1;
    WinPointerOwner3=WinPointerOwner4=map_base(2);expect_inner_owner=2;replace_on_nested_hide=2;go_reset();
    require(seen.size()==6 && hide_count==2 && WinPointerOwner4==nullptr);expect(4,'U',9);expect(5,'T',-1);
    require(cleared(maps[2]) && maps[9].field_1DD88_==88);
}
void selected_recursion(){
    for(int replace=0;replace<2;++replace){
        reset();MapWinTable[2]=&maps[2];MapWinSelectedSlot=2;maps[2].cursor_sprite_=&g_CURSOR_SPRITES[1];
        WinPointerOwner3=WinPointerOwner4=map_base(2);expect_inner_owner=2;
        if(replace){replace_on_title=2;title_selected=7;}
        go_reset();require(seen.size()==9 && hide_count==2 && MapWinSelectedSlot==-1);
        require(WinPointerOwner3==nullptr && WinPointerOwner4==nullptr && maps[2].cursor_sprite_==nullptr && cleared(maps[2]));
        expect(0,'H',-1);expect(1,'C',-1);expect(2,'R',2);expect(3,'H',-1);expect(4,'C',-1);
        expect(5,'U',2);expect(6,'T',-1);expect(7,'U',replace?9:2);expect(8,'T',-1);
        require(seen[5].selected==2 && seen[7].selected==-1);
        if(replace)require(maps[9].field_1DD88_==88);
    }
}
void zorder_controls(){
    reset();WinZOrderCount=-1;plain.update();require(seen.empty());
    reset();plain.update();require(seen.empty());
    reset();WinZOrderArray[0]=&track[0];WinZOrderCount=1;track[0].query_result=nullptr;
    plain.update();require(seen.size()==2);expect(0,'N',30);expect(1,'Q',30);
    reset();WinZOrderArray[0]=&track[0];WinZOrderCount=1;track[0].after_paint=&track[1];track[1].after_query=&track[2];
    track[1].query_result=&track[3];plain.update();
    require(seen.size()==4);expect(0,'N',30);expect(1,'Q',31);expect(2,'U',32);expect(3,'S',-1);
    reset();WinZOrderArray[0]=&track[0];WinZOrderArray[1]=&track[1];WinZOrderCount=2;count_on_sound=0;
    plain.update();require(seen.size()==4 && WinZOrderCount==0);
    reset();WinZOrderArray[0]=&track[0];WinZOrderArray[1]=&track[1];WinZOrderCount=2;track[0].query_count=0;
    plain.update();require(seen.size()==4 && WinZOrderCount==0);expect(2,'U',30);expect(3,'S',-1);
    reset();WinZOrderArray[0]=&track[0];WinZOrderArray[1]=&track[1];WinZOrderArray[2]=&track[2];WinZOrderCount=1;count_on_sound=3;
    plain.update();require(seen.size()==12 && WinZOrderCount==3);
    for(int i=0;i<3;++i){expect(4*i,'N',30+i);expect(4*i+1,'Q',30+i);expect(4*i+2,'U',30+i);expect(4*i+3,'S',-1);}
}
int main(){inherited_dispatch();main_recursion();selected_recursion();zorder_controls();}
'''


def run_host(tmp_path,parts):
    compiler=shutil.which("clang++") or shutil.which("g++")
    if not compiler:pytest.skip("host C++ compiler required")
    source=tmp_path/"dispatch.cpp";binary=tmp_path/"dispatch"
    text=re.sub(r"\b("+"|".join(parts)+r")\b",lambda match:parts[match.group()],HARNESS)
    source.write_text(text)
    compiled=subprocess.run([compiler,"-std=c++11","-O1","-fsanitize=address,undefined","-fno-sanitize-recover=all",str(source),"-o",str(binary)],capture_output=True,text=True,timeout=30)
    assert compiled.returncode==0,compiled.stderr
    return subprocess.run([str(binary)],capture_output=True,text=True,timeout=10,env=dict(os.environ,ASAN_OPTIONS="detect_leaks=0"))


def test_actual_window_dispatch(tmp_path):
    result=run_host(tmp_path,actual_source());assert result.returncode==0,result.stderr


# Each mutation changes extracted product code/declarations and must compile.
# A removed `virtual` on an already-overriding method would NOT be a valid
# negative control: C++ still overrides it. Rename declaration and definition
# together to recreate the missing-slot defect while keeping compilation valid.
MUTANTS=[
    ("missing-map-override",[("MAP_DECL","on_lose_mouse_capture","unused_capture"),
                             ("CAPTURE_BODY","on_lose_mouse_capture","unused_capture")]),
    ("missing-graphic-override",[("GRAPHIC_DECL","on_update","unused_update"),
                                 ("SURFACE_BODY","on_update","unused_update")]),
    ("plan-inherits-capture",[("PLANWIN_BASES","public MapWin","public GraphicWin")]),
    ("console-inherits-capture",[("CONSOLE_BASES","public MapWin","public GraphicWin")]),
    ("capture-default-not-virtual",[("WIN_DEFAULTS","virtual void on_lose_mouse_capture","void on_lose_mouse_capture")]),
    ("surface-default-not-virtual",[("WIN_DEFAULTS","virtual void on_update","void on_update")]),
    ("capture-cancels-go",[("CAPTURE_BODY","go_reset();","")]),
    ("invented-reentrancy-guard",[("RESET_BODY","void __cdecl go_reset() {",
                                   "void __cdecl go_reset() { static bool resetting = false; if (resetting) return; resetting = true;"),
                                   ("RESET_BODY","        MapWinSelectedSlot = -1;\n    }\n}",
                                    "        MapWinSelectedSlot = -1;\n    }\n    resetting = false;\n}")]),
    ("surface-delegation-target",[("SURFACE_BODY","update(0);","update(this);")]),
    ("surface-direct-empty-base",[("SURFACE_BODY","update(0);","Win::on_update();")]),
    ("capture-direct-empty-base",[("RESET_BODY","base->on_lose_mouse_capture();","base->Win::on_lose_mouse_capture();")]),
    ("live-update-after-recursion",[("RESET_BODY","(*slot)->on_update();","window->on_update();")]),
    ("cached-owner4-after-recursion",[("RESET_BODY","WinPointerOwner4 == base","WinPointerOwner4 == static_cast<GraphicWin *>(*slot)")]),
    ("clear-movement-before-recursion",[("RESET_BODY","(*slot)->field_1DD80_ = 0;","")]),
    ("clear-owner-before-recursion",[("RESET_BODY","WinPointerOwner3 = nullptr;","")]),
    ("selected-reset-after-title",[("RESET_BODY","ConsoleStatusWin.set_title(0);\n        MapWinSelectedSlot = -1;",
                                    "MapWinSelectedSlot = -1;\n        ConsoleStatusWin.set_title(0);")]),
    ("zorder-live-query-receiver",[("ZORDER_BODY","WinZOrderArray[i]->on_nc_paint(0, -1);",
                                    "Win *cached = WinZOrderArray[i]; cached->on_nc_paint(0, -1);"),
                                    ("ZORDER_BODY","WinZOrderArray[i]->get_graphic_win()","cached->get_graphic_win()")]),
    ("zorder-live-update-receiver",[("ZORDER_BODY","if (WinZOrderArray[i]->get_graphic_win() != 0)",
                                     "Win *cached = WinZOrderArray[i]; if (cached->get_graphic_win() != 0)"),
                                     ("ZORDER_BODY","WinZOrderArray[i]->on_update();","cached->on_update();")]),
    ("getter-is-only-predicate",[("ZORDER_BODY","if (WinZOrderArray[i]->get_graphic_win() != 0)",
                                 "GraphicWin *returned = WinZOrderArray[i]->get_graphic_win(); if (returned != 0)"),
                                 ("ZORDER_BODY","WinZOrderArray[i]->on_update();","returned->on_update();")]),
    ("zorder-count-live",[("ZORDER_BODY","for (int i = 0; i < WinZOrderCount; ++i)",
                          "const int count = WinZOrderCount; for (int i = 0; i < count; ++i)")]),
    ("zorder-count-signed",[("ZORDER_BODY","i < WinZOrderCount","static_cast<uint32_t>(i) < static_cast<uint32_t>(WinZOrderCount)")]),
    ("zorder-null-getter",[("ZORDER_BODY","get_graphic_win() != 0","get_graphic_win() == 0")]),
    ("zorder-sound-pump",[("ZORDER_BODY","do_sound();","")]),
    ("zorder-nc-flags",[("ZORDER_BODY","on_nc_paint(0, -1)","on_nc_paint(0, 0)")]),
]


@pytest.mark.parametrize("name,changes",MUTANTS,ids=[x[0] for x in MUTANTS])
def test_compiling_window_dispatch_damage(tmp_path,name,changes):
    parts=actual_source()
    for key,before,after in changes:
        assert before in parts[key],(name,"missing mutation anchor")
        parts[key]=parts[key].replace(before,after)
    result=run_host(tmp_path,parts)
    assert result.returncode!=0,(name,"behavior damage escaped controls")
