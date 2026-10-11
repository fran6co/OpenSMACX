"""Pinned council-route logging and callback-visible packet reads."""
from pathlib import Path
import shutil
import subprocess

import pytest

from test_caviar_draw_callbacks_actual_source import extract

ROOT = Path(__file__).resolve().parents[2]
SOURCE = ROOT / "src/netdaemon.cpp"

COUNCIL_BEFORE = "static bool process_council_message(unsigned char *msg,\n                                    unsigned short type) {\n    // The original uses direct x86 dword reads from these wire offsets; keep\n    // that unchecked access/extent behavior instead of adding packet guards.\n    const int *const words = reinterpret_cast<const int *>(msg);\n    switch (type) {\n    case 0x4600:\n        log_say(\"Received CONVENE COUNCIL\", words[4], words[5], words[6]);\n        my_srand(words[2]);\n        council(words[4], words[5], words[6]);\n        return true;\n    case 0x4601:\n        log_say(\"Received COUNCIL VOTE\", words[4], 0, 0);\n        my_srand(words[2]);\n        CouncilWindow.set_vote(words[4], words[5]);\n        return true;\n    case 0x4602:\n        log_say(\"Received COUNCIL ARRIVE\", 0, 0, 0);\n        CouncilWindow.arrived(words[4]);\n        return true;\n    case 0x4603:\n        log_say(\"Received COUNCIL RESOLUTION\", 0, 0, 0);\n        CouncilWindow.make_resolution(1);\n        return true;\n    default:\n        return false;\n    }\n}"
COUNCIL_AFTER = "static bool process_council_message(unsigned char *msg,\n                                    unsigned short type) {\n    // Share sender-owned wire offsets without assuming packet alignment.\n    // Original logs and actions read independently across external calls.\n    const MessageCommandView packet(msg);\n    switch (type) {\n    case 0x4600:\n        log_say(\"Received CONVENE COUNCIL\", packet.payload_10(), packet.payload_14(), packet.payload_18());\n        my_srand(packet.timestamp_bits());\n        council(packet.payload_10(), packet.payload_14(), packet.payload_18());\n        return true;\n    case 0x4601:\n        log_say(\"Received COUNCIL VOTE\", packet.payload_10(), packet.payload_14(), 0);\n        my_srand(packet.timestamp_bits());\n        CouncilWindow.set_vote(packet.payload_10(), packet.payload_14());\n        return true;\n    case 0x4602:\n        log_say(\"Received COUNCIL ARRIVE\", packet.payload_10(), 0, 0);\n        CouncilWindow.arrived(packet.payload_10());\n        return true;\n    case 0x4603:\n        log_say(\"Received COUNCIL RESOLUTION\", 0, 0, 0);\n        CouncilWindow.make_resolution(1);\n        return true;\n    default:\n        return false;\n    }\n}"


def apply_council_wire_migration(source):
    assert source.count(COUNCIL_BEFORE) == 1
    return source.replace(COUNCIL_BEFORE, COUNCIL_AFTER, 1)


def product():
    text = SOURCE.read_text()
    start = text.index("struct MessageDataRecord {")
    end = text.index("// ORIGINAL:", start)
    return (text[start:end] + extract(text, "class MessageCommandView {") + ";\n" +
            extract(text, "static bool process_council_message("))


PREAMBLE = r'''
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <string>
#include <vector>
static unsigned char *packet;
static std::vector<std::string> events;
static int logged[3], args[3], seed, calls;
static bool mutate_log, mutate_rng;
static void put(int offset,int value) { std::memcpy(packet+offset,&value,4); }
static void log_say(const char *label,int a,int b,int c) {
    logged[0]=a; logged[1]=b; logged[2]=c; events.push_back(label);
    if (mutate_log) { put(8,234); put(0x10,9); put(0x14,29); put(0x18,39); }
}
static void my_srand(int value) {
    seed=value; events.push_back("seed");
    if (mutate_rng) { put(0x10,2); put(0x14,21); put(0x18,31); }
}
static void council(int a,int b,int c) {
    ++calls; args[0]=a; args[1]=b; args[2]=c; events.push_back("council");
}
struct Council {
    void set_vote(int a,int b) {
        ++calls; args[0]=a; args[1]=b; events.push_back("vote");
    }
    void arrived(int a) { ++calls; args[0]=a; events.push_back("arrive"); }
    void make_resolution(int a) { ++calls; args[0]=a; events.push_back("resolution"); }
} CouncilWindow;
'''
MAIN = r'''
static void reset() {
    std::memset(packet,0,32); put(8,123); put(0x10,1); put(0x14,11); put(0x18,13);
    events.clear(); seed=-1; calls=0; mutate_log=mutate_rng=false;
}
int main() {
    alignas(4) unsigned char storage[36]; packet=storage+1;
    reset(); mutate_log=mutate_rng=true;
    assert(process_council_message(packet,0x4600));
    assert(logged[0]==1 && logged[1]==11 && logged[2]==13);
    assert(seed==234 && args[0]==2 && args[1]==21 && args[2]==31 && calls==1);
    assert(events.size()==3 && events[1]=="seed" && events[2]=="council");
    reset(); mutate_log=mutate_rng=true;
    assert(process_council_message(packet,0x4601));
    assert(logged[0]==1 && logged[1]==11 && logged[2]==0);
    assert(seed==234 && args[0]==2 && args[1]==21 && calls==1);
    assert(events.size()==3 && events[1]=="seed" && events[2]=="vote");
    reset(); mutate_log=true;
    assert(process_council_message(packet,0x4602));
    assert(logged[0]==1 && logged[1]==0 && logged[2]==0);
    assert(seed==-1 && args[0]==9 && calls==1 && events.size()==2);
    reset(); mutate_log=mutate_rng=true;
    assert(process_council_message(packet,0x4603));
    assert(logged[0]==0 && logged[1]==0 && logged[2]==0);
    assert(seed==-1 && args[0]==1 && calls==1 && events.size()==2);
    reset(); assert(!process_council_message(packet,0x4999));
    assert(events.empty() && calls==0);
}
'''


