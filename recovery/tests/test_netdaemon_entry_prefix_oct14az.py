"""Bounded host controls for process_message's captured entry header."""
from pathlib import Path
import shutil
import subprocess

import pytest

from match_recovery.source import read_file, read_text
from test_caviar_draw_callbacks_actual_source import extract
from test_texture_raster_actual_source import _continuity_fields


ROOT = Path(__file__).resolve().parents[2]
SOURCE = ROOT / "src/netdaemon.cpp"
BASE = "94a5ac6b4ee368f1ee639003a64de2e6eb8378f7"


def entry_prefix(source):
    body = extract(source, "void NetDaemon::process_message(")
    start = body.index("    const MessageCommandView entry_packet(msg);")
    end = body.index("\n\n    // File-transfer opcodes", start)
    return body[start:end]


def identity_read(source):
    body = extract(source, "void NetDaemon::process_message(")
    start = body.index("    const int entry_sender_identity =")
    end = body.index(";", start) + 1
    return body[start:end]


def actual_wire_types(source):
    record = extract(source, "struct MessageDataRecord {") + ";\n"
    view = extract(source, "class MessageCommandView {") + ";\n"
    return record + view


HOST = r'''
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>
static unsigned char *live_message;
static std::vector<int> observed_reads;
static std::vector<int> observed_events;
namespace std {
static void observed_copy(void *destination, const void *source, size_t size) {
    const auto *bytes = static_cast<const unsigned char *>(source);
    observed_reads.push_back(static_cast<int>(bytes-live_message)*100 + (int)size);
    observed_events.push_back(static_cast<int>(bytes-live_message)*100 + (int)size);
    ::memcpy(destination, source, size);
}
}
#define memcpy observed_copy
@@WIRE_TYPES@@
#undef memcpy
static int CommlinkPending = 0;
static int CommlinkFaction = 0;
struct AlphaNet {
    int pid_2_who(unsigned long sender) {
        observed_events.push_back(-1);
        return static_cast<int>(sender) + 10;
    }
};
struct Guard {};
static void netdaemon_transfer_modal_cleanup(Guard &) {}
static void message_data(int opcode, int, int, int, int, int) {
    if (opcode != 0x1505) std::abort();
    const unsigned short changed = 0x4401;
    const uint32_t changed_sequence = 777;
    const int changed_faction = 99;
    ::memcpy(live_message, &changed, sizeof changed);
    ::memcpy(live_message+0x0c, &changed_sequence, sizeof changed_sequence);
    ::memcpy(live_message+4, &changed_faction, sizeof changed_faction);
}
static void fail(const char *message) {
    std::fprintf(stderr, "CONTROL_FAIL: %s\n", message);
    std::exit(97);
}
struct NetDaemon : AlphaNet {
void run_prefix(unsigned char *message) {
    live_message = message;
    unsigned long message_size = 32;
    int sender_id = 7;
    unsigned char *msg = message;
@@IDENTITY_READ@@
    Guard guard;
@@ENTRY_PREFIX@@
    if (entry_sequence != 456 || entry_packet_faction != 7)
        fail("entry header values were not captured before callback");
    if (type != 0x4401) fail("fresh opcode was not read after callback");
    const std::vector<int> expected = {1204, 2, 404, 2};
    if (observed_reads != expected) fail("entry getter read order/width/offset changed");
    const std::vector<int> expected_events = {-1, 1204, 2, 404, 2};
    if (observed_events != expected_events) fail("sender identity did not precede entry reads");
    (void)message_size; (void)sender_id;
}
};
int main() {
    alignas(4) unsigned char message[32] = {};
    const unsigned short opcode = 0x1500;
    const int faction = 7;
    const uint32_t sequence = 456;
    const uint32_t timestamp = 456;
    ::memcpy(message, &opcode, 2);
    ::memcpy(message+4, &faction, 4);
    ::memcpy(message+8, &timestamp, 4);
    ::memcpy(message+0x0c, &sequence, 4);
    NetDaemon daemon;
    daemon.run_prefix(message);
}
'''


