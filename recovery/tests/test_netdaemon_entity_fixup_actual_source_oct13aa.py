"""Actual-source controls for entity and synchronization vehicle fixup.

The fixture is a bounded host test of the original wire fields and Veh members;
it is not the game's native ABI or runtime.
"""
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
TASK = "netdaemon-entity-fixup-closure-oct13aa"
BASE = "fde3ebab5d25fe23e5b2ebfb641b6ad0e4049195"
PIN = "01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf"


def _history(row):
    return tuple((item.kind, item.key, item.prose,
                  tuple(line for _, line in item.lines)) for item in row.origin)


def test_current_netdaemon_annotations_are_continuous_with_immutable_fde3_base():
    task = TaskStore(ROOT).read(TASK)
    assert task.base_revision == BASE
    path = ROOT / "src/netdaemon.cpp"
    before_text = subprocess.check_output(
        ["git", "show", f"{task.base_revision}:src/netdaemon.cpp"],
        cwd=ROOT, text=True, timeout=30)
    before = {row.address: row for row in read_text(before_text, path)}
    current = {row.address: row for row in read_text(path.read_text(), path)}
    assert set(current) == set(before)
    for address, old in before.items():
        new = current[address]
        for item in fields(old):
            if item.name not in {"line", "region", "origin"}:
                assert getattr(new, item.name) == getattr(old, item.name)
        assert _history(new) == _history(old)


def test_actual_fixup_views_and_pinned_original_producers():
    source = (ROOT / "src/netdaemon.cpp").read_text()
    body = extract(source, "int NetDaemon::fixup_message(char * message)")
    for token in ("EntityCommandPrefixReader", "SyncHeaderPrefixReader",
                  "SyncVehiclePayloadView", "convert(", "store_entity_id",
                  "store_argument_2", "store_waypoint_x", "store_order"):
        assert token in body, token
    assert "reinterpret_cast<unsigned short *>(message)" not in body
    assert "reinterpret_cast<int *>(msg" not in body
    assert "struct EntityCommandPrefix" in source
    assert "sizeof(EntityCommandPrefix) == 0x14" in source
    assert "offsetof(EntityCommandPrefix, sequence_) == 0x0C" in source
    assert "offsetof(EntityCommandPrefix, entity_id_) == 0x10" in source
    assert "offsetof(Veh, order)" in source
    assert "offsetof(Veh, waypoint_x)" in source
    assert "static_assert(sizeof(Veh) == 0x34" in (ROOT / "src/veh.h").read_text()

    explicit = os.environ.get("OPENSMACX_IMAGE")
    image_path = Path(explicit) if explicit else ROOT.parents[2] / ".opensmacx/game/terranx_original.exe"
    if not image_path.is_file():
        pytest.skip("pinned original unavailable; set OPENSMACX_IMAGE")
    raw = image_path.read_bytes()
    assert hashlib.sha256(raw).hexdigest() == PIN
    image = PeImage(raw, PIN)
    from capstone import Cs, CS_ARCH_X86, CS_MODE_32
    decoder = Cs(CS_ARCH_X86, CS_MODE_32)
    fixup = list(decoder.disasm(image.read(Span(0x534330, 0x5343FC)), 0x534330))
    assert len(fixup) == 73
    assert fixup[0].address == 0x534330 and fixup[-1].address + fixup[-1].size == 0x5343FC

    # The three ordinary producers establish the sequence/entity offsets and
    # the distinct base-text payload. Sync producer 532E00 is separately pinned
    # by the existing prefix test; its 0x24-byte prefix precedes Veh fields.
    producer_spans = ((0x592E10, 0x592E70), (0x592E70, 0x592EDA),
                      (0x592EE0, 0x592F4C))
    decoded = {}
    for start, end in producer_spans:
        instructions = list(decoder.disasm(image.read(Span(start, end)), start))
        assert instructions and instructions[0].address == start
        assert instructions[-1].address + instructions[-1].size == end
        decoded.update({item.address: (item.mnemonic, item.op_str.lower()) for item in instructions})
    assert decoded[0x592E30] == ("mov", "edx, dword ptr [0x93e8bc]")
    assert decoded[0x592E39] == ("mov", "eax, dword ptr [ebp + 0xc]")
    assert decoded[0x592E2D] == ("mov", "ecx, dword ptr [ebp + 0x10]")
    assert decoded[0x592E9F] == ("lea", "edx, [ebp - 0x18]")
    assert decoded[0x592EA7] == ("call", "0x645460")
    assert decoded[0x592F28] == ("mov", "dword ptr [ebp - 0x10], eax")


