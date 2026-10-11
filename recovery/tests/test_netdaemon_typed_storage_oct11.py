"""Native word geometry, typed transfer storage and shared dispatch tail."""
from pathlib import Path
import shutil
import subprocess
import os

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
static NetDaemon NetDaemonNet;
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
static void action_oblit(int,int) {}
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
            assert(netdaemon_process_table_g(daemon,msg,types[n],packet[1],packet[3]));
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


G_CALLBACK_CONTROL = r'''
#include <cassert>
#include <stdint.h>
#include <cstring>
@@LOCK_STATE@@
struct Primary { uint32_t leading_words_[3]; };
struct Sequence { uint32_t value; void set_sequence_bits(uint32_t v) { value=v; } };
struct NetDaemon { Primary primary_state_; LockStateEntry channels_[2]; Sequence deletion_list_; void route(char *); };
static struct { int faction_id_; } ConsoleGlobal;
// Logical source-member double only; the native table/control pins below
// qualify dispatch, not this fixture's vehicle ABI or action implementation.
static struct { int8_t order; } Vehs[8];
static uint32_t packet[16];
static int opcode, callback_mode, called, args[4], seeds, broadcasts, ack_faction;
static void mutate(int stage) {
    if (!callback_mode) return;
    if (stage==1) { packet[4]=3; packet[5]=7; packet[6]=6; }
    if (stage==2) { packet[4]=4; packet[5]=8; packet[6]=7; }
    if (stage==3) { packet[1]=8; packet[3]=0xDEAD; packet[4]=5; packet[5]=9; packet[6]=0x5A; }
}
static void log_say(const char *text,int a,int,int) {
    if (!std::strncmp(text,"From Server:",12)) mutate(1);
    else if (!std::strncmp(text,"Acknowledging",13)) ack_faction=a;
}
static void log_say(const char *text,char *,int,int,int) { log_say(text,0,0,0); }
static void my_srand(int) { ++seeds; mutate(2); }
static void call(int kind,int a,int b=0,int c=0,int d=0) {
    ++called; args[0]=a;args[1]=b;args[2]=c;args[3]=d;
    assert(kind==opcode);
    mutate(3);
}
static void action_terraform(int a,int b,int c) { call(0x4405,a,b,c); }
static void action_destroy(int a,int b,int c,int d) { call(b==0?0x4413:0x4406,a,b,c,d); }
static void action_destruct(int a) { call(0x4407,a,1); }
static void kill(int a) { call(0x4407,a,0); }
static void action_give(int a,int b) { call(0x4416,a,b); }
static void action_build(int a,char *name) { assert(name==reinterpret_cast<char *>(packet)+0x14); call(0x4408,a); }
static void action_oblit(int a,int b) { call(0x4409,a,b); }
static void action_gate(int a,int b) { call(0x4410,a,b); }
static void do_upgrade(int a,int b,int c) { call(0x4411,a,b,c); }
static void action_sat_attack(int a,int b,int c,int d) { call(0x4412,a,b,c,d); }
static void action_staple(int a) { call(0x4414,a); }
static void retire_proto(int a,int b) { call(0x4415,a,b); }
static void action_tectonic(int a,int b,int c) { call(0x4417,a,b,c); }
static void action_fungal(int a,int b,int c) { call(0x4418,a,b,c); }
static NetDaemon *owner;
static void message_data(int type,int,int,int,int,int) {
    assert(type==0x2214 && owner->channels_[1].trailing_words_[2]==0); ++broadcasts;
}
@@ACTUAL_SOURCE@@
int main() {
    const unsigned short types[]={0x4405,0x4406,0x4407,0x4408,0x4409,0x4410,0x4411,0x4412,0x4413,0x4414,0x4415,0x4416,0x4417,0x4418};
    for(unsigned n=0;n<sizeof(types)/sizeof(*types);++n)
    for(int mutate_packet=0;mutate_packet<2;++mutate_packet)
    for(int host=0;host<2;++host)
    for(int local=0;local<2;++local)
    for(int locked=0;locked<2;++locked) {
        NetDaemon daemon; std::memset(&daemon,0,sizeof daemon); owner=&daemon;
        ConsoleGlobal.faction_id_=3; daemon.primary_state_.leading_words_[0]=host;
        daemon.deletion_list_.value=0xBEEF;
        daemon.channels_[1].leading_words_[3]=99; daemon.channels_[1].trailing_words_[2]=locked;
        daemon.channels_[1].leading_words_[1]=types[n]&~0x4000u;
        std::memset(packet,0,sizeof packet); packet[0]=types[n]; packet[1]=local?3:6;
        packet[3]=0xA123; packet[4]=1; packet[5]=6;packet[6]=2;packet[7]=10;
        const int entry_faction=packet[1]; const uint32_t entry_sequence=packet[3];
        for(int i=0;i<8;++i) Vehs[i].order=0x33;
        opcode=types[n];callback_mode=mutate_packet;called=seeds=broadcasts=0;ack_faction=-1;
        daemon.route(reinterpret_cast<char *>(packet));
        assert(called==1 && seeds==(opcode==0x4410?0:1));
        const int selected=(opcode==0x4409 || opcode==0x4410 || opcode>=0x4417)?1:(mutate_packet?4:1);
        assert(args[0]==selected);
        if(opcode==0x4405) {
            assert(args[1]==(mutate_packet?4:2) && args[2]==1);
            for(int i=0;i<8;++i) assert(Vehs[i].order==(i==selected?(mutate_packet?0x5A:2):0x33));
        } else for(int i=0;i<8;++i) assert(Vehs[i].order==0x33);
        if(opcode==0x4406) assert(args[1]==(mutate_packet?8:6) && args[2]==-1 && args[3]==-1);
        if(opcode==0x4409 || opcode==0x4415 || opcode==0x4416) assert(args[1]==(mutate_packet?8:6));
        if(opcode==0x4410) assert(args[1]==(mutate_packet?7:6));
        if(opcode==0x4411 || opcode==0x4412) assert(args[1]==(mutate_packet?8:6) && args[2]==(mutate_packet?7:2));
        if(opcode==0x4412) assert(args[3]==10);
        if(opcode==0x4413) assert(args[1]==0 && args[2]==(mutate_packet?8:6) && args[3]==(mutate_packet?7:2));
        if(opcode>=0x4417) assert(args[1]==6 && args[2]==2 && ack_faction==entry_faction);
        if(opcode==0x4411) {
            assert(daemon.deletion_list_.value==0xBEEF && broadcasts==0);
            assert(daemon.channels_[1].leading_words_[3]==99 && daemon.channels_[1].trailing_words_[2]==uint32_t(locked));
        } else {
            assert(daemon.deletion_list_.value==(host?0xBEEF:entry_sequence));
            assert(broadcasts==(local?locked:0));
            assert(daemon.channels_[1].trailing_words_[2]==uint32_t(local?0:locked));
            assert(daemon.channels_[1].leading_words_[3]==uint32_t(local&&!locked?0:99));
        }
        assert(daemon.channels_[1].leading_words_[1]==uint32_t(local&&!mutate_packet?0:(types[n]&~0x4000u)));
    }
    NetDaemon daemon; std::memset(&daemon,0,sizeof daemon);
    for(unsigned short type=0x440A;type<=0x440F;++type)
        assert(!netdaemon_process_table_g(daemon,0,type,3,0xA123));
    // Native kill/destruct does not scatter a new order after the action.
    packet[0]=0x4407;packet[1]=6;packet[3]=1;packet[4]=1;packet[5]=0;
    opcode=0x4407;callback_mode=0;called=0;
    daemon.route(reinterpret_cast<char *>(packet));
    assert(called==1 && args[1]==0);
    opcode=0x4405;packet[0]=opcode;packet[5]=0x80000000u;packet[6]=2;
    daemon.route(reinterpret_cast<char *>(packet));
    assert(args[1]==0x7FFFFFFC);
}
'''


