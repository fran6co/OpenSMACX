"""Full actual 6253D0 painter with strict host sanitizers and service doubles.

Original 6253D0..6256CC independently supplies the event/geometry oracle.
Declarations, inheritance, helper functions and the complete painter are read
from source. Palette/fill/box/focus/Sprite/text are explicit callback boundaries,
not pixel, native ABI, real focus-list or game proof. The original leaves center
coordinates uninitialized when the centering sprite is null: a later bevel
callback publishing a sprite is outside this fixture's defined domain. Null
remaining null is covered. LSan is disabled for the ptrace sandbox, not ASan or
UBSan. Fixtures use valid borrowed objects and bounded geometry.
"""
from pathlib import Path
import os
import re
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
    raise AssertionError(signature)


def one(pattern, text):
    result, = re.findall(pattern, text, re.M)
    return result


def actual():
    headers = {n: (ROOT / "src" / (n + ".h")).read_text() for n in
               ("win", "buffer", "graphicwin", "basebutton", "imagebutton", "sprite", "palette", "temp")}
    cpp = (ROOT / "src/imagebutton.cpp").read_text()
    parts = {"BODY": braced(cpp, "void ImageButton::on_redraw()")}
    parts["HELPERS"] = "\n".join(braced(cpp, signature) for signature in (
        "int image_button_paint_signed(", "int image_button_paint_add(",
        "int image_button_paint_sub(", "int image_button_paint_half(",
        "void image_button_paint_inset("))
    for cls, unit in (("Win", "win"), ("GraphicWin", "graphicwin"),
                      ("BaseButton", "basebutton"), ("ImageButton", "imagebutton")):
        parts[cls.upper() + "_BASE"] = one(r"^class " + cls + r" : [^{]+\{", headers[unit])
    parts["WIN_FIELDS"] = one(r"^  Win \*win_parent_;", headers["win"])
    parts["WIN_METHODS"] = "\n".join(one(pattern, headers["win"]) for pattern in
        (r"^  virtual void on_redraw\(\)[^\n]*", r"^  int is_dialog_focus\(\);"))
    parts["BUFFER_RECT"] = one(r"^  RECT rect2_;", headers["buffer"])
    parts["BUFFER_METHODS"] = "\n".join(one(pattern, headers["buffer"]) for pattern in
        (r"^  int sync_to_palette\(Palette \*palette\);", r"^  int box\(RECT \*rect, int color1, int color2\);",
         r"^  int write_cent_l\(LPSTR text, RECT \*rect, int len\);"))
    parts["FILL_DECL"] = one(r"^  void fill\(int color\);", headers["graphicwin"])
    parts["BASE_FIELDS"] = "\n".join(one(r"^  (?:uint32_t |LPSTR )" + name + ";", headers["basebutton"])
        for name in ("name_", "color_", "bevel_upper_", "bevel_lower_", "bevel_thickness_", "field_A94_", "field_A9C_"))
    parts["IMAGE_FIELDS"] = one(r"^  Sprite \*sprite_;", headers["imagebutton"])
    parts["IMAGE_METHOD"] = one(r"^  virtual void on_redraw\(\);", headers["imagebutton"])
    parts["SPRITE_FIELDS"] = "\n".join(one(r"^  (?:int |unsigned char )" + name + ";", headers["sprite"])
        for name in ("cTransparentIndex_", "iSpriteWidth_", "iWidth_", "iHeight_"))
    parts["SPRITE_METHODS"] = "\n".join(braced(headers["sprite"], sig) for sig in
        ("unsigned char transparent_index() const", "int width() const", "int height() const", "int source_width() const"))
    parts["SPRITE_DRAW"] = one(r"  int draw\(Buffer \*destination, int transparent, int x, int y,\s*int numerator, int denominator\);", headers["sprite"])
    parts["GLOBALS"] = "\n".join(one(pattern, (ROOT / "src" / unit).read_text()) for unit, pattern in
        (("temp.cpp", r"^HWND HandleMain;"), ("win.cpp", r"^Win \*WinCallbackWindow;"),
         ("palette.cpp", r"^Palette \*PaletteActive;")))
    return parts


