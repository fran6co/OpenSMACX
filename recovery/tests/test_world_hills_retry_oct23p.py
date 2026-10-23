"""Actual-source host controls for the pinned build_hills retry path.

These controls execute the complete mapped C++ body with narrow deterministic
terrain/RNG doubles. They preserve the odd-y x increment and original attempt
cap; they are not a native ABI or game-runtime proof.
"""
from pathlib import Path
import re
import shutil
import subprocess

import pytest
from match_recovery.source import read_file
from test_net_hotseat_owner_actual_source_oct6 import extract_definition


ROOT = Path(__file__).resolve().parents[2]
SOURCE = ROOT / "src/world.cpp"
ADDRESS = 0x005C2B40
PIN = "01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf"


def actual_parts():
    source = SOURCE.read_text(encoding="utf-8")
    record, = (row for row in read_file(SOURCE) if row.address == ADDRESS)
    assert record.name == "?build_hills@@YAXH@Z"
    assert record.image_spans == ((0x005C2B40, 0x005C2CA8),)
    body = extract_definition(record.region, "void __cdecl build_hills(int altitude)")
    assert "BODY GOES HERE" not in body

    alpha = (ROOT / "src/alpha.h").read_text(encoding="utf-8")
    worldbuilder = extract_definition(alpha, "struct RulesWorldbuilder {") + ";"
    map_header = (ROOT / "src/map.h").read_text(encoding="utf-8")
    terrain = extract_definition(map_header, "enum TerrainAltitude {") + ";"
    helpers = "\n".join(extract_definition(map_header, signature) for signature in (
        "MEASURED inline BOOL __cdecl on_map(",
        "MEASURED inline int __cdecl xrange(",
    ))
    map_source = (ROOT / "src/map.cpp").read_text(encoding="utf-8")
    arrays = []
    for name in ("RadiusOffsetX", "RadiusOffsetY"):
        match = re.search(r"const int " + name + r"\[289\]\s*=\s*\{.*?\};",
                          map_source, re.S)
        assert match is not None, "missing actual radius table " + name
        arrays.append(match.group())
    assert source.count("// ORIGINAL: 0x005C2B40") == 1
    assert "uint32_t plateau_base" in worldbuilder
    return body, worldbuilder, terrain, helpers, "\n".join(arrays)


