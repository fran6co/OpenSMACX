"""Actual caption callback with private typed owners; not native layout proof."""

from pathlib import Path
import shutil
import subprocess

import pytest

from test_win_zorder_pointer_flow_oct10 import extract

ROOT = Path(__file__).resolve().parents[2]


def control(damage):
    friend = "friend int __cdecl def_caption_draw(GraphicWin *, char *, Buffer *);"
    for stem in ("win", "graphicwin", "buffer"):
        assert friend in (ROOT / ("src/" + stem + ".h")).read_text()
    body = extract((ROOT / "src/stringstruct.cpp").read_text(),
                   "int __cdecl def_caption_draw(GraphicWin *window, char *text, Buffer *canvas)")
    assert "reinterpret_cast" not in body
    assert body.index("window->iFlags_") < body.index("strlen(text)")
    if damage:
        assert body.count(damage[0]) == 1
        body = body.replace(*damage, 1)
    sprite = (ROOT / "src/sprite.h").read_text()
    accessors = extract(sprite, "int left_offset() const") + "\n" + extract(sprite, "int top_offset() const")
    return r'''
#include <cassert>
#include <cstring>
#include <vector>
#define __cdecl
struct RECT { int left,top,right,bottom; };
struct Font {};
class GraphicWin; class Buffer;
static std::vector<int> events;
static void (*clip_hook)();
class Sprite {
    int iLeftOffset_,iTopOffset_;
public:
    Sprite(int left,int top):iLeftOffset_(left),iTopOffset_(top){}
    ACCESSORS
};
class Win {
    FRIEND
protected:
    unsigned iFlags_=0;
    Font *field_F8_=nullptr;
    unsigned field_104_=0,field_108_=0,field_10C_=0,field_110_=0;
public:
    void configure(Font *font,unsigned flags){field_F8_=font;iFlags_=flags;field_104_=11;field_108_=22;field_10C_=33;field_110_=44;}
    void style(unsigned flags){iFlags_=flags;}
};
class GraphicWin : public Win {
    FRIEND
    Sprite *frame_left_,*frame_top_;
public:
    GraphicWin(Sprite *left=nullptr,Sprite *top=nullptr):frame_left_(left),frame_top_(top){}
};
class Buffer {
    FRIEND
    RECT rect1_={1,2,301,202};
public:
    int clips=0,written=0,length=-1;
    RECT area={},final_clip={};
    Font *font=nullptr;
    void set_text_color(int a,int b,int c,int d){assert(a==11&&b==22&&c==33&&d==44);events.push_back(1);}
    void set_font(Font *f,int a,int b,int c){assert(!a&&!b&&!c);font=f;events.push_back(2);}
    int set_clip(RECT *r){
        ++clips;events.push_back(clips==1?3:5);
        if(clips==1){assert(r!=&rect1_);area=*r;rect1_=*r;if(clip_hook)clip_hook();return 17;}
        assert(r==&rect1_);final_clip=*r;return 23;
    }
    void write(char *text,RECT *r,unsigned n,int alignment){
        events.push_back(4);written=alignment;length=n;
        assert(!strcmp(text,"caption") && n==7);
        assert(r->left==area.left && r->top==area.top && r->right==area.right && r->bottom==area.bottom);
        // Rendering can change the live clip; the final call must see it.
        rect1_={81,82,83,84};
    }
    void write_l(char *t,RECT *r,unsigned n){write(t,r,n,1);}
    void write_right_l(char *t,RECT *r,unsigned n){write(t,r,n,2);}
    void write_cent_l(char *t,RECT *r,unsigned n){write(t,r,n,3);}
};
static GraphicWin WorldClimateWorldWin;
BODY
static GraphicWin *target;
static unsigned late_style;
static void change_style(){target->style(late_style);}
int main(){
    assert(def_caption_draw(&WorldClimateWorldWin,nullptr,nullptr)==0);
    assert(def_caption_draw(nullptr,nullptr,nullptr)==0);
    assert(events.empty());
    Sprite left(6,99),top(88,9);GraphicWin window(&left,&top);Font font;
    target=&window;clip_hook=change_style;
    const unsigned styles[]={0,0x10000,0x40000,0x50000};
    const int alignments[]={3,1,2,1};
    for(int i=0;i!=4;++i){
        Buffer canvas;window.configure(&font,~styles[i]);late_style=styles[i];events.clear();
        char text[]="caption";
        assert(def_caption_draw(&window,text,&canvas)==23);
        assert(canvas.clips==2 && canvas.font==&font && canvas.written==alignments[i] && canvas.length==7);
        assert(canvas.area.left==42 && canvas.area.top==22 && canvas.area.right==261 && canvas.area.bottom==190);
        assert(canvas.final_clip.left==81 && canvas.final_clip.top==82 && canvas.final_clip.right==83 && canvas.final_clip.bottom==84);
        assert((events==std::vector<int>{1,2,3,4,5}));
    }
}
'''.replace("FRIEND", friend).replace("ACCESSORS", accessors).replace("BODY", body)


@pytest.mark.parametrize("compiler", ("g++", "clang++"))
@pytest.mark.parametrize("damage", (None,
    ("return canvas->set_clip(canvas_rect);", "return canvas->set_clip(&area);"),
    ("window->frame_top_->top_offset()", "window->frame_left_->top_offset()"),
    ("(style & 0x10000)", "(style & 0x40000)")))
def test_actual_caption_owner_state(tmp_path, compiler, damage):
    executable = shutil.which(compiler)
    if executable is None:
        pytest.skip(compiler + " unavailable")
    source = tmp_path / "caption.cpp"
    source.write_text(control(damage))
    binary = tmp_path / "caption"
    result = subprocess.run([executable, "-std=c++11", str(source), "-o", str(binary)], capture_output=True, text=True)
    assert result.returncode == 0, result.stderr
    result = subprocess.run([str(binary)], capture_output=True, timeout=10)
    assert (result.returncode == 0) == (damage is None)
