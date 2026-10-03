"""Actual-source branch control for MapWin::focus, kept private for review."""
import os
import pathlib
import shutil
import subprocess
import tempfile

import pytest


HERE = pathlib.Path(__file__).resolve().parent
SOURCE_ROOT = (pathlib.Path(os.environ["OPENSMACX_SOURCE_ROOT"])
               if "OPENSMACX_SOURCE_ROOT" in os.environ else HERE.parents[1])


def extract_braced(source, marker):
    start = source.index(marker)
    brace = source.index("{", start)
    depth = 0
    for end in range(brace, len(source)):
        if source[end] == "{":
            depth += 1
        elif source[end] == "}":
            depth -= 1
            if depth == 0:
                return source[start:end + 1]
    raise AssertionError("unclosed source body: " + marker)


def control_source():
    text = (SOURCE_ROOT / "src/mapwin.cpp").read_text()
    method = extract_braced(text, "int MapWin::focus(int x_coord, int y_coord)")
    helper_markers = (
        "int map_dest_signed(uint32_t value) {",
        "int mapwin_dest_add(int left, int right) {",
        "int mapwin_dest_sub(int left, int right) {",
        "int mapwin_dest_mul(int left, int right) {",
    )
    helpers = "\n".join(extract_braced(text, marker) for marker in helper_markers)
    fixture = r'''#include <assert.h>
#include <stdint.h>
#define __cdecl
struct RECT { int left, top, right, bottom; };
struct MapWin {
    uint32_t field_1DD70_, field_1DD78_;
    int field_1DDA4_, field_1DDA8_, field_1DDAC_, field_1DDB0_;
    int field_1DDCC_, field_1DDD4_, field_1DDD8_;
    int field_21A44_;
    int field_21A68_, field_21A6C_, field_21A70_;
    int field_23BFC_, field_23BF8_, cursor_active_;
    int called_, center_x_, center_y_, draw_type_;
    void set_center(int x, int y, int draw_type) {
        ++called_; center_x_ = x; center_y_ = y; draw_type_ = draw_type;
    }
    int focus(int, int);
};
struct ConsoleDouble : MapWin { };
static ConsoleDouble ConsoleGlobal;
static RECT MapWinPrimaryCorner = {0, 0, 0, 0};
static int MapIsFlat = 0;
static int MapLongitudeBounds = 100;
static int MapLatitudeBounds = 100;
static int in_box_result = 0;
static int in_box(int, int, RECT *) { return in_box_result; }
''' + helpers + "\n" + method + r'''
static MapWin make_map() {
    MapWin m;
    m.field_1DD70_ = 0; m.field_1DD78_ = 1;
    m.field_1DDA4_ = 20; m.field_1DDA8_ = 20;
    m.field_1DDAC_ = 0; m.field_1DDB0_ = 0;
    m.field_1DDCC_ = 2; m.field_1DDD4_ = 20; m.field_1DDD8_ = 20;
    m.field_21A44_ = 0; m.called_ = 0;
    m.center_x_ = m.center_y_ = m.draw_type_ = -1;
    return m;
}
int main() {
    MapWin interior = make_map();
    assert(interior.focus(40, 40) == 0 && interior.called_ == 0);

    MapWin left = make_map();
    assert(left.focus(22, 40) == 1 && left.called_ == 1);

    // The upper x boundary is the negative control for the old AND/less-than
    // predicate: native code enters the moved path when x >= right - step.
    MapWin right = make_map();
    assert(right.focus(56, 40) == 1 && right.called_ == 1);

    MapIsFlat = 1;
    MapWin flat = make_map();
    assert(flat.focus(56, 40) == 0 && flat.called_ == 0);
    MapIsFlat = 0;

    MapWin wrap = make_map();
    wrap.field_1DDA4_ = 96; wrap.field_1DDD4_ = 10;
    assert(wrap.focus(2, 40) == 1 && wrap.called_ == 1);
    assert(wrap.center_x_ == 12 && wrap.center_y_ == 40 && wrap.draw_type_ == 1);

    MapWinPrimaryCorner.left = MapWinPrimaryCorner.top = 0;
    MapWinPrimaryCorner.right = MapWinPrimaryCorner.bottom = 0;
    in_box_result = 1;
    assert(ConsoleGlobal.focus(40, 40) == 0 && ConsoleGlobal.called_ == 0);
    in_box_result = 0;
    ConsoleGlobal.field_1DD78_ = 0;
    assert(ConsoleGlobal.focus(40, 40) == 1 && ConsoleGlobal.called_ == 1);
    assert(ConsoleGlobal.field_21A44_ == 0);
    return 0;
}
'''
    return fixture


@pytest.mark.parametrize("compiler", ["g++", "clang++"])
def test_actual_focus_body_native_boundary_cases(compiler):
    compiler_path = shutil.which(compiler)
    if compiler_path is None:
        pytest.skip(compiler + " unavailable")
    with tempfile.TemporaryDirectory(prefix="mapwin-focus-review-") as temp:
        cpp = pathlib.Path(temp) / "focus.cpp"
        binary = pathlib.Path(temp) / "focus"
        cpp.write_text(control_source())
        subprocess.run([compiler_path, "-std=c++98", "-Wall", "-Wextra", str(cpp), "-o", str(binary)],
                       check=True, capture_output=True, text=True, timeout=30)
        subprocess.run([str(binary)], check=True, timeout=30)
