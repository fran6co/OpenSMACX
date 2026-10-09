"""Host controls for the source-owned Popup string-list member paths.

These controls execute extracted product code with small host fixtures. They
do not establish native layout, ABI, whole-object extent, or the result of the
original unchecked null-payload load.
"""
from __future__ import annotations

import shutil
import subprocess
from pathlib import Path

from match_recovery.includes import COMMENTS_OR_STRINGS
from match_recovery.source import read_file

ROOT = next(parent for parent in Path(__file__).resolve().parents
            if (parent / "src/IMAGE_PIN").is_file())

def _body(address: int, signature: str) -> str:
    path = ROOT / "src/popup.cpp"
    rows = [row for row in read_file(path) if row.address == address]
    assert len(rows) == 1
    source = path.read_text(encoding="utf-8")
    marker = f"// ORIGINAL: 0x{address:08X}"
    marker_at = source.index(marker)
    masked = COMMENTS_OR_STRINGS.sub(
        lambda match: "".join("\n" if char == "\n" else " "
                               for char in match[0]),
        source,
    )
    start = masked.find(signature, marker_at)
    assert start >= 0, f"mapped body lacks {signature!r}"
    opening = masked.find("{", start)
    depth = 0
    for index in range(opening, len(masked)):
        if masked[index] == "{":
            depth += 1
        elif masked[index] == "}":
            depth -= 1
            if depth == 0:
                return source[start:index + 1]
    raise AssertionError(f"unterminated mapped body at 0x{address:08X}")

def _compile_run(tmp_path: Path, source: str) -> None:
    compiler = shutil.which("g++") or shutil.which("clang++")
    assert compiler, "host GCC or Clang is required for this control"
    cpp, exe = tmp_path / "control.cpp", tmp_path / "control"
    cpp.write_text(source, encoding="utf-8")
    built = subprocess.run(
        [compiler, "-std=c++98", "-Wall", "-Wextra", "-Werror",
         str(cpp), "-o", str(exe)],
        capture_output=True, text=True, timeout=30,
    )
    assert built.returncode == 0, built.stderr
    ran = subprocess.run([str(exe)], capture_output=True, text=True, timeout=10)
    assert ran.returncode == 0, ran.stderr

def test_popup_on_exec_actual_source_ring_walk_messages_and_callbacks(
        tmp_path: Path) -> None:
    body = _body(0x00405910, "int Popup::on_exec() {")
    assert "StringStructEntry *const head = unit_ids_.head_;" in body
    assert "StringStructEntry *const entry = unit_ids_.current_;" in body
    assert "reinterpret_cast<uint32_t>(unit_ids_.head_)" not in body
    assert "reinterpret_cast<uint32_t>(" not in body
    assert "msg = entry->string_payload()->text;" in body
    assert "StringStructPayload *payload = 0;" in body
    assert "msg = payload->text;" in body
    harness = r'''
#include <cassert>
#include <stdint.h>
#include <cstdlib>
#include <cstring>
struct StringStructPayload { const char *text; };
struct StringStructEntry {
    StringStructPayload *payload;
    StringStructEntry *next;
    StringStructEntry *previous;
    StringStructPayload *string_payload() { return payload; }
};
struct StringList {
    int entry_count_;
    StringStructEntry *head_, *current_;
    int current_position_;
};
struct TextBlock {};
struct Popup {
    uint32_t field_30AC_, field_30A8_;
    int (*field_3094_)(TextBlock *, int);
    TextBlock text_block_a_;
    StringList unit_ids_;
    int on_exec();
};
static char StringTemp[128];
static const char *BufferJoinSpace = " ";
static int BaseIDCurrentSelected = 19;
struct MainInterface {
    int calls, id, flags, a, b;
    char message[128];
    void set_message(const char *text, int id_, int flags_, int a_, int b_) {
        ++calls; id = id_; flags = flags_; a = a_; b = b_;
        std::strcpy(message, text);
    }
} MainInterfaceGlobal = {0, 0, 0, 0, 0, ""};
struct InterfaceMessage {
    int calls, id, flags;
    char message[128];
    void add_base(int id_, const char *text, int flags_) {
        ++calls; id = id_; flags = flags_;
        std::strcpy(message, text);
    }
} InterfaceMessageWin = {0, 0, 0, ""};
static int callback_calls, callback_arg;
static TextBlock *callback_receiver;
static int callback(TextBlock *receiver, int value) {
    ++callback_calls; callback_receiver = receiver; callback_arg = value;
    return 73;
}
''' + body + r'''
static void make_ring(StringStructEntry (&entries)[2],
                      StringStructPayload (&payloads)[2], int count) {
    entries[0].payload = &payloads[0];
    entries[0].next = count == 1 ? &entries[0] : &entries[1];
    entries[0].previous = count == 1 ? &entries[0] : &entries[1];
    payloads[0].text = "alpha";
    entries[1].payload = &payloads[1];
    entries[1].next = &entries[0]; entries[1].previous = &entries[0];
    payloads[1].text = "beta";
}
int main() {
    StringStructEntry entries[2]; StringStructPayload payloads[2];
    make_ring(entries, payloads, 2);
    Popup popup = {};
    popup.field_30AC_ = 1;
    popup.field_3094_ = &callback;
    popup.unit_ids_.entry_count_ = 2;
    popup.unit_ids_.head_ = &entries[0];
    popup.unit_ids_.current_ = &entries[1];
    popup.unit_ids_.current_position_ = 0;
    assert(popup.on_exec() == 0);
    assert(std::strcmp(MainInterfaceGlobal.message, "alpha beta") == 0);
    assert(MainInterfaceGlobal.calls == 1 && MainInterfaceGlobal.id == 0x5f
           && MainInterfaceGlobal.flags == 0 && MainInterfaceGlobal.a == 1
           && MainInterfaceGlobal.b == 1);
    assert(callback_calls == 1 && callback_receiver == &popup.text_block_a_
           && callback_arg == 1);
    assert(popup.unit_ids_.current_ == &entries[1]
           && popup.unit_ids_.current_position_ == 1);
    make_ring(entries, payloads, 1);
    popup.field_30AC_ = 3;
    popup.field_3094_ = 0;
    popup.unit_ids_.entry_count_ = 1;
    popup.unit_ids_.head_ = &entries[0];
    popup.unit_ids_.current_ = 0;
    popup.unit_ids_.current_position_ = 0;
    assert(popup.on_exec() == 1);
    assert(std::strcmp(StringTemp, "alpha") == 0);
    assert(popup.unit_ids_.current_ == &entries[0]
           && popup.unit_ids_.current_position_ == 0);
    assert(MainInterfaceGlobal.calls == 1 && callback_calls == 1);
}
'''
    _compile_run(tmp_path, harness)

