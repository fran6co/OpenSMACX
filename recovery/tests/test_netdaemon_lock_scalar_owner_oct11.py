"""Actual lock owner: scalar states, typed receive outputs, signed retry bits."""
from pathlib import Path
import shutil
import subprocess

import pytest

from test_caviar_draw_callbacks_actual_source import extract

ROOT = Path(__file__).resolve().parents[2]
SOURCE = ROOT / "src/netdaemon.cpp"
BASE = "731d01aae"


def apply_lock_scalar_migration(source):
    old = extract(source, "int NetDaemon::lock(")
    body = old
    counts = {
        "channels_[1].leading_words_[2]": 3,
        "channels_[1].trailing_words_[3]": 2,
        "channels_[0].leading_words_[2]": 7,
        "locks_.field_E8_": 3,
        "channels_[1].trailing_words_[4]": 5,
    }
    for field, count in counts.items():
        before = "*(int *)(" + field + ")"
        assert body.count(before) == count
        body = body.replace(before, field)
    edits = (
        ("            void *got = NetDaemonNet.Net::get((unsigned long *)&y, (unsigned long *)&x);",
         "            unsigned long sender, length;\n            void *got = NetDaemonNet.Net::get(&sender, &length);"),
        ("this->process_message(static_cast<char *>(got), (unsigned long)x, y);",
         "this->process_message(static_cast<char *>(got), length, static_cast<int>(sender));"),
        ("                    void *got2 = NetDaemonNet.Net::get((unsigned long *)&y, (unsigned long *)&x);",
         "                    unsigned long sender, length;\n                    void *got2 = NetDaemonNet.Net::get(&sender, &length);"),
        ("this->process_message(static_cast<char *>(got2), (unsigned long)x, y);",
         "this->process_message(static_cast<char *>(got2), length, static_cast<int>(sender));"),
        ("        channels_[1].trailing_words_[4] = channels_[1].trailing_words_[4] + 1;\n        if (channels_[1].trailing_words_[4] > 0 || channels_[0].leading_words_[2] > 2) {",
         "        ++channels_[1].trailing_words_[4];\n        // Native INC wraps, while its JG/JLE comparisons interpret signed bits.\n        int32_t retries, response;\n        std::memcpy(&retries, &channels_[1].trailing_words_[4], sizeof retries);\n        std::memcpy(&response, &channels_[0].leading_words_[2], sizeof response);\n        if (retries > 0 || response > 2) {"),
    )
    for before, after in edits:
        assert body.count(before) == 1, before
        body = body.replace(before, after, 1)
    assert source.count(old) == 1
    return source.replace(old, body, 1)


PREAMBLE = r'''
#include <cassert>
#include <cstdint>
#include <cstring>
#include <string>
#include <vector>
static int IsMultiplayerNet, NetPlayerDropped, NetBusyCount, ExitTurnLoop;
static int sends, receives, waits, spocks, dispatches, closes, checks;
static bool asynchronous, drop_on_send;
static uint32_t reply, clock_ticks;
static std::vector<std::string> popups;
struct Net { void *get(unsigned long *sender, unsigned long *length); };
struct Words { uint32_t leading_words_[4], trailing_words_[10]; };
struct Locks { uint32_t field_E8_; void check_global() { ++checks; } };
struct NetDaemon : Net {
    Words channels_[2]; Locks locks_;
    int lock(int,int,int,int,int,int);
    void process_message(char *,unsigned long,int);
};
NetDaemon NetDaemonNet;
static NetDaemon *owner;
struct { int faction_id_; } ConsoleGlobal;
struct Popup {
    void pop(const char *name,int,int,const char *) { popups.push_back(name); }
    void close() { ++closes; }
} NetMsgPrimary;
static unsigned long timeGetTime() { clock_ticks+=2000; return clock_ticks; }
static void wait_task() { ++waits; NetDaemonNet.channels_[0].leading_words_[3]=999; }
static void check_spock() { ++spocks; }
static void log_say(const char *,int,int,int) {}
static void message_lock(int type,int,int x,int y,int a,int b,int c,int d) {
    assert(type==0x2210 && x==10 && y==11 && a==12 && b==13 && c==14 && d==0);
    ++sends; NetPlayerDropped=drop_on_send;
    assert(owner->channels_[1].leading_words_[2]==1);
    assert(owner->channels_[0].leading_words_[2]==0 && owner->locks_.field_E8_==0);
    if (!asynchronous) owner->channels_[0].leading_words_[2]=reply;
}
void *Net::get(unsigned long *sender,unsigned long *length) {
    assert(this==static_cast<Net*>(&NetDaemonNet)); ++receives;
    *sender=10001; *length=24; static char packet[24]; return packet;
}
void NetDaemon::process_message(char *,unsigned long length,int sender) {
    assert(this==owner && sender==10001 && length==24); ++dispatches;
    if (channels_[0].leading_words_[2]==0) channels_[0].leading_words_[2]=reply;
    else locks_.field_E8_=1;
}
'''

