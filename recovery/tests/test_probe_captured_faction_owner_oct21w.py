"""Focused host controls for the actual Probe monument-choice source regions.

The test extracts the production helpers from probe.cpp and the current noun
getters from faction.h; host structs/stubs only provide bounded observations
and are not claims about the VC6 ABI or complete Probe behavior.
"""
from __future__ import annotations

import re
import shutil
import subprocess
from pathlib import Path

import pytest
from match_recovery.source import read_file

ROOT = next(path for path in Path(__file__).resolve().parents
            if (path / "src/probe.cpp").is_file())
PROBE = ROOT / "src/probe.cpp"


def _mask_cpp(text: str) -> str:
    # Preserve offsets/newlines while blanking comments and quoted literals.
    pattern = re.compile(
        r'//[^\n]*|/\*[\s\S]*?\*/|"(?:\\.|[^"\\])*"|\'(?:\\.|[^\'\\])*\'',
        re.MULTILINE,
    )
    return pattern.sub(lambda match: "".join("\n" if char == "\n" else " "
                                               for char in match.group()), text)


def _extract(path: Path, signature: str) -> str:
    source = path.read_text(encoding="utf-8")
    masked = _mask_cpp(source)
    matches = list(re.finditer(signature, masked, re.MULTILINE))
    assert len(matches) == 1, f"expected one active source definition in {path}: {signature}"
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
    raise AssertionError(f"unterminated definition in {path}: {signature}")


LIST_SIG = r"static\s+int\s+probe_native_monument_choice_list\s*\("
CLEAR_SIG = r"static\s+void\s+probe_native_clear_monument_choice_word\s*\("
COMMIT_SIG = r"static\s+int\s+probe_native_commit_monument_choice\s*\("
def _getter_body_source(address: int) -> Path:
    # Resolve the current mapped owner record first; body migrations may move
    # these definitions from faction.h to faction.cpp without moving identity.
    owner = ROOT / "src/faction.cpp"
    matches = [row for row in read_file(owner) if row.address == address]
    assert len(matches) == 1, "missing/duplicate getter mapping 0x%08X" % address
    record = matches[0]
    assert record.name == ("?get_noun@@YAHH@Z" if address == 0x50B930
                           else "?get_adjective@@YAHH@Z")
    body_path = ROOT / (record.body if record.body else "src/faction.cpp")
    assert body_path.is_file(), "mapped getter body host is missing: %s" % body_path
    return body_path


def _production_functions() -> dict[str, str]:
    noun_sig = r"(?:MEASURED\s+)?(?:inline\s+)?LPSTR\s+__cdecl\s+get_noun\s*\("
    adjective_sig = r"(?:MEASURED\s+)?(?:inline\s+)?LPSTR\s+__cdecl\s+get_adjective\s*\("
    return {
        "list": _extract(PROBE, LIST_SIG),
        "clear": _extract(PROBE, CLEAR_SIG),
        "commit": _extract(PROBE, COMMIT_SIG),
        "noun": _extract(_getter_body_source(0x50B930), noun_sig),
        "adjective": _extract(_getter_body_source(0x50B910), adjective_sig),
    }


