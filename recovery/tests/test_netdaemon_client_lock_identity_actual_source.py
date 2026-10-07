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
static int sent_type = -1, sent_target = -1, sends;
static void log_say(const char *label, int a, int, int) {
    events.push_back(std::string("log:") + label);
    if (std::string(label) == "From Client: LOCK" ||
        std::string(label) == "From Client: LOCK GLOBAL") logged_sender = a;
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
    assert(accepted.locks_.args_[4] == 13 && accepted.locks_.args_[5] == 14);
    assert(accepted.locks_.args_[6] == 15);
    assert(sent_type == 0x1201 && sent_target == 1);

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
    assert(failed.locks_.lock_calls_ == 1 && sent_type == 0x1202 &&
           sent_target == 2);

    NetDaemon global; global.locks_.global_result_ = 1;
    global.locks_.field_E0_ = 1; sends = 0;
    assert(netdaemon_handle_client_lock_message(
        &global, 0x2211, packet, 2));
    assert(logged_sender == 2 && global.locks_.global2_calls_ == 1 &&
           global.locks_.global2_sender_ == 2);
    assert(sent_type == 0x1202 && sent_target == 2);
    assert(events[events.size()-3] == "global2" &&
           events[events.size()-2] == "send" &&
           events[events.size()-1] == "global");
    return 0;
}
"""


def extract_function(source, name):
    match = re.search(r"static\s+bool\s+" + re.escape(name) + r"\s*\(", source)
    assert match, f"missing actual source helper {name}"
    opening = source.index("{", match.end())
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
            if depth == 0: return source[match.start():index + 1]
        index += 1
    raise AssertionError("unterminated lock helper")


def source_parts():
    text = SOURCE.read_text()
    helper = extract_function(text, HELPER)
    process_match = re.search(
        r"if\s*\(type\s*==\s*0x2210\s*\|\|\s*type\s*==\s*0x2211\)\s*\{"
        r"(?P<body>.*?)netdaemon_process_message_marker_tail\(\*this,\s*msg\);",
        text, re.S,
    )
    assert process_match, "missing actual 0x2210/0x2211 dispatcher call"
    return text, helper, process_match.group("body")


def compile_and_run(helper, compiler, directory, name):
    source_path = directory / (name + ".cpp")
    binary = directory / name
    source_path.write_text(HOST_PREAMBLE + "\n" + helper + "\n" + HOST_MAIN)
    built = subprocess.run(
        [compiler, "-std=c++11", "-Wall", "-Wextra", "-Werror",
         str(source_path), "-o", str(binary)], capture_output=True, text=True,
    )
    assert built.returncode == 0, built.stdout + built.stderr
    return subprocess.run([str(binary)], capture_output=True, text=True)


def test_actual_dispatch_passes_resolved_identity_and_not_packet_word():
    text, helper, route = source_parts()
    assert "// ORIGINAL: 0x00534400" in text
    assert "0x00534400-0x005388B9;0x0065E0C2-0x0065E442" in text
    assert re.search(
        r"netdaemon_handle_client_lock_message\(this,\s*type,\s*msg,\s*"
        r"this->AlphaNet::pid_2_who\(static_cast<unsigned long>\(sender_id\)\)\)",
        route,
    )
    assert "message + 4" not in helper
    assert "const int sender = sender_identity;" in helper


def test_actual_helper_controls_and_identity_mutation_under_gcc_and_clang():
    _, helper, _ = source_parts()
    compilers = [shutil.which("g++"), shutil.which("clang++")]
    assert all(compilers), "both GCC and Clang are required for this host control"
    mutant = helper.replace(
        "const int sender = sender_identity;",
        "(void)sender_identity;\n    const int sender = *reinterpret_cast<int *>(message + 4);",
    )
    assert mutant != helper
    with tempfile.TemporaryDirectory(prefix="osmx-2210-identity-") as temporary:
        directory = Path(temporary)
        for number, compiler in enumerate(compilers):
            good = compile_and_run(helper, compiler, directory, f"good_{number}")
            assert good.returncode == 0, good.stdout + good.stderr
            bad = compile_and_run(mutant, compiler, directory, f"mutant_{number}")
            assert bad.returncode != 0, "packet-sender identity mutant escaped control"
