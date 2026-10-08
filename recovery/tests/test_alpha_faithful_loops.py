"""Bounded host controls over exact alpha.cpp loader loops; not parser/game proof."""

from pathlib import Path
import re
import shutil
import subprocess

import pytest


def root_dir():
    for parent in Path(__file__).resolve().parents:
        if (parent / "src/alpha.cpp").exists():
            return parent
    raise AssertionError("repository root not found")


ROOT = root_dir()


def uncomment(text):
    return re.sub(r"/\*.*?\*/|//[^\n]*", "", text, flags=re.S)


def braced_from(text, start):
    opening = text.index("{", start)
    depth = 0
    for end in range(opening, len(text)):
        depth += (text[end] == "{") - (text[end] == "}")
        if depth == 0:
            return text[start:end + 1]
    raise AssertionError("unterminated C++ declaration or block")


def declaration(path, name):
    text = uncomment(path.read_text())
    start = text.index(name)
    return braced_from(text, start) + ";"


def integer_constant(path, name):
    text = uncomment(path.read_text())
    match = re.search(
        rf"static const int {re.escape(name)}\s*=\s*(\d+)\s*;", text
    )
    assert match, name
    return int(match.group(1))


def loop_after(text, anchor, loop_prefix):
    clean = uncomment(text)
    anchor_at = clean.index(anchor)
    start = clean.index(loop_prefix, anchor_at)
    return braced_from(clean, start)


ALPHA = ROOT / "src/alpha.cpp"
FACTION_H = ROOT / "src/faction.h"
VEH_H = ROOT / "src/veh.h"
BASE_H = ROOT / "src/base.h"


def product_loops():
    alpha = ALPHA.read_text()
    faction_source = alpha[
        alpha.index("void __cdecl read_faction(Player *player, int toggle)"):
        alpha.index("/*\nPurpose: Parse the #BONUSNAMES")
    ]
    ideology = loop_after(
        faction_source,
        "void __cdecl read_faction",
        "for (int i = 0; i < 2; i++)",
    )
    # The product ABI is 32-bit. Keep its pointer-to-int key narrowing explicit
    # through the host bridge because this control compiles on a 64-bit host.
    ideology = ideology.replace(
        "(int)SocialCategories[j].type",
        "pointer_key(SocialCategories[j].type)",
    ).replace(
        "(int)SocialCategories[soc_cat_num].name[j]",
        "pointer_key(SocialCategories[soc_cat_num].name[j])",
    )
    reactor = loop_after(
        alpha,
        'if (text_open(AlphaxFileID, "REACTORS"))',
        "for (i = 0; i < MaxReactorNum; i++)",
    )
    facility = loop_after(
        alpha,
        'if (text_open(AlphaxFileID, "FACILITIES"))',
        "for (i = 1; i < MaxFacilityNum; i++)",
    )
    return ideology, reactor, facility


