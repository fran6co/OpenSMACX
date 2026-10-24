"""Actual-source coast and zone-of-control behavioral checks.

Typed helper doubles qualify bounded effects, not native ABI or game acceptance.
"""
from __future__ import annotations

from pathlib import Path
import shutil
import subprocess

import pytest
from match_recovery.source import read_file


ROOT = next(parent for parent in Path(__file__).resolve().parents
            if (parent / 'src/map.cpp').is_file())
MAP_CPP = ROOT / 'src/map.cpp'
MAP_H = ROOT / 'src/map.h'
ADDRESSES = {
    'base': (0x0050DF30, 'int __cdecl base_coast(int base_id) {'),
    'port': (0x0050E030, 'BOOL __cdecl port_to_coast(int base_id, int region) {'),
    'zoc': (0x005C8AC0, 'int __cdecl zoc_veh(int x, int y, int faction_id) {'),
}
VEH_WHO_SIGNATURE = 'MEASURED inline int __cdecl veh_who(int x, int y) {'


def _definition(text: str, signature: str) -> str:
    start = text.find(signature)
    assert start >= 0, f'missing selected signature: {signature}'
    opening = text.find('{', start)
    depth = 0
    state = 'code'
    i = opening
    while i < len(text):
        ch = text[i]
        following = text[i + 1] if i + 1 < len(text) else ''
        if state == 'code':
            if ch == '/' and following == '/':
                state = 'line'; i += 2; continue
            if ch == '/' and following == '*':
                state = 'block'; i += 2; continue
            if ch == '"': state = 'string'
            elif ch == "'": state = 'char'
            elif ch == '{': depth += 1
            elif ch == '}':
                depth -= 1
                if depth == 0:
                    return text[start:i + 1]
        elif state == 'line':
            if ch == '\n': state = 'code'
        elif state == 'block':
            if ch == '*' and following == '/':
                state = 'code'; i += 2; continue
        else:
            if ch == '\\': i += 2; continue
            if state == 'string' and ch == '"': state = 'code'
            elif state == 'char' and ch == "'": state = 'code'
        i += 1
    raise AssertionError(f'unclosed definition: {signature}')


def _actual_bodies() -> dict[str, str]:
    rows = {row.address: row.region for row in read_file(MAP_CPP)}
    out = {}
    for name, (address, signature) in ADDRESSES.items():
        assert address in rows
        out[name] = _definition(rows[address], signature)
    # veh_who is an actual typed inline source body used by zoc_veh.  Keep it
    # live too: its low-nibble owner and <8 guard establish the accumulator's
    # reachable value range.  map_loc remains a typed fixture boundary.
    out['veh_who'] = _definition(MAP_H.read_text(), VEH_WHO_SIGNATURE)
    return out


