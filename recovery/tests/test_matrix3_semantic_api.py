"""Host behavior controls for the chosen mathematical Matrix3 source API.

These checks exercise the actual C++ source. They do not establish original
class ownership, complete object extent, lifetime, caller set, ABI, or x87
precision equivalence.
"""

from pathlib import Path
import shutil
import subprocess

import pytest


ROOT = Path(__file__).resolve().parents[2]
SOURCE = ROOT / "src" / "matrix3.cpp"
HARNESS = r"""
#include <cmath>
#include <limits>
#include "matrix3.h"

static void fill(Matrix3 *m, float first) {
    for (int i = 0; i < 9; ++i) m->cells[i] = first + (float)i;
}
static void descending(Matrix3 *m) {
    for (int i = 0; i < 9; ++i) m->cells[i] = 9.0f - (float)i;
}
static bool same(const Matrix3 *m, const float *values) {
    for (int i = 0; i < 9; ++i) if (m->cells[i] != values[i]) return false;
    return true;
}

int main() {
    const float identity[9] = {1,0,0,0,1,0,0,0,1};
    const float a[9] = {1,2,3,4,5,6,7,8,9};
    const float b[9] = {9,8,7,6,5,4,3,2,1};
    const float sum[9] = {10,10,10,10,10,10,10,10,10};
    const float diff[9] = {-8,-6,-4,-2,0,2,4,6,8};
    const float product[9] = {30,24,18,84,69,54,138,114,90};
    Matrix3 x, y, out;
    if (x.identity() != &x || !same(&x, identity)) return 1;
    fill(&x, 4); x.reset_identity();
    if (!same(&x, identity)) return 2;
    fill(&x, 3); if (x.write_identity() != &x || !same(&x, identity)) return 3;

    fill(&x, 1); descending(&y);
    out = x.add(&y);
    if (!same(&out, sum)) return 4;
    if (!same(&x, a) || !same(&y, b)) return 5;
    x = x.add(&y);
    const float added[9] = {10,10,10,10,10,10,10,10,10};
    if (!same(&x, added)) return 6;
    fill(&x, 1); descending(&y); y = x.add(&y);
    if (!same(&y, added) || !same(&x, a)) return 23;
    fill(&x, 1); descending(&y); x.add_assign(&y);
    if (!same(&x, added)) return 7;

    fill(&x, 1); descending(&y); x.subtract_assign(&y);
    if (!same(&x, diff)) return 8;
    fill(&x, 1); x.subtract_assign(&x);
    const float zero[9] = {0,0,0,0,0,0,0,0,0};
    if (!same(&x, zero)) return 9;

    fill(&x, 1); descending(&y);
    out = x.multiply(&y);
    if (!same(&out, product)) return 10;
    fill(&x, 1); descending(&y);
    x = x.multiply(&y);
    if (!same(&x, product)) return 24;
    fill(&x, 1); descending(&y);
    y = x.multiply(&y);
    if (!same(&y, product) || !same(&x, a)) return 25;
    fill(&x, 1); descending(&y); x.multiply_assign(&y);
    if (!same(&x, product)) return 11;
    fill(&x, 1); x.multiply_assign(&x);
    const float square[9] = {30,36,42,66,81,96,102,126,150};
    if (!same(&x, square)) return 12;

    fill(&x, 1); x.scale_assign(2.0f);
    const float doubled[9] = {2,4,6,8,10,12,14,16,18};
    if (!same(&x, doubled)) return 13;
    fill(&x, 1);
    out = x.scaled(2.0f);
    if (!same(&out, doubled)) return 14;
    fill(&x, 1); x = x.scaled(2.0f);
    if (!same(&x, doubled)) return 15;

    fill(&x, 1);
    if (!x.equals(a)) return 16;
    x.cells[8] = 10.0f; if (x.equals(a)) return 17;
    const float nan = std::numeric_limits<float>::quiet_NaN();
    fill(&x, 1); x.cells[4] = nan;
    if (!x.equals(a)) return 18;
    fill(&x, 1); x.cells[4] = std::numeric_limits<float>::infinity();
    float with_infinity[9];
    for (int i = 0; i < 9; ++i) with_infinity[i] = a[i];
    with_infinity[4] = std::numeric_limits<float>::infinity();
    if (!x.equals(with_infinity)) return 26;
    x.cells[4] = 1.0f; if (x.equals(with_infinity)) return 27;
    float with_nan[9];
    for (int i = 0; i < 9; ++i) with_nan[i] = a[i];
    with_nan[2] = nan; fill(&x, 1);
    if (!x.equals(with_nan)) return 19;

    fill(&x, 1);
    const float input[3] = {2,3,5};
    float transformed[3] = {-1,-1,-1};
    x.transform(transformed, input);
    if (transformed[0] != 23 || transformed[1] != 53 || transformed[2] != 83) return 20;
    float aliased[3] = {2,3,5}; x.transform(aliased, aliased);
    if (aliased[0] != 23 || aliased[1] != 53 || aliased[2] != 83) return 21;
    fill(&x, 1); x.transform(x.cells, input);
    if (x.cells[0] != 23 || x.cells[1] != 53 || x.cells[2] != 83) return 28;

    // Exercise finite edge behavior without promising a particular rounding mode.
    fill(&x, 0.0f); x.cells[0] = std::numeric_limits<float>::max();
    x.scale_assign(2.0f);
    if (!std::isinf(x.cells[0])) return 22;
    return 0;
}
"""


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
def test_typed_matrix3_api_against_actual_source(tmp_path: Path, compiler_name: str) -> None:
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(f"{compiler_name} is unavailable")
    unit = tmp_path / "matrix3_semantic_api.cpp"
    executable = tmp_path / "matrix3_semantic_api"
    unit.write_text(HARNESS)
    built = subprocess.run(
        [compiler, "-std=c++98", "-Wall", "-Wextra", "-Werror", "-pedantic",
         "-iquote", str(ROOT / "src"), str(unit), str(SOURCE), "-o", str(executable)],
        text=True, capture_output=True, timeout=30)
    assert built.returncode == 0, built.stderr
    ran = subprocess.run([str(executable)], text=True, capture_output=True, timeout=10)
    assert ran.returncode == 0, ran.stdout + ran.stderr
