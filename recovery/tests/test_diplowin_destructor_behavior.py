"""Compile actual DiploWin destructor text into host-double lifecycle controls.

In the project this file belongs under recovery/tests beside the C++ fixture.
By default it extracts src/diplowin.cpp from the repository root; the optional
source override is useful for privately checking an unintegrated candidate.
The fixture hierarchy is a host double: passing it checks ordinary C++ member
destruction and unwind behavior, not target layout, executable equivalence, or
game runtime behavior.
"""

import os
from pathlib import Path
import re
import shutil
import subprocess
import tempfile

import pytest


DEFINITION = re.compile(r"(?m)^\s*DiploWin::~DiploWin\s*\(\s*\)\s*\{")
FIXTURE_INCLUDE = '#include "diplowin_destructor_candidate.cpp"'


def repository_root() -> Path:
    configured = os.environ.get("OPENSMACX_SOURCE_ROOT")
    if configured:
        return Path(configured).resolve()
    return Path(__file__).resolve().parents[2]


def extract_destructor(source_text: str) -> str:
    match = DEFINITION.search(source_text)
    assert match is not None, "DiploWin::~DiploWin definition is missing"
    opening = source_text.find("{", match.start(), match.end())
    depth = 0
    quote = None
    escaped = False
    line_comment = False
    block_comment = False
    index = opening
    while index < len(source_text):
        char = source_text[index]
        next_char = source_text[index + 1] if index + 1 < len(source_text) else ""
        if line_comment:
            if char == "\n":
                line_comment = False
        elif block_comment:
            if char == "*" and next_char == "/":
                block_comment = False
                index += 1
        elif quote is not None:
            if escaped:
                escaped = False
            elif char == "\\":
                escaped = True
            elif char == quote:
                quote = None
        elif char == "/" and next_char == "/":
            line_comment = True
            index += 1
        elif char == "/" and next_char == "*":
            block_comment = True
            index += 1
        elif char in ('"', "'"):
            quote = char
        elif char == "{":
            depth += 1
        elif char == "}":
            depth -= 1
            if depth == 0:
                return source_text[match.start() : index + 1]
        index += 1
    raise AssertionError("unterminated DiploWin destructor body")


COMPILERS = [name for name in ("g++", "clang++") if shutil.which(name)]


@pytest.mark.parametrize("compiler", COMPILERS or [None])
def test_diplowin_dtor_uses_typed_lifecycle_and_unwind_controls(compiler):
    if compiler is None:
        pytest.skip("neither g++ nor clang++ is available")

    source_override = os.environ.get("DIPLOWIN_DESTRUCTOR_SOURCE")
    if source_override:
        source_path = Path(source_override).resolve()
    else:
        source_path = repository_root() / "src" / "diplowin.cpp"
    body = extract_destructor(source_path.read_text())

    fixture_path = Path(__file__).with_name(
        "diplowin_destructor_host_control.cpp"
    )
    fixture = fixture_path.read_text()
    assert fixture.count(FIXTURE_INCLUDE) == 1
    generated = fixture.replace(FIXTURE_INCLUDE, body)

    with tempfile.TemporaryDirectory(prefix="diplowin-dtor-control-") as temp:
        temp_path = Path(temp)
        generated_path = temp_path / "diplowin_destructor_host_control.cpp"
        binary_path = temp_path / "diplowin_destructor_host_control"
        generated_path.write_text(generated)
        subprocess.run(
            [
                compiler,
                "-std=c++98",
                "-Wall",
                "-Wextra",
                "-pedantic",
                str(generated_path),
                "-o",
                str(binary_path),
            ],
            check=True,
            capture_output=True,
            text=True,
            timeout=60,
        )
        subprocess.run(
            [str(binary_path)],
            check=True,
            capture_output=True,
            text=True,
            timeout=30,
        )
