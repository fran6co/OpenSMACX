"""Compile MENU/council/report pointer callers from installed source.

The pointer-only StringTable mock checks type enforcement. A separate,
type-correct wrong-slot mutation checks only caller expression behavior; this
small host fixture does not model complete menu runtime behavior.
"""

from __future__ import annotations

from pathlib import Path
import shutil
import subprocess

import pytest


ROOT = Path(__file__).resolve().parents[2]
CALLER_FILES = (
    "src/alphamenu.cpp",
    "src/council.cpp",
    "src/councwin.cpp",
    "src/famewin.cpp",
    "src/picktech.cpp",
    "src/prodpicker.cpp",
    "src/reportif.cpp",
    "src/monuwin.cpp",
)


def get_arguments(source: str) -> list[str]:
    """Return the actual balanced argument expressions to StringTable.get."""
    marker = "StringTable.get("
    arguments: list[str] = []
    cursor = 0
    while True:
        start = source.find(marker, cursor)
        if start < 0:
            return arguments
        opening = start + len(marker) - 1
        depth = 1
        position = opening + 1
        while position < len(source) and depth:
            if source[position] == "(":
                depth += 1
            elif source[position] == ")":
                depth -= 1
            position += 1
        assert depth == 0, "unclosed StringTable.get call in product source"
        arguments.append(source[opening + 1 : position - 1].strip())
        cursor = position


def actual_arguments() -> list[str]:
    arguments: list[str] = []
    for relative in CALLER_FILES:
        arguments.extend(get_arguments((ROOT / relative).read_text(encoding="utf-8")))
    assert arguments
    return arguments


PRELUDE = r"""
#include <cassert>
#include <cstdint>
#include <cstring>
typedef char *LPSTR;
struct Strings { LPSTR get(LPSTR value) const { return value; } };
static Strings StringTable;
struct OrderRow { LPSTR order; LPSTR order_sea; };
struct TerraformRow { LPSTR shortcuts; };
struct ProposalRow { LPSTR name; LPSTR description; };
struct FacilityRow { LPSTR name; LPSTR effect; };
struct MandateRow { LPSTR name_caps; };
static OrderRow Order[32];
static TerraformRow Terraforming[32];
static ProposalRow Proposal[16];
static FacilityRow Facility[128];
static MandateRow Mandate[32];
static LPSTR Difficulty[16];
static LPSTR Triad[16];
static LPSTR order_name;
static LPSTR proposal_text, proposal_description, address;
static int i, terraform_id, proposal, facility, facilityId, kind, category;
static int FacilitySPStart;
struct Record {
  int word_8_; int field_20;
  Record *operator->() { return this; }
};
static Record record;
struct Rule { int triad; };
static Rule rule;
static const int FAC_ASCENT_TO_TRANSCENDENCE = 1;
"""


def translation_unit(arguments: list[str], *, mutation: str | None = None) -> str:
    expressions = list(arguments)
    if mutation == "integer_narrowing":
        original = "Order[i + 4].order"
        assert original in expressions
        expressions[expressions.index(original)] = (
            "static_cast<int>(reinterpret_cast<intptr_t>(Order[i + 4].order))"
        )
    elif mutation == "wrong_valid_slot":
        original = "Order[i + 4].order"
        assert original in expressions
        expressions[expressions.index(original)] = "Order[i + 5].order"
    elif mutation is not None:
        raise AssertionError(f"unknown caller mutation: {mutation}")
    wrappers = "\n".join(
        f"LPSTR caller_{index}() {{ return StringTable.get({expression}); }}"
        for index, expression in enumerate(expressions)
    )
    checks = "\n".join(
        f"assert(caller_{index}() == expected);" for index in range(len(expressions))
    )
    main = r"""
static char expected_text[] = "the original rule text";
static char different_text[] = "a different valid rule slot";
LPSTR expected = expected_text;
int main() {
for (int slot = 0; slot != 32; ++slot) {
  Order[slot].order = expected; Order[slot].order_sea = expected;
  Terraforming[slot].shortcuts = expected; Mandate[slot].name_caps = expected;
}
for (int slot = 0; slot != 16; ++slot) {
  Proposal[slot].name = expected; Proposal[slot].description = expected;
  Difficulty[slot] = expected; Triad[slot] = expected;
}
for (int slot = 0; slot != 128; ++slot) {
  Facility[slot].name = expected; Facility[slot].effect = expected;
}
Order[5].order = different_text;
order_name = expected; i = 0; terraform_id = 0; proposal = 0;
proposal_text = expected; proposal_description = expected; address = expected;
facility = 0; facilityId = 0; kind = 0; category = 0;
FacilitySPStart = 0; record.word_8_ = 0; record.field_20 = 0; rule.triad = 0;
CHECKS
assert(std::strcmp(caller_0(), "the original rule text") == 0);
}
""".replace("CHECKS", checks)
    return PRELUDE + "\n" + wrappers + "\n" + main


COMPILERS = [compiler for compiler in ("g++", "clang++") if shutil.which(compiler)]
pytestmark = pytest.mark.skipif(not COMPILERS, reason="no host C++ compiler available")


@pytest.mark.parametrize("compiler", COMPILERS)
def test_actual_menu_caller_pointer_expressions_compile_and_keep_pointer_identity(
    compiler: str, tmp_path: Path
) -> None:
    source = translation_unit(actual_arguments())
    binary = tmp_path / "menu-pointer-control"
    built = subprocess.run(
        [shutil.which(compiler), "-std=c++11", "-Wall", "-Werror", "-x", "c++", "-", "-o", str(binary)],
        input=source,
        text=True,
        capture_output=True,
        timeout=30,
    )
    assert built.returncode == 0, built.stderr
    ran = subprocess.run([str(binary)], text=True, capture_output=True, timeout=10)
    assert ran.returncode == 0, ran.stderr


@pytest.mark.parametrize("compiler", COMPILERS)
def test_pointer_only_api_rejects_integer_narrowing_at_compile_time(
    compiler: str, tmp_path: Path
) -> None:
    source = translation_unit(actual_arguments(), mutation="integer_narrowing")
    built = subprocess.run(
        [shutil.which(compiler), "-std=c++11", "-Wall", "-Werror", "-x", "c++", "-", "-o", str(tmp_path / "mutant")],
        input=source,
        text=True,
        capture_output=True,
        timeout=30,
    )
    assert built.returncode != 0, "the integer-narrowing mutation compiled"
    assert "get" in built.stderr


@pytest.mark.parametrize("compiler", COMPILERS)
def test_wrong_but_valid_rule_slot_compiles_and_fails_pointer_behavior_check(
    compiler: str, tmp_path: Path
) -> None:
    binary = tmp_path / "wrong-slot-mutant"
    built = subprocess.run(
        [shutil.which(compiler), "-std=c++11", "-Wall", "-Werror", "-x", "c++", "-", "-o", str(binary)],
        input=translation_unit(actual_arguments(), mutation="wrong_valid_slot"),
        text=True,
        capture_output=True,
        timeout=30,
    )
    assert built.returncode == 0, built.stderr
    ran = subprocess.run([str(binary)], text=True, capture_output=True, timeout=10)
    assert ran.returncode != 0, "the valid wrong-slot mutation preserved caller behavior"
