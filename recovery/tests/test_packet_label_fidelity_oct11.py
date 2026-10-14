"""Actual-source ordering controls; not full-game or undefined-path proof."""
from dataclasses import fields
import hashlib
import os
from pathlib import Path
import shutil
import subprocess

import pytest
from match_recovery.source import read_text
from match_recovery.model import Span
from match_recovery.pe_coff import PeImage
from test_net_hotseat_owner_actual_source_oct6 import (
    compile_and_run, extract_definition, installed_packet_declarations,
)

ROOT = Path(__file__).resolve().parents[2]
BASE = "e6e4f71b4fc8afb7656c25c2550546fa50e926ea"
PIN = "01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf"
SIGNATURE = "void __cdecl send_faction_data(int slot_id)"


def test_all_original_annotation_fields_and_claims_preserved():
    for relative in ("src/netwin.cpp", "src/mapwin.cpp"):
        old = subprocess.check_output(["git", "show", BASE + ":" + relative],
                                      cwd=ROOT, text=True)
        current = (ROOT / relative).read_text()
        before = {r.address: r for r in read_text(old, relative)}
        after = {r.address: r for r in read_text(current, relative)}
        assert before.keys() == after.keys()
        for address, row in after.items():
            prior = before[address]
            for field in fields(row):
                if field.name not in ("line", "region", "origin"):
                    assert getattr(row, field.name) == getattr(prior, field.name), (
                        relative, hex(address), field.name)
            assert [(x.kind, x.key, x.prose, tuple(t for _, t in x.lines))
                    for x in row.origin] == [
                (x.kind, x.key, x.prose, tuple(t for _, t in x.lines))
                for x in prior.origin]


def packet_fixture(body):
    declarations = installed_packet_declarations((
        "NetWinFactionRecord", "NetWinFactionDataPrefix",
        "NetWinFactionDataPacketView",
    ))
    return r"""
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <cstdlib>
#include <cstdio>
#define __cdecl
""" + declarations + r"""
static unsigned char frame_storage[4096];
static unsigned allocations, copies, sends, setups, recipient, allocated_size;
struct Dialog {
  NetWinFactionRecord *faction_records_;
  void setup_faction_data(int v) { assert(v == 0); ++setups; }
} NetWinDialog;
struct Net {
  uint32_t local_player_id_ = 1, host_player_id_ = 1;
  uint32_t local_player_id() const { return local_player_id_; }
  uint32_t host_player_id() const { return host_player_id_; }
  void send(void *p, unsigned n, unsigned to, int reliable) {
    assert(copies == 1 && p == frame_storage && n == allocated_size && reliable == 1);
    recipient = to; ++sends;
  }
};
struct Daemon : Net {
  struct Slot { unsigned pid_; } process_slots_[8];
} NetDaemonNet;
static void *mem_get(unsigned n) {
  assert(n <= sizeof(frame_storage)); allocated_size = n;
  std::memset(frame_storage, 0xA5, sizeof(frame_storage)); ++allocations;
  return frame_storage;
}
static void control_fail(const char *name) {
  std::fprintf(stderr, "CONTROL_FAIL:%s\n", name);
  std::exit(97);
}
static void *observed_copy(void *to, const void *from, unsigned n) {
  // Observe initialized fields before reading payload. Reserved bytes have
  // no claimed native value; the poison only detects widened product stores.
  unsigned short opcode; unsigned slot, zero;
  std::memcpy(&opcode, frame_storage, 2);
  std::memcpy(&slot, frame_storage + 4, 4);
  std::memcpy(&zero, frame_storage + 8, 4);
  if (opcode != 0xF0D || slot != 0xFFFFFFFFU || zero != 0 ||
      frame_storage[2] != 0xA5 || frame_storage[3] != 0xA5)
    control_fail("faction-prefix-before-payload");
  for (unsigned i = 12; i < 16; ++i)
    if (frame_storage[i] != 0xA5) control_fail("faction-prefix-before-payload");
  if (to != frame_storage + 16 || from != NetWinDialog.faction_records_)
    control_fail("faction-payload-view");
  if (n + 16 != allocated_size) control_fail("faction-copy-size");
  ++copies;
  return std::memcpy(to, from, n);
}
#define memcpy observed_copy
""" + body + r"""
#undef memcpy
int main() {
  NetWinFactionRecord records[4] = {};
  records[0].record_count_ = 2;
  NetWinDialog.faction_records_ = records;
  NetDaemonNet.process_slots_[3].pid_ = 99;
  send_faction_data(-1);
  assert(allocations == 1 && copies == 1 && sends == 1 && recipient == 0 && setups == 0);
  assert(allocated_size == 2 * 399 + 0x1A4);
  allocations = copies = sends = 0;
  send_faction_data(3);
  assert(allocations == 1 && copies == 1 && sends == 1 && recipient == 99);
  allocations = copies = sends = 0;
  NetDaemonNet.host_player_id_ = 2; send_faction_data(-1);
  assert(allocations == 0 && copies == 0 && sends == 0);
}
"""


