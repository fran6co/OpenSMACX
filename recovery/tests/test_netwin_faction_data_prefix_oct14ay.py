"""Actual-source controls for the partly initialized faction-data wire prefix.

Host doubles verify the extracted sender and source-owned borrowed view. They
do not establish the native ABI/provider, runtime transport, or full-game
acceptance.
"""
from __future__ import annotations

from dataclasses import fields
from pathlib import Path
import subprocess
import shutil

import pytest
from match_recovery.source import read_text

from test_net_hotseat_owner_actual_source_oct6 import (
    extract_definition, installed_packet_declarations,
)


ROOT = next(parent for parent in Path(__file__).resolve().parents
            if (parent / "src/netwin.cpp").is_file())
SOURCE = ROOT / "src/netwin.cpp"
HEADER = ROOT / "src/netwin.h"
BASE = "095d4e2b5b907fbc3d6ce6e07aef2e705a515a5b"
SIGNATURE = "void __cdecl send_faction_data(int slot_id)"
ADDRESS = 0x0047B020
ORIGINAL_SPAN = (0x0047B020, 0x0047B0E1)


def test_prefix_view_and_sender_keep_original_identity_and_named_partial_fields():
    source = SOURCE.read_text(encoding="utf-8")
    prefix = extract_definition(source, "struct NetWinFactionDataPrefix {")
    view = extract_definition(source, "class NetWinFactionDataPacketView {")
    body = extract_definition(source, SIGNATURE)
    rows = [row for row in read_text(SOURCE.read_text(encoding="utf-8"), SOURCE)
            if row.address == ADDRESS]
    assert len(rows) == 1
    row = rows[0]
    assert row.image_spans == (ORIGINAL_SPAN,)
    for field in ("uint16_t opcode_;", "uint8_t unresolved_2_[2];",
                  "int32_t faction_;", "uint32_t timestamp_;",
                  "uint32_t sequence_;"):
        assert field in prefix
    for method in ("set_opcode(uint16_t", "set_faction(int32_t",
                   "set_timestamp(uint32_t", "char *payload() const"):
        assert method in view
    assert "set_sequence" not in view
    assert "NetWinFactionDataPacketView packet(frame);" in body
    assert "packet.set_opcode(0xF0D);" in body
    assert "packet.set_faction(-1);" in body
    assert "packet.set_timestamp(0);" in body
    assert "memcpy(packet.payload(), NetWinDialog.faction_records_," in body
    assert "NetWinDialog.faction_records_ == 0" in body
    assert "slot_id < 0" in body
    assert not any(token in body for token in ("free(", "mem_free(", "delete "))


def test_sender_original_mapping_and_history_match_authorized_task_base():
    baseline_text = subprocess.check_output(
        ["git", "show", f"{BASE}:src/netwin.cpp"], cwd=ROOT, text=True,
        timeout=30,
    )
    current_text = SOURCE.read_text(encoding="utf-8")
    # Protect this recovery's subject, not unrelated owners' future emitted
    # symbol migrations or newly measured claims. Whole-project continuity is
    # checked by the native historical-identity verifier at integration.
    baseline = {row.address: row for row in read_text(baseline_text, SOURCE)
                if row.address == ADDRESS}
    current = {row.address: row for row in read_text(current_text, SOURCE)
               if row.address == ADDRESS}
    assert set(baseline) == {ADDRESS}
    assert current.keys() == baseline.keys()
    for address, prior in baseline.items():
        row = current[address]
        for item in fields(row):
            if item.name in {"line", "region", "origin"}:
                continue
            assert getattr(row, item.name) == getattr(prior, item.name), (
                f"{address:#x} mapping metadata {item.name}")
        history = lambda value: tuple(
            (lesson.kind, lesson.key, lesson.prose,
             tuple(line for _, line in lesson.lines))
            for lesson in value.origin or ()
        )
        assert history(row) == history(prior), f"{address:#x} historical notes"


