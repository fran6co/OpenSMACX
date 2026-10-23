"""Actual-source C++98 checks for goody_at's signed arithmetic."""

from __future__ import annotations

import re
import resource
import shutil
import subprocess
from pathlib import Path

import pytest
from match_recovery.source import read_file


GOODY_ADDRESS = 0x00592140
GOODY_SIGNATURE = "int __cdecl goody_at(int x, int y)"


def _root() -> Path:
    for parent in Path(__file__).resolve().parents:
        if (parent / "src/map.cpp").is_file():
            return parent
    raise AssertionError("could not locate repository root")


def _definition(source: str, signature: str) -> str:
    start = source.index(signature)
    opening = source.index("{", start)
    depth = 0
    for offset in range(opening, len(source)):
        if source[offset] == "{":
            depth += 1
        elif source[offset] == "}":
            depth -= 1
            if depth == 0:
                return source[start : offset + 1]
    raise AssertionError("unterminated definition: " + signature)


def _actual_definitions() -> tuple[str, dict[str, str], dict[str, str]]:
    root = _root()
    map_cpp = root / "src/map.cpp"
    map_h = root / "src/map.h"
    body_source = map_cpp.read_text(encoding="utf-8")
    header_source = map_h.read_text(encoding="utf-8")
    rows = {row.address: row for row in read_file(map_cpp)}
    assert GOODY_ADDRESS in rows
    row = rows[GOODY_ADDRESS]
    assert row.name == "?goody_at@@YAHHH@Z"
    assert row.image_spans == ((GOODY_ADDRESS, 0x00592248),)
    body = _definition(body_source, GOODY_SIGNATURE)
    assert body in row.region

    helpers = {
        "map_tiles": _definition(header_source, "inline Map *&map_tiles()"),
        "map_loc": _definition(header_source, "inline Map *__cdecl map_loc(int x, int y)"),
        "bit_at": _definition(header_source, "inline int __cdecl bit_at(int x, int y)"),
        "altitude_at": _definition(
            header_source, "inline int __cdecl altitude_at(uint32_t x, uint32_t y)"
        ),
        "is_ocean": _definition(header_source, "inline BOOL __cdecl is_ocean(int x, int y)"),
    }
    for helper in helpers.values():
        assert helper in header_source

    # Keep the fixture's compact model synchronized with the actual named
    # constants and owner declarations, without claiming host layout is ABI.
    for constant in (
        "BIT_SUPPLY_REMOVE = 0x100000",
        "BIT_MONOLITH = 0x2000",
        "BIT_UNK_4000000 = 0x4000000",
        "BIT_UNK_8000000 = 0x8000000",
        "BIT_SUPPLY_POD = 0x10000000",
        "ALT_BIT_SHORE_LINE = 0x60",
    ):
        assert constant in header_source
    game_h = (root / "src/game.h").read_text(encoding="utf-8")
    state_h = (root / "src/game_data_state_window.h").read_text(encoding="utf-8")
    assert "RULES_NO_UNITY_SCATTERING = 0x2000" in game_h
    assert "uint32_t game_rules_" in state_h
    assert "uint32_t rand_seed_" in header_source
    globals_ = {}
    for name, value in (
        ("MapPodDifferenceMultiplier", 61),
        ("MapPodAverageMultiplier", 11),
    ):
        match = re.search(r"(?m)^int " + name + r" = (-?\d+);$", body_source)
        assert match and int(match.group(1)) == value
        globals_[name] = "static " + match.group(0)

    # The pinned code uses SAR for avg and signed divide-by-four with a
    # negative correction in the final predicate (0x00592140..0x00592248).
    # This suite exercises the translated source, not execution of the PE.
    assert re.search(r"int\s+avg\s*=\s*\(x\s*\+\s*y\)\s*>>\s*1\s*;", body)
    return body, helpers, globals_


HARNESS = r'''#include <assert.h>
#include <stdint.h>

#define __cdecl
typedef int BOOL;

struct Map {
  uint8_t climate;
  uint8_t contour;
  uint8_t val2;
  uint8_t region;
  uint8_t visibility;
  uint8_t val3;
  uint8_t unk_1;
  int8_t territory;
  uint32_t bit;
  uint32_t bit2;
  uint32_t bit_visible[7];
};
struct MapMetadataModel { uint32_t rand_seed_; };
struct MapStorageWindowModel { MapMetadataModel metadata_; Map *tiles_; };
static MapStorageWindowModel MapStorage;
static int MapLongitude;
#define MapTiles (MapStorage.tiles_)
#define MapRandSeed (MapStorage.metadata_.rand_seed_)

struct GameDataStateFieldsModel { uint32_t game_rules_; };
struct GameDataStateWindowModel { GameDataStateFieldsModel fields_; };
static GameDataStateWindowModel GameDataStateStorage;

enum {
  BIT_MONOLITH = 0x2000,
  BIT_SUPPLY_REMOVE = 0x100000,
  BIT_UNK_4000000 = 0x4000000,
  BIT_UNK_8000000 = 0x8000000,
  BIT_SUPPLY_POD = 0x10000000,
  ALT_BIT_SHORE_LINE = 0x60,
  RULES_NO_UNITY_SCATTERING = 0x2000
};
'''


