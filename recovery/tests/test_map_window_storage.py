"""Actual coordinate conversion and shared slot access, with altitude doubles.

Expected coordinates and call order follow original462F00..46303C. This is
not Win32 layout, blitter execution, window lifecycle or whole-game coverage.
"""
from pathlib import Path
import re
import shutil
import subprocess

import pytest

ROOT = Path(__file__).resolve().parents[2]


def body(text, signature):
    start = text.index(signature)
    first = text.index('{', start)
    depth = 0
    for end in range(first, len(text)):
        depth += (text[end] == '{') - (text[end] == '}')
        if not depth:
            return text[start:end + 1]
    raise AssertionError('unterminated actual body')


def source():
    cpp = (ROOT / 'src/mapwin.cpp').read_text()
    header = (ROOT / 'src/mapwin.h').read_text()
    console = (ROOT / 'src/console.h').read_text()
    typedef, = re.findall(r'typedef MapWin \*MapWinSlots\[8\];', header)
    declaration, = re.findall(r'extern MapWinSlots MapWinTable;', header)
    definition, = re.findall(r'^MapWinSlots MapWinTable;', cpp, re.M)
    return '\n'.join((typedef, declaration, definition,
                      body(console, 'inline MapWin *&console_map_win()'),
                      body(cpp, 'void MapWin::tile_to_pixel(')))


HARNESS = r'''
#include <cstdio>
#include <cstdlib>
#include <cstdint>
#include <vector>
void check_at(bool condition, int line) {
    if (!condition) { std::fprintf(stderr,"assertion at %d\n",line); std::exit(91); }
}
#define check(v) check_at((v),__LINE__)
int MapLongitudeBounds = 100, MapIsFlat = 0;
std::vector<int> calls;
int input_x, input_y, altitude_values[5] = {0,1,2,3,4};
bool mutate_members = false;
struct MapWin {
    // FIELDS
    void tile_to_pixel(int,int,int*,int*);
    int get_alt(int x,int y,int corner) {
        check(x==input_x && y==input_y); calls.push_back(corner);
        if (mutate_members && corner==1) {
            field_1DDA8_=29; field_1DDC0_=7; field_1DDB0_=300;
        }
        return altitude_values[corner];
    }
};
// SOURCE
MapWin window;
void reset() {
    window.field_1DDA4_=20; window.field_1DDA8_=30;
    window.field_1DDCC_=10; window.field_1DDD4_=2;
    window.field_1DDD0_=10; window.field_1DDD8_=2;
    window.field_1DDBC_=4; window.field_1DDC0_=6;
    window.field_1DDAC_=100; window.field_1DDB0_=200;
    MapLongitudeBounds=100; MapIsFlat=0; mutate_members=false;
    for(int i=1;i<=4;++i) altitude_values[i]=i;
}
void sample(int x,int y,int expected_x,int expected_y) {
    input_x=x; input_y=y; calls.clear();
    int px=-9999,py=-9999; window.tile_to_pixel(x,y,&px,&py);
    check(px==expected_x && py==expected_y);
    check(calls==std::vector<int>({4,3,2,1}));
}
int main() {
    for(int i=0;i<8;++i) check(MapWinTable[i]==0);
    MapWin another;
    MapWinTable[0]=&window; MapWinTable[1]=&another;
    check(console_map_win()==&window);
    console_map_win()=&another; check(MapWinTable[0]==&another);
    MapWinTable[0]=&window; check(console_map_win()==&window);
    reset(); sample(22,32,104,204);
    reset(); window.field_1DDA4_=90; window.field_1DDCC_=20;
    sample(2,32,144,204); // wraps across the seam into the visible range
    reset(); sample(80,10,-64,72); // outside vertical range, horizontal x not y
    reset(); sample(0,32,16,204); // outside horizontal range
    reset(); window.field_1DDCC_=50; window.field_1DDD4_=10;
    sample(79,32,332,204); sample(80,32,-64,204); // exclusive right edge
    reset(); MapIsFlat=1; sample(80,10,-64,72); // fallback still wraps on flat maps
    reset(); MapIsFlat=1; window.field_1DDCC_=80;
    sample(80,32,336,204); // in-view flat displacement is not nearest-wrap
    reset(); MapIsFlat=1; window.field_1DDA4_=-10; window.field_1DDCC_=80;
    sample(50,32,336,204); // signed negative origin must not force fallback
    reset(); MapIsFlat=1; window.field_1DDA8_=-10; window.field_1DDCC_=80;
    sample(80,-8,336,204); // signed negative row remains visible
    reset(); altitude_values[4]=-7; sample(22,32,104,207); // sum=-1 uses SAR
    reset(); mutate_members=true; sample(22,32,104,312); // post-altitude member reads
    reset(); MapLongitudeBounds=101; sample(71,10,-104,72); // odd-width signed half
    reset(); sample(70,10,296,72); sample(-30,10,-104,72); // +/-half ties unchanged
    return 0;
}
'''


def run(tmp_path, text, damage_fields=False):
    compiler = shutil.which('g++') or shutil.which('clang++')
    if not compiler:
        pytest.skip('host C++ compiler unavailable')
    cpp = tmp_path / 'map_window.cpp'
    exe = tmp_path / 'map_window'
    header = (ROOT / 'src/mapwin.h').read_text()
    fields = []
    for suffix in ('A4','A8','CC','D4','D0','D8','BC','C0','AC','B0'):
        name = 'field_1DD' + suffix + '_'
        declaration, = re.findall(r'(?:int|uint32_t) ' + name + ';', header)
        fields.append(declaration)
    actual_fields = '\n'.join(fields)
    if damage_fields:
        actual_fields = actual_fields.replace('int field_1DDA4_;','uint32_t field_1DDA4_;')
    cpp.write_text(HARNESS.replace('// FIELDS', actual_fields).replace('// SOURCE', text))
    built = subprocess.run([compiler, '-std=c++11', '-O0', str(cpp), '-o', str(exe)],
                           capture_output=True, text=True)
    assert built.returncode == 0, built.stderr
    return subprocess.run([str(exe)], capture_output=True, text=True)


def test_actual_map_window_storage_and_coordinates(tmp_path):
    result = run(tmp_path, source())
    assert result.returncode == 0, result.stderr


def test_unsigned_origin_damage_is_observed(tmp_path):
    result = run(tmp_path, source(), damage_fields=True)
    assert result.returncode == 91, result.stderr


@pytest.mark.parametrize(('old','new'), [
    ('return MapWinTable[0];', 'return MapWinTable[1];'),
    ('relative_x = tile_x - field_1DDA4_;', 'relative_x = tile_y - field_1DDA4_;'),
    ('field_1DDD4_ + field_1DDCC_', '0 + field_1DDCC_'),
    ('tile_y < field_1DDD8_ + field_1DDD0_ + field_1DDA8_', 'false'),
    ('relative_x > map_width / 2', 'relative_x >= map_width / 2'),
    ('relative_x < -(map_width / 2)', 'relative_x <= -(map_width / 2)'),
    ('get_alt(tile_x, tile_y, 4)', 'get_alt(tile_x, tile_y, 1)'),
    ('(altitude >> 2)', '(altitude / 4)'),
    ('(tile_y - field_1DDA8_ - 1) * field_1DDC0_', '(tile_y - 30 - 1) * 6'),
])
def test_coordinate_damage_is_observed(tmp_path, old, new):
    actual = source()
    assert old in actual
    result = run(tmp_path, actual.replace(old,new))
    assert result.returncode == 91, result.stderr
