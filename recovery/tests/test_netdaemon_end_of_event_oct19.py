"""Actual end_of_event results and side-effect ordering under C++98."""
from pathlib import Path
import shutil
import subprocess

import pytest

from test_listbox_dimensions_actual_source import extract_definition


ROOT = Path(__file__).resolve().parents[2]
SOURCE = ROOT / "src/netdaemon.cpp"
SIGNATURE = "int NetDaemon::end_of_event(int faction_id, int turn_value)"

SUPPORT = r'''
#include <cassert>
#include <string>
#include <vector>
static std::vector<std::string> calls;
struct Word;
static Word *watched;
struct Word {
    int value;
    Word &operator=(int assigned) {
        value = assigned;
        if (this == watched) calls.push_back("clear");
        return *this;
    }
    operator int() const { return value; }
};
struct Words { Word leading_words_[4]; Word trailing_words_[10]; };
struct DeletionList {
    void set_sequence_bits(int value);
};
struct NetDaemon {
    Words primary_state_;
    Words channels_[2];
    DeletionList deletion_list_;
    int end_of_event(int faction_id, int turn_value);
};
static NetDaemon *owner;
static int faction;
struct Console { int faction_id_; } ConsoleGlobal;
static int send_result;
static bool change_faction_on_stamp;
void DeletionList::set_sequence_bits(int value) {
    assert(value == 42);
    calls.push_back("sequence");
    if (change_faction_on_stamp) ConsoleGlobal.faction_id_ = faction;
}
static void log_say(const char *text, int, int, int) {
    assert(std::string(text) == "Remaining Locked2");
    calls.push_back("log");
}
static int message_data(int opcode, int, int, int, int, int) {
    assert(opcode == 0x2214);
    assert(owner->channels_[1].trailing_words_[2] == 0);
    assert(calls.size() == 3);
    assert(calls[0] == "sequence" && calls[1] == "log" && calls[2] == "clear");
    calls.push_back("send");
    return send_result;
}
/* PRODUCT */
static void reset(NetDaemon &daemon) {
    watched = 0;
    for (int i=0; i<4; ++i) daemon.primary_state_.leading_words_[i] = 0;
    for (int c=0; c<2; ++c) {
        for (int i=0; i<4; ++i) daemon.channels_[c].leading_words_[i] = 0;
        for (int i=0; i<10; ++i) daemon.channels_[c].trailing_words_[i] = 0;
    }
    calls.clear(); owner = &daemon; faction = 0; send_result = 0;
    change_faction_on_stamp = false; ConsoleGlobal.faction_id_ = 0;
    watched = &daemon.channels_[1].trailing_words_[2];
}
int main() {
    NetDaemon daemon;

    // The sequence stamp precedes the one current-faction snapshot.
    reset(daemon); change_faction_on_stamp = true; faction = -7;
    assert(daemon.end_of_event(12, 42) == -7);
    assert(calls.size() == 1 && calls[0] == "sequence");

    // A nonzero sequence marker suppresses that update; mismatch returns the
    // current signed faction value unchanged.
    reset(daemon); daemon.primary_state_.leading_words_[0] = 1;
    ConsoleGlobal.faction_id_ = -19;
    assert(daemon.end_of_event(12, 42) == -19);
    assert(calls.empty());

    // Matching faction with loaded-zero channel flag returns zero and clears
    // the separate 1BB0 state without logging or sending.
    reset(daemon); ConsoleGlobal.faction_id_ = 3;
    daemon.channels_[1].leading_words_[3] = 77;
    assert(daemon.end_of_event(3, 42) == 0);
    assert(daemon.channels_[1].leading_words_[3] == 0);
    assert(calls.size() == 1 && calls[0] == "sequence");

    // Nonzero lock path logs, clears before send, and returns signed send EAX.
    reset(daemon); ConsoleGlobal.faction_id_ = 5;
    daemon.channels_[1].trailing_words_[2] = 1;
    send_result = -23;
    calls.clear();
    assert(daemon.end_of_event(5, 42) == -23);
    assert(daemon.channels_[1].trailing_words_[2] == 0);
    assert(calls.size() == 4);
    assert(calls[0] == "sequence" && calls[1] == "log");
    assert(calls[2] == "clear" && calls[3] == "send");
}
'''


def run_body(tmp_path: Path, body: str, compiler: str) -> subprocess.CompletedProcess:
    tmp_path.mkdir()
    source = tmp_path / "end_of_event.cpp"
    binary = tmp_path / "end_of_event"
    source.write_text(SUPPORT.replace("/* PRODUCT */", body))
    subprocess.run([compiler, "-std=c++98", "-Wall", "-Wextra", "-Werror",
                    str(source), "-o", str(binary)], check=True,
                   capture_output=True, text=True, timeout=30)
    return subprocess.run([str(binary)], capture_output=True, text=True, timeout=10)


def assert_single_faction_snapshot(body: str) -> None:
    assert body.count("ConsoleGlobal.faction_id_") == 1


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
def test_actual_end_of_event_results_and_order(compiler_name, tmp_path):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(compiler_name + " is unavailable")
    body = extract_definition(SOURCE.read_text(), SIGNATURE)
    assert_single_faction_snapshot(body)
    assert run_body(tmp_path / "actual", body, compiler).returncode == 0

    forced_zero = body.replace("return result;", "return 0;")
    assert forced_zero != body
    assert run_body(tmp_path / "forced-zero", forced_zero, compiler).returncode != 0

    wrong_store_order = body.replace(
        "channels_[1].trailing_words_[2] = 0;\n            result = message_data",
        "result = message_data", 1,
    ).replace(
        "result = message_data(0x2214, 0, 0, 0, 0, 0);",
        "result = message_data(0x2214, 0, 0, 0, 0, 0);\n            channels_[1].trailing_words_[2] = 0;",
        1,
    )
    assert wrong_store_order != body
    assert run_body(tmp_path / "wrong-store-order", wrong_store_order, compiler).returncode != 0

    multiple_read = body.replace(
        "int result = ConsoleGlobal.faction_id_;",
        "int result = ConsoleGlobal.faction_id_;\n    (void)ConsoleGlobal.faction_id_;", 1,
    )
    with pytest.raises(AssertionError):
        assert_single_faction_snapshot(multiple_read)