@pytest.mark.parametrize("damage", ["none", "vote_log", "arrival_log", "vote_action", "arrival_action", "seed"])
def test_actual_council_logging_and_packet_reloads(tmp_path, damage):
    compiler = shutil.which("g++") or shutil.which("clang++")
    if not compiler:
        pytest.skip("host C++ compiler unavailable")
    body = product()
    changes = {
        "vote_log": ('log_say("Received COUNCIL VOTE", packet.payload_10(), packet.payload_14(), 0)',
                     'log_say("Received COUNCIL VOTE", packet.payload_10(), 0, 0)'),
        "arrival_log": ('log_say("Received COUNCIL ARRIVE", packet.payload_10(), 0, 0)',
                       'log_say("Received COUNCIL ARRIVE", 0, 0, 0)'),
        "vote_action": ("CouncilWindow.set_vote(packet.payload_10(), packet.payload_14())",
                        "CouncilWindow.set_vote(1, 11)"),
        "arrival_action": ("CouncilWindow.arrived(packet.payload_10())", "CouncilWindow.arrived(1)"),
        "seed": ("my_srand(packet.timestamp_bits())", "my_srand(123)"),
    }
    if damage != "none":
        before, after = changes[damage]
        assert before in body
        body = body.replace(before, after)
    source = tmp_path / "council.cpp"
    binary = tmp_path / "council"
    source.write_text(PREAMBLE + body + MAIN)
    subprocess.run([compiler,"-std=c++11","-O1",str(source),"-o",str(binary)],
                   check=True,capture_output=True,text=True,timeout=60)
    result = subprocess.run([str(binary)],capture_output=True,timeout=10)
    assert (result.returncode == 0) == (damage == "none"), result.stderr


def test_literal_council_recipe_refuses_unrelated_edits():
    old = subprocess.check_output(["git","show","5740a46a7:src/netdaemon.cpp"],cwd=ROOT,text=True)
    assert apply_council_wire_migration(old) == SOURCE.read_text()
    with pytest.raises(AssertionError):
        apply_council_wire_migration(old.replace("CouncilWindow.make_resolution(1)", "CouncilWindow.make_resolution(0)"))


def test_pinned_council_log_operands_and_reload_anchors():
    from capstone import Cs, CS_ARCH_X86, CS_MODE_32
    from match_recovery.pe_coff import PeImage
    from match_recovery.model import Span
    path = ROOT / ".opensmacx/game/terranx_original.exe"
    if not path.is_file():
        pytest.skip("pinned original unavailable")
    image = PeImage(path.read_bytes(), "01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf")
    cs = Cs(CS_ARCH_X86,CS_MODE_32)
    instructions = {i.address:(i.mnemonic,i.op_str) for i in cs.disasm(
        image.read(Span(0x5373AF,0x5374A7)),0x5373AF)}
    expected = {
        0x5373CE:("mov","ecx, dword ptr [ebx + 0x18]"),
        0x5373D1:("mov","edx, dword ptr [ebx + 0x14]"),
        0x5373D4:("mov","eax, dword ptr [ebx + 0x10]"),
        0x537424:("mov","ecx, dword ptr [ebx + 0x10]"),
        0x53742B:("push","ecx"),
        0x537431:("call","0x6262f0"),
        0x537436:("mov","edx, dword ptr [ebx + 0x10]"),
        0x537442:("call","0x4271c0"),
        0x53744C:("mov","eax, dword ptr [ebx + 0x14]"),
        0x53744F:("mov","ecx, dword ptr [ebx + 0x10]"),
        0x537454:("push","eax"),
        0x537455:("push","ecx"),
        0x53745B:("call","0x6262f0"),
        0x537460:("mov","edx, dword ptr [ebx + 8]"),
        0x537469:("mov","eax, dword ptr [ebx + 0x14]"),
        0x53746C:("mov","ecx, dword ptr [ebx + 0x10]"),
        0x537479:("call","0x424720"),
        0x53749D:("call","0x424410"),
    }
    for address, instruction in expected.items():
        assert instructions[address] == instruction
