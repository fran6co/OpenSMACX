"""Actual DOUBLE CROSS helper with a packet inside actual receiver storage.

These allocated alias/callback controls establish source behavior; independent
pinned instructions establish original capture/store ordering. They do not
establish the real Windows class ABI, linked preservation, or game acceptance.
"""
from pathlib import Path
import os
import re
import shutil
import subprocess

import pytest

from test_netdaemon_early_routes_oct12 import declaration, PIN
from test_netdaemon_synch_dispatch import _definition, _mask

ROOT=Path(__file__).resolve().parents[2]


def class_declaration(text,name):
    code=_mask(text)
    matches=list(re.finditer(r"\bclass\s+"+name+r"\s*\{",code))
    assert len(matches)==1,name
    start=matches[0].start();opening=matches[0].end()-1;depth=0
    for end in range(opening,len(code)):
        depth+=(code[end]=="{")-(code[end]=="}")
        if depth==0:
            assert code[end+1]==";"
            return text[start:end+2]
    raise AssertionError(name)


def material():
    source=(ROOT/"src/netdaemon.cpp").read_text();header=(ROOT/"src/netdaemon.h").read_text()
    owner=class_declaration((ROOT/"src/deletionlist.h").read_text(),"DeletionList")
    record=declaration((ROOT/"src/deletionlist.h").read_text(),"DeletionRecord")
    declarations="\n".join((record,owner,declaration(header,"PrimaryLockStateTail"),
                               declaration(header,"LockStateEntry"),
                               declaration(source,"MessageDataRecord")))
    declarations+="\n"+class_declaration(source,"MessageCommandView")
    completion=_definition(source,r"static\s+void\s+netdaemon_process_local_player_completion","completion")
    helper=_definition(source,r"static\s+bool\s+netdaemon_handle_five_message_routes","routes")
    return declarations,completion+"\n"+helper


