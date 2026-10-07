"""Exercise the actual timer body, including qualified council dispatch."""

from pathlib import Path
import shutil
import subprocess

import pytest


ROOT = Path(__file__).resolve().parents[2]


@pytest.mark.parametrize("compiler", ["g++", "clang++"])
def test_turn_timer_uses_council_owner_and_original_guards(tmp_path, compiler):
    executable = shutil.which(compiler)
    if executable is None:
        pytest.skip(f"{compiler} unavailable")
    source = (ROOT / "src/time.cpp").read_text()
    body = source.split("void __cdecl turn_timer(int a1) {", 1)[1].split("\n}\n", 1)[0]
    assert "CouncilWindow.GraphicWin::redraw();" in body
    assert "extern int TurnRedrawPending;" in (ROOT / "src/time.h").read_text()
    fixture = r'''
#include <cassert>
int IsMultiplayerNet, TimerHoldFlag, TurnRedrawPending;
int redraws, virtual_redraws, draws;
struct GraphicWin {
    virtual void redraw() {
        ++redraws;
        TimerHoldFlag = 1;
        IsMultiplayerNet = 0;
        TurnRedrawPending = 0;
    }
};
struct CouncWin : GraphicWin {
    void redraw() override { ++virtual_redraws; }
} CouncilWindow;
struct MultiWin { void draw(int mode) { assert(mode == 1); ++draws; } } MultiWindow;
void turn_timer(int a1) { BODY }
int main() {
    for (int net = 0; net != 2; ++net)
    for (int hold = 0; hold != 2; ++hold)
    for (int pending = 0; pending != 2; ++pending) {
        IsMultiplayerNet = net; TimerHoldFlag = hold; TurnRedrawPending = pending;
        redraws = virtual_redraws = draws = 0;
        turn_timer(123);
        assert(redraws == (net && !hold && pending));
        assert(virtual_redraws == 0);
        assert(draws == (net && !hold));
    }
}
'''.replace("BODY", body)
    path = tmp_path / "timer.cpp"
    path.write_text(fixture)
    binary = tmp_path / "timer"
    subprocess.run([executable, "-std=c++11", str(path), "-o", str(binary)], check=True)
    subprocess.run([str(binary)], check=True)
    damaged = tmp_path / "virtual.cpp"
    damaged.write_text(fixture.replace("CouncilWindow.GraphicWin::redraw();", "CouncilWindow.redraw();"))
    damaged_binary = tmp_path / "virtual"
    subprocess.run([executable, "-std=c++11", str(damaged), "-o", str(damaged_binary)], check=True)
    assert subprocess.run([str(damaged_binary)], capture_output=True).returncode != 0
