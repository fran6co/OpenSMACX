"""Typed receive outputs preserve wait's message and channel DWORD semantics."""
from pathlib import Path
import shutil
import subprocess

import pytest

from test_caviar_draw_callbacks_actual_source import extract

ROOT = Path(__file__).resolve().parents[2]
SOURCE = ROOT / "src/netdaemon.cpp"
BASE = "68b669853"
EDITS = {
    "int NetDaemon::wait(": (
        ("        unsigned long outVar;\n        void *result = NetDaemonNet.Net::get(reinterpret_cast<unsigned long *>(&message), &outVar);",
         "        unsigned long sender, length;\n        void *result = NetDaemonNet.Net::get(&sender, &length);"),
        ("process_message(static_cast<char *>(result), outVar, reinterpret_cast<int>(message));",
         "process_message(static_cast<char *>(result), length, static_cast<int>(sender));"),
    ),
    "int NetDaemon::open_channel(": (
        ('reinterpret_cast<char *>("Client requesting Diplo Channel")', '"Client requesting Diplo Channel"'),
        ('reinterpret_cast<const char *>("AWAITCHANNEL")', '"AWAITCHANNEL"'),
        ("  payload_14 = static_cast<int>(timeGetTime_());", "  const DWORD start_time = timeGetTime_();"),
        ("        int elapsed = static_cast<int>(timeGetTime_()) - payload_14;", "        const DWORD elapsed = timeGetTime_() - start_time;"),
        ("      unsigned long local4;\n      void *gotten = NetDaemonNet.Net::get(reinterpret_cast<unsigned long *>(&payload_10), &local4);",
         "      unsigned long sender, length;\n      void *gotten = NetDaemonNet.Net::get(&sender, &length);"),
        ("process_message(static_cast<char *>(gotten), local4, payload_10);",
         "process_message(static_cast<char *>(gotten), length, static_cast<int>(sender));"),
        ("  if (channels_[1].leading_words_[0] < 2) {",
         "  int32_t response;\n  std::memcpy(&response, &channels_[1].leading_words_[0], sizeof response);\n  if (response < 2) {"),
    ),
    "int NetDaemon::add_lock(": (
        ('reinterpret_cast<char *>("Client requesting ADD LOCK")', '"Client requesting ADD LOCK"'),
        ('reinterpret_cast<const char *>("AWAITLOCK")', '"AWAITLOCK"'),
        ('reinterpret_cast<const char *>("NOLOCK")', '"NOLOCK"'),
        ("            void *msgResult = NetDaemonNet.Net::get(reinterpret_cast<unsigned long *>(&x),\n                                                reinterpret_cast<unsigned long *>(&target));",
         "            unsigned long sender, length;\n            void *msgResult = NetDaemonNet.Net::get(&sender, &length);"),
        ("                                 static_cast<unsigned long>(target), x);",
         "                                 length, static_cast<int>(sender));"),
    ),
}


def apply_receive_output_migration(source):
    for signature, edits in EDITS.items():
        old = extract(source, signature)
        body = old
        for before, after in edits:
            assert body.count(before) == 1, before
            body = body.replace(before, after, 1)
        assert source.count(old) == 1
        source = source.replace(old, body, 1)
    from test_netdaemon_transfer_exits_oct11 import apply_transfer_exit_migration
    return apply_transfer_exit_migration(source)


