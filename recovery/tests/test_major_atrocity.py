"""Actual-source host behavior and damage controls for major_atrocity.

The tests project the actual source helper and owner body into a private C++98
fixture and run behavior and mutation controls. The fixture's field projection
is not native layout, linked preservation, runtime or whole-game evidence.
"""

import re
import shutil
import subprocess
from pathlib import Path

import pytest


ROOT = next(parent for parent in Path(__file__).resolve().parents
            if (parent / "src/faction.cpp").is_file())
FACTION_CPP = ROOT / "src" / "faction.cpp"
FACTION_H = ROOT / "src" / "faction.h"
BODY_SIGNATURE = "void __cdecl major_atrocity(int faction_id, int victim_id)"
HELPER_SIGNATURE = "major_atrocity_add_wrapped32"


def _extract_function(text, name):
    match = re.search(
        r"(?:^|\n)[^\n;{}]*\b" + re.escape(name) + r"\s*\([^;{}]*\)\s*\{",
        text,
    )
    if match is None:
        raise AssertionError("missing source definition: " + name)
    start = match.start()
    if text[start] == "\n":
        start += 1
    opening = text.index("{", match.start())
    depth = 0
    state = "code"
    escaped = False
    index = opening
    while index < len(text):
        char = text[index]
        next_char = text[index + 1] if index + 1 < len(text) else ""
        if state == "line_comment":
            if char == "\n":
                state = "code"
        elif state == "block_comment":
            if char == "*" and next_char == "/":
                state = "code"
                index += 1
        elif state in ("string", "character"):
            if escaped:
                escaped = False
            elif char == "\\":
                escaped = True
            elif (state == "string" and char == '"') or (
                state == "character" and char == "'"
            ):
                state = "code"
        elif char == "/" and next_char == "/":
            state = "line_comment"
            index += 1
        elif char == "/" and next_char == "*":
            state = "block_comment"
            index += 1
        elif char == '"':
            state = "string"
        elif char == "'":
            state = "character"
        elif char == "{":
            depth += 1
        elif char == "}":
            depth -= 1
            if depth == 0:
                return text[start:index + 1]
        index += 1
    raise AssertionError("unterminated source definition: " + name)


def _product_definitions():
    source = FACTION_CPP.read_text()
    helper = _extract_function(source, HELPER_SIGNATURE)
    body = _extract_function(source, "major_atrocity")
    assert BODY_SIGNATURE in body, "unexpected major_atrocity signature"
    return helper, body


def _literal_alias(header_text, name, expected):
    declarations = [
        line for line in header_text.splitlines()
        if re.search(r"\b" + re.escape(name) + r"\b", line)
        and not line.lstrip().startswith("//")
    ]
    assert len(declarations) == 1, "expected one declaration for " + name
    declaration = declarations[0]
    assert re.search(r"=\s*\"" + re.escape(expected) + r"\"\s*;", declaration), (
        name + " must be initialized from the source-owned literal " + repr(expected)
    )
    assert not re.search(r"=\s*\(\s*char\s*\*\s*\)\s*(?:0x[0-9A-Fa-f]+)",
                         declaration), name + " still aliases an executable address"