HOST_PREFIX = r'''#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#define __cdecl
// INSERT_ACTUAL_PACKET_DECLARATIONS
static_assert(sizeof(NetWinFactionDataPrefix) == 0x10, "prefix extent");
static_assert(offsetof(NetWinFactionDataPrefix, opcode_) == 0, "opcode offset");
static_assert(offsetof(NetWinFactionDataPrefix, unresolved_2_) == 2, "reserved offset");
static_assert(offsetof(NetWinFactionDataPrefix, faction_) == 4, "faction offset");
static_assert(offsetof(NetWinFactionDataPrefix, timestamp_) == 8, "timestamp offset");
static_assert(offsetof(NetWinFactionDataPrefix, sequence_) == 0xC, "sequence offset");

static void control_fail(const char *name) {
  std::fprintf(stderr, "CONTROL_FAIL:%s\n", name);
  std::exit(97);
}
static void require(bool condition, const char *name) {
  if (!condition) control_fail(name);
}
static unsigned char frame_storage[120000];
alignas(NetWinFactionRecord) static unsigned char table_a[120000];
alignas(NetWinFactionRecord) static unsigned char table_b[120000];
static unsigned expected_allocation, expected_copy, actual_allocation;
static unsigned allocations, copies, sends, setups, setup_argument;
static unsigned sent_size, sent_pid, sent_flags;
static int send_result = -1;
static int expected_setup;
static int replace_table_after_allocation;
static unsigned char *expected_payload;
static unsigned event_trace[8], event_count;
static int NetWinPlayerId = 7;
static int NetWinOtherPlayerId = 7;

struct NetWinDialogDouble {
  NetWinFactionRecord *faction_records_ = nullptr;
  NetWinFactionRecord *setup_records = nullptr;
  void setup_faction_data(int argument) {
    if (!expected_setup) control_fail("optional-rebuild-guard");
    ++setups;
    event_trace[event_count++] = 1;
    setup_argument = static_cast<unsigned>(argument);
    faction_records_ = setup_records;
  }
} NetWinDialog;

struct SafeProcessSlots {
  struct Slot { uint32_t pid_ = 0; } entries_[24];
  Slot &operator[](int index) { return entries_[index + 8]; }
} ;
struct Net {
  int send(char *data, unsigned size, unsigned pid, int flags) {
    require(data == reinterpret_cast<char *>(frame_storage), "send-frame");
    sent_size = size;
    sent_pid = pid;
    sent_flags = static_cast<unsigned>(flags);
    event_trace[event_count++] = 4;
    ++sends;
    return send_result;
  }
};
struct NetDaemonDouble : Net {
  SafeProcessSlots process_slots_;
} NetDaemonNet;

static void *mem_get(unsigned size) {
  if (size != expected_allocation) control_fail("allocation-size");
  actual_allocation = size;
  std::memset(frame_storage, 0xA5, sizeof(frame_storage));
  ++allocations;
  event_trace[event_count++] = 2;
  if (replace_table_after_allocation)
    NetWinDialog.faction_records_ = reinterpret_cast<NetWinFactionRecord *>(table_b);
  return frame_storage;
}
static void *observed_copy(void *to, const void *from, unsigned size) {
  uint16_t opcode = 0;
  int32_t faction = 0;
  uint32_t timestamp = 0;
  std::memcpy(&opcode, frame_storage, sizeof(opcode));
  std::memcpy(&faction, frame_storage + 4, sizeof(faction));
  std::memcpy(&timestamp, frame_storage + 8, sizeof(timestamp));
  if (opcode != 0x0F0D || faction != -1 || timestamp != 0 ||
      frame_storage[2] != 0xA5 || frame_storage[3] != 0xA5)
    control_fail("faction-prefix-before-payload");
  for (unsigned i = 0xC; i < 0x10; ++i)
    if (frame_storage[i] != 0xA5) control_fail("faction-prefix-before-payload");
  if (to != frame_storage + 0x10 || from != expected_payload)
    control_fail("faction-payload-view");
  if (size != expected_copy || size + 0x10 != actual_allocation)
    control_fail("faction-copy-size");
  ++copies;
  event_trace[event_count++] = 3;
  return std::memcpy(to, from, size);
}
#define memcpy observed_copy
// INSERT_ACTUAL_SENDER_BODY
#undef memcpy

static void reset_counts() {
  allocations = copies = sends = setups = setup_argument = 0;
  actual_allocation = sent_size = sent_pid = sent_flags = 0;
  event_count = 0;
  replace_table_after_allocation = 0;
  expected_setup = 0;
  expected_payload = table_a;
  send_result = -1;
}
static void fill_table(unsigned char *table, unsigned char pattern, unsigned count) {
  for (unsigned i = 0; i < 120000; ++i)
    table[i] = static_cast<unsigned char>(pattern + (i % 31));
  table[1] = static_cast<unsigned char>(count);
}
static void verify_frame(unsigned count, unsigned pid) {
  const unsigned size = count * 399u + 0x1A4u;
  const unsigned copied = count * 399u + 0x194u;
  require(actual_allocation == size && sent_size == size, "allocation-size");
  require(copies == 1 && sends == 1, "copy-send-count");
  require(sent_pid == pid, "send-destination");
  require(sent_flags == 1, "send-flags");
  const unsigned expected_events[] = {expected_setup ? 1u : 2u,
                                      expected_setup ? 2u : 3u,
                                      expected_setup ? 3u : 4u, 4u};
  const unsigned event_length = expected_setup ? 4u : 3u;
  require(event_count == event_length, "send-event-count");
  for (unsigned i = 0; i < event_length; ++i)
    require(event_trace[i] == expected_events[i], "setup-copy-send-order");
  require(frame_storage[0] == 0x0D && frame_storage[1] == 0x0F,
          "opcode-bytes");
  require(frame_storage[2] == 0xA5 && frame_storage[3] == 0xA5,
          "allocator-bytes-2-3");
  for (unsigned i = 4; i < 8; ++i)
    require(frame_storage[i] == 0xFF, "signed-faction-bytes");
  for (unsigned i = 8; i < 12; ++i)
    require(frame_storage[i] == 0, "timestamp-bytes");
  for (unsigned i = 12; i < 16; ++i)
    require(frame_storage[i] == 0xA5, "allocator-bytes-c-f");
  require(std::memcmp(frame_storage + 0x10, expected_payload, copied) == 0,
          "payload-copy");
}
static void run_count_case(unsigned count, int slot, unsigned pid) {
  reset_counts();
  fill_table(table_a, 0x31, count);
  NetWinDialog.faction_records_ = reinterpret_cast<NetWinFactionRecord *>(table_a);
  expected_payload = table_a;
  expected_allocation = count * 399u + 0x1A4u;
  expected_copy = count * 399u + 0x194u;
  NetWinPlayerId = NetWinOtherPlayerId = 7;
  send_result = 0;
  send_faction_data(slot);
  require(setups == 0, "present-table-no-rebuild");
  verify_frame(count, pid);
}

int main() {
  for (int slot = -8; slot < 8; ++slot)
    NetDaemonNet.process_slots_[slot].pid_ = 0x5000u + static_cast<unsigned>(slot + 8);

  // A same-player mismatch is a complete no-op despite an available table.
  reset_counts();
  fill_table(table_a, 0x31, 1);
  NetWinDialog.faction_records_ = reinterpret_cast<NetWinFactionRecord *>(table_a);
  expected_payload = table_a;
  NetWinPlayerId = 7; NetWinOtherPlayerId = 8;
  expected_allocation = 1 * 399u + 0x1A4u;
  expected_copy = 1 * 399u + 0x194u;
  send_faction_data(-1);
  require(allocations == 0 && copies == 0 && sends == 0 && setups == 0,
          "same-player-gate");

  run_count_case(0, -2, 0);   // Every negative value broadcasts, not only -1.
  run_count_case(2, 3, NetDaemonNet.process_slots_[3].pid_);
  run_count_case(255, -1, 0);

  // A null table requests exactly one rebuild before count/allocation/send.
  reset_counts();
  fill_table(table_a, 0x31, 1);
  NetWinDialog.faction_records_ = nullptr;
  NetWinDialog.setup_records = reinterpret_cast<NetWinFactionRecord *>(table_a);
  expected_setup = 1;
  expected_payload = table_a;
  expected_allocation = 1 * 399u + 0x1A4u;
  expected_copy = 1 * 399u + 0x194u;
  NetWinPlayerId = NetWinOtherPlayerId = 7;
  send_faction_data(0);
  require(setups == 1 && setup_argument == 0, "optional-rebuild");
  verify_frame(1, NetDaemonNet.process_slots_[0].pid_);

  // mem_get can intervene between the captured count and the later source load.
  reset_counts();
  fill_table(table_a, 0x31, 2);
  fill_table(table_b, 0x72, 9);
  NetWinDialog.faction_records_ = reinterpret_cast<NetWinFactionRecord *>(table_a);
  expected_payload = table_b;
  expected_allocation = 2 * 399u + 0x1A4u;
  expected_copy = 2 * 399u + 0x194u;
  replace_table_after_allocation = 1;
  NetWinPlayerId = NetWinOtherPlayerId = 7;
  send_result = -1;  // Sender ignores this provider result.
  send_faction_data(1);
  require(setups == 0, "present-table-no-rebuild");
  verify_frame(2, NetDaemonNet.process_slots_[1].pid_);
  require(expected_payload == reinterpret_cast<unsigned char *>(NetWinDialog.faction_records_),
          "post-allocation-table-reload");
  return 0;
}
'''


