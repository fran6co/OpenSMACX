import shutil
import subprocess
from pathlib import Path

import pytest


ROOT = Path(__file__).resolve().parent
PROJECT_ROOT = ROOT.parents[1]


def extract_method(source: str, signature: str) -> str:
    start = source.index(signature)
    opening = source.index("{", start)
    depth = 0
    for index in range(opening, len(source)):
        if source[index] == "{":
            depth += 1
        elif source[index] == "}":
            depth -= 1
            if depth == 0:
                return source[start : index + 1]
    raise AssertionError("unbalanced proposed BattleWin body")


@pytest.mark.parametrize("compiler", ["g++", "clang++"])
def test_actual_battle_report_body(compiler, tmp_path):
    executable = shutil.which(compiler)
    if executable is None:
        pytest.skip(f"{compiler} is not installed")

    proposal = (PROJECT_ROOT / "src/battlewin.cpp").read_text()
    body = extract_method(proposal, "void BattleWin::battle_report(")
    harness = (ROOT / "battlewin_battle_report_host_control.cpp").read_text()
    harness = harness.replace('#include "battlewin_battle_report_host_types.h"', "", 1)
    translation_unit = (
        '#include "battlewin_battle_report_host_types.h"\n'
        + body
        + "\n"
        + harness
    )
    source = tmp_path / "battle_report_extracted.cpp"
    binary = tmp_path / "battle_report_host"
    source.write_text(translation_unit)
    built = subprocess.run(
        [executable, "-std=c++98", "-Wall", "-Wextra", "-I", str(ROOT),
         str(source), "-o", str(binary)],
        capture_output=True,
        text=True,
        timeout=30,
    )
    assert built.returncode == 0, built.stdout + built.stderr

    completed = subprocess.run(
        [str(binary)], capture_output=True, text=True, timeout=30
    )
    assert completed.returncode == 0, (
        f"host control returned {completed.returncode}\n"
        + completed.stdout
        + completed.stderr
    )
