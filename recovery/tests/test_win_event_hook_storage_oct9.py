"""Actual-source controls for Win's callable event fields and Sound helper."""
from __future__ import annotations

import re
import shutil
import subprocess
from pathlib import Path

import pytest


ROOT = Path(__file__).resolve().parents[2]
CPP = (ROOT / "src/win.cpp").read_text()
HEADER = (ROOT / "src/win.h").read_text()
COMPILERS = [p for n in ("g++", "clang++") if (p := shutil.which(n))]


def body(signature: str, source: str = CPP) -> str:
    start = source.index(signature)
    opening = source.index("{", start)
    depth = 0
    for end in range(opening, len(source)):
        if source[end] == "{":
            depth += 1
        elif source[end] == "}":
            depth -= 1
            if depth == 0:
                return source[start:end + 1]
    raise AssertionError(f"unterminated {signature}")


DISPATCHERS = (
    "void Win::dispatch_28(", "void Win::dispatch_29(",
    "void Win::dispatch_19(", "void Win::dispatch_21(",
    "void Win::dispatch_22(", "void Win::dispatch_23(",
    "void Win::dispatch_24(",
)

HARNESS = r'''
#include <cassert>
#include <cstdint>
#include <string>
#include <vector>
#define __cdecl
#define __stdcall
typedef void (__cdecl *WinMouseEventCallback)(int,int);
typedef void (__cdecl *WinNullaryEventCallback)();
typedef int (__cdecl *WinKeyClickCallback)(int,int);
typedef int (__cdecl *WinKeyHookFn)(int);
struct Sound {
  std::vector<std::string> *trace;
  int active;
  Sound(std::vector<std::string> *t,int a):trace(t),active(a){}
  int is_playing(){trace->push_back("sound?");return active;}
  void play(){trace->push_back("sound!");}
};
std::vector<std::string> trace;
struct Win;
Win *WinCallbackWindow=nullptr;
static void hook(int x,int y){trace.push_back("hook:"+std::to_string(x)+":"+std::to_string(y));}
struct Win {
  uint32_t iFlags_=0, iSomeFlag_=0;
  WinNullaryEventCallback field_404_=nullptr;
  WinMouseEventCallback field_40C_=nullptr,field_410_=nullptr,field_414_=nullptr,
    field_418_=nullptr,field_41C_=nullptr,field_420_=nullptr,
    field_424_=nullptr,field_428_=nullptr;
  WinKeyClickCallback field_42C_=nullptr;
  WinKeyHookFn field_434_=nullptr;
  Sound *val_17_=nullptr,*val_26_=nullptr,*val_18_=nullptr,*val_27_=nullptr,
    *val_20_=nullptr,*val_22_=nullptr,*val_23_=nullptr,*val_25_=nullptr;
  uintptr_t flags_=0,val_10_=0,val_11_=0,val_12_=0,val_13_=0,val_24_=0;
  void dispatch_28(int,int,int); void dispatch_29(int,int,int);
  void dispatch_19(int,int,int); void dispatch_21(int,int,int);
  void dispatch_22(int,int,int); void dispatch_23(int,int,int);
  void dispatch_24(int,int,int);
  virtual void on_left_double_click(int x,int y){trace.push_back("left2:"+std::to_string(x)+":"+std::to_string(y));}
  virtual void on_right_double_click(int x,int y){trace.push_back("right2:"+std::to_string(x)+":"+std::to_string(y));}
  virtual void on_left_click(int x,int y){trace.push_back("left:"+std::to_string(x)+":"+std::to_string(y));}
  virtual void on_left_up(int x,int y){trace.push_back("leftup:"+std::to_string(x)+":"+std::to_string(y));}
  virtual void on_right_click(int x,int y){trace.push_back("right:"+std::to_string(x)+":"+std::to_string(y));}
  virtual void on_right_down(int x,int y){trace.push_back("rightdown:"+std::to_string(x)+":"+std::to_string(y));}
  virtual void on_right_up(int x,int y){trace.push_back("rightup:"+std::to_string(x)+":"+std::to_string(y));}
  virtual void vslot_38(int x,int y){trace.push_back("38:"+std::to_string(x)+":"+std::to_string(y));}
  virtual void vslot_39(int x,int y){trace.push_back("39:"+std::to_string(x)+":"+std::to_string(y));}
  virtual void vslot_32(int x,int y){trace.push_back("32:"+std::to_string(x)+":"+std::to_string(y));}
  virtual void on_nc_left_up(int x,int y){trace.push_back("34:"+std::to_string(x)+":"+std::to_string(y));}
  virtual void vslot_35(int x,int y){trace.push_back("35:"+std::to_string(x)+":"+std::to_string(y));}
  virtual void vslot_36(int x,int y){trace.push_back("36:"+std::to_string(x)+":"+std::to_string(y));}
  virtual void vslot_37(int x,int y){trace.push_back("37:"+std::to_string(x)+":"+std::to_string(y));}
};
void __stdcall sub_5f86a0(Sound *sound);
__DISPATCHERS__
__SOUND_HELPER__
int main(){
  static_assert(sizeof(WinMouseEventCallback)==sizeof(void*),"host callback representation");
  Win w; Sound active{&trace,1}; Sound idle{&trace,0};
  w.field_40C_=w.field_410_=w.field_414_=w.field_41C_=w.field_420_=
    w.field_424_=w.field_428_=&hook;
  w.val_17_=w.val_18_=w.val_20_=w.val_22_=w.val_23_=w.val_25_=&active;
  w.val_26_=w.val_27_=&active;
  w.flags_=reinterpret_cast<uintptr_t>(&active);
  w.val_10_=w.val_11_=w.val_12_=w.val_13_=w.val_24_=
    reinterpret_cast<uintptr_t>(&active);
  struct Case { void (Win::*call)(int,int,int); const char *self; } cases[] = {
    {&Win::dispatch_28,"left2:11:-7"},{&Win::dispatch_29,"right2:11:-7"},
    {&Win::dispatch_19,"left:11:-7"},{&Win::dispatch_21,"leftup:11:-7"},
    {&Win::dispatch_22,"right:11:-7"},{&Win::dispatch_23,"rightdown:11:-7"},
    {&Win::dispatch_24,"rightup:11:-7"}};
  for(const auto &c:cases){
    trace.clear(); (w.*(c.call))(11,-7,0);
    assert((trace==std::vector<std::string>{"hook:11:-7",c.self,"sound?"}));
    assert(WinCallbackWindow==&w);
    trace.clear(); WinCallbackWindow=nullptr; (w.*(c.call))(3,4,1);
    assert(trace.size()==2 && trace[1]=="sound?" && WinCallbackWindow==nullptr);
    trace.clear(); w.iFlags_=0x200000; (w.*(c.call))(1,2,0); assert(trace.empty());
    w.iFlags_=0;
  }
  trace.clear(); sub_5f86a0(nullptr); assert(trace.empty());
  sub_5f86a0(&active); assert((trace==std::vector<std::string>{"sound?"}));
  trace.clear(); sub_5f86a0(&idle); assert((trace==std::vector<std::string>{"sound?","sound!"}));
}
'''