FIXTURE = r'''#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#ifndef __cdecl
#define __cdecl
#endif

static const int MaxPlayerNum = 8;
static const int MaxProposalNum = 11;
enum CouncilProposals {
    PROP_REPEAL_UN_CHARTER = 8,
    PROP_REINSTATE_UN_CHARTER = 9
};

struct Player {
    int is_leader_female;
    char name_leader[24];
    char title_leader[24];
    char noun_faction[24];
    int noun_gender;
    int is_noun_plural;
};
struct PlayerData {
    uint32_t diplo_treaties[MaxPlayerNum];
    uint32_t diplo_agenda[MaxPlayerNum];
    int diplo_merc[MaxPlayerNum];
    int diplo_spoke[MaxPlayerNum];
    int integrity_blemishes;
    int major_atrocities;
};
struct ConsoleHostProjection { int faction_id_; };

static Player Players[MaxPlayerNum];
static PlayerData PlayersData[MaxPlayerNum];
static int CouncilResolutionCount[MaxProposalNum];
static int CouncilLastResolutionTurn[MaxProposalNum];
static uint32_t LockEnableMask;
static int TurnCurrentNum;
static int GenderDefault;
static int PluralityDefault;
static char StringTemp[1032];
static char ScriptPopupFile[] = "SCRIPT.txt";
static char *const DiploLabelMAJORATROCITY = (char *)"MAJORATROCITY";
static char *const DiploLabelCouncilSmPcx = (char *)"council_sm.pcx";
static ConsoleHostProjection ConsoleGlobal;

struct Event { char kind; int first; int second; int third; const char *text; };
static Event Events[128];
static int EventCount;
static int SpokeAtTreatyCallback;
static int MercAtTreatyCallback;
static void log_event(char kind, int first, int second, int third,
                      const char *text) {
    if (EventCount < 128) {
        Events[EventCount].kind = kind;
        Events[EventCount].first = first;
        Events[EventCount].second = second;
        Events[EventCount].third = third;
        Events[EventCount].text = text;
        ++EventCount;
    }
}
static void __cdecl pact_ends(int actor, int partner) {
    log_event('E', actor, partner, 0, 0);
}
static void __cdecl treaty_on(int actor, int partner, int treaty) {
    log_event('T', actor, partner, treaty, 0);
    SpokeAtTreatyCallback = PlayersData[partner].diplo_spoke[actor];
    MercAtTreatyCallback = PlayersData[actor].diplo_merc[partner];
    PlayersData[actor].diplo_treaties[partner] |= 0x200u;
    TurnCurrentNum = 77;
}
static int __cdecl parse_says(int slot, const char *value, int gender,
                             int plural) {
    log_event('S', slot, GenderDefault, PluralityDefault, value);
    (void)gender; (void)plural;
    return 0;
}
static int __cdecl popp(const char *caption, const char *label, int flags,
                        const char *picture, int (__cdecl *callback)()) {
    log_event('O', flags, 0, 0, label);
    (void)caption; (void)picture; (void)callback;
    return 0;
}
static char *__cdecl _itoa(int value, char *buffer, int radix) {
    if (radix == 10) sprintf(buffer, "%d", value);
    else buffer[0] = '\0';
    return buffer;
}
'''


