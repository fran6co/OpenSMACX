"""Strict host controls for the actual, imported faction.cpp definitions.

Run with ``uv run pytest recovery/tests/test_diplomacy_owner.py``.
An explicit OPENSMACX_ROOT selects a candidate checkout. There is intentionally
no seed/proposal fallback: missing real definitions fail closed.
"""
import os
from pathlib import Path
import re
import shutil
import subprocess

import pytest


SCRIPT = Path(__file__).resolve()
ROOT = Path(os.environ["OPENSMACX_ROOT"]).resolve() if os.environ.get("OPENSMACX_ROOT") else Path(__file__).resolve().parents[2]
SOURCE = ROOT / "src" / "faction.cpp"
HARNESS = Path(__file__).with_name("diplomacy-owner-harness.cpp").read_text()


def definition(source: str, name: str) -> str:
    match = re.search(r"\b(?:void|int)\s+__cdecl\s+" + re.escape(name) + r"\s*\([^;]*?\)\s*\{", source, re.S)
    assert match, "src/faction.cpp must contain the actual definition of " + name
    start = match.start()
    brace = source.find("{", match.start(), match.end())
    depth = 0
    for pos in range(brace, len(source)):
        if source[pos] == "{":
            depth += 1
        elif source[pos] == "}":
            depth -= 1
            if depth == 0:
                body = source[start : pos + 1]
                assert "PENDING_BODY" not in body and "BODY GOES HERE" not in body
                assert len(body) > 100, name + " is not a complete imported body"
                return body.replace("nullptr", "0")
    raise AssertionError("unterminated body for " + name)


@pytest.fixture(scope="session")
def actual_bodies():
    assert SOURCE.is_file(), "actual source file is missing: " + str(SOURCE)
    text = SOURCE.read_text()
    return {name: definition(text, name) for name in ("trade_maps", "do_withdrawal")}


def compilers():
    found = [shutil.which(name) for name in ("g++", "clang++")]
    return [compiler for compiler in found if compiler]


def executable(tmp_path: Path, bodies, compiler: str):
    source = HARNESS.replace("@@TRADE_MAPS@@", bodies["trade_maps"])
    source = source.replace("@@DO_WITHDRAWAL@@", bodies["do_withdrawal"])
    assert "@@TRADE_MAPS@@" not in source and "@@DO_WITHDRAWAL@@" not in source
    cpp = tmp_path / "diplomacy.cpp"
    binary = tmp_path / "diplomacy"
    cpp.write_text(source)
    command = [compiler, "-std=c++03", "-Wall", "-Wextra", "-Werror", "-pedantic",
               "-fsanitize=address,undefined", "-fno-omit-frame-pointer", str(cpp), "-o", str(binary)]
    result = subprocess.run(command, text=True, capture_output=True)
    assert result.returncode == 0, result.stdout + result.stderr
    return binary


@pytest.mark.parametrize("compiler", compilers())
def test_actual_owner_host_controls(tmp_path, actual_bodies, compiler):
    binary = executable(tmp_path, actual_bodies, compiler)
    for mode in ("trade", "withdraw"):
        run_env = dict(os.environ)
        run_env["ASAN_OPTIONS"] = "detect_leaks=0"
        result = subprocess.run([str(binary), mode], text=True, capture_output=True, env=run_env)
        assert result.returncode == 0, result.stdout + result.stderr


def reject_mutant(tmp_path, actual_bodies, compiler, mutate, required_pattern):
    bodies = dict(actual_bodies)
    body = bodies["trade_maps"] if mutate.startswith("slot") else bodies["do_withdrawal"]
    changed, count = re.subn(*required_pattern, body)
    assert count > 0, "actual body no longer has the mutation anchor: " + mutate
    bodies["trade_maps" if mutate.startswith("slot") else "do_withdrawal"] = changed
    binary = executable(tmp_path, bodies, compiler)
    mode = "trade" if mutate.startswith("slot") else "withdraw"
    run_env = dict(os.environ)
    run_env["ASAN_OPTIONS"] = "detect_leaks=0"
    result = subprocess.run([str(binary), mode], text=True, capture_output=True, env=run_env)
    assert result.returncode != 0, "damage control failed to reject " + mutate


@pytest.mark.parametrize("compiler", compilers())
def test_wrong_visibility_slot_is_rejected(tmp_path, actual_bodies, compiler):
    reject_mutant(tmp_path, actual_bodies, compiler, "slot-index",
                  (r"bit_visible\[(faction_id|other_faction_id) - 1\]",
                   r"bit_visible[\1]"))


@pytest.mark.parametrize("compiler", compilers())
def test_wrong_stack_faction_is_rejected(tmp_path, actual_bodies, compiler):
    reject_mutant(tmp_path, actual_bodies, compiler, "wrong-stack-faction",
                  (r"stack_check\(([^,]+),\s*8,\s*other_faction_id,",
                   r"stack_check(\1, 8, faction_id,"))


@pytest.mark.parametrize("compiler", compilers())
def test_cached_vehicle_count_is_rejected(tmp_path, actual_bodies, compiler):
    body = actual_bodies["do_withdrawal"]
    changed, count = re.subn(r"for\s*\(([^;]*);([^;]*?)VehCurrentCount([^;]*);", r"const int cached_vehicle_count = VehCurrentCount;\n    for (\1;\2cached_vehicle_count\3;", body, count=1)
    assert count == 1, "actual do_withdrawal no longer has a live VehCurrentCount loop"
    bodies = dict(actual_bodies)
    bodies["do_withdrawal"] = changed
    binary = executable(tmp_path, bodies, compiler)
    run_env = dict(os.environ)
    run_env["ASAN_OPTIONS"] = "detect_leaks=0"
    result = subprocess.run([str(binary), "withdraw"], text=True, capture_output=True, env=run_env)
    assert result.returncode != 0, "damage control failed to reject cached vehicle count"
