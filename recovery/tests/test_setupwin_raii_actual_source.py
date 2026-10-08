"""Actual SetupWin::close and destructor controls.

The cleanup method, destructor and list helpers come from product source; component
and base destructors are fixture probes so C++ scope unwinding is observable.
"""
from __future__ import annotations

from pathlib import Path
import os
import shutil
import subprocess

import pytest


ROOT = Path(__file__).resolve().parents[2]


def extract_body(text: str, signature: str) -> str:
    start = text.index(signature + " {")
    opening = text.index("{", start)
    depth = 0
    for pos in range(opening, len(text)):
        if text[pos] == "{":
            depth += 1
        elif text[pos] == "}":
            depth -= 1
            if depth == 0:
                return text[start:pos + 1]
    raise AssertionError("unterminated source body: " + signature)


def actual_close_and_list_helpers() -> tuple[str, str, str]:
    setup = (ROOT / "src/setupwin.cpp").read_text()
    close = extract_body(setup, "void SetupWin::close()")
    struct = (ROOT / "src/stringstruct.cpp").read_text()
    clear = extract_body(struct, "inline void clear_list_entries(StringStruct *self)")
    remove = extract_body(struct, "void StringStruct::remove_all()")
    # Existing maintained control's sole C++98 spelling bridge.
    clear = clear.replace("nullptr", "0")
    return close, clear, remove


FIXTURE = r'''
#include <stdio.h>
#include <stdlib.h>

static int events[64];
static int event_count;
static int close_count;
static int shutdown_count;
static int payload_delete_count;
static int entry_delete_count;
static void event(int value) { events[event_count++] = value; }
static void require(bool ok, const char *name) {
    if (!ok) { fprintf(stderr, "FAIL: %s\n", name); abort(); }
}
static void reset_counts() {
    event_count = close_count = shutdown_count = 0;
    payload_delete_count = entry_delete_count = 0;
}

struct ListPayloadBase { virtual ~ListPayloadBase() {} };
struct StringStructEntry {
    ListPayloadBase *payload;
    StringStructEntry *next;
    virtual ~StringStructEntry() { ++entry_delete_count; event(5); }
};
struct StringStruct {
    StringStructEntry *head_;
    StringStructEntry *current_;
    int entry_count_;
    int current_position_;
    StringStruct() : head_(0), current_(0), entry_count_(0), current_position_(0) {}
    virtual void on_remove(void *) {}
    void remove_all();
};
struct StringList : StringStruct {
    int removals;
    StringList() : removals(0) {}
    virtual void on_remove(void *payload) {
        ++removals;
        require(payload == head_->payload, "callback observes current payload");
        event(3);
    }
    ~StringList() { event(6); }
};
struct Payload : ListPayloadBase {
    virtual ~Payload() { ++payload_delete_count; event(4); }
};
struct Buffer {
    struct SetupWin *owner;
    Buffer() : owner(0) {}
    void close();
    ~Buffer() { event(8); }
};
struct Spot {
    struct SetupWin *owner;
    Spot() : owner(0) {}
    void shutdown();
    ~Spot() { event(7); }
};
struct GraphicWin {
    virtual ~GraphicWin() { event(9); }
};
struct SetupWin : GraphicWin {
    unsigned int selected_row_, row_count_, field_A14_, field_A18_;
    Buffer buffer_;
    Spot spot_;
    StringList field_FE0_;
    SetupWin() : selected_row_(19), row_count_(21), field_A14_(23), field_A18_(25) {
        buffer_.owner = this;
        spot_.owner = this;
    }
    void close();
    ~SetupWin();
};
void Buffer::close() {
    require(owner->selected_row_ == 0 && owner->row_count_ == 0 &&
            owner->field_A14_ == 0 && owner->field_A18_ == 0,
            "all SetupWin words clear before Buffer close");
    ++close_count;
    event(1);
}
void Spot::shutdown() {
    require(close_count > 0, "Buffer close precedes Spot shutdown");
    ++shutdown_count;
    event(2);
}
'''


