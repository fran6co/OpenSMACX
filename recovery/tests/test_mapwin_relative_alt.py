"""Actual-source reader controls; host tests do not prove native layout/runtime."""
from pathlib import Path
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
    raise AssertionError("unterminated product definition")


HARNESS = r'''
#include <cassert>
#include <cstdint>
#include <cstring>
#include <climits>
#include <cstddef>
ACTUAL_RECORD
int MapLongitudeBounds = 16;
uint32_t MapLongitude = 8;
struct MapWin {
    TerrainCacheEntry *owned_;
    int get_relative_alt(int, int, int);
    int get_alt(int, int, int);
};
static TerrainCacheEntry records[40];
static int calls, last_x, last_y, last_detail;
static TerrainCacheEntry *replacement;
static int replacement_width;
int MapWin::get_alt(int x, int y, int detail) {
    ++calls; last_x=x; last_y=y; last_detail=detail;
    owned_=replacement;
    MapLongitudeBounds=replacement_width;
    return -77; // reader ignores altitude result but preserves its effects
}
ACTUAL_BODY
int main() {
    static_assert(sizeof(TerrainCacheEntry)==12,"actual cache stride");
    static_assert(offsetof(TerrainCacheEntry,altitude_detail)==10,"actual detail offset");
    MapWin w={records};
    replacement=0; replacement_width=16;
    assert(w.get_relative_alt(4,2,1)==0);
    assert(calls==1 && last_x==4 && last_y==2 && last_detail==1);
    replacement=records;
    assert(w.get_relative_alt(4,2,0)==4);
    assert(calls==2 && last_detail==0);
    for (int i=0;i<40;++i) records[i].altitude_detail=0xEEEE;
    records[18].altitude_detail=0x4321; // (16/2)*2+(4/2), not half-width again
    for (int detail=1;detail<=4;++detail)
        assert(w.get_relative_alt(4,2,detail)==detail);
    // Original shift count is masked to five bits, not a detail range guard.
    const int details[]={-1,-8,5,8,9,10,INT_MIN,INT_MAX};
    for (unsigned i=0;i<sizeof(details)/sizeof(details[0]);++i) {
        const int detail=details[i];
        const unsigned shift=((static_cast<uint32_t>(detail)-1U)*4U)%32U;
        assert(w.get_relative_alt(4,2,detail)==static_cast<int>((0x4321U>>shift)&15U));
    }
    // Width and owner must be read after the get_alt side effects.
    TerrainCacheEntry other[40]={};
    other[14].altitude_detail=0x9876; // (12/2)*2+2
    replacement=other; replacement_width=12;
    w.owned_=records; MapLongitudeBounds=16;
    assert(w.get_relative_alt(4,2,1)==6);
    assert(w.owned_==other && MapLongitudeBounds==12);
    // A centered fixture permits valid negative-index access without adding
    // guards absent from the original. Arithmetic half rounds down.
    replacement=records+20; replacement_width=16;
    records[11].altitude_detail=0xABCD; // center + (8*-1) + (-1>>1)
    assert(w.get_relative_alt(-1,-1,1)==13);
    // Deliberate 32-bit product wrap yields signed row -1, but the centered
    // backing allocation keeps the resulting access valid in this control.
    replacement=records+1; replacement_width=0x1FFFFFFF;
    records[0].altitude_detail=0x5678;
    assert(w.get_relative_alt(30,16,1)==8);
    return 0;
}
'''


def run(compiler_name, tmp_path, mutation=None):
    compiler = shutil.which(compiler_name)
    if not compiler:
        pytest.skip(f"{compiler_name} unavailable")
    record = braced((ROOT / "src/mapwin.h").read_text(), "struct TerrainCacheEntry") + ";"
    body = braced((ROOT / "src/mapwin.cpp").read_text(), "int MapWin::get_relative_alt(int x, int y, int detail)")
    if mutation:
        old, new = mutation
        assert old in body
        body = body.replace(old, new, 1)
    source = tmp_path / "reader.cpp"
    source.write_text(HARNESS.replace("ACTUAL_RECORD", record).replace("ACTUAL_BODY", body))
    binary = tmp_path / "reader"
    compiled = subprocess.run([compiler, "-std=c++11", "-Wall", "-Wextra", "-Werror",
                               "-fsanitize=undefined", "-fno-sanitize-recover=all",
                               str(source), "-o", str(binary)], capture_output=True, text=True, timeout=30)
    assert compiled.returncode == 0, compiled.stderr
    return subprocess.run([str(binary)], capture_output=True, text=True, timeout=10)


@pytest.mark.parametrize("compiler", ("g++", "clang++"))
def test_actual_reader_uses_original_stride_and_detail(compiler, tmp_path):
    result = run(compiler, tmp_path)
    assert result.returncode == 0, result.stderr


@pytest.mark.parametrize("compiler", ("g++", "clang++"))
@pytest.mark.parametrize("mutation", (
    ("MapLongitudeBounds >> 1", "static_cast<int>(MapLongitude) >> 1"),
    ("owned_[row].altitude_detail", "owned_[row + 1].altitude_detail"),
    ("& 31U", "& 15U"),
    ("get_alt(x, y, detail);", "/* missing required effects */"),
    ("return 4;", "return 0;"),
))
def test_reader_damage_controls(compiler, tmp_path, mutation):
    result = run(compiler, tmp_path, mutation)
    assert result.returncode != 0, "damaged reader unexpectedly passed"
