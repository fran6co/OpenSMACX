"""Actual-source controls for MapWin's typed tile/cache ownership.

Method bodies are extracted verbatim from product source; legacy alias
normalization is deliberately absent. The host MapWin is only a local fixture,
not a native ABI proof.
"""
from pathlib import Path
import shutil
import subprocess

import pytest


ROOT = Path(__file__).resolve().parents[2]
CANDIDATE = ROOT / "src/mapwin.cpp"
MAP_H = ROOT / "src/map.h"
MAPWIN_H = ROOT / "src/mapwin.h"


def extract(source: str, signature: str) -> str:
    start = source.index(signature)
    opening = source.index("{", start)
    depth = 0
    for pos in range(opening, len(source)):
        depth += (source[pos] == "{") - (source[pos] == "}")
        if depth == 0:
            return source[start:pos + 1]
    raise AssertionError(f"unterminated body: {signature}")


def actual_inputs() -> tuple[str, str, str]:
    candidate = CANDIDATE.read_text()
    mapwin_h = MAPWIN_H.read_text()
    map_h = MAP_H.read_text()
    cache = extract(mapwin_h, "struct TerrainCacheEntry") + ";"
    map_pod = extract(map_h, "struct Map") + ";"
    bodies = "\n\n".join(
        extract(candidate, signature)
        for signature in (
            "void MapWin::on_redraw()",
            "void MapWin::init_dummy()",
            "static int get_alt_ratio(MapWin * w, int count)",
            "int MapWin::get_alt(int a1, int a2, int a3)",
            "int MapWin::get_point_light(int x, int y, int corner)",
        )
    )
    return cache, map_pod, bodies


