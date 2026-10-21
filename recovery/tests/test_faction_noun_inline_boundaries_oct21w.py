"""Focused actual-source controls for the faction noun API boundary."""

from pathlib import Path
import re
import shutil
import subprocess
import tempfile

import pytest


ROOT = Path(__file__).resolve().parents[2]


def _definition(text: str, signature: str) -> str:
    match = re.search(r"(?m)^\s*" + re.escape(signature) + r"\s*\{", text)
    assert match, "missing source definition: " + signature
    opening = text.index("{", match.start())
    depth = 0
    state = "code"
    index = opening
    while index < len(text):
        char = text[index]
        next_char = text[index + 1] if index + 1 < len(text) else ""
        if state == "code":
            if char == "/" and next_char == "/":
                state = "line"
                index += 1
            elif char == "/" and next_char == "*":
                state = "block"
                index += 1
            elif char == '"':
                state = "string"
            elif char == "'":
                state = "char"
            elif char == "{":
                depth += 1
            elif char == "}":
                depth -= 1
                if depth == 0:
                    return text[match.start():index + 1]
        elif state == "line" and char == "\n":
            state = "code"
        elif state == "block" and char == "*" and next_char == "/":
            state = "code"
            index += 1
        elif state in ("string", "char"):
            if char == "\\":
                index += 1
            elif (state == "string" and char == '"') or (
                    state == "char" and char == "'"):
                state = "code"
        index += 1
    raise AssertionError("unterminated source definition: " + signature)


def _actual_sources() -> tuple[str, str, str]:
    faction = (ROOT / "src/faction.cpp").read_text(encoding="utf-8")
    base = (ROOT / "src/base.cpp").read_text(encoding="utf-8")
    game_map = (ROOT / "src/map.cpp").read_text(encoding="utf-8")
    get_noun = _definition(faction, "MEASURED LPSTR __cdecl get_noun(int faction_id)")
    name_base = _definition(
        base,
        "void __cdecl name_base(int faction_id, LPSTR name_out, BOOL is_final, BOOL is_sea_base)",
    )
    snippet_match = re.search(
        r"name_out\[0\]\s*=\s*0;\s*"
        r"GenderDefault\s*=\s*Players\[faction_id\]\.noun_gender;\s*"
        r"PluralityDefault\s*=\s*Players\[faction_id\]\.is_noun_plural;\s*"
        r"strcat\(name_out,\s*Players\[faction_id\]\.noun_faction\);",
        name_base,
    )
    assert snippet_match, "missing authorized inline name_base noun sequence"
    name_snippet = snippet_match.group(0)
    territory = _definition(
        game_map,
        "int __cdecl base_territory(int faction_id, int x, int y)",
    )
    assert "PlayersData[faction_id].diplo_treaties[owner] & DTREATY_VENDETTA" in territory
    return get_noun, name_snippet, territory


