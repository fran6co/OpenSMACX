"""Actual-source controls for the NetDaemon's direct multimedia clock calls."""
from pathlib import Path
import shutil
import subprocess

import pytest


ROOT = Path(__file__).resolve().parents[2]
SOURCE = ROOT / "src/netdaemon.cpp"


def extract_definition(source: str, signature: str) -> str:
    start = source.find(signature)
    if start < 0 or source.find(signature, start + len(signature)) >= 0:
        raise AssertionError(f"expected one current definition: {signature}")
    opening = source.find("{", start + len(signature))
    if opening < 0:
        raise AssertionError("definition body missing")
    depth = 0
    for pos in range(opening, len(source)):
        if source[pos] == "{":
            depth += 1
        elif source[pos] == "}":
            depth -= 1
            if depth == 0:
                return source[start:pos + 1]
    raise AssertionError("unterminated actual definition")


WAIT_SIGNATURE = "int NetDaemon::wait(char * message, uint32_t * state, int value, int timeout)"
ADD_GLOBAL_SIGNATURE = "int NetDaemon::add_global()"

HARNESS = r'''
#include <cassert>
#include <cstdint>
#include <cstring>
#include <string>

typedef uint32_t DWORD;
#define WINAPI

static std::string clock_calls;
static DWORD clock_value = 1000;
static DWORD clock_a() { clock_calls += 'A'; return clock_value; }
static DWORD clock_b() { clock_calls += 'B'; return clock_value; }
static DWORD clock_c() { clock_calls += 'C'; return clock_value; }
static DWORD clock_d() { clock_calls += 'D'; return clock_value; }
static DWORD clock_e() { clock_calls += 'E'; return clock_value; }
static DWORD clock_f() { clock_calls += 'F'; return clock_value; }
static DWORD (WINAPI *timeGetTime)() = clock_a;

static int scenario;
static int message_calls;
static int message_data_calls;
static uint32_t *wait_state;
static uint32_t wait_value;
static int NetBusyCount;
static int ExitTurnLoop;
static int IsMultiplayerNet;
static int NetPlayerDropped;
static char message_byte;

void wait_task() { }

struct LockStateEntry {
    uint32_t leading_words_[4]{};
    uint32_t trailing_words_[10]{};
};
struct Lock {
    int field_E8_;
    void check_global() { }
};
struct Net {
    void *get(unsigned long *sender, unsigned long *tag) {
        *sender = 1;
        *tag = 2;
        return &message_byte;
    }
};
struct NetDaemon : Net {
    LockStateEntry channels_[2]{};
    Lock locks_{};
    void process_message(char *, unsigned long, int) {
        ++message_calls;
        if (scenario == 1) {
            if (message_calls == 1) timeGetTime = clock_b;
            if (message_calls == 2) *wait_state = wait_value;
        } else if (scenario == 2) {
            if (message_calls == 1) timeGetTime = clock_c;
            if (message_calls == 2) {
                channels_[0].leading_words_[2] = 1;
                timeGetTime = clock_d;
            }
            if (message_calls == 3) timeGetTime = clock_e;
            if (message_calls == 4) {
                locks_.field_E8_ = 1;
                timeGetTime = clock_f;
            }
        }
    }
    void check_spock() { }
    int wait(char *, uint32_t *, int, int);
    int add_global();
} NetDaemonNet;

void message_data(int, int, int, int, int, int) {
    ++message_data_calls;
    NetPlayerDropped = 0;
    NetDaemonNet.channels_[0].leading_words_[2] = 0;
    NetDaemonNet.locks_.field_E8_ = 0;
    timeGetTime = clock_b;
}

struct Popup {
    int pop_count;
    int close_count;
    void pop(const char *, int, int, int) { ++pop_count; }
    void close() { ++close_count; }
} NetMsgPrimary;

ACTUAL_WAIT

ACTUAL_ADD_GLOBAL

static void reset() {
    clock_calls.clear();
    timeGetTime = clock_a;
    scenario = 0;
    message_calls = 0;
    message_data_calls = 0;
    wait_state = nullptr;
    wait_value = 0;
    NetBusyCount = 0;
    ExitTurnLoop = 0;
    IsMultiplayerNet = 0;
    NetPlayerDropped = 0;
    NetDaemonNet = NetDaemon{};
    NetMsgPrimary = Popup{};
}

int main(int argc, char **argv) {
    assert(argc == 2);
    const std::string mode = argv[1];
    if (mode == "offline") {
        reset();
        timeGetTime = clock_a;
        assert(NetDaemonNet.add_global() == 0);
        assert(clock_calls.empty());
        assert(message_data_calls == 0 && message_calls == 0);
        assert(NetBusyCount == 0);
        return 0;
    }
    if (mode == "wait") {
        reset();
        scenario = 1;
        uint32_t state = 7;
        wait_state = &state;
        wait_value = 8;
        NetDaemonNet.channels_[0].leading_words_[3] = 0x12345678u;
        assert(NetDaemonNet.wait(const_cast<char *>("CLOCK"), &state, 7, 10000) == 0);
        assert(clock_calls == "AAB");
        assert(state == 8 && message_calls == 2);
        assert(NetBusyCount == 0);
        assert(NetDaemonNet.channels_[0].leading_words_[3] == 0x12345678u);
        return 0;
    }
    if (mode == "add_global") {
        reset();
        scenario = 2;
        IsMultiplayerNet = 1;
        NetDaemonNet.channels_[0].leading_words_[3] = 0x87654321u;
        assert(NetDaemonNet.add_global() == 0);
        assert(clock_calls == "BBCDDE");
        assert(message_data_calls == 1 && message_calls == 4);
        assert(NetDaemonNet.channels_[0].leading_words_[2] == 1);
        assert(NetDaemonNet.locks_.field_E8_ == 1);
        assert(NetBusyCount == 0);
        assert(NetDaemonNet.channels_[0].leading_words_[3] == 0x87654321u);
        return 0;
    }
    return 2;
}
'''