@pytest.mark.parametrize("compiler", [c for c in ("g++", "clang++") if shutil.which(c)])
def test_actual_table_g_native_routes_and_callback_captures(tmp_path, compiler):
    source=(ROOT/"src/netdaemon.cpp").read_text()
    body="\n".join(extract(source,s) for s in (
        "static void netdaemon_process_message_marker_tail(",
        "static int netdaemon_table_g_word(", "static bool netdaemon_process_table_g("))
    process=extract(source,"void NetDaemon::process_message(")
    capture=process[process.index("    unsigned char *msg ="):process.index("\n\n    // Masked file-transfer")]
    start=process.index("    if (netdaemon_process_table_g(")
    stop=process.index("\n\n    if (netdaemon_handle_table_h",start)
    # Execute the actual caller capture and call expressions. Other dispatcher
    # branches are deliberately outside this bounded Table G control.
    body+="\nvoid NetDaemon::route(char *message) {\n"+capture+"\n"+process[start:stop]+"\nassert(false);\n}"
    declaration=extract((ROOT/"src/netdaemon.h").read_text(),"struct LockStateEntry {")+";"
    declaration=declaration.replace("int *vehicle_selection_follow;","uint32_t vehicle_selection_follow;")
    def run(candidate):
        cpp=tmp_path/"g.cpp";exe=tmp_path/"g"
        cpp.write_text(G_CALLBACK_CONTROL.replace("@@LOCK_STATE@@",declaration).replace("@@ACTUAL_SOURCE@@",candidate))
        build=subprocess.run([compiler,"-std=c++03","-Wall","-Wextra","-Werror","-Wno-unused-function","-fsanitize=address,undefined",
                              str(cpp),"-o",str(exe)],capture_output=True,text=True,timeout=60)
        assert build.returncode==0,build.stderr
        return subprocess.run([str(exe)],capture_output=True,text=True,timeout=10,
                              env=dict(os.environ,ASAN_OPTIONS="detect_leaks=0"))
    result=run(body);assert result.returncode==0,result.stderr
    for before,after in (
        ("action_oblit(vehicle_id,", "action_give(vehicle_id,"),
        ("case 0x4408:", "case 0x4408 + 1:"),
        ("set_sequence_bits(entry_sequence)","set_sequence_bits(netdaemon_table_g_word(msg, 0x0c) + 0u * entry_sequence)"),
        ("entry_packet_faction == ConsoleGlobal.faction_id_","netdaemon_table_g_word(msg, 4) == ConsoleGlobal.faction_id_"),
        ("Vehs[vehicle_id].order", "Vehs[netdaemon_table_g_word(msg, 0x10)].order"),
        ("action_tectonic(vehicle_id, x, y)","action_tectonic(netdaemon_table_g_word(msg, 0x10) + 0 * vehicle_id, x, y)"),
        ("action_fungal(vehicle_id, x, y)","action_fungal(vehicle_id, netdaemon_table_g_word(msg, 0x14) + 0 * x, y)"),
        ("Acknowledging TECTONIC\", entry_packet_faction", "Acknowledging TECTONIC\", 0"),
        ("action_gate(vehicle_id,", "action_gate(netdaemon_table_g_word(msg, 0x10),"),
        ("entry_packet_faction, entry_sequence))", "entry_sequence, entry_packet_faction))"),
    ):
        # Keep every damage control compiling; routing changes must fail at
        # runtime, not through duplicate/missing switch labels or APIs.
        if before=="case 0x4408:":
            mutant=body.replace("case 0x4408:","case 0x440A:",1)
        else:
            assert before in body;mutant=body.replace(before,after)
        damaged=run(mutant);assert damaged.returncode!=0,before
    missing_store=body.replace("        tail = TailDispatch;\n        break;\n    case 0x4416:",
        "        Vehs[netdaemon_table_g_word(msg, 0x10)].order = *reinterpret_cast<int8_t *>(msg + 0x18);\n"
        "        tail = TailDispatch;\n        break;\n    case 0x4416:",1)
    assert missing_store!=body
    assert run(missing_store).returncode!=0


def test_pinned_complete_table_g_targets_captures_and_shared_default():
    from hashlib import sha256
    from struct import unpack
    from capstone import Cs,CS_ARCH_X86,CS_MODE_32
    from match_recovery.model import Span
    from match_recovery.pe_coff import PeImage
    path=Path(os.environ.get("OPENSMACX_IMAGE",ROOT/".opensmacx/game/terranx_original.exe"))
    if not path.is_file(): pytest.skip("pinned original unavailable")
    raw=path.read_bytes();pin="01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf"
    assert sha256(raw).hexdigest()==pin
    image=PeImage(raw,pin)
    targets=unpack("<20I",image.read(Span(0x53895C,0x5389AC)))
    assert targets==(0x5369F2,0x536A51,0x536AC3,0x536B40,0x536B73,
                     0x537F7A,0x537F7A,0x537F7A,0x537F7A,0x537F7A,0x537F7A,
                     0x536BB4,0x536C09,0x536C75,0x536A89,0x536CD3,
                     0x536C41,0x536B0C,0x53695F,0x5369BD)
    assert unpack("<I",image.read(Span(0x5388BC+4*4,0x5388BC+5*4)))==(0x537F7A,) # F09
    ins={i.address:(i.mnemonic,i.op_str.lower())
         for start,end in ((0x536916,0x536DA5),(0x537F7A,0x537FCB),(0x538266,0x53829A))
         for i in Cs(CS_ARCH_X86,CS_MODE_32).disasm(image.read(Span(start,end)),start)}
    pins={
        0x53694A:("add","eax, 0xffffbbfb"),0x53694F:("cmp","eax, 0x13"),
        0x536952:("ja","0x537f7a"),0x536958:("jmp","dword ptr [eax*4 + 0x53895c]"),
        0x536968:("mov","edi, dword ptr [ebx + 0x10]"),
        0x53696C:("mov","dword ptr [ebp + 0xc], ecx"),
        0x53696F:("mov","dword ptr [ebp + 0x10], edx"),0x536972:("call","0x538fb0"),
        0x536980:("call","0x4ce210"),0x536985:("mov","edi, dword ptr [ebp + 8]"),
        0x5369A8:("mov","eax, dword ptr [ebp - 0x10]"),
        0x5369C6:("mov","edi, dword ptr [ebx + 0x10]"),0x5369D0:("call","0x538fb0"),
        0x5369DE:("call","0x4ce790"),
        0x536A14:("mov","edi, dword ptr [ebx + 0x10]"),0x536A17:("sub","edx, 4"),
        0x536A1E:("call","0x4c9b00"),0x536A23:("mov","dl, byte ptr [ebx + 0x18]"),
        0x536A2F:("mov","byte ptr [ecx*4 + 0x952839], dl"),
        0x536A40:("mov","eax, dword ptr [ebp - 0x10]"),0x536A49:("mov","ecx, dword ptr [ebp + 8]"),
        0x536AEE:("call","0x4cd2f0"),0x536AF6:("jmp","0x536a36"),
        0x536AFF:("call","0x5c0b00"),0x536B07:("jmp","0x536a36"),
        0x536B33:("call","0x4cede0"),0x536B48:("lea","edi, [ebx + 0x14]"),
        0x536B61:("mov","ecx, dword ptr [ebx + 0x10]"),0x536B66:("call","0x4c96e0"),
        0x536B76:("mov","edi, dword ptr [ebx + 0x10]"),0x536B8D:("call","0x538fb0"),
        0x536B92:("mov","ecx, dword ptr [ebx + 0x14]"),0x536B97:("call","0x4cd4e0"),
        0x536BB7:("mov","edi, dword ptr [ebx + 0x10]"),0x536BCF:("call","0x4cf380"),
        0x536BF1:("mov","ecx, dword ptr [ebp - 0x10]"),0x536BFA:("mov","edx, dword ptr [ebp + 8]"),
        0x536C3C:("jmp","0x538266"),0x536920:("mov","edx, dword ptr [ebp - 0x10]"),
        0x536929:("mov","eax, dword ptr [ebp + 8]"),
        0x537F7A:("test","ch, 0xa0"),0x537F83:("call","dword ptr [0x669368]"),
        0x537F8C:("mov","dword ptr [ebx + 8], eax"),0x537F8F:("call","0x534330"),
        0x537F9C:("mov","cx, word ptr [ebx]"),0x537FA2:("and","ecx, 0x1fff"),
        0x537FAA:("or","ch, 0x40"),0x537FB7:("inc","edi"),
        0x537FC3:("mov","dword ptr [ebx + 0xc], eax"),0x537FC6:("call","0x532940"),
    }
    for address,expected in pins.items(): assert ins[address]==expected,hex(address)
    clock=next(item for item in image.imports() if item.address==0x669368)
    assert clock.name=="timeGetTime" and clock.module.lower()=="winmm.dll"
    # No action runtime, unbounded packet access or vehicle-allocation proof.


