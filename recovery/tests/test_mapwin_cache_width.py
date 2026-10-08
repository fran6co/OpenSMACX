"""Direct-source controls for MapWin terrain-cache invalidators.

Actual POD, radius tables and complete method bodies are extracted from the
current product files. Independent expected writes use map-coordinate
geometry, never the radius tables used by the implementation.
"""
from pathlib import Path
import os
import re
import shutil
import subprocess

import pytest


ROOT = Path(__file__).resolve().parents[2]


def braced(source: str, signature: str) -> str:
    start = source.index(signature)
    opening = source.index("{", start)
    depth = 0
    for end in range(opening, len(source)):
        depth += (source[end] == "{") - (source[end] == "}")
        if depth == 0:
            return source[start:end + 1]
    raise AssertionError(f"unterminated method: {signature}")


def actual_source() -> tuple[str, str, str, str]:
    header = (ROOT / "src/mapwin.h").read_text()
    pod = braced(header, "struct TerrainCacheEntry") + ";"
    pointer, = re.findall(r"TerrainCacheEntry\s*\*\s*owned_\s*;", header)
    map_source = (ROOT / "src/map.cpp").read_text()
    tables = []
    for name in ("RadiusRange", "RadiusOffsetX", "RadiusOffsetY"):
        declaration, = re.findall(
            r"const int " + name + r"\[\d*\]\s*=\s*\{[^}]*\};", map_source
        )
        tables.append(declaration)
    source = (ROOT / "src/mapwin.cpp").read_text()
    bodies = "\n\n".join(
        braced(source, signature)
        for signature in (
            "void MapWin::UNK6(int UNUSED(x) x, int UNUSED(y) y, int radius)",
            "void MapWin::clear_alt(int x, int y, int radius)",
            "void MapWin::UNK7()",
        )
    )
    return pod, pointer, "\n".join(tables), bodies


HARNESS = r'''
#include <assert.h>
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <type_traits>
#include <initializer_list>
#define UNUSED(x)
void check(bool ok,int line){if(!ok){fprintf(stderr,"assertion line %d\n",line);exit(91);}}
#define require(x) check((x),__LINE__)
ACTUAL_POD
ACTUAL_TABLES
uint32_t MapArea,MapLongitude=8;
int MapLongitudeBounds=16,MapLatitudeBounds=16,MapIsFlat;
struct MapWin {
    ACTUAL_POINTER
    void UNK6(int x,int y,int radius);
    void clear_alt(int x,int y,int radius);
    void UNK7();
};
ACTUAL_BODIES

static_assert(sizeof(TerrainCacheEntry)==12,"record stride");
static_assert(offsetof(TerrainCacheEntry,brightness)==0,"brightness offset");
static_assert(offsetof(TerrainCacheEntry,altitude)==4,"altitude offset");
static_assert(offsetof(TerrainCacheEntry,terrain_bits)==9,"terrain flags offset");
static_assert(offsetof(TerrainCacheEntry,altitude_detail)==10,"detail offset");
static_assert((std::is_same<decltype(TerrainCacheEntry::brightness),unsigned char[4]>::value),"brightness type");
static_assert((std::is_same<decltype(TerrainCacheEntry::altitude),signed char[5]>::value),"altitude type");

static TerrainCacheEntry backing[8192];
static unsigned char expected[sizeof(backing)];
static const int centre=4096;
static void poison(){
    unsigned char *p=reinterpret_cast<unsigned char *>(backing);
    for (size_t i=0;i<sizeof(backing);++i) p[i]=static_cast<unsigned char>((i*37+19)%251);
    memcpy(expected,p,sizeof(backing));
}
static void unchanged(){require(memcmp(expected,backing,sizeof(backing))==0);}
static int floor_half(int x){return x>=0 ? x/2 : -((-x+1)/2);}

static void expected_brightness_radius(int radius,int full_width){
    int extent=radius+1;
    for(int u=-extent;u<=extent;++u) for(int v=-extent;v<=extent;++v){
        int dx=u+v, dy=u-v;
        int index=centre+dy*(full_width>>1)+floor_half(dx);
        require(index>=0 && index<8192);
        for(int corner=0;corner<4;++corner) expected[index*12+corner]=0xFF;
    }
}

static void expected_altitude_radius(int radius,int x,int y,int flat,int full_width,int height){
    int extent=radius+1;
    for(int u=-extent;u<=extent;++u) for(int v=-extent;v<=extent;++v){
        int dx=x+u+v, dy=y+u-v;
        if(!flat){
            if(dx<0) dx+=full_width;
            else if(dx>=full_width) dx-=full_width;
        }
        if(dy<0 || dy>=height || dx<0 || dx>=full_width) continue;
        int index=centre+(full_width>>1)*dy+(dx>>1);
        require(index>=0 && index<8192);
        for(int byte=4;byte<9;++byte) expected[index*12+byte]=0xFF;
    }
}

int main(){
    require(MapLongitudeBounds==16 && MapLongitude==8);
    require(MapLongitudeBounds/2 != MapLongitude/2); // catch the wrong half-row stride
    MapWin window={backing+centre};

    // UNK6 ignores x/y and invalidates brightness around the owner origin.
    // Its row width is full MapLongitudeBounds/2 (8 here), not MapLongitude/2 (4).
    for(int radius : {-1,0,1}){
        poison();
        expected_brightness_radius(radius,MapLongitudeBounds);
        window.UNK6(-113,207,radius);
        unchanged();
        require(window.owned_==backing+centre);
    }

    // clear_alt follows the requested map coordinate, resets altitude bytes
    // only, wraps at both horizontal edges only for round maps, and clips y.
    for(int flat : {0,1}) for(int x : {1,15}) for(int radius : {-1,0,1}){
        poison();MapIsFlat=flat;
        expected_altitude_radius(radius,x,1,flat,MapLongitudeBounds,MapLatitudeBounds);
        window.clear_alt(x,1,radius);
        unchanged();
        require(window.owned_==backing+centre);
    }

    // UNK7 touches only the first four bytes of each signed-MapArea entry.
    for(uint32_t count : {0u,1u,5u}){
        poison();MapArea=count;
        for(uint32_t i=0;i<count;++i) for(int j=0;j<4;++j)
            expected[(centre+static_cast<int>(i))*12+j]=0xFF;
        window.UNK7();unchanged();
    }
    poison();MapArea=5;window.owned_=0;window.UNK7();unchanged();

    // A high-bit MapArea converts to negative int in the original signed loop
    // comparison; a pointer that would fault on any write makes the boundary
    // observable without allocating a huge cache.
    MapArea=0x80000000u;
    window.owned_=reinterpret_cast<TerrainCacheEntry *>(static_cast<uintptr_t>(1));
    window.UNK7();
    window.owned_=backing+centre;
    require(MapLongitudeBounds==16);
}
'''


