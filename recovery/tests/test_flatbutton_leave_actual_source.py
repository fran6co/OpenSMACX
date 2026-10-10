"""Behavior controls for the installed FlatButton leave owner.

Only the complete function body in current src/flatbutton.cpp is compiled.
The host class declarations are deliberate test doubles, not ABI/layout proof.
"""

from pathlib import Path
import os
import re
import shutil
import subprocess

import pytest


ROOT = next(parent for parent in Path(__file__).resolve().parents
            if (parent / "src/flatbutton.cpp").is_file())
SIGNATURE = "void FlatButton::on_mouse_leave(int x, int y)"
COMPILERS = [p for name in ("g++", "clang++") if (p := shutil.which(name))]


def _mask_comments_and_literals(text: str) -> str:
    out = list(text)
    i = 0
    while i < len(text):
        if text.startswith("//", i):
            j = text.find("\n", i)
            if j < 0:
                j = len(text)
            for k in range(i, j):
                out[k] = " "
            i = j
        elif text.startswith("/*", i):
            j = text.find("*/", i + 2)
            assert j >= 0, "unterminated C++ comment"
            j += 2
            for k in range(i, j):
                if text[k] != "\n":
                    out[k] = " "
            i = j
        elif text[i] in "\"'":
            quote, j = text[i], i + 1
            while j < len(text):
                if text[j] == "\\":
                    j += 2
                elif text[j] == quote:
                    j += 1
                    break
                else:
                    j += 1
            for k in range(i, min(j, len(text))):
                if text[k] != "\n":
                    out[k] = " "
            i = j
        else:
            i += 1
    return "".join(out)


def _extract_actual_method() -> str:
    owner = ROOT / "src/flatbutton.cpp"
    scaffold = ROOT / "src/stringstruct.cpp"
    text = owner.read_text(encoding="utf-8")
    assert text.count(SIGNATURE) == 1, "expected one installed owner definition"
    assert "FlatButton_on_mouse_leave" not in scaffold.read_text(encoding="utf-8")
    masked = _mask_comments_and_literals(text)
    start = masked.index(SIGNATURE)
    opening = masked.index("{", start)
    depth = 0
    for end in range(opening, len(masked)):
        depth += (masked[end] == "{") - (masked[end] == "}")
        if depth == 0:
            return text[start:end + 1]
    raise AssertionError("unterminated installed FlatButton method")


