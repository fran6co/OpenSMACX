"""Actual-source controls for the pinned indexed is_coast contract."""
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
FACTION = ROOT / "src/faction.cpp"
BASE = "f774abf53"
ORIGINAL = "// ORIGINAL: 0x004E49D0 ?is_coast@@YAHHHH@Z 0x004E49D0-0x004E4A91"


def extract(text: str, signature: str) -> str:
    start = text.index(signature)
    brace = text.index("{", start)
    depth = 0
    for end in range(brace, len(text)):
        if text[end] == "{":
            depth += 1
        elif text[end] == "}":
            depth -= 1
            if depth == 0:
                return text[start : end + 1]
    raise AssertionError("unterminated source body: " + signature)


def actual_bodies():
    source = SOURCE.read_text()
    header = HEADER.read_text()
    coast = extract(source, "int __cdecl is_coast(")
    on_map = extract(header, "MEASURED inline BOOL __cdecl on_map(")
    return coast, on_map


def test_identity_history_header_and_indexed_consumer_are_preserved():
    baseline = subprocess.run(
        ["git", "show", BASE + ":src/map.cpp"], cwd=ROOT,
        capture_output=True, text=True, check=True,
    ).stdout
    current = SOURCE.read_text()
    header = HEADER.read_text()
    assert ORIGINAL in baseline and ORIGINAL in current
    tried_start = baseline.index("// TRIED:", baseline.index(ORIGINAL))
    tried_end = baseline.index("// size", tried_start)
    assert baseline[tried_start:tried_end] in current
    assert "int __cdecl is_coast(" in header
    assert "int __cdecl is_coast(" in actual_bodies()[0]

    old_rows = {row.address: row for row in read_text(baseline, SOURCE)}
    new_rows = {row.address: row for row in read_file(SOURCE)}
    old, new = old_rows[0x4E49D0], new_rows[0x4E49D0]
    for field in ("address", "name", "symbol", "image_spans", "byte_exact",
                  "levers", "ruled_out", "deferred", "unrecoverable"):
        assert getattr(new, field) == getattr(old, field), field

    faction = FACTION.read_text()
    consumer = faction[faction.index("int coast = is_coast(x, y, 0);"):]
    consumer = consumer[:consumer.index("\n        }") + 10]
    assert "RadiusOffsetX[coast]" in consumer
    assert "RadiusOffsetY[coast]" in consumer


PRELUDE = r'''#include <assert.h>
#include <stdio.h>
#define MEASURED
#define __cdecl
typedef unsigned int uint32_t;
typedef int BOOL;

struct Event { char op; int x; int y; };
static Event events[2048];
static int event_count;
static int RadiusOffsetX[289];
static int RadiusOffsetY[289];
static int origin_x, origin_y, width_value, height_value;
static int flat_first, flat_later, flat_reads;
static int width_reads, height_reads, ocean_reads;
static int ocean_x, ocean_y;

static void record(char op, int x, int y) {
    assert(event_count < 2048);
    events[event_count].op = op;
    events[event_count].x = x;
    events[event_count].y = y;
    ++event_count;
}
static int read_map_flat() {
    int value = flat_reads == 0 ? flat_first : flat_later;
    ++flat_reads;
    record('F', value, flat_reads);
    return value;
}
static int read_longitude_bounds() {
    ++width_reads;
    record('W', width_value, width_reads);
    return width_value;
}
static int read_latitude_bounds() {
    ++height_reads;
    record('H', height_value, height_reads);
    return height_value;
}
#define MapIsFlat read_map_flat()
#define MapLongitudeBounds read_longitude_bounds()
#define MapLatitudeBounds read_latitude_bounds()

static int is_ocean(int x, int y) {
    ++ocean_reads;
    record('O', x, y);
    return x == ocean_x && y == ocean_y;
}
static void reset_case(int x, int y, int width, int height, int flat, int later_flat) {
    int i;
    origin_x = x; origin_y = y;
    width_value = width; height_value = height;
    flat_first = flat; flat_later = later_flat;
    flat_reads = width_reads = height_reads = ocean_reads = event_count = 0;
    ocean_x = 0x7fffffff; ocean_y = 0x7fffffff;
    for (i = 0; i < 289; ++i) {
        RadiusOffsetX[i] = 0;
        RadiusOffsetY[i] = 0;
    }
    for (i = 1; i <= 20; ++i) {
        RadiusOffsetX[i] = i;
        RadiusOffsetY[i] = i;
    }
}
static void set_ocean(int x, int y) { ocean_x = x; ocean_y = y; }
static int saw_event(char op, int x, int y) {
    int i;
    for (i = 0; i < event_count; ++i)
        if (events[i].op == op && events[i].x == x && events[i].y == y) return 1;
    return 0;
}
'''


