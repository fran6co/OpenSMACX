"""Packet faction is not the transport sender ID in transfer interception."""
from pathlib import Path
import shutil
import subprocess
import re

import pytest

from test_caviar_draw_callbacks_actual_source import extract

ROOT = Path(__file__).resolve().parents[2]
SOURCE = ROOT / "src/netdaemon.cpp"
BASE = "6a523c5e6"
TRANSFER_EDITS = (
    ('    // Special-cased "file transfer" message-type band (0x1500-0x15FF).',
     '    // Masked file-transfer response band; 0x1501 bypasses interception.\n'
     '    // Its commlink comparison and reply target use the captured packet faction.'),
    ("CommlinkFaction == sender_id", "CommlinkFaction == entry_packet_faction"),
    ("message_data(0x1505, sender_id, 0, 0, 0, 0)",
     "message_data(0x1505, entry_packet_faction, 0, 0, 0, 0)"),
)


def apply_transfer_faction_migration(source):
    for before, after in TRANSFER_EDITS:
        assert source.count(before) == 1
        source = source.replace(before, after, 1)
    return apply_entry_identity_migration(source)


IDENTITY_EDITS = (
    ("                                                 int sender_id = 0) {", "                                                 int sender_identity) {", 1),
    ("                daemon->AlphaNet::pid_2_who(static_cast<uint32_t>(sender_id)),", "                sender_identity,", 1),
    ("// dispatcher converts it through the same AlphaNet lookup as the native EDI.", "// dispatcher supplies its entry-captured identity, retained in native EDI.", 1),
    ("                                                 int sender_id) {\n    const int sender = daemon->AlphaNet::pid_2_who(\n        static_cast<uint32_t>(sender_id));", "                                                 int sender_identity) {\n    const int sender = sender_identity;", 1),
    ("                                                int sender_id) {\n    const int sender = daemon->AlphaNet::pid_2_who(\n        static_cast<uint32_t>(sender_id));", "                                                int sender_identity) {\n    const int sender = sender_identity;", 1),
    ("    Filemap filemap;\n    // The acknowledged-sync request", "    Filemap filemap;\n    // Pinned53444E/534456 resolves the transport ID once, before packet reads\n    // and dispatch callbacks. Keep this identity distinct from packet faction.\n    const int entry_sender_identity =\n        this->AlphaNet::pid_2_who(static_cast<unsigned long>(sender_id));\n    // The acknowledged-sync request", 1),
    ("netdaemon_handle_server_lock_status(this, type, msg, sender_id)", "netdaemon_handle_server_lock_status(this, type, msg, entry_sender_identity)", 1),
    ("netdaemon_handle_withdraw_end_turn(this, msg, sender_id)", "netdaemon_handle_withdraw_end_turn(this, msg, entry_sender_identity)", 1),
    ("netdaemon_handle_client_done_upkeep(this, msg, sender_id)", "netdaemon_handle_client_done_upkeep(this, msg, entry_sender_identity)", 1),
)


def apply_entry_identity_migration(source):
    # Replace only the six original late call arguments, before adding capture.
    late = "this->AlphaNet::pid_2_who(static_cast<unsigned long>(sender_id))"
    assert source.count(late) == 6
    source = source.replace(late, "entry_sender_identity")
    for before, after, count in IDENTITY_EDITS:
        assert source.count(before) == count, before
        source = source.replace(before, after, count)
    return apply_lock_response_migration(source)


def apply_lock_response_migration(source):
    before = "message_data(0x1201 +\n                         static_cast<int>(daemon->locks_.field_E0_ != 0)"
    after = "message_data(0x1202 +\n                         static_cast<int>(daemon->locks_.field_E0_ != 0)"
    assert source.count(before) == 2
    source = source.replace(before, after)
    before = "                         sender, 0, 0, 0, 0);\n        }\n        daemon->locks_.check_global();"
    after = "                         sender, 0, 0, 0, 0);\n            return true;\n        }\n        message_data(0x1201, sender, 0, 0, 0, 0);\n        daemon->locks_.check_global();"
    assert source.count(before) == 1
    return apply_lock_operand_migration(source.replace(before, after, 1))


def apply_lock_operand_migration(source):
    before = "            field_10, field_14,\n            *reinterpret_cast<int *>(message + 0x1C),\n            *reinterpret_cast<int *>(message + 0x24),"
    after = "            *reinterpret_cast<int *>(message + 0x10),\n            *reinterpret_cast<int *>(message + 0x14),\n            *reinterpret_cast<int *>(message + 0x24),\n            *reinterpret_cast<int *>(message + 0x1C),"
    assert source.count(before) == 1
    return source.replace(before, after, 1)


def product():
    body = extract(SOURCE.read_text(), "void NetDaemon::process_message(")
    start = body.index("    bool skip500 =")
    end = body.index("    // File-transfer opcodes", start)
    return body[start:end]


