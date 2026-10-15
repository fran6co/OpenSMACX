"""Actual-source controls for typed lock-mask byte views and read stages."""

from pathlib import Path
import shutil
import subprocess
import tempfile


ROOT = Path(__file__).resolve().parents[2]


def body(source: str, signature: str) -> str:
    start = source.index(signature)
    opening = source.index("{", start)
    depth = 0
    for pos in range(opening, len(source)):
        if source[pos] == "{":
            depth += 1
        elif source[pos] == "}":
            depth -= 1
            if depth == 0:
                return source[start:pos + 1]
    raise AssertionError("unterminated function: " + signature)


def mask_declarations() -> str:
    header = (ROOT / "src/game_data_state_window.h").read_text()
    start = header.index("struct GameDataLockMaskBytes {")
    end = header.index("struct GameDataStateWindowFields {")
    return header[start:end]


def run_control(program: str, *, expect_success: bool = True) -> None:
    compilers = [shutil.which(name) for name in ("g++", "clang++")]
    compilers = [compiler for compiler in compilers if compiler]
    assert compilers, "GCC or Clang is required for host source controls"
    with tempfile.TemporaryDirectory(prefix="mask-views-oct15-") as temp:
        source = Path(temp) / "control.cpp"
        source.write_text(program)
        for compiler in compilers:
            binary = Path(temp) / Path(compiler).name.replace("+", "p")
            built = subprocess.run(
                [compiler, "-std=c++98", "-Wall", "-Wextra", "-Werror",
                 str(source), "-o", str(binary)],
                capture_output=True, text=True, timeout=45)
            assert built.returncode == 0, built.stdout + built.stderr
            result = subprocess.run([str(binary)], capture_output=True,
                                    text=True, timeout=10)
            assert (result.returncode == 0) == expect_success, result.stderr


def test_any_locks_uses_named_bytes_from_one_snapshot_and_controls_adjacent_bytes():
    source = (ROOT / "src/lock.cpp").read_text()
    actual = body(source, "int Lock::any_locks()")
    assert actual.count("GameDataStateStorage.fields_.lock_masks_.enable_mask_") == 1
    assert "const GameDataLockMaskView mask = {" in actual
    assert "mask.enable_mask_ & 0xFF" in actual
    assert "mask.bytes_.alive_mask_" in actual
    assert "reinterpret_cast<const uint8_t *>(&mask)" not in actual

    harness = r'''#include <stdint.h>
#include <cassert>
#define __cdecl
''' + mask_declarations() + r'''
struct Fields { GameDataLockMaskView lock_masks_; };
struct GameDataStateWindow { Fields fields_; };
GameDataStateWindow GameDataStateStorage = {};
struct Entry { uint8_t flag; };
struct Record { Entry entries_[2]; };
struct Lock {
    uint32_t field_E0_;
    Record records_[8];
    int any_locks();
};
'''
    main = r'''
int main() {
    Lock lock = {};
    lock.records_[1].entries_[0].flag = 1;
    GameDataStateStorage.fields_.lock_masks_.enable_mask_ = 0x00000002u;
    assert(lock.any_locks() == 0); // human bit alone is insufficient
    GameDataStateStorage.fields_.lock_masks_.enable_mask_ = 0x00000200u;
    assert(lock.any_locks() == 0); // alive bit alone is insufficient
    GameDataStateStorage.fields_.lock_masks_.enable_mask_ = 0x00000202u;
    assert(lock.any_locks() == 1);
}
'''
    run_control(harness + actual + main)

    damaged = actual.replace("mask.bytes_.alive_mask_", "mask.bytes_.human_mask_", 1)
    assert damaged != actual
    run_control(harness + damaged + main, expect_success=False)


