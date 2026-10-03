"""Actual source body controls; mocked services are not native/game proof."""
from pathlib import Path
import shutil
import subprocess
import pytest
from match_recovery.source import read_file
from match_recovery.source.reader import _extract_forward_text

ROOT = Path(__file__).resolve().parents[2]
FIXTURES = Path(__file__).with_name("air_monolith_landmark")
CASES = {"mash_planes": ("src/game.cpp", 0x529C00), "monolith": ("src/veh.cpp", 0x57A050)}
COMPILERS = [c for c in ("g++", "clang++") if shutil.which(c)]

def actual(case):
    source, address = CASES[case]
    path = ROOT / source
    record = next(r for r in read_file(path) if r.address == address)
    body = _extract_forward_text(path.read_text().splitlines(), record.line, record.name)
    assert "BODY GOES HERE." not in body and "PENDING_BODY(" not in body
    return body

def execute(tmp_path, compiler, case, body):
    (tmp_path / "actual.inc").write_text(body)
    program = tmp_path / "control.cpp"
    program.write_bytes((FIXTURES / (case + ".cpp")).read_bytes())
    executable = tmp_path / "control"
    subprocess.run([compiler, "-std=c++98", "-Wall", "-Wextra", "-Werror", "-fsanitize=undefined", str(program), "-o", str(executable)], check=True, capture_output=True, text=True)
    return subprocess.run([str(executable)], capture_output=True, timeout=10)

@pytest.mark.parametrize("compiler", COMPILERS)
@pytest.mark.parametrize("case", CASES)
def test_current_product_bodies(tmp_path, compiler, case):
    assert execute(tmp_path, compiler, case, actual(case)).returncode == 0

@pytest.mark.parametrize("compiler", COMPILERS)
@pytest.mark.parametrize("case,before,after", [
    ("mash_planes", "speed(veh_id, false) - veh.moves_expended", "speed(veh_id, false) - veh.terraforming_turns"),
    ("mash_planes", "remaining_speed <= 0", "remaining_speed <= 0 || remaining_speed > 999"),
    ("mash_planes", "target - veh.dmg_incurred", "target - veh.terraforming_turns"),
    ("mash_planes", "growth_chassis.range == 1", "growth_chassis.triad == TRIAD_SEA"),
    ("mash_planes", ".plan", ".weapon_id"),
    ("monolith", "initial_proto_id == 0x10", "VehPrototypes[initial_proto_id].weapon_id == 0x10"),
    ("monolith", "veh.state & VSTATE_MONOLITH_UPGRADED", "veh.flags & VSTATE_MONOLITH_UPGRADED"),
    ("monolith", "bit_visible[site_id - 1]", "bit2"),
    ("monolith", "if (next_morale > 6u) next_morale = 6u;", ""),
])
def test_controls_detect_damaged_operands(tmp_path, compiler, case, before, after):
    body = actual(case)
    assert before in body
    assert execute(tmp_path, compiler, case, body.replace(before, after)).returncode != 0
