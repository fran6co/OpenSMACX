"""Execute the actual ListBox scroll body against pinned branch/effect cases."""
from __future__ import annotations

import shutil
import subprocess
from pathlib import Path

import pytest
from match_recovery.source import Mode, read_text


ROOT = next(parent for parent in Path(__file__).resolve().parents
            if (parent / "src/listbox.cpp").is_file())
SIGNATURE = "void ListBox::on_scrolling(int a, int b)"
ADDRESS = 0x60C5D0
SPAN = (0x60C5D0, 0x60C691)


def extract_body(source: str) -> str:
    rows = [row for row in read_text(source, ROOT / "src/listbox.cpp")
            if row.address == ADDRESS]
    assert len(rows) == 1 and rows[0].mode is Mode.BODY
    assert rows[0].image_spans == (SPAN,) and SIGNATURE in rows[0].region
    assert source.count(SIGNATURE) == 1
    start = source.index(SIGNATURE)
    opening = source.index("{", start + len(SIGNATURE))
    depth, state, escaped = 0, "code", False
    for index in range(opening, len(source)):
        ch = source[index]
        nxt = source[index + 1] if index + 1 < len(source) else ""
        if state == "line":
            if ch == "\n":
                state = "code"
        elif state == "block":
            if ch == "*" and nxt == "/":
                state = "code"
                continue
        elif state in ("string", "char"):
            if escaped:
                escaped = False
            elif ch == "\\":
                escaped = True
            elif (state == "string" and ch == '"') or (state == "char" and ch == "'"):
                state = "code"
        elif ch == "/" and nxt == "/":
            state = "line"
            continue
        elif ch == "/" and nxt == "*":
            state = "block"
            continue
        elif ch == '"':
            state = "string"
        elif ch == "'":
            state = "char"
        elif ch == "{":
            depth += 1
        elif ch == "}":
            depth -= 1
            if depth == 0:
                return source[start:index + 1]
    raise AssertionError("unterminated ListBox::on_scrolling definition")


