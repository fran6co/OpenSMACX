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
