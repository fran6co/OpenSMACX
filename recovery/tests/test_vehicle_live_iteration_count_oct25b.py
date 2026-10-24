"""Actual-source controls for sub_500380's live vehicle-count loop.

The fixture owns only bounded data/services. It exercises two initialized
vehicles and never indexes beyond the initialized rows. It is host behavior
evidence, not a native ABI or game-runtime claim.
"""

from __future__ import annotations

import os
from pathlib import Path
import shutil
import subprocess

import pytest


def _root() -> Path:
    configured = os.environ.get("OPENSMACX_SOURCE_ROOT")
    if configured:
        return Path(configured).resolve()
    for parent in Path(__file__).resolve().parents:
        if (parent / "src/veh.cpp").is_file():
            return parent
    raise AssertionError("could not locate the OpenSMACX source root")


ROOT = _root()
SOURCE = ROOT / "src/veh.cpp"
SIGNATURE = "void __cdecl sub_500380(int faction_id, int x, int y)"
ENTRY = (
    "int i = 0;\n"
    "    if (GameDataStateStorage.fields_.vehicle_current_count_ > 0) {\n"
    "        do {"
)
EXIT = (
    "} while (++i < GameDataStateStorage.fields_.vehicle_current_count_);\n"
    "    }\n"
    "}"
)


def _actual_body() -> str:
    from match_recovery.source import read_file
    from match_recovery.source.reader import function_line
    from match_recovery.local_variants import _bound_function_body_bytes, _marker_identity
    raw = SOURCE.read_bytes()
    text = raw.decode("utf-8")
    records = read_file(SOURCE)
    target, = [row for row in records if row.address == 0x00500380]
    name = "sub_500380"
    low, high = _bound_function_body_bytes(text, name, SOURCE, target,
                                         tuple(_marker_identity(row) for row in records))
    line = function_line(text, name, after_line=target.line)
    start = len("".join(text.splitlines(keepends=True)[:line - 1]).encode("utf-8"))
    return raw[start:high].decode("utf-8")


HARNESS = r'''#define __cdecl
#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <vector>

enum { BIT_BASE_IN_TILE = 1, BIT_AIRBASE = 0x40000,
       VSTATE_MADE_AIRDROP = 0x20 };
struct SecretProject { int space_elevator() const { return -1; } };
struct GameDataFields { SecretProject secret_project_; int vehicle_current_count_; };
struct GameData { GameDataFields fields_; };
GameData GameDataStateStorage;
struct RulesType { int tech_orb_insert_sans_spc_elev; int max_airdrop_sans_orb_insert; };
RulesType Rules;
struct Base { uint8_t faction_id_current; };
Base Bases[8];

struct Veh {
    int proto_id;
    uint8_t faction_id;
    uint8_t state;
    uint8_t moves_expended;
    int x, y;
};
Veh Vehs[2];
struct Map { uint32_t bit; uint32_t val2; };
Map Tiles[4];
int tile_calls;
Map *map_tiles() { ++tile_calls; return Tiles; }

int MapLongitudeBounds, MapLatitudeBounds, MapLongitude, MapIsFlat;
enum { NORMAL, SHRINK_COUNT_ON_FIRST_ABILITY, FIRST_ABILITY_FALSE };
int service_mode;
std::vector<int> ability_prototypes;

int has_tech(int, int) { return 1; }
int has_abil(int proto_id, int ability) {
    assert(ability == 0x10);
    ability_prototypes.push_back(proto_id);
    if (service_mode == SHRINK_COUNT_ON_FIRST_ABILITY &&
        ability_prototypes.size() == 1) {
        GameDataStateStorage.fields_.vehicle_current_count_ = 1;
    }
    if (service_mode == FIRST_ABILITY_FALSE && proto_id == 11) return 0;
    return 1;
}

ACTUAL_SUB_500380

static void reset(int count) {
    GameDataStateStorage.fields_.vehicle_current_count_ = count;
    Rules.tech_orb_insert_sans_spc_elev = 0;
    Rules.max_airdrop_sans_orb_insert = 100;
    MapLongitudeBounds = 100;
    MapLatitudeBounds = 80;
    MapLongitude = 10;
    MapIsFlat = 1;
    service_mode = NORMAL;
    ability_prototypes.clear();
    tile_calls = 0;
    for (int i = 0; i != 4; ++i) { Tiles[i].bit = 0; Tiles[i].val2 = 0; }
    Vehs[0].proto_id = 11; Vehs[0].faction_id = 3;
    Vehs[0].state = 0; Vehs[0].moves_expended = 0;
    Vehs[0].x = 0; Vehs[0].y = 0;
    Vehs[1].proto_id = 12; Vehs[1].faction_id = 3;
    Vehs[1].state = 0; Vehs[1].moves_expended = 0;
    Vehs[1].x = 2; Vehs[1].y = 0;
}

int main() {
    // Both initialized, eligible rows are visited normally.
    reset(2);
    sub_500380(3, 0, 0);
    assert(ability_prototypes.size() == 2);
    assert(ability_prototypes[0] == 11 && ability_prototypes[1] == 12);
    assert(tile_calls == 2);

    // Shrinking the global count during row 0's ability query prevents row 1
    // from being read by the next loop condition.
    reset(2);
    service_mode = SHRINK_COUNT_ON_FIRST_ABILITY;
    sub_500380(3, 0, 0);
    assert(GameDataStateStorage.fields_.vehicle_current_count_ == 1);
    assert(ability_prototypes.size() == 1 && ability_prototypes[0] == 11);
    assert(tile_calls == 1);

    // A rejected first row continues and still evaluates the second row.
    reset(2);
    service_mode = FIRST_ABILITY_FALSE;
    sub_500380(3, 0, 0);
    assert(ability_prototypes.size() == 2);
    assert(ability_prototypes[0] == 11 && ability_prototypes[1] == 12);
    assert(tile_calls == 1);

    // Faction mismatch takes its continue before the ability service.
    reset(2);
    Vehs[0].faction_id = 4;
    sub_500380(3, 0, 0);
    assert(ability_prototypes.size() == 1 && ability_prototypes[0] == 12);
    assert(tile_calls == 1);

    reset(0);
    sub_500380(3, 0, 0);
    assert(ability_prototypes.empty() && tile_calls == 0);
    reset(-2);
    sub_500380(3, 0, 0);
    assert(ability_prototypes.empty() && tile_calls == 0);
}
'''


