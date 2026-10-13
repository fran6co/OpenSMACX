"""Source controls for NetWin's shared session and player assets."""

from pathlib import Path
import importlib.util
import os
import re
import shutil
import subprocess

import pytest


ROOT = Path(__file__).resolve().parents[2]
SOURCE = (ROOT / "src/netwin.cpp").read_text()
HEADER = (ROOT / "src/net_class.h").read_text()
EXTRACTOR = ROOT / "recovery/tests/test_netwin_owner_behavior.py"
SPEC = importlib.util.spec_from_file_location("netwin_owner_extract", EXTRACTOR)
EXTRACT = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(EXTRACT)


def _body(signature: str) -> str:
    start = SOURCE.index(signature)
    opening = SOURCE.index("{", start)
    depth = 0
    for index in range(opening, len(SOURCE)):
        if SOURCE[index] == "{":
            depth += 1
        elif SOURCE[index] == "}":
            depth -= 1
            if depth == 0:
                return SOURCE[opening + 1 : index]
    raise AssertionError(f"unclosed function: {signature}")


def test_config_players_reads_the_max_player_member_as_a_byte():
    body = _body("void NetWin::config_players()")
    assert "reinterpret_cast<const unsigned char *>(" in body
    assert "&NetDaemonNet.allocation_node().max_players_)[0]" in body
    assert "NetWinConfigColor" not in body
    assert "0x0093D4B0" not in SOURCE


def test_config_players_uses_typed_player_fields_and_native_copy_order():
    body = _body("void NetWin::config_players()")
    assert "Player *player = &Players[1];" in body
    assert "++player;" in body
    copies = [
        "strcpy(row->leader_name_, player->name_leader)",
        "strcpy(row->formal_faction_name_, player->formal_name_faction)",
        "strcpy(row->adjective_faction_, player->adj_name_faction)",
        "strcpy(row->leader_title_, player->title_leader)",
        "strcpy(row->adjective_leader_, player->adj_leader)",
        "&player->is_leader_female)[0]",
    ]
    offsets = [body.index(item) for item in copies]
    assert offsets == sorted(offsets)
    assert "row->field_2_ = NetWinSessionOptions.difficulty_" in body
    assert "template_record += 0x59C" not in body


def test_session_name_reloads_the_shared_node_field_in_native_order():
    body = _body("void NetWin::draw_session_info()")
    field = "NetDaemonNet.allocation_node().session_name_"
    assert body.count(field) == 2
    assert body.index(f"if ({field} != 0)") < body.index(
        f"strcat(StringTemp, {field})"
    )
    assert "NetWinSessionName" not in body
    assert "char *session_name" not in body
    assert "const NetAllocationNode &allocation_node() const" in HEADER


