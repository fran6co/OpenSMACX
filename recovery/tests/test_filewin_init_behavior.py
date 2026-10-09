"""Host controls that inject the proposed live FileWin::init body."""
from pathlib import Path
import shutil
import subprocess

import pytest


ROOT = Path(__file__).resolve().parent
SOURCE = ROOT.parents[1] / "src" / "jackal" / "filewin.cpp"
FIXTURE = ROOT / "filewin_init_host_control.cpp"


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
                state = "line"; index += 2; continue
            if char == "/" and nxt == "*":
                state = "block"; index += 2; continue
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
            if char == "\n": state = "code"
        elif state == "block":
            if char == "*" and nxt == "/":
                state = "code"; index += 2; continue
        elif state in ("string", "character"):
            if char == "\\": index += 2; continue
            if (state == "string" and char == '"') or (
                state == "character" and char == "'"
            ):
                state = "code"
        index += 1
    raise AssertionError(f"unclosed function body for {signature}")


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
def test_filewin_init_body(compiler_name, tmp_path):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(f"{compiler_name} is unavailable")
    source = SOURCE.read_text()
    body = extract_function(
        source, "int FileWin::init(char *, GraphicWin *parent, FileWinConstruct *parameters)"
    )
    fixture = FIXTURE.read_text()
    marker = "/* FILEWIN_INIT_BODY */"
    assert fixture.count(marker) == 1
    generated = fixture.replace(marker, body)
    generated_path = tmp_path / "filewin_init_host_control.cpp"
    generated_path.write_text(generated)
    executable = tmp_path / f"filewin-init-{compiler_name}"
    built = subprocess.run(
        [compiler, "-std=c++98", "-Wall", "-Wextra", "-I", str(ROOT),
         str(generated_path), "-o", str(executable)],
        capture_output=True, text=True, timeout=30, check=False,
    )
    assert built.returncode == 0, built.stdout + built.stderr
    ran = subprocess.run(
        [str(executable)], capture_output=True, text=True, timeout=30, check=False
    )
    assert ran.returncode == 0, ran.stdout + ran.stderr
