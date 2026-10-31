"""Actual-source controls for the territory-border Map region reads."""

from pathlib import Path
import re

import pytest

from test_console_typed_owner_members_oct10 import run_control
from test_win_zorder_pointer_flow_oct10 import extract


ROOT = Path(__file__).resolve().parents[2]


def actual_builder():
    source = (ROOT / "src/mapwin.cpp").read_text()
    map_header = (ROOT / "src/map.h").read_text()
    map_lookups = "\n".join((
        extract(map_header, "inline Map *__cdecl map_loc("),
        extract(map_header, "MEASURED inline int __cdecl region_at("),
    ))
    renderer = extract(source, "int __cdecl gen_terrain_poly(")
    border = renderer.index("// THE TERRITORY BORDER")
    call_start = renderer.index(
        "territory_faction = whose_territory(ConsoleGlobal.faction_id_, a7,",
        border,
    )
    call_end = renderer.index(";", call_start) + 1
    center_call = renderer[call_start:call_end]
    center_test = extract(renderer, "if (-1 < (int)territory_faction)")
    builder = center_call + "\n" + center_test

    # A real-product correction is required. Do not project the correction
    # into the extracted body if this test is run against a stale source tree.
    assert "local_c0 = (uint32_t)region_at(a7, a8);" in builder
    assert "(map_loc(a7, a8)->climate & 0xe0)" in builder
    assert ".region == local_c0" in builder
    assert ".contour == local_c0" not in builder
    assert "g_RAINFALL_SINGLE_TILE_TEXTURE.draw_mask_dest(" in builder
    return builder, map_lookups


HARNESS = r'''#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <vector>

#define __cdecl
#define MEASURED
MAP_RECORD
static_assert(sizeof(Map)==0x2c,"Map record stride");
static_assert(offsetof(Map,contour)==1,"contour is byte +1");
static_assert(offsetof(Map,region)==3,"region is byte +3");
DELTAS

static Map storage_a[64], storage_b[64];
static Map *active_tiles=storage_a;
static int MapLongitude=4, MapLatitudeBounds=7, MapLongitudeBounds=8;
static int MapIsFlat=1;
static Map *map_tiles() { return active_tiles; }
MAP_LOOKUPS
struct ConsoleProjection { int faction_id_; } ConsoleGlobal = {1};
static Map *center_tile;
static Map *mutated_neighbor;
static int test_case;
struct Call { int faction,x,y; };
static std::vector<Call> calls;
static uint8_t last_mask[10];
static int draw_calls;
struct Vert { int x,y; };
static Vert MapCornerScratch[4];
static uint8_t TerritoryColorTables[16];
static int BufferObject;
static void *buffer = &BufferObject;
struct MaskTexture {
    void draw_mask_dest(void *, Vert *, uint8_t *mask, uint8_t *, int count,
                        Vert *) {
        assert(count==4);
        ++draw_calls;
        memcpy(last_mask,mask,sizeof last_mask);
    }
} g_RAINFALL_SINGLE_TILE_TEXTURE;

static uint32_t whose_territory(int faction,int x,int y,int,int) {
    Call call={faction,x,y}; calls.push_back(call);
    if(calls.size()==1) {
        if(test_case==2) {
            // Synthetic lookup-order sentinel: force the helper to resolve
            // map_tiles()/MapLongitude after this call. The real original
            // whose_territory is readonly; this does not claim it rebinds the map.
            active_tiles=storage_b;
            MapLongitude=5;
        }
        return 2;
    }
    if(x==6 && y==2) {
        if(test_case==2) {
            // The region qualification for this neighbor must already have
            // happened before this callback changes current map storage.
            map_tiles()[MapLongitude*3+(5>>1)].region=11;
            mutated_neighbor->region=8;
        }
        return 3;
    }
    if(x==3 && y==3) return 4;
    return 2;
}

static void build_territory_border(Map *tile,int a7,int a8) {
    Map center_snapshot=*tile; // Only the timing-damage variant reads this.
    uint32_t territory_faction=0;
    uint32_t local_c0=0,local_44=0,uVar12=0;
    int iVar5=0,iVar19=0;
    char cVar4=0;
    uint8_t territory_mask[10];
    Vert overlay_uv[4]={{0,0},{0,0},{0,0},{0,0}};
    (void)center_snapshot;
    BUILDER
    memcpy(last_mask,territory_mask,sizeof territory_mask);
}

static void reset_case(int which) {
    memset(storage_a,0,sizeof storage_a);
    memset(storage_b,0,sizeof storage_b);
    active_tiles=storage_a;
    memset(last_mask,0,sizeof last_mask);
    calls.clear(); draw_calls=0; test_case=which;
    MapIsFlat=1; MapLongitude=4; MapLongitudeBounds=8; MapLatitudeBounds=7;
    for(int i=0;i<64;++i) {
        storage_a[i].climate=0x80;
        storage_a[i].region=8;
        storage_a[i].contour=0x44;
        storage_b[i].climate=0x80;
        storage_b[i].region=8;
        storage_b[i].contour=0x44;
    }
    center_tile=&storage_a[3*MapLongitude+(5>>1)];
    center_tile->region=(which==2)?7:9;
    center_tile->contour=0x44;
    if(which==2) center_tile->climate=0x00;
    Map *k0=&storage_a[2*MapLongitude+(6>>1)];
    Map *k5=&storage_a[3*MapLongitude+(3>>1)];
    mutated_neighbor=k0;
    if(which==0) {
        k0->region=9;
        k0->contour=0x22; // Same region, deliberately different contour.
    } else if(which==1) {
        k0->region=8;
        k0->contour=0x44; // Different region, same contour.
    } else {
        // The selected live allocation/width will change during the initial
        // territory query. The entry tile remains in storage_a; helper reads
        // and subsequent neighbor tests must use storage_b with width five.
        storage_b[3*5+(5>>1)].region=9;
        storage_b[3*5+(5>>1)].contour=0x44;
        storage_b[2*5+(6>>1)].region=9;
        storage_b[2*5+(6>>1)].contour=0x22;
        storage_b[3*5+(3>>1)].region=9;
        storage_b[3*5+(3>>1)].contour=0x33;
        storage_b[2*5+(6>>1)].region=9;
        mutated_neighbor=&storage_b[2*5+(6>>1)];
    }
}

static void expect_mask(const uint8_t expected[10]) {
    for(int i=0;i<10;++i) assert(last_mask[i]==expected[i]);
}

int main() {
    const uint8_t same_region_different_contour[10]={0,3,3,0,0,0,0,0,3,0};
    const uint8_t different_region_same_contour[10]={0,0,0,0,0,0,0,0,0,0};
    const uint8_t cached_and_prechecked[10]={0,3,3,0,0,0,3,0,3,0};

    reset_case(0);
    build_territory_border(center_tile,5,3);
    assert(draw_calls==1 && calls.size()==2);
    assert(calls[0].x==5 && calls[0].y==3);
    assert(calls[1].x==6 && calls[1].y==2);
    expect_mask(same_region_different_contour);

    reset_case(1);
    build_territory_border(center_tile,5,3);
    assert(draw_calls==0 && calls.size()==1);
    expect_mask(different_region_same_contour);

    reset_case(2);
    build_territory_border(center_tile,5,3);
    assert(active_tiles==storage_b && MapLongitude==5);
    assert(center_tile->region==7); // Entry pointer/storage was not rewritten.
    assert(storage_b[3*5+(5>>1)].region==11 && mutated_neighbor->region==8);
    assert(draw_calls==1 && calls.size()==3);
    assert(calls[0].x==5 && calls[0].y==3);
    assert(calls[1].x==6 && calls[1].y==2);
    assert(calls[2].x==3 && calls[2].y==3);
    expect_mask(cached_and_prechecked);
}
'''


