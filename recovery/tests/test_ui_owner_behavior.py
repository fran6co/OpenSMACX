"""Actual-source C++98 behavior and damage controls for eight UI owners.

Unlike token-only tests, this test extracts each function from its tracked
product owner, drops the extracted definition into an embedded host fixture,
then compiles and executes those exact definitions under GCC and Clang.
Missing product definitions fail explicitly. It never reads an
ignored proposal fragment or relies on a cached host executable.
"""

from __future__ import annotations

import re
import shutil
import subprocess
from pathlib import Path

import pytest


ROOT = Path(__file__).resolve().parents[2]


def _extract_definition(source: str, signature: str) -> str:
    match = re.search(signature, source, re.MULTILINE)
    assert match, "tracked product definition missing: " + signature
    opening = source.find("{", match.end())
    assert opening >= 0
    depth = 0
    state = "code"
    i = opening
    while i < len(source):
        char = source[i]
        nxt = source[i + 1] if i + 1 < len(source) else ""
        if state == "code":
            if char == "/" and nxt == "/":
                state = "line"
                i += 2
                continue
            if char == "/" and nxt == "*":
                state = "block"
                i += 2
                continue
            if char == '"':
                state = "string"
            elif char == "'":
                state = "char"
            elif char == "{":
                depth += 1
            elif char == "}":
                depth -= 1
                if depth == 0:
                    return source[match.start() : i + 1]
        elif state == "line":
            if char == "\n":
                state = "code"
        elif state == "block":
            if char == "*" and nxt == "/":
                state = "code"
                i += 2
                continue
        else:
            if char == "\\":
                i += 2
                continue
            if (state == "string" and char == '"') or (
                state == "char" and char == "'"
            ):
                state = "code"
        i += 1
    raise AssertionError("unterminated definition: " + signature)


SOURCE_SIGNATURES = {
    "setup_ctor": ("src/setupwin.cpp", r"^\s*SetupWin::SetupWin\s*\(\s*\)"),
    "setup_close": ("src/setupwin.cpp", r"^\s*void\s+SetupWin::close\s*\(\s*\)"),
    "setup_dtor": ("src/setupwin.cpp", r"^\s*SetupWin::~SetupWin\s*\(\s*\)"),
    "dialog_unk2": ("src/dialog.cpp", r"^\s*void\s+Dialog::UNK2\s*\("),
    "pass_focus": ("src/dialogs.cpp", r"^\s*void\s+Dialogs::pass_dialog_focus\s*\(\s*\)"),
    "scroll_create": ("src/dialogs.cpp", r"^\s*Scroll\s*\*\s*Dialogs::on_scroll_create\s*\(\s*\)"),
    "setup_key": ("src/setupwin.cpp", r"^\s*int\s+SetupWin::on_key_click\s*\("),
    "newtech_group": ("src/newtechwin.cpp", r"^\s*void\s+NewTechWin::on_group_clicked\s*\("),
    "flatbutton_move": ("src/flatbutton.cpp", r"^\s*void\s+FlatButton::on_mouse_move\s*\("),
}

# Pinned mapping identities remain in their source annotations/seed history;
# these labels make failures auditable without rewriting any claim text here.
MAPPED_ADDRESSES = {
    "setup_ctor": "0x4AD880",
    "setup_close": "0x4AD9C0",
    "setup_dtor": "0x4AE790",
    "dialog_unk2": "0x609BF0",
    "pass_focus": "0x613180",
    "scroll_create": "0x613220",
    "setup_key": "0x4AE3E0",
    "newtech_group": "0x483EB0",
    "flatbutton_move": "0x608500",
}

