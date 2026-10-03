"""Host behavior/damage controls for the actual map.cpp rainfall owner."""
from pathlib import Path
import shutil
import subprocess
import tempfile


ROOT = Path(__file__).resolve().parents[2]
SOURCE = ROOT / "src/map.cpp"
PASS2_SIGNATURE = "static void world_rainfall_pass2(int saved_faction)"
XRANGE_SIGNATURE = "static int world_rainfall_xrange(int x, int width, int flat)"
OWNER_SIGNATURE = "void __cdecl world_rainfall()"
MARKER = "/* INSERT_ACTUAL_RAINFALL_OWNER */"

HOST = r'''#include <stdio.h>
#include <stdint.h>
#include <string.h>
struct Map {
    uint8_t climate, contour, val2, region, visibility, val3, unk_1;
    int8_t territory;
    uint32_t bit, bit2, bit_visible[7];
};
struct RulesWorldbuilder {
    uint32_t padding[15], cloudmass_peaks, cloudmass_hills, rainfall_coeff;
};
struct Base {
    uint8_t coordinate_bytes[4], faction_id_current, remaining_bytes[0x12F];
};
struct ConsoleGlobalFixture { int faction_id_; };
enum { BIT_RIVER = 0x80, BIT_FOREST = 0x200000, BIT_CONDENSER = 0x400000 };
const int RadiusOffsetX[9] = {0,1,2,1,0,-1,-2,-1,0};
const int RadiusOffsetY[9] = {0,-1,0,1,2,1,0,-1,0};
const int RadiusBaseX[9] = {1,2,1,0,-1,-2,-1,0,0};
const int RadiusBaseY[9] = {-1,0,1,2,1,0,-1,-2,0};
#define __cdecl
extern int TerraformTileFaction;
extern int TerraformDrownedBaseId;
extern ConsoleGlobalFixture ConsoleGlobal;
extern uint32_t MapArea;
extern int MapIsFlat;
extern Map *MapTiles;
extern uint32_t MapLongitude;
extern int MapLongitudeBounds;
extern int MapLatitudeBounds;
extern uint32_t MapCloudCover;
extern int TurnCurrentNum;
extern int BaseFindDist;
extern uint32_t LockEnableMask;
extern RulesWorldbuilder WorldBuilder;
extern Base Bases[];
int base_find(int, int);
int base_find(int, int, int);
void do_all_non_input();
void climate_set(int, int, int);
/* INSERT_ACTUAL_RAINFALL_OWNER */

static Map Tiles[15];
static int progress_calls;
static int climate_calls;
static int callbacks[8][4];
static int callback_count;
static int find2_result;
static int find3_result;
Base Bases[512];
Map *MapTiles = Tiles;
uint32_t MapLongitude = 3;
int MapLongitudeBounds = 6;
int MapLatitudeBounds = 5;
uint32_t MapArea = 15;
int MapIsFlat = 1;
uint32_t MapCloudCover = 0;
int TurnCurrentNum = 0;
int BaseFindDist = 1;
uint32_t LockEnableMask = 0;
int TerraformTileFaction = -7;
int TerraformDrownedBaseId = -8;
RulesWorldbuilder WorldBuilder = {{0}, 5, 3, 1};
ConsoleGlobalFixture ConsoleGlobal = {2};
uint32_t UnkBitfield1 = 0;

static void record_callback(int arity, int x, int y, int faction) {
    if (callback_count < 8) {
        callbacks[callback_count][0] = arity;
        callbacks[callback_count][1] = x;
        callbacks[callback_count][2] = y;
        callbacks[callback_count][3] = faction;
    }
    ++callback_count;
}
int base_find(int x, int y) {
    record_callback(2, x, y, -1);
    return find2_result;
}
int base_find(int x, int y, int faction) {
    record_callback(3, x, y, faction);
    return find3_result;
}
void do_all_non_input() { ++progress_calls; }
void climate_set(int x, int y, int rainfall) {
    Map *tile = &MapTiles[MapLongitude * y + (x >> 1)];
    tile->climate &= 0xE7;
    tile->climate |= (rainfall & 3) << 3;
    tile->bit2 |= 0x400000;
    UnkBitfield1 |= 1;
    ++climate_calls;
}

static void reset_world() {
    memset(Tiles, 0, sizeof(Tiles));
    memset(Bases, 0, sizeof(Bases));
    memset(callbacks, 0, sizeof(callbacks));
    int i;
    for (i = 0; i < 15; ++i) {
        Tiles[i].climate = 0x60;
        Tiles[i].unk_1 = 0xFF;
    }
    MapCloudCover = 0;
    MapIsFlat = 1;
    WorldBuilder.cloudmass_peaks = 5;
    WorldBuilder.cloudmass_hills = 3;
    WorldBuilder.rainfall_coeff = 1;
    TurnCurrentNum = 0;
    BaseFindDist = 1;
    LockEnableMask = 0;
    TerraformTileFaction = -7;
    TerraformDrownedBaseId = -8;
    ConsoleGlobal.faction_id_ = 2;
    progress_calls = 0;
    climate_calls = 0;
    callback_count = 0;
    find2_result = -1;
    find3_result = -1;
    UnkBitfield1 = 0;
}
static int fail(const char *message) {
    fprintf(stderr, "FAIL: %s\n", message);
    return 1;
}

int main() {
    int failures = 0;
    int i;
    const int expected_rain[3] = {1, 2, 2};
    for (i = 0; i < 3; ++i) {
        reset_world();
        memset(Tiles, 0, sizeof(Tiles));
        MapCloudCover = static_cast<uint32_t>(i);
        Tiles[7].climate = 2;
        Tiles[7].unk_1 = 0x80;
        world_rainfall_pass2(2);
        if (((Tiles[7].climate >> 3) & 3) != expected_rain[i])
            failures += fail("scratch-0x80 adjustment uses native cloud-cover polarity");
    }
    reset_world();
    world_rainfall();
    if (TerraformTileFaction != -1 || TerraformDrownedBaseId != -1)
        failures += fail("entry resets both shared Terraform results");
    if (progress_calls != 3 * MapLatitudeBounds)
        failures += fail("owner services input once for every row in all three passes");
    for (i = 0; i < 15; ++i)
        if (Tiles[i].unk_1 != 0)
            failures += fail("entry clears scratch and ordinary land writes no hints");
    if (callback_count != 0 || climate_calls != 0)
        failures += fail("quiet land fixture has no base or climate callbacks");

    reset_world();
    Tiles[7].bit = BIT_FOREST;
    world_rainfall();
    if ((Tiles[4].unk_1 & 0x40) == 0 || (Tiles[10].unk_1 & 0x40) == 0
        || (Tiles[8].unk_1 & 0x40) == 0)
        failures += fail("altitude-three forest emits all native pass-1 hints");

    reset_world();
    Tiles[7].climate = 0xC0;
    Tiles[6].climate = 0x60;
    Tiles[8].climate = 0x60;
    world_rainfall();
    if ((Tiles[6].unk_1 & 0x20) == 0 || (Tiles[8].unk_1 & 0x10) == 0)
        failures += fail("high mountain emits lower west/east cloudmass hints");

    reset_world();
    Tiles[7].climate = 0xC0;
    Tiles[6].climate = 0xC0; // equal-height west neighbor ends that scan
    Tiles[8].climate = 0x60; // east scan still runs in the native control flow
    world_rainfall();
    if ((Tiles[8].unk_1 & 0x10) == 0)
        failures += fail("west stop still proceeds to east cloudmass scan");

    reset_world();
    Tiles[7].climate = 0x70; // old class 2: row 2, logical x 2
    Tiles[8].climate = 0x70; // old class 2: row 2, logical x 4
    TurnCurrentNum = 1;
    BaseFindDist = 3; // ensure found base is not retained as drowned
    LockEnableMask = 1u << 3;
    Bases[1].faction_id_current = 3;
    find2_result = 1;
    world_rainfall();
    if (callback_count != 3)
        failures += fail("first drying tile caches faction and suppresses second optional lookup");
    if (callback_count >= 3
        && (callbacks[0][0] != 2 || callbacks[0][1] != 2 || callbacks[0][2] != 2
            || callbacks[1][0] != 3 || callbacks[1][1] != 2 || callbacks[1][2] != 2
            || callbacks[1][3] != 2 || callbacks[2][0] != 3
            || callbacks[2][1] != 4 || callbacks[2][2] != 2
            || callbacks[2][3] != 2))
        {
            fprintf(stderr, "callbacks: n=%d", callback_count);
            for (i = 0; i < callback_count && i < 8; ++i)
                fprintf(stderr, " [%d:%d,%d,%d,%d]", i, callbacks[i][0],
                        callbacks[i][1], callbacks[i][2], callbacks[i][3]);
            fputc('\n', stderr);
            failures += fail("base_find calls retain exact overload, tile, and saved faction order");
        }
    if (TerraformTileFaction != 3 || TerraformDrownedBaseId != -1)
        failures += fail("faction cache survives while distant drowned base is not selected");

    reset_world();
    Tiles[8].bit2 = 0x4; // native pass 2 raises this neighbor to class 2
    MapIsFlat = 0;
    world_rainfall();
    if (((Tiles[6].climate >> 3) & 3) != 1) {
        fprintf(stderr, "wrap target class=%d source class=%d scratch=%02x/%02x/%02x\n",
                (Tiles[6].climate >> 3) & 3, (Tiles[7].climate >> 3) & 3,
                Tiles[6].unk_1, Tiles[7].unk_1, Tiles[8].unk_1);
        failures += fail("wrapped radius neighbor receives moderate-rain propagation");
    }
    if ((Tiles[6].bit2 & 0x400000) == 0 || (UnkBitfield1 & 1) == 0
        || climate_calls < 1) {
        fprintf(stderr, "wrap effects calls=%d global=%u target-bit2=%08x\n",
                climate_calls, UnkBitfield1, Tiles[6].bit2);
        failures += fail("climate_set preserves tile and global side effects");
    }

    reset_world();
    Tiles[8].bit2 = 0x4;
    MapIsFlat = 1;
    world_rainfall();
    if (((Tiles[6].climate >> 3) & 3) != 0
        || (Tiles[6].bit2 & 0x400000) != 0) {
        fprintf(stderr, "flat effects calls=%d target-class=%d target-bit2=%08x\n",
                climate_calls, (Tiles[6].climate >> 3) & 3, Tiles[6].bit2);
        failures += fail("flat edge does not wrap to the same radius neighbor");
    }

    if (failures != 0)
        return 1;
    puts("actual map.cpp world_rainfall owner host: pass");
    return 0;
}
'''


