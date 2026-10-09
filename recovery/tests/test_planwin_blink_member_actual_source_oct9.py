"""Exercise PlanWin::blink's actual source body through its typed member."""

from __future__ import annotations

import shutil
import subprocess
from pathlib import Path

import pytest


ROOT = Path(__file__).resolve().parents[2]


def _method_definition(text: str) -> str:
    start = text.index("void PlanWin::blink()")
    opening = text.index("{", start)
    depth = 0
    for index in range(opening, len(text)):
        if text[index] == "{":
            depth += 1
        elif text[index] == "}":
            depth -= 1
            if depth == 0:
                return text[start : index + 1]
    raise AssertionError("unterminated PlanWin::blink definition")


def test_planwin_blink_reads_owned_member_and_dispatches_virtually(
    tmp_path: Path,
) -> None:
    compiler = shutil.which("g++") or shutil.which("clang++")
    if compiler is None:
        pytest.skip("host C++ compiler unavailable")

    cpp = (ROOT / "src/planwin.cpp").read_text()
    assert "pre_vbase_dword" not in cpp
    body = _method_definition(cpp)
    assert "if (!field_21A68_)" in body

    fixture = r'''
#include <cassert>
#include <cstdint>

struct PlanWin {
    int32_t field_21A68_;
    int32_t field_21A6C_;
    virtual ~PlanWin() = default;
    virtual void on_redraw() { ++redraws; }
    int redraws = 0;
    void blink();
};

struct RedrawSpy : PlanWin {
    void on_redraw() override { ++virtual_redraws; }
    int virtual_redraws = 0;
};
''' + body + r'''
int main() {
    {
        RedrawSpy plan;
        plan.field_21A68_ = 0;
        plan.field_21A6C_ = 7;
        plan.blink();
        assert(plan.field_21A6C_ == 7);
        assert(plan.redraws == 0);
        assert(plan.virtual_redraws == 0);
    }

    const int32_t active_values[] = {1, -1, 0x100, -0x100};
    for (int32_t active : active_values) {
        RedrawSpy plan;
        plan.field_21A68_ = active;
        plan.field_21A6C_ = 0;
        plan.blink();
        assert(plan.field_21A6C_ == 1);
        assert(plan.redraws == 0);
        assert(plan.virtual_redraws == 1);
    }

    {
        RedrawSpy plan;
        plan.field_21A68_ = -3;
        plan.field_21A6C_ = 9;
        plan.blink();
        assert(plan.field_21A6C_ == 0);
        assert(plan.redraws == 0);
        assert(plan.virtual_redraws == 1);
    }
}
'''
    source = tmp_path / "planwin_blink.cpp"
    binary = tmp_path / "planwin_blink"
    source.write_text(fixture)
    subprocess.run(
        [
            compiler,
            "-std=c++11",
            "-Wall",
            "-Wextra",
            "-Werror",
            str(source),
            "-o",
            str(binary),
        ],
        check=True,
        timeout=30,
    )
    subprocess.run([str(binary)], check=True, timeout=30)
