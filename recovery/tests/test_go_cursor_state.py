"""Actual go_reset caller semantics under controlled virtual callbacks.

Complete source body and real relevant MapWin field declarations are extracted.
The fixture uses C++ virtual inheritance and recording overrides, not raw
vtable offsets. Timer, cover, title and cursor operations are explicit doubles;
arbitrary replacement callbacks prove caller load timing, not the real MapWin
override closure, native ABI, recursive input behavior or working go-mode UI.
The original requires a nonnull primary/selected window and valid selected slot.
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
    header=(ROOT/"src/mapwin.h").read_text();fields=[]
    for name in ("cursor_sprite_","cursor_resource_id_","field_1DD74_","field_1DD80_","field_1DD84_","field_1DD88_","field_1DD8C_","field_1DE0C_"):
        declaration,=re.findall(r"^  (?:Sprite \*|uint32_t )"+name+r";",header,re.M);fields.append(declaration)
    bound,=re.findall(r"^static const size_t MapWinTableSlots = \d+;",header,re.M)
    return "\n".join(fields),bound,braced((ROOT/"src/time.cpp").read_text(),"void __cdecl go_reset()")


HARNESS=r'''
#include <cstdint>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <vector>
#define __cdecl
void check(bool ok,int line){if(!ok){std::fprintf(stderr,"assertion line %d\n",line);std::exit(94);}}
#define require(x) check((x),__LINE__)
struct Sprite{};Sprite g_CURSOR_SPRITES[12];
struct Win {virtual ~Win(){} virtual void hide();virtual void on_lose_mouse_capture(){}virtual void on_update(){}int set_cursor(int);};
struct GraphicWin:virtual Win {int padding[5];};
struct MapWin:virtual GraphicWin {
    ACTUAL_FIELDS
    int id;void on_lose_mouse_capture() override;void on_update() override;
};
ACTUAL_BOUND
MapWin windows[10];MapWin *MapWinTable[MapWinTableSlots];int MapWinSelectedSlot;
Win *WinPointerOwner3,*WinPointerOwner4;
struct Event {char kind;int id;};std::vector<Event> seen;
int replace_in04,replace_on_cursor,replace_id,select_on_cursor,owner4_after04,select_on_title;
GraphicWin CoverWindow;
void Win::hide(){require(this==static_cast<Win*>(&CoverWindow));seen.push_back({'H',-1});}
struct Timer{void close(){seen.push_back({'C',-1});}} GoTimer;
struct Status{void set_title(int n){require(n==0);seen.push_back({'T',-1});if(select_on_title>=0){MapWinSelectedSlot=select_on_title;select_on_title=-1;}}} ConsoleStatusWin;
Win *base(int id){return static_cast<GraphicWin*>(&windows[id]);}
int slot_of(MapWin *window){for(size_t i=0;i<MapWinTableSlots;++i)if(MapWinTable[i]==window)return int(i);return -1;}
bool cleared(const MapWin&w){return w.field_1DD88_==1 && w.field_1DD80_==0 && w.field_1DD8C_==0 && w.field_1DD84_==0 && w.field_1DE0C_==0xffffffffu;}
void MapWin::on_lose_mouse_capture(){
    require(WinPointerOwner3==nullptr && cleared(*this));seen.push_back({'A',id});
    if(replace_in04>=0){require(MapWinTable[replace_in04]==this);MapWinTable[replace_in04]=&windows[replace_id];replace_in04=-1;}
    if(owner4_after04>=0)WinPointerOwner4=base(owner4_after04);
}
void MapWin::on_update(){seen.push_back({'B',id});}
int Win::set_cursor(int resource){
    MapWin *window=dynamic_cast<MapWin*>(this);require(window && resource==0x7f00);
    require(window->cursor_sprite_ || window->cursor_resource_id_!=0x7f00);
    seen.push_back({'R',window->id});
    if(replace_on_cursor>=0){require(MapWinTable[replace_on_cursor]==window);MapWinTable[replace_on_cursor]=&windows[replace_id];replace_on_cursor=-1;}
    if(select_on_cursor>=0){MapWinSelectedSlot=select_on_cursor;select_on_cursor=-1;}
    return 19;
}
ACTUAL_BODY
void reset(){
    for(int i=0;i<10;++i){windows[i].id=i;windows[i].cursor_sprite_=nullptr;windows[i].cursor_resource_id_=0x7f00;
        windows[i].field_1DD74_=0;windows[i].field_1DD80_=0;
        windows[i].field_1DD84_=84;windows[i].field_1DD88_=88;windows[i].field_1DD8C_=140;windows[i].field_1DE0C_=12;}
    for(size_t i=0;i<MapWinTableSlots;++i)MapWinTable[i]=nullptr;MapWinTable[0]=&windows[0];
    MapWinSelectedSlot=-1;WinPointerOwner3=WinPointerOwner4=nullptr;seen.clear();
    replace_in04=replace_on_cursor=select_on_cursor=owner4_after04=select_on_title=-1;replace_id=9;
}
void expect(size_t i,char kind,int id){require(i<seen.size() && seen[i].kind==kind && seen[i].id==id);}
void initial(){expect(0,'H',-1);expect(1,'C',-1);}
void main_walk(){
    reset();go_reset();require(seen.size()==2);initial();
    reset();MapWinTable[1]=&windows[1];MapWinTable[2]=&windows[2];MapWinTable[7]=&windows[7];
    windows[0].field_1DD80_=windows[1].field_1DD80_=windows[2].field_1DD80_=windows[7].field_1DD80_=1;
    windows[2].field_1DD74_=windows[7].field_1DD74_=1;go_reset();
    require(seen.size()==8);initial();expect(2,'B',0);expect(3,'T',-1);expect(4,'B',2);expect(5,'T',-1);expect(6,'B',7);expect(7,'T',-1);
    require(cleared(windows[0]) && cleared(windows[2]) && cleared(windows[7]) && windows[1].field_1DD80_==1);
    for(int owner_case=0;owner_case<2;++owner_case){
        reset();MapWinTable[2]=&windows[2];windows[2].field_1DD74_=windows[2].field_1DD80_=1;
        WinPointerOwner3=WinPointerOwner4=base(2);replace_in04=2;owner4_after04=owner_case?9:2;
        go_reset();require(seen.size()==5);initial();expect(2,'A',2);expect(3,'B',9);expect(4,'T',-1);
        require(cleared(windows[2]) && windows[9].field_1DD88_==88 && WinPointerOwner3==nullptr);
        require(WinPointerOwner4==(owner_case?base(9):nullptr));
    }
}
void selected_walk(){
    reset();MapWinSelectedSlot=0;go_reset();require(seen.size()==4 && cleared(windows[0]) && MapWinSelectedSlot==-1);
    expect(2,'B',0);expect(3,'T',-1);
    for(int shape=0;shape<12;++shape){
        reset();MapWinTable[2]=&windows[2];MapWinSelectedSlot=2;windows[2].cursor_sprite_=&g_CURSOR_SPRITES[shape];
        go_reset();initial();require(cleared(windows[2]) && MapWinSelectedSlot==-1);
        if(shape==1 || shape==9){require(seen.size()==5);expect(2,'R',2);expect(3,'B',2);expect(4,'T',-1);require(windows[2].cursor_sprite_==nullptr);}
        else {require(seen.size()==4);expect(2,'B',2);expect(3,'T',-1);require(windows[2].cursor_sprite_==&g_CURSOR_SPRITES[shape]);}
    }
    reset();MapWinTable[2]=&windows[2];MapWinSelectedSlot=2;windows[2].field_1DD74_=windows[2].field_1DD80_=1;
    go_reset();require(seen.size()==4);expect(2,'B',2);expect(3,'T',-1); // selected excluded from main walk.
    for(int owner_case=0;owner_case<2;++owner_case){
        reset();MapWinTable[2]=&windows[2];MapWinSelectedSlot=2;WinPointerOwner3=WinPointerOwner4=base(2);
        replace_in04=2;owner4_after04=owner_case?9:2;go_reset();
        require(seen.size()==5);expect(2,'A',2);expect(3,'B',9);expect(4,'T',-1);
        require(cleared(windows[2]) && windows[9].field_1DD88_==88 && WinPointerOwner4==(owner_case?base(9):nullptr));
    }
    // The selected table SLOT is retained across its cursor callback, even
    // if the callback both replaces the object and changes the global index.
    reset();MapWinTable[2]=&windows[2];MapWinTable[7]=&windows[7];MapWinSelectedSlot=2;
    windows[2].cursor_sprite_=&g_CURSOR_SPRITES[1];replace_on_cursor=2;select_on_cursor=7;
    go_reset();require(seen.size()==5);expect(2,'R',2);expect(3,'B',9);expect(4,'T',-1);
    require(windows[2].cursor_sprite_==nullptr && windows[2].cursor_resource_id_==0x7f00 && windows[2].field_1DD88_==88);
    require(cleared(windows[9]) && windows[7].field_1DD88_==88 && MapWinSelectedSlot==-1);
    reset();MapWinTable[2]=&windows[2];MapWinSelectedSlot=2;select_on_title=7;
    go_reset();require(seen.size()==4 && MapWinSelectedSlot==-1);expect(2,'B',2);expect(3,'T',-1);
}
void primary_cursor(){
    reset();windows[0].cursor_resource_id_=123;go_reset();require(seen.size()==3);expect(2,'R',0);require(windows[0].cursor_resource_id_==0x7f00);
    reset();windows[0].cursor_sprite_=&g_CURSOR_SPRITES[3];replace_on_cursor=0;go_reset();
    require(seen.size()==3 && windows[0].cursor_sprite_==nullptr && MapWinTable[0]==&windows[9]);
    // The selected phase reads the global index after primary set_cursor.
    reset();MapWinTable[2]=&windows[2];MapWinTable[3]=&windows[3];MapWinSelectedSlot=2;
    windows[0].cursor_sprite_=&g_CURSOR_SPRITES[4];select_on_cursor=3;go_reset();
    require(seen.size()==5);expect(2,'R',0);expect(3,'B',3);expect(4,'T',-1);
    require(cleared(windows[3]) && windows[2].field_1DD88_==88 && MapWinSelectedSlot==-1);
}
int main(){main_walk();selected_walk();primary_cursor();}
'''


def run_host(tmp_path,fields,bound,body):
    compiler=shutil.which("clang++") or shutil.which("g++")
    if not compiler:pytest.skip("host C++ compiler required")
    source=tmp_path/"go.cpp";binary=tmp_path/"go"
    source.write_text(HARNESS.replace("ACTUAL_FIELDS",fields).replace("ACTUAL_BOUND",bound).replace("ACTUAL_BODY",body))
    compiled=subprocess.run([compiler,"-std=c++11","-O1","-fsanitize=address,undefined","-fno-sanitize-recover=all",str(source),"-o",str(binary)],capture_output=True,text=True,timeout=30)
    assert compiled.returncode==0,compiled.stderr
    return subprocess.run([str(binary)],capture_output=True,text=True,timeout=10,env=dict(os.environ,ASAN_OPTIONS="detect_leaks=0"))


def test_actual_go_cursor_state(tmp_path):
    result=run_host(tmp_path,*actual_source());assert result.returncode==0,result.stderr


MUTANTS=[
    ("main-live-dispatch","main","(*slot)->on_update();","window->on_update();"),
    ("selected-live-dispatch","selected","(*slot)->on_update();","window->on_update();"),
    ("main-cached-owner4","main","WinPointerOwner4 == base","WinPointerOwner4 == static_cast<GraphicWin *>(*slot)"),
    ("selected-cached-owner4","selected","WinPointerOwner4 == base","WinPointerOwner4 == static_cast<GraphicWin *>(*slot)"),
    ("selected-retained-slot","selected","(*slot)->","MapWinTable[MapWinSelectedSlot]->"),
    ("selected-cached-cursor-owner","selected","window->cursor_sprite_ = 0;","(*slot)->cursor_sprite_ = 0;"),
    ("primary-cached-cursor-owner","main","primary->cursor_sprite_ = 0;","MapWinTable[0]->cursor_sprite_ = 0;"),
    ("second-go-sprite","selected","&g_CURSOR_SPRITES[9]","&g_CURSOR_SPRITES[8]"),
    ("selected-zero-slot","all","if (MapWinSelectedSlot >= 0)","if (MapWinSelectedSlot > 0)"),
    ("primary-active-bypass","main","index == 0 || window->field_1DD74_ != 0","window->field_1DD74_ != 0"),
    ("selected-excluded-main","main","index != MapWinSelectedSlot &&",""),
    ("clear-movement-target","all","field_1DE0C_ = -1;","field_1DE0C_ = 0;"),
    ("last-map-slot","main","index < static_cast<int>(MapWinTableSlots)","index < static_cast<int>(MapWinTableSlots) - 1"),
    ("close-timer","main","GoTimer.close();",""),
    ("final-selected-reset","selected","MapWinSelectedSlot = -1;",""),
]


@pytest.mark.parametrize("name,part,before,after",MUTANTS,ids=[x[0] for x in MUTANTS])
def test_compiling_go_cursor_damage(tmp_path,name,part,before,after):
    fields,bound,body=actual_source()
    split=body.index("    if (MapWinSelectedSlot >= 0)")
    prefix,suffix=body[:split],body[split:]
    selected=body if part=="all" else prefix if part=="main" else suffix
    assert before in selected,(name,"missing mutation anchor")
    damaged=selected.replace(before,after)
    body=damaged if part=="all" else damaged+suffix if part=="main" else prefix+damaged
    result=run_host(tmp_path,fields,bound,body)
    assert result.returncode!=0,(name,"behavior damage escaped controls")
