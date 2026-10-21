"""Actual-source controls for the two original slot-4 noun producers."""
from pathlib import Path
import re
import shutil
import subprocess
import tempfile

import pytest


ROOT = Path(__file__).resolve().parents[2]


def _body(source: str, signature: str) -> str:
    match = re.search(r"(?m)^" + re.escape(signature) + r"\s*\{", source)
    assert match, "missing actual source definition: " + signature
    opening = source.index("{", match.start())
    depth = 0
    for index in range(opening, len(source)):
        if source[index] == "{":
            depth += 1
        elif source[index] == "}":
            depth -= 1
            if depth == 0:
                return source[match.start():index + 1]
    raise AssertionError("unterminated actual source definition: " + signature)


def _block(body: str, needle: str) -> str:
    start = body.find(needle)
    assert start >= 0, "missing actual source branch: " + needle
    opening = body.find("{", start + len(needle))
    assert opening >= 0
    depth = 0
    for index in range(opening, len(body)):
        if body[index] == "{":
            depth += 1
        elif body[index] == "}":
            depth -= 1
            if depth == 0:
                return body[opening + 1:index]
    raise AssertionError("unterminated branch: " + needle)


def _through_statement(body: str, needle: str, terminator: str,
                      last: bool = False) -> str:
    start = body.rfind(needle) if last else body.find(needle)
    assert start >= 0, "missing actual source branch: " + needle
    end = body.find(terminator, start)
    assert end >= 0, "missing branch terminator: " + needle
    return body[start:end + len(terminator)]


def _actual_fragments() -> tuple[str, str, str, str, str]:
    source = (ROOT / "src/faction.cpp").read_text(encoding="utf-8")
    noun = _body(source, "MEASURED LPSTR __cdecl get_noun(int faction_id)")
    war = _body(source, "void __cdecl enemies_war(int faction_id, int faction_id_with)")
    treaty = _body(
        source,
        "void __cdecl enemies_treaty(int faction_id, int faction_id_with,\n                             int check_technology)",
    )
    war_a = _block(
        war,
        "if ((PlayersData[local_faction_id].diplo_treaties[faction_id]",
    )
    war_b = _block(
        war,
        "else if ((PlayersData[local_faction_id].diplo_treaties[faction_id_with]",
    )
    treaty_b = _through_statement(
        treaty, "faction_parse_leader_fields(faction_id_with, 0, 1, 2, 3);",
        "return;", last=True,
    )
    treaty_a = _through_statement(
        treaty, "faction_parse_leader_fields(faction_id, 0, 1, 2, 3);",
        "treaty_on(faction_id, faction_id_with, DTREATY_TREATY);", last=True,
    )
    assert "parse_says(4, get_noun(faction_id), -1, -1);" in war_a
    assert "parse_says(4, get_noun(faction_id), -1, -1);" in war_b
    assert "parse_says(4, get_noun(faction_id), -1, -1);" in treaty_a
    assert "parse_says(4, get_noun(faction_id), -1, -1);" in treaty_b
    return noun, war_a, war_b, treaty_a, treaty_b


