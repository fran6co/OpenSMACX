"""Host behavior controls for FlatButton::on_mouse_move's pointer priority.

Compile the complete current method extracted from src/flatbutton.cpp with
typed test doubles. This checks behavior only; the fixture makes no ABI/layout
claim.
"""
from pathlib import Path
import os
import re
import shutil
import subprocess

import pytest


ROOT = next(parent for parent in Path(__file__).resolve().parents
            if (parent / "src/flatbutton.cpp").is_file())
SOURCE = ROOT / "src/flatbutton.cpp"
SIGNATURE = "void FlatButton::on_mouse_move(int x, int y)"
COMPILERS = [name for name in ("g++", "clang++") if shutil.which(name)]


def _masked(text: str) -> str:
    out = list(text)
    i = 0
    while i < len(text):
        if text.startswith("//", i):
            end = text.find("\n", i)
            if end < 0:
                end = len(text)
            for j in range(i, end):
                out[j] = " "
            i = end
        elif text.startswith("/*", i):
            end = text.find("*/", i + 2)
            assert end >= 0, "unterminated block comment"
            end += 2
            for j in range(i, end):
                if text[j] != "\n":
                    out[j] = " "
            i = end
        elif text[i] in "\"'":
            quote, end = text[i], i + 1
            while end < len(text):
                if text[end] == "\\":
                    end += 2
                elif text[end] == quote:
                    end += 1
                    break
                else:
                    end += 1
            for j in range(i, min(end, len(text))):
                if text[j] != "\n":
                    out[j] = " "
            i = end
        else:
            i += 1
    return "".join(out)


def _actual_method() -> str:
    text = SOURCE.read_text(encoding="utf-8")
    masked = _masked(text)
    starts = []
    pos = 0
    while True:
        pos = masked.find(SIGNATURE, pos)
        if pos < 0:
            break
        starts.append(pos)
        pos += len(SIGNATURE)
    assert len(starts) == 1, "expected one installed FlatButton owner definition"
    start = starts[0]
    opening = masked.find("{", start + len(SIGNATURE))
    assert opening >= 0
    depth = 0
    for end in range(opening, len(masked)):
        if masked[end] == "{":
            depth += 1
        elif masked[end] == "}":
            depth -= 1
            if depth == 0:
                return text[start:end + 1]
    raise AssertionError("unterminated FlatButton::on_mouse_move body")


HOST = r'''
#include <cassert>
#include <cstdint>
#include <vector>

struct RECT { int left, top, right, bottom; };
static std::vector<int> events;
static int hit_result;
static int clear_count;
static int hover_hook_count;
class Win;
class FlatButton;
static Win *WinPointerOwner1;
static Win *WinPointerOwner2;
static void (*WinButtonHoverHook)();

struct Time {
    int id;
    void stop();
    void init(void (*)(int, void *), int, void *, unsigned int, unsigned int);
    void pulse();
};
class Win {
public:
    Win *win_parent_;
    Win() : win_parent_(0) {}
    virtual ~Win() {}
    virtual void on_button_passover(int, int) { events.push_back(30); }
    virtual void redraw() { events.push_back(40); }
    static void clear_bubble_text();
};
class BaseButton : public Win {
public:
    Time time1_;
    uint32_t field_A74_;
    uint32_t field_A78_;
    char *bubble_text_;
    BaseButton() : field_A74_(0), field_A78_(0), bubble_text_(0) {}
    void on_mouse_move(int, int) { events.push_back(10); }
};
class Buffer {
public:
    RECT rect1_;
};
class FlatButton : public BaseButton, public Buffer {
public:
    void on_mouse_move(int, int);
};
int in_box(int, int, const RECT *) { return hit_result; }
void base_button_timer_callback(int, void *) { events.push_back(99); }
void Time::stop() { events.push_back(20 + id); }
void Time::init(void (*)(int, void *), int, void *, unsigned int, unsigned int) {
    events.push_back(50 + id);
}
void Time::pulse() { events.push_back(60 + id); }
void Win::clear_bubble_text() { ++clear_count; events.push_back(70); }
static void hover_hook() { ++hover_hook_count; events.push_back(80); }

// ACTUAL_FUNCTION

static void reset(FlatButton &button, Win &parent) {
    events.clear();
    clear_count = hover_hook_count = 0;
    button.time1_.id = 1;
    button.field_A74_ = 0;
    button.field_A78_ = 37;
    button.bubble_text_ = 0;
    button.win_parent_ = &parent;
    button.rect1_ = RECT{4, 6, 24, 36};
    WinPointerOwner1 = 0;
    WinPointerOwner2 = 0;
    WinButtonHoverHook = &hover_hook;
    hit_result = 1;
}

int main() {
    FlatButton button{};
    Win other{};
    reset(button, other);

    // A different non-null primary owner blocks a matching secondary owner;
    // the callback follows ordinary hover handling, including timer/parent.
    WinPointerOwner1 = &other;
    WinPointerOwner2 = &button;
    button.field_A74_ = 0;
    button.bubble_text_ = const_cast<char *>("tip");
    button.on_mouse_move(8, 9);
    assert((events == std::vector<int>{80, 30, 51, 61, 40}));
    assert(button.field_A74_ == 1 && clear_count == 0);

    // Matching primary owner takes the base-button forwarding path even when
    // the secondary owner is different.
    reset(button, other);
    WinPointerOwner1 = &button;
    WinPointerOwner2 = &other;
    button.on_mouse_move(8, 9);
    assert((events == std::vector<int>{10}));

    // The secondary owner is used when and only when the primary is null.
    reset(button, other);
    WinPointerOwner1 = 0;
    WinPointerOwner2 = &button;
    button.on_mouse_move(8, 9);
    assert((events == std::vector<int>{10}));

    // With neither slot owned by this button, use ordinary hover transitions.
    reset(button, other);
    button.on_mouse_move(8, 9);
    assert((events == std::vector<int>{80, 30, 40}));
    assert(button.field_A74_ == 1);

    // Both-null is the same no-owner case, explicitly retained as a witness.
    reset(button, other);
    WinPointerOwner1 = 0;
    WinPointerOwner2 = 0;
    button.field_A74_ = 1;
    button.bubble_text_ = const_cast<char *>("tip");
    hit_result = 0;
    button.on_mouse_move(8, 9);
    assert((events == std::vector<int>{21, 70, 30, 40}));
    assert(button.field_A74_ == 0 && clear_count == 1);
}
'''


