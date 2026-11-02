"""C++98 actual-source controls for the six vehicle-stat formatters.

The fixture extracts each production definition from its genuine owner. It
uses host-only stubs for the game data/services and does not claim native ABI,
rendering, or full-game behavior. The scratch-buffer overload is accepted from
either its current owner, but exactly one production definition must exist.
"""

from __future__ import annotations

import re
import shutil
import subprocess
from pathlib import Path

import pytest
from match_recovery.includes import COMMENTS_OR_STRINGS
from match_recovery.source import read_file


ROOT = Path(__file__).resolve().parents[2]
OWNERS = (ROOT / "src/veh.cpp", ROOT / "src/vehicle_stats.cpp")
FUNCTIONS = (
    (0x57D560, "void __cdecl say_offense(LPSTR stat, int proto_id)"),
    (0x57D6D0, "void __cdecl say_defense(LPSTR stat, int proto_id)"),
    (0x57D7D0, "void __cdecl say_stats_3(LPSTR stat, int proto_id)"),
    (0x50B9A0, "void __cdecl say_stats_3(int proto_id)"),
    (0x57D8E0, "void __cdecl say_stats_2(LPSTR stat, int proto_id)"),
    (0x57DAA0, "void __cdecl say_stats(LPSTR stat, int proto_id, LPSTR custom_spacer)"),
)


def masked(text: str) -> str:
    return COMMENTS_OR_STRINGS.sub(lambda match: re.sub(r"[^\n]", " ", match[0]), text)


def extract_function(text: str, signature: str) -> str:
    clean = masked(text)
    starts = [match.start() for match in re.finditer(re.escape(signature), clean)]
    assert len(starts) == 1, f"expected one definition for {signature}, found {len(starts)}"
    start = starts[0]
    opening = clean.index("{", start)
    depth = 0
    for pos in range(opening, len(clean)):
        if clean[pos] == "{":
            depth += 1
        elif clean[pos] == "}":
            depth -= 1
            if depth == 0:
                return text[start : pos + 1]
    raise AssertionError(f"unterminated production function: {signature}")


def production_bodies() -> dict[int, str]:
    owner_text = {path: path.read_text(encoding="utf-8") for path in OWNERS}
    result = {}
    for address, signature in FUNCTIONS:
        matches = [
            (path, text)
            for path, text in owner_text.items()
            if signature in masked(text)
        ]
        assert len(matches) == 1, f"expected one live owner for {signature}, got {len(matches)}"
        result[address] = extract_function(matches[0][1], signature)
    declarations = re.findall(
        r'^static char (?:Defense|Stats3)[A-Za-z0-9_]+Text\[\] = "[^"\n]*";$',
        owner_text[ROOT / "src/vehicle_stats.cpp"], re.M)
    if declarations:
        result[0x57D6D0] = "\n".join(declarations) + "\n" + result[0x57D6D0]
    return result


