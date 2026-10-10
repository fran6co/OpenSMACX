"""Actual-body base access controls, not original layout or game acceptance."""
from dataclasses import fields
from pathlib import Path
import shutil
import subprocess

import pytest

from match_recovery.source import read_text, write
from test_socialwin_typed_owner_actual_source import body

ROOT = Path(__file__).resolve().parents[2]
BASE = "44cd2ae617e231e846f539ed864f4e07d377d78c"
EDITS = {
    "src/councwin.cpp": (
        "if (!reinterpret_cast<Win *>(this)->is_visible())",
        "if (!Win::is_visible())",
    ),
    "src/pulldown.cpp": (
        "reinterpret_cast<Win *>(this)->Win::hide();", "Win::hide();",
    ),
}


def test_bounded_base_access_preserves_mappings_claims_and_other_source():
    for relative, (old, new) in EDITS.items():
        path = ROOT / relative
        current = path.read_text()
        before = subprocess.check_output(
            ["git", "show", BASE + ":" + relative], cwd=ROOT, text=True)
        assert before.count(old) == 1
        assert current == before.replace(old, new, 1)
        prior = {r.address: r for r in read_text(before, path)}
        rows = read_text(current, path)
        assert {r.address for r in rows} == set(prior)
        assert write(current, rows) == current
        for row in rows:
            for field in fields(row):
                if field.name not in {"region", "line", "origin"}:
                    assert getattr(row, field.name) == getattr(prior[row.address], field.name)
            assert [(x.kind, x.key, x.prose) for x in row.origin] == [
                (x.kind, x.key, x.prose) for x in prior[row.address].origin]


HARNESS = r'''
#include <cstdlib>
static int events[4], count;
static void check(bool value) { if (!value) std::abort(); }
// Deliberately place Win after another polymorphic base. This host projection
// checks genuine C++ base conversion, not the original VC6 object layout.
struct Prefix { virtual ~Prefix() {} int poison; Prefix():poison(123){} };
struct Win {
  virtual ~Win() {}
  int visible;
  Win():visible(0){}
  int is_visible() { return visible; }
  virtual void hide() { events[count++]=2; }
  void release_modal() { events[count++]=1; }
};
struct Buffer { int unused; };
struct GraphicWin : Prefix, Win, Buffer {};
struct SubInterface {
  int activations;
  SubInterface():activations(0){}
  void set_iface_mode() { ++activations; }
};
struct CouncWin : GraphicWin, SubInterface { void show(int visible); };
struct PullDown : GraphicWin { void hide() override; };
struct DerivedPullDown : PullDown {
  void hide() override { events[count++]=99; }
};
static void do_all_draws() { events[count++]=3; }
COUNCIL_BODY
PULLDOWN_BODY
int main() {
  CouncWin council;
  const int values[]={0,1,-1,123};
  for (int visible=0;visible<2;++visible) {
    for (int i=0;i<4;++i) {
      council.Win::visible=visible;
      council.activations=0;
      council.show(values[i]);
      check(council.activations==(visible?0:1));
    }
  }
  DerivedPullDown menu;
  menu.PullDown::hide();
  check(count==3 && events[0]==1 && events[1]==2 && events[2]==3);
}
'''


@pytest.mark.parametrize("compiler", ["g++", "clang++"])
@pytest.mark.parametrize("damage", [None, "visibility_argument", "virtual_dispatch", "order"])
def test_actual_window_base_calls(tmp_path, compiler, damage):
    executable = shutil.which(compiler)
    if executable is None:
        pytest.skip("host compiler unavailable")
    council = body((ROOT / "src/councwin.cpp").read_text(), "void CouncWin::show(")
    pulldown = body((ROOT / "src/pulldown.cpp").read_text(), "void PullDown::hide(")
    if damage == "visibility_argument":
        anchor = "if (!Win::is_visible())"
        assert anchor in council
        council = council.replace(anchor, "if (visible && !Win::is_visible())", 1)
    elif damage == "virtual_dispatch":
        assert "Win::hide();" in pulldown
        pulldown = pulldown.replace("Win::hide();", "static_cast<Win *>(this)->hide();", 1)
    elif damage == "order":
        assert "release_modal();\n    Win::hide();" in pulldown
        pulldown = pulldown.replace("release_modal();\n    Win::hide();",
                                    "Win::hide();\n    release_modal();", 1)
    source = tmp_path / "base-access.cpp"
    source.write_text(HARNESS.replace("COUNCIL_BODY", council).replace("PULLDOWN_BODY", pulldown))
    binary = tmp_path / "base-access"
    built = subprocess.run([executable, "-std=c++11", "-O2", str(source), "-o", str(binary)],
                           capture_output=True, text=True)
    assert built.returncode == 0, built.stderr
    result = subprocess.run([str(binary)], cwd=tmp_path, capture_output=True)
    assert (result.returncode == 0) == (damage is None)
