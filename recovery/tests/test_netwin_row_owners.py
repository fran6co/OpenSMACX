"""Actual-source controls for the proposed NetWin row/process-slot owner patch.

Maintained owner control. It extracts current method bodies unchanged; the fixture is
a host storage projection, not the VC6 ABI or a whole-game proof.
"""
from pathlib import Path
import importlib.util
import os
import re
import shutil
import subprocess

import pytest

ROOT = next(p for p in Path(__file__).resolve().parents
            if (p / "src/netwin.cpp").is_file())
BASE = ROOT / "recovery/tests/test_netwin_owner_behavior.py"
spec = importlib.util.spec_from_file_location("netwin_owner_extract", BASE)
mod = importlib.util.module_from_spec(spec)
spec.loader.exec_module(mod)


def parts():
    src = (ROOT / "src/netwin.cpp").read_text()
    return {
        "BODY_CONFIG": mod.extract_function(src, "void NetWin::config_players()"),
        "BODY_CHAT": mod.extract_function(src, "void NetWin::new_chat(char *message, int slot)"),
    }


HARNESS = r'''
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cctype>

ACTUAL_ROW_DECLARATION
ACTUAL_SESSION_BLOCK_DECLARATION
typedef int BOOL;
ACTUAL_PLAYER_DECLARATION
static Player Players[8];
static_assert(sizeof(NetWinLobbyRow)==0x17C, "row stride");
static_assert(offsetof(NetWinLobbyRow, owner_)==0x178, "claim field");
static_assert(offsetof(NetWinLobbyRow, faction_or_color_)==3, "mode byte");
NetWinLobbyRow NetWinLobbyRows[8];

NetWinSessionBlock NetWinSessionData;
struct Options {
    uint8_t status_, player_count_, difficulty_, time_control_;
    uint8_t option_4_, option_5_, planet_size_, ocean_coverage_;
    uint8_t erosion_, native_life_, cloud_cover_, reserved_B_;
    uint32_t rule_bits_a_, rule_bits_b_, rule_bits_c_;
};
Options NetWinSessionOptions;

alignas(int) static char label_storage[0xFE8];
struct LabelBlock { char *strings_ptr; } Labels={label_storage};
struct StringTableType {
    int calls=0;
    char *get(int) {
        NetWinSessionOptions.difficulty_=static_cast<uint8_t>(3+calls++);
        return const_cast<char *>("localized-noun");
    }
} StringTable;
static char *label_get(int slot) {
    return Labels.strings_ptr != 0 ? StringTable.get(slot) : 0;
}

struct AlphaNetProcessSlot {
    char name_[0x20]; uint32_t pid_; int8_t identity_; uint8_t f25_,f26_; int8_t f27_;
    uint8_t status_[0x174];
};
static_assert(offsetof(AlphaNetProcessSlot,pid_)==0x20, "process pid");
static_assert(sizeof(AlphaNetProcessSlot)==0x19C, "process stride");
struct AlphaNet { uint8_t prefix_[0x76C]; AlphaNetProcessSlot process_slots_[8]; };
struct NetDaemon : AlphaNet {
    uint32_t last_lookup;
    struct AllocationNode { uint32_t max_players_; } node;
    const AllocationNode &allocation_node() const { return node; }
    char *get_player_name(uint32_t id) {
        last_lookup=id;
        static char result[32]; std::sprintf(result,"P%u",id); return result;
    }
};
NetDaemon NetDaemonNet;
static_assert(offsetof(AlphaNet,process_slots_)==0x76C,"process slot member");
static_assert(0x0093CD90 + 0x76C + 0x20 == 0x0093D51C,"pinned alias identity");

struct StringBox {
    char captured[256]; int slot, mode;
    void add(const char *s, int a, int b) {
        std::strcpy(captured,s); slot=a; mode=b;
    }
};
struct NetWin { StringBox stringBox_; void config_players(); void new_chat(char *,int); };
static char *CharUpperA(char *s) { for(char *p=s; *p; ++p) *p=static_cast<char>(std::toupper(static_cast<unsigned char>(*p))); return s; }
BODY_CONFIG
BODY_CHAT

static void fail(const char *s) { std::fprintf(stderr,"FAIL: %s\n",s); std::exit(1); }
int main() {
    NetWin win;
    std::memset(NetWinLobbyRows,0x5A,sizeof(NetWinLobbyRows));
    NetDaemonNet.node.max_players_=0xA5;
    for(int r=1;r<=7;++r) {
        std::sprintf(Players[r].name_leader,"L%d",r-1);
        std::sprintf(Players[r].formal_name_faction,"FORM%d",r-1);
        std::sprintf(Players[r].adj_name_faction,"ADJ%d",r-1);
        std::sprintf(Players[r].title_leader,"TITLE%d",r-1);
        std::sprintf(Players[r].adj_leader,"LEAD%d",r-1);
        Players[r].is_leader_female=0x3F+r;
    }
    *reinterpret_cast<int *>(label_storage+0xFE4)=73;
    NetWinSessionData.mode_=0x12345678;
    NetWinSessionData.scenario_text_wire_tail_[0]=0xEE;
    NetDaemonNet.node.max_players_=0xA5;
    win.config_players();
    if(NetWinLobbyRows[0].prefix_!=0x5A || NetWinLobbyRows[0].owner_!=0x5A5A5A5A)
        fail("config must not touch row zero");
    for(int r=1;r<=7;++r) {
        const NetWinLobbyRow &row=NetWinLobbyRows[r];
        char expected[16]; std::sprintf(expected,"L%d",r-1);
        if(row.prefix_!=0xFF || row.ready_or_occupied_!=0 || row.field_2_!=r+1 ||
           static_cast<uint8_t>(row.faction_or_color_)!=0xFF || row.field_4_!=2)
            fail("typed prefix fields");
        if(std::strcmp(row.leader_name_,expected)!=0) fail("leader row mapping");
        std::sprintf(expected,"FORM%d",r-1);
        if(std::strcmp(row.formal_faction_name_,expected)!=0) fail("formal row mapping");
        if(std::strcmp(row.noun_faction_,"localized-noun")!=0) fail("localized noun row");
        std::sprintf(expected,"ADJ%d",r-1);
        if(std::strcmp(row.adjective_faction_,expected)!=0) fail("adjective row mapping");
        std::sprintf(expected,"TITLE%d",r-1);
        if(std::strcmp(row.leader_title_,expected)!=0) fail("title row mapping");
        std::sprintf(expected,"LEAD%d",r-1);
        if(std::strcmp(row.adjective_leader_,expected)!=0) fail("leader adjective mapping");
        if(row.tail_[0]!=static_cast<uint8_t>(0x3F+r) || row.owner_!=0)
            fail("tail/owner byte lane");
    }
    if(StringTable.calls!=7 || NetWinSessionOptions.difficulty_!=9)
        fail("label callback mutations remain visible to later rows");
    if(NetWinSessionData.mode_!=0 || NetWinSessionData.scenario_text_wire_tail_[0]!=0 ||
       NetWinSessionOptions.player_count_!=0xA5 || NetWinSessionOptions.status_!=0xFF)
        fail("existing neighboring session/options writes");

    NetDaemonNet.process_slots_[4].pid_=0xA1234567u;
    win.new_chat(const_cast<char *>("hello"),4);
    if(NetDaemonNet.last_lookup!=0xA1234567u || std::strcmp(win.stringBox_.captured,"P2703443303:  hello")!=0 ||
       win.stringBox_.slot!=4 || win.stringBox_.mode!=1)
        fail("actual chat body selects typed AlphaNet process slot");
    return 0;
}
'''


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
def test_extracted_config_players_and_new_chat(tmp_path, compiler_name):
    compiler=shutil.which(compiler_name)
    if not compiler: pytest.skip("host C++ compiler unavailable")
    src=HARNESS
    header=(ROOT/"src/netwin.h").read_text()
    declaration=re.search(r"struct NetWinLobbyRow \{.*?\n\};", header, re.S)
    assert declaration is not None
    src=src.replace("ACTUAL_ROW_DECLARATION", declaration.group())
    session_declaration=re.search(r"struct NetWinSessionBlock \{.*?\n\};", header, re.S)
    assert session_declaration is not None
    src=src.replace("ACTUAL_SESSION_BLOCK_DECLARATION", session_declaration.group())
    player_header=(ROOT/"src/faction.h").read_text()
    player_declaration=re.search(r"struct Player \{.*?\n\};", player_header, re.S)
    assert player_declaration is not None
    src=src.replace("ACTUAL_PLAYER_DECLARATION", player_declaration.group())
    for marker,body in parts().items(): src=src.replace(marker,body)
    cpp=tmp_path/"netwin-row-owner.cpp"; exe=tmp_path/"netwin-row-owner"
    cpp.write_text(src)
    built=subprocess.run([compiler,"-std=c++17","-Wall","-Wextra","-Werror","-Wno-invalid-offsetof",
                          "-fsanitize=address,undefined","-fno-sanitize-recover=all",str(cpp),"-o",str(exe)],
                         capture_output=True,text=True,timeout=30)
    assert built.returncode==0,built.stderr
    run=subprocess.run([str(exe)],capture_output=True,text=True,timeout=10,
                       env=dict(os.environ,ASAN_OPTIONS="detect_leaks=0"))
    assert run.returncode==0,run.stderr


