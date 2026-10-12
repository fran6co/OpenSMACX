"""Actual-source mask controls for eight landmark writers.

The bounded fixture executes the real bit2_set body and captured product
callsite arguments against one mock tile. It does not establish complete
function control flow or live-game/runtime behavior. This file is not run
until the coordinator adopts it after the active epoch.
"""
from pathlib import Path
import os
import re
import shutil
import subprocess
import sys
import tempfile

import pytest
from match_recovery.source import read_file, read_text


ROOT = Path(__file__).resolve().parents[2]
SOURCE = ROOT / "src/map.cpp"
HEADER = ROOT / "src/map.h"
BASE = "361f5176199a52ac0bc3f425cb139f0230730824"

# function: (original address, symbol suffix, BIT2 mask, Natural[] index)
WRITERS = {
    "world_monsoon": (0x005C5EF0, "?world_monsoon@@YAXHH@Z", "BIT2_JUNGLE", "LM_JUNGLE"),
    "world_dune": (0x005C6740, "?world_dune@@YAXHH@Z", "BIT2_DUNES", "LM_DUNES"),
    "world_borehole": (0x005C7020, "?world_borehole@@YAXHH@Z", "BIT2_BOREHOLE", "LM_BOREHOLE"),
    "world_temple": (0x005C7540, "?world_temple@@YAXHH@Z", "BIT2_NEXUS", "LM_NEXUS"),
    "world_unity": (0x005C7750, "?world_unity@@YAXHH@Z", "BIT2_UNITY", "LM_UNITY"),
    "world_fossil": (0x005C7A80, "?world_fossil@@YAXHH@Z", "BIT2_FOSSIL", "LM_FOSSIL"),
    "world_canyon": (0x005C7CB0, "?world_canyon@@YAXHH@Z", "BIT2_CANYON", "LM_CANYON"),
    "world_ridge": (0x005C8150, "?world_ridge@@YAXHH@Z", "BIT2_RIDGE", "LM_RIDGE"),
}
PINNED_IMMEDIATES = {
    "BIT2_JUNGLE": 0x0004,
    "BIT2_DUNES": 0x0040,
    "BIT2_BOREHOLE": 0x1000,
    "BIT2_NEXUS": 0x2000,
    "BIT2_UNITY": 0x4000,
    "BIT2_FOSSIL": 0x8000,
    "BIT2_CANYON": 0x0200,
    "BIT2_RIDGE": 0x0800,
}
NATURAL_INDICES = {
    "LM_JUNGLE": 2,
    "LM_DUNES": 6,
    "LM_BOREHOLE": 12,
    "LM_NEXUS": 13,
    "LM_UNITY": 14,
    "LM_FOSSIL": 15,
    "LM_CANYON": 9,
    "LM_RIDGE": 11,
}

BASELINE_FIELDS = (
    "address", "name", "symbol", "image_spans", "byte_exact", "semantic",
    "kind", "mode", "recipe", "exclusion", "levers", "ruled_out",
    "unrecoverable", "deferred", "source_role", "source_role_reason",
)


def mask_comments_and_literals(text):
    """Replace non-code bytes with spaces while preserving offsets/newlines."""
    chars = list(text)
    i = 0
    state = "code"
    quote = ""
    while i < len(text):
        if state == "code":
            if text.startswith("//", i):
                chars[i] = chars[i + 1] = " "
                i += 2
                state = "line"
                continue
            if text.startswith("/*", i):
                chars[i] = chars[i + 1] = " "
                i += 2
                state = "block"
                continue
            if text[i] in ("'", '"'):
                quote = text[i]
                chars[i] = " "
                i += 1
                state = "literal"
                continue
            i += 1
            continue
        if state == "line":
            if text[i] == "\n":
                state = "code"
            else:
                chars[i] = " "
            i += 1
            continue
        if state == "block":
            if text.startswith("*/", i):
                chars[i] = chars[i + 1] = " "
                i += 2
                state = "code"
            else:
                if text[i] != "\n":
                    chars[i] = " "
                i += 1
            continue
        if state == "literal":
            if text[i] == "\\":
                chars[i] = " "
                if i + 1 < len(text):
                    if text[i + 1] != "\n":
                        chars[i + 1] = " "
                    i += 2
                else:
                    i += 1
            elif text[i] == quote:
                chars[i] = " "
                i += 1
                state = "code"
            else:
                if text[i] != "\n":
                    chars[i] = " "
                i += 1
    return "".join(chars)


def function_body(source, signature):
    masked = mask_comments_and_literals(source)
    start = masked.index(signature)
    brace = masked.index("{", start)
    depth = 0
    for end in range(brace, len(masked)):
        if masked[end] == "{":
            depth += 1
        elif masked[end] == "}":
            depth -= 1
            if depth == 0:
                return source[start:end + 1]
    raise AssertionError("unterminated body: " + signature)


