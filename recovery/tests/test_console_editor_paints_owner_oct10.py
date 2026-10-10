"""Actual-body controls for Console::editor_paints, not complete menu runtime coverage."""

from __future__ import annotations

import shutil
import subprocess
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
SOURCE = ROOT / "src" / "console.cpp"


def function_body(text: str) -> str:
    marker = "void Console::editor_paints(int paint_id)"
    start = text.index(marker)
    opening = text.index("{", start)
    depth = 0
    for offset in range(opening, len(text)):
        if text[offset] == "{":
            depth += 1
        elif text[offset] == "}":
            depth -= 1
            if depth == 0:
                return text[opening + 1 : offset]
    raise AssertionError("unterminated Console::editor_paints body")


def compile_run(tmp_path: Path, source: str) -> subprocess.CompletedProcess[str]:
    compiler = shutil.which("g++")
    if compiler is None:
        raise AssertionError("g++ is required for the actual-source host control")
    cpp = tmp_path / "control.cpp"
    exe = tmp_path / "control"
    cpp.parent.mkdir(parents=True, exist_ok=True)
    cpp.write_text(source)
    compiled = subprocess.run(
        [compiler, "-std=c++98", "-Wall", "-Wextra", "-Werror", str(cpp), "-o", str(exe)],
        text=True,
        capture_output=True,
        check=False,
        timeout=30,
    )
    assert compiled.returncode == 0, compiled.stdout + compiled.stderr
    return subprocess.run([str(exe)], text=True, capture_output=True, check=False, timeout=10)


HOST = r"""
#include <cassert>
#include <vector>
typedef unsigned int uint32_t;
static std::vector<int> events;
static std::vector<int> mask_reads;
static unsigned int paint_masks[20];
struct ButtonGroup {
  uint32_t field_84_;
  int calls, last_id, last_notify;
  ButtonGroup() : field_84_(0), calls(0), last_id(-1), last_notify(-1) {}
  int set(int id, int notify) {
    ++calls; last_id=id; last_notify=notify; field_84_=static_cast<uint32_t>(id);
    events.push_back(4); return id;
  }
};
struct MainInterface {
private:
  ButtonGroup button_group_[4];
public:
  ButtonGroup &button_group(int index) { return button_group_[index]; }
  void draw_edit_pane() { events.push_back(3); }
} MainInterfaceGlobal;
struct InfoWin {
  void set_redraw_phase(int phase) { assert(phase==7); events.push_back(1); }
  void on_redraw() { events.push_back(2); }
} ConsoleInfoWin;
struct Console {
  void editor_paints(int paint_id);
};
int MapEditorPaintId;
int map_editor_paint_mask(int id) {
  assert(id >= 6 && id <= 25);
  mask_reads.push_back(id);
  return static_cast<int>(paint_masks[id - 6]);
}
"""


def program(text: str, main: str) -> str:
    return HOST + "\nvoid Console::editor_paints(int paint_id) {" + function_body(text) + "}\n" + main


def test_source_uses_constructed_group_member_not_fixed_address_aliases() -> None:
    text = SOURCE.read_text()
    body = function_body(text)
    assert "MainInterfaceGlobal.button_group(2).field_84_" in body
    assert "MainInterfaceGlobal.button_group(2).set(result, 1)" in body
    assert "button_group_[" not in body
    assert "PaintCycleSelection" not in text
    assert "PaintCycleGroup" not in text
    assert "(ButtonGroup *)0x007CD1C0" not in text
    assert "(int *)0x007CD244" not in text


