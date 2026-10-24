"""Actual-source tests separating game_data's header and live format words.

The header selects the initial legacy/current record. Later format decisions
consume the independently transferred live state, with native reread points
after encrypted and player-row transfers. This bounded host test does not
initialize or inspect the deliberately indeterminate write-side header.
"""
from pathlib import Path
import shutil
import subprocess

import pytest

from match_recovery.source import Mode, read_file


ROOT = Path(__file__).resolve().parents[2]
SOURCE = ROOT / "src/game.cpp"
FIXTURE = ROOT / "recovery/tests/fixtures/game_data_actual_host_control.cpp"
MAPPED_ID = 0x005A6AC0
SIGNATURE = "int __cdecl game_data(FILE *file, int write_mode) {"
SCENARIO_MARKER = "    /* GAME_DATA_FORMAT_STATE_SCENARIOS */"


def actual_implementation() -> str:
    source = SOURCE.read_text(encoding="utf-8")
    rows = [row for row in read_file(SOURCE) if row.address == MAPPED_ID]
    assert len(rows) == 1 and rows[0].mode is Mode.BODY
    assert rows[0].name == "?game_data@@YAHPAUFILE@@H@Z"
    assert "DEFERRED:" not in rows[0].region
    at = rows[0].region.find(SIGNATURE)
    assert at >= 0
    body = rows[0].region[at:]
    helper_at = source.find("struct GameDataHeaderWire {")
    helper_end = source.find(SIGNATURE, helper_at)
    assert helper_at >= 0 and helper_end > helper_at
    helpers = source[helper_at:helper_end]
    assert "game_data_migrate_legacy_header" in helpers
    assert "game_data_candidate_transfer_players" in helpers
    return helpers + body