MAIN = r'''
static void reset(NetDaemon &daemon) {
    std::memset(&daemon,0,sizeof daemon); std::memset(&NetDaemonNet,0,sizeof NetDaemonNet);
    owner=&daemon; IsMultiplayerNet=1; NetPlayerDropped=0; NetBusyCount=5; ExitTurnLoop=0;
    sends=receives=waits=spocks=dispatches=closes=checks=0; popups.clear(); clock_ticks=0;
    asynchronous=drop_on_send=false; reply=1;
    ConsoleGlobal.faction_id_=2; NetDaemonNet.channels_[0].leading_words_[3]=17;
}
static int run(NetDaemon &daemon,int flags=0) { return daemon.lock(flags,10,11,12,13,14); }
int main() {
    NetDaemon daemon;
    reset(daemon); IsMultiplayerNet=0; NetPlayerDropped=9;
    assert(run(daemon)==0 && NetPlayerDropped==9 && sends==0);
    reset(daemon); daemon.channels_[1].leading_words_[2]=1;
    daemon.channels_[1].trailing_words_[3]=1;
    assert(run(daemon)==1 && sends==0 && popups.empty());
    daemon.channels_[1].trailing_words_[3]=0;
    assert(run(daemon)==1 && popups.back()=="MOVEPENDING");
    reset(daemon); assert(run(daemon)==0);
    assert(daemon.channels_[1].leading_words_[2]==1); // Original success retains pending state.
    assert(NetBusyCount==5 && receives==0 && NetDaemonNet.channels_[0].leading_words_[3]==17);
    reset(daemon); asynchronous=true; assert(run(daemon)==0);
    assert(receives==1 && waits==1 && checks==1 && spocks==1 && closes==1);
    assert(popups.size()==1 && popups[0]=="AWAITLOCK");
    assert(NetBusyCount==5 && NetDaemonNet.channels_[0].leading_words_[3]==17);
    reset(daemon); asynchronous=true; assert(run(daemon,2)==0);
    assert(dispatches==2 && daemon.locks_.field_E8_==1 && closes==2);
    assert(popups.size()==2 && popups[1]=="AWAITGLOBALLOCK");
    assert(NetBusyCount==5 && NetDaemonNet.channels_[0].leading_words_[3]==17);
    reset(daemon); reply=2; assert(run(daemon)==1);
    assert(popups.back()=="NOLOCK" && daemon.channels_[1].leading_words_[2]==0);
    reset(daemon); reply=3; assert(run(daemon)==1 && popups.back()=="NOLOCKGLOBAL");
    reset(daemon); drop_on_send=true; assert(run(daemon)==1);
    assert(daemon.channels_[0].leading_words_[2]==2 && popups.back()=="NOLOCK");
    reset(daemon); reply=0; ExitTurnLoop=1; assert(run(daemon)==1);
    assert(receives==0 && waits==0 && NetBusyCount==5);
    assert(popups.size()==1 && popups[0]=="NOLOCKGLOBAL");
    assert(daemon.channels_[1].leading_words_[2]==0 && daemon.channels_[1].trailing_words_[4]==0);
    reset(daemon); reply=2; daemon.channels_[1].trailing_words_[4]=0xFFFFFFFFu;
    assert(run(daemon)==1 && popups.empty() && daemon.channels_[1].trailing_words_[4]==0);
    reset(daemon); reply=2; daemon.channels_[1].trailing_words_[4]=0x7FFFFFFFu;
    assert(run(daemon)==1 && popups.empty() && daemon.channels_[1].trailing_words_[4]==0x80000000u);
    reset(daemon); reply=0xFFFFFFFFu; daemon.channels_[1].trailing_words_[4]=0xFFFFFFFEu;
    assert(run(daemon)==1 && popups.empty() && daemon.channels_[1].trailing_words_[4]==0xFFFFFFFFu);
}
'''