HARNESS=r'''
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <functional>
#include <string>
#include <vector>
/* ACTUAL DECLARATIONS */
DeletionList::DeletionList() { std::memset(records_,0,sizeof records_); current_owner_=0; }
struct NetDaemon {
    DeletionList deletion_list_;
    PrimaryLockStateTail primary_state_{};
    LockStateEntry channels_[2]{};
} NetDaemonNet;
static_assert(offsetof(NetDaemon,primary_state_)==sizeof(DeletionList),"receiver siblings");
static struct { int faction_id_; } ConsoleGlobal;
static struct { uint8_t faction_id; int8_t order; } Vehs[8];
static int SpockNotifyPending;
static std::vector<std::string> events;
static std::function<void(const std::string&)> callback;
static int cross_args[3],seed;
static void event(const std::string&s) { events.push_back(s);if(callback)callback(s); }
static void log_say(const char*s,int,int,int) { event(std::string("log:")+s); }
static void my_srand(int n) { seed=n;event("seed"); }
static void message_data(int opcode,int target,int,int,int,int) { assert(opcode==0x2214 || opcode==0x2308);assert(target==0);event("send"); }
static void order_veh(int,int,int) { event("order"); }
static void battle_fight(int,int,int,int,void*) { event("fight"); }
static void action_airdrop(int,int,int,int) { event("airdrop"); }
static void action(int) { event("action"); }
static void double_cross(int a,int b,int c) { cross_args[0]=a;cross_args[1]=b;cross_args[2]=c;event("cross"); }
/* ACTUAL HELPERS */
static void put(unsigned char *p,int offset,int value) { std::memcpy(p+offset,&value,4); }
static void reset(NetDaemon &daemon) {
    callback={};events.clear();daemon=NetDaemon{};NetDaemonNet=NetDaemon{};
    ConsoleGlobal.faction_id_=3;daemon.channels_[1].leading_words_[3]=9;
    std::memset(cross_args,0,sizeof cross_args);seed=-1;
}
static unsigned char *owner_packet(NetDaemon &daemon,int faction,int sequence) {
    // Byte view of the whole actual allocated receiver. Packet faction is
    // current_owner_; sequence is primary.leading_words_[1]. The host/timestamp
    // and three payloads lie in the same owner's adjacent primary state.
    unsigned char *begin=reinterpret_cast<unsigned char *>(&daemon);
    const size_t offset=offsetof(NetDaemon,deletion_list_)+offsetof(DeletionList,current_owner_)-4;
    assert(offset+0x1c<=sizeof daemon);
    unsigned char *packet=begin+offset;
    put(packet,0,0x4440);
    put(packet,4,faction);put(packet,8,0);put(packet,0xc,sequence);
    put(packet,0x10,1);put(packet,0x14,2);put(packet,0x18,3);
    return packet;
}
int main() {
    NetDaemon daemon;
    // Store changes packet faction3→7, but native compares the captured3 and
    // completes locally. Entry arguments match the original dispatcher capture;
    // later callback cases separately distinguish them from post-action reads.
    reset(daemon);unsigned char *packet=owner_packet(daemon,3,7);
    assert(netdaemon_handle_five_message_routes(daemon,0x4440,packet,3,7));
    assert(daemon.deletion_list_.sequence_bits()==7 && daemon.channels_[1].leading_words_[3]==0);
    assert(seed==0 && cross_args[0]==1 && cross_args[1]==2 && cross_args[2]==3);
    // Converse witness: store changes faction7→3, but captured7 is remote.
    reset(daemon);packet=owner_packet(daemon,7,3);
    assert(netdaemon_handle_five_message_routes(daemon,0x4440,packet,7,3));
    assert(daemon.deletion_list_.sequence_bits()==3 && daemon.channels_[1].leading_words_[3]==9);
    // After the action callback, reload sequence/faction/host, then capture
    // faction before the sequence-owner write. Do not reuse dispatcher captures.
    reset(daemon);packet=owner_packet(daemon,7,3);
    callback=[&](const std::string&s) { if(s=="cross") { put(packet,4,3);put(packet,0xc,8); } };
    assert(netdaemon_handle_five_message_routes(daemon,0x4440,packet,7,3));
    assert(daemon.deletion_list_.sequence_bits()==8 && daemon.channels_[1].leading_words_[3]==0);
    // A nonaliased allocated packet checks seed/action payload reloads and the
    // host word's post-action capture without tying timestamp to host storage.
    reset(daemon);alignas(4) unsigned char ordinary[32]{};
    put(ordinary,0,0x4440);
    put(ordinary,4,7);put(ordinary,8,55);put(ordinary,0xc,3);
    put(ordinary,0x10,1);put(ordinary,0x14,2);put(ordinary,0x18,3);
    daemon.deletion_list_.set_sequence_bits(12);
    callback=[&](const std::string&s) {
        if(s=="seed") {put(ordinary,0x10,4);put(ordinary,0x14,5);put(ordinary,0x18,6);}
        if(s=="cross") {daemon.primary_state_.leading_words_[0]=1;put(ordinary,4,3);put(ordinary,0xc,8);}
    };
    assert(netdaemon_handle_five_message_routes(daemon,0x4440,ordinary,7,3));
    assert(seed==55 && cross_args[0]==4 && cross_args[1]==5 && cross_args[2]==6);
    assert(daemon.deletion_list_.sequence_bits()==12 && daemon.channels_[1].leading_words_[3]==0);
    // The local completion's pending-lock decision precedes its logging;
    // clear occurs after log and before send, preserving its shared tail.
    reset(daemon);packet=owner_packet(daemon,3,7);daemon.channels_[1].trailing_words_[2]=4;
    callback=[&](const std::string&s) {
        if(s=="log:Remaining Locked2")daemon.channels_[1].trailing_words_[2]=9;
        if(s=="send")assert(daemon.channels_[1].trailing_words_[2]==0);
    };
    assert(netdaemon_handle_five_message_routes(daemon,0x4440,packet,3,7));
    assert(daemon.channels_[1].trailing_words_[2]==0 && daemon.channels_[1].leading_words_[3]==9);
    assert(events[events.size()-2]=="log:Remaining Locked2" && events.back()=="send");
    return 0;
}
'''


def run(tmp_path,compiler,declarations,body):
    cpp=tmp_path/"cross.cpp";exe=tmp_path/"cross"
    cpp.write_text(HARNESS.replace("/* ACTUAL DECLARATIONS */",declarations).replace("/* ACTUAL HELPERS */",body))
    built=subprocess.run([compiler,"-std=c++11","-fsanitize=undefined","-fno-sanitize-recover=all",str(cpp),"-o",str(exe)],capture_output=True,text=True,timeout=60)
    assert built.returncode==0,built.stdout+built.stderr
    return subprocess.run([str(exe)],capture_output=True,text=True,timeout=20)