PREAMBLE = r'''
#include <cassert>
#include <cstdint>
#include <cstring>
#include <string>
#include <vector>
#define WINAPI
typedef uint32_t DWORD;
static int NetBusyCount, ExitTurnLoop, IsMultiplayerNet;
static int receives, dispatches, closes, waits, checks, mode;
static uint32_t response_bits, wait_state;
static bool immediate;
static std::vector<DWORD> clock_values;
static unsigned clock_index;
static std::vector<std::string> popups;
static char wait_name[]="WAITORIGINAL";
struct Net { void *get(unsigned long *,unsigned long *); };
struct Words { uint32_t leading_words_[4], trailing_words_[10]; };
struct Lock { void check_global() { ++checks; } };
struct NetDaemon : Net {
    Words channels_[2]; Lock locks_;
    int wait(char *,uint32_t *,int,int);
    int open_channel(int,int);
    int add_lock(int,int,int);
    void process_message(char *,unsigned long,int);
};
static NetDaemon NetDaemonNet;
static NetDaemon *owner;
struct { int faction_id_; } ConsoleGlobal;
struct NetMsg {
    void pop(const char *name,int,int,int) {
        if (mode==0) assert(name==wait_name); // Never dereference a corrupted pointer.
        popups.push_back(name);
    }
    void close() { ++closes; }
} NetMsgPrimary;
static DWORD timeGetTime() { assert(clock_index<clock_values.size()); return clock_values[clock_index++]; }
static void wait_task() { ++waits; assert(NetBusyCount==6); }
static void check_spock() {}
static void log_say(const char *,int,int,int) {}
static void message_data(int type,int a,int b,int c,int d,int e) {
    assert(type==0x8240 && a==0 && b==10 && c==11 && d==0 && e==0);
    assert(owner->channels_[0].trailing_words_[9]==1 && owner->channels_[1].leading_words_[0]==0);
    if (immediate) owner->channels_[1].leading_words_[0]=response_bits;
}
static void message_lock(int type,int target,int x,int y,int a,int b,int c,int d) {
    assert(type==0x2215 && target==9 && x==10 && y==11 && a==0 && b==-1 && c==-1 && d==0);
    assert(owner->channels_[0].leading_words_[2]==0);
    if (immediate) owner->channels_[0].leading_words_[2]=response_bits;
}
void *Net::get(unsigned long *sender,unsigned long *length) {
    assert(this==static_cast<Net*>(&NetDaemonNet)); ++receives;
    *sender=10001; *length=24; static char packet[24]; return packet;
}
void NetDaemon::process_message(char *,unsigned long length,int sender) {
    assert(this==owner && sender==10001 && length==24); ++dispatches;
    if (mode==0) { if (dispatches==2) wait_state=0; }
    else if (mode==1) channels_[1].leading_words_[0]=response_bits;
    else channels_[0].leading_words_[2]=response_bits;
}
'''

MAIN = r'''
static void reset(NetDaemon &daemon,int kind) {
    std::memset(&daemon,0,sizeof daemon); std::memset(&NetDaemonNet,0,sizeof NetDaemonNet);
    owner=&daemon; mode=kind; NetBusyCount=5; IsMultiplayerNet=1; ExitTurnLoop=0;
    receives=dispatches=closes=waits=checks=0; clock_index=0; popups.clear();
    clock_values={0,100,2001}; response_bits=1; immediate=false; wait_state=1;
    NetDaemonNet.channels_[0].leading_words_[3]=17;
}
int main() {
    NetDaemon daemon;
    reset(daemon,0);
    assert(daemon.wait(wait_name,&wait_state,1,1600)==0);
    assert(receives==2 && waits==2 && dispatches==2 && checks==2 && closes==1);
    assert(popups.size()==1 && popups[0]=="WAITORIGINAL" && NetBusyCount==5);
    assert(NetDaemonNet.channels_[0].leading_words_[3]==17);
    reset(daemon,0); wait_state=0;
    assert(daemon.wait(wait_name,&wait_state,1,1600)==0 && receives==0 && NetBusyCount==5);
    reset(daemon,0); ExitTurnLoop=1;
    assert(daemon.wait(wait_name,&wait_state,1,1600)==0 && receives==0 && popups.empty());
    reset(daemon,1); IsMultiplayerNet=0;
    assert(daemon.open_channel(10,11)==0 && receives==0);
    reset(daemon,1); daemon.channels_[0].trailing_words_[9]=1;
    assert(daemon.open_channel(10,11)==1 && receives==0);
    reset(daemon,1); assert(daemon.open_channel(10,11)==0 && receives==1 && NetBusyCount==5);
    assert(daemon.channels_[0].trailing_words_[9]==1);
    reset(daemon,1); response_bits=2;
    assert(daemon.open_channel(10,11)==1 && daemon.channels_[0].trailing_words_[9]==0);
    reset(daemon,1); clock_values={0,0x80000000u};
    assert(daemon.open_channel(10,11)==0 && popups.size()==1 && popups[0]=="AWAITCHANNEL" && closes==1);
    reset(daemon,1); clock_values={0xFFFFF000u,0};
    assert(daemon.open_channel(10,11)==0 && popups.size()==1);
    // Signed branch qualification only: known producer states are small.
    reset(daemon,1); response_bits=0xFFFFFFFFu; immediate=true;
    assert(daemon.open_channel(10,11)==0 && daemon.channels_[0].trailing_words_[9]==1);
    reset(daemon,2); assert(daemon.add_lock(9,10,11)==1 && receives==0); // No pending request.
    reset(daemon,2); daemon.channels_[1].leading_words_[2]=1;
    assert(daemon.add_lock(9,10,11)==0 && receives==1 && NetBusyCount==5);
    assert(NetDaemonNet.channels_[0].leading_words_[3]==17);
    reset(daemon,2); daemon.channels_[1].leading_words_[2]=1; response_bits=2;
    assert(daemon.add_lock(9,10,11)==1 && popups.back()=="NOLOCK");
}
'''