def _mutate_cached_clock(body: str, variable: str) -> str:
    opening = body.find("{")
    assert opening >= 0
    body = body[:opening + 1] + f"\n    DWORD (WINAPI *const {variable})() = timeGetTime;" + body[opening + 1:]
    assert "timeGetTime()" in body
    return body.replace("timeGetTime()", f"{variable}()")


@pytest.mark.parametrize("mutant", ("actual", "cached_wait", "cached_add_global"))
def test_actual_clock_reads_follow_the_current_iat_target(tmp_path, mutant):
    compiler = shutil.which("g++")
    if not compiler:
        pytest.skip("host C++ compiler unavailable")
    source = SOURCE.read_text(encoding="utf-8")
    wait = extract_definition(source, WAIT_SIGNATURE)
    add_global = extract_definition(source, ADD_GLOBAL_SIGNATURE)
    if mutant == "cached_wait":
        wait = _mutate_cached_clock(wait, "timeGetTimeFn")
    elif mutant == "cached_add_global":
        add_global = _mutate_cached_clock(add_global, "get_time")
    unit_text = HARNESS.replace("ACTUAL_WAIT", wait).replace("ACTUAL_ADD_GLOBAL", add_global)
    unit = tmp_path / "netdaemon_clock_actual.cpp"
    unit.write_text(unit_text, encoding="utf-8")
    executable = tmp_path / "netdaemon_clock_actual"
    build = subprocess.run(
        [compiler, "-std=c++17", "-Wall", "-Wextra", "-Werror",
         str(unit), "-o", str(executable)],
        capture_output=True, text=True,
    )
    assert build.returncode == 0, build.stdout + build.stderr
    modes = ("wait", "add_global", "offline") if mutant == "actual" else (
        "wait" if mutant == "cached_wait" else "add_global",
    )
    for mode in modes:
        run = subprocess.run([str(executable), mode], capture_output=True, text=True)
        if mutant == "actual":
            assert run.returncode == 0, f"{mode}: {run.stdout}{run.stderr}"
        else:
            assert run.returncode != 0, f"{mutant} damage control missed {mode}"

