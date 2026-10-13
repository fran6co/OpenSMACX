"""Bounded GCC/Clang controls for veh_find's actual distance output and stores."""
from __future__ import annotations

import hashlib
import os
import re
import shutil
import subprocess
from pathlib import Path

import pytest

from match_recovery.model import Span
from match_recovery.pe_coff import PeImage


ROOT = Path(__file__).resolve().parents[2]
PIN = "01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf"
DISTANCE_SPAN = Span(0x009B227C, 0x009B2280)
DISTANCE_OBJECT = (
    "// RECOVERY_OBJECT: v2 0x009B227C 0x009B2280 "
    "?VehFindBestDist@@3HA 0x005C0CB0"
)


def actual_source() -> tuple[str, str]:
    source = (ROOT / "src/veh.cpp").read_text()
    assert source.count(DISTANCE_OBJECT) == 1
    declaration_start = source.index(DISTANCE_OBJECT)
    declaration_end = source.index(";", declaration_start) + 1
    declaration = source[declaration_start:declaration_end]
    assert declaration.endswith("int VehFindBestDist = 0;")

    signature = "int __cdecl veh_find(int x, int y, int faction_id, int skip_veh_id)"
    start = source.index(signature)
    opening = source.index("{", start)
    depth = 0
    for end in range(opening, len(source)):
        if source[end] == "{":
            depth += 1
        elif source[end] == "}":
            depth -= 1
            if depth == 0:
                return declaration, source[start:end + 1]
    raise AssertionError("unterminated veh_find definition")


def original_initial_bytes() -> bytes | None:
    configured = os.environ.get("OPENSMACX_IMAGE")
    image_path = Path(configured) if configured else ROOT / ".opensmacx/game/terranx_original.exe"
    if not image_path.is_file():
        return None
    image_bytes = image_path.read_bytes()
    image = PeImage(image_bytes, PIN)
    assert hashlib.sha256(image_bytes).hexdigest() == PIN
    return image.read_initial(DISTANCE_SPAN)


def instrument_stores(function: str) -> str:
    function, sentinel_count = re.subn(
        r"VehFindBestDist\s*=\s*(-?\d+)\s*;",
        r"record_distance_store(\1);",
        function,
    )
    function, best_count = re.subn(
        r"VehFindBestDist\s*=\s*best_dist\s*;",
        "record_distance_store(best_dist);",
        function,
    )
    assert sentinel_count == best_count == 1
    return function


HARNESS = r"""
#include <assert.h>
#include <stdlib.h>
#define __cdecl
struct Vehicle { unsigned int faction_id; int x; int y; };
Vehicle Vehs[8];
struct GameDataState { struct { int vehicle_current_count_; } fields_; };
GameDataState GameDataStateStorage;
int MapIsFlat = 1;
int MapLongitude = 0;
int MapLongitudeBounds = 0;
#ifdef INSTRUMENTED_STORES
int store_count;
int stores[4];
#endif
ACTUAL_DECLARATION
#ifdef INSTRUMENTED_STORES
void require(bool condition) { if (!condition) exit(98); }
void record_distance_store(int value) {
    require(store_count < 4);
    stores[store_count++] = value;
    VehFindBestDist = value;
}
#define TRACE_CHECK(condition) require(condition)
#else
void require(bool condition) { if (!condition) exit(98); }
#define TRACE_CHECK(condition) ((void)0)
#endif
ACTUAL_FUNCTION
void reset_run(int count) {
    GameDataStateStorage.fields_.vehicle_current_count_ = count;
#ifdef INSTRUMENTED_STORES
    store_count = 0;
#endif
    VehFindBestDist = 1234;
    MapIsFlat = 1;
    MapLongitude = 0;
    MapLongitudeBounds = 0;
    for (int i = 0; i < 8; ++i) {
        Vehs[i].faction_id = 0;
        Vehs[i].x = 0;
        Vehs[i].y = 0;
    }
}
int main() {
    // This observes the actual source global before veh_find's first reset.
    require(VehFindBestDist == 0);

    reset_run(0);
    require(veh_find(0, 0, -1, -1) == -1);
    require(VehFindBestDist == 9999);
    TRACE_CHECK(store_count == 1 && stores[0] == 9999);

    // Faction filtering and skip-id filtering leave only vehicles 2 and 3.
    reset_run(4);
    Vehs[0].faction_id = 2; Vehs[0].x = 1;
    Vehs[1].faction_id = 1; Vehs[1].x = 2;
    Vehs[2].faction_id = 1; Vehs[2].x = 6;
    Vehs[3].faction_id = 1; Vehs[3].x = 5;
    require(veh_find(0, 0, 1, 1) == 3);
    require(VehFindBestDist == 4);
    TRACE_CHECK(store_count == 2 && stores[0] == 9999 && stores[1] == 4);

    // With no eligible row, only the entry sentinel store occurs.
    reset_run(2);
    Vehs[0].faction_id = 2; Vehs[1].faction_id = 3;
    require(veh_find(0, 0, 1, -1) == -1);
    require(VehFindBestDist == 9999);
    TRACE_CHECK(store_count == 1 && stores[0] == 9999);

    // Equal distances keep the historical <= rule: the last matching row wins.
    reset_run(2);
    Vehs[0].faction_id = Vehs[1].faction_id = 1;
    Vehs[0].x = Vehs[1].x = 3;
    Vehs[0].y = Vehs[1].y = 4;
    require(veh_find(0, 0, 1, -1) == 1);
    require(VehFindBestDist == 4);
    TRACE_CHECK(store_count == 2 && stores[1] == 4);

    // Cylindrical longitude adjustment maps an 8-step delta through 12-8.
    reset_run(1);
    MapIsFlat = 0; MapLongitude = 5; MapLongitudeBounds = 12;
    Vehs[0].faction_id = 1; Vehs[0].x = 8;
    require(veh_find(0, 0, 1, -1) == 0);
    require(VehFindBestDist == 3);
    TRACE_CHECK(store_count == 2 && stores[1] == 3);
    return 0;
}
"""


