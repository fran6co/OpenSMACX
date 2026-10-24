"""Actual-source controls for prototype cost intermediate widths.

Compile the complete source-owned body with typed host doubles. Keep the signed
combat-divisor diagnostic and isolate byte truncation with an armor-local mutant.
The representable AIR cost-64/reactor-2 witness does not claim stock-data reachability.
Inputs avoid invalid indices, undefined shifts and arithmetic overflow.
"""
from pathlib import Path
import shutil
import subprocess

import pytest


ROOT = next(parent for parent in Path(__file__).resolve().parents
            if (parent / "src/veh.cpp").is_file())
SOURCE = ROOT / "src/veh.cpp"
ADDRESS = 0x005A5A60
SIGNATURE = "int __cdecl proto_cost(int chassis_id, int weapon_id, int armor_id,"


def _definition(record_region: str) -> str:
    """Extract the mapped C++ definition, ignoring braces in comments/literals."""
    start = record_region.find(SIGNATURE)
    assert start >= 0, "missing source-owned proto_cost definition"
    opening = record_region.find("{", start)
    assert opening >= 0
    depth = 0
    index = opening
    state = "code"
    while index < len(record_region):
        char = record_region[index]
        following = record_region[index + 1] if index + 1 < len(record_region) else ""
        if state == "code":
            if char == "/" and following == "/":
                state = "line"
                index += 2
                continue
            if char == "/" and following == "*":
                state = "block"
                index += 2
                continue
            if char == '"':
                state = "string"
                index += 1
                continue
            if char == "'":
                state = "char"
                index += 1
                continue
            if char == "{":
                depth += 1
            elif char == "}":
                depth -= 1
                if depth == 0:
                    return record_region[start:index + 1]
        elif state == "line":
            if char == "\n":
                state = "code"
        elif state == "block":
            if char == "*" and following == "/":
                state = "code"
                index += 2
                continue
        else:
            if char == "\\":
                index += 2
                continue
            if (state == "string" and char == '"') or (state == "char" and char == "'"):
                state = "code"
        index += 1
    raise AssertionError("unterminated source-owned proto_cost definition")


def actual_source_bodies() -> tuple[str, str, str]:
    from match_recovery.source import read_file

    records = read_file(SOURCE)
    record = next(item for item in records if item.address == ADDRESS)
    signed = _definition(record.region)
    # Fail closed if the reviewed one-token source change was not materialized.
    assert signed.count("int combat_mod = armor_cost / 2 + 1;") == 1
    assert "uint32_t combat_mod =" not in signed
    assert signed.count("int armor_cost = Armor[armor_id].cost;") == 1
    assert "uint8_t armor_cost = Armor[armor_id].cost;" not in signed
    combat_mutant = signed.replace(
        "int combat_mod = armor_cost / 2 + 1;",
        "uint32_t combat_mod = armor_cost / 2 + 1;",
        1,
    )
    armor_mutant = signed.replace(
        "int armor_cost = Armor[armor_id].cost;",
        "uint8_t armor_cost = Armor[armor_id].cost;",
        1,
    )
    assert combat_mutant != signed and armor_mutant != signed
    return (
        signed.replace("int __cdecl proto_cost(", "int __cdecl proto_cost_signed(", 1),
        combat_mutant.replace("int __cdecl proto_cost(", "int __cdecl proto_cost_unsigned_mutant(", 1),
        armor_mutant.replace("int __cdecl proto_cost(", "int __cdecl proto_cost_byte_armor_mutant(", 1),
    )


