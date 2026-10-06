"""Standalone host regression control that extracts the current game.cpp body."""
from __future__ import annotations

from pathlib import Path
import shutil
import subprocess
import tempfile

import pytest

from match_recovery.source import read_file
from match_recovery.source.reader import _region_end


ROOT = next(parent for parent in Path(__file__).resolve().parents
            if (parent / "src/game.cpp").is_file())
GAME = ROOT / "src/game.cpp"
ADDRESS = 0x00528EA0

HARNESS = r'''#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define __cdecl
typedef uint32_t DWORD;
struct ConsoleMock { int faction_id_; } ConsoleGlobal;
struct MultiWinMock { int draws; void draw(int) { ++draws; } } MultiWindow;
struct LockStateMock { uint32_t leading_words_[4]; uint32_t trailing_words_[10]; };
struct NetDaemonMock {
    LockStateMock lock_state_[2];
    int tasks, receives;
    void (*task_hook)(); void (*receive_hook)();
    void net_tasks() { ++tasks; if (task_hook) task_hook(); }
    void receive() { ++receives; if (receive_hook) receive_hook(); }
} NetDaemonNet;
struct NetMsgMock {
    int pops, closes, pop_timeout, pop_flags;
    void (*close_hook)(); char popped[256];
    void pop(const char *s, int timeout, int flags, const char *) {
        ++pops; pop_timeout = timeout; pop_flags = flags;
        strncpy(popped, s, sizeof(popped) - 1); popped[sizeof(popped) - 1] = 0;
    }
    void close() { ++closes; if (close_hook) close_hook(); }
} NetMsgPrimary;
struct GameFields { int turn_current_num_; struct { uint32_t enable_mask_; } lock_masks_; };
struct GameStateMock { GameFields fields_; } GameDataStateStorage;
int ExitTurnLoop;
char StringTemp[256];
static int log_count, parse_count, message_count, clock_calls;
static char log_text[8][64];
static int message_opcodes[8], parse_ids[8], parse_values[8];
static DWORD clock_values[8];
void log_say(const char *text, int, int, int) {
    assert(log_count < 8); strncpy(log_text[log_count], text, 63);
    log_text[log_count][63] = 0; ++log_count;
}
int parse_num(int id, int value) {
    assert(parse_count < 8); parse_ids[parse_count] = id;
    parse_values[parse_count++] = value; return 0;
}
uint32_t message_data(int opcode, int, int, int, int, int) {
    assert(message_count < 8); message_opcodes[message_count++] = opcode; return 0;
}
DWORD timeGetTime() { assert(clock_calls < 8); return clock_values[clock_calls++]; }
char *_itoa(int value, char *buffer, int radix) {
    assert(radix == 10); sprintf(buffer, "%d", value); return buffer;
}
'''

