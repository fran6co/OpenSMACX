"""Actual-source behavior controls for the original world_ruin entry.

The fixture exercises direct placement through the original 25-point radius
and checks that ocean classification and the fungus/supply write use each
computed neighbor row. It does not model the full game or establish runtime
reachability. Native cases are lease-guarded and must only be run by the
coordinator after adoption.
"""
from pathlib import Path
import os
import re
import shutil
import subprocess
import sys

import pytest
from match_recovery.source import read_file


ROOT = Path(__file__).resolve().parents[2]
SOURCE = ROOT / "src/map.cpp"
HEADER = ROOT / "src/map.h"
BASE = "361f5176199a52ac0bc3f425cb139f0230730824"
ORIGINAL = "// ORIGINAL: 0x005C64A0 ?world_ruin@@YAXHH@Z 0x005C64A0-0x005C6734"
RADIUS_RANGE = (1, 9, 25, 49, 81, 121, 169, 225, 289)
CONSTANTS = {
    "ALT_SHORE_LINE": 3,
    "BIT_FUNGUS": 0x20,
    "BIT_MONOLITH": 0x2000,
    "BIT_SUPPLY_REMOVE": 0x100000,
    "BIT2_RUINS": 0x20,
    "LM_RUINS": 5,
    "MaxNaturalNum": 16,
}


def extract_body(source):
    start = source.index("void __cdecl world_ruin(")
    brace = source.index("{", start)
    depth = 0
    for end in range(brace, len(source)):
        if source[end] == "{":
            depth += 1
        elif source[end] == "}":
            depth -= 1
            if depth == 0:
                return source[start:end + 1]
    raise AssertionError("unterminated world_ruin body")


def parsed_constants():
    header = HEADER.read_text(encoding="utf-8")
    values = {}
    for name, expected in CONSTANTS.items():
        match = re.search(r"\b" + re.escape(name) + r"\s*=\s*(0x[0-9A-Fa-f]+|\d+)", header)
        assert match is not None, "missing native declaration " + name
        values[name] = int(match.group(1), 0)
        assert values[name] == expected, (name, values[name], expected)
    return values


def source_controls():
    source = SOURCE.read_text(encoding="utf-8")
    body = extract_body(source)
    assert ORIGINAL in source
    assert "if (is_ocean(x_radius, y_radius)) {" in body
    assert "bit_set(x_radius, y_radius, BIT_FUNGUS | BIT_SUPPLY_REMOVE, true)" in body
    assert "bit2_set(x_radius, y_radius, BIT2_RUINS, true)" in body
    assert "code_set(x_radius, y_radius, i)" in body
    assert "if (on_map(x_radius, y_radius)) {" in body
    assert "if (i < RadiusRange[2] || !is_ocean(x_radius, y_radius)) {" in body
    assert "if (i && i < 9)" in body and "if (!i)" in body
    assert "else if (i < 9)" in body
    assert "if (++loc_attempts >= 1000)" in body
    assert "do {" in body and "near_landmark(x, y)" in body
    assert "new_landmark(x, y + 2, StringTable.get(Natural[LM_RUINS].name))" in body
    assert body.index("world_alt_set(x_radius, y_radius, ALT_SHORE_LINE, true)") \
        < body.index("if (is_ocean(x_radius, y_radius)) {")
    assert "const int RadiusRange[9] = { 1, 9, 25, 49, 81, 121, 169, 225, 289 };" in source
    row = {item.address: item for item in read_file(SOURCE)}[0x005C64A0]
    assert row.name == "?world_ruin@@YAXHH@Z"
    assert row.image_spans == ((0x005C64A0, 0x005C6734),)
    assert row.byte_exact is False and row.semantic is False
    marker = source.index(ORIGINAL)
    body_at = source.index("void __cdecl world_ruin(")
    annotation = source[marker:body_at]
    assert "// prototype void (__cdecl ?world_ruin@@YAXHH@Z)(int xCoord, int yCoord)" in annotation
    assert "// LEVER:" in annotation and "// TRIED:" in annotation
    parsed_constants()
    return body


