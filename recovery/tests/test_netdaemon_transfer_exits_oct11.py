"""Actual shared-tail predicate and transfer normal/abort lifetime controls."""
from pathlib import Path
import shutil
import subprocess
import os

import pytest

from test_netdaemon_transfer_modal import extract
from dataclasses import fields

from match_recovery.source import read_text

ROOT = Path(__file__).resolve().parents[2]
SOURCE = ROOT / "src/netdaemon.cpp"

DEFAULT_CONTROL = r'''
#include <cassert>
#include <stdint.h>
#include <cstring>
static uint32_t packet[16];
static int clock_calls,fixup_calls,send_calls,reject,events;
struct Sequence { uint32_t value; uint32_t advance_sequence() { assert(events==2);events=3;return ++value; } };
struct Words { uint32_t leading_words_[4]; };
struct NetDaemon {
    Sequence deletion_list_; Words channels_[2];
    void route(char *,unsigned long);
    int fixup_message(char *) {
        ++fixup_calls;assert(events==1 && packet[2]==0xF1234567u);events=2;
        assert(uint16_t(packet[0])==0x1234);packet[0]=(packet[0]&0xFFFF0000u)|0xBEEF;
        deletion_list_.value=0xFFFFFFFFu; return reject;
    }
    void send_message(char *bytes,unsigned long length,int target) {
        assert(bytes==reinterpret_cast<char *>(packet) && length==64 && target==0);
        assert(events==3 && uint16_t(packet[0])==0x5EEF && packet[3]==0 && deletion_list_.value==0);
        assert((packet[0]&0xFFFF0000u)==0xCDEF0000u);
        events=4;++send_calls;
    }
};
static struct { int faction_id_; } ConsoleGlobal;
static uint32_t timeGetTime() {
    ++clock_calls;assert(events==0);events=1;
    // Clock/fixup mutation is an observation double, not proof that the real
    // system clock reenters. Selected flags precede clock; opcode reload is later.
    packet[0]=(packet[0]&0xFFFF0000u)|0x1234;return 0xF1234567u;
}
@@ACTUAL_SOURCE@@
int main() {
    const unsigned short types[]={0xF09,0x440A,0xFFFF,0xA409,0x8400,0x1234};
    for(unsigned i=0;i<sizeof(types)/sizeof(*types);++i) for(reject=0;reject<2;++reject) {
        NetDaemon daemon;std::memset(&daemon,0,sizeof daemon);
        daemon.deletion_list_.value=7;daemon.channels_[1].leading_words_[1]=0x1EEF;
        std::memset(packet,0,sizeof packet);packet[0]=0xCDEF0000u|types[i];packet[1]=3;packet[2]=0x1111;packet[3]=0x2222;
        ConsoleGlobal.faction_id_=3;clock_calls=fixup_calls=send_calls=events=0;
        daemon.route(reinterpret_cast<char *>(packet),64);
        const bool selected=(types[i]&0xA000)!=0;
        assert(clock_calls==int(selected) && fixup_calls==int(selected));
        assert(send_calls==int(selected&&!reject));
        if(!selected) {
            assert(packet[0]==(0xCDEF0000u|types[i]) && packet[2]==0x1111 && packet[3]==0x2222 && daemon.deletion_list_.value==7);
        } else if(reject) {
            assert(uint16_t(packet[0])==0xBEEF && packet[2]==0xF1234567u && packet[3]==0x2222 && daemon.deletion_list_.value==0xFFFFFFFFu);
        }
        assert(daemon.channels_[1].leading_words_[1]==uint32_t(selected&&!reject?0:0x1EEF));
    }
}
'''


