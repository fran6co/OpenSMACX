"""Host controls for the actual NetDaemon 0x2210/0x2211 lock helper."""

from pathlib import Path
import re
import shutil
import subprocess
import tempfile


def repository_root():
    for candidate in Path(__file__).resolve().parents:
        if (candidate / "src/netdaemon.cpp").is_file():
            return candidate
    raise AssertionError("could not locate OpenSMACX source root")


ROOT = repository_root()
SOURCE = ROOT / "src/netdaemon.cpp"
HELPER = "netdaemon_handle_client_lock_message"

HOST_PREAMBLE = r"""
#include <cstddef>
#include <cassert>
#include <cstdint>
#include <cstring>
#include <string>
#include <vector>

static int SpockHelpActive;
static int TurnControlWord93A954;
static int ConsoleControlTurnActive;
struct Console { int faction_id_; } ConsoleGlobal;
static std::vector<std::string> events;
static int logged_sender = -1;
static int logged_x=-1, logged_y=-1;
static unsigned char *mutated_packet;
static int mutate_lock_byte18_on_lock_ok;
static int sent_type = -1, sent_target = -1, sends;
static void log_say(const char *label, int a, int b, int c) {
    events.push_back(std::string("log:") + label);
    if (std::string(label) == "From Client: LOCK" ||
        std::string(label) == "From Client: LOCK GLOBAL") logged_sender = a;
    if (std::string(label)=="From Client: LOCK") {
        logged_x=b; logged_y=c;
        if (mutated_packet) {
            const int x=101, y=102;
            std::memcpy(mutated_packet+0x10,&x,4);
            std::memcpy(mutated_packet+0x14,&y,4);
        }
    }
    if (std::string(label)=="(Sending: Lock OK)" &&
        mutate_lock_byte18_on_lock_ok && mutated_packet) {
        mutated_packet[0x18] = 2;
    }
}
static void message_data(int type, int target, int, int, int, int) {
    events.push_back("send"); ++sends; sent_type = type; sent_target = target;
}
struct Locks {
    int field_E0_, result_, global_result_, lock_calls_, global2_calls_;
    int global2_sender_, args_[7];
    Locks() : field_E0_(0), result_(0), global_result_(0), lock_calls_(0),
              global2_calls_(0), global2_sender_(-1), args_{} {}
    int lock(int a, int b, int c, int d, int e, int f, int g) {
        events.push_back("lock"); ++lock_calls_;
        args_[0]=a; args_[1]=b; args_[2]=c; args_[3]=d;
        args_[4]=e; args_[5]=f; args_[6]=g; return result_;
    }
    int check_global_2(int sender) {
        events.push_back("global2"); ++global2_calls_;
        global2_sender_=sender; return global_result_;
    }
    void check_global() { events.push_back("global"); }
};
struct NetDaemon { Locks locks_; };
"""