PREAMBLE = r"""
#include <cassert>
#include <cstdint>
#include <cstring>
#include <type_traits>
using std::int8_t;
using std::uint8_t;
using std::uint32_t;

#define __cdecl

enum { TRIAD_LAND = 0, TRIAD_SEA = 1, TRIAD_AIR = 2 };
enum { WPN_MODE_MISSILE = 2, WPN_MODE_INFOWAR = 11 };
enum { RECT_FISSION = 1, AFLAG_COST_INC_LAND_UNIT = 0x400 };
static const int MaxAbilityNum = 29;

// Behavioral doubles retain the actual source declaration widths/types for
// every member read by proto_cost; unrelated table members are omitted.
struct RulesChassis { uint8_t cost; uint8_t triad; uint8_t missile; };
struct RulesWeapon { int8_t offense_rating; uint8_t mode; uint8_t cost; };
struct RulesArmor { uint8_t cost; };
struct RulesAbility { int cost_factor; uint32_t flags; };
static_assert(std::is_same<decltype(RulesChassis::cost), uint8_t>::value, "chassis cost width");
static_assert(std::is_same<decltype(RulesChassis::triad), uint8_t>::value, "triad width");
static_assert(std::is_same<decltype(RulesWeapon::offense_rating), int8_t>::value, "weapon rating width");
static_assert(std::is_same<decltype(RulesWeapon::mode), uint8_t>::value, "weapon mode width");
static_assert(std::is_same<decltype(RulesWeapon::cost), uint8_t>::value, "weapon cost width");
static_assert(std::is_same<decltype(RulesArmor::cost), uint8_t>::value, "armor cost width");
static_assert(std::is_same<decltype(RulesAbility::cost_factor), int>::value, "ability factor type");
static_assert(std::is_same<decltype(RulesAbility::flags), uint32_t>::value, "ability flags width");

static RulesChassis Chassis[19];
static RulesWeapon Weapon[26];
static RulesArmor Armor[14];
static RulesAbility Ability[MaxAbilityNum];

static int range(int value, int low, int high) {
    if (value < low) return low;
    if (value > high) return high;
    return value;
}
"""