HARNESS = r"""
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MEASURED
#define __cdecl
typedef char *LPSTR;
typedef int BOOL;
enum { DTREATY_VENDETTA = 0x10 };

struct Player {
    int noun_gender;
    int is_noun_plural;
    char noun_faction[32];
};
struct PlayerData { unsigned int diplo_treaties[8]; };
Player Players[8];
PlayerData PlayersData[8];
int GenderDefault;
int PluralityDefault;

int TerritoryOwner;
int TerritoryBase;
int TerritoryCalls;
int TerritoryFaction;
int TerritoryX;
int TerritoryY;
int Human[8];
int HumanCalls;
int HumanTrace[4];

int whose_territory(int faction, int x, int y, int *base_id, bool) {
    ++TerritoryCalls;
    TerritoryFaction = faction;
    TerritoryX = x;
    TerritoryY = y;
    *base_id = TerritoryBase;
    return TerritoryOwner;
}
int is_human(int faction) {
    HumanTrace[HumanCalls++] = faction;
    return Human[faction];
}

GET_NOUN

LPSTR name_base_noun(int faction_id, LPSTR name_out) {
    NAME_SNIPPET
    return name_out;
}

BASE_TERRITORY

void require_at(bool condition, int line) {
    if (!condition) {
        fprintf(stderr, "assertion failed at line %d\n", line);
        exit(93);
    }
}
#define require(value) require_at((value), __LINE__)

void reset() {
    memset(Players, 0, sizeof Players);
    memset(PlayersData, 0, sizeof PlayersData);
    memset(Human, 0, sizeof Human);
    GenderDefault = -1;
    PluralityDefault = -1;
    TerritoryOwner = -1;
    TerritoryBase = 37;
    TerritoryCalls = 0;
    TerritoryFaction = TerritoryX = TerritoryY = -1;
    HumanCalls = 0;
}

void test_get_noun() {
    reset();
    Players[1].noun_gender = 4;
    Players[1].is_noun_plural = 1;
    strcpy(Players[1].noun_faction, "Gaian");
    Players[3].noun_gender = 8;
    Players[3].is_noun_plural = 0;
    strcpy(Players[3].noun_faction, "Spartan");
    require(get_noun(1) == Players[1].noun_faction);
    require(GenderDefault == 4 && PluralityDefault == 1);
    require(get_noun(3) == Players[3].noun_faction);
    require(GenderDefault == 8 && PluralityDefault == 0);
    require(strcmp(Players[1].noun_faction, "Gaian") == 0);
}

void test_name_base_inline() {
    reset();
    Players[2].noun_gender = 7;
    Players[2].is_noun_plural = 1;
    strcpy(Players[2].noun_faction, "University");
    Players[5].noun_gender = 9;
    Players[5].is_noun_plural = 0;
    strcpy(Players[5].noun_faction, "Morganite");
    char first[80] = "old";
    char second[80] = "old";
    name_base_noun(2, first);
    require(strcmp(first, "University") == 0);
    require(GenderDefault == 7 && PluralityDefault == 1);
    name_base_noun(5, second);
    require(strcmp(second, "Morganite") == 0);
    require(GenderDefault == 9 && PluralityDefault == 0);
    require(strcmp(Players[2].noun_faction, "University") == 0);
}

void reset_territory(int faction, int owner) {
    reset();
    TerritoryOwner = owner;
    Human[faction] = 1;
}

void test_base_territory_predicate_and_order() {
    reset_territory(1, -1);
    require(base_territory(1, 12, 13) == -1);
    require(TerritoryCalls == 1 && TerritoryFaction == 1);
    require(TerritoryX == 12 && TerritoryY == 13 && HumanCalls == 0);

    reset_territory(1, 1);
    require(base_territory(1, 2, 3) == -1);
    require(HumanCalls == 0);

    reset_territory(1, 2);
    Human[1] = 0;
    Human[2] = 0;
    require(base_territory(1, 2, 3) == -1);
    require(HumanCalls == 2 && HumanTrace[0] == 1 && HumanTrace[1] == 2);

    reset_territory(1, 2);
    PlayersData[1].diplo_treaties[2] = DTREATY_VENDETTA;
    PlayersData[2].diplo_treaties[1] = 0;
    require(base_territory(1, 4, 5) == -1);
    require(HumanCalls == 1 && HumanTrace[0] == 1);

    reset_territory(1, 2);
    PlayersData[1].diplo_treaties[2] = 0;
    PlayersData[2].diplo_treaties[1] = DTREATY_VENDETTA;
    require(base_territory(1, 6, 7) == TerritoryBase);

    reset_territory(1, 2);
    Human[1] = 0;
    Human[2] = 1;
    require(base_territory(1, 8, 9) == TerritoryBase);
    require(HumanCalls == 2 && HumanTrace[0] == 1 && HumanTrace[1] == 2);
}

int main() {
    test_get_noun();
    test_name_base_inline();
    test_base_territory_predicate_and_order();
    return 0;
}
"""


def _program(get_noun: str, name_snippet: str, territory: str) -> str:
    return (HARNESS.replace("GET_NOUN", get_noun)
            .replace("NAME_SNIPPET", name_snippet)
            .replace("BASE_TERRITORY", territory))


def _compile_and_run(compiler: str, source: str, expect_success: bool) -> None:
    with tempfile.TemporaryDirectory(prefix="faction-noun-oct21w-") as tmp:
        source_path = Path(tmp) / "control.cpp"
        output_path = Path(tmp) / "control"
        source_path.write_text(source, encoding="utf-8")
        build = subprocess.run(
            [compiler, "-std=c++98", "-Wall", "-Wextra", "-pedantic",
             str(source_path), "-o", str(output_path)],
            capture_output=True, text=True, check=False,
        )
        assert build.returncode == 0, build.stdout + build.stderr
        run = subprocess.run([str(output_path)], capture_output=True,
                             text=True, check=False)
        if expect_success:
            assert run.returncode == 0, run.stdout + run.stderr
        else:
            assert run.returncode == 93, run.stdout + run.stderr


@pytest.mark.parametrize("compiler_name", ["g++", "clang++"])
def test_actual_noun_and_territory_source_and_compile_success_mutants(compiler_name):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(compiler_name + " is not installed")
    get_noun, name_snippet, territory = _actual_sources()
    _compile_and_run(compiler, _program(get_noun, name_snippet, territory), True)

    mutants = [
        get_noun.replace(
            "return Players[faction_id].noun_faction;",
            "return Players[faction_id + 1].noun_faction;",
        ),
        get_noun.replace(
            "    PluralityDefault = Players[faction_id].is_noun_plural;\n", "",
        ),
        territory.replace(
            "PlayersData[faction_id].diplo_treaties[owner]",
            "PlayersData[owner].diplo_treaties[faction_id]",
        ),
    ]
    assert all(mutant != source for mutant, source in zip(
        mutants, (get_noun, get_noun, territory)))
    for mutant_get_noun, mutant_name, mutant_territory in (
        (mutants[0], name_snippet, territory),
        (mutants[1], name_snippet, territory),
        (get_noun, name_snippet, mutants[2]),
    ):
        _compile_and_run(
            compiler,
            _program(mutant_get_noun, mutant_name, mutant_territory),
            False,
        )
