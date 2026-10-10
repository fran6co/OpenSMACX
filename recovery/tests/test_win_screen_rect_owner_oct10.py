"""Actual Win bodies with controlled SDK, buffers and virtual callbacks.

Pinned 5F7320..749F establishes aliased IntersectRect operands, the six
copy arguments and propagation of the copy result. These are bounded
call/state controls, not a working GUI or byte/layout acceptance.
"""
import pytest

from test_texture_sub61f0d0_actual_source import build_and_run, extract_method

from pathlib import Path

ROOT = next(p for p in Path(__file__).resolve().parents if (p / "src/win.cpp").is_file())
SOURCE = ROOT / "src/win.cpp"
UPDATE = "int __cdecl Win::update_screen(RECT *area, Win *window)"
POSITION = "long Win::on_window_pos_changing(WINDOWPOS * pos)"

HOST = r"""
#include <cassert>
#include <stdint.h>
#include <vector>
#define __cdecl
typedef intptr_t LPARAM;
typedef void *HWND;
struct RECT { int left, top, right, bottom; };
struct WINDOWPOS {
    HWND hwnd, hwndInsertAfter;
    int x, y, cx, cy;
    unsigned int flags;
};
struct GraphicWin {};
class Win;
static Win *WinZOrderArray[3], *WinZOrderWindow;
static int WinZOrderCount, WinBackBuffer, WinFlipSprite;
static int WinFlipSpriteY, WinFlipSpriteX, WinClipWidth, WinClipHeight;
static HWND HandleMain;
static std::vector<int> events;
static Win *replace_slot, *replace_after_nc;
static int visible_result, intersection_calls, copy_calls, fill_calls;
static int copy_args[6], post_calls, default_calls, graphic_calls;
static LPARAM default_pos;
static bool mutate_origins;
struct Buffer;
static Buffer *copy_receiver, *copy_destination;
struct Buffer {
    RECT *filled;
    int copy(Buffer *destination, int a, int b, int c, int d, int e, int f) {
        ++copy_calls;
        copy_receiver=this; copy_destination=destination;
        copy_args[0]=a; copy_args[1]=b; copy_args[2]=c;
        copy_args[3]=d; copy_args[4]=e; copy_args[5]=f;
        events.push_back(90);
        return 17;
    }
    void fill(RECT *area, int colour) { assert(colour==0); ++fill_calls; filled=area; }
    void fill(int colour) { assert(colour==0); ++fill_calls; filled=0; }
};
struct BufferOwner {
    Buffer object;
    Buffer &buffer() { return object; }
};
static BufferOwner ScreenBufferOwner, WinDrawingBufferOwner;
static int IntersectRect(RECT *out, const RECT *first, const RECT *second) {
    ++intersection_calls;
    assert(out==first); // original two LEAs select one complete RECT
    const RECT a=*first, b=*second;
    RECT r;
    r.left=a.left>b.left?a.left:b.left;
    r.top=a.top>b.top?a.top:b.top;
    r.right=a.right<b.right?a.right:b.right;
    r.bottom=a.bottom<b.bottom?a.bottom:b.bottom;
    if (mutate_origins) { WinFlipSpriteY=9; WinFlipSpriteX=19; }
    if (r.left>=r.right || r.top>=r.bottom) {
        RECT zero={0,0,0,0}; *out=zero; return 0;
    }
    *out=r; return 1;
}
static int PostMessageA(HWND, int msg, int wparam, int lparam) {
    assert(msg==3 && wparam==0 && lparam==0); ++post_calls; return 1;
}
static long DefWindowProcA(HWND, int msg, int wparam, LPARAM pos) {
    assert(msg==0x46 && wparam==0);
    ++default_calls; default_pos=pos; return 37;
}
class Win {
public:
    int iSomeFlag_, iFlags_, field_134_, field_138_, iVertScaleNum_, iVertScaleDenom_;
    int identity;
    Win *win_parent_, *poWinBase_;
    GraphicWin *graphic;
    static int update_screen(RECT *, Win *);
    long on_window_pos_changing(WINDOWPOS *);
    virtual int is_visible() {
        events.push_back(identity);
        if (replace_slot) WinZOrderArray[0]=replace_slot;
        return visible_result;
    }
    virtual void on_nc_paint(RECT *, int mode) {
        assert(mode==-1); events.push_back(identity+10);
        if (replace_after_nc) WinZOrderArray[0]=replace_after_nc;
    }
    virtual void on_paint(RECT *) { events.push_back(identity+20); }
    GraphicWin *get_graphic_win() { ++graphic_calls; return graphic; }
    void nonclient_to_client(int *x, int *y) { *x-=4; *y-=6; }
    void client_to_nonclient(int *x, int *y) { *x+=4; *y+=6; }
};
// UPDATE_INSERTION
// POSITION_INSERTION
static Win first, second, parent, other;
static GraphicWin graphic;
static void reset() {
    events.clear(); intersection_calls=copy_calls=fill_calls=0;
    post_calls=default_calls=graphic_calls=0; default_pos=0;
    replace_slot=replace_after_nc=0; visible_result=1; mutate_origins=false;
    WinZOrderCount=1; WinZOrderWindow=0; WinBackBuffer=1; WinFlipSprite=1;
    WinFlipSpriteY=10; WinFlipSpriteX=20; WinClipWidth=30; WinClipHeight=40;
    first.iSomeFlag_=second.iSomeFlag_=parent.iSomeFlag_=other.iSomeFlag_=0;
    first.win_parent_=second.win_parent_=parent.win_parent_=other.win_parent_=0;
    first.poWinBase_=&first; second.poWinBase_=&second;
    parent.poWinBase_=&parent; other.poWinBase_=&other;
    first.identity=1; second.identity=2; parent.identity=3; other.identity=4;
    first.iFlags_=0; first.field_134_=10; first.field_138_=12;
    first.iVertScaleNum_=3; first.iVertScaleDenom_=2; first.graphic=&graphic;
    WinZOrderArray[0]=&first;
}
static void check_copy(int a,int b,int c,int d,int e,int f) {
    const int wanted[6]={a,b,c,d,e,f};
    assert(copy_calls==1);
    assert(copy_receiver==&ScreenBufferOwner.object);
    assert(copy_destination==&WinDrawingBufferOwner.object);
    for (int i=0;i<6;++i) assert(copy_args[i]==wanted[i]);
}
int main() {
    reset();
    assert(Win::update_screen(0,0)==17);
    check_copy(10,20,0,0,30,40);

    reset();
    RECT area={15,27,35,55};
    assert(Win::update_screen(&area,0)==17);
    assert(intersection_calls==1);
    check_copy(15,27,5,7,20,28);

    reset(); mutate_origins=true;
    assert(Win::update_screen(&area,0)==17);
    check_copy(15,27,6,8,20,28);

    reset();
    RECT outside={100,100,110,110};
    assert(Win::update_screen(&outside,0)==0);
    assert(intersection_calls==1 && copy_calls==0);

    reset(); WinZOrderCount=0;
    assert(Win::update_screen(&area,0)==0);
    assert(events.empty() && intersection_calls==0 && copy_calls==0);

    reset(); WinZOrderWindow=&other;
    assert(Win::update_screen(&area,&other)==1);
    assert(events.empty() && intersection_calls==0 && copy_calls==0);

    reset(); WinFlipSprite=0; WinBackBuffer=0;
    Win::update_screen(&area,0);
    assert(fill_calls==1 && ScreenBufferOwner.object.filled==&area && copy_calls==0);
    Win::update_screen(0,0);
    assert(fill_calls==2 && ScreenBufferOwner.object.filled==0);

    reset(); WinFlipSprite=0; first.iSomeFlag_=1;
    first.win_parent_=&parent; second.iSomeFlag_=3; replace_slot=&second;
    replace_after_nc=&other;
    Win::update_screen(0,0);
    assert(events.size()==3 && events[0]==3 && events[1]==12 && events[2]==22);

    reset(); WinFlipSprite=0; first.iSomeFlag_=1; WinZOrderWindow=&first;
    first.win_parent_=&parent; second.iSomeFlag_=3; replace_slot=&second;
    Win::update_screen(0,&first);
    // The target remains its own base while the painted slot changes owner.
    assert(events.size()==3 && events[0]==3 && events[1]==12 && events[2]==22);

    reset(); WinFlipSprite=0; first.iSomeFlag_=3; WinZOrderWindow=&first;
    first.poWinBase_=&other;
    Win::update_screen(0,&first);
    assert(events.size()==1 && events[0]==11); // target is not its own base

    reset(); WinFlipSprite=0; first.iSomeFlag_=1; first.win_parent_=&parent;
    visible_result=0;
    Win::update_screen(0,0);
    assert(events.size()==1 && events[0]==3);

    reset(); WinFlipSprite=0; WinZOrderCount=2;
    WinZOrderArray[1]=&second; first.iSomeFlag_=second.iSomeFlag_=1;
    Win::update_screen(0,0);
    assert(events.size()==2 && events[0]==22 && events[1]==21);

    reset();
    WINDOWPOS pos={0,0,51,52,3,4,0x80000000u};
    assert(first.on_window_pos_changing(&pos)==37);
    assert(pos.cx==10 && pos.cy==12 && pos.x==51 && pos.y==52);
    assert(post_calls==1 && default_calls==1 && default_pos==reinterpret_cast<LPARAM>(&pos));
    assert(pos.flags==0x80000000u);

    reset(); first.iFlags_=0x804000;
    WINDOWPOS scaled={0,reinterpret_cast<HWND>(1),51,52,20,50,0x80000000u};
    first.on_window_pos_changing(&scaled);
    assert(scaled.cx==20 && scaled.cy==30 && scaled.flags==0x80000004u);
    assert(post_calls==0 && graphic_calls==1);

    reset(); first.iFlags_=0x804000;
    WINDOWPOS no_size={0,0,51,52,3,4,1};
    first.on_window_pos_changing(&no_size);
    assert(no_size.cx==3 && no_size.cy==4 && no_size.flags==5 && graphic_calls==0);
}
"""