def run_control(tmp_path: Path, compiler: str, dispatcher_bodies: tuple[str, ...]) -> subprocess.CompletedProcess[str]:
    source = HARNESS.replace("__DISPATCHERS__", "\n".join(dispatcher_bodies)).replace(
        "__SOUND_HELPER__", body("void __stdcall sub_5f86a0("))
    cpp = tmp_path / "win_hooks.cpp"
    binary = tmp_path / "win_hooks"
    cpp.write_text(source)
    built = subprocess.run([compiler, "-std=c++11", "-Wall", "-Wextra", "-Werror",
                           "-Wno-unused-variable", str(cpp), "-o", str(binary)],
                           capture_output=True, text=True)
    if built.returncode:
        return built
    return subprocess.run([str(binary)], capture_output=True, text=True)


def test_win_event_fields_are_typed_and_consumer_calls_are_direct():
    expected = {
        "field_404_": "WinNullaryEventCallback", "field_40C_": "WinMouseEventCallback",
        "field_410_": "WinMouseEventCallback", "field_414_": "WinMouseEventCallback",
        "field_418_": "WinMouseEventCallback", "field_41C_": "WinMouseEventCallback",
        "field_420_": "WinMouseEventCallback", "field_424_": "WinMouseEventCallback",
        "field_428_": "WinMouseEventCallback", "field_42C_": "WinKeyClickCallback",
        "field_434_": "WinKeyHookFn",
    }
    for member, alias in expected.items():
        assert re.search(rf"\b{alias}\s+{member}\s*;", HEADER)
    for sig in DISPATCHERS:
        text = body(sig)
        assert not re.search(r"reinterpret_cast<[^>]+>\s*\(\s*field_", text)
    assert "sub_5f86a0(owner->val_6_);" in CPP
    assert "void __stdcall sub_5f86a0(Sound *sound)" in CPP
    assert "reinterpret_cast<Sound *>(self)" not in body("void __stdcall sub_5f86a0(")


@pytest.mark.parametrize("compiler", COMPILERS or [None])
def test_actual_coordinate_dispatchers_and_sound_helper(compiler, tmp_path):
    if compiler is None:
        pytest.skip("host GCC or Clang required")
    bodies = tuple(body(signature) for signature in DISPATCHERS)
    result = run_control(tmp_path, compiler, bodies)
    assert result.returncode == 0, result.stdout + result.stderr