PREFIX = r'''#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#define __cdecl
typedef char *LPSTR;
typedef int BOOL;
enum {
  TRIAD_LAND=0, TRIAD_SEA=1, TRIAD_AIR=2,
  ABL_AMPHIBIOUS=0x8, ABL_DROP_POD=0x10, ABL_AIR_SUPERIORITY=0x20,
  ABL_AAA=0x100, ABL_COMM_JAMMER=0x200, ABL_EMPATHIC=0x800,
  ABL_BLINK_DISPLACER=0x20000, ABL_TRANCE=0x40000, ABL_NERVE_GAS=0x100000,
  PLAN_OFFENSIVE=0, PLAN_DEFENSIVE=2, PLAN_RECONNAISANCE=3,
  PLAN_NAVAL_TRANSPORT=7, PLAN_PLANET_BUSTER=5,
  WPN_MODE_PROJECTILE=0, WPN_MODE_ENERGY=1, WPN_MODE_MISSILE=2,
  WPN_MODE_TRANSPORT=7, CHSI_FOIL=3,
  WPN_RESONANCE_LASER=12, WPN_RESONANCE_BOLT=13,
  ARM_PULSE_3_ARMOR=10, ARM_RESONANCE_3_ARMOR=11,
  ARM_PULSE_8_ARMOR=12, ARM_RESONANCE_8_ARMOR=13,
  RECT_FISSION=1
};
struct Proto {
  uint8_t plan, chassis_id, weapon_id, ability_flags, armor_id, carry_capacity;
  uint32_t reactor_id;
};
struct ChassisInfo { uint8_t triad, speed; };
struct WeaponInfo { uint8_t mode; };
struct RulesInfo { uint32_t move_rate_roads; };
struct LabelsInfo { LPSTR *strings_ptr; };
static Proto VehPrototypes[4] = {};
static ChassisInfo Chassis[4] = {};
static WeaponInfo Weapon[32] = {};
static RulesInfo Rules = {3};
static LabelsInfo Labels = {};
static LPSTR PlansFullName[16] = {};
static LPSTR PlansShortName[16] = {};
static LPSTR label_storage[313] = {};
static char StringTemp[1032] = {};
static char label_psi[] = "Psi";
static char label_sea[] = "Sea";
static char label_air[] = "Air";
static char label_explore[] = "Explore/Defense";
static char label_combat[] = "Combat";
static char label_cargo[] = "Cargo";
static char label_assault[] = "Assault";
static int ability_mask[4] = {};
static int ability_log[64] = {};
static int ability_count = 0;
static int arty_results[8] = {};
static int arty_count = 0;
static int offense_rating[4] = {1, 1, 1, 1};
static int defense_rating[4] = {1, 1, 1, 1};
static int vehicle_speed[4] = {6, 6, 6, 6};
static int offense_value = 9;
static int armor_value = 7;
static int table_calls = 0;
static LPSTR table_log[32] = {};
static void reset_observations() {
  ability_count = 0;
  arty_count = 0;
  table_calls = 0;
  memset(ability_log, 0, sizeof(ability_log));
  memset(arty_results, 0, sizeof(arty_results));
  memset(table_log, 0, sizeof(table_log));
}
static BOOL has_abil(int proto_id, int ability) {
  ability_log[ability_count++] = ability;
  return (ability_mask[proto_id] & ability) != 0;
}
static BOOL can_arty(int, BOOL) {
  int index = arty_count++;
  return arty_results[index] != 0;
}
static int offense_proto(int, int, BOOL) { return offense_value; }
static int armor_proto(int, int, BOOL) { return armor_value; }
static int speed_proto(int proto_id) { return vehicle_speed[proto_id]; }
static int8_t get_proto_offense_rating(int proto_id) {
  return static_cast<int8_t>(offense_rating[proto_id]);
}
static int8_t get_proto_defense_rating(int proto_id) {
  return static_cast<int8_t>(defense_rating[proto_id]);
}
static uint8_t get_proto_triad(uint32_t proto_id) {
  return Chassis[VehPrototypes[proto_id].chassis_id].triad;
}
struct FakeStringTable {
  LPSTR get(LPSTR address) {
    table_log[table_calls++] = address;
    assert(address != 0);
    return address;
  }
} StringTable;
static char *_itoa(int value, char *output, int radix) {
  assert(radix == 10);
  sprintf(output, "%d", value);
  return output;
}
static void initialize_case() {
  memset(VehPrototypes, 0, sizeof(VehPrototypes));
  memset(Chassis, 0, sizeof(Chassis));
  memset(Weapon, 0, sizeof(Weapon));
  memset(ability_mask, 0, sizeof(ability_mask));
  memset(offense_rating, 0, sizeof(offense_rating));
  memset(defense_rating, 0, sizeof(defense_rating));
  for (int i = 0; i < 4; ++i) {
    offense_rating[i] = 1;
    defense_rating[i] = 1;
    vehicle_speed[i] = 6;
  }
  VehPrototypes[0].reactor_id = RECT_FISSION;
  Chassis[0].triad = TRIAD_LAND;
  Chassis[0].speed = 1;
  Weapon[0].mode = 3;
  Rules.move_rate_roads = 3;
  PlansFullName[3] = label_explore;
  PlansFullName[0] = label_assault;
  PlansFullName[7] = label_cargo;
  PlansShortName[0] = label_combat;
  PlansShortName[2] = label_combat;
  PlansShortName[7] = label_cargo;
  label_storage[196] = label_psi;
  label_storage[163] = label_sea;
  label_storage[162] = label_air;
  label_storage[312] = label_combat;
  Labels.strings_ptr = label_storage;
  StringTemp[0] = 0;
  reset_observations();
}
'''