SUFFIX = r'''
static void reset_fixture() {
    memset(&NetDaemonNet, 0, sizeof(NetDaemonNet));
    memset(&MultiWindow, 0, sizeof(MultiWindow));
    memset(&NetMsgPrimary, 0, sizeof(NetMsgPrimary));
    memset(log_text, 0, sizeof(log_text)); memset(message_opcodes, 0, sizeof(message_opcodes));
    memset(parse_ids, 0, sizeof(parse_ids)); memset(parse_values, 0, sizeof(parse_values));
    memset(clock_values, 0, sizeof(clock_values)); memset(StringTemp, 0, sizeof(StringTemp));
    log_count = parse_count = message_count = clock_calls = 0;
    ConsoleGlobal.faction_id_ = 2; GameDataStateStorage.fields_.turn_current_num_ = 10;
    GameDataStateStorage.fields_.lock_masks_.enable_mask_ = 0; ExitTurnLoop = 0;
}
static void finish_wait() {
    GameDataStateStorage.fields_.lock_masks_.enable_mask_ = 1;
    NetDaemonNet.lock_state_[0].trailing_words_[8] = 1;
}
static void exit_from_task() { ExitTurnLoop = 1; }
static void finish_receive() {
    NetDaemonNet.lock_state_[0].trailing_words_[8] =
        GameDataStateStorage.fields_.lock_masks_.enable_mask_;
}
static void exit_from_close() { ExitTurnLoop = 1; }
static void test_normal_ack() {
    reset_fixture(); GameDataStateStorage.fields_.lock_masks_.enable_mask_ = 3;
    NetDaemonNet.lock_state_[0].trailing_words_[8] = 3;
    assert(net_upkeep_phase(2) == 0);
    assert(MultiWindow.draws == 1 && message_count == 1 && message_opcodes[0] == 0x2303);
    assert(NetMsgPrimary.closes == 1 && NetDaemonNet.tasks == 0);
    assert(NetDaemonNet.lock_state_[1].leading_words_[1] == 0);
    assert(NetDaemonNet.lock_state_[0].trailing_words_[8] == 0);
}
static void test_timeout_callback_mutates_live_mask() {
    reset_fixture(); GameDataStateStorage.fields_.lock_masks_.enable_mask_ = 3;
    NetDaemonNet.lock_state_[0].trailing_words_[8] = 1;
    NetDaemonNet.task_hook = finish_wait;
    clock_values[0] = 100; clock_values[1] = 1201; clock_values[2] = 1202;
    assert(net_upkeep_phase(1) == 0);
    assert(NetDaemonNet.tasks == 1 && parse_count == 2);
    assert(parse_ids[0] == 0 && parse_values[0] == 1);
    assert(parse_ids[1] == 1 && parse_values[1] == 3);
    assert(GameDataStateStorage.fields_.lock_masks_.enable_mask_ == 1);
    assert(NetMsgPrimary.pops == 1 && NetMsgPrimary.pop_timeout == 0 && NetMsgPrimary.pop_flags == 1);
    assert(strcmp(NetMsgPrimary.popped, "UPKEEP1") == 0);
}
static void test_early_exit_skips_cleanup() {
    reset_fixture(); GameDataStateStorage.fields_.lock_masks_.enable_mask_ = 3; ExitTurnLoop = 1;
    assert(net_upkeep_phase(1) == 1);
    assert(NetDaemonNet.tasks == 0 && NetMsgPrimary.closes == 0);
    assert(NetDaemonNet.lock_state_[1].leading_words_[1] == (1u << 2));
}
static void test_callback_exit_and_phase_four_suppression() {
    reset_fixture(); GameDataStateStorage.fields_.lock_masks_.enable_mask_ = 1;
    NetDaemonNet.task_hook = exit_from_task; clock_values[0] = 10; clock_values[1] = 11;
    assert(net_upkeep_phase(4) == 1);
    assert(NetDaemonNet.tasks == 1 && NetMsgPrimary.closes == 0 && NetMsgPrimary.pops == 0);
}
static void test_server_receive_and_cleanup() {
    reset_fixture(); GameDataStateStorage.fields_.lock_masks_.enable_mask_ = 1;
    NetDaemonNet.lock_state_[0].trailing_words_[8] = 1;
    NetDaemonNet.lock_state_[0].leading_words_[1] = 1;
    NetDaemonNet.receive_hook = finish_receive;
    assert(net_upkeep_phase(4) == 0);
    assert(message_count == 2 && message_opcodes[1] == 0x4303 && NetDaemonNet.receives == 1);
    assert(NetMsgPrimary.closes == 1 && NetDaemonNet.lock_state_[0].trailing_words_[8] == 0);
}
static void test_close_callback_exit_is_final_return() {
    reset_fixture(); NetMsgPrimary.close_hook = exit_from_close;
    assert(net_upkeep_phase(0) == 1);
    assert(NetMsgPrimary.closes == 1 && NetDaemonNet.lock_state_[1].leading_words_[1] == 0);
}
int main() {
    test_normal_ack(); test_timeout_callback_mutates_live_mask(); test_early_exit_skips_cleanup();
    test_callback_exit_and_phase_four_suppression(); test_server_receive_and_cleanup();
    test_close_callback_exit_is_final_return(); return 0;
}
'''


def actual_body() -> str:
    text = GAME.read_text()
    rows = [row for row in read_file(GAME) if row.address == ADDRESS]
    assert len(rows) == 1
    record = rows[0]
    assert record.name == "?net_upkeep_phase@@YAHH@Z"
    assert record.image_spans == ((ADDRESS, 0x00529097),)
    lines = text.splitlines()
    matches = [i for i, line in enumerate(lines)
               if line.strip() == "int __cdecl net_upkeep_phase(int phase) {"]
    assert len(matches) == 1
    end = _region_end(lines, matches[0], record.name)
    assert end is not None
    body = "\n".join(lines[matches[0]:end + 1]) + "\n"
    assert "GameDataStateStorage.fields_.lock_masks_.enable_mask_" in body
    assert "LockEnableMask" not in body and "NetUpkeepAckMask" not in body
    return body


@pytest.mark.parametrize("compiler", ["g++", "clang++"])
def test_current_game_cpp_phase_body(compiler: str) -> None:
    executable = shutil.which(compiler)
    if executable is None:
        pytest.skip(f"{compiler} is unavailable")
    source = HARNESS + "\n" + actual_body() + "\n" + SUFFIX
    with tempfile.TemporaryDirectory(prefix="opensmacx-net-upkeep-actual-") as directory:
        root = Path(directory)
        cpp = root / "phase.cpp"
        binary = root / "phase"
        cpp.write_text(source)
        result = subprocess.run(
            [executable, "-std=c++98", "-Wall", "-Wextra", "-Werror",
             "-fsanitize=undefined", str(cpp), "-o", str(binary)],
            text=True, capture_output=True, check=False)
        assert result.returncode == 0, result.stderr
        run = subprocess.run([str(binary)], text=True, capture_output=True,
                             check=False)
        assert run.returncode == 0, run.stderr
