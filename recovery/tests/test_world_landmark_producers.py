"""Actual mesa producer controls, not a map-generation or renderer proof.

The complete world_mesa body, enums, radius constants and bounds helpers are
extracted unchanged. The independent diagonal-grid construction below expresses
the original 49-cell traversal without copying an executable byte table.
5C80E0/5C80F5/5C8113 distinguish the inner 25 heights, terrain flag 0x100,
and 49 sequence values; 5C811C independently selects Natural[8]'s name.
Altitude propagation, tile mutation, ocean/landmark lookup, strings and RNG
are service doubles. Natural name values are opaque integer string tokens at
the existing pointer-to-int API boundary, not host/native layout evidence.
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
    header = (ROOT / "src/map.h").read_text()
    declarations = []
    for name in ("TerrainAltitude", "TerrainLandmarkBit2", "TerrainLandmarkId"):
        declarations.append(braced(header, "enum " + name + " {") + ";")
    for name in ("RadiusRange", "RadiusOffsetX", "RadiusOffsetY"):
        declaration, = re.findall(r"const int " + name + r"\[\]\s*=\s*\{[^}]*\};", header)
        declarations.append(declaration)
    for signature in ("MEASURED inline BOOL __cdecl on_map(",
                      "MEASURED inline int __cdecl xrange("):
        declarations.append(braced(header, signature))
    body = braced((ROOT / "src/map.cpp").read_text(), "void __cdecl world_mesa(")
    return "\n".join(declarations), body


HARNESS = r'''
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>
#define __cdecl
#define MEASURED
typedef int BOOL;
void check(bool ok,int line) {
    if(!ok){std::fprintf(stderr,"assertion line %d\n",line);std::exit(102);}
}
#define require(x) check((x),__LINE__)
int MapLongitudeBounds=32,MapLatitudeBounds=32,MapIsFlat=0;
ACTUAL_DECLARATIONS
struct Point {int x,y;};
std::vector<Point> geometry;
struct Event {char kind;int x,y,value;};
std::vector<Event> expected;
size_t cursor;
std::vector<int> random_values;
size_t random_index;
std::vector<Point> attempts;
std::vector<bool> ocean_values,near_values;
size_t ocean_index,near_index;
int retry_mode=0,random_calls=0,ocean_calls=0,near_calls=0;
bool mutate_dimensions=false;
std::string search_events;
struct NaturalRecord {int name;};
NaturalRecord Natural[16];
char label[]="mesa label from string service";
int mesa_rand() {
    search_events+='R';++random_calls;
    if(retry_mode)return 0;
    require(random_index<random_values.size());return random_values[random_index++];
}
#define rand mesa_rand
BOOL is_ocean(int x,int y) {
    search_events+='O';++ocean_calls;
    if(retry_mode){require(x==0 && y==8);return retry_mode==1;}
    require(ocean_index<attempts.size());
    require(x==attempts[ocean_index].x && y==attempts[ocean_index].y);
    require(ocean_index<ocean_values.size());return ocean_values[ocean_index++];
}
BOOL near_landmark(int x,int y) {
    search_events+='N';++near_calls;
    if(retry_mode){require(x==0 && y==8);return true;}
    require(ocean_index>0 && x==attempts[ocean_index-1].x && y==attempts[ocean_index-1].y);
    require(near_index<near_values.size());return near_values[near_index++];
}
void observe(char kind,int x,int y,int value) {
    require(cursor<expected.size());Event e=expected[cursor++];
    require(kind==e.kind && x==e.x && y==e.y && value==e.value);
}
void world_alt_set(int x,int y,int height,BOOL both) {
    require(both==1);observe('A',x,y,height);
    if(mutate_dimensions && cursor==1){MapLongitudeBounds=8;MapLatitudeBounds=8;MapIsFlat=1;}
}
void bit2_set(int x,int y,int flags,BOOL set) {
    require(set==1);observe('B',x,y,flags);
}
void code_set(int x,int y,int code){observe('C',x,y,code);}
struct Strings {
    char *get(int token){observe('S',0,0,token);return label;}
} StringTable;
int new_landmark(int x,int y,const char *name) {
    require(name==label);observe('L',x,y,0);return 71;
}
ACTUAL_BODY

Point rotate(Point p,int turns) {
    while(turns--){int old=p.x;p.x=-p.y;p.y=old;}return p;
}
void grid() {
    geometry.push_back({0,0});
    for(int q=0;q<4;++q) {
        geometry.push_back(rotate({1,-1},q));
        geometry.push_back(rotate({2,0},q));
    }
    for(int q=0;q<4;++q)geometry.push_back(rotate({2,-2},q));
    for(int q=0;q<4;++q) {
        geometry.push_back(rotate({1,-3},q));
        geometry.push_back(rotate({3,-1},q));
    }
    for(int axis=0;axis<2;++axis)for(int sign : {1,-1})
        geometry.push_back({axis==0?4*sign:0,axis==1?4*sign:0});
    for(int q=0;q<4;++q)for(int x=1;x<6;++x)
        geometry.push_back(rotate({x,x-6},q));
    for(int sign : {1,-1}) {
        geometry.push_back({0,6*sign});geometry.push_back({6*sign,0});
    }
    require(geometry.size()==49);
}
void start(int width=32,int height=32,int flat=0) {
    MapLongitudeBounds=width;MapLatitudeBounds=height;MapIsFlat=flat;
    expected.clear();cursor=0;random_values.clear();random_index=0;
    attempts.clear();ocean_values.clear();near_values.clear();ocean_index=near_index=0;
    retry_mode=0;random_calls=ocean_calls=near_calls=0;mutate_dimensions=false;search_events.clear();
    for(int i=0;i<16;++i)Natural[i].name=1000+i*17;
}
void plan(int cx,int cy,bool change_after_first=false) {
    int width=MapLongitudeBounds,height=MapLatitudeBounds,flat=MapIsFlat;
    for(size_t i=0;i<geometry.size();++i) {
        Point d=geometry[i];int x=cx+d.x,y=cy+d.y;
        if(!(flat&1)){if(x<0)x+=width;else if(x>=width)x-=width;}
        if(x<0||x>=width||y<0||y>=height)continue;
        int altitude=(std::abs(d.x)+std::abs(d.y)<=4)?4:3;
        expected.push_back({'A',x,y,altitude});
        expected.push_back({'B',x,y,0x100});
        expected.push_back({'C',x,y,int(i)});
        if(change_after_first && i==0){width=height=8;flat=1;}
    }
    expected.push_back({'S',0,0,1136}); // 1000 + 8*17, not terrain bit 256.
    expected.push_back({'L',cx,cy+2,0});
}
void finish(int x,int y) {
    world_mesa(x,y);require(cursor==expected.size());
    require(random_index==random_values.size());
}
int main() {
    grid();require(BIT2_MESA==256 && LM_MESA==8 && BIT2_MESA!=LM_MESA);
    // Caller-specified valid centres bypass both random-placement filters.
    for(int flat : {0,1,2,3})for(Point p : {Point{16,16},Point{0,0},Point{31,31},Point{1,15}}) {
        start(32,32,flat);plan(p.x,p.y);finish(p.x,p.y);
        require(random_calls==0 && ocean_calls==0 && near_calls==0);
        if(p.x==16 && p.y==16)require(expected.size()==149);
    }
    start(32,32,0);mutate_dimensions=true;plan(2,2,true);finish(2,2);
    // Ocean rejection skips near_landmark. A land tile rejected by the
    // landmark service consumes another attempt, not a new search budget.
    start();random_values={17,4,0,8,7,6};
    attempts={{5,9},{8,8},{7,15}};ocean_values={true,false,false};near_values={true,false};
    plan(7,15);finish(-1,-1);require(search_events=="RRORRONRRON");
    require(ocean_calls==3 && near_calls==2);
    // y=height and x=width are outside, just as negative coordinates are.
    for(Point invalid : {Point{0,32},Point{32,0},Point{-1,0},Point{0,-1}}) {
        start();random_values={0,0};attempts={{0,8}};ocean_values={false};near_values={false};
        plan(0,8);finish(invalid.x,invalid.y);require(search_events=="RRON");
    }
    // Both random bounds <=1 bypass rand; no zero-divisor expression runs.
    start(1,17,1);attempts={{0,8}};ocean_values={false};near_values={false};
    plan(0,8);finish(-1,-1);require(search_events=="ON");
    start(2,18,1);random_values={1,0};attempts={{1,9}};ocean_values={false};near_values={false};
    plan(1,9);finish(-1,-1);require(search_events=="RRON");
    // Last longitude seed is still admitted before parity adjustment.
    start();random_values={0,31};attempts={{30,8}};ocean_values={false};near_values={false};
    plan(30,8);finish(-1,-1);require(search_events=="RRON");
    // Attempt 1000 is abandoned before ocean/near checks or publication.
    for(int mode : {1,2}) {
        start();retry_mode=mode;world_mesa(-1,-1);
        require(cursor==0 && random_calls==2000 && ocean_calls==999);
        require(near_calls==(mode==1?0:999));
    }
}
'''


def run_host(tmp_path, declarations, body):
    compiler = shutil.which("clang++") or shutil.which("g++")
    if compiler is None:
        pytest.skip("host C++ compiler required")
    source = tmp_path / "mesa.cpp"
    source.write_text(HARNESS.replace("ACTUAL_DECLARATIONS", declarations)
                      .replace("ACTUAL_BODY", body))
    binary = tmp_path / "mesa"
    compiled = subprocess.run(
        [compiler, "-std=c++11", "-O1", "-fsanitize=address,undefined",
         "-fno-sanitize-recover=all", str(source), "-o", str(binary)],
        capture_output=True, text=True, timeout=30)
    assert compiled.returncode == 0, compiled.stderr
    return subprocess.run([str(binary)], capture_output=True, text=True, timeout=10,
                          env=dict(os.environ, ASAN_OPTIONS="detect_leaks=0"))


def test_actual_world_mesa(tmp_path):
    result = run_host(tmp_path, *actual_source())
    assert result.returncode == 0, result.stderr


@pytest.mark.parametrize("before,after", [
    ("BIT2_MESA, true", "LM_MESA, true"),
    ("Natural[LM_MESA]", "Natural[LM_CRATER]"),
    ("i < RadiusRange[3]", "i < RadiusRange[2]"),
    ("i < RadiusRange[3]", "i <= RadiusRange[3]"),
    ("i < RadiusRange[2]", "i <= RadiusRange[2]"),
    ("+ ALT_SHORE_LINE", "+ ALT_OCEAN_SHELF"),
    ("ALT_SHORE_LINE, true", "ALT_SHORE_LINE, false"),
    ("BIT2_MESA, true", "BIT2_MESA, false"),
    ("code_set(x_radius, y_radius, i)", "code_set(x_radius, y_radius, i + 1)"),
    ("xrange(x + RadiusOffsetX[i])", "x + RadiusOffsetX[i]"),
    ("y + RadiusOffsetY[i]", "y + RadiusOffsetX[i]"),
    ("if (on_map(x_radius, y_radius))", "if (true)"),
    ("new_landmark(x, y + 2", "new_landmark(x, y"),
    ("if (!on_map(x, y))", "if (false)"),
    ("++loc_attempts >= 1000", "++loc_attempts >= 999"),
    ("++loc_attempts >= 1000", "++loc_attempts > 1000"),
    ("while (is_ocean(x, y))", "while (false)"),
    ("while (near_landmark(x, y))", "while (false)"),
    ("((x_seed ^ y) & 1) ^ x_seed", "x_seed"),
    ("rand() % lat_bounds", "rand() % (lat_bounds - 1)"),
    ("rand() % lon_bounds", "rand() % (lon_bounds - 1)"),
])
def test_world_mesa_damage_is_observed(tmp_path, before, after):
    declarations, body = actual_source()
    assert before in body
    result = run_host(tmp_path, declarations, body.replace(before, after))
    assert result.returncode == 102 and "assertion line" in result.stderr, result.stderr