H_CALLBACK_CONTROL = r'''
#include <cassert>
#include <stdint.h>
#include <cstring>
#include <cstddef>
typedef int BOOL;
@@FACTION_DECLARATIONS@@
@@LOCK_STATE@@
static Player Players[MaxPlayerNum];
static PlayerData PlayersData[MaxPlayerNum];
struct NetDaemon { LockStateEntry channels_[2]; void route(char *); };
static struct { int faction_id_; } ConsoleGlobal;
static struct { struct { uint32_t game_state_; } fields_; } GameDataStateStorage;
static int GenderDefault,PluralityDefault;
static char StringTemp[1032];
static char script[]="SCRIPT",changed_script[]="CALLBACK_SCRIPT";
static char *ScriptPopupFile=script;
static uint32_t packet[16];
static char *message;
static int opcode,mode,actions,args[4],seeds,parses,pops,year_input,tech_input;
static int spy_calls,spy_result[2],hood_actor,hood_other;
static uint16_t post_tech_opcode;
static int post_recipient;
static const char *popup_key;
static int read_word(int offset) { int value;std::memcpy(&value,message+offset,4);return value; }
static void write_word(int offset,uint32_t value) { std::memcpy(message+offset,&value,4); }
static void set_opcode(uint16_t value) { std::memcpy(message,&value,2); }
static void mutate(int stage) {
    if(mode==0) return;
    if(stage==1) { write_word(0x10,4);write_word(0x14,5);write_word(0x18,6);write_word(0x1C,7); }
    if(stage==2) { write_word(0x10,2);write_word(0x14,3);write_word(0x18,4);write_word(0x1C,5); }
    if(stage==3) {
        write_word(0x10,3);write_word(0x14,4);write_word(0x18,5);write_word(0x1C,6);
        if(mode==2) { set_opcode(post_tech_opcode);write_word(0x18,post_recipient); }
    }
}
static void logger(bool hex,int a,int b,int c) {
    assert(hex==(opcode>=0x4441 && opcode<=0x4446));
    assert(a==read_word(0x10) && b==read_word(0x14));
    assert(c==((opcode==0x4448 || opcode==0x4449 || opcode==0x444A || opcode==0x444D)?0:read_word(0x18)));
    mutate(1);
}
static void log_say(const char *,int a,int b,int c) { logger(false,a,b,c); }
static void log_say_hex(const char *,int a,int b,int c) { logger(true,a,b,c); }
static void my_srand(int value) { assert(value==read_word(8));++seeds;mutate(2); }
static void action(int kind,int a,int b,int c=0,int d=0) {
    assert(kind==opcode || (kind==0x444B && opcode==0x444C));
    ++actions;args[0]=a;args[1]=b;args[2]=c;args[3]=d;mutate(3);
}
static void treaty_on(int a,int b,int c) { action(0x4441,a,b,c); }
static void treaty_off(int a,int b,int c) { action(0x4442,a,b,c); }
static void set_treaty(int a,int b,int c,int d) { action(0x4443,a,b,c,d); }
static void agenda_on(int a,int b,int c) { action(0x4444,a,b,c); }
static void agenda_off(int a,int b,int c) { action(0x4445,a,b,c); }
static void set_agenda(int a,int b,int c,int d) { action(0x4446,a,b,c,d); }
static void trade_maps(int a,int b) { action(0x4448,a,b); }
static void give_a_base(int a,int b) { action(0x4449,a,b); }
static void do_withdrawal(int a,int b) { action(0x444A,a,b); }
static void tech_achieved(int a,int b,int c,int d) { assert(d==0);action(0x444B,a,b,c,d); }
static void pact_ends(int a,int b) { action(0x444D,a,b); }
static int spying(int a) {
    assert(a==read_word(spy_calls==0?0x10:0x14));
    const int value=spy_result[spy_calls++];
    if(mode==3) { write_word(0x10,4);write_word(0x14,5); }
    return value;
}
static bool has_treaty(int a,int b,int mask) { return (PlayersData[a].diplo_treaties[b]&mask)!=0; }
static char hood[]="hood";
static char *get_pact_hood(int a,int b) { hood_actor=a;hood_other=b;assert(a==read_word(0x10)&&b==read_word(0x14));return hood; }
static void parse_says(int index,const char *text,int a,int b) {
    assert(a==-1 && b==-1);++parses;
    const int actor=read_word(0x10),other=read_word(0x14);
    if(opcode==0x4441) {
        if(index<2) {
            const int row=index?other:actor;
            assert(text==Players[row].noun_faction && GenderDefault==Players[row].noun_gender && PluralityDefault==Players[row].is_noun_plural);
        } else assert(text==hood);
    } else if(opcode==0x444F) {
        if(index<2) assert(text==(index?Players[actor].name_leader:Players[actor].title_leader) &&
            GenderDefault==Players[actor].is_leader_female && PluralityDefault==0);
        if(index==2) assert(text==Players[actor].noun_faction && GenderDefault==Players[actor].noun_gender && PluralityDefault==Players[actor].is_noun_plural);
        if(index==3) assert(text==Players[actor].adj_name_faction && GenderDefault==(mode?55:Players[actor].noun_gender) &&
            PluralityDefault==(mode?66:Players[actor].is_noun_plural));
    } else {
        if(index==0) assert(text==Players[actor].noun_faction && GenderDefault==Players[actor].noun_gender && PluralityDefault==Players[actor].is_noun_plural);
        if(index==1) assert(text==StringTemp && StringTemp[0]=='T');
    }
    if(mode) {
        write_word(0x10,actor%7+1);write_word(0x14,other%7+1);
        if(opcode==0x444F) write_word(0x14,other+100);
        if(opcode==0x4441) write_word(0x18,4);
        GenderDefault=55;PluralityDefault=66;
    }
}
static void say_tech(char *text,int tech,int flag) { assert(text==StringTemp && text[0]==0 && tech==read_word(0x14) && flag==0);tech_input=tech;strcpy(text,"Tech"); }
static int game_year(int value) { assert(value==read_word(0x14));year_input=value;return value+2100; }
static void parse_num(int index,int value) { assert(index==0 && value==year_input+2100);ScriptPopupFile=changed_script; }
static void popp(char *file,const char *key,int a,const char *icon,int b) {
    assert(file==ScriptPopupFile && file==changed_script && !strcmp(key,"CORNERWARNING") && !strcmp(icon,"econwin_sm.pcx") && a==0 && b==0);++pops;popup_key=key;
}
static struct NetMessage {
    void pop(const char *key,int a,int b,int c) { assert(a==0 && b==0 && c==0);++pops;popup_key=key; }
} NetMsgPrimary;
@@ACTUAL_SOURCE@@
static void reset(unsigned short type,int callback_mode=0) {
    std::memset(packet,0,sizeof packet);message=reinterpret_cast<char *>(packet);packet[0]=type;
    packet[1]=0;packet[2]=0x12345678;packet[4]=1;packet[5]=2;packet[6]=3;packet[7]=4;
    opcode=type;mode=callback_mode;actions=seeds=parses=pops=spy_calls=0;popup_key=0;year_input=tech_input=-1;
    spy_result[0]=spy_result[1]=0;GameDataStateStorage.fields_.game_state_=0;ConsoleGlobal.faction_id_=0;
    ScriptPopupFile=script;std::memset(PlayersData,0,sizeof PlayersData);
    for(int i=0;i<8;++i) { Players[i].is_leader_female=i%2;Players[i].noun_gender=i+10;Players[i].is_noun_plural=(i+1)%2; }
}
int main() {
    assert(sizeof(Player)==0x59C && sizeof(PlayerData)==0x20CC);
    assert(offsetof(Player,is_leader_female)==0 && offsetof(Player,adj_name_faction)==0x304);
    assert(offsetof(PlayerData,energy_reserves)==0x220 && offsetof(PlayerData,loan_balance)==0xC4 && offsetof(PlayerData,loan_payment)==0xE4);
    for(unsigned short type=0x4441;type<=0x444F;++type) for(int callback_mode=0;callback_mode<2;++callback_mode) {
        reset(type,callback_mode);NetDaemon daemon;std::memset(&daemon,0,sizeof daemon);
        daemon.channels_[1].leading_words_[1]=type&~0x4000u;
        daemon.route(message);
        const bool seeded=(type>=0x4449 && type<=0x444D);
        assert(seeds==int(seeded));
        if(type!=0x4447 && type!=0x444E && type!=0x444F) {
            assert(actions==1 && args[0]==(callback_mode?(seeded?2:4):1) && args[1]==(callback_mode?(seeded?3:5):2));
            if(type<=0x4446 || type==0x444B || type==0x444C) assert(args[2]==(callback_mode?(seeded?4:6):3));
            if(type==0x4443 || type==0x4446) assert(args[3]==(callback_mode?7:4));
        } else assert(actions==0);
        if(type==0x4447) assert(PlayersData[callback_mode?4:1].energy_reserves==(callback_mode?5:2) && PlayersData[callback_mode?6:3].energy_reserves==(callback_mode?7:4));
        if(type==0x444E) assert(PlayersData[callback_mode?4:1].loan_balance[callback_mode?5:2]==(callback_mode?6:3) &&
            PlayersData[callback_mode?4:1].loan_payment[callback_mode?5:2]==(callback_mode?7:4));
        if(type==0x444F) assert(parses==4 && pops==1);
        assert(daemon.channels_[1].leading_words_[1]==0);
    }
    // Every 4441 branch uses post-parser flags; low bit priority and truce's
    // conditional treaty check are independent of actor/other UI reloads.
    const uint32_t flags[]={1,2,3,4,0x14,0x10004,0x10014,0x10,0};
    for(unsigned n=0;n<sizeof(flags)/sizeof(*flags);++n) for(int treaty=0;treaty<2;++treaty) {
        reset(0x4441);packet[6]=flags[n];GameDataStateStorage.fields_.game_state_=0x80;
        PlayersData[1].diplo_treaties[2]=treaty?0x10:0;
        assert(netdaemon_handle_table_h(message));
        const char *expected=(flags[n]&1)?"ENEMIESTEAMUP2":(flags[n]&2)?"ENEMYTREATY":
            ((flags[n]&4)&&(!(flags[n]&0x10000)||treaty))?"ENEMYTRUCE":(flags[n]&0x10)?"ENEMYWAR":0;
        assert(parses==3 && pops==int(expected!=0));if(expected) assert(!strcmp(popup_key,expected));
    }
    reset(0x4441,3);spy_result[0]=spy_result[1]=0;GameDataStateStorage.fields_.game_state_=0x80;
    assert(netdaemon_handle_table_h(message));assert(spy_calls==2 && parses==3 && hood_actor==6 && hood_other==7 && !strcmp(popup_key,"ENEMYTRUCE"));
    for(int spy=0;spy<2;++spy) {
        reset(0x4441);spy_result[0]=spy;spy_result[1]=!spy;
        assert(netdaemon_handle_table_h(message));assert(spy_calls==(spy?1:2)&&parses==3);
    }
    reset(0x4441);ConsoleGlobal.faction_id_=1;assert(netdaemon_handle_table_h(message));assert(spy_calls==0 && parses==0);
    reset(0x444F);ConsoleGlobal.faction_id_=1;assert(netdaemon_handle_table_h(message));assert(parses==0&&pops==0&&seeds==0);
    for(unsigned short initial=0x444B;initial<=0x444C;++initial)
    for(unsigned short final=0x444B;final<=0x444C;++final) for(int local=0;local<2;++local) {
        reset(initial,2);ConsoleGlobal.faction_id_=3;post_tech_opcode=final;post_recipient=local?3:2;
        assert(netdaemon_handle_table_h(message));const bool announce=final==0x444C&&local;
        assert(parses==int(announce?2:0)&&pops==int(announce));if(announce) assert(tech_input==5&&!strcmp(popup_key,"STOLETECH"));
    }
    reset(0x4447);packet[4]=0;packet[6]=0;assert(netdaemon_handle_table_h(message));for(int i=0;i<8;++i) assert(PlayersData[i].energy_reserves==0);
    reset(0x4447);packet[5]=1;packet[6]=0;PlayersData[1].energy_reserves=0x7FFFFFFF;
    assert(netdaemon_handle_table_h(message));assert(uint32_t(PlayersData[1].energy_reserves)==0x80000000u);
    // In-owner aliases model exact load/store order, not transport reachability
    // or fault/neighbor effects. The full actual PlayersData array stays live.
    reset(0x444E);message=reinterpret_cast<char *>(&PlayersData[1])+offsetof(PlayerData,loan_balance)-0x10;
    set_opcode(0x444E);write_word(0x10,1);write_word(0x14,0);write_word(0x18,2);write_word(0x1C,123);
    assert(netdaemon_handle_table_h(message));assert(PlayersData[1].loan_balance[0]==2 && PlayersData[1].loan_payment[0]==0 && PlayersData[2].loan_payment[0]==123);
    reset(0x4447);message=reinterpret_cast<char *>(&PlayersData[1])+offsetof(PlayerData,energy_reserves)-0x18;
    set_opcode(0x4447);write_word(0x10,1);write_word(0x14,1);write_word(0x18,2);write_word(0x1C,7);
    assert(netdaemon_handle_table_h(message));assert(PlayersData[1].energy_reserves==3 && PlayersData[2].energy_reserves==0 && PlayersData[3].energy_reserves==7);
}
'''


