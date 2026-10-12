"""Actual-source behavior controls for world_fossil.

The fixture bounds map and game collaborators. It checks source semantics and
does not claim runtime equivalence, reachability, or full game recovery.
"""
from pathlib import Path
import os
import re
import shutil
import subprocess
import sys
import tempfile

import pytest
from match_recovery.source import read_file


ROOT = Path(__file__).resolve().parents[2]
SOURCE = ROOT / "src/map.cpp"
ORIGINAL = "// ORIGINAL: 0x005C7A80 ?world_fossil@@YAXHH@Z 0x005C7A80-0x005C7CA1"


def current_body():
    source = SOURCE.read_text(encoding="utf-8")
    start = source.index("void __cdecl world_fossil(")
    brace = source.index("{", start)
    depth = 0
    for end in range(brace, len(source)):
        if source[end] == "{":
            depth += 1
        elif source[end] == "}":
            depth -= 1
            if depth == 0:
                return source[start:end + 1]
    raise AssertionError("world_fossil body is unterminated")


def source_controls():
    source = SOURCE.read_text(encoding="utf-8")
    header = (ROOT / "src/map.h").read_text(encoding="utf-8")
    body = current_body()
    assert ORIGINAL in source
    assert "if (ExpansionEnabled)" not in body
    assert "if (++loc_attempts >= 1000)" in body
    assert "for (uint32_t i = 0; i < 6; i++)" in body
    assert "&& is_ocean(x_radius, y_radius)" in body
    assert "bit2_set(x_radius, y_radius, BIT2_FOSSIL, true)" in body
    assert parsed_constants(header) == {
        "ALT_OCEAN": 1, "BIT2_FOSSIL": 0x8000, "LM_FOSSIL": 15, "MaxNaturalNum": 16,
    }

    # The generator, unlike this directly callable helper, owns the expansion
    # gate. Console editor case 15 remains a direct invocation.
    generator = source[source.index("void __cdecl world_landmarks()") :]
    generator = generator[:generator.index("\n}", 0)]
    assert "if (ExpansionEnabled) {\n        world_unity(-1, -1);\n        if (ExpansionEnabled) {\n            world_fossil(-1, -1);\n        }\n    }" in generator
    world_build = source[source.index("void __cdecl world_build()") :]
    world_build = world_build[:world_build.index("\n}", 0)]
    assert "if (ExpansionEnabled != 0) {\n        world_unity(-1, -1);\n        if (ExpansionEnabled != 0) {\n            world_fossil(-1, -1);\n        }\n    }" in world_build
    console = (ROOT / "src/console.cpp").read_text(encoding="utf-8")
    case = "case 15: world_fossil(x, y); break;"
    assert case in console
    case_at = console.index(case)
    place_branch = console.rfind("if (place != 0)", 0, case_at)
    assert place_branch >= 0
    assert "if (ExpansionEnabled)" not in console[place_branch:case_at]

    rows = {row.address: row for row in read_file(SOURCE)}
    row = rows[0x5C7A80]
    assert row.name == "?world_fossil@@YAXHH@Z"
    assert row.symbol == ""
    assert row.image_spans
    assert row.image_spans[0] == (0x5C7A80, 0x5C7CA1)
    assert row.byte_exact is False
    assert row.semantic is False
    assert "// LEVER:" in source[source.index(ORIGINAL):source.index("void __cdecl world_fossil(")]
    assert "// TRIED:" in source[source.index(ORIGINAL):source.index("void __cdecl world_fossil(")]
    return body


def parsed_constants(header=None):
    if header is None:
        header = (ROOT / "src/map.h").read_text(encoding="utf-8")
    patterns = {
        "ALT_OCEAN": r"\bALT_OCEAN\s*=\s*(\d+)",
        "BIT2_FOSSIL": r"\bBIT2_FOSSIL\s*=\s*(0x[0-9A-Fa-f]+)",
        "LM_FOSSIL": r"\bLM_FOSSIL\s*=\s*(\d+)",
        "MaxNaturalNum": r"\bMaxNaturalNum\s*=\s*(\d+)",
    }
    values = {}
    for name, pattern in patterns.items():
        match = re.search(pattern, header)
        assert match is not None, "missing " + name + " declaration"
        values[name] = int(match.group(1), 0)
    return values


