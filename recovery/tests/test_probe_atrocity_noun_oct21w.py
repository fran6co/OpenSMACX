"""Exercise the actual Probe atrocity-followup helper and mapped noun getter."""
from __future__ import annotations

import re
import shutil
import subprocess
from pathlib import Path

import pytest
from match_recovery.source import read_file

ROOT = next(path for path in Path(__file__).resolve().parents
            if (path / "src/probe.cpp").is_file())


def _mask_cpp(text: str) -> str:
    pattern = re.compile(
        r'//[^\n]*|/\*[\s\S]*?\*/|"(?:\\.|[^"\\])*"|\'(?:\\.|[^\'\\])*\'',
        re.MULTILINE,
    )
    return pattern.sub(lambda m: "".join("\n" if c == "\n" else " "
                                          for c in m.group()), text)


def _extract(path: Path, signature: str) -> str:
    source = path.read_text(encoding="utf-8")
    masked = _mask_cpp(source)
    matches = list(re.finditer(signature, masked, re.MULTILINE))
    assert len(matches) == 1, "expected one active definition: %s in %s" % (signature, path)
    start = matches[0].start()
    opening = masked.find("{", matches[0].end())
    assert opening >= 0
    depth = 0
    for index in range(opening, len(masked)):
        if masked[index] == "{":
            depth += 1
        elif masked[index] == "}":
            depth -= 1
            if depth == 0:
                return source[start:index + 1]
    raise AssertionError("unterminated definition: %s" % signature)


def _mapped_getter(address: int, name: str, function: str) -> str:
    records = [row for row in read_file(ROOT / "src/faction.cpp")
               if row.address == address]
    assert len(records) == 1 and records[0].name == name
    record = records[0]
    body_path = ROOT / (record.body if record.body else "src/faction.cpp")
    assert body_path.is_file(), "mapped getter body host missing: %s" % body_path
    return _extract(body_path, r"(?:MEASURED\s+)?(?:inline\s+)?LPSTR\s+__cdecl\s+%s\s*\(" % function)


HELPER_SIG = r"static\s+void\s+probe_native_atrocity_followup\s*\("
HELPER = _extract(ROOT / "src/probe.cpp", HELPER_SIG)
NOUN = _mapped_getter(0x50B930, "?get_noun@@YAHH@Z", "get_noun")
ADJECTIVE = _mapped_getter(0x50B910, "?get_adjective@@YAHH@Z", "get_adjective")


HOST = r'''#include <cassert>
#include <stdint.h>
#include <cstring>
#include <cstdio>

#define __cdecl
#define MEASURED
typedef int BOOL;
typedef char *LPSTR;

struct Player {
    int noun_gender; int is_noun_plural;
    char noun_faction[32]; char adj_name_faction[32];
};
Player Players[8];
int GenderDefault = -1;
int PluralityDefault = -1;
struct PlayerData { unsigned flags; } PlayersData[8];
struct Base {
    int population_size;
    unsigned state;
    int x, y;
    char *name_string;
    int unk_x;
    int nutrients_accumulated;
} Bases[8];
struct Veh { unsigned char morale; } Vehs[8];
struct Console { int faction_id_; } ConsoleGlobal;
struct StateFields { unsigned game_state_; };
struct GameState { StateFields fields_; } GameDataStateStorage;
char StringTemp[256];

int events[64]; int event_count;
int parser_slots[4], parser_genders[4], parser_pluralities[4];
char parser_text[4][64]; int parser_count;
int pop_calls, pop_local, health_calls, rnd_calls, fac_calls;
const char *popped_image;

static void event(int value) { assert(event_count < 64); events[event_count++] = value; }
int probe_policy_signed32(uint32_t bits) { return (int32_t)bits; }
void atrocity(int actor, int target, int a, int b) {
    assert(a == 1 && b == 0); event(10 + actor); event(20 + target);
}
int range(int value, int low, int high) {
    event(30); if (value < low) return low; if (value > high) return high; return value;
}
int has_fac(int facility, int base, int) {
    assert(facility == 0x11 || facility == 0x12); assert(base == 4);
    ++fac_calls; event(40 + facility); return 0;
}
int veh_at(int x, int y) { assert(x == 7 && y == 9); event(50); return 1; }
int veh_health(int id) { assert(id == 1); ++health_calls; event(51); return 4; }
int rnd(int bound, const char *label) {
    ++rnd_calls; if (label) assert(std::strcmp(label, "PROBEASSASSIN") == 0);
    assert(bound == 2); event(52); return 1;
}
int veh_next_stack_id(int id) { assert(id == 1); event(53); return -1; }
void spot_base(int target, int actor) { assert(target == 4 && actor == 2); event(60); }
void draw_tile(int x, int y, int mode) { assert(x == 7 && y == 9 && mode == 2); event(61); }
int parse_says(int slot, char *text, int a, int b) {
    assert(a == -1 && b == -1 && parser_count < 4);
    parser_slots[parser_count] = slot;
    parser_genders[parser_count] = GenderDefault;
    parser_pluralities[parser_count] = PluralityDefault;
    std::strncpy(parser_text[parser_count], text, 63);
    ++parser_count; event(70 + slot); return 1;
}
void says(const char *text) { std::strcat(StringTemp, text); event(80); }
void X_pop(char *text, const char *image, int local) {
    assert(std::strstr(text, "GENETICWARFARE") != 0);
    ++pop_calls; popped_image = image; pop_local = local; event(90);
}
'''


