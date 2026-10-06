"""Actual resize bodies over a bounded projection of the real class hierarchy.

The inheritance clauses, relevant declarations, Dib and both complete handlers
come from source. Unrelated members and SDK APIs are omitted: this is not a
native layout, calling-convention or compiler-adjustor proof. Move/resize,
caption handling and the genuinely empty do_image_buttons are explicit recording
boundaries. Their callback mutations test caller reloads, not effects attributed
to the empty original handler. Valid post-move receivers remain nonnull, as the
original requires; dimensions stay in overflow-free signed arithmetic ranges.
"""

from pathlib import Path
import os
import re
import shutil
import subprocess

import pytest


ROOT = Path(__file__).resolve().parents[2]


def braced(source, signature):
    start = source.index(signature)
    opening = source.index("{", start)
    depth = 0
    for end in range(opening, len(source)):
        depth += (source[end] == "{") - (source[end] == "}")
        if not depth:
            return source[start:end + 1]
    raise AssertionError("unterminated source definition")


def actual_source():
    headers = {name: (ROOT / "src" / (name + ".h")).read_text()
               for name in ("win", "graphicwin", "mapwin", "planwin", "console", "scroll", "buffer")}
    parts = {}
    for unit, cls in (("win", "Win"), ("graphicwin", "GraphicWin"),
                      ("mapwin", "MapWin"), ("planwin", "PlanWin"),
                      ("console", "Console"), ("scroll", "Scroll")):
        declaration, = re.findall(r"^(?:class|struct) " + cls + r"\s*:[^{]+\{", headers[unit], re.M)
        parts[cls.upper() + "_BASES"] = declaration
    parts["DIB"] = braced(headers["buffer"], "struct Dib : BITMAPINFO") + ";"
    parts["DIB_MEMBER"], = re.findall(r"^  Dib dib_;", headers["buffer"], re.M)
    parts["CALLBACK_TYPE"], = re.findall(r"^typedef void \(__cdecl \*WinResizeCallback\).*;", headers["win"], re.M)
    parts["DEFAULT_HANDLER"] = braced(headers["win"], "virtual void on_resize(")
    parts["MAP_HANDLER_DECL"], = re.findall(r"^  virtual void on_resize\([^;]+;", headers["mapwin"], re.M)
    parts["MAP_FLAGS"], = re.findall(r"^  uint32_t field_1DD70_;", headers["mapwin"], re.M)
    for key, pattern in (("EVENT_DECL", r"^  void resize_event\([^;]+;"),
                         ("MOVE_DECL", r"^  int move\([^;]+;"),
                         ("CAPTION_DECL", r"^  void redo_caption_buttons\(\);"),
                         ("RESIZE_DECL", r"^  virtual int resize\([^;]+;")):
        parts[key], = re.findall(pattern, headers["win"], re.M)
    parts["GRAPHIC_RESIZE_DECL"], = re.findall(r"^  virtual int resize\([^;]+;", headers["graphicwin"], re.M)
    fields = []
    for name in ("iFlags_", "outer_rect_", "field_400_", "scroll_vert_", "scroll_horz_"):
        declaration, = re.findall(r"^  (?:uint32_t |RECT |WinResizeCallback |Scroll \*)" + name + r";", headers["win"], re.M)
        fields.append(declaration)
    parts["WIN_FIELDS"] = "\n".join(fields)
    parts["EVENT_BODY"] = braced((ROOT / "src/win.cpp").read_text(), "void Win::resize_event(")
    parts["MAP_BODY"] = braced((ROOT / "src/mapwin.cpp").read_text(), "void MapWin::on_resize(")
    return parts


