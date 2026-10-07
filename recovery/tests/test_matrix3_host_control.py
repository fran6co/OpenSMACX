"""Compile and mutation-check the actual bounded Matrix3ReceiverView source.

This test reads the product-owned matrix3.cpp implementation and appends a
host harness to that actual source text; it has no emitted-source fallback.
"""

from __future__ import annotations

import re
import shutil
import subprocess
from pathlib import Path

import pytest


ROOT = Path(__file__).resolve().parents[2]
SOURCE = ROOT / "src/matrix3.cpp"

HARNESS = r"""
struct GuardedMatrix {
    unsigned int before;
    Matrix3ReceiverView value;
    unsigned int after;
};

static void fill(Matrix3ReceiverView *matrix, float first) {
    for (int row = 0; row < 3; ++row) {
        for (int column = 0; column < 3; ++column) {
            matrix->cells[row * 3 + column] = first + (float)(row * 3 + column);
        }
    }
}

static int equals(const Matrix3ReceiverView *matrix, const float expected[9]) {
    for (int index = 0; index < 9; ++index) {
        if (matrix->cells[index] != expected[index]) return 0;
    }
    return 1;
}

int main() {
    const float identity[9] = {1, 0, 0, 0, 1, 0, 0, 0, 1};
    const float original_left[9] = {1, 2, 3, 4, 5, 6, 7, 8, 9};
    GuardedMatrix identity_case;
    identity_case.before = 0x12345678U;
    identity_case.after = 0x87654321U;
    fill(&identity_case.value, 7.0f);
    if (identity_case.value.sub_6346c0() != &identity_case.value) return 1;
    if (!equals(&identity_case.value, identity)) return 2;
    if (identity_case.before != 0x12345678U || identity_case.after != 0x87654321U) return 3;
    fill(&identity_case.value, 4.0f);
    identity_case.value.sub_6346f0();
    if (!equals(&identity_case.value, identity)) return 4;
    fill(&identity_case.value, 4.0f);
    if (identity_case.value.sub_634b70() != &identity_case.value) return 5;
    if (!equals(&identity_case.value, identity)) return 6;
    if (identity_case.before != 0x12345678U || identity_case.after != 0x87654321U) return 7;

    const float sum[9] = {11, 13, 15, 17, 19, 21, 23, 25, 27};
    const float difference[9] = {-9, -9, -9, -9, -9, -9, -9, -9, -9};
    const float doubled[9] = {2, 4, 6, 8, 10, 12, 14, 16, 18};
    const float zero[9] = {0, 0, 0, 0, 0, 0, 0, 0, 0};
    GuardedMatrix left;
    GuardedMatrix right;
    GuardedMatrix output;
    left.before = right.before = output.before = 0xa5a5a5a5U;
    left.after = right.after = output.after = 0x5a5a5a5aU;
    fill(&left.value, 1.0f);
    fill(&right.value, 10.0f);
    left.value.sub_6347c0(&right.value);
    if (!equals(&left.value, sum)) return 8;
    if (left.before != 0xa5a5a5a5U || left.after != 0x5a5a5a5aU) return 9;
    fill(&left.value, 1.0f);
    left.value.sub_6347c0(&left.value);
    if (!equals(&left.value, doubled)) return 10;

    fill(&left.value, 1.0f);
    fill(&right.value, 10.0f);
    left.value.sub_6348f0(&right.value);
    if (!equals(&left.value, difference)) return 11;
    fill(&left.value, 1.0f);
    left.value.sub_6348f0(&left.value);
    if (!equals(&left.value, zero)) return 12;

    fill(&left.value, 1.0f);
    left.value.sub_6348c0(2.0f);
    if (!equals(&left.value, doubled)) return 13;

    const float product[9] = {30, 24, 18, 84, 69, 54, 138, 114, 90};
    const float add_nine[9] = {10, 12, 14, 16, 18, 20, 22, 24, 26};
    const float square[9] = {30, 36, 42, 66, 81, 96, 102, 126, 150};
    fill(&left.value, 1.0f);
    fill(&right.value, 9.0f);
    left.value.sub_634720(&output.value, &right.value);
    if (!equals(&output.value, add_nine)) return 14;
    if (!equals(&left.value, original_left)) return 15;

    fill(&left.value, 1.0f);
    fill(&right.value, 10.0f);
    left.value.sub_634720(&left.value, &right.value);
    if (!equals(&left.value, sum)) return 16;
    fill(&left.value, 1.0f);
    fill(&right.value, 10.0f);
    left.value.sub_634720(&right.value, &right.value);
    if (!equals(&right.value, sum)) return 17;

    fill(&left.value, 1.0f);
    fill(&right.value, 9.0f);
    for (int index = 0; index < 9; ++index) {
        right.value.cells[index] = 9.0f - (float)index;
    }
    left.value.sub_6347f0(&right.value);
    if (!equals(&left.value, product)) return 18;
    fill(&left.value, 1.0f);
    left.value.sub_6347f0(&left.value);
    if (!equals(&left.value, square)) return 19;
    if (left.before != 0xa5a5a5a5U || left.after != 0x5a5a5a5aU) return 20;
    if (right.before != 0xa5a5a5a5U || right.after != 0x5a5a5a5aU) return 21;
    if (output.before != 0xa5a5a5a5U || output.after != 0x5a5a5a5aU) return 22;
    return 0;
}
"""


