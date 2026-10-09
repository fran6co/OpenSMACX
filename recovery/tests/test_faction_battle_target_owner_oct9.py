"""Host controls for the actual battle-plans selected-target helper."""

from __future__ import annotations

import shutil
import subprocess
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
FACTION_CPP = ROOT / "src/faction.cpp"


def _definition(source: str, start: str) -> str:
    at = source.find(start)
    assert at >= 0, f"missing actual source definition: {start}"
    opening = source.find("{", at)
    depth = 0
    for index in range(opening, len(source)):
        if source[index] == "{":
            depth += 1
        elif source[index] == "}":
            depth -= 1
            if depth == 0:
                return source[at:index + 1]
    raise AssertionError(f"unterminated definition: {start}")


FIXTURE = r"""
#include <assert.h>
#include <stdint.h>
#include <stdio.h>

struct Base { int x, y; };
struct Map { uint8_t climate, contour, val2, region; };
struct PlayerData { int x_target, y_target, base_id_atk_target, unk_71, unk_72; };
struct Veh { int16_t x, y; uint32_t state; uint16_t flags, proto_id, unk_1;
    uint8_t faction_id; };
struct GameDataFields { int vehicle_current_count_; };
struct GameDataState { GameDataFields fields_; };

static PlayerData PlayersData[2];
static Base Bases[4];
static Veh Vehs[4];
static Map MapTiles[64];
static GameDataState GameDataStateStorage;
static const int MapLongitude = 8;
static const uint32_t VSTATE_UNK_40000 = 0x40000;
static const uint32_t VSTATE_UNK_20000 = 0x20000;

static Map *map_loc(int x, int y) {
    return &MapTiles[(x >> 1) + y * MapLongitude];
}
"""


def test_source_order_and_actual_helper_host_controls(tmp_path: Path) -> None:
    source = FACTION_CPP.read_text(encoding="utf-8")
    helper = _definition(source, "static void battle_plans_apply_selected_target(")
    branch_start = source.index("const int selected_base_id = popup.exec(0, pop_wait);")
    branch_end = source.index("\n            }\n        }\n    }", branch_start)
    branch = source[branch_start:branch_end]
    assignment = "PlayersData[faction_b].base_id_atk_target ="
    popup = "x_pops_short(BattlePlansSelectionLabel"
    cleanup = "battle_plans_apply_selected_target(faction_b,"
    assert branch.index(assignment) < branch.index(popup) < branch.index(cleanup)
    assert "const Base &selected_base = Bases[selected_base_id];" in helper
    assert "planned_tile.region == selected_tile.region" in helper
    assert "planned_tile.climate == selected_tile.climate" not in helper

    source_text = FIXTURE + "\n" + helper + r"""
int main() {
    const int faction = 0;
    const int selected = 1;
    PlayersData[faction].base_id_atk_target = -1;
    PlayersData[faction].x_target = 2;
    PlayersData[faction].y_target = 1;
    PlayersData[faction].unk_71 = 11;
    PlayersData[faction].unk_72 = 12;
    Bases[selected].x = 2;
    Bases[selected].y = 1;
    Map *planned = map_loc(2, 1);
    Map *selected_tile = map_loc(2, 1);
    planned->climate = selected_tile->climate = 0x23;
    planned->region = 5;
    selected_tile->region = 5;

    // Native assignment is before the popup. The callback then changes every
    // value the cleanup phase must reread, including selected-base coordinates.
    PlayersData[faction].base_id_atk_target = selected;
    assert(PlayersData[faction].base_id_atk_target == selected);
    Bases[selected].x = 8;
    Bases[selected].y = 2;
    PlayersData[faction].x_target = 10;
    PlayersData[faction].y_target = 2;
    PlayersData[faction].unk_71 = 21;
    PlayersData[faction].unk_72 = 22;
    Map *callback_plan = map_loc(10, 2);
    Map *callback_base = map_loc(8, 2);
    callback_plan->climate = callback_base->climate = 0x23;
    callback_plan->region = 6;
    callback_base->region = 7;
    Vehs[0].faction_id = faction;
    Vehs[0].state = 0x40000 | 0x20000 | 1;
    Vehs[1].faction_id = 1;
    Vehs[1].state = 0x40000 | 0x20000 | 2;
    Vehs[2].faction_id = faction;
    Vehs[2].state = 0x20000 | 4;
    GameDataStateStorage.fields_.vehicle_current_count_ = 3;

    battle_plans_apply_selected_target(faction, selected);
    assert(Vehs[0].state == 1);
    assert(Vehs[1].state == (0x40000 | 0x20000 | 2));
    assert(Vehs[2].state == (0x20000 | 4));
    assert(PlayersData[faction].x_target == -1);
    assert(PlayersData[faction].y_target == -1);
    assert(PlayersData[faction].unk_71 == 0);
    assert(PlayersData[faction].unk_72 == 0);

    // A different climate alone does not trigger cleanup when region matches.
    PlayersData[faction].x_target = 2;
    PlayersData[faction].y_target = 1;
    PlayersData[faction].unk_71 = 31;
    PlayersData[faction].unk_72 = 32;
    Map *same_region_plan = map_loc(2, 1);
    Map *same_region_base = map_loc(Bases[selected].x, Bases[selected].y);
    same_region_plan->region = same_region_base->region = 9;
    same_region_plan->climate = 0x01;
    same_region_base->climate = 0xE7;
    Vehs[0].state = 0x40000 | 0x20000 | 8;
    battle_plans_apply_selected_target(faction, selected);
    assert(Vehs[0].state == (0x40000 | 0x20000 | 8));
    assert(PlayersData[faction].x_target == 2);
    assert(PlayersData[faction].y_target == 1);
    assert(PlayersData[faction].unk_71 == 31);
    assert(PlayersData[faction].unk_72 == 32);
    puts("battle target actual-source controls passed");
}
"""

    cpp = tmp_path / "battle_target_actual_source.cpp"
    cpp.write_text(source_text, encoding="utf-8")
    compilers = [name for name in ("g++", "clang++") if shutil.which(name)]
    assert compilers, "need a host C++ compiler for the extracted production helper"
    for compiler in compilers:
        exe = tmp_path / f"battle-target-{compiler.replace('+', 'p')}"
        built = subprocess.run(
            [compiler, "-std=c++98", "-Wall", "-Wextra", "-Werror",
             "-pedantic", str(cpp), "-o", str(exe)],
            text=True, capture_output=True, check=False,
        )
        assert built.returncode == 0, built.stdout + built.stderr
        ran = subprocess.run([str(exe)], text=True, capture_output=True, check=False)
        assert ran.returncode == 0, ran.stdout + ran.stderr
        assert "battle target actual-source controls passed" in ran.stdout
