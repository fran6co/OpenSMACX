"""Actual-source controls for the evidenced vehicle human/alive byte views."""
from pathlib import Path
import re
import shutil
import subprocess

import pytest


ROOT = Path(__file__).resolve().parents[2]
SOURCE = ROOT / "src/veh.cpp"
MASK_HEADER = ROOT / "src/game_data_state_window.h"
LOCK = "GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_"
ALIVE = "GameDataStateStorage.fields_.lock_masks_.bytes_.alive_mask_"
DWORD = "GameDataStateStorage.fields_.lock_masks_.enable_mask_"
COMPILERS = [p for p in (shutil.which("g++"), shutil.which("clang++")) if p]


def mask_view_declarations():
    text = MASK_HEADER.read_text()
    declarations = []
    for name in ("GameDataLockMaskBytes", "GameDataLockMaskView"):
        match = re.search(rf"(?:struct|union) {name} \{{.*?^\}};", text, re.M | re.S)
        assert match, name
        declarations.append(match.group(0))
    return "\n".join(declarations)


GATES = {
    "name_proto": (
        "void __cdecl name_proto(char *output, int prototype_id, int faction_id,",
        f"{LOCK} & (1u << (faction_id & 0x1f))",
    ),
    "action_airdrop": (
        "int __cdecl action_airdrop(int veh_id, int x, int y, int mode)",
        f"{LOCK} & (1U << faction)",
    ),
    "stack_veh": (
        "int __cdecl stack_veh(int veh_id, int mode)",
        f"(1 << unit_faction) & {LOCK}",
    ),
    "promote": (
        "void __cdecl promote(int veh_id)",
        f"(1 << Vehs[veh_id].faction_id) & {LOCK}",
    ),
    "action_go_to": (
        "void __cdecl action_go_to(int veh_id)",
        f"{LOCK} & (1 << Vehs[veh_id].faction_id)",
    ),
    "action_give": (
        "void __cdecl action_give(int vehicle_id, int faction_id)",
        f"{LOCK} & (1u << (original_owner & 0x1f))",
    ),
    "shoot_it": (
        "int __cdecl shoot_it(int attacker, int defender, int x, int y,",
        f"{LOCK} & (1u << (defender & 0x1f))",
    ),
}


def function_body(text: str, signature: str) -> str:
    start = text.index(signature)
    opening = text.index("{", start)
    depth = 0
    state = "code"
    quote = ""
    escaped = False
    i = opening
    while i < len(text):
        ch = text[i]
        nxt = text[i + 1] if i + 1 < len(text) else ""
        if state == "line_comment":
            if ch == "\n":
                state = "code"
        elif state == "block_comment":
            if ch == "*" and nxt == "/":
                state = "code"
                i += 1
        elif state == "string":
            if escaped:
                escaped = False
            elif ch == "\\":
                escaped = True
            elif ch == quote:
                state = "code"
        else:
            if ch == "/" and nxt == "/":
                state = "line_comment"
                i += 1
            elif ch == "/" and nxt == "*":
                state = "block_comment"
                i += 1
            elif ch in ("'", '"'):
                state, quote = "string", ch
            elif ch == "{":
                depth += 1
            elif ch == "}":
                depth -= 1
                if depth == 0:
                    return text[start:i + 1]
        i += 1
    raise AssertionError(f"unbalanced body: {signature}")


def compact(text: str) -> str:
    return re.sub(r"\s+", " ", text).strip()


def source_expressions(mutant=None):
    text = SOURCE.read_text()
    expressions = {}
    for name, (signature, needle) in GATES.items():
        body = compact(function_body(text, signature))
        assert body.count(needle) == 1, (name, body.count(needle))
        assert body.count(LOCK) == 1, (name, body.count(LOCK))
        expression = needle
        if mutant == name:
            expression = expression.replace(LOCK, DWORD)
        # The actual mask expression is kept intact. Only its surrounding
        # owner/member lookup is adapted to a scalar test input.
        expression = expression.replace("Vehs[veh_id].faction_id", "faction_id")
        expressions[name] = expression
    return expressions


def cxx_harness(expressions):
    functions = []
    checks = []
    for index, (name, expression) in enumerate(expressions.items()):
        functions.append(
            f"int gate_{index}(int faction_id, unsigned int unit_faction, "
            f"int faction, int original_owner, int defender) "
            f"{{ return (({expression}) != 0); }}"
        )
        checks.append(f"assert(gate_{index}(24, 24u, 24, 24, 24) == 0);")
        checks.append(f"assert(gate_{index}(3, 3u, 3, 3, 3) == 1);")
    return r'''#include <cassert>
#include <stdint.h>
#include <stddef.h>
MASK_DECLARATIONS
struct GameDataFields { GameDataLockMaskView lock_masks_; };
struct GameDataState { GameDataFields fields_; };
GameDataState GameDataStateStorage = {};
FUNCTIONS
int main() {
  // A high-byte-only value distinguishes a byte human read from a dword read.
  GameDataStateStorage.fields_.lock_masks_.enable_mask_ = 1u << 24;
  CHECKS_HIGH
  GameDataStateStorage.fields_.lock_masks_.enable_mask_ = 1u << 3;
  CHECKS_TRUE
  GameDataStateStorage.fields_.lock_masks_.enable_mask_ = 0;
  CHECKS_FALSE
  return 0;
}
'''.replace("MASK_DECLARATIONS", mask_view_declarations()) \
    .replace("FUNCTIONS", "\n".join(functions)) \
    .replace("CHECKS_FALSE", "\n  ".join(
        check.replace("== 1", "== 0") for check in checks if "== 1" in check
    )) \
    .replace("CHECKS_TRUE", "\n  ".join(check for check in checks if "== 1" in check)) \
    .replace("CHECKS_HIGH", "\n  ".join(check for check in checks if "== 0" in check))