HARNESS = r'''
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>
#include <functional>
#include <string>
using LPSTR=char*; using HWND=void*;
struct RECT {int32_t left,top,right,bottom;};
struct Palette {int id;};struct AutoSound{};struct Buffer;
struct Sprite { SPRITE_FIELDS SPRITE_METHODS SPRITE_DRAW };
WIN_BASE public: WIN_FIELDS WIN_METHODS };
struct Buffer { BUFFER_RECT BUFFER_METHODS };
GRAPHICWIN_BASE public: FILL_DECL };
BASEBUTTON_BASE public: BASE_FIELDS };
IMAGEBUTTON_BASE public: IMAGE_FIELDS IMAGE_METHOD };
GLOBALS
HELPERS
BODY
void require_at(bool ok,int line){if(!ok){fprintf(stderr,"assertion line %d\n",line);std::_Exit(97);}}
#define require(x) require_at((x),__LINE__)
struct Event {char kind; void*who; RECT area; int a,b,c,d,e,f; std::string text;};
std::vector<Event> events;
std::function<void(char,int,RECT*)> hook;
ImageButton *button; int focus; int box_count; Palette palettes[2]; Win parents[2];
void emit(char kind,void*who,RECT area={},int a=0,int b=0,int c=0,int d=0,int e=0,int f=0,const char*text=""){
 events.push_back({kind,who,area,a,b,c,d,e,f,text});
 if(hook) hook(kind,kind=='B'?++box_count:0,kind=='B'?&area:nullptr);
}
int Buffer::sync_to_palette(Palette*p){require(this==static_cast<Buffer*>(button));emit('P',p);return 8;}
void GraphicWin::fill(int color){require(this==static_cast<GraphicWin*>(button));emit('F',this,{},color);}
int Buffer::box(RECT*r,int a,int b){require(this==static_cast<Buffer*>(button));
 events.push_back({'B',this,*r,a,b,0,0,0,0,""});if(hook)hook('B',++box_count,r);else ++box_count;return 9;}
int Win::is_dialog_focus(){require(this==static_cast<Win*>(button));emit('Q',this);return focus;}
int Sprite::draw(Buffer*dst,int key,int x,int y,int n,int d){require(dst==static_cast<Buffer*>(button));emit('S',this,{},key,x,y,n,d);return 10;}
int Buffer::write_cent_l(char*t,RECT*r,int len){require(this==static_cast<Buffer*>(button));emit('T',this,*r,len,0,0,0,0,0,t);return 11;}
char label[]="Caption";char replacement[]="New text";
void reset(ImageButton&b,Sprite*s){button=&b;events.clear();hook={};focus=0;box_count=0;HandleMain=&parents[0];
 PaletteActive=&palettes[0];WinCallbackWindow=&parents[1];b.win_parent_=&parents[0];b.rect2_={10,20,70,60};
 b.name_=label;b.sprite_=s;b.color_=101;b.bevel_upper_=102;b.bevel_lower_=103;b.bevel_thickness_=2;b.field_A94_=uint32_t(-1);b.field_A9C_=0;}
void rect_eq(RECT a,RECT b){require(a.left==b.left&&a.top==b.top&&a.right==b.right&&a.bottom==b.bottom);}
std::vector<Event> kind(char c){std::vector<Event>v;for(auto&e:events)if(e.kind==c)v.push_back(e);return v;}
int main(){ImageButton b;Sprite a{241,100,13,9}, second{230,7,31,17};
 // Every ordinary branch combination. Geometry expectations use original
 // independent counts, not the implementation's inset or SAR helpers.
 for(int pressed=0;pressed<2;++pressed)for(int outline=0;outline<2;++outline)
 for(int focused=0;focused<2;++focused)for(int thick=0;thick<=3;++thick)
 for(int narrow=0;narrow<2;++narrow)for(int null_sprite=0;null_sprite<2;++null_sprite){
  a.iSpriteWidth_=narrow?100:5;reset(b,null_sprite?nullptr:&a);b.field_A9C_=pressed;b.field_A94_=outline?104:uint32_t(-1);
  focus=focused;b.bevel_thickness_=thick;static_cast<Win*>(&b)->on_redraw();
  require(events.size()>=2);require(events[0].kind=='P'&&events[0].who==&palettes[0]);require(events[1].kind=='F'&&events[1].a==101);
  require(WinCallbackWindow==&parents[0]);require(kind('Q').size()==unsigned(outline));
  int focus_layer=outline&&focused;int bevels=thick>focus_layer?thick-focus_layer:0;
  int borders=outline+focus_layer;int total=borders+bevels;auto boxes=kind('B');require(boxes.size()==unsigned(total));
  for(int i=0;i<total;++i){rect_eq(boxes[i].area,{10+i,20+i,70-i,60-i});
   require(boxes[i].a==(i<borders?104:pressed?103:102));require(boxes[i].b==(i<borders?104:pressed?102:103));}
  auto draws=kind('S');require(draws.size()==unsigned(!null_sprite));
  if(!null_sprite){auto d=draws[0];int cx=(60-13-thick)/2,cy=(40-9-thick)/2;
   int available=60-2*total-(pressed?thick:0);require(d.who==&a&&d.a==241&&d.b==cx+pressed*thick&&d.c==cy+pressed*thick);
   require(d.d==(narrow?available:1)&&d.e==(narrow?100:1));}
  auto texts=kind('T');require(texts.size()==1&&texts[0].text=="Caption"&&texts[0].a==7);
  int shift=pressed*thick;rect_eq(texts[0].area,{10+total+shift,20+total+shift,70-total+shift,60-total+shift});
 }
 reset(b,&a);HandleMain=nullptr;static_cast<Win*>(&b)->on_redraw();require(events.empty()&&WinCallbackWindow==&parents[1]);
 reset(b,&a);b.win_parent_=nullptr;b.on_redraw();require(events.empty()&&WinCallbackWindow==&parents[1]);
 // Negative odd centering uses floor SAR, not C++ truncation or an origin add.
 reset(b,&a);b.rect2_={10,20,16,24};b.bevel_thickness_=0;b.name_=nullptr;b.on_redraw();
 auto d=kind('S')[0];require(d.b==-4&&d.c==-3&&d.d==6&&d.e==100);require(kind('T').empty());
 // Palette callback precedes rectangle/color snapshot; fill changes live
 // geometry for centering but must not change the saved frame rectangle.
 reset(b,&a);hook=[&](char c,int,RECT*){if(c=='P'){require(WinCallbackWindow==&parents[0]);b.rect2_={3,7,53,37};b.color_=201;b.win_parent_=&parents[1];}
  if(c=='F'){b.rect2_={1,2,102,63};b.name_=replacement;HandleMain=nullptr;}};
 b.on_redraw();require(events[1].a==201);rect_eq(kind('B')[0].area,{3,7,53,37});d=kind('S')[0];require(d.b==43&&d.c==25);
 require(kind('T')[0].text=="New text"&&WinCallbackWindow==&parents[0]);
 // Focus outline rereads color, preserves callee RECT changes, and the
 // center is computed after focus but before the first bevel callback.
 reset(b,&a);b.field_A94_=104;focus=1;
 hook=[&](char c,int n,RECT*r){if(c=='B'&&n==1){r->left=30;b.field_A94_=105;}
  if(c=='Q'){b.field_A94_=106;b.rect2_={0,0,43,33};}
  if(c=='B'&&n==3){b.sprite_=&second;b.rect2_={0,0,999,999};b.name_=replacement;}};
 b.on_redraw();auto bx=kind('B');require(bx.size()==3&&bx[0].a==104&&bx[1].a==106);rect_eq(bx[1].area,{31,21,69,59});
 d=kind('S')[0];require(d.who==&second&&d.a==230&&d.b==14&&d.c==11&&d.d==1&&d.e==1);
 // Branch selected once; thickness and bevel colors remain live, including
 // fresh thickness after Sprite draw for the text rectangle translation.
 reset(b,&a);b.field_A9C_=1;
 hook=[&](char c,int n,RECT*){if(c=='B'&&n==1){b.field_A9C_=0;b.bevel_thickness_=3;b.bevel_upper_=202;b.bevel_lower_=203;}
  if(c=='S'){b.bevel_thickness_=7;b.name_=replacement;}};
 b.on_redraw();bx=kind('B');require(bx.size()==3&&bx[1].a==203&&bx[1].b==202);d=kind('S')[0];require(d.b==25&&d.c==17&&d.d==51&&d.e==100);
 rect_eq(kind('T')[0].area,{20,30,74,64});require(kind('T')[0].text=="New text");
 // Removing a sprite after centering suppresses its call; no false draw.
 reset(b,&a);hook=[&](char c,int,RECT*){if(c=='B')b.sprite_=nullptr;};b.on_redraw();require(kind('S').empty());
 // Signed thickness and no-outline sentinel are full 32-bit fields.
 reset(b,&a);b.bevel_thickness_=uint32_t(-1);b.color_=uint32_t(-2);b.on_redraw();require(kind('B').empty()&&events[1].a==-2);
 require(kind('S')[0].b==24&&kind('S')[0].c==16);
 // Exact source-width fit takes the unscaled1/1 arm, not56/56.
 reset(b,&a);a.iSpriteWidth_=56;b.on_redraw();d=kind('S')[0];require(d.d==1&&d.e==1);a.iSpriteWidth_=100;
 // Fill may add an initial sprite (before centering), focus may change the
 // pressed branch, and the shared callback owner is not restored on return.
 reset(b,nullptr);b.field_A94_=104;focus=1;
 hook=[&](char c,int,RECT*){if(c=='F')b.sprite_=&a;if(c=='Q'){b.field_A9C_=1;WinCallbackWindow=&parents[1];}};
 b.on_redraw();require(WinCallbackWindow==&parents[1]);d=kind('S')[0];require(d.b==24&&d.c==16);
 require(kind('B').back().a==103&&kind('B').back().b==102);
 // Even a sprite-less pressed button translates text after its bevels.
 reset(b,nullptr);b.field_A9C_=1;b.on_redraw();rect_eq(kind('T')[0].area,{14,24,70,60});
 puts("imagebutton paint controls passed");}
'''