def test_actual_body_preserves_thresholds_mask_walk_and_conditional_set(tmp_path: Path) -> None:
    main = r"""
static void reset() {
  events.clear(); mask_reads.clear();
  for (int i=0;i!=20;++i) paint_masks[i]=1;
  for (int i=0;i!=4;++i) {
    MainInterfaceGlobal.button_group(i)=ButtonGroup();
    MainInterfaceGlobal.button_group(i).field_84_=static_cast<uint32_t>(0x500+i);
  }
}
int main() {
  Console console;
  reset();
  MainInterfaceGlobal.button_group(2).field_84_=0x3f7;
  console.editor_paints(4);
  assert(MapEditorPaintId==4 && events.size()==3);
  assert(events[0]==1 && events[1]==2 && events[2]==3);
  assert(mask_reads.empty() && MainInterfaceGlobal.button_group(2).calls==0);
  MainInterfaceGlobal.button_group(2).field_84_=0x500;
  events.clear(); console.editor_paints(4);
  assert(MainInterfaceGlobal.button_group(2).calls==1);
  assert(MainInterfaceGlobal.button_group(2).last_id==0x3f7);
  assert(MainInterfaceGlobal.button_group(2).last_notify==1);
  assert(events.size()==4 && events[3]==4);

  reset();
  MainInterfaceGlobal.button_group(2).field_84_=0x3f2;
  console.editor_paints(-1);
  assert(mask_reads.empty() && MainInterfaceGlobal.button_group(2).calls==0);

  reset();
  paint_masks[0]=0;
  console.editor_paints(6);
  assert(mask_reads.size()==20 && MainInterfaceGlobal.button_group(2).calls==1);
  assert(MainInterfaceGlobal.button_group(2).last_id==0x3f8);

  reset();
  for (int i=0;i!=20;++i) paint_masks[i]=0;
  console.editor_paints(25);
  assert(mask_reads.size()==20 && MainInterfaceGlobal.button_group(2).calls==1);
  assert(MainInterfaceGlobal.button_group(2).last_id==0x3f8);

  reset();
  paint_masks[10-6]=0; // limit 4, equal to the initial counter
  paint_masks[8-6]=0;  // limit 2, still no greater than the decremented counter
  paint_masks[12-6]=0; // limit 6, greater than the then-current counter
  MainInterfaceGlobal.button_group(2).field_84_=0x3fb;
  console.editor_paints(10);
  assert(mask_reads.size()==20);
  for (int i=0;i!=20;++i) assert(mask_reads[i]==25-i);
  assert(MainInterfaceGlobal.button_group(2).calls==0);
  assert(events.size()==3 && events[0]==1 && events[1]==2 && events[2]==3);
  MainInterfaceGlobal.button_group(2).field_84_=0;
  events.clear(); mask_reads.clear(); console.editor_paints(10);
  assert(mask_reads.size()==20 && MainInterfaceGlobal.button_group(2).calls==1);
  assert(MainInterfaceGlobal.button_group(2).last_id==0x3fb);
  assert(MainInterfaceGlobal.button_group(2).last_notify==1);
  assert(events.size()==4 && events[3]==4);

  reset();
  MainInterfaceGlobal.button_group(2).field_84_=0x40d;
  console.editor_paints(0x1a);
  assert(mask_reads.empty() && MainInterfaceGlobal.button_group(2).calls==0);
  assert(events.size()==3 && events[0]==1 && events[1]==2 && events[2]==3);
  return 0;
}
"""
    result = compile_run(tmp_path, program(SOURCE.read_text(), main))
    assert result.returncode == 0, result.stdout + result.stderr


def test_compiling_wrong_group_and_wrong_selection_mutants_fail_behavior(tmp_path: Path) -> None:
    text = SOURCE.read_text()
    body = function_body(text)
    main = r"""
int main() {
  Console console;
  MainInterfaceGlobal.button_group(1).field_84_=0x500;
  MainInterfaceGlobal.button_group(2).field_84_=0x3f7;
  MainInterfaceGlobal.button_group(3).field_84_=0x500;
  console.editor_paints(4);
  // Correct source performs no set when the selected paint is already active.
  assert(MainInterfaceGlobal.button_group(2).calls==0);
  assert(MainInterfaceGlobal.button_group(3).calls==0);
  return 0;
}
"""
    correct = compile_run(tmp_path / "correct", program(text, main))
    assert correct.returncode == 0, correct.stdout + correct.stderr

    wrong_group = body.replace("button_group(2)", "button_group(3)")
    assert wrong_group != body
    wrong_group_text = text.replace(body, wrong_group, 1)
    assert wrong_group_text != text
    result = compile_run(tmp_path / "wrong-group", program(wrong_group_text, main))
    assert result.returncode != 0

    wrong_selection = body.replace(
        "button_group(2).field_84_", "button_group(1).field_84_", 1
    )
    assert wrong_selection != body
    wrong_selection_text = text.replace(body, wrong_selection, 1)
    assert wrong_selection_text != text
    result = compile_run(tmp_path / "wrong-selection", program(wrong_selection_text, main))
    assert result.returncode != 0
