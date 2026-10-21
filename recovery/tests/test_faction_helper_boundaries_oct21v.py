"""Actual-source host controls for the faction helper visibility boundary.

These host tests exercise the moved production bodies and a small modeled
state surface; they are not VC6, original-TU, or game acceptance evidence.
"""
from pathlib import Path
import re
import shutil
import subprocess

import pytest

ROOT = Path(__file__).resolve().parents[2]
CPP = ROOT / "src/faction.cpp"
HEADER = ROOT / "src/faction.h"

FUNCTIONS = {
    "has_treaty": "int",
    "get_adjective": "LPSTR",
    "social_upheaval": "int",
}


def function_body(text: str, result_type: str, name: str) -> str:
    matches = list(re.finditer(
        r"(?m)^MEASURED " + re.escape(result_type) + r" __cdecl " +
        re.escape(name) + r"\s*\([^\n]*\)\s*\{", text
    ))
    assert len(matches) == 1, (name, len(matches))
    start = matches[0].start()
    opening = text.find("{", matches[0].end() - 1)
    depth = 0
    for index in range(opening, len(text)):
        if text[index] == "{":
            depth += 1
        elif text[index] == "}":
            depth -= 1
            if depth == 0:
                return text[start:index + 1]
    raise AssertionError((name, "unterminated definition"))


PREAMBLE = r"""
#include <cstdio>
#include <cstdlib>
#include <cstring>
#define MEASURED
#define __cdecl
typedef char *LPSTR;
enum { MaxSocialCatNum = 4, DLVL_LIBRARIAN = 3 };
struct SocialCategory { int models_[MaxSocialCatNum]; };
struct PlayerData {
    int diplo_treaties[8];
    SocialCategory soc_category_active;
    int diff_level;
};
struct Player {
    char *adj_name_faction;
    char *adj_name_other;
};
PlayerData PlayersData[8];
Player Players[8];
int Human[8];
int Alien[8];
int is_human(unsigned int faction_id) { return Human[faction_id]; }
int is_alien_faction(unsigned int faction_id) { return Alien[faction_id]; }

void require(bool ok, const char *what) {
    if (!ok) {
        std::fprintf(stderr, "WITNESS: %s\n", what);
        std::exit(43);
    }
}
"""

MAIN = r"""
void reset_social(int diff, int human, int alien) {
    PlayersData[1].diff_level = diff;
    Human[1] = human;
    Alien[1] = alien;
    for (int i = 0; i < MaxSocialCatNum; ++i) {
        PlayersData[1].soc_category_active.models_[i] = 0;
    }
}

int main() {
    PlayersData[1].diplo_treaties[3] = 0x00A5;
    require(has_treaty(1, 3, 0x0005) == 0x0005,
            "treaty returns the raw selected mask");
    require(has_treaty(1, 3, 0x0008) == 0,
            "absent treaty bit returns zero");
    require(has_treaty(3, 1, 0x0005) == 0,
            "treaty keeps faction argument order");

    char faction_name[] = "faction adjective";
    char other_name[] = "other field";
    Players[1].adj_name_faction = faction_name;
    Players[1].adj_name_other = other_name;
    require(get_adjective(1) == faction_name,
            "adjective returns the faction-name field pointer unchanged");

    SocialCategory category;
    for (int i = 0; i < MaxSocialCatNum; ++i) category.models_[i] = 0;
    reset_social(5, 1, 0);
    require(social_upheaval(1, &category) == 0,
            "unchanged social model has no upheaval cost");
    category.models_[0] = 1;
    require(social_upheaval(1, &category) == 40,
            "human social cost keeps cubic count and difficulty");

    Alien[1] = 1;
    for (int i = 1; i < MaxSocialCatNum; ++i) category.models_[i] = 1;
    require(social_upheaval(1, &category) == 937,
            "human alien cost keeps the one-half surcharge");

    reset_social(0, 0, 1);
    require(social_upheaval(1, &category) == 562,
            "nonhuman alien uses librarian difficulty and surcharge");

    reset_social(-1, 1, 1);
    for (int i = 0; i < MaxSocialCatNum; ++i) category.models_[i] = 0;
    category.models_[0] = 1;
    require(social_upheaval(1, &category) == -12,
            "negative signed cost retains truncating alien half");
    return 0;
}
"""


def actual_definitions():
    text = CPP.read_text(encoding="utf-8")
    return {name: function_body(text, result_type, name)
            for name, result_type in FUNCTIONS.items()}


def program(definitions):
    return PREAMBLE + "\n" + "\n".join(
        definitions[name] for name in FUNCTIONS
    ) + "\n" + MAIN


def compile_and_run(compiler, tmp_path, definitions):
    source = tmp_path / "faction_helper_boundaries.cpp"
    output = tmp_path / "faction_helper_boundaries"
    source.write_text(program(definitions), encoding="utf-8")
    built = subprocess.run(
        [compiler, "-std=c++98", "-Wall", "-Wextra", "-Werror",
         str(source), "-o", str(output)],
        capture_output=True, text=True, timeout=45,
    )
    assert built.returncode == 0, built.stderr
    return subprocess.run([str(output)], capture_output=True, text=True,
                          timeout=15)


def test_headers_declare_and_faction_cpp_owns_the_three_bodies():
    header = HEADER.read_text(encoding="utf-8")
    cpp = CPP.read_text(encoding="utf-8")
    for name, result_type in FUNCTIONS.items():
        declaration = re.compile(
            r"(?m)^MEASURED " + re.escape(result_type) + r" __cdecl " +
            re.escape(name) + r"\s*\([^\n]*\);$"
        )
        assert len(declaration.findall(header)) == 1, name
        assert not re.search(
            r"(?m)^MEASURED inline " + re.escape(result_type) +
            r" __cdecl " + re.escape(name) + r"\s*\(", header
        ), name
        assert len(re.findall(
            r"(?m)^MEASURED " + re.escape(result_type) + r" __cdecl " +
            re.escape(name) + r"\s*\([^\n]*\)\s*\{", cpp
        )) == 1, name


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
def test_actual_moved_bodies_preserve_treaty_adjective_and_social_effects(
    tmp_path, compiler_name
):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip("host compiler unavailable")
    result = compile_and_run(compiler, tmp_path, actual_definitions())
    assert result.returncode == 0, result.stderr


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
@pytest.mark.parametrize("name, old, new, witness", (
    ("has_treaty", "return PlayersData[faction_id].diplo_treaties[faction_id_with] & treaty;",
     "return (PlayersData[faction_id].diplo_treaties[faction_id_with] & treaty) != 0;",
     "treaty returns the raw selected mask"),
    ("get_adjective", "return Players[faction_id].adj_name_faction;",
     "return Players[faction_id].adj_name_other;",
     "adjective returns the faction-name field pointer unchanged"),
    ("social_upheaval", "cost += cost / 2;", "cost += 0;",
     "human alien cost keeps the one-half surcharge"),
))
def test_compile_success_semantic_mutants_are_caught(
    tmp_path, compiler_name, name, old, new, witness
):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip("host compiler unavailable")
    definitions = actual_definitions()
    assert definitions[name].count(old) == 1
    definitions[name] = definitions[name].replace(old, new, 1)
    result = compile_and_run(compiler, tmp_path, definitions)
    assert result.returncode == 43, result.stderr
    assert "WITNESS: " + witness in result.stderr