HARNESS = r"""
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <cctype>
#include <cstdio>
#include <string>
#include <vector>
#define __cdecl
typedef int BOOL;
typedef char *LPSTR;

void check(bool ok) { if (!ok) std::exit(91); }
int _stricmp(const char *a, const char *b) {
    while (*a && *b) {
        unsigned char x = static_cast<unsigned char>(*a++);
        unsigned char y = static_cast<unsigned char>(*b++);
        if (std::tolower(x) != std::tolower(y))
            return std::tolower(x) - std::tolower(y);
    }
    return std::tolower(static_cast<unsigned char>(*a))
         - std::tolower(static_cast<unsigned char>(*b));
}
int _strnicmp(const char *a, const char *b, size_t n) {
    while (n-- && *a && *b) {
        unsigned char x = static_cast<unsigned char>(*a++);
        unsigned char y = static_cast<unsigned char>(*b++);
        if (std::tolower(x) != std::tolower(y))
            return std::tolower(x) - std::tolower(y);
    }
    return n == static_cast<size_t>(-1) ? 0
        : std::tolower(static_cast<unsigned char>(*a))
        - std::tolower(static_cast<unsigned char>(*b));
}

DECL_BLOCK_PLAYER_GENDER
DECL_BLOCK_SOCIAL_EFFECT
DECL_BLOCK_RULES_SOCIAL_EFFECT
DECL_BLOCK_RULES_SOCIAL_CATEGORY
DECL_BLOCK_PLAYER
DECL_BLOCK_RULES_REACTOR
DECL_BLOCK_RULES_FACILITY
const int MaxSocialCatNum = MAX_SOCIAL_CATEGORY;
const int MaxSocialModelNum = MAX_SOCIAL_MODEL;
const int MaxSocialEffectNum = MAX_SOCIAL_EFFECT;
const int MaxReactorNum = MAX_REACTOR;
const int MaxFacilityNum = MAX_FACILITY;
const int FacilitySPStart = FACILITY_SP_START;

RulesSocialCategory SocialCategories[MaxSocialCatNum];
RulesSocialEffect SocialEffects[MaxSocialEffectNum];
RulesReactor Reactor[MaxReactorNum];
RulesFacility Facility[MaxFacilityNum];
std::string category_text[MaxSocialCatNum];
std::string model_text[MaxSocialCatNum][MaxSocialModelNum];

int pointer_key(char *p) {
    return static_cast<int>(reinterpret_cast<intptr_t>(p));
}
struct StringTableDouble {
    int calls;
    StringTableDouble() : calls(0) {}
    char *get(int key) {
        ++calls;
        for (int c = 0; c < MaxSocialCatNum; ++c) {
            if (key == pointer_key(SocialCategories[c].type))
                return const_cast<char *>(category_text[c].c_str());
            for (int m = 0; m < MaxSocialModelNum; ++m)
                if (key == pointer_key(SocialCategories[c].name[m]))
                    return const_cast<char *>(model_text[c][m].c_str());
        }
        check(false);
        return 0;
    }
} StringTable;
int Language = 0;
int j;

std::vector<std::vector<std::string> > rows;
int row_index = -1;
size_t item_index = 0;
int text_get_calls, text_item_calls, number_calls, tech_name_calls;
int facility_effect_string_calls;
bool observe_facility_effect;
std::vector<std::string> tech_inputs;
void reset_parser(const std::vector<std::vector<std::string> > &input) {
    rows = input;
    row_index = -1;
    item_index = 0;
    text_get_calls = text_item_calls = number_calls = tech_name_calls = 0;
    facility_effect_string_calls = 0;
    tech_inputs.clear();
}
void text_get() {
    ++text_get_calls;
    ++row_index;
    item_index = 0;
    check(row_index < static_cast<int>(rows.size()));
}
LPSTR text_item() {
    ++text_item_calls;
    check(row_index >= 0 && item_index < rows[row_index].size());
    return const_cast<char *>(rows[row_index][item_index++].c_str());
}
LPSTR text_item_string() {
    LPSTR value = text_item();
    if (observe_facility_effect && std::strncmp(value, "effect-text", 11) == 0) {
        const int facility_id = row_index + 1;
        check(facility_id > 0 && facility_id < MaxFacilityNum);
        check(Facility[facility_id].free_tech == 2000 + facility_id);
        ++facility_effect_string_calls;
    }
    return value;
}
int text_item_number() {
    ++number_calls;
    return std::atoi(text_item());
}
int tech_name(LPSTR text) {
    ++tech_name_calls;
    tech_inputs.push_back(text);
    return std::atoi(text);
}

void init_social_tables() {
    for (int c = 0; c < MaxSocialCatNum; ++c) {
        category_text[c] = "category" + std::to_string(c);
        SocialCategories[c].type = const_cast<char *>(category_text[c].c_str());
        for (int m = 0; m < MaxSocialModelNum; ++m) {
            model_text[c][m] = "model" + std::to_string(c) + "_" + std::to_string(m);
            SocialCategories[c].name[m] =
                const_cast<char *>(model_text[c][m].c_str());
        }
    }
    for (int e = 0; e < MaxSocialEffectNum; ++e)
        std::sprintf(SocialEffects[e].set1, "effect%d", e);
}

void check_ideology_loop() {
    init_social_tables();
    Player player = {};
    std::vector<std::vector<std::string> > input;
    for (int i = 0; i < 2; ++i) {
        std::vector<std::string> row;
        row.push_back(i == 0 ? "category0" : "category1");
        row.push_back(i == 0 ? "model0_0" : "model1_1");
        row.push_back("effect0");
        input.push_back(row);
    }
    reset_parser(input);
    StringTable.calls = 0;
    run_ideology_loop(&player);
    check(text_get_calls == 2 && text_item_calls == 6);
    check(StringTable.calls == 6);
    check(player.soc_ideology_category == 0);
    check(player.soc_anti_ideology_category == 1);
    check(player.soc_ideology_model == 0);
    check(player.soc_anti_ideology_model == 1);
    check(player.soc_ideology_effect == -1);
    check(player.soc_anti_ideology_effect == -1);
}

void check_reactor_loop() {
    std::vector<std::vector<std::string> > input;
    for (int i = 0; i < MaxReactorNum; ++i) {
        std::vector<std::string> row;
        row.push_back("reactor" + std::to_string(i));
        row.push_back("short" + std::to_string(i));
        row.push_back("777");
        row.push_back(std::to_string(40 + i));
        input.push_back(row);
        Reactor[i].power = 0xA55A; // Existing loader state; not an uninitialized-data claim.
        Reactor[i].preq_tech = -1;
    }
    reset_parser(input);
    run_reactor_loop();
    check(text_get_calls == MaxReactorNum);
    check(text_item_calls == MaxReactorNum * 4);
    check(number_calls == 0);
    check(tech_name_calls == MaxReactorNum);
    for (int i = 0; i < MaxReactorNum; ++i) {
        check(std::strcmp(Reactor[i].name, ("reactor" + std::to_string(i)).c_str()) == 0);
        check(std::strcmp(Reactor[i].name_short, ("short" + std::to_string(i)).c_str()) == 0);
        check(Reactor[i].preq_tech == 40 + i);
        check(Reactor[i].power == 0xA55A);
    }
}

void check_facility_loop() {
    std::vector<std::vector<std::string> > input;
    for (int i = 1; i < MaxFacilityNum; ++i) {
        std::vector<std::string> row;
        row.push_back("facility" + std::to_string(i));
        row.push_back(std::to_string(i + 1));
        row.push_back(std::to_string(i + 2));
        row.push_back(std::to_string(1000 + i));
        row.push_back(std::to_string(2000 + i));
        row.push_back("effect-text" + std::to_string(i));
        if (i >= FacilitySPStart)
            for (int k = 0; k < 5; ++k)
                row.push_back(std::to_string(3000 + i * 5 + k));
        input.push_back(row);
        Facility[i].free_tech = 0x12345678;
    }
    reset_parser(input);
    observe_facility_effect = true;
    run_facility_loop();
    observe_facility_effect = false;
    const int record_count = MaxFacilityNum - 1;
    const int special_count = MaxFacilityNum - FacilitySPStart;
    check(text_get_calls == record_count);
    check(text_item_calls == record_count * 6 + special_count * 5);
    check(number_calls == record_count * 2 + special_count * 5);
    check(tech_name_calls == record_count * 2);
    check(facility_effect_string_calls == record_count);
    for (int i = 1; i < MaxFacilityNum; ++i) {
        check(Facility[i].cost == i + 1);
        check(Facility[i].maint == static_cast<uint32_t>(i + 2));
        check(Facility[i].preq_tech == 1000 + i);
        check(Facility[i].free_tech == -2);
    }
    check(tech_inputs.size() == static_cast<size_t>(record_count * 2));
    check(tech_inputs[0] == "1001" && tech_inputs[1] == "2001");
}

void run_ideology_loop(Player *player) {
    IDEOLOGY_LOOP
}
void run_reactor_loop() {
    int i = 0;
    REACTOR_LOOP
}
void run_facility_loop() {
    int i = 0;
    FACILITY_LOOP
}
int main() {
    check_ideology_loop();
    check_reactor_loop();
    check_facility_loop();
    return 0;
}
"""