PRELUDE = r'''#include <assert.h>
#include <stdio.h>
#include <string.h>
#define __cdecl
#define rand recovery_rand
typedef unsigned int uint32_t;
enum { ALT_OCEAN = @ALT_OCEAN@, BIT2_FOSSIL = @BIT2_FOSSIL@,
       LM_FOSSIL = @LM_FOSSIL@, MaxNaturalNum = @MaxNaturalNum@ };

struct NaturalEntry { int name; };
struct Table {
    const char *get(int value);
};
struct Event { char op; int x; int y; int value; };
static Event events[12000];
static int event_count;
static int ExpansionEnabled;
static int MapLatitudeBounds = 20;
static int MapLongitudeBounds = 10;
static int RadiusOffsetX[6], RadiusOffsetY[6];
static NaturalEntry Natural[MaxNaturalNum];
static Table StringTable;
static int scenario;
static int input_valid;
static int bounds_mask, ocean_mask;
static int rand_values[16], rand_count, rand_at;
static int ocean_count, search_ocean_count, near_count, write_count;
static int table_calls, landmark_calls, xrange_calls;
static int selected_x, selected_y, last_write_index;

const char *Table::get(int value) {
    assert(value == 71); ++table_calls; return "Fossil Ridge";
}

static void event(char op, int x, int y, int value) {
    assert(event_count < (int)(sizeof(events) / sizeof(events[0])));
    events[event_count].op = op; events[event_count].x = x;
    events[event_count].y = y; events[event_count].value = value;
    ++event_count;
}
static void reset_state(int mode) {
    int i;
    scenario = mode; event_count = 0; rand_at = 0;
    ocean_count = search_ocean_count = near_count = write_count = 0;
    table_calls = landmark_calls = xrange_calls = 0;
    selected_x = selected_y = last_write_index = 0;
    input_valid = mode == 0;
    bounds_mask = (1 << 0) | (1 << 2) | (1 << 3) | (1 << 4) | (1 << 5);
    ocean_mask = (1 << 0) | (1 << 3) | (1 << 4);
    ExpansionEnabled = 0;
    for (i = 0; i < 6; ++i) {
        RadiusOffsetX[i] = i * 10;
        RadiusOffsetY[i] = i * 20;
    }
    Natural[LM_FOSSIL].name = 71;
    rand_count = 0;
    if (mode == 1) {
        // Candidate 1 is land; candidate 2 is ocean but occupied; candidate 3
        // is ocean and clear. The chosen values exercise y-parity correction.
        rand_values[0] = 2; rand_values[1] = 3;
        rand_values[2] = 2; rand_values[3] = 3;
        rand_values[4] = 1; rand_values[5] = 5;
        rand_count = 6;
    }
}
static int recovery_rand(void) {
    if (scenario == 2) {
        assert(rand_at < 2000);
        ++rand_at;
        return 0;
    }
    assert(rand_at < rand_count);
    return rand_values[rand_at++];
}
static int xrange(int value) {
    event('X', value, 0, 0);
    int index = xrange_calls++;
    assert(index < 6);
    if (scenario == 0) assert(value == 100 + index * 10);
    else assert(value == selected_x + index * 10);
    return value;
}
static int on_map(int x, int y) {
    event('M', x, y, 0);
    if (event_count == 1) return input_valid;
    if (scenario == 1) return 1;
    assert(y == 200 + ((x - 100) / 10) * 20);
    return (bounds_mask & (1 << ((x - 100) / 10))) != 0;
}
static int is_ocean(int x, int y) {
    event('O', x, y, 0); ++ocean_count;
    if (scenario == 1) {
        if (near_count < 2) {
            ++search_ocean_count;
            if (x == 2 && y == 10) return search_ocean_count == 2;
            return x == 5 && y == 9;
        }
        assert(y == selected_y + ((x - selected_x) / 10) * 20);
        return (ocean_mask & (1 << ((x - selected_x) / 10))) != 0;
    }
    if (scenario == 2) return 0;
    return (ocean_mask & (1 << ((x - 100) / 10))) != 0;
}
static int near_landmark(int x, int y) {
    event('N', x, y, 0); ++near_count;
    assert(scenario == 1);
    if (near_count == 1) return 1;
    selected_x = x; selected_y = y;
    return 0;
}
static void world_alt_set(int x, int y, int value, int enabled) {
    int index;
    assert(value == ALT_OCEAN && enabled);
    index = scenario == 0 ? (x - 100) / 10 : (x - selected_x) / 10;
    assert(index == 0 || index == 3 || index == 4);
    assert(y == (scenario == 0 ? 200 : selected_y) + index * 20);
    last_write_index = index;
    event('A', x, y, value); ++write_count;
}
static void bit2_set(int x, int y, int value, int enabled) {
    assert(value == BIT2_FOSSIL && value != LM_FOSSIL && enabled);
    assert(x == (scenario == 0 ? 100 : selected_x) + last_write_index * 10);
    event('B', x, y, value);
}
static void code_set(int x, int y, int value) {
    assert(x == (scenario == 0 ? 100 : selected_x) + value * 10);
    assert(y == (scenario == 0 ? 200 : selected_y) + value * 20);
    assert(value == last_write_index);
    event('C', x, y, value);
}
static void new_landmark(int x, int y, const char *name) {
    assert(name != 0 && strcmp(name, "Fossil Ridge") == 0);
    assert(x == (scenario == 0 ? 100 : selected_x));
    assert(y == (scenario == 0 ? 200 : selected_y));
    event('L', x, y, 0); ++landmark_calls;
}
'''


