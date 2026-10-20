"""Exercise the complete production world_fungus body at its pinned climate gate.

The 0x5C35B2-0x5C35CA path checks high val3 first, then conditionally reads
climate; only their conjunction skips the tile. 0x5C3622-0x5C362B reloads
climate after the 1-in-40 random call before optional veh_init. These controls
test source behavior and call/write ordering, not game runtime or ABI proof.
"""
from pathlib import Path
import shutil
import subprocess

import pytest


WORKER = Path(__file__).resolve().parents[2]
SOURCE = WORKER / "src/world.cpp"
FUNCTION = "void __cdecl world_fungus() {"


def source_body() -> str:
    source = SOURCE.read_text(encoding="utf-8")
    start = source.index(FUNCTION)
    opening = source.index("{", start)
    depth = 0
    for position in range(opening, len(source)):
        if source[position] == "{":
            depth += 1
        elif source[position] == "}":
            depth -= 1
            if depth == 0:
                return source[start:position + 1]
    raise AssertionError("unterminated world_fungus body")


HARNESS = r"""
#include <assert.h>
#include <stdint.h>
#define __cdecl

enum Event { E_INIT=1, E_SAMPLE, E_MAP_TILES, E_RAND, E_SET_BIT,
             E_CLEAR_BIT, E_VEH_INIT, E_ROW_CALLBACK };
static int events[32];
static int event_count;
static int sample_count;
static int rand_count;
static int map_tiles_count;
static int callback_count;
static int vehicle_count;
static int vehicle_args[4];
static int mutation_after_seed_rand;
static uint32_t MapTileFlagClearMask;
static int MapLatitudeBounds;
static int MapLongitudeBounds;
static int MapLongitude;
static uint32_t MapNativeLifeForms;
static int ExpansionEnabled;
struct Builder { uint32_t fungus; } WorldBuilder;

static void record(int event) {
    assert(event_count < static_cast<int>(sizeof(events) / sizeof(events[0])));
    events[event_count++] = event;
}

struct Map {
    uint32_t bit2;
    uint32_t val3;
    uint32_t climate;
    struct BitWord {
        uint32_t value;
        BitWord &operator|=(uint32_t bits) {
            record(E_SET_BIT);
            value |= bits;
            return *this;
        }
        BitWord &operator&=(uint32_t bits) {
            record(E_CLEAR_BIT);
            value &= bits;
            return *this;
        }
    } bit;
};
static Map tiles[1];

struct Fractal {
    void init() { record(E_INIT); }
    int sample(int x, int y) {
        assert(x == 0 && y == 0);
        ++sample_count;
        record(E_SAMPLE);
        return 0;
    }
};

static Map *map_tiles() {
    ++map_tiles_count;
    record(E_MAP_TILES);
    return tiles;
}
static void do_all_non_input() {
    ++callback_count;
    record(E_ROW_CALLBACK);
}
static int veh_init(int type, int faction, int x, int y) {
    ++vehicle_count;
    vehicle_args[0] = type;
    vehicle_args[1] = faction;
    vehicle_args[2] = x;
    vehicle_args[3] = y;
    record(E_VEH_INIT);
    return 0;
}
static int mock_rand() {
    ++rand_count;
    record(E_RAND);
    if (rand_count == 2 && mutation_after_seed_rand) {
        tiles[0].climate = 0x60;
    }
    return rand_count == 1 ? 1 : 0;
}
#define rand mock_rand

/* PRODUCT_BODY */

static void reset(uint32_t val3, uint32_t climate, int mutate_after_rand) {
    event_count = sample_count = rand_count = map_tiles_count = 0;
    callback_count = vehicle_count = 0;
    vehicle_args[0] = vehicle_args[1] = vehicle_args[2] = vehicle_args[3] = -1;
    mutation_after_seed_rand = mutate_after_rand;
    MapTileFlagClearMask = 0x04;
    MapLatitudeBounds = 1;
    MapLongitudeBounds = 1;
    MapLongitude = 1;
    MapNativeLifeForms = 1;
    ExpansionEnabled = 1;
    WorldBuilder.fungus = 17;
    tiles[0].bit2 = 0;
    tiles[0].val3 = val3;
    tiles[0].climate = climate;
    tiles[0].bit.value = 0;
}

static void expect_events(const int *expected, int count) {
    assert(event_count == count);
    for (int i = 0; i < count; ++i) assert(events[i] == expected[i]);
}

static void check_case(uint32_t val3, uint32_t climate, int mutate,
                       int expected_vehicle, int expected_writes,
                       uint32_t expected_climate) {
    reset(val3, climate, mutate);
    world_fungus();
    assert(sample_count == 2 && map_tiles_count == 1 && callback_count == 1);
    assert(vehicle_count == expected_vehicle && rand_count == (expected_writes ? 2 : 1));
    assert(tiles[0].climate == expected_climate);
    assert(tiles[0].bit.value == (expected_writes ? 0x20u : 0u));
    if (expected_vehicle) {
        assert(vehicle_args[0] == 0x13 && vehicle_args[1] == 0);
        assert(vehicle_args[2] == 0 && vehicle_args[3] == 0);
    }

    int expected[10] = {E_INIT, E_SAMPLE, E_SAMPLE, E_MAP_TILES,
                        E_RAND};
    int count = 5;
    if (expected_writes) {
        expected[count++] = E_SET_BIT;
        expected[count++] = E_CLEAR_BIT;
        expected[count++] = E_RAND;
        if (expected_vehicle) expected[count++] = E_VEH_INIT;
    }
    expected[count++] = E_ROW_CALLBACK;
    expect_events(expected, count);
}

int main() {
    // Cross all four val3/climate combinations. The tile is skipped only when
    // BOTH values are high; high climate alone still reaches the seed test.
    check_case(0x00, 0x40, 0, 0, 1, 0x40);
    check_case(0x00, 0x60, 0, 1, 1, 0x60);
    check_case(0x80, 0x40, 0, 0, 1, 0x40);
    check_case(0x80, 0x60, 0, 0, 0, 0x60);

    // The qualifying branch reads climate again after rand() and sees the
    // live value changed by that call; it must seed and pass exact arguments.
    check_case(0x80, 0x40, 1, 1, 1, 0x60);
}
"""


