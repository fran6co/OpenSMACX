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


# These typed callback controls qualify the actual polling/source operation,
# not the DirectPlay provider, the x86 ABI, or full-game protocol acceptance.
@pytest.mark.parametrize("damage", ["none", "wrong_transport", "swapped_outputs",
                                    "dropped_message", "single_poll",
                                    "wrong_opcode", "wrong_dispatch_owner", "cached_gates"])
def test_actual_receive_reuse_and_live_send_phases(tmp_path, damage):
    source_text = SOURCE.read_text()
    body = "\n".join(extract(source_text, signature) for signature in
                     ("int NetDaemon::receive()", "void NetDaemon::send_game()",
                      "void NetDaemon::send_rules()"))
    edits = {
        "wrong_transport": ("NetDaemonNet.Net::get(&first, &second)", "Net::get(&first, &second)"),
        "swapped_outputs": ("NetDaemonNet.Net::get(&first, &second)", "NetDaemonNet.Net::get(&second, &first)"),
        "dropped_message": ("    process_message(static_cast<char *>(result), second, static_cast<int>(first));", ""),
        "single_poll": ("while (receive())", "if (receive())"),
        "wrong_opcode": ("synch(0x80,", "synch(0x81,"),
        "wrong_dispatch_owner": ("    process_message(static_cast<char *>(result), second, static_cast<int>(first));",
                                 "    NetDaemonNet.process_message(static_cast<char *>(result), second, static_cast<int>(first));"),
    }
    if damage in edits:
        old, new = edits[damage]
        assert body.count(old)==(30 if damage=="single_poll" else 1)
        body = body.replace(old,new)
    elif damage=="cached_gates":
        old="void NetDaemon::send_game() {"
        assert body.count(old)==1
        body=body.replace(old,old+"\n const int cached_turn=GameDataStateStorage.fields_.turn_current_num_; const int cached_replay=ReplayEventBytes;",1)
        body=body.replace("if (GameDataStateStorage.fields_.turn_current_num_ != 0)","if (cached_turn != 0)")
        body=body.replace("if (ReplayEventBytes != 0)","if (cached_replay != 0)")
    program = r'''
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <vector>
#define CHECK(x) do { if (!(x)) { std::fprintf(stderr,"polling: %s\n",#x); std::exit(9); } } while(0)
struct Net { void *get(unsigned long *,unsigned long *); };
struct NetDaemon:Net {
    int receive(); void send_game(); void send_rules();
    void process_message(char *,unsigned long,int);
    void synch(int16_t,int,int,int,char *,int,int16_t);
};
NetDaemon NetDaemonNet;
static NetDaemon *owner;
struct { struct { int turn_current_num_; } fields_; } GameDataStateStorage;
static int ReplayEventBytes;
struct Phase { int code, a, b; Phase(int c,int x=0,int y=0):code(c),a(x),b(y) {} };
static std::vector<Phase> expected;
static unsigned phase;
static int queue_count, received, dispatched, total_received, total_dispatched;
static bool ended, pending, late_gates;
static char packets[3][8];
void NetDaemon::synch(int16_t code,int a,int b,int c,char *text,int d,int16_t flags) {
    CHECK(this==owner && ended && !pending && phase<expected.size());
    CHECK(code==expected[phase].code && a==expected[phase].a && b==expected[phase].b);
    CHECK(c==0 && text==0 && d==1 && flags==0x2101);
    ++phase; received=dispatched=0; ended=false;
}
void *Net::get(unsigned long *sender,unsigned long *length) {
    CHECK(this==static_cast<Net*>(&NetDaemonNet) && phase>0 && !ended && !pending);
    if(received==queue_count) {
        ended=true;
        // The original gates are sampled after preceding network callbacks.
        if(expected[phase-1].code==0x19) GameDataStateStorage.fields_.turn_current_num_=late_gates?1:0;
        if(expected[phase-1].code==0x21) ReplayEventBytes=late_gates?1:0;
        return 0; // No output words are published on this path.
    }
    *sender=0x10203040ul+phase*0x101ul+received;
    *length=37ul+received;
    pending=true; ++total_received;
    return packets[received++];
}
void NetDaemon::process_message(char *message,unsigned long length,int sender) {
    CHECK(this==owner && pending && dispatched<queue_count);
    CHECK(message==packets[dispatched] && length==37ul+dispatched);
    CHECK(sender==static_cast<int>(0x10203040ul+phase*0x101ul+dispatched));
    pending=false; ++dispatched; ++total_dispatched;
}
BODY
static void begin(int count,bool late) {
    queue_count=count; late_gates=late; phase=0; ended=true; pending=false;
    total_received=total_dispatched=0;
    GameDataStateStorage.fields_.turn_current_num_=late?0:1;
    ReplayEventBytes=late?0:1; expected.clear();
}
static void done() {
    CHECK(phase==expected.size() && ended && !pending);
    CHECK(total_received==static_cast<int>(expected.size())*queue_count);
    CHECK(total_dispatched==total_received);
}
int main() {
    NetDaemon daemon; owner=&daemon;
    for(int count=0;count<=3;++count) {
        if(count==2) continue;
        for(int late=0;late<=1;++late) {
            begin(count,late!=0);
            expected.push_back(Phase(0)); expected.push_back(Phase(2));
            expected.push_back(Phase(4)); expected.push_back(Phase(5));
            for(int i=0;i<8;++i) {
                expected.push_back(Phase(6,i)); expected.push_back(Phase(7,i));
                expected.push_back(Phase(15,i*64,64));
            }
            expected.push_back(Phase(0x29)); expected.push_back(Phase(0x12));
            expected.push_back(Phase(0x14)); expected.push_back(Phase(0x19));
            if(late) expected.push_back(Phase(0x20));
            expected.push_back(Phase(0x21));
            if(late) expected.push_back(Phase(0x22));
            expected.push_back(Phase(0x24)); expected.push_back(Phase(0x15));
            daemon.send_game(); done();
            begin(count,late!=0);
            for(int code=0x80;code<=0x8D;++code) expected.push_back(Phase(code));
            daemon.send_rules(); done();
        }
    }
}
'''.replace("BODY",body)
    compiler=shutil.which("g++") or shutil.which("clang++")
    assert compiler, "host C++ compiler is required for the polling control"
    cpp,exe=tmp_path/"polling.cpp",tmp_path/"polling"
    cpp.write_text(program)
    build=subprocess.run([compiler,"-std=c++11","-O2","-DNDEBUG",str(cpp),"-o",str(exe)],
                         capture_output=True,text=True,timeout=60)
    assert build.returncode==0,build.stdout+build.stderr
    run=subprocess.run([str(exe)],capture_output=True,text=True,timeout=10)
    assert run.returncode==(0 if damage=="none" else 9),run.stdout+run.stderr
    if damage!="none": assert "polling:" in run.stderr
