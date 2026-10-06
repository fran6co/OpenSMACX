"""Compile focused host controls from the installed product function text."""
from __future__ import annotations

import shutil
import subprocess
import tempfile
from pathlib import Path

import pytest


ROOT = next(parent for parent in Path(__file__).resolve().parents
            if (parent / "src/movie_media_decoder.cpp").exists())
DECODER = ROOT / "src/movie_media_decoder.cpp"
FIXTURES = Path(__file__).parent / "fixtures"


def _matching_brace_end(text: str, opening: int) -> int:
    depth = 0
    state = "code"
    i = opening
    while i < len(text):
        char = text[i]
        next_char = text[i + 1] if i + 1 < len(text) else ""
        if state == "line_comment":
            if char == "\n":
                state = "code"
        elif state == "block_comment":
            if char == "*" and next_char == "/":
                state = "code"
                i += 1
        elif state == "string":
            if char == "\\":
                i += 1
            elif char == '"':
                state = "code"
        elif state == "character":
            if char == "\\":
                i += 1
            elif char == "'":
                state = "code"
        elif char == "/" and next_char == "/":
            state = "line_comment"
            i += 1
        elif char == "/" and next_char == "*":
            state = "block_comment"
            i += 1
        elif char == '"':
            state = "string"
        elif char == "'":
            state = "character"
        elif char == "{":
            depth += 1
        elif char == "}":
            depth -= 1
            if depth == 0:
                return i + 1
        i += 1
    raise AssertionError("source function has an unterminated body")


def _definition(text: str, signature: str) -> str:
    start = text.index(signature)
    opening = text.index("{", start)
    return text[start:_matching_brace_end(text, opening)]


def _compile_and_run(fixture: Path, source_fragment: str) -> None:
    compiler = shutil.which("g++") or shutil.which("clang++")
    if compiler is None:
        pytest.skip("GCC/Clang host compiler is unavailable")
    template = fixture.read_text()
    token = "/*__ACTUAL_OWNER__*/"
    assert template.count(token) == 1
    with tempfile.TemporaryDirectory(prefix="movie-owner-source-test-") as temp:
        temp_path = Path(temp)
        cpp = temp_path / "actual_source_control.cpp"
        exe = temp_path / "actual_source_control"
        cpp.write_text(template.replace(token, source_fragment))
        subprocess.run(
            [compiler, "-std=c++98", "-w", "-no-pie", str(cpp), "-o", str(exe)],
            check=True, cwd=ROOT,
        )
        subprocess.run([str(exe)], check=True, cwd=ROOT)


def test_actual_5cf150_source_dispatch_and_state() -> None:
    text = DECODER.read_text()
    helper_start = text.index("static LONG movie_media_add_ecx(")
    function_signature = "MediaTransformRegisterContinuation\n__cdecl sub_5cf150() {"
    function_start = text.index(function_signature, helper_start)
    helper_and_body = text[helper_start:function_start] + _definition(
        text, function_signature)
    _compile_and_run(FIXTURES / "movie_transform_actual.cpp", helper_and_body)


def test_actual_5cf2f0_post_call_continuations() -> None:
    text = DECODER.read_text()
    body = _definition(
        text, "MediaTransformRegisterContinuation __cdecl sub_5cf2f0(")
    _compile_and_run(FIXTURES / "movie_predictor_actual.cpp", body)