def run(tmp_path, parts):
    compiler = shutil.which("clang++") or shutil.which("g++")
    if not compiler:
        pytest.skip("host C++ compiler unavailable")
    source = HARNESS
    for key in sorted(parts, key=len, reverse=True):
        source = source.replace(key, parts[key])
    cpp, executable = tmp_path / "paint.cpp", tmp_path / "paint"
    cpp.write_text(source)
    built = subprocess.run([compiler, "-std=c++17", "-O1", "-g", "-fsanitize=address,undefined",
                            "-fno-sanitize-recover=all", "-fno-omit-frame-pointer", str(cpp), "-o", str(executable)],
                           text=True, capture_output=True, timeout=60)
    assert built.returncode == 0, built.stdout + built.stderr
    return subprocess.run([str(executable)], text=True, capture_output=True, timeout=20,
                          env={**os.environ, "ASAN_OPTIONS": "detect_leaks=0", "UBSAN_OPTIONS": "halt_on_error=1"})


def test_actual_imagebutton_painter(tmp_path):
    result = run(tmp_path, actual())
    assert result.returncode == 0, result.stdout + result.stderr


# Every mutant must compile and reach the fixture's explicit assertion97;
# sanitizer failures or crashes are not counted as behavioral damage evidence.
DAMAGE = {
    "handle_guard": ("BODY", "!HandleMain || !win_parent_", "!win_parent_"),
    "parent_guard": ("BODY", "!HandleMain || !win_parent_", "!HandleMain"),
    "callback_receiver": ("BODY", "WinCallbackWindow = win_parent_;", "WinCallbackWindow = this;"),
    "palette_omitted": ("BODY", "canvas->sync_to_palette(PaletteActive);", "(void)PaletteActive;"),
    "frame_snapshot_late": ("BODY", "RECT area = rect2_;\n    GraphicWin::fill(image_button_paint_signed(color_));",
        "GraphicWin::fill(image_button_paint_signed(color_));\n    RECT area = rect2_;"),
    "outline_sentinel": ("BODY", "first_outline != -1", "first_outline == -1"),
    "focus_polarity": ("BODY", "if (is_dialog_focus())", "if (!is_dialog_focus())"),
    "focus_color_cached": ("BODY", "const int focused_outline = image_button_paint_signed(field_A94_);", "const int focused_outline = first_outline;"),
    "focus_layer": ("BODY", "layers = 1;", "layers = 0;"),
    "center_source_width": ("BODY", "centering_sprite->width()", "centering_sprite->source_width()"),
    "center_height": ("BODY", "centering_sprite->height()", "centering_sprite->width()"),
    "center_origin": ("BODY", "x = center_x;", "x = center_x + rect2_.left;"),
    "sar_negative": ("HELPERS", "return value >= 0 ? value / 2 : -1 - ((-1 - value) / 2);", "return value / 2;"),
    "bevel_bound": ("BODY", "while (layers < image_button_paint_signed(bevel_thickness_))", "while (layers <= image_button_paint_signed(bevel_thickness_))"),
    "bevel_pressed_colors": ("BODY", "canvas->box(&area, image_button_paint_signed(bevel_lower_),\n                        image_button_paint_signed(bevel_upper_));",
        "canvas->box(&area, image_button_paint_signed(bevel_upper_),\n                        image_button_paint_signed(bevel_lower_));"),
    "pressed_branch_live": ("BODY", "if (!pressed) {\n            canvas->box", "if (field_A9C_ == 0) {\n            canvas->box"),
    "bevel_thickness_cached": ("BODY", "while (layers < image_button_paint_signed(bevel_thickness_))", "const int saved_thickness = image_button_paint_signed(bevel_thickness_);\n    while (layers < saved_thickness)"),
    "sprite_receiver_cached": ("BODY", "Sprite *const drawn_sprite = sprite_;", "Sprite *const drawn_sprite = centering_sprite;"),
    "scale_display_width": ("BODY", "drawn_sprite->source_width()", "drawn_sprite->width()"),
    "scale_pressed_extra": ("BODY", "image_button_paint_sub(area.right, thickness), area.left", "area.right, area.left"),
    "pressed_x": ("BODY", "x = image_button_paint_add(center_x, thickness);", "x = center_x;"),
    "pressed_y": ("BODY", "y = image_button_paint_add(center_y, thickness);", "y = center_y;"),
    "scale_equal": ("BODY", "if (numerator >= denominator)", "if (numerator > denominator)"),
    "scale_full_denominator": ("BODY", "denominator = 1;", "denominator = 2;"),
    "transparency_signed": ("BODY", "drawn_sprite->transparent_index(),", "static_cast<signed char>(drawn_sprite->transparent_index()),"),
    "text_pressed_offset": ("BODY", "area.left = image_button_paint_add(area.left, thickness);", "area.left = area.left;"),
    "text_height_offset": ("BODY", "area.bottom = image_button_paint_add(area.bottom, thickness);", "area.bottom = area.bottom;"),
    "text_count": ("BODY", "static_cast<int>(strlen(text))", "static_cast<int>(strlen(text)) + 1"),
}


