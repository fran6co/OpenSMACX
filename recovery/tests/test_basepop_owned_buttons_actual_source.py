"""Bounded host controls using the live BasePop owned-button cleanup.

These check source dispatch and callback order, not the original VC6 ABI or game.
"""
import pathlib
import re
import shutil
import subprocess
import tempfile


ROOT = pathlib.Path(__file__).parents[2]
SRC = ROOT / "src"


def actual_loop():
    source = (SRC / "basepop.cpp").read_text()
    match = re.search(
        r"    for \(i = 0; i < 8; i\+\+\) \{\n"
        r"        FlatButton \*item = item_buttons_\[i\];.*?\n"
        r"    \}",
        source,
        re.S,
    )
    assert match, "could not find the live BasePop::close eight-slot loop"
    return match.group(0)


def test_live_layout_and_lifetime_evidence():
    header = (SRC / "basepop.h").read_text()
    ctor = (SRC / "basepop.cpp").read_text()
    assert re.search(r"FlatButton\s*\*\s*item_buttons_\[8\]\s*;", header)
    assert "memset(item_buttons_, 0, sizeof(item_buttons_));" in ctor
    assert "item_buttons_[index] = button;" in ctor

    flat = (SRC / "flatbutton.h").read_text()
    base = (SRC / "basebutton.h").read_text()
    graphic = (SRC / "graphicwin.h").read_text()
    win = (SRC / "win.h").read_text()
    sound = (SRC / "autosound.h").read_text()
    assert re.search(r"class FlatButton\s*:\s*public BaseButton", flat)
    assert re.search(r"class BaseButton\s*:\s*public GraphicWin", base)
    assert re.search(r"class GraphicWin\s*:\s*public Win", graphic)
    assert re.search(r"class Win\s*:\s*public AutoSound", win)
    assert re.search(r"virtual\s*~AutoSound\s*\(", sound)


def harness(loop):
    return r'''#include <cassert>
struct Win { virtual ~Win() {} };
struct GraphicWin : Win {};
struct BaseButton : GraphicWin {};
struct FlatButton : BaseButton {
  static int destroyed;
  static int scalar_deletes;
  FlatButton **slots;
  int slot;
  int clear_later;
  FlatButton(FlatButton **s, int n, int later = -1)
      : slots(s), slot(n), clear_later(later) {}
  virtual ~FlatButton() {
    assert(slot < 0 || slots[slot] == this);
    ++destroyed;
    if (clear_later >= 0) slots[clear_later] = 0;
  }
  static void operator delete(void *p) {
    ++scalar_deletes;
    ::operator delete(p);
  }
};
int FlatButton::destroyed = 0;
int FlatButton::scalar_deletes = 0;
struct BasePop {
  FlatButton *item_buttons_[8];
  FlatButton *outside_slot;
  BasePop() : outside_slot(0) {
    for (int j = 0; j != 8; ++j) item_buttons_[j] = 0;
  }
  void close_items() {
    int i;
LOOP
  }
};
int main() {
  BasePop pop;
  for (int j = 0; j != 8; ++j) assert(pop.item_buttons_[j] == 0);
  pop.item_buttons_[0] = new FlatButton(pop.item_buttons_, 0, 1);
  FlatButton *orphan = new FlatButton(pop.item_buttons_, 1);
  pop.item_buttons_[1] = orphan;
  pop.item_buttons_[7] = new FlatButton(pop.item_buttons_, 7);
  pop.outside_slot = new FlatButton(pop.item_buttons_, -1);
  pop.close_items();
  assert(FlatButton::destroyed == 2);
  assert(FlatButton::scalar_deletes == 2);
  assert(pop.outside_slot != 0);
  for (int j = 0; j != 8; ++j) assert(pop.item_buttons_[j] == 0);
  // The callback intentionally removes the later owner slot. Preserve that
  // original effect, but release the test allocation after validating the loop.
  assert(orphan != 0);
  orphan->slot = -1;
  delete orphan;
  delete pop.outside_slot;
  pop.outside_slot = 0;
  assert(FlatButton::destroyed == 4 && FlatButton::scalar_deletes == 4);
}
'''.replace("LOOP", "\n".join("    " + line for line in loop.splitlines()))


def run_case(compiler, loop, expect_success):
    with tempfile.TemporaryDirectory(prefix="popup-owned-buttons-") as temp:
        temp = pathlib.Path(temp)
        source = temp / "control.cpp"
        binary = temp / "control"
        source.write_text(harness(loop))
        subprocess.run(
            [compiler, "-std=c++98", "-Wall", "-Wextra", "-pedantic", str(source), "-o", str(binary)],
            check=True,
        )
        result = subprocess.run([str(binary)], stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        assert (result.returncode == 0) == expect_success, result.stderr.decode(errors="replace")


def test_extracted_live_loop_and_mutants():
    compilers = [shutil.which(name) for name in ("g++", "clang++")]
    compilers = [compiler for compiler in compilers if compiler]
    assert compilers, "neither GCC nor Clang is available"
    loop = actual_loop()
    mutants = [
        loop.replace("            delete item;\n            item_buttons_[i] = 0;",
                     "            item_buttons_[i] = 0;\n            delete item;"),
        loop.replace("delete item;", "item->BaseButton::~BaseButton();\n            ::operator delete(item);"),
        ("        FlatButton *cached[8];\n"
         "        for (int j = 0; j != 8; ++j) cached[j] = item_buttons_[j];\n" +
         loop).replace("FlatButton *item = item_buttons_[i];",
                       "FlatButton *item = cached[i];", 1),
        loop.replace("i < 8", "i <= 8"),
    ]
    for mutant in mutants:
        assert mutant != loop, "a requested mutation did not apply to the extracted source"
    for compiler in compilers:
        run_case(compiler, loop, True)
        for mutant in mutants:
            run_case(compiler, mutant, False)

