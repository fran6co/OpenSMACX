"""Focused controls for the recovered base-build climate case.

The harness compiles the complete current case 0x1a and the real shared
label_4fef3f range assignment extracted from base.cpp. Its narrow stubs expose
ordering and widths; they are not evidence of the native ABI or full function.
"""

from __future__ import annotations

import os
import re
import resource
import shlex
import subprocess
from pathlib import Path

import pytest


ROOT = Path(__file__).resolve().parents[2]
SOURCE = ROOT / "src" / "base.cpp"


def _matching_brace(text: str, opening: int) -> int:
    depth = 0
    state = "code"
    i = opening
    while i < len(text):
        char = text[i]
        nxt = text[i + 1] if i + 1 < len(text) else ""
        if state == "code":
            if char == "/" and nxt == "/":
                state = "line_comment"
                i += 2
                continue
            if char == "/" and nxt == "*":
                state = "block_comment"
                i += 2
                continue
            if char == '"':
                state = "string"
            elif char == "'":
                state = "char"
            elif char == "{":
                depth += 1
            elif char == "}":
                depth -= 1
                if depth == 0:
                    return i
        elif state == "line_comment":
            if char == "\n":
                state = "code"
        elif state == "block_comment":
            if char == "*" and nxt == "/":
                state = "code"
                i += 2
                continue
        elif state in ("string", "char"):
            if char == "\\":
                i += 2
                continue
            if (state == "string" and char == '"') or (
                state == "char" and char == "'"
            ):
                state = "code"
        i += 1
    raise AssertionError("unclosed function or switch body")


def _extract_base_build(text: str) -> str:
    signatures = list(re.finditer(r"\bint\s+__cdecl\s+base_build\s*\(", text))
    assert len(signatures) == 1, f"expected one base_build definition, got {len(signatures)}"
    opening = text.find("{", signatures[0].end())
    assert opening >= 0
    return text[signatures[0].start() : _matching_brace(text, opening) + 1]


def _extract_case_and_score(text: str) -> tuple[str, str]:
    body = _extract_base_build(text)
    case_matches = list(re.finditer(r"\bcase\s+0x1a\s*:", body))
    assert len(case_matches) == 1, f"expected one climate case, got {len(case_matches)}"
    start = case_matches[0].start()
    next_case = re.search(r"\bcase\s+0x1b\s*:", body[case_matches[0].end() :])
    assert next_case, "case 0x1a must end before case 0x1b"
    end = case_matches[0].end() + next_case.start()
    climate_case = body[start:end]
    assert climate_case.rstrip().endswith("break;")
    assert "goto label_4fef3f;" in climate_case

    label = re.search(r"\blabel_4fef3f\s*:\s*", body)
    assert label, "shared scoring label not found in base_build"
    assignment = re.match(r"v8\s*=\s*range\(v75\s*,\s*v8\s*,\s*v16\s*\)\s*;", body[label.end() :])
    assert assignment, "shared label no longer begins with the expected range assignment"
    return climate_case, assignment.group(0)


HARNESS_PREFIX = r'''#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define __cdecl

struct Base {
    int16_t x;
    int16_t y;
    int8_t population_size;
};
static Base bases[4];
static Base *BaseCurrent;
static int BaseIDCurrentSelected;
struct PlayerData { uint32_t flags; };
static PlayerData PlayersData[8];
struct LockMasks { uint32_t enable_mask_; };
struct GameFields { int32_t climate_sea_level_delta_; LockMasks lock_masks_; };
struct GameState { GameFields fields_; };
static GameState GameDataStateStorage;
struct Worldbuilder { uint32_t sea_level_rises; };
static Worldbuilder WorldBuilder;
static uint32_t AltNatural[11];
static int32_t MapSeaLevel;

static int alt_class;
static uint8_t detail_values[4];
static int detail_count;
static int detail_x[4], detail_y[4];
static int alt_count, alt_x, alt_y;
static int project_values[64];
static int project_count;
static int mutate_base_after_project;

static int alt_at(int x, int y) {
    ++alt_count; alt_x = x; alt_y = y; return alt_class;
}
static int alt_detail_at(int x, int y) {
    if (detail_count >= 4) return 0;
    detail_x[detail_count] = x; detail_y[detail_count] = y;
    return detail_values[detail_count++];
}
int base_project(int id) {
    ++project_count;
    /* A test-only instrumented project read probes the source's live reload. */
    if (mutate_base_after_project && project_count == 1) BaseCurrent = &bases[1];
    return project_values[id];
}
static int range(int input, int minimum, int maximum) {
    if (input < minimum || maximum < minimum) return minimum;
    if (input > maximum) return maximum;
    return input;
}

static int run_climate_case(int faction_id, uint32_t v165, int initial_v8) {
    int16_t v5 = 0, v6 = 0;
    int v8 = initial_v8, v75 = 0, v16 = 0;
    switch (0x1a) {
'''

