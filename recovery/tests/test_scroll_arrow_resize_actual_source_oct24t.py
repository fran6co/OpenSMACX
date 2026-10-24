"""Bounded host controls for Scroll::on_resize's arrow placement.

The test source-selects the complete mapped owner with match_recovery's native
read_file reader and compiles it against a tiny rectangle/button fixture. The
expected coordinates come from the six pinned border/orientation paths. The
resize hook also probes the pinned post-virtual-call reload of bar_thickness_;
it does not claim FlatButton::resize actually reenters Scroll at runtime.
"""
from pathlib import Path
import os
import re
import shutil
import subprocess

import pytest

from match_recovery.source import read_file


ROOT = Path(os.environ.get("OPENSMACX_SOURCE_ROOT", Path(__file__).resolve().parents[2]))
UNIT = ROOT / "src/scroll.cpp"
ADDRESS = 0x00606860
SIGNATURE = "void Scroll::on_resize(int width, int height)"


def _actual_source():
    records = [record for record in read_file(UNIT) if record.address == ADDRESS]
    assert len(records) == 1
    return records[0].region


def _definition(source):
    start = source.index(SIGNATURE)
    opening = source.index("{", start)
    depth = 0
    for end in range(opening, len(source)):
        if source[end] == "{":
            depth += 1
        elif source[end] == "}":
            depth -= 1
            if depth == 0:
                return source[start:end + 1]
    raise AssertionError("unterminated Scroll::on_resize")


SCROLL_FIXTURE = r'''
#include <cassert>
#include <vector>
struct Call {
    int kind, first, second, third;
    Call(int k, int a, int b, int c) : kind(k), first(a), second(b), third(c) {}
    bool operator==(const Call &other) const {
        return kind == other.kind && first == other.first && second == other.second
            && third == other.third;
    }
};
std::vector<Call> calls;
struct Scroll;
struct ButtonProbe {
    Scroll *owner;
    int width, height, flags, x, y;
    bool mutate_parent_thickness;
    int replacement_thickness;
    ButtonProbe() : owner(0), width(0), height(0), flags(0), x(0), y(0),
        mutate_parent_thickness(false), replacement_thickness(0) {}
    virtual void resize(int new_width, int new_height, int resize_flags);
    virtual void move(int new_x, int new_y);
};
struct Rect { int left, top, right, bottom; };
struct Scroll {
    int bar_thickness_;
    int border_color_;
    Rect thumb_rect_;
    ButtonProbe flat_button_right_;
    Scroll() : bar_thickness_(0), border_color_(-1),
        thumb_rect_(), flat_button_right_() { flat_button_right_.owner = this; }
    void on_resize(int width, int height);
};
void ButtonProbe::resize(int new_width, int new_height, int resize_flags) {
    width = new_width; height = new_height; flags = resize_flags;
    calls.push_back(Call(1, width, height, flags));
    if (mutate_parent_thickness) owner->bar_thickness_ = replacement_thickness;
}
void ButtonProbe::move(int new_x, int new_y) {
    x = new_x; y = new_y;
    calls.push_back(Call(2, x, y, 0));
}
'''


def _compile_run(compiler, tmp_path, body, main, label, should_pass):
    source = tmp_path / (label + ".cpp")
    binary = tmp_path / label
    source.write_text(SCROLL_FIXTURE + body + "\n" + main, encoding="utf-8")
    built = subprocess.run(
        [compiler, "-std=c++17", "-Wall", "-Wextra", "-Werror", "-pedantic",
         str(source), "-o", str(binary)], capture_output=True, text=True, timeout=30)
    assert built.returncode == 0, built.stdout + built.stderr
    ran = subprocess.run([str(binary)], capture_output=True, text=True, timeout=10)
    if should_pass:
        assert ran.returncode == 0, ran.stdout + ran.stderr
    else:
        assert ran.returncode != 0, "top-left-corner placement escaped the coordinate oracle"


