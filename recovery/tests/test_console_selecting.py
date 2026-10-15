"""Original-derived controls for the complete Console selection controller.

Extracts its actual body/helper, hierarchy clauses, field/callee declarations,
vehicle type, window-table bound and hotspot definitions. Cursor sprites are
identity-only handles; cursor installation, patrol validation and pumping are
controlled service boundaries. Host base conversion and sanitizers do not prove
native layouts, sprite resources, complete game services or linked acceptance.
Shapes are valid loaded indices, range-enabled vehicle IDs are valid, and
coordinate differences/sums fit signed int without abs(INT_MIN). Callback
replacements retain valid objects. Noncanonical bounded coordinates are named
explicitly rather than claimed reachable gameplay states.
"""

from pathlib import Path
import os
import re
import shutil
import subprocess

import pytest


ROOT = Path(__file__).resolve().parents[2]


def braced(source, signature):
    start = source.index(signature)
    opening = source.index("{", start)
    depth = 0
    for end in range(opening, len(source)):
        depth += (source[end] == "{") - (source[end] == "}")
        if not depth:
            return source[start:end + 1]
    raise AssertionError("unterminated actual definition")


def actual_source():
    h = {name: (ROOT / "src" / (name + ".h")).read_text() for name in
         ("win", "graphicwin", "mapwin", "console", "veh", "map", "sprite")}
    source = (ROOT / "src/console.cpp").read_text()
    parts = {}
    for unit, cls in (("win", "Win"), ("graphicwin", "GraphicWin"),
                      ("mapwin", "MapWin"), ("console", "Console")):
        parts[cls.upper() + "_BASES"], = re.findall(r"^class " + cls + r"\s*:[^{]+\{", h[unit], re.M)
    parts["VEH"] = braced(h["veh"], "struct Veh {") + ";"
    fields = []
    for name in ("field_1DD74_", "cursor_sprite_", "cursor_resource_id_"):
        declaration, = re.findall(r"^  (?:uint32_t |Sprite \*)" + name + r";", h["mapwin"], re.M)
        fields.append(declaration)
    parts["MAP_MEMBERS"] = "\n".join(fields)
    fields = []
    for name in ("selected_vehicle_", "field_23D10_", "field_23D14_"):
        declaration, = re.findall(r"^  (?:uint32_t|int32_t) " + name + r";", h["console"], re.M)
        fields.append(declaration)
    parts["CONSOLE_MEMBERS"] = "\n".join(fields)
    parts["CURSOR_METHOD"], = re.findall(r"^  int set_cursor\(Sprite\* sprite, int hot_x, int hot_y\);", h["win"], re.M)
    parts["SELECTING_METHOD"], = re.findall(r"^  int selecting\([^;{}]*\);", h["console"], re.M)
    parts["PATROL_METHOD"], = re.findall(r"^int __cdecl valid_patrol\([^;{}]*\);", h["veh"], re.M)
    parts["BOUND"], = re.findall(r"^static const size_t MapWinTableSlots = \d+;", h["mapwin"], re.M)
    parts["SPRITES"], = re.findall(r"^extern Sprite g_CURSOR_SPRITES\[12\];", h["sprite"], re.M)
    parts["SPRITES"] = parts["SPRITES"].removeprefix("extern ")
    globals_ = []
    declaration, = re.findall(r"^extern uint32_t MapLongitude;", h["map"], re.M)
    globals_.append(declaration.removeprefix("extern "))
    # The other two names are source macros into MapStorage metadata, so the
    # host fixture supplies same-width independent words for their semantics.
    globals_.extend(("int MapLongitudeBounds;", "BOOL MapIsFlat;"))
    for name in ("CursorHotspotX", "CursorHotspotY"):
        definition, = re.findall(r"^int " + name + r"\[12\] = \{0\};", source, re.M)
        globals_.append(definition)
    parts["GLOBALS"] = "\n".join(globals_)
    parts["HELPER"] = braced(source, "int selecting_signed(")
    parts["BODY"] = braced(source, "int Console::selecting(")
    return parts