HOST_MAIN = r"""
static void put32(unsigned char *p, int at, int value) {
    std::memcpy(p + at, &value, sizeof value);
}
int main() {
    unsigned char packet[0x30] = {};
    put32(packet, 0x04, 4); // Decoy: pinned body does not use packet sender here.
    put32(packet, 0x10, 10); put32(packet, 0x14, 11);
    put32(packet, 0x18, 12); put32(packet, 0x1C, 13);
    put32(packet, 0x20, 15); put32(packet, 0x24, 14);
    ConsoleGlobal.faction_id_ = 1;

    // A nonlocal process can resolve to the local faction identity. The
    // synchronization gate compares that resolved identity, not msg+4.
    SpockHelpActive = 1;
    NetDaemon accepted;
    assert(netdaemon_handle_client_lock_message(
        &accepted, 0x2210, packet, 1));
    assert(logged_sender == 1 && accepted.locks_.lock_calls_ == 1);
    assert(accepted.locks_.args_[0] == 1 && accepted.locks_.args_[1] == 12);
    assert(accepted.locks_.args_[2] == 10 && accepted.locks_.args_[3] == 11);
    assert(accepted.locks_.args_[4] == 14 && accepted.locks_.args_[5] == 13);
    assert(accepted.locks_.args_[6] == 15);
    assert(sent_type == 0x1201 && sent_target == 1);
    // Native logs old coordinates, but reloads both after the callback.
    mutated_packet=packet;
    NetDaemon reentrant;
    assert(netdaemon_handle_client_lock_message(&reentrant,0x2210,packet,1));
    assert(logged_x==10 && logged_y==11);
    assert(reentrant.locks_.args_[2]==101 && reentrant.locks_.args_[3]==102);
    assert(reentrant.locks_.args_[4]==14 && reentrant.locks_.args_[5]==13);
    mutated_packet=nullptr;
    put32(packet,0x10,10); put32(packet,0x14,11);

    // The one-byte payload18 test occurs after the Lock OK callback. Mutating
    // that byte there must affect the following global-lock branch.
    put32(packet,0x18,0);
    mutated_packet=packet; mutate_lock_byte18_on_lock_ok=1;
    NetDaemon fresh_byte18; fresh_byte18.locks_.global_result_=1;
    sends=0; events.clear();
    assert(netdaemon_handle_client_lock_message(&fresh_byte18,0x2210,packet,1));
    assert(events.size()>=5 && events[2]=="log:(Sending: Lock OK)" &&
           events[3]=="global2");
    assert(packet[0x18]==2);
    assert(fresh_byte18.locks_.global2_calls_==1);
    assert(sent_type==0x2216 && sent_target==1);
    mutate_lock_byte18_on_lock_ok=0; mutated_packet=nullptr;

    // Reverse the decoy: resolved remote identity must reject even though
    // the packet word claims the local faction.
    put32(packet, 0x04, 1);
    NetDaemon rejected; sends = 0; ConsoleGlobal.faction_id_ = 1;
    assert(netdaemon_handle_client_lock_message(
        &rejected, 0x2210, packet, 2));
    assert(logged_sender == 2 && rejected.locks_.lock_calls_ == 0);
    assert(sends == 1 && sent_type == 0x1203 && sent_target == 2);
    assert(events[events.size()-2] == "log:(Sending: Lock failed -- REJECT)");

    SpockHelpActive = 0; TurnControlWord93A954 = 0;
    ConsoleControlTurnActive = 1; sends = 0;
    NetDaemon upkeep;
    assert(netdaemon_handle_client_lock_message(
        &upkeep, 0x2210, packet, 2));
    assert(upkeep.locks_.lock_calls_ == 0 && sent_type == 0x1203 &&
           sent_target == 2);
    ConsoleControlTurnActive = 0;

    NetDaemon failed; failed.locks_.result_ = 1; failed.locks_.field_E0_ = 1;
    assert(netdaemon_handle_client_lock_message(
        &failed, 0x2210, packet, 2));
    assert(failed.locks_.lock_calls_ == 1 && sent_type == 0x1203 &&
           sent_target == 2);

    NetDaemon global; global.locks_.global_result_ = 1;
    global.locks_.field_E0_ = 1; sends = 0;
    assert(netdaemon_handle_client_lock_message(
        &global, 0x2211, packet, 2));
    assert(logged_sender == 2 && global.locks_.global2_calls_ == 1 &&
           global.locks_.global2_sender_ == 2);
    assert(sent_type == 0x1203 && sent_target == 2);
    assert(events[events.size()-2] == "global2" &&
           events[events.size()-1] == "send");
    // Both failure opcodes use the live global-lock owner, not player identity.
    for (int owner : {0, 9}) {
        NetDaemon blocked; blocked.locks_.result_=1; blocked.locks_.field_E0_=owner;
        events.clear(); sends=0;
        assert(netdaemon_handle_client_lock_message(&blocked,0x2210,packet,2));
        assert(sends==1 && sent_type==0x1202+(owner!=0) && sent_target==2);
        NetDaemon global_blocked; global_blocked.locks_.global_result_=1;
        global_blocked.locks_.field_E0_=owner; events.clear(); sends=0;
        assert(netdaemon_handle_client_lock_message(&global_blocked,0x2211,packet,2));
        assert(sends==1 && sent_type==0x1202+(owner!=0) && sent_target==2);
        assert(events.back()=="send"); // Failure exits without check_global.
    }
    NetDaemon global_ready; events.clear(); sends=0;
    assert(netdaemon_handle_client_lock_message(&global_ready,0x2211,packet,2));
    assert(sends==1 && sent_type==0x1201 && sent_target==2);
    assert(events[events.size()-3]=="global2" &&
           events[events.size()-2]=="send" && events.back()=="global");
    return 0;
}
"""


