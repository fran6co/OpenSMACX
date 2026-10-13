"""Actual MessageCommandView field reads; not game ABI/runtime acceptance."""
from dataclasses import fields
import hashlib
import os
from pathlib import Path
import shutil
import subprocess

import pytest

from match_recovery.model import Span
from match_recovery.pe_coff import PeImage
from match_recovery.source import read_text
from match_recovery.tasks import TaskStore
from test_netdaemon_transfer_modal import extract


ROOT = Path(__file__).resolve().parents[2]
TASK = "netdaemon-diplomacy-wire-closure-oct13z"
PIN = "01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf"
BASE = "c0790d76332187138ea7f273fd9948807908a6bf"


def lesson_history(row):
    return tuple((lesson.kind, lesson.key, lesson.prose,
                  tuple(line for _, line in lesson.lines))
                 for lesson in row.origin)


def test_current_netdaemon_identities_claims_and_history_match_immutable_task_base():
    task = TaskStore(ROOT).read(TASK)
    assert task.base_revision == BASE
    path = ROOT / "src/netdaemon.cpp"
    baseline_text = subprocess.check_output(
        ["git", "show", f"{task.base_revision}:src/netdaemon.cpp"],
        cwd=ROOT, text=True, timeout=30)
    baseline = {row.address: row for row in read_text(baseline_text, path)}
    current = {row.address: row for row in read_text(path.read_text(), path)}
    assert set(current) == set(baseline)
    for address, old in baseline.items():
        new = current[address]
        for item in fields(old):
            if item.name not in {"line", "region", "origin"}:
                assert getattr(new, item.name) == getattr(old, item.name)
        assert lesson_history(new) == lesson_history(old)


def test_actual_record_view_contract_and_pinned_sender_receiver_anchors():
    source = (ROOT / "src/netdaemon.cpp").read_text()
    record = extract(source, "struct MessageDataRecord {")
    view = extract(source, "class MessageCommandView {")
    assert "int16_t opcode_;" in record and "unsigned char unresolved_2_[2];" in record
    assert "int payload_1C_;" in record
    for method, offset in (("opcode", "opcode_"), ("payload_1C", "payload_1C_")):
        body = extract(view, f"{'uint16_t' if method == 'opcode' else 'int'} {method}() const")
        assert f"offsetof(MessageDataRecord, {offset})" in body
        assert "std::memcpy(" in body
    assert "explicit MessageCommandView(const char *bytes)" in view
    handler = extract(source, "static bool netdaemon_handle_table_h(char *message)")
    assert "const MessageCommandView packet(message);" in handler
    assert "packet.opcode()" in handler and "packet.payload_1C()" in handler
    assert "reinterpret_cast<int *>(message +" not in handler
    assert "reinterpret_cast<unsigned short *>(message)" not in handler

    explicit = os.environ.get("OPENSMACX_IMAGE")
    image_path = Path(explicit) if explicit else ROOT.parents[2] / ".opensmacx/game/terranx_original.exe"
    if not image_path.is_file():
        pytest.skip("pinned original unavailable; set OPENSMACX_IMAGE")
    raw = image_path.read_bytes()
    assert hashlib.sha256(raw).hexdigest() == PIN
    image = PeImage(raw, PIN)
    from capstone import Cs, CS_ARCH_X86, CS_MODE_32
    decoder = Cs(CS_ARCH_X86, CS_MODE_32)
    sender = {item.address: (item.mnemonic, item.op_str.lower())
              for item in decoder.disasm(image.read(Span(0x592EE0, 0x592F4C)), 0x592EE0)}
    assert sender[0x592EE3] == ("sub", "esp, 0x20")
    assert sender[0x592EF0] == ("mov", "word ptr [ebp - 0x20], ax")
    assert sender[0x592F32] == ("push", "0x20")
    assert sender[0x592F3D] == ("call", "0x532940")

    receiver = {item.address: (item.mnemonic, item.op_str.lower())
                for item in decoder.disasm(image.read(Span(0x536DA5, 0x5373E7)), 0x536DA5)}
    assert receiver[0x536DC4] == ("jmp", "dword ptr [eax*4 + 0x5389ac]")
    assert receiver[0x536DED] == ("call", "0x55b870")
    assert receiver[0x5371CC] == ("cmp", "word ptr [ebx], 0x444c")
    assert receiver[0x5373AA] == ("jmp", "0x538266")