MAIN = r'''
static void check(int width, int height, int border, int thickness,
                  int left, int top, int right, int bottom,
                  int resize_width, int resize_height, int move_x, int move_y) {
    Scroll scroll;
    scroll.border_color_ = border;
    calls.clear();
    scroll.on_resize(width, height);
    assert(scroll.bar_thickness_ == thickness);
    assert(scroll.thumb_rect_.left == left && scroll.thumb_rect_.top == top);
    assert(scroll.thumb_rect_.right == right && scroll.thumb_rect_.bottom == bottom);
    assert(scroll.flat_button_right_.width == resize_width);
    assert(scroll.flat_button_right_.height == resize_height);
    assert(scroll.flat_button_right_.flags == 0);
    assert(scroll.flat_button_right_.x == move_x && scroll.flat_button_right_.y == move_y);
    assert(calls == std::vector<Call>({Call(1, resize_width, resize_height, 0),
                                       Call(2, move_x, move_y, 0)}));
}
int main() {
    // Border off: width<height, equality, and width>height.
    check(5, 9, -1, 5, 0, 0, 5, 5, 5, 5, 0, 4);
    check(7, 7, -1, 7, 0, 0, 7, 7, 7, 7, 0, 0);
    check(9, 5, -1, 5, 0, 0, 5, 5, 5, 5, 4, 0);
    // Border on: the one-pixel inset and reduced child size remain ordered.
    check(5, 9, 3, 5, 1, 1, 4, 4, 3, 3, 1, 5);
    check(7, 7, 3, 7, 1, 1, 6, 6, 5, 5, 1, 1);
    check(9, 5, 3, 5, 1, 1, 4, 4, 3, 3, 5, 1);
    // The body adds no dimension guards; negative sizes flow to resize and move.
    check(-3, 4, -1, -3, 0, 0, -3, -3, -3, -3, 0, 7);
    check(-3, 4, 2, -3, 1, 1, -4, -4, -5, -5, 1, 8);

    // Pinned code reloads bar_thickness_ after the child resize call. This
    // controlled hook checks that observation without asserting runtime reentry.
    Scroll reentered;
    reentered.border_color_ = -1;
    reentered.flat_button_right_.mutate_parent_thickness = true;
    reentered.flat_button_right_.replacement_thickness = 2;
    calls.clear();
    reentered.on_resize(5, 9);
    assert(reentered.thumb_rect_.right == 5 && reentered.thumb_rect_.bottom == 5);
    assert(reentered.flat_button_right_.width == 5 && reentered.flat_button_right_.height == 5);
    assert(reentered.flat_button_right_.x == 0 && reentered.flat_button_right_.y == 7);
    assert(calls == std::vector<Call>({Call(1, 5, 5, 0), Call(2, 0, 7, 0)}));

    Scroll horizontal_reload;
    horizontal_reload.border_color_ = -1;
    horizontal_reload.flat_button_right_.mutate_parent_thickness = true;
    horizontal_reload.flat_button_right_.replacement_thickness = 2;
    calls.clear();
    horizontal_reload.on_resize(9, 5);
    assert(horizontal_reload.thumb_rect_.right == 5 && horizontal_reload.thumb_rect_.bottom == 5);
    assert(horizontal_reload.flat_button_right_.width == 5 && horizontal_reload.flat_button_right_.height == 5);
    assert(horizontal_reload.flat_button_right_.x == 7 && horizontal_reload.flat_button_right_.y == 0);
    assert(calls == std::vector<Call>({Call(1, 5, 5, 0), Call(2, 7, 0, 0)}));

    Scroll bordered_vertical_reload;
    bordered_vertical_reload.border_color_ = 3;
    bordered_vertical_reload.flat_button_right_.mutate_parent_thickness = true;
    bordered_vertical_reload.flat_button_right_.replacement_thickness = 2;
    calls.clear();
    bordered_vertical_reload.on_resize(5, 9);
    assert(bordered_vertical_reload.thumb_rect_.left == 1 && bordered_vertical_reload.thumb_rect_.top == 1);
    assert(bordered_vertical_reload.thumb_rect_.right == 4 && bordered_vertical_reload.thumb_rect_.bottom == 4);
    assert(bordered_vertical_reload.flat_button_right_.width == 3 && bordered_vertical_reload.flat_button_right_.height == 3);
    assert(bordered_vertical_reload.flat_button_right_.x == 1 && bordered_vertical_reload.flat_button_right_.y == 8);
    assert(calls == std::vector<Call>({Call(1, 3, 3, 0), Call(2, 1, 8, 0)}));

    Scroll bordered_horizontal_reload;
    bordered_horizontal_reload.border_color_ = 3;
    bordered_horizontal_reload.flat_button_right_.mutate_parent_thickness = true;
    bordered_horizontal_reload.flat_button_right_.replacement_thickness = 2;
    calls.clear();
    bordered_horizontal_reload.on_resize(9, 5);
    assert(bordered_horizontal_reload.thumb_rect_.left == 1 && bordered_horizontal_reload.thumb_rect_.top == 1);
    assert(bordered_horizontal_reload.thumb_rect_.right == 4 && bordered_horizontal_reload.thumb_rect_.bottom == 4);
    assert(bordered_horizontal_reload.flat_button_right_.width == 3 && bordered_horizontal_reload.flat_button_right_.height == 3);
    assert(bordered_horizontal_reload.flat_button_right_.x == 8 && bordered_horizontal_reload.flat_button_right_.y == 1);
    assert(calls == std::vector<Call>({Call(1, 3, 3, 0), Call(2, 8, 1, 0)}));
}
'''


def test_scroll_arrow_placement_and_corner_mutant(tmp_path):
    body = _definition(_actual_source())
    compilers = [shutil.which(name) for name in ("g++", "clang++")]
    compilers = [compiler for compiler in compilers if compiler]
    if not compilers:
        pytest.skip("GCC/Clang host compilers unavailable")

    # Mutate all four source call sites to the previous top-left-corner form:
    # (0,0) without a border and (1,1) with a border. Count anchors prevent a
    # stale body from silently producing an unchanged mutant.
    matches = list(re.finditer(r"flat_button_right_\.move\s*\([^;]*?\);", body, re.S))
    assert len(matches) == 4, "candidate should have four branch move calls"
    mutant = body
    for index in range(len(matches) - 1, -1, -1):
        match = matches[index]
        corner = "flat_button_right_.move(1, 1);" if index < 2 else "flat_button_right_.move(0, 0);"
        mutant = mutant[:match.start()] + corner + mutant[match.end():]
    for compiler in compilers:
        label = Path(compiler).name
        _compile_run(compiler, tmp_path, body, MAIN, "scroll_resize_" + label, True)
        _compile_run(compiler, tmp_path, mutant, MAIN, "scroll_corner_mutant_" + label, False)