HARNESS = r'''
#include <cstdint>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>
#define __cdecl
typedef int BOOL;
struct Sprite{};
struct AutoSound {virtual ~AutoSound(){}};
struct Buffer{};
WIN_BASES public:CURSOR_METHOD};
GRAPHICWIN_BASES public:};
MAPWIN_BASES public:MAP_MEMBERS int id;};
CONSOLE_BASES public:CONSOLE_MEMBERS SELECTING_METHOD};
VEH
static_assert(sizeof(Veh)==52&&offsetof(Veh,x)==0&&offsetof(Veh,y)==2,"actual signed vehicle coordinates");
BOUND
SPRITES
GLOBALS
PATROL_METHOD
MapWin *MapWinTable[MapWinTableSlots];MapWin windows[10];Console console;Veh Vehs[3];
struct Event{char kind;int id,shape,x,y;};std::vector<Event> seen;
int scenario,patrol_result,expected_vehicle,expected_x,expected_y;
void check(bool value,int line){if(!value){std::fprintf(stderr,"assertion line %d scenario %d\n",line,scenario);std::exit(94);}}
#define require(x) check((x),__LINE__)
int valid_patrol(int vehicle,int x,int y){
    require(vehicle==expected_vehicle&&x==expected_x&&y==expected_y);seen.push_back({'V',vehicle,0,x,y});
    if(scenario==1)console.field_23D10_=3;
    if(scenario==2)console.field_23D10_=11;
    return patrol_result;
}
int Win::set_cursor(Sprite *sprite,int x,int y){
    MapWin *owner=dynamic_cast<MapWin*>(this);
    require(owner&&this==static_cast<Win*>(static_cast<GraphicWin*>(owner)));
    require(owner->cursor_sprite_!=sprite);
    int shape=-1;for(int i=0;i<12;++i)if(sprite==&g_CURSOR_SPRITES[i])shape=i;
    require(shape>=0);seen.push_back({'S',owner->id,shape,x,y});
    if(scenario==3&&owner==&windows[0]){
        MapWinTable[0]=&windows[9];MapWinTable[7]=&windows[7];windows[7].field_1DD74_=1;
        console.field_23D10_=4;CursorHotspotX[shape]=101;CursorHotspotY[shape]=202;
    }
    if(scenario==4){console.field_23D10_=4;CursorHotspotX[shape]+=100;CursorHotspotY[shape]+=200;}
    if(scenario==5&&owner==&windows[0]){MapWinTable[2]=&windows[8];windows[8].field_1DD74_=1;}
    if(scenario==6&&owner==&windows[0]){windows[2].field_1DD74_=1;}
    return -3;
}
void do_non_input(){seen.push_back({'P',-1,0,0,0});
    if(scenario==7){console.field_23D10_=77;MapWinTable[7]=&windows[7];windows[7].field_1DD74_=1;}}
HELPER
BODY
void reset(){
    seen.clear();scenario=0;patrol_result=1;expected_vehicle=1;expected_x=10;expected_y=10;
    console.field_23D10_=2;console.field_23D14_=-1;console.selected_vehicle_=1;
    std::memset(Vehs,0,sizeof(Vehs));Vehs[1].x=4;Vehs[1].y=6;
    MapLongitudeBounds=64;MapLongitude=32;MapIsFlat=0;
    for(size_t i=0;i<MapWinTableSlots;++i)MapWinTable[i]=nullptr;
    for(int i=0;i<10;++i){windows[i].id=i;windows[i].field_1DD74_=0;windows[i].cursor_sprite_=nullptr;windows[i].cursor_resource_id_=1000+i;}
    for(int i=0;i<12;++i){CursorHotspotX[i]=10+i;CursorHotspotY[i]=30+i;}
}
void event(size_t index,char kind,int id,int shape=-1){require(index<seen.size()&&seen[index].kind==kind&&seen[index].id==id);
    if(shape>=0)require(seen[index].shape==shape);}
void one_window(){MapWinTable[0]=&windows[0];}
int choose(int x=10,int y=10){expected_x=x;expected_y=y;return console.selecting(x,y);}
void early_controls(){
    for(int mode=0;mode<2;++mode){reset();one_window();console.field_23D10_=mode;console.field_23D14_=0;
        require(choose()==mode&&seen.empty()&&!windows[0].cursor_sprite_);}
    reset();require(choose()==1&&seen.size()==1);event(0,'P',-1);
    reset();one_window();require(choose()==1);event(0,'S',0,2);
    for(int mode=0;mode<2;++mode){reset();console.field_23D10_=mode;console.field_23D14_=0;console.selected_vehicle_=-1;
        require(choose()==mode&&seen.empty());}
    reset();console.selected_vehicle_=-1;one_window();require(choose()==1);event(0,'S',0,2);event(1,'P',-1);
    reset();console.field_23D14_=(-2147483647 - 1);console.selected_vehicle_=-1;one_window();require(choose()==1);event(0,'S',0,2);
    reset();console.field_23D14_=(-2147483647 - 1);console.selected_vehicle_=1;one_window();require(choose()==1);event(0,'S',0,2);
}
void range_controls(){
    // Independent original geometry: (absX wrapped at stored HALF width +
    // absY) /2, range equality admitted; these examples use no candidate helper.
    struct Case{int x,y,range,flat,half,shape;};
    const Case cases[]={
        {10,10,5,0,32,2},{10,10,4,0,32,5},{9,10,4,0,32,2},
        {-2,-2,6,0,32,5},{-2,-2,7,0,32,2},
        {36,6,15,0,32,5},{36,6,16,0,32,2},
        {64,6,2,0,32,2},{64,6,1,0,32,5},
        {64,6,2,1,32,5},{64,6,2,2,32,2},
        {29,6,15,0,20,5},{29,6,15,1,20,2},
        {24,6,10,0,20,2},
        // Bounded noncanonical coordinate: one subtraction produces -5.
        // Its negativity, not a claimed reachable map state, admits range0.
        {73,6,0,0,32,2},
    };
    for(const auto&t:cases){reset();one_window();console.field_23D14_=t.range;MapIsFlat=t.flat;MapLongitude=t.half;
        require(choose(t.x,t.y)==1&&seen.size()==2);event(0,'S',0,t.shape);event(1,'P',-1);
        require(windows[0].cursor_resource_id_==1000);}
    reset();one_window();Vehs[1].x=-10;Vehs[1].y=-12;console.field_23D14_=2;
    require(choose(-6,-12)==1);event(0,'S',0,2);
}
void patrol_controls(){
    reset();one_window();console.field_23D10_=8;require(choose()==1&&seen.size()==3);event(0,'V',1);event(1,'S',0,8);event(2,'P',-1);
    reset();one_window();console.field_23D10_=8;patrol_result=0;require(choose()==1);event(0,'V',1);event(1,'S',0,5);
    reset();one_window();console.field_23D10_=8;console.field_23D14_=0;require(choose()==1&&seen.size()==2);event(0,'S',0,5);
    reset();one_window();console.field_23D10_=8;scenario=1;require(choose()==1);event(1,'S',0,3);require(console.field_23D10_==3);
    reset();one_window();console.field_23D10_=8;scenario=2;patrol_result=0;require(choose()==1);event(1,'S',0,5);require(console.field_23D10_==11);
    reset();one_window();console.field_23D10_=11;require(choose()==1&&seen.size()==2);event(0,'S',0,11);
}
void callback_controls(){
    for(int refused=0;refused<2;++refused){reset();one_window();if(refused)console.field_23D14_=0;
        scenario=3;const int shape=refused?5:2;require(choose()==1&&seen.size()==3);
        event(0,'S',0,shape);event(1,'S',7,shape);event(2,'P',-1);
        require(seen[0].x==10+shape&&seen[0].y==30+shape&&seen[1].x==101&&seen[1].y==202);
        require(MapWinTable[0]==&windows[9]&&windows[0].cursor_sprite_==&g_CURSOR_SPRITES[shape]&&!windows[9].cursor_sprite_);
        require(windows[7].cursor_sprite_==&g_CURSOR_SPRITES[shape]&&console.field_23D10_==4);
        require(windows[0].cursor_resource_id_==1000&&windows[7].cursor_resource_id_==1007);}
    reset();one_window();MapWinTable[1]=&windows[1];MapWinTable[2]=&windows[2];MapWinTable[7]=&windows[7];
    windows[2].field_1DD74_=windows[7].field_1DD74_=1;windows[2].cursor_sprite_=&g_CURSOR_SPRITES[2];scenario=4;
    require(choose()==1&&seen.size()==3);event(0,'S',0,2);event(1,'S',7,2);event(2,'P',-1);
    require(!windows[1].cursor_sprite_&&seen[1].x==112&&seen[1].y==232);
    reset();one_window();MapWinTable[2]=&windows[2];windows[2].field_1DD74_=1;scenario=5;
    require(choose()==1&&seen.size()==3);event(1,'S',8,2);require(!windows[2].cursor_sprite_);
    reset();one_window();MapWinTable[2]=&windows[2];scenario=6;require(choose()==1&&seen.size()==3);event(1,'S',2,2);
    reset();one_window();windows[0].cursor_sprite_=&g_CURSOR_SPRITES[2];require(choose()==1&&seen.size()==1);event(0,'P',-1);
    reset();one_window();scenario=7;require(choose()==1&&seen.size()==2);event(1,'P',-1);
    require(console.field_23D10_==77&&!windows[7].cursor_sprite_);
}
int main(){early_controls();range_controls();patrol_controls();callback_controls();}
'''