@pytest.mark.parametrize("damage", [*DAMAGE, "missing_override", "pressed_cached_early", "text_cached_early", "text_thickness_cached"])
def test_painter_rejects_compiling_damage(tmp_path, damage):
    parts = actual()
    if damage == "missing_override":
        # Removing the redundant virtual keyword would still override Win;
        # rename both declaration and body to remove dispatch genuinely.
        parts["IMAGE_METHOD"] = parts["IMAGE_METHOD"].replace("on_redraw", "wrong_contents")
        parts["BODY"] = parts["BODY"].replace("ImageButton::on_redraw", "ImageButton::wrong_contents")
    elif damage in ("pressed_cached_early", "text_cached_early", "text_thickness_cached"):
        declaration, replacement = {
            "pressed_cached_early": ("const bool pressed = field_A9C_ != 0;", ""),
            "text_cached_early": ("LPSTR const text = name_;", ""),
            "text_thickness_cached": ("const int post_draw_thickness = image_button_paint_signed(bevel_thickness_);", ""),
        }[damage]
        body = parts["BODY"]
        if damage == "text_thickness_cached":
            needle = "// Fresh read after drawing, not the thickness used for the sprite.\n        const int thickness = image_button_paint_signed(bevel_thickness_);"
            assert body.count(needle) == 1
            body = body.replace(needle, "const int thickness = post_draw_thickness;")
        else:
            assert body.count(declaration) == 1
            body = body.replace(declaration, replacement)
        parts["BODY"] = body.replace("void ImageButton::on_redraw() {", "void ImageButton::on_redraw() {\n" + declaration)
    else:
        part, good, bad = DAMAGE[damage]
        assert parts[part].count(good) == 1, damage
        parts[part] = parts[part].replace(good, bad)
    result = run(tmp_path, parts)
    assert result.returncode == 97 and "assertion line" in result.stderr, result.stdout + result.stderr
