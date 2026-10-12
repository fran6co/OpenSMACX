"""Complete Win/GraphicWin painter controls, not native ABI or GUI acceptance.

Both complete product methods execute on header-extracted fields, inheritance,
virtual declarations and dimension accessors. Dependency probes record geometry
and explicitly change members at call boundaries. Original decoding independently
pins each quadrant's arithmetic, signed DIB reads and callback reloads. Native
compiler/layout, undefined extreme arithmetic and linked/runtime fidelity still
require separate evidence. No original image code is executed by these controls.
"""
from dataclasses import fields
import os
from pathlib import Path
import re
import resource
import shutil
import subprocess

import pytest
from capstone import Cs, CS_ARCH_X86, CS_MODE_32
from match_recovery.includes import COMMENTS_OR_STRINGS
from match_recovery.model import Span
from match_recovery.pe_coff import PeImage
from match_recovery.source import read_text

ROOT = Path(__file__).resolve().parents[2]
BASE = "0b1619c4dbb65d57e2b08635ed939f30032d883a"
PIN = "01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf"
WIN = "void Win::on_redraw_nc(RECT * area, int flags)"
GRAPHIC = "void GraphicWin::on_redraw_nc(RECT *area, int side)"


def braced(text, signature):
    masked = COMMENTS_OR_STRINGS.sub(
        lambda m: "".join("\n" if c == "\n" else " " for c in m[0]), text)
    start = masked.index(signature)
    assert masked.find(signature, start + len(signature)) < 0
    opening = masked.index("{", start)
    depth = 0
    for end in range(opening, len(masked)):
        depth += (masked[end] == "{") - (masked[end] == "}")
        if not depth:
            return text[start:end + 1]
    raise AssertionError(signature)


def one(text, pattern):
    value, = re.findall(pattern, text, re.M)
    return value


def test_only_complete_painter_bodies_change_and_obligations_survive():
    for relative, signature, address, end in (
            ("src/win.cpp", WIN, 0x5F2CF0, 0x5F37BA),
            ("src/graphicwin.cpp", GRAPHIC, 0x5D63F0, 0x5D68B7)):
        path = ROOT / relative
        before = subprocess.check_output(["git", "show", BASE + ":" + relative],
                                         cwd=ROOT, text=True)
        after = path.read_text()
        assert after.replace(braced(after, signature), braced(before, signature), 1) == before
        old = read_text(before, path)
        current = read_text(after, path)
        assert len(old) == len(current)
        for a, b in zip(old, current):
            for field in fields(a):
                if field.name not in {"region", "line", "origin"}:
                    assert getattr(a, field.name) == getattr(b, field.name), field.name
            assert [(x.kind, x.key, x.prose) for x in a.origin] == [
                (x.kind, x.key, x.prose) for x in b.origin]
        assert next(row for row in current if row.address == address).image_spans == ((address, end),)


