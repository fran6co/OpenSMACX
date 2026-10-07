"""Compile controls built from the live DiploPop repaint body, not a copy."""

from __future__ import annotations

import re
import shutil
import subprocess
from pathlib import Path

import pytest


ROOT = next(
    parent
    for parent in Path(__file__).resolve().parents
    if (parent / "src/diplopop.cpp").is_file()
)


def _method_body(source: str) -> str:
    signature = "void DiploPop::on_redraw()"
    start = source.find(signature)
    assert start >= 0, "missing actual DiploPop::on_redraw definition"
    opening = source.find("{", start + len(signature))
    assert opening >= 0, "missing on_redraw body"

    depth = 0
    state = "code"
    quote = ""
    index = opening
    while index < len(source):
        char = source[index]
        next_char = source[index + 1] if index + 1 < len(source) else ""
        if state == "line-comment":
            if char == "\n":
                state = "code"
        elif state == "block-comment":
            if char == "*" and next_char == "/":
                state = "code"
                index += 1
        elif state == "string":
            if char == "\\":
                index += 1
            elif char == quote:
                state = "code"
        elif char == "/" and next_char == "/":
            state = "line-comment"
            index += 1
        elif char == "/" and next_char == "*":
            state = "block-comment"
            index += 1
        elif char in ('"', "'"):
            quote = char
            state = "string"
        elif char == "{":
            depth += 1
        elif char == "}":
            depth -= 1
            if depth == 0:
                return source[opening + 1:index]
        index += 1
    raise AssertionError("unterminated DiploPop::on_redraw body")


def _actual_body() -> str:
    source = (ROOT / "src/diplopop.cpp").read_text()
    header = (ROOT / "src/diplopop.h").read_text()
    main_header = (ROOT / "src/maininterface.h").read_text()

    assert re.search(r"\bvoid\s+on_redraw\(\)\s+override\s*;", header), (
        "DiploPop::on_redraw must be a real Win virtual override"
    )
    for name in ("draw_bioscan", "draw_portrait", "draw_info", "draw_hq"):
        assert re.search(rf"\bvoid\s+{name}\(\)\s*;", header)
    assert "RECT right_panel_button_rect_" in header
    assert "const RECT &main_layout_rect() const" in main_header
    assert "const RECT &rect2() const" in main_header
    assert "const RECT &rect3() const" in main_header
    return _method_body(source)


