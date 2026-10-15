"""Actual-source controls for world_climate's ordered map-window clear loop."""
from pathlib import Path
import shutil
import subprocess

import pytest


ROOT = Path(__file__).resolve().parents[2]
SIGNATURE = "void __cdecl world_climate() {"


def source_body() -> str:
    source = (ROOT / "src/map.cpp").read_text()
    start = source.index(SIGNATURE)
    opening = source.index("{", start)
    depth = 0
    for end in range(opening, len(source)):
        if source[end] == "{":
            depth += 1
        elif source[end] == "}":
            depth -= 1
            if depth == 0:
                return source[start : end + 1]
    raise AssertionError("unterminated production world_climate body")


HARNESS = r'''
#include <stdlib.h>
#include <vector>

#define __cdecl
static const int MapWinTableSlots = 8;
static const char WorldClimateStartLog[] = "start";
static const char WorldClimateCompleteLog[] = "complete";
unsigned int WorldClimateSkipTerritoryReset;
unsigned int WorldClimateSkipTerrainClear;
std::vector<int> Events;

enum {
    checksum_two = 10, start_log = 20, shorelines = 30, temperature = 40,
    rivers = 50, rainfall = 60, continents = 70, analysis = 80,
    territory_reset = 90, clock_read = 100, reseed = 110,
    overview_clear = 200, complete_log = 210, checksum_three = 220
};

void event(int value) { Events.push_back(value); }
void require(bool value) { if (!value) exit(1); }
void __cdecl world_climate();

struct MapWin {
    int id_;
    unsigned int field_1DD74_;
    void clear_terrain();
} Windows[8], Replacement, WorldClimateWorldWin;
MapWin *MapWinTable[MapWinTableSlots];
bool MutateLiveState;

void MapWin::clear_terrain() {
    event(id_ == 80 ? overview_clear : 1000 + id_);
    if (MutateLiveState && id_ == 0) {
        // Native code reads later slots and their activity fields after each
        // preceding callback; neither the receiver table nor flags are snapshotted.
        Windows[3].field_1DD74_ = 1;
        MapWinTable[7] = &Replacement;
    }
}

void do_checksums(int phase) {
    require(phase == 2 || phase == 3);
    event(phase == 2 ? checksum_two : checksum_three);
}
void log_say(const char *text, int a, int b, int c) {
    require(a == 0 && b == 0 && c == 0);
    require(text == WorldClimateStartLog || text == WorldClimateCompleteLog);
    event(text == WorldClimateStartLog ? start_log : complete_log);
}
void world_shorelines() { event(shorelines); }
void world_temperature() { event(temperature); }
void world_rivers() { event(rivers); }
void world_rainfall() { event(rainfall); }
struct PathFixture { void continents(); } Paths;
void PathFixture::continents() { event(::continents); }
void world_analysis() { event(analysis); }
void reset_territory() { event(territory_reset); }
unsigned int timeGetTime() { event(clock_read); return 0x1234; }
void random_reseed(unsigned int seed) { require(seed == 0x1234); event(reseed); }

void reset(unsigned int territory_skip, unsigned int terrain_skip) {
    Events.clear();
    WorldClimateSkipTerritoryReset = territory_skip;
    WorldClimateSkipTerrainClear = terrain_skip;
    MutateLiveState = false;
    for (int slot = 0; slot < 8; ++slot) {
        Windows[slot].id_ = slot;
        Windows[slot].field_1DD74_ = 0;
        MapWinTable[slot] = &Windows[slot];
    }
    Replacement.id_ = 70;
    Replacement.field_1DD74_ = 1;
    WorldClimateWorldWin.id_ = 80;
    WorldClimateWorldWin.field_1DD74_ = 1;
}

void append_prefix(std::vector<int> &expected, bool reset_territory) {
    expected.push_back(checksum_two);
    expected.push_back(start_log);
    expected.push_back(shorelines);
    expected.push_back(temperature);
    expected.push_back(rivers);
    expected.push_back(rainfall);
    expected.push_back(continents);
    expected.push_back(analysis);
    if (reset_territory) expected.push_back(territory_reset);
    expected.push_back(clock_read);
    expected.push_back(reseed);
}
void append_suffix(std::vector<int> &expected) {
    expected.push_back(complete_log);
    expected.push_back(checksum_three);
}

void check_all_eight_and_order() {
    reset(0, 0);
    for (int slot = 0; slot < 8; ++slot) Windows[slot].field_1DD74_ = 1;
    std::vector<int> expected;
    append_prefix(expected, true);
    for (int slot = 0; slot < 8; ++slot) expected.push_back(1000 + slot);
    expected.push_back(overview_clear);
    append_suffix(expected);
    world_climate();
    require(Events == expected);
}

void check_live_callback_reads() {
    reset(0, 0);
    Windows[0].field_1DD74_ = 1;
    Windows[5].field_1DD74_ = 1;
    MutateLiveState = true;
    std::vector<int> expected;
    append_prefix(expected, true);
    expected.push_back(1000);   // callback activates slot 3 and replaces slot 7
    expected.push_back(1003);
    expected.push_back(1005);
    expected.push_back(1070);   // reread the replacement receiver at slot 7
    expected.push_back(overview_clear);
    append_suffix(expected);
    world_climate();
    require(Events == expected);
}

void check_skip_flags() {
    reset(1, 0);
    Windows[7].field_1DD74_ = 1;
    std::vector<int> expected;
    append_prefix(expected, false);
    expected.push_back(1007);
    expected.push_back(overview_clear);
    append_suffix(expected);
    world_climate();
    require(Events == expected);

    reset(0, 2);
    std::vector<int> no_terrain;
    append_prefix(no_terrain, true);
    append_suffix(no_terrain);
    world_climate();
    require(Events == no_terrain);
}

int main() {
    check_all_eight_and_order();
    check_live_callback_reads();
    check_skip_flags();
    return 0;
}
'''