@pytest.mark.parametrize("compiler", [c for c in ("g++", "clang++") if shutil.which(c)])
def test_actual_table_h_routes_live_fields_grammar_and_in_owner_aliases(tmp_path,compiler):
    source=(ROOT/"src/netdaemon.cpp").read_text();header=(ROOT/"src/faction.h").read_text()
    declarations="\n".join(extract(header,"struct "+name+" {")+";" for name in ("SocialCategory","SocialEffect","Goal","Player","PlayerData"))
    declarations+="\n"+next(line for line in header.splitlines() if line=="static const int MaxPlayerNum = 8;")
    lock=extract((ROOT/"src/netdaemon.h").read_text(),"struct LockStateEntry {")+";"
    lock=lock.replace("int *vehicle_selection_follow;","uint32_t vehicle_selection_follow;")
    body="\n".join(extract(source,s) for s in ("static void netdaemon_process_message_marker_tail(","static void netdaemon_table_h_add_energy(","static bool netdaemon_handle_table_h("))
    process=extract(source,"void NetDaemon::process_message(");start=process.index("    if (netdaemon_handle_table_h(message))")
    stop=process.index("\n\n    if (netdaemon_handle_five_message_routes",start)
    body+="\nvoid NetDaemon::route(char *message) {\nunsigned char *msg=reinterpret_cast<unsigned char *>(message);\n"+process[start:stop]+"\nassert(false);\n}"
    def run(candidate):
        cpp=tmp_path/"h.cpp";exe=tmp_path/"h"
        cpp.write_text(H_CALLBACK_CONTROL.replace("@@FACTION_DECLARATIONS@@",declarations).replace("@@LOCK_STATE@@",lock).replace("@@ACTUAL_SOURCE@@",candidate))
        built=subprocess.run([compiler,"-std=c++03","-Wall","-Wextra","-Werror","-Wno-unused-function","-fsanitize=address,undefined",str(cpp),"-o",str(exe)],capture_output=True,text=True,timeout=60)
        assert built.returncode==0,built.stderr
        return subprocess.run([str(exe)],capture_output=True,text=True,timeout=10,env=dict(os.environ,ASAN_OPTIONS="detect_leaks=0"))
    result=run(body);assert result.returncode==0,result.stderr
    for before,after in (
        ("treaty_on(*reinterpret_cast<int *>(message + 0x10),", "treaty_on(actor,"),
        ("treaty_off(*reinterpret_cast<int *>(message + 0x10),", "treaty_off(actor,"),
        ("set_treaty(*reinterpret_cast<int *>(message + 0x10),", "set_treaty(actor,"),
        ("agenda_on(*reinterpret_cast<int *>(message + 0x10),", "agenda_on(actor,"),
        ("agenda_off(*reinterpret_cast<int *>(message + 0x10),", "agenda_off(actor,"),
        ("set_agenda(*reinterpret_cast<int *>(message + 0x10),", "set_agenda(actor,"),
        ("trade_maps(*reinterpret_cast<int *>(message + 0x10),", "trade_maps(actor,"),
        ("give_a_base(*reinterpret_cast<int *>(message + 0x10),", "give_a_base(actor,"),
        ("do_withdrawal(*reinterpret_cast<int *>(message + 0x10),", "do_withdrawal(actor,"),
        ("pact_ends(*reinterpret_cast<int *>(message + 0x10),", "pact_ends(actor,"),
        ("tech_achieved(*reinterpret_cast<int *>(message + 0x10),", "tech_achieved(actor,"),
        ("const int checked_actor = *reinterpret_cast<int *>(message + 0x10);", "const int checked_actor = actor;"),
        ("const int noun_other = *reinterpret_cast<int *>(message + 0x14);", "const int noun_other = other;"),
        ("const int announcement_flags = *reinterpret_cast<int *>(message + 0x18);", "const int announcement_flags = flags;"),
        ("(!(announcement_flags & 0x10000) || has_treaty(", "((announcement_flags & 0x10000) && has_treaty("),
        ("const int current_actor = *reinterpret_cast<int *>(message + 0x10);", "const int current_actor = actor;"),
        ("const int current_other = *reinterpret_cast<int *>(message + 0x18);", "const int current_other = other;"),
        ("if (*reinterpret_cast<unsigned short *>(message) == 0x444C &&", "if (opcode == 0x444C &&"),
        ("*reinterpret_cast<int *>(message + 0x18) == ConsoleGlobal.faction_id_", "recipient == ConsoleGlobal.faction_id_"),
        ("const int announced_actor = *reinterpret_cast<int *>(message + 0x10);", "const int announced_actor = actor;"),
        ("say_tech(StringTemp, *reinterpret_cast<int *>(message + 0x14), 0);", "say_tech(StringTemp, tech, 0);"),
        ("log_say(\"Received CORNERING\"", "log_say_hex(\"Received CORNERING\""),
        ("const int leader_actor = *reinterpret_cast<int *>(message + 0x10);", "const int leader_actor = actor;"),
        ("GenderDefault = Players[leader_actor].is_leader_female;", "GenderDefault = 0;"),
        ("GenderDefault = Players[name_actor].is_leader_female;", "(void)GenderDefault;"),
        (".adj_name_faction, -1, -1);", ".adj_name_faction + 4, -1, -1);"),
        ("*reinterpret_cast<int *>(message + 0x14))));", "year)));"),
        ("bits += static_cast<uint32_t>(delta);", "bits += static_cast<uint32_t>(delta) + 1u;"),
        ("netdaemon_process_message_marker_tail(*this, msg);", "(void)msg;"),
    ):
        assert before in body,before
        damaged=run(body.replace(before,after,1));assert damaged.returncode!=0,before
    early_other=body.replace("        if (current_actor != 0)",
        "        const int current_other = *reinterpret_cast<int *>(message + 0x18);\n        if (current_actor != 0)",1)
    early_other=early_other.replace("        const int current_other = *reinterpret_cast<int *>(message + 0x18);\n        if (current_other != 0)",
        "        if (current_other != 0)",1)
    assert early_other!=body and run(early_other).returncode!=0
    stale_loan=body.replace("PlayersData[*reinterpret_cast<int *>(message + 0x10)]\n            .loan_payment", "PlayersData[actor]\n            .loan_payment",1)
    assert stale_loan!=body and run(stale_loan).returncode!=0