def run_host(tmp_path, parts):
    compiler = shutil.which("clang++") or shutil.which("g++")
    if not compiler:
        pytest.skip("host C++ compiler required")
    source, binary = tmp_path / "selecting.cpp", tmp_path / "selecting"
    source.write_text(re.sub(r"\b(" + "|".join(parts) + r")\b", lambda m: parts[m.group()], HARNESS))
    compiled = subprocess.run([compiler, "-std=c++11", "-O1", "-fsanitize=address,undefined", "-fno-sanitize-recover=all",
                               str(source), "-o", str(binary)], capture_output=True, text=True, timeout=30)
    assert compiled.returncode == 0, compiled.stderr
    return subprocess.run([str(binary)], capture_output=True, text=True, timeout=10,
                          env=dict(os.environ, ASAN_OPTIONS="detect_leaks=0"))


def test_actual_console_selecting(tmp_path):
    result = run_host(tmp_path, actual_source())
    assert result.returncode == 0, result.stderr


MUTATIONS = [
    ("mode1_wrong_result", "if (initial_mode == 1) {\n        return 1;", "if (initial_mode == 1) {\n        return 0;"),
    ("mode0_wrong_result", "if (initial_mode == 0) {\n        return 0;", "if (initial_mode == 0) {\n        return 1;"),
    ("mode1_not_early", "if (initial_mode == 1)", "if (false)"),
    ("mode0_not_early", "if (initial_mode == 0)", "if (false)"),
    ("unsigned_range_gate", "field_23D14_ >= 0", "static_cast<uint32_t>(field_23D14_) >= 0U"),
    ("x_absolute_value", "abs(tile_x - vehicle_x)", "(tile_x - vehicle_x)"),
    ("y_absolute_value", "abs(tile_y - vehicle_y)", "(tile_y - vehicle_y)"),
    ("full_instead_of_half_width", "dx > selecting_signed(MapLongitude)", "dx > MapLongitudeBounds"),
    ("half_width_equality", "dx > selecting_signed(MapLongitude)", "dx >= selecting_signed(MapLongitude)"),
    ("derived_instead_of_stored_half", "dx > selecting_signed(MapLongitude)", "dx > MapLongitudeBounds / 2"),
    ("flat_boolean_instead_of_bit", "(MapIsFlat & 1) == 0", "MapIsFlat == 0"),
    ("wrong_wrap_subtraction", "dx = MapLongitudeBounds - dx", "dx = dx - MapLongitudeBounds"),
    ("distance_scaling", "distance_sum / 2", "distance_sum / 4"),
    ("range_equality_refused", "distance > field_23D14_", "distance >= field_23D14_"),
    ("patrol_after_range_refusal", "!forbidden && field_23D10_ == 8", "field_23D10_ == 8"),
    ("patrol_receiver_argument", "valid_patrol(selected_vehicle_,", "valid_patrol(selected_vehicle_ + 1,"),
    ("patrol_result", "valid_patrol(selected_vehicle_, tile_x, tile_y) == 0", "valid_patrol(selected_vehicle_, tile_x, tile_y) != 0"),
    ("initial_instead_of_post_patrol_mode", "forbidden ? 5 : selecting_signed(field_23D10_)", "forbidden ? 5 : selecting_signed(initial_mode)"),
    ("wrong_forbidden_sprite", "forbidden ? 5 :", "forbidden ? 4 :"),
    ("primary_active_gate", "slot == 0 || window->field_1DD74_ != 0", "window->field_1DD74_ != 0"),
    ("inactive_window_admitted", "slot == 0 || window->field_1DD74_ != 0", "true"),
    ("last_table_slot", "slot < static_cast<int>(MapWinTableSlots)", "slot < static_cast<int>(MapWinTableSlots) - 1"),
    ("redundant_cursor_install", "window->cursor_sprite_ != sprite", "true"),
    ("hotspot_axes", "CursorHotspotX[shape], CursorHotspotY[shape]", "CursorHotspotY[shape], CursorHotspotX[shape]"),
    ("replacement_writeback", "window->cursor_sprite_ = sprite;", "MapWinTable[slot]->cursor_sprite_ = sprite;"),
    ("resource_cursor_corruption", "window->cursor_sprite_ = sprite;", "window->cursor_sprite_ = sprite; window->cursor_resource_id_ = 0;"),
    ("missing_terminal_pump", "do_non_input();", ";"),
    ("forbidden_wrong_return", "do_non_input();\n    return 1;", "do_non_input();\n    return forbidden ? 0 : 1;"),
    ("mode_changed_inside_loop", "&g_CURSOR_SPRITES[shape]", "&g_CURSOR_SPRITES[forbidden ? 5 : selecting_signed(field_23D10_)]"),
    ("hotspots_cached_before_callbacks", "for (int slot = 0;", "const int cached_x=CursorHotspotX[shape], cached_y=CursorHotspotY[shape];\n    for (int slot = 0;"),
    ("table_cached_before_callbacks", "for (int slot = 0;", "MapWin *snapshot[MapWinTableSlots]; for(size_t i=0;i<MapWinTableSlots;++i) snapshot[i]=MapWinTable[i];\n    for (int slot = 0;"),
]


@pytest.mark.parametrize("name,old,new", MUTATIONS, ids=[m[0] for m in MUTATIONS])
def test_selecting_damage_controls(tmp_path, name, old, new):
    parts = actual_source()
    assert old in parts["BODY"], name
    parts["BODY"] = parts["BODY"].replace(old, new)
    if name == "hotspots_cached_before_callbacks":
        parts["BODY"] = parts["BODY"].replace("CursorHotspotX[shape], CursorHotspotY[shape]", "cached_x, cached_y")
    if name == "table_cached_before_callbacks":
        parts["BODY"] = parts["BODY"].replace("window = MapWinTable[slot]", "window = snapshot[slot]")
    result = run_host(tmp_path, parts)
    if name == "unsigned_range_gate":
        assert result.returncode != 0 and (
            result.returncode == 94
            or "AddressSanitizer" in result.stderr
            or "runtime error:" in result.stderr
        ), (name, result.stderr)
        return
    assert result.returncode == 94 and "assertion line" in result.stderr, (name, result.stderr)
