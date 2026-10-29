"""Focused host controls for Matrix3 transform input alias and result staging.

Compile the installed implementation itself. These cases cover bounded
float-array behavior only; they do not prove original ownership, extent,
lifetime, callers, or x87 precision equivalence.
"""

from __future__ import annotations

import shutil
import subprocess
from pathlib import Path

import pytest


ROOT = Path(__file__).resolve().parents[2]
SOURCE = ROOT / "src" / "matrix3.cpp"

HARNESS = r"""
struct GuardedMatrix {
    unsigned before;
    Matrix3 value;
    unsigned after;
};
struct GuardedResult {
    unsigned before;
    Matrix3TransformResult value;
    unsigned after;
};

int main() {
    const unsigned before = 0x12345678U;
    const unsigned after = 0x87654321U;
    GuardedMatrix guarded = {before, {{1,2,3,4,5,6,7,8,9}}, after};

    // The input aliases the matrix storage. The returned value has separate
    // storage, and the receiver must remain unchanged.
    GuardedResult transformed;
    transformed.before = 0x0badc0deU;
    transformed.after = 0xfeedfaceU;
    transformed.value = guarded.value.transform(guarded.value.cells);
    if (transformed.value.x_ != 14.0f ||
        transformed.value.y_ != 32.0f ||
        transformed.value.z_ != 50.0f) return 1;
    for (int i = 0; i < 9; ++i)
        if (guarded.value.cells[i] != (float)(i + 1)) return 2;
    if (transformed.before != 0x0badc0deU ||
        transformed.after != 0xfeedfaceU) return 3;
    if (guarded.before != before || guarded.after != after) return 4;

    // All three original row groupings cancel the large pair before adding
    // the unit term. Reassociation loses that final one on float hosts.
    const float huge = 1.0e30f;
    guarded.value.cells[0] = 1.0f;
    guarded.value.cells[1] = huge;
    guarded.value.cells[2] = -huge;
    guarded.value.cells[3] = -huge;
    guarded.value.cells[4] = huge;
    guarded.value.cells[5] = 1.0f;
    guarded.value.cells[6] = -huge;
    guarded.value.cells[7] = huge;
    guarded.value.cells[8] = 1.0f;
    const float ones[3] = {1.0f, 1.0f, 1.0f};
    transformed.value = guarded.value.transform(ones);
    if (transformed.value.x_ != 1.0f ||
        transformed.value.y_ != 1.0f ||
        transformed.value.z_ != 1.0f) return 5;
    if (transformed.before != 0x0badc0deU ||
        transformed.after != 0xfeedfaceU) return 6;
    if (guarded.before != before || guarded.after != after) return 7;

    // Assignment from a value-returning operation must stage the complete
    // product before replacing its aliased receiver.
    guarded.value.cells[0] = 1.0f; guarded.value.cells[1] = 2.0f;
    guarded.value.cells[2] = 3.0f; guarded.value.cells[3] = 4.0f;
    guarded.value.cells[4] = 5.0f; guarded.value.cells[5] = 6.0f;
    guarded.value.cells[6] = 7.0f; guarded.value.cells[7] = 8.0f;
    guarded.value.cells[8] = 9.0f;
    guarded.value = guarded.value.multiply(&guarded.value);
    const float square[9] = {30,36,42,66,81,96,102,126,150};
    for (int i = 0; i < 9; ++i)
        if (guarded.value.cells[i] != square[i]) return 5;
    if (guarded.before != before || guarded.after != after) return 6;
    return 0;
}
"""


def _compiler(name: str) -> str:
    path = shutil.which(name)
    if path is None:
        pytest.skip(f"{name} is unavailable")
    return path


def _source_with_mutant(source: str, label: str) -> str:
    mutations = {
        "reassociate_row0": (
            "result.x_ = m[1] * in[1] + m[2] * in[2] + m[0] * in[0];",
            "result.x_ = m[1] * in[1] + (m[2] * in[2] + m[0] * in[0]);",
        ),
        "reassociate_row1": (
            "result.y_ = m[4] * in[1] + m[3] * in[0] + m[5] * in[2];",
            "result.y_ = m[4] * in[1] + (m[3] * in[0] + m[5] * in[2]);",
        ),
        "reassociate_row2": (
            "result.z_ = m[7] * in[1] + m[6] * in[0] + m[8] * in[2];",
            "result.z_ = m[7] * in[1] + (m[6] * in[0] + m[8] * in[2]);",
        ),
        "zero_result_y": (
            "result.y_ = m[4] * in[1] + m[3] * in[0] + m[5] * in[2];",
            "result.y_ = 0.0f;",
        ),
        "zero_result_z": (
            "result.z_ = m[7] * in[1] + m[6] * in[0] + m[8] * in[2];",
            "result.z_ = 0.0f;",
        ),
    }
    before, after = mutations[label]
    assert source.count(before) == 1
    return source.replace(before, after, 1)


def _compile_and_run(source: str, compiler: str, tmp_path: Path, label: str) -> int:
    unit = tmp_path / f"matrix3-{label}.cpp"
    executable = tmp_path / f"matrix3-{label}"
    unit.write_text(source + "\n" + HARNESS)
    subprocess.run(
        [compiler, "-std=c++98", "-Wall", "-Wextra", "-pedantic",
         "-iquote", str(ROOT / "src"), str(unit), "-o", str(executable)],
        check=True, cwd=ROOT, text=True, capture_output=True, timeout=30,
    )
    return subprocess.run(
        [str(executable)], cwd=ROOT, text=True, capture_output=True, timeout=10,
    ).returncode


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
def test_matrix3_loop_cohort_actual_source(compiler_name: str, tmp_path: Path) -> None:
    result = _compile_and_run(
        SOURCE.read_text(), _compiler(compiler_name), tmp_path, compiler_name,
    )
    assert result == 0, f"actual-source control exited {result}"


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
@pytest.mark.parametrize(
    "mutant", (
        "reassociate_row0", "reassociate_row1", "reassociate_row2",
        "zero_result_y", "zero_result_z",
    ),
)
def test_matrix3_loop_cohort_rejects_compiled_mutants(
    compiler_name: str, mutant: str, tmp_path: Path,
) -> None:
    source = _source_with_mutant(SOURCE.read_text(), mutant)
    result = _compile_and_run(source, _compiler(compiler_name), tmp_path, mutant)
    assert result != 0, f"compiled {mutant} mutant escaped the behavioral controls"
