"""Actual-source host controls for callback-sensitive NetDaemon packet views."""
from pathlib import Path
import shutil
import subprocess

import pytest
from match_recovery.source import read_file, read_text
from test_texture_raster_actual_source import _continuity_fields


def repository_root():
    for candidate in Path(__file__).resolve().parents:
        if (candidate / "src/netdaemon.cpp").is_file():
            return candidate
    raise AssertionError("could not locate OpenSMACX source root")


ROOT = repository_root()
SOURCE = ROOT / "src/netdaemon.cpp"
BASELINE = "6cbccf2ecb9b7c4295f1db09eb8f63a98644ef86"


def extract(source, declaration):
    start = source.index(declaration)
    opening = source.index("{", start)
    depth = 0
    state = "code"
    i = opening
    while i < len(source):
        ch = source[i]
        nxt = source[i + 1] if i + 1 < len(source) else ""
        if state == "code":
            if ch == "/" and nxt == "/": state = "line"; i += 2; continue
            if ch == "/" and nxt == "*": state = "block"; i += 2; continue
            if ch == '"': state = "string"
            elif ch == "'": state = "char"
            elif ch == "{": depth += 1
            elif ch == "}":
                depth -= 1
                if depth == 0:
                    return source[start:i + 1]
        elif state == "line":
            if ch == "\n": state = "code"
        elif state == "block":
            if ch == "*" and nxt == "/": state = "code"; i += 2; continue
        else:
            if ch == "\\": i += 2; continue
            if (state == "string" and ch == '"') or (state == "char" and ch == "'"):
                state = "code"
        i += 1
    raise AssertionError("unterminated actual declaration: " + declaration)


def actual_parts():
    text = SOURCE.read_text(encoding="utf-8")
    lock_record = extract(text, "struct MessageLockRecord {")
    lock_view = extract(text, "class MessageLockView {")
    data_record = extract(text, "struct MessageDataRecord {")
    command_view = extract(text, "class MessageCommandView {")
    lock_helper = extract(text, "static bool netdaemon_handle_client_lock_message(")
    channel_helper = extract(text, "static bool netdaemon_handle_lock_mask_residual(")
    assert "payload_1C()" in lock_view and "payload_20()" in lock_view
    assert "payload_24()" in lock_view and "payload_18_byte()" in lock_view
    # Instrument only the real field-copy operations; all control flow and
    # offset expressions remain extracted from the source unchanged.
    lock_view = lock_view.replace("std::memcpy(", "observed_copy(")
    command_view = command_view.replace("std::memcpy(", "observed_copy(")
    return text, lock_record, lock_view, data_record, command_view, lock_helper, channel_helper


