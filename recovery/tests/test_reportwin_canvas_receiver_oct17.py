"""Actual-source host control for ReportWin::reset's Buffer receiver.

The C++ method body is extracted from src/reportwin.cpp; no body is embedded
here and no other test/draft is imported. It calls the six inherited Buffer
operations with explicit `Buffer::` qualification so the receiver is the
GraphicWin base without an alias conversion. Host spies distinguish that base
from ReportWin::buffer1_. These controls establish source behavior only, not
native ABI, original runtime, or game acceptance.
"""

from __future__ import annotations

import json
import os
from pathlib import Path
import shutil
import subprocess

import pytest


def repository_root() -> Path:
    for parent in Path(__file__).resolve().parents:
        if (parent / "src/reportwin.cpp").is_file() and (parent / "src/IMAGE_PIN").is_file():
            return parent
    raise AssertionError("could not locate repository root")


ROOT = repository_root()


def braced(source: str, signature: str) -> str:
    start = source.index(signature)
    opening = source.index("{", start)
    depth = 0
    for end in range(opening, len(source)):
        depth += (source[end] == "{") - (source[end] == "}")
        if depth == 0:
            return source[start : end + 1]
    raise AssertionError("unterminated source body")


def actual_reset_body() -> str:
    source = (ROOT / "src/reportwin.cpp").read_text()
    return braced(source, "void ReportWin::reset()")


HARNESS = r'''
#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>
#define nullptr 0

struct RECT {
    int left, top, right, bottom;
    RECT(int l=0, int t=0, int r=0, int b=0)
      : left(l), top(t), right(r), bottom(b) {}
};
struct Font { int identity; };
struct BoxSpriteParams { int identity; };
struct Call {
    char kind;
    const void *receiver;
    const void *first;
    const void *second;
    const void *third;
    int values[4];
    RECT rect;
    Call(char k, const void *r, const void *a, const void *b, const void *c,
         int x0, int x1, int x2, int x3, const RECT &copy)
      : kind(k), receiver(r), first(a), second(b), third(c), rect(copy) {
        values[0]=x0; values[1]=x1; values[2]=x2; values[3]=x3;
    }
};
static std::vector<Call> calls;
static void require(bool condition, int line) {
    if (!condition) {
        std::fprintf(stderr, "assertion failed at line %d\n", line);
        std::exit(95);
    }
}
#define REQUIRE(value) require((value), __LINE__)

struct Buffer {
    int receiver_id;
    Buffer() : receiver_id(0) {}
    int set_font(Font *a, Font *b, Font *c, void *d) {
        calls.push_back(Call('F', this, a, b, c, d==0, 0, 0, 0, RECT())); return 0;
    }
    void set_text_color(int a, int b, int c, int d) {
        calls.push_back(Call('1', this, 0, 0, 0, a, b, c, d, RECT()));
    }
    void set_text_color2(int a, int b, int c, int d) {
        calls.push_back(Call('2', this, 0, 0, 0, a, b, c, d, RECT()));
    }
    void set_text_color3(int a, int b, int c, int d) {
        calls.push_back(Call('3', this, 0, 0, 0, a, b, c, d, RECT()));
    }
    int box_sprite(RECT *area, BoxSpriteParams *style) {
        calls.push_back(Call('B', this, area, style, 0, 0, 0, 0, 0, *area)); return 0;
    }
};

struct Win { char distinct_first_base_[16]; };
struct GraphicWin : public Win, public Buffer {};
struct FlatButton {
    int identity;
    FlatButton() : identity(-1) {}
    void close() {
        calls.push_back(Call('C', this, 0, 0, 0, identity, 0, 0, 0, RECT()));
    }
};
struct ReportInterface {
    Font font184_;
    Font font190_;
    Font *field_1E184_;
    Font *field_1E190_;
    ReportInterface() : field_1E184_(&font184_), field_1E190_(&font190_) {}
};
static ReportInterface ReportIfGlobal;
static BoxSpriteParams InterfaceBoxStyles[10];

struct ReportWin : public GraphicWin {
    RECT rect1_;
    FlatButton flatButtons1_[7];
    RECT energy_panel_rect_;
    RECT report_panel_rect_;
    RECT report_work_rect_;
    RECT report_frame_rect_;
    Buffer buffer1_;
    void reset();
};

ACTUAL_RESET

static bool same_rect(const RECT &a, const RECT &b) {
    return a.left==b.left && a.top==b.top && a.right==b.right && a.bottom==b.bottom;
}
static void expect_color(const Call &call, int kind, int value) {
    REQUIRE(call.kind==kind);
    REQUIRE(call.values[0]==value && call.values[1]==-1
            && call.values[2]==1 && call.values[3]==1);
}
int main() {
    ReportWin window;
    Buffer *inherited = static_cast<Buffer *>(&window);
    inherited->receiver_id = 1;
    window.buffer1_.receiver_id = 2;
    for (int i=0; i<7; ++i) window.flatButtons1_[i].identity = i;
    window.rect1_ = RECT(-9,-8,-7,-6);
    window.energy_panel_rect_ = RECT(26,3,676,353);
    window.report_panel_rect_ = RECT(4,5,300,220);
    window.report_work_rect_ = RECT(-1,-2,-3,-4);
    window.report_frame_rect_ = RECT(10,20,100,200);
    InterfaceBoxStyles[9].identity=9;
    InterfaceBoxStyles[3].identity=3;
    RECT original_frame = window.report_frame_rect_;

    window.reset();

    REQUIRE(calls.size()==13);
    for (int i=0; i<7; ++i) {
        REQUIRE(calls[i].kind=='C');
        REQUIRE(calls[i].receiver==&window.flatButtons1_[i]);
        REQUIRE(calls[i].values[0]==i);
    }
    REQUIRE(calls[7].kind=='F' && calls[7].receiver==inherited);
    REQUIRE(calls[7].first==ReportIfGlobal.field_1E184_);
    REQUIRE(calls[7].second==ReportIfGlobal.field_1E184_);
    REQUIRE(calls[7].third==ReportIfGlobal.field_1E190_);
    REQUIRE(calls[7].values[0]==1);
    expect_color(calls[8], '1', 0x99);
    expect_color(calls[9], '2', 0xF2);
    expect_color(calls[10], '3', 0xE1);
    REQUIRE(calls[8].receiver==inherited && calls[9].receiver==inherited
            && calls[10].receiver==inherited);

    REQUIRE(calls[11].kind=='B' && calls[11].receiver==inherited);
    REQUIRE(calls[11].first==&window.report_panel_rect_);
    REQUIRE(calls[11].second==&InterfaceBoxStyles[9]);
    REQUIRE(same_rect(calls[11].rect, window.report_panel_rect_));
    REQUIRE(calls[12].kind=='B' && calls[12].receiver==inherited);
    REQUIRE(calls[12].first==&window.report_frame_rect_);
    REQUIRE(calls[12].second==&InterfaceBoxStyles[3]);
    REQUIRE(same_rect(calls[12].rect, RECT(7,17,103,203)));

    REQUIRE(same_rect(window.rect1_, RECT(26,3,650,350)));
    REQUIRE(same_rect(window.report_work_rect_, window.energy_panel_rect_));
    REQUIRE(same_rect(window.report_frame_rect_, original_frame));
    return 0;
}
'''


