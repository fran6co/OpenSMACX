"""Host/native behavior control for the recovered canyon phase ordering.

The fixture extracts the current actual world_canyon body and bounds its map
collaborators. It checks authored mutation ordering only, not live-game
semantics. The separate Ridge control checks its pinned thirteen-point table.
"""
import os
from pathlib import Path
import re
import shutil
import subprocess
import sys
import tempfile

import pytest

from match_recovery.source import read_file


ROOT = Path(__file__).resolve().parents[2]
SOURCE = ROOT / "src/map.cpp"
HEADER = ROOT / "src/map.h"
BASE = "361f5176199a52ac0bc3f425cb139f0230730824"
ADDRESS = 0x005C7CB0
SYMBOL = "?world_canyon@@YAXHH@Z"
CANYON_OFFSETS = (26, 27, 9, 1, 2, 0, 3, 4, 17, 23, 36, 35)
PINNED = {
    "ALT_1_LEVEL_ABOVE_SEA": 4,
    "ALT_SHORE_LINE": 3,
    "BIT2_CANYON": 0x0200,
    "LM_CANYON": 9,
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


def source_facts():
    source = SOURCE.read_text(encoding="utf-8")
    header = HEADER.read_text(encoding="utf-8")
    values = {}
    for name, expected in PINNED.items():
        match = re.search(r"\b" + re.escape(name) + r"\s*=\s*(0x[0-9A-Fa-f]+|\d+)", header)
        assert match is not None, name
        values[name] = int(match.group(1), 0)
        assert values[name] == expected, (name, values[name], expected)
    rows = {row.address: row for row in read_file(SOURCE)}
    row = rows[ADDRESS]
    assert row.name == SYMBOL and row.address == ADDRESS
    assert row.image_spans == ((ADDRESS, 0x005C7F38),)
    body = function_body(source, "void __cdecl world_canyon(")
    assert "Natural[LM_CANYON].name" in body
    assert "bit2_set(x_radius, y_radius, BIT2_CANYON, true)" in body
    return body, values


def parse_canyon_offsets(body):
    match = re.search(r"WorldCanyonOffsetValues\[12\]\s*=\s*\{([^}]*)\}", body)
    assert match is not None, "actual twelve-entry canyon offset table missing"
    offsets = tuple(int(token.strip(), 0) for token in match.group(1).split(",") if token.strip())
    assert offsets == CANYON_OFFSETS, offsets
    return offsets


PRELUDE = r'''#include <assert.h>
#include <stdio.h>
#define __cdecl
typedef unsigned int uint32_t;
typedef int BOOL;
enum { @ENUMS@ };
struct NaturalEntry { int name; };
struct Table { const char *get(int name); };
struct Event { char kind; int x; int y; int value; };
static Event events[100];
static int event_count;
static int RadiusOffsetX[48], RadiusOffsetY[48];
static int MapLatitudeBounds = 64, MapLongitudeBounds = 64;
static NaturalEntry Natural[MaxNaturalNum];
static Table StringTable;
const char *Table::get(int name) { assert(name == 93); return "landmark"; }
static void record(char kind, int x, int y, int value) {
    assert(event_count < 100);
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
static int rnd(int bound, void *unused) { (void)bound; (void)unused; return 0; }
static void world_alt_set(int x, int y, int altitude, BOOL set) {
    assert(set);
    record(altitude == ALT_1_LEVEL_ABOVE_SEA ? 'H' : 'S', x, y, altitude);
}
static void bit2_set(int x, int y, int mask, BOOL set) {
    assert(set && mask == BIT2_CANYON);
    record('B', x, y, mask);
}
static void code_set(int x, int y, int code) { record('C', x, y, code); }
static void new_landmark(int x, int y, const char *label) {
    assert(label != 0);
    record('L', x, y, 0);
}
'''


def program_source(body, values, offsets):
    enums = dict(values)
    enum_text = ",\n       ".join(name + " = " + hex(value) for name, value in enums.items())
    expected_table = ", ".join(str(value) for value in offsets)
    expected = "static const int ExpectedOffsets[12] = { " + expected_table + " };\n"
    main = r'''
int main() {
    int i;
    Natural[LM_CANYON].name = 93;
    for (i = 0; i < 48; ++i) {
        RadiusOffsetX[i] = i * 10;
        RadiusOffsetY[i] = i * 20;
    }
    world_canyon(100, 200);
    assert(event_count == 49);
    for (i = 0; i < 12; ++i) {
        const int x = 100 + RadiusOffsetX[ExpectedOffsets[i]];
        const int y = 200 + RadiusOffsetY[ExpectedOffsets[i]];
        assert(events[i * 2].kind == 'H');
        assert(events[i * 2].x == x && events[i * 2].y == y);
        assert(events[i * 2].value == ALT_1_LEVEL_ABOVE_SEA);
        assert(events[i * 2 + 1].kind == 'S');
        assert(events[i * 2 + 1].x == x && events[i * 2 + 1].y == y);
        assert(events[i * 2 + 1].value == ALT_SHORE_LINE);
    }
    for (i = 0; i < 12; ++i) {
        const int x = 100 + RadiusOffsetX[ExpectedOffsets[i]];
        const int y = 200 + RadiusOffsetY[ExpectedOffsets[i]];
        const int at = 24 + i * 2;
        assert(events[at].kind == 'B');
        assert(events[at].x == x && events[at].y == y);
        assert(events[at].value == BIT2_CANYON);
        assert(events[at + 1].kind == 'C');
        assert(events[at + 1].x == x && events[at + 1].y == y);
        assert(events[at + 1].value == ExpectedOffsets[i]);
    }
    assert(events[48].kind == 'L' && events[48].x == 100 && events[48].y == 202);
    return 0;
}
'''
    return (PRELUDE.replace("@ENUMS@", enum_text) + expected + body + main)


def compile_run(program, compiler, tmp_path):
    source = tmp_path / "canyon_phases.cpp"
    exe = tmp_path / "canyon_phases"
    source.write_text(program, encoding="utf-8")
    built = subprocess.run([compiler, "-std=c++98", "-Wall", "-Wextra", "-Werror",
                            str(source), "-o", str(exe)], capture_output=True, text=True)
    assert built.returncode == 0, built.stdout + built.stderr
    return subprocess.run([str(exe)], capture_output=True, text=True)


def test_canyon_actual_source_two_pass_phase_order_and_fused_old_body_control(tmp_path):
    body, values = source_facts()
    offsets = parse_canyon_offsets(body)
    compilers = [compiler for compiler in ("g++", "clang++") if shutil.which(compiler)]
    assert compilers
    good_program = program_source(body, values, offsets)
    wrong_offset, changed = re.subn(
        r"(WorldCanyonOffsetValues\[12\]\s*=\s*\{\s*)26\b",
        r"\g<1>25", body, count=1)
    assert changed == 1
    wrong_offset_program = program_source(wrong_offset, values, offsets)
    baseline = subprocess.check_output(["git", "show", BASE + ":src/map.cpp"], cwd=ROOT).decode()
    fused_body = function_body(baseline, "void __cdecl world_canyon(")
    # Keep the old fused traversal's control flow while correcting only its
    # mask argument, so the negative control isolates pass ordering.
    fused_body = fused_body.replace("LM_CANYON, true", "BIT2_CANYON, true")
    fused_program = program_source(fused_body, values, offsets)
    for compiler in compilers:
        good = compile_run(good_program, compiler, tmp_path)
        assert good.returncode == 0, good.stdout + good.stderr
        wrong = compile_run(wrong_offset_program, compiler, tmp_path)
        assert wrong.returncode != 0, "wrong first offset survived pinned PE tuple oracle"
        fused = compile_run(fused_program, compiler, tmp_path)
        assert fused.returncode != 0, "fused per-point old body survived phase-order oracle"


def test_vc6_x86_actual_canyon_behavior_and_fused_damage_control(tmp_path):
    """Lease-guarded native execution; coordinator alone opts into this test."""
    if not os.environ.get("MATCH_RECOVERY_RESOURCE_LEASE"):
        pytest.skip("native canyon phase controls require an existing coordinator lease")
    from match_recovery.measurement import Measurement
    from match_recovery.processes import run_bounded
    from recovery.environment.compile import compile_object
    from recovery.environment.wine import VC6_ROOT, environment

    if not (VC6_ROOT / "BIN/CL.EXE").is_file() or shutil.which("wine32") is None:
        pytest.skip("VC6 and Wine32 are required under the coordinator lease")
    body, values = source_facts()
    offsets = parse_canyon_offsets(body)
    baseline = subprocess.check_output(["git", "show", BASE + ":src/map.cpp"], cwd=ROOT).decode()
    fused_body = function_body(baseline, "void __cdecl world_canyon(")
    fused_body = fused_body.replace("LM_CANYON, true", "BIT2_CANYON, true")
    assert fused_body != body
    settings = Measurement.read(ROOT)
    wrapper = ROOT / "recovery/environment/wine.py"
    command = (sys.executable, str(wrapper), "--tool=cl", "/nologo")
    programs = (program_source(body, values, offsets), program_source(fused_body, values, offsets))
    for profile, flags in enumerate(settings.flag_sets):
        for variant, program in enumerate(programs):
            source = tmp_path / ("canyon-%d-%d.cpp" % (profile, variant))
            obj = tmp_path / ("canyon-%d-%d.obj" % (profile, variant))
            exe = tmp_path / ("canyon-%d-%d.exe" % (profile, variant))
            source.write_text(program, encoding="utf-8")
            obj.write_bytes(compile_object(source, command, tuple(flags) + settings.layout_flags))
            linked = run_bounded([sys.executable, str(wrapper), "--tool=link", "/nologo",
                                  "/subsystem:console", "/incremental:no", "/out:" + str(exe),
                                  str(obj), "kernel32.lib"], cwd=tmp_path, timeout=60)
            assert linked.returncode == 0, (linked.stdout + linked.stderr).decode(errors="replace")
            result = subprocess.run(["wine32", str(exe)], env=environment(),
                                    stdout=subprocess.PIPE, stderr=subprocess.PIPE, timeout=30)
            output = (result.stdout + result.stderr).decode(errors="replace")
            if variant == 0:
                assert result.returncode == 0, (profile, result.returncode, output)
            else:
                assert result.returncode != 0, (profile, "fused old-body damage survived", output)
