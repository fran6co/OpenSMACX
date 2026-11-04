"""Actual canonical find_landmark controls, with fixed pinned search-order cases.

Host controls do not prove same-TU kill_landmark inlining, native ABI or linked
retention. Invalid radius indices stay unsafe and are not executed or clamped.
"""
from pathlib import Path
import re
import shutil
import subprocess
import pytest
from match_recovery.includes import COMMENTS_OR_STRINGS
ROOT = Path(__file__).resolve().parents[2]


def _masked(source):
    return COMMENTS_OR_STRINGS.sub(lambda m: re.sub(r"[^\n]", " ", m[0]), source)


def _definition(source, signature):
    masked = _masked(source)
    matches = list(re.finditer(signature, masked))
    assert len(matches) == 1
    start = matches[0].start()
    opening = masked.index("{", start)
    depth = 0
    for pos in range(opening, len(masked)):
        depth += (masked[pos] == "{") - (masked[pos] == "}")
        if depth == 0:
            return source[start:pos + 1]
    raise AssertionError("unterminated definition")


def test_provider_contract():
    header = _masked((ROOT / "src/map.h").read_text())
    matches = list(re.finditer(r"\bfind_landmark\s*\([^;{}]*\)\s*;", header))
    assert len(matches) == 1
    assert not re.search(r"\bfind_landmark\s*\([^;{}]*\)\s*\{", header)
    statement = header[header.rfind(";", 0, matches[0].start()) + 1:matches[0].end()]
    assert not re.search(r"\b(?:inline|MEASURED|__declspec)\b", statement)
    _definition((ROOT / "src/map_grid.cpp").read_text(), r"\bint\s+__cdecl\s+find_landmark\s*\(")


PREFIX = r'''#include <cassert>
#define __cdecl
using BOOL=int;
int MapLongitudeBounds=10,MapLatitudeBounds=10,MapIsFlat;
struct Landmark { int x,y; char name[32]; };
Landmark MapLandmark[4]={};
int MapLandmarkCount;
'''
MAIN = r'''
void reset() { MapLandmarkCount=0;MapIsFlat=1; }
void add(int x,int y) { MapLandmark[MapLandmarkCount++]={x,y,{}}; }
int main() {
    reset();assert(find_landmark(4,4,1)==-1);
    // RadiusRange[0]=1, [1]=9, [2]=25; ninth position included, tenth excluded.
    add(4,2);assert(find_landmark(4,4,0)==-1);
    assert(find_landmark(4,4,1)==0);
    reset();add(6,2);assert(find_landmark(4,4,1)==-1);
    assert(find_landmark(4,4,2)==0);
    // Neighbor order dominates landmark index; duplicate coordinates use first.
    reset();add(6,4);add(5,3);assert(find_landmark(4,4,1)==1);
    reset();add(4,4);add(4,4);assert(find_landmark(4,4,0)==0);
    reset();add(1,-1);assert(find_landmark(0,0,1)==-1);
    reset();add(8,0);MapIsFlat=0;assert(find_landmark(0,0,1)==0);
    MapIsFlat=2;assert(find_landmark(0,0,1)==0);
    MapIsFlat=1;assert(find_landmark(0,0,1)==-1);
    reset();add(0,4);MapIsFlat=0;assert(find_landmark(10,4,0)==0);
    // Single wrap is not a clamp or repeated modulo for invalid coordinates.
    reset();add(0,4);MapIsFlat=0;assert(find_landmark(-12,4,0)==-1);
    reset();add(4,0);assert(find_landmark(4,-1,0)==-1);
}
'''


