"""Exercise the actual PlanWin copy body through its typed C++ hierarchy.

The control uses finite dimensions. Native INT_MIN negation behavior remains
unresolved and is intentionally not normalized or asserted here.
"""

from __future__ import annotations

import shutil
import subprocess
from pathlib import Path

import pytest


ROOT = Path(__file__).resolve().parents[2]


def _method_definition(text: str) -> str:
    start = text.index("void PlanWin::UNK1()")
    opening = text.index("{", start)
    depth = 0
    for index in range(opening, len(text)):
        if text[index] == "{":
            depth += 1
        elif text[index] == "}":
            depth -= 1
            if depth == 0:
                return text[start : index + 1]
    raise AssertionError("unterminated PlanWin::UNK1 definition")


def test_planwin_copy_uses_typed_virtual_base_and_dimensions(tmp_path: Path) -> None:
    compiler = shutil.which("g++") or shutil.which("clang++")
    if compiler is None:
        pytest.skip("host C++ compiler unavailable")

    cpp = (ROOT / "src/planwin.cpp").read_text()
    assert "virtual_base_of" not in cpp
    assert "void PlanWin::UNK1()" in cpp
    body = _method_definition(cpp)
    fixture = r'''
#include <cassert>

struct Buffer {
    int width = 0;
    int stored_height = 0;
    int calls = 0;
    Buffer *source = nullptr;
    int x = 0, y = 0, copy_width = 0, copy_height = 0;
    int pixel_width() const { return width; }
    int pixel_height() const { return -stored_height; }
    int copy(Buffer *input, int x_arg, int y_arg, int width_arg, int height_arg) {
        ++calls;
        source = input;
        x = x_arg; y = y_arg;
        copy_width = width_arg; copy_height = height_arg;
        return 7;
    }
};
struct Win { virtual ~Win() = default; };
struct GraphicWin : Win, Buffer {};
struct MapWin : virtual GraphicWin {};
struct PlanWin : MapWin {
    Buffer buffer_;
    void UNK1();
};
''' + body + r'''
int main() {
    PlanWin plan;
    GraphicWin *graphic = static_cast<GraphicWin *>(&plan);
    Buffer *receiver = static_cast<Buffer *>(graphic);
    graphic->width = 321;
    graphic->stored_height = -123;
    plan.UNK1();
    assert(receiver->calls == 1);
    assert(receiver->source == &plan.buffer_);
    assert(receiver->x == 0 && receiver->y == 0);
    assert(receiver->copy_width == 321 && receiver->copy_height == 123);
}
'''
    source = tmp_path / "planwin_copy.cpp"
    binary = tmp_path / "planwin_copy"
    source.write_text(fixture)
    subprocess.run(
        [compiler, "-std=c++11", "-Wall", "-Wextra", "-Werror", str(source), "-o", str(binary)],
        check=True,
        timeout=30,
    )
    subprocess.run([str(binary)], check=True, timeout=30)