def test_pinned_complete_table_h_live_reads_owners_and_local_exit():
    from hashlib import sha256
    from struct import unpack
    from capstone import Cs,CS_ARCH_X86,CS_MODE_32
    from match_recovery.model import Span
    from match_recovery.pe_coff import PeImage
    path=Path(os.environ.get("OPENSMACX_IMAGE",ROOT/".opensmacx/game/terranx_original.exe"))
    if not path.is_file(): pytest.skip("pinned original unavailable")
    raw=path.read_bytes();pin="01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf"
    assert sha256(raw).hexdigest()==pin
    image=PeImage(raw,pin)
    assert unpack("<15I",image.read(Span(0x5389AC,0x5389E8)))==(
        0x536DCB,0x536F51,0x536F80,0x536FB3,0x536FE2,0x537011,0x537044,
        0x53710A,0x537133,0x537165,0x537197,0x537197,0x53725D,0x5370A8,0x53728F)
    ins={i.address:(i.mnemonic,i.op_str.lower())
         for i in Cs(CS_ARCH_X86,CS_MODE_32).disasm(image.read(Span(0x536DA5,0x5373E7)),0x536DA5)}
    pins={
        0x536DB0:("je","0x5373af"),0x536DB6:("add","eax, 0xffffbbbf"),
        0x536DBB:("cmp","eax, 0xe"),0x536DBE:("ja","0x537f7a"),
        0x536DC4:("jmp","dword ptr [eax*4 + 0x5389ac]"),
        0x536DDC:("call","0x6263f0"),0x536DE7:("mov","ecx, dword ptr [ebx + 0x10]"),
        0x536DED:("call","0x55b870"),0x536DF2:("mov","eax, dword ptr [ebx + 0x10]"),
        0x536E1C:("mov","edx, dword ptr [ebx + 0x14]"),0x536E39:("mov","ecx, dword ptr [ebx + 0x10]"),
        0x536E74:("mov","ecx, dword ptr [ebx + 0x14]"),
        0x536EAF:("mov","edx, dword ptr [ebx + 0x14]"),0x536EB2:("mov","eax, dword ptr [ebx + 0x10]"),
        0x536ECE:("mov","eax, dword ptr [ebx + 0x18]"),0x536F00:("test","eax, 0x10000"),
        0x536F05:("je","0x536f29"),0x536F07:("mov","ecx, dword ptr [ebx + 0x10]"),
        0x536F1F:("test","byte ptr [ecx*4 + 0x96c9f8], 0x10"),
        0x536F73:("call","0x55b760"),0x536FA6:("call","0x55bb30"),
        0x536FD5:("call","0x55ba80"),0x537004:("call","0x55b820"),0x537037:("call","0x55bba0"),
        0x537055:("call","0x6262f0"),0x53705A:("mov","eax, dword ptr [ebx + 0x10]"),
        0x537071:("mov","edx, dword ptr [ebx + 0x14]"),
        0x537077:("add","dword ptr [eax*4 + 0x96cc00], edx"),
        0x53707E:("mov","eax, dword ptr [ebx + 0x18]"),
        0x53709C:("add","dword ptr [eax*4 + 0x96cc00], edx"),
        0x5370BE:("mov","eax, dword ptr [ebx + 0x10]"),
        0x5370DC:("mov","dword ptr [edx*4 + 0x96caa4], eax"),
        0x5370E3:("mov","eax, dword ptr [ebx + 0x10]"),
        0x5370F6:("mov","eax, dword ptr [ebx + 0x14]"),
        0x5370FE:("mov","dword ptr [edx*4 + 0x96cac4], eax"),
        0x537126:("call","0x543a20"),0x537158:("call","0x54cf40"),
        0x53718A:("call","0x548f80"),0x5371C4:("call","0x5bb000"),
        0x5371CC:("cmp","word ptr [ebx], 0x444c"),
        0x5371DD:("mov","eax, dword ptr [ebx + 0x18]"),0x5371E8:("mov","ecx, dword ptr [ebx + 0x10]"),
        0x53722A:("mov","eax, dword ptr [ebx + 0x14]"),0x537282:("call","0x53c650"),
        0x5372A0:("call","0x6262f0"),0x5372A5:("mov","ecx, dword ptr [ebx + 0x10]"),
        0x5372B2:("je","0x538266"),0x5372D2:("mov","ecx, dword ptr [eax + 0x946a50]"),
        0x5372D8:("lea","edx, [eax + 0x946a9c]"),0x5372EB:("mov","ecx, dword ptr [ebx + 0x10]"),
        0x537306:("mov","ecx, dword ptr [eax + 0x946a50]"),0x53730C:("lea","eax, [eax + 0x946a84]"),
        0x537320:("mov","ecx, dword ptr [ebx + 0x10]"),
        0x53735B:("mov","eax, dword ptr [ebx + 0x10]"),
        0x53736D:("lea","eax, [edx*4 + 0x946d54]"),
        0x53737C:("mov","ecx, dword ptr [ebx + 0x14]"),0x537383:("call","0x5c89a0"),
        0x53738F:("mov","edx, dword ptr [0x691b0c]"),0x5373A2:("call","0x48c0a0"),
        0x5373AA:("jmp","0x538266"),0x5373DA:("call","0x428350"),
    }
    for address,expected in pins.items():assert ins[address]==expected,hex(address)
    for address,literal in ((0x68D060,b"ENEMIESTEAMUP2"),(0x68D070,b"ENEMYTREATY"),
                            (0x68D07C,b"ENEMYTRUCE"),(0x68D088,b"ENEMYWAR"),
                            (0x68D168,b"STOLETECH"),(0x68D19C,b"econwin_sm.pcx"),(0x68D1AC,b"CORNERWARNING")):
        assert image.read(Span(address,address+len(literal)+1))==literal+b"\0"
    # 4600's adjacent council target is not part of 444F. Source row controls
    # prove in-owner selection only, never unbounded memory or action runtime.


