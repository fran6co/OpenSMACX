"""Actual-source controls for vehicle consumers of the nested project owner."""

from __future__ import annotations

import os
from pathlib import Path
import shutil
import subprocess


ROOT = Path(__file__).resolve().parents[2]
VEH_CPP = ROOT / "src/veh.cpp"
BASE_H = ROOT / "src/base.h"
EXISTING_FIXTURE = Path(__file__).with_name("base_secret_project_accessor_fixture.cpp")


def _definition(source: str, signature: str) -> str:
    if source.count(signature) != 1:
        raise AssertionError(f"expected one actual definition: {signature}")
    start = source.index(signature)
    opening = source.index("{", start)
    depth = 0
    state = "code"
    escaped = False
    index = opening
    while index < len(source):
        char = source[index]
        next_char = source[index + 1] if index + 1 < len(source) else ""
        if state == "line":
            if char == "\n":
                state = "code"
        elif state == "block":
            if char == "*" and next_char == "/":
                state = "code"
                index += 1
        elif state in ("string", "char"):
            if escaped:
                escaped = False
            elif char == "\\":
                escaped = True
            elif (state == "string" and char == '"') or (state == "char" and char == "'"):
                state = "code"
        elif char == "/" and next_char == "/":
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
                return source[start:index + 1]
        index += 1
    raise AssertionError(f"unterminated actual definition: {signature}")


def _condition(body: str, member: str, occurrence: int) -> str:
    token = "GameDataStateStorage.fields_.secret_project_." + member + "()"
    starts = []
    cursor = 0
    while (position := body.find(token, cursor)) >= 0:
        opening = body.rfind("if (", 0, position)
        if opening >= 0 and opening not in starts:
            starts.append(opening)
        cursor = position + len(token)
    starts.sort()
    if len(starts) <= occurrence:
        raise AssertionError(f"missing {member} condition occurrence {occurrence}")
    opening = starts[occurrence]
    opening += len("if ")
    depth = 0
    for end in range(opening, len(body)):
        if body[end] == "(":
            depth += 1
        elif body[end] == ")":
            depth -= 1
            if depth == 0:
                return body[opening + 1:end]
    raise AssertionError(f"unterminated {member} if condition")


def _actual_conditions() -> tuple[str, str, str, str, str]:
    vehicle = VEH_CPP.read_text()
    terraform = _definition(vehicle, "int __cdecl action_terraform(")
    airdrop = _definition(vehicle, "void __cdecl sub_500380(")
    assert "SecretProjectBoreholeBase" not in vehicle
    assert "SecretProjectXenoempathyBase" not in vehicle
    assert "SecretProjectAirdropBase" not in vehicle
    # Each native test reloads its selected base ID both for the >= 0 gate
    # and for the following unchecked Bases[] access.
    assert terraform.count("GameDataStateStorage.fields_.secret_project_.xenoempathy_dome()") == 4
    assert terraform.count("GameDataStateStorage.fields_.secret_project_.weather_paradigm()") == 4
    assert airdrop.count("GameDataStateStorage.fields_.secret_project_.space_elevator()") == 2
    project_condition = _condition(airdrop, "space_elevator", 0)
    assignment_start = airdrop.find(
        "anywhere = Bases[GameDataStateStorage.fields_.secret_project_.space_elevator()]"
    )
    if assignment_start < 0:
        raise AssertionError("airdrop branch lost the typed owner-to-base lookup")
    assignment_end = airdrop.find(";", assignment_start)
    assignment = airdrop[assignment_start:assignment_end]
    result_expression = assignment.split("=", 1)[1].strip()
    return (
        _condition(terraform, "xenoempathy_dome", 0),
        _condition(terraform, "xenoempathy_dome", 1),
        _condition(terraform, "weather_paradigm", 0),
        _condition(terraform, "weather_paradigm", 1),
        f"({project_condition}) && ({result_expression})",
    )