VIEW_CONTROL = r'''
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>
static const unsigned char *read_base;
static int read_count;
static size_t read_offsets[32], read_widths[32];
static void observed_read(void *destination, const void *source, size_t width) {
    if (read_count < 32) {
        read_offsets[read_count] = (const unsigned char *)source - read_base;
        read_widths[read_count] = width;
    }
    ++read_count;
    memcpy(destination, source, width);
}
@@ACTUAL_WIRE_TYPES@@
static void store_bytes(unsigned char *where, const void *value, size_t width) {
    memcpy(where, value, width);
}
static void store_word(unsigned char *packet, size_t offset, int value) {
    store_bytes(packet + offset, &value, sizeof value);
}
static bool require(bool condition, const char *label) {
    if (!condition) { fprintf(stderr, "FAIL: %s\n", label); return false; }
    return true;
}
int main() {
    unsigned char storage[0x31] = {};
    unsigned char *unaligned = storage + 1;
    read_base = unaligned;
    uint16_t opcode = 0x444F;
    uint32_t sequence = 0xDEADBEEFu;
    store_bytes(unaligned, &opcode, sizeof opcode);
    unaligned[2] = 0xA5; unaligned[3] = 0x5A;
    store_word(unaligned, 4, 7);
    store_word(unaligned, 8, (int)0x80000001u);
    store_bytes(unaligned + 12, &sequence, sizeof sequence);
    store_word(unaligned, 0x10, -11);
    store_word(unaligned, 0x14, 0x12345678);
    store_word(unaligned, 0x18, -13);
    store_word(unaligned, 0x1C, 0x12345);
    MessageCommandView packet((const char *)unaligned);
    // Exclude constructor reads in the cached-accessor damage control so the
    // later accessor observations remain directly comparable.
    read_count = 0;
    if (!require(packet.opcode() == 0x444F, "opcode-read")) return 98;
    if (!require(packet.faction() == 7, "faction-read")) return 98;
    if (!require((uint32_t)packet.timestamp_bits() == 0x80000001u, "timestamp-read")) return 98;
    if (!require(packet.sequence() == sequence, "sequence-read")) return 98;
    if (!require(packet.payload_10() == -11, "payload-10-read")) return 98;
    if (!require(packet.payload_14() == 0x12345678, "payload-14-read")) return 98;
    if (!require(packet.payload_18() == -13, "payload-18-read")) return 98;
    if (!require(packet.payload_1C() == 0x12345, "payload-1C-read")) return 98;
    const size_t expected_offsets[] = {0, 4, 8, 12, 0x10, 0x14, 0x18, 0x1C};
    const size_t expected_widths[] = {2, 4, 4, 4, 4, 4, 4, 4};
    const int initial_reads = @@INITIAL_READS@@;
    if (!require(read_count == initial_reads, "one-field-per-read-count")) return 98;
    for (int i = 0; i < initial_reads; ++i) {
        if (!require(read_offsets[i] == expected_offsets[i] &&
                     read_widths[i] == expected_widths[i],
                     "unaligned-field-extent")) return 98;
    }
    store_word(unaligned, 0x1C, -1234567);
    if (!require(packet.payload_1C() == -1234567, "payload-1C-fresh-read")) return 98;
    opcode = 0x4411;
    store_bytes(unaligned, &opcode, sizeof opcode);
    if (!require(packet.opcode() == 0x4411, "opcode-callback-reload")) return 98;
    if (!require(read_count == @@FINAL_READS@@ &&
                 read_offsets[@@FINAL_PAYLOAD_INDEX@@] == @@FINAL_PAYLOAD_OFFSET@@ &&
                 read_widths[@@FINAL_PAYLOAD_INDEX@@] == @@FINAL_PAYLOAD_WIDTH@@ &&
                 read_offsets[@@FINAL_OPCODE_INDEX@@] == 0 && read_widths[@@FINAL_OPCODE_INDEX@@] == 2,
                 "fresh-accessor-extents")) return 98;
    return 0;
}
'''


