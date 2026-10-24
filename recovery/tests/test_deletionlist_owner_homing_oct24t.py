"""Actual-source host controls for the seven homed DeletionList methods.

Native read_file selects each complete mapped owner from its production TU;
the harness compiles those exact definitions with the actual deletionlist.h.
The service doubles expose only the live game mask, vehicle position and log
boundary. This is not a native ABI or full networking/runtime proof.
"""
from pathlib import Path
import os
import shutil
import subprocess

import pytest

from match_recovery.source import read_file


ROOT = Path(os.environ.get("OPENSMACX_SOURCE_ROOT", Path(__file__).resolve().parents[2]))
HEADER = ROOT / "src/deletionlist.h"
METHODS = (
    ("src/deletionlist.cpp", 0x0052DCA0, "DeletionList::DeletionList()"),
    ("src/deletionlist.cpp", 0x0052DCD0, "void DeletionList::clear(int index)"),
    ("src/deletionlist.cpp", 0x0052DCF0, "int DeletionList::find_free_slot()"),
    ("src/deletionlist.cpp", 0x0052DD10, "int DeletionList::find_owner_slot(int owner)"),
    ("src/deletionlist.cpp", 0x0052DD50, "void DeletionList::ack(int owner, int faction)"),
    ("src/deletionlist.cpp", 0x0052DDC0, "int DeletionList::convert(int original_id, int new_owner)"),
    ("src/deletionlist.cpp", 0x0052DE70, "int DeletionList::add_deletion(int veh_id)"),
)


def _actual_region(relative_path, address):
    records = [record for record in read_file(ROOT / relative_path)
               if record.address == address]
    assert len(records) == 1, (relative_path, hex(address), len(records))
    return records[0].region


def _definition(source, signature):
    start = source.index(signature)
    opening = source.index("{", start)
    depth = 0
    for end in range(opening, len(source)):
        if source[end] == "{":
            depth += 1
        elif source[end] == "}":
            depth -= 1
            if depth == 0:
                return source[start:end + 1]
    raise AssertionError("unterminated production function: " + signature)


FIXTURE_PREFIX = r'''
#include <cstdint>
using std::int16_t;
using std::uint8_t;
using std::uint32_t;
#include "__DELETIONLIST_HEADER__"
#include <cassert>
#include <cstring>
#include <new>
#include <string>
#include <vector>

struct LockMaskFixture { uint32_t enable_mask_; };
struct FieldsFixture { LockMaskFixture lock_masks_; };
struct GameStoreFixture { FieldsFixture fields_; };
GameStoreFixture GameDataStateStorage = {};
struct VehFixture { int x; int y; };
VehFixture Vehs[2049] = {};
const char StrDelConvertedIndex[] = "converted";
const char StrDeletingVehicle[] = "deleting";
struct LogEvent {
    std::string name;
    int first, second, third;
    LogEvent(const char *label, int a, int b, int c)
        : name(label), first(a), second(b), third(c) {}
    bool operator==(const LogEvent &other) const {
        return name == other.name && first == other.first
            && second == other.second && third == other.third;
    }
};
std::vector<LogEvent> log_events;
void log_say(const char *name, int first, int second, int third) {
    log_events.push_back(LogEvent(name, first, second, third));
}
static void set_mask(uint32_t mask) {
    GameDataStateStorage.fields_.lock_masks_.enable_mask_ = mask;
}
static void initialize_records(DeletionList &list, uint8_t acknowledgements) {
    for (int slot = 0; slot < 24; ++slot) {
        DeletionRecord &record = list.records_[slot];
        record.acknowledgements_ = acknowledgements;
        record.owner_ = -1000 - slot;
        record.count_ = 0;
        for (int index = 0; index < 25; ++index)
            record.vehicle_ids_[index] = static_cast<int16_t>(-200 - index);
    }
}
'''


