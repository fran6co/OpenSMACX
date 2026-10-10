"""Actual owner bodies with service doubles; no native GUI/layout proof."""

from pathlib import Path
import shutil
import subprocess

import pytest

from test_win_zorder_pointer_flow_oct10 import extract

ROOT = Path(__file__).resolve().parents[2]


def run_control(tmp_path, compiler, program, damage):
    executable = shutil.which(compiler)
    if executable is None:
        pytest.skip(compiler + " unavailable")
    if damage:
        assert program.count(damage[0]) == 1
        program = program.replace(*damage, 1)
    source = tmp_path / "owner.cpp"
    source.write_text(program)
    binary = tmp_path / "owner"
    result = subprocess.run([executable, "-std=c++11", "-O0", str(source), "-o", str(binary)], capture_output=True, text=True)
    assert result.returncode == 0, result.stderr
    result = subprocess.run([str(binary)], capture_output=True, timeout=10)
    assert (result.returncode == 0) == (damage is None), result.stderr


def patrol_control():
    body = extract((ROOT / "src/console.cpp").read_text(), "void Console::toggle_patrol()")
    assert "reinterpret_cast" not in body
    return r'''
#include <cassert>
#include <cstdint>
#include <vector>
struct MapWin {
    int id;
    uint32_t field_1DD70_,field_1DD74_;
    void draw_base_dest(int);void draw_map(int);
};
static MapWin *MapWinTable[8],*replacement;
static const int MapWinTableSlots=8;
static std::vector<int> calls;
static bool rebind;
void MapWin::draw_base_dest(int n){
    assert(n==1);calls.push_back(id);
    if(rebind && id==0){MapWinTable[3]=replacement;field_1DD70_=0;}
}
void MapWin::draw_map(int n){assert(n==1);calls.push_back(id+10);}
struct Console {void toggle_patrol();};
BODY
int main(){
    MapWin windows[8];MapWin other{8,0,1};replacement=&other;
    for(int mode=0;mode!=2;++mode){
        calls.clear();rebind=mode==0;
        for(int i=0;i!=8;++i){windows[i]={i,static_cast<uint32_t>(mode?0x400040:0x40),1};MapWinTable[i]=&windows[i];}
        windows[0].field_1DD74_=0; // First slot is admitted anyway.
        MapWinTable[1]=nullptr;windows[2].field_1DD74_=0;
        Console console;console.toggle_patrol();
        if(!mode){
            assert((calls==std::vector<int>{0,8,4,5,6,7}));
            assert(other.field_1DD70_==0x400000 && windows[3].field_1DD70_==0x40);
            assert(windows[7].field_1DD70_==0x400040); // Snapshot survives callback.
        } else {
            assert((calls==std::vector<int>{10,13,14,15,16,17}));
            assert(windows[7].field_1DD70_==0x40);
        }
        assert(windows[2].field_1DD70_==static_cast<uint32_t>(mode?0x400040:0x40));
    }
}
'''.replace("BODY", body)


@pytest.mark.parametrize("compiler", ("g++", "clang++"))
@pytest.mark.parametrize("damage", (None,
    ("p == MapWinTable ||", "false ||"),
    ("int mask = first->field_1DD70_ & 0x400000;", "int mask = 0;"),
    ("p = p + 1;", "p = p + 2;")))
def test_actual_typed_patrol_walk(tmp_path, compiler, damage):
    run_control(tmp_path, compiler, patrol_control(), damage)


def map_control():
    source = (ROOT / "src/console.cpp").read_text()
    body = extract(source, "void Console::editor_fast()")
    record = extract((ROOT / "src/map.h").read_text(), "struct Map {") + ";"
    return r'''
#include <cassert>
#include <cstdint>
#include <cstring>
RECORD
static_assert(sizeof(Map)==44,"actual record stride");
static Map tiles[3],unused[3];
static Map *MapTiles;
static uint32_t MapArea;
static Map *&map_tiles(){return MapTiles;}
struct {struct {uint32_t game_state_;int vehicle_current_count_,base_current_count_;} fields_;} GameDataStateStorage;
static int stage,shown,hidden;
static void auto_undo(){assert(stage++==0);MapTiles=tiles;}
static void world_build(){assert(stage++==1);assert(!(GameDataStateStorage.fields_.game_state_&0x80));GameDataStateStorage.fields_.game_state_=0x41;MapTiles=unused;}
static void draw_map(int n){assert(n==1 && stage++==2);}
struct Button {void show(int n){assert(n==0);++shown;}void hide(){++hidden;}};
struct {Button flat_button_[3];} MainInterfaceGlobal;
struct Console {int cursor_active_,selected_vehicle_;void editor_fast();};
static Console ConsoleGlobal;
BODY
int main(){
    for(int n=-1;n!=4;++n){
        memset(tiles,0xAA,sizeof tiles);memset(unused,0xCC,sizeof unused);
        for(auto &tile:tiles)tile.bit=0xFFFFFFFF;
        MapArea=static_cast<uint32_t>(n);MapTiles=unused;
        for(int flag=0;flag!=2;++flag){
            for(auto &tile:tiles)tile.bit=0xFFFFFFFF;
            stage=shown=hidden=0;
            GameDataStateStorage.fields_={static_cast<uint32_t>(flag?0x80:0),77,88};
            ConsoleGlobal.cursor_active_=9;ConsoleGlobal.selected_vehicle_=4;
            ConsoleGlobal.editor_fast();
            assert(stage==3 && !GameDataStateStorage.fields_.vehicle_current_count_ && !GameDataStateStorage.fields_.base_current_count_);
            assert(!ConsoleGlobal.cursor_active_ && ConsoleGlobal.selected_vehicle_==-1);
            assert(shown==flag && hidden==1-flag);
            for(int i=0;i!=3;++i){assert(tiles[i].bit==static_cast<uint32_t>(i<n?0xFFFFFFFC:0xFFFFFFFF));assert(tiles[i].bit2==0xAAAAAAAA && tiles[i].climate==0xAA);assert(unused[i].bit==0xCCCCCCCC);}
        }
    }
}
'''.replace("RECORD", record).replace("BODY", body)


