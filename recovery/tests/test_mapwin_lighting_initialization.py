"""Actual lighting lifecycle controls, not native ABI or full storage proof."""
from pathlib import Path
import os
import re
import shutil
import subprocess

import pytest

ROOT = Path(__file__).resolve().parents[2]


def braced(text, signature):
    start = text.index(signature)
    opening = text.index('{', start)
    depth = 0
    for end in range(opening, len(text)):
        depth += (text[end] == '{') - (text[end] == '}')
        if depth == 0:
            return text[start:end + 1]
    raise AssertionError(signature)


def actual_fragment():
    source = (ROOT / 'src/mapwin.cpp').read_text()
    header = (ROOT / 'src/mapwin.h').read_text()
    table, = re.findall(r'signed char MapLightingTable\[4 \* 49\];', source)
    ocean, = re.findall(r'static int OceanCornerShades\[16\];', source)
    alt_light, = re.findall(r'static signed char AltLightBit200\[113\];', source)
    leading, = re.findall(r'static signed char MapLightingLeadingReadWindow\[4\];', source)
    trailing, = re.findall(r'static signed char MapLightingTrailingReadWindow\[4\];', source)
    pod = braced(header, 'struct TerrainCacheEntry {') + ';'
    declarations = '\n'.join(
        re.search(r'\b(?:static\s+)?(?:int|void) ' + name + r'\([^;{}]*\);', header).group()
        for name in ('get_alt', 'compute_lighting_table', 'get_brighting'))
    return '\n'.join((
        pod, ocean, table, alt_light, leading, trailing,
        braced(source, 'static __forceinline signed char corner_light_value('),
        'struct MapWin { TerrainCacheEntry *owned_; ' + declarations + ' };',
        braced(source, 'static __forceinline TerrainCacheEntry *brighting_tile('),
        braced(source, 'int MapWin::get_brighting('),
        braced(source, 'void MapWin::compute_lighting_table()'),
    ))


HARNESS = r'''
#include <cassert>
#include <cstdint>
#include <cstring>
#define __forceinline inline
int MapLongitudeBounds=8, MapLatitudeBounds=4;
unsigned char TerrainByte=0x60;
int RootCalls=0;
int quick_root(int n) { ++RootCalls; int r=0; while ((r+1)*(r+1)<=n) ++r; return r; }
unsigned char *map_loc(int x,int y) { assert(x==2 && y==1); return &TerrainByte; }
int nibble[4]={1,2,3,4};
ACTUAL
TerrainCacheEntry cache[8];
int MapWin::get_alt(int x,int y,int corner) {
    assert(x==2 && y==1 && corner>=1 && corner<=4);
    unsigned short word=cache[5].altitude_detail;
    const int shift=4*(corner-1);
    word=static_cast<unsigned short>((word & ~(15u<<shift)) | (nibble[corner-1]<<shift));
    cache[5].altitude_detail=word;
    return -999; // original consumes the cache rather than this return
}
int main() {
    MapWin w; w.owned_=cache;
    for(int i=0;i<196;++i) assert(MapLightingTable[i]==0);
    for(int i=0;i<4;++i) {
        assert(MapLightingLeadingReadWindow[i]==0);
        assert(MapLightingTrailingReadWindow[i]==0);
    }
    cache[5].brightness[1]=0xff;
    assert(w.get_brighting(2,1,1)==0 && RootCalls==0);
    for(int i=0;i<196;++i) assert(MapLightingTable[i]==0);

    cache[5].brightness[1]=0xff;
    for(int i=0;i<196;++i) MapLightingTable[i]=42;
    const int before=MapLightingTable[58];
    int value=w.get_brighting(2,1,1);
    assert(value==before && RootCalls==0);
    for(int i=0;i<196;++i) assert(MapLightingTable[i]==42);
    assert(cache[5].brightness[1]==static_cast<unsigned char>(value+128));

    MapWin::compute_lighting_table();
    assert(RootCalls==196 && MapLightingTable[58]!=before);
    // Independent quarter-turn matrices check every initialized table cell.
    const int rotation[4][4]={{1,0,0,1},{0,-1,1,0},{-1,0,0,-1},{0,1,-1,0}};
    for(int q=0;q<4;++q) for(int r=-3;r<=3;++r) for(int c=-3;c<=3;++c) {
        int x=100*(rotation[q][0]*r+rotation[q][1]*c);
        int y=100*(rotation[q][2]*r+rotation[q][3]*c);
        int square=x*x+y*y, root=0;
        while((root+1)*(root+1)<=square) ++root;
        int expected=-3*(x+y);
        if(root) expected/=root;
        assert(static_cast<int>(MapLightingTable[q*49+(r+3)*7+c+3])==expected);
    }
    cache[5].brightness[1]=0xff;
    value=w.get_brighting(2,1,1);
    assert(value==static_cast<int>(MapLightingTable[58]) && RootCalls==196);
    MapWin::compute_lighting_table(); // explicit calls are not suppressed by a flag
    assert(RootCalls==392);
}
'''


def run_control(tmp_path, compiler_name, fragment):
    compiler = shutil.which(compiler_name)
    if not compiler:
        pytest.skip(f'{compiler_name} unavailable')
    cpp = tmp_path / 'lighting.cpp'
    binary = tmp_path / 'lighting'
    cpp.write_text(HARNESS.replace('ACTUAL', fragment))
    # GCC does not prove the exhaustive 0..3 loop invariant for the original
    # no-default switch. Retain its diagnostic while executing all 196 cells
    # under UBSan and the independent rotation oracle.
    warning_policy = ['-Wno-error=maybe-uninitialized'] if compiler_name == 'g++' else []
    built = subprocess.run(
        [compiler, '-std=c++11', '-Wall', '-Wextra', '-Werror', '-O1',
         *warning_policy, '-fsanitize=undefined', '-fno-sanitize-recover=all', str(cpp), '-o', str(binary)],
        capture_output=True, text=True, timeout=40)
    assert built.returncode == 0, built.stderr
    return subprocess.run([str(binary)], capture_output=True, text=True, timeout=10,
                          env=dict(os.environ))


@pytest.mark.parametrize('compiler', ('g++', 'clang++'))
def test_lookup_preserves_current_table_until_explicit_compute(tmp_path, compiler):
    result = run_control(tmp_path, compiler, actual_fragment())
    assert result.returncode == 0, result.stderr


@pytest.mark.parametrize('compiler', ('g++', 'clang++'))
def test_lazy_fill_damage_control(tmp_path, compiler):
    fragment = actual_fragment()
    anchor = 'int row = y;'
    assert fragment.count(anchor) == 1
    mutant = fragment.replace(anchor, anchor + '\n    compute_lighting_table();')
    result = run_control(tmp_path, compiler, mutant)
    assert result.returncode != 0, 'added lookup-time table writes passed'


@pytest.mark.parametrize('compiler', ('g++', 'clang++'))
def test_quarter_turn_sign_damage_is_rejected(tmp_path, compiler):
    fragment = actual_fragment()
    before = 'case 3:\n                    x = column;'
    assert fragment.count(before) == 1
    result = run_control(tmp_path, compiler, fragment.replace(
        before, 'case 3:\n                    x = -column;'))
    assert result.returncode != 0