MAIN = r'''
static void test_constructor_preserves_unwritten_bytes() {
    alignas(DeletionList) unsigned char backing[sizeof(DeletionList)];
    std::memset(backing, 0xA5, sizeof(backing));
    DeletionList *list = new (backing) DeletionList();
    assert(list->current_owner_ == 0);
    const unsigned char *bytes = backing;
    for (int slot = 0; slot < 24; ++slot) {
        const int offset = slot * 0x3C;
        assert(list->records_[slot].acknowledgements_ == 0xFF);
        assert(list->records_[slot].count_ == 0);
        for (int byte = 1; byte < 8; ++byte) assert(bytes[offset + byte] == 0xA5);
        for (int byte = 0x0A; byte < 0x3C; ++byte) assert(bytes[offset + byte] == 0xA5);
    }
    for (int byte = 0x5A0; byte < 0x5A4; ++byte) assert(bytes[byte] == 0);
    list->~DeletionList();
}

static void test_clear_free_and_owner_lookups() {
    DeletionList list;
    initialize_records(list, 0);
    set_mask(0x05);
    list.records_[0].acknowledgements_ = 0x01;
    list.records_[1].acknowledgements_ = 0x05;
    list.records_[2].acknowledgements_ = 0x05;
    assert(list.find_free_slot() == 1);
    list.records_[1].acknowledgements_ = 0x01;
    list.records_[2].acknowledgements_ = 0x05;
    assert(list.find_free_slot() == 2);
    for (int slot = 0; slot < 24; ++slot) list.records_[slot].acknowledgements_ = 0;
    assert(list.find_free_slot() == 0); // Original full-list fallback.
    set_mask(0);
    assert(list.find_free_slot() == 0);

    set_mask(0x05);
    list.records_[0].owner_ = 73;
    list.records_[0].acknowledgements_ = 0x05; // Fully acknowledged: not occupied.
    list.records_[1].owner_ = 73;
    list.records_[1].acknowledgements_ = 0x01; // Missing active bit 2: occupied.
    list.records_[2].owner_ = 74;
    list.records_[2].acknowledgements_ = 0x00;
    assert(list.find_owner_slot(73) == 1);
    assert(list.find_owner_slot(74) == 2);
    assert(list.find_owner_slot(75) == -1);

    list.records_[1].owner_ = 73;
    list.records_[1].count_ = 1;
    list.records_[1].vehicle_ids_[0] = 18;
    list.clear(1);
    assert(list.records_[1].acknowledgements_ == 0xFF);
    assert(list.records_[1].count_ == 0);
    assert(list.records_[1].owner_ == 73 && list.records_[1].vehicle_ids_[0] == 18);
}

static void test_ack_keeps_count_and_clears_completed_record() {
    DeletionList list;
    initialize_records(list, 0xFF);
    set_mask(0x05);
    DeletionRecord &record = list.records_[3];
    record.owner_ = 42;
    record.acknowledgements_ = 0;
    record.count_ = 2;
    record.vehicle_ids_[0] = 11;
    record.vehicle_ids_[1] = 19;
    list.ack(42, 0);
    assert(record.acknowledgements_ == 0x01);
    assert(record.count_ == 2 && record.vehicle_ids_[0] == 11 && record.vehicle_ids_[1] == 19);
    list.ack(42, 2);
    assert(record.acknowledgements_ == 0xFF);
    assert(record.count_ == 2 && record.vehicle_ids_[0] == 11 && record.vehicle_ids_[1] == 19);
    list.ack(99, 0);
    assert(record.acknowledgements_ == 0xFF && record.count_ == 2);
}

static void test_add_deletion_existing_and_new_records() {
    set_mask(0x05);
    log_events.clear();
    DeletionList existing;
    initialize_records(existing, 0xFF);
    existing.current_owner_ = 123;
    existing.records_[4].owner_ = 123;
    existing.records_[4].acknowledgements_ = 0;
    existing.records_[4].count_ = 1;
    existing.records_[4].vehicle_ids_[0] = 8;
    Vehs[17].x = 4;
    Vehs[17].y = 9;
    assert(existing.add_deletion(17) == 1);
    assert(existing.records_[4].owner_ == 123 && existing.records_[4].acknowledgements_ == 0);
    assert(existing.records_[4].count_ == 2 && existing.records_[4].vehicle_ids_[1] == 17);
    assert(log_events == std::vector<LogEvent>({LogEvent("deleting", 17, 4, 9)}));

    log_events.clear();
    DeletionList fresh;
    initialize_records(fresh, 0xFF);
    fresh.current_owner_ = 88;
    Vehs[21].x = -3;
    Vehs[21].y = 6;
    assert(fresh.add_deletion(21) == 0);
    assert(fresh.records_[0].owner_ == 88 && fresh.records_[0].acknowledgements_ == 0);
    assert(fresh.records_[0].count_ == 1 && fresh.records_[0].vehicle_ids_[0] == 21);
    assert(log_events == std::vector<LogEvent>({LogEvent("deleting", 21, -3, 6)}));
}

static void test_convert_owner_walk_remap_delete_and_logs() {
    DeletionList remapped;
    initialize_records(remapped, 0xFF);
    set_mask(0x05);
    remapped.current_owner_ = 4;
    remapped.records_[0].owner_ = 2;
    remapped.records_[0].acknowledgements_ = 0;
    remapped.records_[0].count_ = 2;
    remapped.records_[0].vehicle_ids_[0] = 3;
    remapped.records_[0].vehicle_ids_[1] = 25;
    remapped.records_[1].owner_ = 3;
    remapped.records_[1].acknowledgements_ = 0x05; // Freed owner is skipped.
    remapped.records_[1].count_ = 1;
    remapped.records_[1].vehicle_ids_[0] = 7;
    remapped.records_[2].owner_ = 4;
    remapped.records_[2].acknowledgements_ = 0;
    remapped.records_[2].count_ = 1;
    remapped.records_[2].vehicle_ids_[0] = 5;
    log_events.clear();
    assert(remapped.convert(20, 1) == 18);
    assert(log_events == std::vector<LogEvent>({LogEvent("converted", 20, 18, 0)}));

    DeletionList deleted;
    initialize_records(deleted, 0xFF);
    deleted.current_owner_ = 2;
    deleted.records_[0].owner_ = 2;
    deleted.records_[0].acknowledgements_ = 0;
    deleted.records_[0].count_ = 2;
    deleted.records_[0].vehicle_ids_[0] = 12;
    deleted.records_[0].vehicle_ids_[1] = 3;
    log_events.clear();
    assert(deleted.convert(12, 1) == -1);
    assert(log_events == std::vector<LogEvent>({LogEvent("converted", 12, -1, 0)}));

    log_events.clear();
    assert(deleted.convert(20, 2) == 20); // Owner range is empty; no message.
    assert(log_events.empty());
}

int main() {
    test_constructor_preserves_unwritten_bytes();
    test_clear_free_and_owner_lookups();
    test_ack_keeps_count_and_clears_completed_record();
    test_add_deletion_existing_and_new_records();
    test_convert_owner_walk_remap_delete_and_logs();
}
'''


def _harness():
    definitions = []
    for relative_path, address, signature in METHODS:
        region = _actual_region(relative_path, address)
        definitions.append(_definition(region, signature))
    prefix = FIXTURE_PREFIX.replace("__DELETIONLIST_HEADER__", HEADER.as_posix())
    return prefix + "\n".join(definitions) + MAIN


def test_deletionlist_homed_methods_actual_source_gcc_and_clang(tmp_path):
    program = _harness()
    compilers = [shutil.which(name) for name in ("g++", "clang++")]
    compilers = [compiler for compiler in compilers if compiler]
    if not compilers:
        pytest.skip("GCC/Clang host compilers unavailable")
    for compiler in compilers:
        source = tmp_path / (Path(compiler).name + ".cpp")
        binary = tmp_path / Path(compiler).name
        source.write_text(program, encoding="utf-8")
        built = subprocess.run(
            [compiler, "-std=c++17", "-Wall", "-Wextra", "-Werror", "-pedantic",
             str(source), "-o", str(binary)], capture_output=True, text=True, timeout=60)
        assert built.returncode == 0, built.stdout + built.stderr
        ran = subprocess.run([str(binary)], capture_output=True, text=True, timeout=20)
        assert ran.returncode == 0, ran.stdout + ran.stderr