MAIN = r'''
int main() {
  initialize_case();

  // Offense: every symbol is in source order, including both live artillery reads.
  ability_mask[0] = ABL_AIR_SUPERIORITY | ABL_DROP_POD | ABL_AMPHIBIOUS |
      ABL_NERVE_GAS | ABL_EMPATHIC | ABL_BLINK_DISPLACER;
  VehPrototypes[0].weapon_id = WPN_RESONANCE_LASER;
  arty_results[0] = 1;
  arty_results[1] = 1;
  char output[512] = {};
  say_offense(output, 0);
  assert(strcmp(output, "<(9^~xe!)>r") == 0);
  assert(arty_count == 2);
  assert(ability_count == 7);
  assert(ability_log[0] == ABL_AIR_SUPERIORITY);
  assert(ability_log[1] == ABL_DROP_POD);
  assert(ability_log[2] == ABL_AMPHIBIOUS);
  assert(ability_log[3] == ABL_NERVE_GAS);
  assert(ability_log[4] == ABL_EMPATHIC);
  assert(ability_log[5] == ABL_BLINK_DISPLACER);
  assert(ability_log[6] == ABL_AIR_SUPERIORITY);

  // Defense suffixes and ability order, including pulse armor naming.
  initialize_case();
  ability_mask[0] = ABL_AAA | ABL_COMM_JAMMER | ABL_TRANCE;
  VehPrototypes[0].armor_id = ARM_PULSE_8_ARMOR;
  output[0] = 0;
  say_defense(output, 0);
  assert(strcmp(output, "<7+t>p") == 0);
  assert(ability_count == 4);
  assert(ability_log[0] == ABL_AAA && ability_log[1] == ABL_COMM_JAMMER);
  assert(ability_log[2] == ABL_TRANCE && ability_log[3] == ABL_AAA);

  // Both PSI substitutions, signed negative speed division, and reactor suffix.
  initialize_case();
  offense_rating[0] = -1;
  defense_rating[0] = -1;
  vehicle_speed[0] = -5;
  VehPrototypes[0].reactor_id = 2;
  output[0] = 0;
  say_stats_3(output, 0);
  assert(strcmp(output, "?-?--1*2") == 0);

  // The int-only scratch-buffer overload must forward to the same formatter.
  StringTemp[0] = 0;
  say_stats_3(0);
  assert(strcmp(StringTemp, "?-?--1*2") == 0);

  // Stats-2: PSI label lookups, signed speed, sea label, and >1 reactor rule.
  initialize_case();
  offense_rating[0] = -1;
  defense_rating[0] = -1;
  vehicle_speed[0] = 6;
  Chassis[0].triad = TRIAD_SEA;
  VehPrototypes[0].reactor_id = 3;
  output[0] = 0;
  say_stats_2(output, 0);
  assert(strcmp(output, "Psi-Psi-2 Sea*3") == 0);
  assert(table_calls == 3);
  assert(table_log[0] == label_psi && table_log[1] == label_psi);
  assert(table_log[2] == label_sea);

  // Full formatter: mode<3 path, custom spacer twice, air label, reactor form.
  initialize_case();
  VehPrototypes[0].plan = PLAN_OFFENSIVE;
  VehPrototypes[0].weapon_id = 0;
  VehPrototypes[0].reactor_id = 2;
  Chassis[0].triad = TRIAD_AIR;
  Weapon[0].mode = WPN_MODE_MISSILE;
  offense_rating[0] = 2;
  defense_rating[0] = 3;
  vehicle_speed[0] = 7;
  output[0] = 0;
  say_stats(output, 0, (LPSTR)":");
  assert(strcmp(output, "Combat, 9:7:2 Air (*2)") == 0);

  // Null custom spacer retains the original slash fallback at both sites.
  output[0] = 0;
  say_stats(output, 0, 0);
  assert(strcmp(output, "Combat, 9/7/2 Air (*2)") == 0);

  // Reconnaissance, land, default ratings, and no abilities take the named-plan path.
  initialize_case();
  VehPrototypes[0].plan = PLAN_RECONNAISANCE;
  VehPrototypes[0].chassis_id = CHSI_FOIL;
  Chassis[CHSI_FOIL].triad = TRIAD_LAND;
  Chassis[CHSI_FOIL].speed = 1;
  Weapon[0].mode = WPN_MODE_TRANSPORT;
  PlansFullName[3] = label_explore;
  PlansFullName[7] = label_cargo;
  output[0] = 0;
  say_stats(output, 0, 0);
  assert(strcmp(output, "Explore/Defense, Cargo") == 0);
  return 0;
}
'''


