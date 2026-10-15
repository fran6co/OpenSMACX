"""Bounded controls for pinned base_terraform mask/read stages.

These execute the exact extracted source expressions, not the whole game
runtime or a reconstruction of its provider state.
"""
from pathlib import Path
import re
import shutil
import subprocess

import pytest

ROOT = Path(__file__).resolve().parents[2]


def balanced(source: str, start: int) -> str:
    assert source[start] == "("
    depth = 0
    for end in range(start, len(source)):
        if source[end] == "(":
            depth += 1
        elif source[end] == ")":
            depth -= 1
            if depth == 0:
                return source[start:end + 1]
    raise AssertionError("unterminated source expression")


def stage_expressions():
    source = (ROOT / "src/base.cpp").read_text(encoding="utf-8")
    start = source.index("void __cdecl base_terraform(int force_improve) {")
    end = source.index("\n}\n", start) + 2
    body = source[start:end]

    population_comment = body.index("// At 4ED522-4ED53A")
    population_if = body.index("if (", population_comment)
    population_condition = balanced(body, population_if + len("if "))

    landing_assignment = body.index("const bool landing_site_rejected")
    landing_start = body.index(
        "((GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_ &", landing_assignment
    )
    landing_condition = balanced(body, landing_start)
    assert "BaseCurrent->population_size > 3" in population_condition
    assert "faction != ConsoleGlobal.faction_id_" in landing_condition
    assert "game_preferences_ & 0x00020000u" in landing_condition
    return population_condition, landing_condition


def mask_declarations():
    header = (ROOT / "src/game_data_state_window.h").read_text(encoding="utf-8")
    match = re.search(
        r"struct GameDataLockMaskBytes\s*\{.*?\};\s*union GameDataLockMaskView\s*\{.*?\};",
        header, re.DOTALL,
    )
    assert match
    return match.group(0)


def harness(population_condition: str, landing_condition: str) -> str:
    return r"""
#include <cassert>
#include <cstdint>
MASK_DECLARATIONS
struct HostFields { GameDataLockMaskView lock_masks_; uint32_t game_preferences_, game_more_preferences_; };
struct HostStorage { HostFields fields_; };
HostStorage GameDataStateStorage{};
struct Base { int8_t population_size; };
Base *BaseCurrent;
struct Console { int faction_id_; } ConsoleGlobal;
int population_stage(int faction_bit, Base &base) {
  (void)base;
  int nearest_open_square_count = 5;
  if (POPULATION_CONDITION) --nearest_open_square_count;
  return nearest_open_square_count;
}
bool landing_stage(int faction, int faction_bit) {
  return LANDING_CONDITION;
}
int main() {
  Base first{3}, live{4};
  BaseCurrent = &first;
  GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_ = 1;
  assert(population_stage(1, first) == 5); // human skips decrement
  GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_ = 0;
  assert(population_stage(1, first) == 5); // population below four
  first.population_size = 4;
  assert(population_stage(1, first) == 4);
  first.population_size = -128;
  assert(population_stage(1, first) == 5);
  first.population_size = -1;
  assert(population_stage(1, first) == 5);
  first.population_size = 127;
  assert(population_stage(1, first) == 4);
  first.population_size = 3;
  BaseCurrent = &live;
  assert(population_stage(1, first) == 4); // live selected-base reread

  ConsoleGlobal.faction_id_ = 2;
  GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_ = 1;
  GameDataStateStorage.fields_.game_preferences_ = 0x20000u;
  GameDataStateStorage.fields_.game_more_preferences_ = 0;
  assert(!landing_stage(2, 1)); // local human with preference is admitted
  assert(landing_stage(3, 1));  // remote human is rejected even with preference
  GameDataStateStorage.fields_.game_preferences_ = 0;
  GameDataStateStorage.fields_.game_more_preferences_ = 0x20000u;
  assert(landing_stage(2, 1));  // wrong preference owner must not admit
  GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_ = 0;
  assert(!landing_stage(3, 1)); // AI bypasses the human-only gate
}
"""


def compile_and_run(compiler: str, source: str, tmp_path: Path, name: str):
    compiler_path = shutil.which(compiler)
    if compiler_path is None:
        pytest.skip(compiler + " unavailable")
    cpp = tmp_path / (name + ".cpp")
    binary = tmp_path / name
    cpp.write_text(source)
    compiled = subprocess.run(
        [compiler_path, "-std=c++11", "-Wall", "-Wextra", "-Werror", str(cpp), "-o", str(binary)],
        capture_output=True, text=True, timeout=30,
    )
    assert compiled.returncode == 0, compiled.stderr
    return subprocess.run([str(binary)], capture_output=True, text=True, timeout=5)


@pytest.mark.parametrize("compiler", ["g++", "clang++"])
def test_pinned_population_and_landing_stages_and_mutants(compiler, tmp_path):
    population, landing = stage_expressions()
    declarations = mask_declarations()
    control = (harness(population, landing)
               .replace("MASK_DECLARATIONS", declarations)
               .replace("POPULATION_CONDITION", population)
               .replace("LANDING_CONDITION", landing))
    result = compile_and_run(compiler, control, tmp_path, "control")
    assert result.returncode == 0, result.stdout + result.stderr

    mutants = {
        "wrong_population_polarity": (population.replace("== 0", "!= 0", 1), landing),
        "stale_base_pointer": (population.replace("BaseCurrent->population_size", "base.population_size", 1), landing),
        "and_instead_of_or": (population, re.sub(r"\s+\|\|\s+", " && ", landing, count=1)),
        "wrong_preferences_owner": (population, landing.replace("game_preferences_", "game_more_preferences_", 1)),
    }
    for name, (mutant_population, mutant_landing) in mutants.items():
        assert (mutant_population, mutant_landing) != (population, landing)
        mutant = (harness(mutant_population, mutant_landing)
                  .replace("MASK_DECLARATIONS", declarations)
                  .replace("POPULATION_CONDITION", mutant_population)
                  .replace("LANDING_CONDITION", mutant_landing))
        result = compile_and_run(compiler, mutant, tmp_path, name)
        assert result.returncode != 0, name + " unexpectedly passed"

