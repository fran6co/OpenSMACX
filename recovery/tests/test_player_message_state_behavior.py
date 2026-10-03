"""Actual-source control for the faction/message reset trio.

When placed at recovery/tests/test_player_message_state_behavior.py, the
default source root is the repository root (`HERE.parents[1]`). The
OPENSMACX_SOURCE_ROOT environment variable permits a private source projection.
The three tested bodies are extracted only from src/game.cpp; none is copied
into this test file.
"""
import os
import pathlib
import shutil
import subprocess
import tempfile

import pytest


HERE = pathlib.Path(__file__).resolve().parent
SOURCE_ROOT = pathlib.Path(os.environ.get("OPENSMACX_SOURCE_ROOT", HERE.parents[1]))
GAME_CPP = SOURCE_ROOT / "src" / "game.cpp"
STRINGSTRUCT_CPP = SOURCE_ROOT / "src" / "stringstruct.cpp"
STRINGLIST_CPP = SOURCE_ROOT / "src" / "stringlist.cpp"


def extract_braced(source, marker):
    start = source.index(marker)
    brace = source.index("{", start)
    depth = 0
    for end in range(brace, len(source)):
        if source[end] == "{":
            depth += 1
        elif source[end] == "}":
            depth -= 1
            if depth == 0:
                return source[start : end + 1]
    raise AssertionError("unclosed function: " + marker)


def actual_source_bodies():
    game = GAME_CPP.read_text()
    stringstruct = STRINGSTRUCT_CPP.read_text()
    stringlist = STRINGLIST_CPP.read_text()
    markers = (
        "void __cdecl clear_all_player_messages(",
        "void __cdecl clear_player_messages(",
        "int __cdecl display_player_messages(",
    )
    bodies = "\n".join(extract_braced(game, marker) for marker in markers)
    removal = extract_braced(stringstruct, "inline void clear_list_entries(")
    removal += "\n" + extract_braced(stringstruct, "void StringStruct::remove_all()")
    hooks = extract_braced(stringlist, "inline void release_text(char *text)")
    hooks += "\n" + extract_braced(
        stringlist, "void StringList::on_remove(StringStructPayload *payload)"
    )
    return bodies, removal, hooks


