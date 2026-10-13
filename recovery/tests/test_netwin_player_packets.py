"""Host controls for the actual per-player faction/difficulty picker bodies.

This fixture stubs only UI, selection, and send boundaries. Actual POD
declarations and method bodies are extracted from the product sources. The
packet's reserved header words are intentionally not inspected: the source
does not initialize them. These are source behavior controls, not native ABI
evidence.
"""

from __future__ import annotations

import shutil
import subprocess
import tempfile
from pathlib import Path

import pytest
from test_net_hotseat_owner_actual_source_oct6 import installed_packet_declarations


ROOT = Path(__file__).resolve().parents[2]
NETWIN_H = ROOT / "src/netwin.h"
NETWIN_CPP = ROOT / "src/netwin.cpp"


def extract_braced(source: str, marker: str) -> str:
    start = source.find(marker)
    if start < 0:
        raise AssertionError(f"missing actual source marker: {marker}")
    opening = source.find("{", start)
    if opening < 0:
        raise AssertionError(f"missing body: {marker}")
    depth = 0
    for pos in range(opening, len(source)):
        if source[pos] == "{":
            depth += 1
        elif source[pos] == "}":
            depth -= 1
            if depth == 0:
                end = pos + 1
                while end < len(source) and source[end].isspace():
                    end += 1
                if end < len(source) and source[end] == ";":
                    end += 1
                return source[start:end]
    raise AssertionError(f"unterminated declaration/body: {marker}")


def actual_declarations() -> str:
    return installed_packet_declarations()


def actual_methods() -> dict[str, str]:
    source = NETWIN_CPP.read_text(encoding="utf-8")
    faction = extract_braced(source, "void NetWin::pick_faction(int slot)")
    difficulty = extract_braced(source, "void NetWin::pick_diff(int slot)")
    return {"faction": faction, "difficulty": difficulty}


