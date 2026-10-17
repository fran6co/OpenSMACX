"""Bounded actual-source control for script item IDs, not full popup/runtime proof."""
from pathlib import Path
import hashlib
import os
import shutil
import subprocess

import pytest

from match_recovery.model import Span
from match_recovery.pe_coff import PeImage
from match_recovery.source import read_file


ROOT = Path(__file__).resolve().parents[2]
SOURCE = ROOT / "src/basepop.cpp"
PIN = "01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf"
COUNT = "dialogs_.Dialog::item_list_.entry_count_"


def plain_line_block():
    records = [r for r in read_file(SOURCE) if r.address == 0x601BF0]
    assert len(records) == 1 and records[0].state.value == "implemented"
    text = SOURCE.read_text(encoding="utf-8")
    method = text.index("int BasePop::start(")
    start = text.index("        if (tok[0] != '#') {", method)
    end = text.index("        // THE DIRECTIVE TABLE.", start)
    block = text[start:end]
    assert block.count("goto read_line;") == 1
    return block


def actual_read_loop():
    source = SOURCE.read_text(encoding="utf-8")
    method = source.index("int BasePop::start(")
    start = source.index("      read_line:", method)
    end = source.index("        // THE DIRECTIVE TABLE.", start)
    return source[start:end]


FIXTURE = r'''
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>
static void require(bool ok, const char *why) {
    if (!ok) { std::fprintf(stderr, "SEMANTIC97: %s\n", why); std::exit(97); }
}
static std::vector<std::string> lines;
static unsigned line_cursor;
static char *text_get() {
    return line_cursor == lines.size() ? nullptr : lines[line_cursor++].data();
}
static void parse_string(char *source, char *destination) { std::strcpy(destination, source); }
struct StringList {
    int entry_count_ = 0;
    char *pending_text_ = nullptr;
    int field_20_ = 0, field_24_ = 0;
    std::vector<int> ids, marks;
    std::vector<std::string> text;
    int add(int id) {
        ids.push_back(id); marks.push_back(field_20_);
        text.push_back(pending_text_); ++entry_count_; return 0;
    }
};
struct Dialog { StringList item_list_; };
struct EditGroup {
    std::vector<std::string> edits;
    std::vector<int> edit_flags;
    void edit(char *text, int zero, int flags) {
        require(zero == 0, "edit argument preserved");
        edits.push_back(text); edit_flags.push_back(flags);
    }
};
struct Dialogs : virtual Dialog, EditGroup {
    int kind_ = 16, count_calls = 0;
    bool fail_item = false;
    std::vector<int> requested;
    int get_num_items() {
        ++count_calls;
        return kind_ == 1 || kind_ == 2 || kind_ == 4 || kind_ == 8 || kind_ == 16
            ? item_list_.entry_count_ : 0;
    }
    int item(char *text, int id) {
        requested.push_back(id);
        if (fail_item) return 4;
        if (kind_ != 16) return 0;
        item_list_.pending_text_ = text;
        return item_list_.add(id);
    }
};
struct BasePop {
    Dialogs dialogs_;
    StringList unit_ids_;
    int field_30A8_ = 0;
    void plain(char *tok, int add_as_escaped, int flags) {
        int marks = 0;
        char *text = nullptr;
        // ACTUAL_BLOCK
        read_line: return;
    }
    int parse_script() {
        // Enter at the already parsed #itemlist state. Admission and other
        // directives are not part of this bounded loop/control fixture.
        int add_as_escaped = 0, flags = 0, marks = 0;
        char *line = nullptr, *text = nullptr;
        char tok[240];
        // ACTUAL_LOOP
        return 73;  // The next directive hands off to unmodeled parsing.
    }
};
static void add(BasePop &pop, const char *s, int escaped = 0, int flags = 0) {
    char tok[240]; std::strcpy(tok, s); pop.plain(tok, escaped, flags);
}
int main() {
    const char *names[] = {"start", "quick", "scenario", "load", "network", "credits", "exit"};
    BasePop loop;
    line_cursor = 0;
    for (int i = 0; i < 7; ++i) lines.push_back(names[i]);
    lines.push_back("#next");
    require(loop.parse_script() == 73, "real read loop reaches next directive");
    require(loop.dialogs_.requested.size() == 7, "all rows in one parser invocation");
    for (int i = 0; i < 7; ++i)
        require(loop.dialogs_.requested[i] == i, "script rows retain distinct pre-insertion count IDs");
    BasePop menu;
    for (int i = 0; i < 7; ++i) add(menu, names[i]);
    require(menu.dialogs_.requested.size() == 7, "all rows appended");
    for (int i = 0; i < 7; ++i)
        require(menu.dialogs_.requested[i] == i, "script rows retain distinct pre-insertion count IDs");
    require(menu.dialogs_.item_list_.ids[1] == 1, "Quick Start ID is one rather than zero");
    require(menu.dialogs_.count_calls == 0, "ordinary item reads count directly without kind query");

    BasePop existing;
    existing.dialogs_.item_list_.entry_count_ = 11;
    add(existing, "later"); add(existing, "last");
    require(existing.dialogs_.requested[0] == 11 && existing.dialogs_.requested[1] == 12,
            "current list count rather than independent ordinal");
    BasePop partial;
    partial.dialogs_.item_list_.entry_count_ = 7;
    partial.dialogs_.fail_item = true; add(partial, "failed");
    partial.dialogs_.fail_item = false; add(partial, "retry"); add(partial, "next");
    require(partial.dialogs_.requested == std::vector<int>({7, 7, 8}),
            "count reread after ignored insertion failure");

    BasePop other_kind;
    other_kind.dialogs_.kind_ = 32;
    other_kind.dialogs_.item_list_.entry_count_ = 9;
    add(other_kind, "not dispatched");
    require(other_kind.dialogs_.requested[0] == 9 && other_kind.dialogs_.count_calls == 0,
            "unconditional shared count operand even for non-item kind");
    BasePop signed_count;
    signed_count.dialogs_.item_list_.entry_count_ = -17;
    signed_count.dialogs_.fail_item = true; add(signed_count, "unchanged");
    require(signed_count.dialogs_.requested[0] == -17, "native count value is not clamped");

    BasePop escaped;
    escaped.unit_ids_.entry_count_ = 3;
    add(escaped, "^", 1); add(escaped, "^one", 1); add(escaped, "^^two", 1);
    require(escaped.unit_ids_.ids == std::vector<int>({0, 0, 0}),
            "escaped text remains the distinct zero-ID list");
    require(escaped.unit_ids_.marks == std::vector<int>({3, 1, 2}), "escaped prefix marks preserved");
    require(escaped.dialogs_.requested.empty(), "escaped list never inserts dialog items");

    BasePop edit;
    edit.field_30A8_ = 4;
    add(edit, "first", 0, 3);
    edit.dialogs_.item_list_.entry_count_ = 10;
    add(edit, "later", 0, 3);
    require(edit.dialogs_.edits == std::vector<std::string>({"first", "later"}), "edit text preserved");
    require(edit.dialogs_.edit_flags == std::vector<int>({0, 3}), "edit threshold flags preserved");
    require(edit.dialogs_.requested.empty(), "edit path never inserts dialog items");
}
'''