HOST = r'''
#include <cassert>
#include <cstdint>
#include <vector>
static std::vector<int> events;
static int clear_hook_mode;
static int stop_hook_mode;
class Win;
class FlatButton;
static Win *WinPointerOwner1;
static Win *WinPointerOwner2;
static int WinSizingFlag;
static FlatButton *current_button;
static Win *replacement_parent;

struct Time {
    void stop();
};

class Win {
public:
    Win *win_parent_;
    Win() : win_parent_(0) {}
    virtual ~Win() {}
    virtual void on_mouse_move(int, int) { events.push_back(1); }
    virtual void on_mouse_leave(int = 0, int = 0) {}
    virtual void on_button_passover(int, int) { events.push_back(3); }
    virtual void redraw() { events.push_back(4); }
    static void clear_bubble_text();
};

class BaseButton : public Win {
public:
    Time time1_;
    uint32_t field_A74_;
    uint32_t field_A78_;
    char *bubble_text_;
    BaseButton() : field_A74_(0), field_A78_(0), bubble_text_(0) {}
};

class FlatButton : public BaseButton {
public:
    void on_mouse_move(int, int) override { events.push_back(2); }
    void on_mouse_leave(int, int) override;
    void redraw() override { events.push_back(4); }
};

class ProbeWin : public Win {
public:
    int payload;
    ProbeWin() : payload(-1) {}
    void on_button_passover(int value, int second) override {
        assert(second == 0);
        payload = value;
        assert(current_button != 0);
        assert(current_button->field_A74_ == 0);
        events.push_back(3);
        if (replacement_parent != 0) current_button->win_parent_ = replacement_parent;
    }
};

void Time::stop() {
    events.push_back(1);
    if (stop_hook_mode == 1 && current_button != 0)
        current_button->field_A78_ = 71;
}
void Win::clear_bubble_text() {
    events.push_back(2);
    if (clear_hook_mode == 1 && current_button != 0) {
        current_button->field_A78_ = 83;
        if (replacement_parent != 0) current_button->win_parent_ = replacement_parent;
    }
}

// ACTUAL_FUNCTION

class ProbeButton : public FlatButton {
public:
    int move_count;
    int redraw_count;
    ProbeButton() : move_count(0), redraw_count(0) {}
    void on_mouse_move(int x, int y) override {
        assert(x == 12 && y == 34);
        ++move_count;
        events.push_back(5);
    }
    void redraw() override { ++redraw_count; events.push_back(4); }
};

static void reset(ProbeButton &button, ProbeWin &parent_a, ProbeWin &parent_b) {
    events.clear();
    button.field_A74_ = 0;
    button.field_A78_ = 0;
    button.bubble_text_ = 0;
    button.win_parent_ = &parent_a;
    button.move_count = 0;
    button.redraw_count = 0;
    parent_a.payload = parent_b.payload = -1;
    current_button = &button;
    replacement_parent = 0;
    clear_hook_mode = stop_hook_mode = 0;
    WinPointerOwner1 = 0;
    WinPointerOwner2 = 0;
    WinSizingFlag = 0;
}

int main() {
    ProbeButton button{};
    ProbeWin parent_a{}, parent_b{};
    reset(button, parent_a, parent_b);

    // Override is reached through the real base type. The owner-2 fallback
    // plus sizing path dispatches slot 17 and returns before release work.
    WinPointerOwner2 = &button;
    WinSizingFlag = 1;
    button.field_A74_ = 1;
    Win *base_view = &button;
    base_view->on_mouse_leave(12, 34);
    assert(button.move_count == 1 && button.redraw_count == 0);
    assert(button.field_A74_ == 1 && events.size() == 1 && events[0] == 5);

    // A matching owner 1 takes precedence over any owner 2.
    reset(button, parent_a, parent_b);
    WinPointerOwner1 = &button;
    WinPointerOwner2 = &parent_a;
    WinSizingFlag = 1;
    button.field_A74_ = 1;
    base_view->on_mouse_leave(12, 34);
    assert(button.move_count == 1 && button.field_A74_ == 1);
    assert((events == std::vector<int>{5}));

    // Matching ownership without sizing mode takes the release path.
    reset(button, parent_a, parent_b);
    WinPointerOwner1 = &button;
    button.field_A74_ = 1;
    button.field_A78_ = 29;
    base_view->on_mouse_leave(12, 34);
    assert((events == std::vector<int>{1, 3, 4}));
    assert(parent_a.payload == 29 && button.move_count == 0);

    // Owner 1 wins even if owner 2 is the button; non-matching owner 1 does
    // not fall through. A pressed state takes the ordinary release path.
    reset(button, parent_a, parent_b);
    WinPointerOwner1 = &parent_a;
    WinPointerOwner2 = &button;
    WinSizingFlag = 1;
    button.field_A74_ = 1;
    button.field_A78_ = 41;
    button.bubble_text_ = const_cast<char *>("bubble");
    base_view->on_mouse_leave(12, 34);
    assert((events == std::vector<int>{1, 2, 3, 4}));
    assert(parent_a.payload == 41 && button.field_A74_ == 0);
    assert(button.redraw_count == 1 && button.move_count == 0);

    // The notification reads the then-current state word, after stop/clear;
    // the parent is read after clear, but captured before state is cleared.
    reset(button, parent_a, parent_b);
    button.field_A74_ = 1;
    button.field_A78_ = 7;
    button.bubble_text_ = const_cast<char *>("bubble");
    replacement_parent = &parent_b;
    stop_hook_mode = 1;
    clear_hook_mode = 1;
    base_view->on_mouse_leave(12, 34);
    assert((events == std::vector<int>{1, 2, 3, 4}));
    assert(parent_a.payload == -1 && parent_b.payload == 83);
    assert(button.field_A74_ == 0 && button.win_parent_ == &parent_b);

    // No state means no timer, bubble, parent callback, or redraw.
    reset(button, parent_a, parent_b);
    button.field_A74_ = 2;
    button.bubble_text_ = const_cast<char *>("bubble");
    base_view->on_mouse_leave(12, 34);
    assert(events.empty());

    // Null parent suppresses only notification; timer, bubble clear and
    // redraw remain ordered and state still clears.
    reset(button, parent_a, parent_b);
    button.field_A74_ = 1;
    button.bubble_text_ = const_cast<char *>("bubble");
    button.win_parent_ = 0;
    base_view->on_mouse_leave(12, 34);
    assert((events == std::vector<int>{1, 2, 4}));
    assert(button.field_A74_ == 0 && button.redraw_count == 1);
}
'''


@pytest.mark.parametrize("compiler", COMPILERS or [None])
def test_flatbutton_leave_actual_body_host_controls(tmp_path, compiler):
    if compiler is None:
        pytest.skip("no GCC or Clang host compiler available")
    body = _extract_actual_method()
    source = tmp_path / "flatbutton_leave.cpp"
    source.write_text(HOST.replace("// ACTUAL_FUNCTION", body), encoding="utf-8")
    binary = tmp_path / "flatbutton_leave"
    build = subprocess.run(
        [compiler, "-std=c++11", "-Wall", "-Wextra", "-Werror",
         "-fsanitize=address,undefined", "-fno-omit-frame-pointer",
         str(source), "-o", str(binary)],
        text=True, capture_output=True, check=False)
    assert build.returncode == 0, build.stdout + build.stderr
    # LeakSanitizer cannot attach under the session's ptrace restrictions.
    # Address and undefined-behavior instrumentation remain enabled.
    environment = dict(os.environ, ASAN_OPTIONS="detect_leaks=0")
    run = subprocess.run([str(binary)], text=True, capture_output=True,
                         check=False, env=environment)
    assert run.returncode == 0, run.stdout + run.stderr


def test_actual_body_preserves_observed_state_and_callback_order():
    body = _extract_actual_method()
    assert body.index("time1_.stop()") < body.index("Win::clear_bubble_text()")
    assert body.index("Win::clear_bubble_text()") < body.index("Win *const buddy = win_parent_")
    assert body.index("Win *const buddy = win_parent_") < body.index("field_A74_ = 0")
    assert body.index("field_A74_ = 0") < body.index("buddy->on_button_passover")
    assert body.index("buddy->on_button_passover") < body.index("redraw()")
    # This read stays inside the non-null parent branch and happens after the
    # earlier callbacks, matching the original live reread behavior.
    notify = body.index("buddy->on_button_passover")
    assert body.index("field_A78_", notify) < body.index("redraw()", notify)


def test_header_declares_a_real_flatbutton_override():
    header = (ROOT / "src/flatbutton.h").read_text(encoding="utf-8")
    assert re.search(r"void\s+on_mouse_leave\s*\(\s*int\s*,\s*int\s*\)\s+override\s*;", header)