HOST = r'''#include <cassert>
#include <cstdint>
#include <vector>
using uint32_t = std::uint32_t;
struct Win;
struct ListBox;
static Win *WinCallbackWindow;
static int ListBoxGlobalDDC;
struct Win {
  virtual ~Win() {}
};
struct GraphicWin : Win {
  Win *parent = nullptr;
  int redraw_count = 0, update_count = 0;
  std::vector<char> *events = nullptr;
  Win *parent_window() { return parent; }
  void redraw() {
    assert(WinCallbackWindow == parent);
    ++redraw_count;
    if (events) events->push_back('R');
  }
  void soft_update() {
    assert(WinCallbackWindow == parent);
    ++update_count;
    if (events) events->push_back('U');
  }
};
struct Dialog {
  int selected_position_ = 0;
  int row_count_ = 4;
};
struct ListBox : GraphicWin, Dialog {
  uint32_t field_18_ = 0;
  int set_calls = 0;
  int set_argument = 0, set_notify = 0;
  std::vector<int> drawn;
  std::vector<char> *events = nullptr;
  void (*on_set)(ListBox *, int, int) = nullptr;
  void (*on_draw)(ListBox *, int) = nullptr;
  void set_selected_pos(int position, int notify) {
    assert(WinCallbackWindow == parent);
    ++set_calls; set_argument = position; set_notify = notify;
    selected_position_ = position;
    if (events) events->push_back('S');
    if (on_set) on_set(this, position, notify);
  }
  void draw_item(int index) {
    assert(WinCallbackWindow == parent);
    drawn.push_back(index);
    if (events) events->push_back('D');
    if (on_draw) on_draw(this, index);
  }
  void on_scrolling(int a, int b);
};
#include "actual_listbox_scroll.inc"

static void set_selection_and_restore_scroll(ListBox *list, int position, int) {
  list->selected_position_ = position - 3;
  list->field_18_ = 10; // live callback store makes the scroll compare equal
}
static void mutate_selection_after_old_row(ListBox *list, int index) {
  if (index == 2) list->selected_position_ = 11;
}
static void reset(ListBox &list, std::vector<char> &events, Win &parent,
                  uint32_t scroll, int selected) {
  list.parent = &parent;
  list.events = &events;
  list.GraphicWin::events = &events;
  list.field_18_ = scroll;
  list.selected_position_ = selected;
  list.redraw_count = list.update_count = list.set_calls = 0;
  list.drawn.clear(); events.clear(); list.on_set = nullptr; list.on_draw = nullptr;
  list.row_count_ = 4;
}
int main() {
  Win parent;
  ListBox list;
  std::vector<char> events;
  reset(list, events, parent, 4, 2);

  // Changed scroll position: redraw only, irrespective of selection.
  list.on_scrolling(-1, 5);
  assert(list.field_18_ == 5 && list.redraw_count == 1 && list.update_count == 0);
  assert(list.drawn.empty() && events.size() == 1 && events[0] == 'R');

  // Unchanged position and unchanged selection: skip both rows, always soft-update.
  reset(list, events, parent, 5, 2);
  list.on_scrolling(42, 999);
  assert(list.field_18_ == 5 && list.redraw_count == 0 && list.update_count == 1);
  assert(list.drawn.empty() && events.size() == 1 && events[0] == 'U');

  // Unchanged position with changed selection: draw old and live new row, then update.
  reset(list, events, parent, 10, 2);
  ListBoxGlobalDDC = 1;
  list.on_scrolling(-1, 10);
  assert((list.drawn == std::vector<int>{2, 10}));
  assert((events == std::vector<char>{'S', 'D', 'D', 'U'}));
  ListBoxGlobalDDC = 0;

  // A draw callback may mutate selection; second row reads the live field anew.
  reset(list, events, parent, 10, 2);
  list.on_draw = mutate_selection_after_old_row;
  ListBoxGlobalDDC = 1;
  list.on_set = set_selection_and_restore_scroll;
  list.on_scrolling(-2, 3);
  assert((list.drawn == std::vector<int>{2, 11}));
  assert(list.update_count == 1 && events.front() == 'S' && events.back() == 'U');
  ListBoxGlobalDDC = 0;

  // Notification is live: setter sees parent publication and can change both
  // selection and scroll, which select the unchanged-position repaint path.
  reset(list, events, parent, 10, 2);
  ListBoxGlobalDDC = 1;
  list.on_set = set_selection_and_restore_scroll;
  list.on_scrolling(-2, 3); // 4 * 3 = 12 before callback
  assert(list.set_calls == 1 && list.set_argument == 12 && list.set_notify == 1);
  assert(list.field_18_ == 10 && (list.drawn == std::vector<int>{2, 9}));
  assert(list.update_count == 1 && events[0] == 'S' && events.back() == 'U');
  ListBoxGlobalDDC = 0;

  // Keep original unsigned 32-bit product and direct thumb-position behavior.
  reset(list, events, parent, 3, 4);
  list.on_scrolling(-2, 0x40000001); // 4 * 0x40000001 wraps to 4
  assert(list.field_18_ == 4 && list.redraw_count == 1 && list.drawn.empty());
  reset(list, events, parent, 3, 4);
  list.on_scrolling(-1, -1);
  assert(list.field_18_ == 0xffffffffu && list.redraw_count == 1);
}
'''


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
def test_actual_scroll_branch_effects_and_compiled_damage_controls(tmp_path, compiler_name):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(f"host {compiler_name} unavailable")

    body = extract_body((ROOT / "src/listbox.cpp").read_text(encoding="utf-8"))

    def build(directory: Path, candidate: str) -> Path:
        directory.mkdir()
        (directory / "actual_listbox_scroll.inc").write_text(candidate, encoding="utf-8")
        source = directory / "control.cpp"
        source.write_text(HOST, encoding="utf-8")
        binary = directory / "control"
        subprocess.run([compiler, "-std=c++11", "-Wall", "-Wextra", "-Werror",
                        "-I", str(directory), str(source), "-o", str(binary)],
                       check=True, capture_output=True, text=True, timeout=30)
        return binary

    good = build(tmp_path / "good", body)
    subprocess.run([str(good)], check=True, timeout=10)

    equality_mutant = body.replace(
        "static_cast<int>(front->field_18_) != old_position",
        "static_cast<int>(front->field_18_) == old_position", 1)
    assert equality_mutant != body
    binary = build(tmp_path / "wrong_equality", equality_mutant)
    result = subprocess.run([str(binary)], capture_output=True, text=True, timeout=10)
    assert result.returncode != 0, "wrong scroll equality branch survived witness"

    unconditional_mutant = body.replace(
        "            front->draw_item(dialog.selected_position_);\n        }\n        entered->soft_update();",
        "        }\n        front->draw_item(dialog.selected_position_);\n"
        "        entered->soft_update();", 1)
    assert unconditional_mutant != body
    binary = build(tmp_path / "unconditional_row", unconditional_mutant)
    result = subprocess.run([str(binary)], capture_output=True, text=True, timeout=10)
    assert result.returncode != 0, "unconditional current-row mutant survived witness"
