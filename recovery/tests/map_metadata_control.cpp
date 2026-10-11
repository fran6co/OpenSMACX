#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define __cdecl
typedef int BOOL;
struct Map { uint32_t bit; };
static int draws[7], draw_count, draw_cursor, wipes, invalid;
static int expected_rules[7];
static int controlled_rand() {
    if (draw_cursor >= draw_count) { invalid = 1; return 0; }
    return draws[draw_cursor++];
}
#define rand controlled_rand

static void map_wipe();
static void brush(int, int, int);
static int xrange(int x) { return x; }
static bool on_map(int, int) { return true; }
static int VehMoveDeltaX[8] = {0, 1, 2, 3, 4, 5, 6, 7};
static int VehMoveDeltaY[8];
static int paint_x[3], paint_calls;
static void draw_map(int) {}
static void build_continent(int) {}
static void build_hills(int) {}
static void world_erosion() {}
static void world_polar_caps() {}
static void world_shorelines() {}
static int world_validate() { return 0; }
static void world_temperature();
static void world_riverbeds();
static Map *map_tiles();
static int river_trials;
static bool is_ocean(int, int) { ++river_trials; return true; }
static uint32_t bit_at(int, int) { return 0; }
static uint32_t alt_at(int, int) { return 4; }
static void bit_set(int, int, int, bool) {}
static const uint32_t BIT_RIVERBED = 0x100, BIT2_UNK_80000000 = 0x80000000, BIT2_CRATER = 1;
static const int BIT_THERMAL_BORE = 0x40000000, ALT_1_LEVEL_ABOVE_SEA = 4, ALT_2_LEVELS_ABOVE_SEA = 5;
static const int RadiusRange[5] = {1,9,25,49,81};
static int RadiusOffsetX[81], RadiusOffsetY[81];
static void do_all_non_input() {}
static uint32_t bit2_at(int, int) { return 0; }
static int code_at(int, int) { return 0; }
static int range(int v, int low, int high) { return v < low ? low : v > high ? high : v; }
static void random_reseed(uint32_t) {}
static uint32_t random(int, int);
static void temp_set(int, int, int);
static int thermal_calls, thermal_bound, thermal_value, thermal_tiles;
static void world_fungus() {}
static void world_monsoon(int, int) {}
static void world_crater(int, int) {}
static void world_volcano(int, int, int) {}
static void world_mesa(int, int) {}
static void world_ridge(int, int) {}
static void world_diamond(int, int) {}
static void world_ruin(int, int) {}
static void world_unity(int, int) {}
static void world_fossil(int, int) {}
static void world_temple(int, int) {}
static void world_borehole(int, int) {}
static void world_sargasso(int, int) {}
static void world_dune(int, int) {}
static void world_fresh(int, int) {}
static void world_geothermal(int, int) {}
static void fixup_landmarks() {}
static void world_linearize_contours() {}
static void world_climate() {}
static void world_rocky() {}
static int AltNatural[11], AltNaturalDefault[11];
static int WorldClimateRedrawRequest, WorldClimateSkipTerritoryReset;
static int WorldClimateSkipTerrainClear = 1;
static int ExpansionEnabled;
static uint32_t BrushVal1, BrushVal2, WorldBuildVal1;
static struct {
    uint32_t land_mod, land_base, islands, hills_mod, hills_base;
    uint32_t rivers_base, rivers_rain_mod, solar_energy, thermal_band, thermal_deviance, global_warming;
} WorldBuilder;
static struct { struct { uint32_t game_state_; } fields_; } GameDataStateStorage;
struct MapWinMock { int field_1DD74_; void clear_terrain() {} };
typedef MapWinMock *MapWinSlots[8];
static MapWinSlots MapWinTable;
static const unsigned MapWinTableSlots = 8;
static MapWinMock WorldClimateWorldWin;

// @@ACTUAL_MAP_OWNER@@

