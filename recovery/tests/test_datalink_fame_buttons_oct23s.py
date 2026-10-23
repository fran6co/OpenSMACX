"""Actual-source controls for two small Datalink and FameWin button handlers."""
from __future__ import annotations

from pathlib import Path
import shutil
import subprocess

import pytest

from test_pact_ascent import definition


ROOT = Path(__file__).resolve().parents[2]
DATALINK_SOURCE = ROOT / "src/datalink.cpp"
FAME_SOURCE = ROOT / "src/famewin.cpp"


HARNESS = r"""
#include <cassert>
#include <cstring>
#include <string>
#include <vector>

static std::vector<std::string> events;
struct ListBox {
  int selected;
  ListBox() : selected(-1) {}
  void set_selected_id(int id) {
    selected = id;
    events.push_back("select");
  }
};
struct ReportWin {
  int last_button;
  int calls;
  ReportWin() : last_button(-1), calls(0) {}
  void on_button_clicked(int id) {
    last_button = id;
    ++calls;
    events.push_back("report");
  }
};
static ReportWin ReportWinGlobal;

struct Datalink {
  int history_[21];
  unsigned int field_2A34_;
  int field_29DC_;
  ListBox listBox_;
  int release_calls, repaint_calls, draw_calls;
  int set_cat_calls, set_cat_category, set_cat_index;
  bool mutate_history_in_set_cat;
  Datalink() : history_{}, field_2A34_(0), field_29DC_(0), release_calls(0),
      repaint_calls(0), draw_calls(0), set_cat_calls(0), set_cat_category(-1),
      set_cat_index(-1), mutate_history_in_set_cat(false) {}
  void release_modal() { ++release_calls; events.push_back("release"); }
  void set_cat(unsigned int category, int index) {
    ++set_cat_calls;
    set_cat_category = static_cast<int>(category);
    set_cat_index = index;
    events.push_back("set_cat");
    if (mutate_history_in_set_cat) {
      history_[field_2A34_] = 450099;
      field_29DC_ = 7;
    }
  }
  void repaint_slot() { ++repaint_calls; events.push_back("repaint"); }
  void draw_entry() { ++draw_calls; events.push_back("draw"); }
  void on_button_clicked(int button_id);
};

struct HallOfFameRecord { unsigned int present, award_flags; };
static HallOfFameRecord HallOfFameRecords[35];
struct BufferStub {
  int closes;
  BufferStub() : closes(0) {}
  void close() { ++closes; events.push_back("buffer_close"); }
};
struct GraphicWin {
  int graphic_closes, releases;
  GraphicWin() : graphic_closes(0), releases(0) {}
  void close() { ++graphic_closes; events.push_back("graphic_close"); }
};
struct ButtonGroup {
  int row, value, calls;
  ButtonGroup() : row(-1), value(-1), calls(0) {}
  void set(int next_row, int next_value) {
    row = next_row;
    value = next_value;
    ++calls;
    events.push_back("group");
  }
};
struct FameWin : GraphicWin {
  BufferStub buffer1_;
  ButtonGroup buttonGroup_;
  unsigned int selectedRecord_;
  int modal_releases, redraws;
  FameWin() : selectedRecord_(99), modal_releases(0), redraws(0) {}
  void release_modal() { ++modal_releases; events.push_back("modal_release"); }
  void redraw() { ++redraws; events.push_back("redraw"); }
  void on_button_clicked(int button_id);
};

/* @@DATALINK_HANDLER@@ */
/* @@FAME_HANDLER@@ */

static void reset() {
  events.clear();
  std::memset(HallOfFameRecords, 0, sizeof HallOfFameRecords);
  ReportWinGlobal.last_button = -1;
  ReportWinGlobal.calls = 0;
}

static void test_datalink() {
  reset();
  Datalink link;
  link.field_2A34_ = 1;
  link.on_button_clicked(0x10);
  assert(link.release_calls == 1 && link.set_cat_calls == 0);
  assert(events == std::vector<std::string>{"release"});

  reset();
  Datalink history;
  history.field_2A34_ = 3;
  history.field_29DC_ = 4;
  history.history_[2] = 20057;
  history.mutate_history_in_set_cat = true;
  history.on_button_clicked(0x10);
  assert(history.field_2A34_ == 2);
  assert(history.set_cat_calls == 1 && history.set_cat_category == 2 &&
         history.set_cat_index == 0);
  assert(history.history_[2] == 450099);
  assert(history.history_[0] == 57);
  assert(history.field_29DC_ == 7);
  assert(history.listBox_.selected == 70057);
  assert(history.repaint_calls == 1 && history.draw_calls == 1);
  assert(events == (std::vector<std::string>{
      "set_cat", "select", "repaint", "draw"}));

  reset();
  Datalink close;
  close.field_2A34_ = 8;
  close.on_button_clicked(0x11);
  assert(close.release_calls == 1 && close.set_cat_calls == 0);
  assert(events == std::vector<std::string>{"release"});

  reset();
  Datalink report;
  report.field_29DC_ = 0xe;
  report.on_button_clicked(0x1234);
  assert(ReportWinGlobal.calls == 1 && ReportWinGlobal.last_button == 0x1234);
  assert(events == std::vector<std::string>{"report"});
  report.field_29DC_ = 3;
  report.on_button_clicked(0x5678);
  assert(ReportWinGlobal.calls == 1 && events.size() == 1);
}

static void test_famewin() {
  reset();
  FameWin closing;
  closing.on_button_clicked(0x7B8);
  assert(closing.buffer1_.closes == 1 && closing.modal_releases == 1 &&
         closing.graphic_closes == 1 && closing.redraws == 0);
  assert(events == (std::vector<std::string>{
      "buffer_close", "modal_release", "graphic_close"}));

  for (int award = 0; award != 6; ++award) {
    reset();
    FameWin win;
    const unsigned int bit = 1u << award;
    HallOfFameRecords[2 * 5 + 3] = {3, 0x3f};
    HallOfFameRecords[4 * 5 + 1] = {1, 0x3f};
    HallOfFameRecords[0 * 5 + 2] = {2, 0x3f};
    assert(bit != 0);
    win.on_button_clicked(0x8AE + award);
    assert(win.buttonGroup_.calls == 1 && win.buttonGroup_.row == 3 &&
           win.buttonGroup_.value == 1);
    assert(win.selectedRecord_ == 3 && win.redraws == 1);
    assert(events == (std::vector<std::string>{"group", "redraw"}));
  }

  reset();
  FameWin outside;
  outside.on_button_clicked(0x8AD);
  outside.on_button_clicked(0x8B4);
  assert(outside.buttonGroup_.calls == 0 && outside.redraws == 0 && events.empty());

  reset();
  FameWin absent;
  HallOfFameRecords[0] = {2, 0x3f};
  absent.on_button_clicked(0x8AE);
  assert(absent.buttonGroup_.calls == 0 && absent.selectedRecord_ == 99 &&
         absent.redraws == 1);
  assert(events == std::vector<std::string>{"redraw"});
}

int main() {
  test_datalink();
  test_famewin();
  return 0;
}
"""


