"""Actual-source controls for calculate_dimensions cache invalidation.

The host harness extracts the product's invalidation block verbatim and gives
it only the actual cache POD plus the fields/globals it consumes. It does not
reimplement the product loops.
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
    for pos in range(opening, len(source)):
        depth += (source[pos] == "{") - (source[pos] == "}")
        if depth == 0:
            return source[start:pos + 1]
    raise AssertionError(f"unterminated body: {signature}")


def actual_parts():
    header = (ROOT / "src/mapwin.h").read_text()
    pod = braced(header, "struct TerrainCacheEntry") + ";"
    source = (ROOT / "src/mapwin.cpp").read_text()
    body = braced(source, "void MapWin::calculate_dimensions()")
    begin = body.index("int32_t cache_area = 0;")
    end = body.index("field_1EEC0_ = field_1DD98_;", begin) + len(
        "field_1EEC0_ = field_1DD98_;"
    )
    # The extracted prefix ends before font setup within the second trigger's
    # enclosing if. Close that harness scope; do not substitute any cache code.
    return pod, body[begin:end] + "\n}", body


HARNESS = r'''#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <type_traits>
void require(bool ok,int line){if(!ok){fprintf(stderr,"line %d\n",line);exit(91);}}
#define CHECK(x) require((x),__LINE__)
ACTUAL_POD
uint32_t MapArea;
struct MapWin {
    TerrainCacheEntry *owned_;
    int32_t field_1DD6C_, field_1EEC4_;
    int32_t field_1DD98_, field_1EEC0_;
};
void invalidate(MapWin *self) {
    TerrainCacheEntry *owned_ = self->owned_;
    int32_t field_1DD6C_ = self->field_1DD6C_;
    int32_t field_1EEC4_ = self->field_1EEC4_;
    int32_t field_1DD98_ = self->field_1DD98_;
    int32_t field_1EEC0_ = self->field_1EEC0_;
    ACTUAL_INVALIDATION
    self->field_1EEC4_ = field_1EEC4_;
    self->field_1EEC0_ = field_1EEC0_;
}
static_assert(sizeof(TerrainCacheEntry)==12,"cache record size");
static_assert(offsetof(TerrainCacheEntry,brightness)==0,"brightness offset");
static_assert(offsetof(TerrainCacheEntry,altitude)==4,"altitude offset");
static_assert(offsetof(TerrainCacheEntry,terrain_bits)==9,"flags offset");
static_assert(offsetof(TerrainCacheEntry,altitude_detail)==10,"detail offset");
int main(){
    TerrainCacheEntry cells[4];
    unsigned char expected[sizeof cells];
    memset(cells,0x35,sizeof cells);
    MapWin w={cells,8,0,0,0};
    MapArea=3;
    memset(expected,0x35,sizeof expected);
    for(int i=0;i<3;++i) for(int j=4;j<9;++j) expected[i*12+j]=0xff;
    invalidate(&w);
    CHECK(memcmp(cells,expected,sizeof cells)==0);
    CHECK(w.field_1EEC4_==8 && w.field_1EEC0_==0);

    // Both independently-triggered paths: brightness + altitude only;
    // bytes 9..11 retain poisoned terrain/detail state.
    memset(cells,0x5a,sizeof cells);
    memset(expected,0x5a,sizeof expected);
    w.field_1DD6C_=9; w.field_1EEC4_=8;
    w.field_1DD98_=13; w.field_1EEC0_=12;
    for(int i=0;i<3;++i) for(int j=0;j<9;++j) expected[i*12+j]=0xff;
    invalidate(&w);
    CHECK(memcmp(cells,expected,sizeof cells)==0);
    CHECK(w.field_1EEC4_==9 && w.field_1EEC0_==13);

    // Zero and signed-negative MapArea take the original signed-loop no-write
    // path; an invalid pointer makes an accidental store observable.
    w.owned_=cells; w.field_1DD6C_=10; w.field_1EEC4_=9;
    memset(cells,0x6b,sizeof cells); memcpy(expected,cells,sizeof cells);
    MapArea=0; invalidate(&w); CHECK(memcmp(cells,expected,sizeof cells)==0);
    w.owned_=reinterpret_cast<TerrainCacheEntry *>(static_cast<uintptr_t>(1));
    w.field_1DD6C_=11; w.field_1DD98_=14;
    MapArea=0x80000000u; invalidate(&w);
    w.owned_=cells;
    return 0;
}
'''


def run_host(tmp_path, pod, block):
    compiler = shutil.which("clang++") or shutil.which("g++")
    if not compiler:
        pytest.skip("host C++ compiler required")
    source = tmp_path / "dimensions-cache.cpp"
    source.write_text(HARNESS.replace("ACTUAL_POD", pod).replace(
        "ACTUAL_INVALIDATION", block
    ))
    binary = tmp_path / "dimensions-cache"
    built = subprocess.run(
        [compiler, "-std=c++11", "-Wall", "-Wextra", "-Werror",
         "-fsanitize=address,undefined", "-fno-sanitize-recover=all",
         str(source), "-o", str(binary)], capture_output=True, text=True,
        timeout=30,
    )
    assert built.returncode == 0, built.stderr
    return subprocess.run([str(binary)], capture_output=True, text=True,
                          timeout=10,
                          env=dict(os.environ, ASAN_OPTIONS="detect_leaks=0"))


def test_actual_invalidation_body_and_typed_fields(tmp_path):
    pod, block, body = actual_parts()
    assert "owned_[ia].altitude[ja]" in block
    assert "owned_[ib].brightness[jb]" in block
    assert "owned_[ic].altitude[jc]" in block
    assert "reinterpret_cast<char *>(owned_)" not in block
    # The original has an initial count test and reloads after each row in all
    # three passes. Keep these explicit; a single saved bound is not equivalent.
    assert len(re.findall(r"cache_area\s*=\s*static_cast<int32_t>\(MapArea\)", block)) == 6
    assert body.index("field_1EEC0_ = field_1DD98_;") < body.index("font_a_.init(")
    result = run_host(tmp_path, pod, block)
    assert result.returncode == 0, result.stderr


def test_snapshot_bound_mutation_removes_required_live_reads():
    _, block, _ = actual_parts()
    # A common but incorrect rewrite hoists one bound and reuses it. Mutation
    # control protects the live count reload contract without faking callbacks
    # or introducing a data race into the host fixture.
    mutant = re.sub(r"cache_area\s*=\s*static_cast<int32_t>\(MapArea\);",
                    "cache_area = 3;", block)
    assert mutant != block
    assert len(re.findall(r"cache_area\s*=\s*static_cast<int32_t>\(MapArea\)", mutant)) == 0
