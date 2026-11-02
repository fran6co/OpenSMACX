"""Exercise MessageWin's installed over-limit unlink path on the host."""
from __future__ import annotations

from pathlib import Path
import re
import shutil
import subprocess

import pytest

from match_recovery.includes import COMMENTS_OR_STRINGS


ROOT = next(p for p in Path(__file__).resolve().parents
            if (p / "src/messagewin.cpp").is_file())
SOURCE = ROOT / "src/messagewin.cpp"
SIGNATURE = "void MessageWin::add_message(char *text, int item_id)"


def extract_method(source: str, signature: str = SIGNATURE) -> str:
    masked = COMMENTS_OR_STRINGS.sub(
        lambda match: re.sub(r"[^\n]", " ", match[0]), source
    )
    assert masked.count(signature) == 1
    start = masked.index(signature)
    opening = masked.index("{", start + len(signature))
    depth = 0
    for index in range(opening, len(masked)):
        if masked[index] == "{":
            depth += 1
        elif masked[index] == "}":
            depth -= 1
            if depth == 0:
                return source[start:index + 1]
    raise AssertionError("unterminated MessageWin::add_message")


FIXTURE = r'''#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <string>
#include <string.h>
#define __cdecl

static std::string events;
static int MessageWinMessageCount;
static int pos_to_id_calls;
static int remove_saw_predecrement_count;
static int singleton_destructor_saw_live_links;

struct StringList;
struct StringStructPayload {
    int field_C_;
    char *text;
    explicit StringStructPayload(int value) : field_C_(value), text(0) {}
    ~StringStructPayload() { events += 'p'; }
};
struct StringStructEntry {
    int id;
    StringStructEntry *next;
    StringStructEntry *previous;
    StringStructPayload *payload;
    StringList *owner;
    StringStructEntry(int item_id, StringStructPayload *item_payload,
                      StringList *list)
        : id(item_id), next(0), previous(0), payload(item_payload), owner(list) {}
    StringStructPayload *string_payload() { return payload; }
    ~StringStructEntry();
};
struct StringList {
    StringStructEntry *head_;
    StringStructEntry *current_;
    int entry_count_;
    int current_position_;
    StringList() : head_(0), current_(0), entry_count_(0), current_position_(0) {}
    int seek_pos(int a1);
    void on_remove(StringStructPayload *) {
        events += 'r';
        if (MessageWinMessageCount == 100) remove_saw_predecrement_count = 1;
    }
};
StringStructEntry::~StringStructEntry() {
    events += 'e';
    if (owner->entry_count_ == 1 && owner->head_ == this && owner->current_ == this)
        singleton_destructor_saw_live_links = 1;
}

struct Dialog {
    StringList item_list_;
    int pos_to_id(int position) {
        ++pos_to_id_calls;
        if (position <= item_list_.entry_count_ - 1) {
            item_list_.current_ = item_list_.head_;
            if (position < 0) {
                int distance = -position;
                while (distance > 0) {
                    item_list_.current_ = item_list_.current_->previous;
                    --distance;
                }
                position += item_list_.entry_count_;
            } else {
                int distance = position;
                while (distance > 0) {
                    item_list_.current_ = item_list_.current_->next;
                    --distance;
                }
            }
            item_list_.current_position_ = position;
        }
        return item_list_.head_ ? item_list_.current_->id : 0;
    }
};
struct ListBox : Dialog {
    int field_20_;
    int item_calls_;
    int update_calls_;
    ListBox() : field_20_(0), item_calls_(0), update_calls_(0) {}
    int item(char *text, int) {
        ++item_calls_;
        if (text == 0) return 3;
        return 0;
    }
    void set_selected_pos(int, int) {}
    void update_changes() { ++update_calls_; }
};
struct MessageWin {
    ListBox listBox_;
    void add_message(char *text, int item_id);
};

static StringStructEntry *append(StringList &list, int id, int height) {
    StringStructPayload *payload = new StringStructPayload(height);
    StringStructEntry *entry = new StringStructEntry(id, payload, &list);
    if (list.head_ == 0) {
        entry->next = entry;
        entry->previous = entry;
        list.head_ = entry;
        list.current_ = entry;
    } else {
        StringStructEntry *tail = list.head_->previous;
        entry->next = list.head_;
        entry->previous = tail;
        tail->next = entry;
        list.head_->previous = entry;
    }
    ++list.entry_count_;
    return entry;
}
'''