def make_control_source():
    bodies, removal, hooks = actual_source_bodies()
    return r'''#include <assert.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <vector>
#define nullptr 0
#define __cdecl
const int MaxPlayerNum = 8;
static std::vector<std::string> events;
static int active_faction = -1;
static int removed[8] = {0,0,0,0,0,0,0,0};
static int wins_seen[8][32];
static int losses_seen[8][32];
static int item_mutation = 0, selection_mutation = 0, update_mutation = 0;
int FactionTurnCombatWins[8], FactionTurnCombatLosses[8];
int MessageWinMessageCount = 0;
struct Heap {};
struct StringStructPayload;
static void payload_destroyed(StringStructPayload *payload);
struct StringStructPayload {
    char *text;
    int field_8_, field_C_;
    ~StringStructPayload() { payload_destroyed(this); }
    static void operator delete(void *) { }
};
static void payload_destroyed(StringStructPayload *payload) {
    if (active_faction >= 0 && active_faction < 8) {
        const int i = removed[active_faction]++;
        if (i < 32) {
            wins_seen[active_faction][i] = FactionTurnCombatWins[active_faction];
            losses_seen[active_faction][i] = FactionTurnCombatLosses[active_faction];
        }
        assert(payload->text == 0); // source StringList::on_remove precedes delete
    }
}
struct StringStructEntry {
    int id;
    StringStructPayload *payload;
    StringStructEntry *next, *previous;
    static void operator delete(void *) { }
};
class StringStruct {
public:
    virtual void on_remove(StringStructPayload *) { }
    StringStructEntry *head_, *current_;
    int entry_count_, current_position_;
    Heap *allocator_;
    StringStruct() : head_(0), current_(0), entry_count_(0), current_position_(0), allocator_(0) { }
    void remove_all();
};
class StringList : public StringStruct {
public:
    int test_faction;
    StringList() : test_faction(-1) { }
    virtual void on_remove(StringStructPayload *payload);
    int seek_pos(int position) {
        if (position > entry_count_ - 1) return 0;
        current_ = head_;
        if (position < 0) {
            if (-position > entry_count_) return 0;
            for (int i = 0; i < -position; ++i) current_ = current_->previous;
            current_position_ = position + entry_count_;
        } else {
            for (int i = 0; i < position; ++i) current_ = current_->next;
            current_position_ = position;
        }
        return 1;
    }
    void remove_all() {
        active_faction = test_faction;
        events.push_back("remove");
        StringStruct::remove_all();
        active_faction = -1;
    }
};
StringList MessageWinDeferredBaseMessages[8];
struct ListBoxDouble {
    std::vector<std::string> text;
    std::vector<unsigned int> ids;
    int selected_calls, update_calls;
    ListBoxDouble() : selected_calls(0), update_calls(0) { }
    int item(char *value, int id) {
        events.push_back("item"); text.push_back(value ? value : "<null>");
        ids.push_back((unsigned int)id);
        if (item_mutation) { MessageWinDeferredBaseMessages[2].entry_count_ = item_mutation; item_mutation = 0; }
        return 0;
    }
    void set_selected_pos(int, int) {
        events.push_back("select"); ++selected_calls;
        if (selection_mutation) { MessageWinDeferredBaseMessages[2].entry_count_ = selection_mutation; selection_mutation = 0; }
    }
    void update_changes() {
        events.push_back("update"); ++update_calls;
        if (update_mutation) MessageWinDeferredBaseMessages[2].entry_count_ = update_mutation;
    }
};
struct MessageWinDouble {
    ListBoxDouble listBox_;
    void clear() {
        events.push_back("clear"); MessageWinMessageCount = 0;
        listBox_.text.clear(); listBox_.ids.clear();
        listBox_.selected_calls = listBox_.update_calls = 0;
    }
};
MessageWinDouble InterfaceMessageWin;
''' + removal + "\n" + hooks + "\n" + bodies + r'''
static StringStructEntry entries[8][4];
static StringStructPayload payloads[8][4];
static char *copy_text(const char *s) {
    const size_t n = strlen(s) + 1;
    char *p = (char *)malloc(n); assert(p); memcpy(p, s, n); return p;
}
static void build_list(int faction, int count) {
    StringList &list = MessageWinDeferredBaseMessages[faction];
    list.test_faction = faction; list.head_ = list.current_ = 0;
    list.entry_count_ = count; list.current_position_ = 7; list.allocator_ = 0;
    if (count <= 0) return;
    for (int i = 0; i < count; ++i) {
        StringStructEntry &entry = entries[faction][i];
        StringStructPayload &payload = payloads[faction][i];
        entry.id = 0x20 + i; payload.text = copy_text(i == 0 ? "first-text" : "later-text");
        payload.field_8_ = payload.field_C_ = 0; entry.payload = &payload;
        entry.next = &entries[faction][(i + 1) % count];
        entry.previous = &entries[faction][(i + count - 1) % count];
    }
    list.head_ = list.current_ = &entries[faction][0];
}
static void clear_observations() {
    events.clear();
    for (int f = 0; f < 8; ++f) {
        removed[f] = 0;
        for (int i = 0; i < 32; ++i) wins_seen[f][i] = losses_seen[f][i] = -999;
    }
}
int main() {
    clear_observations();
    for (int f = 0; f < 8; ++f) { build_list(f, 2); FactionTurnCombatWins[f] = 100 + f; FactionTurnCombatLosses[f] = 200 + f; }
    clear_all_player_messages();
    for (int f = 0; f < 8; ++f) {
        StringList &list = MessageWinDeferredBaseMessages[f];
        assert(FactionTurnCombatWins[f] == 0 && FactionTurnCombatLosses[f] == 0);
        assert(removed[f] == 2 && wins_seen[f][0] == 0 && losses_seen[f][0] == 0);
        assert(list.head_ == 0 && list.entry_count_ == 0 && list.current_position_ == 0 && list.allocator_ == 0);
    }

    clear_observations(); build_list(3, 1);
    FactionTurnCombatWins[3] = 37; FactionTurnCombatLosses[3] = 91;
    FactionTurnCombatWins[2] = 12; FactionTurnCombatLosses[2] = 13;
    clear_player_messages(3);
    assert(removed[3] == 1 && wins_seen[3][0] == 37 && losses_seen[3][0] == 91);
    assert(FactionTurnCombatWins[3] == 0 && FactionTurnCombatLosses[3] == 0);
    assert(FactionTurnCombatWins[2] == 12 && FactionTurnCombatLosses[2] == 13);
    assert(MessageWinDeferredBaseMessages[3].current_position_ == 0 && MessageWinDeferredBaseMessages[3].allocator_ == 0);

    clear_observations();
    StringList &empty = MessageWinDeferredBaseMessages[5];
    empty.test_faction = 5; empty.head_ = empty.current_ = 0; empty.entry_count_ = 1;
    empty.current_position_ = 8; empty.allocator_ = (Heap *)1;
    FactionTurnCombatWins[5] = 55; FactionTurnCombatLosses[5] = 66;
    clear_player_messages(5);
    assert(empty.entry_count_ == 1 && empty.current_position_ == 0 && empty.allocator_ == 0);
    assert(removed[5] == 0 && FactionTurnCombatWins[5] == 0 && FactionTurnCombatLosses[5] == 0);

    clear_observations(); build_list(6, 1); MessageWinDeferredBaseMessages[6].entry_count_ = -2;
    clear_player_messages(6);
    assert(removed[6] == 0 && MessageWinDeferredBaseMessages[6].head_ == 0 && MessageWinDeferredBaseMessages[6].entry_count_ == 0);

    clear_observations(); build_list(2, 3);
    FactionTurnCombatWins[2] = 31; FactionTurnCombatLosses[2] = 42; MessageWinMessageCount = 99;
    item_mutation = 2; selection_mutation = 1; update_mutation = 3;
    const int displayed = display_player_messages(2);
    assert(displayed == 1 && MessageWinMessageCount == 1);
    assert(InterfaceMessageWin.listBox_.text.size() == 1 && InterfaceMessageWin.listBox_.text[0] == "first-text");
    assert(InterfaceMessageWin.listBox_.ids[0] == 0x40000020u && InterfaceMessageWin.listBox_.selected_calls == 1);
    assert(InterfaceMessageWin.listBox_.update_calls == 1);
    assert(events.size() >= 5 && events[0] == "clear" && events[1] == "item" && events[2] == "select" && events[3] == "update" && events[4] == "remove");
    assert(removed[2] == 3 && wins_seen[2][0] == 31 && losses_seen[2][0] == 42);
    assert(FactionTurnCombatWins[2] == 0 && FactionTurnCombatLosses[2] == 0);

    clear_observations(); build_list(4, 2); update_mutation = 0;
    entries[4][0].id = 0x40000000;
    assert(display_player_messages(4) == 2);
    assert(InterfaceMessageWin.listBox_.text[0] == "first-text");
    assert(InterfaceMessageWin.listBox_.ids[0] == 0x40000000u && InterfaceMessageWin.listBox_.ids[1] == 0x40000021u);
    assert(InterfaceMessageWin.listBox_.selected_calls == 2 && InterfaceMessageWin.listBox_.update_calls == 1);
    return 0;
}
'''


@pytest.mark.parametrize("compiler", ["g++", "clang++"])
def test_actual_source_message_state_bodies(compiler):
    compiler_path = shutil.which(compiler)
    if compiler_path is None:
        pytest.skip(compiler + " unavailable")
    source = make_control_source()
    with tempfile.TemporaryDirectory(prefix="message-state-source-control-") as temp:
        cpp = pathlib.Path(temp) / "control.cpp"
        binary = pathlib.Path(temp) / "control"
        cpp.write_text(source)
        subprocess.run([compiler_path, "-std=c++98", "-Wall", "-Wextra", str(cpp), "-o", str(binary)],
                       check=True, capture_output=True, text=True, timeout=30)
        subprocess.run([str(binary)], check=True, timeout=30)
