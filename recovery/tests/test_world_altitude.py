"""Unchanged world_alt_set with bounded, original-derived terrain controls.

Actual on_map/xrange and radius tables accompany the complete product body.
Expected rings are independently derived from the map's diagonal coordinate
geometry, not the source table indexing. Tile setters, occupancy and bonus
lookups are explicit stateful doubles: this is not real flooding, vehicle
relocation, native ABI or full-game evidence.
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
    for name in ("RadiusRange", "RadiusOffsetX", "RadiusOffsetY"):
        declaration, = re.findall(r"const int " + name + r"\[\]\s*=\s*\{[^}]*\};", header)
        declarations.append(declaration)
    players, = re.findall(r"static const int MaxPlayerNum = \d+;",
                          (ROOT / "src/faction.h").read_text())
    declarations.append(players)
    for signature in ("MEASURED inline BOOL __cdecl on_map(",
                      "MEASURED inline int __cdecl xrange("):
        declarations.append(braced(header, signature))
    body = braced((ROOT / "src/map.cpp").read_text(), "void __cdecl world_alt_set(")
    return "\n".join(declarations), body


HARNESS = r'''
#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <initializer_list>
#include <string>
#define __cdecl
#define MEASURED
typedef int BOOL;
void check(bool ok,int line) {
    if(!ok){std::fprintf(stderr,"assertion line %d\n",line);std::exit(83);}
}
#define require(x) check((x),__LINE__)
int MapLongitudeBounds=64,MapLatitudeBounds=64,MapIsFlat;
ACTUAL_DECLARATIONS
struct Guarded {int before;int values[MaxPlayerNum];int after;};
Guarded counts,closest;
int (&MapBaseSubmergedCount)[MaxPlayerNum]=counts.values;
int (&MapBaseIdClosestSubmergedVeh)[MaxPlayerNum]=closest.values;
void ZeroMemory(void *p,size_t bytes){std::memset(p,0,bytes);}
int heights[64][64],expected[64][64],reads[64][64],owners[64][64];
int occupant=-1,bonus=0,set_calls,owner_calls,bonus_calls,anything_calls;
int centre_x,centre_y;
std::string events;
void reset_globals_observed() {
    for(int i=0;i<MaxPlayerNum;++i)
        require(MapBaseSubmergedCount[i]==0 && MapBaseIdClosestSubmergedVeh[i]==-1);
    require(counts.before==91 && counts.after==92 && closest.before==93 && closest.after==94);
}
int alt_at(int x,int y) {
    require(on_map(x,y));++reads[y][x];return heights[y][x];
}
void set(int x,int y,int value,char kind) {
    require(on_map(x,y));reset_globals_observed();++set_calls;
    events+=kind;heights[y][x]=value;
}
void alt_set(int x,int y,uint32_t value){set(x,y,value,'S');}
void alt_set_both(int x,int y,int value){set(x,y,value,'B');}
int anything_at(int x,int y) {
    require(on_map(x,y) && events.back()=='B');
    require(x!=centre_x || y!=centre_y);events+='A';++anything_calls;return occupant;
}
int bonus_at(int x,int y,int mode) {
    require(on_map(x,y) && mode==0 && events.back()=='A');
    events+='P';++bonus_calls;return bonus;
}
void owner_set(int x,int y,int owner) {
    require(on_map(x,y) && owner==-1 && events.back()=='P');
    events+='O';++owner_calls;owners[y][x]=owner;
}
ACTUAL_BODY
void start(int value=3) {
    MapIsFlat=0;occupant=-1;bonus=0;
    set_calls=owner_calls=bonus_calls=anything_calls=0;events.clear();
    counts.before=91;counts.after=92;closest.before=93;closest.after=94;
    for(int i=0;i<MaxPlayerNum;++i){counts.values[i]=70+i;closest.values[i]=80+i;}
    for(int y=0;y<64;++y)for(int x=0;x<64;++x) {
        heights[y][x]=expected[y][x]=value;reads[y][x]=0;owners[y][x]=6;
    }
}
void finish(int x,int y,int altitude,int both) {
    centre_x=x;centre_y=y;world_alt_set(x,y,altitude,both);
    reset_globals_observed();
    require(std::memcmp(heights,expected,sizeof(expected))==0);
}
int ring(int dx,int dy) {
    if((dx+dy)&1)return -1;
    return std::max(std::abs((dx+dy)/2),std::abs((dy-dx)/2));
}
int main() {
    // Uniform extremes ensure every required consecutive ring propagates.
    // In diagonal map coordinates each geometric ring r is clamped to
    // altitude-r or altitude+r, with strict comparisons and altitude0..7.
    // This detects skipping the first eight neighbours as well as bad bounds.
    for(int altitude=0;altitude<8;++altitude)for(int initial : {0,7}) {
        start(initial);int changed=1;
        expected[32][32]=altitude;
        for(int y=16;y<=48;++y)for(int x=16;x<=48;++x) {
            int r=ring(x-32,y-32);if(r<=0)continue;
            int value=initial==0 ? std::max(0,altitude-r) : std::min(7,altitude+r);
            expected[y][x]=value;if(value!=initial)++changed;
        }
        finish(32,32,altitude,0);require(set_calls==changed);
        require(anything_calls==0 && bonus_calls==0 && owner_calls==0);
        for(int y=0;y<64;++y)for(int x=0;x<64;++x) {
            int r=ring(x-32,y-32),visited=0;
            if(r>0) {
                if(initial==0)visited=(r<altitude)+(r==1 && altitude<7);
                else visited=(r==1 && altitude>1)+(r<8-altitude);
            }
            require(reads[y][x]==visited);
        }
    }
    // Independent lower/equal/higher first-ring observations, on both
    // sides of the centre height. Equal values must not invoke setters.
    for(int old=0;old<8;++old) {
        start();heights[31][33]=old;
        expected[31][33]=std::max(2,std::min(4,old));
        finish(32,32,3,0);
        require(set_calls==1+(old<2 || old>4));
    }
    // A successful ring does not keep a later unchanged ring alive.
    start(5);heights[32][34]=0;expected[32][34]=4;
    heights[32][38]=expected[32][38]=0;
    finish(32,32,5,0);require(reads[32][38]==0 && set_calls==2);
    start(2);heights[32][34]=7;expected[32][34]=3;
    heights[32][38]=expected[32][38]=7;
    finish(32,32,2,0);require(reads[32][38]==0 && set_calls==2);
    // Stop immediately when the first ring did not change, even if a later
    // ring contains a value that would otherwise satisfy its predicate.
    start();heights[32][36]=expected[32][36]=7;
    finish(32,32,3,0);require(reads[32][36]==0 && set_calls==1);

    for(int both : {0,1,-7})for(int old : {0,7})
    for(int occupied : {-2,-1,0,7})for(int has_bonus : {0,1,-1}) {
        start();occupant=occupied;bonus=has_bonus;
        heights[31][33]=old;expected[31][33]=old==0 ? 2 : 4;
        finish(32,32,3,both);
        require(set_calls==2);
        require(anything_calls==(both!=0));
        require(bonus_calls==(both!=0 && occupied<0));
        require(owner_calls==(both!=0 && occupied<0 && has_bonus==0));
        require(owners[32][32]==6);
        require(owners[31][33]==(owner_calls ? -1 : 6));
        std::string order=both ? "BBA" : "SS";
        if(both && occupied<0){order+='P';if(!has_bonus)order+='O';}
        require(events==order);
    }
    // Longitude wraps once on cylindrical maps, but never on flat maps;
    // latitude and centre validity are independent of longitude wrapping.
    for(int flat : {0,1})for(int x : {0,63}) {
        start();MapIsFlat=flat;
        int near=x==0 ? 62 : 1;
        heights[20][near]=7;expected[20][near]=flat ? 7 : 4;
        finish(x,20,3,0);require(set_calls==(flat ? 1 : 2));
    }
    start();heights[0][0]=7;expected[0][0]=4;
    finish(-1,-1,3,0);require(set_calls==1); // off-map centre does not abort rings
    start();finish(-100,-100,3,1);require(set_calls==0);
    start();finish(32,64,3,1);require(set_calls==0);
    start();finish(64,32,3,1);require(set_calls==0);
    return 0;
}
'''


def run_host(tmp_path, declarations, body):
    compiler = shutil.which("clang++") or shutil.which("g++")
    if compiler is None:
        pytest.skip("host C++ compiler required")
    source = tmp_path / "altitude.cpp"
    source.write_text(HARNESS.replace("ACTUAL_DECLARATIONS", declarations)
                      .replace("ACTUAL_BODY", body))
    binary = tmp_path / "altitude"
    compiled = subprocess.run(
        [compiler, "-std=c++11", "-O1", "-fsanitize=address,undefined",
         "-fno-sanitize-recover=all", str(source), "-o", str(binary)],
        capture_output=True, text=True, timeout=30)
    assert compiled.returncode == 0, compiled.stderr
    return subprocess.run([str(binary)], capture_output=True, text=True, timeout=10,
                          env=dict(os.environ, ASAN_OPTIONS="detect_leaks=0"))


def test_actual_world_altitude(tmp_path):
    result = run_host(tmp_path, *actual_source())
    assert result.returncode == 0, result.stderr


@pytest.mark.parametrize("before,after", [
    ("alt_at(x_radius, y_radius) > alt", "alt_at(x_radius, y_radius) < alt"),
    ("alt_at(x_radius, y_radius) > alt", "alt_at(x_radius, y_radius) >= alt"),
    ("alt_at(x_radius, y_radius) < alt", "alt_at(x_radius, y_radius) <= alt"),
    ("RadiusRange[i - 1]; j < RadiusRange[i]", "RadiusRange[i]; j < RadiusRange[i + 1]"),
    ("i < altitude", "i <= altitude"),
    ("8 - altitude", "7 - altitude"),
    ("altitude - 1", "altitude - 2"),
    ("alt = i + altitude", "alt = altitude"),
    ("if (!has_set_alt)", "if (false)"),
    ("has_set_alt = false;", "has_set_alt = true;"),
    ("xrange(x + RadiusOffsetX[j])", "(x + RadiusOffsetX[j])"),
    ("y + RadiusOffsetY[j]", "y + RadiusOffsetX[j]"),
    ("on_map(x_radius, y_radius) &&", "true &&"),
    ("if (on_map(x, y))", "if (true)"),
    ("if (is_set_both)", "if (!is_set_both)"),
    ("anything_at(x_radius, y_radius) < 0", "anything_at(x_radius, y_radius) <= 0"),
    ("!bonus_at(x_radius, y_radius, 0)", "bonus_at(x_radius, y_radius, 0)"),
    ("owner_set(x_radius, y_radius, -1)", "owner_set(x_radius, y_radius, 0)"),
    ("memset(MapBaseIdClosestSubmergedVeh, -1", "memset(MapBaseIdClosestSubmergedVeh, 0"),
    ("sizeof(int) * MaxPlayerNum", "sizeof(int) * (MaxPlayerNum - 1)"),
])
def test_world_altitude_damage_is_observed(tmp_path, before, after):
    declarations, body = actual_source()
    assert before in body
    result = run_host(tmp_path, declarations, body.replace(before, after))
    assert result.returncode == 83, result.stderr