SUFFIX = r'''
int main() {
    int index;
    reset_case(40, 40, 200, 200, 1, 1);
    for (index = 1; index <= 8; ++index) {
        reset_case(40, 40, 200, 200, 1, 1);
        set_ocean(40 + RadiusOffsetX[index], 40 + RadiusOffsetY[index]);
        assert(is_coast(40, 40, 0) == index);
        assert(flat_reads == 1);
    }
    for (index = 1; index <= 20; ++index) {
        reset_case(40, 40, 200, 200, 1, 1);
        set_ocean(40 + RadiusOffsetX[index], 40 + RadiusOffsetY[index]);
        assert(is_coast(40, 40, 1) == index);
        assert(flat_reads == 1);
    }
    reset_case(40, 40, 200, 200, 1, 1);
    set_ocean(40 + RadiusOffsetX[20], 40 + RadiusOffsetY[20]);
    assert(is_coast(40, 40, -1) == 20);

    // No matching ocean tile returns zero.
    reset_case(40, 40, 200, 200, 1, 1);
    assert(is_coast(40, 40, 0) == 0);

    // Wrapped left/right, including low-bit clear with a changing later read:
    // one cached MapIsFlat read must govern the whole loop.
    reset_case(2, 10, 32, 32, 0, 0);
    RadiusOffsetX[1] = -3;
    RadiusOffsetY[1] = 0;
    set_ocean(31, 10);
    assert(is_coast(2, 10, 0) == 1);
    reset_case(30, 10, 32, 32, 2, 3);
    RadiusOffsetX[1] = 1;
    RadiusOffsetX[2] = 4;
    RadiusOffsetY[1] = RadiusOffsetY[2] = 0;
    set_ocean(2, 10);
    assert(is_coast(30, 10, 0) == 2);
    assert(flat_reads == 1);

    // Flat maps do not wrap. A single adjustment is not repeated/modulo wrap.
    reset_case(2, 10, 32, 32, 1, 1);
    RadiusOffsetX[1] = -3;
    RadiusOffsetY[1] = 0;
    set_ocean(31, 10);
    assert(is_coast(2, 10, 0) == 0);
    assert(!saw_event('O', -1, 10));
    reset_case(2, 10, 32, 32, 0, 0);
    RadiusOffsetX[1] = -40;
    RadiusOffsetY[1] = 0;
    set_ocean(26, 10);
    assert(is_coast(2, 10, 0) == 0);
    reset_case(30, 10, 32, 32, 0, 0);
    RadiusOffsetX[1] = 40;
    RadiusOffsetY[1] = 0;
    set_ocean(6, 10);
    assert(is_coast(30, 10, 0) == 0);

    // Latitude is tested before longitude. With both invalid and a flat map,
    // the helper reads height but never reaches the longitude bound.
    reset_case(31, 31, 32, 32, 1, 1);
    RadiusOffsetX[1] = 2;
    RadiusOffsetY[1] = 1;
    set_ocean(33, 32);
    assert(is_coast(31, 31, 0) == 0);
    assert(height_reads > 0 && width_reads == 0 && ocean_reads == 0);

    // A longitude-invalid neighbor is rejected only after a valid latitude.
    reset_case(31, 10, 32, 32, 1, 1);
    RadiusOffsetX[1] = 2;
    RadiusOffsetY[1] = 0;
    set_ocean(33, 10);
    assert(is_coast(31, 10, 0) == 0);
    assert(height_reads > 0 && width_reads > 0 && ocean_reads == 0);

    // Index zero is not a radius direction, even if the center is ocean.
    reset_case(40, 40, 200, 200, 1, 1);
    set_ocean(40, 40);
    assert(is_coast(40, 40, 0) == 0);
    assert(ocean_reads == 8 && !saw_event('O', 40, 40));
    puts("is_coast actual-source controls passed");
    return 0;
}
'''


