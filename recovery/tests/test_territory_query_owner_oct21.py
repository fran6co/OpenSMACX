"""Source-extracted controls for the original territory ownership query."""
from pathlib import Path
import shutil
import subprocess

import pytest
from match_recovery.source import read_file


ROOT = next(path for path in Path(__file__).resolve().parents
            if (path / "src/IMAGE_PIN").is_file())
ADDRESS = 0x004E3EF0


def territory_definition():
    records = read_file(ROOT / "src/territory.cpp")
    record = next(row for row in records if row.address == ADDRESS)
    signature = "int __cdecl whose_territory("
    start = record.region.index(signature)
    opening = record.region.index("{", start)
    depth = 0
    for position in range(opening, len(record.region)):
        if record.region[position] == "{":
            depth += 1
        elif record.region[position] == "}":
            depth -= 1
            if depth == 0:
                definition = record.region[start:position + 1]
                assert "BODY GOES HERE." not in definition
                return definition
    raise AssertionError("unterminated whose_territory definition")


HARNESS = r"""
#include <assert.h>
#include <stdint.h>

#define __cdecl
typedef int BOOL;
static const uint32_t STATE_OMNISCIENT_VIEW = 0x80;
static const int DTREATY_COMMLINK = 0x8;
static const int DTREATY_UNK_8000000 = 0x08000000;

struct Map { signed char territory; };
static Map tile;
static int event_log[8], event_count;
static int treaty_calls, region_calls, base_calls;
static int treaty_faction, treaty_owner;
static int region_x, region_y, base_x, base_y, base_region;
struct GameFields { uint32_t game_state_; };
struct GameState { GameFields fields_; };
static GameState GameDataStateStorage;

static void event(int value) { event_log[event_count++] = value; }
static Map *map_loc(int x, int y) {
    (void)x; (void)y;
    event(1);
    return &tile;
}
// The product now reads the actual treaty member directly. This conversion
// records only that field read and its indices; the product applies its mask.
struct TreatyRead {
    uint32_t value;
    int faction, owner;
    operator uint32_t() const {
        event(2);
        ++treaty_calls;
        treaty_faction = faction;
        treaty_owner = owner;
        return value;
    }
};
struct PlayerData { TreatyRead diplo_treaties[8]; };
static PlayerData PlayersData[8];
static int region_at(int x, int y) {
    event(3);
    ++region_calls;
    region_x = x;
    region_y = y;
    return 23;
}
static int base_find(int x, int y, int a, int region, int b, int c) {
    event(4);
    ++base_calls;
    base_x = x; base_y = y; base_region = region;
    assert(a == -1 && b == -1 && c == -1);
    return 71;
}

WHOSE_TERRITORY_DEFINITION

static void reset(int owner, int treaties, uint32_t state) {
    tile.territory = (signed char)owner;
    for (int faction = 0; faction < 8; ++faction) {
        for (int other = 0; other < 8; ++other) {
            TreatyRead &field = PlayersData[faction].diplo_treaties[other];
            field.value = 0;
            field.faction = faction;
            field.owner = other;
        }
    }
    if (owner >= 0 && owner < 8) {
        PlayersData[2].diplo_treaties[owner].value = (uint32_t)treaties;
    }
    GameDataStateStorage.fields_.game_state_ = state;
    event_count = treaty_calls = region_calls = base_calls = 0;
    treaty_faction = treaty_owner = -1;
    region_x = region_y = base_x = base_y = base_region = -1;
}

int main() {
    const int full_mask = DTREATY_COMMLINK | DTREATY_UNK_8000000;
    int base_id;

    reset(0, full_mask, 0);
    base_id = 55;
    assert(whose_territory(2, 7, 9, &base_id, 0) == -1);
    assert(base_id == 55 && event_count == 1 && event_log[0] == 1);
    assert(treaty_calls == 0 && region_calls == 0 && base_calls == 0);

    reset(-1, full_mask, 0);
    base_id = 56;
    assert(whose_territory(2, 7, 9, &base_id, 0) == -1);
    assert(base_id == 56 && event_count == 1 && treaty_calls == 0);

    reset(2, 0, 0);
    base_id = 57;
    assert(whose_territory(2, 7, 9, &base_id, 0) == 2);
    assert(base_id == 57 && event_count == 1 && treaty_calls == 0);

    reset(3, DTREATY_COMMLINK, 0);
    base_id = 58;
    assert(whose_territory(2, 7, 9, &base_id, 0) == -1);
    assert(base_id == 58 && treaty_calls == 1 && region_calls == 0 && base_calls == 0);
    assert(event_count == 2 && event_log[0] == 1 && event_log[1] == 2);
    assert(treaty_faction == 2 && treaty_owner == 3);

    // Pinned AND/CMP 0x08000008 requires both bits, ignores all others.
    reset(3, DTREATY_UNK_8000000, 0);
    assert(whose_territory(2, 7, 9, 0, 0) == -1 && treaty_calls == 1);
    reset(3, 0x100, 0);
    assert(whose_territory(2, 7, 9, 0, 0) == -1 && treaty_calls == 1);
    reset(3, full_mask | 0x100, 0);
    assert(whose_territory(2, 7, 9, 0, 0) == 3 && treaty_calls == 1);

    reset(3, 0, 0);
    base_id = 59;
    assert(whose_territory(2, 7, 9, &base_id, 1) == 3);
    assert(base_id == 71 && treaty_calls == 0);
    assert(region_calls == 1 && base_calls == 1);
    assert(event_count == 3 && event_log[0] == 1 && event_log[1] == 3
           && event_log[2] == 4);
    assert(region_x == 7 && region_y == 9 && base_x == 7 && base_y == 9
           && base_region == 23);

    reset(4, 0, STATE_OMNISCIENT_VIEW);
    base_id = 60;
    assert(whose_territory(2, 11, 13, &base_id, 0) == 4);
    assert(base_id == 71 && treaty_calls == 0 && region_calls == 1 && base_calls == 1);

    reset(5, full_mask, 0);
    base_id = 61;
    assert(whose_territory(2, 17, 19, &base_id, 0) == 5);
    assert(base_id == 71 && treaty_calls == 1 && region_calls == 1 && base_calls == 1);
    assert(event_count == 4 && event_log[0] == 1 && event_log[1] == 2
           && event_log[2] == 3 && event_log[3] == 4);

    reset(6, full_mask, 0);
    assert(whose_territory(2, 21, 22, 0, 0) == 6);
    assert(treaty_calls == 1 && region_calls == 0 && base_calls == 0);

    return 0;
}
"""


