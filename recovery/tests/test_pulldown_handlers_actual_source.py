"""Post-migration host controls: inject only the mapped product definitions."""
from pathlib import Path
import os
import shutil
import subprocess
import tempfile

from match_recovery.source import read_file

ROOT = Path(__file__).resolve().parents[2]
HARNESS = Path(__file__).with_name("pulldown_handler_harness.cpp")
TARGETS = (
    (0x005F93C0, "void PullDown::on_left_down(int x, int y)"),
    (0x005F9480, "void PullDown::on_left_up(int x, int y)"),
)


def definition(source: str, signature: str) -> str:
    start = source.index(signature)
    opening = source.index("{", start)
    depth = 0
    for end in range(opening, len(source)):
        depth += (source[end] == "{") - (source[end] == "}")
        if depth == 0:
            return source[start:end + 1]
    raise AssertionError("unterminated product body: " + signature)


def actual_bodies():
    path = ROOT / "src/pulldown.cpp"
    records = read_file(path)
    source = path.read_text()
    bodies = []
    for address, signature in TARGETS:
        record, = [r for r in records if r.address == address]
        assert Path(record.path) == path
        assert record.region
        assert Path(record.path) == path
        assert signature in record.region
        body = definition(source, signature)
        assert "BODY GOES HERE" not in body
        bodies.append(body)
    return bodies


def test_actual_pulldown_mouse_handlers(tmp_path):
    compiler = shutil.which("clang++") or shutil.which("g++")
    if compiler is None:
        import pytest
        pytest.skip("host GCC or Clang required")
    harness = HARNESS.read_text()
    assert harness.count("// ACTUAL_PULLDOWN_LEFT_DOWN") == 1
    assert harness.count("// ACTUAL_PULLDOWN_LEFT_UP") == 1
    down, up = actual_bodies()
    harness = harness.replace("// ACTUAL_PULLDOWN_LEFT_DOWN", down)
    harness = harness.replace("// ACTUAL_PULLDOWN_LEFT_UP", up)
    source = tmp_path / "pulldown-handlers.cpp"
    binary = tmp_path / "pulldown-handlers"
    source.write_text(harness)
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