CPP = r'''#include <algorithm>
#include <cassert>
#include <cstdint>
#include <cstring>
#include <string>
#include <vector>

#define __cdecl
#define MEASURED
using uint8_t = std::uint8_t;
using int8_t = std::int8_t;
using uint32_t = std::uint32_t;
using BOOL = int;
enum { RegionBounds = 63, BIT_VEH_IN_TILE = 2, DTREATY_PACT = 1 };
static const int TileWidth = 32, TileHeight = 16;
struct Map {
    uint8_t climate, contour, val2, region, visibility, val3, unk_1;
    int8_t territory;
    uint32_t bit, bit2, bit_visible[7];
};
struct Base { int x, y; };
struct Continent { int tile_count; uint8_t sea_coasts[8]; };
static std::vector<std::string> events;
static std::vector<int> ocean_x, region_x, continent_reads;
static std::vector<int> bitmask_inputs, treaty_owners, xrange_inputs;
struct ContinentTable {
    Continent entries[128];
    Continent& operator[](int index) {
        assert(index >= 0 && index < 128);
        continent_reads.push_back(index);
        events.push_back("continent:" + std::to_string(index));
        return entries[index];
    }
};
static Map MapTiles[TileWidth * TileHeight];
static Base Bases[4];
static ContinentTable Continents;
static int VehMoveDeltaX[9] = {1, 2, 1, 0, -1, -2, -1, 0, 0};
static int VehMoveDeltaY[9] = {-1, 0, 1, 2, 1, 0, -1, -2, 0};
static int MapIsFlat, MapLongitudeBounds, MapLatitudeBounds;
static int mutate_mask_on_call;
static bool pact_for_owner[16];

static void event(const char *name, int x = 0, int y = 0) {
    events.push_back(std::string(name) + ":" + std::to_string(x) + ":" +
                     std::to_string(y));
}
static Map *map_loc(int x, int y) {
    assert(x >= 0 && x < TileWidth && y >= 0 && y < TileHeight);
    return &MapTiles[y * TileWidth + x];
}
static int xrange(int x) {
    xrange_inputs.push_back(x);
    if (!(MapIsFlat & 1)) {
        if (x < 0) x += MapLongitudeBounds;
        else if (x >= MapLongitudeBounds) x -= MapLongitudeBounds;
    }
    return x;
}
static BOOL on_map(int x, int y) {
    event("on_map", x, y);
    return y >= 0 && y < MapLatitudeBounds && x >= 0 && x < MapLongitudeBounds;
}
static BOOL is_ocean(int x, int y) {
    ocean_x.push_back(x);
    event("ocean", x, y);
    const bool answer = map_loc(x, y)->climate < 0x60;
    return answer;
}
static int region_at(int x, int y) {
    const int result = map_loc(x, y)->region;
    region_x.push_back(x);
    event("region", x, result);
    return result;
}
static void bitmask(int input, int *offset, int *mask) {
    bitmask_inputs.push_back(input);
    event("bitmask", input, static_cast<int>(bitmask_inputs.size()));
    const int shift = input & 7;
    *offset = input / 8;
    *mask = 1 << shift;
    if (mutate_mask_on_call &&
        static_cast<int>(bitmask_inputs.size()) == mutate_mask_on_call) {
        // Simulates a change at the real external call boundary; the caller
        // must read the destination mask after bitmask returns.
        Continents.entries[52].sea_coasts[*offset] = static_cast<uint8_t>(*mask);
    }
}
static BOOL has_treaty(int faction, int owner, int treaty) {
    assert(treaty == DTREATY_PACT);
    treaty_owners.push_back(owner);
    event("treaty", faction, owner);
    return pact_for_owner[owner];
}

// The extracted actual inline helper preserves the typed nibble/bounds path.
VEH_WHO_BODY

// Three complete current candidate definitions are inserted from the native
// source reader.  Each host service above is an observable typed boundary.
BASE_BODY
PORT_BODY
ZOC_BODY

static void reset_fixture() {
    std::memset(MapTiles, 0, sizeof(MapTiles));
    std::memset(Bases, 0, sizeof(Bases));
    std::memset(&Continents, 0, sizeof(Continents));
    std::memset(pact_for_owner, 0, sizeof(pact_for_owner));
    for (int i = 0; i < TileWidth * TileHeight; ++i) MapTiles[i].climate = 0x60;
    events.clear(); ocean_x.clear(); region_x.clear(); continent_reads.clear();
    bitmask_inputs.clear(); treaty_owners.clear(); xrange_inputs.clear();
    mutate_mask_on_call = 0;
    MapIsFlat = 1; MapLongitudeBounds = TileWidth; MapLatitudeBounds = TileHeight;
    Bases[0].x = 4; Bases[0].y = 8;
}
static void set_tile(int x, int y, bool ocean, int region = 0,
                     uint32_t bit = 0, uint8_t val2 = 0) {
    Map *tile = map_loc(x, y);
    tile->climate = ocean ? 0 : 0x60;
    tile->region = static_cast<uint8_t>(region);
    tile->bit = bit;
    tile->val2 = val2;
}

static void test_base_coast_order_and_sentinel_read() {
    reset_fixture();
    // Qualifying neighbors at directions 0, 3, and 6 prove the source skip
    // expression and last-region result. Direction 6 returns sentinel 127;
    // its tile_count access is in range but discarded in favor of compare=1.
    set_tile(5, 7, true, 65);
    set_tile(4, 10, true, 70);
    set_tile(3, 7, true, 127);
    Continents.entries[65].tile_count = 8;
    Continents.entries[70].tile_count = 5;
    Continents.entries[127].tile_count = 99;
    assert(base_coast(0) == 127);
    assert((ocean_x == std::vector<int>{5, 4, 2, 3}));
    assert((region_x == std::vector<int>{5, 4, 3}));
    assert((continent_reads == std::vector<int>{65, 70, 127}));
}

static void test_base_coast_round_and_flat_wrap() {
    reset_fixture();
    Bases[0].x = 0; Bases[0].y = 8;
    MapLongitudeBounds = 8; MapIsFlat = 0;
    // Direction 4 is x=-1; one wrap reaches 7 and finds this ocean region.
    set_tile(7, 9, true, 66);
    assert(base_coast(0) == 66);
    assert(xrange_inputs.size() >= 5 && xrange_inputs[4] == -1);
    assert(ocean_x.size() >= 5 && ocean_x[4] == 7);

    reset_fixture();
    Bases[0].x = 0; Bases[0].y = 8;
    MapLongitudeBounds = 8; MapIsFlat = 1;
    // In flat mode direction 4's x=-1 stays outside the map; the tile at
    // wrapped x=7 must not be sampled.
    set_tile(7, 9, true, 66);
    assert(base_coast(0) == -1);
    assert(std::find(ocean_x.begin(), ocean_x.end(), 7) == ocean_x.end());
}

static void test_port_same_region_shortcut_and_live_mask_after_callback() {
    reset_fixture();
    set_tile(4, 8, false, 50);
    assert(port_to_coast(0, 50));
    assert(region_x.size() == 1 && region_x[0] == 4);
    assert(ocean_x.empty() && bitmask_inputs.empty());

    reset_fixture();
    set_tile(4, 8, false, 50);  // base region does not equal requested region 52
    set_tile(5, 7, true, 73);   // direction 0 fails mask and skips to direction 3
    set_tile(4, 10, true, 73);  // direction 3 callback installs the matching bit
    mutate_mask_on_call = 2;
    assert(port_to_coast(0, 52));
    assert((bitmask_inputs == std::vector<int>{9, 9}));
    assert((ocean_x == std::vector<int>{5, 4}));
    assert((continent_reads == std::vector<int>{52, 52}));
    auto second_call = std::find(events.begin(), events.end(), "bitmask:9:2");
    auto first_load = std::find(events.begin(), events.end(), "continent:52");
    assert(second_call != events.end() && first_load != events.end());
    auto second_load = std::find(first_load + 1, events.end(), "continent:52");
    assert(second_load != events.end() && second_call < second_load);
}

static void test_zoc_veh_owner_domain_treaties_and_maximum() {
    reset_fixture();
    const int x = 10, y = 8, faction = 1;
    Bases[0].x = x; Bases[0].y = y;
    // Populate the actual eight compass neighbors. veh_who accepts only a
    // VEH bit plus low-nibble owner 0..7; nibble 15 and empty tiles return -1.
    set_tile(11, 7, false, 0, BIT_VEH_IN_TILE, 2);
    set_tile(12, 8, false, 0, BIT_VEH_IN_TILE, 7);
    set_tile(11, 9, false, 0, BIT_VEH_IN_TILE, 4);
    set_tile(10, 10, false, 0, BIT_VEH_IN_TILE, 1); // same faction
    set_tile(9, 9, false, 0, BIT_VEH_IN_TILE, 0);
    set_tile(8, 8, false, 0, BIT_VEH_IN_TILE, 15); // rejected nibble
    set_tile(9, 7, false, 0, 0, 6);                // no VEH bit
    set_tile(10, 6, false, 0, BIT_VEH_IN_TILE, 6);
    pact_for_owner[7] = true;
    assert(zoc_veh(x, y, faction) == 7); // max eligible owner + one (owner 6)
    assert((treaty_owners == std::vector<int>{2, 7, 4, 0, 6}));
    assert(ocean_x.empty());
}

int main() {
    test_base_coast_order_and_sentinel_read();
    test_base_coast_round_and_flat_wrap();
    test_port_same_region_shortcut_and_live_mask_after_callback();
    test_zoc_veh_owner_domain_treaties_and_maximum();
    return 0;
}
'''


