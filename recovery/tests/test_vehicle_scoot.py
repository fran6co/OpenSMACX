"""Actual veh_scoot control/lifetime tests against explicit host dependencies.

Pinned 55A150..55AF11 and EH 65FF70..65FFC7 supply event expectations.
Buffer, Vector, windows, drawing and time are instrumented boundary doubles;
these tests do not establish their implementations, VC6 EH/ABI, native code
matching, actual pixels, or whole-game acceptance. Vector doubles model the
established mutating scale/value return and reference compound operations.
"""

from pathlib import Path
import re
import shutil
import subprocess

import pytest


ROOT = Path(__file__).resolve().parents[2]


def braced(text, signature):
    start = text.index(signature)
    opening = text.index("{", start)
    depth = 0
    for end in range(opening, len(text)):
        depth += (text[end] == "{") - (text[end] == "}")
        if depth == 0:
            return text[start:end + 1]
    raise AssertionError("unterminated actual body")


def actual_source():
    source = (ROOT / "src/veh.cpp").read_text()
    arrays = []
    for name in ("VehMoveDeltaX", "VehMoveDeltaY"):
        found = re.findall(r"^int " + name + r"\[9\]\s*=\s*\{[^}]+\};", source, re.M)
        assert len(found) == 1
        arrays.append(found[0])
    return "\n".join(arrays + [braced(source, "void __cdecl veh_scoot(")])