def test_probe_choice_helper_reloads_alive_byte_after_row_callback():
    source = (ROOT / "src/probe.cpp").read_text()
    helper = body(source, "static int probe_native_result_one_choices(")
    assert "GameDataStateStorage.fields_.lock_masks_.bytes_.alive_mask_" in helper
    assert helper.index("bytes_.alive_mask_") > helper.index("for (int faction_id")
    assert helper.index("bytes_.alive_mask_") < helper.index("parse_set(")

    fixture = (ROOT / "recovery/tests/fixtures/probe_host_stubs.inc").read_text()
    old_view = "struct GameDataLockMaskView { uint32_t enable_mask_; };"
    assert fixture.count(old_view) == 1
    fixture = fixture.replace(old_view, mask_declarations())
    old_parse_set = "static int parse_set(int, int) { return 0; }"
    callback = r'''static int ProbeHostParseSetCalls = 0;
static int parse_set(int faction_marker, int) {
    ++ProbeHostParseSetCalls;
    if (faction_marker == 2)
        GameDataStateStorage.fields_.lock_masks_.bytes_.alive_mask_ &=
            static_cast<uint8_t>(~(1u << 3));
    return 0;
}'''
    assert fixture.count(old_parse_set) == 1
    fixture = fixture.replace(old_parse_set, callback)
    old_rates = "static int success_rates(int, int, int, int) { return 0; }"
    assert fixture.count(old_rates) == 1
    fixture = fixture.replace(old_rates,
                              "static int success_rates(int, int, int, int) { return 1; }")
    # Compile the real extracted helper with the existing host stubs. Only the
    # row callback is adapted to mutate the live mask between candidate rows.
    from test_probe_actual_source_runtime import compilers
    compilers_for_probe = compilers()
    popup_and_helper = r'''
int main() {
    Popup popup;
    Vehs[0].probe_action = 0;
    Players[2].is_leader_female = 2;
    Players[3].is_leader_female = 3;
    PlayersData[1].diplo_treaties[2] = 8u;
    PlayersData[1].diplo_treaties[3] = 8u;
    GameDataStateStorage.fields_.lock_masks_.enable_mask_ = 0x00000C00u;
    const int selected = probe_native_result_one_choices(popup, 1, 7, 0, 0, 0);
    assert(selected == 0);
    assert(ProbeHostParseSetCalls == 1); // row 2 callback removed row 3
    assert((GameDataStateStorage.fields_.lock_masks_.bytes_.alive_mask_ & 8u) == 0);
}
'''

    def run_probe_helper(actual_helper: str, *, expect_success: bool) -> None:
        unit = "#include <cassert>\n" + fixture + "\n" + actual_helper + "\n" + popup_and_helper
        with tempfile.TemporaryDirectory(prefix="probe-mask-view-") as temp:
            cpp = Path(temp) / "probe.cpp"
            cpp.write_text(unit)
            for compiler in compilers_for_probe:
                binary = Path(temp) / Path(compiler).name.replace("+", "p")
                built = subprocess.run(
                    [compiler, "-std=c++98", "-w", str(cpp), "-o", str(binary)],
                    capture_output=True, text=True, timeout=60)
                assert built.returncode == 0, built.stdout + built.stderr
                result = subprocess.run([str(binary)], capture_output=True,
                                        text=True, timeout=10)
                assert (result.returncode == 0) == expect_success, result.stderr

    run_probe_helper(helper, expect_success=True)

    # Test-only damaged control hoists the same named read above the row loop;
    # it must now wrongly retain faction 3 after faction 2's callback.
    read = "        const uint8_t alive =\n            GameDataStateStorage.fields_.lock_masks_.bytes_.alive_mask_;\n"
    assert helper.count(read) == 1
    hoisted = helper.replace(read, "", 1)
    loop = "    int choices = 0;\n    for (int faction_id = 1; faction_id < MaxPlayerNum; ++faction_id) {"
    assert hoisted.count(loop) == 1
    hoisted = hoisted.replace(loop,
        "    int choices = 0;\n" + read +
        "    for (int faction_id = 1; faction_id < MaxPlayerNum; ++faction_id) {", 1)
    run_probe_helper(hoisted, expect_success=False)