SCENARIOS = r'''
    // Feed a fully initialized legacy byte image through the existing stream
    // path. Offsets below are literal pin-derived wire offsets; this oracle
    // does not name or mirror an implementation-side header member.
    #define PUT_LEGACY_DWORD(offset, value) do { \
        const int legacy_word = (value); \
        memcpy(legacy_header + (offset), &legacy_word, 4); \
    } while (0)
    unsigned char legacy_header[0x31C];
    memset(legacy_header, 0, sizeof legacy_header);
    legacy_header[0x10] = 0xA7; // untouched opaque prefix byte (current view also opaque)
    PUT_LEGACY_DWORD(0x38, 0);  // current vehicle count slot
    PUT_LEGACY_DWORD(0x3C, 0);  // current base count slot
    PUT_LEGACY_DWORD(0x104, 0x1234567); // project entry 32
    PUT_LEGACY_DWORD(0x108, 17); // governor dword
    PUT_LEGACY_DWORD(0x10C, 101); // council count first
    PUT_LEGACY_DWORD(0x134, 111); // council count last
    PUT_LEGACY_DWORD(0x138, 201); // council turn first
    PUT_LEGACY_DWORD(0x160, 211); // council turn last
    legacy_header[0x164] = 0x5A; // achieved technology byte 0
    legacy_header[0x1BC] = 0x6B; // achieved technology byte 88
    legacy_header[0x1BD] = 'W'; // save-name first byte
    legacy_header[0x1BE] = '\0';
    legacy_header[0x2BB] = '\0'; // keep the byte before the endpoint terminated
    legacy_header[0x2BC] = 'Z'; // save-name byte 255
    PUT_LEGACY_DWORD(0x2C0, 301); // climate first dword
    PUT_LEGACY_DWORD(0x2C4, 302);
    PUT_LEGACY_DWORD(0x2C8, 303);
    PUT_LEGACY_DWORD(0x2CC, 304);
    PUT_LEGACY_DWORD(0x2D0, 305); // climate/bonus last dword
    PUT_LEGACY_DWORD(0x2D4, 401); // weight 0
    PUT_LEGACY_DWORD(0x2F0, 408); // weight 7
    PUT_LEGACY_DWORD(0x2F4, 501); // first terminal dword
    PUT_LEGACY_DWORD(0x2F8, 502);
    PUT_LEGACY_DWORD(0x2FC, 503);
    PUT_LEGACY_DWORD(0x300, 504);
    PUT_LEGACY_DWORD(0x304, 10); // terminal word 4: legacy version
    PUT_LEGACY_DWORD(0x308, 0);
    PUT_LEGACY_DWORD(0x30C, 0);
    PUT_LEGACY_DWORD(0x310, 0);
    PUT_LEGACY_DWORD(0x314, 0);
    PUT_LEGACY_DWORD(0x318, 509); // last terminal dword
    FILE *legacy_wire = tmpfile();
    assert(legacy_wire != 0);
    assert(fwrite(legacy_header, sizeof legacy_header, 1, legacy_wire) == 1);
    rewind(legacy_wire);
    reset_trace();
    read_header_from_stream = 1;
    fail_at_step = 1; // stop on the first post-migration transfer
    assert(game_data(legacy_wire, 0) == 1);
    assert(transfer_steps.size() == 2);
    assert(transfer_steps[0].size == 0x31C);
    assert(transfer_steps[1].size == 4);
    assert(GameDataStateStorage.fields_.secret_project_.project_base[32] == 0x1234567);
    assert(GameDataStateStorage.fields_.secret_project_.project_base[33] == -1);
    assert(GameDataStateStorage.fields_.secret_project_.project_base[63] == -1);
    assert(GameDataStateStorage.fields_.governor_faction_ == 17);
    assert(GameDataStateStorage.fields_.council_resolution_count_[0] == 101);
    assert(GameDataStateStorage.fields_.council_resolution_count_[10] == 111);
    assert(GameDataStateStorage.fields_.council_last_resolution_turn_[0] == 201);
    assert(GameDataStateStorage.fields_.council_last_resolution_turn_[10] == 211);
    assert(GameDataStateStorage.fields_.game_tech_achieved_[0] == 0x5A);
    assert(GameDataStateStorage.fields_.game_tech_achieved_[88] == 0x6B);
    assert(GameDataStateStorage.fields_.legacy_game_save_name_[0] == 'W');
    assert(GameDataStateStorage.fields_.legacy_game_save_name_[254] == '\0');
    assert(GameDataStateStorage.fields_.legacy_game_save_name_[255] == 'Z');
    assert(GameDataStateStorage.fields_.climate_rise_step_ == 301);
    assert(GameDataStateStorage.fields_.climate_rise_limit_ == 302);
    assert(GameDataStateStorage.fields_.climate_rise_accum_ == 303);
    assert(GameDataStateStorage.fields_.climate_sea_level_delta_ == 304);
    assert(GameDataStateStorage.fields_.transcendence_score_bonus_ == 305);
    assert(GameDataStateStorage.fields_.transcendence_score_weights_[0] == 401);
    assert(GameDataStateStorage.fields_.transcendence_score_weights_[7] == 408);
    assert(GameDataStateStorage.fields_.sunspot_duration_ == 501);
    assert(GameDataStateStorage.fields_.save_daemon_state_word_ == 10);
    assert(GameDataStateStorage.fields_.ecology_event_accumulator_ == 509);
    assert(reinterpret_cast<const unsigned char *>(&GameDataStateStorage)[0x10] == 0xA7);
    fclose(legacy_wire);

    // Version 11 selects the current live-state transfer without first
    // copying/migrating the legacy prefix. Fail that transfer before the
    // mock can change state, then confirm old project sentinels are untouched.
    memset(legacy_header + 0x304, 0, 4);
    PUT_LEGACY_DWORD(0x304, 11);
    legacy_wire = tmpfile();
    assert(legacy_wire != 0);
    assert(fwrite(legacy_header, sizeof legacy_header, 1, legacy_wire) == 1);
    rewind(legacy_wire);
    reset_trace();
    memset(&GameDataStateStorage, 0x5C, sizeof GameDataStateStorage);
    GameDataStateStorage.fields_.game_state_ = 0;
    read_header_from_stream = 1;
    fail_at_step = 1;
    assert(game_data(legacy_wire, 0) == 1);
    assert(transfer_steps.size() == 2);
    assert(transfer_steps[1].buffer == &GameDataStateStorage);
    assert(transfer_steps[1].size == 0x398);
    assert(GameDataStateStorage.fields_.secret_project_.project_base[32] == 0x5C5C5C5C);
    fclose(legacy_wire);
    #undef PUT_LEGACY_DWORD

    // Header v12 selects the 0x398 live-state record, but the transferred
    // state word is v5. Later gates must follow v5, not header.version_.
    reset_trace();
    transfer_header_version = 12;
    transfer_live_format_state[0] = 5;
    transfer_live_format_state[1] = 5;
    GameDataStateStorage.fields_.game_state_ = 0;
    GameDataStateStorage.fields_.vehicle_current_count_ = 0;
    GameDataStateStorage.fields_.base_current_count_ = 0;
    VehPrototypes[0].weapon_id = 12;
    assert(game_data(file, 0) == 0);
    assert(transfer_steps[0].size == 0x31C);
    assert(transfer_steps[1].size == 0x398);
    assert(transfer_steps[3].size == 0x105E0);
    assert(!has_transfer_size(0x10660));
    assert(!has_transfer_size(0x1000));
    assert(!has_transfer_size(0x27A0));
    assert(VehPrototypes[0].weapon_id == 15);
    assert(GameDataStateStorage.fields_.save_daemon_state_word_ == 5);

    // Exercise each distinct live-format boundary while the initial header
    // remains v12: PlayerData, vehicle rows, BungList, monuments/icons,
    // current-base rows, and tectonic counters.
    const int format_versions[] = {0, 1, 2, 3, 4, 5, 6, 7, 10, 11, 12};
    for (int format_index = 0; format_index < 11; ++format_index) {
        const int format = format_versions[format_index];
        reset_trace();
        transfer_header_version = 12;
        transfer_live_format_state[0] = format;
        transfer_live_format_state[1] = format;
        GameDataStateStorage.fields_.game_state_ = 0;
        GameDataStateStorage.fields_.vehicle_current_count_ = 1;
        GameDataStateStorage.fields_.base_current_count_ = 1;
        VehPrototypes[0].weapon_id = 12;
        VehPrototypes[8].icon_offset = -8;
        TectonicDetonationCount[0] = 0x13579BDFu;
        monuments_clear_calls = 0;
        scenario_clear_calls = 0;
        assert(game_data(file, 0) == 0);
        assert(GameDataStateStorage.fields_.save_daemon_state_word_ == format);
        assert(has_transfer_size(format >= 11 ? 0x10660
                                  : format < 3 ? 0xE5E0 : 0x105E0));
        assert(has_transfer_size(format < 3 ? 0x30 : 0x34));
        assert(has_transfer_size(format < 11 ? 0x130 : 0x134));
        assert(has_transfer_buffer(SaveNameBuffer, 0x100) == (format >= 1));
        assert(has_transfer_buffer(&ScenarioSave, 0x98) == (format >= 2));
        assert(has_transfer_size(0x1000) == (format >= 6));
        assert(has_transfer_size(0x27A0) == (format >= 7));
        assert(monuments_clear_calls == (format < 7));
        assert(scenario_clear_calls == (format < 2));
        assert(VehPrototypes[0].weapon_id == (format <= 10 ? 15 : 12));
        assert((TectonicDetonationCount[0] != 0) == (format >= 12));
        assert(VehPrototypes[8].icon_offset == (format <= 7 ? 3 : -8));
    }

    // The encrypted second 0x398 transfer replaces the live state. It crosses
    // the v6 BungList boundary after an initial v5 state selected encryption.
    reset_trace();
    transfer_header_version = 12;
    transfer_live_format_state[0] = 5;
    transfer_live_format_state[1] = 6;
    GameDataStateStorage.fields_.game_state_ = 0x10000000u;
    assert(game_data(file, 0) == 0);
    assert(transfer_steps[1].size == 0x398);
    assert(transfer_steps[2].buffer == &GameDataStateStorage);
    assert(transfer_steps[2].size == 0x398);
    assert(GameDataStateStorage.fields_.save_daemon_state_word_ == 6);
    assert(has_transfer_size(0x1000));
    assert(!has_transfer_size(0x27A0));

    // The helper first sees v2 and transfers old rows. A transfer callback
    // changes the live state to v11 before the native second read at 5A7252;
    // that later decision therefore selects the current PlayerData block.
    reset_trace();
    transfer_header_version = 12;
    transfer_live_format_state[0] = 2;
    transfer_live_format_state[1] = 2;
    live_format_after_legacy_player_transfer = 11;
    GameDataStateStorage.fields_.game_state_ = 0;
    assert(game_data(file, 0) == 0);
    assert(transfer_steps[3].size == 0xE5E0);
    assert(transfer_steps[4].buffer == PlayersData);
    assert(transfer_steps[4].size == 0x10660);

    // In the legacy path, the final ten-dword header migration writes the
    // header's version at +0x304 into the live format word at +0x380.
    reset_trace();
    transfer_header_version = 2;
    GameDataStateStorage.fields_.save_daemon_state_word_ = 99;
    assert(game_data(file, 0) == 0);
    assert(GameDataStateStorage.fields_.save_daemon_state_word_ == 2);

    // Vehicle proto-id conversion is gated by a fresh live-state read after
    // the legacy base transfer callback, not by the earlier vehicle-row
    // snapshot. Initial v3 uses current vehicle rows and legacy base rows;
    // the base callback changes the live word to v2 before conversion.
    reset_trace();
    transfer_header_version = 12;
    transfer_live_format_state[0] = 3;
    transfer_live_format_state[1] = 3;
    live_format_after_base_transfer = 2;
    GameDataStateStorage.fields_.game_state_ = 0;
    GameDataStateStorage.fields_.vehicle_current_count_ = 1;
    GameDataStateStorage.fields_.base_current_count_ = 1;
    Vehs[0].proto_id = 32;
    assert(game_data(file, 0) == 0);
    assert(has_transfer_size(0x34));
    assert(has_transfer_size(0x130));
    assert(GameDataStateStorage.fields_.save_daemon_state_word_ == 2);
    assert(Vehs[0].proto_id == 64);
'''