HARNESS_SUFFIX = r'''
    }
    goto finished;
label_4fef3f:
    SCORE_ASSIGNMENT
finished:
    return v8;
}

static void reset_case(void) {
    memset(bases, 0, sizeof(bases));
    memset(PlayersData, 0, sizeof(PlayersData));
    for (int i = 0; i < 64; ++i) project_values[i] = -1;
    memset(AltNatural, 0, sizeof(AltNatural));
    BaseCurrent = &bases[0]; BaseIDCurrentSelected = 3;
    MapSeaLevel = 0;
    GameDataStateStorage.fields_.climate_sea_level_delta_ = 0;
    GameDataStateStorage.fields_.lock_masks_.enable_mask_ = 0;
    WorldBuilder.sea_level_rises = 0;
    alt_class = 3;
    detail_count = alt_count = project_count = 0;
    mutate_base_after_project = 0;
    memset(detail_values, 0, sizeof(detail_values));
}

int fail(const char *name, int actual, int expected) {
    fprintf(stderr, "%s: actual=%d expected=%d\n", name, actual, expected);
    return 1;
}
'''

POSITIVE_MAIN = r'''
int main(void) {
    reset_case();
    bases[0].x = 12; bases[0].y = -7;
    detail_values[0] = 20;
    MapSeaLevel = 21;
    /* Below sea: signed wrapped contour comparison selects -16. */
    int result = run_climate_case(0, 0, 5);
    if (result != -16) return fail("ordinary below-sea", result, -16);
    if (alt_count != 1 || alt_x != 12 || alt_y != -7 || detail_count != 1) return fail("initial reads", detail_count, 1);

    /* A non-3 initial altitude bypasses both contour tests and projects. */
    reset_case(); alt_class = 2;
    if (run_climate_case(0, 0, 9) != 9 || detail_count || project_count) return fail("alt_at gate", detail_count, 0);

    /* Equality and the adjacent positive side retain the native <= gate. */
    reset_case(); MapSeaLevel = 20; detail_values[0] = 20;
    if (run_climate_case(0, 0, 5) != 5 || project_count != 64) return fail("first-gate equality", project_count, 64);
    reset_case(); MapSeaLevel = 19; detail_values[0] = 20;
    if (run_climate_case(0, 0, 5) != 5 || project_count != 64) return fail("first-gate positive", project_count, 64);

    /* Flag bypass: no project scan or second detail access. */
    reset_case(); MapSeaLevel = 20; detail_values[0] = 20;
    PlayersData[2].flags = 0x80000;
    if (run_climate_case(2, 0, 7) != 0 || project_count || detail_count != 1) return fail("flag bypass", project_count, 0);

    /* First matching project, sea-rise branch, lock-byte score and clamp. */
    reset_case();
    bases[0].x = 4; bases[0].y = 8; bases[0].population_size = 20;
    bases[1].x = -9; bases[1].y = 31; bases[1].population_size = 120;
    AltNatural[3] = 10; MapSeaLevel = 100;
    GameDataStateStorage.fields_.climate_sea_level_delta_ = 5;
    WorldBuilder.sea_level_rises = 7;
    detail_values[0] = 200; detail_values[1] = 200;
    project_values[0] = BaseIDCurrentSelected;
    GameDataStateStorage.fields_.lock_masks_.enable_mask_ = 0x04;
    mutate_base_after_project = 1;
    result = run_climate_case(1, 0x04, 99);
    if (result != 6) return fail("matched high score lower clamp", result, 6);
    if (project_count != 1 || detail_count != 2 || detail_x[1] != -9 || detail_y[1] != 31)
        return fail("live BaseCurrent second read", detail_count, 2);
    if (detail_x[0] != 4 || detail_y[0] != 8) return fail("initial detail coords", detail_x[0], 4);

    /* Same clamp with the human lock byte clear selects its other lower bound. */
    reset_case(); bases[0].population_size = 120;
    detail_values[0] = 20; detail_values[1] = 20;
    project_values[0] = BaseIDCurrentSelected;
    if (run_climate_case(0, 0x04, 99) != 4) return fail("unlocked lower clamp", run_climate_case(0, 0x04, 99), 4);

    /* Last project slot and no-project cases preserve loop bound and output. */
    reset_case(); MapSeaLevel = 20; detail_values[0] = 20;
    for (int i = 0; i < 63; ++i) project_values[i] = -1;
    project_values[63] = BaseIDCurrentSelected;
    detail_values[1] = 20;
    if (run_climate_case(0, 0, 6) != 12 || project_count != 64 || detail_count != 2)
        return fail("last project", project_count, 64);
    reset_case(); MapSeaLevel = 20; detail_values[0] = 20;
    if (run_climate_case(0, 0, 6) != 6 || project_count != 64 || detail_count != 1)
        return fail("no project", project_count, 64);

    /* High-bit operands exercise independent uint32 wrapping/signed ordering. */
    reset_case(); AltNatural[3] = 0x80000001u; MapSeaLevel = INT32_MIN;
    GameDataStateStorage.fields_.climate_sea_level_delta_ = -1;
    detail_values[0] = 0; detail_values[1] = 255;
    project_values[0] = BaseIDCurrentSelected;
    WorldBuilder.sea_level_rises = 0xffffffffu;
    result = run_climate_case(0, 0, 5);
    if (result != 8) return fail("high-bit arithmetic", result, 8);
    return 0;
}
'''