@pytest.mark.parametrize("damage", [None, "radius_index", "omit_wrap", "last_index"])
def test_actual_cases(tmp_path, damage):
    compiler = shutil.which("g++") or shutil.which("clang++")
    if not compiler:
        pytest.skip("host C++ compiler unavailable")
    header = (ROOT / "src/map.h").read_text()
    source = (ROOT / "src/map.cpp").read_text()
    parts = []
    for name in ("RadiusRange", "RadiusOffsetX", "RadiusOffsetY"):
        matches = list(re.finditer(rf"\bconst\s+int\s+{name}\[\d+\]\s*=\s*\{{[^}}]*\}}\s*;", _masked(source)))
        assert len(matches) == 1
        parts.append(source[matches[0].start():matches[0].end()])
    parts.append(_definition(header, r"\bBOOL\s+__cdecl\s+on_map\s*\("))
    parts.append(_definition(header, r"\binline\s+int\s+__cdecl\s+wrap_map_x\s*\("))
    body = _definition((ROOT / "src/map_grid.cpp").read_text(),
                       r"\bint\s+__cdecl\s+find_landmark\s*\(")
    changes = {
        "radius_index": ("RadiusRange[radius_range_offset]", "radius_range_offset"),
        "omit_wrap": ("wrap_map_x(x + RadiusOffsetX[i])", "(x + RadiusOffsetX[i])"),
        "last_index": ("return lm;", "return MapLandmarkCount - 1;"),
    }
    if damage:
        old, new = changes[damage]
        assert body.count(old) == 1
        changed = body.replace(old, new)
        assert changed != body
        body = changed
    unit = tmp_path / "landmark.cpp"
    executable = tmp_path / "landmark"
    unit.write_text(PREFIX + "\n".join(parts) + body + MAIN)
    compiled = subprocess.run([compiler, "-std=c++11", "-O0", str(unit), "-o", str(executable)], text=True, capture_output=True)
    assert compiled.returncode == 0, compiled.stderr
    result = subprocess.run([str(executable)], text=True, capture_output=True)
    if damage is None:
        assert result.returncode == 0, result.stderr
    else:
        assert result.returncode != 0, "compile-success damage escaped fixed cases"


KILL_PREFIX = r'''#include <cassert>
#include <cstring>
#define __cdecl
using BOOL=int;
struct Landmark { int x; int y; char name[32]; };
static_assert(sizeof(Landmark)==40,"production Landmark stride");
static int MapLongitudeBounds=10,MapLatitudeBounds=10,MapIsFlat=1;
static int MapLandmarkCount;
MAX_LANDMARK_DECL
static Landmark MapLandmark[MaxLandmarkNum];
RADIUS_TABLES
ON_MAP
XRANGE
'''
KILL_MAIN = r'''
static void reset_landmarks() {
    MapLandmarkCount=0; MapIsFlat=1;
    std::memset(MapLandmark,0,sizeof(MapLandmark));
}
static void add_landmark(int x,int y,char fill) {
    Landmark &lm=MapLandmark[MapLandmarkCount++];
    lm.x=x; lm.y=y; std::memset(lm.name,fill,sizeof(lm.name));
}
int main() {
    // A miss leaves both count and every record byte intact.
    reset_landmarks(); add_landmark(8,8,'a'); add_landmark(9,8,'b');
    Landmark before[2]; std::memcpy(before,MapLandmark,sizeof(before));
    kill_landmark(4,4);
    assert(MapLandmarkCount==2 && !std::memcmp(before,MapLandmark,sizeof(before)));

    // Offset 1 (5,3) precedes offset 2 (6,4), even when its record occurs later.
    // The first matching record at that winning neighbor is removed; the tail
    // shifts as complete 40-byte records, including each name byte.
    reset_landmarks();
    add_landmark(6,4,'x'); add_landmark(5,3,'y'); add_landmark(5,3,'z');
    Landmark expected[3]; std::memcpy(expected,MapLandmark,sizeof(expected));
    kill_landmark(4,4);
    assert(MapLandmarkCount==2);
    assert(!std::memcmp(&MapLandmark[0],&expected[0],sizeof(Landmark)));
    assert(!std::memcmp(&MapLandmark[1],&expected[2],sizeof(Landmark)));
    assert(MapLandmark[1].name[0]=='z' && MapLandmark[1].name[31]=='z');

    // Duplicate coordinates choose the first index and shift the later record.
    reset_landmarks();
    add_landmark(5,3,'m'); add_landmark(5,3,'n'); add_landmark(8,8,'o');
    Landmark first_index_tail[2];
    std::memcpy(first_index_tail,&MapLandmark[1],sizeof(first_index_tail));
    kill_landmark(4,4);
    assert(MapLandmarkCount==2);
    assert(!std::memcmp(MapLandmark,first_index_tail,sizeof(first_index_tail)));

    // Flat maps wrap x exactly once; y remains bounded by on_map.
    reset_landmarks(); MapIsFlat=0; add_landmark(0,4,'w');
    kill_landmark(10,4); assert(MapLandmarkCount==0);
    reset_landmarks(); add_landmark(8,4,'q'); MapIsFlat=0;
    kill_landmark(-12,4); assert(MapLandmarkCount==1);
    kill_landmark(4,-20); assert(MapLandmarkCount==1);
}
'''


