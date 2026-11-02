"""Source-derived focus comparison control, separate from target ABI proof.

The host table is deliberately larger than the game's 512-base array so the
conversion can be tested without a host out-of-bounds read. This does not make
those focus values valid in the game or establish original fault behavior.
"""
from pathlib import Path
import shutil
import subprocess

import pytest
from match_recovery.local_variants import _function_body_bytes


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
@pytest.mark.parametrize("narrow_focus", (False, True))
def test_product_focus_comparison(compiler_name, narrow_focus, tmp_path):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(f"{compiler_name} unavailable")
    root = Path(__file__).resolve().parents[2]
    source = (root / "src/basewin.cpp").read_bytes()
    lo, hi = _function_body_bytes(source.decode(), "BaseWin::check_veh")
    body = source[lo:hi].decode()
    if narrow_focus:
        anchor = "Vehs[veh_id].home_base_id == base_id"
        assert body.count(anchor) == 1
        body = body.replace(anchor, "Vehs[veh_id].home_base_id == static_cast<int16_t>(base_id)")
    program = r'''
#include <stdint.h>
#include <assert.h>
struct Veh { int16_t x, y, home_base_id; } Vehs[1];
struct Base { int16_t x, y; } Bases[65538];
int phase, computes, redraws;
void base_compute(int mode) {
  assert(mode==1 && phase==1); ++computes; phase=2;
}
struct BaseWin {
  uint32_t focusedBaseID_;
  bool visible;
  bool is_visible() { assert(phase==0); phase=1; return visible; }
  void redraw() { assert(phase==2); ++redraws; phase=3; }
  void check_veh(int);
};
void BaseWin::check_veh(int veh_id) BODY
void run(uint32_t focus, int16_t home, bool same_x, bool same_y,
         bool visible, bool expected) {
  BaseWin w; w.focusedBaseID_=focus; w.visible=visible;
  Bases[focus].x=20; Bases[focus].y=30;
  Vehs[0].x=same_x?20:21; Vehs[0].y=same_y?30:31;
  Vehs[0].home_base_id=home;
  phase=computes=redraws=0;
  w.check_veh(visible?0:-1);
  assert(computes==(expected?1:0) && redraws==computes);
  assert(phase==(expected?3:1));
}
int main() {
  run(1,1,false,false,true,true);
  run(1,2,true,true,true,true);
  run(1,2,true,false,true,false);
  run(1,2,false,true,true,false);
  run(1,1,true,true,false,false);
  run(32768,-32768,false,false,true,false);
  run(65535,-1,false,false,true,false);
  run(65537,1,false,false,true,false);
}
'''.replace("BODY", body)
    cpp = tmp_path / "focus.cpp"
    cpp.write_text(program)
    executable = tmp_path / "focus"
    built = subprocess.run([compiler, "-std=c++98", str(cpp), "-o", str(executable)],
                           capture_output=True, text=True, timeout=30)
    assert built.returncode == 0, built.stdout + built.stderr
    ran = subprocess.run([str(executable)], capture_output=True, text=True, timeout=30)
    assert (ran.returncode != 0) if narrow_focus else (ran.returncode == 0), ran.stdout + ran.stderr
