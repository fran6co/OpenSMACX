"""Actual-source condition control, not whole picker/runtime or target ABI proof."""
from pathlib import Path
import re
import shutil
import subprocess

import pytest

ROOT = Path(__file__).resolve().parents[2]


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
def test_selected_chassis_drives_ability_label_branch(tmp_path, compiler_name):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip("host compiler unavailable")
    source = (ROOT / "src/designwin.cpp").read_text()
    body = source.split("void DesignWin::select_special(int which)", 1)[1]
    condition = re.search(
        r"if \(\((Ability\[ability\]\.flags & 0x400\) != 0\s*\|\| Chassis\[[^\]]+\]\.triad != 0)\) \{",
        body,
    )
    assert condition, "actual plain-ability branch not located"
    expression = "(" + condition[1]
    harness = r'''#include <cassert>
struct AbilityEntry { unsigned flags; };
struct ChassisEntry { unsigned char triad; };
AbilityEntry Ability[39] = {};
ChassisEntry Chassis[14] = {};
struct DesignWin {
    unsigned protoChassisType_;
    bool one_based(int ability, int faction) const {
        (void)faction;
        return EXPRESSION;
    }
};
int main() {
    DesignWin win = {3};
    Chassis[1].triad = 2;
    Chassis[3].triad = 0;
    assert(!win.one_based(4, 1));
    Chassis[1].triad = 0;
    Chassis[3].triad = 1;
    assert(win.one_based(4, 1));
    Chassis[3].triad = 0;
    Ability[4].flags = 0x400;
    assert(win.one_based(4, 1));
    Ability[4].flags = 0;
    win.protoChassisType_ = 1;
    assert(!win.one_based(4, 3));
}
'''.replace("EXPRESSION", expression)
    path = tmp_path / "designwin_condition.cpp"
    binary = tmp_path / "designwin_condition"
    path.write_text(harness)
    subprocess.run([compiler, "-std=c++11", str(path), "-o", str(binary)], check=True)
    subprocess.run([str(binary)], check=True)