def _mutant_main(mutant: str) -> str:
    if mutant == "unsigned":
        return r'''
int main(void) {
    reset_case();
    bases[0].x = 12; bases[0].y = -7;
    detail_values[0] = 20; MapSeaLevel = 21;
    int result = run_climate_case(0, 0, 5);
    return result == -16 ? 0 : 1;
}
'''
    if mutant == "direction":
        return r'''
int main(void) {
    reset_case(); detail_values[0] = 20; MapSeaLevel = 20;
    int result = run_climate_case(0, 0, 5);
    return result == 5 && project_count == 64 ? 0 : 1;
}
'''
    if mutant == "skip_project":
        return r'''
int main(void) {
    reset_case(); detail_values[0] = 20; detail_values[1] = 20;
    project_values[0] = BaseIDCurrentSelected;
    int result = run_climate_case(0, 0, 5);
    return result == 12 && project_count == 1 && detail_count == 2 ? 0 : 1;
}
'''
    return r'''
int main(void) {
    reset_case(); AltNatural[3] = 10; detail_values[0] = 20; detail_values[1] = 20;
    project_values[0] = BaseIDCurrentSelected;
    int result = run_climate_case(0, 0, 5);
    return result == 12 && detail_count == 2 ? 0 : 1;
}
'''


def _source_program(case: str, score: str, main: str) -> str:
    return (
        HARNESS_PREFIX
        + case
        + HARNESS_SUFFIX.replace("SCORE_ASSIGNMENT", score)
        + main
    )


def _compiler(name: str) -> list[str]:
    value = os.environ.get(name)
    if value:
        return shlex.split(value)
    path = {"g++": "g++", "clang++": "clang++"}[name]
    return [path]


def _disable_core() -> None:
    resource.setrlimit(resource.RLIMIT_CORE, (0, 0))


def _compile_and_run(tmp_path: Path, compiler: str, text: str, *, should_pass: bool) -> None:
    source = tmp_path / "actual.cpp"
    binary = tmp_path / "actual"
    source.write_text(text)
    command = _compiler(compiler) + [
        "-std=c++98", "-O2", "-Wall", "-Wextra", "-Werror", "-Wno-sign-compare", "-pedantic",
        "-fsanitize=undefined", "-fno-sanitize-recover=all", str(source), "-o", str(binary),
    ]
    build = subprocess.run(command, capture_output=True, text=True, timeout=30,
                           cwd=tmp_path, env=os.environ.copy(), preexec_fn=_disable_core)
    assert build.returncode == 0, f"{compiler} compile failed:\n{build.stdout}\n{build.stderr}"
    run = subprocess.run([str(binary)], capture_output=True, text=True, timeout=10,
                         cwd=tmp_path, env=os.environ.copy(), preexec_fn=_disable_core)
    if should_pass:
        assert run.returncode == 0, f"{compiler} harness failed:\n{run.stdout}\n{run.stderr}"
    else:
        assert run.returncode != 0, f"{compiler} mutation unexpectedly passed"


@pytest.fixture(scope="module")
def source_parts() -> tuple[str, str, str]:
    text = SOURCE.read_text()
    climate_case, score = _extract_case_and_score(text)
    assert climate_case.count("alt_detail_at(") == 2
    assert "PlayersData[faction_id].flags & 0x80000" in climate_case
    return text, climate_case, score


@pytest.mark.parametrize("compiler", ["g++", "clang++"])
def test_actual_case_against_signed32_oracle(tmp_path: Path, compiler: str, source_parts) -> None:
    _text, climate_case, score = source_parts
    assert _signed32_le(0, 20 - 21) is False  # ordinary below sea
    assert _signed32_le(0, 20 - 20) is True   # exact contour equality
    assert _signed32_le(0, 20 - 19) is True   # ordinary positive contour
    assert oracle_high_bit_case() == 8
    program = _source_program(climate_case, score, POSITIVE_MAIN)
    _compile_and_run(tmp_path, compiler, program, should_pass=True)


