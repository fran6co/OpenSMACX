"""Actual-source host controls for terrain_fixup; not native layout/runtime proof."""
from pathlib import Path
import shutil
import subprocess

import pytest


ROOT = Path(__file__).resolve().parents[2]
CANDIDATE = ROOT / "src/mapwin.cpp"


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
    source = CANDIDATE.read_text()
    map_header = (ROOT / "src/map.h").read_text()
    mapwin_header = (ROOT / "src/mapwin.h").read_text()
    return (
        extract(mapwin_header, "struct TerrainCacheEntry") + ";",
        extract(map_header, "struct Map") + ";",
        extract(source, "void __cdecl terrain_fixup(MapWin * window)"),
    )


HARNESS = r"""
#include <assert.h>
#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <vector>
#define __cdecl

ACTUAL_CACHE
ACTUAL_MAP

uint32_t MapArea=3;
Map *MapTiles=0;
inline Map *&map_tiles() { return MapTiles; }
struct MapWin { TerrainCacheEntry *owned_; };

static int allocation_calls=0;
static int allocation_bytes=-1;
static void *last_allocation=0;
static std::vector<void *> freed;
void *mem_get(int bytes) {
    ++allocation_calls;
    allocation_bytes=bytes;
    const size_t capacity=bytes > 0 ? static_cast<size_t>(bytes) : 64;
    last_allocation=malloc(capacity);
    if (last_allocation) memset(last_allocation,0xA5,capacity);
    return last_allocation;
}
void host_free(void *pointer) {
    freed.push_back(pointer);
    free(pointer);
}
#define free host_free

ACTUAL_BODY

#undef free
static Map tile_storage[4];

int main() {
    MapTiles=tile_storage;
    tile_storage[0].climate=0xB8;
    tile_storage[1].climate=0x01;
    tile_storage[2].climate=0x42;

    // A null receiver returns before free/allocation, as pinned at 4711AC.
    terrain_fixup(0);
    assert(allocation_calls==0 && freed.empty());

    MapWin window={0};
    TerrainCacheEntry *old=static_cast<TerrainCacheEntry *>(malloc(12));
    assert(old);
    memset(old,0x39,12);
    window.owned_=old;
    MapArea=3;
    terrain_fixup(&window);
    assert(freed.size()==1 && freed[0]==old);
    assert(window.owned_==last_allocation);
    assert(allocation_calls==1 && allocation_bytes==36);
    const unsigned char expected_bits=static_cast<unsigned char>(0xB8 & 0xD0);
    const unsigned char *raw=reinterpret_cast<const unsigned char *>(window.owned_);
    for (int entry=0;entry<3;++entry) {
        for (int byte=0;byte<4;++byte) assert(raw[entry*12+byte]==0xFF);
        for (int byte=4;byte<9;++byte) assert(raw[entry*12+byte]==0xFF);
        assert(raw[entry*12+9]==expected_bits);
        assert(raw[entry*12+10]==0xA5 && raw[entry*12+11]==0xA5);
    }
    host_free(window.owned_);
    window.owned_=0;

    // Zero area still allocates and stores the result, but the signed initial
    // JLE skips every cache write; detail bytes and all poison stay untouched.
    MapArea=0;
    terrain_fixup(&window);
    assert(allocation_calls==2 && allocation_bytes==0);
    assert(window.owned_==last_allocation && window.owned_!=0);
    for (int byte=0;byte<12;++byte)
        assert(reinterpret_cast<unsigned char *>(window.owned_)[byte]==0xA5);
    host_free(window.owned_);
    window.owned_=0;

    // High-bit MapArea wraps allocation bytes to zero but is signed-negative
    // at the original JLE, so it must not enter even one do-while record.
    MapArea=0x80000000u;
    terrain_fixup(&window);
    assert(allocation_calls==3 && allocation_bytes==0);
    for (int byte=0;byte<12;++byte)
        assert(reinterpret_cast<unsigned char *>(window.owned_)[byte]==0xA5);
    host_free(window.owned_);
}
"""


def compile_run(compiler_name, tmp_path, body):
    compiler=shutil.which(compiler_name)
    if not compiler:
        pytest.skip(f"{compiler_name} unavailable")
    cache, map_pod, _actual_body=actual_inputs()
    harness=HARNESS.replace("ACTUAL_CACHE",cache).replace("ACTUAL_MAP",map_pod)
    harness=harness.replace("ACTUAL_BODY",body)
    source=tmp_path/"terrain-fixup.cpp"
    source.write_text(harness)
    binary=tmp_path/"terrain-fixup"
    built=subprocess.run(
        [compiler,"-std=c++11","-Wall","-Wextra","-Werror","-fsanitize=undefined", "-fno-sanitize-recover=all",
         str(source),"-o",str(binary)],capture_output=True,text=True,timeout=30)
    assert built.returncode==0,built.stderr
    run=subprocess.run([str(binary)],capture_output=True,text=True,timeout=10)
    return run


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
def test_actual_terrain_fixup_fields_and_paths(compiler_name, tmp_path):
    run=compile_run(compiler_name,tmp_path,actual_inputs()[2])
    assert run.returncode==0,run.stderr


@pytest.mark.parametrize("before,after",[
    ("if (static_cast<int32_t>(MapArea) > 0)","if (MapArea > 0)"),
    ("brightness[j - 1] = 0xff;",
     "altitude[j - 1] = static_cast<signed char>(0xff);"),
    ("altitude[j - 1] = static_cast<signed char>(0xff);",
     "brightness[j - 1] = 0xff;"),
    ("map_tiles()[0].climate","map_tiles()[1].climate"),
    ("window->owned_[count2].terrain_bits = 0;",
     "window->owned_[count2].terrain_bits = 0;\n                window->owned_[count2].altitude_detail = 0;"),
    ("free(window->owned_);","/* leaked old cache */"),
])
@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
def test_terrain_fixup_mutants_are_observed(compiler_name,tmp_path,before,after):
    body=actual_inputs()[2]
    assert before in body
    run=compile_run(compiler_name,tmp_path,body.replace(before,after,1))
    assert run.returncode!=0,"mutant unexpectedly preserved cache behavior"


def test_signed_guards_live_maparea_reload_and_write_order():
    _cache,_map_pod,body=actual_inputs()
    assert "if (static_cast<int32_t>(MapArea) > 0)" in body
    assert "while (count2 < static_cast<int32_t>(MapArea));" in body
    order=(
        "brightness[j - 1] = 0xff;",
        "altitude[j - 1] = static_cast<signed char>(0xff);",
        "terrain_bits = 0;",
        "const unsigned char first_climate = map_tiles()[0].climate;",
        "terrain_bits |= first_climate & 0xd0;",
        "count2 = count2 + 1;",
        "while (count2 < static_cast<int32_t>(MapArea));",
    )
    positions=[body.index(item) for item in order]
    assert positions==sorted(positions)
    assert "altitude_detail" not in body
    assert "map_tile_bytes" not in body