# The host fixtures intentionally model the original relationships.  These
# witnesses ensure they stay anchored to actual declarations/member ownership.
HEADER_WITNESSES = {
    "src/setupwin.h": (
        r"class\s+SetupWin\s*:\s*public\s+GraphicWin",
        r"Buffer\s*\*\s*overlay_buffers_\s*\[\s*3\s*\]",
        r"uint32_t\s+row_count_",
        r"uint32_t\s+selected_row_",
        r"StringList\s+field_FE0_",  # genuine embedded member, not FE0..100C words
        r"StringList\s*\*\s*item_texts_\s*;\s*//\s*0x1010",
    ),
    "src/dialogs.h": (
        r"class\s+Dialogs\s*:\s*public\s+ListBox\s*,\s*public\s+RadioButton",
        r"void\s+pass_dialog_focus\s*\(\s*\)\s*;",
        r"Scroll\s*\*\s*on_scroll_create\s*\(\s*\)\s*;",
    ),
    "src/dialog.h": (r"void\s+UNK2\s*\(\s*int\s+window\s*,\s*int\s+message",),
    "src/win.h": (
        r"bool\s+has_dialog_focus_flag\s*\(\s*\)\s*const",
        r"virtual\s+void\s+redraw\s*\(\s*\)",
        r"virtual\s+void\s+pass_dialog_focus\s*\(\s*\)",
    ),
    "src/win_callbacks.h": (
        r'extern\s+"C"\s+LRESULT\s+__stdcall\s+PrevWndFunc\s*\(\s*HWND(?:\s+\w+)?\s*,\s*UINT(?:\s+\w+)?\s*,\s*WPARAM(?:\s+\w+)?\s*,\s*LPARAM(?:\s+\w+)?\s*\)\s*;',
    ),
    "src/graphicwin.h": (r"class\s+GraphicWin\s*:\s*public\s+Win\s*,\s*public\s+Buffer",),
    "src/flatbutton.h": (r"class\s+FlatButton\s*:\s*public\s+BaseButton",),
    # rect1_ is deep into Buffer's protected section; keep the witness bounded
    # so it cannot accidentally match a later declaration/access section.
    "src/buffer.h": (r"protected\s*:[\s\S]{0,6000}RECT\s+rect1_",),
    "src/newtechwin.h": (
        r"class\s+NewTechWin\s*:\s*public\s+GraphicWin",
        r"uint32_t\s+field_B24_",
        r"StringBox\s+stringBox2_",
    ),
    "src/stringstruct.h": (
        r"class\s+StringList\s*:\s*public\s+StringStruct",
        r"class\s+StringStruct\s*:\s*public\s+virtual\s+StringAllocationBase",
    ),
}


@pytest.fixture(scope="module")
def bodies() -> dict[str, str]:
    texts = {}
    out = {}
    for name, (relative, signature) in SOURCE_SIGNATURES.items():
        if relative not in texts:
            texts[relative] = (ROOT / relative).read_text(encoding="utf-8")
        out[name] = _extract_definition(texts[relative], signature)
    return out


def test_host_relationships_match_tracked_class_and_lifetime_witnesses():
    for relative, patterns in HEADER_WITNESSES.items():
        header = (ROOT / relative).read_text(encoding="utf-8")
        for pattern in patterns:
            assert re.search(pattern, header), relative + " missing witness " + pattern


def test_dialog_keeps_integer_entry_but_calls_typed_prev_window_callback():
    dialog = (ROOT / "src/dialog.cpp").read_text(encoding="utf-8")
    assert not re.search(r'extern\s+"C"\s+int\s+__stdcall\s+PrevWndFunc\s*\(', dialog)
    assert '#include "win_callbacks.h"' in dialog
    win_header = (ROOT / "src/win.h").read_text(encoding="utf-8")
    assert "PrevWndFunc" not in win_header
    callback_users = {
        path.relative_to(ROOT).as_posix()
        for path in (ROOT / "src").rglob("*.cpp")
        if '#include "win_callbacks.h"' in path.read_text(encoding="utf-8")
    }
    assert callback_users == {"src/win.cpp", "src/dialog.cpp"}
    body = _extract_definition(
        dialog, r"^\s*void\s+Dialog::UNK2\s*\(\s*int\s+window\s*,\s*int\s+message\s*,\s*int\s+wparam\s*,\s*int\s+lparam\s*\)")
    assert "CallWindowProcA(&PrevWndFunc," in body
    assert "reinterpret_cast<WNDPROC>(&PrevWndFunc)" not in body


