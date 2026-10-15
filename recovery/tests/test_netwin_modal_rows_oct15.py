"""Actual-source controls for both NetWin modal readiness-row walks."""
from pathlib import Path
import shutil
import subprocess

import pytest


ROOT = Path(__file__).resolve().parents[2]


def extract_body(path: Path, signature: str) -> str:
    source = path.read_text()
    start = source.index(signature)
    opening = source.index("{", start)
    depth = 0
    for end in range(opening, len(source)):
        if source[end] == "{":
            depth += 1
        elif source[end] == "}":
            depth -= 1
            if depth == 0:
                return source[start : end + 1]
    raise AssertionError(f"unterminated production body: {signature}")


INSTANCE_BODY = extract_body(ROOT / "src/netwin.cpp", "int NetWin::modal_callback()")
DAEMON_BODY = extract_body(ROOT / "src/netwin.cpp", "int __cdecl NetWin::modal_callback_daemon()")


HARNESS = r'''
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <vector>

#define __cdecl
enum { ReceiveEvent = 100, InstanceRedraw = 510, DaemonRedraw = 520 };
std::vector<int> Events;
int ExitTurnLoop;

struct NetWinLobbyRow {
    uint8_t prefix_;
    uint8_t ready_or_occupied_;
    uint8_t field_2_;
    int8_t faction_or_color_;
    uint8_t field_4_;
    uint8_t names_[0x170];
    uint8_t tail_[3];
    uint32_t owner_;
};
typedef char production_lobby_row_stride[(sizeof(NetWinLobbyRow) == 0x17C) ? 1 : -1];
typedef char production_ready_byte_offset[
    (offsetof(NetWinLobbyRow, ready_or_occupied_) == 1) ? 1 : -1];
NetWinLobbyRow NetWinLobbyRows[8];

struct NetDaemon {
    int player_count_;
    int receive_results[16];
    int receive_count;
    int receive_cursor;
    int receive() {
        Events.push_back(ReceiveEvent);
        if (receive_cursor < receive_count)
            return receive_results[receive_cursor++];
        return 0;
    }
} NetDaemonNet;

class NetWin {
public:
    int field_7614_;
    int field_7618_;
    int id_;
    int modal_callback();
    static int __cdecl modal_callback_daemon();
    void on_redraw() { Events.push_back(id_ == 1 ? InstanceRedraw : DaemonRedraw); }
};
NetWin NetWinDialog;

void reset_rows() {
    for (int row = 0; row < 8; ++row) {
        NetWinLobbyRows[row].prefix_ = 0xA0 + row;
        NetWinLobbyRows[row].ready_or_occupied_ = 1;
        // Set the adjacent byte to the opposite value so accidental raw
        // reads of field_2_ cannot pass the ready-byte oracle.
        NetWinLobbyRows[row].field_2_ = 0;
        NetWinLobbyRows[row].faction_or_color_ = static_cast<int8_t>(row);
        NetWinLobbyRows[row].field_4_ = 0x5A;
        NetWinLobbyRows[row].owner_ = 0x10203040u + row;
    }
}
void reset_common() {
    Events.clear();
    ExitTurnLoop = 0;
    NetDaemonNet.player_count_ = 0;
    NetDaemonNet.receive_count = 0;
    NetDaemonNet.receive_cursor = 0;
    NetWinDialog.field_7614_ = 0;
    NetWinDialog.field_7618_ = 0;
    NetWinDialog.id_ = 2;
    reset_rows();
}
void set_receive_script(const int *values, int count) {
    NetDaemonNet.receive_count = count;
    NetDaemonNet.receive_cursor = 0;
    for (int i = 0; i < count; ++i) NetDaemonNet.receive_results[i] = values[i];
}
void require(bool value) { if (!value) exit(1); }
'''