MapStorageWindow MapStorage;
static Map *map_tiles() { return MapTiles; }
static uint32_t random(int, int bound) {
    if (++thermal_calls % 2 == 0) thermal_bound = bound;
    return thermal_calls % 2 == 0 ? 1u : 0u;
}
static void temp_set(int, int, int value) { thermal_value = value; ++thermal_tiles; }
static void map_wipe() {
    ++wipes;
    if (memcmp(MapStorage.metadata_.planet_rules_, expected_rules, sizeof(expected_rules)))
        invalid = 1;
    for (int i = 0; i < 11; ++i)
        if (AltNatural[i] != AltNaturalDefault[i]) invalid = 1;
    if (WorldClimateRedrawRequest != 1 || WorldClimateSkipTerritoryReset != 1)
        invalid = 1;
}

static void brush(int x, int, int) {
    if (paint_calls < 3) paint_x[paint_calls] = x;
    if (++paint_calls == 3) BrushVal2 = 3;
}

int main() {
    // Four negative sentinels must draw exactly once, in native ascending order.
    const int initial[7] = {-1, 2, -2, 0, -3, 2, -4};
    const int expected[7] = {2, 2, 2, 0, 1, 2, 2};
    memcpy(MapStorage.metadata_.planet_rules_, initial, sizeof(initial));
    memcpy(expected_rules, expected, sizeof(expected));
    draws[0] = 8; draws[1] = 5; draws[2] = 4; draws[3] = 11;
    draw_count = 4;
    WorldBuilder.solar_energy = WorldBuilder.thermal_band = WorldBuilder.thermal_deviance = WorldBuilder.global_warming = 1;
    for (int i = 0; i < 11; ++i) AltNaturalDefault[i] = i * 20;
    world_build();
    if (invalid || wipes != 1 || draw_cursor != 4 || WorldClimateRedrawRequest ||
        WorldClimateSkipTerritoryReset) return 1;

    // Zero and positive words retain their bytes and consume no random draws.
    const int nonnegative[7] = {0, 2, 1, 2, 0, 1, 3};
    memcpy(MapStorage.metadata_.planet_rules_, nonnegative, sizeof(nonnegative));
    memcpy(expected_rules, nonnegative, sizeof(nonnegative));
    draw_cursor = draw_count = 0;
    world_build();
    if (invalid || wipes != 2 || draw_cursor != 0 ||
        memcmp(MapStorage.metadata_.planet_rules_, nonnegative, sizeof(nonnegative))) return 2;
    if (sizeof(MapMetadataFields) != 0xA9C || sizeof(MapStorage.metadata_.planet_rules_) != 28)
        return 3;

    // 50000 squared has its sign bit set in the native low dword. A second
    // identical direction therefore passes the signed threshold and becomes0.
    MapLandCoverage = 50000;
    draws[0] = draws[1] = draws[2] = 0;
    draw_cursor = 0; draw_count = 3;
    paint_land(0, 0, 1, 3);
    if (invalid || paint_calls != 3 || paint_x[0] != 0 || paint_x[1] != 1 || paint_x[2] != 1)
        return 4;
    MapLandCoverage = 2;
    paint_calls = draw_cursor = 0;
    paint_land(0, 0, 1, 3);
    if (invalid || paint_calls != 3 || paint_x[2] != 2) return 5;

    // A negative signed river target must finish before trying any location.
    Map tile = {BIT_RIVERBED};
    MapTiles = &tile;
    MapArea = 1; MapOceanCoverage = 5; MapCloudCover = 0;
    WorldBuilder.rivers_base = 9600; WorldBuilder.rivers_rain_mod = 0;
    world_riverbeds();
    if (river_trials != 0 || tile.bit != 0) return 6;
    // Positive target1: no eligible tiles, so the original4000-trial limit wins.
    MapOceanCoverage = 1;
    world_riverbeds();
    if (river_trials != 4000) return 7;
    // Low-dword outer multiplication becomes negative; signed /3200 then
    // signed count comparison must skip the trial loop again.
    MapArea = 0xFFFFFFFFu; river_trials = 0;
    world_riverbeds();
    if (river_trials != 0) return 8;

    MapLatitudeBounds = MapLongitudeBounds = 1;
    MapPlanetaryOrbit = 0x7FFFFFFF;
    thermal_calls = thermal_tiles = 0;
    world_temperature();
    if (thermal_calls != 2 || thermal_bound != (-2147483647 - 1) ||
        thermal_tiles != 1 || thermal_value != 1) return 9;
    return 0;
}
