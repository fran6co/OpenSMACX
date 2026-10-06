"""Run the corrected host controls against the installed, parsed game_data.

This control deliberately reuses only the earlier fixture's service
mocks and assertions. The implementation under test is always extracted from
the current mapped product source through match_recovery.source; no candidate
fragment fallback is allowed.

The write-side mock only observes transfer calls and never reads or initializes
the intentionally indeterminate legacy header. These bounded host controls do
not prove write determinism, initialization-prefix behavior, full serializer
runtime, or game acceptance.
"""
from pathlib import Path
import re
import shutil
import subprocess

import pytest


ROOT = Path(__file__).resolve().parents[2]
from match_recovery.source import Mode, read_file

MOCK_FIXTURE = ROOT / "recovery/tests/fixtures/game_data_actual_host_control.cpp"
MAPPED_ID = 0x005A6AC0
SIGNATURE = "int __cdecl game_data(FILE *file, int write_mode) {"


def actual_game_data_implementation() -> str:
    source_path = ROOT / "src/game.cpp"
    source = source_path.read_text(encoding="utf-8")
    rows = [row for row in read_file(source_path) if row.address == MAPPED_ID]
    assert len(rows) == 1, "framework parser must find exactly one mapped game_data identity"
    row = rows[0]
    assert row.mode is Mode.BODY and row.name == "?game_data@@YAHPAUFILE@@H@Z"
    assert "DEFERRED:" not in row.region
    body_at = row.region.find(SIGNATURE)
    assert body_at >= 0, "parsed mapped region must contain the actual game_data definition"
    body = row.region[body_at:]

    helper_start = source.find("struct GameDataHeaderWire {")
    helper_end = source.find(SIGNATURE, helper_start)
    assert helper_start >= 0 and helper_end > helper_start
    helpers = source[helper_start:helper_end]
    assert "game_data_migrate_legacy_header" in helpers
    assert "game_data_candidate_transfer_players" in helpers
    assert "game_data_candidate_string_list" in helpers
    assert "game_data_candidate_transfer" in body
    return helpers + body


@pytest.mark.parametrize("compiler", ["g++", "clang++"])
def test_actual_mapped_game_data_body_cpp98(compiler, tmp_path):
    executable = shutil.which(compiler)
    if executable is None:
        pytest.skip(f"{compiler} is unavailable")
    fixture = MOCK_FIXTURE.read_text(encoding="utf-8")
    implementation_marker = "/* ACTUAL_GAME_DATA_IMPLEMENTATION */"
    state_marker = "/* ACTUAL_GAME_DATA_STATE_HEADER */"
    assert fixture.count(implementation_marker) == 1
    assert fixture.count(state_marker) == 1
    implementation = actual_game_data_implementation()
    source = fixture.replace(implementation_marker, implementation)
    state_header = ROOT / "src/game_data_state_window.h"
    source = source.replace(state_marker, f'#include "{state_header}"')
    source = re.sub(r"\bgame_data_candidate\(", "game_data(", source)
    assert "game_data_candidate(" not in source
    cpp = tmp_path / "game_data_actual.cpp"
    binary = tmp_path / "game_data_actual"
    cpp.write_text(source, encoding="utf-8")
    built = subprocess.run(
        [executable, "-std=c++98", "-Wall", "-Wextra", str(cpp), "-o", str(binary)],
        capture_output=True, text=True, timeout=45)
    assert built.returncode == 0, built.stdout + built.stderr
    ran = subprocess.run([str(binary)], capture_output=True, text=True, timeout=45)
    assert ran.returncode == 0, ran.stdout + ran.stderr


def test_fixture_is_current_and_no_candidate_fragment_is_used():
    implementation = actual_game_data_implementation()
    assert "int __cdecl game_data(FILE *file, int write_mode) {" in implementation
    assert "int __cdecl game_data_candidate(" not in implementation
    assert "// ORIGINAL: 0x005A6AC0" in implementation


def test_manual_integration_retains_identity_history_and_retires_seed():
    source_path = ROOT / "src/game.cpp"
    source = source_path.read_text(encoding="utf-8")
    rows = [row for row in read_file(source_path) if row.address == MAPPED_ID]
    assert len(rows) == 1 and rows[0].mode is Mode.BODY
    assert not (ROOT / "src/unrecovered/005a6ac0.cpp").exists()
    assert "// ORIGINAL: 0x005A6AC0 ?game_data@@YAHPAUFILE@@H@Z 0x005A6AC0-0x005A9279" in source
    assert "1983 lines of Ghidra decompilation and 196 conditionals" in source
    assert "batch of seven diverged only at the prologue" in source
