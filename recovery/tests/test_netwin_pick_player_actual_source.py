from __future__ import annotations

import shutil
import subprocess
import os
from pathlib import Path

from test_net_hotseat_owner_actual_source_oct6 import installed_packet_declarations

ROOT = next(
    parent
    for parent in Path(__file__).resolve().parents
    if (parent / "src/netwin.cpp").is_file()
)


def _installed_method() -> str:
    source = (ROOT / "src/netwin.cpp").read_text()
    start = source.index("void NetWin::pick_player() {")
    brace = source.index("{", start)
    depth = 0
    for end in range(brace, len(source)):
        if source[end] == "{":
            depth += 1
        elif source[end] == "}":
            depth -= 1
            if depth == 0:
                return source[start : end + 1]
    raise AssertionError("installed NetWin::pick_player has no closing brace")


def test_installed_pick_player_source_and_controls(tmp_path: Path) -> None:
    method = _installed_method()
    assert "popup.input_poll_state() == 1" in method
    assert "ParseStrBuffer[5].str" in method
    assert "NetWinLobbyRows[0] = updated" not in method
    assert "if (slot < 1)" in method
    # The original sends unwritten header bytes at +2 and +0xC. Do not
    # normalize them or inspect their unspecified values in the fixture.
    assert "memset(&packet" not in method
    assert "packet.header_.field_8_ = 0;" in method

    compiler = os.environ.get("CXX") or shutil.which("g++") or shutil.which("clang++")
    assert compiler, "a host C++ compiler is required for this source control"
    fixture = tmp_path / "pick_player.cpp"
    executable = tmp_path / "pick_player"
    fixture_source = (
        r'''
#include <cassert>
#include <cctype>
#include <cstdio>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <string>
#include <vector>
@@ACTUAL_DECLARATIONS@@

struct Player {
    int is_leader_female;
    char filename[24], search_key[24], name_leader[24], title_leader[24];
    char adj_leader[128], adj_insult_leader[128], adj_faction[128];
    char adj_insult_faction[128], padding1[128], noun_faction[24];
    int noun_gender, is_noun_plural;
    char adj_name_faction[128], formal_name_faction[40];
};
struct char256 { char str[256]; };
NetWinLobbyRow lobby_storage[9];
NetWinLobbyRow *NetWinLobbyRows = &lobby_storage[1];
NetWinSessionBlock NetWinSessionData;
Player Players[8];
char256 ParseStrBuffer[10];
int NetWinPlayerId = 27, NetWinOtherPlayerId = 39;
char PopupStartCaption[256] = "caption";
int selected_slot = 2, modal_result = 0, modal_state = 0;
char edited_gender[24] = "Male";
char gender_opened_with[24] = {};
std::vector<unsigned char> sent_packet;
uint8_t sent_opcode = 0, sent_flags = 0;
int32_t sent_slot = 0, sent_field_8 = 0;
unsigned long sent_size = 0;
int sent_recipient = 0, sent_flag = 0, send_count = 0;
int popup_start_count = 0, popup_exec_count = 0;
char *edit_initial[7] = {};
char *edit_label[7] = {};
int edit_limit[7] = {};
int edit_count = 0;

char *label_get(int id) {
    static char male[] = "Male";
    static char female[] = "Female";
    static char fallback[] = "DefaultFaction";
    static char labels[0xFE4 / 4 + 1][24] = {};
    if (id == 0xB88 / 4) return male;
    if (id == 0xB8C / 4) return female;
    if (id == 0xFE4 / 4) return fallback;
    std::snprintf(labels[id], sizeof(labels[id]), "label-%x", id * 4);
    return labels[id];
}
int _strnicmp(const char *a, const char *b, unsigned int count) {
    for (unsigned int i = 0; i < count; ++i) {
        const int ca = std::tolower(static_cast<unsigned char>(a[i]));
        const int cb = std::tolower(static_cast<unsigned char>(b[i]));
        if (ca != cb || ca == 0 || cb == 0) return ca - cb;
    }
    return 0;
}
int __cdecl_pick_daemon_callback() { return 0; }
int pick_daemon_callback() { return __cdecl_pick_daemon_callback(); }

class EditGroup {
public:
    int edit(char *label, char *initial, int limit) {
        assert(edit_count < 7);
        edit_label[edit_count] = label;
        edit_initial[edit_count] = initial;
        edit_limit[edit_count] = limit;
        ++edit_count;
        return 0;
    }
};
class Dialogs : public EditGroup {};
class Popup {
    Dialogs dialogs_;
public:
    void start(char *, const char *, int, char *, int, void *) {
        ++popup_start_count;
    }
    Dialogs &dialogs() { return dialogs_; }
    void set_loc(int x, int y) { assert(x == 0x1000 && y == 0x1000); }
    int exec(int flags, int (*callback)()) {
        assert(flags == 0 && callback != 0);
        ++popup_exec_count;
        assert(edit_count == 7);
        std::strcpy(gender_opened_with, edit_initial[6]);
        std::strcpy(edit_initial[6], edited_gender);
        return modal_result;
    }
    int input_poll_state() const { return modal_state; }
};
struct Net {
    void send(void *data, unsigned long size, unsigned long recipient,
              int flag) {
        const unsigned char *bytes = static_cast<const unsigned char *>(data);
        // Capture only originally initialized fields and the copied row.
        // Header bytes +2..3 and +0xC..F deliberately remain unspecified.
        sent_size = size;
        sent_opcode = bytes[0];
        sent_flags = bytes[1];
        std::memcpy(&sent_slot, bytes + 4, sizeof(sent_slot));
        std::memcpy(&sent_field_8, bytes + 8, sizeof(sent_field_8));
        sent_packet.assign(bytes + 0x10, bytes + size);
        sent_recipient = static_cast<int>(recipient);
        sent_flag = flag;
        ++send_count;
    }
};
struct NetDaemon : Net {
    int pid_2_idx(int) { return selected_slot; }
};
NetDaemon NetDaemonNet;
class NetWin { public: void pick_player(); };

'''
        + method
        + r'''

static void clear_run() {
    sent_packet.clear();
    sent_opcode = sent_flags = 0;
    sent_slot = sent_field_8 = 0;
    sent_size = 0;
    sent_recipient = sent_flag = send_count = 0;
    popup_start_count = popup_exec_count = 0;
    edit_count = 0;
    gender_opened_with[0] = 0;
    std::memset(edit_initial, 0, sizeof(edit_initial));
    std::memset(edit_label, 0, sizeof(edit_label));
    std::memset(edit_limit, 0, sizeof(edit_limit));
}
static void seed_row(NetWinLobbyRow &row, int faction, uint8_t gender) {
    std::memset(&row, 0xA5, sizeof(row));
    row.prefix_ = 0x31;
    row.ready_or_occupied_ = 0x42;
    row.field_2_ = 0x53;
    row.faction_or_color_ = static_cast<int8_t>(faction);
    row.field_4_ = 0x64;
    std::strcpy(row.leader_name_, "OldLeader");
    std::strcpy(row.formal_faction_name_, "OldFormal");
    std::strcpy(row.noun_faction_, "OldNoun");
    std::strcpy(row.adjective_faction_, "OldAdjective");
    std::strcpy(row.leader_title_, "OldTitle");
    std::strcpy(row.adjective_leader_, "OldAdjLeader");
    row.tail_[0] = gender;
    row.tail_[1] = 0x75;
    row.tail_[2] = 0x86;
    row.owner_ = 0x12345678;
}
static void verify_packet(const NetWinLobbyRow &expected, int slot) {
    assert(send_count == 1);
    assert(sent_size == sizeof(NetWinLobbySinglePacket));
    assert(sent_opcode == 0x04 && sent_flags == 0x2F);
    assert(sent_slot == slot);
    assert(sent_field_8 == 0);
    assert(sent_packet.size() == sizeof(NetWinLobbyRow));
    assert(std::memcmp(sent_packet.data(), &expected, sizeof(expected)) == 0);
    assert(sent_recipient == NetWinOtherPlayerId && sent_flag == 1);
}
int main() {
    NetWin win;

    // Guarded modes never open the modal or send.
    for (int mode = 2; mode <= 3; ++mode) {
        clear_run();
        NetWinSessionData.mode_ = mode;
        win.pick_player();
        assert(popup_start_count == 0 && send_count == 0);
    }

    // Cancellation leaves the global row and peer untouched after all seven
    // fields were offered in their pinned order and with their original limits.
    clear_run();
    NetWinSessionData.mode_ = 0;
    selected_slot = 2;
    seed_row(NetWinLobbyRows[2], -1, 1);
    NetWinLobbyRow before_cancel = NetWinLobbyRows[2];
    modal_result = -1;
    modal_state = 0;
    win.pick_player();
    assert(popup_start_count == 1 && popup_exec_count == 1);
    assert(edit_count == 7);
    const char *expected_labels[7] = {"label-2b8", "label-2bc", "label-2c0",
                                      "label-2c4", "label-78c", "label-790",
                                      "label-794"};
    for (int i = 0; i < 7; ++i)
        assert(std::strcmp(edit_label[i], expected_labels[i]) == 0);
    assert(edit_limit[0] == 0x17 && edit_limit[1] == 0x27);
    assert(edit_limit[2] == 0x17 && edit_limit[3] == 0x17);
    assert(edit_limit[4] == 0x17 && edit_limit[5] == 0x17);
    assert(edit_limit[6] == 0x17);
    assert(std::strcmp(gender_opened_with, "Female") == 0);
    assert(send_count == 0);
    assert(std::memcmp(&NetWinLobbyRows[2], &before_cancel, sizeof(before_cancel)) == 0);

    // State 1 plus faction -1 restores the localized default name in all six
    // text slots, sets native gender byte 10, keeps unrelated fields, and publishes
    // exactly one 0x2F04 full-row packet.
    clear_run();
    seed_row(NetWinLobbyRows[2], -1, 0);
    NetWinLobbyRow before_default = NetWinLobbyRows[2];
    modal_result = 0;
    modal_state = 1;
    edited_gender[0] = 0;
    std::strcpy(ParseStrBuffer[0].str, "not-used");
    win.pick_player();
    assert(std::memcmp(&NetWinLobbyRows[2], &before_default, sizeof(before_default)) == 0);
    NetWinLobbyRow expected_default = NetWinLobbyRows[2];
    std::strcpy(expected_default.leader_name_, "DefaultFaction");
    std::strcpy(expected_default.formal_faction_name_, "DefaultFaction");
    std::strcpy(expected_default.noun_faction_, "DefaultFaction");
    std::strcpy(expected_default.adjective_faction_, "DefaultFaction");
    std::strcpy(expected_default.leader_title_, "DefaultFaction");
    std::strcpy(expected_default.adjective_leader_, "DefaultFaction");
    expected_default.tail_[0] = 10;
    verify_packet(expected_default, 2);

    // State 1 plus a selected faction copies all six typed Player fields and
    // its female flag, without disturbing row padding/tail/owner bytes.
    clear_run();
    seed_row(NetWinLobbyRows[2], 4, 0);
    NetWinLobbyRow before_player = NetWinLobbyRows[2];
    std::strcpy(Players[4].name_leader, "Leila");
    std::strcpy(Players[4].formal_name_faction, "Faction Four");
    std::strcpy(Players[4].noun_faction, "Four");
    std::strcpy(Players[4].adj_name_faction, "Fourth");
    std::strcpy(Players[4].title_leader, "Leader Four");
    std::strcpy(Players[4].adj_leader, "Leading Four");
    Players[4].is_leader_female = 1;
    modal_state = 1;
    win.pick_player();
    assert(std::memcmp(&NetWinLobbyRows[2], &before_player, sizeof(before_player)) == 0);
    NetWinLobbyRow expected_player = NetWinLobbyRows[2];
    std::strcpy(expected_player.leader_name_, "Leila");
    std::strcpy(expected_player.formal_faction_name_, "Faction Four");
    std::strcpy(expected_player.noun_faction_, "Four");
    std::strcpy(expected_player.adjective_faction_, "Fourth");
    std::strcpy(expected_player.leader_title_, "Leader Four");
    std::strcpy(expected_player.adjective_leader_, "Leading Four");
    expected_player.tail_[0] = 1;
    verify_packet(expected_player, 2);

    // Non-1 poll state publishes the six parsed strings and the first-byte,
    // case-insensitive gender comparison from the seventh edit.
    clear_run();
    seed_row(NetWinLobbyRows[2], 3, 0);
    NetWinLobbyRow before_custom = NetWinLobbyRows[2];
    const char *parsed[6] = {"ParsedLeader", "ParsedFormal", "ParsedNoun",
                             "ParsedAdj", "ParsedTitle", "ParsedAdjLeader"};
    for (int i = 0; i < 6; ++i) std::strcpy(ParseStrBuffer[i].str, parsed[i]);
    std::strcpy(edited_gender, "feminine");
    modal_state = 0;
    win.pick_player();
    assert(std::memcmp(&NetWinLobbyRows[2], &before_custom, sizeof(before_custom)) == 0);
    NetWinLobbyRow expected_custom = NetWinLobbyRows[2];
    std::strcpy(expected_custom.leader_name_, parsed[0]);
    std::strcpy(expected_custom.formal_faction_name_, parsed[1]);
    std::strcpy(expected_custom.noun_faction_, parsed[2]);
    std::strcpy(expected_custom.adjective_faction_, parsed[3]);
    std::strcpy(expected_custom.leader_title_, parsed[4]);
    std::strcpy(expected_custom.adjective_leader_, parsed[5]);
    expected_custom.tail_[0] = 1;
    verify_packet(expected_custom, 2);

    // Slot zero has the accepted local transformation but no packet and no
    // store back to the shared first row.
    clear_run();
    selected_slot = 0;
    seed_row(NetWinLobbyRows[0], -1, 0);
    NetWinLobbyRow before_zero = NetWinLobbyRows[0];
    modal_state = 1;
    win.pick_player();
    assert(send_count == 0);
    assert(std::memcmp(&NetWinLobbyRows[0], &before_zero, sizeof(before_zero)) == 0);

    // Native signed compare rejects packet slots <= 0. Model the preceding
    // addressable row explicitly so the original unchecked slot -1 reads and
    // modal field setup are testable without indexing outside the fixture.
    clear_run();
    selected_slot = -1;
    seed_row(NetWinLobbyRows[-1], -1, 1);
    NetWinLobbyRow before_negative = NetWinLobbyRows[-1];
    modal_state = 1;
    win.pick_player();
    assert(popup_start_count == 1 && edit_count == 7);
    assert(send_count == 0);
    assert(std::memcmp(&NetWinLobbyRows[-1], &before_negative, sizeof(before_negative)) == 0);
}
'''
    )
    fixture_source = fixture_source.replace(
        "@@ACTUAL_DECLARATIONS@@", installed_packet_declarations())
    fixture.write_text(fixture_source)
    subprocess.run(
        [compiler, "-std=c++11", "-Wall", "-Wextra", "-Werror", str(fixture), "-o", str(executable)],
        check=True,
        capture_output=True,
        text=True,
    )
    subprocess.run([str(executable)], check=True)

    # Mutation control: reversing the native state-1 branch must contradict
    # the full-row oracle in the default/player scenarios.
    mutated = method.replace(
        "if (popup.input_poll_state() == 1)",
        "if (popup.input_poll_state() != 1)",
        1,
    )
    assert mutated != method
    mutant_fixture = tmp_path / "pick_player_mutant.cpp"
    mutant_executable = tmp_path / "pick_player_mutant"
    mutant_fixture.write_text(fixture_source.replace(method, mutated, 1))
    subprocess.run(
        [compiler, "-std=c++11", "-Wall", "-Wextra", "-Werror", str(mutant_fixture), "-o", str(mutant_executable)],
        check=True,
        capture_output=True,
        text=True,
    )
    mutant = subprocess.run([str(mutant_executable)], check=False)
    assert mutant.returncode != 0, "branch mutation escaped installed-source controls"
