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
import os
import re
import shutil
import subprocess
from functools import lru_cache

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

// TILE_DAMAGE_TYPE

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
    w.horizontal_damage_.left = w.horizontal_damage_.top = w.horizontal_damage_.right = w.horizontal_damage_.bottom = 0;
    w.vertical_damage_.left = w.vertical_damage_.top = w.vertical_damage_.right = w.vertical_damage_.bottom = 0;
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
    w.vertical_damage_.left = 2; w.vertical_damage_.top = 0; w.vertical_damage_.right = 5; w.vertical_damage_.bottom = 0;
    w.horizontal_damage_.left = 7; w.horizontal_damage_.top = 0; w.horizontal_damage_.right = 9; w.horizontal_damage_.bottom = 0;
    w.gen_map(7, 0);
    check(terrain_calls.size() == 2);   // 2 and 4 in box 1; 6 outside both
    check(terrain_calls[0].x == 2 && terrain_calls[1].x == 4);
    reset(w);
    MapLatitudeBounds = 1; MapLongitudeBounds = 10;
    w.field_1DDD4_ = 4;
    w.field_21A44_ = 1;
    w.horizontal_damage_.left = 7; w.horizontal_damage_.top = 0; w.horizontal_damage_.right = 9; w.horizontal_damage_.bottom = 0;
    w.gen_map(7, 0);
    check(terrain_calls.size() == 0);   // box 2 alone misses 0,2,4,6
    // ---- gen_map: a swapped rectangle is an arc only on a round map ----
    reset(w);
    MapLatitudeBounds = 1; MapLongitudeBounds = 10; MapIsFlat = 1;
    w.field_1DDD4_ = 4;
    w.field_21A44_ = 1;
    w.vertical_damage_.left = 8; w.vertical_damage_.top = 0; w.vertical_damage_.right = 1; w.vertical_damage_.bottom = 0;
    w.gen_map(7, 0);
    check(terrain_calls.empty());   // flat: the empty normal range [8,1) wins
    reset(w);
    MapLatitudeBounds = 1; MapLongitudeBounds = 10; MapIsFlat = 0;
    w.field_1DDD4_ = 4;
    w.field_21A44_ = 1;
    w.vertical_damage_.left = 8; w.vertical_damage_.top = 0; w.vertical_damage_.right = 1; w.vertical_damage_.bottom = 0;
    w.gen_map(7, 0);
    check(terrain_calls.size() == 1);   // the arc admits col < 1 and col >= 8
    check(terrain_calls[0].x == 0);

    // ---- gen_map: vehicle/overlay scans take a two-tile margin ----
    reset(w);
    MapLatitudeBounds = 9; MapLongitudeBounds = 20;
    w.field_1DDA8_ = 2; w.field_1DDD0_ = 4;   // rows 2..6
    w.field_1DDA4_ = 6; w.field_1DDD4_ = 1; w.field_1DDCC_ = 1;
    w.field_21A44_ = 1;
    w.vertical_damage_.left = 8; w.vertical_damage_.top = 4; w.vertical_damage_.right = 8; w.vertical_damage_.bottom = 4;
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
    w.vertical_damage_.left = 8; w.vertical_damage_.top = 4; w.vertical_damage_.right = 8; w.vertical_damage_.bottom = 4;
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
'''.replace('    // FIELDS', actual_fields).replace('// SOURCE', source()).replace(
    '// TILE_DAMAGE_TYPE', body((ROOT / 'src/mapwin.h').read_text(),
                                'struct MapTileDamageRect') + ';')


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
             '1DDD4', '1DDD8', '21A44']
    out = []
    for n in names:
        name = 'field_' + n + '_'
        declaration, = re.findall(r'(?:int|int32_t|uint32_t) ' + name + ';', header)
        out.append(declaration)
    for name in ('horizontal_damage_', 'vertical_damage_'):
        declaration, = re.findall(r'MapTileDamageRect ' + name + ';', header)
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
    ('const int left1 = vertical_damage_.left - 2;', 'const int left1 = vertical_damage_.left;'),
    ('const int bottom1 = vertical_damage_.bottom + 2;',
     'const int bottom1 = vertical_damage_.bottom;'),
    ('if ((field_1DD70_ & 0x2000) != 0) {',
     'if ((field_1DD70_ & 0x4000) != 0) {'),
    ('if ((field_1DD70_ & 0x20000) != 0) {',
     'if ((field_1DD70_ & 0x40000) != 0) {'),
    ('if ((field_1DD70_ & 0x100000) != 0) {',
     'if ((field_1DD70_ & 0x200000) != 0) {'),
    ('wrapped >= (int)vertical_damage_.left ||',
     'wrapped >= (int)vertical_damage_.left &&'),
])
def test_generation_damage_is_observed(tmp_path, old, new):
    text = harness(fields())
    assert text.count(old) >= 1, old
    result = run(tmp_path, text.replace(old, new))
    assert result.returncode == 91, result.stderr


# ---------------------------------------------------------------------------
# gen_terrain_poly argument/receiver controls (journal 56). The body is
# extracted unchanged from src/mapwin.cpp and compiled against host mocks:
# every g_*_TEXTURES / g_TER1_* receiver family records the receiver and
# index each blitter call selected, the fixed-storage bindings above the
# body are re-bound to harness storage, and Buffer::line/Texture::draw2
# use recording member mocks. Host mocks are not ABI, byte or
# whole-game evidence. Distinct synthetic coordinate/UV tables check four
# three-vertex land fans, sixteen ocean vertices, rotated coast quads,
# light-array strides, all 53 coast-remap bytes and landmark rotations.
# Receiver checks cover land/moisture/farm/rockiness, landmarks, fungus,
# jungle (the dead a5 diagonal slot versus the live 8-neighbour mask),
# road/magtube edges and base selection, river, selected improvements,
# wrecks and grid overlays.
# The work scan checks both missing-owner routes for factions 1..7 and
# asymmetric ordinary treaty rows/columns, with actual Veh declarations
# and original-address offset assertions. Field controls distinguish visibility,
# faction views, signed territory, observer -1, rules/preferences and landmark
# sequence. Sensor borders, crop and bonus gates have separate observations.
# Its remaining gates, actual blitter
# pixels, original global storage and unexercised branches remain outside
# these host controls. ASan/UBSan
# inspect the unchanged body and copies of the supplied argument arrays;
# permissive compilation and source substitutions are deliberately absent.
# Clamp controls below observe publication/exit timing through this existing
# service projection, not ownership or real Texture clamping. Shared native
# references (including pending 0x621870) remain separate closure obligations.
TERRAIN_SOURCE_MARK = 'int __cdecl gen_terrain_poly(Buffer *buffer'


def terrain_source():
    cpp = (ROOT / 'src/mapwin.cpp').read_text()
    return '\n'.join((body(cpp, 'static bool terrain_missing_owner_treaty_bit('),
                      body(cpp, TERRAIN_SOURCE_MARK)))


@pytest.mark.parametrize('mutate', [False, True])
def test_actual_sprite_palette_key_member_access(tmp_path, mutate):
    header = (ROOT / 'src/sprite.h').read_text()
    accessor = body(header, 'unsigned char transparent_index() const')
    member, = re.findall(r'unsigned char cTransparentIndex_;', header)
    helper = body((ROOT / 'src/mapwin.cpp').read_text(),
                  'static inline unsigned char sprite_transparent(')
    if mutate:
        accessor = accessor.replace('return cTransparentIndex_;', 'return 0;')
    program = r'''
#include <cstdlib>
class Sprite {
    MEMBER
public:
    explicit Sprite(unsigned char value) : cTransparentIndex_(value) {}
    ACCESSOR
};
HELPER
int main() {
    for (int i = 0; i != 256; ++i) {
        const Sprite sprite(static_cast<unsigned char>(i));
        if (sprite_transparent(sprite) != i) return 91;
    }
}
'''.replace('MEMBER', member).replace('ACCESSOR', accessor).replace('HELPER', helper)
    result = terrain_run(tmp_path, program)
    assert result.returncode == (91 if mutate else 0), result.stderr


def terrain_harness(source_text):
    return r'''
#include <cstdio>
#include <cstdlib>
#include <cstdint>
#include <cstddef>
#include <cstring>
#include <vector>
#include <string>
typedef int BOOL;
#define __cdecl
void check_at(bool condition, int line) {
    if (!condition) { std::fprintf(stderr,"assertion at %d\n",line); std::exit(91); }
}
#define check(v) check_at((v),__LINE__)

struct Vert { int x, y; };
struct Font { int dummy; };
struct RECT { int left, top, right, bottom; };
void observe_clamp(char kind);
struct Buffer {
    int buffer_mark;
    int poly_calls; int poly_color;
    int text_color_calls; int font_calls; int write_calls;
    void poly(Vert *v, int n, int color) { observe_clamp('P'); ++poly_calls; poly_color = color; }
    void set_text_color(int, int, int, int) { ++text_color_calls; }
    void set_font(Font *, Font *, Font *, Font *) { ++font_calls; }
    void write_cent_l(char *, RECT *, int) { ++write_calls; }
    void line(int, int, int, int, int);
};