TESTS = r'''static int fail(const char *message) {
    fprintf(stderr, "FAIL: %s\n", message);
    return 1;
}
static void reset_fixture(void) {
    memset(Players, 0, sizeof Players);
    memset(PlayersData, 0, sizeof PlayersData);
    memset(CouncilResolutionCount, 0, sizeof CouncilResolutionCount);
    memset(CouncilLastResolutionTurn, 0, sizeof CouncilLastResolutionTurn);
    memset(StringTemp, 0, sizeof StringTemp);
    memset(Events, 0, sizeof Events);
    EventCount = 0; LockEnableMask = 0; TurnCurrentNum = 42;
    SpokeAtTreatyCallback = 0; MercAtTreatyCallback = 0;
    GenderDefault = 0; PluralityDefault = 0; ConsoleGlobal.faction_id_ = 2;
    for (int i = 0; i < MaxPlayerNum; ++i) {
        sprintf(Players[i].name_leader, "leader%d", i);
        sprintf(Players[i].title_leader, "title%d", i);
        sprintf(Players[i].noun_faction, "faction%d", i);
        Players[i].is_leader_female = i & 1;
        Players[i].noun_gender = i + 10;
        Players[i].is_noun_plural = i & 1;
    }
}
static int test_relationship_masks_and_callback_order(void) {
    reset_fixture();
    // Faction 1 is human and alive; faction 3 is victim; 4,5,6 are alive.
    LockEnableMask = 0x02u | ((0x02u | 0x08u | 0x10u | 0x20u | 0x40u) << 8);
    // Deliberately disagree by direction: pact/exclusion predicates read the
    // actor row, while the 0x40 update and merc field use the partner row.
    PlayersData[2].diplo_treaties[4] = 1u;
    PlayersData[4].diplo_treaties[2] = 0x10u;
    PlayersData[2].diplo_treaties[5] = 0x10u;
    PlayersData[2].diplo_treaties[6] = 0x10u;
    PlayersData[6].diplo_treaties[2] = 1u;
    PlayersData[2].diplo_spoke[4] = -7;
    PlayersData[4].diplo_merc[2] = 13;
    Players[4].is_leader_female = 1;
    Players[4].is_noun_plural = 1;
    Players[2].is_leader_female = 1;
    Players[2].is_noun_plural = 1;
    major_atrocity(2, 3);
    if (PlayersData[3].diplo_treaties[2] != 0xC0020u) return fail("victim-to-actor treaty mark");
    if (PlayersData[3].diplo_agenda[2] != 4u) return fail("victim agenda mark");
    if (PlayersData[2].integrity_blemishes != 6) return fail("lower clamp");
    if (PlayersData[2].major_atrocities != 1) return fail("atrocity increment");
    if (EventCount != 9) return fail("human/alive/victim/directed-treaty filtering");
    if (Events[0].kind != 'E' || Events[0].first != 2 || Events[0].second != 4)
        return fail("pact callback precedes treaty callback");
    if (Events[1].kind != 'T' || Events[1].first != 4 || Events[1].second != 2 || Events[1].third != 0x18)
        return fail("treaty callback orientation");
    if (Events[2].kind != 'S' || Events[2].first != 0 || strcmp(Events[2].text, "title4") != 0)
        return fail("notification parse sequence");
    for (int slot = 0; slot < 6; ++slot) {
        const int gender = slot == 2 ? 14 : (slot == 5 ? 12 : 1);
        const int plural = slot == 2 || slot == 5 ? 1 : 0;
        if (Events[slot + 2].first != slot ||
            Events[slot + 2].second != gender ||
            Events[slot + 2].third != plural)
            return fail("parser slot/grammar sequence");
    }
    if (Events[8].kind != 'O' || strcmp(Events[8].text, "MAJORATROCITY0") != 0)
        return fail("first popup");
    if (PlayersData[4].diplo_treaties[2] != 0x250u)
        return fail("partner-row relationship bit update");
    if (PlayersData[2].diplo_treaties[4] != 1u ||
        PlayersData[2].diplo_treaties[6] != 0x10u)
        return fail("actor-row relationships remain directional");
    if (PlayersData[4].diplo_merc[2] != 50)
        return fail("partner-row merc update");
    if (PlayersData[2].diplo_spoke[4] != 77 ||
        PlayersData[4].diplo_spoke[2] != 0)
        return fail("actor-row spoke-turn update");
    if (SpokeAtTreatyCallback != -7 || MercAtTreatyCallback != 13)
        return fail("treaty callback precedes merc and live-turn writes");
    return 0;
}
static int test_council_slots_and_early_marks(void) {
    reset_fixture();
    CouncilResolutionCount[PROP_REPEAL_UN_CHARTER] = 2;
    CouncilResolutionCount[PROP_REINSTATE_UN_CHARTER] = 1;
    CouncilLastResolutionTurn[PROP_REPEAL_UN_CHARTER] = 999;
    major_atrocity(2, 3);
    if (PlayersData[3].diplo_treaties[2] != 0xC0020u) return fail("early treaty mark");
    if (PlayersData[3].diplo_agenda[2] != 4u) return fail("early agenda mark");
    if (PlayersData[2].major_atrocities != 0 || EventCount != 0)
        return fail("repeal/reinstate count gate");
    return 0;
}
static int test_wrapping_and_clamps(void) {
    reset_fixture();
    PlayersData[2].integrity_blemishes = 0x7fffffff;
    PlayersData[2].major_atrocities = 0x7fffffff;
    major_atrocity(2, -1);
    if (PlayersData[2].integrity_blemishes != -2147483647) return fail("wrapped null-victim counter");
    if (PlayersData[2].major_atrocities != (-2147483647 - 1)) return fail("wrapped atrocity counter");
    reset_fixture();
    PlayersData[2].integrity_blemishes = 0x7fffffff;
    major_atrocity(2, 3);
    if (PlayersData[2].integrity_blemishes != 6) return fail("wrapped lower clamp");
    reset_fixture();
    PlayersData[2].integrity_blemishes = 98;
    major_atrocity(2, 3);
    if (PlayersData[2].integrity_blemishes != 99) return fail("upper clamp");
    return 0;
}
int main(void) {
    if (test_relationship_masks_and_callback_order()) return 1;
    if (test_council_slots_and_early_marks()) return 1;
    if (test_wrapping_and_clamps()) return 1;
    puts("PASS: tracked major_atrocity host projection");
    return 0;
}
'''


