"""Host controls compiled from the live source-owned name_proto helpers."""

from __future__ import annotations

import shutil
import subprocess
import sys
from pathlib import Path

import pytest


ROOT = Path(__file__).resolve().parents[2]
VEH_CPP = ROOT / "src/veh.cpp"
VEH_H = ROOT / "src/veh.h"


def _definition(source: str, signature: str) -> str:
    start = source.find(signature)
    if start < 0:
        raise AssertionError(f"actual source definition not found: {signature}")
    opening = source.find("{", start + len(signature))
    if opening < 0:
        raise AssertionError(f"opening brace not found: {signature}")
    depth = 0
    for end in range(opening, len(source)):
        if source[end] == "{":
            depth += 1
        elif source[end] == "}":
            depth -= 1
            if depth == 0:
                body = source[start : end + 1]
                if "BODY GOES HERE" in body:
                    raise AssertionError(f"source definition remains unfinished: {signature}")
                return body
    raise AssertionError(f"unterminated source definition: {signature}")


def _actual_helpers() -> tuple[str, str, str]:
    source = VEH_CPP.read_text()
    return (
        _definition(source, "static void add_prototype_name_suffix("),
        _definition(source, "static const char *prototype_collision_name_bytes("),
        _definition(source, "static void apply_prototype_name_collisions("),
    )


def _struct_body(source: str, name: str) -> str:
    return _definition(source, f"struct {name}")


def _assert_fixture_tracks_product_layout() -> None:
    header = VEH_H.read_text()
    ability = _struct_body(header, "RulesAbility")
    prototype = _struct_body(header, "VehPrototype")
    for member in (
        "LPSTR name;", "LPSTR description;", "LPSTR abbreviation;",
        "int cost_factor;", "int unk_1;", "uint32_t flags;",
        "int16_t preq_tech;", "uint16_t padding;",
    ):
        assert member in ability
    for member in (
        "char veh_name[32];", "uint32_t ability_flags;",
        "uint8_t chassis_id;", "uint8_t weapon_id;", "uint8_t armor_id;",
        "uint8_t reactor_id;", "uint16_t flags;", "int16_t preq_tech;",
    ):
        assert member in prototype
    assert "MaxVehProtoNum = 512" in header
    assert "MaxVehProtoFactionNum = 64" in header
    helper = _actual_helpers()[1]
    assert "reinterpret_cast<const unsigned char *>(&Ability)" in helper
    assert "27 * sizeof Ability[0]" in helper
    assert "offsetof(RulesAbility, abbreviation)" in helper
    assert "&Ability[27].abbreviation" not in helper
    assert "memcpy" not in helper
    assert "StringTable" not in helper
    assert "'\\0'" not in helper


HOST_RUNTIME = r"""
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <cstdio>
typedef char *LPSTR;
struct RulesAbility {
    LPSTR name; LPSTR description; LPSTR abbreviation;
    int cost_factor; int unk_1; uint32_t flags;
    int16_t preq_tech; uint16_t padding;
};
struct VehPrototype {
    char veh_name[32]; uint32_t ability_flags;
    uint8_t chassis_id; uint8_t weapon_id; uint8_t armor_id; uint8_t reactor_id;
    uint8_t carry_capacity; uint8_t cost; uint8_t plan; int8_t unk_1;
    uint8_t obsolete_factions; int8_t combat_factions; int8_t icon_offset;
    int8_t padding; uint16_t flags; int16_t preq_tech;
};
static const int MaxVehProtoNum = 512;
static const int MaxVehProtoFactionNum = 64;
static RulesAbility Ability[29];
static VehPrototype VehPrototypes[MaxVehProtoNum];
static_assert(sizeof Ability == 29 * sizeof Ability[0], "whole ability array");
static_assert(offsetof(RulesAbility, abbreviation) == 2 * sizeof(LPSTR),
              "actual abbreviation-member position");
static_assert(sizeof(VehPrototype) == 0x34, "actual prototype stride");
"""