// ---- recording blitters ------------------------------------------------
struct Texture;
struct Sprite;
std::vector<Texture *> textures;
std::vector<Sprite *> sprites;
struct Draw {
    const void *receiver;
    std::string kind;
    Buffer *buffer;
    std::vector<Vert> vertices, uv;
    std::vector<int> light;
    Vert *uv_source;
};
std::vector<Draw> draws;
std::vector<std::vector<unsigned char> > coast_remaps;
// ACTUAL COAST DECLARATION
bool perturb_coast_palette_once = false;
void observe(const void *receiver, const char *kind, Buffer *buffer,
             Vert *vertices, int *light, int count, Vert *uv) {
    observe_clamp('T');
    check(count == 3 || count == 4);
    Draw d = {receiver, kind, buffer, {}, {}, {}, uv};
    for (int i = 0; i < count; ++i) {
        d.vertices.push_back(vertices[i]);
        if (uv) d.uv.push_back(uv[i]);
        if (light) d.light.push_back(light[i]);
    }
    draws.push_back(d);
}
struct Texture {
    std::string name; int index;
    int draw6, draw5, trans, trans3, key4, key5, mask, coast;
    Texture() : index(0), draw6(0), draw5(0), trans(0), trans3(0), key4(0),
                key5(0), mask(0), coast(0) { textures.push_back(this); }
    void draw(Buffer *b, Vert *v, int *l, int n, Vert *uv, int) {
        ++draw6; observe(this, "draw6", b, v, l, n, uv);
    }
    void draw(Buffer *b, Vert *v, int *l, int n, Vert *uv) {
        ++draw5; observe(this, "draw5", b, v, l, n, uv);
    }
    void draw_trans(Buffer *b, Vert *v, int *l, int n, Vert *uv) {
        ++trans; observe(this, "trans", b, v, l, n, uv);
    }
    void draw_trans_3(Buffer *b, Vert *v, int *l, int n, Vert *uv, int) {
        ++trans3; observe(this, "trans3", b, v, l, n, uv);
    }
    void draw_color_key(Buffer *b, Vert *v, int n, Vert *uv) {
        ++key4; observe(this, "key4", b, v, 0, n, uv);
    }
    void draw_color_key(Buffer *b, Vert *v, int n, Vert *uv, int) {
        ++key5; observe(this, "key5", b, v, 0, n, uv);
    }
    void draw_mask_dest(Buffer *b, Vert *v, unsigned char *, unsigned char *,
                        int n, Vert *uv) {
        ++mask; observe(this, "mask", b, v, 0, n, uv);
    }
    void draw_coast(Buffer *b, Vert *v, int *l, Texture *, unsigned char *remap, int n,
                    Vert *uv, int) {
        ++coast; observe(this, "coast", b, v, l, n, uv);
        coast_remaps.push_back(std::vector<unsigned char>(remap, remap + 53));
        if (perturb_coast_palette_once) {
            for (int shade = 4; shade < 8; ++shade)
                TerrainCoastPalette[shade] += 40;
            perturb_coast_palette_once = false;
        }
    }
    void draw2(Buffer *, Vert *, int *, int, Vert *, int, int);
};
struct Sprite {
    void *a, *b; unsigned char cTransparentIndex_;
    int draw_calls, draw_dest_calls;
    Sprite() : a(0), b(0), cTransparentIndex_(0), draw_calls(0),
               draw_dest_calls(0) { sprites.push_back(this); }
    void draw(Buffer *, int, int, int) { ++draw_calls; }
    void draw_dest(Buffer *, int, int, int, unsigned char *) {
        ++draw_dest_calls;
    }
};
static inline unsigned char sprite_transparent(const Sprite &s) {
    return s.cTransparentIndex_;
}
std::vector<std::string> hits;
#define TEX_ARRAY(name, n) Texture name[n];
#define SPR_ARRAY(name, n) Sprite name[n];
TEX_ARRAY(g_FARM_TEXTURES, 9)
TEX_ARRAY(g_RAINY_LAND_TEXTURES, 16)
TEX_ARRAY(g_MOIST_LAND_TEXTURES, 16)
Texture g_DUNE_LAND_TEXTURE;
Texture g_FLAT_ARID_LAND_TEXTURE;
TEX_ARRAY(g_ROCKY_TEXTURES, 4)
TEX_ARRAY(g_OCEAN_TEXTURES, 2)
TEX_ARRAY(g_MOUNT_PLANET_TEXTURES, 3)
TEX_ARRAY(g_SUNNY_MESA_TEXTURES, 8)
TEX_ARRAY(g_GARLAND_CRATER_TEXTURES, 3)
TEX_ARRAY(g_JUNGLE_LAND_TEXTURES, 15)
TEX_ARRAY(g_FOREST_TEXTURES, 16)
TEX_ARRAY(g_FUNGUS_TEXTURES, 30)
Texture g_RAINFALL_SINGLE_TILE_TEXTURE;
TEX_ARRAY(g_RIVER_TEXTURES, 16)
TEX_ARRAY(g_MAGTUBE_TEXTURES, 9)
TEX_ARRAY(g_ROAD_TEXTURES, 9)
TEX_ARRAY(g_RAINFALL_SINGLE_TILE_TEXTURES, 2)
SPR_ARRAY(g_TER1_LANDMARK_RESOURCE_SPRITES, 6)
SPR_ARRAY(g_TER1_SEA_LAND_RESOURCE_SPRITES, 12)
SPR_ARRAY(g_TER1_FARM_SPRITES, 5)
SPR_ARRAY(g_TER1_SOIL_ENRICHER_SPRITES, 5)
Sprite g_TER1_KELP_FARM_SPRITE;
Sprite g_TER1_BUNKER_SPRITE;
Sprite g_TER1_AIRBASE_SPRITE;
Sprite g_TER1_SENSOR_ARRAY_SPRITE;
Sprite g_TER1_MINE_SPRITE;
Sprite g_TER1_MINING_PLATFORM_SPRITE;
Sprite g_TER1_SOLAR_COLLECTOR_SPRITE;
Sprite g_TER1_TIDAL_HARNESS_SPRITE;
Sprite g_TER1_CONDENSER_SPRITE;
Sprite g_TER1_ECHELON_MIRROR_SPRITE;
Sprite g_TER1_BOREHOLE_SPRITE;
Sprite g_TER1_BOREHOLE_CLUSTER_SPRITE;
SPR_ARRAY(g_TER1_MANIFOLD_NEXUS_SPRITES, 6)
SPR_ARRAY(g_TER1WRECK_UNITY_WRECKAGE_SPRITES, 15)
SPR_ARRAY(g_TER1WRECK_UNITY_WRECKAGE_ALT_SPRITES, 4)
SPR_ARRAY(g_FOSSIL_FIELD_RIDGE_SPRITES, 6)
SPR_ARRAY(g_TER1_UNITY_POD_SPRITES, 6)
Sprite g_TER1_MONOLITH_SPRITE;
SPR_ARRAY(g_TER1_WHITE_ORG_YEL_TILE_SPRITES, 6)
SPR_ARRAY(g_TER1_BOTTOM_LEFT_TILE_SPRITES, 9)

// ---- the fixed-storage bindings the body reads --------------------------
int TextureLightFloor, TextureLightCeiling;
struct ClampEvent { char kind; int floor, ceiling; };
std::vector<ClampEvent> clamp_events;
bool perturb_clamp_once = false;
void observe_clamp(char kind) {
    clamp_events.push_back({kind, TextureLightFloor,
                           TextureLightCeiling});
    if (perturb_clamp_once) {
        TextureLightFloor = -13;
        TextureLightCeiling = 29;
        perturb_clamp_once = false;
    }
}
unsigned char LandMoistureTable_storage[256][2];
const unsigned char (*const LandMoistureTable)[2] = LandMoistureTable_storage;
int MapFocusX = -99, MapFocusY = -99;
// ACTUAL LANDMARK OWNERS
// ORIGINAL LANDMARK ORACLE
int OceanDetailShade[80];
int UnworkedCornerLight = 0x37;
Vert LandCornerUVs[12];
Vert EdgeMidUVs[16];
Vert OceanCornerUVs[16];
unsigned char CoastMaskTable_storage[53][8];
const unsigned char (*const CoastMaskTable)[8] = CoastMaskTable_storage;
// ACTUAL COAST OWNER
struct RulesChassis { unsigned char triad; };
RulesChassis Chassis[16];
Vert MapCornerScratch[4];
Vert MapCornerCopy[4];
Vert OceanCornerVerts[16];
int OceanCornerShades_storage[16];
int *const OceanCornerShades = OceanCornerShades_storage;
int LandCornerXStep_storage[12];
const int *const LandCornerXStep = LandCornerXStep_storage;
int LandCornerYStep_storage[12];
const int *const LandCornerYStep = LandCornerYStep_storage;
int LandCornerId_storage[12];
const int *const LandCornerId = LandCornerId_storage;
int CornerOffsetX_storage[4][4];
const int (*const CornerOffsetX)[4] = CornerOffsetX_storage;
int CornerOffsetY_storage[4][4];
const int (*const CornerOffsetY)[4] = CornerOffsetY_storage;
unsigned int TextureRasterKey;
int SpriteScaleNumerator, SpriteScaleDenominator;
unsigned char ShadowColorTable_storage[256];
unsigned char *const ShadowColorTable = ShadowColorTable_storage;
unsigned char TerritoryColorTables_storage[8][256];
unsigned char *const TerritoryColorTables = TerritoryColorTables_storage[0];

// ---- map/player/veh/base state ------------------------------------------
// ACTUAL MAP
static_assert(sizeof(Map) == 0x2c, "original map tile stride");
static_assert(offsetof(Map, val2) == 2, "tile owner");
static_assert(offsetof(Map, visibility) == 4, "tile visibility");
static_assert(offsetof(Map, territory) == 7, "signed sensor owner");
static_assert(offsetof(Map, bit2) == 0xc, "faction zero bit view");
static_assert(offsetof(Map, bit_visible) == 0x10, "factions one through seven");
Map Tiles_storage[64];
Map *Tiles_pointer = Tiles_storage;
inline Map *&map_tiles() { return Tiles_pointer; }
int MapLongitudeBounds = 8, MapLatitudeBounds = 8, MapIsFlat = 1;
unsigned int MapLongitude = 4;
const int RadiusBaseX[8] = {1, 2, 1, 0, -1, -2, -1, 0};
const int RadiusBaseY[8] = {-1, 0, 1, 2, 1, 0, -1, -2};
// The actual body now shares the already mapped direction objects. Retain
// the old arrays above only as decoys for RiverStep damage controls below.
int VehMoveDeltaX[9] = {1, 2, 1, 0, -1, -2, -1, 0, 0};
int VehMoveDeltaY[9] = {-1, 0, 1, 2, 1, 0, -1, -2, 0};
const int RiverStepX[4] = {1, 1, -1, -1};
const int RiverStepY[4] = {-1, 1, 1, -1};
int RadiusOffsetX[289];
int RadiusOffsetY[289];
int MapWinCaptionScale = -1, MapWinCaptionDivisor = -1;
struct PlayerData {
    unsigned int flags;
    int last_turn_new_base;
    unsigned int diplo_treaties[8];
};
PlayerData PlayersData_storage[8];
PlayerData *const PlayersData = PlayersData_storage;
struct GameDataStateWindowFields {
    unsigned int game_preferences_;
    unsigned int game_more_preferences_;
    unsigned int game_rules_;
    unsigned int game_state_;
};
struct GameDataStateWindow { GameDataStateWindowFields fields_; };
GameDataStateWindow GameDataStateStorage;
BOOL ExpansionEnabled;
int InfoFactionTextColors_storage[8];
int *const InfoFactionTextColors = InfoFactionTextColors_storage;
char StringTemp_storage[1032];
char *const StringTemp = StringTemp_storage;
char * _itoa(int value, char *out, int) {
    sprintf(out, "%d", value); return out;
}
// ACTUAL VEH
static_assert(sizeof(Veh) == 0x34, "original vehicle stride");
// Independently reduce the original wrapped row address to the real field:
// 0x96C9F8 - 0x20CC + 4*faction == 0x952828 + vehicle*0x34 + offset.
#define ORIGINAL_TREATY_ADDRESS(f) (0x96C9F8u - 0x20CCu + 4u * (f))
#define VEH_FIELD_ADDRESS(index, field) \
    (0x952828u + (index) * sizeof(Veh) + offsetof(Veh, field))
