"""Actual-source controls for borrowed MessageDataRecord field reads.

The host fixture deliberately passes misaligned storage and mutates packets
from callbacks. It covers C++ field evaluation only, not the VC6 ABI or game.
"""
from pathlib import Path
import shutil
import subprocess

import pytest

from test_caviar_draw_callbacks_actual_source import extract


ROOT = Path(__file__).resolve().parents[2]
SOURCE = ROOT / "src/netdaemon.cpp"
HANDLERS = (
    "static void netdaemon_handle_server_turn_info",
    "static void netdaemon_handle_server_lock_status",
    "static void netdaemon_handle_client_done_upkeep",
    "static void netdaemon_handle_withdraw_end_turn",
)


PREAMBLE = r'''
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <functional>
#include <string>
#include <vector>

#undef assert
#define assert(condition) do { if (!(condition)) { \
    std::fprintf(stderr, "HARNESS_ASSERT: %s\n", #condition); \
    std::exit(98); \
} } while (0)

/* ACTUAL MESSAGE RECORD */
static_assert(sizeof(MessageDataRecord)==0x20, "wire record width");

/* ACTUAL MESSAGE VIEW */
struct Player {
    int is_leader_female=0, noun_gender=0, is_noun_plural=0;
    char title_leader[16]={}, name_leader[16]={}, noun_faction[16]={};
};
struct ProcessSlot { int identity_=0; char name_[16]={}; };
struct LockMasks { uint32_t enable_mask_=0; };
struct GameFields {
    uint32_t net_turn_flags_=0;
    int turn_current_num_=9;
    LockMasks lock_masks_{};
    uint32_t game_state_=0;
};
struct { GameFields fields_{}; } GameDataStateStorage;
struct Entry { uint32_t leading_words_[4]={}; uint32_t trailing_words_[10]={}; };
struct Primary { uint32_t leading_words_[3]={}; uint32_t trailing_words_[10]={}; };
struct NetDaemon {
    Entry channels_[2]{};
    Primary primary_state_{};
    ProcessSlot process_slots_[8]{};
    int player_count_=0;
    struct { int field_E8_=0; } locks_;
    uint32_t &factions_waiting_mask() { return primary_state_.trailing_words_[7]; }
};
static NetDaemon NetDaemonNet;
static Player Players[8];
static int NetUpkeepPhase=3, NextTurnReady=0, SpockNotifyPending=0;
static unsigned int NetDaemonProgressInterval=0, NetDaemonProgressLastTick=0;
struct { int faction_id_=1; } ConsoleGlobal;
static int GenderDefault=0, PluralityDefault=0;
static int srand_value=0, draw_count=0;
static std::vector<std::string> logs, events;
static std::vector<int> sent_opcodes;
static unsigned char *live_packet=nullptr;
static std::function<void(const std::string&)> on_callback;
static void callback(const std::string &name) {
    events.push_back(name);
    if (on_callback) on_callback(name);
}
static void log_say(const char *name,int a,int b,int c) {
    logs.push_back(std::string(name)+":"+std::to_string(a)+":"+
                   std::to_string(b)+":"+std::to_string(c));
    callback(std::string("log:")+name);
}
static void my_srand(int value) { srand_value=value; callback("seed"); }
static unsigned int timeGetTime() { callback("time"); return 12; }
static int bit_count(int value) { return __builtin_popcount(static_cast<unsigned int>(value)); }
static void parse_says(int,const char*,int,int) { callback("parse"); }
static void message_data(int opcode,int,int,int,int,int) {
    sent_opcodes.push_back(opcode); callback("send");
}
struct Multi { void draw(int) { ++draw_count; callback("draw"); } } MultiWindow;
struct Popup { void pop(const char*,int,int,void*) { callback("pop"); } } NetMsgPrimary;
static void put(unsigned char *p,int offset,int value) {
    std::memcpy(p+offset,&value,sizeof value);
}
static void reset() {
    NetDaemonNet=NetDaemon{};
    GameDataStateStorage=decltype(GameDataStateStorage){};
    GameDataStateStorage.fields_.turn_current_num_=9;
    GameDataStateStorage.fields_.lock_masks_.enable_mask_=0x0e0e;
    NetUpkeepPhase=3; NextTurnReady=0; SpockNotifyPending=0;
    NetDaemonProgressInterval=NetDaemonProgressLastTick=0;
    ConsoleGlobal.faction_id_=1; srand_value=draw_count=0;
    logs.clear(); events.clear(); sent_opcodes.clear(); on_callback={};
}
static bool logged(const char *prefix) {
    for (const auto &entry: logs) if (entry.find(prefix)==0) return true;
    return false;
}
static int logged_value(const char *prefix,int field) {
    for (const auto &entry: logs) if (entry.find(prefix)==0) {
        int a,b,c;
        assert(std::sscanf(entry.c_str()+std::strlen(prefix),":%d:%d:%d",&a,&b,&c)==3);
        return field==0?a:field==1?b:c;
    }
    assert(false); return 0;
}
/* ACTUAL HANDLERS */
'''

