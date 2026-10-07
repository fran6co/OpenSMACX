"""Actual-source controls for the consolidated numerical receiver view.

Compile the installed matrix3.cpp plus a bounded host harness. These controls
do not prove original class ownership or native x87 precision equivalence.
"""

from __future__ import annotations

import shutil
import subprocess
from pathlib import Path

import pytest


ROOT = Path(__file__).resolve().parents[2]
SOURCE = ROOT / "src" / "matrix3.cpp"

HARNESS = r"""
static void fill(Matrix3ReceiverView *m, float first) {
  for (int i = 0; i < 9; ++i) m->cells[i] = first + (float)i;
}
static int same(const Matrix3ReceiverView *m, const float *expected) {
  for (int i = 0; i < 9; ++i)
    if (m->cells[i] != expected[i]) return 0;
  return 1;
}
struct GuardedMatrix {
  unsigned before;
  Matrix3ReceiverView value;
  unsigned after;
};
int main() {
  const float ident[9] = {1,0,0,0,1,0,0,0,1};
  const float a[9] = {1,2,3,4,5,6,7,8,9};
  const float b[9] = {9,8,7,6,5,4,3,2,1};
  const float ab[9] = {30,24,18,84,69,54,138,114,90};
  const float sum[9] = {10,12,14,16,18,20,22,24,26};
  const float diff[9] = {-8,-6,-4,-2,0,2,4,6,8};
  const float doubled[9] = {2,4,6,8,10,12,14,16,18};
  const float twice_sum[9] = {20,24,28,32,36,40,44,48,52};
  const float square[9] = {30,36,42,66,81,96,102,126,150};
  const float zero[9] = {0,0,0,0,0,0,0,0,0};
  GuardedMatrix x, y, z;
  x.before = 0x12345678U; x.after = 0x87654321U;
  y.before = z.before = 0xa5a5a5a5U;
  y.after = z.after = 0x5a5a5a5aU;

  fill(&x.value, 7.0f);
  if (x.value.sub_6346c0() != &x.value || !same(&x.value, ident)) return 1;
  x.value.sub_6346f0();
  if (!same(&x.value, ident)) return 2;
  if (x.value.sub_634b70() != &x.value || !same(&x.value, ident)) return 3;

  fill(&x.value, 1.0f); fill(&y.value, 9.0f);
  x.value.sub_634720(&z.value, &y.value);
  if (!same(&z.value, sum) || !same(&x.value, a)) return 4;
  x.value.sub_634720(&x.value, &y.value);
  if (!same(&x.value, sum)) return 5;
  fill(&x.value, 1.0f); fill(&y.value, 9.0f);
  x.value.sub_634720(&y.value, &y.value);
  if (!same(&y.value, sum) || !same(&x.value, a)) return 24;
  fill(&x.value, 1.0f); fill(&y.value, 9.0f);
  x.value.sub_6347c0(&y.value);
  if (!same(&x.value, sum)) return 6;
  fill(&x.value, 1.0f);
  x.value.sub_6347c0(&x.value);
  if (!same(&x.value, doubled)) return 7;
  fill(&x.value, 1.0f);
  for (int i = 0; i < 9; ++i) y.value.cells[i] = 9.0f - (float)i;
  x.value.sub_6348f0(&y.value);
  if (!same(&x.value, diff)) return 8;
  fill(&x.value, 1.0f); x.value.sub_6348f0(&x.value);
  if (!same(&x.value, zero)) return 9;
  fill(&x.value, 1.0f); x.value.sub_6348c0(2.0f);
  if (!same(&x.value, doubled)) return 10;
  fill(&x.value, 1.0f);
  if (!x.value.equals(a)) return 11;
  x.value.cells[8] = 10.0f;
  if (x.value.equals(a)) return 12;

  fill(&x.value, 1.0f); fill(&y.value, 9.0f);
  y.value.cells[0] = 9; y.value.cells[1] = 8; y.value.cells[2] = 7;
  y.value.cells[3] = 6; y.value.cells[4] = 5; y.value.cells[5] = 4;
  y.value.cells[6] = 3; y.value.cells[7] = 2; y.value.cells[8] = 1;
  x.value.multiply(z.value.cells, y.value.cells);
  if (!same(&z.value, ab)) return 13;
  fill(&x.value, 1.0f); fill(&y.value, 9.0f);
  for (int i = 0; i < 9; ++i) y.value.cells[i] = 9.0f - (float)i;
  x.value.multiply(x.value.cells, y.value.cells);
  if (!same(&x.value, ab)) return 14;
  fill(&x.value, 1.0f); fill(&y.value, 9.0f);
  y.value.cells[0] = 9; y.value.cells[1] = 8; y.value.cells[2] = 7;
  y.value.cells[3] = 6; y.value.cells[4] = 5; y.value.cells[5] = 4;
  y.value.cells[6] = 3; y.value.cells[7] = 2; y.value.cells[8] = 1;
  x.value.multiply(y.value.cells, y.value.cells);
  if (!same(&y.value, ab) || !same(&x.value, a)) return 25;
  fill(&x.value, 1.0f); fill(&y.value, 9.0f);
  for (int i = 0; i < 9; ++i) y.value.cells[i] = 9.0f - (float)i;
  x.value.sub_6347f0(&y.value);
  if (!same(&x.value, ab)) return 15;
  fill(&x.value, 1.0f); x.value.sub_6347f0(&x.value);
  if (!same(&x.value, square)) return 16;

  fill(&x.value, 1.0f);
  const float input[3] = {2,3,5};
  float output[3] = {-1,-1,-1};
  x.value.transform(output, input);
  if (output[0] != 23 || output[1] != 53 || output[2] != 83) return 17;
  float alias[3] = {2,3,5};
  x.value.transform(alias, alias);
  if (alias[0] != 23 || alias[1] != 53 || alias[2] != 83) return 18;
  fill(&x.value, 1.0f);
  x.value.transform(x.value.cells, input);
  if (x.value.cells[0] != 23 || x.value.cells[1] != 53 || x.value.cells[2] != 83) return 26;
  fill(&x.value, 1.0f);
  float scaled[9];
  x.value.scale_into(scaled, 2.0f);
  for (int i = 0; i < 9; ++i) if (scaled[i] != 2.0f * (float)(i + 1)) return 19;
  x.value.scale_into(x.value.cells, 2.0f);
  for (int i = 0; i < 9; ++i)
    if (x.value.cells[i] != 2.0f * (float)(i + 1)) return 20;
  if (x.before != 0x12345678U || x.after != 0x87654321U) return 21;
  if (y.before != 0xa5a5a5a5U || y.after != 0x5a5a5a5aU) return 22;
  if (z.before != 0xa5a5a5a5U || z.after != 0x5a5a5a5aU) return 23;
  return 0;
}
"""


