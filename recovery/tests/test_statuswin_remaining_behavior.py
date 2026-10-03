"""Host controls that compile the real StatusWin definitions from src/statuswin.cpp.

The C++ fixtures provide bounded test doubles only. They are not target ABI,
native compiler, or runtime evidence. No candidate body is copied into tests.
"""
import os
from pathlib import Path
import shutil
import subprocess

import pytest


HERE = Path(__file__).resolve().parent
METHODS_FIXTURE = HERE / "statuswin_remaining_host_control.cpp"
HUD_FIXTURE = HERE / "statuswin_full_host_control.cpp"
THREE_MARKER = "/*__STATUSWIN_THREE_METHOD_BODIES__*/"
HUD_MARKER = "/*__STATUSWIN_DRAW_STATUS_BODY__*/"
METHOD_NAMES = ("arrow_left", "arrow_right", "init", "draw_status")


def project_root():
    override = os.environ.get("OPENSMACX_SOURCE_ROOT")
    if override:
        return Path(override).resolve()
    # In recovery/tests/, parents[1] is the repository root.
    return HERE.parents[1]


def extract_definition(source, name):
    signature = "void StatusWin::" + name + "("
    start = source.find(signature)
    assert start >= 0, "missing actual src/statuswin.cpp definition: " + signature
    assert source.find(signature, start + len(signature)) < 0, \
        "ambiguous duplicate definition: " + signature
    brace = source.find("{", start + len(signature))
    assert brace >= 0, "definition has no body: " + signature

    # Count braces while ignoring C++ comments and quoted literals, so strings
    # or comments in the renderer cannot truncate the extracted function.
    depth = 0
    i = brace
    state = "code"
    while i < len(source):
        ch = source[i]
        nxt = source[i + 1] if i + 1 < len(source) else ""
        if state == "code":
            if ch == "/" and nxt == "/":
                state = "line_comment"
                i += 2
                continue
            if ch == "/" and nxt == "*":
                state = "block_comment"
                i += 2
                continue
            if ch == '"':
                state = "string"
            elif ch == "'":
                state = "character"
            elif ch == "{":
                depth += 1
            elif ch == "}":
                depth -= 1
                if depth == 0:
                    body = source[start:i + 1]
                    assert "BODY GOES HERE." not in body, \
                        "refusing placeholder StatusWin method: " + name
                    return body
        elif state == "line_comment":
            if ch == "\n":
                state = "code"
        elif state == "block_comment":
            if ch == "*" and nxt == "/":
                state = "code"
                i += 2
                continue
        elif state in ("string", "character"):
            if ch == "\\":
                i += 2
                continue
            if (state == "string" and ch == '"') or \
               (state == "character" and ch == "'"):
                state = "code"
        i += 1
    raise AssertionError("unterminated StatusWin method: " + name)


def extracted_source_pair():
    source_path = project_root() / "src" / "statuswin.cpp"
    source = source_path.read_text(encoding="utf-8")
    bodies = {name: extract_definition(source, name) for name in METHOD_NAMES}
    three_template = METHODS_FIXTURE.read_text(encoding="utf-8")
    hud_template = HUD_FIXTURE.read_text(encoding="utf-8")
    assert three_template.count(THREE_MARKER) == 1
    assert hud_template.count(HUD_MARKER) == 1
    three = "\n\n".join(bodies[name] for name in METHOD_NAMES[:3])
    hud = bodies["draw_status"]
    return (
        three_template.replace(THREE_MARKER, three),
        hud_template.replace(HUD_MARKER, hud),
    )


@pytest.mark.parametrize("compiler", ["g++", "clang++"])
def test_statuswin_actual_source_methods(compiler, tmp_path):
    executable = shutil.which(compiler)
    if executable is None:
        pytest.skip("host C++ compiler is unavailable: " + compiler)
    three_source, hud_source = extracted_source_pair()
    cases = (
        ("statuswin-three", three_source,
         ["-Wall", "-Wextra", "-Werror"]),
        ("statuswin-hud", hud_source,
         ["-Wno-unused-variable", "-Wno-unused-parameter"]),
    )
    for name, source, flags in cases:
        cpp = tmp_path / (name + ".cpp")
        binary = tmp_path / name
        cpp.write_text(source, encoding="utf-8")
        built = subprocess.run(
            [executable, "-std=c++98"] + flags + [
                "-I", str(HERE), str(cpp), "-o", str(binary)
            ], capture_output=True, text=True, timeout=60, check=False,
        )
        assert built.returncode == 0, built.stdout + built.stderr
        ran = subprocess.run(
            [str(binary)], capture_output=True, text=True,
            timeout=30, check=False,
        )
        assert ran.returncode == 0, ran.stdout + ran.stderr
