"""Bounded controls for migrated real virtual-owner dispatches.

The assertions tie calls to current class APIs and pinned slot evidence. The
tiny executable checks ordinary C++ virtual dispatch and argument/return flow;
it does not emulate VC6 code generation, object-layout evidence, or unknown
Console interface-object behavior.
"""
from pathlib import Path
import shutil
import subprocess

import pytest

ROOT = Path(__file__).resolve().parents[2]


def test_source_uses_existing_owner_contracts():
    win = (ROOT / "src/win.cpp").read_text()
    spritebox = (ROOT / "src/spritebox.cpp").read_text()
    listing = (ROOT / "src/listbox.cpp").read_text()
    console = (ROOT / "src/console.cpp").read_text()
    scenario = (ROOT / "src/scenario.cpp").read_text()
    menu_h = (ROOT / "src/menu.h").read_text()
    scroll_h = (ROOT / "src/scroll.h").read_text()
    graphic_h = (ROOT / "src/graphicwin.h").read_text()
    sound_h = (ROOT / "src/sound.h").read_text()
    main_h = (ROOT / "src/maininterface.h").read_text()

    assert '#include "vtable_shim.h"' not in win + spritebox
    assert '#include "vtable_shim.h"' in listing
    assert "MenuInitLegacyShim" not in win and "menu->init(this)" in win
    assert "class ScrollSlots" not in win
    assert win.count("->on_key_click((int)param2, WinKeyModifiers)") == 2
    assert "int init(Win *owner);" in menu_h
    assert "int on_key_click(int repeat, int key) override;" in scroll_h
    assert "virtual int on_key_click(int = 0, int = 0);" in (ROOT / "src/win.h").read_text()
    assert "static_cast<GraphicWin *>(this)->redraw();" in listing
    assert "legacy_sound_36()" in listing
    assert "virtual int play();" in sound_h
    assert "virtual int is_playing();" in sound_h
    assert "virtual void redraw();" in graphic_h
    assert "MainInterfaceGlobal.flat_button_[2].show(0);" in console
    assert "MainInterfaceGlobal.flat_button_[2].hide();" in console
    assert "reinterpret_cast<VCall" not in console
    assert "InterfaceSingletonAddress" not in scenario
    assert "InterfaceRefreshFunction" not in scenario
    assert "MainInterfaceGlobal.flat_button_[2].hide();" in scenario
    assert "child->play();" in listing
    assert "Sound slots 23/7 at AutoSound" in listing
    assert "FlatButton flat_button_[42];" in main_h


@pytest.mark.parametrize("compiler_name", ["g++", "clang++"])
def test_typed_virtual_dispatch_preserves_slots_args_and_results(compiler_name, tmp_path):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(f"{compiler_name} is unavailable")
    source = tmp_path / "dispatch.cpp"
    binary = tmp_path / "dispatch"
    source.write_text(r"""
#include <cassert>
struct Win { virtual ~Win() {} virtual int on_key_click(int, int) { return 0; }
             virtual void show(int) {} virtual void hide() {} };
struct Menu : Win { int seen; Menu(): seen(-1) {} virtual int init(Win *owner) {
    seen = owner == 0 ? -2 : 7; return seen; } };
struct Scroll : Win { int args; Scroll(): args(0) {} virtual int on_key_click(int a,int b) {
    args = a + b; return args != 0; } };
struct GraphicWin : Win { int redraws; GraphicWin(): redraws(0) {}
    virtual void redraw() { ++redraws; } };
struct Sound { int playing; int plays; Sound(): playing(0), plays(0) {}
    virtual int is_playing() { return playing; }
    virtual int play() { ++plays; return 9; } };
struct Button : Win { int shown; int hidden; Button(): shown(-1), hidden(0) {}
    virtual void show(int value) { shown = value; }
    virtual void hide() { ++hidden; } };
int main() {
    Menu menu; Win owner; assert(menu.init(&owner) == 7 && menu.seen == 7);
    Scroll vertical, horizontal;
    assert(vertical.on_key_click(13, 4) == 1 && vertical.args == 17);
    assert(horizontal.on_key_click(13, 4) == 1 && horizontal.args == 17);
    GraphicWin graphics; graphics.redraw(); assert(graphics.redraws == 1);
    Sound sound; if (sound.is_playing() == 0) sound.play(); assert(sound.plays == 1);
    Button button; button.show(0); assert(button.shown == 0);
    button.hide(); assert(button.hidden == 1);
}
""", encoding="utf-8")
    built = subprocess.run([compiler, "-std=c++98", "-Wall", "-Wextra", "-Werror",
                            str(source), "-o", str(binary)],
                           capture_output=True, text=True, timeout=60)
    assert built.returncode == 0, built.stdout + built.stderr
    ran = subprocess.run([str(binary)], capture_output=True, text=True, timeout=30)
    assert ran.returncode == 0, ran.stdout + ran.stderr