HOST = r'''
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <utility>
#include <vector>

struct Read { int offset; int width; };
static unsigned char *live_packet;
static std::vector<std::vector<Read> > expected_read_groups;
static std::vector<bool> ordered_read_groups;
static std::vector<bool> group_read_seen;
static size_t current_read_group;
static size_t reads_in_group;
static void fail97(const char *message) {
    std::fprintf(stderr, "CONTROL_FAIL: %s\n", message);
    std::exit(97);
}
static void observed_copy(void *destination, const void *source, size_t width) {
    if (live_packet != 0) {
        const uintptr_t base = reinterpret_cast<uintptr_t>(live_packet);
        const uintptr_t from = reinterpret_cast<uintptr_t>(source);
        if (from >= base && from < base + 0x40) {
            const Read actual = { static_cast<int>(from - base), static_cast<int>(width) };
            if (current_read_group >= expected_read_groups.size())
                fail97("wire read offset/width/order changed");
            const std::vector<Read> &group = expected_read_groups[current_read_group];
            size_t matched = group.size();
            if (ordered_read_groups[current_read_group]) {
                if (reads_in_group < group.size() &&
                    group[reads_in_group].offset == actual.offset &&
                    group[reads_in_group].width == actual.width)
                    matched = reads_in_group;
            } else {
                for (size_t i=0;i<group.size();++i) {
                    if (!group_read_seen[i] && group[i].offset == actual.offset &&
                        group[i].width == actual.width) { matched=i; break; }
                }
            }
            if (matched == group.size())
                fail97("wire read offset/width/order changed");
            if (!ordered_read_groups[current_read_group]) group_read_seen[matched]=true;
            ++reads_in_group;
            if (reads_in_group == group.size()) {
                ++current_read_group;
                reads_in_group=0;
                group_read_seen.clear();
                if (current_read_group < expected_read_groups.size())
                    group_read_seen.assign(
                        expected_read_groups[current_read_group].size(), false);
            }
        }
    }
    ::memcpy(destination, source, width);
}

// ACTUAL_RECORDS_AND_VIEWS

static int SpockHelpActive, TurnControlWord93A954, ConsoleControlTurnActive;
struct Console { int faction_id_; } ConsoleGlobal;
struct ChannelWords { uint32_t trailing_words_[12]; };
static std::vector<std::string> events;
static int logged_sender, logged_x, logged_y;
static int callback_mode;
static int sent_type, sent_target, sent_newly_acquired, send_calls;
static int lock_calls, global_calls;
static int lock_args[7];
static class NetDaemon *active_daemon;

struct LockStore {
    int field_E0_;
    int result_;
    int global_result_;
    int lock_calls_;
    int global_calls_;
    int args_[7];
    LockStore() : field_E0_(0), result_(0), global_result_(0),
                  lock_calls_(0), global_calls_(0), args_{} {}
    int lock(int a, int b, int c, int d, int e, int f, int g) {
        ++lock_calls; ++lock_calls_;
        args_[0]=a; args_[1]=b; args_[2]=c; args_[3]=d;
        args_[4]=e; args_[5]=f; args_[6]=g;
        for (int i=0;i<7;++i) lock_args[i]=args_[i];
        return result_;
    }
    int check_global_2(int) { ++global_calls; ++global_calls_; return global_result_; }
    void check_global() { events.push_back("global"); }
};
class NetDaemon {
public:
    LockStore locks_;
    ChannelWords channels_[2];
    NetDaemon() : locks_(), channels_{} {}
};
static void set32(unsigned char *bytes, int offset, int value) {
    ::memcpy(bytes + offset, &value, 4);
}
static void require_reads_complete() {
    if (current_read_group != expected_read_groups.size() || reads_in_group != 0)
        fail97("wire read offset/width/order changed");
}
static void expect(const std::vector<std::vector<Read> > &groups,
                   const std::vector<bool> &ordered) {
    expected_read_groups=groups;
    ordered_read_groups=ordered;
    current_read_group=0;
    reads_in_group=0;
    group_read_seen.clear();
    if (groups.size()!=ordered.size()) fail97("bad fixture read groups");
    if (!groups.empty() && !ordered[0]) fail97("initial callback epoch must stay ordered");
    if (!groups.empty()) group_read_seen.assign(groups[0].size(), false);
}

static void log_say(const char *label, int a, int b, int c) {
    events.push_back(std::string("log:") + label);
    if (std::string(label) == "From Client: LOCK") {
        logged_sender=a; logged_x=b; logged_y=c;
        if (callback_mode != 0) {
            set32(live_packet, 0x10, 201);
            set32(live_packet, 0x14, 202);
            set32(live_packet, 0x18, 0x12345678);
            set32(live_packet, 0x1C, 203);
            set32(live_packet, 0x20, 205);
            set32(live_packet, 0x24, 204);
        }
    } else if (std::string(label) == "(Sending: Lock OK)" && callback_mode != 0) {
        // The following decision observes this fresh byte, not payload_18's
        // earlier four-byte argument value.
        live_packet[0x18] = callback_mode == 1 ? 2 : 0;
    } else if (std::string(label) == "Received OPEN CHANNEL request") {
        if (active_daemon == 0 || active_daemon->channels_[0].trailing_words_[0] == 0)
            fail97("channel mask writes did not precede callback");
        logged_sender=a; logged_x=b; logged_y=c;
    }
}
static void message_data(int type, int target, int newly, int, int, int) {
    events.push_back("send");
    ++send_calls; sent_type=type; sent_target=target; sent_newly_acquired=newly;
}

// ACTUAL_HELPERS

static int run_client_lock(int mutate_late_byte) {
    unsigned char storage[0x50];
    std::memset(storage, 0xA5, sizeof storage);
    unsigned char *packet = storage + 1; // deliberately not dword aligned
    std::memset(packet, 0, 0x30);
    set32(packet, 0x10, 10); set32(packet, 0x14, 11);
    set32(packet, 0x18, 0x80000002); set32(packet, 0x1C, 13);
    set32(packet, 0x20, 15); set32(packet, 0x24, 14);
    live_packet=packet; active_daemon=0;
    expect({{{0x14,4},{0x10,4}},
            {{0x18,4},{0x10,4},{0x14,4},{0x24,4},{0x1C,4},{0x20,4}},
            {{0x18,1}}}, {true,false,true});
    events.clear(); send_calls=lock_calls=global_calls=0;
    ConsoleGlobal.faction_id_=77; SpockHelpActive=0;
    TurnControlWord93A954=0; ConsoleControlTurnActive=0;
    callback_mode=mutate_late_byte;
    NetDaemon daemon; daemon.locks_.global_result_=1;
    const bool handled=netdaemon_handle_client_lock_message(&daemon,0x2210,packet,3);
    require_reads_complete();
    if (!handled) fail97("client lock route was not handled");
    if (logged_sender!=3 || logged_x!=10 || logged_y!=11)
        fail97("initial lock log did not observe entry coordinates and sender_identity");
    if (daemon.locks_.lock_calls_!=1 || lock_args[0]!=3 || lock_args[1]!=0x12345678 ||
        lock_args[2]!=201 || lock_args[3]!=202 || lock_args[4]!=204 ||
        lock_args[5]!=203 || lock_args[6]!=205)
        fail97("lock arguments did not use sender_identity and post-log payload reads");
    if (global_calls != (mutate_late_byte==1 ? 1 : 0))
        fail97("late payload byte did not control global-lock check");
    if (send_calls!=1 || sent_target!=3 ||
        sent_type!=(mutate_late_byte==1 ? 0x2216 : 0x1201))
        fail97("late payload byte selected the wrong lock reply");
    return 0;
}

static int run_reject() {
    unsigned char storage[0x50]={}; unsigned char *packet=storage+1;
    set32(packet,0x10,10); set32(packet,0x14,11);
    live_packet=packet; active_daemon=0;
    expect({{{0x14,4},{0x10,4}}}, {true});
    events.clear(); send_calls=lock_calls=0; callback_mode=0;
    ConsoleGlobal.faction_id_=3; SpockHelpActive=1;
    NetDaemon daemon;
    if (!netdaemon_handle_client_lock_message(&daemon,0x2210,packet,4))
        fail97("rejected client lock route was not handled");
    require_reads_complete();
    if (daemon.locks_.lock_calls_!=0 || send_calls!=1 ||
        sent_type!=0x1203 || sent_target!=4)
        fail97("original client lock reject reply changed");
    return 0;
}

static int run_channel_alias() {
    NetDaemon daemon;
    // Payload word +0x10 aliases indexed channel slot 4 (identity 3). The
    // client route separately exercises a deliberately unaligned packet.
    unsigned char *packet=reinterpret_cast<unsigned char *>(
        daemon.channels_[0].trailing_words_);
    set32(packet,0x10,3); set32(packet,0x14,5);
    live_packet=packet;
    expect({{{0x14,4},{0x10,4}},{{0x10,4},{0x14,4}}}, {true,false});
    events.clear(); send_calls=0; callback_mode=0;
    active_daemon=&daemon;
    if (!netdaemon_handle_lock_mask_residual(daemon,0x8240,packet,3))
        fail97("open-channel route was not handled");
    require_reads_complete();
    const uint32_t mask=(uint32_t(1)<<3)|(uint32_t(1)<<5);
    if (daemon.channels_[0].trailing_words_[0]!=mask ||
        daemon.channels_[0].trailing_words_[4]!=mask)
        fail97("aliased channel aggregate/slot write changed");
    if (logged_sender!=3 || logged_x!=static_cast<int>(mask) || logged_y!=5)
        fail97("open-channel log did not reload payload after aliased writes");
    if (send_calls!=1 || sent_type!=0x1240 || sent_target!=3 ||
        sent_newly_acquired!=1)
        fail97("open-channel acknowledgement changed");
    return 0;
}

int main(int argc, char **argv) {
    if (argc!=2) return 2;
    const std::string mode=argv[1];
    if (mode=="lock") return run_client_lock(1);
    if (mode=="late_zero") return run_client_lock(2);
    if (mode=="reject") return run_reject();
    if (mode=="channel") return run_channel_alias();
    return 2;
}
'''


