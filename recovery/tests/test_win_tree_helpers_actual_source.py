"""Host behavior controls for Win's recursive tree helpers.

The simplified host Win layout tests behavior only: it does not prove native
32-bit member offsets or the production +0x444 GraphicWin/Buffer adjustment.
Those remain grounded in pinned declaration/layout evidence and separate
native checks, not this host fixture.
"""
from pathlib import Path
import re
import shutil
import subprocess

import pytest
from match_recovery.includes import COMMENTS_OR_STRINGS


ROOT = next(parent for parent in Path(__file__).resolve().parents
            if (parent / "src/win.cpp").is_file())
SOURCE = ROOT / "src/win.cpp"
HEADER = ROOT / "src/win.h"


def extract(source: str, signature: str) -> str:
    masked = COMMENTS_OR_STRINGS.sub(
        lambda m: re.sub(r"[^\n]", " ", m[0]), source)
    matches = list(re.finditer(re.escape(signature), masked))
    assert len(matches) == 1, (signature, len(matches))
    opening = masked.find("{", matches[0].end())
    depth = 0
    for end in range(opening, len(masked)):
        if masked[end] == "{":
            depth += 1
        elif masked[end] == "}":
            depth -= 1
            if depth == 0:
                return source[matches[0].start():end + 1]
    raise AssertionError(f"unterminated {signature}")


