"""Source-derived controls for social_upheaval's signed cost contract.

The helper and the social_ai penalty expression are extracted from production
source. Host execution checks C++ arithmetic only; this is not VC6 or game
acceptance evidence.
"""
from pathlib import Path
import re
import shutil
import subprocess

import pytest

ROOT = Path(__file__).resolve().parents[2]


def function_body(path, name):
    text = (ROOT / path).read_text()
    matches = list(re.finditer(
        r"(?m)^MEASURED inline (?:uint32_t|int) __cdecl " + re.escape(name) + r"\s*\([^\n]*\)\s*\{",
        text,
    ))
    assert len(matches) == 1, (path, name, len(matches))
    start = matches[0].start()
    opening = text.find("{", start)
    depth = 0
    for index in range(opening, len(text)):
        if text[index] == "{":
            depth += 1
        elif text[index] == "}":
            depth -= 1
            if depth == 0:
                return text[start:index + 1]
    raise AssertionError((path, name, "unterminated function"))


def actual_penalty_expression():
    text = (ROOT / "src/faction.cpp").read_text()
    matches = re.findall(
        r"(?m)^\s*(unk_val6\s*\+=\s*social_upheaval\(faction_id,\s*&soc_cat\)\s*/\s*-3;)\s*$",
        text,
    )
    assert len(matches) == 1
    return matches[0]


PREAMBLE = r"""
#include <cstdio>
#include <cstdlib>
#define MEASURED
#define __cdecl
typedef unsigned int uint32_t;
enum { MaxSocialCatNum = 4, DLVL_LIBRARIAN = 3 };
struct SocialCategory { int models_[MaxSocialCatNum]; };
struct PlayerData { SocialCategory soc_category_active; int diff_level; };
PlayerData PlayersData[1];
int Human[1];
int Alien[1];
int is_human(uint32_t faction_id) { return Human[faction_id]; }
int is_alien_faction(uint32_t faction_id) { return Alien[faction_id]; }
"""

MAIN = r"""
void require(bool ok, const char *what) {
    if (!ok) { std::fprintf(stderr, "WITNESS: %s\n", what); std::exit(43); }
}
int score_with_actual_caller(int faction_id, SocialCategory &soc_cat) {
    int unk_val6 = 0;
    // ACTUAL_PENALTY_EXPRESSION
    return unk_val6;
}
void setup(int diff, int human, int alien) {
    PlayersData[0].diff_level = diff;
    Human[0] = human;
    Alien[0] = alien;
    for (int i = 0; i < MaxSocialCatNum; ++i) {
        PlayersData[0].soc_category_active.models_[i] = 0;
    }
}
int main() {
    SocialCategory category;
    for (int i = 0; i < MaxSocialCatNum; ++i) category.models_[i] = 0;
    setup(5, 1, 0);
    require(social_upheaval(0, &category) == 0, "no changes return zero");
    category.models_[0] = 1;
    require(social_upheaval(0, &category) == 40, "human cost remains positive");
    require(score_with_actual_caller(0, category) == -13,
            "positive human cost uses signed division by -3");

    Alien[0] = 1;
    for (int i = 1; i < MaxSocialCatNum; ++i) category.models_[i] = 1;
    require(social_upheaval(0, &category) == 937,
            "four changes and human-alien half retain signed-cost result");
    require(score_with_actual_caller(0, category) == -312,
            "maximum ordinary cost retains signed penalty");

    setup(0, 0, 1);
    require(social_upheaval(0, &category) == 562,
            "nonhuman alien uses librarian cost and half");
    require(score_with_actual_caller(0, category) == -187,
            "alien penalty uses signed division");

    setup(-1, 1, 1);
    for (int i = 1; i < MaxSocialCatNum; ++i) category.models_[i] = 0;
    require(static_cast<int>(social_upheaval(0, &category)) == -12,
            "negative cost uses signed truncating alien half");
    require(score_with_actual_caller(0, category) == 4,
            "negative cost follows original signed division semantics");
    return 0;
}
"""


def compile_and_run(tmp_path, compiler, helper, damage=None):
    expression = actual_penalty_expression()
    source = PREAMBLE + helper + MAIN.replace(
        "// ACTUAL_PENALTY_EXPRESSION", expression
    )
    if damage == "logical-half":
        old = "cost += cost / 2;"
        assert helper.count(old) == 1
        helper = helper.replace(old, "cost += static_cast<uint32_t>(cost) / 2;")
        source = PREAMBLE + helper + MAIN.replace(
            "// ACTUAL_PENALTY_EXPRESSION", expression
        )
    elif damage == "unsigned-return":
        old = "MEASURED inline int __cdecl social_upheaval"
        assert helper.count(old) == 1
        helper = helper.replace(old, "MEASURED inline uint32_t __cdecl social_upheaval")
        source = PREAMBLE + helper + MAIN.replace(
            "// ACTUAL_PENALTY_EXPRESSION", expression
        )

    source_path = tmp_path / "social_upheaval.cpp"
    output = tmp_path / "social_upheaval"
    source_path.write_text(source)
    built = subprocess.run(
        [compiler, "-std=c++98", "-Wall", "-Wextra", "-Werror",
         str(source_path), "-o", str(output)],
        capture_output=True, text=True, timeout=30,
    )
    assert built.returncode == 0, built.stderr
    return subprocess.run([str(output)], capture_output=True, text=True, timeout=10)


@pytest.mark.parametrize("compiler", ["g++", "clang++"])
def test_actual_helper_and_actual_social_ai_penalty_expression(tmp_path, compiler):
    if not shutil.which(compiler):
        pytest.skip(compiler + " unavailable")
    helper = function_body("src/faction.h", "social_upheaval")
    result = compile_and_run(tmp_path, compiler, helper)
    assert result.returncode == 0, result.stderr


@pytest.mark.parametrize("compiler", ["g++", "clang++"])
@pytest.mark.parametrize("damage", ["unsigned-return", "logical-half"])
def test_source_mutants_are_rejected(tmp_path, compiler, damage):
    if not shutil.which(compiler):
        pytest.skip(compiler + " unavailable")
    helper = function_body("src/faction.h", "social_upheaval")
    result = compile_and_run(tmp_path, compiler, helper, damage)
    assert result.returncode == 43, result.stderr
    expected = "positive human cost" if damage == "unsigned-return" else "negative cost uses signed truncating alien half"
    assert "WITNESS: " + expected in result.stderr