HARNESS = r"""
#include <assert.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <cstring>

@@ACTUAL_TYPES@@

struct Dialogs {
    int item_count;
    int ids[16];
    char labels[16][40];
    Dialogs() : item_count(0) { memset(ids, 0, sizeof(ids)); memset(labels, 0, sizeof(labels)); }
    void item(char *label, int id) {
        assert(item_count < 16);
        ids[item_count] = id;
        strncpy(labels[item_count], label, sizeof(labels[item_count]) - 1);
        ++item_count;
    }
};

struct PopMenu {
    Dialogs dialog_list;
    int selected_id;
    PopMenu() : selected_id(0) {}
    void init() { dialog_list.item_count = 0; }
    Dialogs &dialogs() { return dialog_list; }
    int get_selected_id() const { return selected_id; }
};

static int g_answer = -1;
static int g_selected_id = 0;
static int g_pass_result = 1;
static int g_pass_calls;
static int g_pass_payload;
static int g_faction_seen_during_pass;
static int g_pick_slot;
static int g_last_item_count;
static int g_last_item_ids[16];
static char g_default_label[] = "default";
static char g_difficulty_labels[6][16] = {
    "diff0", "diff1", "diff2", "diff3", "diff4", "diff5"
};
static char *Difficulty[6] = {
    g_difficulty_labels[0], g_difficulty_labels[1], g_difficulty_labels[2],
    g_difficulty_labels[3], g_difficulty_labels[4], g_difficulty_labels[5]
};
struct FakeStringTable {
    int calls;
    int ids[6];
    FakeStringTable() : calls(0) { memset(ids, 0, sizeof(ids)); }
    char *get(char *name) {
        assert(calls < 6);
        int id = 0;
        while (id < 6 && name != Difficulty[id]) ++id;
        assert(id < 6);
        ids[calls++] = 0x100 + id;
        return name;
    }
} StringTable;

static NetWinLobbyRow NetWinLobbyRows[8];
static NetWinSessionBlock NetWinSessionData;
static int g_player_count;
int *NetWinPlayerCount = &g_player_count;
int NetWinOtherPlayerId = 0x1234;

struct Net {
    int sends;
    int length;
    unsigned long target;
    int flags;
    unsigned char bytes[0x18C];
    Net() : sends(0), length(0), target(0), flags(0) { memset(bytes, 0, sizeof(bytes)); }
    int send(void *data, int size, unsigned long to, int send_flags) {
        assert(size == static_cast<int>(sizeof(NetWinLobbySinglePacket)));
        ++sends;
        length = size;
        target = to;
        flags = send_flags;
        memcpy(bytes, data, static_cast<size_t>(size));
        return 1;
    }
} NetDaemonNet;

class NetWin {
public:
    void *faction_records_;
    NetWin() : faction_records_(0) {}
    int pick_pass(int payload) {
        ++g_pass_calls;
        g_pass_payload = payload;
        g_faction_seen_during_pass = NetWinLobbyRows[g_pick_slot].faction_or_color_;
        return g_pass_result;
    }
    void pick_faction(int slot);
    void pick_diff(int slot);
};

static char *label_get(int) { return g_default_label; }
static int pick_answer(NetWin &, PopMenu &menu) {
    menu.selected_id = g_selected_id;
    g_last_item_count = menu.dialog_list.item_count;
    for (int i = 0; i < g_last_item_count; ++i)
        g_last_item_ids[i] = menu.dialog_list.ids[i];
    return g_answer;
}

@@ACTUAL_HELPERS@@

@@ACTUAL_METHODS@@

static void reset_observations() {
    NetDaemonNet.sends = 0;
    NetDaemonNet.length = 0;
    NetDaemonNet.target = 0;
    NetDaemonNet.flags = 0;
    memset(NetDaemonNet.bytes, 0, sizeof(NetDaemonNet.bytes));
    g_pass_calls = 0;
    g_pass_payload = -1;
    g_faction_seen_during_pass = -1;
    g_pick_slot = 1;
    g_last_item_count = 0;
    memset(g_last_item_ids, 0, sizeof(g_last_item_ids));
    StringTable.calls = 0;
}

static void fill_row(NetWinLobbyRow &row, unsigned char seed) {
    unsigned char *bytes = reinterpret_cast<unsigned char *>(&row);
    for (size_t i = 0; i < sizeof(row); ++i) bytes[i] = static_cast<unsigned char>(seed + i * 13U);
}

static void assert_packet_header(int slot) {
    assert(NetDaemonNet.sends == 1);
    assert(NetDaemonNet.length == 0x18C);
    assert(NetDaemonNet.target == static_cast<unsigned long>(NetWinOtherPlayerId));
    assert(NetDaemonNet.flags == 1);
    assert(NetDaemonNet.bytes[0] == 0x04);
    assert(NetDaemonNet.bytes[1] == 0x2F);
    int32_t packet_slot = -1;
    int32_t packet_field_8 = -1;
    memcpy(&packet_slot, NetDaemonNet.bytes + 4, sizeof(packet_slot));
    memcpy(&packet_field_8, NetDaemonNet.bytes + 8, sizeof(packet_field_8));
    assert(packet_slot == slot);
    assert(packet_field_8 == 0);
    // Do not read/assert header bytes +2..+3 or +0xC..+0xF: the original
    // methods leave those reserved bytes uninitialized before sending.
}

int main() {
    NetWin netwin;
    NetWinFactionRecord records[3];
    memset(records, 0, sizeof(records));
    records[0].record_count_ = 3;
    records[0].active_ = 1;
    records[1].active_ = 1;
    records[2].active_ = 0;
    strcpy(records[0].noun_faction_, "Faction Zero");
    strcpy(records[1].noun_faction_, "Faction One");
    netwin.faction_records_ = records;
    g_player_count = 0;
    NetWinSessionData.mode_ = 0;
    NetWinLobbyRow original;
    fill_row(original, 0x31);
    NetWinLobbyRows[1] = original;
    const NetWinLobbyRow original_slot_zero = NetWinLobbyRows[0];

    // Cancelled faction dialog does not call pick_pass or send.
    reset_observations();
    g_answer = -1;
    g_selected_id = 2;
    netwin.pick_faction(1);
    assert(g_pass_calls == 0 && NetDaemonNet.sends == 0);
    assert(memcmp(&NetWinLobbyRows[1], &original, sizeof(original)) == 0);

    // A successful slot-zero choice temporarily changes shared state for
    // pick_pass, restores it afterward, and does not send a player packet.
    reset_observations();
    g_answer = 0;
    g_selected_id = 2;
    g_pass_result = 1;
    g_pick_slot = 0;
    netwin.pick_faction(0);
    assert(g_pass_calls == 1 && g_pass_payload == 1);
    assert(g_faction_seen_during_pass == 1);
    assert(NetDaemonNet.sends == 0);
    assert(memcmp(&NetWinLobbyRows[0], &original_slot_zero, sizeof(original_slot_zero)) == 0);

    // Rejected faction choice restores the shared row and sends nothing.
    reset_observations();
    g_answer = 0;
    g_selected_id = 2;
    g_pass_result = 0;
    netwin.pick_faction(1);
    assert(g_pass_calls == 1 && g_faction_seen_during_pass == 1);
    assert(memcmp(&NetWinLobbyRows[1], &original, sizeof(original)) == 0);
    assert(NetDaemonNet.sends == 0);

    // Accepted positive slot sends an opcode/slot packet plus a full copied
    // row with the chosen faction; shared row is restored after pick_pass.
    reset_observations();
    g_answer = 0;
    g_selected_id = 2;
    g_pass_result = 1;
    netwin.pick_faction(1);
    assert(g_pass_calls == 1 && g_faction_seen_during_pass == 1);
    assert(memcmp(&NetWinLobbyRows[1], &original, sizeof(original)) == 0);
    assert_packet_header(1);
    NetWinLobbyRow faction_row;
    memcpy(&faction_row, NetDaemonNet.bytes + sizeof(NetWinLobbyPacketHeader), sizeof(faction_row));
    NetWinLobbyRow expected_faction_row = original;
    expected_faction_row.faction_or_color_ = 1;
    assert(memcmp(&faction_row, &expected_faction_row, sizeof(faction_row)) == 0);

    // Difficulty cancellation still sends the unchanged local row for a
    // positive slot; it does not mutate the shared lobby row.
    reset_observations();
    g_answer = -1;
    netwin.pick_diff(1);
    assert(StringTable.calls == 6);
    for (int i = 0; i < 6; ++i) assert(StringTable.ids[i] == 0x100 + i);
    assert(memcmp(&NetWinLobbyRows[1], &original, sizeof(original)) == 0);
    assert_packet_header(1);
    NetWinLobbyRow cancelled_diff_row;
    memcpy(&cancelled_diff_row, NetDaemonNet.bytes + sizeof(NetWinLobbyPacketHeader), sizeof(cancelled_diff_row));
    assert(memcmp(&cancelled_diff_row, &original, sizeof(original)) == 0);

    // Accepted difficulty changes only the copied row's difficulty byte.
    reset_observations();
    g_answer = 4;
    netwin.pick_diff(1);
    assert(memcmp(&NetWinLobbyRows[1], &original, sizeof(original)) == 0);
    assert_packet_header(1);
    NetWinLobbyRow difficulty_row;
    memcpy(&difficulty_row, NetDaemonNet.bytes + sizeof(NetWinLobbyPacketHeader), sizeof(difficulty_row));
    NetWinLobbyRow expected_difficulty_row = original;
    expected_difficulty_row.field_2_ = 4;
    assert(memcmp(&difficulty_row, &expected_difficulty_row, sizeof(difficulty_row)) == 0);

    // Slot zero never transmits, for either a faction or difficulty choice.
    reset_observations();
    g_answer = 0;
    g_selected_id = 2;
    g_pass_result = 1;
    g_pick_slot = 0;
    netwin.pick_faction(0);
    assert(NetDaemonNet.sends == 0);
    reset_observations();
    g_answer = 2;
    netwin.pick_diff(0);
    assert(NetDaemonNet.sends == 0);
    assert(memcmp(&NetWinLobbyRows[1], &original, sizeof(original)) == 0);

    // Picker item identity is defined: faction ids are one-based, difficulty
    // ids are zero-based. The inactive faction is not offered.
    reset_observations();
    g_answer = -1;
    netwin.pick_faction(1);
    assert(g_last_item_count == 3);
    assert(g_last_item_ids[0] == 0);
    assert(g_last_item_ids[1] == 1);
    assert(g_last_item_ids[2] == 2);
    return 0;
}
"""


