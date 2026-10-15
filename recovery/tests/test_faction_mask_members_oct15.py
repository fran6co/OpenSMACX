"""Scoped actual-expression controls for the faction lock-mask byte views.

The generated host program executes the extracted mask declarations and the
gate expressions from the three recovered bodies. It tests byte selection and
the source-level points where independent gates are reevaluated; it is not a
whole-function behavioral harness for diplomacy or social adoption.
"""
from pathlib import Path
import re
import shutil
import subprocess

import pytest


ROOT = Path(__file__).resolve().parents[2]
SOURCE = ROOT / "src/faction.cpp"
MASK_HEADER = ROOT / "src/game_data_state_window.h"
COMPILERS = [p for p in (shutil.which("g++"), shutil.which("clang++")) if p]


def body(text: str, signature: str) -> str:
    start = text.index(signature)
    opening = text.index("{", start)
    depth = 0
    for end in range(opening, len(text)):
        depth += (text[end] == "{") - (text[end] == "}")
        if depth == 0:
            return text[start:end + 1]
    raise AssertionError(f"unterminated body: {signature}")


SOCIAL_SIGNATURE = "void __cdecl social_set(int faction_id)"
ATTEMPT_SIGNATURE = "void __cdecl commlink_attempt(int target_faction)"
ATTER_SIGNATURE = "int __cdecl commlink_attempter(int faction_id, int contact_flags)"


def extracted(mutant=None):
    source = SOURCE.read_text()
    social = body(source, SOCIAL_SIGNATURE)
    attempt = body(source, ATTEMPT_SIGNATURE)
    atter = body(source, ATTER_SIGNATURE)
    header = MASK_HEADER.read_text()

    mask_bytes = re.search(
        r"struct GameDataLockMaskBytes\s*\{.*?\};\s*union GameDataLockMaskView\s*\{.*?\};",
        header, re.S,
    )
    assert mask_bytes, "source-owned lock-mask byte view disappeared"

    social_match = re.search(
        r"\(GameDataStateStorage\.fields_\.lock_masks_\.(?:bytes_\.human_mask_|enable_mask_)\s*&\s*\(1u\s*<<\s*faction_id\)\)\s*!=\s*0",
        social,
    )
    assert social_match, "social_set native human gate not found"

    alive_init = re.search(
        r"const uint32_t alive_mask\s*=\s*([^;]+);", attempt
    )
    target_init = re.search(
        r"const uint32_t target_bit\s*=\s*([^;]+);", attempt
    )
    alive_test = re.search(r"if\s*\(\(alive_mask\s*&\s*target_bit\)\s*==\s*0\)", attempt)
    assert alive_init and target_init and alive_test
    assert "bytes_.alive_mask_" in alive_init.group(1)

    human_tests = re.findall(
        r"\(GameDataStateStorage\.fields_\.lock_masks_\.(?:bytes_\.human_mask_|enable_mask_)\s*&\s*target_bit\)\s*!=\s*0",
        attempt,
    )
    assert len(human_tests) == 2, "both distinct live human-mask tests must remain"

    atter_test = re.search(
        r"\(GameDataStateStorage\.fields_\.lock_masks_\.(?:bytes_\.alive_mask_|enable_mask_)\s*&\s*\(1u\s*<<\s*\(other\s*&\s*31\)\)\)\s*!=\s*0",
        atter,
    )
    assert atter_test, "vendetta alive-byte gate or masked bit index changed"

    # Mutants target each native byte operand independently. For legal faction
    # bit indices, the old dword+shift forms can be mathematically equivalent;
    # those are checked as source-form regressions below rather than dressed up
    # as behavioral mutants.
    social_expr = social_match.group(0)
    alive_expr = alive_init.group(1)
    human_expr_1, human_expr_2 = human_tests
    atter_expr = atter_test.group(0)
    if mutant == "social_wrong_byte":
        social_expr = social_expr.replace("bytes_.human_mask_", "bytes_.alive_mask_")
    elif mutant == "social_wide":
        social_expr = social_expr.replace("bytes_.human_mask_", "enable_mask_")
    elif mutant == "attempt_alive_wrong_byte":
        alive_expr = alive_expr.replace("bytes_.alive_mask_", "bytes_.human_mask_")
    elif mutant == "attempt_human_first_wrong_byte":
        human_expr_1 = human_expr_1.replace("bytes_.human_mask_", "bytes_.alive_mask_")
    elif mutant == "attempt_human_second_wrong_byte":
        human_expr_2 = human_expr_2.replace("bytes_.human_mask_", "bytes_.alive_mask_")
    elif mutant == "attempt_human_first_wide":
        human_expr_1 = human_expr_1.replace("bytes_.human_mask_", "enable_mask_")
    elif mutant == "attempt_human_second_wide":
        human_expr_2 = human_expr_2.replace("bytes_.human_mask_", "enable_mask_")
    elif mutant == "atter_alive_wrong_byte":
        atter_expr = atter_expr.replace("bytes_.alive_mask_", "bytes_.human_mask_")
    elif mutant == "atter_old_shift":
        atter_expr = atter_expr.replace(
            "bytes_.alive_mask_ & (1u << (other & 31))",
            "enable_mask_ & (1u << ((other + 8) & 31))",
        )

    return {
        "mask_decl": mask_bytes.group(0),
        "social": social,
        "attempt": attempt,
        "atter": atter,
        "social_expr": social_expr,
        "alive_expr": alive_expr,
        "target_expr": target_init.group(1),
        "alive_test": alive_test.group(0),
        "human_expr_1": human_expr_1,
        "human_expr_2": human_expr_2,
        "atter_expr": atter_expr,
    }


