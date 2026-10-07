"""Installed-source VC6 runtime controls for the unresolved Mat3 owner."""

from __future__ import annotations

import re
import os
import shutil
import subprocess
import sys
from pathlib import Path

import pytest


ROOT = next(parent for parent in Path(__file__).resolve().parents
            if (parent / "src/general.cpp").is_file())
SOURCE = ROOT / "src/general.cpp"
PIN = "01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf"


def _definition(text: str, signature: str) -> str:
    masked = list(text)
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
    assert len(hits) == 1, (signature, len(hits))
    start = hits[0].start()
    opening = code.find("{", hits[0].end())
    depth = 0
    for position in range(opening, len(code)):
        if code[position] == "{":
            depth += 1
        elif code[position] == "}":
            depth -= 1
            if depth == 0:
                return text[start:position + 1]
    raise AssertionError(f"unterminated product definition: {signature}")


@pytest.mark.parametrize("damage", [None, "unordered_comparison", "early_output_store"])
def test_actual_installed_matrix_methods_on_vc6_both_configured_flags(
    tmp_path: Path, damage: str | None,
) -> None:
    """Boolean/alias runtime control only; not x87 status or byte proof."""
    from match_recovery.measurement import Measurement
    from match_recovery.processes import run_bounded
    from match_recovery.source_inventory import image_pin
    from recovery.environment.compile import compile_object
    from recovery.environment.wine import VC6_ROOT, environment

    if not os.environ.get("MATCH_RECOVERY_RESOURCE_LEASE"):
        pytest.skip("native controls require a coordinator-owned compiler lease")
    assert image_pin(ROOT) == PIN
    if not (VC6_ROOT / "BIN/CL.EXE").is_file() or shutil.which("wine32") is None:
        pytest.skip("VC6 and Wine32 are required")

    text = SOURCE.read_text()
    methods = _definition(text, "bool Mat3::equals(")
    methods += "\n" + _definition(text, "void Matrix3::transform(")
    if damage == "unordered_comparison":
        old = "mine[row * 3 + col] != other[row * 3 + col]"
        assert methods.count(old) == 1
        methods = methods.replace(old,
            "mine[row * 3 + col] < other[row * 3 + col] || "
            "mine[row * 3 + col] > other[row * 3 + col]", 1)
    elif damage == "early_output_store":
        old = "    result[1] = m[4] * in[1] + m[3] * in[0] + m[5] * in[2];"
        assert methods.count(old) == 1
        methods = methods.replace(old, "    out[0] = result[0];\n" + old, 1)
    harness = r"""
#include <cstring>
struct Mat3 { float v[9]; bool equals(const float *other); };
struct Matrix3 { float v[9]; void transform(float *out, const float *in); };
""" + methods + r"""
int main() {
    Mat3 matrix = {{1,2,3,4,5,6,7,8,9}};
    float other[9] = {1,2,3,4,5,6,7,8,9};
    if (!matrix.equals(other)) return 1;
    other[4] = 50;
    if (matrix.equals(other)) return 2;
    other[4] = 5;
    unsigned int qnan_bits = 0x7fc00000u;
    float qnan;
    memcpy(&qnan, &qnan_bits, sizeof(qnan));
    matrix.v[0] = qnan;
    if (!matrix.equals(other)) return 3;
    matrix.v[0] = 1;
    other[8] = qnan;
    if (!matrix.equals(other)) return 4;

    Matrix3 matrix3 = {{1,2,3,4,5,6,7,8,9}};
    const float input[3] = {2,3,4};
    float output[3] = {-1,-1,-1};
    matrix3.transform(output, input);
    if (output[0] != 20 || output[1] != 47 || output[2] != 74) return 5;
    float alias[3] = {2,3,4};
    matrix3.transform(alias, alias);
    if (alias[0] != 20 || alias[1] != 47 || alias[2] != 74) return 6;
    return 0;
}
"""
    settings = Measurement.read(ROOT)
    wrapper = ROOT / "recovery/environment/wine.py"
    command = (sys.executable, str(wrapper), "--tool=cl", "/nologo")
    for index, flags in enumerate(settings.flag_sets):
        source = tmp_path / ("general_matrix_%d.cpp" % index)
        source.write_text(harness)
        object_bytes = compile_object(
            source, command, (*flags, *settings.layout_flags)
        )
        obj = tmp_path / ("general_matrix_%d.obj" % index)
        obj.write_bytes(object_bytes)
        executable = tmp_path / ("general_matrix_%d.exe" % index)
        linked = run_bounded(
            [sys.executable, str(wrapper), "--tool=link", "/nologo",
             "/subsystem:console", "/incremental:no", "/out:" + str(executable),
             str(obj), "kernel32.lib"], cwd=tmp_path, timeout=60,
        )
        assert linked.returncode == 0, (linked.stdout + linked.stderr).decode(errors="replace")
        result = subprocess.run(
            ["wine32", str(executable)], env=environment(),
            capture_output=True, text=True, timeout=30,
        )
        expected = {None: 0, "unordered_comparison": 3, "early_output_store": 6}[damage]
        assert result.returncode == expected, result.stdout + result.stderr
