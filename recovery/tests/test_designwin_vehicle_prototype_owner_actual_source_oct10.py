"""Bounded controls for DesignWin's three source-owned prototype-ID reads."""

from __future__ import annotations

import shutil
import subprocess
from pathlib import Path

import pytest


ROOT = Path(__file__).resolve().parents[2]
DESIGNWIN = ROOT / "src/designwin.cpp"
VEH_H = ROOT / "src/veh.h"


def extract_definition(text: str, signature: str) -> str:
    start = text.index(signature)
    opening = text.index("{", start)
    depth = 0
    for offset in range(opening, len(text)):
        if text[offset] == "{":
            depth += 1
        elif text[offset] == "}":
            depth -= 1
            if depth == 0:
                return text[start : offset + 1]
    raise AssertionError(f"unterminated definition: {signature}")


def source_statements() -> tuple[str, str, str]:
    text = DESIGNWIN.read_text(encoding="utf-8")
    stack = extract_definition(text, "void DesignWin::draw_stack()")
    flash = extract_definition(text, "void DesignWin::draw_flash()")

    stack_call = "say_stats_3(StringTemp, Vehs[unit_id].proto_id);"
    assert stack.count(stack_call) == 1
    assert stack.count("Vehs[") == 1

    read = "int prototype_id = Vehs[shown].proto_id;"
    assert flash.count(read) == 2
    name_start = flash.index(read)
    name_end = flash.index(
        "strcat(StringTemp, VehPrototypes[prototype_id].veh_name);", name_start
    ) + len("strcat(StringTemp, VehPrototypes[prototype_id].veh_name);")
    name_statements = flash[name_start:name_end]

    stats_start = flash.index(read, name_end)
    stats_call = "say_stats_3(StringTemp, prototype_id);"
    stats_end = flash.index(stats_call, stats_start) + len(stats_call)
    stats_statements = flash[stats_start:stats_end]
    assert flash.count("Vehs[") == 2
    assert "VehPrototypes[prototype_id].ability_flags" in flash
    assert "ProtoAbilityIdBase" not in text
    return stack_call, name_statements, stats_statements


def veh_definition() -> str:
    header = VEH_H.read_text(encoding="utf-8")
    start = header.index("struct Veh {")
    opening = header.index("{", start)
    depth = 0
    for offset in range(opening, len(header)):
        if header[offset] == "{":
            depth += 1
        elif header[offset] == "}":
            depth -= 1
            if depth == 0:
                return header[start : offset + 1] + ";"
    raise AssertionError("unterminated Veh definition")


HARNESS = r'''
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstring>

__VEH_DEFINITION__
static_assert(offsetof(Veh, proto_id) == 0x0A, "Veh::proto_id offset");
static_assert(sizeof(Veh) == 0x34, "Veh row stride");

struct VehPrototype { const char *veh_name; unsigned int ability_flags; };
Veh Vehs[32] = {};
VehPrototype VehPrototypes[32] = {};
char StringTemp[128] = {};
int observed_stats = 0;

void say_stats_3(char *, int prototype_id) { observed_stats = prototype_id; }

void draw_stack_statement(int unit_id) {
    __STACK_STATEMENT__
}

void draw_flash_name_statement(int shown) {
    __NAME_STATEMENTS__
}

void draw_flash_stats_statement(int shown) {
    __STATS_STATEMENTS__
}

int main() {
    // Distinct live rows catch confusing the returned unit with its proto id.
    Vehs[3].proto_id = 6;
    Vehs[6].proto_id = 14;
    draw_stack_statement(3);
    assert(observed_stats == 6);

    // The display row selects a prototype through its own signed member.
    Vehs[11].proto_id = 4;
    VehPrototypes[4].veh_name = "four";
    VehPrototypes[11].veh_name = "eleven";
    StringTemp[0] = '\0';
    draw_flash_name_statement(11);
    assert(std::strcmp(StringTemp, "four") == 0);

    // int16_t promotes with sign, not as an unsigned prototype index.
    Vehs[13].proto_id = -2;
    draw_flash_stats_statement(13);
    assert(observed_stats == -2);
    return 0;
}
'''


def compile_and_run(tmp_path: Path, statements: tuple[str, str, str]):
    compiler = shutil.which("c++") or shutil.which("clang++") or shutil.which("g++")
    assert compiler, "a host C++ compiler is required for actual-source controls"
    stack, name, stats = statements
    unit = tmp_path / "designwin_vehicle_prototype_actual.cpp"
    binary = tmp_path / "designwin_vehicle_prototype_actual"
    source = (HARNESS.replace("__VEH_DEFINITION__", veh_definition())
              .replace("__STACK_STATEMENT__", stack)
              .replace("__NAME_STATEMENTS__", name)
              .replace("__STATS_STATEMENTS__", stats))
    unit.write_text(source, encoding="utf-8")
    compiled = subprocess.run(
        [compiler, "-std=c++17", "-O0", str(unit), "-o", str(binary)],
        text=True,
        capture_output=True,
        check=False,
    )
    assert compiled.returncode == 0, compiled.stdout + compiled.stderr
    return subprocess.run([str(binary)], text=True, capture_output=True, check=False)


def test_actual_owner_reads_and_consumers_use_signed_veh_member(tmp_path: Path):
    result = compile_and_run(tmp_path, source_statements())
    assert result.returncode == 0, result.stdout + result.stderr


@pytest.mark.parametrize(
    ("label", "mutate"),
    [
        (
            "wrong stack row",
            lambda s: (s[0].replace("Vehs[unit_id].proto_id",
                                    "Vehs[unit_id + 1].proto_id"), s[1], s[2]),
        ),
        (
            "name indexes the display row instead of its stored prototype id",
            lambda s: (s[0], s[1].replace("VehPrototypes[prototype_id].veh_name",
                                           "VehPrototypes[shown].veh_name"), s[2]),
        ),
        (
            "unsigned short promotion",
            lambda s: (s[0], s[1], s[2].replace(
                "say_stats_3(StringTemp, prototype_id);",
                "say_stats_3(StringTemp, static_cast<uint16_t>(prototype_id));")),
        ),
    ],
)
def test_compiling_source_mutants_are_rejected(tmp_path: Path, label, mutate):
    actual = source_statements()
    mutant = mutate(actual)
    assert mutant != actual, f"mutant did not change source statements: {label}"
    result = compile_and_run(tmp_path, mutant)
    assert result.returncode != 0, f"behavioral control accepted mutant: {label}"