HARNESS = r'''#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>

#pragma pack(push, 1)
/* ACTUAL VEH DECLARATION */
#pragma pack(pop)
/* ACTUAL ENTITY AND SYNC VIEW DECLARATIONS */
static_assert(sizeof(Veh) == 0x34, "actual Veh extent");
static_assert(offsetof(Veh, order) == 0x11, "actual Veh order offset");
static_assert(offsetof(Veh, waypoint_x) == 0x14, "actual Veh waypoint offset");

static int fail(const char *name) {
    std::fprintf(stderr, "entity-fixup: %s\n", name);
    return 98;
}
static unsigned char *callback_packet;
static int callback_mode;
static bool callback_sync;
static int32_t get32(const unsigned char *p, size_t offset);
static int16_t get16(const unsigned char *p, size_t offset);
struct DeletionList {
    int count;
    int results[3];
    int ids[3];
    int owners[3];
    int32_t id_field_before[3];
    int16_t waypoint_before[3];
    int convert(int original_id, int new_owner) {
        if (count >= 3) return 0;
        const int slot = count++;
        ids[slot] = original_id;
        owners[slot] = new_owner;
        id_field_before[slot] = get32(callback_packet, callback_sync ? 0x14 : 0x10);
        waypoint_before[slot] = get16(callback_packet, 0x24 + offsetof(Veh, waypoint_x));
        if (slot == 0 && callback_mode != 0) {
            const int32_t changed_sequence = -17;
            const int16_t changed_waypoint = callback_mode == 2 ? -3 : 7;
            std::memcpy(callback_packet + 0x0C, &changed_sequence, sizeof changed_sequence);
            std::memcpy(callback_packet + 0x24 + offsetof(Veh, waypoint_x),
                        &changed_waypoint, sizeof changed_waypoint);
            if (callback_mode == 3) {
                const int8_t changed_order = 0;
                std::memcpy(callback_packet + 0x24 + offsetof(Veh, order),
                            &changed_order, sizeof changed_order);
            }
        }
        return results[slot];
    }
};
struct NetDaemon {
    DeletionList deletion_list_;
    int fixup_message(char * message);
};
/* ACTUAL FIXUP BODY */

static void put16(unsigned char *p, size_t offset, uint16_t value) {
    std::memcpy(p + offset, &value, sizeof value);
}
static void put32(unsigned char *p, size_t offset, int32_t value) {
    std::memcpy(p + offset, &value, sizeof value);
}
static int32_t get32(const unsigned char *p, size_t offset) {
    int32_t value;
    std::memcpy(&value, p + offset, sizeof value);
    return value;
}
static int16_t get16(const unsigned char *p, size_t offset) {
    int16_t value;
    std::memcpy(&value, p + offset, sizeof value);
    return value;
}
static int8_t get8(const unsigned char *p, size_t offset) {
    int8_t value;
    std::memcpy(&value, p + offset, sizeof value);
    return value;
}
int main() {
    const uint16_t recognized[] = {0x2401, 0x2402, 0x2403, 0x2404,
                                   0x2405, 0x2406, 0x2408, 0x2413};
    for (unsigned k = 0; k < sizeof recognized / sizeof *recognized; ++k) {
        for (int result_index = 0; result_index < 3; ++result_index) {
            unsigned char storage[1 + 0x58 + 8];
            std::memset(storage, 0xA5, sizeof storage);
            unsigned char *p = storage + 1;
            put16(p, 0, recognized[k]); put32(p, 0x0C, -0x1234567);
            put32(p, 0x10, 0x12345678);
            unsigned char expected[sizeof storage - 1];
            std::memcpy(expected, p, sizeof expected);
            NetDaemon daemon = {};
            daemon.deletion_list_.results[0] = result_index == 0 ? -91 : (result_index == 1 ? 0 : 73);
            callback_packet = p; callback_mode = 0; callback_sync = false;
            const int actual = daemon.fixup_message(reinterpret_cast<char *>(p));
            const int expected_result = daemon.deletion_list_.results[0];
            put32(expected, 0x10, expected_result);
            if (actual != (expected_result < 0 ? 1 : 0)) return fail("ordinary-return-gate");
            if (daemon.deletion_list_.count != 1 ||
                daemon.deletion_list_.ids[0] != 0x12345678 ||
                daemon.deletion_list_.owners[0] != -0x1234567 ||
                daemon.deletion_list_.id_field_before[0] != 0x12345678)
                return fail("ordinary-convert-arguments");
            if (get32(p, 0x10) != expected_result) return fail("ordinary-converted-store");
            if (storage[0] != 0xA5 ||
                std::memcmp(p, expected, sizeof expected) != 0)
                return fail("ordinary-neighbor-preservation");
        }
    }

    const uint16_t ignored[] = {0x0000, 0x2102, 0x2400, 0x2407, 0x2409, 0xFFFF};
    for (unsigned k = 0; k < sizeof ignored / sizeof *ignored; ++k) {
        unsigned char storage[1 + 0x58 + 8]; std::memset(storage, 0xA5, sizeof storage);
        unsigned char *p = storage + 1; put16(p, 0, ignored[k]);
        put32(p, 0x0C, 91); put32(p, 0x10, 92);
        unsigned char expected[sizeof storage - 1];
        std::memcpy(expected, p, sizeof expected);
        NetDaemon daemon = {}; callback_packet = p; callback_mode = 0; callback_sync = false;
        if (daemon.fixup_message(reinterpret_cast<char *>(p)) != 0 ||
            daemon.deletion_list_.count != 0 || get32(p, 0x10) != 92 ||
            storage[0] != 0xA5 || std::memcmp(p, expected, sizeof expected) != 0)
            return fail("unrecognized-gate");
    }

    // Wrong inner opcode must not call the converter or touch the serialized
    // entity / payload. This is a distinct opcode gate under outer 0x2101.
    {
        unsigned char storage[1 + 0x58 + 8]; std::memset(storage, 0xA5, sizeof storage);
        unsigned char *p = storage + 1; put16(p, 0, 0x2101); put16(p, 0x10, 0x10);
        put32(p, 0x0C, 4); put32(p, 0x14, 5);
        unsigned char expected[sizeof storage - 1];
        std::memcpy(expected, p, sizeof expected);
        NetDaemon daemon = {}; callback_packet = p; callback_mode = 0; callback_sync = true;
        if (daemon.fixup_message(reinterpret_cast<char *>(p)) != 0 || daemon.deletion_list_.count != 0 ||
            storage[0] != 0xA5 || std::memcmp(p, expected, sizeof expected) != 0)
            return fail("sync-inner-opcode-gate");
    }

    for (int gate = 0; gate < 2; ++gate) {
        unsigned char storage[1 + 0x58 + 8]; std::memset(storage, 0xA5, sizeof storage);
        unsigned char *p = storage + 1; put16(p, 0, 0x2101); put16(p, 0x10, 0x11);
        put32(p, 0x0C, 8); put32(p, 0x14, 9);
        const int8_t order = gate == 0 ? 0 : 1;
        const int16_t waypoint = gate == 0 ? 7 : -1;
        std::memcpy(p + 0x24 + offsetof(Veh, order), &order, sizeof order);
        std::memcpy(p + 0x24 + offsetof(Veh, waypoint_x), &waypoint, sizeof waypoint);
        NetDaemon daemon = {}; daemon.deletion_list_.results[0] = 12;
        callback_packet = p; callback_mode = 0; callback_sync = true;
        if (daemon.fixup_message(reinterpret_cast<char *>(p)) != 0 ||
            daemon.deletion_list_.count != 1)
            return fail(gate == 0 ? "sync-order-gate" : "sync-negative-waypoint-gate");
    }

    // A conversion callback can mutate live sequence and Veh fields. The
    // second conversion must use reread sequence/waypoint values, then store
    // the low 16 bits and clear order only for the FFFF sentinel.
    for (int first_negative = 0; first_negative < 2; ++first_negative) {
        unsigned char storage[1 + 0x58 + 8]; std::memset(storage, 0xA5, sizeof storage);
        unsigned char *p = storage + 1; put16(p, 0, 0x2101); put16(p, 0x10, 0x11);
        put32(p, 0x0C, 0x12345678); put32(p, 0x14, 0x2222);
        const int8_t order = 1; const int16_t waypoint = 5;
        std::memcpy(p + 0x24 + offsetof(Veh, order), &order, sizeof order);
        std::memcpy(p + 0x24 + offsetof(Veh, waypoint_x), &waypoint, sizeof waypoint);
        const int first_result = first_negative ? -1 : 0x3456;
        unsigned char expected[sizeof storage - 1];
        std::memcpy(expected, p, sizeof expected);
        NetDaemon daemon = {}; daemon.deletion_list_.results[0] = first_result;
        daemon.deletion_list_.results[1] = 0x1234FFFF;
        callback_packet = p; callback_mode = 1; callback_sync = true;
        if (daemon.fixup_message(reinterpret_cast<char *>(p)) != 0 || daemon.deletion_list_.count != 2)
            return fail("sync-clear-sentinel-route");
        if (daemon.deletion_list_.ids[0] != 0x2222 || daemon.deletion_list_.owners[0] != 0x12345678 ||
            daemon.deletion_list_.id_field_before[0] != 0x2222 ||
            daemon.deletion_list_.ids[1] != 7 || daemon.deletion_list_.owners[1] != -17 ||
            daemon.deletion_list_.waypoint_before[1] != 7)
            return fail("sync-reread-arguments");
        if (get32(p, 0x14) != first_result || get16(p, 0x24 + offsetof(Veh, waypoint_x)) != -1 ||
            get8(p, 0x24 + offsetof(Veh, order)) != 0)
            return fail("sync-sentinel-store-order");
        put32(expected, 0x0C, -17);
        put32(expected, 0x14, first_result);
        put16(expected, 0x24 + offsetof(Veh, waypoint_x), 0xFFFF);
        expected[0x24 + offsetof(Veh, order)] = 0;
        if (storage[0] != 0xA5 || std::memcmp(p, expected, sizeof expected) != 0)
            return fail("sync-neighbor-preservation");
    }

    // FFFF in the high half is not the low-16 sentinel; the waypoint is
    // updated but order remains unchanged. A negative live waypoint skips the
    // second conversion, while a callback-cleared order also skips it.
    for (int mode = 0; mode < 3; ++mode) {
        unsigned char storage[1 + 0x58 + 8]; std::memset(storage, 0xA5, sizeof storage);
        unsigned char *p = storage + 1; put16(p, 0, 0x2101); put16(p, 0x10, 0x11);
        put32(p, 0x0C, 33); put32(p, 0x14, 44);
        const int8_t order = 1; const int16_t waypoint = mode == 0 ? 0 : (mode == 1 ? -4 : 6);
        std::memcpy(p + 0x24 + offsetof(Veh, order), &order, sizeof order);
        std::memcpy(p + 0x24 + offsetof(Veh, waypoint_x), &waypoint, sizeof waypoint);
        unsigned char expected[sizeof storage - 1];
        std::memcpy(expected, p, sizeof expected);
        NetDaemon daemon = {}; daemon.deletion_list_.results[0] = 55;
        daemon.deletion_list_.results[1] = mode == 0 ? static_cast<int>(0xFFFF1234u) : 9;
        callback_packet = p; callback_mode = mode == 0 ? 0 : (mode == 1 ? 2 : 3);
        callback_sync = true;
        if (daemon.fixup_message(reinterpret_cast<char *>(p)) != 0)
            return fail("sync-nonsentinel-return");
        const int expected_calls = mode == 0 ? 2 : 1;
        if (daemon.deletion_list_.count != expected_calls) return fail("sync-live-gate-reread");
        if (mode == 0) {
            if (get16(p, 0x24 + offsetof(Veh, waypoint_x)) != 0x1234 ||
                get8(p, 0x24 + offsetof(Veh, order)) != 1)
                return fail("sync-low16-not-sentinel");
        }
        put32(expected, 0x14, 55);
        if (mode == 0) {
            put16(expected, 0x24 + offsetof(Veh, waypoint_x), 0x1234);
        } else {
            put32(expected, 0x0C, -17);
            put16(expected, 0x24 + offsetof(Veh, waypoint_x), mode == 1 ? 0xFFFD : 7);
            if (mode == 2) expected[0x24 + offsetof(Veh, order)] = 0;
        }
        if (storage[0] != 0xA5 || std::memcmp(p, expected, sizeof expected) != 0)
            return fail("sync-gate-neighbor-preservation");
    }

    return 0;
}
'''