FIXTURE = r'''
#include <cassert>
#include <stdint.h>
#define __cdecl
static const unsigned int WinFlagVisible=1, WinFlagClipToParent=0x20, WinFlagParentOffset=0x8000;
struct RECT { int left, top, right, bottom; };
struct Win;
struct GraphicWin;
struct Buffer {
    int pixel;
    int get_pixel(int x, int y);
};
struct Win {
    unsigned int iFlags_, iSomeFlag_;
    unsigned char *color_table_;
    Win *poWinBase_, *win_parent_;
    Buffer *buffer1_, *buffer2_, *buffer3_, *buffer4_;
    int caption_height_, border_thickness_, bottom_border_thickness_;
    RECT outer_rect_, client_rect_;
    Win *children_[8];
    int child_count_, visible;
    GraphicWin *graphic_;
    Win() : iFlags_(0), iSomeFlag_(0), color_table_(0), poWinBase_(0),
            win_parent_(0), buffer1_(0), buffer2_(0), buffer3_(0), buffer4_(0),
            caption_height_(0), border_thickness_(0), bottom_border_thickness_(-1),
            outer_rect_(), client_rect_(), child_count_(0), visible(1), graphic_(0) {
        for (int i=0; i<8; ++i) children_[i]=0;
    }
    virtual GraphicWin *get_graphic_win();
    int is_visible() { return visible; }
    void screen_to_client(int *, int *) {}
    Win *get_mouse_window(int *, int *);
};
struct GraphicWin : Win, Buffer {
    GraphicWin *get_graphic_win() { return this; }
};
GraphicWin *Win::get_graphic_win() { return graphic_; }
Buffer *last_pixel_buffer;
int last_pixel_x, last_pixel_y;
int Buffer::get_pixel(int x, int y) {
    last_pixel_buffer=this; last_pixel_x=x; last_pixel_y=y; return pixel;
}
int in_box(int x, int y, const RECT *r) {
    return x >= r->left && x < r->right && y >= r->top && y < r->bottom;
}
Win *WinZOrderWindow;
Win *WinZOrderArray[512];
int WinZOrderCount;
int WinZOrderFlag;
int WinMouseScreenX, WinMouseScreenY, WinMouseDirect;
unsigned char WinFillColour=9;
Win *WinPointerOwner1, *WinPointerOwner2, *WinPointerOwner3, *WinPointerOwner4;
Win *WinFocusWindow, *WinModalWindow, *WinRootWindows[512];
int WinRootCount;
// ACTUAL_TREE_HELPERS
void reset_globals() {
    WinZOrderWindow=0; WinZOrderCount=0; WinZOrderFlag=0;
    WinMouseScreenX=WinMouseScreenY=WinMouseDirect=0;
    last_pixel_x=last_pixel_y=0;
    WinPointerOwner1=WinPointerOwner2=WinPointerOwner3=WinPointerOwner4=0;
    WinFocusWindow=WinModalWindow=0; WinRootCount=0; last_pixel_buffer=0;
    for (int i=0;i<512;++i) { WinZOrderArray[i]=0; WinRootWindows[i]=0; }
}
int main() {
    reset_globals();
    Win root, first, second;
    root.iSomeFlag_=1; root.poWinBase_=&root; root.child_count_=2;
    root.children_[0]=&first; root.children_[1]=&second;
    first.iSomeFlag_=1; first.win_parent_=0;
    second.iSomeFlag_=1; second.iFlags_=0x20; second.win_parent_=0;
    // The first walk appends ordinary children; the second visits the
    // complementary 0x20 class only for the base window, then the root.
    recurse_zorder(&root);
    assert(WinZOrderCount==3);
    assert(WinZOrderArray[0]==&first && WinZOrderArray[1]==&second && WinZOrderArray[2]==&root);

    reset_globals();
    root.iSomeFlag_=1; root.poWinBase_=&root; root.child_count_=2;
    root.children_[0]=&first; root.children_[1]=&second;
    first.iSomeFlag_=1; first.iFlags_=0; second.iSomeFlag_=1; second.iFlags_=0x20;
    WinZOrderWindow=&first;
    recurse_zorder(&root);
    assert(WinZOrderCount==2 && WinZOrderArray[0]==&first && WinZOrderArray[1]==&root);

    reset_globals();
    Win child;
    root.iSomeFlag_=3; root.poWinBase_=&root; root.client_rect_={10,20,70,80};
    root.outer_rect_={0,0,100,100}; root.child_count_=1; root.children_[0]=&child;
    child.iSomeFlag_=1; child.iFlags_=0x8020; child.client_rect_={0,0,80,80};
    child.outer_rect_={0,0,100,100}; child.win_parent_=&root;
    WinMouseScreenX=40; WinMouseScreenY=50;
    int x=40, y=50;
    assert(get_mouse_window_recurse(&root,&x,&y)==&child);
    assert(x==30 && y==30); // root client origin is removed before child recursion
    root.child_count_=0; x=40; y=50;
    assert(get_mouse_window_recurse(&root,&x,&y)==&root);
    assert(x==40 && y==50); // successful client path applies and restores its origin
    x=5; y=6; WinMouseScreenX=80; WinMouseScreenY=80;
    root.outer_rect_={20,20,40,40};
    assert(get_mouse_window_recurse(&root,&x,&y)==0);
    assert(x==5 && y==6); // failed outer hit restores its entry coordinates

    reset_globals();
    Win focus, modal;
    focus.iSomeFlag_=1; focus.poWinBase_=&focus; focus.outer_rect_={20,20,40,40};
    modal.iSomeFlag_=1; modal.poWinBase_=&modal; modal.outer_rect_={0,0,100,100};
    WinFocusWindow=&focus; WinModalWindow=&modal;
    x=5; y=6;
    assert(focus.get_mouse_window(&x,&y)==&modal);
    assert(x==5 && y==6 && WinMouseDirect==0);

    reset_globals();
    GraphicWin graphic;
    graphic.iSomeFlag_=1; graphic.iFlags_=0x01000000;
    graphic.poWinBase_=&graphic; graphic.outer_rect_={0,0,100,100};
    graphic.pixel=4;
    x=7; y=8;
    assert(get_mouse_window_recurse(&graphic,&x,&y)==&graphic);
    assert(last_pixel_buffer==static_cast<Buffer *>(&graphic));

    reset_globals();
    GraphicWin quadrants;
    Buffer top, right, left, bottom;
    quadrants.iSomeFlag_=3; quadrants.iFlags_=0x10000020;
    quadrants.poWinBase_=&quadrants;
    quadrants.client_rect_={0,0,100,100}; quadrants.outer_rect_={0,0,5,5};
    quadrants.border_thickness_=10; quadrants.caption_height_=15;
    quadrants.bottom_border_thickness_=7;
    quadrants.buffer1_=&top; quadrants.buffer2_=&right;
    quadrants.buffer3_=&left; quadrants.buffer4_=&bottom;
    top.pixel=8; right.pixel=9; left.pixel=8; bottom.pixel=9;
    WinMouseDirect=55; x=50; y=5;
    assert(get_mouse_window_recurse(&quadrants,&x,&y)==&quadrants);
    assert(last_pixel_buffer==&top && last_pixel_x==50 && last_pixel_y==5);
    assert(WinMouseDirect==1); // opaque top strip claims the point

    WinMouseDirect=56; x=50; y=95;
    assert(get_mouse_window_recurse(&quadrants,&x,&y)==0);
    assert(last_pixel_buffer==&bottom && last_pixel_x==50 && last_pixel_y==2);
    assert(WinMouseDirect==56); // color key leaves the direct-state word untouched

    WinMouseDirect=57; x=5; y=30;
    assert(get_mouse_window_recurse(&quadrants,&x,&y)==&quadrants);
    assert(last_pixel_buffer==&left && last_pixel_x==5 && last_pixel_y==20);
    assert(WinMouseDirect==1); // opaque left strip claims the point

    WinMouseDirect=58; x=50; y=30;
    assert(get_mouse_window_recurse(&quadrants,&x,&y)==0);
    assert(last_pixel_buffer==&right && last_pixel_x==160 && last_pixel_y==20);
    assert(WinMouseDirect==58); // color key on the right strip remains transparent
}
'''