MAIN = r'''static Map tiles[128];

static void reset_map() {
  for (int i = 0; i < 128; ++i) {
    tiles[i].climate = 0;
    tiles[i].bit = 0;
    tiles[i].bit2 = 0;
  }
  MapStorage.tiles_ = tiles;
  MapLongitude = 32;
  MapRandSeed = 18;
  GameDataStateStorage.fields_.game_rules_ = 0;
}

int main() {
  typedef char host_int_must_be_32_bits[sizeof(int) == 4 ? 1 : -1];
  (void)sizeof(host_int_must_be_32_bits);
  // Conversion from uint32_t back to int is implementation-defined in C++98.
  // This host control is scoped to GCC/Clang's 32-bit two's-complement result,
  // the behavior expected from the pinned 32-bit MSVC code.
  assert(static_cast<int>(static_cast<uint32_t>(-2)) == -2);
  assert((-1 >> 1) == -1); // host arithmetic right shift, as pinned SAR does

  reset_map();
  tiles[0].climate = 0x60;
  tiles[0].bit = BIT_SUPPLY_REMOVE | BIT_MONOLITH | BIT_SUPPLY_POD;
  assert(goody_at(0, 0) == 0); // remove/monolith precedence over pod

  reset_map();
  tiles[0].bit = BIT_MONOLITH | BIT_SUPPLY_POD;
  assert(goody_at(0, 0) == 0); // monolith also suppresses an overlapping pod

  reset_map();
  GameDataStateStorage.fields_.game_rules_ = RULES_NO_UNITY_SCATTERING;
  tiles[0].bit = BIT_UNK_4000000;
  assert(goody_at(0, 0) == 2);
  tiles[0].bit = 0;
  assert(goody_at(0, 0) == 0);

  reset_map();
  tiles[0].bit = BIT_SUPPLY_POD;
  MapRandSeed = 0;
  assert(goody_at(0, 0) == 1); // explicit pod precedes random-seed fallback

  reset_map();
  MapRandSeed = 0;
  assert(goody_at(0, 0) == 0);

  // Positive ordinary non-ocean coordinates exercise the first pattern.
  reset_map();
  tiles[2 * MapLongitude + 1].climate = 0x60;
  assert(goody_at(2, 2) == 2);

  // An ocean tile skips the first pattern and reaches the final multiplier
  // predicate using the two source-owned signed coefficient globals.
  reset_map();
  tiles[0].climate = 0x40;
  MapRandSeed = 24;
  assert(goody_at(0, 0) == 1);

  // Logically out-of-map x=-2 still indexes allocated tile 31 under the
  // original unchecked row-stride expression. No caller reachability is
  // asserted; this safely distinguishes signed avg /4 from unsigned avg /4.
  reset_map();
  tiles[31].climate = 0x60;
  MapRandSeed = 18;
  assert(map_loc(-2, 1) == &tiles[31]);
  assert(altitude_at(static_cast<uint32_t>(-2), 1) == 0x60);
  assert(!is_ocean(-2, 1));
  assert(goody_at(-2, 1) == 0);
  return 0;
}
'''


def _program(body: str, helpers: dict[str, str], globals_: dict[str, str]) -> str:
    return (
        HARNESS
        + globals_["MapPodDifferenceMultiplier"] + "\n"
        + globals_["MapPodAverageMultiplier"] + "\n"
        + helpers["map_tiles"] + "\n"
        + helpers["map_loc"] + "\n"
        + helpers["bit_at"] + "\n"
        + helpers["altitude_at"] + "\n"
        + helpers["is_ocean"] + "\n"
        + body + "\n"
        + MAIN
    )


def _compile_run(tmp_path: Path, program: str, compiler: str) -> int:
    tmp_path.mkdir(parents=True, exist_ok=True)
    unit = tmp_path / "map_goody_actual.cpp"
    exe = tmp_path / "map_goody_actual"
    unit.write_text(program, encoding="utf-8")
    built = subprocess.run(
        [compiler, "-std=c++98", "-O0", str(unit), "-o", str(exe)],
        text=True, capture_output=True, timeout=30,
    )
    assert built.returncode == 0, built.stderr
    def no_core_dumps() -> None:
        _, hard = resource.getrlimit(resource.RLIMIT_CORE)
        resource.setrlimit(resource.RLIMIT_CORE, (0, hard))

    return subprocess.run([str(exe)], text=True, capture_output=True,
                          timeout=10, preexec_fn=no_core_dumps).returncode


def _compilers() -> list[str]:
    found = [shutil.which("g++"), shutil.which("clang++")]
    found = [compiler for compiler in found if compiler]
    if not found:
        pytest.skip("host GCC or Clang unavailable")
    return found


@pytest.mark.parametrize("compiler", _compilers())
def test_actual_goody_body_preserves_signed_pod_arithmetic(
    tmp_path: Path, compiler: str,
) -> None:
    body, helpers, globals_ = _actual_definitions()
    assert _compile_run(tmp_path / "positive", _program(body, helpers, globals_), compiler) == 0


@pytest.mark.parametrize("compiler", _compilers())
def test_unsigned_average_damage_fails_same_in_allocation_witness(
    tmp_path: Path, compiler: str,
) -> None:
    body, helpers, globals_ = _actual_definitions()
    assert _compile_run(
        tmp_path / "positive", _program(body, helpers, globals_), compiler
    ) == 0
    correct = "int avg = (x + y) >> 1;"
    assert body.count(correct) == 1
    damaged = body.replace(correct, "uint32_t avg = (x + y) >> 1;", 1)
    assert _compile_run(tmp_path / "damage", _program(damaged, helpers, globals_), compiler) != 0
