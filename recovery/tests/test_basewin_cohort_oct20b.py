"""Actual-source host controls for the BaseWin queue owner cohort.

The test compiles the literal production definition in a minimal C++98 fixture;
it does not translate the implementation into a separate test model.
"""

from __future__ import annotations

import shutil
import subprocess
import re
from pathlib import Path

import pytest


ROOT = Path(__file__).resolve().parents[2]
SOURCE = ROOT / "src" / "basewin.cpp"
HEADER = ROOT / "src" / "basewin.h"
SIGNATURE = "void BaseWin::delete_queue(int index)"


def function_definition(source: str, signature: str) -> str:
    start = source.index(signature)
    opening = source.index("{", start)
    depth = 0
    for offset in range(opening, len(source)):
        if source[offset] == "{":
            depth += 1
        elif source[offset] == "}":
            depth -= 1
            if depth == 0:
                return source[start : offset + 1]
    raise AssertionError(f"unterminated function definition: {signature}")


def selected_slot_declaration(header: str) -> str:
    matches = re.findall(r"^\s*(int|uint32_t)\s+selected_queue_slot_\s*;\s*// 0x40B2C\s*$", header, re.M)
    assert matches == ["int"], matches
    assert "RECOVERY_MEMBER: v1 BaseWin selected_queue_slot_ 0x40B2C 0x00416DE0" in header
    return matches[0]


def host_program(definition: str, member_type: str = "int") -> str:
    return r"""#include <stdlib.h>
typedef unsigned int uint32_t;

struct Base {
    uint32_t queue_size;
    int queue_production_id[10];
};

static Base base_storage;
Base *BaseCurrent = &base_storage;

struct BaseWin {
    MEMBER_TYPE selected_queue_slot_;
    int draw_calls;
    int last_draw_argument;

    BaseWin() : selected_queue_slot_(0), draw_calls(0), last_draw_argument(0) {}
    void draw_queue(int argument) {
        ++draw_calls;
        last_draw_argument = argument;
    }
    void delete_queue(int index);
};

DEFINITION

static void reset_base(uint32_t count) {
    base_storage.queue_size = count;
    for (int i = 0; i != 10; ++i)
        base_storage.queue_production_id[i] = 100 + i;
}

static int check_oversized_index(BaseWin &window, int before, int expected) {
    reset_base(4);
    window.selected_queue_slot_ = before;
    window.draw_calls = 0;
    window.last_draw_argument = 0;
    window.delete_queue(9);
    if (window.selected_queue_slot_ != expected) return 1;
    if (BaseCurrent->queue_size != 4) return 2;
    if (window.draw_calls != 0) return 3;
    if (window.last_draw_argument != 0) return 4;
    if (BaseCurrent->queue_production_id[0] != 100 ||
        BaseCurrent->queue_production_id[9] != 109) return 5;
    return 0;
}

int main() {
    BaseWin window;
    // Signed low/negative values exercise the native invalid-index branch
    // without invoking signed overflow or the old unsigned wrap model.
    const int inputs[] = {-3, -1, 0, 1, 2, 3};
    const int expected[] = {1, 1, 1, 1, 1, 2};
    for (int i = 0; i != 6; ++i) {
        const int result = check_oversized_index(window, inputs[i], expected[i]);
        if (result != 0) return 10 + i * 10 + result;
    }

    // A valid removal shifts the live queue left, preserves selection 3 when
    // the new count is also 3, and redraws exactly once with argument 1.
    reset_base(4);
    window.selected_queue_slot_ = 3;
    window.draw_calls = 0;
    window.last_draw_argument = 0;
    window.delete_queue(1);
    if (BaseCurrent->queue_size != 3U) return 80;
    if (window.selected_queue_slot_ != 3) return 81;
    if (window.draw_calls != 1 || window.last_draw_argument != 1) return 82;
    if (BaseCurrent->queue_production_id[0] != 100) return 83;
    for (int slot = 1; slot != 9; ++slot)
        if (BaseCurrent->queue_production_id[slot] != 101 + slot) return 84;
    if (BaseCurrent->queue_production_id[9] != 109) return 85;

    // Removal below the selected slot clamps selection to the new queue count.
    reset_base(3);
    window.selected_queue_slot_ = 4;
    window.draw_calls = 0;
    window.last_draw_argument = 0;
    window.delete_queue(0);
    if (BaseCurrent->queue_size != 2U) return 90;
    if (window.selected_queue_slot_ != 2) return 91;
    if (window.draw_calls != 1 || window.last_draw_argument != 1) return 92;
    for (int slot = 0; slot != 9; ++slot)
        if (BaseCurrent->queue_production_id[slot] != 101 + slot) return 93;

    // A signed negative selection compares below the nonnegative new count;
    // the body leaves it intact rather than applying an unsigned comparison.
    reset_base(4);
    window.selected_queue_slot_ = -3;
    window.draw_calls = 0;
    window.last_draw_argument = 0;
    window.delete_queue(1);
    if (BaseCurrent->queue_size != 3U) return 100;
    if (window.selected_queue_slot_ != -3) return 101;
    if (window.draw_calls != 1 || window.last_draw_argument != 1) return 102;

    return 0;
}
""".replace("DEFINITION", definition).replace("MEMBER_TYPE", member_type)


