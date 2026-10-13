"""Host controls for the typed NetWin faction-record owner closure.

These compile actual extracted methods. The fixture models field behavior and
allocation-byte boundary effects; it is not evidence of the target ABI.
"""

from pathlib import Path
import shutil
import subprocess

import pytest
from test_net_hotseat_owner_actual_source_oct6 import installed_packet_declarations


ROOT = Path(__file__).resolve().parents[2]
SOURCE = (ROOT / "src/netwin.cpp").read_text()
HEADER = (ROOT / "src/netwin.h").read_text()


def _definition(text: str, signature: str) -> str:
    start = text.index(signature)
    opening = text.index("{", start)
    depth = 0
    state = "code"
    index = opening
    while index < len(text):
        char = text[index]
        nxt = text[index + 1] if index + 1 < len(text) else ""
        if state == "code":
            if char == "/" and nxt == "/":
                state = "line"
                index += 2
                continue
            if char == "/" and nxt == "*":
                state = "block"
                index += 2
                continue
            if char == '"':
                state = "string"
            elif char == "'":
                state = "character"
            elif char == "{":
                depth += 1
            elif char == "}":
                depth -= 1
                if depth == 0:
                    return text[start:index + 1]
        elif state == "line":
            if char == "\n":
                state = "code"
        elif state == "block":
            if char == "*" and nxt == "/":
                state = "code"
                index += 2
                continue
        elif state in ("string", "character"):
            if char == "\\":
                index += 2
                continue
            if (state == "string" and char == '"') or (
                state == "character" and char == "'"
            ):
                state = "code"
        index += 1
    raise AssertionError(f"unterminated definition: {signature}")


def _struct(text: str, name: str) -> str:
    return _definition(text, f"struct {name}")


def test_faction_record_arrays_have_typed_members_without_changing_stride():
    assert "NetWinFactionRecord *faction_records_;" in HEADER
    assert "NetWinFactionRecord *faction_records_copy_;" in HEADER
    assert "sizeof(NetWinFactionRecord) == 0x190" in HEADER
    for field, offset in (
        ("record_type_", "+0"),
        ("record_count_", "+1"),
        ("owner_", "4"),
        ("color_", "8"),
        ("active_", "+9"),
        ("filename_", "0xA"),
        ("leader_name_", "0x22"),
        ("formal_faction_name_", "0x3A"),
        ("noun_faction_", "0x62"),
        ("adjective_faction_", "0x7A"),
        ("leader_title_", "0xFA"),
        ("unresolved_112_", "0x112"),
    ):
        assert field in _struct(HEADER, "NetWinFactionRecord")
        assert offset in _struct(HEADER, "NetWinFactionRecord")


def test_producer_and_all_record_consumers_use_typed_in_stride_fields():
    producer = _definition(SOURCE, "void NetWin::setup_faction_data(")
    assert "records[record_index].owner_ = 0;" in producer
    assert "records[record_index].color_ =" in producer
    assert "records[record_index].leader_name_" in producer
    assert "records[record_index].formal_faction_name_" in producer
    assert "records[record_index].noun_faction_" in producer
    assert "records[record_index].adjective_faction_" in producer
    assert "records[record_index].leader_title_" in producer
    assert "strcpy(records[record_index].adjective_faction_," in producer
    assert "strcpy(records[record_index].leader_title_," in producer
    assert "records[record_index].active_ = 1;" in producer
    assert "allocation_bytes + offset + 0x112" in producer
    assert "allocation_bytes[offset + 0x192]" in producer
    assert "record_count * 399 + 0x194" in producer
    assert "return;" in producer  # preserve each original section-open failure

    fill = _definition(SOURCE, "void NetWin::fill_player(")
    assert "void NetWin::fill_player(NetWinLobbyRowView lobby_row, int slot)" in fill
    assert "const NetWinFactionRecord &record = faction_records_[slot];" in fill
    assert "record.leader_name_" in fill
    assert "record.formal_faction_name_" in fill
    assert "record.noun_faction_" in fill
    assert "record.adjective_faction_" in fill
    assert "record.leader_title_" in fill
    for field in ("leader_name_", "formal_faction_name_", "noun_faction_",
                  "adjective_faction_", "leader_title_", "adjective_leader_"):
        assert f"lobby_row.{field[:-1]}()" in fill
    assert "lobby_row.set_tail_0(" in fill
    assert "slot * 0x190" in fill
    assert "allocation_bytes[slot * 0x190 + 0x192]" in fill
    assert "strcpy(lobby_row.adjective_faction(), record.adjective_faction_);" in fill
    assert "strcpy(lobby_row.leader_title(), record.leader_title_);" in fill
    assert "memset(" not in producer
    read_faction = producer.index("read_faction(&player, 0)")
    assert "records = faction_records_;" in producer[read_faction:]

    assert "faction_records_[0].record_type_" in _definition(
        SOURCE, "void NetWin::draw_diffs()"
    )
    assert "faction_records_[0].record_type_" in _definition(
        SOURCE, "void NetWin::draw_factions()"
    )
    assert "faction_records_[mode].owner_" in _definition(
        SOURCE, "void NetWin::draw_player_names()"
    )
    assert "faction_records_[mode].noun_faction_" in _definition(
        SOURCE, "void NetWin::draw_factions()"
    )
    assert "faction_records_[8 - slot].owner_" in _definition(
        SOURCE, "void NetWin::draw_session_info()"
    )
    assert "NetWinDialog.faction_records_[0].record_count_" in _definition(
        SOURCE, "void __cdecl send_faction_data(int slot_id)"
    )


