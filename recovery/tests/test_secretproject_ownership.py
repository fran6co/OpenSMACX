"""Strict actual-source controls for the shared SecretProject owner."""
import os
from pathlib import Path
import re
import shutil
import subprocess
import tempfile

import pytest


_root_override = os.environ.get("OPENSMACX_ROOT")
ROOT = (Path(_root_override) if _root_override else Path(__file__).resolve().parents[2]).resolve()
SOCIAL = ROOT / "src/socialwin.cpp"
BASE = ROOT / "src/base.cpp"
GAME = ROOT / "src/game.cpp"
HARNESS = Path(__file__).with_name("secretproject-ownership-harness.cpp")


def _function(source: str, signature: str) -> str:
    if source.count(signature) != 1:
        raise AssertionError(f"expected exactly one actual source definition: {signature}")
    start = source.index(signature)
    brace = source.index("{", start)
    depth = 0
    state = "code"
    escaped = False
    i = brace
    while i < len(source):
        c = source[i]
        n = source[i + 1] if i + 1 < len(source) else ""
        if state == "line":
            if c == "\n":
                state = "code"
        elif state == "block":
            if c == "*" and n == "/":
                state = "code"
                i += 1
        elif state in ("string", "char"):
            if escaped:
                escaped = False
            elif c == "\\":
                escaped = True
            elif (state == "string" and c == '"') or (state == "char" and c == "'"):
                state = "code"
        elif c == "/" and n == "/":
            state = "line"
            i += 1
        elif c == "/" and n == "*":
            state = "block"
            i += 1
        elif c == '"':
            state = "string"
        elif c == "'":
            state = "char"
        elif c == "{":
            depth += 1
        elif c == "}":
            depth -= 1
            if depth == 0:
                body = source[start:i + 1]
                if "BODY GOES HERE." in body or "PLACEHOLDER" in body:
                    raise AssertionError(f"{signature} is still a placeholder")
                return body
        i += 1
    raise AssertionError(f"unterminated definition: {signature}")


def _actual_sources():
    header = ROOT / "src/base.h"
    for path in (SOCIAL, BASE, GAME, header, HARNESS):
        if not path.is_file():
            raise AssertionError(f"required source/fixture missing: {path}")
    social = SOCIAL.read_text()
    base = BASE.read_text()
    game = GAME.read_text()
    header_source = header.read_text()
    immune = _function(social, "int SocialWin::immune(int soc_category, int soc_model, int effect_id)")
    energy = _function(social, "void __cdecl energy_compute(int faction_id, int recompute_bases)")
    remap = _function(base, "static void base_remap_projects(int removed)")
    setup = _function(game, "void __cdecl setup_game(int reload)")
    project_type = _function(header_source, "struct BaseSecretProject")
    reset_match = re.findall(r"memset\(&SecretProject,\s*0xFF,\s*sizeof\s+SecretProject\s*\);", setup)
    if len(reset_match) != 1:
        raise AssertionError("setup_game must reset the actual complete SecretProject object to 0xFF")
    if re.search(r"\bint\s+(?:SpyingBaseIndex|EnergyDiscountBaseIndex)\s*(?:=|;)", social):
        raise AssertionError("social owner still declares independent selector storage")
    return immune, energy, remap, reset_match[0], project_type


def _compile_run(mode: str, bodies, expect_pass: bool = True) -> subprocess.CompletedProcess[str]:
    compiler = shutil.which(os.environ.get("CXX", "g++"))
    if not compiler:
        pytest.skip("host C++ compiler unavailable")
    immune, energy, remap, reset_statement, project_type = bodies
    harness = HARNESS.read_text()
    if (harness.count("/*ACTUAL_METHODS*/") != 1 or harness.count("/*ACTUAL_RESET*/") != 1
            or harness.count("/*ACTUAL_PROJECT_OWNER*/") != 1):
        raise AssertionError("harness insertion anchors must be unique")
    harness = harness.replace("/*ACTUAL_PROJECT_OWNER*/", project_type + ";")
    harness = harness.replace("/*ACTUAL_METHODS*/", immune + "\n\n" + energy + "\n\n" + remap)
    harness = harness.replace("/*ACTUAL_RESET*/", reset_statement)
    with tempfile.TemporaryDirectory(prefix="secretproject-owner-host-") as temp:
        tmp = Path(temp)
        cpp, exe = tmp / "control.cpp", tmp / "control"
        cpp.write_text(harness)
        build = subprocess.run(
            [compiler, "-std=c++03", "-Wall", "-Wextra", "-Werror", "-pedantic",
             "-fsanitize=address,undefined", "-fno-omit-frame-pointer", "-g",
             str(cpp), "-o", str(exe)], text=True, capture_output=True, timeout=60)
        assert build.returncode == 0, f"{build.stdout}\n{build.stderr}"
        env = dict(os.environ)
        env.setdefault("ASAN_OPTIONS", "detect_leaks=0:halt_on_error=1")
        env.setdefault("UBSAN_OPTIONS", "halt_on_error=1:print_stacktrace=1")
        run = subprocess.run([str(exe)], text=True, capture_output=True,
                             timeout=20, env=env)
        if expect_pass:
            assert run.returncode == 0, f"{mode}: {run.stdout}\n{run.stderr}"
        else:
            assert run.returncode != 0, f"mutation escaped ({mode})"
        return run