SUFFIX = r'''
int main(int argc, char **argv) {
    (void)argc; (void)argv;
    if (argc > 1 && strcmp(argv[1], "retry") == 0) {
        reset_state(1);
        world_fossil(-1, -1);
        assert(rand_at == 6 && search_ocean_count == 3 && near_count == 2);
        assert(landmark_calls == 1 && write_count == 3);
        assert(xrange_calls == 6);
        assert(events[event_count - 1].op == 'L');
    } else if (argc > 1 && strcmp(argv[1], "cap") == 0) {
        reset_state(2);
        world_fossil(-1, -1);
        assert(rand_at == 2000 && ocean_count == 999);
        assert(near_count == 0 && write_count == 0 && landmark_calls == 0);
    } else {
        reset_state(0);
        if (argc > 1 && strcmp(argv[1], "direct-enabled") == 0) ExpansionEnabled = 1;
        world_fossil(100, 200);
        // A direct call works with expansion disabled. Six offsets are
        // examined; only bounded ocean cells 0, 3 and 4 are modified.
        assert(write_count == 3 && landmark_calls == 1 && table_calls == 1);
        assert(xrange_calls == 6);
        assert(events[event_count - 1].op == 'L');
    }
    return 0;
}
'''


def product_source(body, damage=None):
    if damage is not None:
        before, after = damage
        assert before in body
        body = body.replace(before, after, 1)
    prelude = PRELUDE
    for name, value in parsed_constants().items():
        prelude = prelude.replace("@" + name + "@", hex(value))
    assert all("@" + name + "@" not in prelude for name in parsed_constants())
    return prelude + "\n" + body + "\n" + SUFFIX


MUTANTS = (
    ("expansion guard", "uint32_t loc_attempts = 0;",
     "if (!ExpansionEnabled) return;\n    uint32_t loc_attempts = 0;", "direct"),
    ("retry cap weakened", "loc_attempts >= 1000", "loc_attempts >= 1000000", "cap"),
    ("ocean polarity reversed", "&& is_ocean(x_radius, y_radius)",
     "&& !is_ocean(x_radius, y_radius)", "direct"),
    ("natural index passed as tile mask", "BIT2_FOSSIL, true", "LM_FOSSIL, true", "direct"),
)