static_assert(VEH_FIELD_ADDRESS(1895, waypoint_y) == ORIGINAL_TREATY_ADDRESS(1), "faction 1");
static_assert(VEH_FIELD_ADDRESS(1895, waypoint_y) + 2 * sizeof(int16_t) == ORIGINAL_TREATY_ADDRESS(2), "faction 2");
static_assert(VEH_FIELD_ADDRESS(1895, morale) == ORIGINAL_TREATY_ADDRESS(3), "faction 3");
static_assert(VEH_FIELD_ADDRESS(1895, moves_expended) == ORIGINAL_TREATY_ADDRESS(4), "faction 4");
static_assert(VEH_FIELD_ADDRESS(1895, probe_action) == ORIGINAL_TREATY_ADDRESS(5), "faction 5");
static_assert(VEH_FIELD_ADDRESS(1895, next_veh_id_stack) == ORIGINAL_TREATY_ADDRESS(6), "faction 6");
static_assert(VEH_FIELD_ADDRESS(1896, x) == ORIGINAL_TREATY_ADDRESS(7), "faction 7");
static_assert(VEH_FIELD_ADDRESS(1895, x) == 0x96C9E0u - 0x20CCu,
              "observer -1 player flags physically alias Vehs[1895].x");
Veh Vehs_storage[1897];
Veh *const Vehs = Vehs_storage;
struct VehPrototype { char veh_name[32]; unsigned int ability_flags;
    unsigned char chassis_id, weapon_id, armor_id, reactor_id,
        carry_capacity, cost, plan; signed char unk_1;
    unsigned char obsolete_factions; signed char combat_factions,
        icon_offset, padding; unsigned short flags; short preq_tech; };
VehPrototype VehPrototypes_storage[4];
VehPrototype *const VehPrototypes = VehPrototypes_storage;
int VehDrawPositionX = -1, VehDrawPositionY = -1, VehDrawPositionVehicle;
struct Base { short x, y; unsigned char faction_id_current,
    faction_id_former; signed char population_size;
    unsigned char assimilation_turns_left, nerve_staple_turns_left, unk_1,
    visibility; unsigned char faction_pop_size_intel[8]; char name_string[25];
    short unk_x, unk_y; unsigned int state; unsigned char pad[0x100];
    unsigned char facilities_built[12]; };
Base Bases_storage[4];
Base *const Bases = Bases_storage;
struct Path { int get(int, int); } Paths_mock;
Path Paths;
int PathGet_result;
int Path::get(int, int) { return PathGet_result; }
enum { ABL_DEEP_RADAR = 0x2 };
unsigned RandomValue;
class Random { public: unsigned seed; Random() : seed(1) {}
    void reseed(unsigned s) { seed = s; }
    unsigned get(int, int) { return RandomValue; } };
struct ConsoleMock { int faction_id_; int cursor_history_x_[4];
    int cursor_history_y_[4]; int cursor_history_index_; } ConsoleGlobal_mock;
ConsoleMock *const ConsoleGlobal_ptr = &ConsoleGlobal_mock;
#define ConsoleGlobal (*ConsoleGlobal_ptr)
int base_at(int, int) { return -1; }
int VehAtResult = -1, VehAtCalls = 0;
int veh_at(int x, int y) {
    check(x == 0 && y == 0);
    ++VehAtCalls;
    return VehAtResult;
}
int IsSensor = 0;
int is_sensor(int x, int y) {
    check(x == 0 && y == 0);
    return IsSensor;
}
BOOL HasDeepRadar = 0;
BOOL has_abil(int, int ability) {
    check(ability == ABL_DEEP_RADAR);
    return HasDeepRadar;
}
int whose_territory(int, int, int, int *, BOOL) { return -1; }
int BonusCalls = 0;
int bonus_at(int x, int y, int flags) {
    check(x == 0 && y == 0 && flags == 0);
    ++BonusCalls;
    return 0;
}
int goody_at(int, int) { return 0; }
int world_site(int, int, BOOL) { return 3; }
void site_set(int, int, int) {}
int CropCalls = 0;
int crop_yield(int faction, int base, int x, int y, int flags) {
    check(faction == 1 && base == -1 && x == 0 && y == 0 && flags == 0);
    ++CropCalls;
    return 2;
}
void bitmask(int, int *o, int *m) { *o = 0; *m = 1; }
int alt_get_ocean_detail(int, int, int corner, int sub) {
    return (corner * 3 + sub) & 7;
}
// ---- recording real member interfaces, not executable-address seams ----
int Line_calls, Line_color; int Draw2_calls;
struct Line { int x0, y0, x1, y1, color; };
std::vector<Line> lines;
void Buffer::line(int x0, int y0, int x1, int y1, int color) {
    observe_clamp('L');
    ++Line_calls; Line_color = color;
    lines.push_back({x0, y0, x1, y1, color});
}
void Texture::draw2(Buffer *b, Vert *v, int *l, int n, Vert *uv, int, int) {
    ++Draw2_calls;
    observe(this, "draw2", b, v, l, n, uv);
}

// ---- the window ---------------------------------------------------------
bool landmark_lighting = false;
struct MapWin : Buffer {
    void *owned_;
    unsigned int field_8_;
    Font font_b_;
    int get_alt(int, int, int) { return 0; }
    int get_point_light(int, int, int corner) { return 0x37 + (landmark_lighting ? corner * 3 : 0); }
    MapWin() : Buffer(), owned_(0), field_8_(0) {}
};

struct WorldWin { MapWin mapWin_; };
static WorldWin WorldClimateWorldWin;

// SOURCE

void treset() {
    landmark_lighting = false;
    clamp_events.clear();
    perturb_clamp_once = false;
    TextureLightFloor = -71;
    TextureLightCeiling = 113;
    for (Texture *t : textures) {
        t->draw6 = t->draw5 = t->trans = t->trans3 = 0;
        t->key4 = t->key5 = t->mask = t->coast = 0;
    }
    for (Sprite *s : sprites) s->draw_calls = s->draw_dest_calls = 0;
    draws.clear();
    coast_remaps.clear();
    perturb_coast_palette_once = false;
    memset(Tiles_storage, 0, sizeof(Tiles_storage));
    for (Map &tile : Tiles_storage) tile.climate = 0x60;
    memset(PlayersData_storage, 0, sizeof(PlayersData_storage));
    memset(Vehs_storage, 0, sizeof(Vehs_storage));
    for (Veh &v : Vehs_storage) {
        v.next_veh_id_stack = -1;
        v.prev_veh_id_stack = -1;
    }
    VehAtResult = -1;
    VehAtCalls = 0;
    HasDeepRadar = 0;
    IsSensor = 0;
    BonusCalls = CropCalls = 0;
    // No default valid radius samples; each scan case opts into one slot.
    for (int i = 0; i < 289; ++i) {
        RadiusOffsetX[i] = 100;
        RadiusOffsetY[i] = 100;
    }
    VehDrawPositionX = VehDrawPositionY = -1;
    VehDrawPositionVehicle = 0;
    for (RulesChassis &chassis : Chassis) chassis.triad = 2;
    PlayersData_storage[1].flags = 0x200;  // map revealed: no wireframe exit
    GameDataStateStorage.fields_.game_state_ = 0;
    GameDataStateStorage.fields_.game_preferences_ = 0;
    GameDataStateStorage.fields_.game_more_preferences_ = 0;
    GameDataStateStorage.fields_.game_rules_ = 0;
    ExpansionEnabled = 0;
    MapLongitudeBounds = MapLatitudeBounds = 8; MapIsFlat = 1;
    MapWinCaptionScale = -1; MapWinCaptionDivisor = -1;
    ConsoleGlobal.faction_id_ = 1; ConsoleGlobal.cursor_history_index_ = 0;
    ConsoleGlobal.cursor_history_x_[0] = -99;
    ConsoleGlobal.cursor_history_y_[0] = -99;
    RandomValue = 0;
    MapFocusX = 0; MapFocusY = 0;
    memset(LandMoistureTable_storage, 0, sizeof(LandMoistureTable_storage));
    for (int i = 0; i < 12; ++i) {
        LandCornerUVs[i].x = 10 + i * 2;
        LandCornerUVs[i].y = 11 + i * 2;
    }
    for (int i = 0; i < 12; ++i) {
        LandCornerXStep_storage[i] = i % 3 - 1;
        LandCornerYStep_storage[i] = i / 3 - 2;
        LandCornerId_storage[i] = i & 3;
    }
    for (int i = 0; i < 16; ++i) {
        CornerOffsetX_storage[i / 4][i % 4] = i % 4 - 2;
        CornerOffsetY_storage[i / 4][i % 4] = i / 4 - 2;
    }
    for (int i = 0; i < 16; ++i) {
        EdgeMidUVs[i].x = 70 + i * 2;
        EdgeMidUVs[i].y = 71 + i * 2;
        OceanCornerUVs[i].x = 110 + i * 2;
        OceanCornerUVs[i].y = 111 + i * 2;
    }
    for (int i = 0; i < 80; ++i) OceanDetailShade[i] = 0x20 + i;
    for (int row = 0; row < 53; ++row)
        for (int mask = 0; mask < 8; ++mask)
            CoastMaskTable_storage[row][mask] = (row * 8 + mask) % 6;
    // Valid original mask values are0..5; values2..5 select shared palette
    // indices4..7. Deliberately distinct surrounding bytes catch shifted bases.
    for (int i = 0; i < 12; ++i) TerrainCoastPalette[i] = 31 + i * 7;
    Line_calls = Draw2_calls = 0;
    lines.clear();
    hits.clear();
    GameDataStateStorage.fields_.game_more_preferences_ = 0; // worked = 1 through the fast gate
}

int drawn(Texture *arr, Texture *unit, int n) {
    for (int i = 0; i < n; ++i)
        if (&arr[i] == unit &&
            (arr[i].draw6 || arr[i].draw5 || arr[i].trans || arr[i].trans3 ||
             arr[i].key4 || arr[i].key5 || arr[i].mask || arr[i].coast))
            return i;
    return -1;
}

void scan_setup(int faction, int radius_slot, int owner) {
    treset();
    GameDataStateStorage.fields_.game_more_preferences_ = 1;
    for (int f = 0; f < 8; ++f) PlayersData[f].flags = 0x200;
    ConsoleGlobal.faction_id_ = faction;
    Tiles_storage[0].climate = 0x40;
    if (owner >= 0) {
        Tiles_storage[0].bit = 2;   // vehicle owner, not a base
        Tiles_storage[0].val2 = owner;
    }
    RadiusOffsetX[radius_slot] = RadiusOffsetY[radius_slot] = 0;
    VehAtResult = 0;
    Vehs[0].faction_id = faction;
    HasDeepRadar = 1;
    // Write the low byte at independently calculated original addresses.
    // The production helper reads typed fields, not this byte expression.
    unsigned char *const bytes = reinterpret_cast<unsigned char *>(Vehs_storage);
    for (int f = 1; f <= 7; ++f)
        bytes[ORIGINAL_TREATY_ADDRESS(f) - 0x952828u] = 0;
}

