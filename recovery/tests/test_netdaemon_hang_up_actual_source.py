"""Actual-source controls for pinned early returns and callback cleanup ordering."""
from pathlib import Path
import json
import shutil
import subprocess
import tempfile

import pytest


def repository_root():
    for candidate in Path(__file__).resolve().parents:
        if (candidate / "src/netdaemon.cpp").is_file():
            return candidate
    raise AssertionError("could not locate OpenSMACX source root")


ROOT = repository_root()
ACTUAL = ROOT / "src/netdaemon.cpp"
SIGNATURE = "void NetDaemon::hang_up()"


def extract_method(text: str, signature=SIGNATURE) -> str:
    start = text.find(signature)
    assert start >= 0 and text.find(signature, start + len(signature)) < 0
    opening = text.find("{", start + len(signature))
    assert opening >= 0
    depth = 0
    quote = None
    escaped = False
    line_comment = block_comment = False
    i = opening
    while i < len(text):
        ch = text[i]
        nxt = text[i + 1] if i + 1 < len(text) else ""
        if line_comment:
            if ch == "\n":
                line_comment = False
        elif block_comment:
            if ch == "*" and nxt == "/":
                block_comment = False
                i += 1
        elif quote:
            if escaped:
                escaped = False
            elif ch == "\\":
                escaped = True
            elif ch == quote:
                quote = None
        elif ch == "/" and nxt == "/":
            line_comment = True
            i += 1
        elif ch == "/" and nxt == "*":
            block_comment = True
            i += 1
        elif ch in ('"', "'"):
            quote = ch
        elif ch == "{":
            depth += 1
        elif ch == "}":
            depth -= 1
            if depth == 0:
                return text[start:i + 1]
        i += 1
    raise AssertionError("unterminated hang_up definition")


HARNESS = r'''#include <cassert>
#include <cstdint>
#include <iostream>
#include <string>
#include <vector>

static std::vector<std::string> events;
static int IsMultiplayerNet;
static int ConsoleControlTurnActive;
static int mutate_global_state_on_8301;
struct ConsoleFixture { int faction_id_; } ConsoleGlobal;
struct GameFields { unsigned int net_turn_flags_; int net_turn_faction_; };
struct GameStorage { GameFields fields_; } GameDataStateStorage;
struct PlayerLock { void clear() { events.push_back("lock.clear"); } };
struct LockGroup { PlayerLock records_[8]; int field_E0_; int field_E4_; };
struct Channel { unsigned int trailing_words_[10]; };
struct PrimaryState { unsigned int leading_words_[3]; unsigned int flags_; };
struct NetDaemon {
    PrimaryState primary_state_;
    LockGroup locks_;
    Channel channels_[1];
    NetDaemon() : primary_state_{}, locks_{}, channels_{} {}
    void hang_up();
};
static NetDaemon NetDaemonNet;
static void message_data(int type, int, int, int, int, int) {
    events.push_back(std::string("message:") + std::to_string(type));
    if (type == 0x8301 && mutate_global_state_on_8301)
        NetDaemonNet.primary_state_.leading_words_[0] = 1;
}
struct NetMsg { void close() { events.push_back("close"); } } NetMsgPrimary;

METHOD

static std::string run_case(const char *name, int flags, int multiplayer,
                            unsigned int turn_flags, int control_active,
                            int faction, int turn_faction, int global_state,
                            int owner, int callback_mutates) {
    events.clear();
    IsMultiplayerNet = multiplayer;
    ConsoleControlTurnActive = control_active;
    ConsoleGlobal.faction_id_ = faction;
    GameDataStateStorage.fields_.net_turn_flags_ = turn_flags;
    GameDataStateStorage.fields_.net_turn_faction_ = turn_faction;
    mutate_global_state_on_8301 = callback_mutates;
    NetDaemonNet = NetDaemon();
    NetDaemonNet.primary_state_.leading_words_[0] = global_state;
    NetDaemon daemon;
    daemon.primary_state_.flags_ = flags;
    daemon.locks_.field_E0_ = owner;
    daemon.locks_.field_E4_ = 77;
    daemon.channels_[0].trailing_words_[0] = 0xFF;
    daemon.channels_[0].trailing_words_[1 + faction] = 0x04;
    daemon.hang_up();
    std::string joined;
    for (size_t i = 0; i < events.size(); ++i) {
        if (i) joined += ",";
        joined += events[i];
    }
    std::cout << name << "|" << joined << "|" << daemon.primary_state_.flags_
              << "|" << daemon.locks_.field_E0_ << "|" << daemon.locks_.field_E4_
              << "|" << daemon.channels_[0].trailing_words_[0] << "\n";
    return joined;
}
int main() {
    // Exercise the same genuine predicate as a standalone operation as well as
    // through hang_up; the outer flag guard must not hide predicate damage.
    GameDataStateStorage.fields_.net_turn_flags_ = 0;
    ConsoleControlTurnActive = 0;
    ConsoleGlobal.faction_id_ = 2;
    GameDataStateStorage.fields_.net_turn_faction_ = 2;
    assert(sub_52db70() == 0);
    GameDataStateStorage.fields_.net_turn_flags_ = 0x10;
    assert(sub_52db70() == 1);
    GameDataStateStorage.fields_.net_turn_faction_ = 3;
    assert(sub_52db70() == 0);
    GameDataStateStorage.fields_.net_turn_faction_ = 2;
    ConsoleControlTurnActive = 1;
    assert(sub_52db70() == 0);

    run_case("flags", 1, 1, 0, 0, 2, 2, 1, 2, 0);
    run_case("offline", 0, 0, 0, 0, 2, 2, 1, 2, 0);
    run_case("active_close", 0, 1, 0, 0, 2, 2, 0, 2, 0);
    run_case("callback_refresh", 0, 1, 0x10, 0, 2, 2, 0, 2, 1);
    run_case("owner_mismatch", 0, 1, 0, 0, 2, 2, 1, 9, 0);
    run_case("control_turn_no_session", 0, 1, 0x10, 1, 2, 2, 0, 2, 0);
    run_case("foreign_turn", 0, 1, 0x10, 0, 2, 3, 0, 2, 1);
    run_case("control_turn_with_session", 0, 1, 0x10, 1, 2, 2, 1, 2, 0);
    return 0;
}
'''