def fixture_source(methods: dict[str, str]) -> str:
    source = NETWIN_CPP.read_text(encoding="utf-8")
    helpers = extract_braced(source, "static void *memcpy_call(")
    return (HARNESS.replace("@@ACTUAL_TYPES@@", actual_declarations())
            .replace("@@ACTUAL_HELPERS@@", helpers)
            .replace("@@ACTUAL_METHODS@@", "\n\n".join(methods.values())))


def compile_and_run(methods: dict[str, str], compiler: str) -> subprocess.CompletedProcess[str]:
    source = fixture_source(methods)
    with tempfile.TemporaryDirectory(prefix="netwin-player-packets-") as temp:
        cpp = Path(temp) / "fixture.cpp"
        exe = Path(temp) / "fixture"
        cpp.write_text(source, encoding="utf-8")
        build = subprocess.run(
            [compiler, "-std=c++03", "-Wall", "-Wextra", "-Werror", str(cpp), "-o", str(exe)],
            text=True, capture_output=True,
        )
        if build.returncode:
            pytest.fail(f"{compiler} compile failed:\n{build.stdout}\n{build.stderr}")
        return subprocess.run([str(exe)], text=True, capture_output=True)


COMPILERS = [pytest.param(path, id=Path(path).name)
             for name in ("g++", "clang++") if (path := shutil.which(name))]


