"""Actual-source controls for Console's post-callback MapWin reloads.

It extracts the mapped function from the current checkout, verifies the native
source span through match_recovery, executes the real source blocks, then applies
causal stale-receiver mutations to those same blocks. The stubbed callbacks
replace a table slot to make receiver timing observable; they do not claim that
normal Win APIs mutate MapWinTable during gameplay.
"""
from pathlib import Path
import subprocess
import shutil

import pytest

from match_recovery.source import Mode, State, read_file
def _block(text, marker):
    start = text.index(marker)
    opening = text.find("{", start)
    assert opening >= 0
    depth, state, i = 0, "code", opening
    while i < len(text):
        c = text[i]
        nxt = text[i + 1] if i + 1 < len(text) else ""
        if state == "code":
            if c == "/" and nxt == "/":
                state, i = "line", i + 2
                continue
            if c == "/" and nxt == "*":
                state, i = "block", i + 2
                continue
            if c == '"':
                state = "string"
            elif c == "'":
                state = "char"
            elif c == "{":
                depth += 1
            elif c == "}":
                depth -= 1
                if depth == 0:
                    return text[start:i + 1]
        elif state == "line":
            if c == "\n": state = "code"
        elif state == "block":
            if c == "*" and nxt == "/":
                state, i = "code", i + 2
                continue
        else:
            if c == "\\":
                i += 2
                continue
            if (state == "string" and c == '"') or (state == "char" and c == "'"):
                state = "code"
        i += 1
    raise AssertionError("unbalanced source block: " + marker)
def _run(tmp_path, harness, label):
    compiler = shutil.which("g++")
    if not compiler:
        pytest.skip("host C++ compiler unavailable")
    cpp, exe = tmp_path / (label + ".cpp"), tmp_path / label
    cpp.write_text(harness)
    built = subprocess.run([compiler, "-std=c++98", "-Wall", "-Wextra", str(cpp), "-o", str(exe)],
                           text=True, capture_output=True)
    assert built.returncode == 0, built.stderr
    return subprocess.run([str(exe)], check=False).returncode
def _mode1_harness(block, key, expected_move_id):
    return r'''#include <assert.h>
#include <stdint.h>
#include <stddef.h>
struct MapWin;
MapWin *MapWinTable[8]; MapWin *replacement; int swap_get;
struct MapWin {
  int id, field_1DD84_, feedback_hover_y_; unsigned field_1DE0C_;
  int mouse_calls, move_calls, last_move_id;
  void get_mouse_pos(int *x, int *y);
  void on_mouse_move(int x, int y) { (void)x; (void)y; ++move_calls; last_move_id=id; }
};
void MapWin::get_mouse_pos(int *x, int *y) {
  (void)x; (void)y; ++mouse_calls;
  if (swap_get) { MapWinTable[0]=replacement; swap_get=0; }
}
struct Veh { unsigned state; unsigned char waypoint_count; int patrol_current_point, order; };
Veh Vehs[1];
struct ConsoleGlobalState { int cursor_active_; } ConsoleGlobal;
int MapWinSelectedSlot=0, selected_vehicle_=0, field_23D10_=0, field_1DD80_=0;
void run(int normalized_character, int a1) {
  const int map_cursor_mode=1, selected_map_slot=0, selected_vehicle_id=0;
  (void)field_23D10_; (void)field_1DD80_; (void)selected_vehicle_;
MODE1
native_done:
  return;
}
int main(){
  MapWin a={1,0,0,0,0,0,0}, b={2,0,0,0,0,0,0};
  replacement=&b; MapWinTable[0]=&a; ConsoleGlobal.cursor_active_=1;
  swap_get=1; run(KEY, KEY);
  assert(a.mouse_calls==1);
  if (KEY == 'j') assert(a.field_1DD84_==1);
  else assert(a.field_1DD84_==0);
  assert(b.move_calls==1 && b.last_move_id==EXPECTED);
  return 0;
}
'''.replace("MODE1", block).replace("KEY", str(key)).replace("EXPECTED", str(expected_move_id))
def _mode8_harness(block):
    return r'''#include <assert.h>
#include <stdint.h>
struct MapWin;
MapWin *MapWinTable[8]; MapWin *replacement; int swap_get;
struct MapWin { int id; unsigned field_1DE0C_; int mouse_calls,move_calls,last_move_id;
  void get_mouse_pos(int *x,int *y) { (void)x;(void)y; ++mouse_calls;
    if(swap_get){MapWinTable[0]=replacement;swap_get=0;} }
  void on_mouse_move(int x,int y){(void)x;(void)y;++move_calls;last_move_id=id;}
};
struct Veh { unsigned char waypoint_count; } Vehs[1];
struct ConsoleGlobalState { int cursor_active_; } ConsoleGlobal;
int field_23D10_=8,field_1DD80_=1,selected_vehicle_=0,normalized_character='g',a1='g';
void run(){
MODE8
native_done: return;
}
int main(){ MapWin a={1,0,0,0,0},b={2,0,0,0,0}; replacement=&b;
  MapWinTable[0]=&a; ConsoleGlobal.cursor_active_=1; swap_get=1; run();
  assert(Vehs[0].waypoint_count==1 && a.mouse_calls==1);
  assert(a.field_1DE0C_==0 && a.move_calls==0);
  assert(b.field_1DE0C_==0xffffffffu && b.move_calls==1 && b.last_move_id==2);
  return 0; }
'''.replace("MODE8", block)
def _cursor_harness(block):
    return r'''#include <assert.h>
#include <stddef.h>
struct MapWin;
MapWin *MapWinTable[8],*replacement; const size_t MapWinTableSlots=8;
struct MapWin { int id,view_active_,cursor_sprite_,cursor_resource_id_;
  unsigned field_1DD70_; int set_calls,draw_calls;
  void set_cursor(int n){assert(n==0x7f00);++set_calls;MapWinTable[0]=replacement;}
  void draw_map(int n){assert(n==1);++draw_calls;}
};
void do_non_input(){}
void run(){
CURSOR
}
int main(){ MapWin a={1,0,1,0,0,0,0},b={2,0,0,0,0x00400000u,0,0};
  replacement=&b; MapWinTable[0]=&a; run();
  assert(a.set_calls==1 && a.cursor_resource_id_==0x7f00 && a.cursor_sprite_==0);
  assert(a.field_1DD70_==0 && a.draw_calls==0);
  assert(b.field_1DD70_==0 && b.draw_calls==1);
  return 0; }
'''.replace("CURSOR", block)