SETUP_DIALOG_HOST = r'''#include <cassert>
#include <stdint.h>
#include <vector>
namespace {
enum Event { graphic_ctor=1, buffer_ctor, spot_ctor, list_ctor, buffer_close,
             spot_shutdown, list_remove, buffer_dtor, list_dtor, spot_dtor,
             graphic_dtor, child_event, userdata_event, redraw_event,
             forward_event, allocation_ctor, stringstruct_ctor,
             stringstruct_cleanup, stringstruct_dtor, allocation_dtor };
std::vector<Event> events;
int count(Event e) { int n=0; for (std::size_t i=0;i<events.size();++i) if(events[i]==e) ++n; return n; }
int HandleMain=1, pending_owner=7, list_captured=0, WinScreenWidth=1024, WinScreenHeight=768;
struct GraphicWin;
GraphicWin *opening_slot = reinterpret_cast<GraphicWin *>(9);
GraphicWin *opening_window() { return opening_slot; }
struct Win {
  bool suppress; int redraws;
  Win() : suppress(false), redraws(0) {}
  bool has_dialog_focus_flag() const { return suppress; }
  void redraw() { ++redraws; events.push_back(redraw_event); }
};
struct GraphicWin { GraphicWin(){events.push_back(graphic_ctor);} ~GraphicWin(){events.push_back(graphic_dtor);} };
struct Buffer { Buffer(){events.push_back(buffer_ctor);} void close(){events.push_back(buffer_close);} ~Buffer(){events.push_back(buffer_dtor);} };
struct Spot { Spot(){events.push_back(spot_ctor);} void shutdown(){events.push_back(spot_shutdown);} ~Spot(){events.push_back(spot_dtor);} };
struct StringAllocationBase {
  int captured;
  StringAllocationBase() : captured(pending_owner) { list_captured=captured; pending_owner=0; events.push_back(allocation_ctor); }
  virtual ~StringAllocationBase(){pending_owner=captured; events.push_back(allocation_dtor);}
};
struct StringStruct : virtual StringAllocationBase {
  StringStruct(){events.push_back(stringstruct_ctor);}
  void remove_all(){events.push_back(list_remove);}
  virtual ~StringStruct(){events.push_back(stringstruct_cleanup); events.push_back(stringstruct_dtor);}
};
struct StringList : StringStruct {
  void *head_; int current_position_;
  StringList():head_(reinterpret_cast<void *>(1)),current_position_(0){events.push_back(list_ctor);}
  void remove_all(){head_=0; events.push_back(list_remove);}
  ~StringList(){remove_all(); events.push_back(list_dtor);}
};
struct SetupWin : GraphicWin {
  uint32_t overlay_buffers_[3]; int row_count_, selected_row_, field_A14_, field_A18_;
  GraphicWin *opening_window_snapshot_; uint32_t menu_popup_; Buffer buffer_; Spot spot_; StringList field_FE0_;
  SetupWin(); void close(); ~SetupWin();
};
typedef void *HWND; typedef intptr_t LONG_PTR; typedef unsigned int UINT;
typedef intptr_t LPARAM; typedef uintptr_t WPARAM; typedef intptr_t LRESULT;
#define CALLBACK
#define __stdcall
typedef LRESULT (CALLBACK *WNDPROC)(HWND, UINT, WPARAM, LPARAM);
const UINT WM_SETFOCUS=7, WM_KILLFOCUS=8; const int GWL_USERDATA=-21;
int prev_calls=0, child_calls=0, userdata_calls=0;
HWND forwarded_window=0; UINT forwarded_message=0;
WPARAM forwarded_wparam=0; LPARAM forwarded_lparam=0;
HWND child_argument=0; bool child_result=false; Win *userdata_owner=0;
extern "C" LRESULT __stdcall PrevWndFunc(HWND w, UINT m, WPARAM a, LPARAM l) { forwarded_window=w; forwarded_message=m; forwarded_wparam=a; forwarded_lparam=l; ++prev_calls; events.push_back(forward_event); return 0; }
bool IsChild(HWND, HWND child) { ++child_calls; child_argument=child; events.push_back(child_event); return child_result; }
LONG_PTR GetWindowLongA(HWND, int) { ++userdata_calls; events.push_back(userdata_event); return reinterpret_cast<LONG_PTR>(userdata_owner); }
LRESULT CallWindowProcA(WNDPROC p, HWND w, UINT m, WPARAM a, LPARAM l) { return p(w,m,a,l); }
struct Dialog { void UNK2(int window, int message, int wparam, int lparam); };
}
// INSERT_SETUP_CTOR
// INSERT_SETUP_CLOSE
// INSERT_SETUP_DTOR
// INSERT_DIALOG_UNK2
int main() {
  { SetupWin w; assert(w.overlay_buffers_[0]==0 && w.overlay_buffers_[1]==0 && w.overlay_buffers_[2]==0);
    assert(w.row_count_==0 && w.selected_row_==0 && w.field_A14_==1024 && w.field_A18_==768);
    assert(w.opening_window_snapshot_==opening_slot && w.menu_popup_==0 && list_captured==7 && pending_owner==0);
    GraphicWin *snapshot = opening_slot;
    opening_slot = reinterpret_cast<GraphicWin *>(10);
    assert(w.opening_window_snapshot_==snapshot); }
  assert(count(list_remove)==2); // explicit cleanup plus derived member cleanup
  assert(count(stringstruct_cleanup)==1 && pending_owner==7);
  assert(events[events.size()-1]==graphic_dtor && events[events.size()-2]==buffer_dtor);
  assert(events[events.size()-3]==spot_dtor && events[events.size()-4]==allocation_dtor);
  assert(events[events.size()-5]==stringstruct_dtor && events[events.size()-7]==list_dtor);
  assert(events[6]==buffer_close && events[7]==spot_shutdown && events[8]==list_remove);
  events.clear(); Win owner; Dialog dialog;
  userdata_owner=&owner; child_result=false; child_calls=userdata_calls=prev_calls=0;
  dialog.UNK2(0x100,WM_SETFOCUS,0x777,0x200);
  assert(child_calls==1 && child_argument==reinterpret_cast<HWND>(0x777) && userdata_calls==1 && owner.redraws==1 && prev_calls==1);
  assert(forwarded_window==reinterpret_cast<HWND>(0x100) && forwarded_message==WM_SETFOCUS && forwarded_wparam==0x777 && forwarded_lparam==0x200);
  owner.suppress=true; dialog.UNK2(0x100,WM_KILLFOCUS,0,0x201);
  assert(owner.redraws==1 && prev_calls==2);
  child_result=true; dialog.UNK2(0x100,WM_SETFOCUS,0,0x202);
  assert(userdata_calls==2 && prev_calls==3);
  userdata_owner=0; child_result=false; dialog.UNK2(0x100,WM_SETFOCUS,0,0);
  assert(userdata_calls==3 && prev_calls==3); // null owner is the native early return
  dialog.UNK2(0x100,0x400,0,0); assert(prev_calls==4 && child_calls==4);
}
'''


