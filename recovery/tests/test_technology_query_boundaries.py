"""Host control for the technology query owner-boundary task.

This deliberately proves only source ownership and has_tech behavior. It is
not a VC6 ABI, instruction-equivalence, or full application test.
"""

from pathlib import Path
import re
import shutil
import subprocess

import pytest
from match_recovery.source import read_file


ROOT = Path(__file__).resolve().parents[2]


def braced(text, start):
    opening = text.index("{", start)
    depth = 0
    for end in range(opening, len(text)):
        depth += (text[end] == "{") - (text[end] == "}")
        if depth == 0:
            return text[start:end + 1]
    raise AssertionError("unterminated source body")


def strip_comments(text):
    return re.sub(r"/\*.*?\*/|//[^\n]*", "", text, flags=re.S)


def has_tech_body():
    source = (ROOT / "src/technology_queries.cpp").read_text()
    signature = "BOOL __cdecl has_tech(int tech_id, int faction_id) {"
    assert source.count(signature) == 1
    return braced(source, source.index(signature))


def test_query_mappings_and_wrapper_calls_have_the_canonical_owner():
    query = (ROOT / "src/technology_queries.cpp").read_text()
    header = (ROOT / "src/technology.h").read_text()
    expected = {
        0x005B9F20: "has_tech",
        0x005BA0E0: "facility_avail",
        0x005BA910: "veh_avail",
        0x005BAB40: "terrain_avail",
        0x005BAC20: "tech_avail",
        0x005BAE60: "tech_effects",
        0x005BCBE0: "tech_val",
    }
    query_records = read_file(ROOT / "src/technology_queries.cpp")
    assert {record.address for record in query_records} >= set(expected)
    old_owners = {
        "base.cpp": {0x005BA0E0},
        "veh.cpp": {0x005BA910},
        "terraforming.cpp": {0x005BAB40},
        "technology.cpp": {0x005BAC20, 0x005BAE60, 0x005BCBE0},
    }
    for filename, addresses in old_owners.items():
        assert not addresses.intersection(
            record.address for record in read_file(ROOT / "src" / filename)
        )
    for address, name in expected.items():
        assert sum(record.address == address for record in query_records) == 1
        assert query.count(f"__cdecl {name}(") == 1
    has_tech_record = next(record for record in query_records if record.address == 0x005B9F20)
    assert has_tech_record.body == "src/technology_queries.cpp"
    assert "BOOL __cdecl has_tech(int tech_id, int faction_id);" in header
    assert not re.search(r"\b(?:MEASURED\s+)?inline\s+BOOL\s+__cdecl\s+has_tech\s*\([^;]*\)\s*\{", header)
    assert "has_tech_call" not in strip_comments(header)
    sources = [
        "technology.cpp", "base.cpp", "terraforming.cpp", "veh.cpp",
        "console.cpp", "enemy.cpp", "faction.cpp", "map.cpp", "probe.cpp",
        "reportif.cpp",
    ]
    for filename in sources:
        code = strip_comments((ROOT / "src" / filename).read_text())
        assert not re.search(r"\bhas_tech_call\s*\(", code), filename

    # The separate tech_name header body is not part of this ownership move.
    tech_records = read_file(ROOT / "src/technology.cpp")
    tech_name = [record for record in tech_records if record.address == 0x005B9EF0]
    assert len(tech_name) == 1 and tech_name[0].body == "src/technology.h"


@pytest.mark.parametrize("compiler_name", ["g++", "clang++"])
def test_actual_has_tech_body_preserves_guard_and_bit_behavior(tmp_path, compiler_name):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(f"{compiler_name} unavailable")

    body = has_tech_body()
    fixture = r"""
#include <cassert>
#define __cdecl
typedef int BOOL;
const int TechNone = -1;
const int MaxTechnologyNum = 90;
struct TechnologyEntry { int preq_tech_1, preq_tech_2; };
TechnologyEntry Technology[MaxTechnologyNum];
struct GameDataFields { int game_tech_achieved_[MaxTechnologyNum]; };
struct GameDataState { GameDataFields fields_; } GameDataStateStorage;
BODY
int main() {
    for (int i = 0; i < MaxTechnologyNum; ++i) {
        Technology[i].preq_tech_1 = TechNone;
        Technology[i].preq_tech_2 = TechNone;
        GameDataStateStorage.fields_.game_tech_achieved_[i] = 0;
    }
    // The nonpositive-faction guard precedes even the TechNone fast path.
    assert(!has_tech(TechNone, 0));
    assert(!has_tech(TechNone, -1));
    assert(has_tech(TechNone, 1));
    assert(!has_tech(-2, 1));
    assert(!has_tech(MaxTechnologyNum - 1, 1));

    Technology[4].preq_tech_1 = -2;
    assert(!has_tech(4, 1));
    Technology[4].preq_tech_1 = 0;
    Technology[4].preq_tech_2 = -2;
    assert(!has_tech(4, 1));

    Technology[4].preq_tech_1 = TechNone;
    Technology[4].preq_tech_2 = TechNone;
    GameDataStateStorage.fields_.game_tech_achieved_[4] = 1 << 2;
    assert(has_tech(4, 2));
    assert(!has_tech(4, 1));

    // Preserve the original exception: a TechNone first prerequisite admits
    // the record even when its second prerequisite is disabled.
    Technology[5].preq_tech_1 = TechNone;
    Technology[5].preq_tech_2 = -2;
    GameDataStateStorage.fields_.game_tech_achieved_[5] = 1 << 2;
    assert(has_tech(5, 2));
}
""".replace("BODY", body)
    cpp = tmp_path / "technology_query.cpp"
    exe = tmp_path / "technology_query"
    cpp.write_text(fixture)
    built = subprocess.run(
        [compiler, "-std=c++11", "-Wall", "-Wextra", "-Werror", str(cpp), "-o", str(exe)],
        capture_output=True, text=True, timeout=30,
    )
    assert built.returncode == 0, built.stderr
    ran = subprocess.run([str(exe)], capture_output=True, text=True, timeout=10)
    assert ran.returncode == 0, ran.stderr