def harness_text(mutations=()):
    ideology, reactor, facility = product_loops()
    for field in ("category", "model", "effect"):
        if "alias_anti_" + field in mutations:
            old = "player->soc_anti_ideology_" + field
            assert ideology.count(old) == 1
            ideology = ideology.replace(old, "player->soc_ideology_" + field, 1)
    if "current_effect_index" in mutations:
        old = "soc_effect_value = -1;"
        assert ideology.count(old) == 2
        pos = ideology.rfind(old)
        ideology = ideology[:pos] + ideology[pos:].replace(old, "soc_effect_value = j;", 1)
    if "store_reactor_power" in mutations:
        old = "text_item();"
        assert reactor.count(old) == 1
        reactor = reactor.replace(old, "Reactor[i].power = (uint16_t)text_item_number();", 1)
    if "retain_facility_free_tech" in mutations:
        old = "Facility[i].free_tech = -2;"
        assert facility.count(old) == 1
        facility = facility.replace(old, "(void)0;", 1)
    if "overwrite_facility_before_effect" in mutations:
        old = "Facility[i].effect = text_item_string();"
        assert facility.count(old) == 1
        facility = facility.replace(
            old,
            "Facility[i].free_tech = -2;\n        " + old,
            1,
        )

    declarations = {
        "DECL_BLOCK_PLAYER_GENDER": declaration(FACTION_H, "enum PlayerGender"),
        "DECL_BLOCK_SOCIAL_EFFECT": declaration(FACTION_H, "struct SocialEffect"),
        "DECL_BLOCK_RULES_SOCIAL_EFFECT": declaration(FACTION_H, "struct RulesSocialEffect"),
        "DECL_BLOCK_RULES_SOCIAL_CATEGORY": declaration(FACTION_H, "struct RulesSocialCategory"),
        "DECL_BLOCK_PLAYER": declaration(FACTION_H, "struct Player"),
        "DECL_BLOCK_RULES_REACTOR": declaration(VEH_H, "struct RulesReactor"),
        "DECL_BLOCK_RULES_FACILITY": declaration(BASE_H, "struct RulesFacility"),
        "MAX_SOCIAL_CATEGORY": str(integer_constant(FACTION_H, "MaxSocialCatNum")),
        "MAX_SOCIAL_MODEL": str(integer_constant(FACTION_H, "MaxSocialModelNum")),
        "MAX_SOCIAL_EFFECT": str(integer_constant(FACTION_H, "MaxSocialEffectNum")),
        "MAX_REACTOR": str(integer_constant(VEH_H, "MaxReactorNum")),
        "MAX_FACILITY": str(integer_constant(BASE_H, "MaxFacilityNum")),
        "FACILITY_SP_START": str(integer_constant(BASE_H, "FacilitySPStart")),
        "IDEOLOGY_LOOP": ideology,
        "REACTOR_LOOP": reactor,
        "FACILITY_LOOP": facility,
    }
    result = HARNESS
    for key, value in declarations.items():
        result = result.replace(key, value)
    result = result.replace(
        "void check_ideology_loop() {",
        "void run_ideology_loop(Player *player);\nvoid check_ideology_loop() {",
    )
    result = result.replace("void check_reactor_loop() {", "void run_reactor_loop();\nvoid check_reactor_loop() {")
    result = result.replace("void check_facility_loop() {", "void run_facility_loop();\nvoid check_facility_loop() {")
    return result


