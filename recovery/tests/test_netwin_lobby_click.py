"""Host execution of the actual recovered lobby-click body with small seams."""

import subprocess
from pathlib import Path


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
    header = (ROOT / "src/netwin.h").read_text()
    leaf = (ROOT / "src/leaf_recoveries.cpp").read_text()
    source = (ROOT / "src/netwin.cpp").read_text()
    declarations = "\n".join(
        _braced(header, name) + ";"
        for name in (
            "struct NetWinLobbyRow",
            "struct NetWinSessionBlock",
            "struct NetWinSessionOptionsState",
            "struct NetWinFactionRecord",
        )
    )
    declarations += "\n" + "\n".join(
        _braced(source, name) + ";"
        for name in (
            "struct NetWinLobbyPacketHeader",
            "struct NetWinLobbyFullPacket",
            "struct NetWinLobbySinglePacket",
            "struct NetWinLobbyLeavePacket",
        )
    )
    body = _braced(source, "void NetWin::on_button_clicked(int clicked_id)")
    assert leaf.count("ORIGINAL: 0x0047B450") == 0
    assert source.count("ORIGINAL: 0x0047B450") == 1
    harness = r"""
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <vector>
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
std::uintptr_t popup_title = 0;
std::uintptr_t *ClimatePopupTitle = &popup_title;
int popup_result = 0;
int popup_calls = 0;
int x_pop_caption_label(char *, const char *, int) {
    ++popup_calls;
    return popup_result;
}
struct Net {
    std::vector<uint8_t> sent;
    int recipient = -1;
    int send(const void *message, int length, int target, int) {
        sent.assign(static_cast<const uint8_t *>(message),
                    static_cast<const uint8_t *>(message) + length);
        recipient = target;
        return length;
    }
};
struct Daemon : Net {
    int player_count_ = 0;
    struct Entry { uint32_t dword_[15] = {}; } lock_state_[1];
    int pid_2_idx(int) { return 1; }
} NetDaemonNet;
struct NetWin {
    int field_7614_ = 0;
    void *faction_records_ = nullptr;
    void on_button_clicked(int clicked_id);
};
""" + body + r"""
int main() {
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
    assert(NetDaemonNet.sent[2] == 0 && NetDaemonNet.sent[3] == 0);
    for (int i = 12; i < 16; ++i) assert(NetDaemonNet.sent[i] == 0);
    assert(NetWinLobbyRows[1].ready_or_occupied_ == 0);
    assert(NetDaemonNet.recipient == NetWinOtherPlayerId);

    NetDaemonNet.lock_state_[0].dword_[1] = 1;
    NetWinSessionData.mode_ = 0;
    win.on_button_clicked(10);
    assert(NetDaemonNet.sent.size() == 0xA74);
    assert(NetDaemonNet.sent[0] == 4 && NetDaemonNet.sent[1] == 0x4F);
    assert(NetDaemonNet.sent[0x13] == 3);
    assert(NetDaemonNet.sent[2] == 0 && NetDaemonNet.sent[3] == 0);
    for (int i = 12; i < 16; ++i) assert(NetDaemonNet.sent[i] == 0);
    assert(NetDaemonNet.recipient == 0);

    uint8_t claims[8 * 0x190] = {};
    *reinterpret_cast<uint32_t *>(claims + 0x194) = 0xDEADBEEF;
    win.faction_records_ = claims;
    NetWinSessionData.mode_ = 2;
    win.on_button_clicked(10);
    assert(popup_calls == 1);
    assert(NetDaemonNet.sent.size() == 0x20);
    assert(NetDaemonNet.sent[0] == 0x0E && NetDaemonNet.sent[1] == 0x2F);
    for (int i = 2; i < 0x20; ++i) assert(NetDaemonNet.sent[i] == 0);
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