HARNESS = r"""
#include <assert.h>
#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

ACTUAL_CACHE
ACTUAL_MAP

int MapLongitudeBounds=8, MapLatitudeBounds=4;
uint32_t MapLongitude=4, MapArea=16;
int MapIsFlat=1;
Map *MapTiles=0;
inline Map *&map_tiles() { return MapTiles; }
int MapWinCaptionScale=0, MapWinCaptionDivisor=1;
int32_t AltStepX[16]={2,0,-2,0};
int32_t AltStepY[16]={0,1,0,-1};
unsigned char AltDirectionTable[256]={};
unsigned char AltBonusArid[256]={}, AltBonusMoist[256]={};
unsigned char AltBonusRainy[256]={}, AltBonusFlat[256]={}, AltBonusOcean[256]={};
signed char AltLightBit1[256]={}, AltLightBit2[256]={}, AltLightBit200[256]={};
const int CornerOffsetX[4][4] = {
    {-2,-1,0,-1}, {-1,0,1,0}, {0,1,2,1}, {-1,0,1,0}
};
const int CornerOffsetY[4][4] = {
    {0,-1,0,1}, {-1,-2,-1,0}, {0,-1,0,1}, {1,0,1,2}
};
static int allocation_size=-1;
void *mem_get(int bytes) {
    allocation_size=bytes;
    void *p=malloc(static_cast<size_t>(bytes));
    if (p) memset(p,0xA7,static_cast<size_t>(bytes));
    return p;
}
static int bright_calls=0;
static int bright_values[16];
int get_brighting(int x,int y,int corner) {
    (void)x; (void)y;
    bright_values[bright_calls++]=corner;
    return corner+1;
}

#pragma pack(push,4)
struct MapWin {
    uint32_t vfptr_word;
    TerrainCacheEntry *owned_;
    int field_1DD6C_;
    int field_1DD98_;
    int redraw_calls;
    int redraw_arg;
    MapWin() : vfptr_word(0), owned_(0), field_1DD6C_(25), field_1DD98_(0),
        redraw_calls(0), redraw_arg(-1) {}
    void draw_map(int mode) { ++redraw_calls; redraw_arg=mode; }
    void on_redraw();
    void init_dummy();
    int get_alt(int x,int y,int direction);
    int get_point_light(int x,int y,int corner);
};
#pragma pack(pop)

ACTUAL_BODIES

static_assert(sizeof(Map)==0x2c,"actual map record extent");
static_assert(sizeof(TerrainCacheEntry)==12,"actual cache record extent");
static_assert(offsetof(Map,climate)==0 && offsetof(Map,bit2)==12,"map fields");
static_assert(offsetof(TerrainCacheEntry,brightness)==0,"cache brightness");
static_assert(offsetof(TerrainCacheEntry,altitude)==4,"cache altitude");
static_assert(offsetof(TerrainCacheEntry,terrain_bits)==9,"cache flags");
static_assert(offsetof(TerrainCacheEntry,altitude_detail)==10,"cache details");
static_assert(offsetof(MapWin,owned_)==4,"fixture owner word position");

static Map tile_storage[16];
static TerrainCacheEntry cache[16];
static int tile_index(int x,int y) { return int(MapLongitude)*y+(x>>1); }
static int cache_index(int x,int y) { return (MapLongitudeBounds>>1)*y+(x>>1); }
static void reset_cache() {
    memset(cache,0,sizeof(cache));
    for (int i=0;i<16;++i) {
        for (int slot=0;slot<5;++slot) cache[i].altitude[slot]=-1;
        cache[i].altitude_detail=0xA5A5;
    }
}

int main() {
    MapTiles=tile_storage;
    for (int i=0;i<16;++i) tile_storage[i].climate=0x80;

    // Redraw is gated by the real owned_ pointer member at the measured +4.
    MapWin w;
    w.on_redraw();
    assert(w.redraw_calls==0);
    w.owned_=reinterpret_cast<TerrainCacheEntry *>(static_cast<uintptr_t>(1));
    w.on_redraw();
    assert(w.redraw_calls==1 && w.redraw_arg==1);

    // init_dummy keeps its original 12-byte allocation and partial writes:
    // first-tile climate initializes only terrain_bits; details stay poisoned.
    w.owned_=0;
    MapArea=3;
    tile_storage[0].climate=0xB8;
    w.init_dummy();
    assert(allocation_size==36);
    for (int i=0;i<3;++i) {
        for (int j=0;j<4;++j) assert(w.owned_[i].brightness[j]==0xFF);
        for (int j=0;j<5;++j) assert(w.owned_[i].altitude[j]==-1);
        assert(w.owned_[i].terrain_bits==(0xB8 & 0xD0));
        assert(w.owned_[i].altitude_detail==0xA7A7);
    }
    free(w.owned_);
    w.owned_=0;
    MapArea=0x80000000u;
    w.init_dummy();
    assert(allocation_size==0); // allocation expression wraps before signed compare
    free(w.owned_);
    w.owned_=0;
    MapArea=16;

    // Base direction writes signed altitude slot zero and returns the
    // unclamped integer result, while preserving other cache slots.
    reset_cache();
    w.owned_=cache;
    tile_storage[tile_index(4,1)].climate=0x80;
    assert(w.get_alt(4,1,0)==25);
    assert(cache[cache_index(4,1)].altitude[0]==25);
    for (int j=1;j<5;++j) assert(cache[cache_index(4,1)].altitude[j]==-1);

    // Nonzero direction reaches the four neighbors recursively, stores each
    // neighbor's selected signed altitude slot, and writes its detail nibble.
    reset_cache();
    for (int i=0;i<16;++i) tile_storage[i].climate=0x80;
    assert(w.get_alt(4,1,1)==25);
    const int nx[4]={6,4,2,4}, ny[4]={1,2,1,0};
    const int cache_slot[4]={3,4,1,2};
    const int detail_shift[4]={8,12,0,4};
    for (int i=0;i<4;++i) {
        int entry=cache_index(nx[i],ny[i]);
        assert(cache[entry].altitude[cache_slot[i]]==25);
        assert(((cache[entry].altitude_detail>>detail_shift[i])&0xF)==4);
    }
    assert(cache[cache_index(4,1)].altitude[1]==-1); // center wasn't a neighbor

    // get_point_light reads the named Map::bit2 word and preserves the
    // corner-zero brighting callback order and result.
    memset(AltDirectionTable,0,sizeof(AltDirectionTable));
    tile_storage[tile_index(4,1)].bit2=1;
    AltDirectionTable[0]=3;
    AltLightBit1[3]=9;
    bright_calls=0;
    assert(w.get_point_light(4,1,0)==11);
    assert(bright_calls==4);
    for (int i=0;i<4;++i) assert(bright_values[i]==i);

    // The pinned AltBonusRainy table contains 206 (signed -50). With ratio
    // 25 and base altitude 25, truncation toward zero yields 25-12 == 13.
    reset_cache();
    w.owned_=cache;
    MapWinCaptionScale=0;
    MapWinCaptionDivisor=1;
    w.field_1DD98_=0;
    tile_storage[tile_index(4,1)].climate=0x80;
    tile_storage[tile_index(4,1)].bit2=0x200;
    AltDirectionTable[0]=3;
    AltBonusRainy[3]=206;
    assert(w.get_alt(4,1,0)==13);
    assert(cache[cache_index(4,1)].altitude[0]==13);

    // Every signed table must take the same negative-byte path; Arid below
    // deliberately does not. Use synthetic bytes, not a copied image table.
    unsigned char *signed_tables[] = {
        AltBonusMoist, AltBonusRainy, AltBonusFlat, AltBonusOcean
    };
    const unsigned int signed_flags[] = {1,0x200,0x100,0x800};
    for (int table=0;table<4;++table) {
        reset_cache();
        tile_storage[tile_index(4,1)].bit2=signed_flags[table];
        signed_tables[table][3]=206;
        assert(w.get_alt(4,1,0)==13);
        assert(cache[cache_index(4,1)].altitude[0]==13);
    }

    // Arid is intentionally different: the pin zero-extends this table byte.
    reset_cache();
    tile_storage[tile_index(4,1)].climate=0x80;
    tile_storage[tile_index(4,1)].bit2=2;
    AltBonusArid[3]=206;
    assert(w.get_alt(4,1,0)==76);
    assert(cache[cache_index(4,1)].altitude[0]==76);

    // A positive, high but signed-safe bonus exercises the positive branch.
    reset_cache();
    tile_storage[tile_index(4,1)].bit2=0x200;
    AltBonusRainy[3]=127;
    assert(w.get_alt(4,1,0)==56);
    assert(cache[cache_index(4,1)].altitude[0]==56);

    // Both saturation directions remain the original +/-127 behavior.
    reset_cache();
    MapWinCaptionScale=10000;
    AltBonusRainy[3]=127;
    assert(w.get_alt(4,1,0)==127);
    assert(cache[cache_index(4,1)].altitude[0]==127);

    reset_cache();
    MapWinCaptionScale=10000;
    tile_storage[tile_index(4,1)].climate=0x60; // valid shore, zero base
    AltBonusRainy[3]=206; // signed -50
    assert(w.get_alt(4,1,0)==-127);
    assert(cache[cache_index(4,1)].altitude[0]==-127);
}
"""


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
@pytest.mark.parametrize("damage", (None, "AltBonusMoist", "AltBonusRainy",
                                   "AltBonusFlat", "AltBonusOcean", "AltBonusArid"))