def source_for(body: str) -> str:
    return HARNESS.replace("ACTUAL_RESET", body)


def available_compiler() -> str:
    compiler = shutil.which("clang++") or shutil.which("g++")
    assert compiler is not None, "a host C++ compiler is required; this control must not skip vacuously"
    return compiler


def run_host(tmp_path: Path, body: str) -> subprocess.CompletedProcess[str]:
    cpp = tmp_path / "reportwin_reset.cpp"
    binary = tmp_path / "reportwin_reset"
    cpp.write_text(source_for(body))
    compiler = available_compiler()
    built = subprocess.run(
        [compiler, "-std=c++98", "-Wall", "-Wextra", "-pedantic", str(cpp), "-o", str(binary)],
        capture_output=True, text=True, timeout=30,
    )
    assert built.returncode == 0, built.stderr
    return subprocess.run([str(binary)], capture_output=True, text=True, timeout=10)


def test_actual_reset_uses_the_inherited_graphicwin_canvas(tmp_path: Path) -> None:
    body = actual_reset_body()
    expected_calls = {
        "Buffer::set_font(": 1,
        "Buffer::set_text_color(": 1,
        "Buffer::set_text_color2(": 1,
        "Buffer::set_text_color3(": 1,
        "Buffer::box_sprite(": 2,
    }
    for call, count in expected_calls.items():
        assert body.count(call) == count
    result = run_host(tmp_path, body)
    assert result.returncode == 0, result.stderr


def test_owned_buffer_mutant_fails_at_runtime(tmp_path: Path) -> None:
    body = actual_reset_body()
    positive = run_host(tmp_path, body)
    assert positive.returncode == 0, positive.stderr

    mutant = body
    wrong_owner_calls = {
        "Buffer::set_font(": "buffer1_.set_font(",
        "Buffer::set_text_color(": "buffer1_.set_text_color(",
        "Buffer::set_text_color2(": "buffer1_.set_text_color2(",
        "Buffer::set_text_color3(": "buffer1_.set_text_color3(",
        "Buffer::box_sprite(": "buffer1_.box_sprite(",
    }
    expected_counts = {"Buffer::box_sprite(": 2}
    for original, replacement in wrong_owner_calls.items():
        count = expected_counts.get(original, 1)
        assert mutant.count(original) == count
        mutant = mutant.replace(original, replacement)
    result = run_host(tmp_path, mutant)
    assert result.returncode == 95, result.stderr
    assert "assertion failed" in result.stderr


def test_pinned_receiver_witness() -> None:
    image = Path(os.environ.get("OPENSMACX_IMAGE", str(ROOT / ".opensmacx/game/terranx_original.exe")))
    assert image.is_file(), "pinned image is required for the native receiver witness"
    env = dict(os.environ)
    result = subprocess.run(
        ["uv", "run", "recover", "inspect", "0x00496BB0", "--image", str(image), "--max-bytes", "400"],
        cwd=ROOT, env=env, capture_output=True, text=True, timeout=60,
    )
    assert result.returncode == 0, result.stderr
    report = json.loads(result.stdout)
    assert report["image_verified"] is True
    lines = report["subjects"][0]["original"]["spans"][0]["disassembly"]
    receiver_at = next(i for i, line in enumerate(lines) if "lea edi, [esi + 0x444]" in line)
    positions = [i for i, line in enumerate(lines)
                 if i > receiver_at and "mov ecx, edi" in line]
    targets = []
    for position in positions:
        call = next((line for line in lines[position + 1 : position + 4] if "call " in line), None)
        if call is not None:
            targets.append(call.rsplit("call ", 1)[1])
    assert targets == ["0x5dac70", "0x5dacb0", "0x5dace0", "0x5dad10",
                       "0x5e2dd3", "0x5e2dd3"]
