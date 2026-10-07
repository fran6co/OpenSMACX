from __future__ import annotations

import shutil
import subprocess
import tempfile
from pathlib import Path
import re

from match_recovery.source import read_file


ROOT = Path(__file__).resolve().parents[2]
SOURCE = ROOT / "src/netdaemon.cpp"
SIGNATURE = "static void netdaemon_handle_session_lobby(NetDaemon &daemon,"
HELPER_TOKEN = "/* ACTUAL_HELPER */"

HARNESS = r'''#include <cstdint>
#include <cstring>
#include <cstdio>

struct NetWinLobbyRow {
    uint8_t prefix_, ready_, field_2_;
    int8_t faction_or_color_;
    uint8_t fields_[0x174];
    uint32_t owner_;
};
static_assert(sizeof(NetWinLobbyRow) == 0x17C, "lobby row stride");
struct NetWinFactionRecord { uint32_t prefix_, owner_; uint8_t tail_[0x188]; };
static_assert(sizeof(NetWinFactionRecord) == 0x190, "faction stride");
struct PlayerData {};
struct ProcessSlot { uint32_t pid_; };
struct LockStateEntry { uint32_t leading_words_[4]; };
struct Net {
    int sends = 0;
    unsigned short last_type = 0;
    unsigned long last_target = 0;
    void send(const unsigned char *p, size_t, unsigned long target, int) {
        ++sends;
        std::memcpy(&last_type, p, sizeof(last_type));
        last_target = target;
    }
};
struct NetDaemon : Net {
    LockStateEntry lock_state_[8] = {};
    int player_count_ = 3;
    ProcessSlot process_slots_[8] = {};
    int sent_messages = 0;
    void send_message(char *, unsigned long, int) { ++sent_messages; }
};
struct Session { unsigned int mode_ = 0; };
struct Dialog {
    void *faction_records_ = nullptr;
    int fills = 0, redraws = 0, last_faction = -99;
    void fill_player(PlayerData *, int f) { ++fills; last_faction = f; }
    void on_redraw() { ++redraws; }
};
NetWinLobbyRow NetWinLobbyRows[8];
Session NetWinSessionData;
Dialog NetWinDialog;
NetDaemon NetDaemonNet;

/* ACTUAL_HELPER */

static void put16(unsigned char *p, unsigned int o, uint16_t x) {
    std::memcpy(p + o, &x, 2);
}
static void put32(unsigned char *p, uint32_t x) { std::memcpy(p, &x, 4); }
static int fail(int n, const char *s) {
    std::fprintf(stderr, "actual-source lobby control %d: %s\n", n, s);
    return n;
}
static void reset(unsigned char *m, size_t n) {
    std::memset(m, 0, n);
    std::memset(NetWinLobbyRows, 0, sizeof(NetWinLobbyRows));
    NetWinSessionData.mode_ = 0;
    NetWinDialog.fills = NetWinDialog.redraws = 0;
    NetWinDialog.last_faction = -99;
    NetDaemonNet = NetDaemon{};
    for (int i = 0; i < 8; ++i) NetDaemonNet.process_slots_[i].pid_ = 1000 + i;
    put32(m + 4, 2);
}
int main() {
    unsigned char m[0xA80];
    NetWinFactionRecord records[128] = {};
    NetWinDialog.faction_records_ = records;

    // Actual 1F04/no-flag path copies one row, then notify+redraw.
    reset(m, sizeof(m)); put16(m, 0, 0x1F04);
    NetWinSessionData.mode_ = 2;
    NetDaemonNet.lock_state_[0].leading_words_[1] = 1;
    std::memset(m + 0x10, 0x5A, sizeof(NetWinLobbyRow));
    netdaemon_handle_session_lobby(NetDaemonNet, m, 0x1F04, sizeof(m));
    if (NetWinLobbyRows[2].prefix_ != 0x5A || NetWinLobbyRows[1].prefix_ != 0)
        return fail(1, "single-row 0x17C copy");
    if (NetWinDialog.redraws != 1 || NetDaemonNet.sends != 1 ||
        NetDaemonNet.last_type != 0x0F0E || NetDaemonNet.last_target != 1002)
        return fail(2, "unmarked notification/redraw");

    // Actual 2F04 marked path selection, with faction -1 still storing the
    // packet owner, then send_message and no modal notify/redraw.
    reset(m, sizeof(m)); put16(m, 0, 0x2F04);
    NetWinSessionData.mode_ = 2;
    NetDaemonNet.lock_state_[0].leading_words_[1] = 1;
    NetWinLobbyRows[2].faction_or_color_ = 3;
    NetWinLobbyRows[2].owner_ = 17;
    put32(m + 4, 2); m[0x13] = 0xFF; put32(m + 0x188, 77);
    netdaemon_handle_session_lobby(NetDaemonNet, m, 0x2F04, sizeof(m));
    if (NetWinLobbyRows[2].faction_or_color_ != -1 ||
        NetWinLobbyRows[2].owner_ != 77)
        return fail(3, "-1 faction stores and owner+0x188");
    if (NetDaemonNet.sent_messages != 1 || NetDaemonNet.sends != 0 ||
        NetWinDialog.redraws != 0)
        return fail(4, "marked send skips notification/redraw");

    // Count=1 still scans row 1. A duplicate exits before selected-row writes.
    reset(m, sizeof(m)); put16(m, 0, 0x2F04);
    NetDaemonNet.player_count_ = 1;
    NetWinSessionData.mode_ = 3;
    NetWinLobbyRows[1].faction_or_color_ = 5;
    NetWinLobbyRows[2].faction_or_color_ = 3;
    put32(m + 4, 2); m[0x13] = 5;
    netdaemon_handle_session_lobby(NetDaemonNet, m, 0x2F04, sizeof(m));
    if (NetWinLobbyRows[2].faction_or_color_ != 3 ||
        NetDaemonNet.sent_messages != 0 || NetWinDialog.redraws != 0)
        return fail(5, "count=1 duplicate bound");

    // The native loop includes row==player_count despite its zero-based EAX
    // compare: the equality arm sees the prior index, still below count.
    reset(m, sizeof(m)); put16(m, 0, 0x2F04);
    NetWinSessionData.mode_ = 3;
    NetWinLobbyRows[3].faction_or_color_ = 5;
    NetWinLobbyRows[2].faction_or_color_ = 3;
    put32(m + 4, 2); m[0x13] = 5;
    netdaemon_handle_session_lobby(NetDaemonNet, m, 0x2F04, sizeof(m));
    if (NetWinLobbyRows[2].faction_or_color_ != 3 ||
        NetDaemonNet.sent_messages != 0)
        return fail(7, "row==player_count duplicate bound");

    // Sender -1/no flag copies all seven rows (0xA64), not just one row.
    reset(m, sizeof(m)); put16(m, 0, 0x1F04);
    put32(m + 4, 0xFFFFFFFFu); std::memset(m + 0x10, 0x2A, 0xA64);
    netdaemon_handle_session_lobby(NetDaemonNet, m, 0x1F04, sizeof(m));
    if (NetWinLobbyRows[1].prefix_ != 0x2A ||
        NetWinLobbyRows[7].prefix_ != 0x2A || NetWinDialog.redraws != 1)
        return fail(6, "sentinel 0xA64 copy");

    std::puts("actual-source session-lobby controls passed");
    return 0;
}
'''