def compile_and_run(compiler: str, tmp_path: Path, implementation: str,
                    scenarios: str = SCENARIOS) -> subprocess.CompletedProcess:
    executable = shutil.which(compiler)
    if executable is None:
        pytest.skip(f"{compiler} is unavailable")
    fixture = FIXTURE.read_text(encoding="utf-8")
    implementation_marker = "/* ACTUAL_GAME_DATA_IMPLEMENTATION */"
    state_marker = "/* ACTUAL_GAME_DATA_STATE_HEADER */"
    assert fixture.count(implementation_marker) == 1
    assert fixture.count(state_marker) == 1
    assert fixture.count(SCENARIO_MARKER) == 1
    source = fixture.replace(implementation_marker, implementation)
    source = source.replace(SCENARIO_MARKER, scenarios)
    state_header = ROOT / "src/game_data_state_window.h"
    source = source.replace(state_marker, f'#include "{state_header}"')
    source = source.replace("game_data_candidate(", "game_data(")
    cpp = tmp_path / "game_data_format_state.cpp"
    binary = tmp_path / "game_data_format_state"
    cpp.write_text(source, encoding="utf-8")
    built = subprocess.run(
        [executable, "-std=c++98", "-Wall", "-Wextra", str(cpp), "-o", str(binary)],
        capture_output=True, text=True, timeout=60)
    assert built.returncode == 0, built.stdout + built.stderr
    return subprocess.run([str(binary)], capture_output=True, text=True, timeout=60)


