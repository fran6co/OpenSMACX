"""Actual terrain-cache invalidators with byte-level, bounded host controls.

The real POD, pointer declaration, radius tables and complete method bodies
are extracted unchanged. Expected offsets come from diagonal map geometry,
not those tables. A centered backing array permits observing negative UNK6
indices: it does NOT make the game's ordinary allocation safe, or justify
adding coordinate centering, wraparound or bounds checks absent in the image.
These controls do not prove native MapWin layout, allocation lifetime or game
behavior. Final-byte observations do not establish the two-pass store order.
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
    raise AssertionError("unterminated actual source")


def actual_source():
    header = (ROOT / "src/mapwin.h").read_text()
    pod = braced(header, "struct TerrainCacheEntry") + ";"
    pointer, = re.findall(r"TerrainCacheEntry\s*\*\s*owned_\s*;", header)
    map_source = (ROOT / "src/map.cpp").read_text()
    tables = []
    for name in ("RadiusRange", "RadiusOffsetX", "RadiusOffsetY"):
        declaration, = re.findall(r"const int " + name + r"\[\d*\]\s*=\s*\{[^}]*\};", map_source)
        tables.append(declaration)
    source = (ROOT / "src/mapwin.cpp").read_text()
    bodies = "\n".join(braced(source, signature) for signature in
                       ("void MapWin::clear_terrain()", "void MapWin::UNK6("))
    return pod, pointer, "\n".join(tables), bodies


HARNESS = r'''
#include <cstdint>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <type_traits>
#include <initializer_list>
#define UNUSED(x)
void check(bool ok,int line){if(!ok){std::fprintf(stderr,"assertion line %d\n",line);std::exit(91);}}
#define require(x) check((x),__LINE__)
ACTUAL_POD
ACTUAL_TABLES
uint32_t MapArea,MapLongitude;
struct MapWin {
    ACTUAL_POINTER
    void clear_terrain();
    void UNK6(int x,int y,int radius);
};
ACTUAL_BODIES
TerrainCacheEntry backing[4096];
unsigned char expected[sizeof(backing)];
const int centre=2048;
void poison(){
    auto *bytes=reinterpret_cast<unsigned char *>(backing);
    for(unsigned i=0;i<sizeof(backing);++i)bytes[i]=static_cast<unsigned char>((i*37+11)%251);
    std::memcpy(expected,backing,sizeof(backing));
}
void unchanged(){require(std::memcmp(expected,backing,sizeof(backing))==0);}
int floor_half(int n){return n>=0 ? n/2 : -((-n+1)/2);}
void expect_clear(int count){
    for(int entry=0;entry<count;++entry)
        for(int byte=0;byte<9;++byte)expected[(centre+entry)*12+byte]=255;
}
void expect_radius(int radius,int longitude){
    // Independent diagonal coordinates (u,v), not RadiusOffset/Range reads.
    int extent=radius+1;
    for(int u=-extent;u<=extent;++u)for(int v=-extent;v<=extent;++v){
        int x=u+v,y=u-v;
        int entry=centre+y*floor_half(longitude)+floor_half(x);
        require(entry>=0 && entry<4096);
        for(int byte=0;byte<4;++byte)expected[entry*12+byte]=255;
    }
}
int main(){
    require(sizeof(TerrainCacheEntry)==12);
    require(std::is_standard_layout<TerrainCacheEntry>::value && std::is_trivial<TerrainCacheEntry>::value);
    ACTUAL_FIELD_CHECKS
    MapWin window={backing+centre};
    for(uint32_t count : {0u,1u,2u,7u,31u,0xffffffffu,0x80000000u}){
        poison();MapArea=count;
        if(count<32)expect_clear(count);
        window.clear_terrain();unchanged();
        require(window.owned_==backing+centre && MapArea==count);
        window.clear_terrain();unchanged();
    }
    window.owned_=nullptr;
    for(uint32_t count : {0u,7u,0xffffffffu}){
        poison();MapArea=count;window.clear_terrain();unchanged();
        window.UNK6(123,-456,0);unchanged();require(window.owned_==nullptr);
    }
    window.owned_=backing+centre;
    for(int longitude : {0,1,2,7,32,129})for(int radius=-1;radius<=7;++radius){
        for(int args : {0,1,2}){
            poison();MapLongitude=longitude;MapArea=args==0 ? 0 : 1;
            expect_radius(radius,longitude);
            window.UNK6(args==0 ? 0 : args==1 ? 101 : -37,
                        args==0 ? 0 : args==1 ? -205 : 81,radius);
            unchanged();require(window.owned_==backing+centre && MapLongitude==uint32_t(longitude));
            window.UNK6(911,-733,radius);unchanged();
        }
    }
}
'''


def run_host(tmp_path, pod, pointer, tables, bodies):
    compiler = shutil.which("clang++") or shutil.which("g++")
    if not compiler:
        pytest.skip("host C++ compiler required")
    checks = """
    require(offsetof(TerrainCacheEntry,brightness)==0 && sizeof(backing[0].brightness)==4);
    require(offsetof(TerrainCacheEntry,altitude)==4 && sizeof(backing[0].altitude)==5);
    require(offsetof(TerrainCacheEntry,terrain_bits)==9);
    require(offsetof(TerrainCacheEntry,altitude_detail)==10);
    require((std::is_same<decltype(backing[0].brightness),unsigned char[4]>::value));
    require((std::is_same<decltype(backing[0].altitude),signed char[5]>::value));
    require((std::is_same<decltype(backing[0].terrain_bits),unsigned char>::value));
    require((std::is_same<decltype(backing[0].altitude_detail),unsigned short>::value));
    require((std::is_same<decltype(MapWin::owned_),TerrainCacheEntry *>::value));
    """
    source = tmp_path / "cache.cpp"
    source.write_text(HARNESS.replace("ACTUAL_POD", pod).replace("ACTUAL_POINTER", pointer)
                      .replace("ACTUAL_TABLES", tables).replace("ACTUAL_BODIES", bodies)
                      .replace("ACTUAL_FIELD_CHECKS", checks))
    binary = tmp_path / "cache"
    compiled = subprocess.run([compiler, "-std=c++11", "-O1", "-fsanitize=address,undefined",
                               "-fno-sanitize-recover=all", str(source), "-o", str(binary)],
                              capture_output=True, text=True, timeout=30)
    assert compiled.returncode == 0, compiled.stderr
    return subprocess.run([str(binary)], capture_output=True, text=True, timeout=10,
                          env=dict(os.environ, ASAN_OPTIONS="detect_leaks=0"))


def test_actual_mapwin_cache(tmp_path):
    result = run_host(tmp_path, *actual_source())
    assert result.returncode == 0, result.stderr


@pytest.mark.parametrize("before,after", [
    ("RadiusRange[radius + 1]", "RadiusRange[radius == -1 ? 0 : radius]"),
    ("int i = 0;", "int i = 1;"),
    ("static_cast<int>(MapLongitude) >> 1", "static_cast<int>(MapLongitude)"),
    ("(RadiusOffsetX[i] >> 1)", "(RadiusOffsetX[i] / 2)"),
    ("owned_[idx].brightness[corner]", "owned_[idx + 1].brightness[corner]"),
    ("owned_[idx].brightness[corner] = 0xFF;",
     "if (idx >= 0) owned_[idx].brightness[corner] = 0xFF;"),
    ("owned_[idx].brightness[corner] = 0xFF;",
     "if (idx >= 0 && idx < (int)MapArea) owned_[idx].brightness[corner] = 0xFF;"),
    ("owned_[idx].brightness[corner]",
     "owned_[idx + y * (int(MapLongitude) / 2) + x / 2].brightness[corner]"),
    ("owned_[idx].brightness[corner] = 0xFF;",
     "owned_[idx].brightness[corner] = 0xFF; owned_[idx].altitude[0] = -1;"),
    ("corner < 4", "corner < 3"),
    ("owned_[idx].brightness[corner] = 0xFF;", "owned_[idx].brightness[corner] = 0;"),
    ("owned_[idx].brightness[corner]",
     "(*reinterpret_cast<TerrainCacheEntry **>(owned_))[idx].brightness[corner]"),
])
def test_radius_cache_damage_is_observed(tmp_path, before, after):
    pod, pointer, tables, bodies = actual_source()
    assert before in bodies
    result = run_host(tmp_path, pod, pointer, tables, bodies.replace(before, after))
    assert result.returncode != 0
    assert result.returncode == 91 or "runtime error:" in result.stderr or "AddressSanitizer" in result.stderr, result.stderr


@pytest.mark.parametrize("before,after", [
    ("entry < static_cast<int32_t>(MapArea)", "entry + 1 < static_cast<int32_t>(MapArea)"),
    ("altitude_entry < static_cast<int32_t>(MapArea)", "altitude_entry + 1 < static_cast<int32_t>(MapArea)"),
    ("owned_[entry].brightness[corner]", "owned_[entry * 2].brightness[corner]"),
    ("owned_[altitude_entry].altitude[slot]", "owned_[altitude_entry * 2].altitude[slot]"),
    ("slot < 5", "slot < 4"),
    ("owned_[entry].brightness[corner] = 0xFF;", "owned_[entry].brightness[corner] = 0;"),
    ("owned_[altitude_entry].altitude[slot] = -1;", "owned_[altitude_entry].altitude[slot] = 0;"),
    ("owned_[entry].brightness[corner] = 0xFF;",
     "owned_[entry].brightness[corner] = 0xFF; owned_[entry].terrain_bits = 0;"),
    ("owned_[altitude_entry].altitude[slot] = -1;",
     "owned_[altitude_entry].altitude[slot] = -1; owned_[altitude_entry].altitude_detail = 0xffff;"),
    ("if (owned_ != 0)", "if (true)"),
    ("owned_[entry].brightness[corner]",
     "(*reinterpret_cast<TerrainCacheEntry **>(owned_))[entry].brightness[corner]"),
    ("for (int entry = 0; entry < static_cast<int32_t>(MapArea);",
     "for (int entry = 0; static_cast<uint32_t>(entry) < MapArea;"),
])
def test_full_cache_damage_is_observed(tmp_path, before, after):
    pod, pointer, tables, bodies = actual_source()
    assert before in bodies
    result = run_host(tmp_path, pod, pointer, tables, bodies.replace(before, after))
    assert result.returncode != 0
    assert result.returncode == 91 or "runtime error:" in result.stderr or "AddressSanitizer" in result.stderr, result.stderr
