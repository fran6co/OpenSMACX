"""Host controls for the installed Spock notification/session helpers.

The test extracts both real definitions from their canonical owner on every
run. Narrow doubles exercise source behavior only; they do not prove VC6 ABI,
native layout, or runtime equivalence to the pinned executable.
"""
from __future__ import annotations

import shutil
import re
import subprocess
import tempfile
from pathlib import Path

from test_net_hotseat_owner_actual_source_oct6 import extract_definition


ROOT = Path(__file__).resolve().parents[2]
NEW_SOURCE = ROOT / "src/netdaemon_spock.cpp"
OLD_SOURCE = ROOT / "src/netdaemon.cpp"
CHECK_SIGNATURE = "void __cdecl check_spock() {"
HELP_SIGNATURE = "void __cdecl help_spock() {"
CHECK_TOKEN = "/* ACTUAL_CHECK_SPOCK */"
HELP_TOKEN = "/* ACTUAL_HELP_SPOCK */"


HARNESS = r'''#include <cstdio>
#include <cstring>

#define __cdecl
enum { ACTIVE_SET = 1, LOCK_EVENT, SYNC_EVENT, RECEIVE_EVENT, PENDING_CLEAR, ACTIVE_CLEAR };
struct Event { int kind; int value; int args[7]; };
static Event events[128];
static int event_count;
static int receive_values[32];
static int receive_count;
static int receive_index;
static int lock_result;
static int lock_calls;
static int mutate_leading_in_lock;

static void record(int kind, int value = 0, const int *args = 0) {
    if (event_count >= 128) return;
    Event &e = events[event_count++]; e.kind = kind; e.value = value;
    for (int i = 0; i != 7; ++i) e.args[i] = args ? args[i] : 0;
}

struct Flag {
    int value;
    int event;
    Flag(int v = 0, int e = 0) : value(v), event(e) {}
    operator int() const { return value; }
    Flag &operator=(int v) {
        value = v;
        if (event) record(v == 1 && event == ACTIVE_CLEAR ? ACTIVE_SET : event, v);
        return *this;
    }
};
Flag SpockNotifyPending(0, PENDING_CLEAR);
Flag SpockHelpActive(0, ACTIVE_CLEAR);
int UnkGlobal0093A934;
int ConsoleControlTurnActive;

struct PrimaryState { int leading_words_[1]; };
struct LockState {
    int any_locks();
};
struct NetDaemon {
    PrimaryState primary_state_;
    LockState locks_;
    void synch(int op, int a, int b, int c, int d, int e, int f) {
        int args[7] = {op, a, b, c, d, e, f};
        record(SYNC_EVENT, op, args);
    }
    int receive() {
        record(RECEIVE_EVENT);
        if (receive_index < receive_count) return receive_values[receive_index++];
        return 0;
    }
} NetDaemonNet;
int LockState::any_locks() {
    ++lock_calls;
    record(LOCK_EVENT);
    if (mutate_leading_in_lock) NetDaemonNet.primary_state_.leading_words_[0] = 0;
    return lock_result;
}
void __cdecl help_spock();

/* ACTUAL_CHECK_SPOCK */
/* ACTUAL_HELP_SPOCK */

static int fail(int n, const char *why) {
    std::fprintf(stderr, "actual Spock source control %d: %s\n", n, why);
    return 97;
}
static void reset() {
    event_count = receive_count = receive_index = lock_calls = 0;
    lock_result = mutate_leading_in_lock = 0;
    SpockNotifyPending.value = 1; SpockHelpActive.value = 0;
    UnkGlobal0093A934 = ConsoleControlTurnActive = 0;
    NetDaemonNet.primary_state_.leading_words_[0] = 1;
}
static int check_single_gate(int which, int gate_value, int expected_notify,
                             int expected_active) {
    reset();
    if (which == 0) NetDaemonNet.primary_state_.leading_words_[0] = gate_value;
    if (which == 1) SpockNotifyPending.value = gate_value;
    if (which == 2) UnkGlobal0093A934 = gate_value;
    if (which == 3) ConsoleControlTurnActive = gate_value;
    check_spock();
    if (event_count != 0 || lock_calls != 0) return 0;
    return SpockNotifyPending.value == expected_notify &&
           SpockHelpActive.value == expected_active;
}
static int exact_success_trace() {
    reset();
    receive_values[0]=7; receive_values[1]=0;
    receive_values[2]=0;
    receive_values[3]=3; receive_values[4]=2; receive_values[5]=0;
    receive_values[6]=0;
    receive_count=7;
    check_spock();
    const int ops[4] = {0x25, 0x12, 0x26, 0x14};
    const int drains[4] = {2, 1, 3, 1};
    if (lock_calls != 1 || SpockNotifyPending.value != 0 || SpockHelpActive.value != 0)
        return 0;
    int pos = 0;
    if (events[pos++].kind != ACTIVE_SET || pos >= event_count ||
        events[pos++].kind != LOCK_EVENT) return 0;
    for (int sweep=0; sweep<4; ++sweep) {
        if (pos >= event_count || events[pos].kind != SYNC_EVENT) return 0;
        Event sync = events[pos++];
        if (sync.value != ops[sweep]) return 0;
        const int want[7] = {ops[sweep], 0, 0, 0, 0, 1, 0x2101};
        for (int i=0; i<7; ++i) if (sync.args[i] != want[i]) return 0;
        for (int n=0; n<drains[sweep]; ++n) {
            if (pos >= event_count || events[pos++].kind != RECEIVE_EVENT) return 0;
        }
    }
    if (pos + 2 != event_count || events[pos].kind != PENDING_CLEAR ||
        events[pos].value != 0 || events[pos+1].kind != ACTIVE_CLEAR ||
        events[pos+1].value != 0) return 0;
    return receive_index == receive_count;
}
int main() {
    if (!check_single_gate(0, 0, 1, 0)) return fail(1, "primary-state gate");
    if (!check_single_gate(1, 0, 0, 0)) return fail(2, "pending gate");
    if (!check_single_gate(2, 1, 1, 0)) return fail(3, "energy-event gate");
    if (!check_single_gate(3, 1, 1, 0)) return fail(4, "turn-active gate");

    reset(); lock_result = 1; check_spock();
    if (lock_calls != 1 || event_count != 2 || events[0].kind != ACTIVE_SET ||
        events[1].kind != LOCK_EVENT ||
        SpockNotifyPending.value != 1 || SpockHelpActive.value != 1)
        return fail(5, "active is set before lock; locked path preserves both flags");

    if (!exact_success_trace()) return fail(6, "four synch/receive sweeps and clear order");

    // The entry gate can pass and then the leading word can change before the
    // helper rechecks it. The helper must still clear both flags, without IO.
    reset(); mutate_leading_in_lock = 1; check_spock();
    if (lock_calls != 1 || event_count != 4 || events[0].kind != ACTIVE_SET ||
        events[1].kind != LOCK_EVENT || events[2].kind != PENDING_CLEAR ||
        events[3].kind != ACTIVE_CLEAR ||
        SpockNotifyPending.value != 0 || SpockHelpActive.value != 0)
        return fail(7, "helper rechecks leading state then clears outside guard");

    reset(); NetDaemonNet.primary_state_.leading_words_[0] = 0;
    help_spock();
    if (event_count != 2 || events[0].kind != PENDING_CLEAR ||
        events[1].kind != ACTIVE_CLEAR || SpockNotifyPending.value != 0 ||
        SpockHelpActive.value != 0)
        return fail(8, "direct helper early-state path still clears both flags");

    std::puts("actual-source Spock controls passed");
    return 0;
}
'''