def compile_and_run(tmp_path: Path, compiler: str, body: str, label: str):
    compiler_path = shutil.which(compiler)
    if compiler_path is None:
        pytest.skip(f"{compiler} is unavailable")
    program = tmp_path / f"world_fungus_{label}.cpp"
    executable = tmp_path / f"world_fungus_{label}"
    program.write_text(HARNESS.replace("/* PRODUCT_BODY */", body), encoding="utf-8")
    compiled = subprocess.run(
        [compiler_path, "-std=c++98", "-Wall", "-Wextra", "-Werror", "-pedantic",
         str(program), "-o", str(executable)], text=True, capture_output=True)
    assert compiled.returncode == 0, compiled.stdout + compiled.stderr
    return subprocess.run([str(executable)], text=True, capture_output=True)


@pytest.mark.parametrize("compiler", ["g++", "clang++"])
def test_complete_world_fungus_gate_and_live_climate_reload(tmp_path, compiler):
    body = source_body()
    accepted = compile_and_run(tmp_path, compiler, body, "candidate")
    assert accepted.returncode == 0, accepted.stdout + accepted.stderr

    old_gate = """if ((tile->val3 & ~0x1F) >= 0x80
                    && (tile->climate & 0xE0) >= 0x60) {
                    continue;
                }"""
    old_split_gate = """if ((tile->val3 & ~0x1F) >= 0x80) {
                    continue;
                }
                if ((tile->climate & 0xE0) >= 0x60) {
                    continue;
                }"""
    assert old_gate in body
    mutant = body.replace(old_gate, old_split_gate, 1)
    rejected = compile_and_run(tmp_path, compiler, mutant, "old_split_mutant")
    assert rejected.returncode != 0, "old independent-continue gate survived controls"
