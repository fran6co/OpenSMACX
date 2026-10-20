"""Execute the actual part-name formatter bodies with a bounded host double.

These checks cover preserved table lookup, callback order, punctuation,
StringTemp side effects and the original signed-rating/unsigned-mode branches.
They are not native compiler, ABI, image-layout or acceptance evidence.
"""
from __future__ import annotations

from pathlib import Path
import shutil
import subprocess
import tempfile

import pytest

from match_recovery.source import read_text


ROOT = Path(__file__).resolve().parents[2]
SOURCE = ROOT / "src/string_formatting.cpp"
TARGETS = (
    (0x00590510, "void __cdecl say_reactor(char *text, int reactor_id, int with_detail)"),
    (0x005905C0, "void __cdecl say_weapon(char *text, int weapon_id, int with_detail)"),
    (0x005906F0, "void __cdecl say_armor(char *text, int armor_id, int with_detail)"),
    (0x00590800, "void __cdecl say_chassis(char *text, int chassis_id, int with_detail)"),
    (0x00590AA0, "void __cdecl say_ability(char *text, int ability_id, int with_detail)"),
)


def extract_body(source: str, signature: str) -> str:
    assert source.count(signature) == 1, signature
    start = source.index(signature)
    opening = source.index("{", start + len(signature))
    depth = 0
    state = "code"
    index = opening
    while index < len(source):
        char = source[index]
        nxt = source[index + 1] if index + 1 < len(source) else ""
        if state == "code":
            if char == "/" and nxt == "*":
                state = "block"
                index += 2
                continue
            if char == "/" and nxt == "/":
                state = "line"
                index += 2
                continue
            if char == '"':
                state = "string"
            elif char == "'":
                state = "char"
            elif char == "{":
                depth += 1
            elif char == "}":
                depth -= 1
                if depth == 0:
                    return source[start:index + 1]
        elif state == "block":
            if char == "*" and nxt == "/":
                state = "code"
                index += 2
                continue
        elif state == "line":
            if char == "\n":
                state = "code"
        else:
            if char == "\\":
                index += 2
                continue
            if (state == "string" and char == '"') or (state == "char" and char == "'"):
                state = "code"
        index += 1
    raise AssertionError(f"unterminated actual body: {signature}")


def installed_source() -> str:
    source = SOURCE.read_text()
    rows = {row.address: row for row in read_text(source, SOURCE)}
    assert {address for address, _ in TARGETS}.issubset(rows)
    assert all(rows[address].source_role.value != "scaffold" for address, _ in TARGETS)
    return source


PRELUDE = r'''
#include <cassert>
#include <cstdio>
#include <cstring>
#include <cstdint>
#include <string>
#include <vector>
#define __cdecl
using LPSTR = char *;
using BOOL = int;
static std::vector<std::string> events;
struct Strings {
  char *get(char *value) {
    events.push_back(std::string("get:") + value);
    return value;
  }
};
struct Label { char **strings_ptr; };
// Relevant fields retain the production types' names and member types from
// veh.h; unrelated table fields are intentionally not part of this host double.
struct RulesReactor { LPSTR name; LPSTR name_short; int16_t preq_tech; uint16_t power; };
struct RulesWeapon { LPSTR name; LPSTR name_short; int8_t offense_rating; int8_t icon; uint8_t mode; uint8_t cost; int16_t preq_tech; int16_t padding; };
struct RulesArmor { LPSTR name; LPSTR name_short; int8_t defense_rating; uint8_t mode; uint8_t cost; uint8_t padding1; int16_t preq_tech; int16_t padding2; };
struct RulesChassis { LPSTR offsv_1_name; LPSTR offsv_2_name; LPSTR offsv_lrg_name; LPSTR defsv_1_name; LPSTR defsv_2_name; LPSTR defsv_lrg_name; uint32_t offsv_1_gender; uint32_t offsv_2_gender; uint32_t offsv_lrg_gender; uint32_t defsv_1_gender; uint32_t defsv_2_gender; uint32_t defsv_lrg_gender; BOOL offsv_1_plural; BOOL offsv_2_plural; BOOL offsv_lrg_plural; BOOL defsv_1_plural; BOOL defsv_2_plural; BOOL defsv_lrg_plural; uint8_t speed; uint8_t triad; };
struct RulesAbility { LPSTR name; LPSTR description; LPSTR abbreviation; int cost_factor; int unk_1; uint32_t flags; int16_t preq_tech; uint16_t padding; };
static char *labels[256];
static Label Labels = { labels };
static RulesReactor Reactor[8];
static RulesWeapon Weapon[8];
static RulesArmor Armor[8];
static RulesChassis Chassis[8];
static RulesAbility Ability[8];
static Strings StringTable;
static char StringTemp[256];
static char *label_get(int label_offset) {
  return StringTable.get(Labels.strings_ptr[label_offset]);
}
static char * _itoa(int value, char *buffer, int radix) {
  assert(radix == 10);
  events.push_back(std::string("itoa:") + std::to_string(value));
  std::snprintf(buffer, 80, "%d", value);
  return buffer;
}
'''