HOST_CASES = r"""
static void reset_candidates() {
    std::memset(VehPrototypes, 0, sizeof VehPrototypes);
    for (int i = 0; i < MaxVehProtoNum; ++i) {
        VehPrototypes[i].chassis_id = 9;
        VehPrototypes[i].weapon_id = 9;
        VehPrototypes[i].armor_id = 9;
        VehPrototypes[i].reactor_id = 3;
        VehPrototypes[i].ability_flags = 0;
    }
}
static void collide(char *primary, char *alternate, int faction,
                    int primary_difference, int &gendered) {
    apply_prototype_name_collisions(primary, alternate, 500, faction,
        1, 2, 3, 0x1234, 1, primary_difference, gendered, "Dup");
}
int main() {
    // This terminated byte view crosses from Ability[27] into Ability[28].
    // Its start is the native negative-candidate address; do not resolve the
    // LPSTR stored there or copy/terminate the production candidate view.
    std::memset(&Ability, 0, sizeof Ability);
    const size_t negative_offset =
        27 * sizeof Ability[0] + offsetof(RulesAbility, abbreviation);
    const char cross_record_name[] = "0123456789ABCDEFGHIJKLMN";
    static_assert(sizeof cross_record_name <= 32, "bounded fixture window");
    unsigned char *const ability_bytes =
        reinterpret_cast<unsigned char *>(&Ability);
    std::memcpy(ability_bytes + negative_offset, cross_record_name,
                sizeof cross_record_name);
    const char *const negative_name = prototype_collision_name_bytes(-1);
    assert(negative_name == reinterpret_cast<const char *>(
        ability_bytes + negative_offset));
    assert(std::strcmp(negative_name, cross_record_name) == 0);

    // A same-name design in faction 1 is ignored; faction 2 participates.
    reset_candidates();
    std::strcpy(VehPrototypes[64].veh_name, "FILTERED");
    std::strcpy(VehPrototypes[128].veh_name, "TARGET");
    char primary_a[64] = "TARGET", alternate_a[64] = "";
    std::strcpy(alternate_a + 1, "TARGET");
    int gender_a = 0;
    collide(primary_a, alternate_a, 2, 0, gender_a);
    assert(std::strcmp(primary_a, "TARGET Dup2") == 0);
    assert(std::strcmp(alternate_a + 1, "TARGET") == 0);
    assert(gender_a == 0);

    // Faction zero participates for every faction.
    reset_candidates();
    std::strcpy(VehPrototypes[0].veh_name, "TARGET");
    char primary_global[64] = "TARGET", alternate_global[64] = "";
    std::strcpy(alternate_global + 1, "TARGET");
    int gender_global = 0;
    collide(primary_global, alternate_global, 2, 0, gender_global);
    assert(std::strcmp(primary_global, "TARGET Dup2") == 0);

    // Equal forms restart at index zero; its candidate advances the suffix.
    reset_candidates();
    std::strcpy(VehPrototypes[1].veh_name, "UNIT");
    std::strcpy(VehPrototypes[0].veh_name, "UNIT Dup2");
    char primary_b[64] = "UNIT", alternate_b[64] = "";
    std::strcpy(alternate_b + 1, "UNIT");
    int gender_b = 0;
    collide(primary_b, alternate_b, 1, 0, gender_b);
    assert(std::strcmp(primary_b, "UNIT Dup3") == 0);
    assert(std::strcmp(alternate_b + 1, "UNIT") == 0);
    assert(gender_b == 0);

    // The differing-form restart compares against bytes "T\0" in the
    // abbreviation pointer member itself, not its pointed-to value.
    reset_candidates();
    std::strcpy(VehPrototypes[0].veh_name, "T Dup2");
    std::strcpy(VehPrototypes[1].veh_name, "ALPHA");
    Ability[27].abbreviation = reinterpret_cast<char *>(uintptr_t(0x54));
    char primary_c[64] = "ALPHA", alternate_c[64] = "";
    std::strcpy(alternate_c + 1, "T");
    int gender_c = 0;
    collide(primary_c, alternate_c, 1, 1, gender_c);
    assert(std::strcmp(primary_c, "ALPHA Dup2") == 0);
    // Candidate zero is seen after the -1 restart and advances the alternate
    // suffix once more, proving the alternate-collision restart as well.
    assert(std::strcmp(alternate_c + 1, "T Dup3") == 0);
    assert(gender_c == 0);

    reset_candidates();
    std::strcpy(VehPrototypes[1].veh_name, "ALPHA");
    alignas(16) char pointed_text[] = "HELLO";
    Ability[27].abbreviation = pointed_text;
    char primary_d[64] = "ALPHA", alternate_d[64] = "";
    std::strcpy(alternate_d + 1, "HELLO");
    int gender_d = 0;
    collide(primary_d, alternate_d, 1, 1, gender_d);
    assert(std::strcmp(alternate_d + 1, "HELLO") == 0);
    assert(gender_d == 1);

    // Reaching suffix 9 exhausts further primary suffixing.
    reset_candidates();
    std::strcpy(VehPrototypes[1].veh_name, "UNIT Dup9");
    char primary_e[64] = "UNIT Dup9", alternate_e[64] = "";
    std::strcpy(alternate_e + 1, "UNIT Dup9");
    int gender_e = 0;
    collide(primary_e, alternate_e, 1, 0, gender_e);
    assert(std::strcmp(primary_e, "UNIT Dup9") == 0);
    assert(gender_e == 0);
    std::puts("actual name_proto collision helpers passed");
}
"""


def _compile_and_run_runtime(tmp_path: Path) -> None:
    compiler = shutil.which("g++") or shutil.which("clang++")
    assert compiler, "GCC or Clang is required for this host control"
    suffix, name_bytes, collisions = _actual_helpers()
    source = tmp_path / "name_proto_collision_control.cpp"
    binary = tmp_path / "name_proto_collision_control"
    source.write_text(HOST_RUNTIME + suffix + name_bytes + collisions + HOST_CASES)
    subprocess.run(
        [compiler, "-std=c++11", "-O0", "-Wall", "-Wextra", "-Werror",
         str(source), "-o", str(binary)],
        check=True, capture_output=True, text=True,
    )
    result = subprocess.run([str(binary)], capture_output=True, text=True, timeout=10)
    assert result.returncode == 0, result.stderr
    assert "actual name_proto collision helpers passed" in result.stdout


