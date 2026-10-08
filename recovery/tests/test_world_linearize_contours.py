"""Actual-source host controls for world_linearize_contours storage fidelity."""
from pathlib import Path
import shutil
import subprocess

import pytest


ROOT = Path(__file__).resolve().parents[2]


def extract_body(source: str, signature: str) -> str:
    start = source.index(signature)
    opening = source.index("{", start)
    depth = 0
    for end in range(opening, len(source)):
        depth += (source[end] == "{") - (source[end] == "}")
        if depth == 0:
            return source[start:end + 1]
    raise AssertionError(f"unterminated body: {signature}")


HARNESS = r"""
#include <assert.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <vector>
#define __cdecl
static const int ALT_3_LEVELS_ABOVE_SEA=6;

@@MAP_POD@@
int MapLongitudeBounds=8;
int MapLatitudeBounds=7;
int MapSeaLevel=4;
uint32_t MapLongitude=4;
Map *MapTiles=0;
uint32_t AltContour[11];
uint32_t AltNaturalDefault[11];
uint32_t AltNatural[11];

inline Map *&map_tiles() { return MapTiles; }
@@MAP_LOC@@

static int bands_seen=0;
static int last_band=0;
static std::vector<int> coord_x, coord_y, put_values, row_callbacks;
static std::vector<uint32_t> natural_during_put, natural_during_row;
static int deltas[7]={-117,-5,0,13,-9,21,37};
static const int row_widths[5]={8,6,10,10,10};

int alt_natural(int x,int y) {
    coord_x.push_back(x); coord_y.push_back(y);
    last_band=bands_seen++ % 7;
    return last_band;
}
int alt_detail_at(int,int) {
    const int call=bands_seen-1;
    return static_cast<int>(AltNaturalDefault[last_band]) + deltas[call % 7];
}
void alt_put_detail(int x,int y,int value) {
    assert(x==(int)coord_x[put_values.size()]);
    assert(y==(int)coord_y[put_values.size()]);
    put_values.push_back(value);
    natural_during_put.push_back(AltNatural[0]);
}
void do_all_non_input() {
    const int row=static_cast<int>(row_callbacks.size());
    row_callbacks.push_back(bands_seen);
    natural_during_row.push_back(AltNatural[0]);
    if (row==0) MapLongitudeBounds=6;
    else if (row==1) MapLongitudeBounds=10;
    else if (row==2) MapLatitudeBounds=5;
}

int range(int input,int minimum,int maximum) {
    return input<minimum ? minimum : input>maximum ? maximum : input;
}

@@ELEV_AT@@
@@LINEARIZE@@

static int32_t expected_value(int i,int detail) {
    const int next=i+((i>=6)?4:1);
    // Original code wraps the subtraction/multiply to 32 bits, then CDQ/IDIV
    // interprets those numerator bits as signed. The denominator is signed;
    // final contour addition wraps again before the full EAX value is passed.
    uint32_t contour_delta=AltContour[next]-AltContour[i];
    uint32_t detail_delta=static_cast<uint32_t>(detail)-AltNaturalDefault[i];
    uint32_t product_bits=contour_delta*detail_delta;
    int32_t numerator=static_cast<int32_t>(product_bits);
    int32_t denominator=static_cast<int32_t>(AltNaturalDefault[i+1]-AltNaturalDefault[i]);
    int32_t quotient=numerator/denominator;
    uint32_t result_bits=AltContour[i]+static_cast<uint32_t>(quotient);
    return static_cast<int32_t>(result_bits);
}

int main() {
    Map tiles[32]={};
    MapTiles=tiles;
    MapLongitude=4;
    AltContour[2]=100;
    AltContour[3]=50;
    AltContour[0]=7;
    AltNaturalDefault[2]=9000;
    AltNaturalDefault[3]=77777;
    AltNatural[2]=13;
    tiles[0].contour=80;
    tiles[0].climate=static_cast<uint8_t>(2<<5);
    tiles[1].contour=110;
    tiles[1].climate=static_cast<uint8_t>(2<<5);
    assert(elev_at(0,0)==1310); // false threshold branch; +10, not a detail table
    assert(elev_at(2,0)==2830); // true branch; uses contour table and deterministic noise
    tiles[2].contour=255;
    tiles[2].climate=static_cast<uint8_t>(2<<5);
    assert(elev_at(4,0)==3500); // upper clamp
    tiles[3].contour=0;
    tiles[3].climate=static_cast<uint8_t>(2<<5);
    MapSeaLevel=20;
    assert(elev_at(6,0)==-3000); // lower clamp
    MapSeaLevel=4;

    const uint32_t contour[11]={0x102,0x11B,0x130,0x146,0x168,0x195,
                                0x27F,0x2A0,0x2D0,0x300,0x330};
    const uint32_t defaults[11]={1000,2000,3000,4000,5000,6000,7000,8000,9000,10000,11000};
    const uint32_t prior_natural[11]={401,503,607,709,811,907,1009,1103,1201,1301,1409};
    memcpy(AltContour,contour,sizeof(contour));
    AltContour[2]+=0x40; // externally supplied/mutated contour input
    uint32_t expected_contour[11];
    memcpy(expected_contour,AltContour,sizeof(expected_contour));
    memcpy(AltNaturalDefault,defaults,sizeof(defaults));
    memcpy(AltNatural,prior_natural,sizeof(prior_natural));
    world_linearize_contours();

    // Parity and per-row continuation are observable through the callbacks.
    assert(coord_x.size()==22 && coord_y.size()==22 && put_values.size()==22);
    size_t cursor=0;
    for(int y=0;y<5;++y) {
        const int first=y&1;
        for(int x=first;x<row_widths[y];x+=2) {
            assert(coord_x[cursor]==x && coord_y[cursor]==y);
            const int band=static_cast<int>(cursor%7);
            const int detail=static_cast<int>(AltNaturalDefault[band])+deltas[cursor%7];
            assert(put_values[cursor]==expected_value(band,detail));
            ++cursor;
        }
        static const int completed[5]={4,7,12,17,22};
        assert(row_callbacks[y]==completed[y]);
    }
    // Every writer/row callback runs before the terminal copy; only on return
    // does AltNatural become the final AltContour contents.
    for(uint32_t observed:natural_during_put) assert(observed==prior_natural[0]);
    for(uint32_t observed:natural_during_row) assert(observed==prior_natural[0]);
    assert(memcmp(AltNatural,AltContour,sizeof(AltNatural))==0);
    assert(memcmp(AltContour,expected_contour,sizeof(expected_contour))==0);
    assert(memcmp(AltNaturalDefault,defaults,sizeof(defaults))==0);
    // Literal witnesses independently pin signed negative division and the
    // i==6 (+4) contour-table branch; unsigned division/+3 produce other values.
    assert(put_values[0]==256);
    assert(put_values[6]==645);

    // Zero height performs no lookups/writes/row callbacks but still performs
    // the unconditional terminal table copy.
    coord_x.clear(); coord_y.clear(); put_values.clear(); row_callbacks.clear();
    natural_during_put.clear(); natural_during_row.clear(); bands_seen=0;
    memcpy(AltNatural,prior_natural,sizeof(prior_natural));
    memcpy(AltContour,expected_contour,sizeof(expected_contour));
    MapLatitudeBounds=0;
    world_linearize_contours();
    assert(coord_x.empty() && coord_y.empty() && put_values.empty() && row_callbacks.empty());
    assert(memcmp(AltNatural,AltContour,sizeof(AltNatural))==0);

    row_callbacks.clear(); natural_during_row.clear(); bands_seen=0;
    memcpy(AltNatural,prior_natural,sizeof(prior_natural));
    MapLatitudeBounds=-1;
    world_linearize_contours();
    assert(coord_x.empty() && coord_y.empty() && put_values.empty() && row_callbacks.empty());
    assert(memcmp(AltNatural,AltContour,sizeof(AltNatural))==0);

    // Positive height with zero width still runs the per-row callback before
    // the same terminal copy, without making any tile calls.
    row_callbacks.clear(); natural_during_row.clear(); bands_seen=0;
    memcpy(AltNatural,prior_natural,sizeof(prior_natural));
    // One positive row is enough to observe the callback; it changes the
    // width afterward, so later rows would intentionally see that new width.
    MapLatitudeBounds=1; MapLongitudeBounds=-2;
    world_linearize_contours();
    assert(coord_x.empty() && coord_y.empty() && put_values.empty());
    assert(row_callbacks.size()==1);
    for(uint32_t observed:natural_during_row) assert(observed==prior_natural[0]);
    assert(memcmp(AltNatural,AltContour,sizeof(AltNatural))==0);
}
"""