@pytest.mark.parametrize("compiler", ["g++", "clang++"])
@pytest.mark.parametrize("mutant", ["unsigned", "direction", "skip_project", "drop_second_read"])
def test_compile_success_mutants_fail(
    tmp_path: Path, compiler: str, mutant: str, source_parts
) -> None:
    _text, climate_case, score = source_parts
    baseline_dir = tmp_path / "unmodified"
    baseline_dir.mkdir()
    witness = _mutant_main(mutant)
    _compile_and_run(
        baseline_dir,
        compiler,
        _source_program(climate_case, score, witness),
        should_pass=True,
    )
    changed = climate_case
    if mutant == "unsigned":
        old_cond = (
            "GameDataStateStorage.fields_.climate_sea_level_delta_ + AltNatural[3] <= "
            "alt_detail_at((int)v6,(int)v5) - MapSeaLevel"
        )
        signed_cond = (
            "static_cast<int32_t>(static_cast<uint32_t>(GameDataStateStorage.fields_.climate_sea_level_delta_) + AltNatural[3])\n"
            "              <= static_cast<int32_t>(static_cast<uint32_t>(alt_detail_at((int)v6,(int)v5)) - static_cast<uint32_t>(MapSeaLevel))"
        )
        unsigned_cond = (
            "static_cast<uint32_t>(static_cast<uint32_t>(GameDataStateStorage.fields_.climate_sea_level_delta_) + AltNatural[3])\n"
            "              <= static_cast<uint32_t>(static_cast<uint32_t>(alt_detail_at((int)v6,(int)v5)) - static_cast<uint32_t>(MapSeaLevel))"
        )
        if old_cond in changed:
            changed = changed.replace(old_cond, "static_cast<uint32_t>(GameDataStateStorage.fields_.climate_sea_level_delta_ + AltNatural[3]) <= static_cast<uint32_t>(alt_detail_at((int)v6,(int)v5) - MapSeaLevel)", 1)
        else:
            assert signed_cond in changed, "candidate first signed gate anchor changed"
            changed = changed.replace(signed_cond, unsigned_cond, 1)
    elif mutant == "direction":
        changed = changed.replace(
            "GameDataStateStorage.fields_.climate_sea_level_delta_ + AltNatural[3] <= alt_detail_at((int)v6,(int)v5) - MapSeaLevel",
            "GameDataStateStorage.fields_.climate_sea_level_delta_ + AltNatural[3] < alt_detail_at((int)v6,(int)v5) - MapSeaLevel",
            1,
        )
        changed = changed.replace(
            "              <= static_cast<int32_t>(static_cast<uint32_t>(alt_detail_at((int)v6,(int)v5)) - static_cast<uint32_t>(MapSeaLevel))",
            "              < static_cast<int32_t>(static_cast<uint32_t>(alt_detail_at((int)v6,(int)v5)) - static_cast<uint32_t>(MapSeaLevel))",
            1,
        )
    elif mutant == "skip_project":
        changed = changed.replace("if (base_project(v75) == BaseIDCurrentSelected)", "if (0)")
    else:
        changed = changed.replace("v8 = alt_detail_at((int)BaseCurrent->x,(int)BaseCurrent->y);", "v8 = 0;", 1)
    assert changed != climate_case, f"mutation {mutant} failed to alter target case"
    mutant_dir = tmp_path / "mutant"
    mutant_dir.mkdir()
    program = _source_program(changed, score, witness)
    _compile_and_run(mutant_dir, compiler, program, should_pass=False)


def _signed32(value: int) -> int:
    """Interpret a wide Python integer's low dword using signed ordering."""
    bits = value & 0xFFFFFFFF
    return bits - 0x100000000 if bits & 0x80000000 else bits


def _signed32_le(left: int, right: int) -> bool:
    return _signed32(left) <= _signed32(right)


def oracle_high_bit_case() -> int:
    """Independent wide-signed oracle for the selected high-bit fixture."""

    threshold = 0x80000001
    sea = -(1 << 31)
    detail_first, detail_second = 0, 255
    delta, rises = -1, 0xFFFFFFFF
    first_left = _signed32(delta + threshold)
    first_contour = _signed32(detail_first - sea)
    if first_left > first_contour:
        return -16
    second_left = _signed32(rises + delta + threshold)
    second_contour = _signed32(detail_second - sea)
    if second_left <= second_contour:
        return 12  # high-branch base score clamps to its maximum
    return 8
