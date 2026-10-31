"""Pinned callback-visible Dialog semantics, exercised through actual source.

The host double does not qualify native layout, deleting ABI or linked selection.
"""
from pathlib import Path
import shutil
import subprocess

import pytest

from match_recovery.includes import COMMENTS_OR_STRINGS
from match_recovery.source import read_file

ROOT = Path(__file__).resolve().parents[2]


def definition(signature):
    source = (ROOT / "src/dialog.cpp").read_text()
    masked = COMMENTS_OR_STRINGS.sub(
        lambda m: "".join("\n" if c == "\n" else " " for c in m[0]), source)
    start = masked.index(signature)
    opening = masked.index("{", start)
    depth = 0
    for i in range(opening, len(masked)):
        depth += (masked[i] == "{") - (masked[i] == "}")
        if depth == 0:
            return source[start:i + 1]
    raise AssertionError("unterminated actual definition")


HARNESS = r'''
#include <cassert>
#include <cstddef>
#include <cstdint>
static int destroyed_payloads, destroyed_entries, last_entry, uncleared_payload;
struct StringStructPayload {
    virtual ~StringStructPayload() { ++destroyed_payloads; }
};
struct StringStructEntry {
    int id;
    StringStructEntry *next;
    StringStructPayload *payload;
    StringStructPayload *string_payload() { return payload; }
    ~StringStructEntry() {
        ++destroyed_entries; last_entry = id;
        uncleared_payload += payload != 0;
    }
};
struct Heap { int init(size_t size); };
struct StringList {
    StringStructEntry *head_, *current_;
    int entry_count_, current_position_;
    Heap *allocator_;
    void on_remove(void *payload);
};
struct Dialog {
    StringList item_list_;
    Heap heap_;
    uint32_t field_1C_;
    int selected_position_;
    int init();
    int id_to_pos(int id);
    void set_selected_id(int id);
};
static Dialog *active;
static StringStructEntry *replacement, *seed_after_heap;
static int change_size, heap_result, heap_calls;
static size_t heap_size;
void StringList::on_remove(void *) {
    if (replacement) { head_ = replacement; replacement = 0; }
    if (change_size) { active->field_1C_ = 64; change_size = 0; }
}
int Heap::init(size_t size) {
    ++heap_calls; heap_size = size;
    if (seed_after_heap) {
        active->item_list_.head_ = seed_after_heap;
        active->item_list_.entry_count_ = 1;
        seed_after_heap = 0;
    }
    return heap_result;
}
static void reset(Dialog &d, uint32_t requested) {
    d.item_list_ = StringList{0, 0, 0, 9, &d.heap_};
    d.field_1C_ = requested; d.selected_position_ = -3;
    active = &d; replacement = seed_after_heap = 0;
    destroyed_payloads = destroyed_entries = last_entry = uncleared_payload = 0;
    change_size = heap_result = heap_calls = 0; heap_size = 0;
}
__ACTUAL_BODIES__
int main() {
    Dialog d;
    // The original re-reads size after callbacks and clears the then-current
    // head, not the entry that supplied the callback's payload.
    reset(d, 0);
    StringStructPayload *sentinel = new StringStructPayload;
    StringStructEntry *old = new StringStructEntry{11, 0, new StringStructPayload};
    replacement = new StringStructEntry{22, 0, sentinel};
    d.item_list_.head_ = old; d.item_list_.entry_count_ = 1; change_size = 1;
    assert(d.init() == 0);
    assert(heap_calls == 1 && heap_size == 64);
    assert(destroyed_payloads == 1 && destroyed_entries == 1);
    assert(last_entry == 22 && uncleared_payload == 0);
    assert(d.item_list_.allocator_ == &d.heap_ && !d.item_list_.head_);
    old->payload = 0; delete old; delete sentinel;

    // Heap initialization may publish a new list; its teardown has the same
    // callback-visible current-head rule as the first teardown.
    reset(d, 32);
    sentinel = new StringStructPayload;
    old = new StringStructEntry{33, 0, new StringStructPayload};
    seed_after_heap = old;
    replacement = new StringStructEntry{44, 0, sentinel};
    assert(d.init() == 0);
    assert(heap_calls == 1 && heap_size == 32);
    assert(destroyed_payloads == 1 && destroyed_entries == 1);
    assert(last_entry == 44 && uncleared_payload == 0);
    old->payload = 0; delete old; delete sentinel;

    reset(d, static_cast<uint32_t>(-1));
    StringStructEntry untouched{55, 0, 0};
    d.item_list_.head_ = &untouched; d.item_list_.entry_count_ = 1;
    assert(d.init() == 1 && heap_calls == 0 && destroyed_entries == 0);
    assert(d.item_list_.head_ == &untouched && d.item_list_.current_position_ == 9);
    reset(d, 0);
    assert(d.init() == 0 && heap_calls == 0 && d.item_list_.allocator_ == 0);
    reset(d, 32); heap_result = 7;
    assert(d.init() == 4 && heap_calls == 1 && d.item_list_.allocator_ == &d.heap_);

    reset(d, 0);
    StringStructEntry first{11, 0, 0}, second{22, 0, 0}; first.next = &second;
    d.item_list_.head_ = &first; d.item_list_.entry_count_ = 2;
    d.set_selected_id(22);
    assert(d.selected_position_ == 1 && d.item_list_.current_ == &second);
    d.set_selected_id(99);
    assert(d.selected_position_ == 2 && d.item_list_.current_ == 0);
    d.item_list_.head_ = 0; d.item_list_.current_position_ = 7;
    d.set_selected_id(11); assert(d.selected_position_ == 7);
    d.item_list_.head_ = &first; d.item_list_.entry_count_ = 0;
    d.set_selected_id(11);
    assert(d.selected_position_ == 0 && d.item_list_.current_ == &first);
    d.item_list_.entry_count_ = -1;
    d.set_selected_id(11); assert(d.selected_position_ == 0);
}
'''


