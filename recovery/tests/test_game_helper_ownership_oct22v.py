"""Host controls for the out-of-line game-year and energy-limit helpers."""

from pathlib import Path
import re
import shutil
import subprocess

import pytest


ROOT = Path(__file__).resolve().parents[2]
COMPILERS = [compiler for compiler in ("g++", "clang++") if shutil.which(compiler)]


def extract_definition(source: str, signature: str) -> str:
    """Return the actual source definition, including its signature and body."""
    assert source.count(signature) == 1, f"expected one definition anchor: {signature}"
    start = source.index(signature)
    opening = source.index("{", start)
    depth = 0
    for pos in range(opening, len(source)):
        if source[pos] == "{":
            depth += 1
        elif source[pos] == "}":
            depth -= 1
            if depth == 0:
                return source[start : pos + 1]
    raise AssertionError(f"unterminated definition: {signature}")


def without_comments(source: str) -> str:
    # These declarations contain no string literals; stripping comments keeps
    # historical BODY IN comments from satisfying the live ownership checks.
    source = re.sub(r"/\*.*?\*/", "", source, flags=re.S)
    return re.sub(r"//[^\n]*", "", source)


def actual_bodies():
    cpp = (ROOT / "src/game.cpp").read_text()
    calendar = (ROOT / "src/game_calendar.cpp").read_text()
    return (
        extract_definition(calendar, "MEASURED uint32_t __cdecl game_year(int turn)"),
        extract_definition(cpp, "MEASURED int __cdecl energy_limit(int)"),
    )


def test_helpers_have_one_out_of_line_definition_and_header_declarations():
    header = without_comments((ROOT / "src/game.h").read_text())
    cpp = (ROOT / "src/game.cpp").read_text()
    assert re.search(
        r"\bMEASURED\s+uint32_t\s+__cdecl\s+game_year\s*\(\s*int\s+turn\s*\)\s*;",
        header,
    )
    assert re.search(
        r"\bMEASURED\s+int\s+__cdecl\s+energy_limit\s*\(\s*int\s*\)\s*;",
        header,
    )
    assert not re.search(r"\binline\b[^;{}]*\bgame_year\s*\(", header)
    assert not re.search(r"\binline\b[^;{}]*\benergy_limit\s*\(", header)
    assert not re.search(
        r"\bMEASURED\s+uint32_t\s+__cdecl\s+game_year\s*\([^;{}]*\)\s*\{",
        header,
    )
    assert not re.search(
        r"\bMEASURED\s+int\s+__cdecl\s+energy_limit\s*\([^;{}]*\)\s*\{",
        header,
    )
    calendar = (ROOT / "src/game_calendar.cpp").read_text()
    assert cpp.count("MEASURED uint32_t __cdecl game_year(int turn)") == 0
    assert calendar.count("MEASURED uint32_t __cdecl game_year(int turn)") == 1
    assert cpp.count("MEASURED int __cdecl energy_limit(int)") == 1
    assert "return ScenarioSave.starting_mission_year + turn;" in actual_bodies()[0]
    assert "return 10;" in actual_bodies()[1]


HARNESS = r"""
#include <cstdio>
#include <cstdlib>
#include <stdint.h>
#include <climits>
#define assert(condition) do { if (!(condition)) { \
  std::fprintf(stderr, "source oracle rejected: %s\n", #condition); \
  std::exit(42); } } while (0)
#define __cdecl
#define MEASURED
struct ScenarioSaveState { uint32_t starting_mission_year; };
ScenarioSaveState ScenarioSave;
__GAME_YEAR__
__ENERGY_LIMIT__
int main() {
  ScenarioSave.starting_mission_year = 2150u;
  assert(game_year(0) == 2150u);
  assert(game_year(32) == 2182u);
  assert(game_year(-1) == 2149u);
  ScenarioSave.starting_mission_year = static_cast<uint32_t>(0xffffffffu);
  assert(game_year(1) == 0u);
  assert(game_year(2) == 1u);
  assert(energy_limit(0) == 10);
  assert(energy_limit(7) == 10);
  assert(energy_limit(-1) == 10);
  assert(energy_limit(INT_MIN) == 10);
}
"""


def compile_and_run(tmp_path: Path, compiler: str, year: str, energy: str):
    source = tmp_path / "game_helpers.cpp"
    executable = tmp_path / "game_helpers"
    source.write_text(
        HARNESS.replace("__GAME_YEAR__", year).replace("__ENERGY_LIMIT__", energy)
    )
    compiled = subprocess.run(
        [compiler, "-std=c++98", "-Wall", "-Wextra", str(source), "-o", str(executable)],
        capture_output=True,
        text=True,
    )
    return compiled, executable


@pytest.mark.parametrize("compiler", COMPILERS)
def test_actual_helpers_preserve_unsigned_year_and_ignored_energy_argument(tmp_path, compiler):
    year, energy = actual_bodies()
    compiled, executable = compile_and_run(tmp_path, compiler, year, energy)
    assert compiled.returncode == 0, compiled.stderr
    ran = subprocess.run([str(executable)], capture_output=True, text=True)
    assert ran.returncode == 0, ran.stderr


@pytest.mark.parametrize(
    ("mutant", "expected", "replacement"),
    [
        (
            "wrong-year-plus-one",
            "return ScenarioSave.starting_mission_year + turn;",
            "return ScenarioSave.starting_mission_year + turn + 1;",
        ),
        ("wrong-energy-limit", "return 10;", "return 11;"),
    ],
)
@pytest.mark.parametrize("compiler", COMPILERS)
def test_compile_success_mutants_are_rejected_by_actual_source_oracles(
    tmp_path, compiler, mutant, expected, replacement
):
    year, energy = actual_bodies()
    if mutant == "wrong-year-plus-one":
        assert year.count(expected) == 1
        year = year.replace(expected, replacement)
    else:
        assert energy.count(expected) == 1
        energy = energy.replace(expected, replacement)
    compiled, executable = compile_and_run(tmp_path, compiler, year, energy)
    assert compiled.returncode == 0, compiled.stderr
    ran = subprocess.run([str(executable)], capture_output=True, text=True)
    assert ran.returncode == 42, f"{mutant}: {ran.returncode}: {ran.stderr}"
    assert "source oracle rejected:" in ran.stderr
