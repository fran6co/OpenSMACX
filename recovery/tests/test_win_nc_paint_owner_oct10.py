"""Actual Win non-client paint receiver/state controls, not GUI acceptance.

Pinned 5F5810..5F5ACA selects the parent visibility receiver, ScreenBuffer
clip state and four member-buffer draw receivers. Host layouts are not ABI proof.
"""
from pathlib import Path

import pytest

from test_texture_sub61f0d0_actual_source import build_and_run, extract_method

ROOT = next(p for p in Path(__file__).resolve().parents if (p / "src/win.cpp").is_file())
SOURCE = ROOT / "src/win.cpp"
SIGNATURE = "void Win::on_nc_paint(RECT * area, int flags)"

HOST = r"""
#include <cassert>
#include <vector>
struct RECT { int left, top, right, bottom; };
struct Buffer;
struct Draw { Buffer *source, *destination; int x,y; };
static std::vector<Draw> draws;
static std::vector<int> events;
static Buffer *clip_owner, *restore_owner;
static RECT *clip_area;
static int WinFillColour=17, WinViewOriginX=3, WinViewOriginY=5;
static int visibility_calls, queried_identity;
struct Buffer {
    void set_clip(RECT *area) {
        clip_owner=this; clip_area=area; events.push_back(2);
    }
    void restore_clip() { restore_owner=this; events.push_back(3); }
    void draw(Buffer *destination, int color, int x, int y, int a, int b) {
        assert(color==17 && a==1 && b==1);
        Draw d={this,destination,x,y}; draws.push_back(d); events.push_back(4);
    }
};
struct Owner { Buffer object; Buffer &buffer() { return object; } };
static Owner ScreenBufferOwner;
static Buffer *WinBackBuffer;
class Win {
public:
    int iFlags_,iSomeFlag_,border_thickness_,caption_height_,bottom_border_thickness_;
    int identity, visible;
    Win *win_parent_;
    Buffer *buffer1_,*buffer2_,*buffer3_,*buffer4_;
    RECT outer_rect_,client_rect_;
    int is_visible() { ++visibility_calls; queried_identity=identity; return visible; }
    void client_to_screen(int *x,int *y) {
        assert(*x==0 && *y==0); *x=100; *y=200; events.push_back(1);
    }
    void on_nc_paint(RECT *,int);
};
// BODY_INSERTION
static Buffer one,two,three,four,back;
static Win window,parent;
static void reset() {
    draws.clear(); events.clear(); clip_owner=0; restore_owner=0; clip_area=0;
    visibility_calls=0; queried_identity=0; WinBackBuffer=0;
    WinViewOriginX=3; WinViewOriginY=5;
    window.iFlags_=1; window.iSomeFlag_=1;
    window.border_thickness_=7; window.caption_height_=11;
    window.bottom_border_thickness_=13;
    window.identity=1; window.visible=0; window.win_parent_=0;
    parent.identity=2; parent.visible=1;
    window.buffer1_=&one; window.buffer2_=&two;
    window.buffer3_=&three; window.buffer4_=&four;
    RECT outer={10,20,50,60},client={4,6,104,206};
    window.outer_rect_=outer; window.client_rect_=client;
}
static void check(int i,Buffer *source,Buffer *destination,int x,int y) {
    assert(draws[i].source==source && draws[i].destination==destination);
    assert(draws[i].x==x && draws[i].y==y);
}
int main() {
    reset(); window.win_parent_=&parent;
    window.on_nc_paint(0,-1);
    assert(visibility_calls==1 && queried_identity==2 && draws.size()==4);
    check(0,&one,&ScreenBufferOwner.object,90,180);
    check(1,&four,&ScreenBufferOwner.object,90,367);
    check(2,&three,&ScreenBufferOwner.object,90,187);
    check(3,&two,&ScreenBufferOwner.object,183,187);
    assert(clip_owner==0 && restore_owner==&ScreenBufferOwner.object);
    assert(events.front()==1 && events.back()==3);

    reset(); WinBackBuffer=&back; window.iFlags_=0x10;
    RECT area={1,2,3,4},saved=area;
    window.on_nc_paint(&area,99); // area forces every side, irrespective of selector
    assert(draws.size()==4 && clip_owner==&ScreenBufferOwner.object && clip_area==&area);
    assert(area.left==saved.left && area.top==saved.top && area.right==saved.right && area.bottom==saved.bottom);
    assert(events[0]==1 && events[1]==2 && events.back()==3);
    check(0,&one,&back,87,175); check(1,&four,&back,87,362);
    check(2,&three,&back,87,186); check(3,&two,&back,180,186);

    reset(); window.iFlags_=0x400010; window.bottom_border_thickness_=-1;
    window.on_nc_paint(0,-1);
    check(1,&four,&ScreenBufferOwner.object,90,373);
    check(2,&three,&ScreenBufferOwner.object,90,187);

    Buffer *sources[4]={&one,&two,&four,&three};
    for(int selector=0;selector!=4;++selector) {
        reset(); window.on_nc_paint(0,selector);
        assert(draws.size()==1 && draws[0].source==sources[selector]);
        assert(restore_owner==&ScreenBufferOwner.object);
    }
    reset(); window.on_nc_paint(0,99);
    assert(draws.empty() && restore_owner==&ScreenBufferOwner.object);
    reset(); window.buffer2_=0; window.buffer4_=0; window.on_nc_paint(0,-1);
    assert(draws.size()==2); check(0,&one,&ScreenBufferOwner.object,90,180);
    check(1,&three,&ScreenBufferOwner.object,90,187);

    reset(); window.win_parent_=&parent; parent.visible=0; window.visible=1;
    window.on_nc_paint(&area,-1);
    assert(visibility_calls==1 && queried_identity==2 && events.empty() && draws.empty());
    reset(); window.iFlags_=0; window.win_parent_=&parent; window.on_nc_paint(&area,-1);
    assert(visibility_calls==0 && events.empty());
    reset(); window.iSomeFlag_=0; window.win_parent_=&parent; window.on_nc_paint(&area,-1);
    assert(visibility_calls==0 && events.empty());
}
"""


def unit(body: str | None = None) -> str:
    return HOST.replace("// BODY_INSERTION", body or extract_method(SOURCE.read_text(), SIGNATURE))


@pytest.mark.parametrize("compiler", ("g++", "clang++"))
def test_actual_non_client_paint(compiler: str) -> None:
    build_and_run(unit(), compiler, expect_success=True)


@pytest.mark.parametrize("compiler", ("g++", "clang++"))
@pytest.mark.parametrize("old,new", (
    ("win_parent_->is_visible()", "this->is_visible()"),
    ("ScreenBufferOwner.buffer().set_clip(area)", "reinterpret_cast<Buffer *>(area)->set_clip(area)"),
    ("buffer1_->draw", "ScreenBufferOwner.buffer().draw"),
    ("buffer4_->draw", "ScreenBufferOwner.buffer().draw"),
    ("buffer3_->draw", "ScreenBufferOwner.buffer().draw"),
    ("buffer2_->draw", "ScreenBufferOwner.buffer().draw"),
    ("flags = -1;", "flags = 0;"),
    ("ScreenBufferOwner.buffer().restore_clip();", "(void)0;"),
))
def test_rejects_wrong_ownership_or_state(compiler: str, old: str, new: str) -> None:
    body=extract_method(SOURCE.read_text(), SIGNATURE)
    assert old in body
    build_and_run(unit(body.replace(old,new)), compiler, expect_success=False)
