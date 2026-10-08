"""Bounded semantic controls for installed numerical matrix methods."""

from __future__ import annotations

import re
import shutil
import subprocess
from pathlib import Path

import pytest


ROOT = next(parent for parent in Path(__file__).resolve().parents
            if (parent / "src/matrix3.cpp").is_file())
SOURCE = ROOT / "src/matrix3.cpp"


def _method(text: str, signature: str) -> str:
    masked = list(text)
    # Ignore comments and literals when locating a signature, preserving offsets.
    index = 0
    state = "code"
    quote = ""
    while index < len(text):
        if state == "code":
            if text.startswith("//", index):
                masked[index:index + 2] = "  "
                index += 2
                state = "line"
            elif text.startswith("/*", index):
                masked[index:index + 2] = "  "
                index += 2
                state = "block"
            elif text[index] in "\"'":
                quote = text[index]
                masked[index] = " "
                index += 1
                state = "literal"
            else:
                index += 1
        elif state == "line":
            if text[index] == "\n":
                state = "code"
            else:
                masked[index] = " "
            index += 1
        elif state == "block":
            if text.startswith("*/", index):
                masked[index:index + 2] = "  "
                index += 2
                state = "code"
            else:
                if text[index] != "\n":
                    masked[index] = " "
                index += 1
        else:
            if text[index] == "\\":
                masked[index] = " "
                if index + 1 < len(text) and text[index + 1] != "\n":
                    masked[index + 1] = " "
                index += 2
            elif text[index] == quote:
                masked[index] = " "
                index += 1
                state = "code"
            else:
                if text[index] != "\n":
                    masked[index] = " "
                index += 1
    code = "".join(masked)
    hits = list(re.finditer(re.escape(signature), code))
    if len(hits) != 1:
        raise AssertionError(f"expected one actual definition of {signature!r}, got {len(hits)}")
    start = hits[0].start()
    opening = code.find("{", hits[0].end())
    if opening < 0:
        raise AssertionError(f"no body for {signature!r}")
    depth = 0
    for position in range(opening, len(code)):
        if code[position] == "{":
            depth += 1
        elif code[position] == "}":
            depth -= 1
            if depth == 0:
                return text[start:position + 1]
    raise AssertionError(f"unterminated actual definition of {signature!r}")


def _run(tmp_path: Path, compiler_name: str, main: str) -> None:
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(f"{compiler_name} is not installed")
    translation_unit = tmp_path / "actual_general_matrix_methods.cpp"
    translation_unit.write_text("#include <limits>\n" + SOURCE.read_text() + "\n" + main)
    executable = tmp_path / f"actual-general-{compiler_name}"
    subprocess.run(
        [compiler, "-std=c++98", "-Wall", "-Wextra", "-Werror", "-pedantic",
         str(translation_unit), "-o", str(executable)],
        cwd=ROOT, check=True, capture_output=True, text=True,
    )
    result = subprocess.run([str(executable)], cwd=ROOT,
                            capture_output=True, text=True)
    assert result.returncode == 0, result.stdout + result.stderr


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
def test_host_mat3_nan_result_is_not_an_x87_equivalence_oracle(
    tmp_path: Path, compiler_name: str
) -> None:
    # This extracts the installed definition, but host NaN behavior is only
    # characterized here; the VC6 runtime test owns x87 boolean behavior.
    _run(tmp_path, compiler_name, r"""
int main() {
    Matrix3ReceiverView matrix = {{1,2,3,4,5,6,7,8,9}};
    float other[9] = {1,2,3,4,5,6,7,8,9};
    const float inf = std::numeric_limits<float>::infinity();
    const float nan = std::numeric_limits<float>::quiet_NaN();
    for (int i = 0; i < 9; ++i) {
        if (!matrix.equals(other)) return 1;
        other[i] = matrix.cells[i] + 1.0f;
        if (matrix.equals(other)) return 2;
        other[i] = matrix.cells[i];

        matrix.cells[i] = 0.0f; other[i] = -0.0f;
        if (!matrix.equals(other)) return 3;
        matrix.cells[i] = -0.0f; other[i] = 0.0f;
        if (!matrix.equals(other)) return 4;
        matrix.cells[i] = 1.0f; other[i] = 1.0f;

        matrix.cells[i] = inf; other[i] = inf;
        if (!matrix.equals(other)) return 5;
        other[i] = -inf;
        if (matrix.equals(other)) return 6;
        matrix.cells[i] = 1.0f; other[i] = inf;
        if (matrix.equals(other)) return 7;
        matrix.cells[i] = inf; other[i] = 1.0f;
        if (matrix.equals(other)) return 8;

        matrix.cells[i] = nan; other[i] = 1.0f;
        if (!matrix.equals(other)) return 9;
        matrix.cells[i] = 1.0f; other[i] = nan;
        if (!matrix.equals(other)) return 10;
        matrix.cells[i] = nan; other[i] = nan;
        if (!matrix.equals(other)) return 11;
        matrix.cells[i] = other[i] = (float)(i + 1);
    }
    return 0;
}
""")

@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
def test_actual_matrix_transform_stages_all_reads_and_preserves_order(
    tmp_path: Path, compiler_name: str
) -> None:
    _run(tmp_path, compiler_name, r"""
int main() {
    Matrix3ReceiverView matrix = {{1,2,3,4,5,6,7,8,9}};
    const float input[3] = {2,3,4};
    float output[3] = {-1,-1,-1};
    matrix.transform(output, input);
    if (output[0] != 20 || output[1] != 47 || output[2] != 74) return 1;

    float aliased[3] = {2,3,4};
    matrix.transform(aliased, aliased);
    if (aliased[0] != 20 || aliased[1] != 47 || aliased[2] != 74) return 2;

    Matrix3ReceiverView matrix_output = {{1,2,3,4,5,6,7,8,9}};
    matrix_output.transform(matrix_output.cells, input);
    if (matrix_output.cells[0] != 20 || matrix_output.cells[1] != 47 ||
        matrix_output.cells[2] != 74) return 3;

    Matrix3ReceiverView rounding = {{1.0e20f,-1.0e20f,3.0f,0,0,0,0,0,0}};
    const float ones[3] = {1,1,1};
    float rounded[3] = {-1,-1,-1};
    rounding.transform(rounded, ones);
    if (rounded[0] != 0.0f) return 4;
    return 0;
}
""")