def _host_source(conditions: tuple[str, ...]) -> str:
    type_source = BASE_H.read_text()
    project = _definition(type_source, "struct BaseSecretProject")
    wrappers = "\n".join(
        f"static bool actual_condition_{index}({parameters}) {{ return {condition}; }}"
        for index, (parameters, condition) in enumerate(zip(
            ("uint8_t v_faction", "uint8_t my_faction", "uint8_t v_faction",
             "uint8_t my_faction", "int faction_id"),
            conditions,
            strict=True,
        ))
    )
    return r"""
#include <assert.h>
#include <stddef.h>
#include <stdint.h>
enum { MaxSecretProjectNum = 64 };
""" + project + ";\n" + r"""
struct FixtureFields { BaseSecretProject secret_project_; };
struct FixtureWindow { FixtureFields fields_; };
static FixtureWindow GameDataStateStorage;
static FixtureWindow AlternateState;
struct FixtureBase { uint8_t faction_id_current; };
static FixtureBase Bases[512];
""" + wrappers + r"""
int main() {
    assert(sizeof(GameDataStateStorage.fields_.secret_project_) == 0x100);
    assert((reinterpret_cast<char *>(&GameDataStateStorage.fields_.secret_project_.weather_paradigm())
            - reinterpret_cast<char *>(&GameDataStateStorage.fields_.secret_project_)) == 0x08);
    assert((reinterpret_cast<char *>(&GameDataStateStorage.fields_.secret_project_.xenoempathy_dome())
            - reinterpret_cast<char *>(&GameDataStateStorage.fields_.secret_project_)) == 0x20);
    assert((reinterpret_cast<char *>(&GameDataStateStorage.fields_.secret_project_.space_elevator())
            - reinterpret_cast<char *>(&GameDataStateStorage.fields_.secret_project_)) == 0x6C);

    GameDataStateStorage.fields_.secret_project_.xenoempathy_dome() = 7;
    GameDataStateStorage.fields_.secret_project_.weather_paradigm() = 8;
    GameDataStateStorage.fields_.secret_project_.space_elevator() = 9;
    AlternateState.fields_.secret_project_.xenoempathy_dome() = 10;
    AlternateState.fields_.secret_project_.weather_paradigm() = 11;
    AlternateState.fields_.secret_project_.space_elevator() = 12;
    Bases[7].faction_id_current = 4;
    Bases[8].faction_id_current = 5;
    Bases[9].faction_id_current = 2;
    Bases[10].faction_id_current = 6;
    Bases[11].faction_id_current = 7;
    Bases[12].faction_id_current = 3;
    assert(actual_condition_0(4));
    assert(actual_condition_1(4));
    assert(!actual_condition_0(5));
    assert(!actual_condition_1(5));
    assert(actual_condition_2(5));
    assert(actual_condition_3(5));
    assert(!actual_condition_2(4));
    assert(!actual_condition_3(4));
    assert(actual_condition_4(2));
    assert(!actual_condition_4(3));

    GameDataStateStorage.fields_.secret_project_.xenoempathy_dome() = -1;
    GameDataStateStorage.fields_.secret_project_.weather_paradigm() = -1;
    GameDataStateStorage.fields_.secret_project_.space_elevator() = -1;
    assert(!actual_condition_0(4) && !actual_condition_1(4));
    assert(!actual_condition_2(5) && !actual_condition_3(5));
    assert(!actual_condition_4(2));
}
"""


def _compile_and_run(tmp_path: Path, conditions: tuple[str, ...], *, expect_pass: bool) -> None:
    compiler = shutil.which(os.environ.get("CXX", "g++"))
    assert compiler, "host C++ compiler unavailable"
    source = tmp_path / "veh_secret_project_owner.cpp"
    binary = tmp_path / "veh_secret_project_owner"
    source.write_text(_host_source(conditions))
    subprocess.run(
        [compiler, "-std=c++11", "-Wall", "-Wextra", "-Werror", "-pedantic",
         str(source), "-o", str(binary)],
        check=True, capture_output=True, text=True,
    )
    result = subprocess.run([str(binary)], capture_output=True, text=True, timeout=10)
    if expect_pass:
        assert result.returncode == 0, result.stderr
    else:
        assert result.returncode != 0, "compiled source mutant escaped the owner control"


def test_live_vehicle_secret_project_conditions_use_typed_slots(tmp_path: Path) -> None:
    conditions = _actual_conditions()
    fixture = EXISTING_FIXTURE.read_text()
    for needle in (
        "&a.weather_paradigm() == &a[2]",
        "&a.xenoempathy_dome() == &a[8]",
        "&a.space_elevator() == &a[27]",
    ):
        assert needle in fixture
    source = VEH_CPP.read_text()
    terraform = _definition(source, "int __cdecl action_terraform(")
    airdrop = _definition(source, "void __cdecl sub_500380(")
    assert "action == TERRA_REMOVE_FUNGUS" in terraform
    assert "else if (GameDataStateStorage.fields_.secret_project_.weather_paradigm() >= 0" in terraform
    assert airdrop.index("has_tech_call(Rules.tech_orb_insert_sans_spc_elev") < airdrop.index(
        "GameDataStateStorage.fields_.secret_project_.space_elevator() >= 0"
    )
    _compile_and_run(tmp_path, conditions, expect_pass=True)


def test_live_vehicle_conditions_reject_wrong_valid_slot_and_wrong_owner(tmp_path: Path) -> None:
    conditions = _actual_conditions()
    wrong_slot = tuple(
        condition.replace("xenoempathy_dome()", "weather_paradigm()")
        if index in (0, 1) else condition
        for index, condition in enumerate(conditions)
    )
    _compile_and_run(tmp_path, wrong_slot, expect_pass=False)
    wrong_owner = tuple(
        condition.replace(
            "GameDataStateStorage.fields_.secret_project_",
            "AlternateState.fields_.secret_project_",
        )
        for condition in conditions
    )
    _compile_and_run(tmp_path, wrong_owner, expect_pass=False)
