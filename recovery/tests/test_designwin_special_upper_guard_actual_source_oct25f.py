"""Host witnesses for DesignWin's original negative-only special-ID guard.

The enlarged Ability fixture makes the nonnegative-over-Max sample a defined
host access. It witnesses that the body does not reject it; it says nothing
about the game's adjacent memory or whether such an ID is normally reachable.
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
    (0x00435B30, "void DesignWin::draw_special_1(int redraw)", "protoAbilityID1_"),
    (0x00435DE0, "void DesignWin::draw_special_2(int redraw)", "protoAbilityID2_"),
)


def actual_body(address, signature, *, restore_upper_guard=False):
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
            body = text[start:end + 1]
            break
    else:
        raise AssertionError("unterminated actual DesignWin special draw body")
    if restore_upper_guard:
        old = "if (ability < 0) {"
        new = "if (ability < 0 || ability >= MaxAbilityNum) {"
        assert body.count(old) == 1
        body = body.replace(old, new, 1)
    return body


HARNESS = r'''#include <cassert>
#include <cstring>
typedef unsigned int uint32_t;
struct RECT { int left, top, right, bottom; };
static char empty_label[] = "empty";
static char caption_label[] = "caption";
static int empty_writes, name_writes, description_writes;
struct Font {
  int unk_1_;
  int line_height() const { return 10; }
  int pixel_height() const { return 9; }
};
struct RulesAbility { int name, description; };
struct Buffer {
  void set_clip(RECT *) {}
  void copy(Buffer *, RECT *) {}
  void set_text_color(int, int, int, int) {}
  void write_cent_l(Font *, char *text, RECT *, int) {
    if (text == empty_label) ++empty_writes;
    else if (text[0] == 'n') ++name_writes;
    else if (text[0] == 'd') ++description_writes;
  }
  int wrap_cent(Font *, char *text, RECT *) {
    if (text[0] == 'n') ++name_writes;
    else if (text[0] == 'd') ++description_writes;
    return 13;
  }
};
static const int MaxAbilityNum = 29;
enum { component_panel_special_one = 0, component_panel_special_two = 1 };
static RulesAbility Ability[MaxAbilityNum + 4];
struct StringStore {
  char name[8];
  char description[8];
  char *get(int id) { return id == 1 ? name : description; }
} StringTable;
static char *label_get(int id) { return id == 0x64 / 4 ? empty_label : caption_label; }
class DesignWin : public Buffer {
public:
  Buffer buffer_;
  Font font1_, font2_;
  RECT component_panel_rects_[2];
  int protoAbilityID1_, protoAbilityID2_;
  DesignWin() : protoAbilityID1_(-1), protoAbilityID2_(-1) {
    component_panel_rects_[0].left = component_panel_rects_[1].left = 0;
    component_panel_rects_[0].top = component_panel_rects_[1].top = 0;
    component_panel_rects_[0].right = component_panel_rects_[1].right = 20;
    component_panel_rects_[0].bottom = component_panel_rects_[1].bottom = 30;
    font1_.unk_1_ = font2_.unk_1_ = 0;
  }
  void draw_special_1(int redraw);
  void draw_special_2(int redraw);
};
static void reset_fixture() {
  empty_writes = name_writes = description_writes = 0;
  std::strcpy(StringTable.name, "name");
  std::strcpy(StringTable.description, "desc");
  for (int i = 0; i < MaxAbilityNum + 4; ++i) {
    Ability[i].name = 1;
    Ability[i].description = 2;
  }
}
/* ACTUAL_DRAW_BODY */
int main() {
  reset_fixture();
  DesignWin negative;
  if (DRAW_SECOND) negative.protoAbilityID2_ = -1;
  else negative.protoAbilityID1_ = -1;
  if (DRAW_SECOND) negative.draw_special_2(0);
  else negative.draw_special_1(0);
  assert(empty_writes == 1);
  assert(name_writes == 0);

  reset_fixture();
  DesignWin normal;
  if (DRAW_SECOND) normal.protoAbilityID2_ = 2;
  else normal.protoAbilityID1_ = 2;
  if (DRAW_SECOND) normal.draw_special_2(0);
  else normal.draw_special_1(0);
  assert(name_writes == 1);
  assert(description_writes == 1);

  // The array deliberately has four valid extra rows. This checks the
  // source-level branch only; no out-of-bounds or production-layout claim.
  reset_fixture();
  DesignWin extended;
  const int beyond_declared_max = MaxAbilityNum + 1;
  if (DRAW_SECOND) extended.protoAbilityID2_ = beyond_declared_max;
  else extended.protoAbilityID1_ = beyond_declared_max;
  if (DRAW_SECOND) extended.draw_special_2(0);
  else extended.draw_special_1(0);
  assert(name_writes == 1);
  assert(description_writes == 1);
  assert(empty_writes == 0);
}
'''


@pytest.mark.parametrize("address,signature,_member", CASES)
@pytest.mark.parametrize("compiler", ("g++", "clang++"))
def test_negative_only_guard_and_safe_extended_fixture(
        tmp_path, compiler, address, signature, _member):
    executable = shutil.which(compiler)
    if not executable:
        pytest.skip(compiler + " unavailable")
    second = "draw_special_2" in signature
    code = HARNESS.replace("/* ACTUAL_DRAW_BODY */",
                           actual_body(address, signature)).replace(
                               "DRAW_SECOND", "1" if second else "0")
    source = tmp_path / ("special-" + hex(address) + ".cpp")
    binary = tmp_path / ("special-" + hex(address))
    source.write_text(code)
    built = subprocess.run(
        [executable, "-std=c++03", "-O1", "-Wall", "-Wextra", "-Werror",
         "-Wno-unused-parameter", str(source), "-o", str(binary)],
        capture_output=True, text=True, timeout=60)
    assert built.returncode == 0, built.stdout + built.stderr
    run = subprocess.run([str(binary)], capture_output=True, text=True, timeout=30)
    assert run.returncode == 0, run.stdout + run.stderr


@pytest.mark.parametrize("address,signature,_member", CASES)
def test_reinstated_upper_guard_mutant_is_rejected(tmp_path, address, signature, _member):
    compiler = shutil.which("g++") or shutil.which("clang++")
    if not compiler:
        pytest.skip("no host C++ compiler")
    second = "draw_special_2" in signature
    code = HARNESS.replace(
        "/* ACTUAL_DRAW_BODY */",
        actual_body(address, signature, restore_upper_guard=True),
    ).replace("DRAW_SECOND", "1" if second else "0")
    source = tmp_path / ("upper-mutant-" + hex(address) + ".cpp")
    binary = tmp_path / ("upper-mutant-" + hex(address))
    source.write_text(code)
    built = subprocess.run(
        [compiler, "-std=c++03", "-O1", "-Wall", "-Wextra", "-Werror",
         "-Wno-unused-parameter", str(source), "-o", str(binary)],
        capture_output=True, text=True, timeout=60)
    assert built.returncode == 0, built.stdout + built.stderr
    run = subprocess.run([str(binary)], capture_output=True, text=True, timeout=30)
    assert run.returncode != 0, "upper-bound guard mutant survived"
