"""Host controls for seven pinned, formerly get_noun-inlined consumers."""

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


def _one(pattern: str, body: str, label: str) -> str:
    matches = list(re.finditer(pattern, body, re.S))
    assert len(matches) == 1, f"{label}: expected one source sequence, found {len(matches)}"
    return matches[0].group(0)


def _actual_snippets():
    technology = (ROOT / "src/technology.cpp").read_text(encoding="utf-8")
    dipedit = (ROOT / "src/dipedit.cpp").read_text(encoding="utf-8")
    reportif = (ROOT / "src/reportif.cpp").read_text(encoding="utf-8")
    tech = _definition(technology, "void __cdecl say_tech(LPSTR output, int tech_id, BOOL category_lvl)")
    click = _definition(dipedit, "void DipEdit::on_button_clicked(int button_id)")
    init = _definition(dipedit, "void DipEdit::init(int faction_index)")
    status = _definition(reportif, "void ReportIf::on_status(int mode)")

    tech_seq = _one(
        r"GenderDefault\s*=\s*Players\[tech_id\s*-\s*MaxTechnologyNum\]\.noun_gender;\s*"
        r"PluralityDefault\s*=\s*Players\[tech_id\s*-\s*MaxTechnologyNum\]\.is_noun_plural;\s*"
        r"strcat\(output,\s*Players\[tech_id\s*-\s*MaxTechnologyNum\]\.noun_faction\);",
        tech, "say_tech",
    )
    parse_pattern = (
        r"GenderDefault\s*=\s*Players\[faction_index_\]\.noun_gender;\s*"
        r"PluralityDefault\s*=\s*Players\[faction_index_\]\.is_noun_plural;\s*"
        r"parse_says\(0,\s*Players\[faction_index_\]\.noun_faction,\s*-1,\s*-1\);"
    )
    parse_seqs = list(re.finditer(parse_pattern, click, re.S))
    assert len(parse_seqs) == 2, f"DipEdit click parse sites: {len(parse_seqs)}"
    click_seqs = [m.group(0) for m in parse_seqs]
    title_seq = _one(
        r"GenderDefault\s*=\s*Players\[faction_index_\]\.noun_gender;\s*"
        r"PluralityDefault\s*=\s*Players\[faction_index_\]\.is_noun_plural;\s*"
        r"strcat\(StringTemp,\s*Players\[faction_index_\]\.noun_faction\);",
        init, "DipEdit title",
    )
    list_seq = _one(
        r"StringTemp\[0\]\s*=\s*0;\s*"
        r"GenderDefault\s*=\s*Players\[i\]\.noun_gender;\s*"
        r"PluralityDefault\s*=\s*Players\[i\]\.is_noun_plural;\s*"
        r"strcat\(StringTemp,\s*Players\[i\]\.noun_faction\);\s*"
        r"list_box_\.item\(StringTemp,\s*i\);",
        init, "DipEdit list",
    )
    other_seq = _one(
        r"StringTemp\[0\]\s*=\s*0;\s*"
        r"GenderDefault\s*=\s*Players\[other\]\.noun_gender;\s*"
        r"PluralityDefault\s*=\s*Players\[other\]\.is_noun_plural;\s*"
        r"strcat\(StringTemp,\s*Players\[other\]\.noun_faction\);",
        status, "ReportIf other",
    )
    faction_seq = _one(
        r"GenderDefault\s*=\s*Players\[faction\]\.noun_gender;\s*"
        r"PluralityDefault\s*=\s*Players\[faction\]\.is_noun_plural;\s*"
        r"strcat\(StringTemp,\s*Players\[faction\]\.noun_faction\);",
        status, "ReportIf faction",
    )
    return tech_seq, click_seqs, title_seq, list_seq, other_seq, faction_seq