def _actual_declarations():
    net = (ROOT / "src/netdaemon.cpp").read_text()
    veh_text = (ROOT / "src/veh.h").read_text()
    fragments = [
        extract(net, "struct EntityCommandPrefix {") + ";",
        extract(net, "class EntityCommandPrefixReader {") + ";",
        extract(net, "class EntityCommandPrefixWriter {") + ";",
        extract(net, "struct SyncHeaderPrefix {") + ";",
        extract(net, "class SyncHeaderPrefixReader {") + ";",
        extract(net, "class SyncHeaderPrefixWriter {") + ";",
        extract(net, "class SyncVehiclePayloadView {") + ";",
        extract(veh_text, "struct Veh {") + ";",
    ]
    return "\n".join(fragments[:-1]), fragments[-1]


def _fixture_source(damage="none"):
    net = (ROOT / "src/netdaemon.cpp").read_text()
    entity, veh = _actual_declarations()
    body = extract(net, "int NetDaemon::fixup_message(char * message)")
    if damage == "wrong-entity-offset":
        before = "offsetof(EntityCommandPrefix, entity_id_)"
        assert before in entity
        entity = entity.replace(before, "offsetof(EntityCommandPrefix, sequence_)", 1)
    elif damage == "stale-sync-sequence":
        before = "        const int first = deletion_list_.convert(\n            header.argument_2(), header.signed_sequence());"
        assert body.count(before) == 1
        body = body.replace(before,
                            "        const int32_t captured_sequence = header.signed_sequence();\n" +
                            before.replace("header.signed_sequence()", "captured_sequence"), 1)
        assert body.count("header.signed_sequence()") == 2
        end = body.rfind("header.signed_sequence()")
        body = body[:end] + "captured_sequence" + body[end + len("header.signed_sequence()"):]
    elif damage == "signed-waypoint-gate":
        before = "if (waypoint < 0) return 0;"
        assert body.count(before) == 1
        body = body.replace(before, "if (waypoint <= 0) return 0;", 1)
    elif damage == "wrong-low16-sentinel":
        before = "if (converted_bits != 0xFFFFu) return 0;"
        assert body.count(before) == 1
        body = body.replace(before, "if (converted_bits == 0xFFFFu) return 0;", 1)
    elif damage == "premature-order-clear":
        before = "        if (converted_bits != 0xFFFFu) return 0;\n\n        vehicle.store_order(0);"
        after = "        vehicle.store_order(0);\n        if (converted_bits != 0xFFFFu) return 0;"
        assert body.count(before) == 1
        body = body.replace(before, after, 1)
    elif damage == "accept-2407":
        before = "    case 0x2406:\n    case 0x2408:"
        assert body.count(before) == 1
        body = body.replace(before, "    case 0x2406:\n    case 0x2407:\n    case 0x2408:", 1)
    return (HARNESS.replace("/* ACTUAL ENTITY AND SYNC VIEW DECLARATIONS */", entity)
            .replace("/* ACTUAL VEH DECLARATION */", veh)
            .replace("/* ACTUAL FIXUP BODY */", body))