HOST = r'''#include <cassert>
#include <stdint.h>
#include <cstdio>
#include <cstring>

#define __cdecl
#define MEASURED
typedef int BOOL;
typedef char *LPSTR;

struct Player {
    int noun_gender;
    int is_noun_plural;
    char noun_faction[24];
    char adj_name_faction[24];
};
Player Players[8];
int GenderDefault = -9;
int PluralityDefault = -9;

struct MonumentRecord {
    int year_;
    int word_4_;
    int word_8_;
    int owner_faction_;
    int achieved_;
    int word_14_;
    uint8_t opaque_[0x20];
};
struct MonumentFaction {
    MonumentRecord records_[21];
    int latest_type_;
    uint8_t opaque_tail_prefix_[0x10];
    char founding_base_name_[24];
    char facility_base_name_[24];
    char conquered_base_name_[24];
};
typedef char record_must_be_0x38[(sizeof(MonumentRecord) == 0x38) ? 1 : -1];
typedef char faction_must_be_0x4f4[(sizeof(MonumentFaction) == 0x4f4) ? 1 : -1];
MonumentFaction FactionMonuments[8];

char StringTemp[512];
char append_log[64][48];
int append_count = 0;
char parser_text[16][48];
int parser_slot[16];
int parser_gender[16];
int parser_plural[16];
int parser_count = 0;

static void append(const char *text) {
    assert(append_count < 64);
    std::strncpy(append_log[append_count++], text, 47);
    std::strcat(StringTemp, text);
}
char *get_title(int faction) {
    static char values[8][24];
    std::sprintf(values[faction], "title%d", faction);
    return values[faction];
}
char *get_name(int faction) {
    static char values[8][24];
    std::sprintf(values[faction], "name%d", faction);
    return values[faction];
}
int parse_says(int slot, char *text, int, int) {
    assert(parser_count < 16);
    parser_slot[parser_count] = slot;
    std::strncpy(parser_text[parser_count], text, 47);
    parser_gender[parser_count] = GenderDefault;
    parser_plural[parser_count] = PluralityDefault;
    ++parser_count;
    return 1;
}
void says(char *text) { append(text); }
void space() { append(" "); }
void say_label(int label) {
    assert(label == 0xB4);
    append("<label>");
}

struct Picture {
    int init(char *, int, int) { return 0; }
};
struct Popup {
    Picture picture;
    int starts;
    int selections;
    int selected;
    int item_count;
    int item_ids[8];
    char item_text[8][512];
    Popup() : starts(0), selections(0), selected(-1), item_count(0) {}
    void start(const char *, int) { ++starts; }
    Picture &owned_picture() { return picture; }
    void select_owned_picture() {}
    void item(char *text, int id) {
        assert(item_count < 8);
        item_ids[item_count] = id;
        std::strncpy(item_text[item_count], text, 511);
        ++item_count;
    }
    int exec() { ++selections; return selected; }
};
struct Buffer {
    static int get_pcx_dimensions(char *, int *width, int *height) {
        *width = 1; *height = 1; return 1;
    }
};

struct Console {
    int faction_id_;
    int updates;
    Console() : faction_id_(3), updates(0) {}
    void update_data(int) { ++updates; }
} ConsoleGlobal;
struct Vehicle { int home_base_id; } Vehs[8];
struct PlayerData {
    int unk_108;
    int diplo_friction[8];
    int diplo_patience[8];
    int diplo_spoke[8];
    int global_reputation;
} PlayersData[8];
struct StateFields { uint32_t game_rules_; };
struct GameState { StateFields fields_; } GameDataStateStorage;
const int MaxTechnologyNum = 8;
int ProbeNativeUnknown7AD330 = 1;
int veh_kill_calls = 0;
int setup_calls = 0;
int net_calls = 0;
int diplomacy_calls = 0;
int pop_calls = 0;

void probe_native_set_monument_selection_lock_bit(int) {}
void veh_kill(int) { ++veh_kill_calls; }
int setup_player(int, int, int) { ++setup_calls; return 0; }
int has_tech(int, int) { return 0; }
void net_tech(int, int, int, int) { ++net_calls; }
void net_treaty_off(int, int, int, int) { ++net_calls; }
void net_treaty_on(int, int, int, int) { ++net_calls; }
void net_set_treaty(int, int, int, int, int) { ++net_calls; }
void diplomacy_check(int, int, int) { ++diplomacy_calls; }
struct NetMsg { void pop(const char *, const char *) { ++pop_calls; } } NetMsgPrimary;

'''


