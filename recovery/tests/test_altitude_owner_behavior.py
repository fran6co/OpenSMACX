"""Actual terrain source controls with synthetic tables, not ABI/game proof."""
from pathlib import Path
import shutil
import subprocess
import pytest
from match_recovery.source import read_file
from match_recovery.source.reader import _extract_forward_text, _block_state_after
ROOT = Path(__file__).resolve().parents[2]
FIXTURES = Path(__file__).with_name("altitude_owners")
COMPILERS = [c for c in ("g++", "clang++") if shutil.which(c)]

def actual(address):
    path = ROOT / "src/map.cpp"
    record = next(r for r in read_file(path) if r.address == address)
    lines = path.read_text().splitlines()
    block = False
    for line in lines[:record.line - 1]:
        block = _block_state_after(line, block)
    return _extract_forward_text(lines, record.line, record.name, in_block=block)

def execute(tmp_path, compiler, case, body):
    fixture = FIXTURES / ("alt_set.cpp" if case == "alt_set" else "control.cpp")
    source = tmp_path / "control.cpp"
    source.write_text(fixture.read_text().replace("/* ACTUAL_BODIES */", body))
    # Synthetic valid lookup rows distinguish row/corner indexing. They do not
    # claim original data; pinned initializer and native data checks are separate.
    values = [row if column == 0 else (row + column * 17) % 113
              for row in range(49) for column in range(5)]
    (tmp_path / "alt_direction_table.inc").write_text(",".join(map(str, values)))
    binary = tmp_path / "control"
    subprocess.run([compiler, "-std=c++98", "-Wall", "-Wextra", "-Werror",
                    "-fsanitize=undefined", "-fno-sanitize-recover=all", str(source),
                    "-o", str(binary)], check=True, capture_output=True, text=True)
    return subprocess.run([str(binary)], capture_output=True, timeout=10)

@pytest.mark.parametrize("compiler", COMPILERS)
@pytest.mark.parametrize("case", ["alt_set", "delete_landmark"])
def test_actual_source_transitions(tmp_path, compiler, case):
    body = actual(0x591290) if case == "alt_set" else actual(0x592940) + "\n" + actual(0x5927D0)
    result = execute(tmp_path, compiler, case, body)
    assert result.returncode == 0, result.stderr

@pytest.mark.parametrize("compiler", COMPILERS)
@pytest.mark.parametrize("case,before,after", [
    ("alt_set", "!has_fac_built_call(FAC_PRESSURE_DOME, base_id)", "has_fac_built_call(FAC_PRESSURE_DOME, base_id)"),
    ("delete_landmark", "if (old_flags == 0)", "if (old_flags != 0)"),
    ("delete_landmark", "window->field_1DD74_ != 0", "window->field_1DD74_ == 0"),
    ("delete_landmark", "AltBonusFlat[digits[i]] = 0", "AltBonusOcean[digits[i]] = 0"),
    ("delete_landmark", "static_cast<int32_t>(tile->bit2) >> 24", "tile->bit2 >> 24"),
    ("delete_landmark", "AltBonusMoist[digits[i]] = 0", "AltBonusMoist[digits[i]] = digits[4]"),
])
def test_damage_controls(tmp_path, compiler, case, before, after):
    body = actual(0x591290) if case == "alt_set" else actual(0x592940) + "\n" + actual(0x5927D0)
    assert before in body
    result = execute(tmp_path, compiler, case, body.replace(before, after))
    assert result.returncode != 0
