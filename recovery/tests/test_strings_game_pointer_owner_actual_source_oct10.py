"""Actual game callers preserve rule-string pointers through the Strings API."""
from __future__ import annotations

import shutil
import subprocess
import tempfile
import re
from pathlib import Path

import pytest


ROOT = Path(__file__).resolve().parents[2]
OWNERS = (
    "base.cpp", "basewin.cpp", "faction.cpp", "map.cpp",
    "maininterface.cpp", "newgame.cpp", "battlewin.cpp",
)


def _actual_call() -> str:
    source = (ROOT / "src/base.cpp").read_text(encoding="utf-8")
    needle = "say_result = parse_say(1, Facility[absolute_location].name, -1, -1);"
    assert source.count(needle) == 1
    return needle


def _compile_and_run(statement: str, compiler: str, directory: Path, name: str) -> int:
    harness = r'''#include <cassert>
using LPSTR = char *;
struct FacilityRecord { LPSTR name; };
static FacilityRecord Facility[2];
static LPSTR observed;
static int calls;
static int parse_say(int slot, LPSTR value, int gender, int plural) {
    assert(slot == 1 && gender == -1 && plural == -1);
    observed = value;
    ++calls;
    return 23;
}
int main() {
    char first[] = "first";
    char second[] = "second";
    Facility[0].name = first;
    Facility[1].name = second;
    const int absolute_location = 1;
    int say_result = 0;
    ACTUAL_STATEMENT
    assert(say_result == 23 && calls == 1 && observed == second);
}
'''.replace("ACTUAL_STATEMENT", statement)
    source_path = directory / f"{name}.cpp"
    executable = directory / name
    source_path.write_text(harness, encoding="utf-8")
    built = subprocess.run(
        [compiler, "-std=c++11", "-Wall", "-Wextra", str(source_path), "-o", str(executable)],
        capture_output=True, text=True, timeout=45,
    )
    assert built.returncode == 0, built.stdout + built.stderr
    return subprocess.run([str(executable)], capture_output=True, text=True, timeout=20).returncode


def test_game_callers_pass_rule_name_pointers_without_integer_narrowing() -> None:
    texts = {name: (ROOT / "src" / name).read_text(encoding="utf-8") for name in OWNERS}
    compact = {name: re.sub(r"\s+", "", text) for name, text in texts.items()}
    assert "reinterpret_cast<int>" not in "\n".join(texts.values())
    assert "static_cast<int>(reinterpret_cast" not in "\n".join(texts.values())
    assert "parse_say(1,Facility[absolute_location].name,-1,-1)" in compact["base.cpp"]
    assert "parse_say(0,Facility[-entry].name,-1,-1)" in compact["basewin.cpp"]
    assert "parse_say(2,Proposal[selected_proposal].name,-1,-1)" in compact["faction.cpp"]
    assert "StringTable.get(Natural[LM_CRATER].name)" in compact["map.cpp"]
    assert "StringTable.get(Difficulty[i])" in compact["newgame.cpp"]
    assert "StringTable.get(Weapon[prototype.weapon_id].name)" in compact["battlewin.cpp"]
    assert "StringTable.get(Proposal[proposal].name)" in compact["faction.cpp"]
    assert "StringTable.get(Terraforming[tool].name)" in compact["maininterface.cpp"]


@pytest.mark.parametrize("compiler", ("g++", "clang++"))
def test_actual_parse_say_caller_preserves_pointer_identity_and_status(
    compiler: str, tmp_path: Path,
) -> None:
    executable = shutil.which(compiler)
    if executable is None:
        pytest.skip(f"{compiler} unavailable")
    assert _compile_and_run(_actual_call(), executable, tmp_path, "actual_pointer") == 0


def test_compiling_wrong_facility_pointer_mutant_is_detected() -> None:
    compiler = shutil.which("g++")
    if compiler is None:
        pytest.skip("g++ unavailable")
    statement = _actual_call()
    mutant = statement.replace("Facility[absolute_location].name", "Facility[0].name", 1)
    assert mutant != statement
    with tempfile.TemporaryDirectory(prefix="strings-game-pointer-mutant-") as temp:
        assert _compile_and_run(mutant, compiler, Path(temp), "wrong_pointer") != 0
