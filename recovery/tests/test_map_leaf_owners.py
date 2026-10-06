"""Host controls for proven map bodies, extracted from product source."""
from pathlib import Path
import shutil
import subprocess


ROOT = next(p for p in Path(__file__).resolve().parents if (p / 'src/IMAGE_PIN').is_file())


def definition(signature: str) -> str:
    hits = []
    for path in (ROOT / "src").rglob("*.cpp"):
        source = path.read_text(encoding="utf-8")
        start = source.find(signature)
        if start >= 0:
            hits.append((path, source, start))
    assert len(hits) == 1, f"expected one product definition for {signature}: {[x[0] for x in hits]}"
    _path, source, start = hits[0]
    opening = source.find("{", start)
    assert opening >= 0
    depth = 0
    for index in range(opening, len(source)):
        if source[index] == "{":
            depth += 1
        elif source[index] == "}":
            depth -= 1
            if depth == 0:
                body = source[start:index + 1]
                assert "BODY GOES HERE." not in body
                return body
    raise AssertionError(f"unterminated product definition: {signature}")


HARNESS = r"""
#include <assert.h>
#include <stdint.h>
#include <string.h>
#define __cdecl
#define FALSE 0
#define TRUE 1

struct Map {
    uint8_t climate, contour, val2, region, visibility, val3, unk_1;
    int8_t territory;
    uint32_t bit, bit2, bit_visible[7];
};
static Map tiles[32];
Map *map_tiles() { return tiles; }
uint32_t MapLongitude = 4;
uint32_t MapTileFlagClearMask = 0x00288050;
struct Player { uint32_t flags; } PlayersData[8];
struct Console { int faction_id_; } ConsoleGlobal;
struct GameFields { uint32_t game_state_; };
struct GameState { GameFields fields_; } GameDataStateStorage;

int bit_calls, altitude_calls, sync_calls, draw_calls;
int altitude_x, altitude_y, altitude_value, altitude_both;
int sync_x, sync_y, sync_faction, draw_x, draw_y, draw_mode;
void bit_set(int x, int y, int bit, int set) {
    ++bit_calls;
    uint32_t &tile_bits = tiles[MapLongitude * y + (x >> 1)].bit;
    if (set) tile_bits |= bit; else tile_bits &= ~static_cast<uint32_t>(bit);
}
void world_alt_set(int x, int y, int altitude, int both) {
    ++altitude_calls; altitude_x=x; altitude_y=y;
    altitude_value=altitude; altitude_both=both;
}
void synch_bit(int x, int y, int faction) {
    ++sync_calls; sync_x=x; sync_y=y; sync_faction=faction;
}
void draw_tile(int x, int y, int mode) {
    ++draw_calls; draw_x=x; draw_y=y; draw_mode=mode;
}

struct MapWin { uint32_t field_1DD9C_; };
MapWin *test_mapwins[8];
MapWin **MapWinTable = test_mapwins;
int MapLongitudeBounds;

BODY_4CE6B0
BODY_4468F0

static void reset_observers() {
    bit_calls=altitude_calls=sync_calls=draw_calls=0;
    altitude_x=altitude_y=altitude_value=altitude_both=-1;
    sync_x=sync_y=sync_faction=draw_x=draw_y=draw_mode=-1;
}

int main() {
    memset(tiles, 0, sizeof(tiles));
    memset(PlayersData, 0, sizeof(PlayersData));
    memset(&GameDataStateStorage, 0, sizeof(GameDataStateStorage));
    ConsoleGlobal.faction_id_=3;

    reset_observers();
    Map &tile=tiles[MapLongitude * 2 + 1];
    tile.climate=0x20;
    tile.visibility=0;
    sub_4ce6b0(2,2);
    assert(bit_calls==3);
    assert((tile.bit & 0x20)!=0);
    assert((tile.bit & 0x28805C)==0);
    assert((tile.bit & MapTileFlagClearMask)==0);
    assert(altitude_calls==1 && altitude_x==2 && altitude_y==2);
    assert(altitude_value==2 && altitude_both==0);
    assert(sync_calls==0 && draw_calls==0);

    reset_observers();
    tile.climate=0x40;
    tile.visibility=1 << ConsoleGlobal.faction_id_;
    sub_4ce6b0(2,2);
    assert(altitude_calls==0);
    assert(sync_calls==1 && draw_calls==1);
    assert(sync_x==2 && sync_y==2 && sync_faction==3);
    assert(draw_x==2 && draw_y==2 && draw_mode==2);

    reset_observers();
    tile.visibility=0;
    PlayersData[3].flags=0x200;
    sub_4ce6b0(2,2);
    assert(sync_calls==1 && draw_calls==1);

    MapWin primary;
    primary.field_1DD9C_=40;
    test_mapwins[0]=&primary;
    MapLongitudeBounds=100;
    assert(sub_4468f0(42)==1); // strict lower edge is coordinate 42
    assert(sub_4468f0(40)==-1);
    assert(sub_4468f0(88)==1);
    assert(sub_4468f0(90)==-1); // half-coordinate equal to edge is excluded

    // Preserve the wrapped, asymmetric second branch at both strict edges.
    primary.field_1DD9C_=60;
    assert(sub_4468f0(10)==1); // edge == half-coordinate 5 is accepted
    assert(sub_4468f0(12)==-1);
    assert(sub_4468f0(60)==1); // half-coordinate == half-base is accepted
    return 0;
}
"""


def compile_and_run(tmp_path):
    compiler = shutil.which("g++")
    if compiler is None:
        import pytest
        pytest.skip("g++ is unavailable")
    clear = definition("extern \"C\" void __cdecl sub_4ce6b0(")
    bounds = definition("extern \"C\" int __cdecl sub_4468f0(")
    assert "0x007D3C3C" not in bounds and "0x00949870" not in bounds
    program = HARNESS.replace("BODY_4CE6B0", clear).replace("BODY_4468F0", bounds)
    source_path = tmp_path / "actual_source.cpp"
    binary_path = tmp_path / "actual_source"
    source_path.write_text(program, encoding="utf-8")
    built = subprocess.run([compiler, "-std=c++98", "-Wall", "-Wextra", "-Werror",
                            str(source_path), "-o", str(binary_path)],
                           capture_output=True, text=True)
    assert built.returncode == 0, built.stderr
    return subprocess.run([str(binary_path)], capture_output=True, timeout=10)


def test_proven_map_bodies_from_product_source(tmp_path):
    result = compile_and_run(tmp_path)
    assert result.returncode == 0, result.stderr.decode(errors="replace")