def source_bodies():
    datalink = definition(
        DATALINK_SOURCE.read_text(encoding="utf-8"),
        r"void\s+Datalink::on_button_clicked\s*\(\s*int\s+button_id\s*\)",
    )
    fame = definition(
        FAME_SOURCE.read_text(encoding="utf-8"),
        r"void\s+FameWin::on_button_clicked\s*\(\s*int\s+button_id\s*\)",
    )
    return datalink, fame


def replace_once(source: str, old: str, new: str) -> str:
    assert source.count(old) == 1, f"expected exactly one mutation anchor: {old}"
    return source.replace(old, new, 1)


def render(datalink: str, fame: str, mutation: str | None = None) -> str:
    if mutation == "reload_history_after_callback":
        old = (
            "        const int entry = history_[field_2A34_];\n"
            "        const int category = entry / 10000;\n"
            "        set_cat(static_cast<unsigned int>(category), 0);"
        )
        new = (
            "        const int category = history_[field_2A34_] / 10000;\n"
            "        set_cat(static_cast<unsigned int>(category), 0);\n"
            "        const int entry = history_[field_2A34_];"
        )
        datalink = replace_once(datalink, old, new)
    elif mutation == "wrong_selected_column":
        fame = replace_once(fame, "selectedRecord_ = column;", "selectedRecord_ = row;")
    elif mutation == "wrong_award_bit":
        fame = replace_once(
            fame,
            "(entry.award_flags & (1 << button_id))",
            "(entry.award_flags & (1 << (button_id + 1)))",
        )
    elif mutation is not None:
        raise AssertionError(f"unknown mutation: {mutation}")

    assert HARNESS.count("/* @@DATALINK_HANDLER@@ */") == 1
    assert HARNESS.count("/* @@FAME_HANDLER@@ */") == 1
    return HARNESS.replace("/* @@DATALINK_HANDLER@@ */", datalink, 1).replace(
        "/* @@FAME_HANDLER@@ */", fame, 1
    )


def compile_and_run(tmp_path: Path, compiler_name: str, mutation: str | None = None):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(f"{compiler_name} unavailable")
    datalink, fame = source_bodies()
    source = render(datalink, fame, mutation)
    cpp = tmp_path / f"datalink_fame_{compiler_name}.cpp"
    exe = tmp_path / f"datalink_fame_{compiler_name}"
    cpp.write_text(source, encoding="utf-8")
    built = subprocess.run(
        [compiler, "-std=c++17", "-Wall", "-Wextra", "-Werror", str(cpp), "-o", str(exe)],
        capture_output=True, text=True, timeout=30,
    )
    assert built.returncode == 0, built.stdout + built.stderr
    return subprocess.run([str(exe)], capture_output=True, text=True, timeout=10)


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
def test_actual_datalink_and_famewin_handlers(compiler_name, tmp_path):
    result = compile_and_run(tmp_path, compiler_name)
    assert result.returncode == 0, result.stdout + result.stderr


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
@pytest.mark.parametrize(
    "mutation", ("reload_history_after_callback", "wrong_selected_column", "wrong_award_bit")
)
def test_compiled_handler_mutants_fail(compiler_name, mutation, tmp_path):
    result = compile_and_run(tmp_path, compiler_name, mutation)
    assert result.returncode != 0, f"behavior control missed {mutation}"