def test_bounded_netwin_member_apis_match_existing_layout_accessors():
    key_body = _definition(SOURCE, "int NetWin::on_key_down(int key)")
    assert "editBox_.text_" in key_body
    assert "reinterpret_cast<char *>(this) + 0x42F4" not in key_body
    assert "void set_wrap_width(uint32_t width) { field_10_ = width; }" in (
        ROOT / "src/buffer.h"
    ).read_text()
    draw = _definition(SOURCE, "void NetWin::draw_session_info()")
    assert "buffer->set_wrap_width(0x14);" in draw
    assert "reinterpret_cast<unsigned char *>(buffer)" not in draw


SETUP_FIXTURE = r"""
#include <cassert>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>
RECORD_TYPE
MESSAGE_TYPE
struct Player {
    char filename[24], search_key[24], name_leader[24], formal_name_faction[40];
    char noun_faction[24], adj_name_faction[128], title_leader[24], adj_leader[128];
    int is_leader_female;
};
static char StringTemp[512];
static char NetWinPcxSuffix[] = ".pcx";
static char NetWinDefaultPcx[] = "default.pcx";
static const int AlphaxFileID = 1;
static const int NetWinSectionCustomCount = 2;
static const int NetWinSectionFactions = 3;
static const int NetWinSectionNewFactions = 4;
static const int NetWinSectionCustom = 5;
static int NetWinPlayerId = 10, NetWinOtherPlayerId = 11;
static int active_section, text_index, current_file, fail_factions;
static int close_calls, read_calls;
static unsigned last_allocation_size;
static char current_item[32];
static char *text_item() { return current_item; }
static void text_get() {
    if (current_file) {
        std::strcpy(current_item, "leader-source");
    } else if (active_section == NetWinSectionCustomCount) {
        current_item[0] = 0;
    } else if (active_section == NetWinSectionFactions) {
        std::sprintf(current_item, "F%d", text_index++);
    } else if (active_section == NetWinSectionNewFactions) {
        std::sprintf(current_item, "E%d", text_index++);
    } else {
        current_item[0] = 0;
    }
}
static int text_open(int, int section) {
    active_section = section;
    text_index = 0;
    if (section == NetWinSectionFactions && fail_factions) return 1;
    return 0;
}
static int text_open(char *, char *) { current_file = 1; return 0; }
static void text_close() { ++close_calls; current_file = 0; }
static void *mem_get(unsigned size) {
    last_allocation_size = size;
    void *memory = std::malloc(size);
    std::memset(memory, 0xCC, size);
    return memory;
}
static void memcpy_call(void *dst, const void *src, unsigned size) {
    std::memcpy(dst, src, size);
}
struct Buffer {
    int load_pcx(char *, int, int, int) { return 0; }
    int get_pixel(int, int) { return 0xA7; }
};
static void read_faction(Player *player, int) {
    const int row = read_calls++;
    std::strcpy(player->name_leader, "FactionLeader");
    std::strcpy(player->formal_name_faction, "FormalName");
    std::strcpy(player->noun_faction, "Noun");
    std::strcpy(player->adj_name_faction, "FactionAdjective");
    std::strcpy(player->title_leader, "LeaderTitle");
    std::memset(player->adj_leader, 'a' + row % 20, sizeof(player->adj_leader));
    player->adj_leader[0x7F] = 0;
    player->is_leader_female = 0x60 + row;
}
struct NetWin {
    NetWinFactionRecord *faction_records_ = nullptr;
    NetWinFactionRecord *faction_records_copy_ = nullptr;
    void setup_faction_data(MessageFactionData *);
};
SETUP_BODY
int main() {
    NetWin win;
    win.setup_faction_data(nullptr);
    assert(win.faction_records_ != nullptr && win.faction_records_copy_ != nullptr);
    assert(win.faction_records_[0].record_type_ == 7);
    assert(win.faction_records_[0].record_count_ == 14);
    assert(last_allocation_size == 14 * 399 + 0x194);
    assert(win.faction_records_[0].active_ == 1 && win.faction_records_[13].active_ == 1);
    assert(win.faction_records_[0].color_ == 0xA7);
    assert(std::strcmp(win.faction_records_[0].filename_, "F0") == 0);
    assert(std::strcmp(win.faction_records_[7].filename_, "E0") == 0);
    assert(std::strcmp(win.faction_records_[0].leader_name_, "FactionLeader") == 0);
    assert(std::strcmp(win.faction_records_[0].formal_faction_name_, "FormalName") == 0);
    assert(std::strcmp(win.faction_records_[0].noun_faction_, "Noun") == 0);
    assert(std::strcmp(win.faction_records_[0].adjective_faction_,
                       "FactionAdjective") == 0);
    assert(std::strcmp(win.faction_records_[0].leader_title_,
                       "LeaderTitle") == 0);
    const unsigned char *bytes = reinterpret_cast<const unsigned char *>(win.faction_records_);
    assert(bytes[2] == 0xCC && bytes[3] == 0xCC); // original padding is not cleared
    assert(bytes[0x193] == 0xCC); // gender store leaves the following byte untouched
    assert(bytes[0x192] == 0x60); // row-zero write aliases row one's +2 bytes
    assert(bytes[0x112] == 'a' && bytes[0x191] == 0);
    assert(bytes[13 * 0x190 + 0x192] == 0x6D); // last-row spill stays allocated
    assert(bytes[13 * 0x190 + 0x112] == 'n');
    assert(bytes[13 * 0x190 + 0x191] == 0);
    assert(std::memcmp(win.faction_records_, win.faction_records_copy_,
                       14 * 399 + 0x194) == 0);
    assert(close_calls == 1 && read_calls == 14);
    std::free(win.faction_records_);
    std::free(win.faction_records_copy_);

    NetWin failed;
    fail_factions = 1;
    failed.setup_faction_data(nullptr);
    assert(failed.faction_records_ != nullptr && failed.faction_records_copy_ != nullptr);
    assert(close_calls == 1); // this early return does not close the text source
    std::free(failed.faction_records_);
    std::free(failed.faction_records_copy_);
    return 0;
}
"""


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
def test_actual_setup_faction_data_runtime_fields_spill_and_failure(
    tmp_path: Path, compiler_name: str
):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(f"{compiler_name} unavailable")
    record = _struct(HEADER, "NetWinFactionRecord") + ";"
    message = _struct(HEADER, "MessageFactionData") + ";"
    body = _definition(SOURCE, "void NetWin::setup_faction_data(")
    # Strip the repository's source qualification only by providing the same
    # method signature and the small set of dependencies this body calls.
    harness = (SETUP_FIXTURE.replace("RECORD_TYPE", record)
              .replace("MESSAGE_TYPE", message).replace("SETUP_BODY", body))
    cpp = tmp_path / "netwin-setup-faction-data.cpp"
    exe = tmp_path / "netwin-setup-faction-data"
    cpp.write_text(harness)
    built = subprocess.run(
        [compiler, "-std=c++17", "-Wall", "-Wextra", "-Werror",
         str(cpp), "-o", str(exe)],
        capture_output=True, text=True, timeout=30, check=False,
    )
    assert built.returncode == 0, built.stdout + built.stderr
    ran = subprocess.run([str(exe)], capture_output=True, text=True,
                         timeout=10, check=False)
    assert ran.returncode == 0, ran.stdout + ran.stderr