def _main() -> str:
    return r'''static void reset_case(int console, unsigned state) {
    std::memset(PlayersData, 0, sizeof(PlayersData));
    std::memset(Bases, 0, sizeof(Bases));
    std::memset(Vehs, 0, sizeof(Vehs));
    event_count = parser_count = pop_calls = health_calls = rnd_calls = fac_calls = 0;
    ConsoleGlobal.faction_id_ = console;
    GameDataStateStorage.fields_.game_state_ = state;
    GenderDefault = PluralityDefault = -1;
    StringTemp[0] = '\0'; popped_image = 0; pop_local = -1;
    Bases[4].population_size = 8; Bases[4].x = 7; Bases[4].y = 9;
    Bases[4].name_string = (char *)"TargetBase";
    Bases[4].unk_x = 0x1200; Bases[4].state = 0;
    PlayersData[4].flags = 0;
    Players[2].noun_gender = 22; Players[2].is_noun_plural = 1;
    std::strcpy(Players[2].noun_faction, "actor-noun");
    Players[4].noun_gender = 44; Players[4].is_noun_plural = 0;
    std::strcpy(Players[4].noun_faction, "target-noun");
}

static void assert_effect_prefix() {
    const int expected[] = {12, 24, 30, 57, 58, 50, 51, 52, 51,
                            57, 58, 53, 60, 61};
    assert(event_count >= 14);
    for (int i = 0; i < 14; ++i) assert(events[i] == expected[i]);
    assert(health_calls == 2 && rnd_calls == 1);
    assert(fac_calls == 4);                     // population + vehicle checks
    assert(Bases[4].population_size == 7);
    assert((Bases[4].state & 0x20000u) != 0);
    assert(PlayersData[4].flags == 0x400u);
    assert(Bases[4].nutrients_accumulated == 0);
    assert(Vehs[1].morale == 3);
}

static void assert_notice_order(const char *marker) {
    assert(event_count == 19);
    assert(events[14] == 70 && events[15] == 71); // parser slots 0, then 1
    assert(events[16] == 80 && events[17] == 80); // message, then local/remote marker
    assert(events[18] == 90);                     // popup follows message assembly
    char expected[64];
    std::strcpy(expected, "GENETICWARFARE");
    std::strcat(expected, marker);
    assert(std::strcmp(StringTemp, expected) == 0);
}

int main() {
    for (int i = 0; i < 8; ++i) {
        Players[i].noun_gender = 10 + i;
        Players[i].is_noun_plural = i & 1;
        std::sprintf(Players[i].noun_faction, "noun%d", i);
        std::sprintf(Players[i].adj_name_faction, "adj%d", i);
    }
    // Local actor receives the notice; noun defaults must be live at slot 1.
    reset_case(2, 0);
    probe_native_atrocity_followup(2, 4, 4, 0, 0, 1);
    assert_effect_prefix();
    assert(parser_count == 2 && parser_slots[0] == 0 && parser_slots[1] == 1);
    assert(std::strcmp(parser_text[0], "TargetBase") == 0);
    assert(std::strcmp(parser_text[1], "actor-noun") == 0);
    assert(parser_genders[1] == 22 && parser_pluralities[1] == 1);
    assert(pop_calls == 1 && pop_local == 0);
    assert(std::strcmp(popped_image, "al_infil_sm.pcx") == 0);
    assert_notice_order("0");

    // Target-local path is also visible, but uses remote selector/artwork.
    reset_case(4, 0);
    probe_native_atrocity_followup(2, 4, 4, 0, 0, 0);
    assert_effect_prefix();
    assert(parser_count == 2 && parser_slots[0] == 0 && parser_slots[1] == 1);
    assert(std::strcmp(parser_text[1], "actor-noun") == 0);
    assert(parser_genders[1] == 22 && parser_pluralities[1] == 1);
    assert(pop_calls == 1 && pop_local == 0);
    assert(std::strcmp(popped_image, "infil_sm.pcx") == 0);
    assert_notice_order("1");

    // Local-visibility bit admits the notice when neither faction is local.
    reset_case(3, 0x80);
    probe_native_atrocity_followup(2, 4, 4, 0, 0, 0);
    assert_effect_prefix();
    assert(parser_count == 2 && std::strcmp(parser_text[1], "actor-noun") == 0);
    assert(pop_calls == 1);
    assert_notice_order("1");

    // Without actor, target, or visibility, all gameplay effects remain but
    // the parser and popup path is skipped.
    reset_case(3, 0);
    probe_native_atrocity_followup(2, 4, 4, 0, 0, 1);
    assert_effect_prefix();
    assert(parser_count == 0 && pop_calls == 0);
    assert(event_count == 14);
    return 0;
}
'''


