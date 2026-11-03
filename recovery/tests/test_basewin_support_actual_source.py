"""Actual support-map body: pinned zoom arithmetic and Buffer/callback phases."""
from pathlib import Path
import shutil
import subprocess

import pytest


ROOT = Path(__file__).resolve().parents[2]


def actual_body():
    source = (ROOT / "src/basewin.cpp").read_text()
    start = source.index("void BaseWin::draw_support() {")
    opening = source.index("{", start)
    depth = 0
    for end in range(opening, len(source)):
        depth += (source[end] == "{") - (source[end] == "}")
        if depth == 0:
            return source[start:end + 1]
    raise AssertionError("unterminated support body")


PRELUDE = r'''
#include <cstdlib>
#include <cstdio>
#include <vector>
struct RECT { int left, top, right, bottom; };
bool equal(RECT a, RECT b) {
    return a.left==b.left && a.top==b.top && a.right==b.right && a.bottom==b.bottom;
}
std::vector<int> events;
struct Buffer {
    RECT rect1_;
    int set_clip(RECT *r) { events.push_back(1); rect1_=*r; return 0; }
};
int MapLongitudeBounds, MapLatitudeBounds, MapWinCaptionScale, MapWinCaptionDivisor;
struct Base { short x, y; } base, replacement, *BaseCurrent;
struct MapWin {
    int field_1DD6C_, field_1DD70_, zoom_offset_, center_x_, center_y_;
    int width, height, copy_args[6];
    Buffer *destination;
    void init(int a, int b) {
        if(a!=2 || b!=0) std::abort();
        events.push_back(3);
        BaseCurrent=&replacement;
    }
    int pixel_width() { return width; }
    int pixel_height() { return height; }
    void redraw() {
        events.push_back(4);
        BaseCurrent=&base;
    }
    void copy(Buffer *to, int sx, int sy, int x, int y, int w, int h) {
        if(sx || sy) std::abort();
        destination=to;
        copy_args[0]=x; copy_args[1]=y; copy_args[2]=w; copy_args[3]=h;
        events.push_back(5);
    }
};
struct BaseWin : Buffer {
    RECT rect1_;
    MapWin mapWin_;
    void draw_support();
    int fill(RECT *area, int color) {
        if(area!=&rect1_ || color!=0) std::abort();
        events.push_back(2);
        return 0;
    }
    void soft_update(RECT *area) {
        if(area!=&rect1_) std::abort();
        events.push_back(6);
    }
};
#define require(x) do { if(!(x)) { std::fprintf(stderr,"failed:%d\n",__LINE__); std::exit(1); } } while(0)
'''


SUFFIX = r'''
void run_case(int scale, int divisor, int longitude, int latitude,
              int width, int height, int expected_zoom) {
    BaseWin window = {};
    RECT saved = {2,3,40,50};
    window.Buffer::rect1_=saved;
    window.rect1_=RECT{10,20,610,420};
    window.mapWin_.width=width;
    window.mapWin_.height=height;
    window.mapWin_.field_1DD6C_=10;
    base.x=1; base.y=2; replacement.x=21; replacement.y=23;
    BaseCurrent=&base;
    MapWinCaptionScale=scale; MapWinCaptionDivisor=divisor;
    MapLongitudeBounds=longitude; MapLatitudeBounds=latitude;
    events.clear();
    window.draw_support();
    require(window.mapWin_.zoom_offset_==expected_zoom);
    require(window.mapWin_.field_1DD6C_==5);
    require(window.mapWin_.field_1DD70_==0x200402);
    require(window.mapWin_.center_x_==21 && window.mapWin_.center_y_==23);
    require(window.mapWin_.destination==static_cast<Buffer*>(&window));
    require(window.mapWin_.copy_args[0]==310-width/2);
    require(window.mapWin_.copy_args[1]==220-height/2);
    require(window.mapWin_.copy_args[2]==width && window.mapWin_.copy_args[3]==height);
    require(equal(window.Buffer::rect1_,saved));
    const int expected[]={1,2,3,4,5,6,1};
    require(events==std::vector<int>(expected,expected+7));
}
int main() {
    // Pin408F82..91: scale*50/divisor; 408FAB..B8: scale*25/divisor.
    // The second dimension fits here only with the original factor25.
    run_case(8,16,40,40,500,300,2);
    // Positive scale does not shrink with width_step: all17 failures remain.
    run_case(8,16,40,40,400,300,-15);
    // Nonpositive scale takes the step/16 arms, even with a zero divisor.
    run_case(0,0,20,20,500,300,0);
    run_case(-8,0,20,20,500,300,0);
    // Odd dimensions use signed division toward zero before multiplication.
    run_case(8,16,41,41,500,240,2);
    run_case(8,16,-3,-3,0,0,2);
    // Vertical failure and minimum-step termination keep the original -15.
    run_case(8,16,40,40,500,239,-15);
    return 0;
}
'''


@pytest.mark.parametrize("compiler", ["g++", "clang++"])
def test_actual_support_zoom_and_callback_phases(tmp_path, compiler):
    path = shutil.which(compiler)
    if path is None:
        pytest.skip(f"{compiler} unavailable")
    body = actual_body()
    for tag, source, expected in (
        ("actual", body, 0),
        ("wrong_vertical_scale", body.replace("caption_scale * 25", "caption_scale * 50"), 1),
        ("skip_zoom_descent", body.replace("--zoom;", "++zoom;"), 1),
        ("capture_base_too_early", body.replace(
            "    mapWin_.init(2, 0);",
            "    Base *early_base = BaseCurrent;\n    mapWin_.init(2, 0);").replace(
                "BaseCurrent->x", "early_base->x").replace("BaseCurrent->y", "early_base->y"), 1),
    ):
        if tag != "actual":
            assert source != body
        cpp = tmp_path / (tag + ".cpp")
        exe = tmp_path / tag
        cpp.write_text(PRELUDE + source + SUFFIX)
        result = subprocess.run([path, "-std=c++11", "-O0", "-Wall", "-Wextra", "-Werror",
                                 str(cpp), "-o", str(exe)], capture_output=True, text=True)
        assert result.returncode == 0, result.stderr
        result = subprocess.run([str(exe)], capture_output=True, text=True)
        assert result.returncode == expected, (tag, result.stdout, result.stderr)