@pytest.mark.parametrize("damage", [None, "cached_head", "cached_size", "cursor"])
def test_original_dialog_callback_and_selection_semantics(tmp_path, damage):
    records = read_file(ROOT / "src/dialog.cpp")
    assert sum(r.address == 0x609150 for r in records) == 1
    assert sum(r.address == 0x6099D0 for r in records) == 1
    body = definition("int Dialog::init() {")
    body += "\n" + definition("int Dialog::id_to_pos(int id) {")
    body += "\n" + definition("void Dialog::set_selected_id(int id) {")
    if damage == "cached_head":
        anchor = "list->head_->payload = 0;"
        assert body.count(anchor) == 3
        damaged = body.replace(anchor, "entry->payload = 0;")
    elif damage == "cached_size":
        anchor = "    const int32_t requested = static_cast<int32_t>(field_1C_);\n"
        assert body.count(anchor) == 1
        damaged = body.replace(anchor, "")
        damaged = damaged.replace("    if (static_cast<int32_t>(field_1C_) < 0) {",
                                  anchor + "    if (requested < 0) {")
    elif damage == "cursor":
        anchor = "selected_position_ = id_to_pos(id);"
        assert body.count(anchor) == 1
        damaged = body.replace(anchor, "selected_position_ = id_to_pos(id) + 1;")
    else:
        damaged = body
    assert damage is None or damaged != body
    compiler = shutil.which("clang++") or shutil.which("g++")
    assert compiler, "host C++ compiler required"
    cpp = tmp_path / "dialog.cpp"
    exe = tmp_path / "dialog"
    cpp.write_text(HARNESS.replace("__ACTUAL_BODIES__", damaged))
    built = subprocess.run([compiler, "-std=c++11", "-Wall", "-Wextra", "-Werror",
                            str(cpp), "-o", str(exe)], capture_output=True, text=True)
    assert built.returncode == 0, built.stdout + built.stderr
    result = subprocess.run([str(exe)], capture_output=True, text=True)
    assert (result.returncode == 0) == (damage is None), result.stdout + result.stderr