def test_pinned_complete_painters_and_positive_geometry_reload_anchors():
    path = Path(os.environ.get("OPENSMACX_IMAGE", ROOT / ".opensmacx/game/terranx_original.exe"))
    if not path.is_file():
        pytest.skip("set OPENSMACX_IMAGE for pinned original evidence")
    image = PeImage(path.read_bytes(), PIN)
    decoder = Cs(CS_ARCH_X86, CS_MODE_32)
    decoded = {}
    for start, end in ((0x5F2CF0, 0x5F37BA), (0x5D63F0, 0x5D68B7)):
        rows = list(decoder.disasm(image.read(Span(start, end)), start))
        assert sum(row.size for row in rows) == end - start
        decoded.update({row.address: (row.mnemonic, row.op_str) for row in rows})
    anchors = {
        0x5F2FD1: ("mov", "eax, dword ptr [esi + 0x128]"),
        0x5F2FD9: ("mov", "dword ptr [esp + 0x34], eax"),
        0x5F2FE0: ("mov", "dword ptr [esp + 0x38], eax"),
        0x5F2FE8: ("add", "edx, ebp"),
        0x5F2FF4: ("mov", "edx, dword ptr [esp + 0x14]"),
        0x5F3047: ("mov", "eax, dword ptr [esi + 0x98]"),
        0x5F30D7: ("mov", "eax, dword ptr [esi + 0xb4]"),
        0x5F3134: ("mov", "eax, dword ptr [esi + 0x158]"),
        0x5F314B: ("sub", "eax, ecx"),
        0x5F3151: ("sub", "eax, ecx"),
        0x5F3157: ("sub", "eax, ecx"),
        0x5F31C8: ("mov", "edx, dword ptr [esp + 0x30]"),
        0x5F3209: ("mov", "eax, dword ptr [esi + 0x150]"),
        0x5F320F: ("sub", "eax, ebx"),
        0x5F3288: ("mov", "ecx, dword ptr [esp + 0x28]"),
        0x5F3294: ("mov", "eax, dword ptr [esi + 0x150]"),
        0x5F34B3: ("mov", "dword ptr [esp + 0x34], ecx"),
        0x5F34CB: ("add", "ecx, ebx"),
        0x5F36F7: ("mov", "eax, dword ptr [esi + 0x14c]"),
        0x5F3701: ("sub", "eax, dword ptr [esi + 0x154]"),
        0x5F3713: ("add", "eax, dword ptr [esi + 0x118]"),
        0x5F371D: ("add", "ecx, edi"),
        0x5F371F: ("add", "ebx, edi"),
        0x5D6531: ("mov", "edx, dword ptr [esi + 0x154]"),
        0x5D6554: ("mov", "ecx, dword ptr [esi + 0xe0]"),
        0x5D657F: ("mov", "ecx, dword ptr [esi + 0xb4]"),
        0x5D6585: ("mov", "edx, dword ptr [esi + 0xe0]"),
        0x5D6591: ("call", "dword ptr [eax + 0x164]"),
        0x5D6597: ("mov", "ecx, dword ptr [esi + 0xb4]"),
        0x5D65EF: ("add", "edx, dword ptr [eax + 0x84]"),
        0x5D65F5: ("neg", "edx"),
        0x5D667C: ("mov", "edi, dword ptr [ecx + 0x84]"),
        0x5D668C: ("neg", "edi"),
        0x5D671F: ("mov", "edi, dword ptr [eax + 0x84]"),
        0x5D6725: ("sub", "edx, edi"),
        0x5D675E: ("sub", "ebx, dword ptr [ecx + 0x84]"),
        0x5D6811: ("mov", "ebx, dword ptr [eax + 0x84]"),
        0x5D6819: ("sub", "edx, ebx"),
        0x5D685E: ("sub", "ebp, dword ptr [ecx + 0x84]"),
    }
    for at, expected in anchors.items():
        assert decoded[at] == expected, hex(at)


