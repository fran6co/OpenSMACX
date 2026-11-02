"""Actual outline bodies checked against pinned corners and callback read phases."""
from pathlib import Path
import subprocess

import pytest

from test_movie_runtime_decoder_ownership_actual_source_oct9 import _compile_run, _definition


SOURCE = Path(__file__).resolve().parents[2] / "src/worldwin.cpp"


def _unit(source: str) -> str:
    bodies = "\n".join(_definition(source, signature) for signature in (
        "void WorldWin::draw_viewport_outline()",
        "void WorldWin::draw_box()",
    ))
    return r'''#include <cassert>
#include <functional>
#include <vector>
// Keep LONG's distinct nominal type so int-output APIs require real temporaries.
// These bounded coordinates do not depend on the host long width.
struct RECT { long left, top, right, bottom; };
struct DIB { struct { int biWidth, biHeight; } bmiHeader; };
struct GraphicWin { DIB dib_; };
struct MapWin : GraphicWin {
    int field_1DDA4_, field_1DDA8_, field_1DDD4_, field_1DDD8_;
    int field_1DDCC_, field_1DDD0_, field_1DDAC_, field_1DDB0_;
};
MapWin ConsoleGlobal, first_detail, second_detail, decoy;
MapWin *MapWinTable[2];
struct Line { char kind; int a,b,c,color; };
static std::vector<Line> lines;
static std::vector<int> input;
static std::function<void(int)> after_tile;
static std::function<void(int)> after_line;
static int tile_calls, tile_failure, left_value, right_value;
static int *saved_left;
struct Buffer {
    DIB dib_;
    void box(RECT *r,int a,int b) {
        assert(a==b); lines.push_back({'B',r->left,r->right,r->top,a});
    }
    void hline(int a,int b,int c,int color) {
        lines.push_back({'H',a,b,c,color});
        if(after_line) after_line(static_cast<int>(lines.size()));
    }
    void vline(int a,int b,int c,int color) {
        lines.push_back({'V',a,b,c,color});
        if(after_line) after_line(static_cast<int>(lines.size()));
    }
};
struct WorldWin : Buffer {
    int viewport_outline_enabled_, field_239C4_;
    MapWin mapWin_;
    void draw_viewport_outline();
    void draw_box();
    int tile_to_pixel(int x,int y,int *out_x,int *out_y) {
        ++tile_calls; input.push_back(x); input.push_back(y);
        *out_x=tile_calls==1?left_value:right_value;
        *out_y=tile_calls==1?10:40;
        if(tile_calls==1) saved_left=out_x;
        if(after_tile) after_tile(tile_calls);
        return tile_calls==tile_failure;
    }
};
''' + bodies + r'''
static void reset(WorldWin &w) {
    w={};w.viewport_outline_enabled_=1;w.field_239C4_=3;
    w.dib_.bmiHeader.biWidth=60;w.mapWin_.dib_.bmiHeader.biWidth=100;
    ConsoleGlobal={};ConsoleGlobal.field_1DDA4_=10;ConsoleGlobal.field_1DDA8_=20;
    ConsoleGlobal.field_1DDCC_=3;ConsoleGlobal.field_1DDD0_=4;
    ConsoleGlobal.field_1DDD4_=5;ConsoleGlobal.field_1DDD8_=6;
    decoy={};MapWinTable[0]=&decoy;
    first_detail=ConsoleGlobal;first_detail.field_1DDA4_=100;first_detail.field_1DDA8_=200;
    second_detail=ConsoleGlobal;second_detail.field_1DDA4_=300;second_detail.field_1DDA8_=400;
    second_detail.field_1DDCC_=7;second_detail.field_1DDD0_=8;
    second_detail.field_1DDD4_=9;second_detail.field_1DDD8_=10;
    MapWinTable[1]=&first_detail;
    lines.clear();input.clear();tile_calls=0;tile_failure=0;
    left_value=90;right_value=20;after_tile={};after_line={};saved_left=nullptr;
}
static void check_lines(int color) {
    const Line expected[]={
        {'H',3,19,10,color},{'H',3,18,39,color},{'V',19,11,39,color},
        {'H',91,102,10,color},{'H',90,206,39,color},{'V',90,10,38,color}
    };
    assert(lines.size()==6);
    for(int i=0;i<6;++i) {
        const Line &a=lines[i],&b=expected[i];
        assert(a.kind==b.kind&&a.a==b.a&&a.b==b.b&&a.c==b.c&&a.color==b.color);
    }
}
int main() {
    WorldWin w;
    reset(w);
    after_tile=[](int n){if(n==1)MapWinTable[1]=&second_detail;};
    after_line=[&](int n){if(n==4){w.field_239C4_=7;w.mapWin_.dib_.bmiHeader.biWidth=200;*saved_left=1;}};
    w.draw_viewport_outline();
    assert((input==std::vector<int>{100,200,316,418}));
    check_lines(0xfb);

    // Console storage is independent of the mutable table's first pointer.
    reset(w);
    after_line=[&](int n){if(n==4){w.field_239C4_=7;w.mapWin_.dib_.bmiHeader.biWidth=200;*saved_left=1;}};
    w.draw_box();
    assert((input==std::vector<int>{10,20,18,30}));
    check_lines(0xff);

    for(int which=0;which<2;++which) {
        reset(w);left_value=20;right_value=90;
        if(which)w.draw_box();else w.draw_viewport_outline();
        assert(lines.size()==1&&lines[0].kind=='B');
        assert(lines[0].a==20&&lines[0].b==90&&lines[0].c==10);
        for(int fail=1;fail<=2;++fail){
            reset(w);tile_failure=fail;
            if(which)w.draw_box();else w.draw_viewport_outline();
            assert(lines.empty()&&tile_calls==fail);
        }
    }
    reset(w);w.viewport_outline_enabled_=0;w.draw_viewport_outline();assert(tile_calls==0);
    for(int gate=0;gate<3;++gate){
        reset(w);
        if(gate==0)ConsoleGlobal.field_1DDCC_=0;
        if(gate==1)ConsoleGlobal.field_1DDAC_=1;
        if(gate==2)ConsoleGlobal.field_1DDB0_=1;
        w.draw_box();assert(tile_calls==0&&lines.empty());
    }
}
'''


def test_actual_outline_operands_and_callback_reads(tmp_path: Path) -> None:
    _compile_run(tmp_path, _unit(SOURCE.read_text()), "outline_reads")


@pytest.mark.parametrize(("old", "new", "name"), [
    ("    detail_map = MapWinTable[1];", "", "retained_detail_pointer"),
    ("MapWin *primary_map = &ConsoleGlobal;", "MapWin *primary_map = MapWinTable[0];", "table_primary"),
    ("Buffer::hline(line_left, bottom_edge, line_bottom, 0xFB);",
     "Buffer::hline(line_right, bottom_edge, line_bottom, 0xFB);", "wrong_left_edge"),
    ("Buffer::hline(line_left, bottom_edge, line_bottom, 0xFF);",
     "Buffer::hline(line_left, edge, line_bottom, 0xFF);", "cached_second_edge"),
])
def test_outline_read_mutants_are_detected(tmp_path: Path, old: str, new: str, name: str) -> None:
    source = SOURCE.read_text()
    assert source.count(old) == 1
    with pytest.raises(subprocess.CalledProcessError):
        _compile_run(tmp_path, _unit(source.replace(old, new, 1)), name)