@pytest.mark.parametrize("damage", ["none", "overwrite_message", "signed_elapsed", "unsigned_response", "receive_order"])
def test_actual_receive_owners_and_original_wait_semantics(tmp_path, damage):
    compiler = shutil.which("g++") or shutil.which("clang++")
    if not compiler:
        pytest.skip("host C++ compiler unavailable")
    body = "\n".join(extract(SOURCE.read_text(), signature) for signature in EDITS)
    edits = {
        "overwrite_message": ("process_message(static_cast<char *>(result), length, static_cast<int>(sender));",
                              "message=reinterpret_cast<char *>(sender); process_message(static_cast<char *>(result), length, static_cast<int>(sender));"),
        "signed_elapsed": ("const DWORD elapsed = timeGetTime_() - start_time;", "const int32_t elapsed = static_cast<int32_t>(timeGetTime_() - start_time);"),
        "unsigned_response": ("if (response < 2)", "if (channels_[1].leading_words_[0] < 2)"),
        "receive_order": ("NetDaemonNet.Net::get(&sender, &length)", "NetDaemonNet.Net::get(&length, &sender)"),
    }
    if damage != "none":
        before, after = edits[damage]
        assert body.count(before) == (3 if damage=="receive_order" else 1)
        body = body.replace(before, after)
    source = tmp_path / "receive.cpp"
    binary = tmp_path / "receive"
    source.write_text(PREAMBLE + body + MAIN)
    subprocess.run([compiler,"-std=c++11",str(source),"-o",str(binary)],check=True,capture_output=True,timeout=60)
    result = subprocess.run([str(binary)],capture_output=True,timeout=10)
    assert (result.returncode==0) == (damage=="none"), result.stderr


def test_fixed_receive_recipe_preserves_other_source():
    old = subprocess.check_output(["git","show",BASE+":src/netdaemon.cpp"],cwd=ROOT,text=True)
    assert apply_receive_output_migration(old) == SOURCE.read_text()
    with pytest.raises(AssertionError):
        apply_receive_output_migration(old.replace("reinterpret_cast<int>(message)", "0"))


def test_original_wait_retains_message_and_channel_branches_are_unsigned_then_signed():
    from capstone import Cs, CS_ARCH_X86, CS_MODE_32
    from match_recovery.pe_coff import PeImage
    from match_recovery.model import Span
    path = ROOT / ".opensmacx/game/terranx_original.exe"
    if not path.is_file():
        pytest.skip("pinned original unavailable")
    image = PeImage(path.read_bytes(),"01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf")
    instructions = {i.address:(i.mnemonic,i.op_str) for i in
        Cs(CS_ARCH_X86,CS_MODE_32).disasm(image.read(Span(0x5303B0,0x530600)),0x5303B0)}
    expected = {
        0x5303E9:("mov","ebx, dword ptr [ebp + 8]"),
        0x530411:("push","ebx"),
        0x530429:("lea","ecx, [ebp + 8]"),
        0x530433:("call","0x630a00"),
        0x53043C:("mov","edx, dword ptr [ebp + 8]"),
        0x530549:("cmp","eax, 0x640"),
        0x53054E:("jbe","0x53056a"),
        0x53057C:("call","0x630a00"),
        0x5305D8:("cmp","eax, 2"),
        0x5305DC:("jl","0x5305f5"),
    }
    for address,instruction in expected.items():
        assert instructions[address]==instruction