def extract(source, signature):
    assert source.count(signature) == 1, f"expected one source owner for {signature}"
    start = source.index(signature)
    brace = source.index("{", start)
    depth = 0
    for end in range(brace, len(source)):
        if source[end] == "{":
            depth += 1
        elif source[end] == "}":
            depth -= 1
            if depth == 0:
                return source[start:end + 1]
    raise AssertionError(f"unterminated body: {signature}")


def run(compiler, *, damage=None):
    source = SOURCE.read_text()
    fragments = [extract(source, signature) for signature in
                 (XRANGE_SIGNATURE, PASS2_SIGNATURE, OWNER_SIGNATURE)]
    proposal = HOST.replace(MARKER, "\n\n".join(fragments))
    if damage == "cloud_polarity":
        condition = "if (cloud_cover >= 1)"
        assert proposal.count(condition) == 1
        proposal = proposal.replace(condition, "if (cloud_cover < 1)")
    if damage == "pass1_nesting":
        marker = "            const int altitude = tile->climate >> 5;"
        assert proposal.count(marker) == 1
        start = proposal.index(marker)
        proposal = proposal[:start] + "            if ((tile->climate & 0xE0) < 0x60) {\n" + proposal[start:]
        row_close = proposal.index("\n        }\n        do_all_non_input();", start)
        proposal = proposal[:row_close] + "\n            }" + proposal[row_close:]
    with tempfile.TemporaryDirectory(prefix="rainfall-actual-owner-") as temp:
        root = Path(temp)
        cpp = root / "actual.cpp"
        binary = root / "actual"
        cpp.write_text(proposal)
        built = subprocess.run(
            [compiler, "-std=c++98", "-Wall", "-Wextra", "-Werror",
             str(cpp), "-o", str(binary)], capture_output=True, text=True,
            timeout=30,
        )
        assert built.returncode == 0, built.stdout + built.stderr
        return subprocess.run([str(binary)], capture_output=True, text=True,
                              timeout=30)


def test_admitted_map_source_owner_against_behavior_and_damage_controls():
    compilers = [shutil.which(c) for c in ("g++", "clang++")]
    compilers = [c for c in compilers if c]
    assert compilers, "GCC or Clang is required for this host-only control"
    for compiler in compilers:
        clean = run(compiler)
        assert clean.returncode == 0, clean.stdout + clean.stderr
        assert "actual map.cpp world_rainfall owner host: pass" in clean.stdout
        damaged = run(compiler, damage="pass1_nesting")
        assert damaged.returncode != 0
        assert "altitude-three forest" in damaged.stderr
        assert "high mountain" in damaged.stderr
        damaged = run(compiler, damage="cloud_polarity")
        assert damaged.returncode != 0
        assert "native cloud-cover polarity" in damaged.stderr