def _compiler(name: str) -> str:
    path = shutil.which(name)
    if path is None:
        pytest.skip(f"{name} is not installed")
    return path


def _compile_and_run(compiler: str, source: Path, harness: Path, output: Path) -> subprocess.CompletedProcess[str]:
    unit = harness.with_name("matrix3_actual_source_unit.cpp")
    unit.write_text(source.read_text() + "\n" + harness.read_text())
    subprocess.run(
        [compiler, "-std=c++98", "-Wall", "-Wextra", "-Werror", "-pedantic",
         str(unit), "-o", str(output)],
        check=True, cwd=ROOT, text=True, capture_output=True,
    )
    return subprocess.run([str(output)], cwd=ROOT, text=True, capture_output=True)


def _code_mask(text: str) -> str:
    """Blank comments and literals while preserving offsets for source anchors."""
    result = list(text)
    index = 0
    state = "code"
    quote = ""
    while index < len(text):
        if state == "code":
            if text.startswith("//", index):
                result[index:index + 2] = "  "
                index += 2
                state = "line"
                continue
            if text.startswith("/*", index):
                result[index:index + 2] = "  "
                index += 2
                state = "block"
                continue
            if text[index] in "\"'":
                quote = text[index]
                result[index] = " "
                index += 1
                state = "literal"
                continue
            index += 1
            continue
        if state == "line":
            if text[index] == "\n":
                state = "code"
            else:
                result[index] = " "
            index += 1
            continue
        if state == "block":
            if text.startswith("*/", index):
                result[index:index + 2] = "  "
                index += 2
                state = "code"
            else:
                if text[index] != "\n":
                    result[index] = " "
                index += 1
            continue
        if state == "literal":
            if text[index] == "\\":
                result[index] = " "
                if index + 1 < len(text) and text[index + 1] != "\n":
                    result[index + 1] = " "
                index += 2
            elif text[index] == quote:
                result[index] = " "
                index += 1
                state = "code"
            else:
                if text[index] != "\n":
                    result[index] = " "
                index += 1
    return "".join(result)


def _method_body(text: str, name: str) -> tuple[int, int]:
    masked = _code_mask(text)
    hits = list(re.finditer(r"Matrix3ReceiverView\s*::\s*" + re.escape(name) + r"\s*\(", masked))
    if len(hits) != 1:
        raise AssertionError(f"expected one code definition for {name}, found {len(hits)}")
    opening = masked.find("{", hits[0].end())
    if opening < 0:
        raise AssertionError(f"no body for {name}")
    depth = 0
    for position in range(opening, len(masked)):
        if masked[position] == "{":
            depth += 1
        elif masked[position] == "}":
            depth -= 1
            if depth == 0:
                return opening, position
    raise AssertionError(f"unterminated body for {name}")