MAIN = r'''
int main() {
    alignas(4) unsigned char storage[64]={};
    unsigned char *packet=storage+1;
    live_packet=packet;
    assert(reinterpret_cast<uintptr_t>(packet)%alignof(int)==1);

    // TURN INFO logs entry values, then rereads the packet after the callback.
    reset();
    put(packet,0x10,1); put(packet,0x14,1); put(packet,0x18,8);
    on_callback=[](const std::string &name) {
        if(name=="log:From Server: TURN INFO") {
            put(live_packet,0x10,-1); put(live_packet,0x14,1); put(live_packet,0x18,9);
        }
    };
    netdaemon_handle_server_turn_info(&NetDaemonNet,packet);
    assert(logged_value("From Server: TURN INFO",0)==1);
    assert((NetDaemonNet.channels_[0].leading_words_[0]&0x80000000u)!=0);

    // NEXT UPKEEP rereads signed payloads and timestamp after logging.
    reset(); put(packet,8,0); put(packet,0x10,0); put(packet,0x14,0);
    on_callback=[](const std::string &name) {
        if(name=="log:From Server: NEXT UPKEEP") {
            put(live_packet,8,-123); put(live_packet,0x10,3); put(live_packet,0x14,9);
        }
    };
    netdaemon_handle_server_lock_status(&NetDaemonNet,0x4303,packet,2);
    assert(!logged("(Upkeep instruction for wrong phase!)"));
    assert(!logged("(Upkeep instruction for wrong turn!)"));
    assert(srand_value==-123 && NetDaemonNet.channels_[0].leading_words_[1]==0xFF);

    // DONE UPKEEP uses an unsigned 32-bit faction bit and rereads its final
    // phase after draw invokes a callback.
    reset(); put(packet,0x10,3); put(packet,0x14,9);
    on_callback=[](const std::string &name) {
        if(name=="draw") put(live_packet,0x10,77);
    };
    netdaemon_handle_client_done_upkeep(&NetDaemonNet,packet,-1);
    assert(NetDaemonNet.channels_[0].leading_words_[1]==0x80000000u);
    assert(logged_value("From Client: DONE UPKEEP",1)==77);

    // WITHDRAW reads payload after logging; the callback can make the
    // request current before it clears the owning singleton mask.
    reset(); put(packet,0x10,8);
    NetDaemonNet.primary_state_.trailing_words_[7]=1u<<2;
    on_callback=[](const std::string &name) {
        if(name=="log:From Client: WITHDRAW END-OF-TURN") put(live_packet,0x10,9);
    };
    netdaemon_handle_withdraw_end_turn(&NetDaemonNet,packet,2);
    assert(NetDaemonNet.primary_state_.trailing_words_[7]==0);
    assert(sent_opcodes.size()==1 && sent_opcodes[0]==0x1302);

    // The packet starts at an unaligned address; no aligned dword load or
    // full-record copy is needed for the fields these handlers consume.
    return 0;
}
'''


def source_product() -> tuple[str, str, str]:
    text = SOURCE.read_text()
    record = extract(text, "struct MessageDataRecord {") + ";"
    view = extract(text, "class MessageCommandView {") + ";"
    bodies = "\n".join(extract(text, signature) for signature in HANDLERS)
    return record, view, bodies