def _fixture(body: str) -> str:
    return r'''
#include <cstdlib>

struct RECT { int left, top, right, bottom; };
struct BoxSpriteParams { int id; };
BoxSpriteParams InterfaceBoxStyles[22];

enum { RESTORE = 1, UNION = 2, BIOSCAN = 4, PORTRAIT = 5, INFO = 6, HQ = 7 };
struct Trace {
    int events[16];
    int count;
    const void *receivers[16];
    const void *first_inputs[16];
    const void *second_inputs[16];
    RECT frames[4];
    RECT union_outputs[4];
    int frame_count;
    Trace() : count(0), frame_count(0) {}
    void record(int event, const void *receiver) {
        events[count] = event;
        receivers[count++] = receiver;
    }
};
Trace trace;

class Buffer {
public:
    int nonempty_secondary_base;
    void box_sprite(RECT *rect, BoxSpriteParams *style) {
        trace.record(style->id, this);
        trace.frames[trace.frame_count++] = *rect;
    }
};
struct PrimaryBase { int words[3]; };

class MainInterface : public PrimaryBase, public Buffer {
    RECT rect1_, rect2_, rect3_;
public:
    MainInterface() {
        rect1_.left = 10; rect1_.top = 20;
        rect1_.right = 210; rect1_.bottom = 120;
        rect2_.left = 0; rect2_.top = 0;
        rect2_.right = 40; rect2_.bottom = 50;
        rect3_.left = 30; rect3_.top = 25;
        rect3_.right = 80; rect3_.bottom = 75;
    }
    void restore_back(int value) {
        if (value == 1) trace.record(RESTORE, this);
    }
    const RECT &main_layout_rect() const { return rect1_; }
    const RECT &rect2() const { return rect2_; }
    const RECT &rect3() const { return rect3_; }
};
MainInterface MainInterfaceGlobal;

int UnionRect(RECT *destination, const RECT *first, const RECT *second) {
    trace.record(UNION, destination);
    trace.first_inputs[trace.count - 1] = first;
    trace.second_inputs[trace.count - 1] = second;
    destination->left = first->left < second->left ? first->left : second->left;
    destination->top = first->top < second->top ? first->top : second->top;
    destination->right = first->right > second->right ? first->right : second->right;
    destination->bottom = first->bottom > second->bottom ? first->bottom : second->bottom;
    trace.union_outputs[trace.count - 1] = *destination;
    return 1;
}

class DiploPop {
public:
    RECT right_panel_button_rect_;
    DiploPop() {
        right_panel_button_rect_.left = 60;
        right_panel_button_rect_.top = 5;
        right_panel_button_rect_.right = 100;
        right_panel_button_rect_.bottom = 45;
    }
    void on_redraw();
    void draw_bioscan() { trace.record(BIOSCAN, this); }
    void draw_portrait() { trace.record(PORTRAIT, this); }
    void draw_info() { trace.record(INFO, this); }
    void draw_hq() { trace.record(HQ, this); }
};

void DiploPop::on_redraw() {
BODY
}

static bool rect_is(const RECT &rect, int left, int top, int right, int bottom) {
    return rect.left == left && rect.top == top && rect.right == right &&
           rect.bottom == bottom;
}

int main() {
    InterfaceBoxStyles[3].id = 3;
    InterfaceBoxStyles[21].id = 21;
    DiploPop receiver;
    DiploPop other;
    receiver.on_redraw();
    const int expected[] = { RESTORE, 3, UNION, UNION, 21,
                             BIOSCAN, PORTRAIT, INFO, HQ };
    if (trace.count != 9 || trace.frame_count != 2) return EXIT_FAILURE;
    for (int index = 0; index < 9; ++index) {
        if (trace.events[index] != expected[index]) return EXIT_FAILURE;
    }
    const Buffer *canvas = static_cast<const Buffer *>(&MainInterfaceGlobal);
    if (trace.receivers[0] != &MainInterfaceGlobal ||
        trace.receivers[1] != canvas || trace.receivers[4] != canvas ||
        trace.receivers[2] != trace.receivers[3] ||
        trace.first_inputs[2] != &MainInterfaceGlobal.rect2() ||
        trace.second_inputs[2] != &MainInterfaceGlobal.rect3() ||
        trace.first_inputs[3] != trace.receivers[3] ||
        trace.second_inputs[3] != &receiver.right_panel_button_rect_) {
        return EXIT_FAILURE;
    }
    for (int index = 5; index < 9; ++index) {
        if (trace.receivers[index] != &receiver) return EXIT_FAILURE;
    }
    if (!rect_is(trace.frames[0], 14, 24, 206, 116) ||
        !rect_is(trace.union_outputs[2], 0, 0, 80, 75) ||
        !rect_is(trace.union_outputs[3], 0, 0, 100, 75) ||
        !rect_is(trace.frames[1], -3, -3, 103, 78) ||
        !rect_is(receiver.right_panel_button_rect_, 60, 5, 100, 45) ||
        !rect_is(other.right_panel_button_rect_, 60, 5, 100, 45)) {
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
'''.replace("BODY", body)


def _compile_and_run(compiler: str, body: str, tmp_path: Path) -> int:
    source = tmp_path / "on_redraw.cpp"
    executable = tmp_path / "on_redraw"
    source.write_text(_fixture(body))
    built = subprocess.run(
        [compiler, "-std=c++98", "-Wall", "-Wextra", "-pedantic",
         str(source), "-o", str(executable)],
        check=False,
        capture_output=True,
        text=True,
    )
    assert built.returncode == 0, built.stderr
    return subprocess.run([str(executable)], check=False).returncode


@pytest.mark.parametrize("compiler_name", ["g++", "clang++"])
def test_actual_on_redraw_owner_trace_and_damage_mutants(compiler_name, tmp_path):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(f"{compiler_name} is unavailable")
    body = _actual_body()
    assert _compile_and_run(compiler, body, tmp_path) == 0

    mutants = (
        body.replace("static_cast<Buffer &>(MainInterfaceGlobal)",
                     "reinterpret_cast<Buffer &>(MainInterfaceGlobal)", 1),
        body.replace("rc.right -= 4;", "rc.right += 4;", 1),
        body.replace("    draw_bioscan();", "    draw_hq();", 1),
        body.replace(
            "    draw_bioscan();\n    draw_portrait();",
            "    draw_portrait();\n    draw_bioscan();",
            1,
        ),
        body.replace("    draw_portrait();\n", "", 1),
    )
    for mutant in mutants:
        assert mutant != body
        assert _compile_and_run(compiler, mutant, tmp_path) != 0