def _host_source(body: str) -> str:
    declarations = installed_packet_declarations((
        "NetWinFactionRecord", "NetWinFactionDataPrefix",
        "NetWinFactionDataPacketView",
    ))
    return (HOST_PREFIX.replace("// INSERT_ACTUAL_PACKET_DECLARATIONS", declarations)
            .replace("// INSERT_ACTUAL_SENDER_BODY", body))


def _compile_run(compiler: str, source: str, tmp_path: Path, name: str):
    cpp = tmp_path / f"{name}.cpp"
    binary = tmp_path / name
    cpp.write_text(source, encoding="utf-8")
    built = subprocess.run(
        [compiler, "-std=c++17", "-O0", "-Wall", "-Wextra", str(cpp), "-o", str(binary)],
        capture_output=True, text=True, timeout=60,
    )
    assert built.returncode == 0, built.stdout + built.stderr
    return subprocess.run([str(binary)], capture_output=True, text=True, timeout=10)


def _replace_once(text: str, old: str, new: str) -> str:
    assert text.count(old) == 1, old
    return text.replace(old, new, 1)


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
def test_actual_sender_prefix_sizes_gates_and_destinations(compiler_name, tmp_path):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(f"{compiler_name} unavailable")
    source = SOURCE.read_text(encoding="utf-8")
    body = extract_definition(source, SIGNATURE)
    result = _compile_run(compiler, _host_source(body), tmp_path,
                          f"faction_prefix_{compiler_name.replace('+', 'p')}")
    assert result.returncode == 0, result.stdout + result.stderr


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
@pytest.mark.parametrize(("case", "diagnostic"), (
    ("gate", "same-player-gate"),
    ("rebuild", "optional-rebuild-guard"),
    ("allocation", "allocation-size"),
    ("copy", "faction-copy-size"),
    ("header", "faction-prefix-before-payload"),
    ("broadcast", "send-destination"),
    ("slot_pid", "send-destination"),
    ("reload", "faction-payload-view"),
))
def test_compile_success_semantic_mutants_hit_exact_diagnostics(
    compiler_name, case, diagnostic, tmp_path,
):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(f"{compiler_name} unavailable")
    body = extract_definition(SOURCE.read_text(encoding="utf-8"), SIGNATURE)
    if case == "gate":
        mutant = _replace_once(body,
            "if (NetWinPlayerId == NetWinOtherPlayerId)",
            "if (NetWinPlayerId != NetWinOtherPlayerId)")
    elif case == "rebuild":
        mutant = _replace_once(body,
            "if (NetWinDialog.faction_records_ == 0)",
            "if (NetWinDialog.faction_records_ != 0)")
    elif case == "allocation":
        mutant = _replace_once(body, "records * 3 + 0x1A4", "records * 3 + 0x1A3")
    elif case == "copy":
        mutant = _replace_once(body, "records * 3 + 0x194", "records * 3 + 0x193")
    elif case == "header":
        mutant = _replace_once(body, "packet.set_faction(-1);", "packet.set_faction(0);")
    elif case == "broadcast":
        mutant = _replace_once(body, "if (slot_id < 0)", "if (slot_id == -1)")
    elif case == "slot_pid":
        mutant = _replace_once(body, "process_slots_[slot_id].pid_",
                               "process_slots_[slot_id + 1].pid_")
    else:
        capture = "        const uint8_t count = NetWinDialog.faction_records_[0].record_count_;"
        mutant = _replace_once(body, capture,
            "        NetWinFactionRecord *const initial_records = NetWinDialog.faction_records_;\n" + capture)
        mutant = _replace_once(mutant,
            "memcpy(packet.payload(), NetWinDialog.faction_records_,",
            "memcpy(packet.payload(), initial_records,")
    result = _compile_run(compiler, _host_source(mutant), tmp_path,
                          f"faction_{case}_{compiler_name.replace('+', 'p')}")
    assert result.returncode == 97, result.stdout + result.stderr
    assert f"CONTROL_FAIL:{diagnostic}" in result.stderr
