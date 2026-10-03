"""Actual Console cursor bodies with callback replacement observations.

Field declarations and complete bodies are extracted unchanged. The host uses
genuine virtual-base conversions, not fabricated object offsets. Win setters,
MapWin drawing and the message pump are recording doubles: this does not prove
the native vbtable ABI, actual cursor resources, reentrant input or game runtime.
Only valid shape indices and nonnull replacement slots in the original domain
are exercised; no safety guarantee is invented for callback-induced null slots.
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
    cpp=(ROOT/"src/console.cpp").read_text();map_header=(ROOT/"src/mapwin.h").read_text();console=(ROOT/"src/console.h").read_text()
    fields=[]
    for name in ("cursor_sprite_","cursor_resource_id_","field_1DD74_","field_1DD70_"):
        declaration,=re.findall(r"^  (?:Sprite \*|uint32_t )"+name+r";",map_header,re.M);fields.append(declaration)
    console_fields=[]
    for name in ("field_23D10_","field_23D14_"):
        declaration,=re.findall(r"^  uint32_t "+name+r";",console,re.M);console_fields.append(declaration)
    bound,=re.findall(r"^static const size_t MapWinTableSlots = \d+;",map_header,re.M)
    bodies="\n".join(braced(cpp,signature) for signature in
        ("void Console::cursor_on(int cursor_shape)","void Console::cursor_on(int cursor_shape, int veh_id)","void Console::cursor_off()","int Console::selected()"))
    return "\n".join(fields),"\n".join(console_fields),bound,bodies


HARNESS=r'''
#include <cstdint>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <vector>
void check(bool ok,int line){if(!ok){std::fprintf(stderr,"assertion line %d\n",line);std::exit(93);}}
#define require(x) check((x),__LINE__)
struct Sprite {int id;};Sprite g_CURSOR_SPRITES[12];int CursorHotspotX[12],CursorHotspotY[12];
struct MapWin;
struct Win {virtual ~Win(){} int set_cursor(Sprite*,int,int);int set_cursor(int);};
struct GraphicWin:virtual Win {int padding[5];};
struct MapWin:virtual GraphicWin {
    ACTUAL_MAP_FIELDS
    int id;void draw_map(int);
};
struct Console {
    ACTUAL_CONSOLE_FIELDS
    void cursor_on(int);void cursor_on(int,int);void cursor_off();int selected();
} console;
ACTUAL_BOUND
MapWin *MapWinTable[MapWinTableSlots];MapWin windows[10];
struct Event{char kind;int id,a,b;Sprite *sprite;};std::vector<Event> seen;
int replace_slot,replace_id,activate_on_pump,hotspot_shape,change_mode_on_pump;
uint32_t expected_mode,expected_vehicle;
void callback(MapWin *window){
    require(console.field_23D10_==expected_mode && console.field_23D14_==expected_vehicle);
    if(replace_slot>=0){require(MapWinTable[replace_slot]==window);MapWinTable[replace_slot]=&windows[replace_id];replace_slot=-1;}
    if(hotspot_shape>=0){CursorHotspotX[hotspot_shape]+=100;CursorHotspotY[hotspot_shape]+=200;}
}
int Win::set_cursor(Sprite *sprite,int x,int y){
    MapWin *window=dynamic_cast<MapWin*>(this);require(window && this==static_cast<Win*>(static_cast<GraphicWin*>(window)));
    require(window->cursor_sprite_!=sprite);seen.push_back({'S',window->id,x,y,sprite});callback(window);return 17;
}
int Win::set_cursor(int resource){
    MapWin *window=dynamic_cast<MapWin*>(this);require(window && resource==0x7f00);
    require(window->cursor_sprite_ || window->cursor_resource_id_!=0x7f00);
    seen.push_back({'R',window->id,resource,0,nullptr});callback(window);return 19;
}
void MapWin::draw_map(int full){require(full==1 && !(field_1DD70_&0xc40000u));seen.push_back({'D',id,full,0,nullptr});}
void do_non_input(){
    require(console.field_23D10_==expected_mode && console.field_23D14_==expected_vehicle);
    seen.push_back({'P',-1,0,0,nullptr});
    if(activate_on_pump>=0){MapWinTable[activate_on_pump]->field_1DD74_=1;activate_on_pump=-1;}
    if(change_mode_on_pump>=0){console.field_23D10_=expected_mode=change_mode_on_pump;change_mode_on_pump=-1;}
}
ACTUAL_BODIES
void reset(){
    for(int i=0;i<10;++i){windows[i].id=i;windows[i].cursor_sprite_=nullptr;windows[i].cursor_resource_id_=0x7f00;
        windows[i].field_1DD74_=0;windows[i].field_1DD70_=0;}
    for(size_t i=0;i<MapWinTableSlots;++i)MapWinTable[i]=nullptr;
    for(int i=0;i<12;++i){g_CURSOR_SPRITES[i].id=i;CursorHotspotX[i]=10+i;CursorHotspotY[i]=30+i;}
    console.field_23D10_=expected_mode=71;console.field_23D14_=expected_vehicle=73;
    replace_slot=-1;replace_id=9;activate_on_pump=hotspot_shape=change_mode_on_pump=-1;seen.clear();
}
void expect(size_t index,char kind,int id){require(index<seen.size() && seen[index].kind==kind && seen[index].id==id);}
void run_on(bool vehicle){if(vehicle){expected_mode=4;expected_vehicle=uint32_t(-7);console.cursor_on(4,-7);}else console.cursor_on(4);}
void on_controls(){
    for(int overload=0;overload<2;++overload){
        reset();run_on(overload);require(seen.size()==1);expect(0,'P',-1);
        reset();MapWinTable[0]=&windows[0];MapWinTable[1]=&windows[1];MapWinTable[2]=&windows[2];MapWinTable[7]=&windows[7];
        windows[2].field_1DD74_=windows[7].field_1DD74_=1;windows[2].cursor_sprite_=&g_CURSOR_SPRITES[4];
        hotspot_shape=4;run_on(overload);
        require(seen.size()==3);expect(0,'S',0);expect(1,'S',7);expect(2,'P',-1);
        require(seen[0].a==14 && seen[0].b==34 && seen[1].a==114 && seen[1].b==234);
        require(windows[0].cursor_sprite_==&g_CURSOR_SPRITES[4] && windows[7].cursor_sprite_==&g_CURSOR_SPRITES[4]);
        require(windows[1].cursor_sprite_==nullptr && windows[0].cursor_resource_id_==0x7f00);
        reset();MapWinTable[0]=&windows[0];replace_slot=0;run_on(overload);
        require(MapWinTable[0]==&windows[9] && windows[0].cursor_sprite_==&g_CURSOR_SPRITES[4] && windows[9].cursor_sprite_==nullptr);
        require(seen.size()==2);expect(0,'S',0);expect(1,'P',-1);
    }
}
int run_off(bool selected,uint32_t mode){console.field_23D10_=mode;expected_mode=0;if(selected)return console.selected();console.cursor_off();return -1;}
void off_controls(){
    for(int selected=0;selected<2;++selected){
        reset();require(run_off(selected,0)==(selected?0:-1));require(seen.empty() && console.field_23D10_==0);
        reset();MapWinTable[0]=&windows[0];MapWinTable[1]=&windows[1];MapWinTable[2]=&windows[2];MapWinTable[7]=&windows[7];
        windows[0].cursor_sprite_=&g_CURSOR_SPRITES[4];windows[0].field_1DD70_=0xffffffffu;
        windows[1].cursor_sprite_=&g_CURSOR_SPRITES[4];windows[1].field_1DD70_=0xffffffffu;
        windows[2].field_1DD74_=windows[7].field_1DD74_=1;windows[2].field_1DD70_=0xc40021;
        windows[7].cursor_resource_id_=123;
        require(run_off(selected,7)==(selected?7:-1));require(seen.size()==7);
        expect(0,'R',0);expect(1,'D',0);expect(2,'P',-1);expect(3,'D',2);expect(4,'P',-1);expect(5,'R',7);expect(6,'P',-1);
        require(windows[0].field_1DD70_==0xff3bffffu && windows[2].field_1DD70_==0x21);
        require(windows[0].cursor_sprite_==nullptr && windows[7].cursor_resource_id_==0x7f00);
        require(windows[1].cursor_sprite_==&g_CURSOR_SPRITES[4] && windows[1].field_1DD70_==0xffffffffu);
        reset();MapWinTable[0]=&windows[0];windows[0].cursor_sprite_=&g_CURSOR_SPRITES[4];windows[0].field_1DD70_=0xc40033;
        windows[9].cursor_sprite_=&g_CURSOR_SPRITES[2];windows[9].cursor_resource_id_=99;windows[9].field_1DD70_=0xc40011;replace_slot=0;
        require(run_off(selected,7)==(selected?7:-1));require(seen.size()==3);expect(0,'R',0);expect(1,'D',9);expect(2,'P',-1);
        require(windows[0].cursor_sprite_==nullptr && windows[0].cursor_resource_id_==0x7f00 && windows[0].field_1DD70_==0xc40033);
        require(windows[9].cursor_sprite_==&g_CURSOR_SPRITES[2] && windows[9].cursor_resource_id_==99 && windows[9].field_1DD70_==0x11);
        reset();MapWinTable[0]=&windows[0];MapWinTable[1]=&windows[1];activate_on_pump=1;
        require(run_off(selected,7)==(selected?7:-1));require(seen.size()==2);expect(0,'P',-1);expect(1,'P',-1);
        reset();MapWinTable[0]=&windows[0];MapWinTable[1]=&windows[1];windows[1].field_1DD74_=1;change_mode_on_pump=77;
        require(run_off(selected,7)==(selected?7:-1));require(seen.size()==2 && console.field_23D10_==77);
    }
    reset();MapWinTable[0]=&windows[0];windows[0].cursor_sprite_=&g_CURSOR_SPRITES[4];console.field_23D10_=0;
    console.field_23D10_=0;require(console.selected()==0);require(seen.empty() && windows[0].cursor_sprite_==&g_CURSOR_SPRITES[4]);
}
int main(){on_controls();off_controls();}
'''


def run_host(tmp_path,fields,console_fields,bound,bodies):
    compiler=shutil.which("clang++") or shutil.which("g++")
    if not compiler:pytest.skip("host C++ compiler required")
    source=tmp_path/"cursor.cpp";binary=tmp_path/"cursor"
    source.write_text(HARNESS.replace("ACTUAL_MAP_FIELDS",fields).replace("ACTUAL_CONSOLE_FIELDS",console_fields).replace("ACTUAL_BOUND",bound).replace("ACTUAL_BODIES",bodies))
    compiled=subprocess.run([compiler,"-std=c++11","-O1","-fsanitize=address,undefined","-fno-sanitize-recover=all",str(source),"-o",str(binary)],capture_output=True,text=True,timeout=30)
    assert compiled.returncode==0,compiled.stderr
    return subprocess.run([str(binary)],capture_output=True,text=True,timeout=10,env=dict(os.environ,ASAN_OPTIONS="detect_leaks=0"))


def test_actual_console_cursor_state(tmp_path):
    result=run_host(tmp_path,*actual_source());assert result.returncode==0,result.stderr


MUTANTS=[
    ("primary-active-bypass","slot == 0 || window->field_1DD74_ != 0","window->field_1DD74_ != 0"),
    ("inactive-filter","slot == 0 || window->field_1DD74_ != 0","true"),
    ("last-slot","slot < static_cast<int>(MapWinTableSlots)","slot < static_cast<int>(MapWinTableSlots) - 1"),
    ("sprite-cached-writeback","window->cursor_sprite_ = sprite;","MapWinTable[slot]->cursor_sprite_ = sprite;"),
    ("resource-cached-writeback","window->cursor_resource_id_ = 0x7f00;","MapWinTable[slot]->cursor_resource_id_ = 0x7f00;"),
    ("flags-live-slot","const uint32_t flags = MapWinTable[slot]->field_1DD70_;","const uint32_t flags = window->field_1DD70_;"),
    ("draw-live-slot","MapWinTable[slot]->draw_map(1);","window->draw_map(1);"),
    ("flag-clear-mask","flags & 0xff3bffff","flags & 0xfffbffff"),
    ("selected-zero-noop","if (selected_mode != 0)","if (true)"),
    ("hotspot-axis","CursorHotspotX[cursor_shape], CursorHotspotY[cursor_shape]","CursorHotspotY[cursor_shape], CursorHotspotX[cursor_shape]"),
    ("message-pump","do_non_input();",""),
    ("redundant-sprite-set","if (window->cursor_sprite_ != sprite)","if (true)"),
]


@pytest.mark.parametrize("name,before,after",MUTANTS,ids=[x[0] for x in MUTANTS])
def test_compiling_cursor_state_damage(tmp_path,name,before,after):
    fields,console,bound,bodies=actual_source();assert before in bodies,(name,"missing mutation anchor")
    result=run_host(tmp_path,fields,console,bound,bodies.replace(before,after))
    assert result.returncode!=0,(name,"behavior damage escaped controls")
