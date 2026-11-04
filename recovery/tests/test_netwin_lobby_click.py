"""Actual lobby-click controls for original-defined packet fields and payloads.

Untouched header/tail bytes are copied as object representation, never assigned
an expected value. These host doubles do not prove native ABI or stack contents.
"""

import subprocess
from pathlib import Path
import pytest
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


@pytest.mark.parametrize("damage", [
    "none", "full_after_gate", "single_after_guard", "entry_after_guards", "old_count_cache",
])
def test_netwin_lobby_click_packets_and_branches(tmp_path: Path, damage) -> None:
    assert_leaf_collection_retired(ROOT)
    assert_current_owner(ROOT, 0x0047B450, "src/netwin.cpp")
    source = (ROOT / "src/netwin.cpp").read_text()
    declarations = installed_packet_declarations()
    body = _braced(source, "void NetWin::on_button_clicked(int clicked_id)")
    header = (ROOT / "src/netwin.h").read_text()
    constructed_row = _braced(header, "struct NetWinConstructedLobbyRow {") + ";"
    constructor = _braced(source, "NetWinConstructedLobbyRow::NetWinConstructedLobbyRow()")
    # Observe completion of the actual six-store constructor in this host
    # projection; its product statements and field types remain unchanged.
    constructor = constructor[:-1] + "    observe_row_construction(this);\n}"
    if damage == "full_after_gate":
        start = body.index("        struct ButtonFullLobbyPacket {")
        guard = ("        if (NetDaemonNet.primary_state_.leading_words_[0] == 0) {\n"
                 "            return;\n        }\n")
        end = body.index(guard, start)
        construction = body[start:end]
        assert body.count(construction) == body.count(guard) == 1
        body = body.replace(construction, "", 1).replace(guard, guard + construction, 1)
    elif damage == "single_after_guard":
        start = body.index("        struct ButtonSingleLobbyPacket {")
        guard = "        if (local_slot >= 1) {\n"
        end = body.index(guard, start)
        construction = body[start:end]
        assert body.count(construction) == body.count(guard) == 1
        body = body.replace(construction, "", 1).replace(guard, guard + construction, 1)
    elif damage == "entry_after_guards":
        construction = "    NetWinConstructedLobbyRow row;\n"
        after_guards = "    const int local_slot ="
        assert body.count(construction) == body.count(after_guards) == 1
        body = body.replace(construction, "", 1).replace(after_guards, construction + after_guards, 1)
    elif damage == "old_count_cache":
        read = "NetDaemonNet.player_count_ + 1"
        assert body.count(read) == 3
        body = body.replace(read, "cached_player_count + 1")
        location = "    const int local_slot ="
        assert body.count(location) == 1
        body = body.replace(location, "    const int cached_player_count = NetDaemonNet.player_count_;\n" + location, 1)
    assert source.count("ORIGINAL: 0x0047B450") == 1
    harness = r"""
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <vector>
#include <new>
#define __fastcall
using std::uint8_t;
using std::uint16_t;
using std::uint32_t;
using std::int8_t;
using std::int32_t;
""" + declarations + constructed_row + r"""
static_assert(sizeof(NetWinConstructedLobbyRow) == 0x17C);
static_assert(offsetof(NetWinConstructedLobbyRow, payload_) == 0);
int constructed_rows = 0;
uintptr_t constructed_addresses[8];
static void observe_row_construction(const NetWinConstructedLobbyRow *row) {
    assert(row->payload_.owner_ == 0 && row->payload_.prefix_ == 0xFF);
    assert(row->payload_.ready_or_occupied_ == 0 && row->payload_.field_2_ == 0);
    assert(row->payload_.faction_or_color_ == -1 && row->payload_.field_4_ == 2);
    assert(constructed_rows < 8);
    uintptr_t address = reinterpret_cast<uintptr_t>(row);
    constructed_addresses[constructed_rows++] = address;
}
""" + constructor + r"""
static_assert(sizeof(NetWinLobbyRow) == 0x17C);
static_assert(offsetof(NetWinLobbyRow, owner_) == 0x178);
static_assert(sizeof(NetWinLobbyFullPacket) == 0xA74);
static_assert(sizeof(NetWinLobbySinglePacket) == 0x18C);
static_assert(sizeof(NetWinLobbyLeavePacket) == 0x20);
static_assert(sizeof(NetWinSessionOptionsState) == 0x18);
static_assert(offsetof(NetWinSessionOptionsState, rule_bits_a_) == 0xC);
static_assert(sizeof(NetWinFactionRecord) == 0x190);
static_assert(offsetof(NetWinFactionRecord, owner_) == 4);
struct NetWinLobbyRowsMock { NetWinLobbyRow rows_[8]; } NetWinLobbyRows = {};
struct NetWinSessionDataMock { NetWinSessionBlock fields_; } NetWinSessionData = {};
struct NetWinSessionOptionsMock { NetWinSessionOptionsState fields_; } NetWinSessionOptions = {};
char *ScriptPopupFile = nullptr;
int popup_result = 0;
int popup_calls = 0;
void (*popup_callback)() = nullptr;
int X_pop(char *, const char *, int) {
    ++popup_calls;
    if (popup_callback != nullptr) popup_callback();
    return popup_result;
}
struct Net {
    uint32_t local_player_id_, host_player_id_;
    Net() : local_player_id_(7), host_player_id_(8) {}
    uint32_t local_player_id() const { return local_player_id_; }
    uint32_t host_player_id() const { return host_player_id_; }
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
    int selected_slot_ = 1;
    struct Entry { uint32_t leading_words_[4]; union Payload { uint32_t flags; int *vehicle_selection_follow; } payload_; uint32_t trailing_words_[10]; } channels_[2];
    struct { uint32_t leading_words_[3]; uint32_t flags_; int session_phase_; uint32_t trailing_words_[9]; } primary_state_;
    int pid_2_idx(int) { return selected_slot_; }
} NetDaemonNet;
static void clear_gate_after_prompt() {
    assert(constructed_rows == 1);
    NetDaemonNet.primary_state_.leading_words_[0] = 0;
}
static void grow_count_after_prompt() {
    assert(constructed_rows == 1);
    NetDaemonNet.player_count_ = 3;
}
struct NetWin {
    int field_7614_ = 0;
    void *faction_records_ = nullptr;
    void on_button_clicked(int clicked_id);
};
""" + body + r"""
int main() {
    (void)NetWinSessionOptions;
    auto packet_word = [](int offset) {
        uint32_t value;
        std::memcpy(&value, NetDaemonNet.sent.data() + offset, sizeof(value));
        return value;
    };
    NetWin win;
    win.on_button_clicked(-2);
    assert(win.field_7614_ == -1);
    assert(constructed_rows == 1);
    win.field_7614_ = 0;
    constructed_rows = 0;
    win.on_button_clicked(9);
    assert(NetDaemonNet.sent.empty());
    assert(constructed_rows == 1);

    NetWinLobbyRows.rows_[1].ready_or_occupied_ = 0;
    NetWinLobbyRows.rows_[1].faction_or_color_ = 3;
    NetWinLobbyRows.rows_[1].owner_ = 0x12345678;
    constructed_rows = 0;
    win.on_button_clicked(10);
    assert(NetDaemonNet.sent.size() == 0x18C);
    assert(NetDaemonNet.sent[0] == 4 && NetDaemonNet.sent[1] == 0x2F);
    assert(NetDaemonNet.sent[0x11] == 1);
    assert(NetDaemonNet.sent[0x13] == 3);
    assert(packet_word(4) == 1 && packet_word(8) == 0);
    NetWinLobbyRow expected_row = NetWinLobbyRows.rows_[1];
    expected_row.ready_or_occupied_ = 1;
    assert(std::memcmp(NetDaemonNet.sent.data() + 0x10,
                       &expected_row, sizeof(expected_row)) == 0);
    assert(NetWinLobbyRows.rows_[1].ready_or_occupied_ == 0);
    assert(static_cast<uint32_t>(NetDaemonNet.recipient) == NetDaemonNet.host_player_id_);
    assert(constructed_rows == 2);

    NetDaemonNet.primary_state_.leading_words_[0] = 1;
    NetWinSessionData.fields_.mode_ = 0;
    constructed_rows = 0;
    win.on_button_clicked(10);
    assert(NetDaemonNet.sent.size() == 0xA74);
    assert(NetDaemonNet.sent[0] == 4 && NetDaemonNet.sent[1] == 0x4F);
    assert(NetDaemonNet.sent[0x13] == 3);
    assert(packet_word(4) == 0xFFFFFFFFu && packet_word(8) == 0);
    assert(std::memcmp(NetDaemonNet.sent.data() + 0x10,
                       &NetWinLobbyRows.rows_[1], 7 * sizeof(NetWinLobbyRow)) == 0);
    assert(NetDaemonNet.recipient == 0);
    assert(constructed_rows == 8);
    for (int i = 2; i != 8; ++i)
        assert(constructed_addresses[i] == constructed_addresses[1] + (i - 1) * sizeof(NetWinConstructedLobbyRow));

    uint8_t claims[8 * 0x190] = {};
    *reinterpret_cast<uint32_t *>(claims + 0x194) = 0xDEADBEEF;
    win.faction_records_ = claims;
    NetWinSessionData.fields_.mode_ = 2;
    constructed_rows = 0;
    win.on_button_clicked(10);
    assert(popup_calls == 1);
    assert(NetDaemonNet.sent.size() == 0x20);
    assert(NetDaemonNet.sent[0] == 0x0E && NetDaemonNet.sent[1] == 0x2F);
    // The original does not write the remaining leave-packet bytes.
    assert(constructed_rows == 1);

    // A modal callback disables sending after entering the host branch.
    // The native local seven-row construction still precedes the late gate.
    NetWinSessionData.fields_.mode_ = 0;
    NetDaemonNet.player_count_ = 2;
    NetWinLobbyRows.rows_[2].ready_or_occupied_ = 0;
    NetDaemonNet.primary_state_.leading_words_[0] = 1;
    NetDaemonNet.sent.clear();
    popup_callback = clear_gate_after_prompt;
    constructed_rows = 0;
    win.on_button_clicked(10);
    assert(constructed_rows == 8);
    assert(NetDaemonNet.sent.empty() && popup_calls == 2);
    popup_callback = nullptr;

    // The modal callback changes the live count after the readiness scan.
    // Faction validation and the final readiness writes both use that count.
    NetWinSessionData.fields_.mode_ = 2;
    NetDaemonNet.player_count_ = 2;
    NetDaemonNet.primary_state_.leading_words_[0] = 1;
    NetWinLobbyRows.rows_[2].ready_or_occupied_ = 0;
    NetWinLobbyRows.rows_[3].ready_or_occupied_ = 0;
    NetWinLobbyRows.rows_[3].faction_or_color_ = 1;
    NetWinLobbyRows.rows_[3].owner_ = 0xDEADBEEF;
    NetDaemonNet.sent.clear();
    popup_callback = grow_count_after_prompt;
    constructed_rows = 0;
    win.on_button_clicked(10);
    assert(NetDaemonNet.sent.size() == 0xA74);
    assert(NetWinLobbyRows.rows_[3].ready_or_occupied_ == 1);
    assert(constructed_rows == 8 && popup_calls == 3);
    popup_callback = nullptr;

    // Slot zero also constructs its packet row before the signed send guard.
    NetDaemonNet.selected_slot_ = 0;
    NetDaemonNet.primary_state_.leading_words_[0] = 0;
    NetDaemonNet.sent.clear();
    NetWinLobbyRow old_zero = NetWinLobbyRows.rows_[0];
    constructed_rows = 0;
    win.on_button_clicked(10);
    assert(constructed_rows == 2);
    assert(NetDaemonNet.sent.empty());
    assert(std::memcmp(&NetWinLobbyRows.rows_[0], &old_zero, sizeof(old_zero)) == 0);

    // The actual constructor changes only the six original fields.
    alignas(NetWinConstructedLobbyRow) unsigned char storage[sizeof(NetWinConstructedLobbyRow)];
    std::memset(storage, 0xA5, sizeof(storage));
    constructed_rows = 0;
    NetWinConstructedLobbyRow *placed = new (storage) NetWinConstructedLobbyRow;
    assert(constructed_rows == 1);
    for (unsigned i = 5; i < offsetof(NetWinLobbyRow, owner_); ++i)
        assert(storage[i] == 0xA5);
    placed->~NetWinConstructedLobbyRow();
}
"""
    source = tmp_path / "lobby.cpp"
    binary = tmp_path / "lobby"
    source.write_text(harness)
    subprocess.run(
        ["g++", "-std=c++17", "-Wall", "-Wextra", "-Werror", str(source), "-o", str(binary)],
        check=True,
    )
    result = subprocess.run([str(binary)], capture_output=True, text=True)
    if damage == "none":
        assert result.returncode == 0, result.stdout + result.stderr
    else:
        assert result.returncode != 0
        expected = {
            "full_after_gate": "constructed_rows == 8",
            "single_after_guard": "constructed_rows == 2",
            "entry_after_guards": "constructed_rows == 1",
            "old_count_cache": "NetDaemonNet.sent.size() == 0xA74",
        }
        assert expected[damage] in result.stderr