def actual_body() -> tuple[str, str, str, str]:
    source = (ROOT / "src/map.cpp").read_text()
    header = (ROOT / "src/map.h").read_text()
    return (
        extract_body(header, "struct Map {") + ";",
        extract_body(header, "inline Map *__cdecl map_loc(int x, int y)"),
        extract_body(source, "int __cdecl elev_at(int x, int y)"),
        extract_body(source, "void __cdecl world_linearize_contours()"),
    )


def run_host(compiler_name, tmp_path, bodies):
    compiler = shutil.which(compiler_name)
    if not compiler:
        pytest.skip(f"{compiler_name} unavailable")
    source = tmp_path / "world-linearize.cpp"
    if isinstance(bodies, tuple):
        map_pod, map_loc, elev, linearize = bodies
    else:
        map_pod, map_loc, elev, _linearize = actual_body()
        linearize = bodies
    harness = HARNESS.replace("@@MAP_POD@@", map_pod)
    harness = harness.replace("@@MAP_LOC@@", map_loc)
    harness = harness.replace("@@ELEV_AT@@", elev)
    harness = harness.replace("@@LINEARIZE@@", linearize)
    source.write_text(harness)
    binary = tmp_path / "world-linearize"
    built = subprocess.run(
        [compiler, "-std=c++11", "-Wall", "-Wextra", "-Werror", "-fsanitize=undefined",
         str(source), "-o", str(binary)],
        capture_output=True, text=True, timeout=30,
    )
    assert built.returncode == 0, built.stderr
    return subprocess.run([str(binary)], capture_output=True, text=True, timeout=10)


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
def test_actual_world_linearize_contours_storage(compiler_name, tmp_path):
    run = run_host(compiler_name, tmp_path, actual_body())
    assert run.returncode == 0, run.stderr