def projected_dependencies() -> str:
    # Project the real inline cursor operation into the existing typed double.
    signature = "MEASURED inline int StringStruct::seek_pos(int a1)"
    seek = extract_method((ROOT / "src/stringstruct.h").read_text(), signature)
    seek = seek.replace(signature, "int StringList::seek_pos(int a1)", 1)
    checked_text = extract_method(SOURCE.read_text(),
        "inline char *current_message_text(StringList &items)")
    # VC6 sees the C integer abs declaration. Select that same overload in
    # the host library while retaining the actual seek body/cast unchanged.
    integer_abs = ("typedef int (*AbsFnT)(int);\n"
                   "static int (&projected_integer_abs)(int) = *static_cast<AbsFnT>(&abs);\n"
                   "#define abs projected_integer_abs\n")
    return integer_abs + seek + "\n#undef abs\n" + checked_text


MAIN = r'''
int main() {
    {
        MessageWin window;
        MessageWinMessageCount = 103;
        window.listBox_.item_list_.current_position_ = 7;
        window.add_message(0, 1); // ListBox::item's null-text failure leaves it empty.
        assert(MessageWinMessageCount == 100); // Original drains regardless of head.
        assert(pos_to_id_calls == 4 && window.listBox_.update_calls_ == 1);
        assert(window.listBox_.item_list_.entry_count_ == 0);
        assert(window.listBox_.item_list_.current_position_ == 7);
    }
    {
        MessageWin window;
        StringList &list = window.listBox_.item_list_;
        append(list, 11, 4);
        StringStructEntry *second = append(list, 22, 9);
        list.current_ = second;
        list.current_position_ = 1;
        window.listBox_.field_20_ = 20;
        MessageWinMessageCount = 100;
        window.add_message(0, 1);
        assert(MessageWinMessageCount == 100 && pos_to_id_calls == 5);
        assert(list.entry_count_ == 1 && list.head_ == second && list.current_ == second);
        assert(list.current_position_ == 0 && window.listBox_.field_20_ == 16);
        assert(remove_saw_predecrement_count == 1 && events.substr(events.size() - 3) == "rpe");
    }
    {
        MessageWin window;
        StringList &list = window.listBox_.item_list_;
        StringStructEntry *only = append(list, 33, 5);
        list.current_position_ = 8;
        window.listBox_.field_20_ = 12;
        MessageWinMessageCount = 100;
        window.add_message(0, 1);
        assert(list.entry_count_ == 0 && list.head_ == 0);
        assert(list.current_position_ == -1 && window.listBox_.field_20_ == 7);
        assert(singleton_destructor_saw_live_links == 1);
        (void)only; // The original leaves current_ pointing at the destroyed singleton.
    }
    {
        MessageWin window;
        StringList &list = window.listBox_.item_list_;
        list.entry_count_ = 2; // Inconsistent empty head: pos_to_id still resets the cursor.
        list.current_position_ = 9;
        MessageWinMessageCount = 100;
        window.add_message(0, 1);
        assert(list.head_ == 0 && list.current_ == 0 && list.current_position_ == 0);
        assert(list.entry_count_ == 2 && MessageWinMessageCount == 100);
    }
    return 0;
}
'''


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
def test_actual_add_message_preserves_cursor_removal_and_empty_drain(compiler_name, tmp_path):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(f"{compiler_name} unavailable")
    body = extract_method(SOURCE.read_text(encoding="utf-8"))
    source = FIXTURE + "\n" + projected_dependencies() + "\n" + body + "\n" + MAIN
    cpp = tmp_path / "messagewin_add_message.cpp"
    exe = tmp_path / "messagewin_add_message"
    cpp.write_text(source, encoding="utf-8")
    built = subprocess.run(
        [compiler, "-std=c++98", "-Wall", "-Wextra", "-Wno-unused-parameter",
         str(cpp), "-o", str(exe)], capture_output=True, text=True, timeout=30,
    )
    assert built.returncode == 0, built.stdout + built.stderr
    result = subprocess.run([str(exe)], capture_output=True, text=True, timeout=10)
    assert result.returncode == 0, result.stdout + result.stderr


def test_actual_add_message_source_keeps_original_count_only_loop():
    body = extract_method(SOURCE.read_text(encoding="utf-8"))
    assert "while (MessageWinMessageCount > 100)" in body
    assert "while (MessageWinMessageCount > 100 && items.head_ != 0)" not in body
    assert "listBox_.pos_to_id(0)" in body