def build_host(tmp_path, compiler, parts, mutations=()):
    text, lock_record, lock_view, data_record, command_view, lock_helper, channel_helper = parts
    if "cache_lock_payload" in mutations:
        lock_helper = lock_helper.replace(
            "        const int field_14 = packet.payload_14();",
            "        const int cached_payload_18 = packet.payload_18();\n"
            "        const int field_14 = packet.payload_14();", 1)
        lock_helper = lock_helper.replace(
            "const int payload_18 = packet.payload_18();",
            "const int payload_18 = cached_payload_18;", 1)
    if "wrong_sender_identity" in mutations:
        lock_helper = lock_helper.replace(
            "const int sender = sender_identity;",
            "const int sender = sender_identity + 1;", 1)
    if "reorder_lock_payload_group" in mutations:
        original = (
            "        const int payload_18 = packet.payload_18();\n"
            "        const int payload_10 = packet.payload_10();\n"
            "        const int payload_14 = packet.payload_14();\n"
            "        const int payload_24 = packet.payload_24();\n"
            "        const int payload_1C = packet.payload_1C();\n"
            "        const int payload_20 = packet.payload_20();")
        reordered = (
            "        const int payload_20 = packet.payload_20();\n"
            "        const int payload_1C = packet.payload_1C();\n"
            "        const int payload_24 = packet.payload_24();\n"
            "        const int payload_14 = packet.payload_14();\n"
            "        const int payload_10 = packet.payload_10();\n"
            "        const int payload_18 = packet.payload_18();")
        assert original in lock_helper
        lock_helper = lock_helper.replace(original, reordered, 1)
    if "wrong_byte_offset" in mutations or "wide_byte_read" in mutations:
        byte_accessor = extract(lock_view, "    unsigned char payload_18_byte()")
        changed = byte_accessor
        if "wrong_byte_offset" in mutations:
            changed = changed.replace("field_18_", "field_1C_", 1)
        if "wide_byte_read" in mutations:
            changed = changed.replace("sizeof value", "sizeof(uint32_t)", 1)
        lock_view = lock_view.replace(byte_accessor, changed, 1)
    harness = (HOST.replace("// ACTUAL_RECORDS_AND_VIEWS",
                            lock_record + ";\n" + lock_view + ";\n" +
                            data_record + ";\n" + command_view + ";")
               .replace("// ACTUAL_HELPERS", lock_helper + "\n" + channel_helper))
    cpp = tmp_path / "netdaemon_receiver_views.cpp"
    exe = tmp_path / "netdaemon_receiver_views"
    tmp_path.mkdir(parents=True, exist_ok=True)
    cpp.write_text(harness, encoding="utf-8")
    built = subprocess.run(
        [compiler, "-std=c++11", "-Wall", "-Wextra", "-Werror",
         str(cpp), "-o", str(exe)],
        capture_output=True, text=True, timeout=45,
    )
    assert built.returncode == 0, built.stdout + built.stderr
    return exe


