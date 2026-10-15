"""Actual-source controls for MapWin's cursor and click owner bodies.

The member and free cursor functions remain separate originals. These host
controls exercise their complete extracted bodies plus the two typed click
overrides. A real C++ virtual base makes the click receiver nonzero-offset;
this is behavioral evidence only, not native VC6 layout or thunk proof.
"""

from pathlib import Path
import os
import shutil
import subprocess

import pytest


ROOT = Path(__file__).resolve().parents[2]


def extract_body(source, signature):
    start = source.index(signature)
    opening = source.index("{", start)
    depth = 0
    for end in range(opening, len(source)):
        depth += (source[end] == "{") - (source[end] == "}")
        if depth == 0:
            return source[start:end + 1]
    raise AssertionError("unterminated function: " + signature)


def source_bodies():
    mapwin = (ROOT / "src/mapwin.cpp").read_text()
    # The member must live with its MapWin owner; the free function has its
    # separate original identity and remains independently extracted.
    return {
        "member_cursor": extract_body(mapwin, "void MapWin::draw_cursor() {"),
        "free_cursor": extract_body(mapwin, "void __cdecl draw_cursor() {"),
        "left_click": extract_body(mapwin, "void MapWin::on_left_click(int a1, int a2) {"),
        "right_click": extract_body(mapwin, "void MapWin::on_right_click(int a, int b) {"),
    }


