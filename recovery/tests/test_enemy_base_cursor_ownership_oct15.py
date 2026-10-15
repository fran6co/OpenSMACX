"""Actual-source controls for removed local30/local44 cursor aliases."""
from pathlib import Path
import shutil
import subprocess

from test_enemy_move_product_source import PRODUCT, actual_unit


def _compile_and_run(tmp_path, compiler, mutation=None, record_mutation=None):
    name = mutation or record_mutation or "base_cursor"
    source = tmp_path / (name + ".cpp")
    executable = tmp_path / (name + ".exe")
    source.write_text(actual_unit(cursor_mutation=mutation,
                                 cursor_record_mutation=record_mutation))
    subprocess.run([
        compiler, "-std=c++98", "-Wall", "-Wextra", "-Werror", "-pedantic",
        "-Wno-unused-function", "-Wno-unused-label", "-Wno-array-bounds",
        "-O0", str(source), "-o", str(executable),
    ], check=True, capture_output=True, text=True, timeout=60)
    return subprocess.run([str(executable)], capture_output=True, text=True,
                          timeout=30)


def test_actual_indexed_base_regions_cover_zero_one_and_two_bases(tmp_path):
    compiler = shutil.which("g++") or shutil.which("clang++")
    assert compiler, "a host C++ compiler is required"
    result = _compile_and_run(tmp_path, compiler)
    assert result.returncode == 0, result.stderr


def test_skipping_a_base_in_any_actual_traversal_is_detected(tmp_path):
    compiler = shutil.which("g++") or shutil.which("clang++")
    assert compiler, "a host C++ compiler is required"
    for mutation in ("cursor30", "cursor44"):
        result = _compile_and_run(tmp_path, compiler, mutation)
        assert result.returncode != 0, mutation + " unexpectedly passed"


def test_repeating_first_base_record_is_detected_by_callback_and_data(tmp_path):
    compiler = shutil.which("g++") or shutil.which("clang++")
    assert compiler, "a host C++ compiler is required"
    result = _compile_and_run(tmp_path, compiler, record_mutation="cursor44")
    assert result.returncode != 0, "cursor44 repeated-first-record mutation passed"


def test_cursor_aliases_are_gone_but_scalar_frame_words_remain():
    source = PRODUCT.read_text()
    assert "base_cursor_30" not in source
    assert "base_cursor_44" not in source
    assert "union { int local_48; const short *base_cursor_48; };" in source
    assert source.count("f.base_cursor_48 =") == 3
    assert source.count("f.base_cursor_48 += sizeof(Base)/sizeof(short);") == 1
    for scalar in ("int local_30;", "int local_44;"):
        assert scalar in source
    for access in ("Bases[f.local_24].faction_id_current", "Bases[f.local_24].x",
                   "Bases[f.local_24].y", "Bases[f.local_78].x",
                   "Bases[f.local_78].y"):
        assert access in source
