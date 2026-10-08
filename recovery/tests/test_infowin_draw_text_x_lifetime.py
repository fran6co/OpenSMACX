"""Actual-source control for draw_text's persistent x stack slot."""
from pathlib import Path
import shutil
import subprocess

import pytest


ROOT = Path(__file__).resolve().parents[2]
HARNESS = Path(__file__).with_name("infowin_draw_text_x_harness.cpp")


def function_source(source: str) -> str:
    signature = "void InfoWin::draw_text(int pane)"
    start = source.index(signature)
    opening = source.index("{", start + len(signature))
    depth = 0
    state = "code"
    escaped = False
    i = opening
    while i < len(source):
        c = source[i]
        n = source[i + 1] if i + 1 < len(source) else ""
        if state == "line":
            if c == "\n": state = "code"
        elif state == "block":
            if c == "*" and n == "/":
                state = "code"
                i += 1
        elif state in ("string", "char"):
            if escaped:
                escaped = False
            elif c == "\\":
                escaped = True
            elif (state == "string" and c == '"') or (state == "char" and c == "'"):
                state = "code"
        elif c == "/" and n == "/":
            state = "line"
            i += 1
        elif c == "/" and n == "*":
            state = "block"
            i += 1
        elif c == '"':
            state = "string"
        elif c == "'":
            state = "char"
        elif c == "{":
            depth += 1
        elif c == "}":
            depth -= 1
            if depth == 0:
                return source[start:i + 1]
        i += 1
    raise AssertionError("unterminated InfoWin::draw_text")


def validate_x_lifetime(body: str) -> None:
    """Require one persistent x declaration before both row/column loops."""
    assert body.count("int x;") == 1
    x_at = body.index("int x;")
    column_at = body.index("for (int column")
    row_at = body.index("for (; row")
    assert x_at < column_at < row_at


def compile_and_run(tmp_path: Path, compiler_name: str) -> None:
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(f"{compiler_name} unavailable")
    source = (ROOT / "src/infowin.cpp").read_text()
    body = function_source(source)
    harness = HARNESS.read_text()
    assert harness.count("/*ACTUAL_SOURCE*/") == 1
    translation_unit = harness.replace("/*ACTUAL_SOURCE*/", body)
    cpp = tmp_path / f"draw_text_{compiler_name}.cpp"
    exe = tmp_path / f"draw_text_{compiler_name}"
    cpp.write_text(translation_unit)
    subprocess.run([compiler, "-std=c++03", "-O0", "-Wall", "-Wextra", "-Werror",
                    str(cpp), "-o", str(exe)], check=True, timeout=45)
    subprocess.run([str(exe)], check=True, timeout=15)


@pytest.mark.parametrize("compiler_name", ["g++", "clang++"])
def test_actual_source_carries_x_from_positive_partial_row_to_zero_row(
        tmp_path: Path, compiler_name: str) -> None:
    source = (ROOT / "src/infowin.cpp").read_text()
    body = function_source(source)
    # The mapped function has one function-lifetime x, before both loops. This
    # is essential: in the fixture, row 0 consumes the budget exactly and row
    # 1's zero-budget tail must use the value returned from row 0's first write.
    validate_x_lifetime(body)
    compile_and_run(tmp_path, compiler_name)


def test_inner_partial_block_lifetime_mutation_is_rejected() -> None:
    source = (ROOT / "src/infowin.cpp").read_text()
    body = function_source(source)
    damaged = body.replace("    int x;\n", "", 1).replace(
        "                    char prefix[80];\n",
        "                    char prefix[80];\n                    int x;\n", 1)
    assert damaged != body
    # Structural negative control: the same contract used for the actual
    # method must reject a block-scoped per-row declaration. This does not
    # claim a host runtime test can establish the original UB behavior.
    with pytest.raises(AssertionError):
        validate_x_lifetime(damaged)
