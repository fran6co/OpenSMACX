"""Exercise BaseButton's existing nullable one-ID callback call views.

The methods are extracted unchanged. This proves source call order, argument
bits and guards with explicit services; it does not identify original callback
installers or infer their discarded return type.
"""
from pathlib import Path
import re
import shutil
import subprocess

import pytest


ROOT = Path(__file__).resolve().parents[2]


def body(source, signature):
    start = source.index(signature)
    opening = source.index('{', start)
    depth = 0
    for end in range(opening, len(source)):
        depth += (source[end] == '{') - (source[end] == '}')
        if depth == 0:
            return source[start:end + 1]
    raise AssertionError(f'unterminated {signature}')


BASEBUTTON = (ROOT / 'src/basebutton.cpp').read_text()
CLICK_SIG = 'void BaseButton::on_left_click(int, int) {'
DOWN_SIG = 'void BaseButton::on_left_down(int, int) {'
CLICK = body(BASEBUTTON, CLICK_SIG)
DOWN = body(BASEBUTTON, DOWN_SIG)


HARNESS = r'''
#include <cstdio>
#include <cstdlib>
#include <cstdint>
#include <string>
#include <vector>
#define __cdecl
typedef void (__cdecl *WinNotificationCallback)(int selected_id);
typedef uint32_t DWORD;
std::vector<std::string> events;
void check(bool ok, int line) { if (!ok) { std::fprintf(stderr,"line %d events:",line); for (const auto &event : events) std::fprintf(stderr," %s",event.c_str()); std::fprintf(stderr,"\n"); std::exit(71); } }
#define CHECK(v) check((v),__LINE__)
void log_id(const char *name, int id) { events.push_back(std::string(name)+":"+std::to_string(id)); }
struct Sound { int is_playing() { events.push_back("sound-check"); return 1; } void play(){events.push_back("sound-play");} };
struct Win {
  Sound *sound28=0, *sound30=0, *sound31=0;
  void clear_bubble_text(){events.push_back("clear-bubble");}
  Sound *legacy_sound_28(){events.push_back("sound28");return sound28;}
  Sound *legacy_sound_30(){events.push_back("sound30");return sound30;}
  void *legacy_value_31(){events.push_back("sound31");return sound31;}
  int vslot_41(int id){log_id("v41",id);return 1;}
  int vslot_43(int id){log_id("v43",id);return 1;}
  void on_button_clicked(int id){log_id("parent-click",id);}
};
struct ButtonGroup { int result=1; int button_click(int id){log_id("group-click",id);return result;} };
struct BaseButton;
void __cdecl base_button_timer_callback(int, void*);
BaseButton *WinPointerOwner3=0, *WinPointerOwner4=0;
Win *WinCallbackWindow=0;
void (*WinOkHook)()=0, (*WinCancelHook)()=0;
struct Time {
  void pulse(void (__cdecl *callback)(int,void*), int a, void* context, uint32_t c, int d) {
    CHECK(callback==base_button_timer_callback && a==1 && context!=0 && c==20 && d==5);
    events.push_back("pulse");
  }
};
uint32_t timeGetTime(){events.push_back("time");return 100;}
void Sleep(uint32_t) { events.push_back("sleep"); }
void hook_ok(){events.push_back("ok-hook");}
void hook_cancel(){events.push_back("cancel-hook");}
void callback_click(int id){log_id("callback-click",id);}
void callback_down(int id){log_id("callback-down",id);}
struct BaseButton : Win {
  uint32_t field_A9C_=0, field_A78_=0, field_A44_=0xffffffffU, field_A48_=0;
  uint32_t field_AA0_=0, field_AA4_=0, field_AAC_=0;
  Win *win_parent_=0; ButtonGroup *group_=0; Time time2_;
  WinNotificationCallback field_AB0_=0, field_AB4_=0;
  void on_lose_mouse_capture(){events.push_back("lose-capture");}
  void redraw(){events.push_back("redraw");}
  void on_left_click(int,int);
  void on_left_down(int,int);
};
void __cdecl base_button_timer_callback(int, void*) {}
// ACTUAL BODIES
int main(){
  Win parent; BaseButton b; b.win_parent_=&parent; b.field_A78_=static_cast<uint32_t>(-2);
  WinPointerOwner3=0; WinPointerOwner4=0; WinOkHook=hook_ok; WinCancelHook=hook_cancel;
  b.field_AAC_=0; b.group_=0; b.field_AB0_=callback_click;
  b.on_left_click(3,4);
  CHECK(events==std::vector<std::string>({"cancel-hook","sound31","parent-click:-2","callback-click:-2"}));

  events.clear(); ButtonGroup group; group.result=0; b.group_=&group; b.field_A78_=static_cast<uint32_t>(-1);
  b.on_left_click(0,0);
  CHECK(events==std::vector<std::string>({"ok-hook","sound31","group-click:-1","callback-click:-1"}));

  events.clear(); b.field_AAC_=1; b.field_AB0_=0; b.on_left_click(0,0);
  CHECK(events.empty()); // AAC suppresses parent path; null callback suppresses tail.
  b.field_AB0_=callback_click; b.on_left_click(0,0);
  CHECK(events==std::vector<std::string>({"callback-click:-1"}));

  events.clear(); WinPointerOwner3=&b; WinPointerOwner4=0; b.field_A9C_=1;
  b.field_AAC_=0; b.field_A78_=static_cast<uint32_t>(-23); b.field_A44_=20;
  b.field_AB4_=callback_down; parent.sound28=0;
  b.on_left_down(0,0);
  CHECK(events==std::vector<std::string>({"clear-bubble","time","sound28","v41:-23","pulse","callback-down:-23"}));

  events.clear(); b.field_AAC_=1; b.field_AB4_=callback_down; b.on_left_down(0,0);
  CHECK(events==std::vector<std::string>({"clear-bubble","time","callback-down:-23"}));

  events.clear(); b.field_A9C_=0; b.field_A44_=0xffffffffU; b.field_AB4_=callback_down; b.on_left_down(0,0);
  CHECK(events==std::vector<std::string>({"clear-bubble","sound30","v43:-23","redraw"}));
  return 0;
}
'''