void check_worked(bool worked) {
    check(((Tiles_storage[0].bit2 & 0x200000) == 0) == worked);
    check(g_OCEAN_TEXTURES[1].draw5 == (worked ? 4 : 0));
    check(g_OCEAN_TEXTURES[1].draw6 == (worked ? 0 : 4));
}

int main() {
    MapWin window;
    Buffer buffer = {};
    Map *tile = &map_tiles()[0];

    // Clamp entry publication follows the original land admission; the
    // off-map and special-window returns precede it. Poison distinguishes
    // preservation from a seemingly helpful unconditional default reset.
    for (int edge = 0; edge != 4; ++edge) {
        treset();
        const int x = edge == 0 ? -1 : edge == 1 ? 8 : 0;
        const int y = edge == 2 ? -1 : edge == 3 ? 8 : 0;
        check(gen_terrain_poly(&buffer, &window, 5, 1, 0, 1,
                               x, y, 100, 100) == 0);
        check(TextureLightFloor == -71 && TextureLightCeiling == 113);
        check(clamp_events.empty());
    }
    treset();
    // The special receiver is the embedded world map, not a fake address.
    check(gen_terrain_poly(&buffer, &WorldClimateWorldWin.mapWin_, 5, 1, 0, 1,
                           0, 0, 100, 100) == 1);
    check(TextureLightFloor == -71 && TextureLightCeiling == 113);
    check(clamp_events.empty());

    // Four land draws see the narrow pair. A callback mutation persists
    // into later calls until the original exit writes the wide defaults.
    for (int perturb = 0; perturb != 2; ++perturb) {
        treset(); perturb_clamp_once = perturb != 0;
        gen_terrain_poly(&buffer, &window, 5, 1, 0, 1, 0, 0, 100, 100);
        check(clamp_events.size() == 4);
        for (size_t i = 0; i != clamp_events.size(); ++i) {
            check(clamp_events[i].kind == 'T');
            check(clamp_events[i].floor == (perturb && i ? -13 : -4));
            check(clamp_events[i].ceiling == (perturb && i ? 29 : 4));
        }
        check(TextureLightFloor == -256 && TextureLightCeiling == 256);
        // No reset between calls: ocean consumes the previous exit state.
        clamp_events.clear(); tile->climate = 0x40;
        gen_terrain_poly(&buffer, &window, 5, 1, 0, 1, 0, 0, 100, 100);
        check(clamp_events.size() == 8);
        for (const ClampEvent &event : clamp_events)
            check(event.floor == -256 && event.ceiling == 256);
        check(TextureLightFloor == -256 && TextureLightCeiling == 256);
    }
    // Ocean has no narrow-pair entry assignment: poison is visible to its
    // first draw, and a changed callback value remains visible to its rest.
    treset(); tile->climate = 0x40; perturb_clamp_once = true;
    gen_terrain_poly(&buffer, &window, 5, 1, 0, 1, 0, 0, 100, 100);
    check(clamp_events.size() == 8);
    for (size_t i = 0; i != clamp_events.size(); ++i) {
        check(clamp_events[i].floor == (i ? -13 : -71));
        check(clamp_events[i].ceiling == (i ? 29 : 113));
    }
    check(TextureLightFloor == -256 && TextureLightCeiling == 256);

    // Distinct early flat exits: editor solid mode, land no-terrain, and
    // ocean no-terrain. The poly callback precedes each restoration.
    for (int mode = 0; mode != 3; ++mode) {
        treset(); if (mode == 2) tile->climate = 0x40;
        perturb_clamp_once = true;
        const int flags = mode == 0 ? 0x1000000 : int(0x80000000u);
        gen_terrain_poly(&buffer, &window, 5, 1, 0, flags, 0, 0, 100, 100);
        check(!clamp_events.empty() && clamp_events.back().kind == 'P');
        check(clamp_events.front().floor == (mode == 2 ? -71 : -4));
        check(clamp_events.front().ceiling == (mode == 2 ? 113 : 4));
        check(TextureLightFloor == -256 && TextureLightCeiling == 256);
    }
    // Unrevealed wireframe reaches the common restoration after all lines.
    treset(); PlayersData[1].flags = 0; perturb_clamp_once = true;
    gen_terrain_poly(&buffer, &window, 5, 1, 0, 1, 0, 0, 100, 100);
    check(clamp_events.size() == 4 && clamp_events[0].kind == 'L');
    check(clamp_events[0].floor == -4 && clamp_events[0].ceiling == 4);
    check(clamp_events[1].floor == -13 && clamp_events[1].ceiling == 29);
    check(TextureLightFloor == -256 && TextureLightCeiling == 256);

    // ---- faction views are bit2 for zero, then seven bit_visible slots ----
    for (int faction = 0; faction <= 7; ++faction) {
        treset();
        PlayersData[faction].flags = 0;
        tile->visibility = 1u << faction;
        tile->val2 = 0;   // deliberately not the visibility byte
        if (faction == 0) tile->bit2 = 0x8000;
        else tile->bit_visible[faction - 1] = 0x8000;
        gen_terrain_poly(&buffer, &window, 5, faction, 0, 1, 0, 0, 100, 100);
        check(g_FARM_TEXTURES[0].draw6 == 4);
        check(g_FLAT_ARID_LAND_TEXTURE.draw6 == 0 && Line_calls == 0);
    }

    // Visibility, player flags and GameRules are independent gates;
    // neither val2's owner byte nor GamePreferences nor a treaty substitutes.
    for (int preference = 0; preference < 2; ++preference)
    for (int rules = 0; rules < 2; ++rules)
    for (int visible = 0; visible < 2; ++visible)
    for (int omniscient = 0; omniscient < 2; ++omniscient) {
        treset();
        tile->climate = 0x40;
        tile->visibility = visible ? 2 : 0;
        tile->val2 = visible ? 0 : 2;
        PlayersData[1].flags = omniscient ? 0x200 : 0;
        PlayersData[1].diplo_treaties[0] = omniscient ? 0 : 2;
        GameDataStateStorage.fields_.game_preferences_ = preference ? 0x100 : 0;
        GameDataStateStorage.fields_.game_rules_ = rules ? 0x100 : 0;
        gen_terrain_poly(&buffer, &window, 5, 1, 0, 1, 0, 0, 100, 100);
        const bool fog = rules && !visible && !omniscient;
        const bool wireframe = !rules && !visible && !omniscient;
        check(((tile->bit2 & 0x200000) != 0) == fog);
        check(Line_calls == (wireframe ? 4 : 0));
        if (!wireframe) check_worked(!fog);
    }

    // Observer -1 has a separate original alias: flags bit 0x200 comes
    // from Vehs[1895].x, not ConsoleGlobal's faction or vehicle morale.
    for (int value = 0; value < 4; ++value) {
        treset();
        tile->climate = 0x40;
        GameDataStateStorage.fields_.game_rules_ = 0x100;
        Vehs[1895].x = value * 0x100;
        PlayersData[1].flags = (value & 2) ? 0 : 0x200;
        gen_terrain_poly(&buffer, &window, 5, -1, 0, 1, 0, 0, 100, 100);
        check_worked((value & 2) != 0);
    }

    // A sensor's physical owner is the signed territory byte, not val3's
    // worked/faction bits. -1 must not become faction seven by masking.
    for (int match = 0; match < 2; ++match) {
        scan_setup(7, 9, -1);
        IsSensor = 1;
        tile->territory = match ? 7 : -1;
        tile->val3 = match ? 0 : 7;
        gen_terrain_poly(&buffer, &window, 5, 7, 0, 1, 0, 0, 100, 100);
        check_worked(match != 0);
        check(VehAtCalls == 0);
    }

    // Signed landmark sequence -1 skips tables rather than indexing 255.
    treset();
    tile->bit2 = 0xff000002u;
    gen_terrain_poly(&buffer, &window, 5, 1, 0, 1, 0, 0, 100, 100);
    check(draws.size() == 4);

    // ---- sensor borders follow four diagonal neighbours, not radius[0:4] ----
    for (int axis = 0; axis < 2; ++axis) {
        treset();
        GameDataStateStorage.fields_.game_preferences_ = 0x200;
        MapFocusX = axis == 0 ? 1 : 4;
        MapFocusY = axis == 0 ? 4 : 1;
        gen_terrain_poly(&buffer, &window, 5, 1, 0, 1, 4, 4, 100, 100);
        check(lines.size() == 2);
        // From western focus, eastern diagonals are outside: edges1->2,
        // 2->3. From northern focus, southern diagonals give2->3,3->0.
        for (int i = 0; i < 2; ++i) {
            const int corner = axis + i + 1;
            check(lines[i].color == 0xf9);
            check(lines[i].x0 == 164 && lines[i].x1 == 164);
            check(lines[i].y0 == 100 + corner * 16);
            check(lines[i].y1 == 100 + ((corner + 1) & 3) * 16);
        }
    }

    // Crop display independently requests yield at 0x1000, including far
    // zoom; ordinary improvements need 0x20, farm and zoom > -8 together.
    for (int zoom = -8; zoom <= -7; ++zoom)
    for (int mode = 0; mode < 3; ++mode)
    for (int farm = 0; farm < 2; ++farm) {
        treset();
        tile->bit = farm ? 0x8000 : 0;
        const int flags = mode == 2 ? 0x1000 : (mode == 1 ? 0x20 : 0);
        gen_terrain_poly(&buffer, &window, zoom, 1, 0, flags, 0, 0, 100, 100);
        check(CropCalls == (mode == 2 || (mode == 1 && farm && zoom > -8)));
    }
    for (int zoom = -8; zoom <= -7; ++zoom) {
        treset();
        gen_terrain_poly(&buffer, &window, zoom, 1, 0, 0x200, 0, 0, 100, 100);
        check(BonusCalls == 1);
    }

    // ---- first missing column reads real last_turn_new_base bit zero ----
    // 4637B9: row is viewing faction; owner -1 selects +0x14, not a
    // diplomacy element. The second lookup is held false by zero aliases.
    for (int faction = 1; faction <= 7; ++faction) {
        for (int value = 0; value <= 3; ++value) {
            scan_setup(faction, 0, -1);
            HasDeepRadar = 0;
            PlayersData[faction].last_turn_new_base = value;
            PlayersData[(faction % 7) + 1].last_turn_new_base = value ^ 1;
            gen_terrain_poly(&buffer, &window, 5, faction, 0, 1, 0, 0, 100, 100);
            check_worked((value & 1) != 0);
            check(VehAtCalls == (value & 1));
        }
    }

    // ---- second missing row aliases seven real vehicle-state fields ----
    // Radius slot 9 bypasses the first nine-sample owner test. Pattern 2
    // must stay false and pattern 3 true: this is a low-bit test, not !=0.
    for (int faction = 1; faction <= 7; ++faction) {
        for (int value = 0; value <= 3; ++value) {
            scan_setup(faction, 9, -1);
            unsigned char *const bytes = reinterpret_cast<unsigned char *>(Vehs_storage);
            bytes[ORIGINAL_TREATY_ADDRESS(faction) - 0x952828u] = value;
            gen_terrain_poly(&buffer, &window, 5, faction, 0, 1, 0, 0, 100, 100);
            check_worked((value & 1) != 0);
            check(VehAtCalls == (value & 1));
        }
    }

    // Ordinary owners distinguish opposite row/column orders. For the
    // first route, disable radar so a second-route scan cannot grant work.
    for (int bit = 0; bit < 2; ++bit) {
        scan_setup(2, 0, 1);
        HasDeepRadar = 0;
        PlayersData[2].diplo_treaties[1] = bit;
        PlayersData[1].diplo_treaties[2] = bit ^ 1;
        gen_terrain_poly(&buffer, &window, 5, 2, 0, 1, 0, 0, 100, 100);
        check_worked(bit != 0);
        scan_setup(2, 9, 1);
        PlayersData[2].diplo_treaties[1] = bit ^ 1;
        PlayersData[1].diplo_treaties[2] = bit;
        gen_terrain_poly(&buffer, &window, 5, 2, 0, 1, 0, 0, 100, 100);
        check_worked(bit != 0);
        check(VehAtCalls == bit);
    }

    // ---- base-land rows: aridity 0/1/2 and the dune bit ----
    // climate bits: altitude 0x60 (land), rainfall (aridity) << 3,
    // detail << 5 = 3 << 5 so the lit path runs.
    {
        const int arid[3] = {0, 1, 2};
        for (int a = 0; a < 3; ++a) {
            treset();
            tile->climate = 0x60 | (arid[a] << 3);
            gen_terrain_poly(&buffer, &window, 5, 1, 0, 1, 0, 0, 100, 100);
            check(drawn(&g_FLAT_ARID_LAND_TEXTURE,
                        &g_FLAT_ARID_LAND_TEXTURE, 1) >= 0 || a != 0);
            if (a == 0)
                check(g_FLAT_ARID_LAND_TEXTURE.draw6 == 4);
            if (a == 1)
                check(g_MOIST_LAND_TEXTURES[0].draw6 == 4);
            if (a == 2)
                check(g_RAINY_LAND_TEXTURES[0].draw6 == 4);
            if (a == 0) {
                // Original 463E9C..463F3A writes twelve interleaved
                // (x,y)/(u,v) pairs; 464215 consumes four consecutive
                // three-vertex groups. Distinct synthetic table entries
                // catch byte offsets accidentally used as element indices.
                check(draws.size() == 4);
                for (int tri = 0; tri < 4; ++tri) {
                    const Draw &d = draws[tri];
                    check(d.receiver == &g_FLAT_ARID_LAND_TEXTURE);
                    check(d.buffer == &buffer && d.kind == "draw6");
                    check(d.vertices.size() == 3 && d.uv.size() == 3);
                    for (int c = 0; c < 3; ++c) {
                        const int index = tri * 3 + c;
                        check(d.vertices[c].x == 164 + (c - 1) * 32);
                        check(d.vertices[c].y == 132 + (tri - 2) * 16);
                        check(d.uv[c].x == 10 + index * 2);
                        check(d.uv[c].y == 11 + index * 2);
                        check(d.light[c] == 0x37);
                    }
                }
                for (int corner = 0; corner < 4; ++corner) {
                    check(MapCornerScratch[corner].x == 164);
                    check(MapCornerScratch[corner].y ==
                          132 + (corner - 2) * 16);
                }
            }
        }
        // the moisture row the 8-neighbour mask selects
        treset();
        tile->climate = 0x60 | (1 << 3);
        // The (1,1) neighbour is RadiusBase[2], tested at mask bit 3
        // because the original scan uses (i-1)&7.
        map_tiles()[4].climate = 0x68;
        LandMoistureTable_storage[8][0] = 5;     // row 5 for mask 8
        gen_terrain_poly(&buffer, &window, 5, 1, 0, 1, 0, 0, 100, 100);
        check(g_MOIST_LAND_TEXTURES[5].draw6 == 4);
        // the dune bit replaces the aridity rows
        treset();
        tile->climate = 0x60 | (1 << 3);
        tile->bit2 = 0x40;
        LandMoistureTable_storage[0][1] = 2;
        gen_terrain_poly(&buffer, &window, 5, 1, 0, 1, 0, 0, 100, 100);
        check(g_DUNE_LAND_TEXTURE.draw6 == 4);
        check(g_MOIST_LAND_TEXTURES[0].draw6 == 0);
        for (int tri = 0; tri < 4; ++tri) {
            // The dune path borrows the canonical triangle, even when the
            // ordinary moisture path prepared differently rotated UVs.
            check(draws[tri].uv_source == LandCornerUVs + tri * 3);
            for (int c = 0; c < 3; ++c) {
                check(draws[tri].uv[c].x == 10 + (tri * 3 + c) * 2);
                check(draws[tri].uv[c].y == 11 + (tri * 3 + c) * 2);
            }
        }
    }

    // ---- the farm bit (a6 & 1 + tile->bit & 0x8000): variant rows ----
    {
        treset();
        tile->climate = 0x60 | (0 << 3);
        tile->bit = 0x8000;
        RandomValue = 1;   // variant 1 -> aridity 0 + 3*1 = row 3
        gen_terrain_poly(&buffer, &window, 5, 1, 0, 1, 0, 0, 100, 100);
        check(g_FARM_TEXTURES[3].draw6 == 4);
        check(g_FLAT_ARID_LAND_TEXTURE.draw6 == 0);
    }

    // ---- the rocky overlay index variant + 2*rockiness - 2 ----
    {
        treset();
        tile->climate = 0x60 | 0;
        tile->val3 = 1 << 6;             // rockiness 1
        RandomValue = 0;
        gen_terrain_poly(&buffer, &window, 5, 1, 0, 1, 0, 0, 100, 100);
        check(g_ROCKY_TEXTURES[0].trans3 == 4);
        treset();
        tile->climate = 0x60 | 0;
        tile->val3 = 2 << 6;             // rockiness 2, variant 1
        RandomValue = 1;
        gen_terrain_poly(&buffer, &window, 5, 1, 0, 1, 0, 0, 100, 100);
        check(g_ROCKY_TEXTURES[3].trans3 == 4);
    }

    // ---- ocean uses distinct overloads for worked/unworked shading ----
    for (int worked = 0; worked < 2; ++worked) {
        treset();
        tile->climate = 0x40;
        // An empty work-radius scan selects unworked shading.
        GameDataStateStorage.fields_.game_more_preferences_ = worked ? 0 : 1;
        gen_terrain_poly(&buffer, &window, 5, 1, 0, 1, 0, 0, 100, 100);
        check(g_OCEAN_TEXTURES[1].draw5 == (worked ? 4 : 0));
        check(g_OCEAN_TEXTURES[1].draw6 == (worked ? 0 : 4));
        check(Draw2_calls == 4 && draws.size() == 8);
        for (int corner = 0; corner < 4; ++corner) {
            const Draw &d = draws[corner * 2];
            const Draw &under = draws[corner * 2 + 1];
            check(d.vertices.size() == 4 && under.vertices.size() == 4);
            for (int sub = 0; sub < 4; ++sub) {
                const int dir = (corner - 2 + sub) & 3;
                check(d.vertices[sub].x == 164 + (dir - 2) * 32);
                check(d.vertices[sub].y == 132 + (corner - 2) * 16);
                check(d.uv[sub].x == 110 + (corner * 4 + sub) * 2);
                check(d.uv[sub].y == 111 + (corner * 4 + sub) * 2);
                check(d.light[sub] == 0x10 + ((corner * 3 + sub) & 7));
                check(under.light[sub] == d.light[sub] + 0x10);
            }
        }
    }

    // ---- coast's independent per-corner vertices, UVs, light and mask ----
    {
        treset();
        map_tiles()[4].climate = 0x40;   // RadiusBase[2]: corners 2 and 3
        gen_terrain_poly(&buffer, &window, 5, 1, 0, 1, 0, 0, 100, 100);
        check(g_OCEAN_TEXTURES[0].coast == 2 && draws.size() == 6);
        for (int i = 0; i < 2; ++i) {
            const int corner = i + 2;
            const Draw &d = draws[4 + i];
            check(d.kind == "coast" && d.vertices.size() == 4);
            const int x[4] = {132, 180, 196, 180};
            for (int sub = 0; sub < 4; ++sub) {
                check(d.vertices[sub].x == x[sub]);
                const int row = sub == 3 ? corner : ((corner - 1) & 3);
                check(d.vertices[sub].y == 100 + row * 16);
                check(d.uv[sub].x == 70 + corner * 8 + sub * 2);
                check(d.uv[sub].y == 71 + corner * 8 + sub * 2);
                check(d.light[sub] == 0x20 + ((corner * 3 + sub) & 7));
            }
            const int code = corner == 2 ? 4 : 1;
            for (int entry = 0; entry < 53; ++entry) {
                const int value = (entry * 8 + code) % 6;
                check(coast_remaps[i][entry] ==
                      (value > 1 ? 31 + (value + 2) * 7 : value));
            }
        }
    }

    // Full ocean neighborhood produces code7 at all four corners. Original
    //4653CF/EB/407 mirror the SAME palette[7,6,5,4] into three four-byte
    // runs only when (tile_y-tile_x)&2. A draw callback changes those four
    // colors, demonstrating shared/live publication between corner calls.
    for (int mirror = 0; mirror < 2; ++mirror) {
        for (int perturb = 0; perturb < 2; ++perturb) {
            treset();
            const int center_y = mirror ? 4 : 2;
            for (Map &neighbor : Tiles_storage) neighbor.climate = 0x40;
            Tiles_storage[center_y * 4 + 1].climate = 0x60; // center(2,y)
            perturb_coast_palette_once = perturb != 0;
            gen_terrain_poly(&buffer, &window, 5, 1, 0, 1,
                             2, center_y, 100, 100);
            check(coast_remaps.size() == 4);
            for (int corner = 0; corner < 4; ++corner) {
                check(coast_remaps[corner].size() == 53);
                const int changed = perturb && corner != 0 ? 40 : 0;
                for (int entry = 0; entry < 53; ++entry) {
                    const int value = (entry * 8 + 7) % 6;
                    int expected = value > 1 ? 31 + (value + 2) * 7 + changed : value;
                    if (mirror && (entry == 5 || (entry >= 14 && entry <= 17)))
                        expected = 0;
                    if (mirror && entry >= 18 && entry < 30) {
                        const int backwards[4] = {80, 73, 66, 59};
                        expected = backwards[(entry - 18) % 4] + changed;
                    }
                    check(coast_remaps[corner][entry] == expected);
                }
            }
            for (int shade = 0; shade < 12; ++shade)
                check(TerrainCoastPalette[shade] ==
                      31 + shade * 7 + (perturb && shade >= 4 && shade < 8 ? 40 : 0));
        }
    }

    // Original4647BD..464AFF: independent pinned-image pairs drive the
    // expected receivers, not the candidate's field loads or initializers.
    // All49 codes are legitimate with overlapping editor-created flags;
    // Mount's normal producer alone uses only0..8. Explicit Monsoon overlap
    // can retain these flags while publishing unsupported codes beyond48.
    // Compare only accesses within the measured 49-row source objects. The
    // original has no upper-bound branch; larger codes may reach adjacent
    // storage, which this bounded fixture cannot model as original behavior.
    Texture *families[] = {g_MOUNT_PLANET_TEXTURES, g_SUNNY_MESA_TEXTURES,
                          g_GARLAND_CRATER_TEXTURES};
    const int family_sizes[] = {3, 8, 3};
    const unsigned gates[] = {2, 0x100, 1};
    for (int sequence = 0; sequence < 49; ++sequence) {
        for (int enabled = 0; enabled < 8; ++enabled) {
            treset(); landmark_lighting = true;
            tile->bit2 = static_cast<unsigned>(sequence) << 24;
            for (int family = 0; family < 3; ++family)
                if (enabled & (1 << family)) tile->bit2 |= gates[family];
            gen_terrain_poly(&buffer, &window, 5, 1, 0, 1, 0, 0, 100, 100);
            int expected_draws = 0;
            for (int family = 0; family < 3; ++family) {
                const int receiver = landmark_expected[family][sequence][0];
                const int rotation = landmark_expected[family][sequence][1];
                const bool admitted = (enabled & (1 << family)) && receiver != -1;
                int found = 0;
                for (const Draw &d : draws) {
                    bool belongs = false;
                    for (int index = 0; index < family_sizes[family]; ++index)
                        belongs |= d.receiver == families[family] + index;
                    if (!belongs) continue;
                    ++found; check(admitted);
                    check(receiver >= 0 && receiver < family_sizes[family]);
                    check(d.receiver == families[family] + receiver);
                    const bool keyed = family == 0 && sequence == 0;
                    check(d.kind == (keyed ? "key4" : "trans"));
                    check(d.vertices.size() == 4 && d.uv.size() == 4);
                    check(d.light.size() == (keyed ? 0u : 4u));
                    for (int c = 0; c < 4; ++c) {
                        const int corner = (rotation + c) & 3;
                        check(d.vertices[c].x == 164);
                        check(d.vertices[c].y == 100 + corner * 16);
                        if (!keyed) check(d.light[c] == 55 + ((corner * 3 + 1) & 3) * 3);
                        const int mount_uv[4][2] = {{0,0},{55,0},{55,55},{0,55}};
                        const int other_uv[4][2] = {{0,55},{0,0},{55,0},{55,55}};
                        check(d.uv[c].x == (family == 0 ? mount_uv[c][0] : other_uv[c][0]));
                        check(d.uv[c].y == (family == 0 ? mount_uv[c][1] : other_uv[c][1]));
                    }
                }
                check(found == (admitted ? 1 : 0));
                expected_draws += admitted ? 1 : 0;
            }
            check(draws.size() == 4u + expected_draws);
        }
        treset(); tile->bit2 = (static_cast<unsigned>(sequence) << 24) | 0x103;
        GameDataStateStorage.fields_.game_rules_ = 0x100; PlayersData[1].flags = 0; tile->visibility = 0;
        gen_terrain_poly(&buffer, &window, 5, 1, 0, 1, 0, 0, 100, 100);
        check(draws.size() == 4); // fog suppresses all three families
    }
    // Negative signed-byte codes are within the native negative-index skip
    // path. Exercise these safely; unlike positive codes >=49, they do not
    // reach any of the bounded source arrays.
    for (unsigned sequence = 128; sequence < 256; ++sequence) {
        for (int enabled = 0; enabled < 8; ++enabled) {
            treset(); tile->bit2 = sequence << 24; tile->bit = 0x400000;
            for (int family = 0; family < 3; ++family)
                if (enabled & (1 << family)) tile->bit2 |= gates[family];
            check(gen_terrain_poly(&buffer, &window, 5, 1, 0, 0x21, 0, 0, 100, 100) == 1);
            check(draws.size() == 4);
            for (const Draw &d : draws) check(d.vertices.size() == 3);
            check(g_TER1_CONDENSER_SPRITE.draw_calls == 1);
            check(TextureLightFloor == -256 && TextureLightCeiling == 256);
        }
    }
    // ---- the fungus row: 8-neighbour mask; worked land is NOT +15 ----
    {
        treset();
        tile->climate = 0x60 | 0;
        tile->bit = 0x20;                        // fungus on this tile
        map_tiles()[4].climate = 0x60;           // land west neighbour
        map_tiles()[4].bit = 0x20;               // with fungus
        LandMoistureTable_storage[8][0] = 4;       // row for mask 8
        LandMoistureTable_storage[8][1] = 2;
        gen_terrain_poly(&buffer, &window, 5, 1, 0, 1, 0, 0, 100, 100);
        // Original 4650A6 tests local_18 (water), not local_1c (worked).
        check(g_FUNGUS_TEXTURES[4].key5 == 1);
        check(g_FUNGUS_TEXTURES[4 + 15].key5 == 0);
        for (int corner = 0; corner < 4; ++corner) {
            const int vertex = ((corner + 2) & 3) * 3 + 1;
            check(draws.back().uv[corner].x == 10 + vertex * 2);
            check(draws.back().uv[corner].y == 11 + vertex * 2);
        }
        treset();
        tile->climate = 0x40;
        tile->bit = 0x20;
        map_tiles()[4].climate = 0x40;
        map_tiles()[4].bit = 0x20;
        LandMoistureTable_storage[8][0] = 4;
        gen_terrain_poly(&buffer, &window, 5, 1, 0, 1, 0, 0, 100, 100);
        // Normal ocean reaches the same overlay after its ocean pass.
        check(g_FUNGUS_TEXTURES[4 + 15].key5 == 1);
        check(g_FUNGUS_TEXTURES[4].key5 == 0);
    }

    // ---- the jungle overlay: base-radius mask row, keyed quad ----
    // Original 464E24..464EA2 collects a DIAGONAL neighbour mask into the
    // dead a5 slot, then 464EB6 indexes the receiver with the EARLIER
    // 8-neighbour base-radius mask. A non-diagonal base-radius neighbour
    // selects its row while the diagonal mask stays empty, and a diagonal
    // neighbour must not redirect the row through the dead a5 slot.
    {
        treset();
        tile->climate = 0x60 | 0;
        tile->bit2 = 0x4;               // base radius in range: local_30
        map_tiles()[1].climate = 0x60;  // RadiusBase[1] = (2,0): mask bit 2
        map_tiles()[1].bit2 = 0x4;      // land neighbour with base radius
        LandMoistureTable_storage[4][0] = 7;   // row 7 for mask 4
        gen_terrain_poly(&buffer, &window, 5, 1, 0, 1, 0, 0, 100, 100);
        check(g_JUNGLE_LAND_TEXTURES[7].key4 == 1);
        check(g_JUNGLE_LAND_TEXTURES[0].key4 == 0);  // not the empty diagonal mask
        treset();
        tile->climate = 0x60 | 0;
        tile->bit2 = 0x4;
        map_tiles()[4].climate = 0x60;  // RiverStep[1] = (1,1): a5 bit 1
        map_tiles()[4].bit2 = 0x4;      // and RadiusBase[2]: mask bit 3
        LandMoistureTable_storage[8][0] = 6;   // 8-neighbour row 6
        LandMoistureTable_storage[8][1] = 2;   // rotation 2
        LandMoistureTable_storage[2][0] = 1;   // dead a5 row: must not win
        gen_terrain_poly(&buffer, &window, 5, 1, 0, 1, 0, 0, 100, 100);
        check(g_JUNGLE_LAND_TEXTURES[6].key4 == 1);
        check(g_JUNGLE_LAND_TEXTURES[1].key4 == 0);
        check(draws.size() == 5);
        const Draw &d = draws.back();
        check(d.kind == "key4" && d.receiver == &g_JUNGLE_LAND_TEXTURES[6]);
        check(d.vertices.size() == 4 && d.light.empty());
        for (int corner = 0; corner < 4; ++corner) {
            const int vertex = ((2 + corner) & 3) * 3 + 1;
            check(d.vertices[corner].x == 164);
            check(d.vertices[corner].y == 100 + corner * 16);
            check(d.uv[corner].x == 10 + vertex * 2);
            check(d.uv[corner].y == 11 + vertex * 2);
        }
    }

    // ---- the road/magtube edges and the base selection ----
    {
        treset();
        tile->climate = 0x60 | 0;
        tile->bit = 0x4;                          // road on this tile
        map_tiles()[1 * 4 + 0].climate = 0x60;    // neighbour (x=1)
        map_tiles()[1 * 4 + 0].bit = 0x4;         // with road (bit 2)
        map_tiles()[4].bit_visible[0] = 0x4;
        gen_terrain_poly(&buffer, &window, 5, 1, 0, 0x21, 0, 0, 100, 100);
        check(g_ROAD_TEXTURES[1 + ((2 + 2) & 7)].trans == 1);
        check(g_MAGTUBE_TEXTURES[1].trans == 0);
        // a magtube neighbour (bit 3) flips the receiver family
        treset();
        tile->climate = 0x60 | 0;
        tile->bit = 0xc;
        map_tiles()[1 * 4 + 0].climate = 0x60;
        map_tiles()[1 * 4 + 0].bit = 0xc;
        map_tiles()[4].bit_visible[0] = 0xc;
        gen_terrain_poly(&buffer, &window, 5, 1, 0, 0x21, 0, 0, 100, 100);
        check(g_MAGTUBE_TEXTURES[1 + ((2 + 2) & 7)].trans == 1);
        check(g_ROAD_TEXTURES[1].trans == 0);
        // no road/base neighbours at all: the base selection draws the
        // plain centre tile from row 0 of the matching family
        treset();
        tile->climate = 0x60 | 0;
        tile->bit = 0x4;
        gen_terrain_poly(&buffer, &window, 5, 1, 0, 0x21, 0, 0, 100, 100);
        check(g_ROAD_TEXTURES[0].trans == 1);
        check(g_MAGTUBE_TEXTURES[0].trans == 0);
        for (int i = 1; i < 9; ++i) {
            check(g_ROAD_TEXTURES[i].trans == 0);
            check(g_MAGTUBE_TEXTURES[i].trans == 0);
        }
        // a magtube on this tile (bit 3) flips the row-0 family too
        treset();
        tile->climate = 0x60 | 0;
        tile->bit = 0xc;
        gen_terrain_poly(&buffer, &window, 5, 1, 0, 0x21, 0, 0, 100, 100);
        check(g_MAGTUBE_TEXTURES[0].trans == 1);
        check(g_ROAD_TEXTURES[0].trans == 0);
    }

    // ---- fresh river-neighbour mask, plus independent source overlay ----
    {
        treset();
        tile->bit = 0x80 | 0x200;
        map_tiles()[4].bit = 0x80;    // RiverStep[1] = (1,1): bit 1
        gen_terrain_poly(&buffer, &window, 5, 1, 0, 0x401, 0, 0, 100, 100);
        check(g_RIVER_TEXTURES[2].trans3 == 1);
        check(g_RIVER_TEXTURES[0].trans3 == 1);
    }

    // ---- the wreck case selection ----
    {
        treset();
        tile->climate = 0x60 | 0;
        tile->bit2 = 0x4000 | (0x0cu << 24);      // wreck, sequence 12
        ExpansionEnabled = 1;
        gen_terrain_poly(&buffer, &window, 5, 1, 0, 0x21, 0, 0, 100, 100);
        check(g_TER1WRECK_UNITY_WRECKAGE_SPRITES[10].draw_calls == 1);
        treset();
        tile->climate = 0x60 | 0;
        tile->bit2 = 0x4000 | (0x14u << 24);      // sequence 20 -> [14]
        ExpansionEnabled = 1;
        gen_terrain_poly(&buffer, &window, 5, 1, 0, 0x21, 0, 0, 100, 100);
        check(g_TER1WRECK_UNITY_WRECKAGE_SPRITES[14].draw_calls == 1);
    }

    // ---- the improvement sprite flag guards ----
    {
        treset();
        tile->climate = 0x60 | 0;
        tile->bit = 0x400000;                      // condenser bit
        gen_terrain_poly(&buffer, &window, 5, 1, 0, 0x21, 0, 0, 100, 100);
        check(g_TER1_CONDENSER_SPRITE.draw_calls == 1);
        check(g_TER1_BUNKER_SPRITE.draw_calls == 0);
        treset();
        tile->climate = 0x60 | 0;
        tile->bit = 0x800;                         // bunker bit
        gen_terrain_poly(&buffer, &window, 5, 1, 0, 0x21, 0, 0, 100, 100);
        check(g_TER1_BUNKER_SPRITE.draw_calls == 1);
        // the borehole cluster select on bit2 0x1000
        treset();
        tile->climate = 0x60 | 0;
        tile->bit = 0x1000000;
        tile->bit2 = 0x1000;
        gen_terrain_poly(&buffer, &window, 5, 1, 0, 0x21, 0, 0, 100, 100);
        check(g_TER1_BOREHOLE_CLUSTER_SPRITE.draw_calls == 1);
        check(g_TER1_BOREHOLE_SPRITE.draw_calls == 0);
    }

    // ---- the grid overlays ----
    {
        treset();
        tile->climate = 0x60 | 2;      // independent temperature selects row 4
        gen_terrain_poly(&buffer, &window, 5, 1, 0, 0x21 | 8, 0, 0, 100,
                         100);
        check(g_TER1_WHITE_ORG_YEL_TILE_SPRITES[4].draw_calls == 1);
        treset();
        tile->climate = 0x60 | (1 << 3);
        gen_terrain_poly(&buffer, &window, 5, 1, 0, 0x21 | 0x10, 0, 0, 100,
                         100);
        // (climate & 0x18) >> 3 == 1 -> row 3 with grid_row 0
        check(g_TER1_BOTTOM_LEFT_TILE_SPRITES[3].draw_calls == 1);
    }

    // ---- the wireframe exit draws four 0xc2 lines ----
    {
        treset();
        tile->climate = 0x60 | 0;
        gen_terrain_poly(&buffer, &window, 5, 1, 0x16, 1, 0, 0, 100, 100);
        check(Line_calls == 4 && Line_color == 0xc2);
        for (int i = 0; i < 4; ++i) {
            check(lines[i].x0 == 164 && lines[i].x1 == 164);
            check(lines[i].y0 == 100 + i * 16);
            check(lines[i].y1 == 100 + ((i + 1) & 3) * 16);
        }
    }
    return 0;
}
'''.replace('// ACTUAL LANDMARK OWNERS', landmark_owners()).replace(
    '// ORIGINAL LANDMARK ORACLE', landmark_oracle()).replace(
    '// ACTUAL COAST DECLARATION', coast_palette_declaration()).replace(
    '// ACTUAL COAST OWNER', coast_palette_owner()).replace('// ACTUAL MAP', body((ROOT / 'src/map.h').read_text(),
                                'struct Map {') + ';').replace('// ACTUAL VEH', body((ROOT / 'src/veh.h').read_text(),
                                'struct Veh {') + ';').replace('// SOURCE', source_text)


def coast_palette_declaration():
    declaration, = re.findall(r'^extern unsigned char TerrainCoastPalette\[12\];$',
                             (ROOT / 'src/game.h').read_text(), re.M)
    return declaration


def coast_palette_owner():
    owner, = re.findall(r'^unsigned char TerrainCoastPalette\[12\];$',
                       (ROOT / 'src/game.cpp').read_text(), re.M)
    return owner


LANDMARKS = (
    ('MountPlanetTable', 'terrain_mount_planet.inc', 0x685A98),
    ('SunnyMesaTable', 'terrain_sunny_mesa.inc', 0x685AFC),
    ('GarlandCraterTable', 'terrain_garland_crater.inc', 0x685BD4),
)


@lru_cache(maxsize=1)
def landmark_original():
    """Keep extracted proprietary pairs only in memory and pytest's /tmp.

    This fixture uses real source bindings and the pinned PE reader; separate
    native data checks establish emitted layout/bytes. Missing images skip
    these combined terrain-body cases, not the independent MapWin controls.
    Invalid explicit images fail rather than falling back to invented data.
    """
    from match_recovery.object_source import read_objects
    from match_recovery.pe_coff import PeImage
    from match_recovery.source_inventory import image_pin
    from match_recovery.model import Span

    configured = os.environ.get('OPENSMACX_IMAGE')
    candidates = ([Path(configured)] if configured else
                  [parent / '.opensmacx/game/terranx_original.exe'
                   for parent in (ROOT, *ROOT.parents)])
    original = next((path for path in candidates if path.is_file()), None)
    if original is None:
        if configured:
            pytest.fail('configured OPENSMACX_IMAGE is unavailable')
        pytest.skip('pinned original required for actual landmark-table controls')
    pin = image_pin(ROOT)
    image = PeImage(original.read_bytes(), pin)
    bindings = read_objects(ROOT, pin)
    result = {}
    for name, include, address in LANDMARKS:
        binding, = [entry for entry in bindings
                    if entry.unit == 'src/mapwin.cpp' and entry.symbol == '_' + name]
        assert binding.span == Span(address, address + 98)
        raw = image.read(binding.span)
        assert image.read_initial(binding.span) == raw
        assert not image.absolute_sites(binding.span)
        result[include] = tuple((int.from_bytes(raw[i:i+1], 'little', signed=True),
                                 int.from_bytes(raw[i+1:i+2], 'little', signed=True))
                                for i in range(0, 98, 2))
    return result


def landmark_owners():
    cpp = (ROOT / 'src/mapwin.cpp').read_text()
    parts = [body(cpp, 'struct LandmarkTextureChoice {') + ';', '''
