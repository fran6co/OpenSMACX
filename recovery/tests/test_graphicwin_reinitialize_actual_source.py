"""Bounded host controls for the relocated 0x5D50F0 product definition.

The fixture supplies service/member doubles only. The mapped definition is
extracted verbatim from current src/graphicwin.cpp after coordinator
publication; this proposal test intentionally does not inject BODY from the
proposal generator.
"""

from pathlib import Path
import os
import shutil
import subprocess

from match_recovery.source.reader import read_file

ROOT = Path(__file__).resolve().parents[2]
HERE = Path(__file__).resolve().parent
HARNESS = HERE / "graphicwin_reinitialize_harness.cpp"
ADDRESS = 0x005D50F0
SIGNATURE = "int GraphicWin::reinitialize(int a1, int a2, int width, int height,"


def _mask_cpp(text: str) -> str:
    out = list(text)
    i = 0
    while i < len(text):
        if text.startswith("//", i):
            end = text.find("\n", i)
            if end < 0:
                end = len(text)
            for j in range(i, end):
                out[j] = " "
            i = end
        elif text.startswith("/*", i):
            end = text.find("*/", i + 2)
            if end < 0:
                raise AssertionError("unterminated product comment")
            end += 2
            for j in range(i, end):
                if text[j] != "\n":
                    out[j] = " "
            i = end
        elif text[i] in "\"'":
            quote, j = text[i], i + 1
            while j < len(text):
                if text[j] == "\\":
                    j += 2
                elif text[j] == quote:
                    j += 1
                    break
                else:
                    j += 1
            for k in range(i, min(j, len(text))):
                if text[k] != "\n":
                    out[k] = " "
            i = j
        else:
            i += 1
    return "".join(out)


def _definition(source: str) -> str:
    masked = _mask_cpp(source)
    start = masked.find(SIGNATURE)
    assert start >= 0, "relocated product method is missing"
    assert masked.find(SIGNATURE, start + 1) < 0, "product method is ambiguous"
    opening = masked.find("{", start)
    assert opening >= 0
    depth = 0
    for end in range(opening, len(masked)):
        depth += (masked[end] == "{") - (masked[end] == "}")
        if depth == 0:
            return source[start:end + 1]
    raise AssertionError("unterminated product function")


def _actual_body() -> str:
    path = ROOT / "src/graphicwin.cpp"
    records = [row for row in read_file(path) if row.address == ADDRESS]
    assert len(records) == 1, "0x5D50F0 mapping must have one current owner"
    assert records[0].path == path.resolve()
    assert records[0].region and SIGNATURE in records[0].region
    body = _definition(path.read_text(encoding="utf-8"))
    assert "BODY GOES HERE" not in body
    return body


def test_graphicwin_reinitialize_actual_body_with_available_host_compilers(tmp_path):
    compilers = [shutil.which(name) for name in ("clang++", "g++")]
    compilers = [compiler for compiler in compilers if compiler]
    if not compilers:
        import pytest
        pytest.skip("host Clang or GCC is required")

    harness = HARNESS.read_text(encoding="utf-8")
    anchor = "// ACTUAL_GRAPHICWIN_REINITIALIZE"
    assert harness.count(anchor) == 1
    harness = harness.replace(anchor, _actual_body())
    source = tmp_path / "graphicwin-reinitialize.cpp"
    source.write_text(harness, encoding="utf-8")

    for index, compiler in enumerate(compilers):
        binary = tmp_path / f"graphicwin-reinitialize-{index}"
        built = subprocess.run(
            [compiler, "-std=c++17", "-O1", "-Wall", "-Wextra", "-Werror",
             "-fsanitize=address,undefined", "-fno-sanitize-recover=all",
             str(source), "-o", str(binary)],
            text=True, capture_output=True, timeout=30,
        )
        assert built.returncode == 0, built.stdout + built.stderr
        run = subprocess.run(
            [str(binary)], text=True, capture_output=True, timeout=10,
            env=dict(os.environ, ASAN_OPTIONS="detect_leaks=0"),
        )
        assert run.returncode == 0, run.stdout + run.stderr