def _fixture(body: str) -> str:
    marker = "ACTUAL_SUB_500380"
    assert HARNESS.count(marker) == 1
    return HARNESS.replace(marker, body)


def _captured_limit_mutant(body: str) -> str:
    assert body.count(ENTRY) == 1, "live-loop opening mutation anchor must match once"
    assert body.count(EXIT) == 1, "live-loop exit mutation anchor must match once"
    old_entry = (
        "const int count = GameDataStateStorage.fields_.vehicle_current_count_;\n"
        "    for (int i = 0; i < count; ++i) {"
    )
    old_exit = "    }"
    mutant = body.replace(ENTRY, old_entry, 1)
    # Match the final do/while-plus-if close, leaving every loop-body statement
    # and continue exactly as extracted from product source.
    assert mutant.count(EXIT) == 1
    mutant = mutant.replace(EXIT, old_exit + "\n}", 1)
    assert mutant != body
    return mutant


def _compile_and_run(tmp_path: Path, compiler: str, source: str, name: str) -> subprocess.CompletedProcess[str]:
    cpp = tmp_path / f"{name}.cpp"
    exe = tmp_path / name
    cpp.write_text(source, encoding="utf-8")
    subprocess.run(
        [compiler, "-std=c++98", "-Wall", "-Wextra", "-Werror", str(cpp), "-o", str(exe)],
        check=True,
        capture_output=True,
        text=True,
        timeout=60,
    )
    return subprocess.run([str(exe)], capture_output=True, text=True, timeout=30)


def test_actual_vehicle_walk_reloads_live_count_and_keeps_continue_paths(tmp_path: Path) -> None:
    body = _actual_body()
    compilers = [shutil.which(name) for name in ("g++", "clang++")]
    compilers = [compiler for compiler in compilers if compiler]
    if not compilers:
        pytest.skip("no host C++ compiler available")
    for index, compiler in enumerate(compilers):
        result = _compile_and_run(tmp_path, compiler, _fixture(body), f"live-{index}")
        assert result.returncode == 0, result.stderr


def test_captured_entry_count_mutant_is_compilable_and_rejected(tmp_path: Path) -> None:
    body = _actual_body()
    mutant = _captured_limit_mutant(body)
    compilers = [shutil.which(name) for name in ("g++", "clang++")]
    compilers = [compiler for compiler in compilers if compiler]
    if not compilers:
        pytest.skip("no host C++ compiler available")
    for index, compiler in enumerate(compilers):
        result = _compile_and_run(tmp_path, compiler, _fixture(mutant), f"captured-{index}")
        assert result.returncode != 0, f"captured-count mutant passed with {compiler}"