def compile_run(tmp_path, compiler, text):
    cpp = tmp_path / "vehicle-mask-members.cpp"
    exe = tmp_path / "vehicle-mask-members"
    cpp.write_text(text)
    build = subprocess.run(
        [compiler, "-std=c++17", "-Wall", "-Wextra", "-Werror",
         "-Wno-unused-parameter", str(cpp), "-o", str(exe)],
        capture_output=True, text=True, timeout=30,
    )
    if build.returncode:
        return build, None
    return build, subprocess.run([str(exe)], capture_output=True,
                                 text=True, timeout=10)


def test_seven_human_gates_use_the_byte_member_without_changing_shift_forms():
    expressions = source_expressions()
    assert set(expressions) == set(GATES)
    source = SOURCE.read_text()
    goody = compact(function_body(source, "int __cdecl goody_box(int veh_id, int type)"))
    assert f"const uint32_t live_players = {ALIVE};" in goody
    # This separate low-byte projection is intentionally outside this task.
    assert f"const uint32_t human_players = {DWORD} & 0xFFu;" in goody
    assert f"const uint32_t live_players = ({DWORD} >> 8) & 0xFFu;" not in goody


@pytest.mark.parametrize("compiler", COMPILERS)
def test_actual_human_gate_expressions_match_byte_storage(compiler, tmp_path):
    build, run = compile_run(tmp_path, compiler, cxx_harness(source_expressions()))
    assert build.returncode == 0, build.stderr
    assert run.returncode == 0, run.stderr


@pytest.mark.parametrize("compiler", COMPILERS)
def test_positive_control_rejects_always_false_gates(compiler, tmp_path):
    expressions = {name: "0" for name in source_expressions()}
    build, run = compile_run(tmp_path, compiler, cxx_harness(expressions))
    assert build.returncode == 0, build.stderr
    assert run.returncode != 0, "low-byte positive controls did not execute"


@pytest.mark.parametrize("compiler", COMPILERS)
@pytest.mark.parametrize("mutant", GATES)
def test_each_dword_human_gate_mutant_fails_independently(compiler, mutant, tmp_path):
    build, run = compile_run(
        tmp_path, compiler, cxx_harness(source_expressions(mutant=mutant))
    )
    assert build.returncode == 0, build.stderr
    assert run.returncode != 0, f"dword mutant escaped {mutant} byte-width control"


def test_goody_box_reads_the_alive_byte_and_preserves_human_projection(tmp_path):
    text = SOURCE.read_text()
    body = compact(function_body(text, "int __cdecl goody_box(int veh_id, int type)"))
    expected = f"const uint32_t live_players = {ALIVE};"
    old_width = f"const uint32_t live_players = ({DWORD} >> 8) & 0xFFu;"
    human_projection = f"const uint32_t human_players = {DWORD} & 0xFFu;"
    assert body.count(expected) == 1
    assert old_width not in body
    assert body.count(human_projection) == 1

    # Both source spellings project the same value, so the width control here
    # is structural: compile the actual source-selected byte member and verify
    # its offset/value, while the source check above rejects the dword form.
    cpp = r'''#include <cassert>
#include <stddef.h>
#include <stdint.h>
MASK_DECLARATIONS
struct Fields { GameDataLockMaskView lock_masks_; };
struct State { Fields fields_; };
State GameDataStateStorage = {};
int main() {
  static_assert(offsetof(GameDataLockMaskBytes, alive_mask_) == 1, "alive byte offset");
  GameDataStateStorage.fields_.lock_masks_.enable_mask_ = 0xA5C3B27Fu;
  const uint32_t live_players = ALIVE_EXPR;
  assert(live_players == 0xB2u);
}
'''
    assert "bytes_.alive_mask_" in expected
    expression = expected.split(" = ", 1)[1].rstrip(";")
    cpp = cpp.replace("MASK_DECLARATIONS", mask_view_declarations()).replace("ALIVE_EXPR", expression)
    for compiler in COMPILERS:
        build, run = compile_run(tmp_path, compiler, cpp)
        assert build.returncode == 0, build.stderr
        assert run.returncode == 0, run.stderr


@pytest.mark.parametrize("compiler", COMPILERS)
def test_goody_box_wrong_alive_byte_mutant_is_detected(compiler, tmp_path):
    text = SOURCE.read_text()
    body = compact(function_body(text, "int __cdecl goody_box(int veh_id, int type)"))
    expected = f"const uint32_t live_players = {ALIVE};"
    assert body.count(expected) == 1
    mutant = expected.replace("bytes_.alive_mask_", "bytes_.human_mask_")
    body = body.replace(expected, mutant)
    expression = mutant.split(" = ", 1)[1].rstrip(";")
    cpp = r'''#include <cassert>
#include <stddef.h>
#include <stdint.h>
MASK_DECLARATIONS
struct Fields { GameDataLockMaskView lock_masks_; };
struct State { Fields fields_; };
State GameDataStateStorage = {};
int main() {
  GameDataStateStorage.fields_.lock_masks_.enable_mask_ = 0xA5C3B27Fu;
  const uint32_t live_players = ALIVE_EXPR;
  assert(live_players == 0xB2u);
}
'''.replace("MASK_DECLARATIONS", mask_view_declarations()).replace("ALIVE_EXPR", expression)
    build, run = compile_run(tmp_path, compiler, cpp)
    assert build.returncode == 0, build.stderr
    assert run.returncode != 0, "wrong alive-byte mutant escaped"