def _compile_mutant_and_expect_safe_control_failure(tmp_path: Path,
                                                     replacement: str) -> None:
    compiler = shutil.which("g++") or shutil.which("clang++")
    assert compiler, "GCC or Clang is required for the compiled mutation control"
    suffix, name_bytes, collisions = _actual_helpers()
    correct_return = (
        "return reinterpret_cast<const char *>(ability_bytes + name_offset);"
    )
    assert correct_return in name_bytes
    name_bytes = name_bytes.replace(correct_return, replacement, 1)
    source = tmp_path / "name_proto_collision_mutant.cpp"
    binary = tmp_path / "name_proto_collision_mutant"
    source.write_text(HOST_RUNTIME + suffix + name_bytes + collisions + HOST_CASES)
    subprocess.run(
        [compiler, "-std=c++11", "-O0", "-Wall", "-Wextra", "-Werror",
         str(source), "-o", str(binary)],
        check=True, capture_output=True, text=True,
    )
    result = subprocess.run([str(binary)], capture_output=True, text=True, timeout=10)
    assert result.returncode != 0, "the host control failed to detect a compiled mutant"


def _compile_x86_layout(tmp_path: Path) -> None:
    compiler = shutil.which("g++")
    if not compiler:
        pytest.skip("g++ is required for the 32-bit syntax control")
    suffix, name_bytes, collisions = _actual_helpers()
    source = tmp_path / "name_proto_collision_x86.cpp"
    source.write_text(
        r"""
typedef __SIZE_TYPE__ size_t;
#define offsetof(type, member) __builtin_offsetof(type, member)
typedef unsigned int uint32_t; typedef unsigned char uint8_t;
typedef signed char int8_t; typedef short int16_t; typedef unsigned short uint16_t;
typedef char *LPSTR;
struct RulesAbility { LPSTR name; LPSTR description; LPSTR abbreviation;
    int cost_factor; int unk_1; uint32_t flags; int16_t preq_tech; uint16_t padding; };
struct VehPrototype { char veh_name[32]; uint32_t ability_flags;
    uint8_t chassis_id; uint8_t weapon_id; uint8_t armor_id; uint8_t reactor_id;
    uint8_t carry_capacity; uint8_t cost; uint8_t plan; int8_t unk_1;
    uint8_t obsolete_factions; int8_t combat_factions; int8_t icon_offset;
    int8_t padding; uint16_t flags; int16_t preq_tech; };
static const int MaxVehProtoNum = 512;
static const int MaxVehProtoFactionNum = 64;
static RulesAbility Ability[29]; static VehPrototype VehPrototypes[MaxVehProtoNum];
static_assert(sizeof Ability == 29 * 0x1c, "whole native ability array extent");
static_assert(sizeof(LPSTR) == 4, "the game image is 32-bit");
static_assert(__builtin_offsetof(RulesAbility, abbreviation) == 8, "member offset");
static_assert(sizeof(RulesAbility) == 0x1c, "RulesAbility stride");
static_assert(sizeof(VehPrototype) == 0x34, "VehPrototype stride");
static_assert(27 * sizeof Ability[0] + __builtin_offsetof(RulesAbility, abbreviation) == 764,
              "native negative-candidate byte start");
static_assert(764 + 32 <= sizeof Ability, "32-byte view within current array");
extern "C" size_t strlen(const char *);
extern "C" char *strcat(char *, const char *);
extern "C" char *strstr(const char *, const char *);
namespace std { int strcmp(const char *, const char *); }
""" + suffix + name_bytes + collisions
    )
    subprocess.run(
        [compiler, "-m32", "-std=c++11", "-fsyntax-only", "-nostdinc++", str(source)],
        check=True, capture_output=True, text=True,
    )


def test_live_source_collision_helpers_cover_restart_and_member_bytes(tmp_path: Path) -> None:
    _assert_fixture_tracks_product_layout()
    if sys.byteorder != "little":
        pytest.skip("the pinned x86 member-byte case is little-endian")
    _compile_and_run_runtime(tmp_path)


def test_live_source_array_view_rejects_resolved_and_misoffset_mutants(
    tmp_path: Path,
) -> None:
    if sys.byteorder != "little":
        pytest.skip("the pinned x86 member-byte case is little-endian")
    _compile_mutant_and_expect_safe_control_failure(
        tmp_path,
        "(void)ability_bytes; (void)name_offset; return Ability[27].abbreviation;",
    )
    _compile_mutant_and_expect_safe_control_failure(
        tmp_path,
        "return reinterpret_cast<const char *>(ability_bytes + name_offset + 1);",
    )


def test_live_source_collision_helpers_fit_original_32_bit_storage(tmp_path: Path) -> None:
    _assert_fixture_tracks_product_layout()
    _compile_x86_layout(tmp_path)