MAIN = r'''
int main() {
  static char reactor_name[] = "Reactor";
  static char reactor_detail[] = "reactor-detail";
  static char weapon_name[] = "Weapon";
  static char weapon_zero[] = "unarmed";
  static char weapon_rating[] = "offense";
  static char negative[] = "negative";
  static char armor_name[] = "Armor";
  static char armor_detail[] = "armor-detail";
  static char chassis_name[] = "Chassis";
  static char chassis_detail[] = "chassis-detail";
  static char triad_one[] = "triad-one";
  static char triad_two[] = "triad-two";
  static char ability_name[] = "Ability";
  static char ability_detail[] = "ability-detail";
  Reactor[1].name = reactor_name;
  Weapon[2] = {weapon_name, 0, 3, 0, 2, 0, 0, 0};
  Weapon[3] = {weapon_name, 0, 0, 0, 2, 0, 0, 0};
  Weapon[4] = {weapon_name, 0, -2, 0, 2, 0, 0, 0};
  Armor[1] = {armor_name, 0, -1, 2, 0, 0, 0, 0};
  Chassis[1].offsv_1_name = chassis_name; Chassis[1].speed = 5; Chassis[1].triad = 1;
  Chassis[2].offsv_1_name = chassis_name; Chassis[2].speed = 6; Chassis[2].triad = 2;
  Chassis[3].offsv_1_name = chassis_name; Chassis[3].speed = 7; Chassis[3].triad = 3;
  Ability[1].name = ability_name;
  for (char *&label : labels) label = negative;
  labels[0x13C / 4] = reactor_detail;
  labels[0x284 / 4] = weapon_zero;
  labels[0x204 / 4] = weapon_rating;
  labels[0x310 / 4] = negative;
  labels[0x154 / 4] = armor_detail;
  labels[0x140 / 4] = chassis_detail;
  labels[0x28C / 4] = triad_one;
  labels[0x288 / 4] = triad_two;
  labels[0xA4] = ability_detail;

  char text[256];
  std::strcpy(text, "prefix"); std::strcpy(StringTemp, "seed"); events.clear();
  say_reactor(text, 1, -1);
  assert(std::strcmp(text, "prefixReactor (: 2)") == 0);
  assert(std::strcmp(StringTemp, "seedreactor-detail") == 0);
  assert((events == std::vector<std::string>{"get:Reactor", "get:reactor-detail", "itoa:2"}));

  std::strcpy(text, ""); std::strcpy(StringTemp, "unchanged"); events.clear();
  say_reactor(text, 1, 0);
  assert(std::strcmp(text, "Reactor") == 0 && std::strcmp(StringTemp, "unchanged") == 0);
  assert((events == std::vector<std::string>{"get:Reactor"}));

  std::strcpy(text, ""); std::strcpy(StringTemp, "weapon-scratch"); events.clear();
  say_weapon(text, 2, 1);
  assert(std::strcmp(text, "Weapon (offense: 3**)") == 0);
  assert(std::strcmp(StringTemp, "weapon-scratch") == 0);
  assert((events == std::vector<std::string>{"get:Weapon", "get:offense", "itoa:3"}));

  std::strcpy(text, ""); events.clear();
  say_weapon(text, 3, 1);
  assert(std::strcmp(text, "Weapon (unarmed)") == 0);
  assert((events == std::vector<std::string>{"get:Weapon", "get:unarmed"}));

  std::strcpy(text, ""); events.clear();
  say_weapon(text, 4, 1);
  assert(std::strcmp(text, "Weapon (offense: negative**)") == 0);
  assert((events == std::vector<std::string>{"get:Weapon", "get:offense", "get:negative"}));

  std::strcpy(text, ""); std::strcpy(StringTemp, "seed"); events.clear();
  say_armor(text, 1, 1);
  assert(std::strcmp(text, "Armor (: negative**)") == 0);
  assert(std::strcmp(StringTemp, "seedarmor-detail") == 0);
  assert((events == std::vector<std::string>{"get:Armor", "get:armor-detail", "get:negative"}));

  std::strcpy(text, ""); std::strcpy(StringTemp, "seed"); events.clear();
  say_chassis(text, 1, 1);
  assert(std::strcmp(text, "Chassis (: 5 triad-one)") == 0);
  assert(std::strcmp(StringTemp, "seedchassis-detail") == 0);
  assert((events == std::vector<std::string>{"get:Chassis", "get:chassis-detail", "itoa:5", "get:triad-one"}));

  std::strcpy(text, ""); events.clear();
  say_chassis(text, 2, 1);
  assert(std::strcmp(text, "Chassis (: 6 triad-two)") == 0);
  assert(events.back() == "get:triad-two");
  std::strcpy(text, ""); events.clear();
  say_chassis(text, 3, 1);
  assert(std::strcmp(text, "Chassis (: 7)") == 0);
  assert((events == std::vector<std::string>{"get:Chassis", "get:chassis-detail", "itoa:7"}));

  std::strcpy(text, ""); std::strcpy(StringTemp, "seed"); events.clear();
  say_ability(text, 1, -4);
  assert(std::strcmp(text, "Ability ()") == 0);
  assert(std::strcmp(StringTemp, "seedability-detail") == 0);
  // label_get(0xA4) is StringTable.get(Labels.strings_ptr[0xA4]);
  // The expected event proves that table read/callback as well as the scratch append.
  assert((events == std::vector<std::string>{"get:Ability", "get:ability-detail"}));

  std::strcpy(text, "no-detail"); std::strcpy(StringTemp, "unchanged"); events.clear();
  say_ability(text, 1, 0);
  assert(std::strcmp(text, "no-detailAbility") == 0);
  assert(std::strcmp(StringTemp, "unchanged") == 0);
  assert((events == std::vector<std::string>{"get:Ability"}));
}
'''


