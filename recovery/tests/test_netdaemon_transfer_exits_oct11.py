"""Actual shared-tail predicate and transfer normal/abort lifetime controls."""
from pathlib import Path
import shutil
import subprocess

import pytest

from test_netdaemon_transfer_modal import extract

ROOT = Path(__file__).resolve().parents[2]
SOURCE = ROOT / "src/netdaemon.cpp"
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
    return source


def test_fixed_exit_recipe_preserves_all_other_source_and_identities():
    old = subprocess.check_output(["git", "show", BASE + ":src/netdaemon.cpp"], cwd=ROOT, text=True)
    assert apply_transfer_exit_migration(old) == SOURCE.read_text()
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
    path = ROOT / ".opensmacx/game/terranx_original.exe"
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