MAIN = r"""
static void clear_tables() {
    std::memset(Chassis, 0, sizeof Chassis);
    std::memset(Weapon, 0, sizeof Weapon);
    std::memset(Armor, 0, sizeof Armor);
    std::memset(Ability, 0, sizeof Ability);
}

static void same_valid_result(int chassis, int weapon, int armor,
                              int ability, int reactor) {
    const int signed_result = proto_cost_signed(chassis, weapon, armor, ability, reactor);
    const int unsigned_result = proto_cost_unsigned_mutant(chassis, weapon, armor, ability, reactor);
    assert(signed_result == unsigned_result);
}

int main() {
    // Baseline / original special case, then representative sea and air paths.
    clear_tables();
    Chassis[0].cost = 1; Chassis[0].triad = TRIAD_LAND;
    Weapon[0].cost = 1; Armor[0].cost = 1;
    assert(proto_cost_signed(0, 0, 0, 0, RECT_FISSION) == 1);
    same_valid_result(0, 0, 0, 0, RECT_FISSION);

    clear_tables();
    Chassis[0].cost = 2; Chassis[0].triad = TRIAD_SEA;
    Weapon[0].cost = 3; Weapon[0].mode = 0; Armor[0].cost = 4;
    assert(proto_cost_signed(0, 0, 0, 0, 2) == 3);
    same_valid_result(0, 0, 0, 0, 2);

    clear_tables();
    Chassis[0].cost = 1; Chassis[0].triad = TRIAD_AIR;
    Weapon[0].cost = 2; Weapon[0].mode = WPN_MODE_MISSILE; Armor[0].cost = 3;
    assert(proto_cost_signed(0, 0, 0, 0, RECT_FISSION) == 2);
    same_valid_result(0, 0, 0, 0, RECT_FISSION);

    // Land ability ratio/cost-factor and land-only flag paths.
    clear_tables();
    Chassis[0].cost = 3; Chassis[0].triad = TRIAD_LAND;
    Weapon[0].cost = 4; Armor[0].cost = 2;
    Ability[0].cost_factor = -2;
    Ability[0].flags = AFLAG_COST_INC_LAND_UNIT;
    Ability[1].cost_factor = 5;
    assert(proto_cost_signed(0, 0, 0, 3, RECT_FISSION) == 25);
    same_valid_result(0, 0, 0, 3, RECT_FISSION);

    // The -1 ratio ability must retain its source guard when armor cost is zero.
    clear_tables();
    Chassis[0].cost = 2; Chassis[0].triad = TRIAD_LAND;
    Weapon[0].cost = 5; Armor[0].cost = 0;
    Ability[0].cost_factor = -1;
    assert(proto_cost_signed(0, 0, 0, 1, RECT_FISSION) == 3);
    same_valid_result(0, 0, 0, 1, RECT_FISSION);

    // Preserve the early missile return after its original preceding setup.
    clear_tables();
    Chassis[0].cost = 9; Chassis[0].triad = TRIAD_LAND; Chassis[0].missile = 1;
    Weapon[0].cost = 6; Weapon[0].offense_rating = 99; Armor[0].cost = 7;
    assert(proto_cost_signed(0, 0, 0, 0, RECT_FISSION) == 6);
    same_valid_result(0, 0, 0, 0, RECT_FISSION);

    // Isolated width witness: Armor.cost=64 is representable as uint8_t and
    // all indices/reactor values are in bounds. Stock-data reachability is unknown.
    clear_tables();
    Chassis[0].cost = 1; Chassis[0].triad = TRIAD_AIR;
    Weapon[0].cost = 2; Weapon[0].mode = WPN_MODE_MISSILE;
    Armor[0].cost = 64;
    assert(proto_cost_signed(0, 0, 0, 0, 2) == 1052);
    assert(proto_cost_byte_armor_mutant(0, 0, 0, 0, 2) == 3);

    // Boundary grid uses representable unsigned-byte costs, valid table indices,
    // and reactor ids 1..4. It verifies ordinary results are unchanged by the
    // type correction. The unsigned mutant is diagnostic only: with these real
    // member widths all cost intermediates are nonnegative and bounded, so no
    // valid-input mutant failure is expected or forced.
    const uint8_t costs[] = {0, 1, 2, 7, 255};
    const uint8_t modes[] = {0, WPN_MODE_MISSILE, WPN_MODE_INFOWAR};
    for (int triad = TRIAD_LAND; triad <= TRIAD_AIR; ++triad) {
        for (uint8_t chassis_cost : costs) {
            for (uint8_t weapon_cost : costs) {
                for (uint8_t armor_cost : costs) {
                    for (int reactor = 1; reactor <= 4; ++reactor) {
                        for (uint8_t mode : modes) {
                            clear_tables();
                            Chassis[0].cost = chassis_cost;
                            Chassis[0].triad = static_cast<uint8_t>(triad);
                            Weapon[0].cost = weapon_cost;
                            Weapon[0].mode = mode;
                            Armor[0].cost = armor_cost;
                            same_valid_result(0, 0, 0, 0, reactor);
                        }
                    }
                }
            }
        }
    }
}
"""


def _available_compilers():
    return tuple(path for name in ("g++", "clang++") if (path := shutil.which(name)))


@pytest.mark.parametrize("compiler", _available_compilers() or (None,))
def test_actual_proto_cost_signed_type_and_unsigned_diagnostic(tmp_path, compiler):
    if compiler is None:
        pytest.skip("GCC or Clang host compiler unavailable")
    signed_body, unsigned_mutant, byte_armor_mutant = actual_source_bodies()
    source = tmp_path / "proto_cost_signed.cpp"
    binary = tmp_path / "proto_cost_signed"
    source.write_text(PREAMBLE + signed_body + "\n" + unsigned_mutant + "\n" + byte_armor_mutant + "\n" + MAIN)
    subprocess.run(
        [compiler, "-std=c++11", "-O2", str(source), "-o", str(binary)],
        check=True, capture_output=True, timeout=60,
    )
    subprocess.run([str(binary)], check=True, capture_output=True, timeout=10)