@pytest.mark.parametrize("compiler", [c for c in ("g++", "clang++") if shutil.which(c)])
def test_actual_shared_default_clock_fixup_reload_and_marker(tmp_path, compiler):
    source=SOURCE.read_text()
    body=extract(source,"static void netdaemon_forward_default_message(")+"\n"+extract(source,"static void netdaemon_process_message_marker_tail(")
    process=extract(source,"void NetDaemon::process_message(")
    footer=process[process.index("    // Default path,"):process.rfind("}")]
    body+="\nvoid NetDaemon::route(char *message,unsigned long message_size) {\nunsigned char *msg=reinterpret_cast<unsigned char *>(message);\n"+footer+"\n}"
    def run(candidate):
        cpp=tmp_path/"default.cpp";exe=tmp_path/"default"
        cpp.write_text(DEFAULT_CONTROL.replace("@@ACTUAL_SOURCE@@",candidate))
        built=subprocess.run([compiler,"-std=c++03","-Wall","-Wextra","-Werror","-Wno-unused-function","-fsanitize=address,undefined",str(cpp),"-o",str(exe)],capture_output=True,text=True,timeout=60)
        assert built.returncode==0,built.stderr
        return subprocess.run([str(exe)],capture_output=True,text=True,timeout=10,env=dict(os.environ,ASAN_OPTIONS="detect_leaks=0"))
    result=run(body);assert result.returncode==0,result.stderr
    for before,after in (
        ("memcpy(bytes + 8, &timestamp, sizeof timestamp);", "memcpy(bytes + 4, &timestamp, sizeof timestamp);"),
        ("memcpy(&opcode, bytes, sizeof opcode);", "opcode=0xA409;"),
        ("if (daemon.fixup_message(message) != 0) return;", "if (daemon.fixup_message(message) == 0) return;"),
        ("(opcode & 0x1FFFu) | 0x4000u", "(opcode & 0xFFFFu) | 0x4000u"),
        ("memcpy(bytes + 0x0C, &sequence, sizeof sequence);", "memcpy(bytes + 0x10, &sequence, sizeof sequence);"),
        ("netdaemon_forward_default_message(*this, message, message_size);", "(void)message_size;"),
    ):
        assert before in body;assert run(body.replace(before,after,1)).returncode!=0,before
    early=body.replace("    const uint32_t timestamp =", "    if (daemon.fixup_message(message) != 0) return;\n    const uint32_t timestamp =",1)
    early=early.replace("    if (daemon.fixup_message(message) != 0) return;\n    uint16_t opcode;", "    uint16_t opcode;",1)
    assert run(early).returncode!=0
    process=extract(source,"void NetDaemon::process_message(")
    transfer=extract(source,"bool NetDaemon::process_file_transfer_message(")
    assert "netdaemon_forward_default_message(*this, message, message_size);" in process
    assert "netdaemon_forward_default_message(*this, message, message_size);" in transfer
    assert "netdaemon_process_table_g(*this, msg, type,\n                                  entry_packet_faction, entry_sequence)" in process
BASE = "8fcb993ca"
EDITS = (
    ("    void arm() { armed_ = true; }",
     "    void arm() { armed_ = true; }\n    bool modal_abort() const { return armed_; }"),
    ("    if (type == 0xF08) { // decimal 3848: peer ready for one file\n        if (!local_is_host) return netdaemon_transfer_modal_cleanup(guard);",
     "    if (type == 0xF08) { // decimal 3848: peer ready for one file\n        if (!local_is_host) return true;"),
    ("        records = netdaemon_file_index_records();\n        if (records == 0) return true;",
     "        records = netdaemon_file_index_records();\n        if (records == 0) return netdaemon_transfer_modal_cleanup(guard);"),
    ("        if ((message[1] & 0x40u) != 0 && (type & 0x4000u) != 0 &&\n            static_cast<uint32_t>(type | 0x4000u) == pending &&",
     "        if ((message[1] & 0x40u) != 0 &&\n            static_cast<uint32_t>(type) == (pending | 0x4000u) &&"),
    ("        if (process_file_transfer_message(message, message_size, sender_id,\n                                          guard, filemap)) {\n            return;",
     "        if (process_file_transfer_message(message, message_size, sender_id,\n                                          guard, filemap)) {\n            // Native normal routes join 538266; modal aborts instead unwind\n            // through 534DCD. Keep the shared marker update out of that unwind.\n            if (!guard.modal_abort()) {\n                netdaemon_process_message_marker_tail(*this, msg);\n            }\n            return;"),
)


def apply_transfer_exit_migration(source):
    for before, after in EDITS:
        assert source.count(before) == 1, before
        source = source.replace(before, after, 1)
    from test_netdaemon_transfer_receive_oct11 import apply_transfer_receive_migration
    return apply_transfer_receive_migration(source)


def test_fixed_exit_recipe_preserves_all_other_source_and_identities():
    old = subprocess.check_output(["git", "show", BASE + ":src/netdaemon.cpp"], cwd=ROOT, text=True)
    migrated = apply_transfer_exit_migration(old)
    before_rows = {row.address: row for row in read_text(old, SOURCE)}
    after_rows = {row.address: row for row in read_text(migrated, SOURCE)}
    assert set(after_rows) == set(before_rows)
    for address, before in before_rows.items():
        after = after_rows[address]
        for item in fields(before):
            if item.name not in {"line", "region", "origin"}:
                assert getattr(after, item.name) == getattr(before, item.name)
        assert tuple((lesson.kind, lesson.key, lesson.prose,
                      tuple(line for _, line in lesson.lines)) for lesson in after.origin) == \
               tuple((lesson.kind, lesson.key, lesson.prose,
                      tuple(line for _, line in lesson.lines)) for lesson in before.origin)
    with pytest.raises(AssertionError):
        apply_transfer_exit_migration(old.replace("if (records == 0) return true;", "if (records == 0) return false;"))


