"""Compile extracted owner bodies with reentrant host callback controls."""

from pathlib import Path
import shutil
import subprocess

import pytest


ROOT = Path(__file__).resolve().parents[2]
HERE = Path(__file__).resolve().parent
TRACKED_OWNER = ROOT / "src/factionprofilewin.cpp"
SIGNATURES = (
    "void FactionProfileWin::initialize(int faction_id, int profile_mode)",
    "int FactionProfileWin::show_profile()",
)


def extract_definition(source, signature):
    start = source.find(signature)
    if start < 0:
        raise AssertionError("owner source missing " + signature)
    opening = source.find("{", start + len(signature))
    if opening < 0:
        raise AssertionError("owner method has no body: " + signature)
    depth = 0
    state = "code"
    index = opening
    while index < len(source):
        char = source[index]
        following = source[index + 1] if index + 1 < len(source) else ""
        if state == "code":
            if char == "/" and following == "/":
                state = "line_comment"
                index += 1
            elif char == "/" and following == "*":
                state = "block_comment"
                index += 1
            elif char == '"':
                state = "string"
            elif char == "'":
                state = "character"
            elif char == "{":
                depth += 1
            elif char == "}":
                depth -= 1
                if depth == 0:
                    return source[start:index + 1]
        elif state == "line_comment":
            if char == "\n":
                state = "code"
        elif state == "block_comment":
            if char == "*" and following == "/":
                state = "code"
                index += 1
        else:
            quote = '"' if state == "string" else "'"
            if char == "\\":
                index += 1
            elif char == quote:
                state = "code"
        index += 1
    raise AssertionError("unterminated method: " + signature)


def compiler_candidates():
    found = [shutil.which(name) for name in ("g++", "clang++")]
    assert all(found), "GCC and Clang are both required"
    return found


@pytest.mark.parametrize("compiler", compiler_candidates())
def test_reentrant_owner_reload_body_controls(compiler, tmp_path):
    # Test only the real owner; missing product source is a failure.
    owner = TRACKED_OWNER
    source = owner.read_text(encoding="utf-8")
    definitions = [extract_definition(source, signature)
                   for signature in SIGNATURES]
    generated = tmp_path / "factionprofilewin_host.cpp"
    harness = (HERE / "factionprofilewin_control.cpp").as_posix()
    generated.write_text(
        '#include "' + harness + '"\n\n'
        "// Verbatim definitions extracted from %s.\n\n" % owner
        + "\n\n".join(definitions) + "\n",
        encoding="utf-8",
    )
    executable = tmp_path / "factionprofilewin_host"
    compiled = subprocess.run(
        [compiler, "-std=c++98", "-Wall", "-Wextra", "-pedantic",
         "-I", str(HERE), str(generated), "-o", str(executable)],
        capture_output=True, text=True,
    )
    assert compiled.returncode == 0, compiled.stdout + compiled.stderr
    ran = subprocess.run([str(executable)], capture_output=True, text=True)
    assert ran.returncode == 0, ran.stdout + ran.stderr
