"""Host controls for the actual NetDaemon::wait body and state-word contract.

The fixture extracts the installed definition on every run. It checks source
behavior under a host API shim; it does not prove Windows ABI, native runtime,
or the original compiler's code generation.
"""
from pathlib import Path
import shutil
import subprocess

import pytest


ROOT = Path(__file__).resolve().parents[2]
SOURCE = ROOT / "src/netdaemon.cpp"
SIGNATURE = "int NetDaemon::wait(char * message, uint32_t * state, int value, int timeout)"


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
    raise AssertionError("unterminated actual wait definition")


HARNESS = r'''
#include <cassert>
#include <cstdint>
#include <cstring>
#include <string>
#include <vector>

typedef uint32_t DWORD;
#define WINAPI
static std::vector<std::string> events;
static std::vector<DWORD> ticks;
static size_t tick_index;
static uint32_t *state_to_clear;
static uint32_t state_after_message;
static bool return_message;
static int get_count;
static int message_available_after_get;
static char message_byte;
static unsigned long message_tag = 77;
static int NetBusyCount, ExitTurnLoop;

DWORD timeGetTime() {
    events.push_back("time");
    assert(tick_index < ticks.size());
    return ticks[tick_index++];
}
void wait_task() {
    events.push_back("wait_task");
}

struct LockStateEntry {
    uint32_t leading_words_[4];
    union Payload { uint32_t flags; void *pointer; } payload_;
    uint32_t trailing_words_[10];
};
struct Lock { void check_global() { events.push_back("check_global"); } };
struct Net {
    void *get(unsigned long *, unsigned long *tag) {
        events.push_back("get");
        ++get_count;
        *tag = message_tag;
        return return_message && get_count >= message_available_after_get
            ? &message_byte : 0;
    }
};
struct NetDaemon : Net {
    LockStateEntry channels_[2]{};
    Lock locks_;
    void process_message(char *message, unsigned long tag, int) {
        assert(message == &message_byte && tag == message_tag);
        events.push_back("process_message");
        channels_[0].leading_words_[3] = 0xdeadbeefu;
        if (state_to_clear) *state_to_clear = state_after_message;
    }
    void check_spock() { events.push_back("check_spock"); }
    int wait(char *, uint32_t *, int, int);
} NetDaemonNet;
struct Popup {
    int pops = 0;
    int closes = 0;
    void pop(char *, int, int, int) { ++pops; events.push_back("pop"); }
    void close() { ++closes; events.push_back("close"); }
} NetMsgPrimary;

ACTUAL_WAIT

static void reset() {
    events.clear(); ticks.clear(); tick_index = 0;
    state_to_clear = 0; state_after_message = 0; return_message = false;
    get_count = 0; message_available_after_get = 1;
    NetBusyCount = 0; ExitTurnLoop = 0;
    NetMsgPrimary.pops = NetMsgPrimary.closes = 0;
    NetDaemonNet = NetDaemon{};
}

int main() {
    // Equality remains bitwise for a high-bit state. The actual network
    // callback clears the live owner word; saved global mode is restored.
    reset();
    uint32_t state = 0x80000000u;
    int signed_high_bit;
    std::memcpy(&signed_high_bit, &state, sizeof(state));
    NetDaemonNet.channels_[0].leading_words_[3] = 0x11223344u;
    state_to_clear = &state; state_after_message = 0x7fffffffu;
    return_message = true; ticks = {100u, 101u};
    assert(NetDaemonNet.wait(const_cast<char *>("HIGH"), &state, signed_high_bit, 200) == 0);
    assert(state == 0x7fffffffu && tick_index == 2);
    assert(NetBusyCount == 0 && NetMsgPrimary.pops == 0 && NetMsgPrimary.closes == 0);
    assert(NetDaemonNet.channels_[0].leading_words_[3] == 0x11223344u);
    assert((events == std::vector<std::string>{"time", "time", "wait_task", "get",
        "process_message", "check_global", "check_spock"}));

    // Timeout is emitted once, the poll still runs, and close follows the
    // callback/pump sequence. The word is changed by a consumer, not copied.
    reset(); state = 1; state_to_clear = &state; state_after_message = 0;
    return_message = true; message_available_after_get = 2;
    ticks = {10u, 11u, 12u};
    assert(NetDaemonNet.wait(const_cast<char *>("ONCE"), &state, 1, 0) == 0);
    assert(state == 0 && NetMsgPrimary.pops == 1 && NetMsgPrimary.closes == 1);
    assert(get_count == 2 && tick_index == 2);
    assert(NetDaemonNet.channels_[0].leading_words_[3] == 0);
    assert((events == std::vector<std::string>{"time", "time", "pop", "wait_task", "get",
        "check_global", "check_spock", "wait_task", "get", "process_message",
        "check_global", "check_spock", "close"}));

    // Elapsed DWORD subtraction wraps across FFFFFFFF, then compares unsigned.
    reset(); state = 9; state_to_clear = &state; state_after_message = 8;
    return_message = true; ticks = {0xfffffffeu, 1u};
    NetDaemonNet.channels_[0].leading_words_[3] = 0x87654321u;
    assert(NetDaemonNet.wait(const_cast<char *>("WRAP"), &state, 9, 2) == 0);
    assert(NetMsgPrimary.pops == 1 && NetMsgPrimary.closes == 1 && state == 8);
    assert(NetDaemonNet.channels_[0].leading_words_[3] == 0x87654321u);

    // 0x80000000 elapsed is greater than a positive signed timeout's DWORD
    // representation, but not greater than timeout -1's FFFFFFFF pattern.
    reset(); state = 3; state_to_clear = &state; state_after_message = 4;
    return_message = true; ticks = {0u, 0x80000000u};
    assert(NetDaemonNet.wait(const_cast<char *>("HALF"), &state, 3, 0x7fffffff) == 0);
    assert(NetMsgPrimary.pops == 1 && NetMsgPrimary.closes == 1);

    reset(); state = 3; state_to_clear = &state; state_after_message = 4;
    return_message = true; ticks = {0u, 0x80000000u};
    assert(NetDaemonNet.wait(const_cast<char *>("NEG"), &state, 3, -1) == 0);
    assert(NetMsgPrimary.pops == 0 && NetMsgPrimary.closes == 0);

    // ExitTurnLoop breaks before timeout sampling/pumping, while cleanup
    // restores the saved global mode and balances the busy counter.
    reset(); state = 5; ExitTurnLoop = 1;
    NetDaemonNet.channels_[0].leading_words_[3] = 0xaabbccddu;
    ticks = {20u};
    assert(NetDaemonNet.wait(const_cast<char *>("EXIT"), &state, 5, 0) == 0);
    assert(tick_index == 1 && events == std::vector<std::string>{"time"});
    assert(NetBusyCount == 0 && NetDaemonNet.channels_[0].leading_words_[3] == 0xaabbccddu);
    return 0;
}
'''


