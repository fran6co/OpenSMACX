"""Run the actual FileBox declarations and bodies with a common-dialog double.

The host pointer width and instrumented OPENFILENAME constructor are not ABI
or original stack witnesses. Native compiler checks cover the target separately.
"""
from pathlib import Path
import shutil
import subprocess

import pytest


ROOT = Path(__file__).resolve().parents[2]
FIXTURE = Path(__file__).with_name("filebox_save_host_control.cpp")


def definition(text, signature):
    start = text.index(signature)
    opening = text.index("{", start)
    depth = 0
    for end in range(opening, len(text)):
        if text[end] == "{":
            depth += 1
        elif text[end] == "}":
            depth -= 1
            if depth == 0:
                return text[start:end + 1]
    raise AssertionError("unterminated actual declaration/body")


@pytest.mark.parametrize("compiler", ["g++", "clang++"])
def test_actual_filebox_save_and_cancellation(compiler, tmp_path):
    executable = shutil.which(compiler)
    if executable is None:
        pytest.skip(f"host compiler unavailable: {compiler}")
    header = (ROOT / "src/filebox.h").read_text()
    source = (ROOT / "src/filebox.cpp").read_text()
    declaration = definition(header, "class FileBox {") + ";"
    bodies = "\n\n".join(definition(source, signature) for signature in (
        "FileBox::FileBox()", "FileBox::~FileBox()", "char *FileBox::save()",
    ))
    fixture = FIXTURE.read_text()
    assert fixture.count("/* ACTUAL_FILEBOX_CLASS */") == 1
    assert fixture.count("/* ACTUAL_FILEBOX_BODIES */") == 1
    cpp = tmp_path / "actual.cpp"
    binary = tmp_path / "control"
    cpp.write_text(fixture.replace("/* ACTUAL_FILEBOX_CLASS */", declaration)
                   .replace("/* ACTUAL_FILEBOX_BODIES */", bodies))
    built = subprocess.run([
        executable, "-std=c++11", "-Wall", "-Wextra", "-Werror",
        str(cpp), "-o", str(binary),
    ], capture_output=True, text=True, timeout=30)
    assert built.returncode == 0, built.stdout + built.stderr
    ran = subprocess.run([str(binary)], capture_output=True, text=True, timeout=30)
    assert ran.returncode == 0, ran.stdout + ran.stderr