#include <type_traits>
static_assert(sizeof(LandmarkTextureChoice) == 2, "actual pair stride");
static_assert(offsetof(LandmarkTextureChoice, texture) == 0, "texture byte");
static_assert(offsetof(LandmarkTextureChoice, rotation) == 1, "rotation byte");
static_assert(std::is_same<decltype(LandmarkTextureChoice::texture), signed char>::value, "signed sentinel");
static_assert(std::is_same<decltype(LandmarkTextureChoice::rotation), signed char>::value, "signed rotation");
''']
    for name, include, _ in LANDMARKS:
        owner, = re.findall(
            rf'static const LandmarkTextureChoice {name}\[49\]\s*=\s*\{{\s*'
            rf'#include "{include}"\s*\}};', cpp)
        parts.append(owner)
    return '\n'.join(parts)


def landmark_oracle():
    # The consumer oracle is decoded from pinned original bytes independently
    # of candidate constants and expressions; no table bytes enter Git.
    original = landmark_original()
    tables = ['{' + ','.join('{%d,%d}' % pair for pair in original[include]) + '}'
              for _, include, _ in LANDMARKS]
    return 'const int landmark_expected[3][49][2] = {' + ','.join(tables) + '};'


def terrain_run(tmp_path, text):
    compiler = shutil.which('clang++') or shutil.which('g++')
    if not compiler:
        pytest.skip('host C++ compiler unavailable')
    cpp = tmp_path / 'terrain_poly.cpp'
    exe = tmp_path / 'terrain_poly'
    if 'struct LandmarkTextureChoice' in text:
        for _, include, _ in LANDMARKS:
            pairs = landmark_original()[include]
            (tmp_path / include).write_text('\n'.join('{%d,%d},' % pair for pair in pairs) + '\n')
    cpp.write_text(text)
    built = subprocess.run([compiler, '-std=c++11', '-O0', '-g',
                            '-fsanitize=address,undefined',
                            '-fno-sanitize-recover=all', '-fno-omit-frame-pointer',
                            '-no-pie',
                            str(cpp), '-o', str(exe)],
                           capture_output=True, text=True)
    assert built.returncode == 0, built.stderr
    # LeakSanitizer cannot enumerate threads under the traced test runner;
    # address and undefined-behaviour instrumentation remain fully enabled.
    env = dict(os.environ, ASAN_OPTIONS='detect_leaks=0')
    return subprocess.run([str(exe)], capture_output=True, text=True, env=env)


def test_actual_terrain_poly_receivers(tmp_path):
    result = terrain_run(tmp_path, terrain_harness(terrain_source()))
    assert result.returncode == 0, result.stderr


@pytest.mark.parametrize('family', ['MountPlanetTable', 'SunnyMesaTable', 'GarlandCraterTable'])
@pytest.mark.parametrize('damage', ['receiver', 'rotation', 'sentinel', 'rotation_sentinel'])
def test_landmark_pair_reads_are_observed(tmp_path, family, damage):
    source_text = terrain_source()
    index = 'local_94' if family == 'MountPlanetTable' else 'iVar5'
    field = f'{family}[{index}]'
    if damage == 'receiver':
        # Preserve the sentinel guard so the wrong receiver is a defined,
        # in-bounds draw rejected by the oracle rather than an ASan crash.
        old, new = f'[{field}.texture]', '[0]'
    elif damage == 'rotation':
        old, new = f'(uint32_t){field}.rotation', '(uint32_t)0'
    elif damage == 'sentinel':
        old, new = f'{field}.texture != -1', f'{field}.texture > 0'
    else:
        old = f'{field}.texture != -1'
        # A paired-sentinel assumption is observable for Crater31 only;
        # for the other families test a false exclusion of valid rotation0.
        comparison = '>= 0' if family == 'GarlandCraterTable' else '!= 0'
        new = f'({old} && {field}.rotation {comparison})'
    assert old in source_text
    result = terrain_run(tmp_path, terrain_harness(source_text.replace(old, new)))
    assert result.returncode == 91 and 'assertion at' in result.stderr, result.stderr


@pytest.mark.parametrize(('old', 'new'), [
    ('if (local_94 == 0) {', 'if (local_94 == 1) {'),
    ('if ((local_34 & 0x100) != 0 && local_70 == 0 && -1 < iVar5 &&',
     'else if ((local_34 & 0x100) != 0 && local_70 == 0 && -1 < iVar5 &&'),
    ('if ((local_34 & 1) != 0 && local_70 == 0 && -1 < iVar5 &&',
     'else if ((local_34 & 1) != 0 && local_70 == 0 && -1 < iVar5 &&'),
    ('quad_z[1] = local_b8[k1];', 'quad_z[1] = local_b8[k0];'),
    ('quad[2].y = MapCornerScratch[k2].y;', 'quad[2].y = MapCornerScratch[k1].y;'),
    ('local_70 == 0 && (local_34 & 2) != 0', 'true && (local_34 & 2) != 0'),
    ('(local_34 & 0x100) != 0 && local_70 == 0', '(local_34 & 0x100) != 0 && true'),
    ('(local_34 & 1) != 0 && local_70 == 0', '(local_34 & 1) != 0 && true'),
])
def test_landmark_control_and_geometry_damage_is_observed(tmp_path, old, new):
    source_text = terrain_source()
    assert old in source_text
    result = terrain_run(tmp_path, terrain_harness(source_text.replace(old, new)))
    assert result.returncode == 91 and 'assertion at' in result.stderr, result.stderr


@pytest.mark.parametrize('access', ['bVar11 + 2', '7 - shade'])
def test_coast_palette_callback_reload_is_observed(tmp_path, access):
    """A frozen palette snapshot loses the next corner's live owner reads.

    This checks renderer service-boundary semantics only. The actual loader,
    Texture rasterizer, native linkage and original process aren't executed.
    """
    actual = terrain_source()
    anchor = 'if (-8 < a3 && local_18 == 0) {'
    assert actual.count(anchor) == 1
    actual = actual.replace(anchor, anchor + '''
        unsigned char frozen_palette[12];
        for (int sample = 0; sample < 12; ++sample)
            frozen_palette[sample] = TerrainCoastPalette[sample];