HARNESS = r'''#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#define __cdecl
#define MEASURED
#define rand fixture_rand
typedef int BOOL;
typedef char host_int_must_be_32[(sizeof(int)==4) ? 1 : -1];
WORLD_BUILDER
TERRAIN_ENUM
RADIUS_TABLES
int MapLongitudeBounds=40, MapLatitudeBounds=40, MapIsFlat=0;
int MapOceanCoverage=0;
#define MapLongitudeBounds MapLongitudeBounds
#define MapLatitudeBounds MapLatitudeBounds
#define MapIsFlat MapIsFlat
#define MapOceanCoverage MapOceanCoverage
HELPERS
static RulesWorldbuilder WorldBuilder;
static int raw_values[256];
static int rand_calls, alt_calls, reads_this_seed, mode, reject_seed, reject_read;
static int paint_calls, callback_calls, painted_x, painted_y, painted_altitude, painted_radius;
static int target_altitude;
static std::string events;
static void require(int ok, int line) {
    if (!ok) { fprintf(stderr, "assertion line %d\n", line); exit(83); }
}
#define check(x) require((x), __LINE__)
static int fixture_rand() {
    if ((rand_calls & 1) == 0) reads_this_seed=0;
    events += 'r';
    return raw_values[rand_calls++];
}
static int signed_bits(uint32_t bits) {
    int value;
    memcpy(&value,&bits,sizeof(value));
    return value;
}
static int alt_at(int x, int y) {
    (void)x; (void)y;
    events += 'a';
    ++alt_calls;
    ++reads_this_seed;
    if (mode == 1 && (rand_calls / 2 - 1) == reject_seed
        && reads_this_seed == reject_read)
        return target_altitude - 2;
    if (mode == 2) return target_altitude - 2;
    return target_altitude - 1;
}
static void paint_land(int x, int y, int altitude, int radius) {
    events += 'p';
    ++paint_calls;
    painted_x=x; painted_y=y; painted_altitude=altitude; painted_radius=radius;
}
void do_all_non_input() { events += 'c'; ++callback_calls; }
BODY
static void reset_case() {
    rand_calls=alt_calls=reads_this_seed=0;
    paint_calls=callback_calls=0;
    reject_seed=0; reject_read=1; mode=0;
    MapLongitudeBounds=40; MapLatitudeBounds=40; MapIsFlat=0;
    MapOceanCoverage=0;
    WorldBuilder.plateau_mod=8; WorldBuilder.plateau_base=4;
    target_altitude=3;
    events.clear();
    for (int i=0;i<256;++i) raw_values[i]=0;
}
static int main_retry() {
    reset_case();
    raw_values[0]=4; raw_values[1]=0; // seed (4,4)
    raw_values[2]=5; raw_values[3]=1; // odd x and odd y preserve x++ quirk
    mode=1; reject_seed=0; reject_read=1;
    build_hills(target_altitude);
    check(rand_calls==4 && alt_calls==10);
    check(events==std::string("rra")+"rr"+"aaaaaaaaa"+"pc");
    check(paint_calls==1 && callback_calls==1);
    check(painted_x==5 && painted_y==5); // x decremented, then incremented for odd y
    check(painted_altitude==3);
    return 0;
}
static int main_first_valid_and_order() {
    reset_case();
    raw_values[0]=5; raw_values[1]=0;
    build_hills(target_altitude);
    check(rand_calls==2 && alt_calls==9);
    check(events==std::string("rr")+"aaaaaaaaa"+"pc");
    check(paint_calls==1 && callback_calls==1);
    check(painted_x==4 && painted_y==4); // odd x rounds down
    return 0;
}
static int main_ninth_neighbor_and_strict_threshold() {
    reset_case();
    mode=1; reject_seed=0; reject_read=9;
    raw_values[0]=39; raw_values[1]=0; // cylindrical x wraps at the radius edge
    raw_values[2]=7; raw_values[3]=0;
    build_hills(target_altitude);
    check(alt_calls==18 && rand_calls==4);
    check(events==std::string("rr")+"aaaaaaaaa"+"rr"+"aaaaaaaaa"+"pc");
    check(painted_x==6 && painted_y==4);
    // The accepted second seed's equal-to-threshold neighbors are not rejected.
    check(alt_calls==18);
    return 0;
}
static int main_offmap_neighbors_are_ignored() {
    reset_case();
    MapLongitudeBounds=8; MapLatitudeBounds=2; MapIsFlat=1;
    build_hills(target_altitude);
    check(rand_calls==0 && alt_calls==0);
    check(events=="pc" && paint_calls==1 && callback_calls==1);
    check(painted_x==4 && painted_y==4);
    return 0;
}
static int main_flat_flag_uses_low_bit() {
    const int flat_values[4]={2,3,signed_bits(0x80000000u),signed_bits(0x80000001u)};
    for (int n=0;n<4;++n) {
        reset_case();
        MapIsFlat=flat_values[n];
        raw_values[0]=25; raw_values[1]=0;
        build_hills(target_altitude);
        check(paint_calls==1 && callback_calls==1);
        check(painted_x==((flat_values[n]&1) ? 28 : 24));
        check(painted_y==4);
    }
    return 0;
}
static int main_attempt_cap_keeps_rng_before_cap() {
    reset_case();
    mode=2; // every tested seed is rejected at its first valid neighbor
    build_hills(target_altitude);
    check(rand_calls==102); // includes both random draws for attempt 51
    check(alt_calls==50);   // attempt 51 exits before neighbor scanning
    check(paint_calls==0 && callback_calls==0);
    check(events.size()==152);
    for (int n=0;n<50;++n) check(events.substr(n*3,3)=="rra");
    check(events[150]=='r' && events[151]=='r');
    return 0;
}
static int main_signed_radius_cases() {
    reset_case();
    MapOceanCoverage=1;
    WorldBuilder.plateau_mod=0x80000000u;
    WorldBuilder.plateau_base=0x80000001u;
    build_hills(3);
    check(paint_calls==1);
    check(static_cast<uint32_t>(painted_radius)==0x60000001u);

    reset_case();
    MapOceanCoverage=2;
    WorldBuilder.plateau_mod=0x7FFFFFFFu;
    WorldBuilder.plateau_base=0x80000001u;
    target_altitude=ALT_1_LEVEL_ABOVE_SEA;
    build_hills(ALT_1_LEVEL_ABOVE_SEA); // no /4; signed base /2 remains
    check(paint_calls==1);
    check(painted_radius==-1073741823);

    reset_case();
    MapOceanCoverage=0;
    WorldBuilder.plateau_mod=0x40000001u;
    WorldBuilder.plateau_base=7;
    target_altitude=ALT_1_LEVEL_ABOVE_SEA;
    build_hills(ALT_1_LEVEL_ABOVE_SEA);
    check(static_cast<uint32_t>(painted_radius)==0x80000009u); // low-32-bit radius sum
    return 0;
}
int main(int argc, char **argv) {
    if (argc!=2) return 82;
    if (!strcmp(argv[1],"retry")) return main_retry();
    if (!strcmp(argv[1],"first")) return main_first_valid_and_order();
    if (!strcmp(argv[1],"ninth")) return main_ninth_neighbor_and_strict_threshold();
    if (!strcmp(argv[1],"offmap")) return main_offmap_neighbors_are_ignored();
    if (!strcmp(argv[1],"flat")) return main_flat_flag_uses_low_bit();
    if (!strcmp(argv[1],"cap")) return main_attempt_cap_keeps_rng_before_cap();
    if (!strcmp(argv[1],"radius")) return main_signed_radius_cases();
    return 82;
}
'''


