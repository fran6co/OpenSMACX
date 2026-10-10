"""Host controls for threaten's source-owned signed reserve cap."""
from pathlib import Path
import shutil
import subprocess

import pytest


ROOT = Path(__file__).resolve().parents[2]
SOURCE = ROOT / "src" / "faction.cpp"
FIXTURE = ROOT / "recovery" / "tests" / "fixtures" / "threaten_reserve_cap_owner_oct10.cpp"


def _braced_definition(text: str, signature: str) -> str:
    start = text.index(signature)
    opening = text.index("{", start)
    depth = 0
    for end in range(opening, len(text)):
        if text[end] == "{":
            depth += 1
        elif text[end] == "}":
            depth -= 1
            if depth == 0:
                return text[start:end + 1]
    raise AssertionError(f"unterminated definition: {signature}")


def _parts(source_text: str):
    multiply = _braced_definition(source_text, "static int diplo_mul(int a, int b) {")
    threaten = _braced_definition(source_text, "void __cdecl threaten(int faction_id, int other_faction_id)")
    start = threaten.index("    int demand = energy_delta / 16;")
    stop = threaten.index("\n\n    if (DiploScratchFA98 == 0)", start)
    cap_block = threaten[start:stop]
    assignment = "demand = "
    expression_start = cap_block.index(assignment, cap_block.index("if (PlayersData[other_faction_id].energy_reserves < demand)")) + len(assignment)
    expression_end = cap_block.index(";", expression_start)
    expression = cap_block[expression_start:expression_end]
    candidate = expression.replace("PlayersData[other_faction_id].energy_reserves", "reserve")
    assert candidate != expression
    return multiply, cap_block, candidate


@pytest.mark.parametrize("compiler", ["g++", "clang++"])
@pytest.mark.parametrize("damage", [False, True])
def test_actual_threaten_reserve_cap(compiler, damage, tmp_path):
    executable = shutil.which(compiler)
    if executable is None:
        pytest.skip("host compiler unavailable")

    source_text = SOURCE.read_text()
    multiply, cap_block, candidate = _parts(source_text)
    if damage:
        assert "/ 25" in cap_block
        cap_block = cap_block.replace("/ 25", "/ 100", 1)
        candidate = candidate.replace("/ 25", "/ 100", 1)

    fixture = FIXTURE.read_text()
    fixture = fixture.replace("/* SOURCE_DIPLO_MUL */", multiply, 1)
    fixture = fixture.replace("/* SOURCE_CAP_EXPRESSION */", f"return {candidate};", 1)
    fixture = fixture.replace("/* SOURCE_CAP_BLOCK */", cap_block, 1)
    unit = tmp_path / "threaten_reserve_cap.cpp"
    binary = tmp_path / "threaten_reserve_cap"
    unit.write_text(fixture)
    built = subprocess.run(
        [executable, "-std=c++98", "-Wall", "-Wextra", "-Werror", str(unit), "-o", str(binary)],
        capture_output=True, text=True, timeout=60,
    )
    assert built.returncode == 0, built.stdout + built.stderr
    run = subprocess.run([str(binary)], capture_output=True, text=True, timeout=30)
    if damage:
        assert run.returncode != 0 and "cap_candidate(25)" in run.stderr, run.stdout + run.stderr
    else:
        assert run.returncode == 0, run.stdout + run.stderr
