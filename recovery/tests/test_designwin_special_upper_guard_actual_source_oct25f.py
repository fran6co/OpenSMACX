"""Actual-source controls for the pinned special-ability drawing sequence.

The native bodies test only ability_id < 0. The extended fixture supplies
valid records above MaxAbilityNum so that this historical unchecked path can
be exercised without host out-of-bounds behavior.
"""
import os
from pathlib import Path
import shutil
import subprocess

import pytest

from match_recovery.source import read_file

_HERE = Path(__file__).resolve()
_DEFAULT_ROOT = next(parent for parent in _HERE.parents
                     if (parent / "src/designwin.cpp").is_file())
ROOT = Path(os.environ.get("OPENSMACX_SOURCE_ROOT", _DEFAULT_ROOT))
SOURCE = ROOT / "src/designwin.cpp"
CASES = (
    (0x00435B30, "void DesignWin::draw_special_1(int redraw)", "protoAbilityIDs_[0]"),
    (0x00435DE0, "void DesignWin::draw_special_2(int redraw)", "protoAbilityIDs_[1]"),
)


def actual_body(address, signature):
    rows = read_file(SOURCE)
    row, = [item for item in rows if item.address == address]
    assert Path(row.path).resolve() == SOURCE.resolve()
    assert row.region and signature in row.region
    text = SOURCE.read_text()
    start = text.index(signature)
    opening = text.index("{", start)
    depth = 0
    for end in range(opening, len(text)):
        depth += (text[end] == "{") - (text[end] == "}")
        if depth == 0:
            return text[start:end + 1]
    raise AssertionError("unterminated DesignWin special draw body")


HARNESS = r'''#include <cassert>
#include <cstring>
#include <string>
#include <vector>
struct RECT { int left, top, right, bottom; };
struct Font {
  int unk_1_, line_height_, height_;
  int line_height() const { return line_height_; }
  int pixel_height() const { return height_; }
};
struct RulesAbility { int name, description; };
static const int MaxAbilityNum = 29;
static RulesAbility Ability[MaxAbilityNum + 4];
static char StringTemp[2048];
static char stored_name[] = "name";
static char stored_description[] = "description";
static char empty_label[] = "none";
static char title_label[] = "ability";
struct Trace {
  std::vector<std::string> events;
  RECT title_rect;
  RECT body_rect;
  int width;
  int height;
} trace;
struct Strings {
  char *get(int id) { return id == 1 ? stored_name : stored_description; }
} StringTable;
static char *label_get(int id) {
  return id == 0x228 / 4 ? title_label : empty_label;
}
static char *CharUpperA(char *text) {
  for (char *p = text; *p; ++p)
    if (*p >= 'a' && *p <= 'z') *p = static_cast<char>(*p - 'a' + 'A');
  return text;
}
struct Buffer {
  static void set_clip(RECT *) { trace.events.push_back("clip"); }
  void copy(Buffer *, RECT *) { trace.events.push_back("copy"); }
  static void set_text_color(int, int, int, int) {
    trace.events.push_back("color");
  }
  static void set_font(Font *, int, int, int) {
    trace.events.push_back("font1");
  }
  static int wrap_cent(Font *, char *text, RECT *rect) {
    trace.events.push_back(std::string("title:") + text);
    trace.title_rect = *rect;
    return 0;
  }
  static int wrap_height(char *text, int width) {
    trace.events.push_back(std::string("height:") + text);
    trace.width = width;
    return trace.height;
  }
  static int wrap_cent(char *text, RECT *rect) {
    trace.events.push_back(std::string("wrap:") + text);
    trace.body_rect = *rect;
    return 0;
  }
  static void write_cent_l(char *text, RECT *rect, int length) {
    assert(length == static_cast<int>(std::strlen(text)));
    trace.events.push_back(std::string("write:") + text);
    trace.body_rect = *rect;
  }
};
enum { component_panel_special_one, component_panel_special_two };
struct DesignWin : Buffer {
  RECT component_panel_rects_[2];
  Font font1_, font2_;
  Buffer buffer_;
  int protoAbilityIDs_[2];
  DesignWin() {
    component_panel_rects_[0] = RECT{10, 20, 210, 100};
    component_panel_rects_[1] = RECT{30, 40, 230, 120};
    font1_ = Font{-1, 9, 10};
    font2_ = Font{-1, 12, 11};
    protoAbilityIDs_[0] = protoAbilityIDs_[1] = -1;
  }
  void draw_special_1(int redraw);
  void draw_special_2(int redraw);
};
/* ACTUAL_DRAW_BODY */
static void reset_fixture() {
  trace.events.clear();
  trace.width = trace.height = 0;
  for (int i = 0; i < MaxAbilityNum + 4; ++i)
    Ability[i] = RulesAbility{1, 2};
  StringTemp[0] = 0;
}
static int event(const char *wanted) {
  for (int i = 0; i < static_cast<int>(trace.events.size()); ++i)
    if (trace.events[i] == wanted) return i;
  return -1;
}
static void check_negative(DesignWin &win, bool second) {
  reset_fixture();
  if (second) win.draw_special_2(0); else win.draw_special_1(0);
  assert(event("clip") == 0 && event("copy") == 1);
  assert(trace.events[3] == (second ? "title:ABILITY 2" : "title:ABILITY 1"));
  assert(trace.title_rect.left == (second ? 35 : 15));
  assert(trace.title_rect.top == (second ? 42 : 22));
  assert(trace.title_rect.right == (second ? 225 : 205));
  assert(trace.title_rect.bottom == (second ? 52 : 32));
  assert(event("font1") > 2);
  assert(event("write:none") > event("font1"));
  assert(trace.body_rect.left == (second ? 30 : 10));
  assert(trace.body_rect.top == (second ? 40 : 20));
  assert(trace.body_rect.right == (second ? 230 : 210));
  assert(trace.body_rect.bottom == (second ? 120 : 100));
  int colors = 0;
  for (std::size_t i = 0; i < trace.events.size(); ++i)
    colors += trace.events[i] == "color";
  assert(colors == 2);
}
static void check_positive(DesignWin &win, bool second, int ability) {
  reset_fixture();
  if (second) win.protoAbilityIDs_[1] = ability;
  else win.protoAbilityIDs_[0] = ability;
  trace.height = 17;
  if (second) win.draw_special_2(0); else win.draw_special_1(0);
  const char *title = second ? "title:ABILITY 2" : "title:ABILITY 1";
  assert(event(title) >= 0);
  const char *body = "{name:  }description";
  const std::string height_event = std::string("height:") + body;
  const std::string wrap_event = std::string("wrap:") + body;
  assert(event(height_event.c_str()) > event("font1"));
  assert(event(wrap_event.c_str()) > event(height_event.c_str()));
  assert(trace.width == (second ? 200 : 200));
  assert(trace.body_rect.left == (second ? 30 : 10));
  assert(trace.body_rect.right == (second ? 230 : 210));
  assert(trace.body_rect.top == (second ? 103 : 83));
  assert(trace.body_rect.bottom == (second ? 120 : 100));
  int colors = 0;
  for (std::size_t i = 0; i < trace.events.size(); ++i)
    colors += trace.events[i] == "color";
  assert(colors == 3);
}
int main() {
  DesignWin first, second;
  check_negative(first, false);
  check_negative(second, true);
  check_positive(first, false, 2);
  check_positive(second, true, 2);
  // The host fixture deliberately includes these rows. This checks the
  // original negative-only branch without reading outside the fixture.
  check_positive(first, false, MaxAbilityNum + 1);
  check_positive(second, true, MaxAbilityNum + 1);
  return 0;
}
'''