FILL_FIXTURE = r"""
#include <cassert>
#include <cstdint>
#include <cstring>
#include <cstddef>
ACTUAL_TYPES
static_assert(sizeof(NetWinFactionRecord) == 0x190, "record stride");
static_assert(offsetof(NetWinFactionRecord, adjective_faction_) == 0x7A,
              "faction adjective slot");
static_assert(offsetof(NetWinFactionRecord, leader_title_) == 0xFA,
              "leader title slot");
static char *label_get(int) { return const_cast<char *>("fallback"); }
class NetWin {
public:
    NetWinFactionRecord *faction_records_;
    void fill_player(NetWinLobbyRowView, int);
};
FILL_BODY
int run_control() {
    NetWinFactionRecord records[3] = {};
    NetWin win;
    win.faction_records_ = records;
    std::strcpy(records[1].leader_name_, "leader");
    std::strcpy(records[1].formal_faction_name_, "formal");
    std::strcpy(records[1].noun_faction_, "noun");
    std::strcpy(records[1].adjective_faction_, "adj-faction");
    std::strcpy(records[1].leader_title_, "title");
    unsigned char *bytes = reinterpret_cast<unsigned char *>(records);
    char *adjective = reinterpret_cast<char *>(bytes + 0x190 + 0x112);
    std::memset(adjective, 'x', 0x7F);
    adjective[0x7F] = 0;
    bytes[0x190 + 0x192] = 0xE7;
    NetWinLobbyRow player = {};
    win.fill_player(NetWinLobbyRowView(player), 1);
    assert(std::strcmp(player.leader_name_, "leader") == 0);
    assert(std::strcmp(player.formal_faction_name_, "formal") == 0);
    assert(std::strcmp(player.noun_faction_, "noun") == 0);
    assert(std::strcmp(player.adjective_faction_, "adj-faction") == 0);
    assert(std::strcmp(player.leader_title_, "title") == 0);
    assert(player.adjective_leader_[0] == 'x');
    assert(player.adjective_leader_[0x7E] == 'x');
    assert(player.adjective_leader_[0x7F] == 0);
    assert(player.tail_[0] == 0xE7);
    std::strcpy(player.noun_faction_, "old");
    win.fill_player(NetWinLobbyRowView(player), -1);
    assert(std::strcmp(player.noun_faction_, "fallback") == 0);
    assert(std::strcmp(player.leader_name_, "leader") == 0);

    // A received row is still only bytes. The view points into the live
    // packet payload and writes the observed row slots without constructing a
    // NetWinLobbyRow over that storage or copying through a temporary.
    char message[0x18C];
    std::memset(message, 0xA9, sizeof message);
    NetWinLobbyRowView received(message + 0x10);
    assert(received.leader_name() == message + 0x10 + offsetof(NetWinLobbyRow, leader_name_));
    assert(received.adjective_leader() == message + 0x10 + offsetof(NetWinLobbyRow, adjective_leader_));
    win.fill_player(received, 1);
    assert(std::strcmp(received.leader_name(), "leader") == 0);
    assert(std::strcmp(received.formal_faction_name(), "formal") == 0);
    assert(std::strcmp(received.adjective_leader(), adjective) == 0);
    assert(static_cast<unsigned char>(message[0x10 + offsetof(NetWinLobbyRow, tail_)]) == 0xE7);
    assert(static_cast<unsigned char>(message[0x10 + offsetof(NetWinLobbyRow, prefix_)]) == 0xA9);
    assert(static_cast<unsigned char>(message[0x10 + offsetof(NetWinLobbyRow, owner_)]) == 0xA9);
    return 0;
}
int main() { return run_control(); }
"""


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
def test_actual_fill_player_boundary_runtime_and_mutants(
    tmp_path: Path, compiler_name: str
):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(f"{compiler_name} unavailable")
    body = _definition(SOURCE, "void NetWin::fill_player(")
    types = installed_packet_declarations(("NetWinFactionRecord", "NetWinLobbyRow",
                                           "NetWinLobbyRowView"))

    def compile_run(method: str, expected: bool, label: str) -> None:
        harness = FILL_FIXTURE.replace("ACTUAL_TYPES", types).replace("FILL_BODY", method)
        cpp = tmp_path / f"netwin-fill-{label}.cpp"
        exe = tmp_path / f"netwin-fill-{label}"
        cpp.write_text(harness)
        built = subprocess.run(
            [compiler, "-std=c++17", "-Wall", "-Wextra", "-Werror",
             str(cpp), "-o", str(exe)],
            capture_output=True, text=True, timeout=30, check=False,
        )
        assert built.returncode == 0, built.stdout + built.stderr
        ran = subprocess.run([str(exe)], capture_output=True, text=True,
                             timeout=10, check=False)
        assert (ran.returncode == 0) is expected, ran.stdout + ran.stderr

    compile_run(body, True, "current")
    compile_run(body.replace("record.leader_name_", "record.formal_faction_name_", 1),
                False, "wrong-field")
    compile_run(body.replace("slot * 0x190\n                                          + 0x112",
                             "slot * 0x190\n                                          + 0x111", 1),
                False, "wrong-string-boundary")
    compile_run(body.replace("slot * 0x190 + 0x192",
                             "slot * 0x190 + 0x190", 1),
                False, "wrong-spill-byte")
