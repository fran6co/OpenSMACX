"""Actual-source host behavior controls for two recovered MapWin handlers.

This deliberately extracts current definitions without substituting a proposed
body. Fake classes model behavior only; native VC6 layout/vtable evidence is a
separate requirement.
"""

from pathlib import Path
import re
import shutil
import subprocess

import pytest


HERE = Path(__file__).resolve()
ROOT = HERE.parents[2]
MAPWIN_CPP = ROOT / "src/mapwin.cpp"
FIXTURE = HERE.with_name("mapwin_input_harness.cpp")


def _mask(text):
    out = list(text)
    mode = "code"
    i = 0
    while i < len(text):
        c = text[i]
        n = text[i + 1] if i + 1 < len(text) else ""
        if mode == "code":
            if c == "/" and n == "/":
                out[i] = out[i + 1] = " "
                i += 2
                mode = "line"
                continue
            if c == "/" and n == "*":
                out[i] = out[i + 1] = " "
                i += 2
                mode = "block"
                continue
            if c == '"':
                out[i] = " "
                mode = "string"
            elif c == "'":
                out[i] = " "
                mode = "char"
        elif mode == "line":
            if c == "\n":
                mode = "code"
            else:
                out[i] = " "
        elif mode == "block":
            out[i] = " "
            if c == "*" and n == "/":
                out[i + 1] = " "
                i += 2
                mode = "code"
                continue
        else:
            out[i] = " "
            if c == "\\" and i + 1 < len(text):
                out[i + 1] = " "
                i += 2
                continue
            if (mode == "string" and c == '"') or (mode == "char" and c == "'"):
                mode = "code"
        i += 1
    return "".join(out)


def _definition(text, signature):
    masked = _mask(text)
    found = []
    for match in re.finditer(re.escape(signature) + r"\s*\(", masked):
        depth = 1
        pos = masked.find("(", match.start()) + 1
        while pos < len(masked) and depth:
            depth += (masked[pos] == "(") - (masked[pos] == ")")
            pos += 1
        while pos < len(masked) and masked[pos].isspace():
            pos += 1
        if depth or pos >= len(masked) or masked[pos] != "{":
            continue
        opening = pos
        depth = 0
        for end in range(opening, len(masked)):
            depth += (masked[end] == "{") - (masked[end] == "}")
            if depth == 0:
                found.append(text[match.start():end + 1])
                break
    assert len(found) == 1, f"expected unique actual definition for {signature}, got {len(found)}"
    assert "BODY GOES HERE" not in found[0]
    return found[0]


def _actual_bodies():
    text = MAPWIN_CPP.read_text()
    close_body = _definition(text, "void MapWin::on_sys_close")
    owned_close_body = _definition(text, "void MapWin::close")
    right_body = _definition(text, "void MapWin::on_right_down")
    assert "from_graphic_base(this)" not in close_body, \
        "host owner test requires typed MapWin member receiver, not raw entered-base subtraction"
    assert "on_mouse_move(x_coord, y_coord);" in right_body, \
        "right-down must route through the existing typed Win virtual callback"
    assert "Slot17Shim" not in right_body and "MapWinEnteredOffset" not in right_body
    return owned_close_body, close_body, right_body


def _run(tmp_path, compiler):
    owned_close_body, close_body, right_body = _actual_bodies()
    fixture = FIXTURE.read_text()
    assert fixture.count("/* ACTUAL_CLOSE */") == 1
    assert fixture.count("/* ACTUAL_ON_SYS_CLOSE */") == 1
    assert fixture.count("/* ACTUAL_ON_RIGHT_DOWN */") == 1
    source = fixture.replace("/* ACTUAL_CLOSE */", owned_close_body)
    source = source.replace("/* ACTUAL_ON_SYS_CLOSE */", close_body)
    source = source.replace("/* ACTUAL_ON_RIGHT_DOWN */", right_body)
    cpp = tmp_path / "mapwin_input_actual.cpp"
    exe = tmp_path / "mapwin_input_actual"
    cpp.write_text(source)
    built = subprocess.run(
        [compiler, "-std=c++11", "-Wall", "-Wextra", "-Werror", "-pedantic",
         "-fsanitize=address,undefined", "-fno-omit-frame-pointer", str(cpp), "-o", str(exe)],
        capture_output=True, text=True,
    )
    assert built.returncode == 0, built.stdout + built.stderr
    run = subprocess.run(
        [str(exe)], capture_output=True, text=True, timeout=30,
        env={**__import__("os").environ, "ASAN_OPTIONS": "detect_leaks=0",
             "UBSAN_OPTIONS": "halt_on_error=1"},
    )
    assert run.returncode == 0, run.stdout + run.stderr
    assert "MapWin input owner actual-source controls passed" in run.stdout


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
def test_actual_mapwin_input_owner_host_controls(tmp_path, compiler_name):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(compiler_name + " unavailable")
    _run(tmp_path, compiler)
