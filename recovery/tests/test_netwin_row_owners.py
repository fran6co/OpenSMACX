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
static_assert(sizeof(NetWinLobbyRow)==0x17C, "row stride");
static_assert(offsetof(NetWinLobbyRow, owner_)==0x178, "claim field");
static_assert(offsetof(NetWinLobbyRow, faction_or_color_)==3, "mode byte");
NetWinLobbyRow NetWinLobbyRows[8];

struct NetWinSessionBlock { uint32_t mode_; uint8_t state_[0x164]; };
NetWinSessionBlock NetWinSessionData;
static unsigned int *NetWinEighthSlotWord=&NetWinSessionData.mode_;
static unsigned char *NetWinScenarioText=&NetWinSessionData.state_[0x20];
struct Options {
    uint8_t status_, player_count_, difficulty_, time_control_;
    uint8_t option_4_, option_5_, planet_size_, ocean_coverage_;
    uint8_t erosion_, native_life_, cloud_cover_, reserved_B_;
    uint32_t rule_bits_a_, rule_bits_b_, rule_bits_c_;
};
Options NetWinSessionOptions;

static unsigned char config_color=0xA5;
static unsigned char *NetWinConfigColor=&config_color;
static char template_storage[7*0x59C];
static char *NetWinFactionTemplate=template_storage;
alignas(int) static char label_storage[0xFE8];
struct LabelBlock { char *strings_ptr; } Labels={label_storage};
struct StringTableType { char *get(int) { return const_cast<char *>("localized-noun"); } };
static StringTableType StringTable;

