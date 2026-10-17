"""Actual-source host controls for the three recovered pact parsers.

These tests exercise the selected Player strings and observable call order in
bounded C++98 doubles. They do not establish target-ABI or whole-game behavior.
"""
import shutil
import subprocess
import tempfile
from pathlib import Path

import pytest

from recovery.tests.test_pact_ascent import definition


ROOT = next(parent for parent in Path(__file__).resolve().parents
            if (parent / "src/faction.cpp").is_file())
FIXTURE = r'''#include <assert.h>
#include <string.h>

#ifndef __cdecl
#define __cdecl
#endif

typedef int BOOL;
enum { FALSE = 0, TRUE = 1 };
int events[32];
int event_count;

struct Player {
    int is_leader_female;
    char search_key[24];
    char name_leader[24];
    char title_leader[24];
    char noun_faction[24];
    int noun_gender;
    int is_noun_plural;
};

struct ParserEvent {
    int slot;
    const char *text;
    int gender;
    int plurality;
    int arg3;
    int arg4;
};

struct NetMessage {
    int calls;
    const char *label;
    int delay;
    int mode;
    int destination;
    void pop(const char *value, int wait, int kind, int target) {
        ++calls;
        label = value;
        delay = wait;
        mode = kind;
        destination = target;
        events[event_count++] = 900;
    }
};

Player Players[8];
int GenderDefault;
BOOL PluralityDefault;
ParserEvent parser_events[16];
int parser_count;
NetMessage NetMsgPrimary;
const char DiploLabelPACTATTACKS[] = "PACTATTACKS";
const char DiploLabelPACTTRUCE[] = "PACTTRUCE";
const char DiploLabelPACTUNPACT[] = "PACTUNPACT";
char he_a[] = "HE_LOCAL";
char his_a[] = "HIS_LOCAL";
char hood_ab[] = "PACT_HOOD_LOCAL_OTHER";
int he_calls;
int his_calls;
int hood_calls;

int __cdecl parse_says(int slot, const char *text, int arg3, int arg4) {
    parser_events[parser_count].slot = slot;
    parser_events[parser_count].text = text;
    parser_events[parser_count].gender = GenderDefault;
    parser_events[parser_count].plurality = PluralityDefault;
    parser_events[parser_count].arg3 = arg3;
    parser_events[parser_count].arg4 = arg4;
    ++parser_count;
    events[event_count++] = 100 + slot;
    return 0;
}

char *__cdecl get_he_she(int faction, int form) {
    assert(faction == 2 && form == 0);
    ++he_calls;
    events[event_count++] = 200;
    return he_a;
}

char *__cdecl get_his_her(int faction, int form) {
    assert(faction == 2 && form == 0);
    ++his_calls;
    events[event_count++] = 201;
    return his_a;
}

char *__cdecl get_pact_hood(int faction, int faction_with) {
    assert(faction == 2 && faction_with == 5);
    ++hood_calls;
    events[event_count++] = 202;
    return hood_ab;
}

// @@PACT_ATTACKS@@
// @@PACT_TRUCE@@
// @@PACT_UNPACT@@

static void reset_fixture() {
    parser_count = 0;
    event_count = 0;
    he_calls = 0;
    his_calls = 0;
    hood_calls = 0;
    NetMsgPrimary.calls = 0;
    Players[2].is_leader_female = 1;
    strcpy(Players[2].search_key, "SEARCH_LOCAL");
    strcpy(Players[2].name_leader, "NAME_LOCAL");
    strcpy(Players[2].title_leader, "TITLE_LOCAL");
    strcpy(Players[2].noun_faction, "NOUN_LOCAL");
    Players[2].noun_gender = 7;
    Players[2].is_noun_plural = 1;
    Players[5].is_leader_female = 0;
    strcpy(Players[5].search_key, "SEARCH_OTHER");
    strcpy(Players[5].name_leader, "NAME_OTHER");
    strcpy(Players[5].title_leader, "TITLE_OTHER");
    strcpy(Players[5].noun_faction, "NOUN_OTHER");
    Players[5].noun_gender = 9;
    Players[5].is_noun_plural = 0;
}

static void assert_parse(int index, int slot, const char *text,
                         int gender, int plurality) {
    assert(index < parser_count);
    assert(parser_events[index].slot == slot);
    assert(strcmp(parser_events[index].text, text) == 0);
    assert(parser_events[index].gender == gender);
    assert(parser_events[index].plurality == plurality);
    assert(parser_events[index].arg3 == -1);
    assert(parser_events[index].arg4 == -1);
}

static void assert_log(const int *expected, int count) {
    assert(event_count == count);
    for (int i = 0; i < count; ++i) assert(events[i] == expected[i]);
}

static void assert_popup(const char *label) {
    assert(NetMsgPrimary.calls == 1);
    assert(strcmp(NetMsgPrimary.label, label) == 0);
    assert(NetMsgPrimary.delay == 5000);
    assert(NetMsgPrimary.mode == 0);
    assert(NetMsgPrimary.destination == 0);
}

static void check_attacks() {
    static const int order[] = {100, 101, 102, 200, 103, 104, 900};
    reset_fixture();
    pact_attacks(2, 5);
    assert(parser_count == 5 && he_calls == 1 && his_calls == 0 && hood_calls == 0);
    assert_parse(0, 0, "TITLE_LOCAL", 1, FALSE);
    assert_parse(1, 1, "NAME_LOCAL", 1, FALSE);
    assert_parse(2, 2, "NOUN_LOCAL", 7, TRUE);
    assert_parse(3, 3, "HE_LOCAL", 7, TRUE);
    assert_parse(4, 4, "NOUN_OTHER", 9, FALSE);
    assert_popup("PACTATTACKS");
    assert_log(order, sizeof(order) / sizeof(order[0]));
}

static void check_truce() {
    static const int order[] = {100, 101, 102, 200, 103, 104, 105, 106, 900};
    reset_fixture();
    pact_truce(2, 5);
    assert(parser_count == 7 && he_calls == 1 && his_calls == 0 && hood_calls == 0);
    assert_parse(0, 0, "TITLE_LOCAL", 1, FALSE);
    assert_parse(1, 1, "NAME_LOCAL", 1, FALSE);
    assert_parse(2, 2, "NOUN_LOCAL", 7, TRUE);
    assert_parse(3, 3, "HE_LOCAL", 7, TRUE);
    assert_parse(4, 4, "TITLE_OTHER", 0, FALSE);
    assert_parse(5, 5, "NAME_OTHER", 0, FALSE);
    assert_parse(6, 6, "NOUN_OTHER", 9, FALSE);
    assert_popup("PACTTRUCE");
    assert_log(order, sizeof(order) / sizeof(order[0]));
}

static void check_unpact() {
    static const int order[] = {
        100, 101, 102, 200, 103, 201, 104, 202, 105, 106, 107, 108, 900
    };
    reset_fixture();
    pact_unpact(2, 5);
    assert(parser_count == 9 && he_calls == 1 && his_calls == 1 && hood_calls == 1);
    assert_parse(0, 0, "TITLE_LOCAL", 1, FALSE);
    assert_parse(1, 1, "NAME_LOCAL", 1, FALSE);
    assert_parse(2, 2, "NOUN_LOCAL", 7, TRUE);
    assert_parse(3, 3, "HE_LOCAL", 7, TRUE);
    assert_parse(4, 4, "HIS_LOCAL", 7, TRUE);
    assert_parse(5, 5, "PACT_HOOD_LOCAL_OTHER", 7, TRUE);
    assert_parse(6, 6, "TITLE_OTHER", 0, FALSE);
    assert_parse(7, 7, "NAME_OTHER", 0, FALSE);
    assert_parse(8, 8, "NOUN_OTHER", 9, FALSE);
    assert_popup("PACTUNPACT");
    assert_log(order, sizeof(order) / sizeof(order[0]));
}

int main() {
    check_attacks();
    check_truce();
    check_unpact();
    return 0;
}
'''