def _program(helper: str = HELPER) -> str:
    return HOST + NOUN + "\n" + ADJECTIVE + "\n" + helper + "\n" + _main()


def _compile_run(tmp_path: Path, source: str, compiler: str, stem: str):
    cpp = tmp_path / (stem + ".cpp")
    exe = tmp_path / stem
    cpp.write_text(source, encoding="utf-8")
    built = subprocess.run(
        [compiler, "-std=c++98", "-Wall", "-Wextra", "-Werror", str(cpp), "-o", str(exe)],
        capture_output=True, text=True, timeout=30,
    )
    assert built.returncode == 0, built.stdout + built.stderr
    return subprocess.run([str(exe)], capture_output=True, text=True, timeout=10)


def test_actual_atrocity_helper_uses_mapped_actor_noun():
    assert HELPER.count("parse_says(0, base.name_string, -1, -1)") == 1
    assert HELPER.count("parse_says(1, get_noun(actor_faction_id), -1, -1)") == 1
    assert HELPER.count("parse_says(1, get_adjective(actor_faction_id), -1, -1)") == 0
    assert "spot_base(target_base_id, actor_faction_id);" in HELPER
    assert "if (actor_faction_id != ConsoleGlobal.faction_id_" in HELPER


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
def test_actual_helper_branches_and_compiling_noun_damage_controls(tmp_path: Path,
                                                                  compiler_name: str):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(compiler_name + " unavailable")
    result = _compile_run(tmp_path, _program(), compiler, "probe_atrocity_actual")
    assert result.returncode == 0, result.stdout + result.stderr

    mutations = (
        ("get_noun(actor_faction_id)", "get_adjective(actor_faction_id)"),
        ("get_noun(actor_faction_id)", "get_noun(target_faction_id)"),
        ("parse_says(0, base.name_string, -1, -1);\n    parse_says(1, get_noun(actor_faction_id), -1, -1);",
         "parse_says(1, get_noun(actor_faction_id), -1, -1);\n    parse_says(0, base.name_string, -1, -1);"),
    )
    for index, (old, new) in enumerate(mutations):
        assert HELPER.count(old) == 1, old
        mutant = HELPER.replace(old, new, 1)
        run = _compile_run(tmp_path, _program(mutant), compiler,
                           "probe_atrocity_mutant_%d" % index)
        assert run.returncode != 0, "compiled mutant escaped: %s -> %s" % (old, new)