def compile_and_run(tmp_path, compiler, definition, label):
    program = HARNESS.replace("WHOSE_TERRITORY_DEFINITION", definition)
    source_path = tmp_path / (label + ".cpp")
    binary_path = tmp_path / label
    source_path.write_text(program, encoding="utf-8")
    built = subprocess.run(
        [compiler, "-std=c++98", "-Wall", "-Wextra", "-Werror",
         "-Wno-sign-compare", "-Wno-unused-parameter",
         str(source_path), "-o", str(binary_path)],
        capture_output=True, text=True,
    )
    assert built.returncode == 0, built.stderr
    return subprocess.run([str(binary_path)], capture_output=True, timeout=10)


@pytest.mark.parametrize("compiler_name", ["g++", "clang++"])
def test_actual_territory_query_paths_and_meaningful_mutants(tmp_path, compiler_name):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(f"host C++ compiler unavailable: {compiler_name}")
    definition = territory_definition()
    result = compile_and_run(tmp_path, compiler, definition, "territory_actual")
    assert result.returncode == 0, result.stderr.decode(errors="replace")

    mutants = (
        ("zero_owner", "if (owner <= 0)", "if (owner < 0)"),
        ("ignore_comm", "!ignore_comm &&", "true &&"),
        ("treaty_mask", "!= (DTREATY_COMMLINK | DTREATY_UNK_8000000)",
         "!= DTREATY_COMMLINK"),
        ("omniscience", "!(GameDataStateStorage.fields_.game_state_ & STATE_OMNISCIENT_VIEW)",
         "true"),
        ("base_output", "if (base_id)", "if (base_id || true)"),
    )
    for label, before, after in mutants:
        assert definition.count(before) == 1, (label, before)
        mutated = definition.replace(before, after, 1)
        assert mutated != definition
        result = compile_and_run(tmp_path, compiler, mutated, "territory_" + label)
        assert result.returncode != 0, label