def _actual_bodies():
    source = (ROOT / "src/faction.cpp").read_text()
    return {
        "attacks": definition(source, r"void\s+__cdecl\s+pact_attacks\s*\("),
        "truce": definition(source, r"void\s+__cdecl\s+pact_truce\s*\("),
        "unpact": definition(source, r"void\s+__cdecl\s+pact_unpact\s*\("),
    }


def _program(bodies):
    result = FIXTURE
    for name, token in (
        ("attacks", "// @@PACT_ATTACKS@@"),
        ("truce", "// @@PACT_TRUCE@@"),
        ("unpact", "// @@PACT_UNPACT@@"),
    ):
        assert result.count(token) == 1
        result = result.replace(token, bodies[name], 1)
    assert "@@PACT_" not in result
    return result


def _compile_and_run(compiler, directory, label, bodies, should_pass):
    source = directory / (label + ".cpp")
    binary = directory / label
    source.write_text(_program(bodies))
    compiled = subprocess.run(
        [compiler, "-std=c++98", "-Wall", "-Wextra", "-Werror", "-pedantic",
         str(source), "-o", str(binary)],
        capture_output=True, text=True, timeout=45,
    )
    assert compiled.returncode == 0, compiled.stdout + compiled.stderr
    result = subprocess.run([str(binary)], capture_output=True, text=True, timeout=15)
    if should_pass:
        assert result.returncode == 0, result.stdout + result.stderr
    else:
        assert result.returncode != 0, "actual-source field mutation escaped: " + label


def _field_damage_cases(bodies):
    cases = []
    # Each parser slot is independently changed back to the old wrong field.
    # The participants' title/name/search strings are pairwise distinct.
    for function, participant, title_slot, name_slot in (
        ("attacks", "faction_id", 0, 1),
        ("truce", "faction_id", 0, 1),
        ("truce", "faction_id_with", 4, 5),
        ("unpact", "faction_id", 0, 1),
        ("unpact", "faction_id_with", 6, 7),
    ):
        body = bodies[function]
        title = "Players[" + participant + "].title_leader"
        name = "Players[" + participant + "].name_leader"
        search = "Players[" + participant + "].search_key"
        title_call = "parse_says(" + str(title_slot) + ", " + title
        name_call = "parse_says(" + str(name_slot) + ", " + name
        assert body.count(title_call) == 1, (function, title_call)
        assert body.count(name_call) == 1, (function, name_call)
        damaged = dict(bodies)
        damaged[function] = body.replace(title_call,
                                         "parse_says(" + str(title_slot) + ", " + name, 1)
        cases.append((function + "_slot" + str(title_slot) + "_name_instead_of_title",
                      damaged))
        damaged = dict(bodies)
        damaged[function] = body.replace(name_call,
                                         "parse_says(" + str(name_slot) + ", " + search, 1)
        cases.append((function + "_slot" + str(name_slot) + "_search_instead_of_name",
                      damaged))
    return cases


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
def test_actual_pact_sources_select_title_then_name_with_ordered_side_effects(compiler_name):
    compiler = shutil.which(compiler_name)
    if not compiler:
        pytest.skip(compiler_name + " unavailable")
    bodies = _actual_bodies()
    with tempfile.TemporaryDirectory(prefix="pact-parse-fields-oct17-") as temp:
        directory = Path(temp)
        _compile_and_run(compiler, directory, "actual", bodies, True)
        for label, damaged in _field_damage_cases(bodies):
            _compile_and_run(compiler, directory, "damage_" + label, damaged, False)