def compile_and_run(compiler: str, tmp_path: Path, declaration: str,
                    function: str, label: str, should_pass: bool,
                    instrumented: bool = False) -> None:
    code = HARNESS.replace("ACTUAL_DECLARATION", declaration)
    if instrumented:
        code = code.replace("ACTUAL_FUNCTION", instrument_stores(function))
        code = code.replace("#define __cdecl", "#define INSTRUMENTED_STORES\n#define __cdecl")
    else:
        code = code.replace("ACTUAL_FUNCTION", function)
    cpp = tmp_path / f"{label}.cpp"
    executable = tmp_path / label
    cpp.write_text(code)
    built = subprocess.run(
        [compiler, "-std=c++98", "-O2", str(cpp), "-o", str(executable)],
        capture_output=True,
        text=True,
        timeout=30,
    )
    assert built.returncode == 0, built.stdout + built.stderr
    ran = subprocess.run([str(executable)], capture_output=True, text=True, timeout=10)
    if should_pass:
        assert ran.returncode == 0, ran.stdout + ran.stderr
    else:
        assert ran.returncode == 98, f"{label} mutant did not fail a runtime assertion: {ran.returncode}"


def test_pinned_window_is_four_zero_initialized_bytes():
    initial = original_initial_bytes()
    if initial is None:
        pytest.skip("pinned PE image is unavailable for the optional initial-byte control")
    assert initial == b"\0\0\0\0"


@pytest.mark.parametrize("compiler_name", ["g++", "clang++"])
def test_actual_veh_find_and_store_mutants(compiler_name, tmp_path):
    compiler = shutil.which(compiler_name)
    if not compiler:
        pytest.skip(f"{compiler_name} is not installed")
    declaration, actual = actual_source()
    assert "// ORIGINAL: 0x005C0CB0 ?veh_find@@" in (ROOT / "src/veh.cpp").read_text()
    assert "if (best_idx >= 0)" in actual

    no_tie_update = actual.replace("if (dist <= best_dist)", "if (dist < best_dist)", 1)
    no_entry_sentinel = actual.replace("VehFindBestDist = 9999;", "VehFindBestDist = 0;", 1)
    skip_ignored = actual.replace("i != skip_veh_id", "true", 1)
    no_longitude_wrap = actual.replace(
        "d1 = MapLongitudeBounds - d1;", "d1 = d1;", 1
    )
    unconditional_best_store = actual.replace(
        "if (best_idx >= 0) {\n            VehFindBestDist = best_dist;\n        }",
        "VehFindBestDist = best_dist;",
        1,
    )
    assert all(candidate != actual for candidate in (
        no_tie_update, no_entry_sentinel, skip_ignored,
        no_longitude_wrap, unconditional_best_store,
    ))

    compile_and_run(compiler, tmp_path, declaration, actual, "actual_direct", True)
    compile_and_run(compiler, tmp_path, declaration, actual, "actual_trace", True,
                    instrumented=True)
    for label, mutant in (
        ("tie", no_tie_update),
        ("sentinel", no_entry_sentinel),
        ("skip", skip_ignored),
        ("longitude_wrap", no_longitude_wrap),
        ("unconditional_store", unconditional_best_store),
    ):
        compile_and_run(compiler, tmp_path, declaration, mutant, label, False,
                        instrumented=True)