@pytest.mark.parametrize("compiler", COMPILERS or [None])
def test_actual_dispatch_control_rejects_missing_callback(compiler, tmp_path):
    if compiler is None:
        pytest.skip("host GCC or Clang required")
    bodies = [body(signature) for signature in DISPATCHERS]
    bodies[0] = bodies[0].replace("hook(a, b);", "/* dropped callback */")
    assert bodies[0] != body(DISPATCHERS[0])
    result = run_control(tmp_path, compiler, tuple(bodies))
    assert result.returncode != 0


KEY_HARNESS = r'''
#include <cassert>
#include <cstdint>
#include <string>
#include <vector>
#define __cdecl
typedef int (__cdecl *WinKeyClickCallback)(int,int);
typedef int (__cdecl *WinKeyHookFn)(int);
struct Win;
struct FocusNode { Win *focus_owner_=nullptr; };
struct FocusList { int count_=0; FocusNode *head_=nullptr,*current_=nullptr; };
std::vector<std::string> trace;
Win *WinCallbackWindow=nullptr;
static int click_hook(int key,int flags){trace.push_back("hook:"+std::to_string(key)+":"+std::to_string(flags));return 5;}
static int up_hook(int key){trace.push_back("up:"+std::to_string(key));return 6;}
struct Sound { int is_playing(){return 1;} void play(){} };
struct Win {
  uint32_t iFlags_=0,iSomeFlag_=0;
  WinKeyClickCallback field_42C_=nullptr;
  WinKeyHookFn field_434_=nullptr;
  FocusList list_;
  Sound *val_14_=nullptr,*val_16_=nullptr;
  int click_result=-2,up_result=-3;
  int key_click_event(int,int);
  int key_up_event(int);
  virtual void on_button_clicked(int id){trace.push_back("button:"+std::to_string(id));}
  virtual void pass_dialog_focus(){}
  virtual int on_key_click(int key,int flags){trace.push_back("self:"+std::to_string(key)+":"+std::to_string(flags));return click_result;}
  virtual int vslot_27(int key){trace.push_back("virtual:"+std::to_string(key));return up_result;}
};
__KEY_CLICK__
__KEY_UP__
int main(){
  Win w; w.field_42C_=&click_hook; w.field_434_=&up_hook;
  assert(w.key_click_event(0x42,0x31)==3);
  assert((trace==std::vector<std::string>{"hook:66:49","self:66:49"}));
  assert(WinCallbackWindow==&w);
  trace.clear(); WinCallbackWindow=nullptr; w.field_42C_=nullptr; w.click_result=9;
  assert(w.key_click_event(0x21,0x31)==9);
  assert((trace==std::vector<std::string>{"self:33:49"}));
  trace.clear(); w.iFlags_=0x200000; w.field_42C_=&click_hook;
  assert(w.key_click_event(1,2)==0 && trace.empty());
  w.iFlags_=0; trace.clear(); w.field_434_=&up_hook;
  assert(w.key_up_event(0x55)==3);
  assert((trace==std::vector<std::string>{"up:85","virtual:85"}));
  trace.clear(); w.field_434_=nullptr; w.up_result=4;
  assert(w.key_up_event(0x56)==4);
  assert((trace==std::vector<std::string>{"virtual:86"}));
  trace.clear(); w.iSomeFlag_=8; w.field_434_=&up_hook;
  assert(w.key_up_event(0x57)==0 && trace.empty());
}
'''


@pytest.mark.parametrize("compiler", COMPILERS or [None])
def test_actual_key_hook_returns_and_aggregation(compiler, tmp_path):
    if compiler is None:
        pytest.skip("host GCC or Clang required")
    source = KEY_HARNESS.replace("__KEY_CLICK__", body("int Win::key_click_event("))
    source = source.replace("__KEY_UP__", body("int Win::key_up_event("))
    cpp = tmp_path / "win_key_hooks.cpp"
    binary = tmp_path / "win_key_hooks"
    cpp.write_text(source)
    built = subprocess.run([compiler, "-std=c++11", "-Wall", "-Wextra", "-Werror",
                           "-Wno-unused-variable", str(cpp), "-o", str(binary)],
                           capture_output=True, text=True)
    assert built.returncode == 0, built.stderr
    ran = subprocess.run([str(binary)], capture_output=True, text=True)
    assert ran.returncode == 0, ran.stdout + ran.stderr


def test_actual_on_key_keeps_the_same_typed_key_hook_view():
    on_key = body("void Win::on_key(")
    assert "WinKeyHookFn cb = field_434_;" in on_key
    assert "cb(with_modifiers);" in on_key
