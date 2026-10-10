"""Actual-source controls for popup wave callback event selection."""
from pathlib import Path
import resource
import re
import shutil
import subprocess
import tempfile

import pytest


ROOT = Path(__file__).resolve().parents[2]
SOURCE = ROOT / "src" / "popup.cpp"
FIXTURE = Path(__file__).parent / "fixtures" / "popup_production_dispatch_owner_oct10.cpp"
SIGNATURE = r"void\s+__cdecl\s+popup_wave_callback\s*\(BasePopTextBlock\s*\*popup,\s*int\s*\)"


def _matching_brace(text: str, opening: int) -> int:
    depth = 0
    state = "code"
    i = opening
    while i < len(text):
        char = text[i]
        nxt = text[i + 1] if i + 1 < len(text) else ""
        if state == "line":
            if char == "\n": state = "code"
        elif state == "block":
            if char == "*" and nxt == "/": state = "code"; i += 1
        elif state in ("string", "char"):
            if char == "\\": i += 1
            elif (state == "string" and char == '"') or (state == "char" and char == "'"):
                state = "code"
        elif char == "/" and nxt == "/": state = "line"; i += 1
        elif char == "/" and nxt == "*": state = "block"; i += 1
        elif char == '"': state = "string"
        elif char == "'": state = "char"
        elif char == "{": depth += 1
        elif char == "}":
            depth -= 1
            if depth == 0: return i
        i += 1
    raise AssertionError("unterminated actual callback")


def _definition(source: str) -> str:
    match = re.search(SIGNATURE, source)
    assert match, "actual popup_wave_callback definition not found"
    opening = source.index("{", match.end())
    return source[match.start():_matching_brace(source, opening) + 1]


def _materialize(source: str) -> str:
    body = _definition(source)
    assert "FactionMusic->production()" in body
    assert "FactionMusic->general()" not in body
    fixture = FIXTURE.read_text()
    return fixture.replace("/* ACTUAL_POPUP_WAVE_CALLBACK */", body)


def _compile_and_run(code: str, directory: Path) -> subprocess.CompletedProcess:
    compiler = shutil.which("c++") or shutil.which("g++") or shutil.which("clang++")
    if not compiler:
        pytest.skip("host C++ compiler unavailable")
    source_path = directory / "callback.cpp"
    executable = directory / "callback"
    source_path.write_text(code)
    built = subprocess.run([compiler, "-std=c++17", "-O0", str(source_path), "-o", str(executable)],
                           text=True, capture_output=True)
    assert built.returncode == 0, built.stdout + built.stderr

    def disable_core_dumps() -> None:
        resource.setrlimit(resource.RLIMIT_CORE, (0, 0))

    return subprocess.run([str(executable)], text=True, capture_output=True,
                          preexec_fn=disable_core_dumps)


def test_full_actual_callback_and_compiling_wrong_event_mutant():
    source = SOURCE.read_text()
    actual = _materialize(source)
    # The adjacent input callback is a separate general-event path and stays so.
    wave_source = (ROOT / "src" / "wave.cpp").read_text()
    assert re.search(r"void\s+__cdecl\s+input_callback\s*\(\)\s*\{\s*if\s*\(FactionMusic\s*!=\s*0\)\s*FactionMusic->general\(\);", wave_source)
    with tempfile.TemporaryDirectory(prefix="popup-production-dispatch-") as temporary:
        directory = Path(temporary)
        result = _compile_and_run(actual, directory)
        assert result.returncode == 0, result.stdout + result.stderr

        mutant = actual.replace("FactionMusic->production()", "FactionMusic->general()", 1)
        assert mutant != actual
        result = _compile_and_run(mutant, directory)
        assert result.returncode != 0, "wrong-general dispatch mutant escaped the source control"
