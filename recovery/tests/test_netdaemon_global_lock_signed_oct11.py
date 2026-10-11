"""Original global-lock retry arithmetic and signed status branches."""
from pathlib import Path
import shutil
import subprocess

import pytest

from test_caviar_draw_callbacks_actual_source import extract

ROOT = Path(__file__).resolve().parents[2]
SOURCE = ROOT / "src/netdaemon.cpp"
BASE = "0ca820280"
BEFORE = """        int newBc8 = channels_[1].trailing_words_[4] + 1;
        channels_[1].trailing_words_[4] = newBc8;
        if (newBc8 > 0 || channels_[0].leading_words_[2] > 2) {"""
AFTER = """        ++channels_[1].trailing_words_[4];
        // Native INC wraps, while its JG/JLE comparisons interpret signed bits.
        int32_t retries, response;
        std::memcpy(&retries, &channels_[1].trailing_words_[4], sizeof retries);
        std::memcpy(&response, &channels_[0].leading_words_[2], sizeof response);
        if (retries > 0 || response > 2) {"""


def apply_global_lock_signed_migration(source):
    old = extract(source, "int NetDaemon::add_global(")
    assert old.count(BEFORE) == 1
    assert source.count(old) == 1
    return source.replace(old, old.replace(BEFORE, AFTER, 1), 1)


PREAMBLE = r'''
#include <cassert>
#include <cstdint>
#include <cstring>
#include <string>
#include <vector>
#define WINAPI
typedef unsigned long DWORD;
static int IsMultiplayerNet, NetPlayerDropped, NetBusyCount, ExitTurnLoop;
static unsigned long ticks;
static uint32_t reply;
static bool asynchronous, dropped;
static int receives, waits, checks, dispatches, closes;
static std::vector<std::string> popups;
struct Net { void *get(unsigned long *,unsigned long *); };
struct Words { uint32_t leading_words_[4], trailing_words_[10]; };
struct Locks { uint32_t field_E8_; void check_global() { ++checks; } };
struct NetDaemon : Net {
    Words channels_[2]; Locks locks_;
    int add_global();
    void process_message(char *,unsigned long,int);
};
static NetDaemon NetDaemonNet;
static NetDaemon *owner;
struct Popup {
    void pop(const char *name,int,int,int) { popups.push_back(name); }
    void close() { ++closes; }
} NetMsgPrimary;
static unsigned long timeGetTime() { ticks+=2000; return ticks; }
static void wait_task() { ++waits; assert(NetBusyCount==6); }
static void check_spock() {}
static void message_data(int type,int a,int b,int c,int d,int e) {
    assert(type==0x2211 && a==0 && b==0 && c==0 && d==0 && e==0);
    assert(owner->channels_[0].leading_words_[2]==0 && owner->locks_.field_E8_==0);
    NetPlayerDropped=dropped;
    if (!asynchronous) { owner->channels_[0].leading_words_[2]=reply; owner->locks_.field_E8_=1; }
}
void *Net::get(unsigned long *sender,unsigned long *length) {
    assert(this==static_cast<Net*>(&NetDaemonNet)); ++receives;
    *sender=10001; *length=24; static char packet[24]; return packet;
}
void NetDaemon::process_message(char *,unsigned long length,int sender) {
    assert(this==owner && length==24 && sender==10001); ++dispatches;
    if (channels_[0].leading_words_[2]==0) channels_[0].leading_words_[2]=reply;
    else locks_.field_E8_=1;
}
'''