HARNESS = r'''#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>

static void require(bool ok, int line, const char *what) {
    if (!ok) { std::fprintf(stderr, "line %d: %s\n", line, what); std::exit(91); }
}
#define CHECK(x) require((x), __LINE__, #x)
#define __cdecl

struct GraphicWin { virtual ~GraphicWin() {} };
struct PrimaryBase { virtual ~PrimaryBase() {} };
static void mutate_cursor();
struct MapWin : PrimaryBase, virtual GraphicWin {
    virtual GraphicWin *base_view() { return static_cast<GraphicWin *>(this); }
    int field_1DD74_;
    int field_1DDA4_, field_1DDA8_;
    int field_1DDCC_, field_1DDD0_, field_1DDD4_, field_1DDD8_;
    int radius_calls, radius_x, radius_y, radius_a, radius_b;
    int click_calls, click_x, click_y, click_button;
    int mutate_cursor_on_radius;
    MapWin *expected_this;
    void draw_radius(int x, int y, int a, int b) {
        CHECK(this == expected_this);
        ++radius_calls; radius_x=x; radius_y=y; radius_a=a; radius_b=b;
        if (mutate_cursor_on_radius) mutate_cursor();
    }
    void click(int x, int y, int button) {
        CHECK(this == expected_this);
        ++click_calls; click_x=x; click_y=y; click_button=button;
    }
    void draw_cursor();
    void on_left_click(int a1, int a2);
    void on_right_click(int a, int b);
};

struct Console {
    int survey_mode_;
    int cursor_history_index_;
    int32_t cursor_history_x_[32];
    int32_t cursor_history_y_[32];
} ConsoleGlobal;
static void mutate_cursor() { ConsoleGlobal.cursor_history_index_=1; }
typedef MapWin *MapWinSlots[8];
MapWinSlots MapWinTable;
const std::size_t MapWinTableSlots=8;
int MapLongitudeBounds=16;
int MapLongitude=8; // distinct half-width row stride; never cursor seam width
uint32_t MapIsFlat=0;
int MapInputEnabled=0;
struct Tutorial { int calls; void draw_arrow() { ++calls; } } TutorialWindow;

MEMBER_CURSOR
FREE_CURSOR
LEFT_CLICK
RIGHT_CLICK

static void reset(MapWin &w) {
    w.field_1DD74_=1;
    w.field_1DDA4_=14; w.field_1DDA8_=0;
    w.field_1DDCC_=4; w.field_1DDD0_=4;
    w.field_1DDD4_=0; w.field_1DDD8_=0;
    w.radius_calls=w.click_calls=0;
    w.radius_x=w.radius_y=w.radius_a=w.radius_b=0;
    w.click_x=w.click_y=w.click_button=0;
    w.mutate_cursor_on_radius=0; w.expected_this=&w;
    ConsoleGlobal.survey_mode_=0;
    ConsoleGlobal.cursor_history_index_=0;
    for (int i=0;i<32;++i) {
        ConsoleGlobal.cursor_history_x_[i]=0;
        ConsoleGlobal.cursor_history_y_[i]=0;
    }
    for (std::size_t i=0;i<MapWinTableSlots;++i) MapWinTable[i]=0;
    TutorialWindow.calls=0; MapIsFlat=0; MapLongitudeBounds=16; MapLongitude=8;
}

static void member_cursor_cases() {
    MapWin window; reset(window);
    CHECK(reinterpret_cast<char *>(&window) !=
          reinterpret_cast<char *>(static_cast<GraphicWin *>(&window)));
    ConsoleGlobal.cursor_history_x_[0]=1;
    ConsoleGlobal.cursor_history_y_[0]=2;
    // Left seam wraps 1 -> 17 for inclusion in [14,18), but the actual
    // draw_radius call deliberately receives the original X=1.
    window.draw_cursor();
    CHECK(window.radius_calls==1 && window.radius_x==1 && window.radius_y==2);
    CHECK(window.radius_a==0 && window.radius_b==2);

    reset(window);
    window.field_1DDA4_=-4; window.field_1DDD4_=0; window.field_1DDCC_=4;
    ConsoleGlobal.cursor_history_x_[0]=15; ConsoleGlobal.cursor_history_y_[0]=1;
    // Opposite seam: 15 -> -1 only for containment; radius still gets 15.
    window.draw_cursor();
    CHECK(window.radius_calls==1 && window.radius_x==15);

    reset(window);
    window.field_1DDA4_=14; window.field_1DDD4_=0; window.field_1DDCC_=4;
    ConsoleGlobal.cursor_history_x_[0]=14; ConsoleGlobal.cursor_history_y_[0]=0;
    window.draw_cursor();
    CHECK(window.radius_calls==1 && window.radius_x==14 && window.radius_y==0);

    reset(window);
    ConsoleGlobal.cursor_history_x_[0]=1; ConsoleGlobal.cursor_history_y_[0]=-1;
    window.draw_cursor();
    CHECK(window.radius_calls==0);
    ConsoleGlobal.cursor_history_y_[0]=4; // upper bound is exclusive
    window.draw_cursor();
    CHECK(window.radius_calls==0);

    reset(window); MapIsFlat=1;
    ConsoleGlobal.cursor_history_x_[0]=1; ConsoleGlobal.cursor_history_y_[0]=2;
    window.draw_cursor();
    CHECK(window.radius_calls==0); // flat map does not wrap the left edge
}

static void free_cursor_cases() {
    MapWin first, second, inactive;
    reset(first); reset(second); reset(inactive);
    first.field_1DDA4_=14; first.field_1DDCC_=4;
    second.field_1DDA4_=40; second.field_1DDCC_=4;
    inactive.field_1DDA4_=14; inactive.field_1DDCC_=4;
    first.field_1DD74_=0; second.field_1DD74_=1; inactive.field_1DD74_=0;
    first.mutate_cursor_on_radius=1;
    MapWinTable[0]=&first; MapWinTable[1]=&second; MapWinTable[2]=&inactive;
    ConsoleGlobal.cursor_history_index_=0;
    ConsoleGlobal.cursor_history_x_[0]=1; ConsoleGlobal.cursor_history_y_[0]=2;
    ConsoleGlobal.cursor_history_x_[1]=42; ConsoleGlobal.cursor_history_y_[1]=2;
    draw_cursor();
    CHECK(first.radius_calls==1 && first.radius_x==1);
    CHECK(second.radius_calls==1 && second.radius_x==42);
    CHECK(inactive.radius_calls==0);
    CHECK(TutorialWindow.calls==1);

    // The survey early suppression still falls through to the tutorial arrow.
    ConsoleGlobal.survey_mode_=1;
    draw_cursor();
    CHECK(first.radius_calls==1 && second.radius_calls==1);
    CHECK(TutorialWindow.calls==2);
}

static void click_cases() {
    MapWin window; reset(window);
    CHECK(reinterpret_cast<char *>(&window) !=
          reinterpret_cast<char *>(static_cast<GraphicWin *>(&window)));
    MapInputEnabled=0;
    window.on_left_click(3,7); window.on_right_click(5,9);
    CHECK(window.click_calls==0);
    MapInputEnabled=1;
    window.on_left_click(3,7);
    CHECK(window.click_calls==1 && window.click_x==3 && window.click_y==7 &&
          window.click_button==0);
    window.on_right_click(5,9);
    CHECK(window.click_calls==2 && window.click_x==5 && window.click_y==9 &&
          window.click_button==1);
}

int main() {
    member_cursor_cases();
    free_cursor_cases();
    click_cases();
    return 0;
}
'''