def extract_function(source: str, signature: str) -> str:
    start = source.find(signature)
    assert start >= 0 and source.find(signature, start + 1) < 0
    opening = source.find("{", start)
    assert opening >= 0
    depth = 0
    for i in range(opening, len(source)):
        if source[i] == "{":
            depth += 1
        elif source[i] == "}":
            depth -= 1
            if depth == 0:
                return source[start:i + 1]
    raise AssertionError("unbalanced C++ helper braces")


def actual_helper_and_source() -> tuple[str, str]:
    source = SOURCE.read_text()
    helper = extract_function(source, SIGNATURE)
    return helper, source


def compile_and_run(helper: str, compiler: str, expected_success: bool) -> None:
    assert HARNESS.count(HELPER_TOKEN) == 1
    program = HARNESS.replace(HELPER_TOKEN, helper)
    with tempfile.TemporaryDirectory(prefix="osmx-lobby-actual-") as tmp:
        cpp = Path(tmp) / "actual_lobby.cpp"
        exe = Path(tmp) / "actual_lobby"
        cpp.write_text(program)
        built = subprocess.run(
            [compiler, "-std=c++11", "-Wall", "-Wextra", "-Werror",
             str(cpp), "-o", str(exe)], capture_output=True, text=True,
            check=False)
        if not expected_success and built.returncode != 0:
            return
        assert built.returncode == 0, built.stderr
        ran = subprocess.run([str(exe)], capture_output=True, text=True,
                             check=False)
        if expected_success:
            assert ran.returncode == 0, ran.stderr
            assert "actual-source session-lobby controls passed" in ran.stdout
        else:
            assert ran.returncode != 0