DIALOGS_HOST = r'''#include <cassert>
#include <stdint.h>
struct Scroll {};
struct Win { int focuses, slot8_calls; Scroll *slot8_result; Win():focuses(0),slot8_calls(0),slot8_result(0){}
  virtual ~Win(){} virtual Scroll *on_listbox_scroll_create(){++slot8_calls; return slot8_result;}
  virtual void pass_dialog_focus(){++focuses;} };
struct GraphicWin : Win {};
struct Dialog {};
struct ListBox : virtual GraphicWin, virtual Dialog {};
struct RadioButton : virtual GraphicWin, virtual Dialog {};
struct CheckBox : virtual GraphicWin, virtual Dialog {};
struct SpriteBox : virtual GraphicWin, virtual Dialog {};
struct EditGroup : virtual GraphicWin, virtual Dialog { int edit_focuses; EditGroup():edit_focuses(0){}
  void pass_dialog_focus(){++edit_focuses;} };
class Dialogs : public ListBox, public RadioButton, public CheckBox, public SpriteBox, public EditGroup {
 public: int kind_; uint32_t iFlags_; Win *win_parent_;
  Dialogs():kind_(0),iFlags_(0),win_parent_(0){}
  void pass_dialog_focus(); Scroll *on_scroll_create();
};
// INSERT_PASS_FOCUS
// INSERT_SCROLL_CREATE
int main(){ Dialogs d; Win parent; Scroll expected;
  d.kind_=1; d.pass_dialog_focus(); d.kind_=2; d.pass_dialog_focus();
  d.kind_=8; d.pass_dialog_focus(); d.kind_=16; d.pass_dialog_focus();
  assert(d.focuses==4 && d.EditGroup::edit_focuses==0);
  d.kind_=4; d.pass_dialog_focus(); assert(d.focuses==4 && d.EditGroup::edit_focuses==1);
  d.kind_=3; d.pass_dialog_focus(); d.kind_=0; d.pass_dialog_focus();
  assert(d.focuses==4 && d.EditGroup::edit_focuses==1);
  d.kind_=2; d.iFlags_=0x40; d.win_parent_=&parent; parent.slot8_result=&expected;
  assert(d.on_scroll_create()==&expected && parent.slot8_calls==1);
  d.iFlags_=0; assert(d.on_scroll_create()==0 && parent.slot8_calls==1);
  d.iFlags_=0x40; d.win_parent_=0; assert(d.on_scroll_create()==0 && parent.slot8_calls==1);
  d.win_parent_=&parent; d.kind_=1; assert(d.on_scroll_create()==0 && parent.slot8_calls==1);
}
'''