HOST = r'''
#include <cassert>
#include <cstdint>
#include <cstring>
#include <string>
#include <vector>
#include <functional>
#include <algorithm>
#define __cdecl
typedef void *HCURSOR;
struct RECT { int32_t left,top,right,bottom; };
struct BITMAPINFOHEADER { int32_t biWidth,biHeight; };
struct RGBQUAD { unsigned char b,g,r,a; };
struct BITMAPINFO { BITMAPINFOHEADER bmiHeader;RGBQUAD bmiColors[1]; };
@DIB@
class Win;class GraphicWin;class Buffer;class Sprite;class Font {};
class Sound;class ImageButton;class Menu;class Scroll;
static const unsigned char WinFillColour=9;
struct Event {
  std::string op;Buffer *owner;const void *source;RECT rect;int a,b,c,d;char *text;
};
static std::vector<Event> events;
static std::function<void(const Event&)> boundary;
static RECT *box_argument;
static void log(const char *op,Buffer *owner,const void *source=0,RECT r={0,0,0,0},
                int a=0,int b=0,int c=0,int d=0,char *text=0) {
  Event e={op,owner,source,r,a,b,c,d,text};events.push_back(e);if(boundary)boundary(e);
}
static int IntersectRect(RECT *out,const RECT *a,const RECT *b) {
  RECT r={std::max(a->left,b->left),std::max(a->top,b->top),
          std::min(a->right,b->right),std::min(a->bottom,b->bottom)};
  bool hit=r.left<r.right&&r.top<r.bottom;if(!hit)r={0,0,0,0};*out=r;return hit;
}
class Sprite { public:
@SPRITE_FIELDS@
@SPRITE_ACCESSORS@
  int draw(Buffer *target,int transparent,int x,int y,int nx,int ny) {
    assert(transparent==cTransparentIndex_&&nx==1&&ny==1);
    log("sprite",target,this,{x,y,0,0});return 0;
  }
};
class Buffer { public:
@BUFFER_FIELDS@
@BUFFER_ACCESSORS@
  virtual ~Buffer(){}
  int fill(int c) {log("fill",this,0,{0,0,0,0},c);return 0;}
  int set_clip(RECT *r) {log("clip",this,0,*r);return 0;}
  int box(RECT *r,int a,int b) {box_argument=r;log("box",this,0,*r,a,b);box_argument=0;return 0;}
  int tile(Buffer *p,int sx,int sy,int x,int y,int w,int h) {
    log("pattern",this,p,{x,y,w,h},sx,sy);return 0;
  }
  int tile(Sprite *p,int sx,int sy,int x,int y,int w,int h) {
    log("tile",this,p,{x,y,w,h},sx,sy);return 0;
  }
  void set_text_color(int a,int b,int c,int d) {log("color",this,0,{0,0,0,0},a,b,c,d);}
  int set_font(Font *f,Font *a,Font *b,Font *c) {
    assert(!a&&!b&&!c);log("font",this,f);return 0;
  }
  int write_l(char *s,RECT *r,int n) {log("left",this,0,*r,n,0,0,0,s);return 0;}
  int write_right_l(char *s,RECT *r,int n) {log("right",this,0,*r,n,0,0,0,s);return 0;}
  int write_cent_l(char *s,RECT *r,int n) {log("center",this,0,*r,n,0,0,0,s);return 0;}
};
#include "winnodelist.h"
@AUTO@
AutoSound::AutoSound(){}void AutoSound::close(){}
@CALLBACKS@
class Win : public AutoSound { public:
@WIN_FIELDS@
@WIN_DECLARATION@
@POST_DECLARATION@
};
@GRAPHIC_INHERITANCE@
 public:
@GRAPHIC_FIELDS@
@GRAPHIC_DECLARATION@
@CAPTION_DECLARATION@
};
void Win::on_post_redraw_nc_buffer(Buffer *canvas,int side) {log("post",canvas,this,{0,0,0,0},side);}
void GraphicWin::on_draw_caption(char *text,Buffer *canvas) {log("caption",canvas,this,{0,0,0,0},0,0,0,0,text);}
@WIN_BODY@
@GRAPHIC_BODY@
static Buffer canvases[8],pattern;
static Sprite sprites[15];
static char old_title[]="old",new_title[]="new";
static Font font;
static void dimensions(Buffer &b,int w,int h) {b.dib_.bmiHeader={w,-h};b.rect2_={0,0,w,h};}
static void setup(Win &w) {
  events.clear();boundary=nullptr;
  w.iFlags_=0x10;w.iSomeFlag_=2;w.caption_height_=24;w.border_thickness_=3;w.bottom_border_thickness_=7;
  w.client_rect_={10,20,110,100};w.outer_rect_={3,4,103,84};
  w.field_FC_=0;w.field_100_=17;w.field_120_=18;w.field_124_=19;w.field_128_=2;
  w.field_104_=20;w.field_108_=21;w.field_10C_=22;w.field_110_=23;w.field_F8_=&font;w.field_E0_=old_title;
  dimensions(canvases[0],100,24);dimensions(canvases[1],3,49);
  dimensions(canvases[2],3,49);dimensions(canvases[3],100,7);
  for(int i=4;i<8;++i)dimensions(canvases[i],120+i,60+i);
  w.buffer1_=&canvases[0];w.buffer2_=&canvases[1];w.buffer3_=&canvases[2];w.buffer4_=&canvases[3];
}
static bool equal(RECT a,RECT b) {return a.left==b.left&&a.top==b.top&&a.right==b.right&&a.bottom==b.bottom;}
static std::vector<Event> select(const char *op,Buffer *owner=0) {
  std::vector<Event> out;for(auto e:events)if(e.op==op&&(!owner||e.owner==owner))out.push_back(e);return out;
}
static void expect_boxes(Buffer *b,RECT outside,RECT inside,int depth=2) {
  auto boxes=select("box",b);assert(static_cast<int>(boxes.size())==depth*2);
  for(int i=0;i<depth;++i) {
    RECT a={outside.left+i,outside.top+i,outside.right-i,outside.bottom-i};
    RECT z={inside.left-i,inside.top-i,inside.right+i,inside.bottom+i};
    assert(equal(boxes[i].rect,a)&&boxes[i].a==18&&boxes[i].b==19);
    assert(equal(boxes[i+depth].rect,z)&&boxes[i+depth].a==19&&boxes[i+depth].b==18);
  }
}
static void win_geometry() {
  Win w;setup(w);w.on_redraw_nc(0,-1);
  expect_boxes(&canvases[0],{0,0,100,80},{2,23,104,74});
  expect_boxes(&canvases[3],{0,-73,100,7},{2,-50,104,1});
  expect_boxes(&canvases[2],{0,-24,100,56},{2,-1,104,50});
  expect_boxes(&canvases[1],{-97,-24,3,56},{-95,-1,7,50});
  auto posts=select("post");assert(posts.size()==4);
  int order[]={0,2,3,1};Buffer *owners[]={&canvases[0],&canvases[3],&canvases[2],&canvases[1]};
  for(int i=0;i<4;++i)assert(posts[i].a==order[i]&&posts[i].owner==owners[i]);
  auto text=select("center");assert(text.size()==1&&equal(text[0].rect,{2,2,98,22})&&text[0].a==3);
  auto color=select("color");assert(color.size()==1&&color[0].a==20&&color[0].b==21&&color[0].c==22&&color[0].d==23);
  for(int align:{0,0x10000,0x40000,0x50000}) {
    setup(w);w.iFlags_|=align;w.on_redraw_nc(0,0);
    const char *op=(align&0x10000)?"left":((align&0x40000)?"right":"center");assert(select(op).size()==1);
  }
  for(int side:{0,1,2,3,99}) {
    setup(w);w.on_redraw_nc(0,side);auto p=select("post");
    assert(p.size()==static_cast<unsigned>(side==99?0:1));if(side!=99)assert(p[0].a==side);
  }
  for(int flags:{0,0x20}) {setup(w);w.iFlags_=flags;w.on_redraw_nc(0,-1);assert(events.empty());}
  setup(w);w.iSomeFlag_=0;w.on_redraw_nc(0,-1);assert(events.empty());
  setup(w);w.buffer2_=0;w.buffer4_=0;w.on_redraw_nc(0,-1);assert(select("post").size()==2);
  setup(w);RECT area={-1000,-1000,1000,1000},saved=area;w.on_redraw_nc(&area,99);
  assert(select("post").size()==4&&equal(area,saved));
  setup(w);area={10,20,110,100};w.on_redraw_nc(&area,99);
  // Retain the original positive bottom clipping shift, including its empty
  // intersection here. A normalised negative shift would incorrectly paint it.
  assert(select("post").size()==4&&select("box",&canvases[3]).empty());
  setup(w);area={10,-53,110,-46};w.on_redraw_nc(&area,99);
  assert(select("box",&canvases[3]).size()==4);
  assert(equal(select("clip",&canvases[3])[0].rect,{0,0,100,7}));
  setup(w);w.bottom_border_thickness_=-1;w.iFlags_|=0x400000;w.on_redraw_nc(0,-1);
  expect_boxes(&canvases[3],{0,-77,100,3},{2,-75,104,1});
  expect_boxes(&canvases[2],{0,-3,100,77},{2,-1,104,75});
  for(int depth:{0,-1,1,3}) {
    setup(w);w.field_128_=static_cast<uint32_t>(depth);w.on_redraw_nc(0,0);
    assert(select("box").size()==static_cast<unsigned>(depth>0?depth*2:0));
    assert(equal(select("center")[0].rect,{depth,depth,100-depth,24-depth}));
  }
  setup(w);assert(reinterpret_cast<uintptr_t>(&pattern)<=0xFFFFFFFFu);
  w.field_FC_=static_cast<uint32_t>(reinterpret_cast<uintptr_t>(&pattern));w.on_redraw_nc(0,-1);
  auto tiles=select("pattern");assert(tiles.size()==4&&select("fill").empty());
  int sx[]={0,0,0,97},sy[]={0,73,24,24};
  for(int i=0;i<4;++i)assert(tiles[i].source==&pattern&&tiles[i].a==sx[i]&&tiles[i].b==sy[i]&&equal(tiles[i].rect,{0,0,100,80}));
}
static void win_mutations() {
  Win w;setup(w);bool changed=false;
  boundary=[&](const Event&e){if(!changed&&e.op=="fill"){changed=true;w.buffer1_=&canvases[4];w.iFlags_|=0x10000;}};
  w.on_redraw_nc(0,0);boundary=nullptr;
  auto boxes=select("box");assert(boxes.size()==4);for(auto e:boxes)assert(e.owner==&canvases[4]);
  assert(select("left").size()==1&&select("left")[0].owner==&canvases[4]);
  assert(select("post")[0].owner==&canvases[4]);
  setup(w);boundary=[&](const Event&e){if(e.op=="font"){w.field_E0_=0;w.buffer1_=&canvases[5];}};
  w.on_redraw_nc(0,0);boundary=nullptr;assert(select("center").empty()&&select("post")[0].owner==&canvases[5]);
  setup(w);int calls=0;
  boundary=[&](const Event&e){if(e.op=="box"&&e.owner==&canvases[1]&&++calls==1){w.client_rect_.right=120;w.border_thickness_=4;}};
  w.on_redraw_nc(0,1);boundary=nullptr;
  auto b=select("box");assert(equal(b[0].rect,{-97,-24,3,56}));
  assert(equal(b[2].rect,{-104,-1,-2,50}));
  setup(w);boundary=[&](const Event&e){if(e.op=="post"&&e.a==0){w.client_rect_.bottom=120;}};
  w.on_redraw_nc(0,-1);boundary=nullptr;
  expect_boxes(&canvases[3],{0,-93,100,-13},{2,-70,104,-19});
  setup(w);calls=0;boundary=[&](const Event&e){if(e.op=="box"&&++calls==1)w.field_128_=1;};
  w.on_redraw_nc(0,0);boundary=nullptr;assert(select("box").size()==2);
  assert(equal(select("center")[0].rect,{1,1,99,23}));
  setup(w);bool modified=false;
  boundary=[&](const Event&e){if(!modified&&e.op=="box"&&box_argument){modified=true;box_argument->left=7;}};
  w.on_redraw_nc(0,0);boundary=nullptr;assert(select("box")[1].rect.left==8);
}
static int caption_hook(GraphicWin *w,char *text,Buffer *canvas) {log("hook",canvas,w,{0,0,0,0},0,0,0,0,text);return 0;}
static void graphic_setup(GraphicWin&w) {
  setup(w);w.iFlags_=0x10000010;w.caption_height_=5;w.border_thickness_=2;
  w.client_rect_={0,0,40,30};dimensions(canvases[0],40,5);dimensions(canvases[1],2,20);
  dimensions(canvases[2],2,20);dimensions(canvases[3],40,6);
  int widths[]={4,6,5,6,2,2,2,2,3,3,3},heights[]={3,3,4,5,2,2,2,2,3,3,3};
  for(int i=0;i<11;++i){sprites[i].iWidth_=widths[i];sprites[i].iHeight_=heights[i];sprites[i].cTransparentIndex_=7;}
  w.frame_top_left_=&sprites[0];w.frame_top_right_=&sprites[1];w.frame_bottom_left_=&sprites[2];w.frame_bottom_right_=&sprites[3];
  w.frame_top_=&sprites[4];w.frame_left_=&sprites[5];w.frame_right_=&sprites[6];w.frame_bottom_=&sprites[7];
  w.frame_noncaption_left_=&sprites[8];w.frame_noncaption_right_=&sprites[9];w.frame_noncaption_middle_=&sprites[10];
  w.field_A0C_=caption_hook;
}
static void graphic_geometry() {
  GraphicWin w;graphic_setup(w);w.on_redraw_nc(0,-1);
  auto draws=select("sprite"),tiles=select("tile"),posts=select("post");
  assert(draws.size()==8&&tiles.size()==4&&posts.size()==4);
  RECT positions[]={{0,0,0,0},{34,0,0,0},{0,2,0,0},{34,1,0,0},{0,-5,0,0},{0,23,0,0},{-4,-5,0,0},{-4,22,0,0}};
  for(int i=0;i<8;++i)assert(equal(draws[i].rect,positions[i]));
  RECT areas[]={{4,0,30,5},{5,4,29,2},{0,-2,2,25},{0,-2,2,24}};
  for(int i=0;i<4;++i)assert(equal(tiles[i].rect,areas[i]));
  for(int side:{0,1,2,3,99}) {
    graphic_setup(w);w.on_redraw_nc(0,side);auto p=select("post");
    assert(p.size()==static_cast<unsigned>(side==99?0:1));if(side!=99)assert(p[0].a==side);
  }
  graphic_setup(w);RECT area={99,88,77,66};w.on_redraw_nc(&area,2);assert(select("post").size()==4);
  graphic_setup(w);w.iSomeFlag_=0;w.on_redraw_nc(0,-1);assert(events.empty());
  graphic_setup(w);w.iFlags_&=~0x10000000u;w.on_redraw_nc(0,-1);
  assert(select("sprite").empty()&&select("box").size()==16&&select("post").size()==4);
  graphic_setup(w);w.iFlags_|=0x400000;w.bottom_border_thickness_=-1;w.on_redraw_nc(0,-1);
  auto t=select("tile");assert(t[0].source==&sprites[10]&&equal(t[0].rect,{3,0,34,5}));
  assert(equal(t[2].rect,{0,1,2,17})&&equal(t[3].rect,{0,1,2,16}));
  graphic_setup(w);w.frame_top_left_=w.frame_top_right_=w.frame_bottom_left_=w.frame_bottom_right_=0;
  w.on_redraw_nc(0,-1);assert(select("sprite").empty()&&select("tile").empty()&&select("post").size()==4);
  graphic_setup(w);w.buffer2_=w.buffer4_=0;w.on_redraw_nc(0,-1);assert(select("post").size()==2);
  // A positive stored DIB height still follows native signed subtraction;
  // it must not be normalised with abs or clamped to positive dimensions.
  graphic_setup(w);canvases[3].dib_.bmiHeader.biHeight=6;w.on_redraw_nc(0,2);
  assert(equal(select("sprite")[0].rect,{0,-10,0,0}));
}
static void graphic_mutations() {
  GraphicWin w;graphic_setup(w);
  boundary=[&](const Event&e){
    if(e.op=="hook"){assert(e.text==old_title&&e.owner==&canvases[0]);w.field_E0_=new_title;w.buffer1_=&canvases[4];}
    if(e.op=="caption"){assert(e.text==new_title&&e.owner==&canvases[4]);w.buffer1_=&canvases[5];}
  };
  w.on_redraw_nc(0,0);boundary=nullptr;
  assert(select("hook").size()==1&&select("caption").size()==1&&select("post")[0].owner==&canvases[5]);
  graphic_setup(w);boundary=[&](const Event&e){if(e.op=="hook")w.field_E0_=0;};
  w.on_redraw_nc(0,0);boundary=nullptr;assert(select("caption")[0].text==0);
  for(int side:{0,2,3,1}) {
    graphic_setup(w);bool changed=false;
    boundary=[&](const Event&e){if(!changed&&e.op=="fill"){
      changed=true;Buffer *next=&canvases[4];
      if(side==0)w.buffer1_=next;if(side==2)w.buffer4_=next;if(side==3)w.buffer3_=next;if(side==1)w.buffer2_=next;
    }};
    w.on_redraw_nc(0,side);boundary=nullptr;
    for(auto e:select("sprite"))assert(e.owner==&canvases[4]);
    for(auto e:select("tile"))assert(e.owner==&canvases[4]);
    assert(select("post")[0].owner==&canvases[4]);
  }
  graphic_setup(w);bool changed=false;
  boundary=[&](const Event&e){if(!changed&&e.op=="sprite"){changed=true;w.client_rect_.right=60;}};
  w.on_redraw_nc(0,0);boundary=nullptr;assert(select("tile")[0].rect.right==50);
  graphic_setup(w);boundary=[&](const Event&e){if(e.op=="fill"){w.field_E0_=new_title;}};
  w.field_E0_=0;w.on_redraw_nc(0,0);boundary=nullptr;assert(select("hook").size()==1&&select("hook")[0].text==new_title);
  graphic_setup(w);boundary=[&](const Event&e){if(e.op=="post"&&e.a==0){w.buffer4_=0;w.frame_top_left_=&sprites[8];}};
  w.on_redraw_nc(0,-1);boundary=nullptr;assert(select("fill").size()==3);
  auto r=select("sprite",&canvases[2]);assert(r[0].source==&sprites[0]); // original top-left snapshot
}
int main(){win_geometry();win_mutations();graphic_geometry();graphic_mutations();}
'''