def extract_definition(source, marker):
    start = source.find(marker)
    assert start >= 0, f"missing actual source declaration {marker}"
    opening = source.find("{", start)
    assert opening >= 0, f"missing actual source body {marker}"
    depth = 0
    quote = None
    escaped = False
    line_comment = block_comment = False
    index = opening
    while index < len(source):
        char = source[index]
        nxt = source[index + 1] if index + 1 < len(source) else ""
        if line_comment:
            if char == "\n": line_comment = False
        elif block_comment:
            if char == "*" and nxt == "/": block_comment = False; index += 1
        elif quote is not None:
            if escaped: escaped = False
            elif char == "\\": escaped = True
            elif char == quote: quote = None
        elif char == "/" and nxt == "/": line_comment = True; index += 1
        elif char == "/" and nxt == "*": block_comment = True; index += 1
        elif char in ('"', "'"): quote = char
        elif char == "{": depth += 1
        elif char == "}":
            depth -= 1
            if depth == 0: return source[start:index + 1]
        index += 1
    raise AssertionError("unterminated source definition: " + marker)


def extract_function(source, name):
    match = re.search(r"static\s+bool\s+" + re.escape(name) + r"\s*\(", source)
    assert match, f"missing actual source helper {name}"
    return extract_definition(
        source,
        match.group(0),
    )


def source_parts():
    text = SOURCE.read_text()
    lock_types = "\n".join((
        extract_definition(text, "struct MessageLockRecord {") + ";",
        extract_definition(text, "class MessageLockView {") + ";",
    ))
    helper = extract_function(text, HELPER)
    process_match = re.search(
        r"if\s*\(type\s*==\s*0x2210\s*\|\|\s*type\s*==\s*0x2211\)\s*\{"
        r"(?P<body>.*?)netdaemon_process_message_marker_tail\(\*this,\s*msg\);",
        text, re.S,
    )
    assert process_match, "missing actual 0x2210/0x2211 dispatcher call"
    return text, lock_types, helper, process_match.group("body")


def compile_and_run(lock_types, helper, compiler, directory, name):
    source_path = directory / (name + ".cpp")
    binary = directory / name
    source_path.write_text(HOST_PREAMBLE + "\n" + lock_types + "\n" + helper + "\n" + HOST_MAIN)
    built = subprocess.run(
        [compiler, "-std=c++11", "-Wall", "-Wextra", "-Werror",
         str(source_path), "-o", str(binary)], capture_output=True, text=True,
    )
    assert built.returncode == 0, built.stdout + built.stderr
    return subprocess.run([str(binary)], capture_output=True, text=True)


def test_actual_dispatch_passes_resolved_identity_and_not_packet_word():
    text, lock_types, helper, route = source_parts()
    assert "// ORIGINAL: 0x00534400" in text
    assert "0x00534400-0x005388B9;0x0065E0C2-0x0065E442" in text
    assert re.search(
        r"netdaemon_handle_client_lock_message\(this,\s*type,\s*msg,\s*"
        r"entry_sender_identity\)",
        route,
    )
    assert text.count("this->AlphaNet::pid_2_who(static_cast<unsigned long>(sender_id))") == 1
    assert text.index("const int entry_sender_identity =") < text.index("uint32_t entry_sequence;")
    assert "message + 4" not in helper
    assert "const int sender = sender_identity;" in helper


