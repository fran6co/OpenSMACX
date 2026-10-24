"""Bounded C++98 controls for BasePop's real item-list width walk.

The fixture extracts UNK2 and the list methods from their current source
owners. It only supplies the node, BasePop field, and Font service surfaces
needed to compile those definitions; list traversal is not reimplemented.
The null-payload case models one Font callback mutating the live list between
iterations. Its UBSan result is a target-effect control for the pinned null
load, not a claim that abstract C++ defines null dereference behavior or that
all malformed list states are equivalent.
"""

from __future__ import annotations

import os
import shutil
import subprocess
from pathlib import Path

import pytest

from recovery.tests.test_pact_ascent import definition


ROOT = Path(__file__).resolve().parents[2]


def _actual_definitions():
    basepop = (ROOT / "src/basepop.cpp").read_text(encoding="utf-8")
    header = (ROOT / "src/stringstruct.h").read_text(encoding="utf-8")
    string_cpp = (ROOT / "src/stringstruct.cpp").read_text(encoding="utf-8")
    typedef = "typedef int (__cdecl *AbsFnT)(int);"
    assert header.count(typedef) == 1
    return {
        "basepop": definition(basepop, r"int BasePop::UNK2\(\)"),
        "seek_pos": definition(header, r"MEASURED inline int StringStruct::seek_pos\(int a1\)"),
        "next_entry": definition(
            header, r"MEASURED inline StringStructPayload \*StringStruct::next_entry\(\)"),
        "current_entry": definition(
            string_cpp, r"StringStructPayload \*StringStruct::current_entry\(\)"),
        "string_payload": definition(
            header, r"inline StringStructPayload \*StringStructEntry::string_payload\(\)"),
        "abs_typedef": typedef,
    }


HOST = r'''#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>

#define __cdecl
#define MEASURED
ABS_TYPE

struct StringStructPayload { char *text; };
struct StringStructEntry {
    StringStructPayload *payload;
    StringStructEntry *next;
    StringStructEntry *previous;
    StringStructPayload *string_payload();
};
struct StringStruct {
    StringStructEntry *head_;
    StringStructEntry *current_;
    int entry_count_;
    int current_position_;
    StringStruct() : head_(0), current_(0), entry_count_(0), current_position_(0) {}
    int seek_pos(int position);
    StringStructPayload *next_entry();
    StringStructPayload *current_entry();
};
struct StringList : StringStruct {};

struct Font {
    std::vector<std::string> seen;
    StringList *list;
    const char *clear_on_text;
    bool clear_current_too;
    int null_width_calls;
    Font() : list(0), clear_on_text(0), clear_current_too(false), null_width_calls(0) {}
    int width(const char *text) {
        seen.push_back(text ? text : "<null>");
        if (text && clear_on_text && std::strcmp(text, clear_on_text) == 0) {
            list->head_ = 0;
            if (clear_current_too) list->current_ = 0;
            clear_on_text = 0;
        }
        if (!text) { ++null_width_calls; return 0; }
        return static_cast<int>(std::strlen(text));
    }
};

char *BasePopDefaultOkText;
char *BasePopDefaultCancelText;
struct BasePop {
    int field_30B0_;
    int field_30A8_;
    Font *button_font1_;
    char *ok_text_;
    char *cancel_text_;
    StringList item_ids_;
    BasePop() : field_30B0_(0), field_30A8_(0), button_font1_(0),
                ok_text_(0), cancel_text_(0), item_ids_() {}
    int UNK2();
};

STRING_PAYLOAD_METHOD
SEEK_POS_METHOD
NEXT_ENTRY_METHOD
CURRENT_ENTRY_METHOD
BASEPOP_METHOD

static void require(bool value, const char *why) {
    if (!value) { std::fprintf(stderr, "CHECK FAILED: %s\n", why); std::exit(91); }
}

static void link_three(StringStructEntry *nodes) {
    nodes[0].next = &nodes[1]; nodes[0].previous = &nodes[2];
    nodes[1].next = &nodes[2]; nodes[1].previous = &nodes[0];
    nodes[2].next = &nodes[0]; nodes[2].previous = &nodes[1];
}

static int ordinary_caption_and_list_case() {
    char ok[] = "OK", cancel[] = "Cancel";
    char one[] = "one", longest[] = "longest", five[] = "five";
    StringStructPayload payloads[3] = {{one}, {longest}, {five}};
    StringStructEntry nodes[3] = {{&payloads[0], 0, 0}, {&payloads[1], 0, 0},
                                  {&payloads[2], 0, 0}};
    link_three(nodes);
    Font font;
    BasePop pop;
    BasePopDefaultOkText = ok; BasePopDefaultCancelText = cancel;
    pop.field_30B0_ = 1; pop.field_30A8_ = 0x40; pop.button_font1_ = &font;
    pop.item_ids_.head_ = &nodes[0]; pop.item_ids_.current_ = &nodes[1];
    pop.item_ids_.entry_count_ = 3; pop.item_ids_.current_position_ = 1;
    int width = pop.UNK2();
    require(width == 7, "maximum includes captions and all list entries");
    const char *expected[] = {"OK", "Cancel", "one", "longest", "five"};
    require(font.seen.size() == 5, "two captions and three list entries are measured");
    for (unsigned i = 0; i != 5; ++i)
        require(font.seen[i] == expected[i], "caption/list order is preserved");
    return 0;
}

static int callback_case(bool clear_current_too, bool require_fault) {
    char trigger[] = "trigger", second[] = "second";
    StringStructPayload payloads[2] = {{trigger}, {second}};
    StringStructEntry nodes[2] = {{&payloads[0], 0, 0}, {&payloads[1], 0, 0}};
    nodes[0].next = &nodes[1]; nodes[0].previous = &nodes[1];
    nodes[1].next = &nodes[0]; nodes[1].previous = &nodes[0];
    Font font;
    BasePop pop;
    pop.field_30B0_ = 1;
    pop.field_30A8_ = 0x10000; // skip caption calls; first width call is list text
    pop.button_font1_ = &font;
    pop.item_ids_.head_ = &nodes[0]; pop.item_ids_.current_ = &nodes[1];
    pop.item_ids_.entry_count_ = 2; pop.item_ids_.current_position_ = 1;
    font.list = &pop.item_ids_; font.clear_on_text = trigger;
    font.clear_current_too = clear_current_too;
    int width = pop.UNK2();
    if (require_fault) {
        require(false, "expected null payload fault did not occur");
    }
    require(width == 7, "first list width remains the result after a live callback");
    if (clear_current_too) {
        require(font.seen.size() == 2 && font.seen[0] == "trigger" &&
                font.seen[1] == "<null>" && font.null_width_calls == 1,
                "clearing both live pointers continues with width(nullptr)");
    } else {
        require(font.seen.size() == 2 && font.seen[0] == "trigger" &&
                font.seen[1] == "<null>" && font.null_width_calls == 1,
                "safe-ternary mutant continues with width(nullptr)");
    }
    return 0;
}

int main(int argc, char **argv) {
    if (argc > 1 && std::strcmp(argv[1], "head-only") == 0)
        return callback_case(false, true);
    if (argc > 1 && std::strcmp(argv[1], "clear-both") == 0)
        return callback_case(true, false);
    ordinary_caption_and_list_case();
    callback_case(true, false);
    return 0;
}
'''