def run_host(tmp_path, mutations=()):
    compiler = shutil.which("g++")
    if compiler is None:
        pytest.skip("host g++ required")
    source = tmp_path / "alpha_faithful_loops.cpp"
    executable = tmp_path / "alpha_faithful_loops"
    source.write_text(harness_text(mutations))
    compiled = subprocess.run(
        [compiler, "-std=c++11", "-O1", "-Wall", "-Wextra", "-Werror", "-fsanitize=undefined",
         "-fno-sanitize-recover=undefined", str(source), "-o", str(executable)],
        capture_output=True, text=True, timeout=30,
    )
    assert compiled.returncode == 0, compiled.stderr
    return subprocess.run([str(executable)], capture_output=True, text=True, timeout=10)


def test_actual_alpha_loops_preserve_pinned_effect_and_rule_storage(tmp_path):
    result = run_host(tmp_path)
    assert result.returncode == 0, result.stderr


@pytest.mark.parametrize("mutation", [
    "current_effect_index",
    "store_reactor_power",
    "retain_facility_free_tech",
    "overwrite_facility_before_effect",
    "alias_anti_category",
    "alias_anti_model",
    "alias_anti_effect",
])
def test_original_improvement_mutants_are_rejected(tmp_path, mutation):
    result = run_host(tmp_path, (mutation,))
    assert result.returncode == 91, result.stderr