def test_popup_text_selection_actual_source_with_intercepted_null_load(
        tmp_path: Path) -> None:
    body = _body(0x00405910, "int Popup::on_exec() {")
    start = body.index("        StringStructEntry *const entry = unit_ids_.current_;")
    end = body.index("        strcat(StringTemp, msg);", start)
    selection = body[start:end]
    assert selection.count("payload->text") == 1
    assert "else if (head == 0)" in selection
    # Intercept only the unchecked final member load; preserve the source branch.
    selection = selection.replace("payload->text", "safe_payload_text(payload)", 1)
    harness = r'''
#include <cassert>
struct StringStructPayload { const char *text; };
struct StringStructEntry {
    StringStructPayload *payload;
    StringStructPayload *string_payload() { return payload; }
};
struct StringList { StringStructEntry *current_; };
static int intercepted_loads;
static StringStructPayload *intercepted_payload;
static const char *safe_payload_text(StringStructPayload *payload) {
    ++intercepted_loads; intercepted_payload = payload;
    return payload == 0 ? "<intercepted address-4 load>" : payload->text;
}
static const char *select_text(StringList &unit_ids_, StringStructEntry *head) {
''' + selection + r'''
    return msg;
}
int main() {
    StringStructPayload payload = {"actual text"};
    StringStructEntry entry = {&payload}; StringList list = {&entry};
    assert(select_text(list, &entry) == payload.text);
    assert(intercepted_loads == 0);
    intercepted_payload = (StringStructPayload *)1;
    assert(select_text(list, 0) != 0);
    assert(intercepted_loads == 1 && intercepted_payload == 0);
    list.current_ = 0; intercepted_loads = 0;
    assert(select_text(list, 0) == 0 && intercepted_loads == 0);
}
'''
    _compile_run(tmp_path, harness)

def test_pop2_uses_previous_member_and_preserves_faulting_text_branch() -> None:
    body = _body(0x00405140, "int __cdecl POP2(const char *a1,")
    assert "entry = entry->previous;" in body
    assert "reinterpret_cast<char *>(entry) + 0x10" not in body
    assert "reinterpret_cast<int *>(4)" not in body
    assert "else if (pop.unit_ids_.head_ == 0)" in body
    assert "text = payload->text;" in body
    assert "StringStructEntry *entry = pop.unit_ids_.current_;" in body