def test_extractor_requires_product_owners_and_full_reset() -> None:
    immune, energy, remap, reset, project_type = _actual_sources()
    assert "SecretProject.cloning_vats()" in immune
    assert "SecretProject.network_backbone()" in immune
    assert "Bases[base_id].faction_id_current" in immune
    assert "SecretProject.self_aware_colony()" in energy
    assert "BaseCurrentCount" in energy
    assert "&SecretProject.network_backbone()" in remap
    assert "&SecretProject.cloning_vats()" in remap
    assert "&SecretProject.self_aware_colony()" in remap
    assert "&SecretProject.empath_guild()" in remap
    assert reset == "memset(&SecretProject, 0xFF, sizeof SecretProject);"
    assert "RECOVERY_MEMBER: v1 BaseSecretProject empath_guild 0x10" in project_type
    assert "RECOVERY_MEMBER: v1 BaseSecretProject network_backbone 0x54" in project_type
    assert "RECOVERY_MEMBER: v1 BaseSecretProject cloning_vats 0x60" in project_type
    assert "RECOVERY_MEMBER: v1 BaseSecretProject self_aware_colony 0x64" in project_type


def test_actual_immune_energy_reset_and_remap_controls() -> None:
    _compile_run("actual-controls", _actual_sources())


def test_energy_selector_is_reloaded_after_base_callback() -> None:
    bodies = list(_actual_sources())
    energy = bodies[1]
    anchor = "int discount_carry = 0;"
    assert energy.count(anchor) == 1
    energy = energy.replace(
        anchor, anchor + "\n    const int cached_selector = SecretProject.self_aware_colony();", 1)
    live_guard = ("if (SecretProject.self_aware_colony() >= 0 &&\n"
                  "            SecretProject.self_aware_colony() < BaseCurrentCount &&\n"
                  "            Bases[SecretProject.self_aware_colony()].faction_id_current == faction_id)")
    assert live_guard in energy
    energy = energy.replace(live_guard,
                            live_guard.replace("SecretProject.self_aware_colony()", "cached_selector"), 1)
    bodies[1] = energy
    _compile_run("cached-selector-timing-mutant", tuple(bodies), expect_pass=False)


@pytest.mark.parametrize(
    "body_index,needle,replacement,mode",
    [
        (0, "SecretProject.cloning_vats()", "SecretProject.network_backbone()", "immune"),
        (0, "SecretProject.network_backbone()", "SecretProject.cloning_vats()", "immune"),
        (0, "== factionID_", "== factionID_ + 1", "immune"),
        (1, "SecretProject.self_aware_colony()", "SecretProject.network_backbone()", "energy"),
        (1, "discount_carry = maintenance & 1;", "discount_carry = 0;", "energy"),
        (2, "&SecretProject.cloning_vats()", "&SecretProject.network_backbone()", "remap"),
        (2, "&SecretProject.empath_guild()", "&SecretProject.network_backbone()", "remap"),
    ],
)
def test_actual_source_mutants_are_detected(body_index, needle, replacement, mode) -> None:
    bodies = list(_actual_sources())
    assert needle in bodies[body_index], f"mutation anchor missing: {mode}: {needle}"
    bodies[body_index] = bodies[body_index].replace(needle, replacement, 1)
    _compile_run(mode, tuple(bodies), expect_pass=False)