HARNESS = r'''#include <cassert>
#include <cstdint>
#define __cdecl
MASK_DECL
struct Fields { GameDataLockMaskView lock_masks_; };
struct Storage { Fields fields_; };
Storage GameDataStateStorage = {};

bool social_gate(int faction_id) { return SOCIAL_EXPR; }
bool attempt_alive_gate(int target_faction) {
  const uint32_t alive_mask = ALIVE_EXPR;
  const uint32_t target_bit = TARGET_EXPR;
  if ((alive_mask & target_bit) == 0) return false;
  return true;
}
bool attempt_human_first(uint32_t target_bit) { return HUMAN_FIRST; }
bool attempt_human_second(uint32_t target_bit) { return HUMAN_SECOND; }
bool atter_alive_gate(int other) { return ATTER_EXPR; }

int main() {
  auto &mask = GameDataStateStorage.fields_.lock_masks_;
  // social_set reevaluates the human byte in the base loop. A callback after
  // the first accepted row changes it; the next iteration sees the new byte.
  mask.enable_mask_ = 1u << 2;
  assert(social_gate(2));
  mask.enable_mask_ = 1u << (8 + 2);
  assert(!social_gate(2));
  mask.enable_mask_ = (1u << 2) | (1u << (8 + 2));
  assert(social_gate(2));

  // Expression-only boundary control: these extracted gates do not index
  // Players/Bases, so bit position 24 safely distinguishes byte from dword.
  mask.enable_mask_ = 1u << 24;
  assert(!social_gate(24));

  // commlink_attempt's initial alive gate reads byte one, while its later
  // human gates independently read byte zero.
  mask.enable_mask_ = 1u << (8 + 2);
  assert(attempt_alive_gate(2));
  assert(!attempt_human_first(1u << 2));
  mask.enable_mask_ = 1u << 2;
  assert(!attempt_alive_gate(2));
  assert(attempt_human_first(1u << 2));
  // The second human test is a separate live source read after intervening
  // diplomacy/network work; changing the byte between calls is observable.
  mask.enable_mask_ = 1u << 2;
  assert(attempt_human_second(1u << 2));
  mask.enable_mask_ = 0;
  assert(!attempt_human_second(1u << 2));

  mask.enable_mask_ = 1u << 24;
  assert(!attempt_human_first(1u << 24));
  assert(!attempt_human_second(1u << 24));

  // Vendetta's "not met" choice uses the alive byte with a faction-relative
  // bit index (not a human-byte index plus eight).
  mask.enable_mask_ = 1u << (8 + 3);
  assert(atter_alive_gate(3));
  mask.enable_mask_ = 1u << 3;
  assert(!atter_alive_gate(3));
  // The old `(other + 8) & 31` dword expression aliases bit 24 to bit 0.
  mask.enable_mask_ = 1u;
  assert(!atter_alive_gate(24));
  return 0;
}
'''


