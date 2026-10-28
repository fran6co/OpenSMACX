"""Callback-sensitivity host control for live BaseCurrent radius reads.

The fake yield callbacks deliberately replace BaseCurrent and the selected
base id between passes. This proves the source re-reads those globals at each
pass and writes through the post-callback BaseCurrent. It does not claim that
ordinary game yield callbacks mutate these globals.
"""
from __future__ import annotations

from pathlib import Path
import re
import shutil
import subprocess

import pytest


ROOT = Path(__file__).resolve().parents[2]
FUNCTIONS = (
    ("sub_4e7e50", r"int\s+__cdecl\s+sub_4e7e50\s*\(\s*int\s+radius_index\s*\)\s*\{"),
    ("sub_4e7f80", r"int\s+__cdecl\s+sub_4e7f80\s*\(\s*int\s+radius_index\s*\)\s*\{"),
)

HOST = r'''#include <cassert>
#include <cstdint>
#define __cdecl
struct Base { short x=0,y=0; uint8_t faction_id_current=0; uint32_t state=0; int intake_2[4]={}; };
struct Map { uint8_t climate=0; };
Base bases[4], *BaseCurrent=bases;
Map tiles[8];
int MapLongitude=4, MapLongitudeBounds=4, MapLatitudeBounds=4, MapIsFlat=1;
int RadiusOffsetX[8]={}, RadiusOffsetY[8]={};
int BaseIDCurrentSelected=0, TileYieldRestricted=9;
Map *map_tiles(){ return tiles; }
static int seen_owner[3], seen_id[3], callback_index=0;
static void move_current(){
  ++callback_index;
  BaseCurrent=bases+callback_index;
  ++BaseIDCurrentSelected;
}
int crop_yield(uint8_t owner,int id,int,int,int){ seen_owner[0]=owner;seen_id[0]=id;move_current();return 3; }
int mine_yield(uint8_t owner,int id,int,int,int){ seen_owner[1]=owner;seen_id[1]=id;move_current();return 4; }
int energy_yield(uint8_t owner,int id,int,int,int){ seen_owner[2]=owner;seen_id[2]=id;move_current();return 5; }
'''

MAIN = r'''
static void reset_case(int first_owner, int first_id) {
  for (int i=0;i<4;++i) { bases[i]=Base{}; bases[i].faction_id_current=uint8_t(first_owner+i); }
  BaseCurrent=bases; BaseIDCurrentSelected=first_id; callback_index=0;
  for (int i=0;i<3;++i) { seen_owner[i]=0; seen_id[i]=0; }
  tiles[0].climate=0x60; TileYieldRestricted=9;
}
int main(){
  reset_case(1,5);
  assert(sub_4e7e50(0)==12);
  assert(seen_owner[0]==1 && seen_owner[1]==2 && seen_owner[2]==3);
  assert(seen_id[0]==5 && seen_id[1]==6 && seen_id[2]==7);
  assert(bases[1].intake_2[0]==3 && bases[2].intake_2[1]==4);
  assert(bases[3].intake_2[2]==5 && bases[3].intake_2[3]==0);
  assert(TileYieldRestricted==0);
  reset_case(11,15);
  for (int i=0;i<4;++i) for (int j=0;j<4;++j) bases[i].intake_2[j]=100;
  assert(sub_4e7f80(0)==12);
  assert(seen_owner[0]==11 && seen_owner[1]==12 && seen_owner[2]==13);
  assert(seen_id[0]==15 && seen_id[1]==16 && seen_id[2]==17);
  assert(bases[1].intake_2[0]==97 && bases[2].intake_2[1]==96);
  assert(bases[3].intake_2[2]==95 && bases[3].intake_2[3]==100);
  assert(TileYieldRestricted==0);
}
'''


def _function(name: str, pattern: str) -> str:
    source = (ROOT / "src/base.cpp").read_text(encoding="utf-8")
    matches = list(re.finditer(pattern, source))
    assert len(matches) == 1, f"expected one active {name} definition"
    start = matches[0].start()
    opening = source.find("{", matches[0].start(), matches[0].end())
    depth = 0
    for i in range(opening, len(source)):
        if source[i] == "{":
            depth += 1
        elif source[i] == "}":
            depth -= 1
            if depth == 0:
                return source[start:i + 1]
    raise AssertionError(f"unterminated {name}")


def _run(tmp_path: Path, compiler: str, bodies: list[str]) -> subprocess.CompletedProcess[str]:
    cpp, exe = tmp_path / "live_owner.cpp", tmp_path / "live_owner"
    cpp.write_text(HOST + "\n".join(bodies) + MAIN, encoding="utf-8")
    built = subprocess.run(
        [compiler, "-std=c++14", "-Wall", "-Wextra", "-Werror", str(cpp), "-o", str(exe)],
        capture_output=True, text=True, timeout=30,
    )
    assert built.returncode == 0, built.stdout + built.stderr
    return subprocess.run([str(exe)], capture_output=True, text=True, timeout=10)


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
def test_actual_source_rereads_live_owner_and_intake_target(compiler_name: str,
                                                            tmp_path: Path) -> None:
    compiler = shutil.which(compiler_name)
    if not compiler:
        pytest.skip(f"{compiler_name} unavailable")
    bodies = [_function(name, pattern) for name, pattern in FUNCTIONS]
    actual = _run(tmp_path, compiler, bodies)
    assert actual.returncode == 0, actual.stdout + actual.stderr

    # Recreate the previous captured-owner implementation. The callback
    # sequence makes that version fail the changing-owner assertions above.
    old_owner_bodies = []
    for body in bodies:
        body = body.replace(
            "    int sum = 0;\n    for (int pass = 0; pass < 4; ++pass) {",
            "    int sum = 0;\n    const uint8_t owner = base->faction_id_current;\n"
            "    for (int pass = 0; pass < 4; ++pass) {",
            1,
        )
        body = body.replace("        const uint8_t owner = BaseCurrent->faction_id_current;\n", "", 1)
        old_owner_bodies.append(body)
    old = _run(tmp_path, compiler, old_owner_bodies)
    assert old.returncode != 0, "captured-owner control unexpectedly passed"