HIGH_CALLBACK_CONTROL = r'''
#include <cassert>
#include <stdint.h>
#include <cstring>
#include <cstddef>
#include <string>
#include <vector>
typedef int BOOL;
typedef char *LPSTR;
@@DECLARATIONS@@
static Player Players[8];
static Landmark MapLandmark[64];
static RulesTimeControl TimeControl[8];
static struct { struct Fields { int turn_control_duration_milliseconds_, time_controls_selection_; } fields_; } GameDataStateStorage;
static struct { int faction_id_; } ConsoleGlobal;
static int NetDaemonWaitTicks, GenderDefault, PluralityDefault, MultiWindowContactStatusMask, CommlinkPending;
static char StringTemp[1024];
static unsigned char *active_packet;
static std::vector<std::string> events;
static int mode, read_count, name_forms, logged_a, logged_b, logged_c, action_x, action_y, draws, slot, recipient;
static void put(int offset,int value) { std::memcpy(active_packet+offset,&value,4); }
static void observed_read(void *target,const void *source,size_t size) { ++read_count; std::memcpy(target,source,size); }
static const char *observed_name(const unsigned char *message) { ++name_forms; return reinterpret_cast<const char *>(message+0x20); }
static void log_say(const char *label,int a,int b,int c) {
    events.push_back(label); logged_a=a; logged_b=b; logged_c=c;
    if(mode==1) { put(4,3); put(0x10,7); put(0x14,9); }
    if(mode==6) active_packet[0]=0x42;
    if(mode==7) ConsoleGlobal.faction_id_=2;
}
static void log_say(const char *label,const char *,int a,int b,int c) { log_say(label,a,b,c); }
static unsigned long timeGetTime() { events.push_back("clock"); NetDaemonWaitTicks=30; return 100; }
struct Net { char *get_player_name(unsigned long pid) { assert(pid==99); events.push_back("name"); return const_cast<char *>("peer"); } };
struct NetDaemon : Net {
    PrimaryLockStateTail primary_state_; LockStateEntry channels_[2];
    int pid_2_idx(uint32_t pid);
    void route(unsigned char *,unsigned short,int,int);
};
static NetDaemon NetDaemonNet;
int NetDaemon::pid_2_idx(uint32_t pid) { assert(this==&NetDaemonNet && pid==99); events.push_back("lookup"); return slot; }
static void my_srand(int value) { events.push_back("seed"); recipient=value; }
static void council(int a,int b,int c) { action_x=a;action_y=b;recipient=c;events.push_back("council"); }
static struct {
    void set_vote(int a,int b) { action_x=a;action_y=b;events.push_back("vote"); }
    void arrived(int a) { action_x=a;events.push_back("arrive"); }
    void make_resolution(int a) { assert(a==1);events.push_back("resolution"); }
} CouncilWindow;
struct GraphicWin { void redraw() { events.push_back("redraw"); if(mode==2) GameDataStateStorage.fields_.time_controls_selection_=2; } };
static GraphicWin MultiWindow;
static struct { void play(int id) { assert(id==21); events.push_back("sound"); if(mode==3) NetDaemonNet.primary_state_.trailing_words_[4]=0x80; } } g_FX;
static struct { void pop(const char *label,int,int,void *) { events.push_back(label); } } NetMsgPrimary;
static void parse_says(int id,const char *text,int,int) {
    events.push_back(std::string("says")+char('0'+id)+":"+text);
    if(mode==2) GameDataStateStorage.fields_.time_controls_selection_=3;
}
static int parse_say(int id,char *text,int,int) {
    assert(id==1); events.push_back(std::string("say1:")+text);
    if(mode==2) { GameDataStateStorage.fields_.time_controls_selection_=1; ConsoleGlobal.faction_id_=4; }
    return 0;
}
static void parse_num(int id,int value) {
    assert(value==((mode==2 ? (id%3)+1 : 1)*100)+id);
    events.push_back(std::string("num")+char('0'+id));
    if(mode==2) GameDataStateStorage.fields_.time_controls_selection_=(id+1)%3+1;
}
static void say_num(int value) { events.push_back(value?"suffix1":"suffix0"); strcat(StringTemp,value?"1":"0"); }
static void set_time_controls() { events.push_back("set"); GameDataStateStorage.fields_.turn_control_duration_milliseconds_=250; }
static struct { void new_chat(char *,int value) { recipient=value; events.push_back("lobby"); } } NetWinDialog;
static struct { void new_chat(char *,int value) { recipient=value; events.push_back("diplo"); } } DiploWindow;
static struct { void chat(char *text,int faction) { events.push_back(std::string("chat:")+text); recipient=faction; } } MainInterfaceGlobal;
static void new_landmark(int x,int y,const char *) { action_x=x;action_y=y;events.push_back("new"); if(mode==1) put(4,2); }
static void kill_landmark(int x,int y) { new_landmark(x,y,0); events.back()="kill"; }
static int find_landmark(int x,int y,int flag) { assert(flag==1); new_landmark(x,y,0);events.back()="find"; return slot; }
static void draw_map(int flag) { assert(flag==1); ++draws; events.push_back("draw"); }
static void message_data(int op,int who,int acquired,int,int,int) { assert(op==0x1240&&who==1); recipient=acquired; events.push_back("reply"); }
static char *observed_cat(char *dest,const char *text) {
    char *result=std::strcat(dest,text);
    if(mode==4 && text==Players[2].title_leader) put(4,3);
    if(mode==4 && text==reinterpret_cast<char *>(active_packet+0x10)) put(4,4);
    return result;
}
#define strcat observed_cat
@@ACTUAL_SOURCE@@
#undef strcat
static unsigned char storage[128];
static void reset(NetDaemon &daemon) {
    std::memset(&daemon,0,sizeof daemon);std::memset(&NetDaemonNet,0,sizeof NetDaemonNet);
    std::memset(storage,0,sizeof storage);active_packet=storage;
    put(4,2);put(0xc,11);put(0x10,4);put(0x14,6);std::strcpy(reinterpret_cast<char *>(storage+0x20),"place");
    ConsoleGlobal.faction_id_=2;NetDaemonWaitTicks=40;GenderDefault=8;PluralityDefault=9;
    GameDataStateStorage.fields_.turn_control_duration_milliseconds_=200;
    GameDataStateStorage.fields_.time_controls_selection_=1;
    mode=read_count=name_forms=draws=MultiWindowContactStatusMask=CommlinkPending=0;slot=0;recipient=-1;events.clear();
    StringTemp[0]=0;
}
int main() {
    NetDaemon daemon;
    for(int i=0;i<8;++i) {
        std::strcpy(Players[i].title_leader,i==2?"Title2":"Title3");std::strcpy(Players[i].name_leader,i==3?"Name3":"Name2");Players[i].is_leader_female=i;
        TimeControl[i].name=const_cast<char *>(i==3?"control3":"control1");
        TimeControl[i].turn=i*100;TimeControl[i].base=i*100+1;TimeControl[i].unit=i*100+2;
        TimeControl[i].extra=i*100+3;TimeControl[i].refresh=i*100+4;TimeControl[i].accumulated=i*100+5;
    }
    for(int op=0x4600;op<=0x4603;++op) {
        reset(daemon);mode=1;ConsoleGlobal.faction_id_=3;put(8,17);put(0x18,13);daemon.channels_[1].leading_words_[1]=op&0x1fff;
        daemon.route(storage,op,99,1);assert(daemon.channels_[1].leading_words_[1]==0);
        if(op<=0x4601)assert(action_x==7&&action_y==9&&events[1]=="seed");
        if(op==0x4600)assert(recipient==13&&events.back()=="council");
        if(op==0x4602)assert(action_x==7&&events.back()=="arrive");
        if(op==0x4603)assert(events.back()=="resolution");
    }
    reset(daemon);assert(!netdaemon_handle_chat_landmark_message(daemon,storage,0x9999,99));assert(read_count==0&&events.empty());
    for(int op=0x4e01;op<=0x4e03;++op) {
        reset(daemon);mode=1;put(4,3);daemon.channels_[1].leading_words_[1]=op&0x1fff;
        daemon.route(storage,op,99,1);assert(action_x==7&&action_y==9&&draws==1);
        assert(name_forms==(op==0x4e02?0:1));
        assert(daemon.channels_[1].leading_words_[1]==0);
        if(op==0x4e03)assert(std::string(MapLandmark[0].name)=="place");
    }
    reset(daemon);mode=1;slot=-1;assert(netdaemon_handle_chat_landmark_message(daemon,storage,0x4e03,99));assert(draws==1);
    reset(daemon);mode=6;daemon.channels_[1].leading_words_[1]=0x0e01;
    daemon.route(storage,0x4e01,99,1);assert(daemon.channels_[1].leading_words_[1]==0x0e01);
    for(int op=0x1e00;op<=0x4e00;op+=0x3000) {
        reset(daemon);mode=1;MultiWindowContactStatusMask=8;
        daemon.route(storage,op,99,1);assert(events.size()==1);
        reset(daemon);daemon.primary_state_.trailing_words_[0]=1;slot=5;daemon.route(storage,op,99,1);
        assert(recipient==5&&events.back()=="lobby");
        reset(daemon);daemon.primary_state_.trailing_words_[0]=1;slot=-1;daemon.route(storage,op,99,1);assert(recipient==-1);
        reset(daemon);CommlinkPending=1;mode=1;daemon.route(storage,op,99,1);assert(recipient==3&&events.back()=="diplo");
        reset(daemon);daemon.primary_state_.trailing_words_[0]=0xffffffffu;
        std::strcpy(reinterpret_cast<char *>(storage+0x10),"hello");daemon.route(storage,op,99,1);
        assert(events.back()=="chat:peer: hello");
        reset(daemon);daemon.primary_state_.trailing_words_[0]=2;mode=4;
        std::strcpy(reinterpret_cast<char *>(storage+0x10),"hello");daemon.route(storage,op,99,1);
        assert(events.back()=="chat:Title2 Name3: hello"&&recipient==4&&GenderDefault==3&&PluralityDefault==0);
    }
    reset(daemon);daemon.route(storage,0x4b00,99,1);assert(events.back()=="CHANGINGSERVERS");
    reset(daemon);daemon.route(storage,0x4c00,99,33);assert(daemon.primary_state_.trailing_words_[4]==2&&daemon.primary_state_.trailing_words_[5]==70);
    reset(daemon);daemon.primary_state_.trailing_words_[4]=3;daemon.route(storage,0x4c01,99,0);assert(daemon.primary_state_.trailing_words_[4]==2);
    reset(daemon);daemon.primary_state_.trailing_words_[4]=1;daemon.route(storage,0x4c01,99,0);assert(daemon.primary_state_.trailing_words_[4]==0&&events[1]=="sound");
    reset(daemon);ConsoleGlobal.faction_id_=1;put(0x10,1);daemon.route(storage,0x4c02,99,1);
    assert(NetDaemonWaitTicks==90&&events.size()==7&&events[2]=="redraw"&&events[3]=="name"&&events[4]=="says0:peer"&&events[5]=="say1:control1"&&events.back()=="WECHANGEDTIME");
    // Popup adds a seventh event after the unconditional parser work.
    reset(daemon);ConsoleGlobal.faction_id_=7;put(0x10,1);mode=2;daemon.route(storage,0x4c02,99,1);
    assert(NetDaemonWaitTicks==90&&events[5]=="say1:control3"&&events.back()=="HECHANGEDTIME1");
    reset(daemon);ConsoleGlobal.faction_id_=1;put(0x10,1);mode=2;daemon.route(storage,0x4c02,99,1);
    assert(events.back()=="HECHANGEDTIME1"); // Local comparison follows both parser callbacks.
    reset(daemon);NetDaemonWaitTicks=0x7fffffff;GameDataStateStorage.fields_.turn_control_duration_milliseconds_=-2147483647-1;
    ConsoleGlobal.faction_id_=1;put(0x10,1);daemon.route(storage,0x4c02,99,1);assert(NetDaemonWaitTicks==249);
    // Original lock mask stores can alias the packet within the real channel.
    reset(daemon);active_packet=reinterpret_cast<unsigned char *>(&daemon.channels_[0]);
    put(0x10,1);put(0x14,32);daemon.route(active_packet,0x8240,99,1);
    assert(logged_b==1&&logged_c==35&&recipient==1&&daemon.channels_[0].trailing_words_[2]==3);
    reset(daemon);daemon.channels_[0].trailing_words_[0]=6;daemon.channels_[0].trailing_words_[2]=2;
    daemon.route(storage,0x8241,99,1);assert(logged_b==2&&logged_c==4&&daemon.channels_[0].trailing_words_[2]==0);
}
'''