def _mutate(text: str, method: str, before: str, after: str) -> str:
    opening, closing = _method_body(text, method)
    body = text[opening + 1:closing]
    if before not in body:
        raise AssertionError(f"mutation anchor missing from {method}: {before!r}")
    if body.count(before) != 1:
        raise AssertionError(f"mutation anchor is ambiguous in {method}: {before!r}")
    mutated = body.replace(before, after, 1)
    return text[:opening + 1] + mutated + text[closing:]


COMPILERS = ("g++", "clang++")


@pytest.mark.parametrize("compiler_name", COMPILERS)
def test_actual_tracked_source_host_control(tmp_path: Path, compiler_name: str) -> None:
    assert SOURCE.is_file(), "integrate src/matrix3.cpp before promoting this control"
    harness = tmp_path / "matrix3_host_control.cpp"
    harness.write_text(HARNESS)
    result = _compile_and_run(_compiler(compiler_name), SOURCE, harness,
                              tmp_path / f"matrix3-{compiler_name}")
    assert result.returncode == 0, result.stdout + result.stderr


@pytest.mark.parametrize("compiler_name", COMPILERS)
@pytest.mark.parametrize(
    "method,before,after",
    (
        ("sub_6346c0", "cells[4] = 1.0f", "cells[4] = 0.0f"),
        ("sub_6347c0", "other->cells[row * 3 + column] +", "other->cells[row * 3 + column] -"),
        ("sub_6348f0", "cells[row * 3 + column] -", "cells[row * 3 + column] +"),
        ("sub_6348c0", "scale * cells[row * 3 + column]", "scale + cells[row * 3 + column]"),
        ("sub_6347f0", "other->cells[depth * 3 + column]", "other->cells[column * 3 + depth]"),
    ),
)
def test_meaningful_source_mutants_fail(
    tmp_path: Path, compiler_name: str, method: str, before: str, after: str
) -> None:
    original = SOURCE.read_text()
    mutated = _mutate(original, method, before, after)
    mutant_source = tmp_path / f"{method}-mutant.cpp"
    mutant_source.write_text(mutated)
    harness = tmp_path / "matrix3_host_control.cpp"
    harness.write_text(HARNESS)
    result = _compile_and_run(_compiler(compiler_name), mutant_source, harness,
                              tmp_path / f"mutant-{method}-{compiler_name}")
    assert result.returncode != 0, f"the {method} source mutant escaped the harness"


def test_add_source_order_is_part_of_original_witness() -> None:
    text = SOURCE.read_text()
    body_start, body_end = _method_body(text, "sub_634720")
    body = text[body_start + 1:body_end]
    source_order = r"other->cells\[row \* 3 \+ column\]\s*\+\s*cells\[row \* 3 \+ column\]"
    reversed_order = r"cells\[row \* 3 \+ column\]\s*\+\s*other->cells\[row \* 3 \+ column\]"
    assert len(re.findall(source_order, body)) == 1
    assert not re.search(reversed_order, body)
    mutated = re.sub(source_order, "cells[row * 3 + column] + other->cells[row * 3 + column]", body)
    assert not re.search(source_order, mutated)


def test_mutation_anchors_ignore_lexical_decoys() -> None:
    decoy = "/* lexical decoy: Matrix3ReceiverView::sub_6348f0() { cells[i] + other->cells[i]; } */\n"
    source = decoy + SOURCE.read_text()
    mutated = _mutate(source, "sub_6348f0", "cells[row * 3 + column] -",
                      "cells[row * 3 + column] +")
    assert mutated.startswith(decoy), "the lexical decoy must remain untouched"
    assert mutated != source