def split_arguments(text):
    masked = mask_comments_and_literals(text)
    result = []
    start = 0
    parens = brackets = braces = 0
    for index, char in enumerate(masked):
        if char == "(":
            parens += 1
        elif char == ")":
            parens -= 1
        elif char == "[":
            brackets += 1
        elif char == "]":
            brackets -= 1
        elif char == "{":
            braces += 1
        elif char == "}":
            braces -= 1
        elif char == "," and parens == brackets == braces == 0:
            result.append(text[start:index].strip())
            start = index + 1
    result.append(text[start:].strip())
    return result


def bit2_calls(body):
    masked = mask_comments_and_literals(body)
    calls = []
    for match in re.finditer(r"\bbit2_set\s*\(", masked):
        opening = masked.index("(", match.start())
        depth = 1
        end = opening + 1
        while end < len(masked) and depth:
            if masked[end] == "(":
                depth += 1
            elif masked[end] == ")":
                depth -= 1
            end += 1
        assert depth == 0, "unterminated bit2_set call"
        args = split_arguments(body[opening + 1:end - 1])
        assert len(args) == 4, args
        calls.append(tuple(args))
    return calls


def enum_values(header):
    names = {value[2] for value in WRITERS.values()} | {value[3] for value in WRITERS.values()}
    names |= {"BIT2_CRATER", "BIT2_UNK_80000000"}
    values = {}
    for name in names:
        match = re.search(r"\b" + re.escape(name) + r"\s*=\s*(0x[0-9A-Fa-f]+|\d+)", header)
        assert match is not None, "missing enum value " + name
        values[name] = int(match.group(1), 0)
    assert values["BIT2_FOSSIL"] == 0x8000
    assert values["LM_FOSSIL"] == 15
    assert values["BIT2_UNK_80000000"] == 0x80000000
    for _function, (_address, _symbol, mask, _landmark_id) in WRITERS.items():
        assert values[mask] == PINNED_IMMEDIATES[mask], (mask, values[mask])
    for name, value in NATURAL_INDICES.items():
        assert values[name] == value, (name, values[name])
    return values


def extracted_controls():
    source = SOURCE.read_text(encoding="utf-8")
    header = HEADER.read_text(encoding="utf-8")
    constants = enum_values(header)
    records = {row.address: row for row in read_file(SOURCE)}
    calls = []
    bodies = {}
    for function, (address, symbol, mask, landmark_id) in WRITERS.items():
        body = function_body(source, "void __cdecl " + function + "(")
        bodies[function] = body
        row = records[address]
        assert row.name == symbol
        assert row.symbol == ""
        assert row.address == address
        assert row.image_spans and row.image_spans[0][0] == address
        assert row.image_spans[0][1] > address
        assert row.byte_exact is False and row.semantic is False
        assert "Natural[" + landmark_id + "].name" in body
        actual = bit2_calls(body)
        expected_count = 2 if function == "world_borehole" else 1
        assert len(actual) == expected_count, (function, actual)
        for args in actual:
            assert args[2] == mask, (function, args[2], mask)
            assert args[3] == "true", (function, args)
            calls.append((function, mask, args))
    assert len(calls) == 9
    helper = function_body(source, "void __cdecl bit2_set(")
    assert "*field |= bit2;" in helper
    return constants, calls, helper


def test_source_masks_helper_and_all_baseline_claims_are_preserved():
    baseline = subprocess.check_output(
        ["git", "show", BASE + ":src/map.cpp"], cwd=ROOT,
    ).decode("utf-8")
    old_rows = {row.address: row for row in read_text(baseline, SOURCE)}
    current_rows = {row.address: row for row in read_file(SOURCE)}
    assert old_rows.keys() == current_rows.keys()
    for address, old in old_rows.items():
        current = current_rows[address]
        for field in BASELINE_FIELDS:
            assert getattr(current, field) == getattr(old, field), (hex(address), field)
    constants, calls, helper = extracted_controls()
    assert constants["BIT2_FOSSIL"] == 0x8000
    assert constants["LM_FOSSIL"] == 15
    assert "*field |= bit2;" in helper
    assert len(calls) == 9


PRELUDE = r'''#include <assert.h>
#include <stdio.h>
#define __cdecl
typedef unsigned int uint32_t;
typedef int BOOL;
enum { @ENUMS@ };
struct Tile { uint32_t bit2; };
static Tile test_tile;
static Tile *map_loc(int x, int y) {
    assert(x == 17 && y == 29);
    return &test_tile;
}
'''

MAIN_END = r'''
int main() {
    const uint32_t initial = 0x80000001u;
@CASES@
    return 0;
}
'''


