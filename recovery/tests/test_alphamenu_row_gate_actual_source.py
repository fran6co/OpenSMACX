from __future__ import annotations

import shutil
import subprocess
import tempfile
from pathlib import Path

import pytest

from match_recovery.source import State, read_file


ROOT = Path(__file__).resolve().parents[2]
SOURCE = ROOT / "src/alphamenu.cpp"


def _expression(source: str) -> str:
    start = source.index("const bool row_action_flag =")
    end = source.index(";", start) + 1
    return source[start:end]


def _compile_and_run(compiler: str, expression: str) -> subprocess.CompletedProcess[str]:
    program = f"""
struct Map {{ unsigned int bit; }};
enum {{ BIT_FARM = 0x8000 }};
static bool row_gate(unsigned int bits, int terraform_id) {{
    Map storage = {{ bits }};
    const Map *selected_tile = &storage;
    {expression}
    return row_action_flag;
}}
int main() {{
    if (!row_gate(0x8000u, 1)) return 1;
    if (row_gate(0x80000000u, 1)) return 2;
    if (row_gate(0u, 1)) return 3;
    if (!row_gate(0x4u, 6)) return 4;
    if (row_gate(0u, 6)) return 5;
    if (!row_gate(0u, 2)) return 6;
    return 0;
}}
"""
    with tempfile.TemporaryDirectory(prefix="osmx-alphamenu-row-") as name:
        directory = Path(name)
        cpp = directory / "row_gate.cpp"
        binary = directory / "row_gate"
        cpp.write_text(program)
        compile_result = subprocess.run(
            [compiler, "-std=c++98", "-Wall", "-Wextra", "-Werror",
             str(cpp), "-o", str(binary)],
            text=True, capture_output=True, check=False,
        )
        assert compile_result.returncode == 0, compile_result.stdout + compile_result.stderr
        return subprocess.run([str(binary)], text=True, capture_output=True,
                              check=False)


def test_actual_owner_identity_and_row_gate() -> None:
    source = SOURCE.read_text()
    record = next(item for item in read_file(SOURCE)
                  if item.address == 0x00460DD0)
    assert record.name == "?check@MainMenu@@QAEXH@Z"
    assert record.symbol == "?check@AlphaMenu@@QAEXH@Z"
    assert record.image_spans == ((0x00460DD0, 0x0046207E),)
    assert record.state is State.IMPLEMENTED
    assert "BIT_FARM = 0x8000" in (ROOT / "src/map.h").read_text()
    expr = _expression(source)
    assert "terraform_id == 1 && (selected_tile->bit & BIT_FARM) != 0" in expr
    assert "terraform_id == 6 && (selected_tile->bit & 0x4u) != 0" in expr
    for compiler in (shutil.which("g++"), shutil.which("clang++")):
        if not compiler:
            pytest.skip("both GCC and Clang are required for this bounded control")
        result = _compile_and_run(compiler, expr)
        assert result.returncode == 0, result.stdout + result.stderr


@pytest.mark.parametrize("old,new", [
    ("BIT_FARM", "0x80000000u"),
    ("0x4u", "BIT_FARM"),
])
def test_row_gate_mutants_are_rejected(old: str, new: str) -> None:
    expr = _expression(SOURCE.read_text())
    mutant = expr.replace(old, new, 1)
    assert mutant != expr
    for compiler in (shutil.which("g++"), shutil.which("clang++")):
        if not compiler:
            pytest.skip("both GCC and Clang are required for this bounded control")
        result = _compile_and_run(compiler, mutant)
        assert result.returncode != 0, f"surviving mutant: {compiler}\n{result.stdout}{result.stderr}"