def run_variant(method: str, compiler_name="g++", predicate=None):
    compiler = shutil.which(compiler_name)
    assert compiler, "host C++ compiler unavailable"
    if predicate is None:
        predicate = extract_method(ACTUAL.read_text(), "int __cdecl sub_52db70()")
    # Only the platform calling-convention spelling is absent from this host.
    predicate = predicate.replace("int __cdecl sub_52db70()", "int sub_52db70()", 1)
    source = HARNESS.replace("METHOD", predicate + "\n\n" + method)
    with tempfile.TemporaryDirectory(prefix="opensmacx-hangup-host-") as tmp:
        cpp = Path(tmp) / "control.cpp"
        exe = Path(tmp) / "control"
        cpp.write_text(source)
        subprocess.run([compiler, "-std=c++17", "-Wall", "-Wextra", "-Werror", str(cpp), "-o", str(exe)], check=True)
        output = subprocess.check_output([str(exe)], text=True)
    rows = {}
    for line in output.splitlines():
        name, events, flags, owner, tail, mask = line.split("|")
        rows[name] = (events.split(",") if events else [], int(flags), int(owner), int(tail), int(mask))
    return rows


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
def test_pinned_early_returns_and_tail_order(compiler_name):
    proposed = run_variant(extract_method(ACTUAL.read_text()), compiler_name)
    assert proposed["flags"] == ([], 1, 2, 77, 255)
    assert proposed["offline"] == ([], 0, 2, 77, 255)
    assert proposed["active_close"] == (["close"], 1, 2, 77, 255)
    assert proposed["callback_refresh"] == (
        ["message:33537", "lock.clear", "message:19200", "close"], 1, 0, 0, 251)
    assert proposed["owner_mismatch"] == (
        ["lock.clear", "message:19200", "close"], 1, 9, 77, 251)
    assert proposed["control_turn_no_session"] == (["close"], 1, 2, 77, 255)
    assert proposed["foreign_turn"] == (["close"], 1, 2, 77, 255)
    assert proposed["control_turn_with_session"] == (
        ["message:33537", "lock.clear", "message:19200", "close"], 1, 0, 0, 251)


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
def test_popup_close_on_early_return_damage_is_detected(compiler_name):
    method = extract_method(ACTUAL.read_text())
    for guard, scenario, flags in (
        ("if (primary_state_.flags_ != 0) return;", "flags", 1),
        ("if (IsMultiplayerNet == 0) return;", "offline", 0),
    ):
        assert method.count(guard) == 1
        damaged = method.replace(guard, guard.replace("return;", "goto L145;"), 1)
        observed = run_variant(damaged, compiler_name)
        assert observed[scenario] != ([], flags, 2, 77, 255)


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
@pytest.mark.parametrize("needle,replacement", (
    ("net_turn_flags_ & 0x10) == 0", "net_turn_flags_ & 0x10) != 0"),
    ("ConsoleControlTurnActive != 0", "ConsoleControlTurnActive == 0"),
    ("net_turn_faction_ == ConsoleGlobal.faction_id_", "net_turn_faction_ != ConsoleGlobal.faction_id_"),
))
def test_actual_predicate_damage_is_detected(compiler_name, needle, replacement):
    text = ACTUAL.read_text()
    predicate = extract_method(text, "int __cdecl sub_52db70()")
    assert predicate.count(needle) == 1
    damaged = predicate.replace(needle, replacement, 1)
    with pytest.raises(subprocess.CalledProcessError):
        run_variant(extract_method(text), compiler_name, damaged)
