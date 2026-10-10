"""Actual terrain neighbor loops; bounded controls, not full rendering proof."""

from pathlib import Path
import re

import pytest

from test_console_typed_owner_members_oct10 import run_control
from test_win_zorder_pointer_flow_oct10 import extract


ROOT = Path(__file__).resolve().parents[2]


def control():
    source = (ROOT / "src/mapwin.cpp").read_text()
    renderer = extract(source, "int __cdecl gen_terrain_poly(")
    assert "pbVar18" not in renderer
    assert "Map *neighbor;" in renderer
    land = extract(renderer,
        "if (local_84 != 0 || (local_24 & 0x20) != 0 || local_30 != 0)")
    ocean = extract(renderer, "else if ((local_24 & 0x20) != 0)")
    record = extract((ROOT / "src/map.h").read_text(), "struct Map") + ";"
    deltas = []
    vehicle = (ROOT / "src/veh.cpp").read_text()
    for axis in ("X", "Y"):
        found = re.findall(r"^int VehMoveDelta" + axis + r"\[9\] = \{[^}]+\};",
                           vehicle, re.M)
        assert len(found) == 1
        deltas.append(found[0])
    return r'''
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <initializer_list>
RECORD
static_assert(sizeof(Map)==0x2c,"tile stride");
static_assert(offsetof(Map,climate)==0,"climate byte");
static_assert(offsetof(Map,bit)==8,"terrain flags");
static_assert(offsetof(Map,bit2)==12,"secondary flags");
DELTAS
static Map tiles[32];
static int MapLongitude,MapLatitudeBounds,MapLongitudeBounds,MapIsFlat;
static Map *map_tiles(){return tiles;}
struct Masks { uint32_t fungus,jungle,moisture; };
static Masks actual(bool land,uint32_t local_24,uint32_t local_34,
                    uint32_t local_84,int a7,int a8) {
    int iVar5,iVar6,iVar19;
    uint32_t uVar12,local_30=0,local_40=0,local_2c=0,local_28=0;
    uint8_t bVar11;
    Map *neighbor;
    if(land) {
        local_30=(uint32_t)((local_34&0x80000004)==4);
        LAND
    } OCEAN
    return {local_40,local_2c,local_28};
}
// Independent per-direction oracle. It does not use the production pointer
// expression or the actual-source mask writes; wrapping is mathematical modulo.
static Masks expected(bool land,uint32_t flags,uint32_t flags2,
                      uint32_t moisture,int x,int y) {
    Masks out={0,0,0};
    const bool jungle=(flags2&0x80000004u)==4;
    if(land ? (!moisture && !(flags&0x20u) && !jungle) : !(flags&0x20u))return out;
    for(unsigned direction=0;direction<8;++direction) {
        const unsigned delta=(direction+7)%8;
        int column=x+VehMoveDeltaX[delta],row=y+VehMoveDeltaY[delta];
        if(!MapIsFlat)column=(column%MapLongitudeBounds+MapLongitudeBounds)%MapLongitudeBounds;
        if(row<0 || row>=MapLatitudeBounds || column<0 || column>=MapLongitudeBounds)continue;
        const Map &tile=tiles[row*MapLongitude+column/2];
        const unsigned altitude=tile.climate/32;
        if((tile.bit&0x20u) && altitude>=2)out.fungus|=1u<<direction;
        if(land && jungle && !(tile.bit2&0x80000000u) && (tile.bit2&4u) && altitude>=3)
            out.jungle|=1u<<direction;
        if(land && moisture && (altitude<3 || ((tile.climate/8)%4)>=moisture))
            out.moisture|=1u<<direction;
    }
    return out;
}
int main() {
    const unsigned seeds[]={0,1,7,8,0x17,0x18,0x1f,0x20,0x3f,0x40,0x5f,0x60,0x7f,0x80,0x9f,0xff};
    const uint32_t secondary[]={0,4,0x80000004u,0xffffffffu};
    for(int width: {4,8})for(int rows: {3,5}) {
        MapLongitudeBounds=width;MapLongitude=width/2;MapLatitudeBounds=rows;
        for(unsigned seed:seeds) {
            for(int i=0;i<MapLongitude*rows;++i) {
                tiles[i].climate=(uint8_t)(seed+i*37);
                tiles[i].bit=((seed+i)%2 ? 0x20u:0u)|0x200000u;
                tiles[i].bit2=secondary[(seed+i)%4];
            }
            for(int flat=0;flat<2;++flat)for(int land=0;land<2;++land)
            for(unsigned flags: {0u,0x20u})for(uint32_t flags2:secondary)
            for(unsigned moisture=0;moisture<4;++moisture)
            for(int y=0;y<rows;++y)for(int x=0;x<width;++x) {
                MapIsFlat=flat;
                const Masks a=actual(land,flags,flags2,moisture,x,y);
                const Masks e=expected(land,flags,flags2,moisture,x,y);
                assert(a.fungus==e.fungus && a.jungle==e.jungle && a.moisture==e.moisture);
            }
        }
    }
}
'''.replace("RECORD", record).replace("DELTAS", "\n".join(deltas)).replace("LAND", land).replace("OCEAN", ocean)


@pytest.mark.parametrize("compiler", ("g++", "clang++"))
@pytest.mark.parametrize("damage", (None,
    ("neighbor->bit2 & 0x80000004", "neighbor->bit & 0x80000004"),
    ("0x5f < (neighbor->climate & 0xe0)", "0x7f < (neighbor->climate & 0xe0)"),
    ("(int)(neighbor->climate >> 3 & 3)", "(int)(neighbor->climate >> 3 & 1)"),
    ("bVar11 = (uint8_t)iVar5;", "bVar11 = (uint8_t)(iVar5+1);")))
def test_actual_neighbor_loops(tmp_path, compiler, damage):
    run_control(tmp_path, compiler, control(), damage)