def test_net_not_my_turn_reads_named_bytes_after_each_network_pump():
    source = (ROOT / "src/stringstruct.cpp").read_text()
    actual = body(source, "void __cdecl net_not_my_turn()")
    pump = actual.index("NetDaemonNet.net_tasks();")
    human = actual.index("lock_bytes->human_mask_")
    alive = actual.index("lock_bytes->alive_mask_")
    assert pump < human < alive
    assert "reinterpret_cast<const unsigned char *>" not in actual

    harness = r'''#include <stdint.h>
#include <cassert>
#define __cdecl
''' + mask_declarations() + r'''
struct Fields { GameDataLockMaskView lock_masks_; int net_turn_faction_; };
struct GameDataStateWindow { Fields fields_; };
GameDataStateWindow GameDataStateStorage = {};
static int ExitTurnLoop = 0, NextTurnReady = 0, next_calls = 0;
static uint32_t waiting_mask = 0;
struct Console {
    int faction_id_, field_23BE4_, field_23BE8_;
    void set_view(int) {}
};
Console ConsoleGlobal = {};
struct Multi { void draw(int) {} };
Multi MultiWindow;
struct Daemon {
    struct State { uint32_t leading_words_[1]; } primary_state_;
    uint32_t &factions_waiting_mask() { return waiting_mask; }
    void net_tasks() {
        primary_state_.leading_words_[0] = 1;
        GameDataStateStorage.fields_.net_turn_faction_ = ConsoleGlobal.faction_id_;
        GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_ =
            static_cast<uint8_t>(1u << ConsoleGlobal.faction_id_);
        GameDataStateStorage.fields_.lock_masks_.bytes_.alive_mask_ =
            static_cast<uint8_t>(1u << ConsoleGlobal.faction_id_);
    }
};
Daemon NetDaemonNet;
static void next_player_turn() { ++next_calls; ExitTurnLoop = 1; }
'''
    main = r'''
int main() {
    ConsoleGlobal.faction_id_ = 1;
    GameDataStateStorage.fields_.net_turn_faction_ = 2;
    GameDataStateStorage.fields_.lock_masks_.enable_mask_ = 0;
    net_not_my_turn();
    assert(next_calls == 0); // pump published both bytes before they were read
    assert((waiting_mask & (1u << 1)) != 0);
    assert(ConsoleGlobal.field_23BE4_ == 0 && ConsoleGlobal.field_23BE8_ == 0);
}
'''
    run_control(harness + actual + main)

    # A test-only entry snapshot is stale: net_tasks() publishes the current
    # faction in both bytes, and native reads them only after that pump.
    insertion = "void __cdecl net_not_my_turn() {"
    snapshot = ("\n    const GameDataLockMaskBytes saved_mask = "
                "GameDataStateStorage.fields_.lock_masks_.bytes_;")
    damaged = actual.replace(insertion, insertion + snapshot, 1)
    current_pointer = ("const GameDataLockMaskBytes *const lock_bytes =\n"
                       "                    &GameDataStateStorage.fields_.lock_masks_.bytes_;")
    assert damaged.count(current_pointer) == 1
    damaged = damaged.replace(
        current_pointer,
        "const GameDataLockMaskBytes *const lock_bytes = &saved_mask;", 1)
    assert damaged != actual
    run_control(harness + damaged + main, expect_success=False)


def test_probe_shared_stage_uses_two_named_byte_locals_after_update():
    source = (ROOT / "src/probe.cpp").read_text()
    start = source.index("label_5a2012:")
    end = source.index("native_target_bit =", start)
    stage = source[start:end]
    assert stage.index("ConsoleGlobal.update_data(0)") < stage.index("native_human_bits =")
    assert "native_human_bits =\n            GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_;" in stage
    assert "native_alive_bits =\n            GameDataStateStorage.fields_.lock_masks_.bytes_.alive_mask_;" in stage
    assert stage.index("native_human_bits =") < stage.index("native_alive_bits =")
    assert "reinterpret_cast<const uint8_t *>" not in stage