@pytest.mark.parametrize("before,after", [
    ("const uint32_t contour = AltContour[band];",
     "const uint32_t contour = AltNatural[band];"),
    ("const uint32_t input_floor = AltNaturalDefault[band];",
     "const uint32_t input_floor = AltNatural[band];"),
    ("((band >= ALT_3_LEVELS_ABOVE_SEA) ? 4 : 1)",
     "((band >= ALT_3_LEVELS_ABOVE_SEA) ? 3 : 1)"),
    ("static_cast<uint32_t>(numerator / denominator)",
     "static_cast<uint32_t>(static_cast<uint32_t>(numerator) / static_cast<uint32_t>(denominator))"),
    ("alt_put_detail(x, y, static_cast<int32_t>(result));",
     "alt_put_detail(x, y, static_cast<uint8_t>(result));"),
    ("memcpy(AltNatural, AltContour, sizeof(AltContour));",
     "memcpy(AltNatural, AltNaturalDefault, sizeof(AltNatural));"),
])
@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
def test_contour_storage_mutants_are_observed(compiler_name, tmp_path, before, after):
    body = actual_body()[3]
    assert before in body
    run = run_host(compiler_name, tmp_path, body.replace(before, after, 1))
    assert run.returncode != 0, "mutant unexpectedly preserved contour behavior"


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
def test_contour_copy_moved_before_row_callbacks_is_observed(compiler_name, tmp_path):
    body = actual_body()[3]
    before = "        do_all_non_input();"
    assert before in body
    mutant = body.replace(
        before,
        "        memcpy(AltNatural, AltContour, sizeof(AltContour));\n" + before,
        1,
    )
    run = run_host(compiler_name, tmp_path, mutant)
    assert run.returncode != 0, "premature copy was not visible to row callbacks"


def test_source_load_order_and_full_detail_argument():
    body = actual_body()[3]
    order = [
        "const int band = alt_natural(x, y);",
        "const uint32_t detail = static_cast<uint32_t>(alt_detail_at(x, y));",
        "const uint32_t contour = AltContour[band];",
        "const uint32_t input_floor = AltNaturalDefault[band];",
        "const uint32_t target = AltContour[band",
        "const int32_t denominator =",
        "const int32_t numerator =",
        "alt_put_detail(x, y, static_cast<int32_t>(result));",
    ]
    positions = [body.index(item) for item in order]
    assert positions == sorted(positions)
    assert "alt_put_detail(x, y, static_cast<uint8_t>" not in body

