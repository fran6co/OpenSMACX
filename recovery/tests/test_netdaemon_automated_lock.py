"""Source-extracted controls for automated vehicle lock lifetime and retry order."""

from pathlib import Path
import shutil
import subprocess

import pytest


def test_automated_lock_native_retry_and_cleanup(tmp_path):
    compiler = shutil.which("g++")
    if not compiler:
        pytest.skip("host C++ compiler unavailable")
    text = (Path(__file__).parents[2] / "src/netdaemon.cpp").read_text()
    start = text.index("int NetDaemon::automated_lock(")
    opening = text.index("{", start)
    depth = 1
    end = opening + 1
    while depth:
        depth += (text[end] == "{") - (text[end] == "}")
        end += 1
    body = text[start:end]
    harness = r'''
#include <cassert>
#include <cstdint>
#include <cstring>
#include <vector>
#define WINAPI
typedef uint32_t DWORD;
struct Veh { uint8_t order; };
Veh Vehs[4];
int IsMultiplayerNet, NetBusyCount, ExitTurnLoop;
int LockCalls, WaitCalls, GetCalls, CheckCalls, SpockCalls;
int PopCalls, CloseCalls, ProcessCalls, Failures, AbortOnPump;
int RewriteId, TimeCalls;
std::vector<DWORD> Clock;
std::vector<int> Events;
DWORD timeGetTime() {
    assert(TimeCalls < static_cast<int>(Clock.size()));
    return Clock[TimeCalls++];
}
void log_say(const char *label, int id, int order, int zero) {
    assert(zero == 0);
    if (!std::strcmp(label, "Getting auto lock")) {
        assert(id == 0 && order == -128);
        Events.push_back(1);
    } else if (!std::strcmp(label, "Got auto lock")) {
        assert(id == (RewriteId ? 1 : 0));
        assert(order == (RewriteId ? 127 : -128));
        assert(NetBusyCount == 1);
        Events.push_back(9);
    } else {
        assert(!std::strcmp(label, "(Auto lock delay)"));
        Events.push_back(3);
    }
}
struct Popup {
    void pop(const char *label, int a, int b, int c) {
        assert(!std::strcmp(label, "AWAITORDERS"));
        assert(a == 0 && b == 2 && c == 0);
        ++PopCalls;
        Events.push_back(7);
    }
    void close() { ++CloseCalls; Events.push_back(8); }
} PopupObject;
Popup &NetMsgPrimary = PopupObject;
struct Net {
    int get(unsigned long *first, unsigned long *second) {
        ++GetCalls;
        *first = 17;
        *second = 19;
        Events.push_back(5);
        return 1234;
    }
};
struct Lock {
    void check_global() { ++CheckCalls; Events.push_back(6); }
};
struct State { uint32_t leading_words_[4]; union Payload { uint32_t flags; int *vehicle_selection_follow; } payload_; uint32_t trailing_words_[10]; };
struct NetDaemon : Net {
    State lock_state_[24];
    Lock locks_;
    int automated_lock(int *, int);
    int lock_veh(int *id, int target, int a, int b, int c) {
        assert(target == 4 && a == -1 && b == -1 && c == 0);
        assert(NetBusyCount == 1 && lock_state_[2].payload_.flags == 1);
        ++LockCalls;
        Events.push_back(2);
        if (RewriteId) *id = 1;
        return LockCalls <= Failures;
    }
    void process_message(char *message, unsigned long second, int first) {
        assert(reinterpret_cast<intptr_t>(message) == 1234);
        assert(second == 19 && first == 17);
        ++ProcessCalls;
    }
} NetDaemonNet;
void wait_task() {
    ++WaitCalls;
    Events.push_back(4);
    if (AbortOnPump) ExitTurnLoop = AbortOnPump;
}
void check_spock() { ++SpockCalls; }
BODY
void reset() {
    IsMultiplayerNet = 1;
    NetBusyCount = ExitTurnLoop = 0;
    LockCalls = WaitCalls = GetCalls = CheckCalls = SpockCalls = 0;
    PopCalls = CloseCalls = ProcessCalls = Failures = AbortOnPump = 0;
    RewriteId = TimeCalls = 0;
    std::memset(NetDaemonNet.lock_state_, 0, sizeof(NetDaemonNet.lock_state_));
    Vehs[0].order = 128;
    Vehs[1].order = 127;
    Clock.clear();
    Events.clear();
}
int main() {
    reset();
    IsMultiplayerNet = 0;
    int id = 0;
    assert(NetDaemonNet.automated_lock(&id, 4) == 0);
    assert(LockCalls == 0 && TimeCalls == 0 && Events.empty());
    reset();
    ExitTurnLoop = 3;
    Clock = {100};
    assert(NetDaemonNet.automated_lock(&id, 4) == 3);
    assert(LockCalls == 0 && NetBusyCount == 0);
    assert(NetDaemonNet.lock_state_[2].payload_.flags == 0);
    reset();
    RewriteId = 1;
    Clock = {100};
    assert(NetDaemonNet.automated_lock(&id, 4) == 0 && id == 1);
    assert(LockCalls == 1 && NetBusyCount == 0 && PopCalls == 0);
    assert(NetDaemonNet.lock_state_[2].payload_.flags == 0);
    reset();
    id = 0;
    Failures = 1;
    AbortOnPump = 5;
    Clock = {0, 100, 100, 200, 210};
    assert(NetDaemonNet.automated_lock(&id, 4) == 5);
    assert(WaitCalls == 1 && GetCalls == 1 && ProcessCalls == 1);
    assert(CheckCalls == 1 && SpockCalls == 2 && NetBusyCount == 0);
    assert((Events == std::vector<int>{1,2,3,4,5,6,9}));
    reset();
    Failures = 3;
    // A strict >500 warning measured from the original timestamp, even
    // though the independent pacing timestamp advanced after each delay.
    Clock = {0, 250, 250, 250, 500, 500, 500, 501};
    assert(NetDaemonNet.automated_lock(&id, 4) == 0);
    assert(LockCalls == 4 && TimeCalls == 8 && PopCalls == 1 && CloseCalls == 1);
    assert(WaitCalls == 0 && SpockCalls == 3 && NetBusyCount == 0);
    reset();
    Failures = 1;
    // The DWORD subtraction is unsigned and wraps across the SDK tick epoch.
    Clock = {0xFFFFFFF0u, 0x200u};
    assert(NetDaemonNet.automated_lock(&id, 4) == 0);
    assert(PopCalls == 1 && CloseCalls == 1 && NetBusyCount == 0);
}
'''
    unit = tmp_path / "automated_lock.cpp"
    unit.write_text(harness.replace("BODY", body))
    executable = tmp_path / "automated_lock"
    subprocess.run([compiler, "-std=c++11", "-Wall", "-Wextra", str(unit),
                    "-o", str(executable)], check=True, capture_output=True, text=True)
    subprocess.run([str(executable)], check=True, capture_output=True, text=True)