def test_candidate_closes_all_table_aliases_but_not_buffer_scope():
    source=(ROOT/"src/netwin.cpp").read_text()
    assert "NetWinFactionTable" not in source
    assert "NetWinSlotRecords" not in source
    assert "NetWinLobbyRows[1]" in source
    assert "NetDaemonNet.process_slots_" in source
    # One-past row 8 may be a loop endpoint; there must not be a ninth-row
    # field access. Buffer/RECT receiver substitutions are owned separately.
    assert not __import__("re").search(r"NetWinLobbyRows\s*\[\s*8\s*\]\s*\.", source)
    session = mod.extract_function(source, "void NetWin::draw_session_info()")
    assert "const NetWinLobbyRow *scan = &NetWinLobbyRows[1];" in session
    assert "scan->owner_ == owned" in session
    assert "++scan;" in session
    assert "reinterpret_cast<unsigned long *>" not in session
    assert "scan += 0x5F" not in session
    players = mod.extract_function(source, "void NetWin::draw_player_names()")
    assert "NetWinLobbyRow *lobby_row = &NetWinLobbyRows[1];" in players
    assert "++lobby_row" in players
    assert "lobby_row->ready_or_occupied_" in players


def test_original_overlay_and_stride_witnesses():
    src=(ROOT/"src/netwin.cpp").read_text()
    assert "// RECOVERY_OBJECT: v2 0x0090DB98 0x0090E778" in src
    assert "// RECOVERY_OBJECT: v2 0x0090E778 0x0090E8E0" in src
    assert "// RECOVERY_OBJECT: v2 0x0090E8E0 0x0090E8F8" in src
    assert 0x0090DB98 + 8*0x17C == 0x0090E778
    assert 0x0090DB98 + 8*0x17C + 0x178 == 0x0090E8F0
    assert 0x0090E778 + 1 == 0x0090E779
    assert "NetWinSessionOptions.rule_bits_b_" in src