def program(damage=None):
    wh=(ROOT/"src/win.h").read_text();gh=(ROOT/"src/graphicwin.h").read_text()
    bh=(ROOT/"src/buffer.h").read_text();sh=(ROOT/"src/sprite.h").read_text()
    win=braced((ROOT/"src/win.cpp").read_text(),WIN)
    graphic=braced((ROOT/"src/graphicwin.cpp").read_text(),GRAPHIC)
    mutations={
        "caption-inset":("win","r.left = inset; r.top = inset;","r.left = 0; r.top = 0;"),
        "bottom-clip":("win","- bottomThick - savedTop;","- capHeight - savedTop;"),
        "bottom-pattern":("win","0, clientH - bottomThick, 0, 0","0, capHeight, 0, 0"),
        "bottom-box":("win","r.top = shiftY;","r.top = -capHeight;"),
        "right-box":("win","r.bottom = outerH - capHeight;","r.bottom = outerH;"),
        "left-box":("win","r.left = loop2Left + borderM1;","r.left = loop2Left + borderM1 - capHeight;"),
        "left-refresh":("win","r.left = loop2Left + borderM1;","r.left = loop1Left + borderM1;"),
        "alignment-refresh":("win","win_flags = this->iFlags_;\n                    char *title", "/* stale flags */\n                    char *title"),
        "win-buffer-refresh":("win","buf = this->buffer1_;","/* stale buffer */"),
        "bottom-sprite":("graphic","buffer->pixel_height() - frame_bottom_left_->height()","-(buffer->pixel_height() + frame_bottom_left_->height())"),
        "bottom-tile":("graphic","buffer->pixel_height() - frame_bottom_->height()","-(buffer->pixel_height() + frame_bottom_->height())"),
        "side-sprite":("graphic","bottom_inset + buffer->pixel_height()","bottom_inset - buffer->pixel_height()"),
        "side-tile":("graphic","(((bottom_inset + buffer->pixel_height())","(((bottom_inset - buffer->pixel_height())"),
        "caption-refresh":("graphic","on_draw_caption(field_E0_, buffer1_);","on_draw_caption(caption_snapshot, buffer);"),
        "post-refresh":("graphic","on_post_redraw_nc_buffer(buffer1_, 0);","on_post_redraw_nc_buffer(buffer, 0);"),
        "client-width-refresh":("graphic","(client_rect_.right - client_rect_.left)","40"),
        "graphic-buffer-refresh":("graphic","buffer = buffer4_;","/* stale bottom buffer */"),
    }
    if damage:
        owner,old,new=mutations[damage]
        target=win if owner=="win" else graphic
        assert old in target,damage
        # Receiver damage removes every explicit reload in this owner; geometry
        # and callback damage changes exactly one independent branch witness.
        if damage.endswith("buffer-refresh"):
            target=re.sub(r"(?m)^([ \t]*)"+re.escape(old),r"\1"+new,target)
        else:
            target=target.replace(old,new,1)
        if damage=="caption-refresh":
            target=target.replace("    // Quadrant 0:","    char *caption_snapshot = field_E0_;\n    // Quadrant 0:",1)
        if owner=="win":win=target
        else:graphic=target
    fields_start=wh.index("  uint32_t iFlags_;")
    win_fields=wh[fields_start:wh.index("\n public:",fields_start)]
    graphic_fields=gh[gh.index("  Sprite *frame_top_left_;"):gh.index("\n};",gh.index("  Sprite *frame_top_left_;"))]
    replace={
        "DIB":braced(bh,"struct Dib : BITMAPINFO {")+";",
        "SPRITE_FIELDS":"\n".join(one(sh,r"^  ("+p+r";)") for p in ("int iWidth_","int iHeight_","unsigned char cTransparentIndex_")),
        "SPRITE_ACCESSORS":"\n".join(braced(sh,p) for p in ("unsigned char transparent_index() const","int width() const","int height() const")),
        "BUFFER_FIELDS":"\n".join(one(bh,r"^  ("+p+r";)") for p in ("RECT rect2_","Dib dib_")),
        "BUFFER_ACCESSORS":"\n".join(braced(bh,p) for p in ("int pixel_width() const","int pixel_height() const")),
        "AUTO":braced((ROOT/"src/autosound.h").read_text(),"class AutoSound {")+";",
        "CALLBACKS":"\n".join(re.findall(r"^typedef .*;$",wh,re.M)+re.findall(r"^typedef .*;$",gh,re.M)),
        "WIN_FIELDS":win_fields,
        "WIN_DECLARATION":one(wh,r"^  (virtual void on_redraw_nc\(RECT \*area, int flags\);).*"),
        "POST_DECLARATION":one(wh,r"^  (virtual void on_post_redraw_nc_buffer\([^\{]+)\{").rstrip()+";",
        "GRAPHIC_INHERITANCE":one(gh,r"^(class GraphicWin :[^\{]+\{)"),
        "GRAPHIC_FIELDS":graphic_fields,
        "GRAPHIC_DECLARATION":one(gh,r"^  (virtual void on_redraw_nc\(RECT \*area, int flags\);).*"),
        "CAPTION_DECLARATION":one(gh,r"^  (virtual void on_draw_caption\([^\{]+)\{").rstrip()+";",
        "WIN_BODY":win,"GRAPHIC_BODY":graphic,
    }
    result=HOST
    for key,value in replace.items():
        assert "@"+key+"@" in result;result=result.replace("@"+key+"@",value)
    assert all("@"+key+"@" not in result for key in replace)
    return result