@pytest.mark.parametrize("damage", ["none", "reverse_pending", "abort_tail", "skip_tail", "ignore_sender"])
def test_actual_normal_route_and_pending_marker_tail(tmp_path, damage):
    compiler = shutil.which("g++") or shutil.which("clang++")
    if compiler is None:
        pytest.skip("host C++ compiler unavailable")
    source = SOURCE.read_text()
    tail = extract(source, "static void netdaemon_process_message_marker_tail(")
    process = extract(source, "void NetDaemon::process_message(")
    start = process.index("    if (type >= 0xF05 && type <= 0xF0F)")
    stop = process.index("\n\n    if (netdaemon_handle_diplo_cd_message", start)
    route = process[start:stop]
    replacements = {
        "reverse_pending": ("static_cast<uint32_t>(type) == (pending | 0x4000u)", "static_cast<uint32_t>(type | 0x4000u) == pending"),
        "abort_tail": ("if (!guard.modal_abort())", "if (true)"),
        "skip_tail": ("if (!guard.modal_abort())", "if (false)"),
        "ignore_sender": ("static_cast<uint32_t>(ConsoleGlobal.faction_id_)", "*reinterpret_cast<uint32_t *>(message + 4)"),
    }
    body = tail + "\nvoid NetDaemon::route(unsigned short type, unsigned char *msg, bool aborted) {\nchar *message=reinterpret_cast<char *>(msg); unsigned long message_size=32; int sender_id=9; Guard guard={aborted}; int filemap=0;\n" + route + "\n}"
    if damage != "none":
        before, after = replacements[damage]
        assert body.count(before) == 1
        body = body.replace(before, after, 1)
    preamble = r'''
#include <cassert>
#include <stdint.h>
#include <cstring>
struct Guard { bool aborted; bool modal_abort() const { return aborted; } };
struct Words { uint32_t leading_words_[4]; };
struct NetDaemon {
    Words channels_[2];
    bool process_file_transfer_message(char *,unsigned long,int,Guard &,int &) { return true; }
    void route(unsigned short,unsigned char *,bool);
};
static struct { int faction_id_; } ConsoleGlobal;
'''
    main = r'''
int main() {
    NetDaemon daemon; ConsoleGlobal.faction_id_=7;
    uint32_t packet[8]={0}; packet[1]=7;
    unsigned char *bytes=reinterpret_cast<unsigned char *>(packet);
    const uint32_t pending_values[]={0,0x1F04,0x5F04,0xFFFF,0x10000,0x80001F04};
    const unsigned short types[]={0x1F04,0x5F04,0x5F05,0xFFFF};
    for (unsigned p=0;p<sizeof(pending_values)/sizeof(*pending_values);++p)
        for (unsigned t=0;t<sizeof(types)/sizeof(*types);++t)
            for (int sender=6;sender<=7;++sender) {
                const uint32_t pending=pending_values[p];
                daemon.channels_[1].leading_words_[1]=pending;
                std::memcpy(bytes,&types[t],2); packet[1]=sender;
                netdaemon_process_message_marker_tail(daemon,bytes);
                const bool clears=pending!=0 && (types[t]&0x4000)!=0 &&
                    uint32_t(types[t])==(pending|0x4000) && sender==7;
                assert(daemon.channels_[1].leading_words_[1]==(clears?0:pending));
            }
    // Qualify the real caller branch after a handler mutates the packet.
    const unsigned short marked=0x5F04; std::memcpy(bytes,&marked,2); packet[1]=7;
    for (unsigned short opcode=0xF05;opcode<=0xF0F;++opcode) {
        daemon.channels_[1].leading_words_[1]=0x1F04;
        daemon.route(opcode,bytes,false);
        assert(daemon.channels_[1].leading_words_[1]==0);
        daemon.channels_[1].leading_words_[1]=0x1F04;
        daemon.route(opcode,bytes,true);
        assert(daemon.channels_[1].leading_words_[1]==0x1F04);
    }
}
'''
    cpp = tmp_path / "exits.cpp"
    exe = tmp_path / "exits"
    cpp.write_text(preamble + body + main)
    subprocess.run([compiler, "-std=c++03", str(cpp), "-o", str(exe)], check=True, capture_output=True, timeout=60)
    result = subprocess.run([str(exe)], capture_output=True, timeout=10)
    assert (result.returncode == 0) == (damage == "none"), result.stderr


def test_native_transfer_band_bias_normal_client_exit_and_pending_relation():
    from capstone import Cs, CS_ARCH_X86, CS_MODE_32
    from match_recovery.model import Span
    from match_recovery.pe_coff import PeImage
    path = Path(os.environ.get("OPENSMACX_IMAGE", ROOT / ".opensmacx/game/terranx_original.exe"))
    if not path.is_file():
        pytest.skip("pinned original unavailable")
    image = PeImage(path.read_bytes(), "01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf")
    decoder = Cs(CS_ARCH_X86, CS_MODE_32)
    bands = [(0x53456F,0x5345A2),(0x534AE7,0x534B43),(0x538266,0x53829F)]
    instructions = {i.address:(i.mnemonic,i.op_str) for start,end in bands
                    for i in decoder.disasm(image.read(Span(start,end)),start)}
    assert instructions[0x53458D] == ("add", "eax, 0xfffff0fb")
    # Operand/order assertions are tied to actual decoded native instructions.
    assert ("or", "ch, 0x40") in instructions.values()
    assert ("cmp", "eax, ecx") in instructions.values() or ("cmp", "ax, cx") in instructions.values()
    assert ("je", "0x538266") in instructions.values()
    assert ("je", "0x534d21") in instructions.values()