def _kill_program(map_header, map_source, kill_body):
    landmark = _definition(map_header, r"struct\s+Landmark\s*")
    landmark += ";"
    # Keep the array extent tied to the production declaration, rather than a
    # test-invented capacity.
    assert "landmarks_[MaxLandmarkNum]" in map_header
    max_count = re.search(r"static\s+const\s+int\s+MaxLandmarkNum\s*=\s*(\d+)\s*;",
                          _masked(map_header))
    assert max_count and max_count.group(1) == "64"
    arrays = []
    masked_source = _masked(map_source)
    for name in ("RadiusRange", "RadiusOffsetX", "RadiusOffsetY"):
        table_source = (ROOT / "src/map.cpp").read_text()
        masked_source = _masked(table_source)
        match = re.search(rf"\bconst\s+int\s+{name}\[\d+\]\s*=\s*\{{[^}}]*\}}\s*;",
                          masked_source)
        assert match
        arrays.append(table_source[match.start():match.end()])
    on_map = _definition(map_header, r"\bBOOL\s+__cdecl\s+on_map\s*\(")
    xrange = _definition(map_header, r"\binline\s+int\s+__cdecl\s+wrap_map_x\s*\(")
    assert "find_landmark(" not in kill_body
    assert "RadiusRange[1]" in kill_body
    return (KILL_PREFIX.replace("struct Landmark { int x; int y; char name[32]; };",
                                landmark)
            .replace("MAX_LANDMARK_DECL",
                     "static const int MaxLandmarkNum = " + max_count.group(1) + ";")
            .replace("RADIUS_TABLES", "\n".join(arrays))
            .replace("ON_MAP", on_map)
            .replace("XRANGE", xrange)
            + kill_body + KILL_MAIN)


@pytest.mark.parametrize("damage", [None, "reverse_neighbor_order", "omit_name_shift"])
def test_actual_kill_landmark_search_and_full_record_removal(tmp_path, damage):
    compilers = [shutil.which(name) for name in ("g++", "clang++")]
    compilers = [compiler for compiler in compilers if compiler]
    assert compilers
    header = (ROOT / "src/map.h").read_text()
    source = (ROOT / "src/map_grid.cpp").read_text()
    body = _definition(source, r"\bvoid\s+__cdecl\s+kill_landmark\s*\(")
    if damage == "reverse_neighbor_order":
        old = "for (int i = 0; i < radius; i++) {"
        assert body.count(old) == 1
        body = body.replace(old, "for (int i = radius - 1; i >= 0; i--) {", 1)
    elif damage == "omit_name_shift":
        old = "MapLandmark[i] = MapLandmark[i + 1];"
        assert body.count(old) == 1
        body = body.replace(old,
                            "MapLandmark[i].x = MapLandmark[i + 1].x;\n"
                            "            MapLandmark[i].y = MapLandmark[i + 1].y;", 1)
    program = _kill_program(header, source, body)
    for compiler in compilers:
        unit = tmp_path / ("kill_" + Path(compiler).name + ".cpp")
        executable = unit.with_suffix("")
        unit.write_text(program)
        compiled = subprocess.run(
            [compiler, "-std=c++11", "-Wall", "-Wextra", "-Werror",
             str(unit), "-o", str(executable)], text=True, capture_output=True,
            timeout=30)
        assert compiled.returncode == 0, compiled.stderr
        result = subprocess.run([str(executable)], text=True, capture_output=True,
                                timeout=10)
        if damage is None:
            assert result.returncode == 0, result.stderr
        else:
            assert result.returncode != 0, "compile-success damage escaped kill cases"
