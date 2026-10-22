"""Host controls for the actual mapped DeletionList owner search."""

from __future__ import annotations

import shutil
import subprocess
from pathlib import Path

import pytest
from match_recovery.source import read_file
from test_net_hotseat_owner_actual_source_oct6 import extract_definition


ADDRESS = 0x0052DD10
SOURCE_RELATIVE = Path("src/deletionlist.cpp")
SIGNATURE = "int DeletionList::find_owner_slot(int owner)"
SYMBOL = "?find_owner_slot@DeletionList@@QAEHH@Z"
CATALOGUE_NAME = "?UNK2@DeletionList@@QAEHH@Z"
SPAN = (0x0052DD10, 0x0052DD4B)


def _root() -> Path:
    for parent in Path(__file__).resolve().parents:
        if (parent / SOURCE_RELATIVE).is_file():
            return parent
    raise AssertionError("cannot locate OpenSMACX checkout")


def _actual_body() -> str:
    root = _root()
    path = root / SOURCE_RELATIVE
    source = path.read_text(encoding="utf-8")
    records = [row for row in read_file(path) if row.address == ADDRESS]
    assert len(records) == 1
    record = records[0]
    assert record.name == CATALOGUE_NAME
    assert (record.symbol or record.name) == SYMBOL
    assert record.image_spans == (SPAN,)
    assert Path(record.path).resolve() == path.resolve()
    assert source.count("// ORIGINAL: 0x0052DD10") == 1
    assert source.count("// symbol    " + SYMBOL) == 1
    body = extract_definition(source, SIGNATURE)
    assert body in record.region
    return body


HARNESS_PREFIX = r'''#include <assert.h>
#include <new>
#include <stdint.h>
#include "deletionlist.h"

struct GameDataStateStorageType {
    struct {
        struct {
            uint32_t enable_mask_;
        } lock_masks_;
    } fields_;
} GameDataStateStorage;

struct NetDaemon {
    static DeletionList *start_without_constructor(void *storage) {
        return new (storage) DeletionList(DeletionList::Uninitialized());
    }
};

'''

HARNESS_SUFFIX = r'''
static void set_record(DeletionList &list, int slot,
                       unsigned char acknowledgements, int owner) {
    list.records_[slot].acknowledgements_ = acknowledgements;
    list.records_[slot].owner_ = owner;
}

int main() {
    union Storage {
        void *pointer_alignment;
        double double_alignment;
        char bytes[sizeof(DeletionList)];
    } storage;
    DeletionList &list = *NetDaemon::start_without_constructor(storage.bytes);
    int i;

    // A zero byte mask makes every entry fail the occupied-bit predicate.
    GameDataStateStorage.fields_.lock_masks_.enable_mask_ = 0;
    for (i = 0; i < 24; ++i) set_record(list, i, 0x7F, 41);
    assert(list.find_owner_slot(41) == -1);

    // The mask is narrowed to one byte: its high bit still distinguishes
    // occupied records from entries that have acknowledged that faction.
    GameDataStateStorage.fields_.lock_masks_.enable_mask_ = 0x80000080u;
    for (i = 0; i < 24; ++i) set_record(list, i, 0x80, 100 + i);
    set_record(list, 0, 0x00, 17);
    set_record(list, 23, 0x7F, 23);
    assert(list.find_owner_slot(17) == 0);
    assert(list.find_owner_slot(23) == 23);
    assert(list.find_owner_slot(999) == -1);

    // Match only an occupied record with the requested owner, then retain
    // the original first-match behavior when duplicate owners exist.
    GameDataStateStorage.fields_.lock_masks_.enable_mask_ = 0x05;
    for (i = 0; i < 24; ++i) set_record(list, i, 0x05, -10 - i);
    set_record(list, 3, 0x01, 42);
    set_record(list, 4, 0x04, 99);
    assert(list.find_owner_slot(99) == 4);
    assert(list.find_owner_slot(42) == 3);
    assert(list.find_owner_slot(1000) == -1);
    set_record(list, 7, 0x00, 42);
    assert(list.find_owner_slot(42) == 3);

    // If all 24 records are occupied but none belongs to the owner, the
    // method returns -1 (distinct from find_free_slot's slot-zero fallback).
    GameDataStateStorage.fields_.lock_masks_.enable_mask_ = 0xFF;
    for (i = 0; i < 24; ++i) set_record(list, i, 0x00, 200 + i);
    assert(list.find_owner_slot(999) == -1);
    return 0;
}
'''


def _mutate(body: str, old: str, new: str) -> str:
    assert body.count(old) == 1, (old, body.count(old))
    return body.replace(old, new, 1)


def _mutant(body: str, kind: str) -> str:
    if kind == "omit_last_record":
        return _mutate(body, "slot < 24", "slot < 23")
    if kind == "wrong_exhaustion_value":
        return _mutate(body, "return -1;", "return 0;")
    if kind == "reverse_occupied_predicate":
        return _mutate(body, "(record.acknowledgements_ & mask) != mask",
                       "(record.acknowledgements_ & mask) == mask")
    raise AssertionError("unknown mutant: " + kind)


def _compile_run(tmp_path: Path, compiler: str, body: str, label: str):
    source = tmp_path / (label + ".cpp")
    binary = tmp_path / label
    source.write_text(HARNESS_PREFIX + body + HARNESS_SUFFIX, encoding="utf-8")
    result = subprocess.run(
        [compiler, "-std=c++11", "-O0", "-iquote", str(_root() / "src"),
         str(source), "-o", str(binary)],
        capture_output=True, text=True, timeout=45, check=False,
    )
    if result.returncode:
        return result, None
    try:
        ran = subprocess.run([str(binary)], capture_output=True, text=True,
                             timeout=15, check=False)
    except subprocess.TimeoutExpired:
        return result, "timeout"
    return result, ran


@pytest.fixture(params=["g++", "clang++"])
def compiler(request):
    resolved = shutil.which(request.param)
    if resolved is None:
        pytest.skip(request.param + " is not installed")
    return resolved


def test_actual_owner_cursor_boundaries_and_masks(tmp_path, compiler):
    built, ran = _compile_run(tmp_path, compiler, _actual_body(), "cursor_actual")
    assert built.returncode == 0, built.stderr
    assert ran != "timeout", "actual bounded scan unexpectedly timed out"
    assert ran.returncode == 0, ran.stderr


@pytest.mark.parametrize("kind", [
    "omit_last_record", "wrong_exhaustion_value", "reverse_occupied_predicate",
])
def test_compiling_damage_mutants_fail_controls(tmp_path, compiler, kind):
    mutant = _mutant(_actual_body(), kind)
    built, ran = _compile_run(tmp_path, compiler, mutant, "cursor_" + kind)
    assert built.returncode == 0, (kind, built.stderr)
    assert ran == "timeout" or ran.returncode != 0, kind

