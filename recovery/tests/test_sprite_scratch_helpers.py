"""Actual-source side-effect controls; not native ABI or game acceptance."""
from pathlib import Path
import shutil
import subprocess

import pytest

from match_recovery.source import read_file
from test_game_logic_seed_batch_behavior import _definition


ROOT = Path(__file__).resolve().parents[2]
ADDRESSES = (0x404860, 0x450DE0, 0x450E30, 0x450E80, 0x450ED0, 0x450F30, 0x450F90)
NAMES = ("sprite_draw_func", "sub_450de0", "sub_450e30", "sub_450e80",
         "sub_450ed0", "sub_450f30", "sub_450f90")


def actual_bodies():
    records = {r.address: r for r in read_file(ROOT / "src/sprite.cpp")}
    bodies = []
    for address, name in zip(ADDRESSES, NAMES):
        record = records[address]
        assert record.region and "BODY GOES HERE." not in record.region
        result_type = "void" if name in ("sprite_draw_func", "sub_450f30") else "int"
        bodies.append(_definition(record.region, result_type + " __cdecl " + name + "("))
    return "\n".join(bodies)


def run_control(tmp_path, compiler, bodies):
    fixture = (Path(__file__).parent / "sprite_scratch_helpers.cpp").read_text()
    assert fixture.count("// ACTUAL_SOURCE_BODIES") == 1
    source = tmp_path / "control.cpp"
    source.write_text(fixture.replace("// ACTUAL_SOURCE_BODIES", bodies))
    binary = tmp_path / "control"
    subprocess.run([compiler, "-std=c++98", "-Wall", "-Wextra", "-Werror",
                    "-pedantic", str(source), "-o", str(binary)],
                   check=True, capture_output=True)
    return subprocess.run([str(binary)], capture_output=True)


@pytest.mark.parametrize("compiler_name", ["g++", "clang++"])
def test_actual_scratch_helpers_and_secondary_base(tmp_path, compiler_name):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(compiler_name + " is unavailable")
    result = run_control(tmp_path, compiler, actual_bodies())
    assert result.returncode == 0, result.stderr.decode(errors="replace")


@pytest.mark.parametrize("damage", ["base_adjustment", "remap_order", "dimensions", "texture_dimensions", "transparency"])
def test_controls_detect_damaged_source(tmp_path, damage):
    compiler = shutil.which("g++")
    if compiler is None:
        pytest.skip("g++ is unavailable")
    bodies = actual_bodies()
    replacements = {
        "base_adjustment": ("static_cast<Buffer *>(owner)", "reinterpret_cast<Buffer *>(owner)"),
        "remap_order": ("buffer->change_color(0x107, 0x109);", "buffer->change_color(0x106, 0x109);"),
        "dimensions": ("100, 50, 0", "100, 51, 0"),
        "texture_dimensions": ("0x38, 0x38, 0", "0x39, 0x38, 0"),
        "transparency": ("sprite->transparent_index()", "8"),
    }
    old, new = replacements[damage]
    assert old in bodies
    result = run_control(tmp_path, compiler, bodies.replace(old, new, 1))
    assert result.returncode < 0, "damaged helper must fail at execution"