@pytest.mark.parametrize("compiler", [name for name in ("g++", "clang++") if shutil.which(name)])
def test_actual_fixup_wire_gates_conversion_callbacks_and_vehicle_sentinel(tmp_path, compiler):
    source = _fixture_source()
    executable = tmp_path / "entity-fixup"
    built = subprocess.run([compiler, "-std=c++11", "-Wall", "-Wextra", "-Werror",
                            "-pedantic", "-x", "c++", "-", "-o", str(executable)],
                           input=source, text=True, capture_output=True, timeout=45)
    assert built.returncode == 0, built.stdout + built.stderr
    result = subprocess.run([str(executable)], text=True, capture_output=True, timeout=10)
    assert result.returncode == 0, result.stderr


@pytest.mark.parametrize("compiler", [name for name in ("g++", "clang++") if shutil.which(name)])
@pytest.mark.parametrize(("damage", "diagnostic"), [
    ("wrong-entity-offset", "ordinary-convert-arguments"),
    ("stale-sync-sequence", "sync-reread-arguments"),
    ("signed-waypoint-gate", "sync-live-gate-reread"),
    ("wrong-low16-sentinel", "sync-sentinel-store-order"),
    ("premature-order-clear", "sync-low16-not-sentinel"),
    ("accept-2407", "unrecognized-gate"),
])
def test_actual_fixup_named_mutants_compile_then_fail_98(tmp_path, compiler, damage, diagnostic):
    source = _fixture_source(damage)
    executable = tmp_path / ("entity-fixup-" + damage)
    built = subprocess.run([compiler, "-std=c++11", "-Wall", "-Wextra", "-Werror",
                            "-pedantic", "-x", "c++", "-", "-o", str(executable)],
                           input=source, text=True, capture_output=True, timeout=45)
    assert built.returncode == 0, built.stdout + built.stderr
    result = subprocess.run([str(executable)], text=True, capture_output=True, timeout=10)
    assert result.returncode == 98, result.stderr
    assert "entity-fixup: " + diagnostic in result.stderr