def _fixture(definitions: dict[str, str], *, old_safe_guard: bool = False) -> str:
    body = definitions["basepop"]
    source = HOST
    if old_safe_guard:
        guarded = "item_ids_.current_entry()->text"
        safe = "(item_ids_.head_ != 0 ? item_ids_.current_->string_payload()->text : static_cast<char *>(0))"
        assert body.count(guarded) == 1
        body = body.replace(guarded, safe, 1)
        source = source.replace('return callback_case(false, true);',
                                'return callback_case(false, false);')
    return (source.replace("ABS_TYPE", definitions["abs_typedef"])
            .replace("STRING_PAYLOAD_METHOD", definitions["string_payload"])
            .replace("SEEK_POS_METHOD", definitions["seek_pos"])
            .replace("NEXT_ENTRY_METHOD", definitions["next_entry"])
            .replace("CURRENT_ENTRY_METHOD", definitions["current_entry"])
            .replace("BASEPOP_METHOD", body))


def _compile(compiler: str, source: str, tmp_path: Path, name: str) -> Path:
    cpp = tmp_path / (name + ".cpp")
    exe = tmp_path / name
    cpp.write_text(source, encoding="utf-8")
    built = subprocess.run(
        [compiler, "-std=c++98", "-O2", "-Wall", "-Wextra", "-Werror",
         "-fsanitize=undefined", "-fno-sanitize-recover=all", str(cpp), "-o", str(exe)],
        capture_output=True, text=True, timeout=45)
    assert built.returncode == 0, built.stdout + built.stderr
    return exe


def _run(exe: Path, *args: str):
    environment = dict(os.environ)
    environment["UBSAN_OPTIONS"] = "halt_on_error=1:print_stacktrace=0"
    return subprocess.run([str(exe), *args], capture_output=True, text=True,
                          timeout=10, env=environment)


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
def test_actual_basepop_width_walk_and_pinned_null_payload_path(tmp_path, compiler_name):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(compiler_name + " is unavailable")
    definitions = _actual_definitions()
    actual = _compile(compiler, _fixture(definitions), tmp_path, "basepop_actual")
    ordinary = _run(actual)
    assert ordinary.returncode == 0, ordinary.stdout + ordinary.stderr
    clear_both = _run(actual, "clear-both")
    assert clear_both.returncode == 0, clear_both.stdout + clear_both.stderr

    head_only = _run(actual, "head-only")
    assert head_only.returncode != 0, "actual candidate unexpectedly avoided the pinned null-payload path"
    assert "member access within null pointer" in head_only.stderr, head_only.stderr
    assert "StringStructPayload" in head_only.stderr, head_only.stderr

    mutant = _compile(compiler, _fixture(definitions, old_safe_guard=True),
                      tmp_path, "basepop_old_safe_guard_mutant")
    old_guard = _run(mutant, "head-only")
    assert old_guard.returncode == 0, old_guard.stdout + old_guard.stderr
    assert "runtime error:" not in old_guard.stderr, old_guard.stderr
