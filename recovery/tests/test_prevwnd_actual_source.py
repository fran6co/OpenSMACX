"""Host behavior controls for the actual Win-owned PrevWndFunc body.

The fake Win methods prove only the extracted C++ body's routing and value
conversion. They do not prove VC6's 32-bit Windows ABI or physical vtable
layout; those require native build/COFF and pinned-image evidence.
"""

from pathlib import Path
import re
import shutil
import subprocess

import pytest


HERE = Path(__file__).resolve()
ROOT = HERE.parents[2]
SOURCE_ROOT = ROOT / "src"
FIXTURE = HERE.with_name("prevwnd_harness.cpp")


def _mask_comments_strings(text):
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


def _actual_definition():
    signature = re.compile(
        r'extern\s+"C"\s+int\s+__stdcall\s+PrevWndFunc\s*\('
    )
    matches = []
    for path in sorted(SOURCE_ROOT.glob("*.cpp")):
        text = path.read_text()
        masked = _mask_comments_strings(text)
        for match in signature.finditer(text):
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
                    matches.append((path, text[match.start():end + 1]))
                    break
    assert len(matches) == 1, f"expected unique current PrevWndFunc owner, got {len(matches)}"
    path, body = matches[0]
    assert "BODY GOES HERE" not in body
    assert "owner->on_size(" in body
    assert "owner->on_set_cursor(" in body
    return path, body


def _compile_run(tmp_path, compiler, source):
    fixture = FIXTURE.read_text()
    assert fixture.count("/* ACTUAL_SOURCE */") == 1
    cpp = tmp_path / "prevwnd_actual.cpp"
    exe = tmp_path / "prevwnd_actual"
    cpp.write_text(fixture.replace("/* ACTUAL_SOURCE */", source))
    built = subprocess.run(
        [compiler, "-std=c++11", "-Wall", "-Wextra", "-Werror", "-pedantic", "-Wno-int-to-pointer-cast",
         "-fsanitize=address,undefined", "-fno-omit-frame-pointer", str(cpp), "-o", str(exe)],
        capture_output=True, text=True,
    )
    assert built.returncode == 0, built.stdout + built.stderr
    run = subprocess.run(
        [str(exe)], capture_output=True, text=True, timeout=20,
        env={**__import__("os").environ, "ASAN_OPTIONS": "detect_leaks=0",
             "UBSAN_OPTIONS": "halt_on_error=1"},
    )
    assert run.returncode == 0, run.stdout + run.stderr
    assert "PrevWndFunc actual-source controls passed" in run.stdout


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
def test_actual_prevwindow_source_host_controls(tmp_path, compiler_name):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(compiler_name + " unavailable")
    _, body = _actual_definition()
    _compile_run(tmp_path, compiler, body)


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
def test_wrong_prevwindow_method_or_word_extension_is_rejected(tmp_path, compiler_name):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(compiler_name + " unavailable")
    _, body = _actual_definition()
    mutants = (
        body.replace("owner->on_size(", "owner->on_set_cursor(", 1),
        body.replace("static_cast<short>(lparam)",
                     "static_cast<unsigned short>(lparam)", 1),
        body.replace("static_cast<short>(lparam >> 16)",
                     "static_cast<unsigned short>(lparam >> 16)", 1),
        body.replace("return owner->on_set_cursor(",
                     "return 0 && owner->on_set_cursor(", 1),
    )
    assert all(mutant != body for mutant in mutants)
    for mutant in mutants:
        with pytest.raises(AssertionError):
            _compile_run(tmp_path, compiler, mutant)