ROOT = next(p for p in Path(__file__).resolve().parents if (p / "src/console.cpp").is_file())
SOURCE_CPP = ROOT / "src/console.cpp"


def _blocks():
    records = [r for r in read_file(SOURCE_CPP) if r.address == 0x5178C0]
    (record,) = records
    assert record.mode is Mode.BODY and record.state is State.IMPLEMENTED
    assert record.image_spans == ((0x5178C0, 0x51B66F), (0x65D4D0, 0x65D4E2))
    text = SOURCE_CPP.read_text()
    body = _block(text, "int Console::on_key_click(int a1, int a2)")
    return {
        "mode1": _block(body, "if (map_cursor_mode == 1 &&"),
        "mode8": _block(body, "if (field_23D10_ == 8 &&"),
        "cursor": _block(body, "for (size_t cursor_window_slot = 0;"),
    }


def _replace_once(text, before, after):
    assert text.count(before) == 1, f"expected one mutation anchor: {before!r}"
    return text.replace(before, after, 1)


def _replace_first(text, before, after):
    assert before in text, f"missing mutation anchor: {before!r}"
    return text.replace(before, after, 1)


def _stale_j(block):
    branch = _block(block, "if (normalized_character == 'j')")
    branch = _replace_once(
        branch,
        "MapWinTable[selected_map_slot]->get_mouse_pos(&mouse_x, &mouse_y);",
        "MapWin *const stale_mouse_map = MapWinTable[selected_map_slot];\n"
        "            stale_mouse_map->get_mouse_pos(&mouse_x, &mouse_y);",
    )
    branch = _replace_once(
        branch,
        "MapWin *const j_after_mouse_map = MapWinTable[selected_map_slot];",
        "MapWin *const j_after_mouse_map = stale_mouse_map;",
    )
    return _replace_first(block, _block(block, "if (normalized_character == 'j')"), branch)


def _stale_g(block):
    marker = "if (normalized_character == 'g' || a1 == 0x20)"
    branch = _block(block, marker)
    branch = _replace_once(
        branch,
        "MapWinTable[selected_map_slot]->get_mouse_pos(&mouse_x, &mouse_y);",
        "MapWin *const stale_mouse_map = MapWinTable[selected_map_slot];\n"
        "            stale_mouse_map->get_mouse_pos(&mouse_x, &mouse_y);",
    )
    branch = _replace_once(
        branch,
        "MapWin *const g_after_mouse_map = MapWinTable[selected_map_slot];",
        "MapWin *const g_after_mouse_map = stale_mouse_map;",
    )
    return _replace_first(block, _block(block, marker), branch)


def _stale_mode8(block):
    block = _replace_once(
        block,
        "MapWinTable[0]->get_mouse_pos(&mouse_x, &mouse_y);",
        "MapWin *const stale_mouse_map = MapWinTable[0];\n"
        "        stale_mouse_map->get_mouse_pos(&mouse_x, &mouse_y);",
    )
    return _replace_once(
        block,
        "MapWin *const mode8_after_mouse_map = MapWinTable[0];",
        "MapWin *const mode8_after_mouse_map = stale_mouse_map;",
    )


def _stale_cursor_flags(block):
    return _replace_once(
        block,
        "MapWin *const cursor_flags_window = MapWinTable[cursor_window_slot];",
        "MapWin *const cursor_flags_window = window;",
    )


def test_actual_source_routes_and_stale_receiver_controls(tmp_path):
    source = _blocks()

    # Actual extracted source passes the table-swap controls.
    assert _run(tmp_path, _mode1_harness(source["mode1"], "'j'", 2), "actual-j") == 0
    assert _run(tmp_path, _mode1_harness(source["mode1"], "'g'", 2), "actual-g") == 0
    assert _run(tmp_path, _mode8_harness(source["mode8"]), "actual-mode8") == 0
    assert _run(tmp_path, _cursor_harness(source["cursor"]), "actual-cursor") == 0

    # Each mutant moves only the post-callback receiver back to its old value.
    assert _run(tmp_path, _mode1_harness(_stale_j(source["mode1"]), "'j'", 2),
                     "stale-j") != 0
    assert _run(tmp_path, _mode1_harness(_stale_g(source["mode1"]), "'g'", 2),
                     "stale-g") != 0
    assert _run(tmp_path, _mode8_harness(_stale_mode8(source["mode8"])),
                     "stale-mode8") != 0
    assert _run(tmp_path, _cursor_harness(_stale_cursor_flags(source["cursor"])),
                     "stale-cursor") != 0