@pytest.mark.parametrize("compiler", ("g++", "clang++"))
@pytest.mark.parametrize("damage", (None,
    ("base[i].bit &= 0xfffffffc;", "base[i].bit2 &= 0xfffffffc;"),
    ("Map *base = map_tiles();", "Map *base = unused;")))
def test_actual_map_member_updates(tmp_path, compiler, damage):
    run_control(tmp_path, compiler, map_control(), damage)


def selection_control():
    source = extract((ROOT / "src/console.cpp").read_text(), "void Console::time_controls()")
    statement = "popup.set_value(GameDataStateStorage.fields_.time_controls_selection_);"
    assert source.count(statement)==1
    assert source.index("time_controls_dialog(&popup);") < source.index(statement) < source.index("popup.exec(0, 0)")
    setter = extract((ROOT / "src/basepop.h").read_text(), "void set_value(int value)")
    return r'''
#include <cassert>
#include <cstdint>
struct BasePop {int decoy_=91,value_=-1;SETTER};
struct Popup:BasePop {};
struct {struct {int time_controls_selection_;} fields_;} GameDataStateStorage;
int main(){for(int value:{-1,0,5,(-2147483647-1),2147483647}){Popup popup;GameDataStateStorage.fields_.time_controls_selection_=value;STATEMENT assert(popup.value_==value && popup.decoy_==91);}}
'''.replace("SETTER", setter).replace("STATEMENT", statement).replace("#include <cstdint>","#include <cstdint>\n#include <initializer_list>")


@pytest.mark.parametrize("compiler", ("g++", "clang++"))
@pytest.mark.parametrize("damage", (None,("value_ = value;", "decoy_ = value;")))
def test_actual_popup_selection_store(tmp_path, compiler, damage):
    # Bounded member/store control, not the entire time-controls UI.
    run_control(tmp_path, compiler, selection_control(), damage)


def splitter_control():
    body = extract((ROOT / "src/console.cpp").read_text(), "void Console::on_nc_mouse_move(int x_coord, int y_coord)")
    header = (ROOT / "src/sprite.h").read_text()
    getters = extract(header,"int width() const") + extract(header,"int height() const")
    return r'''
#include <cassert>
#include <cstdint>
struct RECT {int left,top,right,bottom;};
struct Sprite {int iWidth_,iHeight_;GETTERS};
static int ConsoleDragLineX,ConsoleDragLineY,updates,lines;
struct GraphicWin {void update(int,int,int,int,void *){++updates;}};
struct Console:GraphicWin {
    Sprite sprite_a_{20,70},sprite_b_{80,30};
    uint32_t field_23D80_=5,field_23D84_=60,field_23D88_=40,field_23D8C_=90;
    int border_thickness_=2,bottom_border_thickness_=3,caption_height_=4;
    RECT client_rect_{0,0,200,160},outer_rect_{0,0,200,160};
    struct {struct {int biWidth=200,biHeight=-160;} bmiHeader;} dib_;
    void redraw_nc_buffer(int){sprite_a_.iWidth_=180;sprite_b_.iHeight_=140;}
    void update_nc_buffer(int){}
    void window_line_raw(int,int,int,int,int,int,int){++lines;}
    void on_nc_mouse_move(int,int);
};
BODY
int main(){
    for(int slot=0;slot!=4;++slot)for(int upper=0;upper!=2;++upper){
        Console console;
        uint32_t *slots[]={&console.field_23D80_,&console.field_23D84_,&console.field_23D88_,&console.field_23D8C_};
        *slots[slot]=0xFFFFFFFF;updates=lines=0;
        console.on_nc_mouse_move(upper?1000:-200,upper?1000:-200);
        assert(updates==2 && lines==2 && *slots[slot]==0xFFFFFFFF);
        if(slot<2){assert(ConsoleDragLineX==(upper?186:10) && ConsoleDragLineY==40);}
        else {assert(ConsoleDragLineY==(upper?138:15) && ConsoleDragLineX==(slot==2?5:60));}
    }
    Console idle;updates=lines=0;idle.on_nc_mouse_move(10,20);assert(!updates && !lines);
}
'''.replace("GETTERS", getters).replace("BODY", body)


@pytest.mark.parametrize("compiler", ("g++", "clang++"))
@pytest.mark.parametrize("damage", (None,
    ("sprite_a_.width()", "sprite_a_.height()"),
    ("sprite_b_.height()", "sprite_b_.width()")))
def test_actual_splitter_member_geometry(tmp_path, compiler, damage):
    run_control(tmp_path, compiler, splitter_control(), damage)