def translation_unit() -> str:
    close, clear, remove = actual_close_and_list_helpers()
    destructor = extract_body((ROOT / "src/setupwin.cpp").read_text(), "SetupWin::~SetupWin()")
    return FIXTURE + "\n" + clear + "\n" + remove + "\n" + close + "\n" + destructor + r'''
static StringStructEntry *one_entry() {
    StringStructEntry *entry = new StringStructEntry;
    entry->payload = new Payload;
    entry->next = 0;
    return entry;
}
static void check_one_scope(const char *name) {
    require(close_count == 1 && shutdown_count == 1,
            "destructor invokes Buffer/Spot cleanup exactly once");
    require(payload_delete_count == 1 && entry_delete_count == 1,
            "actual list helper releases payload and entry once");
    require(event_count == 9, "body plus member/base destruction event count");
    int expected[9] = {1, 2, 3, 4, 5, 6, 7, 8, 9};
    for (int i = 0; i < 9; ++i)
        if (events[i] != expected[i]) {
            fprintf(stderr, "FAIL: %s destruction order at %d: got %d expected %d\n",
                    name, i, events[i], expected[i]);
            abort();
        }
}
static void ordinary_scope() {
    SetupWin window;
    window.field_FE0_.head_ = one_entry();
    window.field_FE0_.current_ = window.field_FE0_.head_;
    window.field_FE0_.entry_count_ = 1;
    window.field_FE0_.current_position_ = 4;
}
static void early_return() {
    SetupWin window;
    window.field_FE0_.head_ = one_entry();
    window.field_FE0_.current_ = window.field_FE0_.head_;
    window.field_FE0_.entry_count_ = 1;
    window.field_FE0_.current_position_ = 5;
    return;
}
static void exceptional_exit() {
    SetupWin window;
    window.field_FE0_.head_ = one_entry();
    window.field_FE0_.current_ = window.field_FE0_.head_;
    window.field_FE0_.entry_count_ = 1;
    window.field_FE0_.current_position_ = 6;
    throw 17;
}
int main() {
    reset_counts();
    ordinary_scope();
    check_one_scope("normal scope exit");

    reset_counts();
    early_return();
    check_one_scope("early return");

    reset_counts();
    try { exceptional_exit(); }
    catch (int value) { require(value == 17, "original exception propagates"); }
    check_one_scope("exceptional exit");
    return 0;
}
'''


@pytest.mark.parametrize("compiler", ["g++", "clang++"])
def test_actual_close_with_raii_destructor_runs_once_on_all_exits(compiler, tmp_path):
    if shutil.which(compiler) is None:
        pytest.skip(f"{compiler} unavailable")
    source = translation_unit()
    assert "field_FE0_.remove_all()" in source
    assert extract_body((ROOT / "src/setupwin.cpp").read_text(), "SetupWin::~SetupWin()").count("close();") == 1
    assert "void SetupWin::close()" in source
    cpp = tmp_path / "setupwin_raii.cpp"
    exe = tmp_path / "setupwin_raii"
    cpp.write_text(source)
    built = subprocess.run(
        [compiler, "-std=c++98", "-Wall", "-Wextra", "-Werror",
         "-fsanitize=address,undefined", "-fno-omit-frame-pointer",
         str(cpp), "-o", str(exe)], capture_output=True, text=True)
    assert built.returncode == 0, built.stdout + built.stderr
    env = dict(os.environ)
    # LeakSanitizer is unavailable under this runner's ptrace; ASan/UBSan stay on.
    env["ASAN_OPTIONS"] = "detect_leaks=0"
    result = subprocess.run([str(exe)], capture_output=True, text=True, env=env)
    assert result.returncode == 0, result.stdout + result.stderr


@pytest.mark.parametrize("signature", [
    "int __cdecl size_of_planet(int mode)",
    "int __cdecl custom_planet(int show_preview, int use_saved_choices)",
    "int __cdecl map_menu(int quick_start)",
    "int __cdecl top_menu(int mode)",
    "int __cdecl config_game(int mode)",
])
def test_actual_callers_leave_terminal_cleanup_to_automatic_owners(signature):
    method = extract_body((ROOT / "src/newgame.cpp").read_text(), signature)
    assert "SetupWin setup;" in method and "Popup popup;" in method
    assert "setup.close();" not in method
    if "config_game" in signature:
        # The one nonterminal close precedes reuse of the same popup.
        assert method.count("popup.close();") == 1
        assert "if (use_expansion_picker) popup.close();" in method
    else:
        assert "popup.close();" not in method
    popup_first = "size_of_planet" in signature or "custom_planet" in signature or "config_game" in signature
    assert (method.index("Popup popup;") < method.index("SetupWin setup;")) == popup_first