@pytest.mark.parametrize("damage", ["none", "pointer", "unsigned_counter", "unsigned_response", "receive_order", "clear_success"])
def test_actual_lock_state_and_wait_paths(tmp_path, damage):
    compiler = shutil.which("g++") or shutil.which("clang++")
    if not compiler:
        pytest.skip("host C++ compiler unavailable")
    body = extract(SOURCE.read_text(), "int NetDaemon::lock(")
    edits = {
        "pointer": ("if (channels_[1].leading_words_[2] != 0)", "if (*reinterpret_cast<int *>(static_cast<uintptr_t>(channels_[1].leading_words_[2])) != 0)"),
        "unsigned_counter": ("if (retries > 0 || response > 2)", "if (channels_[1].trailing_words_[4] > 0 || response > 2)"),
        "unsigned_response": ("if (retries > 0 || response > 2)", "if (retries > 0 || channels_[0].leading_words_[2] > 2)"),
        "receive_order": ("NetDaemonNet.Net::get(&sender, &length)", "NetDaemonNet.Net::get(&length, &sender)"),
        "clear_success": ("if (NetPlayerDropped == 0) {\n            return 0;", "if (NetPlayerDropped == 0) {\n            channels_[1].leading_words_[2]=0;\n            return 0;"),
    }
    if damage != "none":
        before, after = edits[damage]
        assert body.count(before) == (2 if damage == "receive_order" else 1)
        body = body.replace(before, after)
    source = tmp_path / "lock.cpp"
    binary = tmp_path / "lock"
    source.write_text(PREAMBLE + body + MAIN)
    subprocess.run([compiler,"-std=c++11",str(source),"-o",str(binary)],
                   check=True,capture_output=True,timeout=60)
    result = subprocess.run([str(binary)],capture_output=True,timeout=10)
    assert (result.returncode == 0) == (damage == "none"), result.stderr


def test_fixed_scalar_migration_preserves_all_other_source():
    old = subprocess.check_output(["git","show",BASE+":src/netdaemon.cpp"],cwd=ROOT,text=True)
    assert apply_lock_scalar_migration(old) == SOURCE.read_text()


def test_pinned_lock_uses_direct_scalar_members_and_signed_tests():
    from capstone import Cs, CS_ARCH_X86, CS_MODE_32
    from match_recovery.pe_coff import PeImage
    from match_recovery.model import Span
    path = ROOT / ".opensmacx/game/terranx_original.exe"
    if not path.is_file():
        pytest.skip("pinned original unavailable")
    image = PeImage(path.read_bytes(),"01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf")
    instructions = {i.address: (i.mnemonic,i.op_str) for i in
        Cs(CS_ARCH_X86,CS_MODE_32).disasm(image.read(Span(0x530630,0x53091E)),0x530630)}
    expected = {
        0x530650:("mov","ecx, dword ptr [esi + 0x1bac]"),
        0x53065A:("cmp","dword ptr [esi + 0x1bc4], eax"),
        0x53068B:("mov","dword ptr [esi + 0x1bac], 1"),
        0x530695:("mov","dword ptr [esi + 0x1b70], eax"),
        0x53069B:("mov","dword ptr [esi + 0x1588], eax"),
        0x5307B3:("mov","dword ptr [esi + 0x1bc8], 0"),
        0x5308BC:("mov","edx, dword ptr [esi + 0x1bc8]"),
        0x5308C2:("inc","edx"),
        0x5308CD:("jg","0x5308d7"),
        0x5308D5:("jle","0x530908"),
        0x530908:("mov","dword ptr [esi + 0x1bac], 0"),
    }
    for address,instruction in expected.items():
        assert instructions[address] == instruction
