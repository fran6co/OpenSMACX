"""Native word geometry, typed transfer storage and shared dispatch tail."""
from pathlib import Path
import shutil
import subprocess

import pytest

from test_netdaemon_transfer_modal import extract

ROOT=Path(__file__).resolve().parents[2]
BASE="8deec1275"
FIXTURE=Path(__file__).with_name("netdaemon-typed-storage-oct11.patch")


def hunks(relative):
    section=next(part for part in FIXTURE.read_text().split("diff --git ")[1:]
                 if part.startswith("a/"+relative+" b/"+relative+"\n"))
    result=[]
    for hunk in section.split("\n@@ ")[1:]:
        lines=hunk.splitlines(keepends=True)[1:]
        result.append(("".join(line[1:] for line in lines if line.startswith((" ","-"))),
                       "".join(line[1:] for line in lines if line.startswith((" ","+")))))
    assert result
    return result


def apply_typed_storage_migration(source,relative="src/netdaemon.cpp",reverse=False):
    if relative == "src/netdaemon.cpp" and reverse:
        from test_netdaemon_session_status_oct11 import apply_session_status_migration
        source = apply_session_status_migration(source, reverse=True)
    edits=hunks(relative)
    for before,after in (reversed(edits) if reverse else edits):
        if reverse: before,after=after,before
        assert before and source.count(before)==1,before
        source=source.replace(before,after,1)
    if relative == "src/netdaemon.cpp" and not reverse:
        from test_netdaemon_session_status_oct11 import apply_session_status_migration
        source = apply_session_status_migration(source)
    return source


@pytest.mark.parametrize("relative",["src/netdaemon.cpp","src/netdaemon.h"])
def test_only_reviewed_storage_changes_and_all_original_identities_survive(relative):
    before=subprocess.check_output(["git","show",BASE+":"+relative],cwd=ROOT,text=True)
    current=(ROOT/relative).read_text()
    assert current==apply_typed_storage_migration(before,relative)
    assert apply_typed_storage_migration(current,relative,True)==before
    assert "NetDaemonFileIndexRecord *file_index_records_;" in (ROOT/"src/netdaemon.h").read_text()
    from dataclasses import fields
    from match_recovery.source import read_text
    old={record.address:record for record in read_text(before,ROOT/relative)}
    now={record.address:record for record in read_text(current,ROOT/relative)}
    assert old.keys()==now.keys()
    for address,record in now.items():
        for field in fields(record):
            if field.name not in {"line","region","origin"}:
                assert getattr(record,field.name)==getattr(old[address],field.name)


PREAMBLE=r'''
#include <cassert>
#include <stdint.h>
#include <cstring>
#include <cstddef>
@@LOCK_STATE@@
struct Primary { uint32_t leading_words_[3]; };
struct Sequence { uint32_t value; void set_sequence_bits(uint32_t v) { value=v; } };
struct NetDaemon { Primary primary_state_; LockStateEntry channels_[2]; Sequence deletion_list_; };
static struct { int faction_id_; } ConsoleGlobal;
static struct { int8_t order; } Vehs[8];
static int broadcasts,logs;
static void log_say(const char *,int,int,int) { ++logs; }
static void log_say(const char *,char *,int,int,int) { ++logs; }
static void my_srand(int) {}
static void action_terraform(int,int,int) {}
static void action_destroy(int,int,int,int) {}
static void action_destruct(int) {}
static void kill(int) {}
static void action_give(int,int) {}
static void action_build(int,char *) {}
static void action_gate(int,int) {}
static void do_upgrade(int,int,int) {}
static void action_sat_attack(int,int,int,int) {}
static void action_staple(int) {}
static void retire_proto(int,int) {}
static void action_tectonic(int,int,int) {}
static void action_fungal(int,int,int) {}
static NetDaemon *owner;
static void message_data(int type,int,int,int,int,int) {
    if (type==0x2214) assert(owner->channels_[1].trailing_words_[2]==0);
    ++broadcasts;
}
'''