HARNESS = r'''#include <cassert>
#include <cstring>
#include <string>
#include <vector>

#define MEASURED
#define __cdecl
#define FALSE 0
#define nullptr 0
typedef char *LPSTR;
typedef int BOOL;
enum { DTREATY_TREATY = 2, DTREATY_TRUCE = 4, DTREATY_PACT = 8 };

struct Player {
    int noun_gender;
    int is_noun_plural;
    int is_leader_female;
    char noun_faction[32];
    char title_leader[32];
    char name_leader[32];
};
struct PlayerData { unsigned int diplo_treaties[8]; };
struct Console { int faction_id_; };
struct Parsed { int slot; std::string text; int gender; int plural; };
struct Notice { std::string label; int parsed_count; int slot4_gender; int slot4_plural; std::string slot4_text; };

Player Players[8];
PlayerData PlayersData[8];
Console ConsoleGlobal;
int GenderDefault;
int PluralityDefault;
std::vector<Parsed> parsed;
std::vector<std::string> effects;
std::vector<Notice> notices;

void parse_says(int slot, LPSTR text, int, int) {
    Parsed value = { slot, text, GenderDefault, PluralityDefault };
    parsed.push_back(value);
    effects.push_back("parse");
}
LPSTR get_he_she(int, int) {
    static char pronoun[] = "they";
    return pronoun;
}
void faction_parse_leader_fields(int faction, int a, int b, int c, int d) {
    int fields[4] = { a, b, c, d };
    for (int i = 0; i != 4; ++i) {
        LPSTR text = fields[i] == 0 ? Players[faction].title_leader
                   : fields[i] == 1 ? Players[faction].name_leader
                                    : Players[faction].noun_faction;
        Parsed value = { i, text, Players[faction].is_leader_female, 0 };
        parsed.push_back(value);
        effects.push_back("parse");
    }
}
struct NetMessage {
    void pop(const char *label, int, int, void *) {
        Notice notice;
        notice.label = label;
        notice.parsed_count = (int)parsed.size();
        notice.slot4_gender = parsed.at(4).gender;
        notice.slot4_plural = parsed.at(4).plural;
        notice.slot4_text = parsed.at(4).text;
        notices.push_back(notice);
        effects.push_back("pop");
    }
} NetMsgPrimary;
void treaty_on(int, int, int) { effects.push_back("treaty_on"); }

NOUN_BODY

void war_a(int faction_id, int faction_id_with) {
    const int local_faction_id = ConsoleGlobal.faction_id_;
    WAR_A_BODY
}
void war_b(int faction_id, int faction_id_with) {
    const int local_faction_id = ConsoleGlobal.faction_id_;
    WAR_B_BODY
}
void treaty_a(int faction_id, int faction_id_with) {
    TREATY_A_BODY
}
void treaty_b(int faction_id, int faction_id_with) {
    TREATY_B_BODY
}

void reset() {
    std::memset(Players, 0, sizeof Players);
    std::memset(PlayersData, 0, sizeof PlayersData);
    ConsoleGlobal.faction_id_ = 3;
    GenderDefault = -1;
    PluralityDefault = -1;
    parsed.clear(); effects.clear(); notices.clear();
    Players[1].noun_gender = 11; Players[1].is_noun_plural = 1;
    std::strcpy(Players[1].noun_faction, "A-noun");
    Players[1].is_leader_female = 1;
    std::strcpy(Players[1].title_leader, "A-title");
    std::strcpy(Players[1].name_leader, "A-leader");
    Players[2].noun_gender = 22; Players[2].is_noun_plural = 0;
    std::strcpy(Players[2].noun_faction, "B-noun");
    Players[2].is_leader_female = 0;
    std::strcpy(Players[2].title_leader, "B-title");
    std::strcpy(Players[2].name_leader, "B-leader");
}
void check_slot4(const char *notice, int count) {
    assert(parsed.size() == (unsigned int)count);
    assert(parsed[4].slot == 4);
    assert(parsed[4].text == "A-noun");
    assert(parsed[4].gender == 11 && parsed[4].plural == 1);
    assert(notices.size() == 1 && notices[0].label == notice);
    assert(notices[0].parsed_count == count);
    assert(notices[0].slot4_text == "A-noun");
    assert(notices[0].slot4_gender == 11 && notices[0].slot4_plural == 1);
    assert(effects.size() >= 6 && effects[5] == "pop");
}
int main() {
    reset(); war_a(1, 2);
    check_slot4("PACTATTACKS", 5);
    assert(parsed[2].text == "A-noun" && parsed[2].gender == 11 && parsed[2].plural == 1);
    assert(effects.size() == 6 && effects[5] == "pop");

    reset(); war_b(1, 2);
    check_slot4("PACTATTACKS", 5);
    assert(parsed[2].text == "B-noun" && parsed[2].gender == 22 && parsed[2].plural == 0);

    reset(); treaty_a(1, 2);
    check_slot4("PACTTREATY", 5);
    assert(effects.size() == 7 && effects[5] == "pop" && effects[6] == "treaty_on");

    reset(); treaty_b(1, 2);
    check_slot4("PACTTREATY", 5);
    assert(effects.size() == 7 && effects[5] == "pop" && effects[6] == "treaty_on");
    return 0;
}
'''


def _program(fragments: tuple[str, str, str, str, str]) -> str:
    noun, war_a, war_b, treaty_a, treaty_b = fragments
    return (HARNESS.replace("NOUN_BODY", noun)
            .replace("WAR_A_BODY", war_a)
            .replace("WAR_B_BODY", war_b)
            .replace("TREATY_A_BODY", treaty_a)
            .replace("TREATY_B_BODY", treaty_b))


def _run(compiler: str, source: str, success: bool) -> None:
    with tempfile.TemporaryDirectory(prefix="enemies-nouns-oct21w-") as temp:
        cpp = Path(temp) / "control.cpp"
        exe = Path(temp) / "control"
        cpp.write_text(source, encoding="utf-8")
        build = subprocess.run(
            [compiler, "-std=c++98", "-Wall", "-Wextra", "-pedantic",
             str(cpp), "-o", str(exe)], capture_output=True, text=True,
            check=False,
        )
        assert build.returncode == 0, build.stdout + build.stderr
        run = subprocess.run([str(exe)], capture_output=True, text=True,
                             check=False)
        if success:
            assert run.returncode == 0, run.stdout + run.stderr
        else:
            assert run.returncode != 0, "wrong noun producer escaped controls"


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
def test_actual_enemies_noun_producers_and_reversal_mutants(compiler_name):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(compiler_name + " is not installed")
    fragments = _actual_fragments()
    _run(compiler, _program(fragments), True)

    noun, war_a, war_b, treaty_a, treaty_b = fragments
    assert "parse_says(4, get_noun(faction_id), -1, -1);" in war_a
    assert "parse_says(4, get_noun(faction_id), -1, -1);" in treaty_a
    wrong_war_a = war_a.replace(
        "parse_says(4, get_noun(faction_id), -1, -1);",
        "parse_says(4, get_noun(faction_id_with), -1, -1);", 1,
    )
    wrong_treaty_a = treaty_a.replace(
        "parse_says(4, get_noun(faction_id), -1, -1);",
        "parse_says(4, get_noun(faction_id_with), -1, -1);", 1,
    )
    assert wrong_war_a != war_a and wrong_treaty_a != treaty_a
    _run(compiler, _program((noun, wrong_war_a, war_b, treaty_a, treaty_b)), False)
    _run(compiler, _program((noun, war_a, war_b, wrong_treaty_a, treaty_b)), False)
