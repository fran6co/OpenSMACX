"""Strict host controls for actual extracted faction.cpp helper definitions."""
from __future__ import annotations

import shutil
import subprocess
import tempfile
import os
from pathlib import Path

import pytest


ROOT = Path(os.environ["OPENSMACX_ROOT"]).resolve() if os.environ.get("OPENSMACX_ROOT") else Path(__file__).resolve().parents[2]
SOURCE = ROOT / "src/faction.cpp"
HARNESS = Path(__file__).with_name("diplomacy-lifecycle-harness.cpp")


def _extract_definition(text: str, signature: str, marker: str) -> str:
    marker_at = text.find(marker)
    assert marker_at >= 0, "source identity marker missing: " + marker
    start = text.find(signature, marker_at)
    assert start >= 0, "definition missing after identity marker: " + signature
    brace = text.find("{", start)
    assert brace >= 0, "definition body missing: " + signature

    depth = 0
    i = brace
    state = "code"
    while i < len(text):
        ch = text[i]
        following = text[i + 1] if i + 1 < len(text) else ""
        if state == "code":
            if ch == "/" and following == "/":
                state = "line_comment"
                i += 2
                continue
            if ch == "/" and following == "*":
                state = "block_comment"
                i += 2
                continue
            if ch == '"':
                state = "string"
            elif ch == "'":
                state = "character"
            elif ch == "{":
                depth += 1
            elif ch == "}":
                depth -= 1
                if depth == 0:
                    definition = text[start : i + 1]
                    assert "BODY GOES HERE." not in definition, "unfinished body refused"
                    assert "(void)target_faction" not in definition, "placeholder body refused"
                    assert "(void)faction_id" not in definition, "placeholder body refused"
                    return definition
        elif state == "line_comment":
            if ch == "\n":
                state = "code"
        elif state == "block_comment":
            if ch == "*" and following == "/":
                state = "code"
                i += 2
                continue
        elif state == "string":
            if ch == "\\":
                i += 2
                continue
            if ch == '"':
                state = "code"
        elif state == "character":
            if ch == "\\":
                i += 2
                continue
            if ch == "'":
                state = "code"
        i += 1
    raise AssertionError("unbalanced definition: " + signature)


def _actual_helpers() -> str:
    text = SOURCE.read_text(encoding="utf-8")
    intervention = _extract_definition(
        text,
        "void __cdecl intervention(int target_faction, int actor_faction)",
        "// ORIGINAL: 0x0050BA50 ?intervention@@YAXHH@Z",
    )
    pact_withdraw = _extract_definition(
        text,
        "int __cdecl pact_withdraw(int faction_id, int faction_id_with)",
        "// ORIGINAL: 0x0053C370 ?pact_withdraw@@YAHHH@Z",
    )
    # Refuse the source until there is substantive behavior in both methods.
    assert "parse_says(6," in intervention
    assert "treaty_on(" in intervention
    assert "veh_lift(" in pact_withdraw
    assert "veh_drop(" in pact_withdraw
    return intervention + "\n\n" + pact_withdraw


@pytest.mark.parametrize("damage", [None, "wrong_air_member", "late_destination_read"])
def test_extracted_diplomacy_helpers_under_gcc_and_clang_sanitizers(damage) -> None:
    helpers = _actual_helpers()  # Fail closed if either source body is absent.
    if damage == "wrong_air_member":
        anchor = "vehicle.terraforming_turns = 0;"
        assert helpers.count(anchor) == 1
        helpers = helpers.replace(anchor, "vehicle.morale = 0;")
    elif damage == "late_destination_read":
        anchor = "veh_drop(lifted_vehicle, destination_x, destination_y);"
        assert helpers.count(anchor) == 1
        helpers = helpers.replace(anchor,
            "(void)destination_x; (void)destination_y; "
            "veh_drop(lifted_vehicle, Bases[best_base].x, Bases[best_base].y);")
    template = HARNESS.read_text(encoding="utf-8")
    assert template.count("/*EXTRACTED_HELPERS*/") == 1
    generated = template.replace("/*EXTRACTED_HELPERS*/", helpers)

    compilers = [shutil.which("g++"), shutil.which("clang++")]
    compilers = [compiler for compiler in compilers if compiler]
    assert compilers, "neither GCC nor Clang is available for required controls"

    for compiler in compilers:
        with tempfile.TemporaryDirectory(prefix="diplomacy-lifecycle-", dir="/tmp") as tmp:
            directory = Path(tmp)
            source = directory / "extracted.cpp"
            executable = directory / "extracted-controls"
            source.write_text(generated, encoding="utf-8")
            build = subprocess.run(
                [
                    compiler,
                    "-std=c++03",
                    "-O0",
                    "-g",
                    "-Wall",
                    "-Wextra",
                    "-Werror",
                    "-pedantic",
                    "-fsanitize=address,undefined",
                    "-fno-omit-frame-pointer",
                    str(source),
                    "-o",
                    str(executable),
                ],
                text=True,
                stdout=subprocess.PIPE,
                stderr=subprocess.STDOUT,
                timeout=60,
            )
            assert build.returncode == 0, compiler + " C++03 sanitizer build failed:\n" + build.stdout
            run = subprocess.run(
                [str(executable)],
                text=True,
                stdout=subprocess.PIPE,
                stderr=subprocess.STDOUT,
                env=dict(os.environ, ASAN_OPTIONS="detect_leaks=0"),
                timeout=30,
            )
            if damage is None:
                assert run.returncode == 0, compiler + " extracted helper controls failed:\n" + run.stdout
                assert "diplomacy lifecycle extracted-source controls passed" in run.stdout
            else:
                assert run.returncode != 0, compiler + " failed to reject " + damage