MAIN = r'''
static void reset(NetDaemon &daemon) {
    std::memset(&daemon,0,sizeof daemon); std::memset(&NetDaemonNet,0,sizeof NetDaemonNet);
    owner=&daemon; IsMultiplayerNet=1; NetPlayerDropped=0; NetBusyCount=5; ExitTurnLoop=0;
    ticks=0; reply=1; asynchronous=dropped=false;
    receives=waits=checks=dispatches=closes=0; popups.clear();
    NetDaemonNet.channels_[0].leading_words_[3]=17;
}
int main() {
    NetDaemon daemon;
    reset(daemon); IsMultiplayerNet=0; NetPlayerDropped=9;
    assert(daemon.add_global()==0 && NetPlayerDropped==9 && NetBusyCount==5);
    reset(daemon); assert(daemon.add_global()==0 && NetBusyCount==5);
    assert(NetDaemonNet.channels_[0].leading_words_[3]==17 && receives==0 && popups.empty());
    reset(daemon); asynchronous=true;
    assert(daemon.add_global()==0 && waits==2 && receives==2 && checks==2 && dispatches==2);
    assert(NetBusyCount==5 && closes==2 && popups.size()==2);
    assert(popups[0]=="AWAITGLOBALLOCK" && popups[1]=="AWAITGLOBALLOCK");
    reset(daemon); dropped=true;
    assert(daemon.add_global()==1 && NetBusyCount==5 && popups.back()=="NOLOCK");
    for (uint32_t status=2;status<=4;++status) {
        reset(daemon); reply=status;
        assert(daemon.add_global()==1 && NetBusyCount==5);
        assert(popups.back()==(status==2 ? "NOLOCK" : "NOLOCKGLOBAL"));
    }
    reset(daemon); asynchronous=true; ExitTurnLoop=1;
    assert(daemon.add_global()==1 && waits==0 && NetBusyCount==5 && popups.back()=="NOLOCKGLOBAL");
    reset(daemon); reply=2; daemon.channels_[1].trailing_words_[4]=0xFFFFFFFFu;
    assert(daemon.add_global()==1 && popups.empty() && daemon.channels_[1].trailing_words_[4]==0);
    reset(daemon); reply=2; daemon.channels_[1].trailing_words_[4]=0x7FFFFFFFu;
    assert(daemon.add_global()==1 && popups.empty() && daemon.channels_[1].trailing_words_[4]==0x80000000u);
    // Synthetic bit-pattern controls qualify the native signed predicate;
    // observed original response producers write only 0..4, not these values.
    reset(daemon); reply=0xFFFFFFFFu; daemon.channels_[1].trailing_words_[4]=0xFFFFFFFEu;
    assert(daemon.add_global()==1 && popups.empty() && daemon.channels_[1].trailing_words_[4]==0xFFFFFFFFu);
}
'''


@pytest.mark.parametrize("damage", ["none", "unsigned_retry", "unsigned_response", "receive_order"])
def test_actual_global_lock_paths_and_signed_bit_patterns(tmp_path, damage):
    compiler = shutil.which("g++") or shutil.which("clang++")
    if not compiler:
        pytest.skip("host C++ compiler unavailable")
    body = extract(SOURCE.read_text(), "int NetDaemon::add_global(")
    edits = {
        "unsigned_retry": ("if (retries > 0 || response > 2)", "if (channels_[1].trailing_words_[4] > 0 || response > 2)"),
        "unsigned_response": ("if (retries > 0 || response > 2)", "if (retries > 0 || channels_[0].leading_words_[2] > 2)"),
        "receive_order": ("NetDaemonNet.Net::get(&out_id, &out_len)", "NetDaemonNet.Net::get(&out_len, &out_id)"),
    }
    if damage != "none":
        before, after = edits[damage]
        assert body.count(before) == 1
        body = body.replace(before, after, 1)
    source = tmp_path / "global_lock.cpp"
    binary = tmp_path / "global_lock"
    source.write_text(PREAMBLE + body + MAIN)
    subprocess.run([compiler,"-std=c++11",str(source),"-o",str(binary)],check=True,capture_output=True,timeout=60)
    result = subprocess.run([str(binary)],capture_output=True,timeout=10)
    assert (result.returncode == 0) == (damage == "none"), result.stderr


def test_fixed_global_lock_recipe_preserves_other_source():
    old = subprocess.check_output(["git","show",BASE+":src/netdaemon.cpp"],cwd=ROOT,text=True)
    assert apply_global_lock_signed_migration(old) == SOURCE.read_text()
    with pytest.raises(AssertionError):
        apply_global_lock_signed_migration(old.replace("int newBc8 =", "unsigned int newBc8 ="))


def test_pinned_global_lock_increment_and_signed_branches():
    from capstone import Cs, CS_ARCH_X86, CS_MODE_32
    from match_recovery.pe_coff import PeImage
    from match_recovery.model import Span
    path = ROOT / ".opensmacx/game/terranx_original.exe"
    if not path.is_file():
        pytest.skip("pinned original unavailable")
    image = PeImage(path.read_bytes(),"01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf")
    instructions = {i.address:(i.mnemonic,i.op_str) for i in
        Cs(CS_ARCH_X86,CS_MODE_32).disasm(image.read(Span(0x530AA0,0x530CFF)),0x530AA0)}
    for address,instruction in {
        0x530CAC:("inc","edx"),
        0x530CAF:("mov","dword ptr [esi + 0x1bc8], edx"),
        0x530CB7:("jg","0x530cc1"),
        0x530CB9:("cmp","dword ptr [esi + 0x1b70], ecx"),
        0x530CBF:("jle","0x530ced"),
    }.items():
        assert instructions[address] == instruction
