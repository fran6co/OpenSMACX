"""Actual-source controls for SocialWin's signed hover and effect selectors."""
from pathlib import Path
import shutil
import subprocess

import pytest


ROOT = Path(__file__).resolve().parents[2]
SOURCE = ROOT / "src/socialwin.cpp"
HEADER = ROOT / "src/socialwin.h"


def body(text: str, signature: str) -> str:
    start = text.index(signature)
    opening = text.index("{", start)
    depth = 0
    for end in range(opening, len(text)):
        depth += (text[end] == "{") - (text[end] == "}")
        if depth == 0:
            return text[start:end + 1]
    raise AssertionError(f"unterminated body: {signature}")


COMPILERS = [p for p in (shutil.which("g++"), shutil.which("clang++")) if p]


HARNESS = r'''#include <cassert>
#include <vector>
struct Win {
  static int clears;
  static void clear_bubble_text() { ++clears; }
};
int Win::clears=0;
struct SocialWin;
SocialWin *active=0;
int all_draws=0, mouse_draws=0, overview_draws=0, unks=0, redraws=0;
std::vector<int> mouse_state, overview_state, maximize_state;
struct ButtonGroup { void set(int,int); };
struct ModelEffect { int values_[11]; };
struct Category { ModelEffect model_effect[4]; };
Category SocialCategories[4];
int available[4][4];
int society_avail(int c,int r,int) { return available[c][r]; }
struct SocialWin : Win {
  int hover_selection_=-1;
  int effect_hover_button_id_=-1;
  int effectMaximize_=-1;
  int factionID_=0;
  ButtonGroup groups_[4];
  ButtonGroup *buttonGroups_=groups_;
  void on_mouse_leave(int,int);
  void on_button_passover(int,int);
  void maximize_effect(int);
  void draw_mouseover() { ++mouse_draws; mouse_state.push_back(hover_selection_); }
  void draw_overview() { ++overview_draws; overview_state.push_back(effect_hover_button_id_); }
  void do_all_draws() { ++all_draws; }
  void UNK1(int,int) { ++unks; maximize_state.push_back(effectMaximize_); }
  void redraw() { ++redraws; maximize_state.push_back(effectMaximize_); }
};
void ButtonGroup::set(int,int) { maximize_state.push_back(active->effectMaximize_); }
MOUSE_LEAVE
BUTTON_PASSOVER
MAXIMIZE_EFFECT
int main() {
  SocialWin w; active=&w;
  for(int c=0;c<4;++c) for(int r=0;r<4;++r) {
    available[c][r]=1;
    for(int e=0;e<11;++e) SocialCategories[c].model_effect[r].values_[e]=r+1;
  }

  // Negative sentinels survive leave; ordinary selectors clear and redraw.
  w.hover_selection_=-2;
  w.on_mouse_leave(0,0);
  assert(w.hover_selection_==-2 && mouse_draws==0 && Win::clears==2);
  w.hover_selection_=-1;
  w.on_mouse_leave(0,0);
  assert(w.hover_selection_==-1 && mouse_draws==0 && Win::clears==4);
  w.hover_selection_=15;
  w.on_mouse_leave(0,0);
  assert(w.hover_selection_==-1 && mouse_draws==1 && mouse_state.back()==-1);
  assert(Win::clears==6);

  // Both ends of the raw 0x11..0x1B effect-button range are retained.
  w.on_button_passover(17,1);
  assert(w.effect_hover_button_id_==17 && overview_state.back()==17);
  w.on_button_passover(27,1);
  assert(w.effect_hover_button_id_==27 && overview_state.back()==27);
  int old_overview=overview_draws;
  w.on_button_passover(16,1);
  w.on_button_passover(28,1);
  assert(w.effect_hover_button_id_==27 && overview_draws==old_overview);
  w.on_button_passover(27,0);
  assert(w.effect_hover_button_id_==-1 && overview_state.back()==-1);

  // Category selectors retain signed index values and mouse redraw sees the
  // newly published state; faction-power is the distinct -2 selector.
  w.on_button_passover(0,1);
  assert(w.hover_selection_==0 && mouse_state.back()==0);
  w.on_button_passover(15,1);
  assert(w.hover_selection_==15 && mouse_state.back()==15);

  // The maximize member is live during each group/model application and redraw,
  // then returns to the signed no-selection sentinel.
  maximize_state.clear(); unks=0; redraws=0;
  w.maximize_effect(3);
  assert(unks==4 && redraws==1 && maximize_state.size()==9);
  for(int state : maximize_state) assert(state==3);
  assert(w.effectMaximize_==-1);
  return 0;
}
'''


def source_harness(*, unsigned_leave=False, short_effect_bound=False):
    text = SOURCE.read_text()
    leave = body(text, "void SocialWin::on_mouse_leave(int, int)")
    passover = body(text, "void SocialWin::on_button_passover(int a1, int a2)")
    maximize = body(text, "void SocialWin::maximize_effect(int effect_id)")
    if unsigned_leave:
        leave = leave.replace(
            "hover_selection_ >= 0",
            "static_cast<unsigned int>(hover_selection_) >= 0u",
            1,
        )
    if short_effect_bound:
        passover = passover.replace("a1 <= 0x1b", "a1 < 0x1b", 1)
    assert "void SocialWin::on_mouse_leave" in leave
    assert "void SocialWin::on_button_passover" in passover
    assert "void SocialWin::maximize_effect" in maximize
    return (HARNESS.replace("MOUSE_LEAVE", leave)
            .replace("BUTTON_PASSOVER", passover)
            .replace("MAXIMIZE_EFFECT", maximize))


def compile_run(tmp_path, compiler, text):
    source = tmp_path / "socialwin-signed-selectors.cpp"
    executable = tmp_path / "socialwin-signed-selectors"
    source.write_text(text)
    build = subprocess.run(
        [compiler, "-std=c++17", "-Wall", "-Wextra", "-Wno-unused-parameter",
         str(source), "-o", str(executable)],
        capture_output=True, text=True, timeout=30,
    )
    if build.returncode:
        return build, None
    return build, subprocess.run([str(executable)], capture_output=True,
                                 text=True, timeout=10)


def test_signed_selector_members_have_load_bearing_recovery_evidence():
    header = HEADER.read_text()
    assert "RECOVERY_MEMBER: v1 SocialWin hover_selection_ 0xA2C 0x004B1FCD" in header
    assert "int hover_selection_;  // 0xA2C" in header
    assert "RECOVERY_MEMBER: v1 SocialWin effect_hover_button_id_ 0xA30 0x004B041E" in header
    assert "int effect_hover_button_id_;  // 0xA30" in header
    assert "RECOVERY_MEMBER: v1 SocialWin effectMaximize_ 0x2C24 0x004AF11E" in header
    assert "int effectMaximize_;  // 0x2C24" in header


@pytest.mark.parametrize("compiler", COMPILERS)
def test_actual_signed_selector_callbacks_and_maximize_state(compiler, tmp_path):
    build, run = compile_run(tmp_path, compiler, source_harness())
    assert build.returncode == 0, build.stderr
    assert run.returncode == 0, run.stderr


@pytest.mark.parametrize("compiler", COMPILERS)
@pytest.mark.parametrize("mutant", ["unsigned_leave", "short_effect_bound"])
def test_actual_selector_damage_mutants_compile_then_fail(compiler, tmp_path, mutant):
    build, run = compile_run(
        tmp_path, compiler,
        source_harness(**{mutant: True}),
    )
    assert build.returncode == 0, build.stderr
    assert run.returncode != 0