def _compile_run(tmp_path: Path, compiler: str, body: str):
    source = tmp_path / "flatbutton_move.cpp"
    binary = tmp_path / "flatbutton_move"
    source.write_text(HOST.replace("// ACTUAL_FUNCTION", body), encoding="utf-8")
    build = subprocess.run(
        [shutil.which(compiler), "-std=c++11", "-Wall", "-Wextra", "-Werror",
         "-fsanitize=address,undefined", "-fno-omit-frame-pointer",
         str(source), "-o", str(binary)],
        cwd=tmp_path, text=True, capture_output=True, timeout=30)
    assert build.returncode == 0, build.stdout + build.stderr
    env = dict(os.environ, ASAN_OPTIONS="detect_leaks=0")
    return subprocess.run([str(binary)], cwd=tmp_path, text=True,
                          capture_output=True, timeout=10, env=env)


@pytest.mark.parametrize("compiler", COMPILERS or [None])
def test_actual_move_body_preserves_owner_priority_and_hover_callbacks(tmp_path, compiler):
    if compiler is None:
        pytest.skip("no GCC or Clang host compiler available")
    result = _compile_run(tmp_path, compiler, _actual_method())
    assert result.returncode == 0, result.stdout + result.stderr


@pytest.mark.parametrize("compiler", COMPILERS or [None])
def test_old_or_guard_fails_the_primary_owner_priority_witness(tmp_path, compiler):
    if compiler is None:
        pytest.skip("no GCC or Clang host compiler available")
    body = _actual_method()
    correct = "WinPointerOwner1 == this || WinPointerOwner2 == this"
    assert body.count(correct) == 0, "actual owner still contains the old OR guard"
    mutant, replacements = re.subn(
        r"Win\s*\*\s*active\s*=\s*WinPointerOwner1\s*;\s*"
        r"if\s*\(\s*active\s*==\s*(?:nullptr|0)\s*\)\s*\{\s*"
        r"active\s*=\s*WinPointerOwner2\s*;\s*\}\s*"
        r"if\s*\(\s*active\s*==\s*this\s*\)",
        "if (WinPointerOwner1 == this || WinPointerOwner2 == this)", body, count=1)
    assert replacements == 1, "actual owner no longer exposes the corrected priority guard"
    result = _compile_run(tmp_path, compiler, mutant)
    assert result.returncode != 0, "the old OR guard unexpectedly passed the priority witness"
    assert "assert" in result.stderr.lower() and "events" in result.stderr.lower(), (
        "the mutant must fail the priority witness, not crash for an unrelated reason:\n"
        + result.stdout + result.stderr)