def actual_sources() -> tuple[str, str, str, str]:
    old = OLD_SOURCE.read_text(encoding="utf-8")
    new = NEW_SOURCE.read_text(encoding="utf-8")
    check = extract_definition(new, CHECK_SIGNATURE)
    help_body = extract_definition(new, HELP_SIGNATURE)
    return old, new, check, help_body


def source_ownership() -> None:
    old, new, _check, _help = actual_sources()
    for address, signature in (("0051F1B0", CHECK_SIGNATURE), ("0051F0C0", HELP_SIGNATURE)):
        marker = f"// ORIGINAL: 0x{address}"
        assert new.count(marker) == 1
        assert old.count(marker) == 0
        assert new.count(signature) == 1
        assert old.count(signature) == 0
    # The two scalar storage identities and original definitions remain with
    # the NetDaemon state owner; only the method bodies moved.
    for marker, declaration in (
        ("0x0093F670 ?SpockNotifyPending@@3HA", r"(?m)^int SpockNotifyPending;"),
        ("0x0093F674 ?SpockHelpActive@@3HA", r"(?m)^int SpockHelpActive;"),
    ):
        assert old.count(marker) == 1 and len(re.findall(declaration, old)) == 1
        assert marker not in new and re.search(declaration, new) is None


def compile_and_run(check: str, help_body: str, compiler: str,
                    expected_success: bool, failure: str = "") -> None:
    assert HARNESS.count(CHECK_TOKEN) == HARNESS.count(HELP_TOKEN) == 1
    program = HARNESS.replace(CHECK_TOKEN, check).replace(HELP_TOKEN, help_body)
    with tempfile.TemporaryDirectory(prefix="opensmacx-spock-actual-") as tmp:
        cpp = Path(tmp) / "spock_actual.cpp"
        exe = Path(tmp) / "spock_actual"
        cpp.write_text(program, encoding="utf-8")
        built = subprocess.run(
            [compiler, "-std=c++11", "-Wall", "-Wextra", "-Werror", str(cpp), "-o", str(exe)],
            capture_output=True, text=True, check=False)
        assert built.returncode == 0, built.stderr
        ran = subprocess.run([str(exe)], capture_output=True, text=True, check=False)
        if expected_success:
            assert ran.returncode == 0, ran.stdout + ran.stderr
            assert "actual-source Spock controls passed" in ran.stdout
        else:
            assert ran.returncode == 97, ran.stdout + ran.stderr
            assert failure in ran.stderr