def test_actual_header_is_written_before_payload_and_old_order_mutant_fails(tmp_path):
    body = extract_definition((ROOT / "src/netwin.cpp").read_text(), SIGNATURE)
    copy_statement = (
        "        memcpy(packet.payload(), NetWinDialog.faction_records_,\n"
        "               records * 3 + 0x194);"
    )
    assert body.count(copy_statement) == 1
    mutant = body.replace(copy_statement + "\n", "", 1)
    anchor = "        packet.set_opcode(0xF0D);"
    assert mutant.count(anchor) == 1
    mutant = mutant.replace(anchor, copy_statement + "\n" + anchor, 1)
    compilers = [shutil.which(c) for c in ("g++", "clang++") if shutil.which(c)]
    assert compilers
    for n, compiler in enumerate(compilers):
        result = compile_and_run(compiler, packet_fixture(body), tmp_path, f"packet-{n}")
        assert result.returncode == 0, result.stderr
        result = compile_and_run(compiler, packet_fixture(mutant), tmp_path, f"wrong-order-{n}")
        assert result.returncode == 97
        assert "CONTROL_FAIL:faction-prefix-before-payload" in result.stderr


def test_original_native_order_is_independently_pinned():
    path = Path(os.environ.get("OPENSMACX_IMAGE",
                               ROOT / ".opensmacx/game/terranx_original.exe"))
    if not path.is_file():
        pytest.skip("external original image unavailable")
    capstone = pytest.importorskip("capstone")
    raw = path.read_bytes()
    assert hashlib.sha256(raw).hexdigest() == PIN
    image = PeImage(raw, PIN)
    decoder = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
    packet = {i.address: i for i in decoder.disasm(
        image.read(Span(0x47B020, 0x47B0E1)), 0x47B020)}
    stores = {
        0x47B07A: "word ptr [esi], 0xf0d",
        0x47B07F: "dword ptr [esi + 4], 0xffffffff",
        0x47B086: "dword ptr [esi + 8], 0",
    }
    assert tuple(stores) == tuple(sorted(stores))
    assert max(stores) < 0x47B098
    for address, operands in stores.items():
        assert packet[address].mnemonic == "mov"
        assert packet[address].op_str == operands
    sender = {
        0x47B023: ("mov", "eax, dword ptr [0x93d4f0]"),
        0x47B028: ("mov", "ecx, dword ptr [0x93d4f4]"),
        0x47B032: ("jne", "0x47b0dd"),
        0x47B043: ("mov", "ecx, 0x80a6f8"),
        0x47B048: ("call", "0x481fd0"),
        0x47B056: ("mov", "al, byte ptr [ecx + 1]"),
        0x47B06B: ("call", "0x5d4510"),
        0x47B079: ("push", "eax"),
        0x47B093: ("lea", "edx, [esi + 0x10]"),
        0x47B098: ("call", "0x645930"),
        0x47B0A8: ("jge", "0x47b0bc"),
        0x47B0B3: ("call", "0x630080"),
        0x47B0C9: ("mov", "edx, dword ptr [ecx*4 + 0x93d51c]"),
        0x47B0D8: ("call", "0x630080"),
    }
    for address, (mnemonic, operands) in sender.items():
        assert packet[address].mnemonic == mnemonic
        assert packet[address].op_str == operands
    assert packet[0x47B098].mnemonic == "call"
    assert packet[0x47B098].op_str == "0x645930"
    # Whole original caption extent is decoded; no guessed sub-block start.
    caption = {i.address: i for i in decoder.disasm(
        image.read(Span(0x467DE0, 0x4688E0)), 0x467DE0)}
    for address in (0x468470, 0x468479, 0x4684A3, 0x4684B0, 0x468589, 0x4685DA, 0x4685EE):
        assert caption[address].mnemonic == "call"
    assert caption[0x4685A5].mnemonic == "mov"
    assert caption[0x4685AA].mnemonic == "jl"


def test_normal_caption_keeps_unchecked_name_flow_and_prefix_before_queue_read():
    body = extract_definition((ROOT / "src/mapwin.cpp").read_text(),
                              "void MapWin::draw_base_names()")
    start = body.index("else if (base.faction_id_current == faction ||")
    branch = body[start:body.index("int landmark_intensity", start)]
    assert branch.index('strcat(detail, "(")') < branch.index("is_objective(base_id)")
    assert branch.index("label_get(descriptor_label)") < branch.index("base.queue_production_id[0]")
    assert "MaxFacilityNum" not in branch and "MaxVehProtoNum" not in branch
    for guard in ("if (production_name)", "if (name)", "if (descriptor)", "if (objective_name)"):
        assert guard not in branch

