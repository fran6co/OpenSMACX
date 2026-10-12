"""Host controls for the actual Matrix3 value-result contracts."""

from pathlib import Path
import shutil
import subprocess

import pytest


ROOT = Path(__file__).resolve().parents[2]
SOURCE = ROOT / "src/matrix3.cpp"
HARNESS = r"""
static bool same(const Matrix3 &matrix, const float *expected) {
    for (int i = 0; i < 9; ++i)
        if (matrix.cells[i] != expected[i]) return false;
    return true;
}

int main() {
    Matrix3 left;
    Matrix3 right;
    for (int i = 0; i < 9; ++i) {
        left.cells[i] = (float)(i + 1);
        right.cells[i] = (float)(i + 10);
    }
    const float sum[9] = {11,13,15,17,19,21,23,25,27};
    Matrix3 separate = left.add(&right);
    if (!same(separate, sum)) return 1;
    left = left.add(&right);
    if (!same(left, sum)) return 2;
    for (int i = 0; i < 9; ++i) left.cells[i] = (float)(i + 1);
    right = left.add(&right);
    if (!same(right, sum)) return 3;

    const float descending[9] = {9,8,7,6,5,4,3,2,1};
    for (int i = 0; i < 9; ++i) {
        left.cells[i] = (float)(i + 1);
        right.cells[i] = descending[i];
    }
    const float product[9] = {30,24,18,84,69,54,138,114,90};
    separate = left.multiply(&right);
    if (!same(separate, product)) return 4;
    left = left.multiply(&right);
    if (!same(left, product)) return 5;
    for (int i = 0; i < 9; ++i) left.cells[i] = (float)(i + 1);
    right = left.multiply(&right);
    if (!same(right, product)) return 6;

    for (int i = 0; i < 9; ++i) left.cells[i] = (float)(i + 1);
    const float tripled[9] = {3,6,9,12,15,18,21,24,27};
    separate = left.scaled(3.0f);
    if (!same(separate, tripled)) return 7;
    left = left.scaled(3.0f);
    if (!same(left, tripled)) return 8;
    return 0;
}
"""


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
def test_value_results_assign_to_distinct_and_aliased_objects(tmp_path, compiler_name):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(f"{compiler_name} not installed")
    unit = tmp_path / "matrix3_value_results.cpp"
    executable = tmp_path / "matrix3_value_results"
    unit.write_text(SOURCE.read_text() + "\n" + HARNESS)
    built = subprocess.run(
        [compiler, "-std=c++98", "-Wall", "-Wextra", "-Werror", "-pedantic",
         "-iquote", str(ROOT / "src"), str(unit), "-o", str(executable)],
        text=True, capture_output=True, timeout=30)
    assert built.returncode == 0, built.stderr
    ran = subprocess.run([str(executable)], text=True, capture_output=True, timeout=10)
    assert ran.returncode == 0, ran.stdout + ran.stderr