def host_compile_run(body, compiler, scenario=None, damage=None, tmp_path=None):
    source = tmp_path / "world_fossil_control.cpp"
    exe = tmp_path / "world_fossil_control"
    source.write_text(product_source(body, damage), encoding="utf-8")
    built = subprocess.run([compiler, "-std=c++98", "-Wall", "-Wextra", "-Werror",
                            str(source), "-o", str(exe)], capture_output=True, text=True)
    assert built.returncode == 0, built.stdout + built.stderr
    command = [str(exe)] + ([scenario] if scenario else [])
    return subprocess.run(command, capture_output=True, text=True)


def test_body_source_identity_callers_and_host_damage_controls(tmp_path):
    body = source_controls()
    compilers = [c for c in ("g++", "clang++") if shutil.which(c)]
    assert compilers
    for compiler in compilers:
        for scenario in (None, "direct-enabled", "retry", "cap"):
            result = host_compile_run(body, compiler, scenario, tmp_path=tmp_path)
            assert result.returncode == 0, result.stdout + result.stderr
        for _name, before, after, scenario in MUTANTS:
            result = host_compile_run(body, compiler, scenario, (before, after), tmp_path)
            assert result.returncode != 0, _name + " mutant survived"


def test_vc6_x86_actual_source_behavior_and_damage_controls(tmp_path):
    """Lease-guarded native execution; coordinator alone opts into this test."""
    if not os.environ.get("MATCH_RECOVERY_RESOURCE_LEASE"):
        pytest.skip("native world_fossil controls require an existing coordinator lease")
    from match_recovery.measurement import Measurement
    from match_recovery.processes import run_bounded
    from recovery.environment.compile import compile_object
    from recovery.environment.wine import VC6_ROOT, environment

    if not (VC6_ROOT / "BIN/CL.EXE").is_file() or shutil.which("wine32") is None:
        pytest.skip("VC6 and Wine32 are required under the coordinator lease")
    settings = Measurement.read(ROOT)
    wrapper = ROOT / "recovery/environment/wine.py"
    command = (sys.executable, str(wrapper), "--tool=cl", "/nologo")
    body = source_controls()

    def build(name, flags, damage=None):
        source = tmp_path / (name + ".cpp")
        obj = tmp_path / (name + ".obj")
        exe = tmp_path / (name + ".exe")
        source.write_text(product_source(body, damage), encoding="utf-8")
        obj.write_bytes(compile_object(source, command, tuple(flags) + settings.layout_flags))
        linked = run_bounded([sys.executable, str(wrapper), "--tool=link", "/nologo",
                              "/subsystem:console", "/incremental:no", "/out:" + str(exe),
                              str(obj), "kernel32.lib"], cwd=tmp_path, timeout=60)
        assert linked.returncode == 0, (linked.stdout + linked.stderr).decode(errors="replace")
        return exe

    def run(exe, scenario):
        result = subprocess.run(["wine32", str(exe), scenario], env=environment(),
                                stdout=subprocess.PIPE, stderr=subprocess.PIPE, timeout=30)
        return result.returncode, (result.stdout + result.stderr).decode(errors="replace")

    for profile, flags in enumerate(settings.flag_sets):
        good = build("fossil-good-%d" % profile, flags)
        for scenario in ("direct", "direct-enabled", "retry", "cap"):
            code, output = run(good, scenario)
            assert code == 0, (profile, scenario, code, output)
        for index, (_name, before, after, scenario) in enumerate(MUTANTS):
            damaged = build("fossil-bad-%d-%d" % (profile, index), flags, (before, after))
            code, output = run(damaged, scenario)
            assert code != 0, (profile, index, "mutant survived", output)
