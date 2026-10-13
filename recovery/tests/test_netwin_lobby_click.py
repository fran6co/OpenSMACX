"""Actual lobby-click controls for original-defined packet fields and payloads.

Untouched header/tail bytes are copied as object representation, never assigned
an expected value. These host doubles do not prove native ABI or stack contents.
"""

import subprocess
from pathlib import Path
from owner_transfer_test_support import assert_current_owner, assert_leaf_collection_retired
from test_net_hotseat_owner_actual_source_oct6 import installed_packet_declarations


ROOT = Path(__file__).resolve().parents[2]


def _braced(source: str, prefix: str) -> str:
    start = source.index(prefix)
    opening = source.index("{", start)
    depth = 0
    for index in range(opening, len(source)):
        if source[index] == "{":
            depth += 1
        elif source[index] == "}":
            depth -= 1
            if depth == 0:
                return source[start : index + 1]
    raise AssertionError(f"unclosed {prefix}")


def test_netwin_lobby_click_packets_and_branches(tmp_path: Path) -> None:
    assert_leaf_collection_retired(ROOT)
    assert_current_owner(ROOT, 0x0047B450, "src/netwin.cpp")
    source = (ROOT / "src/netwin.cpp").read_text()
    declarations = installed_packet_declarations()
    body = _braced(source, "void NetWin::on_button_clicked(int clicked_id)")
    assert source.count("ORIGINAL: 0x0047B450") == 1
    harness = r"""
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <vector>
#define __fastcall
using std::uint8_t;
using std::uint16_t;
using std::uint32_t;
using std::int8_t;
using std::int32_t;
""" + declarations + r"""
static_assert(sizeof(NetWinLobbyRow) == 0x17C);
static_assert(offsetof(NetWinLobbyRow, owner_) == 0x178);
static_assert(sizeof(NetWinLobbyFullPacket) == 0xA74);
static_assert(sizeof(NetWinLobbySinglePacket) == 0x18C);
static_assert(sizeof(NetWinLobbyLeavePacket) == 0x20);
static_assert(sizeof(NetWinSessionOptionsState) == 0x18);
static_assert(offsetof(NetWinSessionOptionsState, rule_bits_a_) == 0xC);
static_assert(sizeof(NetWinFactionRecord) == 0x190);
static_assert(offsetof(NetWinFactionRecord, owner_) == 4);
NetWinLobbyRow NetWinLobbyRows[8] = {};
NetWinSessionBlock NetWinSessionData = {};
NetWinSessionOptionsState NetWinSessionOptions = {};
int NetWinPlayerId = 7;
int NetWinOtherPlayerId = 8;
char *ScriptPopupFile = nullptr;
int popup_result = 0;
int popup_calls = 0;
int X_pop(char *, const char *, int) {
    ++popup_calls;
    return popup_result;
}
struct Net {
    std::vector<uint8_t> sent;
    int recipient = -1;
    int send(const void *message, int length, int target, int) {
        sent.resize(length);
        std::memcpy(sent.data(), message, length);
        recipient = target;
        return length;
    }
};
struct Daemon : Net {
    int player_count_ = 0;
    struct Entry { uint32_t leading_words_[4]; union Payload { uint32_t flags; int *vehicle_selection_follow; } payload_; uint32_t trailing_words_[10]; } channels_[2];
    struct { uint32_t leading_words_[3], flags_, trailing_words_[10]; } primary_state_;
    int pid_2_idx(int) { return 1; }
} NetDaemonNet;
struct NetWin {
    int field_7614_ = 0;
    void *faction_records_ = nullptr;
    void on_button_clicked(int clicked_id);
};
""" + body + r"""
int main() {
    auto packet_word = [](int offset) {
        uint32_t value;
        std::memcpy(&value, NetDaemonNet.sent.data() + offset, sizeof(value));
        return value;
    };
    NetWin win;
    win.on_button_clicked(-2);
    assert(win.field_7614_ == -1);
    win.field_7614_ = 0;
    win.on_button_clicked(9);
    assert(NetDaemonNet.sent.empty());

    NetWinLobbyRows[1].ready_or_occupied_ = 0;
    NetWinLobbyRows[1].faction_or_color_ = 3;
    NetWinLobbyRows[1].owner_ = 0x12345678;
    win.on_button_clicked(10);
    assert(NetDaemonNet.sent.size() == 0x18C);
    assert(NetDaemonNet.sent[0] == 4 && NetDaemonNet.sent[1] == 0x2F);
    assert(NetDaemonNet.sent[0x11] == 1);
    assert(NetDaemonNet.sent[0x13] == 3);
    assert(packet_word(4) == 1 && packet_word(8) == 0);
    NetWinLobbyRow expected_row = NetWinLobbyRows[1];
    expected_row.ready_or_occupied_ = 1;
    assert(std::memcmp(NetDaemonNet.sent.data() + 0x10,
                       &expected_row, sizeof(expected_row)) == 0);
    assert(NetWinLobbyRows[1].ready_or_occupied_ == 0);
    assert(NetDaemonNet.recipient == NetWinOtherPlayerId);

    NetDaemonNet.primary_state_.leading_words_[0] = 1;
    NetWinSessionData.mode_ = 0;
    win.on_button_clicked(10);
    assert(NetDaemonNet.sent.size() == 0xA74);
    assert(NetDaemonNet.sent[0] == 4 && NetDaemonNet.sent[1] == 0x4F);
    assert(NetDaemonNet.sent[0x13] == 3);
    assert(packet_word(4) == 0xFFFFFFFFu && packet_word(8) == 0);
    assert(std::memcmp(NetDaemonNet.sent.data() + 0x10,
                       &NetWinLobbyRows[1], 7 * sizeof(NetWinLobbyRow)) == 0);
    assert(NetDaemonNet.recipient == 0);

    uint8_t claims[8 * 0x190] = {};
    *reinterpret_cast<uint32_t *>(claims + 0x194) = 0xDEADBEEF;
    win.faction_records_ = claims;
    NetWinSessionData.mode_ = 2;
    win.on_button_clicked(10);
    assert(popup_calls == 1);
    assert(NetDaemonNet.sent.size() == 0x20);
    assert(NetDaemonNet.sent[0] == 0x0E && NetDaemonNet.sent[1] == 0x2F);
    // The original does not write the remaining leave-packet bytes.
}
"""
    source = tmp_path / "lobby.cpp"
    binary = tmp_path / "lobby"
    source.write_text(harness)
    subprocess.run(
        ["g++", "-std=c++17", "-Wall", "-Wextra", "-Werror", str(source), "-o", str(binary)],
        check=True,
    )
    subprocess.run([str(binary)], check=True)