def _program(helper, body):
    return "\n".join((FIXTURE, helper, body, TESTS))


def _compile_and_run(compiler_name, text, tmp_path, stem):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(compiler_name + " is unavailable")
    source_path = tmp_path / (stem + ".cpp")
    binary_path = tmp_path / stem
    source_path.write_text(text)
    subprocess.run(
        [compiler, "-std=c++98", "-Wall", "-Wextra", "-pedantic", "-Werror",
         str(source_path), "-o", str(binary_path)],
        check=True,
    )
    return subprocess.run([str(binary_path)], check=False)


@pytest.mark.parametrize(
    "name, expected",
    [("DiploLabelMAJORATROCITY", "MAJORATROCITY"),
     ("DiploLabelCouncilSmPcx", "council_sm.pcx")],
)
def test_diplomacy_labels_are_source_owned_literals(name, expected):
    _literal_alias(FACTION_H.read_text(), name, expected)


@pytest.mark.parametrize("compiler", ["g++", "clang++"])
def test_tracked_product_body_and_service_projection(compiler, tmp_path):
    helper, body = _product_definitions()
    result = _compile_and_run(compiler, _program(helper, body), tmp_path,
                              "major-atrocity-product-" + compiler)
    assert result.returncode == 0


@pytest.mark.parametrize("compiler", ["g++", "clang++"])
@pytest.mark.parametrize(
    "label, old, new",
    [
        ("wrong-last-turn-gate",
         "CouncilResolutionCount[PROP_REINSTATE_UN_CHARTER]",
         "CouncilLastResolutionTurn[PROP_REPEAL_UN_CHARTER]"),
        ("wrong-treaty-direction",
         "PlayersData[victim_id].diplo_treaties[faction_id] |= 0xC0020;",
         "PlayersData[faction_id].diplo_treaties[victim_id] |= 0xC0020;"),
        ("wrong-wrapped-counter",
         "bits += static_cast<uint32_t>(amount);",
         "bits += static_cast<uint32_t>(amount - 1);"),
        ("wrong-clamp",
         "PlayersData[faction_id].integrity_blemishes = 0x63;",
         "PlayersData[faction_id].integrity_blemishes = 0x64;"),
        ("actor-relation-direction",
         "PlayersData[faction_id].diplo_treaties[other_id];",
         "PlayersData[other_id].diplo_treaties[faction_id];"),
        ("missing-merc-write",
         "PlayersData[other_id].diplo_merc[faction_id] = 50;",
         "PlayersData[other_id].diplo_merc[faction_id] = 0;"),
        ("spoke-direction",
         "PlayersData[faction_id].diplo_spoke[other_id] = TurnCurrentNum;",
         "PlayersData[other_id].diplo_spoke[faction_id] = TurnCurrentNum;"),
        ("stale-turn",
         "PlayersData[faction_id].diplo_spoke[other_id] = TurnCurrentNum;",
         "PlayersData[faction_id].diplo_spoke[other_id] = 42;"),
        ("human-mask", "(faction_bit & human_mask) != 0 ||",
         "((faction_bit & human_mask) != 0 && false) ||"),
        ("alive-mask", "(faction_bit & alive_mask) == 0) {",
         "((faction_bit & alive_mask) == 0 && false)) {"),
        ("callback-sequence",
         "            pact_ends(faction_id, other_id);\n"
         "        }\n        treaty_on(other_id, faction_id, 0x18);",
         "            treaty_on(other_id, faction_id, 0x18);\n"
         "        }\n        pact_ends(faction_id, other_id);"),
    ],
)
def test_tracked_body_damage_mutations_fail(compiler, label, old, new, tmp_path):
    helper, body = _product_definitions()
    if label == "wrong-wrapped-counter":
        assert old in helper, "tracked helper shape changed; review mutation anchor"
        damaged_helper = _replace_once(helper, old, new)
        damaged_body = body
    else:
        assert old in body, "tracked body shape changed; review mutation anchor: " + label
        damaged_helper = helper
        damaged_body = _replace_once(body, old, new)
    result = _compile_and_run(compiler, _program(damaged_helper, damaged_body), tmp_path,
                              compiler + "-product-damage-" + label)
    assert result.returncode != 0, label + " escaped behavior controls"


def _replace_once(text, old, new):
    assert old in text, old
    return text.replace(old, new, 1)
