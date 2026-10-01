"""Actual-source ordering and owned rectangle controls for BattleWin::clear."""
from pathlib import Path
import shutil
import subprocess

import pytest

from test_monument_records_behavior import body


ROOT = Path(__file__).resolve().parents[2]


@pytest.mark.parametrize("damage", [False, True])
def test_battlewin_clear_shared_canvas_and_live_rectangles(tmp_path, damage):
    compiler = shutil.which("c++")
    if compiler is None:
        pytest.skip("host C++ compiler unavailable")
    implementation = body("src/battlewin.cpp", "void BattleWin::clear() {")
    if damage:
        implementation = implementation.replace("battle_rects_[2]", "battle_rects_[1]")
    harness = r'''
#include <cassert>
struct RECT { int left,top,right,bottom; };
struct BoxSpriteParams {} InterfaceBoxStyles[51];
struct BattleWin { RECT battle_rects_[4]; void clear(); };
BattleWin owner;
int count;
struct Buffer {
    int box_sprite(RECT *rect,BoxSpriteParams *style) {
        assert(count<4 && rect==&owner.battle_rects_[count]);
        assert(style==&InterfaceBoxStyles[0]);
        assert(rect->left==count+10);
        ++count;
        if(count==1) owner.battle_rects_[1].left=11;
        return 0;
    }
};
struct MainInterface : Buffer {
    void redraw_complete() { assert(count==4); ++count; }
    void soft_update() { assert(count==5); ++count; }
} MainInterfaceGlobal;
__BODY__
int main() {
    for(int i=0;i<4;++i) owner.battle_rects_[i].left=i+10;
    owner.battle_rects_[1].left=-1;
    owner.clear();
    assert(count==6);
}
'''
    source = tmp_path / "clear.cpp"
    executable = tmp_path / "clear"
    source.write_text(harness.replace("__BODY__", implementation))
    subprocess.run([compiler,"-std=c++98",str(source),"-o",str(executable)],
                   check=True,capture_output=True,text=True,timeout=30)
    result = subprocess.run([str(executable)],capture_output=True,text=True,timeout=10)
    assert (result.returncode==0) == (not damage)


def test_battle_rectangles_are_owned_without_changing_extent():
    header = (ROOT / "src/battlewin.h").read_text()
    assert "RECT battle_rects_[4];" in header
    assert "BattleWin battle_rects_ 0x30 0x00422C50" in header
    assert 'sizeof(BattleWin) == 0xA0' in header
    assert "field_30_" not in header