def compiled_program(tmp_path, damage=None, compiler=None):
    compiler = compiler or shutil.which("g++") or shutil.which("clang++")
    if compiler is None:
        pytest.skip("host C++ compiler required")
    body, worldbuilder, terrain, helpers, tables = actual_parts()
    if damage:
        before, after = damage
        assert body.count(before) == 1
        body = body.replace(before, after, 1)
    generated = (HARNESS.replace("WORLD_BUILDER", worldbuilder)
                 .replace("TERRAIN_ENUM", terrain)
                 .replace("RADIUS_TABLES", tables)
                 .replace("HELPERS", helpers)
                 .replace("BODY", body))
    tmp_path.mkdir(parents=True, exist_ok=True)
    source = tmp_path / "world_hills_actual.cpp"
    source.write_text(generated, encoding="utf-8")
    binary = tmp_path / "world_hills_actual"
    built = subprocess.run([compiler, "-std=c++98", "-Wall", "-Wextra", "-Werror",
                            "-Wno-sign-compare", str(source), "-o", str(binary)],
                           capture_output=True, text=True, timeout=30)
    assert built.returncode == 0, built.stderr
    return binary


def run_case(tmp_path, case, damage=None, compiler=None):
    binary = compiled_program(tmp_path, damage, compiler)
    return subprocess.run([str(binary), case], capture_output=True, text=True, timeout=10)


@pytest.mark.parametrize("compiler_name", ["g++", "clang++"])
def test_actual_build_hills_retry_radius_and_order(tmp_path, compiler_name):
    compiler = shutil.which(compiler_name)
    if not compiler:
        pytest.skip(compiler_name + " unavailable")
    for case in ("retry", "first", "ninth", "offmap", "flat", "cap", "radius"):
        result = run_case(tmp_path / (compiler_name + "_" + case), case, compiler=compiler)
        assert result.returncode == 0, case + ": " + result.stderr


@pytest.mark.parametrize(("case", "before", "after"), [
    ("retry", "if (rejected_seed) {\n            continue;\n        }", "if (rejected_seed) {\n            ;\n        }"),
    ("cap", "if (i > 100)", "if (i >= 100)"),
    ("radius", "int plat_base;", "uint32_t plat_base;"),
    ("first", "do_all_non_input();", "/* callback omitted */"),
    ("flat", "((MapIsFlat & 1) * 8)", "(MapIsFlat * 8)"),
])
def test_compiling_behavior_damage_is_detected(tmp_path, case, before, after):
    # Each negative control first proves the unmodified actual body satisfies
    # this exact harness, so a failed mutant is not a fixture false positive.
    positive = run_case(tmp_path / "positive", case)
    assert positive.returncode == 0, positive.stderr
    damaged = run_case(tmp_path / "damaged", case, (before, after))
    assert damaged.returncode == 83, damaged.stderr
