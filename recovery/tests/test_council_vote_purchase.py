"""Actual-product-source regression harness for buy_council_vote.

The fixture contains typed dependency doubles; only the production function is
extracted from src/faction.cpp. It refuses the retained BODY GOES HERE marker.
"""
from pathlib import Path
import os
import re
import shutil
import subprocess
import tempfile

import pytest
from match_recovery.source import read_file


ROOT = Path(__file__).resolve().parents[2]
PRODUCT = ROOT / "src/faction.cpp"
HARNESS = Path(__file__).with_name("council_vote_purchase_harness.cpp")
MARKER = "// PRODUCT_BODY_INSERTION_POINT"
SIGNATURE = "void __cdecl buy_council_vote("


class PlaceholderBody(RuntimeError):
    pass


def extract_body(source: str) -> str:
    start = source.find(SIGNATURE)
    if start < 0:
        raise RuntimeError("product buy_council_vote definition was not found")
    opening = source.find("{", start)
    if opening < 0:
        raise RuntimeError("product buy_council_vote has no body")
    depth = 0
    for pos in range(opening, len(source)):
        if source[pos] == "{":
            depth += 1
        elif source[pos] == "}":
            depth -= 1
            if depth == 0:
                body = source[start:pos + 1]
                if "BODY GOES HERE" in body:
                    raise PlaceholderBody("refusing product placeholder")
                return body
    raise RuntimeError("product buy_council_vote body is unterminated")


def compile_and_run(body: str) -> subprocess.CompletedProcess:
    compiler = shutil.which("clang++") or shutil.which("g++")
    if compiler is None:
        pytest.skip("host C++ compiler required")
    template = HARNESS.read_text()
    assert template.count(MARKER) == 1
    with tempfile.TemporaryDirectory(prefix="council-vote-purchase-") as td:
        source = Path(td) / "harness.cpp"
        executable = Path(td) / "harness"
        source.write_text(template.replace(MARKER, body))
        subprocess.run([compiler, "-std=c++17", "-Wall", "-Wextra", "-Werror",
                        "-fsanitize=address,undefined", "-fno-sanitize-recover=all",
                        str(source), "-o", str(executable)],
                       check=True, capture_output=True, text=True, timeout=30)
        return subprocess.run([str(executable)], capture_output=True, text=True, timeout=10,
                              env=dict(os.environ, ASAN_OPTIONS="detect_leaks=0"))


def actual_body() -> str:
    record, = [r for r in read_file(PRODUCT) if r.address == 0x53EB50]
    assert record.name == "?buy_council_vote@@YAXHHHH@Z"
    assert record.state.value == "implemented"
    try:
        return extract_body(PRODUCT.read_text())
    except PlaceholderBody as exc:
        pytest.fail(f"actual-source behavior test refused: {exc}")


def test_actual_product_body_behavior():
    result = compile_and_run(actual_body())
    assert result.returncode == 0, result.stdout + result.stderr


def corrupted_bodies(body: str):
    # Existing controls distinguish each native role, lifecycle, sequencing,
    # callback reload, and reciprocal-treaty index listed by the task.
    changes = {
        "role swap (FACTION instead of OTHER governor)": (
            "if (other == GovernorFaction)", "if (faction == GovernorFaction)"),
        "extra Popup close before modal return": (
            "if (menu_result <= 0) {\n                return;",
            "if (menu_result <= 0) {\n                popup.close();\n                return;"),
        "reverse vote-query call order": (
            "const int player_votes = council_votes(player);\n"
            "                    const int other_votes = council_votes(other);",
            "const int other_votes = council_votes(other);\n"
            "                    const int player_votes = council_votes(player);"),
        "stale technology turn scale": (
            "int value = tech_val(DiploScratchFA18, other, 1);\n"
            "                turn_rounded = diplo_add(TurnCurrentNum, 7);",
            "int value = tech_val(DiploScratchFA18, other, 1);\n"
            "                turn_rounded = diplo_add(8, 7);"),
        "wrong reciprocal treaty row": (
            "PlayersData[faction].diplo_treaties[other]",
            "PlayersData[other].diplo_treaties[faction]"),
    }
    for name, (before, after) in changes.items():
        if name == "reverse vote-query call order":
            damaged, substitutions = re.subn(
                r"(?m)^([ \t]*)const int player_votes = council_votes\(player\);\n"
                r"[ \t]*const int other_votes = council_votes\(other\);",
                r"\1const int other_votes = council_votes(other);\n"
                r"\1const int player_votes = council_votes(player);",
                body)
            assert substitutions > 0, f"damage-control anchor missing: {name}"
        else:
            assert before in body, f"damage-control anchor missing: {name}"
            damaged = body.replace(before, after)
        assert damaged != body
        yield name, damaged


@pytest.mark.parametrize("damage", range(5))
def test_damage_controls_fail_the_fixture(damage):
    name, mutant = list(corrupted_bodies(actual_body()))[damage]
    result = compile_and_run(mutant)
    assert result.returncode != 0, f"fixture failed to detect {name}"
