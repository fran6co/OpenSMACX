"""Actual-source generation controls for MapWin::gen_tile and MapWin::gen_map.

The two bodies are extracted unchanged from src/mapwin.cpp and compiled against
a mock window hierarchy: Buffer/GraphicWin are host stands-in whose fill is a
recorder, tile_to_pixel is a deterministic transform double, and the five
unrecovered callees plus on_map record their arguments. Expected sequences
follow original 0x00468C20..0x00468CC5 and 0x00469CA0..0x0046A292: the
in-place parameter transform feeding gen_terrain_poly's pre/post pairs, the
exactly-minus-one forward, the null-buffer selection arm, bounds gating, the
three scans' parity bases, seam wrapping, zoom-rectangle membership with and
without the two-tile vehicle/overlay margin, and the fill/base-name/legend/war
gates. Host layout and the mocked callees are not native ABI, blitter or
whole-game evidence; the null-receiver case exercises the conversion arm the
original's calling convention keeps unreachable and is undefined behaviour a
real member call would never perform.
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
    map_h = (ROOT / 'src/map.h').read_text()
    on_map, = re.findall(r'inline BOOL __cdecl on_map\(int x, int y\) \{[^}]*\}',
                         map_h)
    return '\n'.join((on_map,
                      body(cpp, 'void MapWin::gen_tile(int x, int y, int a3, int a4) {'),
                      body(cpp, 'void MapWin::gen_map(int a1, int a2) {')))


def harness(actual_fields):
    return r'''
#include <cstdio>
#include <cstdlib>
#include <cstdint>
#include <vector>
typedef int BOOL;
#define __cdecl
void check_at(bool condition, int line) {
    if (!condition) { std::fprintf(stderr,"assertion at %d\n",line); std::exit(91); }
}
#define check(v) check_at((v),__LINE__)

struct Buffer { int buffer_mark; };
struct GraphicWin : Buffer {
    int fill_calls; int fill_color;
    GraphicWin() : fill_calls(0), fill_color(0) {}
    void fill(int color) { ++fill_calls; fill_color = color; }
};
int MapLongitudeBounds, MapLatitudeBounds, MapIsFlat;
struct Veh { int x, y, a, b, c; };

struct MapWin : GraphicWin {
    // FIELDS
    void tile_to_pixel(int x, int y, int *px, int *py) {
        *px = x * 32 + 1000; *py = y * 16 + 2000;
    }
    void draw_vehicles(int x, int y, int px, int py, int draw_type) {
        vehicles.push_back({x, y, px, py, draw_type});
    }
    void gen_overlays(int x, int y) { overlays.push_back({x, y}); }
    void draw_base_names() { ++base_names; }
    void draw_legend() { ++legends; }
    void draw_wars() { ++wars; }
    void gen_tile(int x, int y, int a3, int a4);
    void gen_map(int a1, int a2);
    std::vector<Veh> vehicles;
    std::vector<Veh> overlays;
    int base_names = 0, legends = 0, wars = 0;
    MapWin() : GraphicWin() {}
};
struct Terrain { int buffer_null, window_is_self, texture, draw_type, minus_one,
                 flags, x, y, px, py; };
std::vector<Terrain> terrain_calls;
int __cdecl gen_terrain_poly(Buffer *buffer, MapWin *window, int texture,
                             int draw_type, int minus_one, int flags, int x,
                             int y, int px, int py) {
    Terrain call = {buffer == 0, window != 0, texture, draw_type, minus_one,
                    flags, x, y, px, py};
    terrain_calls.push_back(call);
    return 0;
}
// SOURCE

void reset(MapWin &w) {
    w.field_1DD70_ = 0; w.field_1DD98_ = 55; w.field_1DDC0_ = 7;
    w.field_1DDA4_ = 0; w.field_1DDA8_ = 0;
    w.field_1DDCC_ = 0; w.field_1DDD0_ = 0; w.field_1DDD4_ = 0;
    w.field_1DDD8_ = 0;
    w.field_21A44_ = 0;
    w.field_21A48_ = w.field_21A4C_ = w.field_21A50_ = w.field_21A54_ = 0;
    w.field_21A58_ = w.field_21A5C_ = w.field_21A60_ = w.field_21A64_ = 0;
    w.fill_calls = 0; w.fill_color = 0;
    w.vehicles.clear(); w.overlays.clear();
    w.base_names = w.legends = w.wars = 0;
    terrain_calls.clear();
    MapLongitudeBounds = 10; MapLatitudeBounds = 10; MapIsFlat = 1;
}

int main() {
    MapWin w;
    // ---- gen_tile: forwarding shape, in-place transform, -1 literal ----
    reset(w);
    w.field_1DD70_ = 0x2000;
    w.gen_tile(3, 4, 9, 1);
    check(terrain_calls.size() == 1);
    {
        const Terrain &t = terrain_calls[0];
        check(t.buffer_null == 0);
        check(t.window_is_self == 1);
        check(t.texture == 55);
        check(t.draw_type == 9);
        check(t.minus_one == -1);
        check(t.flags == 0x2000);
        check(t.x == 3 && t.y == 4);            // pre-transform pair
        check(t.px == 3 * 32 + 1000);           // post-transform pair
        check(t.py == 4 * 16 + 2000);
    }
    check(w.vehicles.size() == 1);
    check(w.vehicles[0].x == 3 && w.vehicles[0].y == 4);
    check(w.vehicles[0].a == 3 * 32 + 1000);
    check(w.vehicles[0].b == 4 * 16 + 2000 - 7);  // py - field_1DDC0_
    check(w.vehicles[0].c == 9);
    check(w.overlays.size() == 1);
    check(w.overlays[0].x == 3 && w.overlays[0].y == 4);

    // ---- gen_tile: bounds gating on the ORIGINAL pair ----
    reset(w);
    w.gen_tile(-1, 4, 9, 1);   // x < 0
    check(terrain_calls.size() == 1 && w.vehicles.empty() && w.overlays.empty());
    reset(w);
    w.gen_tile(10, 4, 9, 1);   // x == MapLongitudeBounds
    check(terrain_calls.size() == 1 && w.vehicles.empty());
    reset(w);
    w.gen_tile(3, -1, 9, 1);   // y < 0
    check(terrain_calls.size() == 1 && w.vehicles.empty());
    reset(w);
    w.gen_tile(3, 10, 9, 1);   // y == MapLatitudeBounds
    check(terrain_calls.size() == 1 && w.vehicles.empty());
    reset(w);
    w.gen_tile(9, 9, 9, 1);    // inside on both edges
    check(terrain_calls.size() == 1 && w.vehicles.size() == 1);
    reset(w);
    w.gen_tile(3, 4, 9, 0);    // fourth argument gates the tail only
    check(terrain_calls.size() == 1 && w.vehicles.empty() && w.overlays.empty());

    // ---- gen_tile: the buffer argument is the window's own Buffer base ----
    // The select's null arm is unreachable from any real member call: the
    // body reads field_1DD70_ through the receiver before it, so a null
    // receiver faults there in the original as well (0x00468C3F). The
    // non-null arm is the conversion this checks.
    reset(w);
    w.gen_tile(3, 4, 9, 1);
    check(terrain_calls[0].buffer_null == 0);
    check(terrain_calls[0].window_is_self == 1);

    // ---- gen_map: terrain scan parity, counts and column step ----
    reset(w);
    MapLatitudeBounds = 3;
    w.field_1DDD0_ = 2; w.field_1DDD8_ = 0;   // rows 0..2
    w.field_1DDD4_ = 2;                        // even rows: two columns
    w.field_1DDCC_ = 3;                        // odd rows: three columns
    w.gen_map(7, 0);
    check(w.fill_calls == 1 && w.fill_color == 0x41);
    check(terrain_calls.size() == 7);
    const int expected_xy[7][2] = {{0,0},{2,0},{1,1},{3,1},{5,1},{0,2},{2,2}};
    for (int i = 0; i < 7; ++i) {
        check(terrain_calls[i].x == expected_xy[i][0]);
        check(terrain_calls[i].y == expected_xy[i][1]);
        check(terrain_calls[i].draw_type == 7);
        check(terrain_calls[i].minus_one == -1);
        check(terrain_calls[i].texture == 55);
        check(terrain_calls[i].flags == 0);
    }
    check(w.base_names == 1);

    // ---- gen_map: seam wrap in both directions, flat bypass ----
    reset(w);
    MapLongitudeBounds = 4; MapIsFlat = 0;
    w.field_1DDA4_ = 3; w.field_1DDD4_ = 3;
    w.gen_map(7, 0);
    check(terrain_calls.size() == 3);
    check(terrain_calls[0].x == 3);   // 3 stays
    check(terrain_calls[1].x == 1);   // 5 wraps to 1
    check(terrain_calls[2].x == 3);   // 7 wraps to 3
    reset(w);
    MapLongitudeBounds = 4; MapIsFlat = 0;
    w.field_1DDA4_ = -3; w.field_1DDD4_ = 2;
    w.gen_map(7, 0);
    check(terrain_calls.size() == 2);
    check(terrain_calls[0].x == 1);   // -3 wraps up to 1
    check(terrain_calls[1].x == 3);   // -1 wraps up to 3
    reset(w);
    MapLongitudeBounds = 4; MapIsFlat = 1;  // flat: raw columns pass through
    w.field_1DDA4_ = 3; w.field_1DDD4_ = 2;
    w.gen_map(7, 0);
    check(terrain_calls.size() == 1);  // 3 stays, 5 exceeds the flat bounds
    check(terrain_calls[0].x == 3);

    // ---- gen_map: a2 != 0 skips fill and terrain, keeps vehicles ----
    reset(w);
    MapLatitudeBounds = 3;
    w.field_1DDD0_ = 2; w.field_1DDD4_ = 1; w.field_1DDCC_ = 1;
    w.gen_map(7, 1);
    check(w.fill_calls == 0);
    check(terrain_calls.empty());
    check(w.vehicles.size() == 3);    // vehicle scan runs for all three rows
    check(w.vehicles[0].y == 0 && w.vehicles[1].y == 1 && w.vehicles[2].y == 2);
    check(w.vehicles[1].x == 1);      // odd row base is one right
    check(w.base_names == 1);

    // ---- gen_map: the 0x82000000 and zoom flags suppress the fill ----
    reset(w);
    w.field_1DD70_ = 0x80000000;
    w.gen_map(7, 0);
    check(w.fill_calls == 0);
    reset(w);
    w.field_1DD70_ = 0x02000000;
    w.gen_map(7, 0);
    check(w.fill_calls == 0);
    reset(w);
    w.field_21A44_ = 1;
    w.gen_map(7, 0);
    check(w.fill_calls == 0);

    // ---- gen_map: zoom rectangles gate terrain without a margin ----
    reset(w);
    MapLatitudeBounds = 1; MapLongitudeBounds = 10;
    w.field_1DDD4_ = 4;                 // columns 0,2,4,6 on row 0
    w.field_21A44_ = 1;
    w.field_21A58_ = 2; w.field_21A5C_ = 0; w.field_21A60_ = 5; w.field_21A64_ = 0;
    w.field_21A48_ = 7; w.field_21A4C_ = 0; w.field_21A50_ = 9; w.field_21A54_ = 0;
    w.gen_map(7, 0);
    check(terrain_calls.size() == 2);   // 2 and 4 in box 1; 6 outside both
    check(terrain_calls[0].x == 2 && terrain_calls[1].x == 4);
    reset(w);
    MapLatitudeBounds = 1; MapLongitudeBounds = 10;
    w.field_1DDD4_ = 4;
    w.field_21A44_ = 1;
    w.field_21A48_ = 7; w.field_21A4C_ = 0; w.field_21A50_ = 9; w.field_21A54_ = 0;
    w.gen_map(7, 0);
    check(terrain_calls.size() == 0);   // box 2 alone misses 0,2,4,6
    // ---- gen_map: a swapped rectangle is an arc only on a round map ----
    reset(w);
    MapLatitudeBounds = 1; MapLongitudeBounds = 10; MapIsFlat = 1;
    w.field_1DDD4_ = 4;
    w.field_21A44_ = 1;
    w.field_21A58_ = 8; w.field_21A5C_ = 0; w.field_21A60_ = 1; w.field_21A64_ = 0;
    w.gen_map(7, 0);
    check(terrain_calls.empty());   // flat: the empty normal range [8,1) wins
    reset(w);
    MapLatitudeBounds = 1; MapLongitudeBounds = 10; MapIsFlat = 0;
    w.field_1DDD4_ = 4;
    w.field_21A44_ = 1;
    w.field_21A58_ = 8; w.field_21A5C_ = 0; w.field_21A60_ = 1; w.field_21A64_ = 0;
    w.gen_map(7, 0);
    check(terrain_calls.size() == 1);   // the arc admits col < 1 and col >= 8
    check(terrain_calls[0].x == 0);

    // ---- gen_map: vehicle/overlay scans take a two-tile margin ----
    reset(w);
    MapLatitudeBounds = 9; MapLongitudeBounds = 20;
    w.field_1DDA8_ = 2; w.field_1DDD0_ = 4;   // rows 2..6
    w.field_1DDA4_ = 6; w.field_1DDD4_ = 1; w.field_1DDCC_ = 1;
    w.field_21A44_ = 1;
    w.field_21A58_ = 8; w.field_21A5C_ = 4; w.field_21A60_ = 8; w.field_21A64_ = 4;
    w.gen_map(7, 0);
    // margin box: rows 2..6, columns [6, 10)
    check(w.vehicles.size() == 5);      // one column on every row 2..6
    check(w.vehicles[0].y == 2 && w.vehicles[4].y == 6);
    check(w.vehicles[0].x == 6);
    check(w.overlays.empty());          // 0x2000 not set
    check(terrain_calls.empty());       // no-margin box misses the columns

    // ---- gen_map: the three tail gates ----
    reset(w);
    MapLatitudeBounds = 9; MapLongitudeBounds = 20;
    w.field_1DDA8_ = 2; w.field_1DDD0_ = 4;
    w.field_1DDA4_ = 6; w.field_1DDD4_ = 1; w.field_1DDCC_ = 1;
    w.field_21A44_ = 1;
    w.field_21A58_ = 8; w.field_21A5C_ = 4; w.field_21A60_ = 8; w.field_21A64_ = 4;
    w.field_1DD70_ = 0x2000;
    w.gen_map(7, 0);
    check(w.overlays.size() == 5);      // the overlay scan takes the margin too
    check(w.overlays[0].y == 2 && w.overlays[4].y == 6 && w.overlays[0].x == 6);
    check(w.vehicles.size() == 5);      // the unflagged vehicle scan drew them
    reset(w);
    w.field_1DD70_ = 0x2000;
    w.gen_map(7, 0);
    check(w.overlays.size() == 0);      // empty rectangle admits nothing
    reset(w);
    w.field_1DD70_ = 0x20000;
    w.gen_map(7, 0);
    check(w.legends == 1 && w.wars == 0 && w.overlays.empty());
    reset(w);
    w.field_1DD70_ = 0x100000;
    w.gen_map(7, 0);
    check(w.wars == 1 && w.legends == 0);
    reset(w);
    w.field_1DD70_ = 0x122000;
    w.gen_map(7, 0);
    check(w.legends == 1 && w.wars == 1 && w.overlays.empty());
    return 0;
}
'''.replace('    // FIELDS', actual_fields).replace('// SOURCE', source())


def run(tmp_path, text):
    compiler = shutil.which('g++') or shutil.which('clang++')
    if not compiler:
        pytest.skip('host C++ compiler unavailable')
    cpp = tmp_path / 'mapwin_generation.cpp'
    exe = tmp_path / 'mapwin_generation'
    cpp.write_text(text)
    built = subprocess.run([compiler, '-std=c++11', '-O0', str(cpp), '-o', str(exe)],
                           capture_output=True, text=True)
    assert built.returncode == 0, built.stderr
    return subprocess.run([str(exe)], capture_output=True, text=True)


def fields():
    header = (ROOT / 'src/mapwin.h').read_text()
    names = ['1DD70', '1DD98', '1DDC0', '1DDA4', '1DDA8', '1DDCC', '1DDD0',
             '1DDD4', '1DDD8', '21A44', '21A48', '21A4C', '21A50', '21A54',
             '21A58', '21A5C', '21A60', '21A64']
    out = []
    for n in names:
        name = 'field_' + n + '_'
        declaration, = re.findall(r'(?:int|uint32_t) ' + name + ';', header)
        out.append(declaration)
    return '\n    '.join(out)


def test_actual_generation_bodies(tmp_path):
    result = run(tmp_path, harness(fields()))
    assert result.returncode == 0, result.stderr


@pytest.mark.parametrize(('old', 'new'), [
    ('gen_terrain_poly(this, this, field_1DD98_, a3, -1, field_1DD70_,',
     'gen_terrain_poly(this, this, field_1DD98_, a3, -2, field_1DD70_,'),
    ('tile_to_pixel(x, y, &x, &y);',
     'tile_to_pixel(y, x, &x, &y);'),
    ('tile_x, tile_y, x, y);', 'tile_x, tile_y, y, x);'),
    ('draw_vehicles(tile_x, tile_y, x, y - field_1DDC0_, a3);',
     'draw_vehicles(tile_x, tile_y, x, y, a3);'),
    ('tile_y >= 0 && tile_y < MapLatitudeBounds',
     'tile_y >= 0 && tile_y <= MapLatitudeBounds'),
    ('gen_overlays(tile_x, tile_y);', 'gen_overlays(tile_y, tile_x);'),
    ('int col = field_1DDA4_ + (i & 1);', 'int col = field_1DDA4_;'),
    ('const int count = (i & 1) ? field_1DDCC_ : field_1DDD4_;',
     'const int count = (i & 1) ? field_1DDD4_ : field_1DDCC_;'),
    ('n--, col += 2) {', 'n--, col += 1) {'),
    ('wrapped += MapLongitudeBounds;', 'wrapped += MapLongitudeBounds + 1;'),
    ('wrapped -= MapLongitudeBounds;', 'wrapped -= MapLongitudeBounds + 1;'),
    ('const int flat = MapIsFlat & 1;', 'const int flat = 0;'),
    ('GraphicWin::fill(0x41);', 'GraphicWin::fill(0x42);'),
    ('(field_1DD70_ & 0x82000000) == 0', '(field_1DD70_ & 0x82000000) != 0'),
    ('const int left1 = field_21A58_ - 2;', 'const int left1 = field_21A58_;'),
    ('const int bottom1 = field_21A64_ + 2;',
     'const int bottom1 = field_21A64_;'),
    ('if ((field_1DD70_ & 0x2000) != 0) {',
     'if ((field_1DD70_ & 0x4000) != 0) {'),
    ('if ((field_1DD70_ & 0x20000) != 0) {',
     'if ((field_1DD70_ & 0x40000) != 0) {'),
    ('if ((field_1DD70_ & 0x100000) != 0) {',
     'if ((field_1DD70_ & 0x200000) != 0) {'),
    ('wrapped >= (int)field_21A58_ ||',
     'wrapped >= (int)field_21A58_ &&'),
])
def test_generation_damage_is_observed(tmp_path, old, new):
    text = harness(fields())
    assert text.count(old) >= 1, old
    result = run(tmp_path, text.replace(old, new))
    assert result.returncode == 91, result.stderr