def compile_and_run(
    compiler: str,
    definition: str,
    tmp_path: Path,
    member_type: str = "int",
    warnings_as_errors: bool = True,
) -> subprocess.CompletedProcess[str]:
    tmp_path.mkdir(parents=True, exist_ok=True)
    source_path = tmp_path / "delete_queue_host.cpp"
    executable = tmp_path / "delete_queue_host"
    source_path.write_text(host_program(definition, member_type))
    flags = [compiler, "-std=c++98", "-Wall", "-Wextra"]
    if warnings_as_errors:
        flags.append("-Werror")
    built = subprocess.run(
        [*flags, "-O0", str(source_path), "-o", str(executable)],
        text=True,
        capture_output=True,
        check=False,
    )
    assert built.returncode == 0, built.stdout + built.stderr
    return subprocess.run([str(executable)], text=True, capture_output=True, check=False)


def test_actual_delete_queue_body_and_damage_controls(tmp_path: Path) -> None:
    source = SOURCE.read_text()
    member_type = selected_slot_declaration(HEADER.read_text())
    original = function_definition(source, SIGNATURE)
    assert "int selected = --selected_queue_slot_;" in original
    assert "if (selected <= 1)" in original
    assert "selected_queue_slot_ = selected;" in original

    compilers = [path for name in ("g++", "clang++") if (path := shutil.which(name))]
    if not compilers:
        pytest.skip("neither g++ nor clang++ is installed")

    wrong_decrement = original.replace(
        "int selected = --selected_queue_slot_;",
        "int selected = ++selected_queue_slot_;",
        1,
    )
    assert wrong_decrement != original
    missing_clamp = original.replace(
        "        if (selected <= 1) {\n"
        "            selected = 1;\n"
        "        }\n",
        "",
        1,
    )
    assert missing_clamp != original

    for compiler in compilers:
        good = compile_and_run(compiler, original, tmp_path / Path(compiler).name, member_type=member_type)
        assert good.returncode == 0, good.stdout + good.stderr

        wrong = compile_and_run(compiler, wrong_decrement, tmp_path / (Path(compiler).name + "-wrong"))
        assert wrong.returncode != 0, f"{compiler}: wrong-decrement mutant survived"

        unclamped = compile_and_run(compiler, missing_clamp, tmp_path / (Path(compiler).name + "-unclamped"))
        assert unclamped.returncode != 0, f"{compiler}: missing-clamp mutant survived"

        unsigned = compile_and_run(
            compiler,
            original,
            tmp_path / (Path(compiler).name + "-unsigned-member"),
            member_type="unsigned int",
            warnings_as_errors=False,
        )
        assert unsigned.returncode != 0, f"{compiler}: unsigned-member mutant survived"