HARNESS = r'''
#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>
#define __cdecl
typedef uint32_t DWORD;
void require(bool ok) { if (!ok) std::exit(42); }
struct RECT { int left, top, right, bottom; };
struct Event { std::string kind; std::vector<int> a; };
std::vector<Event> events;
void event(const char *kind, std::initializer_list<int> args = {}) {
    events.push_back({kind, args});
}
bool track = false, replace_slots = false, change_preference = false;
int throwing = 0, buffer_count = 0, vector_count = 0, tile_calls = 0;
int clip_calls = 0, intersect_calls = 0, reject_intersection = 0;
int turn_frames = 1, time_index = 0;
DWORD clock_value = 0;
std::vector<DWORD> times;
int MapIsFlat = 0, MapLongitudeBounds = 100;
int IsMultiplayerNet = 0, FastUnitAnim = 1;
int VehDrawPositionX = -1, VehDrawPositionY = -1, VehDrawPositionVehicle = -1;
struct { int faction_id_; } ConsoleGlobal = {1};
struct Vehicle { uint8_t faction_id; int16_t proto_id; int8_t facing; } Vehs[2] = {{1,0,-1},{2,0,-1}};
struct Prototype { uint8_t chassis_id; } VehPrototypes[2] = {};
struct ChassisEntry { uint8_t triad; } Chassis[19] = {};
struct MapWin;
extern MapWin *MapWinTable[8];
void replace_slot(int index);

struct Buffer {
    int saved_id;
    int view_id;
    Buffer() : saved_id(track ? buffer_count++ : -1), view_id(-1) {
        if (track) event("buffer_ctor", {saved_id});
    }
    ~Buffer() { if (saved_id >= 0) event("buffer_dtor", {saved_id}); }
    int init(int width, int height, int flags, void *surface) {
        require(saved_id >= 0);
        event("init", {saved_id, width, height, flags, surface != nullptr});
        if (change_preference) FastUnitAnim = -1;
        if (replace_slots) replace_slot(5);
        if (throwing == 2) throw 77;
        return 0;
    }
    int copy(Buffer *destination, int rx, int ry, int wx, int wy, int width, int height) {
        event(saved_id >= 0 ? "restore" : "capture",
            {saved_id >= 0 ? saved_id : view_id,
             destination->saved_id >= 0 ? destination->saved_id : destination->view_id,
             rx, ry, wx, wy, width, height});
        return 0;
    }
};

struct Vector {
    float x_, y_, z_;
    int id;
    Vector() : x_(0), y_(0), z_(0), id(vector_count++) { event("vector_ctor", {id}); }
    Vector(const Vector &v) : x_(v.x_), y_(v.y_), z_(v.z_), id(vector_count++) {
        event("vector_copy", {id, v.id});
    }
    Vector &operator=(const Vector &v) { x_=v.x_; y_=v.y_; z_=v.z_; return *this; }
    ~Vector() {
        require(z_ == 0); // All caller-owned z components retain constructor zero.
        event("vector_dtor", {id, static_cast<int>(x_), static_cast<int>(y_)});
    }
    Vector operator-(Vector &v) {
        event("subtract", {id, v.id});
        Vector result; result.x_=x_-v.x_; result.y_=y_-v.y_; result.z_=z_-v.z_;
        return result;
    }
    Vector scale(float scalar) {
        event("scale", {id}); x_*=scalar; y_*=scalar; z_*=scalar; return *this;
    }
    Vector &operator+=(Vector &v) {
        event("add", {id, v.id}); x_+=v.x_; y_+=v.y_; z_+=v.z_; return *this;
    }
    Vector &operator-=(Vector &v) {
        event("undo", {id, v.id}); x_-=v.x_; y_-=v.y_; z_-=v.z_; return *this;
    }
};

struct MapWin : Buffer {
    int field_1DD74_ = 1;
    int field_1DDA4_ = 0, field_1DDA8_ = 0;
    int field_1DDD4_ = 0, field_1DDD8_ = 0;
    int field_1DDCC_ = 100, field_1DDD0_ = 100;
    uint32_t field_1DDB4_ = 12, field_1DDB8_ = 12, field_1DDC0_ = 2;
    int field_1DD98_ = 7;
    int tile_to_pixel(int x, int y, int *px, int *py) {
        event("tile", {view_id,x,y}); *px=x*6; *py=y*6+2;
        ++tile_calls;
        if (replace_slots && tile_calls <= 2) replace_slot(tile_calls);
        return 0;
    }
    int compute_clip(int x, int y, int flags, RECT *r) {
        event("clip", {view_id,x,y,flags});
        *r = {x*6-2,y*6-2,x*6+10,y*6+10};
        ++clip_calls;
        if (replace_slots && clip_calls <= 2) replace_slot(clip_calls+2);
        return 0;
    }
    int update(RECT *r, int flags) {
        event("update", {view_id,r->left,r->top,r->right,r->bottom,flags});
        if (replace_slots) replace_slot(7);
        if (throwing == 3) throw 77;
        return 0;
    }
    void soft_update(RECT *r) {
        event("soft", {view_id,r->left,r->top,r->right,r->bottom});
    }
};
MapWin views[8];
MapWin *MapWinTable[8] = {};
void replace_slot(int index) { MapWinTable[0] = &views[index]; }
struct World {
    uint32_t viewport_outline_enabled_ = 0;
    void set_detail_map() { event("detail"); }
} WorldClimateWorldWin;
struct Wave {
    void play() { event("play"); if (throwing == 1) throw 77; }
} g_SCOOT_WAVE;
void draw_tile(int x, int y, int flags) { event("draw_tile", {x,y,flags}); }
int UnionRect(RECT *out, const RECT *a, const RECT *b) {
    RECT result = {std::min(a->left,b->left),std::min(a->top,b->top),
                   std::max(a->right,b->right),std::max(a->bottom,b->bottom)};
    *out=result; event("union", {out->left,out->top,out->right,out->bottom}); return 1;
}
int IntersectRect(RECT *out, const RECT *a, const RECT *b) {
    event("intersect", {a->left,a->top,a->right,a->bottom,b->left,b->top,b->right,b->bottom});
    ++intersect_calls;
    *out={std::max(a->left,b->left),std::max(a->top,b->top),
          std::min(a->right,b->right),std::min(a->bottom,b->bottom)};
    if (intersect_calls == reject_intersection) return 0;
    return out->left<out->right && out->top<out->bottom;
}
int vehdraw_turn_start(int id,int direction,int speed,int flags) {
    event("turn_start", {id,direction,speed,flags}); return turn_frames;
}
void vehdraw_scoot_start(int id,int direction,int frames,int flags) {
    event("scoot_start", {id,direction,frames,flags});
}
int veh_draw(Buffer *target,int id,int x,int y,int zoom,int flags,int frame) {
    event("draw", {target->view_id,id,x,y,zoom,flags,frame,VehDrawPositionX,VehDrawPositionY});
    if (replace_slots && flags==0x12) replace_slot(6);
    return 0;
}
DWORD timeGetTime() {
    DWORD result;
    if (time_index < static_cast<int>(times.size())) result=times[time_index++];
    else { result=clock_value; clock_value+=16; }
    event("time"); return result;
}
void do_non_input() { event("pump"); }
void do_all_draws() { event("all_draws"); }
void flush_input() { event("flush", {VehDrawPositionX,VehDrawPositionY,VehDrawPositionVehicle}); }
ACTUAL

std::vector<Event> select(const char *name) {
    std::vector<Event> result;
    for (const Event &e:events) if(e.kind==name) result.push_back(e);
    return result;
}
void equal(const Event &e,std::initializer_list<int> values) {
    require(e.a==std::vector<int>(values));
}
void lifetime(bool completed) {
    require(buffer_count==8 && vector_count>=17);
    auto bd=select("buffer_dtor"),vd=select("vector_dtor");
    require(bd.size()==8 && vd.size()==static_cast<size_t>(vector_count));
    for(int i=0;i<8;++i) equal(bd[i],{7-i});
    std::vector<int> seen(vector_count,0),base;
    for(const auto&e:vd) { require(e.a[0]>=0 && e.a[0]<vector_count); ++seen[e.a[0]];
        if(e.a[0]<17) base.push_back(e.a[0]); }
    for(int count:seen) require(count==1);
    for(int i=0;i<17;++i) require(base[i]==16-i);
    // All persistent Vectors finish before the first saved Buffer destructor.
    bool buffers=false;
    for(const auto&e:events) { if(e.kind=="buffer_dtor") buffers=true;
        if(e.kind=="vector_dtor") require(!buffers); }
    if(completed) { auto flush=select("flush");require(flush.size()==1);equal(flush[0],{-1,-1,0}); }
    else require(select("flush").empty());
}
void run(int flags=0,int x=2,int y=2,int direction=2) {
    track=true;veh_scoot(0,x,y,direction,flags);track=false;lifetime(true);
}
void geometry() {
    run();
    auto inits=select("init"),captures=select("capture"),restores=select("restore");
    require(inits.size()==1 && captures.size()==1 && restores.size()==4);
    equal(inits[0],{0,26,26,0,0}); equal(captures[0],{0,0,6,6,0,0,26,26});
    equal(restores[0],{0,0,2,2,8,8,26,26});
    equal(restores[1],{0,0,4,4,10,10,26,26});
    equal(restores[2],{0,0,6,6,12,12,26,26});
    equal(restores[3],{0,0,8,8,14,14,26,26});
    auto draws=select("draw");require(draws.size()==5);
    equal(draws[0],{0,0,12,12,7,0x12,0,2,2});
    equal(draws[1],{0,0,14,14,7,0x22,0,2,2});
    equal(draws[2],{0,0,16,16,7,0x22,1,2,2});
    equal(draws[3],{0,0,18,18,7,0x22,2,2,2});
    equal(draws[4],{0,0,18,18,7,2,1,3,3});
    for(const auto&e:select("update")) equal(e,{0,6,6,32,32,0});
    auto soft=select("soft");require(soft.size()==1);equal(soft[0],{0,6,6,32,32});
    auto undo=select("undo");require(undo.size()==8);
    for(int i=0;i<8;++i)equal(undo[i],{i,i+9});
    require(select("subtract").size()==1 && select("scale").size()==1);
    equal(select("subtract")[0],{8,0}); equal(select("scale")[0],{9});
    const std::vector<std::string> relevant = {
        "capture","turn_start","draw","update","restore","scoot_start",
        "soft","all_draws","flush"};
    std::vector<std::string> order;
    for(const auto&e:events)
        if(std::find(relevant.begin(),relevant.end(),e.kind)!=relevant.end())order.push_back(e.kind);
    require(order==std::vector<std::string>({
        "capture","turn_start","draw","update","restore","scoot_start",
        "draw","update","restore","draw","update","restore",
        "draw","update","restore","draw","soft","all_draws","flush"}));
    // Completed return temporaries must already be gone before Buffer::init.
    int temporaries=0;
    for(const auto&e:events) { if(e.kind=="init")break;
        if(e.kind=="vector_dtor" && e.a[0]>=17)++temporaries; }
    require(temporaries>=2);
}
void flags_case(bool immediate) {
    run(immediate ? 0x4001 : 1);
    require(select("scoot_start").empty());
    auto draws=select("draw");require(draws.size()==1);
    if(immediate) {
        require(select("turn_start").empty() && select("init").empty() && select("pump").empty());
        require(select("capture").empty() && select("restore").empty());
        equal(draws[0],{0,0,18,18,7,2,1,3,3});require(Vehs[0].facing==2);
        require(select("soft").size()==1);
    } else {
        require(select("turn_start").size()==1 && select("soft").empty());
        equal(draws[0],{0,0,12,12,7,0x12,0,2,2});
    }
}
void wrapping(bool flat) {
    MapIsFlat=flat;run(0x4000,0,2,6);
    auto tiles=select("draw_tile");require(tiles.size()==2);
    equal(tiles[0],{0,2,0});equal(tiles[1],{flat?-1:99,1,0});
    auto draws=select("draw");require(draws.size()==1);
    require(draws[0].a[7]==(flat?-1:99) && draws[0].a[8]==1);
}
void retained_origin() {
    run(0x4000,2,2,8);auto draws=select("draw");require(draws.size()==1);
    require(draws[0].a[7]==2 && draws[0].a[8]==2 && Vehs[0].facing==0);
    equal(select("draw_tile")[1],{3,1,0});
}
void callbacks() {
    replace_slots=true;run();
    auto tiles=select("tile"),clips=select("clip");
    equal(tiles[0],{0,2,2});equal(tiles[1],{1,3,3});
    equal(clips[0],{2,3,3,0});equal(clips[1],{3,2,2,0});
    require(select("capture")[0].a[0]==5);
    require(select("draw")[0].a[0]==5);
    require(select("update")[0].a[0]==6);
    require(select("restore")[0].a[1]==7);
    require(select("draw").back().a[0]==7);
}
void preference() {
    change_preference=true;run();
    equal(select("turn_start")[0],{0,2,25,2});
    equal(select("scoot_start")[0],{0,2,3,2});
}
void network(bool foreign) {
    IsMultiplayerNet=1;Vehs[0].faction_id=foreign?2:1;run();
    equal(select("scoot_start")[0],{0,2,4,2});require(Vehs[0].facing==2);
    require(select("detail").size()==(foreign?1u:0u));
    require(WorldClimateWorldWin.viewport_outline_enabled_==(foreign?1u:0u));
    if(!foreign) require(select("pump").size()==5); // even zero waits pump once
}
void rejection() {
    turn_frames=0;reject_intersection=1;run();
    auto intersections=select("intersect");require(intersections.size()==3);
    require(intersections[0].a[0]==14 && intersections[1].a[0]==16 && intersections[2].a[0]==18);
    auto draws=select("draw");require(draws.size()==3);
    require(draws[0].a[2]==16 && draws[0].a[6]==1);
    require(select("restore").size()==2 && select("undo").size()==8);
}
void clock_wrap() {
    times={0xFFFFFFFCu,0xFFFFFFFDu,3u,4u,20u,36u,52u};run();
    require(time_index==7 && select("pump").size()==6);
}
void no_windows() {
    MapWinTable[0]=nullptr;run();
    require(select("tile").empty() && select("draw").empty() && select("capture").empty());
    require(select("scoot_start").size()==1 && select("undo").size()==8);
}
void multiple_windows() {
    MapWinTable[1]=&views[1];views[1].field_1DD74_=0;
    MapWinTable[2]=&views[2];
    // Primary view remains eligible regardless of its own secondary enable flag.
    views[0].field_1DD74_=0;
    run();
    auto inits=select("init"),captures=select("capture");
    require(inits.size()==2 && captures.size()==2 && select("draw").size()==10);
    require(inits[0].a[0]==0 && inits[1].a[0]==2);
    equal(captures[1],{2,2,6,6,0,0,26,26});
    for(const auto&e:select("tile"))require(e.a[0]!=1);
    require(select("undo").size()==8);
}
void exception_case(int phase) {
    throwing=phase;track=true;bool caught=false;
    try { veh_scoot(0,2,2,2,0); } catch(int value) { require(value==77);caught=true; }
    track=false;require(caught);lifetime(false);
    require(VehDrawPositionX==2 && VehDrawPositionY==2 && VehDrawPositionVehicle==0);
    if(phase==1) require(select("draw_tile").empty());
    if(phase==2) require(select("capture").empty());
    if(phase==3) require(select("restore").empty());
}
int main(int argc,char **argv) {
    require(argc==2);
    for(int i=0;i<8;++i)views[i].view_id=i;
    MapWinTable[0]=&views[0];
    std::string s=argv[1];
    if(s=="geometry")geometry();
    else if(s=="flag1")flags_case(false);
    else if(s=="immediate")flags_case(true);
    else if(s=="wrap")wrapping(false);
    else if(s=="flat")wrapping(true);
    else if(s=="retain")retained_origin();
    else if(s=="callbacks")callbacks();
    else if(s=="preference")preference();
    else if(s=="local_net")network(false);
    else if(s=="foreign_net")network(true);
    else if(s=="reject")rejection();
    else if(s=="clock")clock_wrap();
    else if(s=="null")no_windows();
    else if(s=="multiple")multiple_windows();
    else if(s=="throw_play")exception_case(1);
    else if(s=="throw_init")exception_case(2);
    else if(s=="throw_update")exception_case(3);
    else return 43;
}
'''


