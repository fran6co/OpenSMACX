"""Actual named slot declaration and complete Win dispatcher control.

This declaration projection tests virtual callback routing, not native slot
indices, 32-bit layout, non-null legacy hook fields or whole GUI behavior.
Native compiler/table checks separately establish the physical slot.
"""
from pathlib import Path
import os
import re
import shutil
import subprocess

import pytest

from test_window_dispatch import braced

ROOT = Path(__file__).resolve().parents[2]


@pytest.mark.parametrize("compiler_name", ["g++", "clang++"])
@pytest.mark.parametrize("damage", [False, True])
def test_actual_left_click_virtual_dispatch(tmp_path, compiler_name, damage):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(f"{compiler_name} unavailable")
    declaration = braced((ROOT / "src/win.h").read_text(),
                         "virtual void on_left_click(")
    implementation = braced((ROOT / "src/win.cpp").read_text(),
                            "void Win::dispatch_19(")
    if damage:
        declaration = declaration.replace("virtual ", "", 1)
    harness = r'''
#include <cassert>
#include <cstdint>
#define __cdecl
class Win;
Win *WinCallbackWindow;
class Sound { public: int is_playing() { return 0; } void play() {} };
class Win {
 public:
    unsigned int iFlags_=0, iSomeFlag_=0;
    uintptr_t field_414_=0, flags_=0, val_20_=0;
    __DECLARATION__
    virtual int vslot_23() { return 0; }
    virtual void vslot_07() {}
    virtual void vslot_32(int,int) {}
    void dispatch_19(int,int,int);
};
__IMPLEMENTATION__
class ClickOwner : public Win {
 public:
    int clicks=0, alternate=0, x_seen=0, y_seen=0;
    void on_left_click(int x,int y) override {
        assert(WinCallbackWindow==this);
        ++clicks; x_seen=x; y_seen=y;
    }
    void vslot_32(int x,int y) override {
        ++alternate; x_seen=x; y_seen=y;
    }
};
int main() {
    ClickOwner owner;
    Win *base=&owner;
    base->dispatch_19(17,-9,0);
    assert(owner.clicks==1 && owner.x_seen==17 && owner.y_seen==-9);
    base->dispatch_19(4,8,1);
    assert(owner.clicks==1 && owner.alternate==1);
    assert(owner.x_seen==4 && owner.y_seen==8);
    owner.iFlags_=0x200000;
    base->dispatch_19(1,2,0);
    assert(owner.clicks==1);
    owner.iFlags_=0; owner.iSomeFlag_=8;
    base->dispatch_19(1,2,0);
    assert(owner.clicks==1);
}
'''.replace("__DECLARATION__", declaration).replace("__IMPLEMENTATION__", implementation)
    source = tmp_path / "left_click.cpp"
    executable = tmp_path / "left_click"
    source.write_text(harness)
    result = subprocess.run([compiler, "-std=c++11", "-Wall", "-Wextra", "-Werror",
                             "-fsanitize=address,undefined", str(source), "-o", str(executable)],
                            capture_output=True, text=True)
    if damage:
        assert result.returncode != 0 and "override" in result.stderr
        return
    assert result.returncode == 0, result.stderr
    environment = dict(os.environ, ASAN_OPTIONS="detect_leaks=0")
    run = subprocess.run([str(executable)], capture_output=True, text=True, env=environment)
    assert run.returncode == 0, run.stderr