def _compiler(name: str) -> str:
    path = shutil.which(name)
    if path is None:
        pytest.skip(f"{name} is unavailable")
    return path


def _body_span(text: str, method: str) -> tuple[int, int]:
    import re

    masked = list(text)
    # Mask comments and literals before locating a source definition.
    i = 0
    state = "code"
    quote = ""
    while i < len(text):
        if state == "code":
            if text.startswith("//", i):
                masked[i:i + 2] = [" ", " "]
                i += 2; state = "line"; continue
            if text.startswith("/*", i):
                masked[i:i + 2] = [" ", " "]
                i += 2; state = "block"; continue
            if text[i] in "\"'":
                quote = text[i]; masked[i] = " "; i += 1; state = "literal"; continue
            i += 1; continue
        if state == "line":
            if text[i] == "\n": state = "code"
            else: masked[i] = " "
            i += 1; continue
        if state == "block":
            if text.startswith("*/", i):
                masked[i:i + 2] = [" ", " "]
                i += 2; state = "code"; continue
            if text[i] != "\n": masked[i] = " "
            i += 1; continue
        if state == "literal":
            if text[i] == "\\":
                masked[i] = " "
                if i + 1 < len(text) and text[i + 1] != "\n": masked[i + 1] = " "
                i += 2; continue
            if text[i] == quote:
                masked[i] = " "; i += 1; state = "code"; continue
            if text[i] != "\n": masked[i] = " "
            i += 1
    source = "".join(masked)
    hits = list(re.finditer(r"Matrix3ReceiverView\s*::\s*" + re.escape(method) + r"\s*\(", source))
    if len(hits) != 1:
        raise AssertionError(f"expected one actual definition of {method}, found {len(hits)}")
    match = hits[0]
    opening = source.find("{", match.end())
    if opening < 0:
        raise AssertionError(f"missing body: {method}")
    depth = 0
    for i in range(opening, len(source)):
        if source[i] == "{": depth += 1
        elif source[i] == "}":
            depth -= 1
            if depth == 0: return opening, i
    raise AssertionError(f"unclosed body: {method}")


