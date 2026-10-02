"""Compile live DiploWin bodies against behavioral host doubles.

These checks exercise source-derived control flow and state transitions. The
fixture's classes are intentionally not the target ABI and passing is not
native or runtime acceptance.
"""
import os
from pathlib import Path
import shutil
import subprocess

import pytest


PROJECT_ROOT = Path(__file__).resolve().parents[2]
SOURCE_ROOT = Path(os.environ.get("DIPLOWIN_SOURCE_ROOT", PROJECT_ROOT))
FIXTURE = Path(__file__).with_name("diplowin_exec_host_control.cpp")


def extract_function(source: str, signature: str) -> str:
    start = source.index(signature)
    opening = source.index("{", start)
    depth = 0
    state = "code"
    index = opening
    while index < len(source):
        char = source[index]
        nxt = source[index + 1] if index + 1 < len(source) else ""
        if state == "code":
            if char == "/" and nxt == "/":
                state = "line"
                index += 2
                continue
            if char == "/" and nxt == "*":
                state = "block"
                index += 2
                continue
            if char == '"':
                state = "string"
            elif char == "'":
                state = "character"
            elif char == "{":
                depth += 1
            elif char == "}":
                depth -= 1
                if depth == 0:
                    return source[start:index + 1]
        elif state == "line":
            if char == "\n":
                state = "code"
        elif state == "block":
            if char == "*" and nxt == "/":
                state = "code"
                index += 2
                continue
        elif state in ("string", "character"):
            if char == "\\":
                index += 2
                continue
            if (state == "string" and char == '"') or (
                state == "character" and char == "'"
            ):
                state = "code"
        index += 1
    raise AssertionError(f"unclosed function body for {signature}")


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
def test_diplowin_exec_and_transcript_bodies(compiler_name, tmp_path):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(f"{compiler_name} is unavailable")

    diplowin_path = SOURCE_ROOT / "src" / "diplowin.cpp"
    leaf_path = SOURCE_ROOT / "src" / "leaf_recoveries.cpp"
    assert diplowin_path.is_file(), f"DiploWin source not found: {diplowin_path}"
    assert leaf_path.is_file(), f"leaf recovery source not found: {leaf_path}"
    assert FIXTURE.is_file(), f"host fixture not found: {FIXTURE}"

    diplowin = diplowin_path.read_text()
    leaf = leaf_path.read_text()
    methods = "\n\n".join(
        extract_function(diplowin, signature)
        for signature in (
            "static RECT inset_rect(RECT value, int left, int top, int right, int bottom)",
            "void __cdecl DiploWin::string_callback(int faction_id, int unused)",
            "void DiploWin::exec(int faction_id, int faction_id_with)",
        )
    )
    transcript = extract_function(
        leaf, 'extern "C" void __cdecl sub_441100(int faction_id, int faction_id_with)'
    )

    generated = FIXTURE.read_text()
    for marker, body in (
        ("/* DIPLOWIN_EXEC_FUNCTIONS */", methods),
        ("/* DIPLOWIN_TRANSCRIPT_FUNCTION */", transcript),
    ):
        assert generated.count(marker) == 1
        generated = generated.replace(marker, body)

    generated_path = tmp_path / "diplowin_exec_host_control.cpp"
    generated_path.write_text(generated)
    executable = tmp_path / f"diplowin-control-{compiler_name}"
    command = [
        compiler, "-std=c++98", "-Wall", "-Wextra",
        "-I", str(FIXTURE.parent), str(generated_path), "-o", str(executable),
    ]
    built = subprocess.run(
        command, capture_output=True, text=True, timeout=30, check=False
    )
    assert built.returncode == 0, built.stdout + built.stderr
    ran = subprocess.run(
        [str(executable)], capture_output=True, text=True, timeout=30, check=False
    )
    assert ran.returncode == 0, ran.stdout + ran.stderr