PREAMBLE = r'''
#include <cassert>
#include <cstring>
static int CommlinkPending, CommlinkFaction, send_count, send_target;
static bool change_type_on_send;
static char *live_packet;
struct Guard { int closed; };
static void netdaemon_transfer_modal_cleanup(Guard &guard) { ++guard.closed; }
static void message_data(int opcode,int target,int a,int b,int c,int d) {
    assert(opcode==0x1505 && a==0 && b==0 && c==0 && d==0);
    ++send_count; send_target=target;
    if (change_type_on_send) {
        unsigned short next=0x4600;
        std::memcpy(live_packet,&next,sizeof next);
    }
}
static void transfer(unsigned short &type, int sender_id, int entry_packet_faction,
                     char *message, Guard &guard) {
'''
MAIN = r'''
}
static void run(unsigned short opcode,int pending,int commlink,int expected_send,
                int expected_close,bool mutate=false) {
    alignas(4) char message[16]={0};
    std::memcpy(message,&opcode,sizeof opcode);
    live_packet=message; CommlinkPending=pending; CommlinkFaction=commlink;
    send_count=0; send_target=-1; change_type_on_send=mutate;
    Guard guard={0}; unsigned short type=opcode;
    // Keep the transport ID deliberately distinct from packet faction.
    transfer(type,10001,3,message,guard);
    assert(send_count==expected_send && guard.closed==expected_close);
    if (expected_send) assert(send_target==3);
    if (expected_close==0) assert(type==(mutate && expected_send ? 0x4600 : opcode));
}
int main() {
    run(0x1502,0,0,1,0); // Ordinary interception replies to the packet faction.
    run(0x1502,1,3,0,0); // Matching commlink faction bypasses interception.
    run(0x1502,1,10001,1,0); // Matching only the transport ID is not sufficient.
    run(0x1505,0,0,0,1); // Cancel closes and returns, without reply.
    run(0x1505,1,3,0,0); // Matching commlink bypass precedes cancel.
    run(0x1501,0,0,0,0); // Original special-case bypass.
    run(0x1602,0,0,0,0); // Non-band packet bypass.
    run(0x0502,0,0,1,0); // Masked band, not a narrowed 1500-only test.
    run(0x1502,0,0,1,0,true); // Reload the opcode after the external send.
}
'''


@pytest.mark.parametrize("damage", ["none", "comparison", "target", "bypass", "type_reload"])
def test_actual_transfer_guard_uses_packet_faction(tmp_path, damage):
    compiler = shutil.which("g++") or shutil.which("clang++")
    if not compiler:
        pytest.skip("host C++ compiler unavailable")
    body = product()
    changes = {
        "comparison": ("CommlinkFaction == entry_packet_faction", "CommlinkFaction == sender_id"),
        "target": ("message_data(0x1505, entry_packet_faction", "message_data(0x1505, sender_id"),
        "bypass": ("(type == 0x1501)", "(type == 0x1502)"),
        "type_reload": ("type = *reinterpret_cast<unsigned short *>(message);", "(void)message;"),
    }
    if damage != "none":
        before, after = changes[damage]
        assert body.count(before)==1
        body = body.replace(before,after,1)
    source = tmp_path / "transfer.cpp"
    executable = tmp_path / "transfer"
    source.write_text(PREAMBLE + body + MAIN)
    subprocess.run([compiler,"-std=c++11","-O1",str(source),"-o",str(executable)],
                   check=True,capture_output=True,text=True,timeout=60)
    result = subprocess.run([str(executable)],capture_output=True,timeout=10)
    assert (result.returncode == 0) == (damage == "none"), result.stderr


def test_only_reviewed_transfer_and_entry_identity_migration_changed():
    old = subprocess.check_output(["git","show",BASE+":src/netdaemon.cpp"],cwd=ROOT,text=True)
    assert apply_transfer_faction_migration(old) == SOURCE.read_text()
    with pytest.raises(AssertionError):
        apply_transfer_faction_migration(old.replace("message_data(0x1505, sender_id", "message_data(0x1506, sender_id"))


def test_pinned_transfer_guard_and_reply_faction_anchors():
    from capstone import Cs, CS_ARCH_X86, CS_MODE_32
    from match_recovery.pe_coff import PeImage
    from match_recovery.model import Span
    path = ROOT / ".opensmacx/game/terranx_original.exe"
    if not path.is_file():
        pytest.skip("pinned original unavailable")
    image = PeImage(path.read_bytes(),"01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf")
    cs = Cs(CS_ARCH_X86,CS_MODE_32)
    instructions={}
    for start,end in ((0x534453,0x5344A1),(0x534556,0x534575)):
        instructions.update({i.address:(i.mnemonic,i.op_str)
            for i in cs.disasm(image.read(Span(start,end)),start)})
    expected = {
        0x53445E:("mov","edx, dword ptr [ebx + 4]"),
        0x534466:("mov","dword ptr [ebp + 8], edx"),
        0x53448F:("cmp","dword ptr [0x73b790], edx"),
        0x534495:("je","0x53456f"),
        0x534556:("mov","edx, dword ptr [ebp + 8]"),
        0x534561:("push","edx"),
        0x534562:("push","0x1505"),
        0x534567:("call","0x592ee0"),
        0x53456F:("mov","cx, word ptr [ebx]"),
    }
    for address,instruction in expected.items():
        assert instructions[address]==instruction