def compile_and_run(compiler: str, body: str, tmp_path: Path):
    source = tmp_path / "world_climate.cpp"
    executable = tmp_path / "world_climate"
    source.write_text(HARNESS + "\n" + body + "\n")
    built = subprocess.run(
        [compiler, "-std=c++98", "-Wall", "-Wextra", "-Werror", str(source), "-o", str(executable)],
        capture_output=True,
        text=True,
        timeout=30,
    )
    assert built.returncode == 0, built.stdout + built.stderr
    return subprocess.run([str(executable)], capture_output=True, text=True, timeout=10)


COMPILERS = [compiler for compiler in (shutil.which("g++"), shutil.which("clang++")) if compiler]


@pytest.mark.parametrize("compiler", COMPILERS or [None])
def test_actual_world_climate_order_eight_slots_live_reads_and_skip_flags(compiler, tmp_path):
    if compiler is None:
        pytest.skip("host C++ compiler unavailable")
    body = source_body()
    assert "for (int slot = 0; slot < 8; ++slot)" in body
    assert "MapWin *const window = MapWinTable[slot];" in body
    result = compile_and_run(compiler, body, tmp_path)
    assert result.returncode == 0, result.stdout + result.stderr


@pytest.mark.parametrize("compiler", COMPILERS or [None])
@pytest.mark.parametrize(
    "name,mutate",
    (
        ("miss_last_slot", lambda body: body.replace("slot < 8", "slot < 7", 1)),
        (
            "overview_before_map_slots",
            lambda body: body.replace(
                "        for (int slot = 0; slot < 8; ++slot) {",
                "        WorldClimateWorldWin.clear_terrain();\n        for (int slot = 0; slot < 8; ++slot) {",
                1,
            ).replace("        WorldClimateWorldWin.clear_terrain();\n    }", "    }", 1),
        ),
        (
            "snapshot_receivers_before_callbacks",
            lambda body: body.replace(
                "        for (int slot = 0; slot < 8; ++slot) {",
                "        MapWin *snapshot[8];\n        for (int saved = 0; saved < 8; ++saved) snapshot[saved] = MapWinTable[saved];\n        for (int slot = 0; slot < 8; ++slot) {",
                1,
            ).replace(
                "MapWin *const window = MapWinTable[slot];",
                "MapWin *const window = snapshot[slot];",
                1,
            ),
        ),
        (
            "invert_terrain_skip_gate",
            lambda body: body.replace(
                "WorldClimateSkipTerrainClear == 0",
                "WorldClimateSkipTerrainClear != 0",
                1,
            ),
        ),
    ),
)
def test_actual_world_climate_damage_controls_compile_then_fail(
    compiler, name, mutate, tmp_path
):
    if compiler is None:
        pytest.skip("host C++ compiler unavailable")
    body = source_body()
    damaged = mutate(body)
    assert damaged != body, f"{name} mutation anchor missing"
    result = compile_and_run(compiler, damaged, tmp_path)
    assert result.returncode != 0, name