def run_host(tmp_path, block, compiler, loop=None):
    source = tmp_path / "script-items.cpp"
    binary = tmp_path / "script-items"
    source.write_text(FIXTURE.replace("// ACTUAL_BLOCK", block).replace(
        "// ACTUAL_LOOP", actual_read_loop() if loop is None else loop), encoding="utf-8")
    subprocess.run([compiler, "-std=c++17", "-Wall", "-Wextra", "-Werror",
                    str(source), "-o", str(binary)], check=True,
                   capture_output=True, text=True, timeout=30)
    return subprocess.run([str(binary)], capture_output=True, text=True, timeout=10)


@pytest.mark.parametrize("name", ["g++", "clang++"])
def test_actual_script_item_count_ids(tmp_path, name):
    compiler = shutil.which(name)
    if compiler is None:
        pytest.skip(f"{name} is required for this host control")
    result = run_host(tmp_path, plain_line_block(), compiler)
    assert result.returncode == 0, result.stdout + result.stderr


@pytest.mark.parametrize("replacement,diagnostic", [
    ("0", "script rows retain distinct pre-insertion count IDs"),
    ("dialogs_.get_num_items()", "ordinary item reads count directly without kind query"),
    (COUNT + " + 1", "script rows retain distinct pre-insertion count IDs"),
])
def test_wrong_id_operands_fail_after_positive_control(tmp_path, replacement, diagnostic):
    compiler = shutil.which("g++") or shutil.which("clang++")
    if compiler is None:
        pytest.skip("a host C++ compiler is required")
    block = plain_line_block()
    assert block.count(COUNT) == 1
    positive = run_host(tmp_path, block, compiler)
    assert positive.returncode == 0, positive.stdout + positive.stderr
    negative = run_host(tmp_path, block.replace(COUNT, replacement), compiler,
                        actual_read_loop().replace(COUNT, replacement))
    assert negative.returncode == 97, negative.stdout + negative.stderr
    assert diagnostic in negative.stderr


def test_preloop_count_snapshot_fails_after_positive_control(tmp_path):
    compiler = shutil.which("g++") or shutil.which("clang++")
    if compiler is None:
        pytest.skip("a host C++ compiler is required")
    positive = run_host(tmp_path, plain_line_block(), compiler)
    assert positive.returncode == 0, positive.stdout + positive.stderr
    loop = actual_read_loop()
    assert loop.count(COUNT) == 1
    # This mutates the actual multi-row loop, not seven separate invocations.
    cached = "const int frozen_count = " + COUNT + ";\n" + loop.replace(COUNT, "frozen_count")
    negative = run_host(tmp_path, plain_line_block(), compiler, cached)
    assert negative.returncode == 97, negative.stdout + negative.stderr
    assert "script rows retain distinct pre-insertion count IDs" in negative.stderr


def test_pinned_parser_uses_shared_dialog_count_not_zero():
    image = Path(os.environ.get("OPENSMACX_IMAGE", ROOT / ".opensmacx/game/terranx_original.exe"))
    if not image.is_file():
        pytest.skip("pinned original required for native instruction witness")
    data = image.read_bytes()
    assert hashlib.sha256(data).hexdigest() == PIN
    from capstone import Cs, CS_ARCH_X86, CS_MODE_32
    pe = PeImage(data, PIN)
    instructions = [(i.mnemonic, i.op_str) for i in Cs(CS_ARCH_X86, CS_MODE_32).disasm(
        pe.read(Span(0x6025AD, 0x6025CE)), 0x6025AD)]
    assert instructions == [
        ("mov", "edx, dword ptr [esi + 0x21d0]"),
        ("lea", "ecx, [esi + 0x21d0]"),
        ("mov", "eax, dword ptr [edx + 8]"),
        ("mov", "edx, dword ptr [eax + esi + 0x229c]"),
        ("lea", "eax, [esp + 0x1c]"),
        ("push", "edx"), ("push", "eax"), ("call", "0x612a70"),
    ]
    # Existing source layout evidence supplies the interpretation; no host ABI claim.
    assert 0x229C == 0x21D0 + 0xBC + 0x10