def compile_and_run(tmp_path: Path, compiler_name: str,
                    damage: str | None = None) -> subprocess.CompletedProcess:
    compiler = shutil.which(compiler_name)
    if not compiler:
        pytest.skip(f"host {compiler_name} unavailable")
    record, view, bodies = source_product()
    if damage:
        edits = {
            "turn-cache": (
                "if (packet.payload_18() !=\n        GameDataStateStorage.fields_.turn_current_num_)",
                "if (turn !=\n        GameDataStateStorage.fields_.turn_current_num_)"),
            "upkeep-cache": (
                "if (packet.payload_10() != NetUpkeepPhase)",
                "if (phase != NetUpkeepPhase)"),
            "done-upkeep-cache": (
                'packet.payload_10(),\n            static_cast<int>(already_notified)',
                'phase,\n            static_cast<int>(already_notified)'),
            "withdraw-cache": (
                "    const uint32_t sender_bit = static_cast<uint32_t>(1) << (sender & 0x1F);\n    log_say(\"From Client: WITHDRAW END-OF-TURN\", sender, 0, 0);",
                "    const uint32_t sender_bit = static_cast<uint32_t>(1) << (sender & 0x1F);\n    const int packet_turn = packet.payload_10();\n    log_say(\"From Client: WITHDRAW END-OF-TURN\", sender, 0, 0);"),
        }
        before, after = edits[damage]
        assert bodies.count(before)==1, damage
        bodies=bodies.replace(before,after,1)
        if damage=="withdraw-cache":
            before="packet.payload_10() ==\n            GameDataStateStorage.fields_.turn_current_num_"
            after="packet_turn ==\n            GameDataStateStorage.fields_.turn_current_num_"
            assert bodies.count(before)==1
            bodies=bodies.replace(before,after,1)
    cpp=PREAMBLE.replace("/* ACTUAL MESSAGE RECORD */",record).replace(
        "/* ACTUAL MESSAGE VIEW */",view).replace(
        "/* ACTUAL HANDLERS */",bodies)+MAIN
    source=tmp_path/"turn_packets.cpp"
    binary=tmp_path/"turn_packets"
    source.write_text(cpp)
    built=subprocess.run([compiler,"-std=c++11","-O1","-Wall","-Wextra",
                          "-Wno-unused-parameter",str(source),"-o",str(binary)],
                         capture_output=True,text=True,timeout=60)
    assert built.returncode==0,built.stdout+built.stderr
    return subprocess.run([str(binary)],capture_output=True,text=True,timeout=10)


@pytest.mark.parametrize("compiler_name", ["g++", "clang++"])
def test_actual_turn_packet_view_reads_and_callback_reloads(tmp_path, compiler_name):
    result=compile_and_run(tmp_path,compiler_name)
    assert result.returncode==0,result.stdout+result.stderr


@pytest.mark.parametrize("compiler_name", ["g++", "clang++"])
@pytest.mark.parametrize("damage", ["turn-cache","upkeep-cache","done-upkeep-cache","withdraw-cache"])
def test_callback_reload_controls_reject_field_snapshots(tmp_path, compiler_name, damage):
    result=compile_and_run(tmp_path,compiler_name,damage)
    expected_assertion = {
        "turn-cache": "channels_[0].leading_words_[0]",
        "upkeep-cache": "Upkeep instruction for wrong phase",
        "done-upkeep-cache": "logged_value",
        "withdraw-cache": "primary_state_.trailing_words_[7]",
    }[damage]
    assert result.returncode==98, (damage,result.returncode,result.stdout,result.stderr)
    assert "HARNESS_ASSERT:" in result.stderr and expected_assertion in result.stderr, result.stderr


def test_view_uses_actual_record_and_field_sized_memcpy_without_length_policy():
    text=SOURCE.read_text()
    record=extract(text,"struct MessageDataRecord {")
    _,view,_=source_product()
    assert "unresolved_2_[2]" in record
    for offset in ("opcode_","faction_","timestamp_","sequence_","payload_10_","payload_14_","payload_18_","payload_1C_"):
        assert f"offsetof(MessageDataRecord, {offset})" in view
    assert view.count("std::memcpy(&value,")==8
    assert "memcpy(this" not in view and "sizeof(MessageDataRecord)" not in view
    for signature in HANDLERS:
        body=extract(text,signature)
        assert "length" not in body.lower()
        assert "MessageDataRecord packet" not in body