def make_fixture(bodies: dict[int, str], mutant: str | None = None) -> str:
    changed = dict(bodies)
    if mutant == "offense_second_arty_read":
        address = 0x57D560
        old = '    if (can_arty(proto_id, true)) {\n        strcat(stat, ")");\n    }'
        assert changed[address].count(old) == 1
        changed[address] = changed[address].replace(old, "", 1)
    elif mutant == "defense_pulse_suffix":
        address = 0x57D6D0
        old = '        strcat(stat, DefensePulseText);'
        assert changed[address].count(old) == 1
        changed[address] = changed[address].replace(old, '        strcat(stat, DefensePulseText + 1);', 1)
    elif mutant == "unsigned_speed_division":
        total = sum(body.count("(int)Rules.move_rate_roads") for body in changed.values())
        assert total == 3
        changed = {address: body.replace("(int)Rules.move_rate_roads", "Rules.move_rate_roads")
                   for address, body in changed.items()}
    elif mutant == "stats2_sea_label":
        address = 0x57D8E0
        old = "Labels.strings_ptr[163]"
        assert changed[address].count(old) == 1
        changed[address] = changed[address].replace(old, "Labels.strings_ptr[162]", 1)
    elif mutant == "custom_spacer":
        address = 0x57DAA0
        old = '        if (custom_spacer) {\n            strcat(stat, custom_spacer);\n        } else {\n            strcat(stat, "/");\n        }'
        assert changed[address].count(old) == 1
        changed[address] = changed[address].replace(old, '        strcat(stat, "/");', 1)
    elif mutant == "scratch_overload_forwarding":
        address = 0x50B9A0
        old = "    say_stats_3(StringTemp, proto_id);"
        assert changed[address].count(old) == 1
        changed[address] = changed[address].replace(
            old, '    strcat(StringTemp, proto_id ? "!" : "?");', 1)
    elif mutant is not None:
        raise AssertionError(f"unknown mutant: {mutant}")
    ordered = [changed[address] for address, _signature in FUNCTIONS]
    return PREFIX + "\n".join(ordered) + MAIN


def compile_and_run(tmp_path: Path, compiler: str, source: str, name: str):
    cpp = tmp_path / f"{name}.cpp"
    binary = tmp_path / name
    cpp.write_text(source, encoding="utf-8")
    build = subprocess.run(
        [compiler, "-std=c++98", "-Wall", "-Wextra", "-Werror", str(cpp), "-o", str(binary)],
        capture_output=True, text=True, timeout=45, check=False,
    )
    assert build.returncode == 0, build.stdout + build.stderr
    return subprocess.run([str(binary)], capture_output=True, text=True, timeout=15, check=False)


def test_six_mapped_formatter_identities_and_unique_definitions():
    records = {}
    for owner in OWNERS:
        for row in read_file(owner):
            if row.address in {address for address, _signature in FUNCTIONS}:
                assert row.address not in records, f"duplicate production identity {row.address:#x}"
                records[row.address] = row
    assert set(records) == {address for address, _signature in FUNCTIONS}
    expected_names = {
        0x57D560: "?say_offense@@YAXPADH@Z",
        0x57D6D0: "?say_defense@@YAXPADH@Z",
        0x57D7D0: "?say_stats_3@@YAXPADH@Z",
        0x50B9A0: "?say_stats_3@@YAXH@Z",
        0x57D8E0: "?say_stats_2@@YAXPADH@Z",
        0x57DAA0: "?say_stats@@YAXPADHPAD@Z",
    }
    for address, row in records.items():
        assert row.name == expected_names[address]
        assert row.image_spans == ((address, {0x57D560: 0x57D6C5, 0x57D6D0: 0x57D7C4,
                                               0x57D7D0: 0x57D8D3, 0x50B9A0: 0x50B9B6,
                                               0x57D8E0: 0x57DA94, 0x57DAA0: 0x57DED8}[address]),)
    assert records[0x57D6D0].byte_exact
    assert records[0x50B9A0].byte_exact
    assert not records[0x57D7D0].byte_exact


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
def test_actual_six_formatters_cpp98_outputs_and_read_order(compiler_name, tmp_path):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(f"{compiler_name} unavailable")
    result = compile_and_run(tmp_path, compiler, make_fixture(production_bodies()), "vehicle-stats")
    assert result.returncode == 0, result.stdout + result.stderr


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
@pytest.mark.parametrize("mutant", (
    "offense_second_arty_read", "defense_pulse_suffix", "unsigned_speed_division",
    "stats2_sea_label", "custom_spacer", "scratch_overload_forwarding",
))
def test_compile_success_mutants_are_observed(compiler_name, mutant, tmp_path):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(f"{compiler_name} unavailable")
    source = make_fixture(production_bodies(), mutant)
    result = compile_and_run(tmp_path, compiler, source, f"vehicle-stats-{mutant}")
    assert result.returncode != 0, f"{mutant} mutant survived output/read-order checks"