@pytest.mark.parametrize("compiler", [c for c in ("g++","clang++") if shutil.which(c)])
def test_actual_high_routes_guards_callbacks_owners_and_common_tail(tmp_path,compiler):
    source=(ROOT/"src/netdaemon.cpp").read_text()
    faction=(ROOT/"src/faction.h").read_text();net=(ROOT/"src/netdaemon.h").read_text()
    declarations="\n".join(extract(faction,"struct "+name+" {")+";" for name in ("SocialCategory","SocialEffect","Goal","Player"))
    declarations+="\n"+extract(net,"struct PrimaryLockStateTail {")+";\n"+extract(net,"struct LockStateEntry {").replace("int *vehicle_selection_follow;","uint32_t vehicle_selection_follow;")+";\n"
    declarations+=extract((ROOT/"src/alpha.h").read_text(),"struct RulesTimeControl {")+";\n"+extract((ROOT/"src/map.h").read_text(),"struct Landmark {")+";"
    declarations+="\n"+extract(source,"struct MessageDataRecord {")+";"
    view=extract(source,"class MessageCommandView {").replace("std::memcpy(","observed_read(")+";"
    body=view+"\n"+"\n".join(extract(source,s) for s in (
        "static void netdaemon_process_message_marker_tail(","static bool process_council_message(","static bool netdaemon_handle_lock_mask_residual(",
        "static bool netdaemon_handle_server_change_pause(","static bool netdaemon_handle_chat_landmark_message(",
        "static int netdaemon_subtract_i32_bits(","static bool netdaemon_handle_time_control("))
    # Observe only evaluation of the real source pointer expression, within
    # allocated fixture storage. This is not a short-packet/OOB runtime test.
    body=body.replace("reinterpret_cast<const char *>(msg + 0x20)","observed_name(msg)")
    parent=extract(source,"void NetDaemon::process_message(")
    start=parent.index("    if (process_council_message(msg, type))");end=parent.index("    if (type == 0x4D00)",start)
    body+="\nvoid NetDaemon::route(unsigned char *msg,unsigned short type,int sender_id,int entry_sender_identity) {\nstd::memcpy(msg,&type,2);\n"+parent[start:end]+"assert(false);\n}"
    def run(candidate):
        cpp=tmp_path/"high.cpp";exe=tmp_path/"high"
        cpp.write_text(HIGH_CALLBACK_CONTROL.replace("@@DECLARATIONS@@",declarations).replace("@@ACTUAL_SOURCE@@",candidate))
        built=subprocess.run([compiler,"-std=c++11","-Wall","-Wextra","-Werror","-Wno-unused-variable","-Wno-unused-function",str(cpp),"-o",str(exe)],capture_output=True,text=True,timeout=60)
        assert built.returncode==0,built.stderr
        return subprocess.run([str(exe)],capture_output=True,text=True,timeout=10)
    result=run(body);assert result.returncode==0,result.stderr
    for before,after in (
        ("MultiWindow.GraphicWin::redraw();", "/* omitted native redraw */"),
        ("    parse_say(1, TimeControl[GameDataStateStorage.fields_.time_controls_selection_].name,\n              -1, -1);", "/* omitted native name */"),
        ("parse_num(1, TimeControl[GameDataStateStorage.fields_.time_controls_selection_].base)","parse_num(1, TimeControl[1].base)"),
        ("const int slot = NetDaemonNet.pid_2_idx(","const int slot = daemon.pid_2_idx("),
        ("                                 packet.faction());","                                 sender_id);"),
        ("if (state < 2)","if (daemon.primary_state_.trailing_words_[0] < 2)"),
        ("MainInterfaceGlobal.chat(StringTemp, packet.faction())","MainInterfaceGlobal.chat(StringTemp, 2)"),
        ("new_landmark(packet.payload_10(), packet.payload_14(), name)","new_landmark(4, 6, name)"),
        ("kill_landmark(packet.payload_10(), packet.payload_14())","kill_landmark(4, 6)"),
        ("find_landmark(packet.payload_10(), packet.payload_14(), 1)","find_landmark(4, 6, 1)"),
        ("if (packet.faction() == ConsoleGlobal.faction_id_)","if (3 == ConsoleGlobal.faction_id_)"),
        ("                *reinterpret_cast<int *>(message + 0x14));", "                second);"),
    ):
        assert before in body
        assert run(body.replace(before,after)).returncode!=0,before
    guard="    if (type != 0x1E00"
    assert guard in body
    assert run(body.replace(guard,"    observed_read(&read_count, msg + 4, 4);\n"+guard,1)).returncode!=0
    declaration="        const char *const name = observed_name(msg);\n"
    assert body.count(declaration)==2
    hoisted=body.replace(declaration,"").replace("    if (type == 0x4E01) {", "    const char *const name = observed_name(msg);\n    if (type == 0x4E01) {",1)
    assert run(hoisted).returncode!=0