def run_host(tmp_path, compiler, bodies):
    text = HARNESS.replace("MEMBER_CURSOR", bodies["member_cursor"])
    text = text.replace("FREE_CURSOR", bodies["free_cursor"])
    text = text.replace("LEFT_CLICK", bodies["left_click"])
    text = text.replace("RIGHT_CLICK", bodies["right_click"])
    cpp = tmp_path / "mapwin-cursor-click.cpp"
    cpp.write_text(text)
    binary = tmp_path / "mapwin-cursor-click"
    built = subprocess.run(
        [compiler, "-std=c++11", "-Wall", "-Wextra", "-Werror",
         "-fsanitize=address,undefined", "-fno-sanitize-recover=all",
         str(cpp), "-o", str(binary)], capture_output=True, text=True, timeout=60)
    assert built.returncode == 0, built.stderr
    return subprocess.run([str(binary)], capture_output=True, text=True,
                          timeout=10,
                          env=dict(os.environ, ASAN_OPTIONS="detect_leaks=0"))


@pytest.mark.parametrize("compiler_name", ["g++", "clang++"])
def test_actual_mapwin_cursor_and_click_bodies(tmp_path, compiler_name):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(compiler_name + " unavailable")
    bodies = source_bodies()
    assert "void MapWin::draw_cursor()" in bodies["member_cursor"]
    assert "void __cdecl draw_cursor()" in bodies["free_cursor"]
    assert "draw_radius(x, y, 0, 2)" in bodies["member_cursor"]
    assert "draw_radius(tile_x, tile_y, 0, 2)" in bodies["free_cursor"]
    assert "click(a1, a2, 0)" in bodies["left_click"]
    assert "click(a, b, 1)" in bodies["right_click"]
    assert "0x21A6C" not in bodies["left_click"] + bodies["right_click"]
    result = run_host(tmp_path, compiler, bodies)
    assert result.returncode == 0, result.stderr


@pytest.mark.parametrize("old,new,expected,target", [
    ("draw_radius(x, y, 0, 2)", "draw_radius(adjusted_x, y, 0, 2)",
     "window.radius_calls==1 && window.radius_x==1", "member_cursor"),
    ("click(a1, a2, 0)", "click(a1, a2, 1)",
     "window.click_button==0", "left_click"),
])
def test_cursor_and_click_mutants_fail_expected_control(
        tmp_path, old, new, expected, target):
    compiler = shutil.which("g++") or shutil.which("clang++")
    if compiler is None:
        pytest.skip("host C++ compiler required")
    bodies = source_bodies()
    assert old in bodies[target]
    bodies[target] = bodies[target].replace(old, new, 1)
    result = run_host(tmp_path, compiler, bodies)
    assert result.returncode != 0
    assert expected in result.stderr


def test_member_halfstride_mutant_fails_seam_control(tmp_path):
    compiler = shutil.which("g++") or shutil.which("clang++")
    if compiler is None:
        pytest.skip("host C++ compiler required")
    bodies = source_bodies()
    body = bodies["member_cursor"]
    body = body.replace("MapLongitudeBounds + x", "MapLongitude + x", 1)
    body = body.replace("adjusted_x - MapLongitudeBounds",
                        "adjusted_x - MapLongitude", 1)
    assert body != bodies["member_cursor"]
    bodies["member_cursor"] = body
    result = run_host(tmp_path, compiler, bodies)
    assert result.returncode != 0
    assert "window.radius_calls==1 && window.radius_x==1" in result.stderr