HARNESS = r'''
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <type_traits>
#include <vector>
#define __cdecl
void check(bool value,int line){if(!value){std::fprintf(stderr,"assertion line %d\n",line);std::exit(95);}}
#define require(x) check((x),__LINE__)
// Win32 SDK widths, including SIGNED LONG dimensions, independent of host long.
struct RECT {int32_t left,top,right,bottom;};
struct RGBQUAD {uint8_t blue,green,red,reserved;};
struct BITMAPINFOHEADER {
    uint32_t biSize;int32_t biWidth,biHeight;uint16_t biPlanes,biBitCount;
    uint32_t biCompression,biSizeImage;int32_t biXPelsPerMeter,biYPelsPerMeter;
    uint32_t biClrUsed,biClrImportant;
};
struct BITMAPINFO {BITMAPINFOHEADER bmiHeader;RGBQUAD bmiColors[1];};
DIB
static_assert(sizeof(Dib)==0x428,"actual Dib with Win32 SDK widths");
struct AutoSound {virtual ~AutoSound(){}};
struct Buffer {DIB_MEMBER};
class Scroll;
CALLBACK_TYPE
WIN_BASES
public:
    WIN_FIELDS
    EVENT_DECL
    DEFAULT_HANDLER
    MOVE_DECL
    CAPTION_DECL
    RESIZE_DECL
    int id;
};
GRAPHICWIN_BASES public:GRAPHIC_RESIZE_DECL};
SCROLL_BASES public:};
MAPWIN_BASES public:MAP_HANDLER_DECL MAP_FLAGS void do_image_buttons();};
PLANWIN_BASES public:};
CONSOLE_BASES public:};
static_assert(std::is_same<decltype(&Win::resize_event),void(Win::*)(int,int)>::value,"resize event is genuinely void");
Win *WinCallbackWindow;
struct Event {char kind;int id,x,y;};std::vector<Event> events;
Win *parent;
Scroll bars[6];
int scenario;
void note(char kind,int id,int x=0,int y=0){events.push_back({kind,id,x,y});}
void callback_a(int width,int height);
void callback_b(int width,int height);
int Win::move(int x,int y){
    require(WinCallbackWindow==parent);note('M',id,x,y);
    if(scenario==1 && id==10){
        parent->scroll_vert_=&bars[2];parent->scroll_horz_=&bars[3];
        parent->outer_rect_={13,17,113,97};parent->iFlags_|=0x40;
        bars[2].dib_.bmiHeader.biWidth=-21;bars[3].dib_.bmiHeader.biHeight=-9;
        parent->field_400_=callback_b;
    }else if(scenario==1 && id==14){
        parent->scroll_vert_=&bars[1];parent->scroll_horz_=&bars[5];
        parent->outer_rect_={23,29,163,129};
        bars[1].dib_.bmiHeader.biWidth=-31;bars[5].dib_.bmiHeader.biHeight=-15;
    }else if(scenario==2 && id==10){parent->scroll_horz_=nullptr;
    }else if(scenario==3 && id==11){parent->scroll_vert_=nullptr;
    }else if(scenario==5 && id==10){parent->scroll_horz_=&bars[1];
    }else if(scenario==6 && id==11){
        parent->outer_rect_={-5,-7,95,73};bars[1].dib_.bmiHeader.biHeight=23;
    }
    return -81; // ignored, including negative failure-like result
}
int Win::resize(int width,int height,int repaint){note('W',id,width,height);return -82;}
int GraphicWin::resize(int width,int height,int repaint){
    require(WinCallbackWindow==parent && repaint==0);note('R',id,width,height);
    if(scenario==1 && id==12){
        parent->scroll_horz_=&bars[4];parent->outer_rect_={19,23,139,113};
        bars[4].dib_.bmiHeader.biHeight=33;
    }else if(scenario==4 && id==10){parent->scroll_horz_=nullptr;}
    return -83;
}
void Win::redo_caption_buttons(){note('C',id);}
void MapWin::do_image_buttons(){note('I',id);}
void callback_a(int width,int height){note('A',parent->id,width,height);}
void callback_b(int width,int height){note('B',parent->id,width,height);WinCallbackWindow=&bars[0];}
EVENT_BODY
MAP_BODY
struct Observe:Win {void on_resize(int width,int height) override {note('V',id,width,height);}};
Observe observed;
Win plain;
MapWin map;
PlanWin plan;
Console console;
void reset(Win &window){
    events.clear();scenario=0;parent=&window;window.id=1;
    window.iFlags_=0;window.outer_rect_={7,11,107,71};window.field_400_=callback_a;
    window.scroll_vert_=window.scroll_horz_=nullptr;WinCallbackWindow=nullptr;
    for(int i=0;i<6;++i){bars[i].id=10+i;bars[i].dib_={};bars[i].dib_.bmiHeader.biWidth=8+i;bars[i].dib_.bmiHeader.biHeight=12+i;}
}
void expect(size_t i,char kind,int id,int x=0,int y=0){require(i<events.size());
    const Event&e=events[i];require(e.kind==kind && e.id==id && e.x==x && e.y==y);}
void ordinary_and_signed(){
    reset(observed);observed.resize_event(301,-41);
    require(events.size()==3 && WinCallbackWindow==&observed);expect(0,'A',1,301,-41);expect(1,'V',1,301,-41);expect(2,'C',1);
    reset(plain);plain.field_400_=nullptr;plain.resize_event(6,7);require(events.size()==1);expect(0,'C',1);
    reset(observed);observed.scroll_vert_=&bars[0];observed.scroll_horz_=&bars[1];
    observed.resize_event(301,-41);require(events.size()==7);
    expect(0,'M',10,100,0);expect(1,'R',10,8,73);expect(2,'M',11,0,60);expect(3,'R',11,92,-13);
    expect(4,'A',1,301,-41);expect(5,'V',1,301,-41);expect(6,'C',1);
    reset(observed);observed.scroll_vert_=&bars[0];observed.resize_event(1,2);
    require(events.size()==5);expect(0,'M',10,100,0);expect(1,'R',10,8,60);
    reset(observed);observed.scroll_horz_=&bars[1];bars[1].dib_.bmiHeader.biHeight=-19;
    observed.resize_event(1,2);require(events.size()==5);expect(0,'M',11,0,60);expect(1,'R',11,100,19);
    reset(observed);observed.scroll_vert_=&bars[0];observed.scroll_horz_=&bars[1];observed.iFlags_=0x40;
    observed.resize_event(3,4);require(events.size()==3 && WinCallbackWindow==&observed);expect(0,'A',1,3,4);expect(1,'V',1,3,4);expect(2,'C',1);
}
void reload_controls(){
    reset(observed);scenario=1;observed.scroll_vert_=&bars[0];observed.scroll_horz_=&bars[1];
    observed.resize_event(307,-47);require(events.size()==7 && WinCallbackWindow==&bars[0]);
    expect(0,'M',10,100,0);expect(1,'R',12,-21,71);expect(2,'M',14,0,90);expect(3,'R',15,171,15);
    expect(4,'B',1,307,-47);expect(5,'V',1,307,-47);expect(6,'C',1);
    reset(observed);scenario=2;observed.scroll_vert_=&bars[0];observed.scroll_horz_=&bars[1];
    observed.resize_event(1,2);require(events.size()==5);expect(1,'R',10,8,60);
    reset(observed);scenario=3;observed.scroll_vert_=&bars[0];observed.scroll_horz_=&bars[1];
    observed.resize_event(1,2);require(events.size()==7);expect(3,'R',11,100,-13);
    reset(observed);scenario=4;observed.scroll_vert_=&bars[0];observed.scroll_horz_=&bars[1];
    observed.resize_event(1,2);require(events.size()==5);expect(1,'R',10,8,73);
    reset(observed);scenario=5;observed.scroll_vert_=&bars[0];
    observed.resize_event(1,2);require(events.size()==7);expect(1,'R',10,8,73);expect(2,'M',11,0,60);
    reset(observed);scenario=6;observed.scroll_horz_=&bars[1];
    observed.resize_event(1,2);require(events.size()==5);expect(0,'M',11,0,60);expect(1,'R',11,100,-23);
}
void set_high_bit(int width,int height){note('H',parent->id,width,height);map.field_1DD70_|=0x80000000u;}
void clear_high_bit(int width,int height){note('L',parent->id,width,height);map.field_1DD70_&=0x7fffffffu;}
void hierarchy_controls(){
    MapWin *owners[]={&map,&plan,&console};
    for(MapWin *owner:owners){
        Win *base=static_cast<GraphicWin*>(owner);reset(*base);owner->field_1DD70_=0x7fffffffu;
        base->resize_event(51,61);require(events.size()==3);expect(0,'A',1,51,61);expect(1,'I',1);expect(2,'C',1);
        reset(*base);owner->field_1DD70_=0x80000000u;base->resize_event(51,61);
        require(events.size()==2);expect(0,'A',1,51,61);expect(1,'C',1);
    }
    reset(map);map.field_1DD70_=0;map.field_400_=set_high_bit;map.resize_event(71,81);
    require(events.size()==2);expect(0,'H',1,71,81);expect(1,'C',1);
    reset(map);map.field_1DD70_=0x80000000u;map.field_400_=clear_high_bit;map.resize_event(71,81);
    require(events.size()==3);expect(0,'L',1,71,81);expect(1,'I',1);expect(2,'C',1);
}
int main(){ordinary_and_signed();reload_controls();hierarchy_controls();}
'''


