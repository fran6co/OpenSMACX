"""Shared ocean-tail alias controls, not complete lighting/game verification.

Extract the real owner, cache POD, addressing helper and complete get_brighting
body and the real generated-table lookup helper. map_loc and get_alt are explicit
service doubles. Table contents are independently seeded test data, not original
initialization proof; CornerLightTable doubles the unresolved exceptional indices.
Original 470704 selects signed bytes at 7F6680+7*v1+v2, overlapping the final
DWORD of the sixteen-shade owner only for indices 0..3. Other cases start at
49/98/147. Native layout, renderer publication, real get_alt and allocation
lifetime require separate checks. A null final owner is outside this harness:
the original unconditionally writes through it, rather than returning safely.
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
    raise AssertionError("unterminated source")


def actual_source():
    source = (ROOT / "src/mapwin.cpp").read_text()
    header = (ROOT / "src/mapwin.h").read_text()
    pod = braced(header, "struct TerrainCacheEntry") + ";"
    owner, = re.findall(r"static int OceanCornerShades\[16\];", source)
    pointer, = re.findall(r"TerrainCacheEntry\s*\*\s*owned_\s*;", header)
    signatures = []
    for name in ("get_brighting", "get_alt"):
        signature, = re.findall(r"int " + name + r"\([^;{}]*\);", header)
        signatures.append(signature)
    tile_helper = braced(source, "static __forceinline char *brighting_tile(")
    table, = re.findall(r"^signed char MapLightingTable\[4 \* 49\];", source, re.M)
    light_helper = braced(source, "static __forceinline signed char corner_light_value(int biased_index)")
    body = braced(source, "int MapWin::get_brighting(")
    return "\n".join((pod, owner, table, "struct MapWin {", pointer,
                      *signatures, "};", tile_helper, light_helper, body))


HARNESS = r'''
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <type_traits>
#include <initializer_list>
#define __forceinline inline
void check(bool ok,int line) {
    if(!ok) { std::fprintf(stderr,"assertion line %d\n",line); std::_Exit(98); }
}
#define require(x) check((x),__LINE__)
int MapLongitudeBounds=8,MapLatitudeBounds=4;
signed char lighting[268];
const signed char *CornerLightTable=lighting; // explicitly unrecovered service
unsigned char terrain;
int map_calls,alt_calls,mode,null_mask;
int wanted[4],expected_x,expected_y;
unsigned char *map_loc(int x,int y) {
    require(x==expected_x && y==expected_y); ++map_calls; return &terrain;
}
ACTUAL_SOURCE
TerrainCacheEntry banks[5][40];
unsigned char before[sizeof banks];
MapWin window;
int signed_byte(unsigned v) { return v>=128 ? int(v)-256 : int(v); }
int location(int width,int x,int y) { return width/2*y+x/2; }
int oracle(int corner,const int *v) {
    const int next=(corner+1)%4;
    const int index=49*corner+7*v[corner]+v[next];
    if(index<4) {
        // Independent division rather than the implementation's shift/mask.
        uint32_t word=uint32_t(OceanCornerShades[15]);
        uint32_t divisor=1;
        for(int j=0;j<index;++j)divisor*=256;
        return signed_byte((word/divisor)%256);
    }
    return (index*37+13)%256-128;
}
int MapWin::get_alt(int x,int y,int corner) {
    require(x==expected_x && y==expected_y);
    require(corner==++alt_calls && corner<=4);
    if(mode==1) {
        MapLongitudeBounds=8+2*corner;
        owned_=(null_mask&(1<<(corner-1))) ? nullptr : banks[corner];
    }
    if(owned_) {
        // Poison non-selected nibbles: all four reads must occur immediately
        // after their respective call, using its freshly published owner/width.
        unsigned word=mode==1 ? 0xdddd : 0;
        if(mode==0) {
            for(int i=0;i<4;++i)word|=unsigned(wanted[i])<<(4*i);
        } else {
            word &= ~(15u<<(4*(corner-1)));
            word |= unsigned(wanted[corner-1])<<(4*(corner-1));
        }
        owned_[location(MapLongitudeBounds,x,y)].altitude_detail=word;
    }
    return -777; // original consumes the cache, never this return value
}
void reset(int x=2,int y=1) {
    expected_x=x;expected_y=y;map_calls=alt_calls=mode=null_mask=0;
    MapLongitudeBounds=8;MapLatitudeBounds=4;terrain=0x60;
    std::memset(banks,0x35,sizeof banks);window.owned_=banks[0];
    for(int bank=0;bank<5;++bank)for(int cell=0;cell<40;++cell)
        for(int c=0;c<4;++c)banks[bank][cell].brightness[c]=255;
    for(int i=0;i<268;++i)lighting[i]=static_cast<signed char>((i*37+13)%256-128);
    for(int i=0;i<196;++i)MapLightingTable[i]=lighting[i+8];
    for(int i=0;i<16;++i)OceanCornerShades[i]=int(0x10203000u+unsigned(i));
    for(int i=0;i<4;++i)wanted[i]=0;
}
void verify_call(int corner,int expected) {
    int saved_shades[16];std::memcpy(saved_shades,OceanCornerShades,sizeof saved_shades);
    int result=window.get_brighting(expected_x,expected_y,corner);
    require(result==expected && map_calls==1 && alt_calls==4);
    require(std::memcmp(saved_shades,OceanCornerShades,sizeof saved_shades)==0);
    require(window.owned_[location(MapLongitudeBounds,expected_x,expected_y)].brightness[corner]
            ==static_cast<unsigned char>(expected+128));
    // A second request observes the cache, except 127 encodes FF (uncached).
    if(expected!=127) {
        OceanCornerShades[15]^=0x55555555;
        require(window.get_brighting(expected_x,expected_y,corner)==expected);
        require(map_calls==1 && alt_calls==4);
    }
}
int main() {
    require(sizeof(int)==4 && sizeof(OceanCornerShades)==64);
    require((std::is_same<decltype(OceanCornerShades),int[16]>::value));
    for(int i=0;i<16;++i)require(OceanCornerShades[i]==0);
    // All 256 bit patterns, independently in each of the four tail bytes.
    for(int byte=0;byte<4;++byte)for(unsigned value=0;value<256;++value) {
        reset();wanted[1]=byte;
        uint32_t word=0xa35c19e7u;
        word=(word&~(255u<<(8*byte)))|(value<<(8*byte));
        std::memcpy(&OceanCornerShades[15],&word,4);
        verify_call(0,signed_byte(value));
        for(int i=0;i<15;++i)require(OceanCornerShades[i]==int(0x10203000u+unsigned(i)));
    }
    // Every pair of altitude nibbles for every corner, including all indices
    // beyond the known overlap. Non-selected nibbles intentionally differ.
    for(int corner=0;corner<4;++corner)for(int a=0;a<16;++a)for(int b=0;b<16;++b) {
        reset(3,2);
        wanted[0]=3;wanted[1]=8;wanted[2]=12;wanted[3]=5;
        wanted[corner]=a;wanted[(corner+1)%4]=b;
        verify_call(corner,oracle(corner,wanted));
    }
    // Live owner and row width are reloaded after each altitude call. Transient
    // null owners produce zero nibbles; the last call restores a valid owner.
    for(int corner=0;corner<4;++corner)for(int nulls=0;nulls<8;++nulls) {
        reset();mode=1;null_mask=nulls;
        int effective[4];
        for(int i=0;i<4;++i) {
            wanted[i]=i*3+2;effective[i]=(nulls&(1<<i)) ? 0 : wanted[i];
        }
        verify_call(corner,oracle(corner,effective));
        require(window.owned_==banks[4] && MapLongitudeBounds==16);
        require(banks[0][5].brightness[corner]==255);
    }
    for(int corner=0;corner<4;++corner)for(unsigned value=0;value<255;++value) {
        reset();banks[0][5].brightness[corner]=value;
        require(window.get_brighting(2,1,corner)==int(value)-128);
        require(map_calls==0 && alt_calls==0);
    }
    for(int corner=0;corner<4;++corner)for(unsigned climate : {0u,31u,32u,63u,64u,95u}) {
        reset();terrain=climate;
        std::memcpy(before,banks,sizeof banks);
        require(window.get_brighting(2,1,corner)==0);
        require(map_calls==1 && alt_calls==0 && banks[0][5].brightness[corner]==128);
        before[(5*12)+corner]=128;
        require(std::memcmp(before,banks,sizeof banks)==0);
    }
    reset();window.owned_=nullptr;
    require(window.get_brighting(2,1,0)==-128 && !map_calls && !alt_calls);
    const int outside[4][2]={{-1,1},{8,1},{1,-1},{1,4}};
    for(const auto &xy : outside) {
        int x=xy[0],y=xy[1];
        require(window.get_brighting(x,y,0)==0 && !map_calls && !alt_calls);
    }
}
'''


def run_host(tmp_path, actual):
    compiler = shutil.which("clang++") or shutil.which("g++")
    if not compiler:
        pytest.skip("host C++ compiler required")
    source = tmp_path / "shared_shades.cpp"
    source.write_text(HARNESS.replace("ACTUAL_SOURCE", actual))
    binary = tmp_path / "shared_shades"
    compiled = subprocess.run([compiler, "-std=c++11", "-O1", "-fsanitize=address,undefined",
                               "-fno-sanitize-recover=all", str(source), "-o", str(binary)],
                              capture_output=True, text=True, timeout=40)
    assert compiled.returncode == 0, compiled.stderr
    return subprocess.run([str(binary)], capture_output=True, text=True, timeout=15,
                          env=dict(os.environ, ASAN_OPTIONS="detect_leaks=0"))


def test_actual_shared_shade_tail(tmp_path):
    result = run_host(tmp_path, actual_source())
    assert result.returncode == 0, result.stderr


@pytest.mark.parametrize("before,after", [
    ("static int OceanCornerShades[16];", "static int OceanCornerShades[16] = {1};"),
    ("OceanCornerShades[15]", "OceanCornerShades[14]"),
    ("static_cast<unsigned int>(OceanCornerShades[15])",
     "static_cast<unsigned int>([]() { static int shadow[16]; return shadow[15]; }())"),
    ("int row = y;", "OceanCornerShades[15] = 0; int row = y;"),
    ("if (index < 4)", "if (index < 3)"),
    ("if (index < 4)", "if (false)"),
    ("(index * 8)", "((3 - index) * 8)"),
    ("(index * 8)", "(index * 4)"),
    ("static_cast<int>(byte) - 0x100", "static_cast<int>(byte)"),
    ("byte < 0x80U", "byte <= 0x80U"),
    ("v1 * 7 + v2", "v1 * 7 + v3"),
    ("0x31 + v2 * 7 + v3", "0x31 + v1 * 7 + v3"),
    ("0x62 + v3 * 7 + v4", "0x62 + v3 * 7 + v1"),
    ("0x93 + v4 * 7 + v1", "0x93 + v4 * 7 + v2"),
    ("corner_light_value(index)", "corner_light_value(index + 1)"),
    ("if (v0 != 0xff)", "if (v0 != 0xfe)"),
    ("return (int)v0 - 0x80;", "return (int)v0 - 0x7f;"),
    ("(terrain & 0xe0) < 0x60", "(terrain & 0xe0) < 0x40"),
    ("*tileByteAddr = (char)0x80;", "*tileByteAddr = (char)0x81;"),
    ("return -0x80;", "return 0;"),
    ("finalTile[corner] = (char)(result + 0x80);", "finalTile[corner] = (char)result;"),
    ("MapLongitudeBounds >> 1", "8 >> 1"),
    ("v1 = 0;", "v1 = 1;"),
    ("v2 = 0;", "v2 = 1;"),
    ("v3 = 0;", "v3 = 1;"),
    ("get_alt(col, row, 1);", "get_alt(col, row, 2);"),
])
def test_compiled_damage_rejected(tmp_path, before, after):
    actual = actual_source()
    assert before in actual
    result = run_host(tmp_path, actual.replace(before, after))
    assert result.returncode == 98 and "assertion line" in result.stderr, result.stderr