def test_moved_owner_annotations_and_flag_storage_are_separate():
    source_ownership()


def test_actual_extracted_bodies_pass_gcc_and_clang_controls():
    _old, _new, check, help_body = actual_sources()
    compilers = [name for name in ("g++", "clang++") if shutil.which(name)]
    assert compilers
    for compiler in compilers:
        compile_and_run(check, help_body, compiler, True)


def test_compiled_runtime_mutants_fail_guard_order_opcode_and_clear_controls():
    _old, _new, check, help_body = actual_sources()
    mutants: list[tuple[str, str, str]] = []
    guard = "UnkGlobal0093A934 == 0"
    assert check.count(guard) == 1
    mutants.append((check.replace(guard, "UnkGlobal0093A934 != 0", 1), help_body,
                    "energy-event gate"))

    opcode = "NetDaemonNet.synch(0x12, 0, 0, 0, 0, 1, 0x2101);"
    assert help_body.count(opcode) == 1
    mutants.append((check, help_body.replace(opcode,
                    "NetDaemonNet.synch(0x13, 0, 0, 0, 0, 1, 0x2101);", 1),
                    "four synch/receive sweeps"))

    first = "NetDaemonNet.synch(0x25, 0, 0, 0, 0, 1, 0x2101);"
    second = "NetDaemonNet.synch(0x12, 0, 0, 0, 0, 1, 0x2101);"
    assert help_body.count(first) == help_body.count(second) == 1
    swapped = help_body.replace(first, "NetDaemonNet.synch(0x7F, 0, 0, 0, 0, 1, 0x2101);", 1)
    swapped = swapped.replace(second, first, 1).replace(
        "NetDaemonNet.synch(0x7F, 0, 0, 0, 0, 1, 0x2101);", second, 1)
    mutants.append((check, swapped, "four synch/receive sweeps"))

    clears = "    SpockNotifyPending = 0;\n    SpockHelpActive = 0;"
    assert help_body.count(clears) == 1
    mutants.append((check, help_body.replace(clears,
                    "    SpockHelpActive = 0;\n    SpockNotifyPending = 0;", 1),
                    "four synch/receive sweeps and clear order"))

    compilers = [name for name in ("g++", "clang++") if shutil.which(name)]
    assert compilers
    for compiler in compilers:
        for mutant_check, mutant_help, failure in mutants:
            compile_and_run(mutant_check, mutant_help, compiler, False, failure)