SETUP_KEY_HOST = r'''#include <cassert>
#include <stdint.h>
struct GraphicWin { int updates; GraphicWin():updates(0){} void soft_update(){++updates;} };
struct SetupWin : GraphicWin { uint32_t selected_row_, row_count_; int modal_releases, draws;
  SetupWin():selected_row_(0),row_count_(3),modal_releases(0),draws(0){}
  void release_modal(){++modal_releases;} void draw_item(int,int){++draws;}
  int on_key_click(int,int); };
// INSERT_SETUP_KEY
int main(){ SetupWin w;
  assert(w.on_key_click(0,0x26)==1 && w.selected_row_==2 && w.draws==2 && w.updates==1);
  assert(w.on_key_click(0,0x28)==1 && w.selected_row_==0 && w.draws==4 && w.updates==2);
  assert(w.on_key_click(0,0x1b)==1 && w.selected_row_==static_cast<uint32_t>(-1) && w.modal_releases==1);
  assert(w.on_key_click(0,0x0d)==1 && w.modal_releases==2);
  assert(w.on_key_click(0,0x20)==0 && w.draws==4 && w.updates==2);
}
'''


NEWTECH_HOST = r'''#include <cassert>
#include <cstdio>
#include <cstring>
char StringTemp[1032]; char BlurbFile[]="BLURBSX"; char *DatalinkBlurbFile=BlurbFile;
char *itoa_host(int value,char *buffer,int radix){assert(radix==10); std::sprintf(buffer,"%d",value); return buffer;}
#define _itoa itoa_host
struct StringList { char file[32],section[32]; int clears,loads,fixups;
  StringList():clears(0),loads(0),fixups(0){}
  void load(char *f,char *s,int clear,int){std::strcpy(file,f);std::strcpy(section,s);if(clear)++clears;++loads;}
};
struct StringBox { StringList list; int clear_calls,fixup_calls; StringBox():clear_calls(0),fixup_calls(0){}
  void clear(){++clear_calls;} StringList &strings(){return list;} void add_fixup(){++fixup_calls;} };
struct NewTechWin { unsigned field_B24_; StringBox stringBox2_; NewTechWin():field_B24_(42){}
  void on_group_clicked(int,int); };
// INSERT_NEWTECH_GROUP
int main(){NewTechWin w; w.on_group_clicked(0,1);
  assert(std::strcmp(StringTemp,"TECH42")==0 && std::strcmp(w.stringBox2_.list.file,"BLURBSX")==0);
  assert(std::strcmp(w.stringBox2_.list.section,"TECH42")==0 && w.stringBox2_.clear_calls==1 && w.stringBox2_.fixup_calls==1);
  w.on_group_clicked(0,2); assert(std::strcmp(w.stringBox2_.list.file,"techlongs")==0);
  assert(std::strcmp(w.stringBox2_.list.section,"TECH42")==0 && w.stringBox2_.clear_calls==2 && w.stringBox2_.fixup_calls==2);
  w.on_group_clicked(0,3); assert(w.stringBox2_.list.loads==2 && w.stringBox2_.fixup_calls==2);
}
'''