''')
    old = f'TerrainCoastPalette[{access}]'
    assert actual.count(old) == 1
    actual = actual.replace(old, f'frozen_palette[{access}]')
    result = terrain_run(tmp_path, terrain_harness(actual))
    assert result.returncode == 91, result.stderr


@pytest.mark.parametrize('damage', [
    'land_floor', 'land_ceiling', 'ocean_entry', 'early_reset',
    *[f'{word}_exit_{index}' for word in ('floor', 'ceiling') for index in range(4)],
])
def test_terrain_clamp_publication_damage_is_observed(tmp_path, damage):
    """Compiled caller mutations, not a fake Texture ownership proof.

    Original clamp writes are observed at real-body service boundaries. In
    particular a good final default cannot mask incorrect in-call publication,
    and a good common exit cannot mask one of the three early flat exits.
    """
    source = terrain_source()
    if '_exit_' in damage:
        word, index = damage.split('_exit_')
        good = ('TextureLightFloor = 0xffffff00;' if word == 'floor'
                else 'TextureLightCeiling = 0x100;')
        assert source.count(good) == 4
        chunks = source.split(good)
        site = int(index)
        source = good.join(chunks[:site + 1]) + '(void)0;' + good.join(chunks[site + 1:])
    else:
        good, bad = {
            'land_floor': ('TextureLightFloor = 0xfffffffc;', 'TextureLightFloor = -3;'),
            'land_ceiling': ('TextureLightCeiling = 4;', 'TextureLightCeiling = 5;'),
            'ocean_entry': ('if (local_18 == 0) {\n        TextureLightFloor',
                            'if (true) {\n        TextureLightFloor'),
            'early_reset': ('if (a8 < 0 || MapLatitudeBounds <= a8 || a7 < 0 ||',
                            'TextureLightFloor = -256;\n    TextureLightCeiling = 256;\n    if (a8 < 0 || MapLatitudeBounds <= a8 || a7 < 0 ||'),
        }[damage]
        assert source.count(good) == 1, damage
        source = source.replace(good, bad)
    result = terrain_run(tmp_path, terrain_harness(source))
    assert result.returncode == 91 and 'assertion at' in result.stderr, result.stderr


def test_actual_land_fan_tables():
    """Real initializers agree with the independently established tile fan.

    The executable-backed native data check establishes original bytes; this
    control expresses geometry and keeps the host body's synthetic tables
    from becoming the only coverage of these now-real source definitions.
    """
    cpp = (ROOT / 'src/mapwin.cpp').read_text()
    values = {}
    for name in ('LandCornerXStep', 'LandCornerYStep', 'LandCornerId'):
        match, = re.findall(
            rf'static const int {name}\[12\]\s*=\s*\{{([^}}]+)\}};', cpp)
        values[name] = [int(value) for value in re.findall(r'-?\d+', match)]
    corners = {0: (0, 0), 1: (-2, 0), 2: (0, -2),
               3: (2, 0), 4: (0, 2)}
    ids = [0, 1, 2, 0, 2, 3, 0, 3, 4, 0, 4, 1]
    assert values['LandCornerId'] == ids
    assert list(zip(values['LandCornerXStep'], values['LandCornerYStep'])) == [
        corners[corner] for corner in ids]


@pytest.mark.parametrize(('old', 'new'), [
    ('g_FARM_TEXTURES[local_84 + 3 * local_c4]',
     'g_FARM_TEXTURES[local_84 + 3 * (local_c4 + 1)]'),
    ('g_ROCKY_TEXTURES[local_c4 + 2 * local_38 - 2]',
     'g_ROCKY_TEXTURES[local_c4 + 2 * local_38]'),
    ('g_MOUNT_PLANET_TEXTURES[MountPlanetTable[local_94].texture]',
     'g_SUNNY_MESA_TEXTURES[MountPlanetTable[local_94].texture]'),
    ('g_ROAD_TEXTURES[1 + ((k + 2) & 7)]',
     'g_ROAD_TEXTURES[1 + ((k + 3) & 7)]'),
    ('g_TER1WRECK_UNITY_WRECKAGE_SPRITES[10]',
     'g_TER1WRECK_UNITY_WRECKAGE_SPRITES[11]'),
    ('g_TER1_CONDENSER_SPRITE.draw(', 'g_TER1_BUNKER_SPRITE.draw('),
    ('(tile->climate & 7) * 2', '(tile->climate & 7) * 1'),
    ('g_RIVER_TEXTURES[uVar12]', 'g_RIVER_TEXTURES[0]'),
    ('g_JUNGLE_LAND_TEXTURES[LandMoistureTable[local_2c][0]]',
     'g_JUNGLE_LAND_TEXTURES[LandMoistureTable[a5 & 0xff][0]]'),
    ('g_ROAD_TEXTURES[0].draw_trans(', 'g_ROAD_TEXTURES[1].draw_trans('),
    ('LandCornerXStep[vertex] * local_78 + a9',
     'LandCornerXStep[vertex] * local_78 + a10'),
    ('LandCornerUVs[uv].y', 'LandCornerUVs[uv].x'),
    ('LandCornerUVs[uVar17 * 3 + 1]', 'LandCornerUVs[uVar17 * 3]'),
    ('LandCornerUVs + tri * 3, shade', 'land_uv + tri * 3, shade'),
    ('land_vertices + tri * 3', 'land_vertices + tri * 2'),
    ('EdgeMidUVs[vertex].y', 'EdgeMidUVs[vertex].x'),
    ('CoastMaskTable[iVar19][uVar12]', 'CoastMaskTable[iVar19][0]'),
    ('TerrainCoastPalette[bVar11 + 2]', 'TerrainCoastPalette[bVar11]'),
    ('TerrainCoastPalette[bVar11 + 2]', 'TerrainCoastPalette[bVar11 + 3]'),
    ('TerrainCoastPalette[7 - shade]', 'TerrainCoastPalette[4 + shade]'),
    ('TerrainCoastPalette[7 - shade]', 'TerrainCoastPalette[8 - shade]'),
    ('TerrainCoastPalette[7 - shade]', 'TerrainCoastPalette[7]'),
    ('0x12 + run * 4 + shade', '0x12 + run * 3 + shade'),
    ('run < 3', 'run < 2'),
    ('shade < 4', 'shade < 3'),
    ('((a8 - a7 & 2) != 0)', '((a8 - a7 & 2) == 0)'),
    ('const int next = (fan_index + 1) & 3;',
     'const int next = fan_index;'),
    ('OceanCornerShades + corner * 4,', 'OceanCornerShades + corner,'),
    ('PlayersData[a4].last_turn_new_base', 'PlayersData[a4].flags'),
    ('PlayersData[a4].diplo_treaties[uVar12]',
     'PlayersData[uVar12].diplo_treaties[a4]'),
    ('PlayersData[uVar12].diplo_treaties[a4]',
     'PlayersData[a4].diplo_treaties[uVar12]'),
    ('15 * (local_18 != 0)', '15 * (local_1c != 0)'),
    ('tile->visibility &', 'tile->val2 &'),
    ('tile->bit_visible[a4 - 1]', 'tile->bit_visible[a4]'),
    ('a4 == 0 ? tile->bit2', 'a4 == 0 ? tile->bit'),
    ('PlayersData[a4].flags & 0x200', 'PlayersData[a4].flags & 2'),
    ('Vehs[1895].x & 0x200', 'Vehs[1895].morale & 0x200'),
    ('GameDataStateStorage.fields_.game_rules_ & 0x100',
     'GameDataStateStorage.fields_.game_preferences_ & 0x100'),
    ('].territory ==', '].val3 =='),
    ('RiverStepX[local_30b - 1]', 'RadiusBaseX[local_30b - 1]'),
    ('RiverStepY[local_30b - 1]', 'RadiusBaseY[local_30b - 1]'),
    ('(a6 & 0x1000) != 0', '(a6 & 0x1000) == 0'),
    ('if ((a6 & 0x200) != 0) {\n        iVar5 = bonus_at',
     'if ((a6 & 0x200) != 0 && -8 < a3) {\n        iVar5 = bonus_at'),
    *[(f'({field} & 1)', '(0 & 1)') for field in (
        'Vehs[1895].waypoint_y[0]', 'Vehs[1895].waypoint_y[2]',
        'Vehs[1895].morale', 'Vehs[1895].moves_expended',
        'Vehs[1895].probe_action', 'Vehs[1895].next_veh_id_stack',
        'Vehs[1896].x',
    )],
])
def test_terrain_receiver_damage_is_observed(tmp_path, old, new):
    actual = terrain_source()
    assert actual.count(old) >= 1, old
    result = terrain_run(tmp_path, terrain_harness(actual.replace(old, new)))
    assert result.returncode == 91, result.stderr
