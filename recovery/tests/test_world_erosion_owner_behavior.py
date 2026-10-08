"""Private actual-source controls for the future map.cpp world_erosion body.

The test intentionally has no dependency on ignored candidate or host files.
It is parked with the frozen packet until the coordinator installs the body;
once installed, it extracts and compiles that actual source definition.
"""
from pathlib import Path
import shutil
import subprocess

import pytest


REPO = Path(__file__).resolve().parents[2]
SOURCE = REPO / "src" / "map.cpp"
SIGNATURE = "void __cdecl world_erosion()"


def extract_function(source: str, signature: str) -> str:
    start = source.find(signature)
    assert start >= 0, "world_erosion owner body missing from src/map.cpp"
    opening = source.index("{", start)
    depth = 0
    state = "code"
    index = opening
    while index < len(source):
        char = source[index]
        nxt = source[index + 1] if index + 1 < len(source) else ""
        if state == "code":
            if char == "/" and nxt == "/":
                state = "line"
                index += 2
                continue
            if char == "/" and nxt == "*":
                state = "block"
                index += 2
                continue
            if char == '"':
                state = "string"
            elif char == "'":
                state = "character"
            elif char == "{":
                depth += 1
            elif char == "}":
                depth -= 1
                if depth == 0:
                    return source[start:index + 1]
        elif state == "line":
            if char == "\n":
                state = "code"
        elif state == "block":
            if char == "*" and nxt == "/":
                state = "code"
                index += 2
                continue
        elif state in ("string", "character"):
            if char == "\\":
                index += 2
                continue
            if (state == "string" and char == '"') or (
                state == "character" and char == "'"
            ):
                state = "code"
        index += 1
    raise AssertionError(f"unclosed function body for {signature}")


FIXTURE_PREFIX = r'''
#include <stdint.h>
#include <stdio.h>
#include <string.h>

struct Map {
    uint8_t climate, contour, val2, region, visibility, val3, unk_1;
    int8_t territory;
    uint32_t bit, bit2, bit_visible[7];
};
struct RulesWorldbuilder {
    int deep_water, shelf, plains, beach;
    uint32_t hills;
    int peaks;
};
struct Continent { uint32_t tile_count; uint8_t padding[0x18]; };
struct PathFixture { void continents(); };
struct Fractal { void init(); int sample(int, int); };

#define __cdecl
const int VehMoveDeltaX[9] = {1,2,1,0,-1,-2,-1,0,0};
const int VehMoveDeltaY[9] = {-1,0,1,2,1,0,-1,-2,0};
const int RadiusOffsetX[25] = {
    0,1,2,1,0,-1,-2,-1,0,2,2,-2,-2,1,3,3,1,-1,-3,-3,-1,4,-4,0,0
};
const int RadiusOffsetY[25] = {
    0,-1,0,1,2,1,0,-1,-2,-2,2,2,-2,-3,-1,1,3,3,1,-1,-3,0,0,4,-4
};
extern uint32_t MapArea;
extern uint32_t MapLongitude;
extern int MapLongitudeBounds;
extern int MapLatitudeBounds;
extern int MapIsFlat;
extern uint32_t MapLandCoverage;
extern uint32_t MapErosiveForces;
extern uint32_t MapOceanCoverage;
extern uint32_t AltNatural[11];
extern RulesWorldbuilder WorldBuilder;
extern Continent Continents[128];
extern PathFixture Paths;
Map *map_tiles();
int quick_root(int);
void alt_put_detail(int, int, int);
void do_all_non_input();
void world_shorelines();

'''