def test_entry_header_metadata_and_source_order():
    old = subprocess.check_output(
        ["git", "show", f"{BASE}:src/netdaemon.cpp"], cwd=ROOT,
        text=True, timeout=30,
    )
    before = read_text(old, SOURCE)
    after = read_file(SOURCE)
    old_subject = next(row for row in before if row.address == 0x534400)
    current_subject = next(row for row in after if row.address == 0x534400)
    assert (_continuity_fields(old_subject), old_subject.symbol,
            old_subject.function_fragment) == (
        _continuity_fields(current_subject), current_subject.symbol,
        current_subject.function_fragment,
    )

    source = SOURCE.read_text(encoding="utf-8")
    body = extract(source, "void NetDaemon::process_message(")
    identity = body.index("this->AlphaNet::pid_2_who(")
    reads = body.index("const MessageCommandView entry_packet(msg);")
    sequence = body.index("entry_packet.sequence()", reads)
    opcode = body.index("entry_packet.opcode()", sequence)
    faction = body.index("entry_packet.faction()", opcode)
    skip = body.index("bool skip500", faction)
    callback = body.index("message_data(0x1505, entry_packet_faction", skip)
    reload = body.index("type = entry_packet.opcode();", callback)
    assert identity < reads < sequence < opcode < faction < skip < callback < reload
    assert body.count("const MessageCommandView entry_packet(msg);") == 1
    assert body.count("\n    type = entry_packet.opcode();") == 1


@pytest.mark.parametrize("damage,diagnostic", [
    ("none", None),
    ("cached_opcode", "fresh opcode was not read after callback"),
    ("wrong_sequence", "entry getter read order/width/offset changed"),
    ("reordered_reads", "entry getter read order/width/offset changed"),
])
def test_actual_entry_prefix_and_callback_mutation(tmp_path, damage, diagnostic):
    compiler = shutil.which("g++") or shutil.which("clang++")
    if not compiler:
        pytest.skip("host C++ compiler unavailable")
    source_text = SOURCE.read_text(encoding="utf-8")
    types = actual_wire_types(source_text)
    prefix = entry_prefix(source_text)
    identity = identity_read(source_text)
    if damage == "cached_opcode":
        prefix = prefix.replace("\n    type = entry_packet.opcode();",
                                "\n    type = type;", 1)
        assert "type = type;" in prefix
    elif damage == "wrong_sequence":
        prefix = prefix.replace("entry_packet.sequence()", "entry_packet.timestamp_bits()")
        assert "entry_packet.timestamp_bits()" in prefix
    elif damage == "reordered_reads":
        first = "    uint32_t entry_sequence = entry_packet.sequence();\n"
        second = "    unsigned short type = entry_packet.opcode();\n"
        assert first + second in prefix
        prefix = prefix.replace(first + second, second + first, 1)
    harness = (HOST.replace("@@WIRE_TYPES@@", types)
               .replace("@@IDENTITY_READ@@", identity)
               .replace("@@ENTRY_PREFIX@@", prefix))
    cpp = tmp_path / "entry_prefix.cpp"
    exe = tmp_path / "entry_prefix"
    cpp.write_text(harness, encoding="utf-8")
    built = subprocess.run(
        [compiler, "-std=c++11", "-O1", str(cpp), "-o", str(exe)],
        capture_output=True, text=True, timeout=60,
    )
    assert built.returncode == 0, built.stdout + built.stderr
    ran = subprocess.run([str(exe)], capture_output=True, text=True, timeout=10)
    if diagnostic is None:
        assert ran.returncode == 0, ran.stdout + ran.stderr
    else:
        assert ran.returncode == 97
        assert ran.stderr == f"CONTROL_FAIL: {diagnostic}\n"