def test_real_mapping_spans_and_dispatcher_route_are_retained():
    helper, source = actual_helper_and_source()
    mappings = [item for item in read_file(SOURCE) if item.address == 0x00534400]
    assert len(mappings) == 1
    assert tuple(mappings[0].image_spans) == (
        (0x00534400, 0x005388B9), (0x0065E0C2, 0x0065E442))
    process = extract_function(
        source, "void NetDaemon::process_message(char *message,")
    assert "type == 0x1F04 || type == 0x2F04 || type == 0x4F04" in process
    route = process.index("netdaemon_handle_session_lobby(*this, msg, type, message_size);")
    route_tail = process[route:route + 260]
    assert "netdaemon_process_message_marker_tail(*this, msg);" in route_tail
    assert "return;" in route_tail
    assert helper.count("edx_sender_snapshot") == 0


def test_actual_extracted_helper_runs_under_gcc_and_clang():
    helper, _ = actual_helper_and_source()
    compilers = [name for name in ("g++", "clang++") if shutil.which(name)]
    assert compilers
    for compiler in compilers:
        compile_and_run(helper, compiler, True)


def test_actual_helper_mutants_fail_native_semantic_controls():
    helper, _ = actual_helper_and_source()
    mutants = []
    faction_store = "NetWinLobbyRows[sender].faction_or_color_ ="
    assert faction_store in helper
    mutants.append(helper.replace(
        faction_store,
        "if (faction != -1) NetWinLobbyRows[sender].faction_or_color_ =", 1))
    # Add a redraw immediately after the marked-path send. The control has a
    # modal marker and requires the original direct jump to 0x538266.
    send_stmt = re.compile(
        r"daemon\.send_message\(\s*reinterpret_cast<char \*>\(message\),"
        r"\s*message_size,\s*0\);")
    matches = list(send_stmt.finditer(helper))
    assert len(matches) >= 2
    at = matches[-1].end()
    mutants.append(helper[:at] + "\n        NetWinDialog.on_redraw();" + helper[at:])
    assert "row_index <= NetDaemonNet.player_count_" in helper
    mutants.append(helper.replace(
        "row_index <= NetDaemonNet.player_count_",
        "row_index < NetDaemonNet.player_count_", 1))
    assert "std::memcpy(&NetWinLobbyRows[1], message + 0x10, 0xA64)" in helper
    mutants.append(helper.replace("0xA64", "0x17C", 1))
    assert all(mutant != helper for mutant in mutants)
    for compiler in ("g++", "clang++"):
        if shutil.which(compiler):
            for mutant in mutants:
                compile_and_run(mutant, compiler, False)