def actual_wire_types(damage):
    source = (ROOT / "src/netdaemon.cpp").read_text()
    record = extract(source, "struct MessageDataRecord {") + ";"
    view = extract(source, "class MessageCommandView {") + ";"
    method = extract(view, "int payload_1C() const")
    if damage == "wrong-offset":
        old = "offsetof(MessageDataRecord, payload_1C_)"
        assert method.count(old) == 1
        view = view.replace(method, method.replace(old, "offsetof(MessageDataRecord, payload_18_)"), 1)
    elif damage == "cached-payload":
        constructor = """explicit MessageCommandView(const char *bytes)
        : bytes_(reinterpret_cast<const unsigned char *>(bytes)) {}"""
        assert constructor in view
        view = view.replace(constructor, """explicit MessageCommandView(const char *bytes)
        : bytes_(reinterpret_cast<const unsigned char *>(bytes)), cached_payload_1C_(0) {
        std::memcpy(&cached_payload_1C_, bytes_ + offsetof(MessageDataRecord, payload_1C_), sizeof cached_payload_1C_);
    }""", 1)
        old_method = """int payload_1C() const {
        int value;
        std::memcpy(&value, bytes_ + offsetof(MessageDataRecord, payload_1C_), sizeof value);
        return value;
    }"""
        assert old_method in view
        view = view.replace(old_method, "int payload_1C() const { return cached_payload_1C_; }", 1)
        field = "    const unsigned char *bytes_;"
        assert field in view
        view = view.replace(field, field + "\n    int cached_payload_1C_;", 1)
    view = view.replace("std::memcpy(", "observed_read(")
    return record + "\n" + view


@pytest.mark.parametrize("compiler", ["g++", "clang++"])
@pytest.mark.parametrize("damage", ["none", "wrong-offset", "cached-payload"])
def test_actual_wire_view_unaligned_extents_and_callback_rereads(tmp_path, compiler, damage):
    if not shutil.which(compiler):
        pytest.skip(f"{compiler} unavailable")
    source = VIEW_CONTROL.replace("@@ACTUAL_WIRE_TYPES@@", actual_wire_types(damage))
    if damage == "cached-payload":
        source = source.replace("@@INITIAL_READS@@", "7")
        source = source.replace("@@FINAL_READS@@", "8")
        source = source.replace("@@FINAL_PAYLOAD_INDEX@@", "7")
        source = source.replace("@@FINAL_PAYLOAD_OFFSET@@", "0")
        source = source.replace("@@FINAL_PAYLOAD_WIDTH@@", "2")
        source = source.replace("@@FINAL_OPCODE_INDEX@@", "7")
    else:
        source = source.replace("@@INITIAL_READS@@", "8")
        source = source.replace("@@FINAL_READS@@", "10")
        source = source.replace("@@FINAL_PAYLOAD_INDEX@@", "8")
        source = source.replace("@@FINAL_PAYLOAD_OFFSET@@", "0x1C")
        source = source.replace("@@FINAL_PAYLOAD_WIDTH@@", "4")
        source = source.replace("@@FINAL_OPCODE_INDEX@@", "9")
    executable = tmp_path / "wire_view"
    built = subprocess.run([compiler, "-std=c++03", "-Wall", "-Wextra", "-Werror",
                            "-x", "c++", "-", "-o", str(executable)],
                           input=source, text=True, capture_output=True, timeout=30)
    assert built.returncode == 0, built.stderr
    result = subprocess.run([str(executable)], text=True, capture_output=True, timeout=10)
    if damage == "none":
        assert result.returncode == 0, result.stderr
    else:
        assert result.returncode == 98, result.stderr
        expected = "FAIL: payload-1C-read" if damage == "wrong-offset" else "FAIL: payload-1C-fresh-read"
        assert expected in result.stderr