def run(tmp_path, click=CLICK, down=DOWN):
    compiler = shutil.which('clang++') or shutil.which('g++')
    if not compiler:
        pytest.skip('host C++ compiler unavailable')
    # Host fixtures keep the borrowed owner pointer genuine at native width.
    code = HARNESS.replace('// ACTUAL BODIES', click + '\n' + down)
    cpp, exe = tmp_path / 'callbacks.cpp', tmp_path / 'callbacks'
    cpp.write_text(code)
    built = subprocess.run([compiler, '-std=c++11', '-O0', str(cpp), '-o', str(exe)],
                           capture_output=True, text=True, timeout=30)
    assert built.returncode == 0, built.stderr
    return subprocess.run([str(exe)], capture_output=True, text=True, timeout=10)


def test_actual_callable_slots_preserve_gates_bits_and_order(tmp_path):
    assert 'WinNotificationCallback field_AB0_' in (ROOT / 'src/basebutton.h').read_text()
    assert 'WinNotificationCallback field_AB4_' in (ROOT / 'src/basebutton.h').read_text()
    result = run(tmp_path)
    assert result.returncode == 0, result.stderr


@pytest.mark.parametrize('mutated', [
    (CLICK.replace('field_AB0_(static_cast<int>(field_A78_));', ''), DOWN),
    (CLICK.replace('field_AB0_(static_cast<int>(field_A78_));', 'field_AB0_(0);'), DOWN),
    (CLICK.replace('        field_AB0_(static_cast<int>(field_A78_));\n    }', '    }').replace(
        '    WinCallbackWindow = parent;',
        '    if (field_AB0_ != 0) field_AB0_(static_cast<int>(field_A78_));\n    WinCallbackWindow = parent;'), DOWN),
    (CLICK, DOWN.replace('field_AB4_(static_cast<int>(field_A78_));', '')),
])
def test_consumer_mutations_are_observed(tmp_path, mutated):
    click, down = mutated
    result = run(tmp_path, click, down)
    assert result.returncode != 0