MAIN = r'''int main() {
    std::memset(FactionMonuments, 0, sizeof(FactionMonuments));
    std::memset(PlayersData, 0, sizeof(PlayersData));
    for (int f = 0; f < 8; ++f) {
        Players[f].noun_gender = 20 + f;
        Players[f].is_noun_plural = f & 1;
        std::sprintf(Players[f].noun_faction, "noun%d", f);
        std::sprintf(Players[f].adj_name_faction, "adj%d", f);
    }

    MonumentFaction &faction = FactionMonuments[2];
    for (int i = 0; i < 21; ++i) {
        MonumentRecord &r = faction.records_[i];
        r.year_ = (i + 1) & 7;
        r.word_4_ = 100 + i;
        r.word_8_ = (7 - i) & 7;
        r.owner_faction_ = 200 + i;
        r.achieved_ = 1;
        r.word_14_ = 300 + i;
        std::memset(r.opaque_, i, sizeof(r.opaque_));
    }
    // Deliberately different producer values and display years. Records 0..5
    // are tempting but are not this choice-list cohort.
    for (int i = 0; i < 6; ++i) {
        faction.records_[i].year_ = i;
        faction.records_[i].word_8_ = 7 - i;
        faction.records_[i].achieved_ = 1;
    }
    faction.records_[15].year_ = 1981; faction.records_[15].word_8_ = 2; faction.records_[15].achieved_ = 1;
    faction.records_[16].year_ = 1982; faction.records_[16].word_8_ = 3; faction.records_[16].achieved_ = 0;
    faction.records_[17].year_ = 1983; faction.records_[17].word_8_ = 3; faction.records_[17].achieved_ = 1;
    faction.records_[18].year_ = 1984; faction.records_[18].word_8_ = 2; faction.records_[18].achieved_ = 2;
    faction.records_[19].year_ = 1985; faction.records_[19].word_8_ = 0; faction.records_[19].achieved_ = 1;
    faction.records_[20].year_ = 1986; faction.records_[20].word_8_ = 5; faction.records_[20].achieved_ = 1;
    MonumentRecord before[21];
    std::memcpy(before, faction.records_, sizeof(before));

    Popup popup;
    popup.selected = -7;
    const int list_result = probe_native_monument_choice_list(popup, faction, 0);
    assert(list_result == -7 && popup.starts == 1 && popup.selections == 1);
    assert(popup.item_count == 5);
    const int expected_ids[5] = {2, 3, 2, 0, 5};
    const char *expected_text[5] = {
        "title2 name2<label> noun2", "title3 name3<label> noun3",
        "title2 name2<label> noun2", "title0 name0<label> noun0",
        "title5 name5<label> noun5"};
    for (int i = 0; i < 5; ++i) {
        assert(popup.item_ids[i] == expected_ids[i]);
        assert(std::strcmp(popup.item_text[i], expected_text[i]) == 0);
    }
    assert(append_count == 30);
    // Each noun producer must run after the label/space and before its item.
    for (int i = 0; i < 5; ++i) {
        const int noun_call = i * 6 + 5;
        assert(std::strcmp(append_log[noun_call], expected_ids[i] == 0 ? "noun0" :
            expected_ids[i] == 2 ? "noun2" : expected_ids[i] == 3 ? "noun3" : "noun5") == 0);
    }
    assert(std::memcmp(before, faction.records_, sizeof(before)) == 0);

    // A negative selection returns before teardown/state mutations.
    Popup negative_popup;
    const int negative = probe_native_commit_monument_choice(
        negative_popup, 1, 2, 0, -1, 0, 0);
    assert(negative == -1 && setup_calls == 0 && net_calls == 0);
    assert(diplomacy_calls == 0 && pop_calls == 0);
    assert(std::memcmp(before, faction.records_, sizeof(before)) == 0);

    // The initial selected-faction slot remains the native adjective path;
    // a canceled list still returns before commit/accounting effects.
    Popup canceled_list_popup;
    const int canceled_list = probe_native_commit_monument_choice(
        canceled_list_popup, 3, 2, 0, 2, 8u, 0);
    assert(canceled_list == -1 && canceled_list_popup.item_count == 5);
    assert(parser_count == 1 && parser_slot[0] == 1);
    assert(std::strcmp(parser_text[0], "adj2") == 0);
    assert(setup_calls == 0 && net_calls == 0 && diplomacy_calls == 0);
    parser_count = 0;

    // Positive selection emits title, name, noun in original parser-slot
    // order. The real extracted get_noun body sets defaults before slot 2.
    Vehs[0].home_base_id = 0;
    GameDataStateStorage.fields_.game_rules_ = 0x02000000u;
    const int selected = probe_native_commit_monument_choice(
        negative_popup, 1, 2, 0, 3, 0, 0);
    assert(selected == 3 && parser_count == 3);
    assert(parser_slot[0] == 0 && std::strcmp(parser_text[0], "title3") == 0);
    assert(parser_slot[1] == 1 && std::strcmp(parser_text[1], "name3") == 0);
    assert(parser_slot[2] == 2 && std::strcmp(parser_text[2], "noun3") == 0);
    assert(parser_gender[2] == Players[3].noun_gender);
    assert(parser_plural[2] == Players[3].is_noun_plural);
    assert(faction.records_[15].achieved_ == 1); // counterpart 2, not selected 3
    assert(faction.records_[16].achieved_ == 0); // unachieved counterpart 3
    assert(faction.records_[17].achieved_ == 0); // duplicate counterpart 3 cleared
    assert(faction.records_[18].achieved_ == 2); // different counterpart 2 retained
    assert(faction.records_[19].achieved_ == 1); // counterpart zero, not selected
    assert(faction.records_[20].achieved_ == 1);
    MonumentRecord after_selected_three[21];
    std::memcpy(after_selected_three, faction.records_, sizeof(after_selected_three));
    // Zero is a valid counterpart ID. The native gate is achieved_, not a
    // truthiness test on word_8_; duplicate matching entries are handled too.
    probe_native_clear_monument_choice_word(faction, 0);
    assert(faction.records_[19].achieved_ == 0);
    for (int i = 0; i < 15; ++i) {
        assert(std::memcmp(&faction.records_[i], &before[i], sizeof(MonumentRecord)) == 0);
    }
    for (int i = 15; i < 21; ++i) {
        const MonumentRecord &old = after_selected_three[i];
        assert(faction.records_[i].year_ == old.year_);
        assert(faction.records_[i].word_8_ == old.word_8_);
        assert(faction.records_[i].word_4_ == old.word_4_);
        assert(faction.records_[i].owner_faction_ == old.owner_faction_);
        assert(faction.records_[i].word_14_ == old.word_14_);
        assert(std::memcmp(faction.records_[i].opaque_, old.opaque_, 0x20) == 0);
        if (i != 19) assert(faction.records_[i].achieved_ == old.achieved_);
    }
    return 0;
}
'''