@pytest.mark.parametrize("compiler",["g++","clang++"])
def test_actual_receiver_alias_and_post_action_reads(tmp_path,compiler):
    compiler=shutil.which(compiler)
    if not compiler:pytest.skip("host compiler unavailable")
    result=run(tmp_path,compiler,*material());assert result.returncode==0,result.stdout+result.stderr


@pytest.mark.parametrize("damage",["post_store_faction","entry_faction","entry_sequence","pre_action_host","pre_seed_payload"])
def test_double_cross_damage_controls(tmp_path,damage):
    compiler=shutil.which("g++")
    if not compiler:pytest.skip("host compiler unavailable")
    declarations,body=material()
    changes={
        "post_store_faction":("if (received_faction == ConsoleGlobal.faction_id_)","if (packet.faction() == ConsoleGlobal.faction_id_)"),
        "entry_faction":("const int received_faction = packet.faction();","const int received_faction = entry_packet_faction;"),
        "entry_sequence":("const uint32_t received_sequence = packet.sequence();","const uint32_t received_sequence = entry_sequence;"),
        "pre_action_host":("        const uint32_t host = daemon.primary_state_.leading_words_[0];", ""),
        "pre_seed_payload":("double_cross(packet.payload_10(),\n                     packet.payload_14(),\n                     packet.payload_18());","double_cross(faction, partner, victim);"),
    }
    before,after=changes[damage];assert before in body;body=body.replace(before,after,1)
    if damage=="pre_action_host":
        body=body.replace("    case 0x4440: {","    case 0x4440: {\n        const uint32_t host = daemon.primary_state_.leading_words_[0];",1)
    result=run(tmp_path,compiler,declarations,body);assert result.returncode!=0,"failed to detect "+damage


def test_pinned_double_cross_capture_store_and_common_tail():
    from capstone import Cs,CS_ARCH_X86,CS_MODE_32
    from match_recovery.model import Span
    from match_recovery.pe_coff import PeImage
    path=Path(os.environ.get("OPENSMACX_IMAGE","/home/fran6co/code/OpenSMACX/.opensmacx/game/terranx_original.exe"))
    image=PeImage(path.read_bytes(),PIN);decoder=Cs(CS_ARCH_X86,CS_MODE_32)
    decoded=list(decoder.disasm(image.read(Span(0x536D0F,0x536DA5)),0x536D0F))
    assert decoded[-1].address+decoded[-1].size==0x536DA5
    ins={i.address:(i.mnemonic,i.op_str) for i in decoded}
    expected={
        0x536D20:("call","0x6262f0"),0x536D29:("call","0x538fb0"),
        0x536D2E:("mov","edx, dword ptr [ebx + 0x18]"),
        0x536D31:("mov","eax, dword ptr [ebx + 0x14]"),
        0x536D34:("mov","ecx, dword ptr [ebx + 0x10]"),
        0x536D3A:("call","0x50bcc0"),
        0x536D3F:("mov","edx, dword ptr [esi + 0x1b30]"),
        0x536D45:("mov","eax, dword ptr [ebx + 0xc]"),
        0x536D48:("mov","ecx, dword ptr [ebx + 4]"),
        0x536D50:("jne","0x536d58"),
        0x536D52:("mov","dword ptr [esi + 0x1b2c], eax"),
        0x536D58:("cmp","ecx, dword ptr [0x939284]"),
        0x536D5E:("jne","0x538266"),
        0x536D64:("mov","eax, dword ptr [esi + 0x1bc0]"),
        0x536D6E:("je","0x536d9a"),0x536D78:("call","0x6262f0"),
        0x536D87:("mov","dword ptr [esi + 0x1bc0], edi"),
        0x536D8D:("call","0x592ee0"),
        0x536D9A:("mov","dword ptr [esi + 0x1bb0], edi"),
    }
    for address,value in expected.items():assert ins[address]==value,hex(address)
    assert 0x158C+0x5A0==0x1B2C
