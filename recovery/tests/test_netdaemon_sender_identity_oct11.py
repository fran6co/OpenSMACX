"""Packet faction is not the transport sender ID in transfer interception."""
from pathlib import Path
import shutil
import subprocess

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
    return source


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


def test_only_two_reviewed_transfer_expressions_and_band_comment_changed():
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
