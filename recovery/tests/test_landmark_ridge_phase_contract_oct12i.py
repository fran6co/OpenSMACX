"""Actual-source control for the pinned 13-point Ridge passes.

The reviewed pinned range 0x00685908..0x0068593C resolves the last offset as
45. The source array's old 455 typo and 12-iteration bound hid its thirteenth
point. This bounded fixture covers mutation order and bounds only; it is not a
live-game or complete function-control-flow proof.
"""
from pathlib import Path
import os
import re
import shutil
import subprocess
import sys

import pytest


ROOT = Path(__file__).resolve().parents[2]
SOURCE = ROOT / "src/map.cpp"
HEADER = ROOT / "src/map.h"
BASE = "361f5176199a52ac0bc3f425cb139f0230730824"
PINNED_OFFSETS = (47, 44, 24, 20, 8, 7, 0, 5, 4, 17, 23, 35, 45)
PINNED_ENUMS = {
    "ALT_1_LEVEL_ABOVE_SEA": 4,
    "BIT2_RIDGE": 0x0800,
    "LM_RIDGE": 11,
    "MaxNaturalNum": 16,
}


def mask_comments_and_literals(text):
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
            elif text.startswith("/*", i):
                chars[i] = chars[i + 1] = " "
                i += 2
                state = "block"
            elif text[i] in ("'", '"'):
                quote = text[i]
                chars[i] = " "
                i += 1
                state = "literal"
            else:
                i += 1
        elif state == "line":
            if text[i] == "\n":
                state = "code"
            else:
                chars[i] = " "
            i += 1
        elif state == "block":
            if text.startswith("*/", i):
                chars[i] = chars[i + 1] = " "
                i += 2
                state = "code"
            else:
                if text[i] != "\n":
                    chars[i] = " "
                i += 1
        else:
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


def pinned_values():
    header = HEADER.read_text(encoding="utf-8")
    values = {}
    for name, expected in PINNED_ENUMS.items():
        match = re.search(r"\b" + re.escape(name) + r"\s*=\s*(0x[0-9A-Fa-f]+|\d+)", header)
        assert match is not None, "missing enum " + name
        values[name] = int(match.group(1), 0)
        assert values[name] == expected, (name, values[name], expected)
    return values


def actual_body():
    source = SOURCE.read_text(encoding="utf-8")
    body = function_body(source, "void __cdecl world_ridge(")
    assert "bit2_set(x_radius, y_radius, BIT2_RIDGE, true)" in body
    assert "Natural[LM_RIDGE].name" in body
    table = array_definition(source, "world_ridge_offset_val", 13)
    offsets = parse_ridge_offsets(table)
    assert offsets == PINNED_OFFSETS, offsets
    loops = re.findall(r"for\s*\(\s*uint32_t\s+([ij])\s*=\s*0\s*;\s*\1\s*<\s*13\s*;", body)
    assert len(loops) == 2, "Ridge must visit all thirteen native points in both passes"
    body = remove_table_declaration(body, "world_ridge_offset_val", 13)
    rows = {row.address: row for row in __import__("match_recovery.source", fromlist=["read_file"]).read_file(SOURCE)}
    row = rows[0x005C8150]
    assert row.name == "?world_ridge@@YAXHH@Z"
    assert row.image_spans == ((0x005C8150, 0x005C83AB),)
    assert row.byte_exact is False and row.semantic is False
    return body, offsets, table


def array_definition(source, name, count):
    pattern = (r"(?:(?:const\s+)?uint32_t)\s+" + re.escape(name)
               + r"\s*\[\s*" + str(count) + r"\s*\]\s*=\s*\{[^}]*\}\s*;")
    matches = re.findall(pattern, source)
    assert len(matches) == 1, "expected one source-owned definition for " + name
    return matches[0]


def remove_table_declaration(body, name, count):
    pattern = (r"(?:(?:const\s+)?uint32_t)\s+" + re.escape(name)
               + r"\s*\[\s*" + str(count) + r"\s*\]\s*=\s*\{[^}]*\}\s*;")
    return re.sub(pattern, "", body, count=1)


def parse_ridge_offsets(table):
    match = re.search(r"world_ridge_offset_val\[13\]\s*=\s*\{([^}]*)\}", table)
    assert match is not None, "actual 13-entry Ridge table absent"
    return tuple(int(token.strip(), 0) for token in match.group(1).split(",") if token.strip())


PRELUDE = r'''#include <assert.h>
#include <stdio.h>
#define __cdecl
#define rand fixture_rand
typedef unsigned int uint32_t;
typedef int BOOL;
enum { @ENUMS@ };
struct CheckedOffsets {
    int values[60];
    int &operator[](int index) {
        assert(index >= 0 && index < 60);
        return values[index];
    }
};
struct NaturalEntry { int name; };
struct Table { const char *get(int name); };
struct Event { char kind; int x; int y; int value; };
static CheckedOffsets RadiusOffsetX, RadiusOffsetY;
static NaturalEntry Natural[MaxNaturalNum];
static Table StringTable;
static Event events[60];
static int event_count;
static int MapLatitudeBounds = 64, MapLongitudeBounds = 64;
const char *Table::get(int name) { assert(name == 111); return "ridge"; }
static void record(char kind, int x, int y, int value) {
    assert(event_count < 60);
    events[event_count].kind = kind;
    events[event_count].x = x;
    events[event_count].y = y;
    events[event_count].value = value;
    ++event_count;
}
static int xrange(int x) { return x; }
static int on_map(int x, int y) { (void)x; (void)y; return 1; }
static int is_ocean(int x, int y) { (void)x; (void)y; return 0; }
static int near_landmark(int x, int y) { (void)x; (void)y; return 0; }
static int fixture_rand(void) { return 0; }
static void world_alt_set(int x, int y, int altitude, BOOL set) {
    assert(set && altitude == ALT_1_LEVEL_ABOVE_SEA);
    record('H', x, y, altitude);
}
static void bit2_set(int x, int y, int mask, BOOL set) {
    assert(set && mask == BIT2_RIDGE);
    record('B', x, y, mask);
}
static void code_set(int x, int y, int code) { record('C', x, y, code); }
static void new_landmark(int x, int y, const char *label) {
    assert(label != 0);
    record('L', x, y, 0);
}
'''