def _program(functions: dict[str, str]) -> str:
    return HOST + "\n".join(functions[key] for key in
                             ("noun", "adjective", "clear", "list", "commit")) + MAIN


def _compile_run(tmp_path: Path, source: str, compiler: str, stem: str) -> subprocess.CompletedProcess:
    cpp = tmp_path / (stem + ".cpp")
    exe = tmp_path / stem
    cpp.write_text(source, encoding="utf-8")
    built = subprocess.run(
        [compiler, "-std=c++98", "-Wall", "-Wextra", "-Werror", str(cpp), "-o", str(exe)],
        capture_output=True, text=True, timeout=30,
    )
    assert built.returncode == 0, built.stdout + built.stderr
    return subprocess.run([str(exe)], capture_output=True, text=True, timeout=10)


def test_current_probe_helpers_are_extracted_from_active_source():
    functions = _production_functions()
    assert "record = 15" in functions["list"] and "record < 21" in functions["list"]
    assert "entry.achieved_ == 0" in functions["list"]
    assert "get_title(entry.word_8_)" in functions["list"]
    assert "get_name(entry.word_8_)" in functions["list"]
    assert "get_noun(entry.word_8_)" in functions["list"]
    assert "popup.item(StringTemp, entry.word_8_)" in functions["list"]
    assert "entry.word_8_ == selected_faction_id" in functions["clear"]
    assert "entry.achieved_ = 0" in functions["clear"]
    assert "parse_says(2, get_noun(selected_faction_id), -1, -1)" in functions["commit"]


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
def test_actual_probe_choice_helpers_and_compiling_damage_controls(tmp_path: Path,
                                                                   compiler_name: str):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(compiler_name + " unavailable")
    functions = _production_functions()
    result = _compile_run(tmp_path, _program(functions), compiler, "probe_choice_actual")
    assert result.returncode == 0, result.stdout + result.stderr

    mutations = (
        ("list", "for (int record = 15; record < 21; ++record)",
         "for (int record = 0; record < 6; ++record)"),
        ("list", "if (entry.achieved_ == 0) continue;",
         "if (entry.word_8_ == 0) continue;"),
        ("list", "says(get_title(entry.word_8_));",
         "says(get_title(entry.year_));"),
        ("list", "says(get_noun(entry.word_8_));",
         "says(get_adjective(entry.word_8_));"),
        ("list", "popup.item(StringTemp, entry.word_8_);",
         "popup.item(StringTemp, entry.year_);"),
        ("clear", "for (int record = 15; record < 21; ++record)",
         "for (int record = 0; record < 6; ++record)"),
        ("clear", "entry.word_8_ == selected_faction_id",
         "entry.year_ == selected_faction_id"),
        ("clear", "entry.achieved_ = 0;", "entry.word_8_ = 0;"),
        ("clear", "entry.achieved_ != 0", "entry.word_8_ != 0"),
        ("commit", "parse_says(2, get_noun(selected_faction_id), -1, -1);",
         "parse_says(2, get_adjective(selected_faction_id), -1, -1);"),
    )
    for index, (owner, old, new) in enumerate(mutations):
        changed = dict(functions)
        assert changed[owner].count(old) == 1, (owner, old)
        changed[owner] = changed[owner].replace(old, new, 1)
        mutant = _compile_run(tmp_path, _program(changed), compiler,
                              "probe_choice_mutant_%d" % index)
        assert mutant.returncode != 0, "compiled mutant escaped: %s / %s" % (owner, old)