def test_pinned_complete_high_family_guards_calls_and_capture_boundaries():
    from hashlib import sha256
    from capstone import Cs,CS_ARCH_X86,CS_MODE_32
    from match_recovery.model import Span
    from match_recovery.pe_coff import PeImage
    path=Path(os.environ.get("OPENSMACX_IMAGE",ROOT/".opensmacx/game/terranx_original.exe"))
    if not path.is_file(): pytest.skip("pinned original unavailable")
    raw=path.read_bytes();pin="01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf"
    assert sha256(raw).hexdigest()==pin
    image=PeImage(raw,pin)
    decoded={i.address:i for start,end in ((0x516490,0x5165A6),(0x592EE0,0x592F50),(0x593110,0x593190),
             (0x535A17,0x535A3D),(0x536781,0x536939),(0x536DA5,0x538266))
             for i in Cs(CS_ARCH_X86,CS_MODE_32).disasm(image.read(Span(start,end)),start)}
    targets={0x4600:0x5373AF,0x4601:0x53744C,0x4602:0x537424,0x4603:0x537483,
             0x4B00:0x5374B2,0x4C00:0x5374D5,0x4C01:0x537709,0x4C02:0x537574,
             0x4D00:0x537775,0x4E00:0x53797C,0x4E01:0x537ADF,0x4E02:0x537BA5,
             0x4E03:0x537B44,0x8240:0x537CE9,0x8241:0x537F35}
    # Mechanically execute only the scalar opcode-selection instructions.
    # No packet memory/action is emulated, and no low-page/OOB access is run.
    def select(opcode,start=0x536DA5):
        pc=start;eax=opcode;comparison=0;unsigned_above=False
        for _ in range(50):
            if pc in set(targets.values())|{0x537F7A}: return pc
            i=decoded[pc];m=i.mnemonic;s=i.op_str;next_pc=pc+i.size
            if m=="cmp":
                assert s.startswith("eax, ");operand=int(s.split(", ")[1],0)
                comparison=eax-operand;unsigned_above=(eax&0xffffffff)>(operand&0xffffffff)
            elif m in ("sub","add"):
                assert s.startswith("eax, ");v=int(s.split(", ")[1],0)
                eax=(eax+v if m=="add" else eax-v)&0xffffffff
                if eax&0x80000000:eax-=0x100000000
                comparison=eax
            elif m=="dec": assert s=="eax";eax-=1;comparison=eax
            elif m in ("je","jne","jg","jle","ja"):
                take={"je":comparison==0,"jne":comparison!=0,"jg":comparison>0,"jle":comparison<=0,"ja":unsigned_above}[m]
                if take:next_pc=int(s,0)
            else: raise AssertionError((hex(pc),m,s))
            pc=next_pc
        raise AssertionError("opcode guard did not terminate")
    for opcode,target in targets.items(): assert select(opcode)==target,hex(opcode)
    assert select(0x1E00,0x535A17)==targets[0x4E00]
    assert select(0x4440,0x536DB6)==0x537F7A # Guard's unsigned underflow, not a TableH arm.
    for opcode in (0x4604,0x4AFF,0x4B01,0x4C03,0x4CFF,0x4D01,0x4DFF,0x4E04,0x4EFF,0x823F,0x8242,0x8300):
        assert select(opcode)==0x537F7A,hex(opcode)
    pins={
        0x5373C0:("call","0x6262f0"),0x5373C5:("mov","eax, dword ptr [ebx + 8]"),
        0x5373CE:("mov","ecx, dword ptr [ebx + 0x18]"),0x5373DA:("call","0x428350"),
        0x537431:("call","0x6262f0"),0x537436:("mov","edx, dword ptr [ebx + 0x10]"),0x537442:("call","0x4271c0"),
        0x53745B:("call","0x6262f0"),0x537469:("mov","eax, dword ptr [ebx + 0x14]"),0x537479:("call","0x424720"),
        0x53749D:("call","0x424410"),0x5374BD:("call","0x6262f0"),
        0x5374DF:("call","dword ptr [0x669368]"),0x5374E5:("mov","ecx, dword ptr [0x93a960]"),
        0x5374FF:("mov","eax, dword ptr [esi + 0x1b50]"),0x537519:("mov","dword ptr [esi + 0x1b50], eax"),
        0x537732:("mov","eax, dword ptr [esi + 0x1b50]"),0x537755:("call","0x631a20"),
        0x53757E:("call","0x6262f0"),0x537590:("mov","ecx, dword ptr [ebx + 0x10]"),0x53759F:("call","0x523c60"),
        0x5375AF:("mov","ecx, 0x7fd648"),0x5375BA:("call","0x5d5a70"),0x5375C9:("call","0x631a20"),
        0x5375D6:("mov","ecx, dword ptr [0x9a64e0]"),0x5375EC:("call","0x625e50"),
        0x5375F9:("cmp","edi, eax"),0x537610:("mov","eax, dword ptr [0x9a64e0]"),
        0x537626:("mov","edx, dword ptr [0x9a64e0]"),0x53763D:("mov","ecx, dword ptr [0x9a64e0]"),
        0x537654:("mov","eax, dword ptr [0x9a64e0]"),0x53766A:("mov","edx, dword ptr [0x9a64e0]"),
        0x537681:("mov","ecx, dword ptr [0x9a64e0]"),0x5376AE:("mov","edx, dword ptr [0x9a64e0]"),
        0x537775:("mov","eax, dword ptr [ebx + 0x10]"),0x537778:("mov","ecx, dword ptr [ebx + 0x18]"),
        0x53777B:("mov","edi, dword ptr [ebx + 4]"),0x5377A5:("mov","dword ptr [ebp + 0xc], eax"),
        0x5377A8:("call","0x6169a0"),0x5377B3:("call","0x626250"),0x5377C8:("mov","eax, dword ptr [ebx + 0x14]"),
        0x537807:("mov","eax, dword ptr [ebp + 8]"),0x53784F:("mov","ecx, dword ptr [ebp + 0xc]"),
        0x53790F:("mov","eax, dword ptr [ebx + 0x14]"),0x537919:("mov","edx, dword ptr [ebp - 0x14]"),
        0x537932:("mov","edx, dword ptr [ebp + 0x10]"),
        0x537993:("mov","ecx, dword ptr [ebx + 4]"),0x5379BB:("mov","ecx, 0x93cd90"),
        0x5379C0:("call","0x4e25e0"),0x5379EF:("push","ecx"),0x5379F6:("call","0x4412f0"),
        0x537A10:("jge","0x537a2d"),0x537A2D:("mov","ecx, dword ptr [ebx + 4]"),
        0x537A70:("mov","ecx, dword ptr [ebx + 4]"),0x537AC4:("mov","edx, dword ptr [ebx + 4]"),
        0x537AF7:("mov","edx, dword ptr [ebx + 0x14]"),0x537B00:("call","0x592600"),0x537B05:("mov","ecx, dword ptr [ebx + 4]"),
        0x537AE7:("lea","edi, [ebx + 0x20]"),0x537B4C:("lea","edi, [ebx + 0x20]"),
        0x537B5C:("mov","ecx, dword ptr [ebx + 0x14]"),0x537B66:("call","0x592550"),
        0x537BB9:("mov","edx, dword ptr [ebx + 0x14]"),0x537BC1:("call","0x5926f0"),
        0x537D28:("mov","ecx, dword ptr [ebx + 0x14]"),0x537D4B:("call","0x592ee0"),
        0x537F4D:("mov","eax, dword ptr [esi + edi*4 + 0x1b80]"),0x537F63:("mov","dword ptr [esi + edi*4 + 0x1b80], 0"),
        0x5367B9:("mov","eax, dword ptr [0x93e8c0]"),0x536893:("mov","dword ptr [0x93e904], 2"),
        0x592F32:("push","0x20"),0x592F3D:("call","0x532940"),
        0x593179:("push","0x40"),0x593181:("call","0x532940"),
    }
    for address,expected in pins.items():
        i=decoded[address];assert (i.mnemonic,i.op_str.lower())==expected,hex(address)
    kill=[i for address,i in decoded.items() if 0x537BA5<=address<0x537BD5]
    assert [i.op_str for i in kill if i.mnemonic=="call"]==["0x6262f0","0x5926f0"]
    assert not any("[ebx + 0x20]" in i.op_str for i in kill)
    erase=[i for address,i in decoded.items() if 0x516490<=address<0x5165A6]
    calls=[i for i in erase if i.mnemonic=="call" and i.op_str=="0x592ee0"]
    assert len(calls)==1
    assert any(i.mnemonic=="push" and i.op_str=="0x2e02" for i in erase)
    # These are scalar branches, not a guessed jump table or contiguous owner.
    # Host controls model valid allocated objects and callback observations;
    # native short-packet, out-of-owner and asynchronous lifetime effects remain unknown.