def run_case(tmp_path, compiler, parts, mode, mutations=()):
    exe = build_host(tmp_path, compiler, parts, mutations)
    return subprocess.run([str(exe), mode], capture_output=True, text=True, timeout=10)


def test_process_message_mapping_identity_and_spans_are_retained():
    text = SOURCE.read_text(encoding="utf-8")
    old = subprocess.run(
        ["git", "show", BASELINE + ":src/netdaemon.cpp"], cwd=ROOT,
        check=True, text=True, capture_output=True, timeout=30,
    ).stdout
    before = read_text(old, SOURCE)
    after = read_file(SOURCE)
    prior = next(row for row in before if row.address == 0x00534400)
    current = next(row for row in after if row.address == 0x00534400)
    assert (_continuity_fields(prior), prior.symbol, prior.function_fragment) == (
        _continuity_fields(current), current.symbol, current.function_fragment,
    )
    marker = ("// ORIGINAL: 0x00534400 ?process_message@NetDaemon@@QAEXPADKH@Z "
              "0x00534400-0x005388B9;0x0065E0C2-0x0065E442 FILE")
    assert text.count(marker) == 1
    following = text[text.index(marker):text.index(marker) + 2200]
    assert "Historical seed status:" in following
    assert "TRIED:" in following
    def mapped_history(source):
        start = source.index(marker)
        end = source.index("void NetDaemon::process_message(", start)
        return source[start:end]
    assert mapped_history(old) == mapped_history(text)


@pytest.mark.parametrize("compiler_name", ["g++", "clang++"])
def test_actual_lock_and_channel_helpers_use_unaligned_callback_sensitive_views(
        tmp_path, compiler_name):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(compiler_name + " unavailable")
    parts = actual_parts()
    for mode in ("lock", "late_zero", "reject", "channel"):
        result = run_case(tmp_path / mode, compiler, parts, mode)
        assert result.returncode == 0, result.stdout + result.stderr


@pytest.mark.parametrize("compiler_name", ["g++", "clang++"])
@pytest.mark.parametrize("mutation,mode,diagnostic", [
    ("cache_lock_payload", "lock", "wire read offset/width/order changed"),
    ("wrong_byte_offset", "lock", "wire read offset/width/order changed"),
    ("wide_byte_read", "lock", "wire read offset/width/order changed"),
    ("wrong_sender_identity", "lock",
     "initial lock log did not observe entry coordinates and sender_identity"),
])
def test_compile_success_receiver_wire_mutants_fail_semantically(
        tmp_path, compiler_name, mutation, mode, diagnostic):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(compiler_name + " unavailable")
    result = run_case(tmp_path, compiler, actual_parts(), mode, (mutation,))
    assert result.returncode == 97, (result.returncode, result.stdout, result.stderr)
    assert result.stderr == "CONTROL_FAIL: " + diagnostic + "\n"


@pytest.mark.parametrize("compiler_name", ["g++", "clang++"])
def test_pure_lock_payload_read_group_does_not_require_incidental_order(
        tmp_path, compiler_name):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(compiler_name + " unavailable")
    result = run_case(tmp_path, compiler, actual_parts(), "lock",
                      ("reorder_lock_payload_group",))
    assert result.returncode == 0, result.stdout + result.stderr
