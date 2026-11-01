"""Execute the product StringList::sort body against a circular host list."""

import subprocess
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]


def _sort_body() -> str:
    source = (ROOT / "src/stringlist.cpp").read_text()
    start = source.index("void StringList::sort(int) {")
    opening = source.index("{", start)
    depth = 0
    for index in range(opening, len(source)):
        if source[index] == "{":
            depth += 1
        elif source[index] == "}":
            depth -= 1
            if depth == 0:
                return source[start : index + 1]
    raise AssertionError("unclosed StringList::sort body")


def test_string_list_sort_keeps_ids_with_text_and_cursor(tmp_path: Path) -> None:
    harness = r"""
#include <cassert>
#include <cstring>

struct ListPayloadBase { virtual ~ListPayloadBase() {} };
struct StringStructPayload : ListPayloadBase {
    char *text;
    StringStructPayload() : text(0) {}
};
struct StringStructEntry {
    int id;
    ListPayloadBase *payload;
    StringStructEntry *next;
    StringStructEntry *previous;
    StringStructPayload *string_payload() {
        return static_cast<StringStructPayload *>(payload);
    }
};
struct StringList {
    StringStructEntry *head_ = nullptr;
    StringStructEntry *current_ = nullptr;
    int entry_count_ = 0;
    int current_position_ = 0;
    int seek_calls_ = 0;
    int failed_seek_ = 0;
    int seek_pos(int position) {
        if (++seek_calls_ == failed_seek_) return 0;
        if (position >= entry_count_) return 0;
        current_ = head_;
        for (int i = 0; i < position; ++i) current_ = current_->next;
        current_position_ = position;
        return 1;
    }
    StringStructPayload *next_entry() {
        current_ = current_->next;
        current_position_ = (current_position_ + 1) % entry_count_;
        return current_->string_payload();
    }
    void sort(int);
};
""" + _sort_body() + r"""
int main() {
    StringList empty;
    empty.sort(99);
    assert(empty.head_ == nullptr && empty.entry_count_ == 0);

    char a[] = "zeta", b[] = "alpha", c[] = "alpha", d[] = "beta";
    StringStructPayload payloads[4];
    payloads[0].text = a;
    payloads[1].text = b;
    payloads[2].text = c;
    payloads[3].text = d;
    StringStructEntry entries[4] = {};
    for (int i = 0; i < 4; ++i) {
        entries[i].id = 100 + i;
        entries[i].payload = &payloads[i];
        entries[i].next = &entries[(i + 1) % 4];
        entries[i].previous = &entries[(i + 3) % 4];
    }
    StringList list;
    list.head_ = entries;
    list.current_ = &entries[2];
    list.entry_count_ = 4;
    list.current_position_ = 2;
    list.sort(42); // the original ignores the key argument
    assert(std::strcmp(entries[0].string_payload()->text, "alpha") == 0);
    assert(std::strcmp(entries[1].string_payload()->text, "alpha") == 0);
    assert(std::strcmp(entries[2].string_payload()->text, "beta") == 0);
    assert(std::strcmp(entries[3].string_payload()->text, "zeta") == 0);
    assert(entries[0].id == 101 && entries[1].id == 102);
    assert(entries[2].id == 103 && entries[3].id == 100);
    assert(list.head_ == entries && list.entry_count_ == 4);
    assert(list.current_ == &entries[2] && list.current_position_ == 2);

    StringList one;
    one.head_ = one.current_ = &entries[0];
    one.entry_count_ = 1;
    one.sort(0);
    assert(one.current_ == &entries[0] && one.current_position_ == 0);

    // Original seek failure branches skip the swap and retain the cursor.
    // Exercise failure while reacquiring either the winning or outer entry.
    for (int failed_seek = 2; failed_seek <= 3; ++failed_seek) {
        StringStructEntry pair[2] = {};
        pair[0].id = 20; pair[0].payload = &payloads[0];
        pair[1].id = 21; pair[1].payload = &payloads[1];
        pair[0].next = pair[0].previous = &pair[1];
        pair[1].next = pair[1].previous = &pair[0];
        StringList interrupted;
        interrupted.head_ = interrupted.current_ = pair;
        interrupted.entry_count_ = 2;
        interrupted.failed_seek_ = failed_seek;
        interrupted.sort(0);
        assert(interrupted.seek_calls_ == failed_seek);
        assert(pair[0].id == 20 && pair[0].payload == &payloads[0]);
        assert(pair[1].id == 21 && pair[1].payload == &payloads[1]);
        assert(interrupted.current_ == &pair[1]);
        assert(interrupted.current_position_ == 1);
    }
}
"""
    source = tmp_path / "sort.cpp"
    binary = tmp_path / "sort"
    source.write_text(harness)
    subprocess.run(
        ["g++", "-std=c++17", "-Wall", "-Wextra", "-Werror", str(source), "-o", str(binary)],
        check=True,
    )
    subprocess.run([str(binary)], check=True)