PRELUDE = r'''#include <assert.h>
#include <stdio.h>
#include <string.h>
#define __cdecl
#define rand fixture_rand
typedef unsigned int uint32_t;
typedef int BOOL;
enum { @CONSTANTS@ };
const int RadiusRange[9] = { 1, 9, 25, 49, 81, 121, 169, 225, 289 };
int RadiusOffsetX[289], RadiusOffsetY[289];
static int MapLatitudeBounds = 64, MapLongitudeBounds = 64;
struct NaturalEntry { int name; };
struct Table { const char *get(int value); };
static NaturalEntry Natural[MaxNaturalNum];
static Table StringTable;
static int ocean_index;
static int map_index;
static int alt_count, bit_count, bit2_count, code_count, label_count, rand_count, near_count;
static int fungus_supply_count, monolith_count, center_clear_count;
static int ocean_mask;
static int tile_x[25], tile_y[25];
const char *Table::get(int value) { assert(value == 57); return "The Ruins"; }
static int xrange(int x) { return x; }
static int on_map(int x, int y) {
    if (map_index++ == 0) { assert(x == 100 && y == 200); return 1; }
    assert(map_index >= 2 && map_index <= 26);
    return 1;
}
static int is_ocean(int x, int y) {
    assert(ocean_index < 25);
    assert(x == tile_x[ocean_index] && y == tile_y[ocean_index]);
    return (ocean_mask & (1 << ocean_index++)) != 0;
}
static int fixture_rand(void) { ++rand_count; return 0; }
static int near_landmark(int x, int y) { (void)x; (void)y; ++near_count; return 0; }
static void world_alt_set(int x, int y, int altitude, BOOL set) {
    int i = alt_count++;
    assert(i < 25 && x == tile_x[i] && y == tile_y[i]);
    assert(altitude == ALT_SHORE_LINE && set);
}
static void bit_set(int x, int y, int mask, BOOL set) {
    int index = -1, i;
    for (i = 0; i < 25; ++i) if (tile_x[i] == x && tile_y[i] == y) { index = i; break; }
    assert(index >= 0 && y == tile_y[index]);
    if (mask == (BIT_FUNGUS | BIT_SUPPLY_REMOVE)) {
        assert(set);
        assert((ocean_mask & (1 << index)) == 0);
        ++fungus_supply_count;
    } else if (mask == BIT_MONOLITH) {
        assert(set && index > 0 && index < 9);
        ++monolith_count;
    } else {
        assert(!set && index == 0 && mask == BIT_FUNGUS);
        ++center_clear_count;
    }
    ++bit_count;
}
static void bit2_set(int x, int y, int mask, BOOL set) {
    int i = bit2_count++;
    assert(i < 25 && x == tile_x[i] && y == tile_y[i]);
    assert(mask == BIT2_RUINS && set);
}
static void code_set(int x, int y, int code) {
    int i = code_count++;
    assert(i < 25 && code == i && x == tile_x[i] && y == tile_y[i]);
}
static void new_landmark(int x, int y, const char *name) {
    assert(x == 100 && y == 202 && name != 0 && strcmp(name, "The Ruins") == 0);
    ++label_count;
}
'''


MAIN = r'''
int main() {
    int i;
    Natural[LM_RUINS].name = 57;
    for (i = 0; i < 25; ++i) {
        RadiusOffsetX[i] = i - 12;
        RadiusOffsetY[i] = 2 * i + 1;
        tile_x[i] = 100 + RadiusOffsetX[i];
        tile_y[i] = 200 + RadiusOffsetY[i];
    }
    // Exercise ocean and land branches, including the center and both sides
    // of the i < 9 monolith cutoff.
    ocean_mask = (1 << 1) | (1 << 3) | (1 << 9) | (1 << 24);
    world_ruin(100, 200);
    assert(ocean_index == 25 && map_index == 26);
    assert(alt_count == 25 && bit2_count == 25 && code_count == 25);
    assert(bit_count == 30 && fungus_supply_count == 21);
    assert(monolith_count == 8 && center_clear_count == 1 && label_count == 1);
    assert(rand_count == 0 && near_count == 0);
    return 0;
}
'''


