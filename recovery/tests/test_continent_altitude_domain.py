"""Actual-source controls for continent seed altitude-domain comparisons."""
from pathlib import Path
import re
import shutil
import subprocess

from match_recovery.includes import COMMENTS_OR_STRINGS


ROOT = Path(__file__).resolve().parents[2]


def _masked(source):
    return COMMENTS_OR_STRINGS.sub(lambda m: re.sub(r"[^\n]", " ", m[0]), source)


def _definition(source, signature):
    masked = _masked(source)
    matches = list(re.finditer(signature, masked))
    assert len(matches) == 1, signature
    start = matches[0].start()
    opening = masked.index("{", start)
    depth = 0
    for pos in range(opening, len(masked)):
        depth += (masked[pos] == "{") - (masked[pos] == "}")
        if depth == 0:
            return source[start:pos + 1]
    raise AssertionError("unterminated definition: " + signature)


HARNESS = r'''#include <cassert>
#include <cstdint>
#include <cstring>
#define __cdecl
#define MEASURED
MAP_RECORD
ALTITUDE_ENUM
ALTITUDE_VALUE_ENUM
static int MapLongitude=8;
static int MapLongitudeBounds=16,MapLatitudeBounds=16,MapArea=1000,MapIsFlat=0;
static int MapLandCoverage=2,BrushVal1=0,WorldBuildVal1=100;
static Map MapTilesStorage[1100];
static Map *map_tiles() { return MapTilesStorage; }
static unsigned map_loc_calls;
MAP_LOC
ALT_AT
ALTITUDE_AT

struct RulesWorldbuilder;
WORLD_BUILDER_TYPE
RulesWorldbuilder WorldBuilder = {};

static int random_values[512];
static int random_length;
static int random_cursor;
static int next_rand() {
    assert(random_cursor < random_length);
    return random_values[random_cursor++];
}
#define rand next_rand
static int paint_calls,do_calls;
static int painted_x,painted_y,painted_altitude,painted_radius;
static void paint_land(int x,int y,int altitude,int radius) {
    ++paint_calls; painted_x=x; painted_y=y;
    painted_altitude=altitude; painted_radius=radius;
}
static void do_all_non_input() { ++do_calls; }

BUILD_CONTINENT

static void setup() {
    std::memset(MapTilesStorage,0,sizeof(MapTilesStorage));
    for(int i=0;i<1100;++i) MapTilesStorage[i].region=0x35;
    MapLandCoverage=2; BrushVal1=0; WorldBuildVal1=100;
    MapLongitude=8; MapLongitudeBounds=16; MapLatitudeBounds=16;
    MapArea=1000; MapIsFlat=0;
    WorldBuilder.continent_base=20; WorldBuilder.continent_mod=1;
    WorldBuilder.cont_size_ratio[0]=0; WorldBuilder.cont_size_ratio[1]=0;
    WorldBuilder.cont_size_ratio[2]=0; WorldBuilder.cont_size_ratio[3]=0;
    WorldBuilder.cont_size_ratio[4]=0;
    random_length=0; random_cursor=0; map_loc_calls=0;
    paint_calls=0; do_calls=0; painted_x=painted_y=-1;
    painted_altitude=painted_radius=-1;
}
static void push_candidate(int x_random,int y_random) {
    random_values[random_length++]=x_random;
    random_values[random_length++]=y_random;
}
static void set_altitude(int x,int y,int altitude) {
    MapTilesStorage[(x >> 1) + y * MapLongitude].climate=(uint8_t)altitude;
}
static void assert_region_reset() {
    for(int i=0;i<MapArea;++i) assert(MapTilesStorage[i].region==0);
    assert(MapTilesStorage[MapArea].region==0x35);
}

int main() {
    // Shelf water at the selected center is accepted and painted once.
    setup(); push_candidate(4,2); set_altitude(4,6,ALT_BIT_OCEAN_SHELF);
    build_continent(1);
    assert(paint_calls==1 && do_calls==1);
    assert(painted_x==4 && painted_y==6 && painted_altitude==ALT_SHORE_LINE);
    assert(painted_radius==6 && map_loc_calls==2 && random_cursor==2);
    assert_region_reset();

    // Shoreline (3) and highest land (7) are both rejected; the third water
    // candidate is retained. Coordinates are consumed as the source does.
    setup(); push_candidate(2,0); push_candidate(6,0); push_candidate(8,2);
    set_altitude(2,4,ALT_BIT_SHORE_LINE);
    set_altitude(6,4,0xE0); // raw altitude band 7
    set_altitude(8,6,ALT_BIT_OCEAN);
    build_continent(1);
    assert(paint_calls==1 && do_calls==1);
    assert(painted_x==8 && painted_y==6 && map_loc_calls==4 && random_cursor==6);

    // A land candidate followed by water paints the second candidate.
    setup(); push_candidate(2,0); push_candidate(8,0);
    set_altitude(2,4,ALT_BIT_2_LEVELS_ABOVE_SEA);
    set_altitude(8,4,ALT_BIT_OCEAN_SHELF);
    build_continent(1);
    assert(paint_calls==1 && painted_x==8 && painted_y==4);
    assert(map_loc_calls==3 && random_cursor==4);

    // Preserve the original y-parity quirk: odd y increments x after x was
    // made even, so the selected/painted x is odd again.
    setup(); push_candidate(5,1); set_altitude(5,5,ALT_BIT_OCEAN);
    build_continent(1);
    assert(paint_calls==1 && painted_x==5 && painted_y==5);

    // Candidate 100 still gets the final fresh altitude check: its water tile
    // can paint after 99 rejected land candidates.
    setup();
    for(int i=0;i<99;++i) push_candidate(2,0);
    push_candidate(8,2);
    set_altitude(2,4,0xE0);
    set_altitude(8,6,ALT_BIT_OCEAN_SHELF);
    build_continent(1);
    assert(random_cursor==200 && map_loc_calls==100);
    assert(paint_calls==1 && do_calls==1 && painted_x==8 && painted_y==6);

    // The pre-increment cap makes 100 coordinate pairs. The first 99 are
    // condition reads, and the final acceptance check rereads candidate 100.
    // An always-land final candidate is not painted.
    setup();
    for(int i=0;i<100;++i) push_candidate(2,0);
    for(int i=0;i<1100;++i) MapTilesStorage[i].climate=0xE0;
    build_continent(1);
    assert(map_loc_calls==100 && random_cursor==200);
    assert(paint_calls==0 && do_calls==0);
    assert_region_reset();
    return 0;
}
'''


