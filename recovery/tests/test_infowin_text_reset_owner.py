"""Actual timer/reset source controls; not a game runtime or ABI proof."""

from pathlib import Path
import re
import shutil
import subprocess

import pytest


ROOT = Path(__file__).resolve().parents[2]


def function_body(text, signature):
    opening = text.index("{", text.index(signature) + len(signature))
    depth = 0
    for index in range(opening, len(text)):
        if text[index] == "{":
            depth += 1
        elif text[index] == "}":
            depth -= 1
            if depth == 0:
                return text[opening + 1:index]
    raise AssertionError("unterminated source body")


@pytest.mark.parametrize("compiler_name", ["g++", "clang++"])
def test_actual_timer_change_uses_actual_buffer_reset(tmp_path, compiler_name):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(f"{compiler_name} unavailable")
    infowin = (ROOT / "src/infowin.cpp").read_text()
    body = function_body(infowin, "void InfoWin::timer_change(int)")
    reset = function_body((ROOT / "src/buffer.h").read_text(), "void reset_text_state()")
    assert "InfoAnimationCounters" not in infowin
    assert body.count("MainInterfaceGlobal.reset_text_state();") == 1
    assert re.findall(r"(\w+)\s*=\s*0;", reset) == [
        "write_font_slot_", "font_slot_", "field_514_", "markup_pending_"
    ]
    source = r'''
#include <cassert>
#include <stdint.h>
struct RECT { int left, top, right, bottom; };
static int WinScreenWidth = 1024;
static int InterfaceBoxStyles[19];
static int events[8], event_count;
struct Buffer {
    uint32_t before, write_font_slot_, field_514_, font_slot_, markup_pending_, after;
    void reset_text_state() { RESET_BODY }
};
struct MainInterface : Buffer {
    int box_sprite(RECT *, int *) {
        assert(write_font_slot_ == 0 && field_514_ == 0);
        assert(font_slot_ == 0 && markup_pending_ == 0);
        assert(before == 0xB0 && after == 0xB1);
        events[event_count++] = 2;
        return 0;
    }
};
static MainInterface MainInterfaceGlobal;
struct InfoWin {
    int field_94_, active_pane_, redraw_phase_, pane_states_[2];
    int visible_characters_[2], field_A8_;
    RECT pane_rects_[2];
    int active_slot, validity_count;
    InfoWin() : field_94_(9), active_pane_(1), redraw_phase_(-1),
        field_A8_(7), active_slot(0), validity_count(0) {
        pane_states_[0] = 0; pane_states_[1] = 0;
        visible_characters_[0] = 8; visible_characters_[1] = 9;
    }
    bool active_interface_slot_occupied() const { return active_slot != 0; }
    bool state_valid(int) {
        assert(MainInterfaceGlobal.write_font_slot_ == 0xA1);
        assert(MainInterfaceGlobal.field_514_ == 0xA2);
        assert(MainInterfaceGlobal.font_slot_ == 0xA3);
        assert(MainInterfaceGlobal.markup_pending_ == 0xA4);
        return ++validity_count == 3;
    }
    void setup_text() { events[event_count++] = 3; }
    void draw_text(int) { events[event_count++] = 4; }
    void timer_change(int);
};
void InfoWin::timer_change(int) { TIMER_BODY }
int main() {
    MainInterfaceGlobal.before = 0xB0;
    MainInterfaceGlobal.after = 0xB1;
    MainInterfaceGlobal.write_font_slot_ = 0xA1;
    MainInterfaceGlobal.field_514_ = 0xA2;
    MainInterfaceGlobal.font_slot_ = 0xA3;
    MainInterfaceGlobal.markup_pending_ = 0xA4;
    InfoWin blocked;
    blocked.active_slot = 1;
    blocked.timer_change(0);
    assert(blocked.validity_count == 0 && event_count == 0);
    assert(MainInterfaceGlobal.write_font_slot_ == 0xA1);
    InfoWin changed;
    changed.timer_change(0);
    assert(changed.validity_count == 3);
    assert(MainInterfaceGlobal.write_font_slot_ == 0);
    assert(MainInterfaceGlobal.field_514_ == 0);
    assert(MainInterfaceGlobal.font_slot_ == 0);
    assert(MainInterfaceGlobal.markup_pending_ == 0);
    assert(event_count == 3 && events[0] == 2 && events[1] == 3 && events[2] == 4);
    assert(changed.visible_characters_[0] == 0 && changed.visible_characters_[1] == 0);
}
'''.replace("RESET_BODY", reset).replace("TIMER_BODY", body)
    path = tmp_path / "timer.cpp"
    binary = tmp_path / "timer"
    path.write_text(source)
    subprocess.run([compiler, "-std=c++03", "-Wall", "-Wextra", "-Werror",
                    str(path), "-o", str(binary)], check=True)
    subprocess.run([str(binary)], check=True)
