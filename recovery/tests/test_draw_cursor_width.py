"""Actual-source width-binding control for draw_cursor's seam probes."""
from pathlib import Path
import os
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
    raise AssertionError("unterminated draw_cursor")


HARNESS = r'''#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
void require(bool ok,int line){if(!ok){fprintf(stderr,"line %d\n",line);exit(91);}}
#define CHECK(x) require((x),__LINE__)
struct MapWin {
    int field_1DD74_,field_1DDA8_,field_1DDA4_,field_1DDD4_,field_1DDCC_,field_1DDD8_,field_1DDD0_;
    int calls,last_x,last_y,last_c,last_d;
    void draw_radius(int x,int y,int c,int d){++calls;last_x=x;last_y=y;last_c=c;last_d=d;}
};
typedef MapWin *MapWinSlots[8];
static const size_t MapWinTableSlots=8;
MapWinSlots MapWinTable;
int MapLongitudeBounds=16;
uint32_t MapLongitude=8; // true half-width tile-row stride
int MapIsFlat=0;
struct ConsoleType {int survey_mode_,cursor_history_index_,cursor_history_y_[4],cursor_history_x_[4];} ConsoleGlobal;
struct TutWinType {int calls;void draw_arrow(){++calls;}} tut_window;
TutWinType *TutWinGlobal=&tut_window;
ACTUAL_BODY
int main(){
    CHECK(MapLongitudeBounds==16 && MapLongitude==8);
    MapWin first={};
    first.field_1DD74_=1;
    first.field_1DDA8_=0; first.field_1DDD8_=0; first.field_1DDD0_=4;
    // left=14, right=18; tile 1 must wrap by full map width to 17.
    first.field_1DDA4_=14; first.field_1DDD4_=0; first.field_1DDCC_=4;
    MapWinTable[0]=&first;
    ConsoleGlobal.cursor_history_index_=0;
    ConsoleGlobal.cursor_history_y_[0]=2;
    ConsoleGlobal.cursor_history_x_[0]=1;
    draw_cursor();
    CHECK(first.calls==1 && first.last_x==1 && first.last_y==2);
    CHECK(first.last_c==0 && first.last_d==2 && tut_window.calls==1);

    // A negative viewport range makes the opposite subtract branch observable:
    // [−4,0), tile 15 wraps to −1 with the full width, not −7 with stride 8.
    first.calls=0; first.field_1DDA4_=-4; first.field_1DDD4_=0; first.field_1DDCC_=4;
    ConsoleGlobal.cursor_history_x_[0]=15;
    draw_cursor();
    CHECK(first.calls==1 && tut_window.calls==2);

    // Flat maps do not wrap, and arrow dispatch remains unconditional.
    first.calls=0; MapIsFlat=1; ConsoleGlobal.cursor_history_x_[0]=1;
    draw_cursor();
    CHECK(first.calls==0 && tut_window.calls==3);
    return 0;
}
'''


def source_body():
    return braced((ROOT / "src/mapwin.cpp").read_text(), "void __cdecl draw_cursor()")


def run_host(tmp_path, body):
    compiler = shutil.which("clang++") or shutil.which("g++")
    if not compiler:
        pytest.skip("host C++ compiler required")
    source = tmp_path / "draw-cursor.cpp"
    source.write_text(HARNESS.replace("ACTUAL_BODY", body))
    binary = tmp_path / "draw-cursor"
    built = subprocess.run(
        [compiler,"-std=c++11","-Wall","-Wextra","-Werror",
         "-fsanitize=address,undefined","-fno-sanitize-recover=all",
         str(source),"-o",str(binary)],capture_output=True,text=True,timeout=30
    )
    assert built.returncode == 0, built.stderr
    return subprocess.run([str(binary)],capture_output=True,text=True,timeout=10,
                          env=dict(os.environ,ASAN_OPTIONS="detect_leaks=0"))


def test_actual_draw_cursor_uses_full_bounds_for_wrap(tmp_path):
    body = source_body()
    assert body.count("MapLongitudeBounds + tile_x") == 1
    assert body.count("wrapped_x - MapLongitudeBounds") == 1
    assert "MapLongitude + tile_x" not in body
    assert "wrapped_x - MapLongitude;" not in body
    result = run_host(tmp_path, body)
    assert result.returncode == 0, result.stderr


def test_half_stride_mutant_fails_width_distinguishing_controls(tmp_path):
    body = source_body()
    mutant = body.replace("MapLongitudeBounds + tile_x", "MapLongitude + tile_x")
    mutant = mutant.replace("wrapped_x - MapLongitudeBounds", "wrapped_x - MapLongitude")
    assert mutant != body
    result = run_host(tmp_path, mutant)
    assert result.returncode != 0
