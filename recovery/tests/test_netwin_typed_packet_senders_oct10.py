"""Actual sender bodies and product packet layouts, with bounded host controls.

Reserved bytes have no expected value. These checks do not prove original
uninitialized stack contents, linked preservation, or full-game acceptance.
"""
import shutil
import subprocess
from dataclasses import fields, replace
from pathlib import Path

import pytest
from match_recovery.source import read_text, write
from test_net_hotseat_owner_actual_source_oct6 import (
    compile_and_run, extract_definition, installed_packet_declarations,
    repository_root,
)


BASELINE = "6fa50e5ef0431bb749702030084eea603c803982"
SIGNATURES = (
    "void __cdecl send_all_player_packets(int slot_id)",
    "void __cdecl sub_47b1d0(int faction_id, void *payload)",
    "void __cdecl send_session_packet(int slot_id, int target)",
    "void __cdecl send_session2_packet(int slot_id, int target)",
)


def sender_source():
    return (repository_root() / "src/netwin.cpp").read_text(encoding="utf-8")


def test_original_identities_and_historical_obligations_are_unchanged():
    root = repository_root()
    original = subprocess.run(
        ["git", "show", BASELINE + ":src/netwin.cpp"], cwd=root,
        capture_output=True, text=True, check=True, timeout=20,
    ).stdout
    current = sender_source()
    old_rows = {r.address: r for r in read_text(original, "src/netwin.cpp")}
    rows = {r.address: r for r in read_text(current, "src/netwin.cpp")}
    assert rows.keys() == old_rows.keys()
    assert write(current, read_text(current, "src/netwin.cpp")) == current
    for address, row in rows.items():
        old = old_rows[address]
        # Later bounded tail closure changes only these three complete extents;
        # its production recorder freshly banks new_chat. Keep every other
        # metadata field and all historical lessons strictly baseline-identical.
        tail_ends = {0x481DA0: 0x481E8D, 0x483190: 0x483234, 0x483420: 0x4834C5}
        if address in tail_ends:
            old = replace(old, image_spans=((address, tail_ends[address]),))
        if address == 0x483420:
            old = replace(old, byte_exact=True, semantic=False)
        for field in fields(row):
            if address == 0x481DA0 and field.name == "symbol":
                assert old.symbol == "?fill_player@NetWin@@QAEXPAUNetWinLobbyRow@@H@Z"
                assert row.symbol == "?fill_player@NetWin@@QAEXVNetWinLobbyRowView@@H@Z"
                continue
            if field.name not in {"line", "region", "origin"}:
                assert getattr(row, field.name) == getattr(old, field.name), (
                    hex(address), field.name)
        def lessons(record):
            return [(x.kind, x.key, x.prose, tuple(t for _, t in x.lines))
                    for x in record.origin]
        assert lessons(row) == lessons(old)


def test_overwritten_row_initialization_and_guard_order_remain_explicit():
    source = sender_source()
    full = extract_definition(source, SIGNATURES[0])
    single = extract_definition(source, SIGNATURES[1])
    assert full.index("row->owner_ = 0;") < full.index("if (slot_id != -1")
    assert full.index("if (slot_id != -1") < full.index("memcpy(packet.rows_")
    assert single.index("packet.row_.owner_ = 0;") < single.index("if (faction_id > 0")
    assert single.index("if (faction_id > 0") < single.index("memcpy(&packet.row_")
    for signature in SIGNATURES:
        body = extract_definition(source, signature)
        assert "reinterpret_cast" not in body
        assert "packet{}" not in body and "packet = {}" not in body
        assert "memset" not in body