def projected_program(source: str, mutation: str | None = None) -> str:
    bodies = {address: extract_body(source, signature) for address, signature in TARGETS}
    if mutation == "reactor_number_offset":
        bodies[0x00590510] = bodies[0x00590510].replace("reactor_id + 1", "reactor_id", 1)
    elif mutation == "clear_scratch_first":
        signature = TARGETS[0][1]
        bodies[0x00590510] = bodies[0x00590510].replace(signature + " {", signature + " {\n    StringTemp[0] = 0;", 1)
    elif mutation == "wrong_scratch_destination":
        bodies[0x005906F0] = bodies[0x005906F0].replace(
            "strcat(StringTemp, StringTable.get(", "strcat(text, StringTable.get(", 1)
    elif mutation == "zero_weapon_gets_stars":
        bodies[0x005905C0] = bodies[0x005905C0].replace(
            "if (weapon.offense_rating == 0)", "if (weapon.offense_rating != 0)", 1)
    elif mutation == "swap_chassis_triad_labels":
        bodies[0x00590800] = bodies[0x00590800].replace(
            "chassis.triad == 1 ? 0x28C : 0x288",
            "chassis.triad == 1 ? 0x288 : 0x28C", 1)
    elif mutation == "wrong_ability_label_index":
        bodies[0x00590AA0] = bodies[0x00590AA0].replace(
            "label_get(0xA4)", "label_get(0xA3)", 1)
    elif mutation is not None:
        raise AssertionError(f"unknown mutation: {mutation}")
    return PRELUDE + "\n".join(bodies[address] for address, _ in TARGETS) + MAIN


COMPILERS = [compiler for compiler in ("g++", "clang++") if shutil.which(compiler)]
pytestmark = pytest.mark.skipif(not COMPILERS, reason="no host C++ compiler available")


@pytest.mark.parametrize("compiler", COMPILERS)
def test_part_name_formatters_and_original_behavior_mutants(compiler: str) -> None:
    source = installed_source()
    cases = (
        (None, True),
        ("reactor_number_offset", False),
        ("clear_scratch_first", False),
        ("wrong_scratch_destination", False),
        ("zero_weapon_gets_stars", False),
        ("swap_chassis_triad_labels", False),
        ("wrong_ability_label_index", False),
    )
    for mutation, should_pass in cases:
        program = projected_program(source, mutation)
        with tempfile.TemporaryDirectory(prefix="string-formatting-owner-") as temp_name:
            binary = Path(temp_name) / "control"
            built = subprocess.run(
                [compiler, "-std=c++11", "-Wall", "-Werror", "-x", "c++", "-", "-o", str(binary)],
                input=program, text=True, capture_output=True, timeout=30,
            )
            assert built.returncode == 0, built.stderr
            ran = subprocess.run([str(binary)], text=True, capture_output=True, timeout=10)
            if should_pass:
                assert ran.returncode == 0, ran.stderr
            else:
                assert ran.returncode != 0, f"mutant survived: {mutation}"