def complete_harness() -> str:
    return (
        HARNESS
        + "\n"
        + INSTANCE_BODY
        + "\n"
        + DAEMON_BODY
        + r'''

static int run_instance(int player_count) {
    reset_common();
    NetWin receiver;
    receiver.field_7614_ = 0;
    receiver.field_7618_ = 0;
    receiver.id_ = 1;
    NetDaemonNet.player_count_ = player_count;
    return receiver.modal_callback();
}
static int run_daemon(int player_count) {
    reset_common();
    NetDaemonNet.player_count_ = player_count;
    return NetWin::modal_callback_daemon();
}

void check_count_domains_and_named_ready_byte() {
    // 0 and negative player counts skip the row walk and preserve the image's
    // existing state transitions; there is deliberately no new range guard.
    require(run_instance(0) == 0);
    require(run_instance(-1) == 0);
    require(run_daemon(0) == 0);
    require(run_daemon(-1) == 0);

    reset_common();
    NetDaemonNet.player_count_ = 1;
    NetWinLobbyRows[1].ready_or_occupied_ = 1;
    require(run_instance(1) == 0);
    require(Events.size() == 1 && Events[0] == ReceiveEvent);
    require(run_daemon(1) == 0);

    reset_common();
    NetDaemonNet.player_count_ = 1;
    NetWinLobbyRows[1].ready_or_occupied_ = 0;
    NetWin receiver;
    receiver.field_7614_ = 0;
    receiver.field_7618_ = 0;
    receiver.id_ = 1;
    require(receiver.modal_callback() == 1);
    require(receiver.field_7618_ == 0 && receiver.field_7614_ == 0);
    reset_common();
    NetDaemonNet.player_count_ = 1;
    NetWinLobbyRows[1].ready_or_occupied_ = 0;
    ExitTurnLoop = 1;
    require(NetWin::modal_callback_daemon() == 0);
    require(NetWinDialog.field_7618_ == 0 && NetWinDialog.field_7614_ == 0);

    reset_common();
    NetDaemonNet.player_count_ = 7;
    require(run_instance(7) == 0);
    require(run_daemon(7) == 0);

    // A zero in exactly row 2 must be found by both the inclusive live bound
    // and the seven-row countdown.
    reset_common();
    NetDaemonNet.player_count_ = 2;
    NetWinLobbyRows[2].ready_or_occupied_ = 0;
    receiver.field_7614_ = 0;
    receiver.field_7618_ = 0;
    receiver.id_ = 1;
    require(receiver.modal_callback() == 1);
    reset_common();
    NetDaemonNet.player_count_ = 2;
    NetWinLobbyRows[2].ready_or_occupied_ = 0;
    ExitTurnLoop = 1;
    require(NetWin::modal_callback_daemon() == 0);
    require(NetWinDialog.field_7618_ == 0);
}

void check_receive_drain_and_redraw_order() {
    const int one_then_empty[] = {1, 0};
    reset_common();
    NetWin receiver;
    receiver.field_7614_ = 1;
    receiver.field_7618_ = 0;
    receiver.id_ = 1;
    set_receive_script(one_then_empty, 2);
    require(receiver.modal_callback() == 0);
    require(Events.size() == 3 && Events[0] == ReceiveEvent &&
            Events[1] == ReceiveEvent && Events[2] == InstanceRedraw);

    const int drain_to_zero[] = {1, 1, 1, 0};
    reset_common();
    NetWinDialog.field_7614_ = 1;
    set_receive_script(drain_to_zero, 4);
    require(NetWin::modal_callback_daemon() == 0);
    require(Events.size() == 5 && Events[0] == ReceiveEvent &&
            Events[1] == ReceiveEvent && Events[2] == ReceiveEvent &&
            Events[3] == ReceiveEvent && Events[4] == DaemonRedraw);

    reset_common();
    NetWinDialog.field_7614_ = 1;
    const int no_receive[] = {0};
    set_receive_script(no_receive, 1);
    require(NetWin::modal_callback_daemon() == 0);
    require(Events.size() == 1 && Events[0] == ReceiveEvent);
}

int main() {
    check_count_domains_and_named_ready_byte();
    check_receive_drain_and_redraw_order();
    return 0;
}
'''
    )


def compile_and_run(compiler: str, source: str, tmp_path: Path):
    cpp = tmp_path / "netwin_modal_rows.cpp"
    executable = tmp_path / "netwin_modal_rows"
    cpp.write_text(source)
    built = subprocess.run(
        [compiler, "-std=c++98", "-Wall", "-Wextra", "-Werror", "-Wno-unused-parameter", str(cpp), "-o", str(executable)],
        capture_output=True,
        text=True,
        timeout=30,
    )
    assert built.returncode == 0, built.stdout + built.stderr
    return subprocess.run([str(executable)], capture_output=True, text=True, timeout=10)


COMPILERS = [compiler for compiler in (shutil.which("g++"), shutil.which("clang++")) if compiler]


def test_production_row_definition_and_modal_loop_contracts():
    header = (ROOT / "src/netwin.h").read_text()
    assert "uint8_t ready_or_occupied_;       // +1" in header
    assert 'static_assert(sizeof(NetWinLobbyRow) == 0x17C' in header
    assert "NetWinLobbyRow *record = &NetWinLobbyRows[1];" in INSTANCE_BODY
    assert "record->ready_or_occupied_ == 0" in INSTANCE_BODY
    assert "int index = 1;" in INSTANCE_BODY
    assert "while (index <= NetDaemonNet.player_count_);" in INSTANCE_BODY
    assert "NetWinLobbyRow *record = &NetWinLobbyRows[1];" in DAEMON_BODY
    assert "int count = NetDaemonNet.player_count_;" in DAEMON_BODY
    assert "while (--count != 0);" in DAEMON_BODY
    assert "reinterpret_cast<char *>(&NetWinLobbyRows[1].ready_or_occupied_)" not in (
        INSTANCE_BODY + DAEMON_BODY
    )


@pytest.mark.parametrize("compiler", COMPILERS or [None])
def test_actual_netwin_modal_callbacks_counts_ready_bytes_and_receive_order(compiler, tmp_path):
    if compiler is None:
        pytest.skip("host C++ compiler unavailable")
    result = compile_and_run(compiler, complete_harness(), tmp_path)
    assert result.returncode == 0, result.stdout + result.stderr


@pytest.mark.parametrize("compiler", COMPILERS or [None])
@pytest.mark.parametrize(
    "name,mutate",
    (
        (
            "read_adjacent_field",
            lambda source: source.replace(
                "record->ready_or_occupied_ == 0", "record->field_2_ == 0", 1
            ),
        ),
        (
            "skip_second_instance_row",
            lambda source: source.replace(
                "while (index <= NetDaemonNet.player_count_);",
                "while (index < NetDaemonNet.player_count_);",
                1,
            ),
        ),
        (
            "stop_daemon_countdown_early",
            lambda source: source.replace("while (--count != 0);", "while (--count > 1);", 1),
        ),
        (
            "omit_daemon_redraw",
            lambda source: source.replace("NetWinDialog.on_redraw();", "(void)NetWinDialog;", 1),
        ),
    ),
)
def test_actual_netwin_modal_damage_controls_compile_then_fail(
    compiler, name, mutate, tmp_path
):
    if compiler is None:
        pytest.skip("host C++ compiler unavailable")
    body = complete_harness()
    damaged = mutate(body)
    assert damaged != body, f"{name} mutation anchor missing"
    result = compile_and_run(compiler, damaged, tmp_path)
    assert result.returncode != 0, name
