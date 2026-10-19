"""Exercise the mapped scalar and RECT right-aligned Buffer writers."""

from __future__ import annotations

from pathlib import Path
import shutil

import pytest

from test_buffer_typed_font_consumers_oct10 import (
    ROOT,
    compile_and_run,
    source_program,
)


@pytest.mark.parametrize("compiler_name", ["g++", "clang++"])
def test_actual_right_writers_preserve_lengths_coordinates_and_callback_order(
        compiler_name: str, tmp_path: Path) -> None:
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(f"{compiler_name} unavailable")
    result = compile_and_run(compiler, source_program(ROOT), tmp_path, "right_writers")
    assert result.returncode == 0, result.stdout + result.stderr


@pytest.mark.parametrize("compiler_name", ["g++", "clang++"])
@pytest.mark.parametrize("mutation", [
    "cached_right_strlen", "scalar_x_direction", "rect_wrong_y",
])
def test_actual_right_writer_controls_reject_semantic_mutants(
        compiler_name: str, mutation: str, tmp_path: Path) -> None:
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(f"{compiler_name} unavailable")
    result = compile_and_run(
        compiler, source_program(ROOT, mutation), tmp_path, mutation)
    assert result.returncode != 0, f"{compiler_name} accepted {mutation} mutation"