FIXTURE_SUFFIX = r'''
static Map Tiles[15];
uint32_t MapArea = 15;
uint32_t MapLongitude = 3;
int MapLongitudeBounds = 6;
int MapLatitudeBounds = 5;
int MapIsFlat = 1;
uint32_t MapLandCoverage = 2;
uint32_t MapErosiveForces = 2;
uint32_t MapOceanCoverage = 1;
uint32_t AltNatural[11];
RulesWorldbuilder WorldBuilder = {20,5,1,3,10,3};
Continent Continents[128];
PathFixture Paths;
static int sample_value;
static int fractal_inits;
static int fractal_samples;
static int continent_calls;
static int shoreline_calls;
static int progress_calls;
static int altitude_writes;

void Fractal::init() { ++fractal_inits; }
int Fractal::sample(int, int) { ++fractal_samples; return sample_value; }
void PathFixture::continents() { ++continent_calls; }
Map *map_tiles() { return Tiles; }
int quick_root(int value) { return value > 0 ? 1 : 0; }
void alt_put_detail(int x, int y, int detail) {
    Tiles[MapLongitude * y + (x >> 1)].contour = static_cast<uint8_t>(detail);
    ++altitude_writes;
}
void do_all_non_input() { ++progress_calls; }
void world_shorelines() { ++shoreline_calls; }

static void reset(int altitude, int contour, int sample) {
    memset(Tiles, 0, sizeof(Tiles));
    memset(Continents, 0, sizeof(Continents));
    memset(AltNatural, 0, sizeof(AltNatural));
    int i;
    for (i = 0; i < 15; ++i) {
        Tiles[i].climate = static_cast<uint8_t>(altitude << 5);
        Tiles[i].contour = static_cast<uint8_t>(contour);
        Tiles[i].region = 1;
    }
    sample_value = sample;
    AltNatural[3] = 100;
    MapLandCoverage = 2;
    MapErosiveForces = 2;
    MapOceanCoverage = 1;
    WorldBuilder.deep_water = 20;
    WorldBuilder.shelf = 5;
    WorldBuilder.plains = 1;
    WorldBuilder.beach = 3;
    WorldBuilder.hills = 10;
    WorldBuilder.peaks = 3;
    fractal_inits = 0;
    fractal_samples = 0;
    continent_calls = 0;
    shoreline_calls = 0;
    progress_calls = 0;
    altitude_writes = 0;
}

static int fail(const char *message, int tile) {
    fprintf(stderr, "FAIL: %s (tile %d detail %u)\n", message, tile,
            Tiles[tile].contour);
    return 1;
}

int main() {
    int failures = 0;
    int i;

    reset(5, 20, 50);
    world_erosion();
    if (fractal_inits != 1 || fractal_samples != 60)
        failures += fail("two octave samples occur for every visited tile/pass", 0);
    if (continent_calls != 2 || shoreline_calls != 1)
        failures += fail("continent rebuild runs twice and shoreline once", 0);
    if (progress_calls != 10 || altitude_writes != 15)
        failures += fail("both passes service each row; only high tiles settle", 0);
    for (i = 0; i < 15; ++i)
        if (Tiles[i].contour != 25)
            failures += fail("high-altitude plains branch includes fractal level", i);

    reset(0, 50, 50);
    world_erosion();
    for (i = 0; i < 15; ++i)
        if (Tiles[i].contour != 20)
            failures += fail("deep-water branch subtracts configured depth", i);

    reset(3, 50, 50);
    world_erosion();
    for (i = 0; i < 15; ++i)
        if (Tiles[i].contour != 37)
            failures += fail("altitude-three coast blend without a boundary", i);

    // The centre's radius-9 neighbor crosses a different land region. Native
    // 5C30DE adds four and clamps local_8 before the coast blend.
    reset(3, 50, 50);
    Tiles[7].region = 2;
    Tiles[0].climate = static_cast<uint8_t>(4 << 5);
    Tiles[0].region = 3;
    world_erosion();
    if (Tiles[7].contour != 34)
        failures += fail("altitude-three boundary clamp affects coast blend", 7);

    // Native 5C30B6 jumps directly to the common blend when adjusted terrain
    // remains below AltNatural[altitude+1]; no coast scaling is applied.
    reset(2, 40, 105); // intensity 20 from the deterministic two samples
    AltNatural[3] = 60;
    MapErosiveForces = 1;
    WorldBuilder.shelf = 10;
    WorldBuilder.plains = 16;
    world_erosion();
    if (Tiles[7].contour != 35)
        failures += fail("altitude-two low-threshold path bypasses coast scaling", 7);

    reset(4, 20, 50);
    world_erosion();
    for (i = 0; i < 15; ++i)
        if (Tiles[i].contour != 26)
            failures += fail("altitude-four branch uses hills and peaks parameters", i);

    reset(2, 50, 50);
    world_erosion();
    // Intensity 9 plus shelf 5 is below threshold 100: native settles
    // directly to (14 + 50 + 1) / 2, independent of neighbor counts.
    if (Tiles[7].contour != 32)
        failures += fail("altitude-two interior branch counts four-ring water", 7);
    if (Tiles[0].contour != 32)
        failures += fail("altitude-two edge neighbors retain bounded scan behavior", 0);
    if (Tiles[6].contour != 32)
        failures += fail("altitude-two lower-row edge retains bounded scan behavior", 6);

    if (failures != 0)
        return 1;
    puts("world_erosion actual-source host: pass");
    return 0;
}
'''


def compile_run(compiler, body, tmp_path):
    source = tmp_path / "world_erosion_actual_source.cpp"
    binary = tmp_path / "world_erosion_actual_source"
    source.write_text(FIXTURE_PREFIX + body + FIXTURE_SUFFIX)
    built = subprocess.run(
        [compiler, "-std=c++98", "-Wall", "-Wextra", "-Werror",
         str(source), "-o", str(binary)],
        text=True, capture_output=True, timeout=30,
    )
    assert built.returncode == 0, built.stdout + built.stderr
    return subprocess.run([str(binary)], text=True, capture_output=True,
                          timeout=30)


def source_body():
    return extract_function(SOURCE.read_text(), SIGNATURE)


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
def test_world_erosion_actual_source(compiler_name, tmp_path):
    compiler = shutil.which(compiler_name)
    if not compiler:
        pytest.skip(f"{compiler_name} is not installed")
    clean = compile_run(compiler, source_body(), tmp_path)
    assert clean.returncode == 0, clean.stdout + clean.stderr
    assert "world_erosion actual-source host: pass" in clean.stdout


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
@pytest.mark.parametrize(
    "damage,needle,replacement,witness",
    (
        ("remove_coast_clamp", "local_8 += 4;", "local_8 += 0;",
         "altitude-three boundary clamp affects coast blend"),
        ("reverse_altitude_two_threshold",
         "adjusted < static_cast<int>(AltNatural[altitude + 1])",
         "static_cast<int>(AltNatural[altitude + 1]) <= adjusted",
         "altitude-two low-threshold path bypasses coast scaling"),
    ),
)
def test_world_erosion_actual_source_damage_controls(
    compiler_name, damage, needle, replacement, witness, tmp_path
):
    compiler = shutil.which(compiler_name)
    if not compiler:
        pytest.skip(f"{compiler_name} is not installed")
    body = source_body()
    assert needle in body, f"{damage} anchor missing from recovered body"
    damaged_body = body.replace(needle, replacement)
    damaged = compile_run(compiler, damaged_body, tmp_path)
    assert damaged.returncode != 0
    assert witness in damaged.stderr
