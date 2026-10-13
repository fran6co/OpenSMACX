"""Actual destination routing/render dispatch with original-derived controls.

Complete source bodies/helpers and selected real declarations are extracted,
without rewriting the implementation. Native geometry, object layout, Path,
cursor pumping, hex costs, map storage lifecycle and rasterization are separate
dependencies: the recording services below do not prove those implementations.
In particular the nine-argument Buffer::line remains explicitly unfinished;
testing its call does not make patterned rendering operational. Fixtures use
valid indexed cells/factions/prototypes/directions and initialized viewports.
Callback mutations test observable load ordering, not real-service reachability.
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
    headers = {name: (ROOT / "src" / (name + ".h")).read_text() for name in
               ("mapwin", "map", "veh", "base", "alpha", "faction", "game", "time", "path", "win", "buffer", "graphicwin")}
    cpp = (ROOT / "src/mapwin.cpp").read_text()
    parts = {}
    parts["STRUCTURES"] = "\n".join(braced(headers[unit], "struct " + name + " {") + ";" for unit, name in
        (("map", "Map"), ("veh", "VehPrototype"), ("veh", "RulesWeapon"), ("base", "Base"), ("base", "BaseSecretProject")))
    parts["RULE_FIELD"], = re.findall(r"^    uint32_t move_rate_roads;", headers["alpha"], re.M)
    # Only the accessed PlayerData field is projected; no claim about its host stride.
    player = braced(headers["faction"], "struct PlayerData {")
    parts["PLAYER_FIELD"], = re.findall(r"^    uint32_t flags;", player, re.M)
    parts["FIELDS"] = "\n".join(re.findall(r"^  (?:int|uint32_t) field_" + suffix + r"_;", headers["mapwin"], re.M)[0]
        for suffix in ("1DDA4", "1DDA8", "1DDCC", "1DDD0", "1DDD4", "1DDD8", "1DDBC", "1DDC0", "1DD84"))
    parts["MAP_METHODS"] = "\n".join(re.findall(r"^  (?:void|int) (?:direct_dest_line|dest_line|tile_to_pixel)\([^;{}]*\);", headers["mapwin"], re.M))
    parts["BUFFER_METHODS"] = "\n".join(re.findall(r"^  void line\([^;{}]*\);", headers["buffer"], re.M))
    parts["WIN_METHODS"] = "\n".join(re.findall(r"^  (?:static )?(?:void|int) (?:update_cursor\(Win \*window, int tgl|screen_to_client\(int \*x, int \*y)\);", headers["win"], re.M))
    parts["PATH_METHOD"], = re.findall(r"^  int find\([^;{}]*\);", headers["path"], re.M)
    for unit, cls in (("win", "Win"), ("graphicwin", "GraphicWin"), ("mapwin", "MapWin")):
        parts[cls.upper() + "_BASES"], = re.findall(r"^class " + cls + r"\s*:[^{]+\{", headers[unit], re.M)
    globals_ = []
    for unit, names in (("map", ("MapIsFlat", "MapLongitude", "MapLongitudeBounds")),
                        ("mapwin", ("MapDestinationCost",)), ("time", ("PlanLineCounter",))):
        for name in names:
            decl, = re.findall(r"^extern (?:int|uint32_t|BOOL) " + name + r";", headers[unit], re.M)
            globals_.append(decl.removeprefix("extern "))
    parts["GLOBALS"] = "\n".join(globals_)
    parts["CALLEES"] = "\n".join(re.findall(r"^(?:int|BOOL) __cdecl (?:base_at|base_on_sea|compass_move|hex_cost|speed_proto)\([^;{}]*\);", "\n".join(headers.values()), re.M))
    # port_to_port is an existing inline implementation; isolate it as a service.
    parts["CALLEES"] += "\n" + re.search(r"BOOL __cdecl port_to_port\([^{}]*\)", headers["map"]).group() + ";"
    parts["MAP_LOC"] = braced(headers["map"], "inline Map *__cdecl map_loc(")
    veh = (ROOT / "src/veh.cpp").read_text()
    parts["DIRECTIONS"] = "\n".join(re.findall(r"^int VehMoveDelta[XY]\[9\] = \{[^}]+\};", veh, re.M))
    parts["BODY"] = "\n".join(braced(cpp, signature) for signature in (
        "int map_dest_signed(", "int mapwin_dest_add(int left, int right) {",
        "int mapwin_dest_sub(int left, int right) {", "int mapwin_dest_mul(int left, int right) {",
        "int map_dest_abs(", "int map_dest_half(", "int map_dest_distance(", "bool map_dest_in_view(",
        "void MapWin::direct_dest_line(", "void MapWin::dest_line("))
    assert all(parts.values())
    return parts


HARNESS = r'''
#include <cstdint>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>
#include <array>
#include <functional>
#include <algorithm>
#include <limits>
#define __cdecl
typedef int BOOL;typedef char *LPSTR;
struct AutoSound {virtual ~AutoSound(){}};
STRUCTURES
struct RulesProjection {RULE_FIELD} Rules;
struct GameDataStateFieldsProjection { uint32_t game_rules_, game_state_; BaseSecretProject secret_project_; int base_current_count_, vehicle_current_count_; };
struct GameDataStateProjection { GameDataStateFieldsProjection fields_; } GameDataStateStorage;
struct PlayerProjection {PLAYER_FIELD} PlayersData[8];
WIN_BASES public: WIN_METHODS};
struct Buffer {BUFFER_METHODS};
GRAPHICWIN_BASES public:};
MAPWIN_BASES public:FIELDS MAP_METHODS};
struct Path {PATH_METHOD};
GLOBALS
DIRECTIONS
CALLEES
MapWin window;GraphicWin CoverWindow;Path Paths;
Map map_a[1024],map_b[1024],*active_map=map_a;
Map *map_tiles(){return active_map;}
MAP_LOC
Base Bases[4];BaseSecretProject SecretProject;VehPrototype VehPrototypes[16];RulesWeapon Weapon[26];
struct Event {char kind;std::vector<int> args;int cost;};
std::vector<Event> events;
std::array<int,256> counts;
std::function<void(char,int)> hook;
std::vector<int> route_answers,base_answers;
int scenario,route_index,base_index,connected,speed_result,cost_result,direction,screen_dx,screen_dy;
void check(bool ok,int line){if(!ok){std::fprintf(stderr,"assertion line %d scenario %d\n",line,scenario);std::exit(93);}}
#define require(x) check((x),__LINE__)
void note(char kind,std::initializer_list<int> args={}){
    events.push_back({kind,args,MapDestinationCost});int count=++counts[static_cast<unsigned char>(kind)];
    if(hook)hook(kind,count);
}
int Win::update_cursor(Win *w,int flag){require(w==nullptr&&flag==1);note('U');return 0;}
void MapWin::tile_to_pixel(int x,int y,int *px,int *py){require(this==&window);note('T',{x,y});*px=x*10;*py=y*10;}
void Win::screen_to_client(int *x,int *y){require(this==static_cast<Win*>(&window));note('S',{*x,*y});*x+=screen_dx;*y+=screen_dy;}
void Buffer::line(int x,int y,int xx,int yy,int color){require(this==static_cast<Buffer*>(&CoverWindow));note('L',{x,y,xx,yy,color});}
void Buffer::line(int x,int y,int xx,int yy,int a,int color,int on,int off,int phase){require(this==static_cast<Buffer*>(&window));note('D',{x,y,xx,yy,a,color,on,off,phase});}
int Path::find(int x,int y,int xx,int yy,int proto,int faction,int flags,int last){
    require(this==&Paths);note('F',{x,y,xx,yy,proto,faction,flags,last});
    return route_index<static_cast<int>(route_answers.size())?route_answers[route_index++]:direction;
}
int compass_move(int x,int y,int xx,int yy){note('C',{x,y,xx,yy});return direction;}
int base_at(int x,int y){note('B',{x,y});return base_index<static_cast<int>(base_answers.size())?base_answers[base_index++]:-1;}
BOOL port_to_port(int a,int b){note('P',{a,b});return connected;}
BOOL base_on_sea(int a,int region){note('O',{a,region});return connected;}
int speed_proto(int proto){note('V',{proto});return speed_result;}
int hex_cost(int proto,int faction,int x,int y,int xx,int yy,BOOL toggle){note('H',{proto,faction,x,y,xx,yy,toggle});return cost_result;}
BODY
std::vector<Event> only(char kind){std::vector<Event> out;for(auto &e:events)if(e.kind==kind)out.push_back(e);return out;}
void args(const Event &event,std::initializer_list<int> expected){require(event.args==std::vector<int>(expected));}
void reset(int id=0){
    scenario=id;events.clear();counts.fill(0);hook={};route_answers.clear();base_answers.clear();route_index=base_index=0;
    connected=1;speed_result=3;cost_result=2;direction=1;screen_dx=-7;screen_dy=-11;
    std::memset(map_a,0,sizeof map_a);std::memset(map_b,0,sizeof map_b);active_map=map_a;
    for(auto &tile:map_a){tile.region=4;tile.visibility=8;}for(auto &tile:map_b){tile.region=9;tile.visibility=8;}
    std::memset(Bases,0,sizeof Bases);std::memset(&GameDataStateStorage.fields_.secret_project_,0,sizeof GameDataStateStorage.fields_.secret_project_);
    std::memset(VehPrototypes,0,sizeof VehPrototypes);std::memset(Weapon,0,sizeof Weapon);std::memset(PlayersData,0,sizeof PlayersData);
    MapIsFlat=1;MapLongitude=10;MapLongitudeBounds=20;GameDataStateStorage.fields_.game_rules_=GameDataStateStorage.fields_.game_state_=0;MapDestinationCost=77;PlanLineCounter=6;
    Rules.move_rate_roads=3;GameDataStateStorage.fields_.secret_project_.xenoempathy_dome()=-1;Weapon[0].offense_rating=1;
    window.field_1DDA4_=window.field_1DDA8_=0;window.field_1DDCC_=window.field_1DDD0_=20;
    window.field_1DDD4_=window.field_1DDD8_=0;window.field_1DDBC_=3;window.field_1DDC0_=5;window.field_1DD84_=0;
}
void route(int triad=0,int repeat=0,int cover=1){window.dest_line(2,4,8,4,1,3,triad,repeat,3,5,cover);}
void basic_two_pass(){
    reset(1);route();require(counts['F']==4&&counts['C']==2&&counts['H']==3&&counts['L']==3&&counts['U']==11&&MapDestinationCost==6);
    auto paths=only('F');args(paths[0],{2,4,8,4,1,3,0xc0,-1});args(paths[1],{4,4,8,4,1,3,0xc0,1});args(paths[2],{2,4,8,4,1,3,0xc0,-1});
    auto costs=only('H');args(costs[0],{1,3,2,4,4,4,1});args(costs[1],{1,3,4,4,6,4,1});args(costs[2],{1,3,6,4,8,4,1});
    auto lines=only('L');args(lines[0],{16,34,36,34,250});args(lines[1],{36,34,56,34,250});args(lines[2],{56,34,76,34,250});
    require(lines[0].cost==2&&lines[1].cost==4&&lines[2].cost==6);
    int f=0,c=0;for(auto &e:events){if(e.kind=='F')++f;if(e.kind=='C')++c;if(e.kind=='H')break;}require(f==3&&c==1);
    reset(2);hook=[](char k,int n){if(k=='U'&&n==1)MapDestinationCost=999;if(k=='U'&&n==2)require(MapDestinationCost==0);};route();
    reset(3);hook=[](char k,int){if(k=='H')MapDestinationCost=100;if(k=='L')MapDestinationCost+=1000;};route();
    require(MapDestinationCost==1102);for(auto&e:only('L'))require(e.cost==102);
    reset(4);speed_result=7;hook=[](char k,int){if(k=='V')Rules.move_rate_roads=7;};route();for(auto&e:only('H'))require(e.args[6]==1);
}
void connectivity(){
    for(int triad: {0,1,2,3})for(int same=0;same<2;++same){
        reset(10+triad*2+same);if(!same)map_loc(8,4)->region=7;route(triad);
        const bool direct=triad==2||((triad==0||triad==1)&&!same);
        require(counts['H']==(direct?0:3)&&counts['L']==(direct?1:3)&&MapDestinationCost==(direct?9:6));
    }
    for(int a: {-1,0})for(int b:{-1,1})for(int yes=0;yes<2;++yes){
        reset(20);base_answers={a,b};connected=yes;map_loc(8,4)->region=7;
        route(1);require(counts['B']==2);const bool direct=(a<0&&b<0)||!yes;
        require(counts['H']==(direct?0:3));
        if(a>=0&&b>=0){require(counts['P']==1);args(only('P')[0],{0,1});}
        else if(a>=0){require(counts['O']==1);args(only('O')[0],{0,7});}
        else if(b>=0){require(counts['O']==1);args(only('O')[0],{1,4});}
    }
    reset(21);base_answers={0,-1};hook=[](char k,int n){if(k=='B'&&n==2){active_map=map_b;MapLongitude=12;map_b[4+4*12].region=13;}};
    route(1);args(only('O')[0],{0,13});require(counts['H']==3);
    reset(22);base_answers={-1,-1};hook=[](char k,int n){if(k=='B'&&n==2){active_map=map_b;MapLongitude=12;map_b[4+4*12].region=13;}};
    route(1);require(counts['H']==0&&MapDestinationCost==9);
}
void flags_and_failures(){
    for(int repeat=0;repeat<2;++repeat)for(int offense:{-1,0,1})for(int ownership=0;ownership<3;++ownership){
        reset(30);Weapon[0].offense_rating=offense;GameDataStateStorage.fields_.secret_project_.xenoempathy_dome()=ownership?1:-1;Bases[1].faction_id_current=ownership==2?3:4;
        route(0,repeat);for(auto&e:only('F'))require(e.args[6]==((repeat?0x80:0xc0)|(offense==0&&ownership!=2?0x10:0)));
    }
    reset(31);hook=[](char k,int n){if(k=='U'&&n==2){VehPrototypes[1].weapon_id=2;Weapon[2].offense_rating=0;}
        if(k=='F'&&n==1){GameDataStateStorage.fields_.secret_project_.xenoempathy_dome()=1;Bases[1].faction_id_current=3;}};
    route();require(only('F')[0].args[6]==0xd0&&only('F')[1].args[6]==0xc0);
    reset(32);route_answers={-1};route();require(counts['F']==1&&counts['C']==0&&counts['H']==0&&counts['L']==1&&MapDestinationCost==9);
    reset(33);route_answers={1,1,-1};route();require(counts['F']==3&&counts['C']==1&&counts['H']==0&&counts['L']==1&&MapDestinationCost==9);
    reset(34);route_answers={1,1,1,-1};route();require(counts['H']==1&&counts['L']==2&&MapDestinationCost==9);
    reset(35);direction=8;MapLongitudeBounds=2;route();require(counts['F']==5&&counts['H']==0&&counts['L']==1&&MapDestinationCost==9);
    // Zero is a valid returned direction (NE), not a failed path.
    reset(36);direction=0;window.dest_line(2,8,5,5,1,3,0,0,3,5,1);require(counts['H']==3&&counts['L']==3&&MapDestinationCost==6);
}
void fog_and_live_map(){
    for(int exception=0;exception<5;++exception){
        reset(40+exception);map_loc(4,4)->visibility=0;map_loc(4,4)->val2=8;
        if(exception==1)PlayersData[3].flags=0x200;if(exception==2)GameDataStateStorage.fields_.game_state_=0x80;if(exception==3)GameDataStateStorage.fields_.game_rules_=0x100;
        if(exception==4)map_loc(4,4)->visibility=8;
        route();require(counts['H']==(exception?3:0)&&counts['L']==(exception?3:1));
    }
    reset(45);map_loc(8,4)->visibility=0;route();require(counts['H']==3&&counts['L']==3);
    reset(46);hook=[](char k,int n){if(k=='F'&&n==1){active_map=map_b;MapLongitude=12;map_b[2+4*12].visibility=0;}};
    route();require(counts['H']==0&&counts['L']==1);
    reset(47);hook=[](char k,int n){if(k=='L'&&n==1){active_map=map_b;MapLongitude=12;map_b[2+4*12].visibility=0;}};
    route();require(counts['H']==1&&counts['L']==2&&MapDestinationCost==9);
}
void viewport_and_wrap(){
    reset(50);window.field_1DDA8_=5;route();require(counts['L']==0&&counts['H']==3&&MapDestinationCost==6);
    reset(51);window.field_1DDA8_=0;window.field_1DDD0_=4;route();require(counts['L']==0);
    reset(52);window.field_1DDA4_=4;window.field_1DDCC_=2;route();require(counts['L']==2);
    reset(53);window.field_1DDA4_=4;window.field_1DDCC_=0;window.field_1DDD4_=2;route();require(counts['L']==2);
    reset(54);window.field_1DDD0_=0;window.field_1DDD8_=5;route();require(counts['L']==3);
    reset(55);MapIsFlat=0;window.field_1DDA4_=20;window.field_1DDCC_=10;route();require(counts['L']==3);args(only('T')[0],{2,4});
    reset(56);MapIsFlat=0;window.field_1DDA4_=-20;window.field_1DDCC_=10;route();require(counts['L']==3);args(only('T')[0],{2,4});
    reset(57);MapIsFlat=0;window.dest_line(18,4,2,4,1,3,0,0,3,5,1);
    require(counts['H']==2&&MapDestinationCost==4);args(only('H')[0],{1,3,18,4,0,4,1});args(only('H')[1],{1,3,0,4,2,4,1});
    reset(58);MapIsFlat=0;direction=5;window.dest_line(0,4,16,4,1,3,0,0,3,5,1);
    require(counts['H']==2);args(only('H')[0],{1,3,0,4,18,4,1});
    reset(59);hook=[](char k,int n){if(k=='H'&&n==1)window.field_1DDA8_=5;if(k=='H'&&n==2)window.field_1DDA8_=0;};route();require(counts['L']==2);
    reset(60);MapIsFlat=0;window.dest_line(18,4,2,4,1,3,2,0,3,5,1);require(MapDestinationCost==6);
    reset(61);window.dest_line(2,4,7,4,1,3,2,0,3,5,1);require(MapDestinationCost==6);
}
void direct_dispatch(){
    for(int cover=0;cover<2;++cover)for(int triad:{0,1,2,9})for(int forced=0;forced<2;++forced){
        reset(70+triad);window.field_1DD84_=forced;window.direct_dest_line(2,4,8,6,triad,3,5,cover);
        int color=forced?251:(triad==0?250:triad==1?254:triad==2?249:4);
        require(counts['U']==1&&counts['T']==2&&counts['S']==(cover?2:0)&&counts['H']==0);
        if(cover)args(only('L')[0],{16,34,76,54,color});
        else {int dx=triad==1?1:0,dy=triad==1?1:triad==2?-1:0;args(only('D')[0],{23+dx,45+dy,83+dx,65+dy,0,color,3,5,6});}
    }
    reset(80);hook=[](char k,int n){if(k=='T'&&n==1){window.field_1DDBC_=30;window.field_1DDC0_=40;}
        if(k=='T'&&n==2){window.field_1DDBC_=50;window.field_1DDC0_=60;window.field_1DD84_=1;}
        if(k=='S'&&n==1){window.field_1DD84_=0;window.field_1DDBC_=90;}
        if(k=='S'&&n==2){screen_dx=-17;screen_dy=-21;}};
    window.direct_dest_line(2,4,8,6,0,3,5,1);args(only('L')[0],{63,89,113,99,251});
    reset(81);hook=[](char k,int n){if(k=='T'&&n==2)PlanLineCounter=13;};window.direct_dest_line(2,4,8,6,0,2,4,0);
    require(only('D')[0].args[8]==5);
    reset(82);hook=[](char k,int){if(k=='U')require(MapDestinationCost==9);if(k=='L')MapDestinationCost=123;};route(2);require(MapDestinationCost==123);
    reset(83);window.field_1DDBC_=0x7fffffffu;window.field_1DDC0_=0xffffffffu;
    window.direct_dest_line(1,2,3,4,0,3,5,0);args(only('D')[0],{-2147483639,19,-2147483619,39,0,250,3,5,6});
    require(map_dest_half(-3)==-2&&map_dest_abs(std::numeric_limits<int>::min())==std::numeric_limits<int>::min());
}
int main(){basic_two_pass();connectivity();flags_and_failures();fog_and_live_map();viewport_and_wrap();direct_dispatch();}
'''


def run_host(tmp_path, parts):
    compiler = shutil.which("clang++") or shutil.which("g++")
    if not compiler:
        pytest.skip("host C++ compiler required")
    source = HARNESS
    for key in sorted(parts, key=len, reverse=True):
        source = source.replace(key, parts[key])
    cpp = tmp_path / "routes.cpp"
    binary = tmp_path / "routes"
    cpp.write_text(source)
    compiled = subprocess.run([compiler, "-std=c++11", "-O1", "-fsanitize=address,undefined",
                               "-fno-sanitize-recover=all", str(cpp), "-o", str(binary)],
                              capture_output=True, text=True, timeout=60)
    assert compiled.returncode == 0, compiled.stderr
    return subprocess.run([str(binary)], capture_output=True, text=True, timeout=10,
                          env=dict(os.environ, ASAN_OPTIONS="detect_leaks=0"))


def test_actual_destination_routes(tmp_path):
    result = run_host(tmp_path, actual_source())
    assert result.returncode == 0, result.stderr


MUTANTS = [
    ("connected-land", "fallback = map_loc(x1, y1)->region != map_loc(x2, y2)->region;", "fallback = map_loc(x1, y1)->region == map_loc(x2, y2)->region;"),
    ("connected-ports", "fallback = port_to_port(base1, base2) == 0;", "fallback = port_to_port(base1, base2) != 0;"),
    ("connected-coast", "fallback = base_on_sea(base1, map_loc(x2, y2)->region) == 0;", "fallback = base_on_sea(base1, map_loc(x2, y2)->region) != 0;"),
    ("air-direct", "} else if (triad == 2) {", "} else if (triad != 2) {"),
    ("two-passes", "pass < 2 && !fallback", "pass < 1 && !fallback"),
    ("seed-pass-vs-step", "if (pass != 0) {", "if (steps != 0) {"),
    ("charge-first-edge", "if (proto_id >= 0) {", "if (proto_id >= 0 && steps != 0) {"),
    ("zero-path-is-valid", "if (direction < 0) {", "if (direction <= 0) {"),
    ("repeat-flags", "repeat != 0 ? 0x80 : 0xc0", "repeat != 0 ? 0xc0 : 0x80"),
    ("project-owner", "Bases[project_base].faction_id_current != faction", "Bases[project_base].faction_id_current == faction"),
    ("weapon-zero", "offense_rating == 0", "offense_rating <= 0"),
    ("last-direction", "last_direction = direction;", "last_direction = -1;"),
    ("live-cost", "MapDestinationCost = mapwin_dest_add(MapDestinationCost, cost);", "MapDestinationCost = cost;"),
    ("arrival-before-fog", "if (next_x == x2 && next_y == y2)", "if (false)"),
    ("visibility-field", "map_loc(next_x, next_y)->visibility", "map_loc(next_x, next_y)->val2"),
    ("visibility-bit", "static_cast<uint32_t>(faction) & 31", "static_cast<uint32_t>(faction + 1) & 31"),
    ("player-visibility", "(PlayersData[faction].flags & 0x200) == 0", "true"),
    ("editor-visibility", "(GameDataStateStorage.fields_.game_state_ & 0x80) == 0", "true"),
    ("rules-visibility", "(GameDataStateStorage.fields_.game_rules_ & 0x100) == 0", "true"),
    ("step-strict-limit", "steps > mapwin_dest_mul(MapLongitudeBounds, 2)", "steps >= mapwin_dest_mul(MapLongitudeBounds, 2)"),
    ("both-endpoints", "map_dest_in_view(*this, current_x, current_y) ||", "map_dest_in_view(*this, current_x, current_y) &&"),
    ("exclusive-bottom", "if (y >= bottom)", "if (y > bottom)"),
    ("exclusive-right", "return x >= left && x < right;", "return x >= left && x <= right;"),
    ("extent-margin", "window.field_1DDD4_,", "0,"),
    ("wrapped-distance", "dx = mapwin_dest_sub(MapLongitudeBounds, dx);", "dx = dx;"),
    ("fallback-distance-half", "map_dest_half(map_dest_distance(x1, y1, x2, y2))", "map_dest_distance(x1, y1, x2, y2)"),
    ("source-y-default-color", "color = y1;", "color = triad;"),
    ("forced-color", "color = 0xfb;", "color = 0xfa;"),
    ("cover-conversions", "Win::screen_to_client(&pixel_x2, &pixel_y2);", ""),
    ("pattern-phase", "static_cast<uint32_t>(PlanLineCounter)", "0U"),
    ("sea-offset", "pixel_x1 = mapwin_dest_add(pixel_x1, 1);", "pixel_x1 = mapwin_dest_sub(pixel_x1, 1);"),
    ("air-offset", "pixel_y1 = mapwin_dest_sub(pixel_y1, 1);", "pixel_y1 = mapwin_dest_add(pixel_y1, 1);"),
]


@pytest.mark.parametrize("name,before,after", MUTANTS, ids=[entry[0] for entry in MUTANTS])
def test_compiling_route_damage(tmp_path, name, before, after):
    parts = actual_source()
    assert before in parts["BODY"], (name, "missing mutation anchor")
    parts["BODY"] = parts["BODY"].replace(before, after)
    result = run_host(tmp_path, parts)
    assert result.returncode == 93 and "assertion line" in result.stderr, (
        name, "expected an explicit behavioral assertion, not a crash or sanitizer failure",
        result.returncode, result.stderr,
    )


def body(signature):
    source = (ROOT / "src/mapwin.cpp").read_text()
    start = source.index(signature)
    return source[start:source.index("\n}", start) + 2]

def execute(tmp_path, harness, candidate, name):
    compiler = shutil.which("g++")
    if not compiler:
        pytest.skip("host C++ compiler unavailable")
    executable = tmp_path / name
    subprocess.run([compiler, "-std=c++17", "-x", "c++", "-", "-o", str(executable)],
                   input=harness.replace("BODY", candidate), text=True,
                   capture_output=True, check=True)
    return subprocess.run([str(executable)], capture_output=True).returncode

ROUTES = r'''
#include <cassert>
#include <cstdint>
struct Base {enum AutoForwardSlot {AutoForwardLand,AutoForwardSea,
    AutoForwardAir,AutoForwardReserved,AutoForwardSlotCount};
    int faction_id_current,x,y;int auto_forward_slots[AutoForwardSlotCount];};
struct Veh {int faction_id,proto_id,x,y,order;uint32_t state;unsigned char waypoint_count;short waypoint_x[4],waypoint_y[4];};
Base Bases[2];Veh Vehs[2];struct GameDataStateFieldsProjection { int base_current_count_, vehicle_current_count_; };struct GameDataStateProjection { GameDataStateFieldsProjection fields_; } GameDataStateStorage;
int MapIsFlat=1,MapLongitudeBounds=100;struct {int faction_id_;}ConsoleGlobal;
const int ORDER_MOVE_TO=0x18;const uint32_t VSTATE_UNK_2000000=0x2000000;
int mapwin_dest_add(int a,int b){return a+b;}int mapwin_dest_sub(int a,int b){return a-b;}
int get_proto_triad(int){return 2;}
struct Line {int x,y,dx,dy,style;}lines[16];int count,updates;
struct MapWin {
 uint32_t field_1DD70_;int field_1DDA8_=0,field_1DDD8_=10,field_1DDD0_=0;
 int field_1DDA4_=0,field_1DDD4_=10,field_1DDCC_=0;
 void draw_base_dest(int);
 void dest_line(int x,int y,int dx,int dy,int,int,int,int,int style,int,int){lines[count++]={x,y,dx,dy,style};}
 void on_update(){++updates;}
};
BODY
void reset(){count=updates=0;GameDataStateStorage.fields_.base_current_count_=0;GameDataStateStorage.fields_.vehicle_current_count_=1;Vehs[0]=Veh{};
 Vehs[0].order=ORDER_MOVE_TO;Vehs[0].state=0x1000200;
 for(int i=0;i<4;++i){Vehs[0].waypoint_x[i]=10+i;Vehs[0].waypoint_y[i]=20+i;}}
int main(){MapWin w;w.field_1DD70_=0xC00000;
 reset();w.draw_base_dest(1);assert(count==0&&updates==1);
 reset();Vehs[0].waypoint_count=2;w.draw_base_dest(1);
 assert(count==3&&updates==1);
 for(int i=0;i<3;++i){assert(lines[i].style==3);assert(lines[i].x==10+i);assert(lines[i].dx==10+(i+1)%3);}
 reset();w.field_1DD70_=0x800000;Vehs[0].waypoint_count=2;
 Vehs[0].state|=VSTATE_UNK_2000000;w.draw_base_dest(1);
 assert(count==3&&updates==1);for(int i=0;i<3;++i)assert(lines[i].style==2);
 reset();w.field_1DD70_=0xC00000;Vehs[0].faction_id=1;
 w.draw_base_dest(1);assert(count==0&&updates==0);
 reset();w.draw_base_dest(0);assert(count==0&&updates==0);
}
'''

LEGEND = r'''
#include <cassert>
#include <cstring>
#include <cstdio>
struct Font {void init(const char*,int height,int style){assert(height==12&&style==1);}};
const char*DefaultFontFace="Arial";
struct {int cursor_history_index_;int cursor_history_x_[32],cursor_history_y_[32];}ConsoleGlobal;
int MapLongitudeBounds=100,MapLatitudeBounds=100,MapIsFlat=1;
DIRECTIONS
const int RadiusBaseX[9]={1,2,1,0,-1,-2,-1,0,0};
const int RadiusBaseY[9]={-1,0,1,2,1,0,-1,-2,0};
const int MapDirectionPermutation[9]={9,6,3,2,1,4,7,8,5};char StringTemp[256];
char*_itoa(int n,char*p,int base){assert(base==10);std::sprintf(p,"%d",n);return p;}
int calls,xs[8],ys[8],digits[8];
struct Buffer {
 void set_text_color(int a,int b,int c,int d){assert(a==255&&b==0&&c==1&&d==1);}
 void write_cent_l(Font*,const char*s,int x,int y,int width,int height,int n){
  assert(width==24&&height==18&&n==1);xs[calls]=x;ys[calls]=y;digits[calls++]=s[0]-'0';}
};
struct MapWin:Buffer {int field_1DDB8_=18,field_1DDB4_=24;void draw_legend();
 void tile_to_pixel(int x,int y,int*px,int*py){*px=x;*py=y;}};
BODY
int main(){MapWin w;ConsoleGlobal.cursor_history_x_[0]=40;ConsoleGlobal.cursor_history_y_[0]=50;
 VehMoveDeltaX[0]=7;VehMoveDeltaY[0]=9;w.draw_legend();
 assert(calls==8&&xs[0]==47&&ys[0]==59&&digits[0]==9);
 for(int i=1;i<8;++i){assert(xs[i]==40+VehMoveDeltaX[i]);assert(ys[i]==50+VehMoveDeltaY[i]);assert(digits[i]==MapDirectionPermutation[i]);}
 calls=0;ConsoleGlobal.cursor_history_x_[0]=0;MapIsFlat=0;VehMoveDeltaX[0]=-1;
 w.draw_legend();assert(calls==8&&xs[0]==99);
}
'''

def test_route_precedence_and_zero_waypoint_refresh(tmp_path):
    # Original467C21..467D8E selects only one route and publishes work even
    # when the repeated route emits no segment.467DCA refreshes afterward.
    candidate = body("void MapWin::draw_base_dest(int draw_type) {")
    assert execute(tmp_path, ROUTES, candidate, "routes") == 0
    double_route = candidate.replace(
        "} else {\n                dest_line(vehicle.x",
        "}\n            if (ordered_route) {\n                dest_line(vehicle.x")
    assert double_route != candidate
    assert execute(tmp_path, ROUTES, double_route, "double_route") != 0
    missing_refresh = candidate.replace("            any = true;",
                                        "            any = count != 0;")
    assert missing_refresh != candidate
    assert execute(tmp_path, ROUTES, missing_refresh, "missing_refresh") != 0

def test_legend_reads_live_delta_owners(tmp_path):
    # Original469787/4697AF read66EF50/66EF74, not immutable radius tables.
    candidate = body("void MapWin::draw_legend() {")
    legend = LEGEND.replace("DIRECTIONS", actual_source()["DIRECTIONS"])
    assert execute(tmp_path, legend, candidate, "legend") == 0
    damaged = candidate.replace("VehMoveDeltaX[direction]", "RadiusBaseX[direction]")
    damaged = damaged.replace("VehMoveDeltaY[direction]", "RadiusBaseY[direction]")
    assert damaged != candidate
    assert execute(tmp_path, legend, damaged, "wrong_owner") != 0