MAIN = r'''
int main() {
    int i;
    Natural[LM_RIDGE].name = 111;
    for (i = 0; i < 60; ++i) {
        RadiusOffsetX[i] = i * 10;
        RadiusOffsetY[i] = i * 20;
    }
    world_ridge(100, 200);
    assert(event_count == 40);
    for (i = 0; i < 13; ++i) {
        const int x = 100 + RadiusOffsetX[ExpectedOffsets[i]];
        const int y = 200 + RadiusOffsetY[ExpectedOffsets[i]];
        assert(events[i].kind == 'H');
        assert(events[i].x == x && events[i].y == y);
        assert(events[i].value == ALT_1_LEVEL_ABOVE_SEA);
    }
    for (i = 0; i < 13; ++i) {
        const int x = 100 + RadiusOffsetX[ExpectedOffsets[i]];
        const int y = 200 + RadiusOffsetY[ExpectedOffsets[i]];
        const int at = 13 + i * 2;
        assert(events[at].kind == 'B');
        assert(events[at].x == x && events[at].y == y);
        assert(events[at].value == BIT2_RIDGE);
        assert(events[at + 1].kind == 'C');
        assert(events[at + 1].x == x && events[at + 1].y == y);
        assert(events[at + 1].value == ExpectedOffsets[i]);
    }
    assert(events[39].kind == 'L' && events[39].x == 100 && events[39].y == 202);
    return 0;
}
'''


def build_program(body, values, offsets, table):
    enum_text = ",\n       ".join(name + " = " + hex(value) for name, value in values.items())
    expected = "static const int ExpectedOffsets[13] = { "
    expected += ", ".join(str(value) for value in offsets) + " };\n"
    return PRELUDE.replace("@ENUMS@", enum_text) + expected + table + "\n" + body + MAIN


def run_program(program, compiler, tmp_path):
    source = tmp_path / "ridge_phase.cpp"
    exe = tmp_path / "ridge_phase"
    source.write_text(program, encoding="utf-8")
    built = subprocess.run([compiler, "-std=c++98", "-Wall", "-Wextra", "-Werror",
                            str(source), "-o", str(exe)], capture_output=True, text=True)
    assert built.returncode == 0, built.stdout + built.stderr
    return subprocess.run([str(exe)], capture_output=True, text=True)


def test_ridge_actual_source_two_13_point_passes_and_damage_controls(tmp_path):
    body, offsets, table = actual_body()
    values = pinned_values()
    compilers = [compiler for compiler in ("g++", "clang++") if shutil.which(compiler)]
    assert compilers
    good_program = build_program(body, values, offsets, table)

    shortened, loop_count = re.subn(r"([ij]\s*<\s*)13(?=\s*;)", r"\g<1>12", body)
    assert shortened != body and loop_count == 2

    bad_offset, count = re.subn(r"(,\s*)45(\s*\})", r"\g<1>455\g<2>", table)
    assert count == 1

    baseline = subprocess.check_output(["git", "show", BASE + ":src/map.cpp"], cwd=ROOT).decode()
    fused_body = function_body(baseline, "void __cdecl world_ridge(")
    fused_body = remove_table_declaration(fused_body, "world_ridge_offset_val", 13)
    fused_body = fused_body.replace("LM_RIDGE, true", "BIT2_RIDGE, true")
    for compiler in compilers:
        good = run_program(good_program, compiler, tmp_path)
        assert good.returncode == 0, good.stdout + good.stderr
        for label, bad_body, bad_table in (("12-point passes", shortened, table),
                                           ("455 out-of-range offset", body, bad_offset),
                                           ("baseline fused 12-point body", fused_body, table)):
            bad = run_program(build_program(bad_body, values, offsets, bad_table), compiler, tmp_path)
            assert bad.returncode != 0, label + " damage survived"


def test_native_ridge_phase_controls_both_profiles_when_leased(tmp_path):
    """Optional coordinator-lease companion for both native flag profiles."""
    if not os.environ.get("MATCH_RECOVERY_RESOURCE_LEASE"):
        pytest.skip("native Ridge phase controls require an existing coordinator lease")
    from match_recovery.measurement import Measurement
    from match_recovery.processes import run_bounded
    from recovery.environment.compile import compile_object
    from recovery.environment.wine import VC6_ROOT, environment

    if not (VC6_ROOT / "BIN/CL.EXE").is_file() or shutil.which("wine32") is None:
        pytest.skip("VC6 and Wine32 are required under the coordinator lease")
    settings = Measurement.read(ROOT)
    wrapper = ROOT / "recovery/environment/wine.py"
    command = (sys.executable, str(wrapper), "--tool=cl", "/nologo")
    body, offsets, table = actual_body()
    program = build_program(body, pinned_values(), offsets, table)
    for profile, flags in enumerate(settings.flag_sets):
        source = tmp_path / ("ridge-phase-%d.cpp" % profile)
        obj = tmp_path / ("ridge-phase-%d.obj" % profile)
        exe = tmp_path / ("ridge-phase-%d.exe" % profile)
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
