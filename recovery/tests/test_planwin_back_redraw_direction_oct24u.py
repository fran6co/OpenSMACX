"""Proposed actual-source direction control for PlanWin::back_redraw.

The harness distinguishes the inherited GraphicWin buffer (copy source) from
PlanWin::buffer_ (copy destination). It models only the Buffer::copy contract
and relevant owner calls; it does not claim GDI or native ABI coverage.
"""

from __future__ import annotations

import shutil
import subprocess
from pathlib import Path

import pytest


ROOT = Path(__file__).resolve().parents[2]
SOURCE = ROOT / "src/planwin_backing.cpp"
SIGNATURE = "void PlanWin::back_redraw()"
COPY_LINE = (
    "Buffer::copy(&buffer_, 0, 0, dib_.bmiHeader.biWidth,\n"
    "                 -dib_.bmiHeader.biHeight);"
)


def _body() -> str:
    from match_recovery.source import read_file
    from match_recovery.source.reader import function_line
    from match_recovery.local_variants import _bound_function_body_bytes, _marker_identity
    raw = SOURCE.read_bytes()
    records = read_file(SOURCE)
    target, = [row for row in records if row.address == 0x0048B3F0]
    text = raw.decode("utf-8")
    low, high = _bound_function_body_bytes(text, "PlanWin::back_redraw", SOURCE,
                                         target, tuple(_marker_identity(row) for row in records))
    line = function_line(text, "PlanWin::back_redraw", after_line=target.line)
    start = len("".join(text.splitlines(keepends=True)[:line - 1]).encode("utf-8"))
    return raw[start:high].decode("utf-8")


def _control(body: str) -> str:
    marker = "// ACTUAL_BACK_REDRAW"
    fixture = r'''#include <cassert>
#include <vector>

struct BitmapHeader { int biWidth; int biHeight; };
struct Dib { BitmapHeader bmiHeader; };

static std::vector<char> events;
static unsigned int field_seen_by_fill;
static unsigned int field_seen_by_draw;
static unsigned int field_seen_by_copy;
static int width_after_draw = 37;
static int height_after_draw = -19;

struct Buffer {
    Dib dib_;
    int pixel;
    int copy_calls;
    Buffer *copy_source;
    Buffer *copy_destination;
    int copied_x, copied_y, copied_width, copied_height;

    Buffer() : pixel(0), copy_calls(0), copy_source(0),
        copy_destination(0), copied_x(0), copied_y(0),
        copied_width(0), copied_height(0) {
        dib_.bmiHeader.biWidth = 11;
        dib_.bmiHeader.biHeight = -7;
    }

    int copy(Buffer *destination, int x, int y, int width, int height) {
        events.push_back('C');
        ++copy_calls;
        copy_source = this;
        copy_destination = destination;
        copied_x = x; copied_y = y;
        copied_width = width; copied_height = height;
        field_seen_by_copy = *owner_field;
        destination->pixel = pixel;
        return 1;
    }

    const unsigned int *owner_field;
};

struct GraphicWin : Buffer {
    void fill(int color) {
        assert(color == 0);
        events.push_back('F');
        pixel = color;
        field_seen_by_fill = *owner_field;
    }
};

struct MapWin : virtual GraphicWin {
    unsigned int field_1DD70_;
    MapWin() : field_1DD70_(0) {}
    void draw_map(int mode) {
        assert(mode == 0);
        events.push_back('D');
        field_seen_by_draw = field_1DD70_;
        // The source dimensions are live values read after draw_map returns.
        dib_.bmiHeader.biWidth = width_after_draw;
        dib_.bmiHeader.biHeight = height_after_draw;
        pixel = 0x1357;
    }
};

struct PlanWin : MapWin {
    Buffer buffer_;
    void back_redraw();
};

// ACTUAL_BACK_REDRAW

int main() {
    PlanWin plan;
    Buffer *source = static_cast<Buffer *>(static_cast<GraphicWin *>(&plan));
    Buffer *destination = &plan.buffer_;
    source->owner_field = &plan.field_1DD70_;
    const unsigned int destination_field = 0xFFFFFFFFu;
    destination->owner_field = &destination_field;
    source->pixel = 0x1357;
    destination->pixel = 0x2222;

    plan.back_redraw();

    assert(events.size() == 3);
    assert(events[0] == 'F' && events[1] == 'D' && events[2] == 'C');
    assert(plan.field_1DD70_ == 0x84010002);
    assert(field_seen_by_fill == 0x84010002);
    assert(field_seen_by_draw == 0x84010002);
    assert(field_seen_by_copy == 0x84010002);
    assert(source->copy_calls == 1);
    assert(source->copy_source == source);
    assert(source->copy_destination == destination);
    assert(destination->copy_calls == 0);
    assert(source->copied_x == 0 && source->copied_y == 0);
    assert(source->copied_width == 37 && source->copied_height == 19);
    assert(source->dib_.bmiHeader.biWidth == 37);
    assert(source->dib_.bmiHeader.biHeight == -19);
    assert(source->pixel == 0x1357 && destination->pixel == 0x1357);
}
'''
    assert fixture.count(marker) == 1
    return fixture.replace(marker, body)


def _compile_run(tmp_path: Path, source: str, compiler: str, name: str) -> subprocess.CompletedProcess[str]:
    cpp = tmp_path / f"{name}.cpp"
    exe = tmp_path / name
    cpp.write_text(source, encoding="utf-8")
    subprocess.run(
        [compiler, "-std=c++98", "-Wall", "-Wextra", "-Werror", str(cpp), "-o", str(exe)],
        check=True,
        capture_output=True,
        text=True,
        timeout=60,
    )
    return subprocess.run([str(exe)], capture_output=True, text=True, timeout=30)


def test_back_redraw_copies_inherited_graphic_surface_into_member_buffer(tmp_path: Path) -> None:
    body = _body()
    assert COPY_LINE in body
    compilers = [shutil.which(name) for name in ("g++", "clang++")]
    compilers = [compiler for compiler in compilers if compiler]
    if not compilers:
        pytest.skip("no host C++ compiler available")
    for index, compiler in enumerate(compilers):
        result = _compile_run(tmp_path, _control(body), compiler, f"direction-{index}")
        assert result.returncode == 0, result.stderr


def test_opposite_copy_direction_is_a_compilable_behavioral_mutant(tmp_path: Path) -> None:
    body = _body()
    assert body.count(COPY_LINE) == 1, "candidate copy direction anchor must match once"
    opposite = (
        "buffer_.copy(static_cast<Buffer *>(static_cast<GraphicWin *>(this)), "
        "0, 0, dib_.bmiHeader.biWidth, -dib_.bmiHeader.biHeight);"
    )
    mutant = body.replace(COPY_LINE, opposite, 1)
    assert mutant != body
    fixture = _control(mutant)
    compilers = [shutil.which(name) for name in ("g++", "clang++")]
    compilers = [compiler for compiler in compilers if compiler]
    if not compilers:
        pytest.skip("no host C++ compiler available")
    for index, compiler in enumerate(compilers):
        result = _compile_run(tmp_path, fixture, compiler, f"opposite-{index}")
        assert result.returncode != 0, f"opposite copy direction passed with {compiler}"
