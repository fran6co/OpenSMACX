"""Actual-source controls for visible opening-window publication."""
import shutil

import pytest

from recovery.tests.test_newgame_menu import projected, run_control


@pytest.mark.parametrize("compiler_name", ["g++", "clang++"])
def test_opening_is_shown_before_each_actual_draw(tmp_path, compiler_name):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(compiler_name + " unavailable")
    result = run_control(tmp_path, compiler, projected("top_menu"))
    assert result.returncode == 0, result.stderr.decode(errors="replace")


@pytest.mark.parametrize("compiler_name", ["g++", "clang++"])
def test_soft_update_mutant_compiles_but_fails_visibility_witness(
        tmp_path, compiler_name):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(compiler_name + " unavailable")
    mutant = projected("top_menu", ("opening_window()->show(0);",
                                    "opening_window()->soft_update();"))
    result = run_control(tmp_path, compiler, mutant)
    assert result.returncode == 97, (
        result.returncode, result.stderr.decode(errors="replace"))
    assert b"SEMANTIC97: opening window was not visible before draw" in result.stderr


@pytest.mark.parametrize("compiler_name", ["g++", "clang++"])
def test_retry_presentation_must_publish_visibility(tmp_path, compiler_name):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(compiler_name + " unavailable")
    mutant = projected("top_menu", (
        "opening_window()->show(0);",
        "if (opening_change_calls != 3) opening_window()->show(0);"))
    result = run_control(tmp_path, compiler, mutant)
    assert result.returncode == 97, (
        result.returncode, result.stderr.decode(errors="replace"))
    assert b"SEMANTIC97: opening window was not shown before draw" in result.stderr


@pytest.mark.parametrize("compiler_name", ["g++", "clang++"])
def test_show_uses_original_zero_flags(tmp_path, compiler_name):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(compiler_name + " unavailable")
    mutant = projected("top_menu", ("opening_window()->show(0);",
                                    "opening_window()->show(4);"))
    result = run_control(tmp_path, compiler, mutant)
    assert result.returncode == 97, (
        result.returncode, result.stderr.decode(errors="replace"))
    assert b"SEMANTIC97: opening show flags were not zero" in result.stderr


@pytest.mark.parametrize("compiler_name", ["g++", "clang++"])
def test_popup_section_selector_mutant_is_rejected(tmp_path, compiler_name):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(compiler_name + " unavailable")
    mutant = projected("top_menu", (
        'edi == 0 ? "TOPMENU" : "HOTSEAT"',
        'edi == 0 ? "HOTSEAT" : "TOPMENU"'))
    result = run_control(tmp_path, compiler, mutant)
    assert result.returncode == 97, (
        result.returncode, result.stderr.decode(errors="replace"))
    assert b"SEMANTIC97: popup section does not match selected mode" in result.stderr