@pytest.mark.parametrize("mutant", (
    "actual", "repeat_timeout", "omit_mode_restore", "signed_timeout",
))
def test_actual_wait_body_controls(tmp_path, mutant):
    compiler = shutil.which("g++")
    if not compiler:
        pytest.skip("host C++ compiler unavailable")
    source = SOURCE.read_text(encoding="utf-8")
    body = extract_definition(source, SIGNATURE)
    assert "uint32_t * state" in body
    assert "static_cast<DWORD>(now - startTime) > static_cast<DWORD>(timeout)" in body
    changes = {
        "repeat_timeout": ("if (!triggeredTimeout)", "if (true)"),
        "omit_mode_restore": (
            "NetDaemonNet.channels_[0].leading_words_[3] = saved;", "(void)saved;"),
        "signed_timeout": (
            "static_cast<DWORD>(now - startTime) > static_cast<DWORD>(timeout)",
            "static_cast<int>(now - startTime) > timeout"),
    }
    if mutant != "actual":
        old, new = changes[mutant]
        assert body.count(old) == 1, "damage control must modify the actual body"
        body = body.replace(old, new)
    unit = tmp_path / "wait_actual.cpp"
    unit.write_text(HARNESS.replace("ACTUAL_WAIT", body), encoding="utf-8")
    executable = tmp_path / "wait_actual"
    build = subprocess.run(
        [compiler, "-std=c++17", "-Wall", "-Wextra",
         "-fpermissive", str(unit), "-o", str(executable)],
        capture_output=True, text=True,
    )
    assert build.returncode == 0, build.stdout + build.stderr
    run = subprocess.run([str(executable)], capture_output=True, text=True)
    if mutant == "actual":
        assert run.returncode == 0, run.stdout + run.stderr
    else:
        assert run.returncode != 0, "actual-source control failed to detect damaged behavior"


def test_wait_state_storage_is_passed_without_integer_pointer_casts():
    source = SOURCE.read_text(encoding="utf-8")
    header = (ROOT / "src/netdaemon.h").read_text(encoding="utf-8")
    assert "int wait(char *message, uint32_t *state, int value, int timeout);" in header
    assert source.count("wait(\"AWAITEXEC\", acknowledged, 1,") == 1
    assert source.count("wait(\"AWAITEXEC\", flag, 1, 2000)") == 3
    assert "reinterpret_cast<int *>(&channels_[1].leading_words_[3])" not in source
    assert "reinterpret_cast<int *>(&channels_[1].trailing_words_[0])" not in source