struct AlphaNetProcessSlot {
    char name_[0x20]; uint32_t pid_; int8_t identity_; uint8_t f25_,f26_; int8_t f27_;
    uint8_t status_[0x174];
};
static_assert(offsetof(AlphaNetProcessSlot,pid_)==0x20, "process pid");
static_assert(sizeof(AlphaNetProcessSlot)==0x19C, "process stride");
struct AlphaNet { uint8_t prefix_[0x76C]; AlphaNetProcessSlot process_slots_[8]; };
struct NetDaemon : AlphaNet {
    uint32_t last_lookup;
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
    for(int r=0;r<7;++r) {
        char *rec=template_storage+r*0x59C;
        char tag[16]; std::sprintf(tag,"L%d",r); std::strcpy(rec,tag);
        std::sprintf(tag,"FORM%d",r); std::strcpy(rec+0x350,tag);
        std::sprintf(tag,"ADJ%d",r); std::strcpy(rec+0x31C,tag);
        std::sprintf(tag,"TITLE%d",r); std::strcpy(rec+0x18,tag);
        std::sprintf(tag,"LEAD%d",r); std::strcpy(rec+0x30,tag);
        rec[0xC]=static_cast<char>(0x40+r);
    }
    *reinterpret_cast<int *>(label_storage+0xFE4)=73;
    NetWinSessionData.mode_=0x12345678;
    NetWinSessionData.state_[0x20]=0xEE;
    win.config_players();
    if(NetWinLobbyRows[0].prefix_!=0x5A || NetWinLobbyRows[0].owner_!=0x5A5A5A5A)
        fail("config must not touch row zero");
    for(int r=1;r<=7;++r) {
        const NetWinLobbyRow &row=NetWinLobbyRows[r];
        char expected[16]; std::sprintf(expected,"L%d",r-1);
        if(row.prefix_!=0xFF || row.ready_or_occupied_!=0 || row.field_2_!=2 ||
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
    if(NetWinSessionData.mode_!=0 || NetWinSessionData.state_[0x20]!=0 ||
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
    assert "&NetWinLobbyRows[1].owner_" in source
    assert "&NetWinLobbyRows[1].ready_or_occupied_" in source


def test_original_overlay_and_stride_witnesses():
    src=(ROOT/"src/netwin.cpp").read_text()
    assert "// RECOVERY_OBJECT: v2 0x0090DB98 0x0090E778" in src
    assert "// RECOVERY_OBJECT: v2 0x0090E778 0x0090E8E0" in src
    assert "// RECOVERY_OBJECT: v2 0x0090E8E0 0x0090E8F8" in src
    assert 0x0090DB98 + 8*0x17C == 0x0090E778
    assert 0x0090DB98 + 8*0x17C + 0x178 == 0x0090E8F0
    assert 0x0090E778 + 1 == 0x0090E779
    assert "NetWinSessionOptions.rule_bits_b_" in src


def test_complete_painter_bodies_preserve_non_owner_operations():
    """Full source comparisons constrain ordering/branches, not native matching."""
    before=subprocess.check_output(["git","show","0271c80d3:src/netwin.cpp"],
                                   cwd=ROOT,text=True)
    after=(ROOT/"src/netwin.cpp").read_text()
    for name in ("draw_player_names", "draw_diffs", "draw_factions"):
        signature=f"void NetWin::{name}()"
        body=mod.extract_function(before,signature)
        replacements={
            "NetWinSlotRecord *slot_record = NetWinSlotRecords + 1;":
                "AlphaNetProcessSlot *slot_record = NetDaemonNet.process_slots_ + 1;",
            "NetWinSlotRecords + 8": "NetDaemonNet.process_slots_ + 8",
            "NetWinSlotRecords + 1": "NetDaemonNet.process_slots_ + 1",
            "slot_record->id": "slot_record->pid_",
            "    unsigned char *record = reinterpret_cast<unsigned char *>(\n        NetWinFactionTable + 0x2F1);":
                "    NetWinLobbyRow *lobby_row = &NetWinLobbyRows[1];",
            "++row, record += 0x17C, slot_record += 1":
                "++row, ++lobby_row, slot_record += 1",
            "record[-0x177]": "lobby_row->ready_or_occupied_",
            "record[-0x178]": "lobby_row->prefix_",
            "static_cast<signed char>(record[-0x175])": "lobby_row->faction_or_color_",
            "record[-0x175]": "static_cast<uint8_t>(lobby_row->faction_or_color_)",
            "unsigned long claimed =\n                    *reinterpret_cast<unsigned long *>(record);":
                "unsigned long claimed = lobby_row->owner_;",
            "*reinterpret_cast<unsigned long *>(record)": "lobby_row->owner_",
            "reinterpret_cast<const char *>(record - 0x173)": "lobby_row->leader_name_",
            "const char *name =\n                    lobby_row->leader_name_;":
                "const char *name = lobby_row->leader_name_;",
            "const char *scan =\n                    reinterpret_cast<const char *>(NetWinFactionTable + 0x17E);":
                "const NetWinLobbyRow *scan = &NetWinLobbyRows[1];",
            "strcmp(name, scan)": "strcmp(name, scan->leader_name_)",
            "scan += 0x17C;": "++scan;",
            "NetWinFactionTable[pid_idx * 0x17C]":
                "static_cast<uint8_t>(NetWinLobbyRows[pid_idx].faction_or_color_)",
            "    unsigned char *record =\n        reinterpret_cast<unsigned char *>(NetWinFactionTable) + 0x179;\n    for (int index = 0; record < reinterpret_cast<unsigned char *>(NetWinFactionTable)\n                   + 0x179 + 7 * 0x17C;\n         ++index, record += 0x17C, ++rect) {":
                "    NetWinLobbyRow *row = &NetWinLobbyRows[1];\n    for (int index = 0; row < &NetWinLobbyRows[8];\n         ++index, ++row, ++rect) {",
            "    unsigned char *record =\n        reinterpret_cast<unsigned char *>(NetWinFactionTable) + 0x179 + 0x45;\n    for (int index = 0;\n         record < reinterpret_cast<unsigned char *>(NetWinFactionTable)\n                   + 0x179 + 0x45 + 7 * 0x17C;\n         ++index, record += 0x17C, ++rect) {":
                "    NetWinLobbyRow *row = &NetWinLobbyRows[1];\n    for (int index = 0; row < &NetWinLobbyRows[8];\n         ++index, ++row, ++rect) {",
            "record[0]": "row->prefix_",
            "record[2]": "row->field_2_",
            "record[-0x45]": "row->prefix_",
            "static_cast<signed char>(record[-0x42])": "row->faction_or_color_",
            "reinterpret_cast<const char *>(record)": "row->noun_faction_",
        }
        for old,new in replacements.items():
            body=body.replace(old,new)
        assert body == mod.extract_function(after,signature),name
