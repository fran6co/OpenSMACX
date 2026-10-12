"""Actual-source controls for coast_or_border's result and read ordering.

The fixture supplies only bounded collaborators. It does not model a live map,
game reachability, or the original executable's runtime.
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
BASE = "53d435132dd96d4691dcbf4fc21665dbfb83407c"
ORIGINAL = "// ORIGINAL: 0x0056B480 ?coast_or_border@@YAHHHHHH@Z 0x0056B480-0x0056B5A9"


def current_body():
    source = (ROOT / "src/map.cpp").read_text()
    start = source.index("int __cdecl coast_or_border(")
    brace = source.index("{", start)
    depth = 0
    for end in range(brace, len(source)):
        if source[end] == "{":
            depth += 1
        elif source[end] == "}":
            depth -= 1
            if depth == 0:
                return source[start : end + 1]
    raise AssertionError("coast_or_border body is unterminated")


def test_source_keeps_original_identity_history_and_int_contract():
    baseline = subprocess.run(
        ["git", "show", BASE + ":src/map.cpp"], cwd=ROOT,
        capture_output=True, text=True, check=True,
    ).stdout
    current = (ROOT / "src/map.cpp").read_text()
    header = (ROOT / "src/map.h").read_text()
    assert ORIGINAL in baseline and ORIGINAL in current
    tried_start = baseline.index("// TRIED:", baseline.index(ORIGINAL))
    tried_end = baseline.index("BOOL __cdecl coast_or_border(", tried_start)
    assert baseline[tried_start:tried_end] in current
    assert "int __cdecl coast_or_border(" in header
    assert "int __cdecl coast_or_border(" in current_body()
    assert "return i;" in current_body()
    old_rows = {row.address: row for row in read_text(baseline, SOURCE)}
    new_rows = {row.address: row for row in read_file(SOURCE)}
    old, new = old_rows[0x56B480], new_rows[0x56B480]
    for field in ("address", "name", "symbol", "image_spans", "byte_exact",
                  "levers", "ruled_out", "deferred", "unrecoverable"):
        assert getattr(new, field) == getattr(old, field), field


PRELUDE = r'''#include <cassert>
#include <stdio.h>
#include <vector>
#define __cdecl
typedef unsigned int uint32_t;
typedef int BOOL;

struct Event { char op; int x; int y; };
static std::vector<Event> events;
static int RadiusOffsetX[9];
static int RadiusOffsetY[9];
static const int AX = 1000, AY = 2000, BX = 3000, BY = 4000;
static int target_index, reason, invalid_mask, initial_match;
static unsigned int region_a_calls;

static void record(char op, int x, int y) {
    Event event; event.op = op; event.x = x; event.y = y;
    events.push_back(event);
}
static int neighbor_index(int x, int y) {
    assert(y == AY + (x - AX) * 2);
    assert((x - AX) % 10 == 0);
    int index = (x - AX) / 10;
    assert(index >= 1 && index <= 8);
    return index;
}
static void reset_case(int target, int why, int invalid, int initial_ok) {
    events.clear();
    target_index = target; reason = why; invalid_mask = invalid;
    initial_match = initial_ok; region_a_calls = 0;
}
static int whose_territory(int faction, int x, int y, void *, bool) {
    record('W', x, y);
    if (x == AX && y == AY) return initial_match ? faction : faction + 1;
    int index = neighbor_index(x, y);
    return (reason == 1 && index == target_index) ? faction + 1 : faction;
}
static uint32_t region_at(int x, int y) {
    record('R', x, y);
    if (x == BX && y == BY) return 77;
    assert(x == AX && y == AY);
    ++region_a_calls;
    return (reason == 2 && region_a_calls == (unsigned int)target_index) ? 78 : 77;
}
static int xrange(int value) { record('X', value, 0); return value; }
static int on_map(int x, int y) {
    record('M', x, y);
    int index = neighbor_index(x, y);
    return (invalid_mask & (1 << index)) == 0;
}
static int is_ocean(int x, int y) {
    record('O', x, y);
    return reason == 0 && neighbor_index(x, y) == target_index;
}
static void setup_offsets() {
    for (int i = 0; i < 9; ++i) {
        RadiusOffsetX[i] = i * 10;
        RadiusOffsetY[i] = i * 20;
    }
}
static void expect_event(unsigned int index, char op, int x, int y) {
    assert(index < events.size());
    if (events[index].op != op || events[index].x != x || events[index].y != y) {
        fprintf(stderr, "event %u: got %c(%d,%d), wanted %c(%d,%d)\n", index,
                     events[index].op, events[index].x, events[index].y, op, x, y);
        assert(false);
    }
}
static void expect_neighbor_prefix(unsigned int &at, int index) {
    int x = AX + index * 10, y = AY + index * 20;
    expect_event(at++, 'X', x, 0);
    expect_event(at++, 'M', x, y);
}
'''


SUFFIX = r'''
int main() {
    int index;
    (void)on_map;
    setup_offsets();
    for (index = 1; index <= 8; ++index) {
        reset_case(index, (index - 1) % 3, 0, 1);
        assert(coast_or_border(AX, AY, BX, BY, 4) == index);
    }

    // Initial territory rejection happens before either region read.
    reset_case(0, 0, 0, 0);
    assert(coast_or_border(AX, AY, BX, BY, 4) == 0);
    assert(events.size() == 1);
    expect_event(0, 'W', AX, AY);

    // An off-map first neighbor is skipped; the next ocean neighbor returns
    // its index without a territory call or a point-A region read.
    reset_case(2, 0, 1 << 1, 1);
    assert(coast_or_border(AX, AY, BX, BY, 4) == 2);
    unsigned int at = 0;
    expect_event(at++, 'W', AX, AY);
    expect_event(at++, 'R', BX, BY);
    expect_neighbor_prefix(at, 1);
    expect_neighbor_prefix(at, 2);
    expect_event(at++, 'O', AX + 20, AY + 40);
    assert(at == events.size());

    // A same-owner land neighbor reads point A only after territory succeeds;
    // a region mismatch at neighbor 2 returns 2, not boolean 1.
    reset_case(2, 2, 0, 1);
    assert(coast_or_border(AX, AY, BX, BY, 4) == 2);
    at = 0;
    expect_event(at++, 'W', AX, AY);
    expect_event(at++, 'R', BX, BY);
    for (index = 1; index <= 2; ++index) {
        int x = AX + index * 10, y = AY + index * 20;
        expect_neighbor_prefix(at, index);
        expect_event(at++, 'O', x, y);
        expect_event(at++, 'W', x, y);
        expect_event(at++, 'R', AX, AY);
    }
    assert(at == events.size());

    // An out-of-territory land neighbor short-circuits before point-A region.
    reset_case(1, 1, 0, 1);
    assert(coast_or_border(AX, AY, BX, BY, 4) == 1);
    assert(events.size() == 6);
    expect_event(0, 'W', AX, AY);
    expect_event(1, 'R', BX, BY);
    expect_event(2, 'X', AX + 10, 0);
    expect_event(3, 'M', AX + 10, AY + 20);
    expect_event(4, 'O', AX + 10, AY + 20);
    expect_event(5, 'W', AX + 10, AY + 20);

    // Exhaustion visits all eight valid same-owner, same-region neighbors.
    reset_case(0, 0, 0, 1);
    assert(coast_or_border(AX, AY, BX, BY, 4) == 0);
    assert(events.size() == 42);
    puts("coast actual-source controls passed");
    return 0;
}
'''


def compile_and_run(body: str, compiler: str):
    program = PRELUDE + body + SUFFIX
    with tempfile.TemporaryDirectory(prefix="coast-native-contract-") as tmp:
        cpp = Path(tmp) / "control.cpp"
        exe = Path(tmp) / "control"
        cpp.write_text(program)
        built = subprocess.run(
            [compiler, "-std=c++98", "-Wall", "-Wextra", "-Werror", str(cpp), "-o", str(exe)],
            capture_output=True, text=True,
        )
        assert built.returncode == 0, built.stdout + built.stderr
        return subprocess.run([str(exe)], capture_output=True, text=True)


def test_actual_body_neighbor_indices_short_circuits_and_region_reads():
    body = current_body()
    compilers = [compiler for compiler in ("g++", "clang++") if shutil.which(compiler)]
    assert compilers
    for compiler in compilers:
        run = compile_and_run(body, compiler)
        assert run.returncode == 0, run.stdout + run.stderr
        assert "controls passed" in run.stdout


def damage_mutants(body: str):
    region_b = "uint32_t region_b = region_at(x_point_b, y_point_b);"
    region_a_test = "region_at(x_point_a, y_point_a) != region_b"
    assert body.count("return i;") == 1
    assert body.count(region_b) == 1
    assert body.count(region_a_test) == 1
    assert body.count("on_map(x_radius, y_radius) &&") == 1

    cached = body.replace(region_b,
                          "uint32_t region_a = region_at(x_point_a, y_point_a);\n    " + region_b, 1)
    cached = cached.replace(region_a_test, "region_a != region_b", 1)
    predicate = re.compile(
        r"\(is_ocean\(x_radius, y_radius\)\s+\|\|\s+"
        r"whose_territory\(faction_id, x_radius, y_radius, NULL, false\)"
        r" != \(int\)faction_id\s+\|\|\s+"
        r"region_at\(x_point_a, y_point_a\) != region_b\)"
    )
    reordered, substitutions = predicate.subn(
        "(region_at(x_point_a, y_point_a) != region_b\n"
        "                || is_ocean(x_radius, y_radius)\n"
        "                || whose_territory(faction_id, x_radius, y_radius, NULL, false) != (int)faction_id)",
        body, count=1,
    )
    assert substitutions == 1
    no_map_gate = body.replace("on_map(x_radius, y_radius) &&", "1 &&", 1)
    mutants = {
        "boolean result": body.replace("return i;", "return true;", 1),
        "eager point-A region cache": cached,
        "region read before ocean short-circuit": reordered,
        "removed on_map gate": no_map_gate,
    }
    return mutants


def test_damage_controls_reject_boolean_return_cached_or_reordered_region_and_missing_map_gate():
    body = current_body()
    mutants = damage_mutants(body)
    compilers = [compiler for compiler in ("g++", "clang++") if shutil.which(compiler)]
    assert compilers
    for label, mutant in mutants.items():
        assert mutant != body, label
        for compiler in compilers:
            run = compile_and_run(mutant, compiler)
            assert run.returncode != 0, f"{label} mutant survived with {compiler}"


def test_vc6_x86_actual_body_and_damage_controls_under_existing_lease(tmp_path):
    """Optional 32-bit source control; coordinator supplies and owns the lease."""
    if not os.environ.get("MATCH_RECOVERY_RESOURCE_LEASE"):
        pytest.skip("native coast controls require an existing coordinator compiler lease")

    from match_recovery.measurement import Measurement
    from match_recovery.processes import run_bounded
    from recovery.environment.compile import compile_object
    from recovery.environment.wine import VC6_ROOT, environment

    if not (VC6_ROOT / "BIN/CL.EXE").is_file() or shutil.which("wine32") is None:
        pytest.skip("VC6 and Wine32 are required under the coordinator lease")

    settings = Measurement.read(ROOT)
    wrapper = ROOT / "recovery/environment/wine.py"
    command = (sys.executable, str(wrapper), "--tool=cl", "/nologo")
    body = current_body()

    def build_native(name, flags, selected_body):
        source = tmp_path / (name + ".cpp")
        obj = tmp_path / (name + ".obj")
        exe = tmp_path / (name + ".exe")
        source.write_text(PRELUDE + selected_body + SUFFIX, encoding="utf-8")
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

    mutants = damage_mutants(body)
    for profile_index, flags in enumerate(settings.flag_sets):
        good = build_native("coast-good-" + str(profile_index), flags, body)
        code, output = run_native(good)
        assert code == 0, f"profile {profile_index}: {output} ({code})"
        for mutant_index, (label, mutant) in enumerate(mutants.items()):
            damaged = build_native("coast-bad-%d-%d" % (profile_index, mutant_index), flags, mutant)
            code, output = run_native(damaged)
            assert code != 0, f"profile {profile_index} {label} mutant survived: {output}"