def compile_host(tmp_path, source):
    compiler = shutil.which("g++")
    if not compiler:
        pytest.skip("host g++ required")
    unit = tmp_path / "vehicle_scoot.cpp"
    executable = tmp_path / "vehicle_scoot"
    unit.write_text(HARNESS.replace("ACTUAL", source))
    result = subprocess.run(
        [compiler, "-std=c++11", "-O1", "-fsanitize=undefined",
         "-fno-sanitize-recover=all", str(unit), "-o", str(executable)],
        capture_output=True, text=True, timeout=30)
    assert result.returncode == 0, result.stderr
    return executable


@pytest.fixture(scope="module")
def actual_executable(tmp_path_factory):
    return compile_host(tmp_path_factory.mktemp("vehicle-scoot"), actual_source())


@pytest.mark.parametrize("scenario", [
    "geometry", "flag1", "immediate", "wrap", "flat", "retain", "callbacks",
    "preference", "local_net", "foreign_net", "reject", "clock", "null",
    "multiple", "throw_play", "throw_init", "throw_update",
])
def test_actual_vehicle_scoot(actual_executable, scenario):
    result = subprocess.run([str(actual_executable), scenario], capture_output=True,
                            text=True, timeout=10)
    assert result.returncode == 0, result.stderr


@pytest.mark.parametrize("before,after,scenario", [
    ("const bool immediate = (flags & 0x4000) != 0;",
     "const bool immediate = (flags & 1) != 0;", "flag1"),
    ("if (flags & 1) {", "if (false) {", "flag1"),
    ("retain_origin = true;", "retain_origin = false;", "retain"),
    ("if (!(MapIsFlat & 1)) {", "if (false) {", "wrap"),
    ("MapWinTable[i]->tile_to_pixel(next_x, next_y, &end_x[i], &end_y[i]);",
     "window->tile_to_pixel(next_x, next_y, &end_x[i], &end_y[i]);", "callbacks"),
    ("margin_x[i] = (area[i].right - area[i].left) / 4;",
     "margin_x[i] = (area[i].right - area[i].left) / 2;", "geometry"),
    ("start_x[i] - margin_x[i] - area[i].left,",
     "start_x[i] - area[i].left,", "geometry"),
    ("MapWinTable[i]->update(&area[i], 0);",
     "MapWinTable[i]->update(&intersection, 0);", "geometry"),
    ("increment[i].scale(1.0f / static_cast<float>(move_frames));",
     "increment[i].scale(1.0f / static_cast<float>(move_frames + 1));", "geometry"),
    ("position[i] += increment[i];", "/* missing accumulation */", "reject"),
    ("for (int i = 0; i < 8; ++i) position[i] -= increment[i];",
     "for (int i = 0; i < 8; ++i) if (active[i]) position[i] -= increment[i];", "geometry"),
    ("move_frames = 4;", "move_frames = 3;", "local_net"),
    ("MapWinTable[final_slot]->soft_update(&area[final_slot]);",
     "/* missing final update */", "geometry"),
    ("static_cast<Buffer *>(MapWinTable[i])->copy(&saved[i],",
     "saved[i].copy(static_cast<Buffer *>(MapWinTable[i]),", "geometry"),
    ("saved[i].copy(static_cast<Buffer *>(MapWinTable[i]),",
     "static_cast<Buffer *>(MapWinTable[i])->copy(&saved[i],", "geometry"),
    ("if (i != 0 && !window->field_1DD74_) continue;",
     "if (!window->field_1DD74_) continue;", "multiple"),
    ("Buffer saved[8];\n    Vector position[8];\n    Vector destination;\n    Vector increment[8];",
     "Vector position[8];\n    Vector destination;\n    Vector increment[8];\n    Buffer saved[8];", "throw_play"),
])
def test_damage_compiles_but_fails(tmp_path, before, after, scenario):
    source = actual_source()
    assert before in source
    executable = compile_host(tmp_path, source.replace(before, after))
    result = subprocess.run([str(executable), scenario], capture_output=True,
                            text=True, timeout=10)
    assert result.returncode == 42, result.stderr