def run(tmp_path,compiler_name,damage=None):
    compiler=shutil.which(compiler_name)
    if not compiler:pytest.skip("host compiler unavailable")
    source=tmp_path/"painter.cpp";binary=tmp_path/"painter"
    source.write_text(program(damage))
    # Win's existing field_FC_ is an original 32-bit word, with unresolved
    # header typing outside this task. Static named host pattern storage in a
    # non-PIE executable keeps this exercised pointer value representable;
    # there is no original executable address or native-layout claim here.
    built=subprocess.run([compiler,"-std=c++17","-no-pie","-iquote",str(ROOT/"src"),
                          str(source),"-o",str(binary)],capture_output=True,text=True,timeout=60)
    assert built.returncode==0,built.stderr
    def no_core():resource.setrlimit(resource.RLIMIT_CORE,(0,0))
    return subprocess.run([str(binary)],capture_output=True,text=True,timeout=15,preexec_fn=no_core)


@pytest.mark.parametrize("compiler",["g++","clang++"])
def test_complete_actual_painter_methods(tmp_path,compiler):
    result=run(tmp_path,compiler)
    assert result.returncode==0,result.stderr


@pytest.mark.parametrize("damage",[
    "caption-inset","bottom-clip","bottom-pattern","bottom-box","right-box","left-box","left-refresh",
    "alignment-refresh","win-buffer-refresh","bottom-sprite","bottom-tile","side-sprite","side-tile",
    "caption-refresh","post-refresh","client-width-refresh","graphic-buffer-refresh",
])
def test_controls_reject_executed_damage(tmp_path,damage):
    result=run(tmp_path,"g++",damage)
    assert result.returncode!=0,"damage escaped: "+damage