def test_pinned_sender_lookup_is_entry_owned_and_retained_by_routes():
    """Evidence for the next migration; this does not certify current timing."""
    from capstone import Cs, CS_ARCH_X86, CS_MODE_32
    from match_recovery.pe_coff import PeImage
    from match_recovery.model import Span
    path = ROOT / ".opensmacx/game/terranx_original.exe"
    if not path.is_file():
        pytest.skip("pinned original unavailable")
    image = PeImage(path.read_bytes(), "01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf")
    instructions = list(Cs(CS_ARCH_X86, CS_MODE_32).disasm(
        image.read(Span(0x534400, 0x5388B9)), 0x534400))
    assert instructions[-1].address + instructions[-1].size == 0x5388B9
    lookups = [i.address for i in instructions
               if i.mnemonic == "call" and i.op_str == "0x4e2610"]
    assert lookups == [0x53444E]
    by_address = {i.address: (i.mnemonic, i.op_str) for i in instructions}
    expected = {
        0x534444: ("mov", "eax, dword ptr [ebp + 0x10]"),
        0x534449: ("push", "eax"),
        0x534456: ("mov", "edi, eax"),
        # LOCK logs and compares the captured identity, not packet faction.
        0x535BE3: ("push", "edi"),
        0x535C03: ("cmp", "edi, dword ptr [0x939284]"),
        # DONE UPKEEP uses it after the phase/turn diagnostic callbacks.
        0x536072: ("mov", "ecx, edi"),
        # WITHDRAW END-OF-TURN retains it across logging as well.
        0x536121: ("push", "edi"),
        0x536146: ("mov", "ecx, edi"),
    }
    for address, instruction in expected.items():
        assert by_address[address] == instruction


@pytest.mark.parametrize("damage", ["none", "late_lookup", "transport_id", "packet_faction"])
def test_actual_entry_capture_survives_reentrant_identity_change(tmp_path, damage):
    compiler = shutil.which("g++") or shutil.which("clang++")
    if not compiler:
        pytest.skip("host C++ compiler unavailable")
    body = extract(SOURCE.read_text(), "void NetDaemon::process_message(")
    start = body.index("    const int entry_sender_identity =")
    end = body.index(";", start) + 1
    capture = body[start:end]
    call = re.search(r"netdaemon_handle_client_lock_message\(this,.*?\);", body, re.S).group()
    if damage != "none":
        value = {"late_lookup": "this->AlphaNet::pid_2_who(static_cast<unsigned long>(sender_id))",
                 "transport_id": "sender_id", "packet_faction": "3"}[damage]
        assert call.count("entry_sender_identity") == 1
        call = call.replace("entry_sender_identity", value)
    harness = r'''
#include <cassert>
struct AlphaNet {
    int identity=2, lookups=0;
    int pid_2_who(unsigned long id) { assert(id==10001); ++lookups; return identity; }
};
struct NetDaemon : AlphaNet { void run(); };
static int received=-1;
static void netdaemon_handle_client_lock_message(NetDaemon *, unsigned short type,
                                                unsigned char *, int identity) {
    assert(type==0x2210); received=identity;
}
void NetDaemon::run() {
    int sender_id=10001;
    unsigned short type=0x2210;
    unsigned char packet[32]={}; unsigned char *msg=packet;
'''
    harness += capture + "\n    identity=7; // A dispatch callback changes the live process table.\n" + call
    harness += "\n}\nint main() { NetDaemon daemon; daemon.run(); assert(received==2 && daemon.lookups==1); }\n"
    source = tmp_path / "entry.cpp"
    binary = tmp_path / "entry"
    source.write_text(harness)
    subprocess.run([compiler, "-std=c++11", str(source), "-o", str(binary)],
                   check=True, capture_output=True, timeout=60)
    result = subprocess.run([str(binary)], capture_output=True, timeout=10)
    assert (result.returncode == 0) == (damage == "none")


def test_identity_propagation_does_not_replace_transport_consumers():
    source = SOURCE.read_text()
    body = extract(source, "void NetDaemon::process_message(")
    assert body.count("pid_2_who(") == 1
    assert body.index("Filemap filemap;") < body.index("const int entry_sender_identity =") < body.index("unsigned char *msg =")
    for helper in ("netdaemon_handle_server_lock_status", "netdaemon_handle_withdraw_end_turn",
                   "netdaemon_handle_client_done_upkeep"):
        assert "pid_2_who" not in extract(source, "static void " + helper + "(")
    assert "netdaemon_handle_table_f(this, type, msg, sender_id)" in body
    assert "*this, type, sender_id,\n            entry_sender_identity" in body
    assert "*this, msg, type, sender_id,\n            entry_sender_identity" in body
    assert "*this, msg, type, sender_id))" in body