PRELUDE = r"""
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <vector>
#define __cdecl
"""
DOUBLES = r"""
static_assert(sizeof(NetWinLobbyPacketHeader) == 0x10, "header");
static_assert(sizeof(NetWinLobbyRow) == 0x17C, "row");
static_assert(sizeof(NetWinLobbyFullPacket) == 0xA74, "full");
static_assert(offsetof(NetWinLobbyFullPacket, rows_) == 0x10, "rows");
static_assert(sizeof(NetWinLobbySinglePacket) == 0x18C, "single");
static_assert(offsetof(NetWinLobbySinglePacket, row_) == 0x10, "row payload");
static_assert(sizeof(NetWinSessionOptionsPacket) == 0x28, "options");
static_assert(offsetof(NetWinSessionOptionsPacket, options_) == 0x10, "options payload");
static_assert(sizeof(NetWinSessionBlockPacket) == 0x178, "session");
static_assert(offsetof(NetWinSessionBlockPacket, session_) == 0x10, "session payload");
static_assert(sizeof(NetWinSessionBlock) == 0x168, "session wire block");
static_assert(offsetof(NetWinSessionBlock, scenario_text_wire_tail_) == 0x24,
              "scenario text is a named view in the opaque wire remainder");
static std::vector<unsigned char> sent;
static unsigned recipient, calls;
struct Net {
  void send(void *p, unsigned n, unsigned to, int reliable) {
    assert(reliable == 1);
    sent.resize(n);
    std::memcpy(sent.data(), p, n);
    recipient = to;
    ++calls;
  }
};
struct Daemon : Net {
  uint32_t local_player_id_ = 0, host_player_id_ = 77;
  uint32_t local_player_id() const { return local_player_id_; }
  uint32_t host_player_id() const { return host_player_id_; }
  struct Slot { unsigned pid_; } process_slots_[8];
} NetDaemonNet;
NetWinLobbyRow NetWinLobbyRows[8];
NetWinSessionOptionsState NetWinSessionOptions;
NetWinSessionBlock NetWinSessionData;
static unsigned gate;
static unsigned *NetWinRulesGate = &gate;
static void reset() { sent.clear(); calls = 0; recipient = 0; }
static unsigned word(unsigned at, unsigned width) {
  unsigned v = 0;
  std::memcpy(&v, sent.data() + at, width);
  return v;
}
"""
MAIN = r"""
int main() {
  for (unsigned i = 0; i != 8; ++i) NetDaemonNet.process_slots_[i].pid_ = 0x1200 + i;
  for (unsigned i = 0; i < sizeof(NetWinLobbyRows); ++i)
    reinterpret_cast<unsigned char *>(NetWinLobbyRows)[i] = (i * 7 + 13) & 255;
  for (unsigned i = 0; i < sizeof(NetWinSessionOptions); ++i)
    reinterpret_cast<unsigned char *>(&NetWinSessionOptions)[i] = (i * 9 + 17) & 255;
  for (unsigned i = 0; i < sizeof(NetWinSessionData); ++i)
    reinterpret_cast<unsigned char *>(&NetWinSessionData)[i] = (i * 11 + 19) & 255;
  gate = 0; reset(); send_all_player_packets(-1); assert(calls == 0);
  reset(); send_all_player_packets(-2);
  assert(calls == 1 && recipient == 0 && sent.size() == 0xA74 && word(0,2) == 0x4F04);
  assert(word(4,4) == 0xFFFFFFFFu && word(8,4) == 0);
  assert(!std::memcmp(sent.data()+0x10, &NetWinLobbyRows[1], 0xA64));
  gate = 1; reset(); send_all_player_packets(-1);
  assert(calls == 1 && recipient == 0 && word(0,2) == 0x4F04);
  gate = 0; reset(); send_all_player_packets(3);
  assert(calls == 1 && recipient == 0x1203 && word(0,2) == 0x1F04);
  assert(!std::memcmp(sent.data()+0x10, &NetWinLobbyRows[1], 0xA64));
  unsigned char payload[0x17C];
  for (unsigned i = 0; i < sizeof(payload); ++i) payload[i] = (i * 13 + 31) & 255;
  reset(); sub_47b1d0(0,payload); assert(calls == 0);
  reset(); sub_47b1d0(-1,payload); assert(calls == 0);
  reset(); sub_47b1d0(2,0); assert(calls == 0);
  reset(); sub_47b1d0(2,payload);
  assert(calls == 1 && recipient == 77 && sent.size() == 0x18C);
  assert(word(0,2) == 0x2F04 && word(4,4) == 2 && word(8,4) == 0);
  assert(!std::memcmp(sent.data()+0x10, payload, sizeof(payload)));
  for (int target = -2; target <= 3; ++target) {
    reset(); send_session_packet(123,target);
    assert(calls == 1 && sent.size() == 0x28);
    assert(word(0,2) == unsigned(target < 0 ? 0x4F02 : target == 0 ? 0x2F02 : 0x1F02));
    assert(recipient == unsigned(target < 0 ? 0 : target == 0 ? 77 : 0x1200+target));
    assert(!std::memcmp(sent.data()+0x10, &NetWinSessionOptions, 0x18));
    reset(); send_session2_packet(456,target);
    assert(calls == 1 && sent.size() == 0x178);
    assert(word(0,2) == unsigned(target < 0 ? 0x4F03 : target == 0 ? 0x2F03 : 0x1F03));
    assert(recipient == unsigned(target < 0 ? 0 : target == 0 ? 77 : 0x1200+target));
    assert(!std::memcmp(sent.data()+0x10, &NetWinSessionData, 0x168));
  }
}
"""

MUTATIONS = (
    (0, "slot_id != -1", "slot_id < 0"),
    (0, "sizeof(packet.rows_)", "sizeof(packet.rows_) - 1"),
    (0, "packet.header_.flags_ |= 0x40;", "packet.header_.flags_ |= 0x10;"),
    (1, "faction_id > 0", "faction_id >= 0"),
    (1, "sizeof(packet.row_)", "sizeof(packet.row_) - 1"),
    (2, "if (target != 0)", "if (target == 0)"),
    (3, "sizeof(packet.session_)", "sizeof(packet.session_) - 1"),
)


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
def test_actual_senders_and_behavioral_damage_controls(tmp_path: Path, compiler_name):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip("host compiler unavailable: " + compiler_name)
    source = sender_source()
    bodies = [extract_definition(source, s) for s in SIGNATURES]
    prefix = PRELUDE + installed_packet_declarations() + DOUBLES
    baseline = compile_and_run(compiler, prefix + "\n".join(bodies) + MAIN,
                               tmp_path, "positive")
    assert baseline.returncode == 0, baseline.stderr
    for index, (subject, old, new) in enumerate(MUTATIONS):
        assert bodies[subject].count(old) == 1
        changed = bodies.copy()
        changed[subject] = changed[subject].replace(old, new, 1)
        result = compile_and_run(compiler, prefix + "\n".join(changed) + MAIN,
                                 tmp_path, "damage-" + str(index))
        assert result.returncode != 0, (compiler_name, subject, old)