def run_host(tmp_path, parts):
    compiler = shutil.which("clang++") or shutil.which("g++")
    if not compiler:
        pytest.skip("host C++ compiler required")
    source = tmp_path / "resize.cpp"
    binary = tmp_path / "resize"
    source.write_text(re.sub(r"\b(" + "|".join(parts) + r")\b", lambda match: parts[match.group()], HARNESS))
    compiled = subprocess.run([compiler, "-std=c++11", "-O1", "-fsanitize=address,undefined",
                               "-fno-sanitize-recover=all", str(source), "-o", str(binary)],
                              capture_output=True, text=True, timeout=30)
    assert compiled.returncode == 0, compiled.stderr
    return subprocess.run([str(binary)], capture_output=True, text=True, timeout=10,
                          env=dict(os.environ, ASAN_OPTIONS="detect_leaks=0"))


def test_actual_window_resize(tmp_path):
    result = run_host(tmp_path, actual_source())
    assert result.returncode == 0, result.stderr


# These change extracted implementation/declarations, not a surrogate algorithm.
# All damaged programs must compile before their behavior is rejected.
MUTANTS = [
    ("horizontal-move-receiver", [("EVENT_BODY", "scroll_horz_->move(0,", "move(0,")]),
    ("vertical-move-receiver", [("EVENT_BODY", "scroll_vert_->move(", "move(")]),
    ("callback-window-published", [("EVENT_BODY", "WinCallbackWindow = this;", "")]),
    ("callback-window-not-restored", [("EVENT_BODY", "redo_caption_buttons();", "redo_caption_buttons(); WinCallbackWindow = this;")]),
    ("flag-tested-once", [("EVENT_BODY", "if (scroll_horz_ != 0)", "if ((iFlags_ & 0x40) == 0 && scroll_horz_ != 0)")]),
    ("flag-suppresses-scrollbars", [("EVENT_BODY", "if ((iFlags_ & 0x40) == 0)", "if (true)")]),
    ("vertical-width-not-height", [("EVENT_BODY", "vertical->resize(vertical->dib_.bmiHeader.biWidth,", "vertical->resize(vertical->dib_.bmiHeader.biHeight,")]),
    ("vertical-adds-horizontal-height", [("EVENT_BODY", "horizontal->dib_.bmiHeader.biHeight -\n                                outer_rect_.top", "0 -\n                                outer_rect_.top")]),
    ("horizontal-subtracts-vertical-width", [("EVENT_BODY", "outer_rect_.right - vertical->dib_.bmiHeader.biWidth", "outer_rect_.right + vertical->dib_.bmiHeader.biWidth")]),
    ("horizontal-height-negated", [("EVENT_BODY", "-horizontal->dib_.bmiHeader.biHeight", "horizontal->dib_.bmiHeader.biHeight")]),
    ("vertical-move-zero-y", [("EVENT_BODY", "scroll_vert_->move(outer_rect_.right - outer_rect_.left, 0)", "scroll_vert_->move(outer_rect_.right - outer_rect_.left, outer_rect_.top)")]),
    ("horizontal-move-height", [("EVENT_BODY", "scroll_horz_->move(0, outer_rect_.bottom - outer_rect_.top)", "scroll_horz_->move(0, outer_rect_.bottom)")]),
    ("resize-repaint-zero", [("EVENT_BODY", "biWidth, resize_height, 0)", "biWidth, resize_height, 1)")]),
    ("vertical-resize-virtual", [("EVENT_BODY", "vertical->resize(", "vertical->Win::resize(")]),
    ("horizontal-resize-virtual", [("EVENT_BODY", "horizontal->resize(", "horizontal->Win::resize(")]),
    ("reload-after-vertical-move", [("EVENT_BODY",
        "scroll_vert_->move(outer_rect_.right - outer_rect_.left, 0);\n            Scroll *horizontal = scroll_horz_;\n            Scroll *vertical = scroll_vert_;",
        "Scroll *horizontal = scroll_horz_;\n            Scroll *vertical = scroll_vert_;\n            scroll_vert_->move(outer_rect_.right - outer_rect_.left, 0);")]),
    ("reload-after-horizontal-move", [("EVENT_BODY",
        "scroll_horz_->move(0, outer_rect_.bottom - outer_rect_.top);\n            Scroll *vertical = scroll_vert_;\n            Scroll *horizontal = scroll_horz_;",
        "Scroll *vertical = scroll_vert_;\n            Scroll *horizontal = scroll_horz_;\n            scroll_horz_->move(0, outer_rect_.bottom - outer_rect_.top);")]),
    ("geometry-after-callbacks", [("EVENT_BODY", "WinCallbackWindow = this;", "WinCallbackWindow = this; const RECT saved = outer_rect_;"),
                                   ("EVENT_BODY", "outer_rect_.", "saved.")]),
    ("live-resize-callback", [("EVENT_BODY", "WinCallbackWindow = this;", "WinCallbackWindow = this; WinResizeCallback saved_callback = field_400_;"),
                               ("EVENT_BODY", "if (field_400_ != 0)", "if (saved_callback != 0)"),
                               ("EVENT_BODY", "field_400_(width, height);", "saved_callback(width, height);")]),
    ("callback-before-virtual", [("EVENT_BODY", "    if (field_400_ != 0) {", "    on_resize(width, height);\n    if (field_400_ != 0) {"),
                                  ("EVENT_BODY", "    on_resize(width, height);\n    redo_caption_buttons();", "    redo_caption_buttons();")]),
    ("caption-last", [("EVENT_BODY", "    on_resize(width, height);\n    redo_caption_buttons();", "    redo_caption_buttons();\n    on_resize(width, height);")]),
    ("resize-virtual-dispatch", [("EVENT_BODY", "on_resize(width, height);", "Win::on_resize(width, height);")]),
    ("original-event-dimensions", [("EVENT_BODY", "field_400_(width, height);", "field_400_(outer_rect_.right, outer_rect_.bottom);")]),
    ("original-virtual-dimensions", [("EVENT_BODY", "on_resize(width, height);", "on_resize(height, width);")]),
    ("map-override", [("MAP_HANDLER_DECL", "on_resize", "unused_resize"), ("MAP_BODY", "on_resize", "unused_resize")]),
    ("map-high-bit", [("MAP_BODY", "0x80000000", "0x40")]),
    ("map-high-bit-polarity", [("MAP_BODY", "== 0", "!= 0")]),
    ("map-image-buttons", [("MAP_BODY", "do_image_buttons();", "")]),
]


@pytest.mark.parametrize("name,changes", MUTANTS, ids=[case[0] for case in MUTANTS])
def test_compiling_window_resize_damage(tmp_path, name, changes):
    parts = actual_source()
    for key, before, after in changes:
        assert before in parts[key], (name, "missing mutation anchor")
        parts[key] = parts[key].replace(before, after)
    result = run_host(tmp_path, parts)
    assert result.returncode != 0, (name, "behavior damage escaped controls")