def test_ready_styles_and_difficulty_names_use_existing_arrays():
    names = _body("void NetWin::draw_player_names()")
    assert "&InterfaceBoxStyles[48 + ready]" in names
    assert "NetWinReadyPanels" not in names
    assert "NetWinDifficultyNames" not in SOURCE
    assert "Difficulty[row->field_2_]" in _body("void NetWin::draw_diffs()")
    assert "Difficulty[index]" in _body("void NetWin::pick_diff(int slot)")


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
def test_extracted_config_players_preserves_byte_and_player_array_behavior(
    tmp_path, compiler_name
):
    compiler = shutil.which(compiler_name)
    if not compiler:
        pytest.skip("host C++ compiler unavailable")
    row_decl = re.search(r"struct NetWinLobbyRow \{.*?\n\};",
                         (ROOT / "src/netwin.h").read_text(), re.S)
    assert row_decl
    session_decl = re.search(r"struct NetWinSessionBlock \{.*?\n\};",
                             (ROOT / "src/netwin.h").read_text(), re.S)
    assert session_decl
    player_decl = re.search(r"struct Player \{.*?\n\};",
                            (ROOT / "src/faction.h").read_text(), re.S)
    assert player_decl
    harness = r'''
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cstddef>
ROW_DECL
SESSION_DECL
static NetWinLobbyRow NetWinLobbyRows[8];
typedef int BOOL;
ACTUAL_PLAYER_DECLARATION
static_assert(offsetof(Player, is_leader_female)==0, "leader gender byte offset");
static_assert(offsetof(Player, name_leader)==0x34, "leader name offset");
static_assert(offsetof(Player, adj_name_faction)==0x304, "faction name adjective offset");
static_assert(offsetof(Player, formal_name_faction)==0x384, "formal name offset");
static_assert(sizeof(Player)==0x59C, "native player array stride");
static Player Players[8];
struct NetAllocationNode { uint32_t max_players_; };
struct Net { NetAllocationNode node; const NetAllocationNode &allocation_node() const { return node; } };
static Net NetDaemonNet;
static NetWinSessionBlock NetWinSessionData;
static unsigned int *NetWinEighthSlotWord=&NetWinSessionData.mode_;
static char *NetWinScenarioText=NetWinSessionData.scenario_text_wire_tail_;
struct Options { uint8_t status_, option_4_, option_5_, reserved_B_, player_count_, difficulty_, planet_size_, ocean_coverage_, erosion_, native_life_, cloud_cover_; };
static Options NetWinSessionOptions;
static char noun[]="localized noun";
static char *label_get(int) { return noun; }
struct NetWin { void config_players(); };
BODY_CONFIG
int main() {
    NetDaemonNet.node.max_players_=0x1234A5;
    for (int i=1; i<8; ++i) {
        Players[i].is_leader_female=static_cast<BOOL>(0x12340000 | (0x30+i));
        std::sprintf(Players[i].name_leader,"Leader%d",i);
        std::sprintf(Players[i].formal_name_faction,"Formal%d",i);
        std::sprintf(Players[i].adj_name_faction,"AdjFaction%d",i);
        std::sprintf(Players[i].title_leader,"Title%d",i);
        std::sprintf(Players[i].adj_leader,"AdjLeader%d",i);
    }
    NetWin win;
    win.config_players();
    if (NetWinSessionOptions.player_count_!=0xA5) return 1;
    for (int i=1; i<8; ++i) {
        const NetWinLobbyRow &row=NetWinLobbyRows[i];
        char expected[32];
        std::sprintf(expected,"Leader%d",i);
        if (std::strcmp(row.leader_name_,expected)) return 2;
        std::sprintf(expected,"Formal%d",i);
        if (std::strcmp(row.formal_faction_name_,expected)) return 3;
        std::sprintf(expected,"AdjFaction%d",i);
        if (std::strcmp(row.adjective_faction_,expected)) return 4;
        std::sprintf(expected,"Title%d",i);
        if (std::strcmp(row.leader_title_,expected)) return 5;
        std::sprintf(expected,"AdjLeader%d",i);
        if (std::strcmp(row.adjective_leader_,expected)) return 6;
        if (row.tail_[0]!=static_cast<unsigned char>(0x30+i)) return 7;
        if (row.field_2_!=2) return 8;
    }
    return 0;
}
'''
    harness = harness.replace("ROW_DECL", row_decl.group())
    harness = harness.replace("SESSION_DECL", session_decl.group())
    harness = harness.replace("ACTUAL_PLAYER_DECLARATION", player_decl.group())
    harness = harness.replace(
        "BODY_CONFIG", EXTRACT.extract_function(SOURCE, "void NetWin::config_players()")
    )
    cpp = tmp_path / "netwin-session-assets.cpp"
    exe = tmp_path / "netwin-session-assets"
    cpp.write_text(harness)
    built = subprocess.run(
        [compiler, "-std=c++17", "-Wall", "-Wextra", "-Werror", str(cpp), "-o", str(exe)],
        capture_output=True, text=True, timeout=30,
    )
    assert built.returncode == 0, built.stderr
    run = subprocess.run([str(exe)], capture_output=True, text=True, timeout=10,
                         env=dict(os.environ))
    assert run.returncode == 0, run.stderr