def test_player_count_mapping_is_with_its_method() -> None:
    source = NETWIN_CPP.read_text(encoding="utf-8")
    marker = "// ORIGINAL: 0x0047E010 "
    assert source.count(marker) == 1
    annotation_end = source.index("*/", source.index(marker)) + 2
    assert source[annotation_end:].lstrip().startswith("void NetWin::pick_num_players() {")


@pytest.mark.parametrize("compiler", COMPILERS)
def test_actual_pick_methods(compiler: str) -> None:
    result = compile_and_run(actual_methods(), compiler)
    assert result.returncode == 0, result.stdout + result.stderr


@pytest.mark.parametrize("compiler", COMPILERS)
def test_defined_field_and_row_damage_controls(compiler: str) -> None:
    baseline = actual_methods()
    mutants = (
        ("wrong faction packet zero field", "faction",
         "packet.header_.field_8_ = 0;", "packet.header_.field_8_ = 1;"),
        ("wrong difficulty packet zero field", "difficulty",
         "packet.header_.field_8_ = 0;", "packet.header_.field_8_ = 1;"),
        ("wrong chosen faction byte", "faction",
         "static_cast<int8_t>(chosen_faction)", "static_cast<int8_t>(chosen_faction + 1)"),
        ("omitted copied-row faction override", "faction",
         "updated.faction_or_color_ = static_cast<int8_t>(chosen_faction);",
         "(void)chosen_faction;"),
        ("failed to restore shared row", "faction",
         "NetWinLobbyRows[slot].faction_or_color_ = previous;", "(void)previous;"),
        ("wrong difficulty byte", "difficulty",
         "row.field_2_ = static_cast<uint8_t>(answer);",
         "row.field_2_ = static_cast<uint8_t>(answer + 1);"),
        ("wrong packet slot", "difficulty",
         "packet.header_.slot_ = slot;", "packet.header_.slot_ = slot + 1;"),
    )
    for label, method, old, new in mutants:
        damaged = dict(baseline)
        assert old in damaged[method], f"missing mutation anchor: {label}"
        damaged[method] = damaged[method].replace(old, new, 1)
        result = compile_and_run(damaged, compiler)
        assert result.returncode != 0, f"{label} survived with {compiler}"