def test_actual_helper_controls_and_identity_mutation_under_gcc_and_clang():
    _, lock_types, helper, _ = source_parts()
    compilers = [shutil.which("g++"), shutil.which("clang++")]
    assert all(compilers), "both GCC and Clang are required for this host control"
    mutant = helper.replace(
        "const int sender = sender_identity;",
        "(void)sender_identity;\n    const int sender = *reinterpret_cast<int *>(message + 4);",
    )
    assert mutant != helper
    reply_tail = "        message_data(0x1201, sender, 0, 0, 0, 0);\n        daemon->locks_.check_global();"
    assert helper.count(reply_tail) == 1
    failure_tail = "                         sender, 0, 0, 0, 0);\n            return true;\n        }\n        message_data(0x1201, sender, 0, 0, 0, 0);"
    assert helper.count(failure_tail) == 1
    mutants = {
        "identity": mutant,
        "failure_opcode": helper.replace("message_data(0x1202 +", "message_data(0x1201 +"),
        "missing_ready_reply": helper.replace(reply_tail, "        daemon->locks_.check_global();"),
        "failure_fallthrough": helper.replace(failure_tail, failure_tail.replace("            return true;\n", "")),
        "failure_global_check": helper.replace(failure_tail, failure_tail.replace("            return true;", "            daemon->locks_.check_global();\n            return true;")),
        "cached_coordinates": helper.replace(
            "const int payload_10 = packet.payload_10();",
            "const int payload_10 = field_10;").replace(
            "const int payload_14 = packet.payload_14();",
            "const int payload_14 = field_14;"),
        "swapped_operands": helper.replace(
            "            payload_24, payload_1C, payload_20);",
            "            payload_1C, payload_24, payload_20);"),
        "cached_lock_byte18": helper.replace(
            '        log_say("(Sending: Lock OK)", 0, 0, 0);\n'
            '        if ((packet.payload_18_byte() & 2u) != 0 &&',
            '        const unsigned char cached_byte18 = packet.payload_18_byte();\n'
            '        log_say("(Sending: Lock OK)", 0, 0, 0);\n'
            '        if ((cached_byte18 & 2u) != 0 &&'),
    }
    with tempfile.TemporaryDirectory(prefix="osmx-2210-identity-") as temporary:
        directory = Path(temporary)
        for number, compiler in enumerate(compilers):
            good = compile_and_run(lock_types, helper, compiler, directory, f"good_{number}")
            assert good.returncode == 0, good.stdout + good.stderr
            for name, damaged in mutants.items():
                assert damaged != helper
                bad = compile_and_run(lock_types, damaged, compiler, directory, f"{name}_{number}")
                assert bad.returncode != 0, name + " mutant escaped control"


def test_pinned_lock_reply_opcodes_and_global_branch_order():
    import pytest
    from capstone import Cs, CS_ARCH_X86, CS_MODE_32
    from match_recovery.pe_coff import PeImage
    from match_recovery.model import Span
    path = ROOT / ".opensmacx/game/terranx_original.exe"
    if not path.is_file():
        pytest.skip("pinned original unavailable")
    image = PeImage(path.read_bytes(),
                   "01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf")
    cs = Cs(CS_ARCH_X86, CS_MODE_32)
    instructions = {}
    for start, end in ((0x535C4C, 0x535D07), (0x535D07, 0x535D70)):
        instructions.update({i.address: (i.mnemonic, i.op_str)
                            for i in cs.disasm(image.read(Span(start, end)), start)})
    expected = {
        0x535C4C: ("mov", "eax, dword ptr [ebx + 0x20]"),
        0x535C4F: ("mov", "edx, dword ptr [ebx + 0x1c]"),
        0x535C52: ("push", "eax"),
        0x535C53: ("mov", "eax, dword ptr [ebx + 0x24]"),
        0x535C56: ("push", "edx"),
        0x535C57: ("mov", "edx, dword ptr [ebx + 0x14]"),
        0x535C5A: ("push", "eax"),
        0x535C5B: ("mov", "eax, dword ptr [ebx + 0x10]"),
        0x535C5E: ("push", "edx"),
        0x535C5F: ("mov", "edx, dword ptr [ebx + 0x18]"),
        0x535C62: ("push", "eax"),
        0x535C63: ("push", "edx"),
        0x535C6A: ("push", "edi"),
        0x535C6B: ("call", "0x590300"),
        0x535C8C: ("push", "edi"),
        0x535C9A: ("add", "eax, 0x1202"),
        0x535D20: ("call", "0x5902c0"),
        0x535D2F: ("push", "edi"),
        0x535D30: ("je", "0x535d53"),
        0x535D3F: ("add", "ecx, 0x1202"),
        0x535D46: ("call", "0x592ee0"),
        0x535D4E: ("jmp", "0x538266"),
        0x535D53: ("push", "0x1201"),
        0x535D58: ("call", "0x592ee0"),
        0x535D66: ("call", "0x5901d0"),
    }
    for address, instruction in expected.items():
        assert instructions[address] == instruction