FLATBUTTON_HOST = r'''#include <cassert>
#include <stdint.h>
#define __cdecl
struct RECT { int left,top,right,bottom; };
struct Buffer { protected: RECT rect1_; };
struct Win { int parent_events,state,redraws; Win():parent_events(0),state(-1),redraws(0){}
  virtual ~Win(){} void on_button_passover(int,int s){++parent_events;state=s;} void redraw(){++redraws;}
  static int cleared; static void clear_bubble_text(){++cleared;} }; int Win::cleared=0;
struct GraphicWin:Win,Buffer{};
struct Time {int stops,inits,pulses;Time():stops(0),inits(0),pulses(0){}
  void stop(){++stops;} void init(void (__cdecl *)(int,void*),int,void*,unsigned,unsigned){++inits;} void pulse(){++pulses;} };
struct BaseButton:GraphicWin {void *bubble_text_;uint32_t field_A74_,field_A78_;Time time1_;Win *win_parent_;int base_moves;
  BaseButton():bubble_text_(0),field_A74_(0),field_A78_(7),win_parent_(0),base_moves(0){}
  void on_mouse_move(int,int){++base_moves;} };
struct FlatButton:BaseButton {void on_mouse_move(int,int);void set_rect(int l,int t,int r,int b){rect1_.left=l;rect1_.top=t;rect1_.right=r;rect1_.bottom=b;}};
Win *WinPointerOwner1=0,*WinPointerOwner2=0; void (__cdecl *WinButtonHoverHook)()=0;
void __cdecl base_button_timer_callback(int,void*){}
int in_box(int x,int y,const RECT *r){return x>=r->left&&x<r->right&&y>=r->top&&y<r->bottom;}
// INSERT_FLATBUTTON_MOVE
int main(){FlatButton b;Win p;b.set_rect(4,8,24,18);b.win_parent_=&p;b.bubble_text_=(void *)1;
  b.on_mouse_move(5,9);assert(b.field_A74_==1&&p.state==1&&p.parent_events==1&&b.redraws==1);
  assert(b.time1_.inits==1&&b.time1_.pulses==1);
  b.on_mouse_move(30,9);assert(b.field_A74_==0&&p.state==0&&p.parent_events==2&&b.redraws==2);
  assert(b.time1_.stops==1&&Win::cleared==1);
  WinPointerOwner1=&b;b.on_mouse_move(5,9);assert(b.base_moves==1&&b.field_A74_==0);
}
'''


FIXTURES = {
    "setup_dialog": (SETUP_DIALOG_HOST, {
        "setup_ctor": "// INSERT_SETUP_CTOR",
        "setup_close": "// INSERT_SETUP_CLOSE",
        "setup_dtor": "// INSERT_SETUP_DTOR",
        "dialog_unk2": "// INSERT_DIALOG_UNK2",
    }),
    "dialogs": (DIALOGS_HOST, {
        "pass_focus": "// INSERT_PASS_FOCUS",
        "scroll_create": "// INSERT_SCROLL_CREATE",
    }),
    "setup_key": (SETUP_KEY_HOST, {"setup_key": "// INSERT_SETUP_KEY"}),
    "newtech": (NEWTECH_HOST, {"newtech_group": "// INSERT_NEWTECH_GROUP"}),
    "flatbutton": (FLATBUTTON_HOST, {"flatbutton_move": "// INSERT_FLATBUTTON_MOVE"}),
}


