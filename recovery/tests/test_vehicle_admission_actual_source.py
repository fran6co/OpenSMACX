"""Actual vehicle admission predicates; host controls are not VC6/game proof."""
from pathlib import Path
import shutil
import subprocess

import pytest

from match_recovery.source import read_file

ROOT = Path(__file__).resolve().parents[2]


def declaration(path, signature):
    source = path.read_text()
    start = source.index(signature)
    opening = source.index("{", start)
    depth = 1
    end = opening + 1
    while depth:
        depth += (source[end] == "{") - (source[end] == "}")
        end += 1
    return source[start:end]


def actual(address):
    rows = [row for unit in ("veh.cpp", "vehicle_orders.cpp")
            for row in read_file(ROOT / "src" / unit)
            if row.address == address]
    assert len(rows) == 1 and not rows[0].extract_error
    return rows[0].region


HARNESS = r"""
#include <stdint.h>
#include <assert.h>
#include <string.h>
#define __cdecl
#define MEASURED
typedef int BOOL;
SOURCE_TYPES
static_assert(sizeof(Veh) == 0x34, "actual vehicle fields");
static_assert(sizeof(Map) == 0x2c, "actual tile fields");
Veh Vehs[2]; Map tiles[16];
int MapLongitude = 2, MapLongitudeBounds = 4, MapLatitudeBounds = 4;
struct ConsoleFixture { int faction_id_; } ConsoleGlobal;
int jail_result, jail_calls, ability_calls, ability_first, ability_second;
int ability_ids[2], ability_masks[2], map_calls;
int veh_jail(int) { ++jail_calls; return jail_result; }
int has_abil(int prototype_id, int mask) {
    assert(ability_calls < 2);
    ability_ids[ability_calls] = prototype_id;
    ability_masks[ability_calls++] = mask;
    return mask == 0x40 ? ability_first : ability_second;
}
Map *map_loc(int x, int y) {
    ++map_calls;
    return tiles + MapLongitude * y + (x >> 1);
}
ON_MAP
SELECTABLE
CAN_LEAVE
void reset() {
    memset(Vehs, 0, sizeof(Vehs)); memset(tiles, 0, sizeof(tiles));
    Vehs[0].x = 2; Vehs[0].y = 2; Vehs[0].proto_id = 10;
    Vehs[0].faction_id = 2; ConsoleGlobal.faction_id_ = 2;
    jail_result = jail_calls = ability_calls = ability_first = ability_second = map_calls = 0;
}
int main() {
    reset(); assert(veh_selectable(0) == 1 && jail_calls == 1);
    reset(); Vehs[0].flags = 0x40; assert(veh_selectable(0) == 1);
    reset(); Vehs[0].flags = 0x400; assert(veh_selectable(0) == 1);
    reset(); Vehs[0].flags = 0x440; assert(veh_selectable(0) == 0 && jail_calls == 0);
    reset(); Vehs[0].flags = 0xffff; assert(veh_selectable(0) == 0);
    reset(); assert(veh_selectable(-1) == 0 && jail_calls == 0);
    for (int axis = 0; axis < 2; ++axis) {
        for (int edge = 0; edge < 2; ++edge) {
            reset();
            const int coordinate = edge ? 4 : -1;
            if (axis) Vehs[0].y = coordinate; else Vehs[0].x = coordinate;
            assert(veh_selectable(0) == 0 && jail_calls == 0);
        }
    }
    reset(); Vehs[0].faction_id = 3; assert(veh_selectable(0) == 0 && jail_calls == 0);
    reset(); ConsoleGlobal.faction_id_ = 0x102; assert(veh_selectable(0) == 0 && jail_calls == 0);
    reset(); jail_result = -1; assert(veh_selectable(0) == 0 && jail_calls == 1);

    reset(); assert(veh_can_leave_stack(0) == 0 && ability_calls == 2);
    assert(ability_masks[0] == 0x40 && ability_masks[1] == 4);
    assert(ability_ids[0] == 10 && ability_ids[1] == 10);
    reset(); ability_first = 1; assert(veh_can_leave_stack(0) == 1 && ability_calls == 1);
    reset(); ability_second = 1; assert(veh_can_leave_stack(0) == 1 && ability_calls == 2);
    reset(); Vehs[0].flags = 0x400;
    assert(veh_can_leave_stack(0) == 1 && ability_calls == 2 && map_calls == 0);
    const unsigned char climates[] = {0, 63, 64, 127, 255};
    const int expected[] = {0, 0, 2, 2, 2};
    for (int index = 0; index < 5; ++index) {
        reset(); tiles[5].bit = 0x20; tiles[5].climate = climates[index];
        assert(veh_can_leave_stack(0) == expected[index]);
    }
    reset(); tiles[5].climate = 255; assert(veh_can_leave_stack(0) == 0);
    reset(); tiles[5].bit = 0x20; tiles[5].climate = 255; Vehs[0].proto_id = 0x13;
    assert(veh_can_leave_stack(0) == 0);
    return 0;
}
"""


@pytest.mark.parametrize("compiler_name", ["g++", "clang++"])
def test_actual_admission_predicates_and_compiling_damage_controls(tmp_path, compiler_name):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(f"{compiler_name} unavailable")
    types = declaration(ROOT / "src/veh.h", "struct Veh {") + ";\n"
    types += declaration(ROOT / "src/map.h", "struct Map {") + ";"
    on_map = declaration(ROOT / "src/map.h", "MEASURED inline BOOL __cdecl on_map(")
    selectable = actual(0x5C1A20)
    leave = actual(0x5B61A0)
    variants = [(selectable, leave, True)]
    damages = [
        ("selectable", "0x400", "0x200"),
        ("selectable", "Vehs[veh_id].faction_id != ConsoleGlobal.faction_id_",
         "Vehs[veh_id].faction_id == ConsoleGlobal.faction_id_"),
        ("leave", "tile->bit & 0x20u", "tile->bit & 0x10u"),
        ("leave", "(tile->climate & ~0x1F) >= 0x40", "(tile->climate & ~0x1F) > 0x40"),
    ]
    for target, before, after in damages:
        body = selectable if target == "selectable" else leave
        assert body.count(before) == 1
        changed = body.replace(before, after, 1)
        variants.append((changed if target == "selectable" else selectable,
                         changed if target == "leave" else leave, False))
    for index, (select_body, leave_body, should_pass) in enumerate(variants):
        program = HARNESS.replace("SOURCE_TYPES", types).replace("ON_MAP", on_map)
        program = program.replace("SELECTABLE", select_body).replace("CAN_LEAVE", leave_body)
        source = tmp_path / f"admission_{index}.cpp"
        executable = tmp_path / f"admission_{index}"
        source.write_text(program)
        built = subprocess.run([compiler, "-std=c++11", "-O2", "-Wall", "-Wextra",
                                "-Werror", "-fsanitize=undefined", "-fno-sanitize-recover=all",
                                str(source), "-o", str(executable)],
                               capture_output=True, text=True, timeout=30)
        assert built.returncode == 0, built.stderr
        result = subprocess.run([str(executable)], capture_output=True, timeout=10)
        assert (result.returncode == 0) == should_pass, result.stderr
        if not should_pass:
            assert b"assertion" in result.stderr.lower(), result.stderr