HARNESS = r"""
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define __cdecl
typedef int BOOL;
typedef char *LPSTR;
enum { MaxTechnologyNum = 7 };
struct Player { int noun_gender; int is_noun_plural; char noun_faction[32]; };
Player Players[8];
int GenderDefault, PluralityDefault;
char StringTemp[128];
struct ParseRecord { int slot, first, second; char text[32]; } parse_record;
int parse_count;
void parse_says(int slot, const char *text, int first, int second) {
    ++parse_count; parse_record.slot = slot; parse_record.first = first;
    parse_record.second = second; strcpy(parse_record.text, text);
}
struct ListBox { int id; char text[32]; void item(const char *s, int n) {
    id = n; strcpy(text, s);
} };
ListBox list_box_;

static void technology_case(int tech_id, const char *expected) {
    char output[128] = "Comm Frequency (";
    /* TECH */
    strcat(output, ")");
    if (strcmp(output, expected) || GenderDefault != 31 || PluralityDefault != 41) exit(11);
}
static void parse_case_2(int faction_index_) {
    /* PARSE2 */
    if (parse_record.slot != 0 || parse_record.first != -1 || parse_record.second != -1
        || strcmp(parse_record.text, Players[faction_index_].noun_faction)
        || GenderDefault != Players[faction_index_].noun_gender
        || PluralityDefault != Players[faction_index_].is_noun_plural) exit(12);
}
static void parse_case_3(int faction_index_) {
    /* PARSE3 */
    if (parse_record.slot != 0 || parse_record.first != -1 || parse_record.second != -1
        || strcmp(parse_record.text, Players[faction_index_].noun_faction)
        || GenderDefault != Players[faction_index_].noun_gender
        || PluralityDefault != Players[faction_index_].is_noun_plural) exit(13);
}
static void title_case(int faction_index_) {
    strcpy(StringTemp, "Leader ");
    /* TITLE */
    if (strcmp(StringTemp, "Leader noun-five")
        || GenderDefault != 51 || PluralityDefault != 61) exit(14);
}
static void list_case(int i) {
    /* LIST */
    if (strcmp(list_box_.text, "noun-six") || list_box_.id != i
        || GenderDefault != 71 || PluralityDefault != 81) exit(15);
}
static void other_case(int other) {
    /* OTHER */
    if (strcmp(StringTemp, "noun-two") || GenderDefault != 21 || PluralityDefault != 22) exit(16);
}
static void faction_case(int faction) {
    strcpy(StringTemp, "Leader Name label ");
    /* FACTION */
    if (strcmp(StringTemp, "Leader Name label noun-seven")
        || GenderDefault != 91 || PluralityDefault != 92) exit(17);
}
int main() {
    Players[2].noun_gender = 21; Players[2].is_noun_plural = 22;
    strcpy(Players[2].noun_faction, "noun-two");
    Players[3].noun_gender = 31; Players[3].is_noun_plural = 41;
    strcpy(Players[3].noun_faction, "noun-three");
    Players[5].noun_gender = 51; Players[5].is_noun_plural = 61;
    strcpy(Players[5].noun_faction, "noun-five");
    Players[6].noun_gender = 71; Players[6].is_noun_plural = 81;
    strcpy(Players[6].noun_faction, "noun-six");
    Players[7].noun_gender = 91; Players[7].is_noun_plural = 92;
    strcpy(Players[7].noun_faction, "noun-seven");
    technology_case(10, "Comm Frequency (noun-three)");
    parse_case_2(5); if (parse_count != 1) return 18;
    parse_case_3(5); if (parse_count != 2) return 19;
    title_case(5); list_case(6); other_case(2); faction_case(7);
    return 0;
}
"""


def _compile_and_run(compiler: str, source: str, tmp: Path, suffix: str) -> int:
    cpp = tmp / (suffix + ".cpp")
    exe = tmp / suffix
    cpp.write_text(source, encoding="utf-8")
    build = subprocess.run(
        [compiler, "-std=c++98", "-Wall", "-Wextra", str(cpp), "-o", str(exe)],
        capture_output=True, text=True,
    )
    assert build.returncode == 0, build.stdout + build.stderr
    return subprocess.run([str(exe)], capture_output=True).returncode


@pytest.mark.parametrize("compiler", ["g++", "clang++"])
def test_seven_actual_inline_noun_consumers_and_compile_success_mutants(compiler):
    compiler_path = shutil.which(compiler)
    if not compiler_path:
        pytest.skip(compiler + " is unavailable")
    tech, parses, title, list_item, other, faction = _actual_snippets()
    source = HARNESS.replace("/* TECH */", tech).replace("/* PARSE2 */", parses[0])
    source = source.replace("/* PARSE3 */", parses[1]).replace("/* TITLE */", title)
    source = source.replace("/* LIST */", list_item).replace("/* OTHER */", other)
    source = source.replace("/* FACTION */", faction)
    with tempfile.TemporaryDirectory(prefix="noun-inline-oct21w-") as temp:
        tmp = Path(temp)
        assert _compile_and_run(compiler_path, source, tmp, "positive") == 0

        mutants = {
            "wrong-faction": source.replace(
                "Players[tech_id - MaxTechnologyNum].noun_gender",
                "Players[tech_id - MaxTechnologyNum + 1].noun_gender", 1,
            ),
            "missing-grammar": source.replace(
                "PluralityDefault = Players[faction_index_].is_noun_plural;", "", 1,
            ),
            "wrong-parser-slot": source.replace(
                "parse_says(0, Players[faction_index_].noun_faction", 
                "parse_says(1, Players[faction_index_].noun_faction", 1,
            ),
            "wrong-noun-pointer": source.replace(
                "strcat(StringTemp, Players[i].noun_faction);",
                "strcat(StringTemp, Players[7].noun_faction);", 1,
            ),
        }
        for name, mutant in mutants.items():
            assert mutant != source, "mutation did not apply: " + name
            assert _compile_and_run(compiler_path, mutant, tmp, name) != 0, name