def _fixture_source(name: str, bodies: dict[str, str], mutant=None) -> str:
    template, markers = FIXTURES[name]
    source = template
    for body_name, marker in markers.items():
        body = bodies[body_name]
        if mutant and mutant[0] == body_name:
            before, after = mutant[1], mutant[2]
            assert before in body, (body_name, before)
            body = body.replace(before, after, 1)
        assert marker in source
        source = source.replace(marker, body, 1)
    return source


def _compile_and_run(tmp_path: Path, compiler: str, source: str, label: str):
    executable = tmp_path / (label + "-" + Path(compiler).name)
    cpp = tmp_path / (label + ".cpp")
    cpp.write_text(source, encoding="utf-8")
    built = subprocess.run(
        [compiler, "-std=c++98", "-Wall", "-Wextra", "-pedantic", str(cpp), "-o", str(executable)],
        text=True, capture_output=True,
    )
    return built, executable


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
@pytest.mark.parametrize("fixture_name", tuple(FIXTURES))
def test_actual_tracked_bodies_compile_and_execute_cxx98(
    tmp_path, bodies, compiler_name, fixture_name
):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(compiler_name + " is not installed")
    source = _fixture_source(fixture_name, bodies)
    built, executable = _compile_and_run(tmp_path, compiler, source, fixture_name)
    assert built.returncode == 0, built.stdout + built.stderr
    ran = subprocess.run([str(executable)], text=True, capture_output=True)
    assert ran.returncode == 0, ran.stdout + ran.stderr


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
@pytest.mark.parametrize(
    "fixture_name,body_name,before,after",
    (
        ("setup_dialog", "setup_ctor", "row_count_ = 0;", "row_count_ = 1;"),
        ("setup_dialog", "setup_close", "field_FE0_.remove_all();", ";"),
        ("setup_dialog", "dialog_unk2", "owner->redraw();", ";"),
        ("setup_dialog", "dialog_unk2", "static_cast<LONG_PTR>(wparam)", "static_cast<LONG_PTR>(lparam)"),
        ("dialogs", "pass_focus", "GraphicWin::pass_dialog_focus();", ";"),
        ("dialogs", "scroll_create", "return win_parent_->on_listbox_scroll_create();", "return 0;"),
        ("setup_key", "setup_key", "release_modal();", ";"),
        ("newtech", "newtech_group", "stringBox2_.add_fixup();", ";"),
    ),
)
def test_behavior_assertions_reject_damaged_extracted_bodies(
    tmp_path, bodies, compiler_name, fixture_name, body_name, before, after
):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(compiler_name + " is not installed")
    source = _fixture_source(fixture_name, bodies, (body_name, before, after))
    built, executable = _compile_and_run(tmp_path, compiler, source, "mutant-" + body_name)
    assert built.returncode == 0, built.stdout + built.stderr
    ran = subprocess.run([str(executable)], text=True, capture_output=True)
    assert ran.returncode != 0, "host assertions accepted damaged body " + body_name


def test_old_flatbutton_cast_fails_protected_access_control(tmp_path, bodies):
    compiler = shutil.which("g++")
    if compiler is None:
        pytest.skip("g++ is not installed")
    body = bodies["flatbutton_move"]
    old = "this->Buffer::rect1_"
    assert old in body
    damaged = body.replace(old, "static_cast<Buffer *>(this)->rect1_", 1)
    source = _fixture_source("flatbutton", dict(bodies, flatbutton_move=damaged))
    cpp = tmp_path / "protected-mutant.cpp"
    cpp.write_text(source, encoding="utf-8")
    built = subprocess.run(
        [compiler, "-std=c++98", "-Wall", "-Wextra", "-pedantic", "-c", str(cpp), "-o", str(tmp_path / "protected-mutant.o")],
        text=True, capture_output=True,
    )
    assert built.returncode != 0
    assert "protected" in built.stderr