def wrong_header_mutant(implementation: str) -> str:
    anchor = "GameDataStateStorage.fields_.save_daemon_state_word_"
    body_at = implementation.find(SIGNATURE)
    assert body_at >= 0
    helpers, body = implementation[:body_at], implementation[body_at:]
    assert body.count(anchor) >= 10
    return helpers + body.replace(anchor, "header_version")


def stale_encrypted_state_mutant(implementation: str) -> str:
    body_at = implementation.find(SIGNATURE)
    gate = implementation.find(
        "if (GameDataStateStorage.fields_.save_daemon_state_word_ >= 4)",
        body_at)
    assert body_at >= 0 and gate >= 0
    anchor = "GameDataStateStorage.fields_.save_daemon_state_word_"
    replacement = "const int stale_format = " + anchor + ";\n    if (stale_format >= 4)"
    old_gate = "if (" + anchor + " >= 4)"
    changed = implementation[:gate] + implementation[gate:].replace(
        old_gate, replacement, 1)
    body_at = changed.find(SIGNATURE)
    gate_end = changed.find(replacement, body_at) + len(replacement)
    assert gate_end > body_at
    return (changed[:gate_end]
            + changed[gate_end:].replace(anchor, "stale_format"))


def early_vehicle_conversion_mutant(implementation: str) -> str:
    anchor = "    // 5A7EE7 supplies both the icon correction and base-record layout tests."
    assert implementation.count(anchor) == 1
    conversion = (
        "    if (GameDataStateStorage.fields_.save_daemon_state_word_ < 3) {\n"
        "        for (int expanded_vehicle_id = 0;\n"
        "             expanded_vehicle_id < GameDataStateStorage.fields_.vehicle_current_count_;\n"
        "             ++expanded_vehicle_id)\n"
        "            Vehs[expanded_vehicle_id].proto_id = static_cast<int16_t>(\n"
        "                game_data_expand_legacy_prototype_id(\n"
        "                    Vehs[expanded_vehicle_id].proto_id));\n"
        "    }\n")
    return implementation.replace(anchor, conversion + anchor, 1)


@pytest.mark.parametrize("compiler", ["g++", "clang++"])
def test_actual_source_keeps_header_and_live_format_state_independent(compiler, tmp_path):
    ran = compile_and_run(compiler, tmp_path, actual_implementation())
    assert ran.returncode == 0, ran.stdout + ran.stderr


@pytest.mark.parametrize("compiler", ["g++", "clang++"])
@pytest.mark.parametrize("mutant", [wrong_header_mutant,
                                      stale_encrypted_state_mutant])