def program_source(body, values):
    enums = ",\n       ".join(name + " = " + hex(value) for name, value in values.items())
    return PRELUDE.replace("@CONSTANTS@", enums) + "\n" + body + "\n" + MAIN


def compile_run(program, compiler, tmp_path, stem):
    source = tmp_path / (stem + ".cpp")
    exe = tmp_path / stem
    source.write_text(program, encoding="utf-8")
    built = subprocess.run([compiler, "-std=c++98", "-Wall", "-Wextra", "-Werror",
                            str(source), "-o", str(exe)], capture_output=True, text=True)
    assert built.returncode == 0, built.stdout + built.stderr
    return subprocess.run([str(exe)], capture_output=True, text=True)


OLD_OCEAN_ROW = ("if (is_ocean(x_radius, y_radius)) {", "if (is_ocean(x_radius, y)) {")
OLD_FUNGUS_ROW = (
    "bit_set(x_radius, y_radius, BIT_FUNGUS | BIT_SUPPLY_REMOVE, true)",
    "bit_set(x_radius, y, BIT_FUNGUS | BIT_SUPPLY_REMOVE, true)",
)
OLD_LABEL_Y = (
    "new_landmark(x, y + 2, StringTable.get(Natural[LM_RUINS].name))",
    "new_landmark(x, y, StringTable.get(Natural[LM_RUINS].name))",
)


def test_ruin_neighbor_rows_and_three_independent_contract_mutants(tmp_path):
    body = source_controls()
    values = parsed_constants()
    compilers = [compiler for compiler in ("g++", "clang++") if shutil.which(compiler)]
    assert compilers
    good = program_source(body, values)
    for mutant in (OLD_OCEAN_ROW, OLD_FUNGUS_ROW, OLD_LABEL_Y):
        assert mutant[0] in body
    for compiler in compilers:
        result = compile_run(good, compiler, tmp_path, "ruin-good")
        assert result.returncode == 0, result.stdout + result.stderr
        for index, mutant in enumerate((OLD_OCEAN_ROW, OLD_FUNGUS_ROW, OLD_LABEL_Y)):
            damaged = body.replace(mutant[0], mutant[1], 1)
            assert damaged != body
            result = compile_run(program_source(damaged, values), compiler, tmp_path,
                                 "ruin-center-row-%d" % index)
            assert result.returncode != 0, "coordinate/label mutant survived: " + mutant[0]


def test_vc6_x86_ruin_neighbor_rows_both_profiles_when_leased(tmp_path):
    """Optional native companion; coordinator lease is required to execute."""
    if not os.environ.get("MATCH_RECOVERY_RESOURCE_LEASE"):
        pytest.skip("native Ruin controls require an existing coordinator lease")
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
    values = parsed_constants()
    for profile, flags in enumerate(settings.flag_sets):
        for mutant_index, mutant in enumerate((None, OLD_OCEAN_ROW, OLD_FUNGUS_ROW, OLD_LABEL_Y)):
            candidate = body if mutant is None else body.replace(mutant[0], mutant[1], 1)
            assert mutant is None or candidate != body
            stem = "ruin-%d-%d" % (profile, mutant_index)
            source = tmp_path / (stem + ".cpp")
            obj = tmp_path / (stem + ".obj")
            exe = tmp_path / (stem + ".exe")
            source.write_text(program_source(candidate, values), encoding="utf-8")
            obj.write_bytes(compile_object(source, command, tuple(flags) + settings.layout_flags))
            linked = run_bounded([sys.executable, str(wrapper), "--tool=link", "/nologo",
                                  "/subsystem:console", "/incremental:no", "/out:" + str(exe),
                                  str(obj), "kernel32.lib"], cwd=tmp_path, timeout=60)
            assert linked.returncode == 0, (linked.stdout + linked.stderr).decode(errors="replace")
            result = subprocess.run(["wine32", str(exe)], env=environment(),
                                    stdout=subprocess.PIPE, stderr=subprocess.PIPE, timeout=30)
            output = (result.stdout + result.stderr).decode(errors="replace")
            if mutant is None:
                assert result.returncode == 0, (profile, result.returncode, output)
            else:
                assert result.returncode != 0, (profile, mutant_index, "coordinate/label mutant survived", output)