MAIN=r'''
int main() {
    assert(sizeof(LockStateEntry)==0x3C && offsetof(LockStateEntry,trailing_words_)==0x14);
    NetDaemon daemon; owner=&daemon; ConsoleGlobal.faction_id_=3;
    uint32_t packet[16]={0}; unsigned char *msg=reinterpret_cast<unsigned char *>(packet);
    packet[1]=3; packet[3]=0xA123; packet[4]=1; packet[5]=2;
    const unsigned short types[]={0x4405,0x4406,0x4407,0x4408,0x4409,0x4410,0x4411,0x4412,0x4413,0x4414,0x4415,0x4416,0x4417,0x4418};
    for(unsigned n=0;n<sizeof(types)/sizeof(*types);++n) {
        for(int locked=0;locked<2;++locked) for(int high=0;high<2;++high) {
            std::memset(&daemon,0,sizeof daemon); broadcasts=0;
            const uint32_t pending=(types[n]&~0x4000u)|(high?0x80000000u:0u);
            daemon.channels_[1].leading_words_[1]=pending;
            daemon.channels_[1].leading_words_[3]=99;
            daemon.channels_[1].trailing_words_[2]=locked;
            std::memcpy(msg,&types[n],2);
            assert(netdaemon_process_table_g(daemon,msg,types[n]));
            assert(daemon.channels_[1].leading_words_[1]==(high?pending:0));
            if(types[n]==0x4411) {
                assert(daemon.channels_[1].leading_words_[3]==99 && daemon.channels_[1].trailing_words_[2]==uint32_t(locked));
                assert(daemon.deletion_list_.value==0 && broadcasts==0);
            } else {
                assert(daemon.deletion_list_.value==packet[3]);
                assert(daemon.channels_[1].trailing_words_[2]==0);
                assert(daemon.channels_[1].leading_words_[3]==uint32_t(locked?99:0));
                assert(broadcasts==locked);
            }
        }
    }
    for(int identity=0;identity<8;++identity) {
        std::memset(&daemon,0,sizeof daemon); broadcasts=0;
        daemon.channels_[0].trailing_words_[0]=0x80000000u;
        assert(netdaemon_handle_lock_mask_residual(daemon,0x8240,msg,identity));
        assert(daemon.channels_[0].trailing_words_[0]==0x80000006u);
        assert(daemon.channels_[0].trailing_words_[1+identity]==6u);
        assert(netdaemon_handle_lock_mask_residual(daemon,0x8241,msg,identity));
        assert(daemon.channels_[0].trailing_words_[0]==0x80000000u);
        assert(daemon.channels_[0].trailing_words_[1+identity]==0u);
        assert(broadcasts==1);
        // Busy mask does not overwrite an existing peer assignment.
        daemon.channels_[0].trailing_words_[0]=6;
        daemon.channels_[0].trailing_words_[1+identity]=0x10;
        assert(netdaemon_handle_lock_mask_residual(daemon,0x8240,msg,identity));
        assert(daemon.channels_[0].trailing_words_[0]==6 && daemon.channels_[0].trailing_words_[1+identity]==0x10);
    }
}
'''


@pytest.mark.parametrize("damage",["none","wrong_local_word","wrong_peer_word","truncate_pending"])
def test_actual_dispatch_uses_real_channel_members_and_full_pending_width(tmp_path,damage):
    compiler=shutil.which("g++") or shutil.which("clang++")
    if compiler is None: pytest.skip("host C++ compiler unavailable")
    source=(ROOT/"src/netdaemon.cpp").read_text()
    body="\n".join(extract(source,s) for s in (
        "static void netdaemon_process_message_marker_tail(",
        "static int netdaemon_table_g_word(","static bool netdaemon_process_table_g(",
        "static bool netdaemon_handle_lock_mask_residual("))
    changes={
        "wrong_local_word":("channels_[1].trailing_words_[2]","channels_[1].trailing_words_[1]"),
        "wrong_peer_word":("channels_[0].trailing_words_[1 + identity]","channels_[0].trailing_words_[identity]"),
        "truncate_pending":("(pending | 0x4000u)","static_cast<unsigned short>(pending | 0x4000u)"),
    }
    if damage!="none":
        before,after=changes[damage]; assert before in body; body=body.replace(before,after)
    declaration=extract((ROOT/"src/netdaemon.h").read_text(),"struct LockStateEntry {")+";"
    # Host-only word-width model: no pointer is consumed here. Native header
    # assertions plus a fresh32-bit application build qualify the actual ABI.
    declaration=declaration.replace("int *vehicle_selection_follow;","uint32_t vehicle_selection_follow;")
    cpp=tmp_path/"storage.cpp"; exe=tmp_path/"storage"
    cpp.write_text(PREAMBLE.replace("@@LOCK_STATE@@",declaration)+body+MAIN)
    subprocess.run([compiler,"-std=c++03","-fsanitize=undefined,bounds","-fno-sanitize-recover=all",str(cpp),"-o",str(exe)],check=True,capture_output=True,timeout=60)
    result=subprocess.run([str(exe)],capture_output=True,timeout=10)
    assert (result.returncode==0)==(damage=="none"),result.stderr


def test_no_remaining_literal_channel_prefix_overruns():
    import re
    source=(ROOT/"src/netdaemon.cpp").read_text()
    assert not re.search(r"channels_\[[^]]+\]\.leading_words_\[(?:[4-9]|\d\d+|6 \+ identity)\]",source)


def test_pinned_native_channel_offsets_and_pending_width():
    from capstone import Cs,CS_ARCH_X86,CS_MODE_32
    from match_recovery.pe_coff import PeImage
    from match_recovery.model import Span
    path=ROOT/".opensmacx/game/terranx_original.exe"
    if not path.is_file(): pytest.skip("pinned original unavailable")
    image=PeImage(path.read_bytes(),"01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf")
    bands=[(0x536D5E,0x536DA4),(0x537CE9,0x537D30),(0x537F35,0x537F5F),(0x538266,0x53829A)]
    instructions={i.address:(i.mnemonic,i.op_str) for start,end in bands
                  for i in Cs(CS_ARCH_X86,CS_MODE_32).disasm(image.read(Span(start,end)),start)}
    expected={
        0x536D64:("mov","eax, dword ptr [esi + 0x1bc0]"),
        0x536D87:("mov","dword ptr [esi + 0x1bc0], edi"),
        0x536D9A:("mov","dword ptr [esi + 0x1bb0], edi"),
        0x537CFD:("mov","ecx, dword ptr [esi + 0x1b7c]"),
        0x537D13:("mov","dword ptr [esi + edi*4 + 0x1b80], edx"),
        0x537D22:("mov","dword ptr [esi + 0x1b7c], eax"),
        0x537F35:("mov","edx, dword ptr [esi + edi*4 + 0x1b80]"),
        0x537F46:("mov","dword ptr [esi + 0x1b7c], eax"),
        0x538280:("cmp","eax, ecx"),
    }
    for address,instruction in expected.items(): assert instructions[address]==instruction