def compile_and_run(compiler, tmp_path, bodies):
    code = HARNESS.replace("/* ACTUAL_DRAW_BODY */", bodies)
    source = tmp_path / "actual_special_draw.cpp"
    binary = tmp_path / "actual_special_draw"
    source.write_text(code, encoding="utf-8")
    built = subprocess.run(
        [compiler, "-std=c++11", "-O1", "-Wall", "-Wextra", "-Werror",
         "-Wno-unused-parameter", str(source), "-o", str(binary)],
        capture_output=True, text=True, timeout=60)
    assert built.returncode == 0, built.stdout + built.stderr
    return subprocess.run([str(binary)], capture_output=True, text=True,
                          timeout=30)


@pytest.mark.parametrize("address,signature,_member", CASES)
@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
def test_special_draw_preserves_negative_only_guard_and_mapped_render_flow(
        tmp_path, compiler_name, address, signature, _member):
    compiler = shutil.which(compiler_name)
    if not compiler:
        pytest.skip(compiler_name + " unavailable")
    bodies = "\n".join(actual_body(a, s) for a, s, _ in CASES)
    result = compile_and_run(compiler, tmp_path, bodies)
    assert result.returncode == 0, result.stdout + result.stderr


@pytest.mark.parametrize("address,signature,_member", CASES)
@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
def test_added_upper_guard_mutant_is_rejected_safely(
        tmp_path, compiler_name, address, signature, _member):
    compiler = shutil.which(compiler_name)
    if not compiler:
        pytest.skip(compiler_name + " unavailable")
    bodies = [actual_body(a, s) for a, s, _ in CASES]
    body_index = next(i for i, (a, _, _) in enumerate(CASES) if a == address)
    body = bodies[body_index]
    old = "if (ability < 0) {"
    new = "if (ability < 0 || ability >= MaxAbilityNum) {"
    assert body.count(old) == 1
    bodies[body_index] = body.replace(old, new, 1)
    result = compile_and_run(compiler, tmp_path, "\n".join(bodies))
    assert result.returncode != 0, "added upper-bound guard survived"


@pytest.mark.parametrize("address,signature,_member", CASES)
def test_missing_persistent_font_state_mutant_is_rejected(
        tmp_path, address, signature, _member):
    compiler = shutil.which("g++") or shutil.which("clang++")
    if not compiler:
        pytest.skip("no host C++ compiler")
    bodies = [actual_body(a, s) for a, s, _ in CASES]
    body_index = next(i for i, (a, _, _) in enumerate(CASES) if a == address)
    body = bodies[body_index]
    # The former override-based shape did not persist font1 into Buffer state.
    # Removing the persistent write must fail the call-order control.
    assert "Buffer::set_font(&font1_, 0, 0, 0);" in body
    bodies[body_index] = body.replace("Buffer::set_font(&font1_, 0, 0, 0);", "", 1)
    result = compile_and_run(compiler, tmp_path, "\n".join(bodies))
    assert result.returncode != 0, "persistent-font removal survived"