def program_source(coast_body: str):
    on_map_body = extract(HEADER.read_text(), "MEASURED inline BOOL __cdecl on_map(")
    return PRELUDE + on_map_body + coast_body + SUFFIX


def compile_host(coast_body: str, compiler: str):
    with tempfile.TemporaryDirectory(prefix="is-coast-control-") as tmp:
        source = Path(tmp) / "control.cpp"
        exe = Path(tmp) / "control"
        source.write_text(program_source(coast_body))
        built = subprocess.run(
            [compiler, "-std=c++98", "-Wall", "-Wextra", "-Werror", str(source), "-o", str(exe)],
            capture_output=True, text=True,
        )
        assert built.returncode == 0, built.stdout + built.stderr
        return subprocess.run([str(exe)], capture_output=True, text=True)


def damage_mutants(body: str):
    assert body.count("return i;") == 1
    boolean = body.replace("return i;", "return 1;", 1)
    direction = body.replace("return i;", "return i + 1;", 1)
    center, center_count = re.subn(
        r"(\bint\s+i\s*=\s*)1",
        r"\g<1>0", body, count=1,
    )
    radius, radius_count = re.subn(r"\?\s*21\s*:\s*9", "? 9 : 21", body, count=1)
    assert center_count == radius_count == 1
    return {
        "boolean return": boolean,
        "center included as a direction": center,
        "reversed radius selector": radius,
        "wrong returned direction index": direction,
    }


def test_host_actual_body_and_damage_controls():
    body, _ = actual_bodies()
    compilers = [compiler for compiler in ("g++", "clang++") if shutil.which(compiler)]
    assert compilers
    for compiler in compilers:
        result = compile_host(body, compiler)
        assert result.returncode == 0, result.stdout + result.stderr
        assert "controls passed" in result.stdout
        for label, mutant in damage_mutants(body).items():
            assert mutant != body
            result = compile_host(mutant, compiler)
            assert result.returncode != 0, label + " survived with " + compiler


def test_vc6_native_actual_body_and_damage_controls_under_existing_lease(tmp_path):
    if not os.environ.get("MATCH_RECOVERY_RESOURCE_LEASE"):
        pytest.skip("native is_coast controls require an existing coordinator compiler lease")

    from match_recovery.measurement import Measurement
    from match_recovery.processes import run_bounded
    from recovery.environment.compile import compile_object
    from recovery.environment.wine import VC6_ROOT, environment

    if not (VC6_ROOT / "BIN/CL.EXE").is_file() or shutil.which("wine32") is None:
        pytest.skip("VC6 and Wine32 are required under the coordinator lease")

    settings = Measurement.read(ROOT)
    wrapper = ROOT / "recovery/environment/wine.py"
    command = (sys.executable, str(wrapper), "--tool=cl", "/nologo")
    body, _ = actual_bodies()

    def build_native(name, flags, selected_body):
        source = tmp_path / (name + ".cpp")
        obj = tmp_path / (name + ".obj")
        exe = tmp_path / (name + ".exe")
        source.write_text(program_source(selected_body), encoding="utf-8")
        obj.write_bytes(compile_object(source, command, tuple(flags) + settings.layout_flags))
        linked = run_bounded(
            [sys.executable, str(wrapper), "--tool=link", "/nologo", "/subsystem:console",
             "/incremental:no", "/out:" + str(exe), str(obj), "kernel32.lib"],
            cwd=tmp_path, timeout=60,
        )
        assert linked.returncode == 0, (linked.stdout + linked.stderr).decode(errors="replace")
        return exe

    def run_native(exe):
        with tempfile.TemporaryFile() as diagnostics:
            result = subprocess.run(["wine32", str(exe)], env=environment(), stdout=diagnostics,
                                    stderr=diagnostics, timeout=30)
            diagnostics.seek(0)
            output = diagnostics.read().decode(errors="replace")
        return result.returncode, output

    for profile_index, flags in enumerate(settings.flag_sets):
        good = build_native("is-coast-good-" + str(profile_index), flags, body)
        code, output = run_native(good)
        assert code == 0, "profile %d: %s (%d)" % (profile_index, output, code)
        for mutant_index, (label, mutant) in enumerate(damage_mutants(body).items()):
            damaged = build_native("is-coast-bad-%d-%d" % (profile_index, mutant_index), flags, mutant)
            code, output = run_native(damaged)
            assert code != 0, "profile %d %s mutant survived: %s" % (profile_index, label, output)
