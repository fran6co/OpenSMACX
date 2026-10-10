"""Actual-source host controls for the pact/treaty 50-energy buckets."""
from pathlib import Path
import re
import resource
import shutil
import subprocess
import tempfile

import pytest


ROOT = Path(__file__).resolve().parents[2]
SOURCE = ROOT / "src" / "faction.cpp"
FIXTURE = Path(__file__).parent / "fixtures" / "diplo_gift_buckets_owner_oct10.cpp"


def _matching_brace(text: str, opening: int) -> int:
    depth = 0
    for i in range(opening, len(text)):
        if text[i] == "{":
            depth += 1
        elif text[i] == "}":
            depth -= 1
            if depth == 0:
                return i
    raise AssertionError("unclosed source block")


def _block_after(source: str, needle: str) -> str:
    start = source.index(needle)
    opening = source.index("{", start)
    return source[start:_matching_brace(source, opening) + 1]


def _materialize(source: str) -> str:
    pact_expr = re.search(r"int cap = diplo_mul\([^;]+;", source)
    assert pact_expr, "actual make_pact cap expression not found"
    pact_adjust = _block_after(source, "if (DiplomacyProposalMode != 5 && cap != 0)")
    treaty_start = source.index(
        "if (DiplomacyProposalMode != 5 && treaty_score > PlayersData[actor].energy_reserves)"
    )
    treaty_end = source.index("\n    }\n    if (treaty_score < 50)", treaty_start) + len("\n    }")
    treaty_block = source[treaty_start:treaty_end]
    minimum_block = "if (treaty_score < 50) treaty_score = 0;"

    assert "cap_numerator / cap_denominator / 50" in pact_expr.group(0)
    assert "excess_plus_49 / 50u" in treaty_block
    assert "rounded_fifties" in treaty_block
    fixture = FIXTURE.read_text()
    return (fixture.replace("/* PACT_CAP_EXPRESSION */", pact_expr.group(0))
            .replace("/* PACT_ADJUSTMENT_BLOCK */", pact_adjust)
            .replace("/* TREATY_BUCKET_BLOCK */", treaty_block)
            .replace("/* TREATY_MINIMUM_BLOCK */", minimum_block))


def _compile_and_run(code: str, tmp: Path) -> subprocess.CompletedProcess:
    compiler = shutil.which("c++") or shutil.which("g++") or shutil.which("clang++")
    if not compiler:
        pytest.skip("host C++ compiler unavailable")
    source_path = tmp / "control.cpp"
    executable = tmp / "control"
    source_path.write_text(code)
    built = subprocess.run([compiler, "-std=c++17", "-O0", "-fwrapv",
                            str(source_path), "-o", str(executable)],
                           text=True, capture_output=True)
    assert built.returncode == 0, built.stdout + built.stderr
    def disable_core_dumps() -> None:
        resource.setrlimit(resource.RLIMIT_CORE, (0, 0))

    return subprocess.run([str(executable)], text=True, capture_output=True,
                          preexec_fn=disable_core_dumps)


def test_actual_pact_and_treaty_bucket_sources_and_mutants():
    source = SOURCE.read_text()
    actual = _materialize(source)
    with tempfile.TemporaryDirectory(prefix="diplo-gift-buckets-") as temp:
        tmp = Path(temp)
        result = _compile_and_run(actual, tmp)
        assert result.returncode == 0, result.stdout + result.stderr

        # A compiler-successful wrong pact divisor must fail runtime controls.
        pact_mutant = actual.replace("cap_numerator / cap_denominator / 50",
                                     "cap_numerator / cap_denominator / 100", 1)
        result = _compile_and_run(pact_mutant, tmp)
        assert result.returncode != 0, "pact /100 mutant escaped bucket controls"

        # The treaty excess uses unsigned 50-energy buckets; changing the
        # divisor to 100 must fail the boundary controls.
        treaty_mutant = actual.replace("excess_plus_49 / 50u",
                                       "excess_plus_49 / 100u", 1)
        result = _compile_and_run(treaty_mutant, tmp)
        assert result.returncode != 0, "treaty /100u mutant escaped bucket controls"

        clamp_mutant = actual.replace(
            "cap = diplo_sub(cap, 50);\n                if (cap < 0) { cap = 0; break; }\n                if (cap > 0x1869f) cap = 0x1869f;",
            "if (cap > 0x1869f) cap = 0x1869f;\n                cap = diplo_sub(cap, 50);\n                if (cap < 0) { cap = 0; break; }", 1)
        assert clamp_mutant != actual, "pact clamp-order mutation anchor missing"
        result = _compile_and_run(clamp_mutant, tmp)
        assert result.returncode != 0, "pact clamp-order mutant escaped controls"


def test_source_preserves_original_unguarded_and_branch_ordering():
    source = SOURCE.read_text()
    cap_line = re.search(r"int cap = diplo_mul\([^;]+;", source).group(0)
    assert "/ cap_denominator / 50" in cap_line
    # No safe fallback is introduced around either native signed division.
    assert "if (cap_denominator" not in source
    adjustment = _block_after(source, "if (DiplomacyProposalMode != 5 && cap != 0)")
    assert adjustment.index("while (cap > energySnapshot)") < adjustment.index("cap = diplo_sub(cap, 50)")
    assert adjustment.index("if (cap < 0)") < adjustment.index("if (cap > 0x1869f)")
    assert "if (DiplomacyProposalMode != 5 && treaty_score > PlayersData[actor].energy_reserves)" in source
    assert source.index("const uint32_t excess_plus_49") < source.index("excess_plus_49 / 50u")
    assert source.index("excess_plus_49 / 50u") < source.index("if (treaty_score < 50)")