def program(mutant=None):
    x = extracted(mutant)
    return (HARNESS.replace("MASK_DECL", x["mask_decl"])
            .replace("SOCIAL_EXPR", x["social_expr"])
            .replace("ALIVE_EXPR", x["alive_expr"])
            .replace("TARGET_EXPR", x["target_expr"])
            .replace("HUMAN_FIRST", x["human_expr_1"])
            .replace("HUMAN_SECOND", x["human_expr_2"])
            .replace("ATTER_EXPR", x["atter_expr"]))


def compile_run(tmp_path, compiler, text):
    cpp = tmp_path / "faction-mask-members.cpp"
    exe = tmp_path / "faction-mask-members"
    cpp.write_text(text)
    build = subprocess.run(
        [compiler, "-std=c++17", "-Wall", "-Wextra", "-Werror", "-Wno-unused-parameter",
         str(cpp), "-o", str(exe)], capture_output=True, text=True, timeout=30,
    )
    if build.returncode:
        return build, None
    return build, subprocess.run([str(exe)], capture_output=True, text=True, timeout=10)


def test_only_authorized_mask_gates_use_native_byte_members_and_keep_reload_stages():
    x = extracted()
    assert x["social"].count("bytes_.human_mask_") == 1
    assert x["attempt"].count("bytes_.human_mask_") == 2
    assert x["attempt"].count("bytes_.alive_mask_") == 1
    assert x["atter"].count("bytes_.alive_mask_") == 1
    assert "enable_mask_" not in x["social"]
    assert "enable_mask_" not in x["attempt"]
    assert "enable_mask_" not in x["atter"]
    # The social mask access remains inside the base loop, after each base's
    # cost updates; it is not hoisted across loop iterations.
    loop = x["social"].index("for (int base_id")
    assert loop < x["social"].index("bytes_.human_mask_") < x["social"].index("\n    }\n\n    if (faction_id")
    # Both commlink human reads remain distinct and follow the attempter call.
    assert x["attempt"].index("commlink_attempter(") < x["attempt"].index("bytes_.human_mask_")
    assert x["attempt"].count("target_bit) != 0") == 2
    assert "other & 31" in x["atter"] and "other + 8" not in x["atter"]


@pytest.mark.parametrize("compiler", COMPILERS)
def test_actual_mask_expressions_and_live_gate_rereads(compiler, tmp_path):
    build, run = compile_run(tmp_path, compiler, program())
    assert build.returncode == 0, build.stderr
    assert run.returncode == 0, run.stderr


@pytest.mark.parametrize("compiler", COMPILERS)
@pytest.mark.parametrize("mutant", [
    "social_wrong_byte", "social_wide", "attempt_alive_wrong_byte",
    "attempt_human_first_wrong_byte", "attempt_human_second_wrong_byte",
    "attempt_human_first_wide", "attempt_human_second_wide",
    "atter_alive_wrong_byte", "atter_old_shift",
])
def test_old_or_wrong_mask_operand_mutants_compile_then_fail(compiler, mutant, tmp_path):
    build, run = compile_run(tmp_path, compiler, program(mutant))
    assert build.returncode == 0, build.stderr
    assert run.returncode != 0, f"{mutant} escaped the actual-expression controls"