def test_actual_mapwin_typed_closure(compiler_name, damage, tmp_path):
    compiler = shutil.which(compiler_name)
    if not compiler:
        pytest.skip(f"{compiler_name} unavailable")
    cache, map_pod, bodies = actual_inputs()
    if damage == "AltBonusArid":
        before = "bonus = (int)AltBonusArid[byteVal];"
        after = "bonus = static_cast<signed char>(AltBonusArid[byteVal]);"
    elif damage:
        before = f"bonus = static_cast<signed char>({damage}[byteVal]);"
        after = f"bonus = (int){damage}[byteVal];"
    if damage:
        assert bodies.count(before) == 1
        bodies = bodies.replace(before, after, 1)
    # The extracted candidate methods must not retain the raw tile/cache aliases.
    assert "*map_tiles" not in bodies
    assert "*MapRowStride" not in bodies
    assert "char *)this->owned_" not in bodies
    source = tmp_path / "mapwin-typed.cpp"
    source.write_text(HARNESS.replace("ACTUAL_CACHE", cache)
                      .replace("ACTUAL_MAP", map_pod)
                      .replace("ACTUAL_BODIES", bodies))
    binary = tmp_path / "mapwin-typed"
    built = subprocess.run([compiler, "-std=c++11", "-Wall", "-Wextra", "-Werror",
                            str(source), "-o", str(binary)],
                           capture_output=True, text=True, timeout=30)
    assert built.returncode == 0, built.stderr
    run = subprocess.run([str(binary)], capture_output=True, text=True, timeout=10)
    if damage is None:
        assert run.returncode == 0, run.stderr
    else:
        assert run.returncode != 0, "incorrect table extension escaped the control"
        assert "Assertion" in run.stderr or "Assertion" in run.stdout