def _harness(bodies: dict[str, str]) -> str:
    result = CPP.replace('VEH_WHO_BODY', bodies['veh_who'])
    result = result.replace('BASE_BODY', bodies['base'])
    result = result.replace('PORT_BODY', bodies['port'])
    return result.replace('ZOC_BODY', bodies['zoc'])


def _compile_run(compiler: str, source_text: str, directory: Path, name: str) -> subprocess.CompletedProcess[str]:
    source = directory / f'{name}.cpp'
    binary = directory / name
    source.write_text(source_text)
    built = subprocess.run(
        [compiler, '-std=c++17', '-O0', '-UNDEBUG', str(source), '-o', str(binary)],
        capture_output=True, text=True, timeout=60,
    )
    assert built.returncode == 0, built.stdout + built.stderr
    return subprocess.run([str(binary)], capture_output=True, text=True, timeout=20)


@pytest.mark.parametrize('compiler_name', ['g++', 'clang++'])
def test_map_signed_candidate_behavior_and_causal_controls(compiler_name: str, tmp_path: Path) -> None:
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(f'{compiler_name} unavailable')
    bodies = _actual_bodies()
    positive = _compile_run(compiler, _harness(bodies), tmp_path, 'positive')
    assert positive.returncode == 0, positive.stdout + positive.stderr

    # The sentinel correction restores the actual load at valid index 127.
    # The prior ternary skipped this access; this mutant must fail the access
    # trace while avoiding any execution of out-of-range byte regions 128..255.
    sentinel_mutant = dict(bodies)
    old = ('int compare = Continents[region].tile_count;\n'
           '            if (region >= 127) {\n'
           '                compare = 1;\n'
           '            }')
    new = 'int compare = (region >= 127) ? 1 : Continents[region].tile_count;'
    assert sentinel_mutant['base'].count(old) == 1
    sentinel_mutant['base'] = sentinel_mutant['base'].replace(old, new, 1)

    skip_base = dict(bodies)
    assert skip_base['base'].count('i += (2 - (i & 1));') == 1
    skip_base['base'] = skip_base['base'].replace('i += (2 - (i & 1));', 'i += 0;', 1)

    skip_port = dict(bodies)
    assert skip_port['port'].count('i += (2 - (i & 1));') == 1
    skip_port['port'] = skip_port['port'].replace('i += (2 - (i & 1));', 'i += 0;', 1)

    first_zoc = dict(bodies)
    old_max = 'if (ret <= owner) {\n                    ret = owner;'
    new_max = 'if (ret == 0) {\n                    ret = owner;'
    assert first_zoc['zoc'].count(old_max) == 1
    first_zoc['zoc'] = first_zoc['zoc'].replace(old_max, new_max, 1)

    for label, mutant in (
        ('sentinel_load', sentinel_mutant),
        ('coast_skip', skip_base),
        ('port_skip', skip_port),
        ('zoc_first_owner', first_zoc),
    ):
        result = _compile_run(compiler, _harness(mutant), tmp_path, label)
        assert result.returncode != 0, f'{label} mutant escaped the behavioral oracle'

