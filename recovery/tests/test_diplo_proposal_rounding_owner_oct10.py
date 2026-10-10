"""Actual-source controls for three pinned signed diplomacy rounders."""
from pathlib import Path
import re
import resource
import shutil
import subprocess
import tempfile

import pytest


ROOT = Path(__file__).resolve().parents[2]
SOURCE = ROOT / "src" / "faction.cpp"
FIXTURE = Path(__file__).parent / "fixtures" / "diplo_proposal_rounding_owner_oct10.cpp"


def _matching_brace(text: str, opening: int) -> int:
    depth = 0
    for index in range(opening, len(text)):
        if text[index] == "{":
            depth += 1
        elif text[index] == "}":
            depth -= 1
            if depth == 0:
                return index
    raise AssertionError("unclosed C++ block")


def _function(source: str, signature: str) -> str:
    start = source.index(signature)
    opening = source.index("{", start)
    return source[start:_matching_brace(source, opening) + 1]


def _statement(source: str, prefix: str) -> str:
    start = source.index(prefix)
    end = source.index(";", start) + 1
    return source[start:end]


def _materialize(source: str) -> str:
    helper = _function(source, "static bool propose_pact_ahead_refusal(")
    actor_energy = _statement(source, "const int actor_energy = PlayersData[actor].energy_reserves")
    actor_strength = _statement(source, "const int actor_strength = PlayersData[actor].mil_strength_1")
    other_strength = _statement(source, "const int other_strength = PlayersData[other].mil_strength_1")
    weighted = _statement(source, "const int weighted_energy =")
    treaty_score = _statement(source, "int treaty_score = diplo_mul(weighted_energy")
    attack_score = _statement(source, "const int score_after_class =")
    attack_threshold = _statement(source, "const int proposal_attack_threshold =")

    assert "comparison / 50, 50" in helper
    assert "weighted_energy / 50, 50" in treaty_score
    assert "score_after_class, 24) / 25, 25" in attack_threshold

    fixture = FIXTURE.read_text()
    return (fixture.replace("/* ACTUAL_PACT_HELPER */", helper)
            .replace("/* ACTUAL_TREATY_INPUTS */",
                     "\n    ".join((actor_energy, actor_strength, other_strength)))
            .replace("/* ACTUAL_WEIGHTED_ENERGY_AND_TREATY_SCORE */",
                     weighted + "\n    " + treaty_score)
            .replace("/* ACTUAL_PROPOSAL_ATTACK_STAGES */",
                     attack_score + "\n    " + attack_threshold))


def _compile_run(source: str, tmp_path: Path, compiler: str):
    cpp = tmp_path / "actual_source.cpp"
    executable = tmp_path / "actual_source"
    cpp.write_text(source)
    built = subprocess.run(
        [compiler, "-std=c++17", "-O0", "-fwrapv", str(cpp), "-o", str(executable)],
        text=True, capture_output=True)
    assert built.returncode == 0, built.stdout + built.stderr

    def disable_core_dumps() -> None:
        resource.setrlimit(resource.RLIMIT_CORE, (0, 0))

    return subprocess.run([str(executable)], text=True, capture_output=True,
                          preexec_fn=disable_core_dumps)


@pytest.mark.parametrize("compiler_name", ["g++", "clang++"])
def test_actual_diplomacy_rounding_pipeline_and_three_wrong_formula_mutants(
        compiler_name, tmp_path):
    compiler = shutil.which(compiler_name)
    if not compiler:
        pytest.skip(f"{compiler_name} unavailable")

    source = SOURCE.read_text()
    actual = _materialize(source)
    result = _compile_run(actual, tmp_path, compiler)
    assert result.returncode == 0, result.stdout + result.stderr

    mutants = (
        ("comparison / 50, 50", "comparison / 100, 50", "pact /100 mutant"),
        ("weighted_energy / 50, 50", "weighted_energy / 100, 50",
         "treaty /100*50 mutant"),
        ("score_after_class, 24) / 25, 25",
         "score_after_class, 24) / 100, 25", "attack /100*25 mutant"),
    )
    for correct, wrong, label in mutants:
        mutant = actual.replace(correct, wrong, 1)
        assert mutant != actual, f"{label} replacement anchor missing"
        result = _compile_run(mutant, tmp_path, compiler)
        assert result.returncode != 0, f"{label} compiled but escaped controls"

