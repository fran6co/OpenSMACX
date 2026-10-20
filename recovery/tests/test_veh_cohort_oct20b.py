"""Host behavior controls for the actual body; not ABI or native-layout proof."""
from pathlib import Path
import shutil
import subprocess

import pytest
from test_netdaemon_synch_dispatch import _definition


ROOT = Path(__file__).resolve().parents[2]


HARNESS = r"""
#include <cassert>
#include <cstddef>
#include <stdint.h>

#define __cdecl
struct Map {
    uint8_t climate, contour, val2, region, visibility, val3, unk_1;
    int8_t territory;
    uint32_t bit, bit2, bit_visible[7];
};
typedef char map_visibility_offset[(offsetof(Map, visibility) == 4) ? 1 : -1];
typedef char map_bit_offset[(offsetof(Map, bit) == 8) ? 1 : -1];
struct Veh { int proto_id; uint32_t flags; int x, y; };
Veh Vehs[1];
Map tiles[32];
uint32_t MapLongitude = 4;
int ability_answers[2], ability_calls, ability_args[2], map_calls, bad_map_location;
Map *selected_tile;
Map *map_loc(int x, int y) {
    ++map_calls;
    if (x != 2 || y != 2) bad_map_location = 1;
    int index = (int)(MapLongitude * y + (x >> 1));
    if (index < 0 || index >= (int)(sizeof tiles / sizeof tiles[0])) {
        bad_map_location = 1;
        return &tiles[0];
    }
    selected_tile = &tiles[index];
    if (selected_tile != &tiles[9]) bad_map_location = 1;
    return selected_tile;
}
int has_abil(int, int ability) {
    if (ability_calls < 2) ability_args[ability_calls] = ability;
    int answer = ability_calls < 2 ? ability_answers[ability_calls] : 0;
    ++ability_calls;
    return answer;
}

BODY

static void reset() {
    for (int i = 0; i < 32; ++i) tiles[i] = Map();
    Vehs[0].proto_id = 1;
    Vehs[0].flags = 0;
    Vehs[0].x = 2;
    Vehs[0].y = 2;
    ability_answers[0] = ability_answers[1] = 0;
    ability_calls = map_calls = bad_map_location = 0;
    ability_args[0] = ability_args[1] = -1;
    selected_tile = 0;
}

int main() {
    int failures = 0;
    Map &tile = tiles[9];
    reset();
    tile.climate = 0x40;
    tile.visibility = 0x20;
    tile.bit = 0;
    if (veh_can_leave_stack(0) != 0) failures |= 1; // visibility-only contrast

    reset();
    tile.visibility = 0;
    tile.bit = 0x20;
    tile.climate = 0x40;
    if (veh_can_leave_stack(0) != 2) failures |= 2; // bit-only contrast
    if (bad_map_location || selected_tile != &tiles[9]) failures |= 128;

    reset();
    tile.bit = 0x20;
    tile.climate = 0x20;
    if (veh_can_leave_stack(0) != 0) failures |= 4; // altitude threshold unchanged

    reset();
    tile.bit = 0x20;
    tile.climate = 0x40;
    Vehs[0].proto_id = 0x13;
    if (veh_can_leave_stack(0) != 0) failures |= 8; // exceptional prototype unchanged

    reset();
    ability_answers[0] = 1;
    if (veh_can_leave_stack(0) != 1 || ability_calls != 1 || map_calls != 0
        || ability_args[0] != 0x40) failures |= 16; // first ability exits before map lookup

    reset();
    ability_answers[1] = 1;
    if (veh_can_leave_stack(0) != 1 || ability_calls != 2 || map_calls != 0
        || ability_args[0] != 0x40 || ability_args[1] != 4) failures |= 32;

    reset();
    Vehs[0].flags = 0x400;
    if (veh_can_leave_stack(0) != 1 || ability_calls != 2 || map_calls != 0
        || ability_args[0] != 0x40 || ability_args[1] != 4) failures |= 64;
    return failures;
}
"""


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
def test_pinned_vehicle_stack_exit_member_and_branch_controls(tmp_path, compiler_name):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(compiler_name + " is unavailable")
    source = (ROOT / "src/veh.cpp").read_text(encoding="utf-8")
    signature = r"int\s+__cdecl\s+veh_can_leave_stack"
    body = _definition(source, signature, "veh_can_leave_stack")
    assert body.count("tile->bit & 0x20u") == 1
    assert body.count("tile->visibility & 0x20") == 0

    for variant, expected in (("actual", body),
                              ("visibility_mutant", body.replace(
                                  "tile->bit & 0x20u", "tile->visibility & 0x20"))):
        assert expected.count("veh_can_leave_stack") == 1
        cpp = tmp_path / f"veh_can_leave_stack_{variant}.cpp"
        binary = tmp_path / f"veh_can_leave_stack_{variant}"
        cpp.write_text(HARNESS.replace("BODY", expected), encoding="utf-8")
        built = subprocess.run(
            [compiler, "-std=c++98", "-Wall", "-Wextra", "-Werror", str(cpp), "-o", str(binary)],
            capture_output=True,
            text=True,
            timeout=30,
        )
        assert built.returncode == 0, built.stderr
        ran = subprocess.run([str(binary)], capture_output=True, timeout=10)
        if variant == "actual":
            assert ran.returncode == 0, f"{compiler_name}: {ran.returncode}; {ran.stderr.decode(errors='replace')}"
        else:
            assert ran.returncode == 3, "visibility mutant must fail both independent field contrasts"