def test_compile_success_wrong_format_state_mutants_are_rejected(
        compiler, mutant, tmp_path):
    changed = mutant(actual_implementation())
    ran = compile_and_run(compiler, tmp_path, changed)
    assert ran.returncode != 0, f"{compiler} accepted {mutant.__name__}"


@pytest.mark.parametrize("compiler", ["g++", "clang++"])
def test_base_transfer_failure_precedes_legacy_vehicle_id_conversion(compiler, tmp_path):
    scenarios = SCENARIOS + r'''
    // First obtain the precise base-row transfer index with a legacy v2
    // vehicle ID that visibly changes under the native remapping.
    reset_trace();
    transfer_header_version = 12;
    transfer_live_format_state[0] = 2;
    transfer_live_format_state[1] = 2;
    GameDataStateStorage.fields_.game_state_ = 0;
    GameDataStateStorage.fields_.vehicle_current_count_ = 1;
    GameDataStateStorage.fields_.base_current_count_ = 1;
    legacy_vehicle_proto_id = 32;
    assert(game_data(file, 0) == 0);
    int base_step = -1;
    for (size_t step = 0; step < transfer_steps.size(); ++step)
        if (transfer_steps[step].size == 0x130) base_step = static_cast<int>(step);
    assert(base_step >= 0);
    assert(Vehs[0].proto_id == 64);

    // Fail that exact base row on a fresh run. The earlier vehicle record has
    // decoded the configured proto byte, but the post-base conversion point
    // has not yet run; failure returns with the raw ID intact.
    reset_trace();
    transfer_header_version = 12;
    transfer_live_format_state[0] = 2;
    transfer_live_format_state[1] = 2;
    GameDataStateStorage.fields_.game_state_ = 0;
    GameDataStateStorage.fields_.vehicle_current_count_ = 1;
    GameDataStateStorage.fields_.base_current_count_ = 1;
    legacy_vehicle_proto_id = 32;
    fail_at_step = base_step;
    assert(game_data(file, 0) == 1);
    assert(static_cast<int>(transfer_steps.size()) == base_step + 1);
    assert(Vehs[0].proto_id == 32);
'''
    ran = compile_and_run(compiler, tmp_path, actual_implementation(), scenarios)
    assert ran.returncode == 0, ran.stdout + ran.stderr


@pytest.mark.parametrize("compiler", ["g++", "clang++"])
def test_compile_success_early_vehicle_conversion_mutant_is_rejected(compiler, tmp_path):
    changed = early_vehicle_conversion_mutant(actual_implementation())
    scenarios = SCENARIOS + r'''
    reset_trace();
    transfer_header_version = 12;
    transfer_live_format_state[0] = 2;
    transfer_live_format_state[1] = 2;
    GameDataStateStorage.fields_.game_state_ = 0;
    GameDataStateStorage.fields_.vehicle_current_count_ = 1;
    GameDataStateStorage.fields_.base_current_count_ = 1;
    legacy_vehicle_proto_id = 32;
    int base_step = -1;
    // The matching successful call supplies the failure index for this same
    // input. The fixture is rebuilt in this executable, so compute the known
    // unique legacy base-row call from the recorded successful prefix below.
    assert(game_data(file, 0) == 0);
    for (size_t step = 0; step < transfer_steps.size(); ++step)
        if (transfer_steps[step].size == 0x130) base_step = static_cast<int>(step);
    assert(base_step >= 0);
    reset_trace();
    transfer_header_version = 12;
    transfer_live_format_state[0] = 2;
    transfer_live_format_state[1] = 2;
    GameDataStateStorage.fields_.game_state_ = 0;
    GameDataStateStorage.fields_.vehicle_current_count_ = 1;
    GameDataStateStorage.fields_.base_current_count_ = 1;
    legacy_vehicle_proto_id = 32;
    fail_at_step = base_step;
    assert(game_data(file, 0) == 1);
    assert(Vehs[0].proto_id == 32);
'''
    ran = compile_and_run(compiler, tmp_path, changed, scenarios)
    assert ran.returncode != 0, f"{compiler} accepted early vehicle conversion"


def test_fixture_model_keeps_wire_selector_and_live_state_independent():
    fixture = FIXTURE.read_text(encoding="utf-8")
    assert "transfer_header_version = 12;" in fixture
    assert "transfer_live_format_state[0] = 5;" in SCENARIOS
    assert "save_daemon_state_word_ =\n                transfer_live_format_state[state_index];" in fixture
    assert "read_state_from_stream" in fixture
    assert "fread(buffer, size, 1, file)" in fixture
