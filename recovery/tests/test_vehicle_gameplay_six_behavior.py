"""Execute six current product bodies against bounded C++98 service doubles.

These controls cover selected behavior and corrected native operands/order.
They do not establish original ABI, byte fidelity, linking or game acceptance.
"""
from pathlib import Path
import re
import shutil
import subprocess

import pytest
from match_recovery.source import read_file
from match_recovery.source.reader import _extract_forward_text

ROOT = Path(__file__).resolve().parents[2]
FIXTURES = Path(__file__).with_name("vehicle_gameplay_six")
CASES = {
    "alien_start": ("src/game.cpp", (0x588F90,)),
    "fauna": ("src/game.cpp", (0x522180,)),
    "upkeep": ("src/game.cpp", (0x5258C0,)),
    "combat": ("src/veh.cpp", (0x505D80, 0x506650)),
    "passengers": ("src/veh.cpp", (0x564B90,)),
}
COMPILERS = [compiler for compiler in ("g++", "clang++") if shutil.which(compiler)]


def actual(case):
    source, addresses = CASES[case]
    path = ROOT / source
    text = path.read_text()
    lines = text.splitlines()
    records = {record.address: record for record in read_file(path)}
    definitions = []
    if case == "fauna":
        for name in ("alien_random_x", "alien_random_y", "alien_normalize_x",
                     "alien_signed_wrap", "alien_local_vehicle_visible",
                     "alien_tile_vehicle_visible"):
            line = next(index + 1 for index, value in enumerate(lines)
                        if re.match(r"static (?:int|bool) " + name + r"\(", value))
            definitions.append(_extract_forward_text(lines, line, name))
    for address in addresses:
        record = records[address]
        definitions.append(_extract_forward_text(lines, record.line, record.name))
    body = "\n".join(definitions)
    assert "BODY GOES HERE." not in body and "PENDING_BODY(" not in body
    return body


def execute(tmp_path, compiler, case, body):
    (tmp_path / "actual.inc").write_text(body)
    (tmp_path / "fixture.h").write_bytes((FIXTURES / "fixture.h").read_bytes())
    program = tmp_path / "control.cpp"
    program.write_bytes((FIXTURES / (case + ".cpp")).read_bytes())
    executable = tmp_path / "control"
    subprocess.run([compiler, "-std=c++98", "-Wall", "-Wextra", "-Werror",
                    str(program), "-o", str(executable)], check=True,
                   capture_output=True, text=True)
    return subprocess.run([str(executable)], capture_output=True, timeout=10)


@pytest.mark.parametrize("compiler", COMPILERS)
@pytest.mark.parametrize("case", CASES)
def test_current_product_bodies(tmp_path, compiler, case):
    assert execute(tmp_path, compiler, case, actual(case)).returncode == 0


@pytest.mark.parametrize("compiler", COMPILERS)
@pytest.mark.parametrize("case,before,after", [
    ("combat", "Vehs[vehicle_id].state &= 0xFCFFBDFFU;", ""),
    ("fauna", "3u - MapNativeLifeForms", "3u - static_cast<uint32_t>(DiffLevelCurrent)"),
    ("passengers", "unk_77[region] & 0x0800", "unk_77[region] & 0x0400"),
    ("passengers", "mode < 3", "mode < 4"),
])
def test_behavior_damage_controls(tmp_path, compiler, case, before, after):
    body = actual(case)
    assert before in body
    assert execute(tmp_path, compiler, case, body.replace(before, after, 1)).returncode != 0