def control(builder):
    map_record = extract((ROOT / "src/map.h").read_text(), "struct Map {") + ";"
    vehicle_source = (ROOT / "src/veh.cpp").read_text()
    deltas = []
    for axis in ("X", "Y"):
        found = re.findall(r"^int VehMoveDelta" + axis + r"\[9\] = \{[^}]+\};",
                           vehicle_source, re.M)
        assert len(found)==1
        deltas.append(found[0])
    body, map_lookups = builder
    return (HARNESS.replace("MAP_RECORD",map_record)
            .replace("DELTAS","\n".join(deltas))
            .replace("MAP_LOOKUPS",map_lookups)
            .replace("BUILDER",body))


@pytest.mark.parametrize("compiler", ("g++","clang++"))
@pytest.mark.parametrize("damage", (None,
    ("local_c0 = (uint32_t)region_at(a7, a8);",
     "local_c0 = (uint32_t)map_loc(a7, a8)->contour;"),
    (".region == local_c0", ".contour == local_c0"),
    ("local_c0 = (uint32_t)region_at(a7, a8);",
     "local_c0 = (uint32_t)tile->region;"),
    ("local_c0 = (uint32_t)region_at(a7, a8);",
     "local_c0 = (uint32_t)center_snapshot.region;"),
    ("return &map_tiles()[(x >> 1) + y * MapLongitude];",
     "return &storage_a[(x >> 1) + y * 4];"),
    ("(map_loc(a7, a8)->climate & 0xe0) < 0x60",
     "(tile->climate & 0xe0) < 0x60")))
def test_terrain_border_uses_actual_region_members_and_callback_timing(
        tmp_path,compiler,damage):
    run_control(tmp_path,compiler,control(actual_builder()),damage)