def translation(mutant=None):
    text = SOURCE.read_text(encoding="utf-8")
    zorder = extract(text, "void __cdecl recurse_zorder(Win * window)")
    mouse = extract(text, "Win *__cdecl get_mouse_window_recurse(Win * window, int * x, int * y)")
    wrapper = extract(text, "Win *Win::get_mouse_window(int *x, int *y)")
    if mutant == "skip_second_pass":
        changed = zorder.replace("window->poWinBase_ == window", "window->poWinBase_ != window", 1)
        assert changed != zorder
        zorder = changed
    if mutant == "wrong_coordinate_restore":
        needle = "*x += window->client_rect_.left;"
        before, separator, after = mouse.rpartition(needle)
        assert separator
        mouse = before + "*x -= window->client_rect_.left;" + after
    return FIXTURE.replace("// ACTUAL_TREE_HELPERS", "\n".join((zorder, mouse, wrapper)))


def run_fixture(tmp_path, source, compiler="g++"):
    tool = shutil.which(compiler)
    if not tool:
        pytest.skip(f"{compiler} unavailable")
    cpp, binary = tmp_path / "tree.cpp", tmp_path / "tree"
    cpp.write_text(source, encoding="utf-8")
    built = subprocess.run([tool, "-std=c++11", "-Wall", "-Wextra",
                            "-Wno-unused-parameter", str(cpp), "-o", str(binary)],
                          capture_output=True, text=True, timeout=45)
    assert built.returncode == 0, built.stdout + built.stderr
    return subprocess.run([str(binary)], capture_output=True, text=True, timeout=30)


def test_helpers_use_named_win_storage_and_real_buffer_base():
    text = SOURCE.read_text(encoding="utf-8")
    zorder = extract(text, "void __cdecl recurse_zorder(Win * window)")
    mouse = extract(text, "Win *__cdecl get_mouse_window_recurse(Win * window, int * x, int * y)")
    header = HEADER.read_text(encoding="utf-8")
    assert "friend Win *__cdecl get_mouse_window_recurse(Win *window, int *x, int *y);" in header
    assert "reinterpret_cast<char *>(window)" not in zorder + mouse
    assert "static_cast<Buffer *>(slotResult)" in mouse
    for offset in ("0xc4", "0x3fc", "0x1a4", "0xa8", "0x444"):
        assert offset not in zorder + mouse


@pytest.mark.parametrize("compiler", ("g++", "clang++"))
def test_actual_helpers_preserve_tree_mouse_modal_and_coordinate_paths(compiler, tmp_path):
    run = run_fixture(tmp_path, translation(), compiler)
    assert run.returncode == 0, run.stdout + run.stderr


@pytest.mark.parametrize("mutant", ("skip_second_pass", "wrong_coordinate_restore"))
def test_actual_helpers_host_control_detects_structural_mutants(mutant, tmp_path):
    run = run_fixture(tmp_path, translation(mutant))
    assert run.returncode != 0