def run_host(tmp_path, source_tuple):
    compiler = shutil.which("clang++") or shutil.which("g++")
    if not compiler:
        pytest.skip("host C++ compiler required")
    pod, pointer, tables, bodies = source_tuple
    source = tmp_path / "cache-width.cpp"
    source.write_text(HARNESS.replace("ACTUAL_POD", pod)
                      .replace("ACTUAL_POINTER", pointer)
                      .replace("ACTUAL_TABLES", tables)
                      .replace("ACTUAL_BODIES", bodies))
    binary = tmp_path / "cache-width"
    built = subprocess.run(
        [compiler, "-std=c++11", "-Wall", "-Wextra", "-Werror",
         "-Wno-unused-parameter", "-fsanitize=address,undefined",
         "-fno-sanitize-recover=all", str(source), "-o", str(binary)],
        capture_output=True, text=True, timeout=30,
    )
    assert built.returncode == 0, built.stderr
    return subprocess.run([str(binary)], capture_output=True, text=True,
                          timeout=10, env=dict(os.environ, ASAN_OPTIONS="detect_leaks=0"))


def test_actual_cache_invalidator_bytes(tmp_path):
    result = run_host(tmp_path, actual_source())
    assert result.returncode == 0, result.stderr


def test_clear_alt_width_reload_structure():
    source = (ROOT / "src/mapwin.cpp").read_text()
    body = braced(source, "void MapWin::clear_alt(int x, int y, int radius)")
    loads = [m.start() for m in re.finditer(r"width\s*=\s*MapLongitudeBounds\s*;", body)]
    assert len(loads) == 2
    store = body.index("cell.altitude[4] = -1;")
    assert loads[0] < body.index("do {") < store < loads[1]
    # Verify lexical block ownership: width reset belongs to the valid-store
    # arm, not merely an earlier text position in the loop.
    valid = body.index("if (dy >= 0 && dy < MapLatitudeBounds && dx >= 0 && dx < width)")
    opening = body.index("{", valid)
    depth = 0
    close = None
    for pos in range(opening, len(body)):
        depth += (body[pos] == "{") - (body[pos] == "}")
        if depth == 0:
            close = pos
            break
    assert close is not None and opening < loads[1] < close


@pytest.mark.parametrize("before,after", [
    ("RadiusOffsetY[i] * (MapLongitudeBounds >> 1)",
     "RadiusOffsetY[i] * (static_cast<int>(MapLongitude) >> 1)"),
    ("int width = MapLongitudeBounds;", "int width = static_cast<int>(MapLongitude);"),
    ("owned_[i].brightness[j] = 0xFF;", "owned_[i].altitude[j] = -1;"),
    ("cell.altitude[4] = -1;",
     "reinterpret_cast<unsigned char *>(&cell)[9] = 0xFF;"),
])
def test_cache_invalidator_damage_is_observed(tmp_path, before, after):
    pod, pointer, tables, bodies = actual_source()
    assert before in bodies
    damaged = bodies.replace(before, after, 1)
    result = run_host(tmp_path, (pod, pointer, tables, damaged))
    assert result.returncode != 0
    assert (result.returncode == 91 or "AddressSanitizer" in result.stderr or
            "runtime error:" in result.stderr), result.stderr


def test_clear_alt_reload_tail_mutant_is_rejected():
    source = (ROOT / "src/mapwin.cpp").read_text()
    body = braced(source, "void MapWin::clear_alt(int x, int y, int radius)")
    correct = "                    width = MapLongitudeBounds;\n                }\n                i++;"
    moved = "                }\n                i++;\n                width = MapLongitudeBounds;"
    assert correct in body
    mutant = body.replace(correct, moved, 1)
    opening = mutant.index("{", mutant.index(
        "if (dy >= 0 && dy < MapLatitudeBounds && dx >= 0 && dx < width)"))
    depth = 0
    close = None
    for pos in range(opening, len(mutant)):
        depth += (mutant[pos] == "{") - (mutant[pos] == "}")
        if depth == 0:
            close = pos
            break
    reset = mutant.index("width = MapLongitudeBounds;", mutant.index("cell.altitude[4] = -1;"))
    assert close is not None and reset > close