def _program(damage=None):
    map_header=(ROOT / "src/map.h").read_text()
    alpha_header=(ROOT / "src/alpha.h").read_text()
    world_source=(ROOT / "src/world.cpp").read_text()
    map_record=_definition(map_header,r"struct\s+Map\s*{") + ";"
    altitude_enum=_definition(map_header,r"enum\s+TerrainAltitudeBit\s*{") + ";"
    altitude_value_enum=_definition(map_header,r"enum\s+TerrainAltitude\s*{") + ";"
    map_loc=_definition(map_header,r"\binline\s+Map\s*\*\s*__cdecl\s+map_loc\s*\(")
    map_loc=map_loc.replace("{","{ ++map_loc_calls;",1)
    alt_at=_definition(map_header,r"\bMEASURED\s+inline\s+int\s+__cdecl\s+alt_at\s*\(")
    altitude_at=_definition(map_header,r"\binline\s+int\s+__cdecl\s+altitude_at\s*\(")
    world_builder=_definition(alpha_header,r"struct\s+RulesWorldbuilder\s*") + ";"
    body=_definition(world_source,r"\bvoid\s+__cdecl\s+build_continent\s*\(")
    assert body.count("altitude_at(x, y)")==2
    if damage == "old-scaled-altitude":
        body=body.replace("altitude_at(x, y)","alt_at(x, y)")
    elif damage == "old-postincrement-cap":
        assert body.count("++count < 100")==1
        body=body.replace("++count < 100","count++ < 100")
    return (HARNESS.replace("MAP_RECORD",map_record)
            .replace("ALTITUDE_ENUM",altitude_enum)
            .replace("ALTITUDE_VALUE_ENUM",altitude_value_enum)
            .replace("MAP_LOC",map_loc)
            .replace("ALT_AT",alt_at)
            .replace("ALTITUDE_AT",altitude_at)
            .replace("WORLD_BUILDER_TYPE",world_builder)
            .replace("BUILD_CONTINENT",body))


def test_continent_uses_raw_altitude_domain_and_keeps_seed_loop_semantics(tmp_path):
    compilers=[shutil.which(name) for name in ("g++","clang++")]
    compilers=[compiler for compiler in compilers if compiler]
    assert compilers
    for damage,name in ((None,"actual"),
                        ("old-scaled-altitude","old-scaled-altitude"),
                        ("old-postincrement-cap","old-postincrement-cap")):
        program=_program(damage)
        for compiler in compilers:
            unit=tmp_path / (name + "_" + Path(compiler).name + ".cpp")
            executable=unit.with_suffix("")
            unit.write_text(program)
            compiled=subprocess.run(
                [compiler,"-std=c++11","-Wall","-Wextra","-Werror",
                 str(unit),"-o",str(executable)],text=True,capture_output=True,
                timeout=30)
            assert compiled.returncode==0, compiled.stderr
            result=subprocess.run([str(executable)],text=True,capture_output=True,
                                  timeout=10)
            if damage:
                assert result.returncode!=0,damage + " escaped actual-source controls"
            else:
                assert result.returncode==0,result.stderr