def unit(update: str | None = None, position: str | None = None) -> str:
    source = SOURCE.read_text()
    return (HOST.replace("// UPDATE_INSERTION", update or extract_method(source, UPDATE))
            .replace("// POSITION_INSERTION", position or extract_method(source, POSITION)))


@pytest.mark.parametrize("compiler", ("g++", "clang++"))
def test_actual_screen_update_and_window_position(compiler: str) -> None:
    build_and_run(unit(), compiler, expect_success=True)


@pytest.mark.parametrize("compiler", ("g++", "clang++"))
@pytest.mark.parametrize("old,new", (
    ("w = WinZOrderArray[i];", "(void)0;"),
    ("target == target->poWinBase_", "target == w->poWinBase_"),
    ("IntersectRect(&rectangle, &rectangle, area)", "IntersectRect(&rectangle, area, &rectangle)"),
    ("origin_y = WinFlipSpriteX;", "origin_y = WinFlipSpriteY;"),
    ("rectangle.left, rectangle.top,", "rectangle.top, rectangle.left,"),
    ("return ScreenBufferOwner.buffer().copy(", "return 0 * ScreenBufferOwner.buffer().copy("),
    ("return ScreenBufferOwner.buffer().copy(", "return WinDrawingBufferOwner.buffer().copy("),
    ("&WinDrawingBufferOwner.buffer(), rectangle.left", "&ScreenBufferOwner.buffer(), rectangle.left"),
))
def test_screen_update_rejects_reconstruction_mutants(compiler: str, old: str, new: str) -> None:
    body = extract_method(SOURCE.read_text(), UPDATE)
    assert old in body
    # The table reload anchor excludes the initial declaration.
    if old == "w = WinZOrderArray[i];":
        old="                    "+old
        new="                    "+new
        assert body.count(old)==1
    build_and_run(unit(update=body.replace(old,new,1)),compiler,expect_success=False)


@pytest.mark.parametrize("compiler", ("g++", "clang++"))
@pytest.mark.parametrize("old,new", (
    ("pos->cx = cx;", "pos->cy = cx;"),
    ("pos->cy = ncy;", "pos->cx = ncy;"),
    ("pos->flags |= 4;", "pos->flags = 4;"),
    ("pos->hwndInsertAfter == 0", "pos->hwnd == 0"),
))
def test_position_rejects_wrong_fields(compiler: str, old: str, new: str) -> None:
    body=extract_method(SOURCE.read_text(), POSITION)
    assert old in body
    build_and_run(unit(position=body.replace(old,new,1)),compiler,expect_success=False)