def _mutate(text: str, method: str, before: str, after: str) -> str:
    opening, closing = _body_span(text, method)
    body = text[opening + 1:closing]
    if body.count(before) != 1:
        raise AssertionError(f"mutation anchor must occur once in {method}: {before!r}")
    return text[:opening + 1] + body.replace(before, after, 1) + text[closing:]


def _compile_and_run(text: str, compiler: str, tmp_path: Path, label: str) -> subprocess.CompletedProcess[str]:
    unit = tmp_path / f"matrix3-{label}.cpp"
    binary = tmp_path / f"matrix3-{label}"
    unit.write_text(text + "\n" + HARNESS)
    subprocess.run([compiler, "-std=c++98", "-Wall", "-Wextra", "-pedantic",
                    str(unit), "-o", str(binary)], check=True, cwd=ROOT,
                   text=True, capture_output=True)
    return subprocess.run([str(binary)], cwd=ROOT, text=True, capture_output=True)


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
def test_consolidated_actual_source_owner(compiler_name: str, tmp_path: Path) -> None:
    text = SOURCE.read_text()
    for method in ("sub_6346c0", "sub_6346f0", "sub_634720", "sub_6347c0",
                   "sub_6347f0", "sub_6348c0", "sub_6348f0", "sub_634b70",
                   "equals", "multiply", "transform", "scale_into"):
        _body_span(text, method)
    result = _compile_and_run(text, _compiler(compiler_name), tmp_path, compiler_name)
    assert result.returncode == 0, result.stdout + result.stderr


def test_add_preserves_original_x87_operand_load_order() -> None:
    text = SOURCE.read_text()
    opening, closing = _body_span(text, "sub_634720")
    body = text[opening + 1:closing]
    def has_original_order(candidate: str) -> bool:
        return (candidate.count("other->cells[row * 3 + column] +") == 1 and
                candidate.count("cells[row * 3 + column];") == 1 and
                candidate.index("other->cells[row * 3 + column] +") <
                candidate.index("cells[row * 3 + column];"))
    assert has_original_order(body)
    reversed_body = body.replace(
        "other->cells[row * 3 + column] +\n                                        cells[row * 3 + column]",
        "cells[row * 3 + column] +\n                                        other->cells[row * 3 + column]",
    )
    assert reversed_body != body
    assert not has_original_order(reversed_body)


MUTANTS = (
    ("sub_6346c0", "cells[4] = 1.0f", "cells[4] = 0.0f"),
    ("sub_6347c0", "other->cells[row * 3 + column] +",
     "other->cells[row * 3 + column] -"),
    ("sub_6347f0", "other->cells[depth * 3 + column]",
     "other->cells[column * 3 + depth]"),
    ("sub_6348c0", "scale * cells[row * 3 + column]",
     "scale + cells[row * 3 + column]"),
    ("sub_6348f0", "cells[row * 3 + column] -",
     "cells[row * 3 + column] +"),
    ("equals", "!=", "=="),
    ("multiply", "other[depth * 3 + column]", "other[column * 3 + depth]"),
    ("multiply", "for (int i = 0; i < 9; ++i)", "for (int i = 0; i < 8; ++i)"),
    ("multiply", "out[i] = local[i];", "out[i] = out[i];"),
    ("transform", "m[1] * in[1]", "m[0] * in[1]"),
    ("scale_into", "scale * cells[i]", "scale + cells[i]"),
)


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
@pytest.mark.parametrize("method,before,after", MUTANTS)
def test_source_mutants_are_detected(
    compiler_name: str, method: str, before: str, after: str, tmp_path: Path
) -> None:
    original = SOURCE.read_text()
    mutated = _mutate(original, method, before, after)
    result = _compile_and_run(mutated, _compiler(compiler_name), tmp_path,
                              f"mutant-{method}-{compiler_name}")
    assert result.returncode != 0, f"{method} mutation escaped semantic control"
