"""Host controls for the actual bounded matrix output-pointer contracts.

Compile the entire product matrix3.cpp without rewriting its implementation.
These controls do not establish original ownership, hidden class-result semantics,
VC6 ABI emission, or floating-point equivalence with the pinned executable.
"""

from pathlib import Path
import shutil
import subprocess

import pytest


ROOT = Path(__file__).resolve().parents[2]
SOURCE = ROOT / "src/matrix3.cpp"
HARNESS = r"""
int main() {
    Matrix3ReceiverView left;
    Matrix3ReceiverView right;
    Matrix3ReceiverView out;
    for (int i = 0; i < 9; ++i) {
        left.cells[i] = (float)(i + 1);
        right.cells[i] = (float)(i + 10);
        out.cells[i] = -99.0f;
    }

    const float sum[9] = {11,13,15,17,19,21,23,25,27};
    if (left.sub_634720(&out, &right) != &out) return 1;
    for (int i = 0; i < 9; ++i) if (out.cells[i] != sum[i]) return 2;

    // The original routines stage into local storage before copying out; keep
    // valid alias cases explicit and verify the EAX-source pointer contract.
    if (left.sub_634720(&left, &right) != &left) return 3;
    for (int i = 0; i < 9; ++i) if (left.cells[i] != sum[i]) return 4;
    if (left.sub_634720(&right, &right) != &right) return 5;
    for (int i = 0; i < 9; ++i) if (right.cells[i] != sum[i] + (float)(i + 10)) return 6;

    const float descending[9] = {9,8,7,6,5,4,3,2,1};
    for (int i = 0; i < 9; ++i) {
        left.cells[i] = (float)(i + 1);
        right.cells[i] = descending[i];
    }
    const float product[9] = {30,24,18,84,69,54,138,114,90};
    float separate[9];
    if (left.multiply(separate, &right.cells[0]) != separate) return 7;
    for (int i = 0; i < 9; ++i) if (separate[i] != product[i]) return 8;
    if (left.multiply(&left.cells[0], &right.cells[0]) != &left.cells[0]) return 9;
    for (int i = 0; i < 9; ++i) if (left.cells[i] != product[i]) return 10;

    for (int i = 0; i < 9; ++i) left.cells[i] = (float)(i + 1);
    if (left.scale_into(&left.cells[0], 3.0f) != &left.cells[0]) return 11;
    for (int i = 0; i < 9; ++i) if (left.cells[i] != 3.0f * (float)(i + 1)) return 12;
    float scaled[9];
    if (left.scale_into(scaled, 2.0f) != scaled) return 13;
    for (int i = 0; i < 9; ++i) if (scaled[i] != 6.0f * (float)(i + 1)) return 14;
    return 0;
}
"""


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
def test_explicit_output_pointer_return_and_aliasing(tmp_path, compiler_name):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(f"{compiler_name} not installed")
    unit = tmp_path / "matrix3_pointer_return.cpp"
    executable = tmp_path / "matrix3_pointer_return"
    unit.write_text(SOURCE.read_text() + "\n" + HARNESS)
    built = subprocess.run(
        [compiler, "-std=c++98", "-Wall", "-Wextra", "-Werror", "-pedantic",
         str(unit), "-o", str(executable)],
        text=True, capture_output=True, timeout=30)
    assert built.returncode == 0, built.stderr
    ran = subprocess.run([str(executable)], text=True, capture_output=True, timeout=10)
    assert ran.returncode == 0, ran.stdout + ran.stderr


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
def test_missing_pointer_return_is_rejected(tmp_path, compiler_name):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(f"{compiler_name} not installed")
    original = SOURCE.read_text()
    for index, returned in enumerate(("destination", "out", "dest")):
        anchor = "return " + returned + ";"
        assert original.count(anchor) == 1
        damaged = original.replace(anchor, "return 0;", 1)
        unit = tmp_path / ("damaged_%d.cpp" % index)
        executable = tmp_path / ("damaged_%d" % index)
        unit.write_text(damaged + "\n" + HARNESS)
        built = subprocess.run(
            [compiler, "-std=c++98", "-Wall", "-Wextra", "-Werror", "-pedantic",
             str(unit), "-o", str(executable)],
            text=True, capture_output=True, timeout=30)
        assert built.returncode == 0, built.stderr
        ran = subprocess.run([str(executable)], text=True, capture_output=True, timeout=10)
        assert ran.returncode != 0, "missing %s result survived" % returned