def build_program(constants, calls, helper, overrides=None):
    overrides = overrides or {}
    enum_text = ",\n       ".join(
        name + " = " + hex(value) for name, value in constants.items()
        if name != "BIT2_UNK_80000000"
    )
    wrappers = []
    checks = []
    for index, (_function, expected_mask, args) in enumerate(calls):
        function_name = "invoke_call_%02d" % index
        expression = overrides.get(index, args[2])
        call_args = ", ".join((args[0], args[1], expression, args[3]))
        wrappers.append(
            "static void %s(int x, int y, int x_radius, int y_radius) {\n"
            "    (void)x; (void)y; (void)x_radius; (void)y_radius;\n"
            "    bit2_set(%s);\n}\n" % (function_name, call_args)
        )
        target = constants[expected_mask]
        checks.append(
            "    test_tile.bit2 = initial;\n"
            "    %s(17, 29, 17, 29);\n"
            "    assert(test_tile.bit2 == (initial | 0x%08Xu));\n"
            % (function_name, target)
        )
    cases = "\n".join(checks)
    helper_definition = "void __cdecl bit2_set" + helper[helper.index("("):]
    return (PRELUDE.replace("@ENUMS@", enum_text) +
            "\n" +
            "/* actual src/map.cpp bit2_set implementation */\n" +
            helper_definition + "\n" +
            "\n".join(wrappers) + MAIN_END.replace("@CASES@", cases))


def compile_run(program, compiler, tmp_path):
    source = tmp_path / "landmark_masks.cpp"
    exe = tmp_path / "landmark_masks"
    source.write_text(program, encoding="utf-8")
    built = subprocess.run([compiler, "-std=c++98", "-Wall", "-Wextra", "-Werror",
                            str(source), "-o", str(exe)], capture_output=True, text=True)
    assert built.returncode == 0, built.stdout + built.stderr
    return subprocess.run([str(exe)], capture_output=True, text=True)


def test_actual_callsite_masks_or_exact_bits_and_damage_controls(tmp_path):
    constants, calls, helper = extracted_controls()
    compilers = [compiler for compiler in ("g++", "clang++") if shutil.which(compiler)]
    assert compilers
    good_program = build_program(constants, calls, helper)
    wrong_indices = {
        index: WRITERS[function][3]
        for index, (function, _mask, _args) in enumerate(calls)
    }
    zero_masks = {index: "0" for index in range(len(calls))}
    all_masks = {index: "0xFFFFFFFFu" for index in range(len(calls))}
    mutants = (
        ("LM index", wrong_indices),
        ("zero mask", zero_masks),
        ("all mask", all_masks),
    )
    for compiler in compilers:
        good = compile_run(good_program, compiler, tmp_path)
        assert good.returncode == 0, good.stdout + good.stderr
        for name, overrides in mutants:
            damaged = compile_run(build_program(constants, calls, helper, overrides),
                                  compiler, tmp_path)
            assert damaged.returncode != 0, name + " mutant survived"


def test_vc6_x86_actual_bit2_set_body_and_callsite_expressions(tmp_path):
    """Lease-only companion control; coordinator executes under authorization."""
    if not os.environ.get("MATCH_RECOVERY_RESOURCE_LEASE"):
        pytest.skip("native landmark mask controls require an existing coordinator lease")
    from match_recovery.measurement import Measurement
    from match_recovery.processes import run_bounded
    from recovery.environment.compile import compile_object
    from recovery.environment.wine import VC6_ROOT, environment

    if not (VC6_ROOT / "BIN/CL.EXE").is_file() or shutil.which("wine32") is None:
        pytest.skip("VC6 and Wine32 are required under the coordinator lease")
    settings = Measurement.read(ROOT)
    wrapper = ROOT / "recovery/environment/wine.py"
    command = (sys.executable, str(wrapper), "--tool=cl", "/nologo")
    constants, calls, helper = extracted_controls()
    program = build_program(constants, calls, helper)

    for profile, flags in enumerate(settings.flag_sets):
        source = tmp_path / ("landmark-masks-%d.cpp" % profile)
        obj = tmp_path / ("landmark-masks-%d.obj" % profile)
        exe = tmp_path / ("landmark-masks-%d.exe" % profile)
        source.write_text(program, encoding="utf-8")
        obj.write_bytes(compile_object(source, command, tuple(flags) + settings.layout_flags))
        linked = run_bounded([sys.executable, str(wrapper), "--tool=link", "/nologo",
                              "/subsystem:console", "/incremental:no", "/out:" + str(exe),
                              str(obj), "kernel32.lib"], cwd=tmp_path, timeout=60)
        assert linked.returncode == 0, (linked.stdout + linked.stderr).decode(errors="replace")
        result = subprocess.run(["wine32", str(exe)], env=environment(),
                                stdout=subprocess.PIPE, stderr=subprocess.PIPE, timeout=30)
        output = (result.stdout + result.stderr).decode(errors="replace")
        assert result.returncode == 0, (profile, result.returncode, output)
