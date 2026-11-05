"""Actual text accessor guard controls; not original layout/runtime proof."""
from pathlib import Path
import shutil
import subprocess

import pytest

ROOT = Path(__file__).resolve().parents[2]


def definition(text, signature):
    start = text.index(signature)
    opening = text.index("{", start)
    depth = 0
    for end in range(opening, len(text)):
        depth += (text[end] == "{") - (text[end] == "}")
        if depth == 0:
            return text[start:end + 1]
    raise AssertionError("Unclosed accessor")


@pytest.mark.parametrize("compiler_name", ["g++", "clang++"])
def test_real_text_accessor_guards_head_not_current(tmp_path, compiler_name):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(f"{compiler_name} unavailable")
    source = (ROOT / "src/stringstruct.cpp").read_text()
    buffer_source = (ROOT / "src/buffer.cpp").read_text()
    body = definition(buffer_source, "char *StringList::current_text()")
    current_entry = definition(source,
                               "StringStructPayload *StringStruct::current_entry()")
    fixture = r"""
#include <cassert>
struct StringStructPayload { char *text; };
struct StringStructEntry {
    StringStructPayload *payload;
    StringStructPayload *string_payload() { return payload; }
};
struct StringStruct {
    StringStructEntry *head_, *current_;
    StringStructPayload *current_entry();
};
struct StringList : StringStruct {
    char *current_text();
};
CURRENT_ENTRY
BODY
int main() {
    char first[] = "first", second[] = "second";
    StringStructPayload p = {first}, q = {second};
    StringStructEntry a = {&p}, b = {&q};
    StringList list;
    list.head_ = 0;
    list.current_ = &a;
    assert(list.current_text() == 0);
    list.head_ = &a;
    list.current_ = &b;
    assert(list.current_text() == second);
    list.current_ = &a;
    assert(list.current_text() == first);
    p.text = 0;
    assert(list.current_text() == 0);
}
"""

    def run(candidate, name):
        source, executable = tmp_path / (name + ".cpp"), tmp_path / name
        source.write_text(fixture.replace("CURRENT_ENTRY", current_entry).replace("BODY", candidate))
        subprocess.run([compiler, "-std=c++98", "-Wall", "-Wextra", "-Werror",
                        str(source), "-o", str(executable)], check=True,
                       capture_output=True, timeout=30)
        return subprocess.run([str(executable)], capture_output=True,
                              timeout=10).returncode

    assert run(body, "actual") == 0
    wrong_guard = body.replace("head_ != 0", "current_ != 0")
    assert wrong_guard != body
    assert run(wrong_guard, "wrong_guard") != 0


@pytest.mark.parametrize("compiler_name", ["g++", "clang++"])
def test_editgroup_actual_cursor_rectangles_and_live_bounds(tmp_path, compiler_name):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(f"{compiler_name} unavailable")
    body = definition((ROOT / "src/editgroup.cpp").read_text(),
                      "void EditGroup::on_redraw()")
    seek = definition((ROOT / "src/stringstruct.h").read_text(),
                      "MEASURED inline int StringStruct::seek_pos(int a1)")
    fixture = r"""
#include <cassert>
#include <cstring>
#include <cstdlib>
typedef unsigned int uint32_t;
struct RECT { int left, top, right, bottom; };
struct StringStructPayload { char *text; };
struct StringStructEntry {
    StringStructEntry *next, *previous;
    StringStructPayload *payload;
    StringStructPayload *string_payload() { return payload; }
};
struct StringStruct {
    StringStructEntry *head_, *current_;
    int entry_count_, current_position_;
    int seek_pos(int);
};
#define MEASURED
typedef int (*AbsFnT)(int);
SEEK_BODY
struct Dialog {
    uint32_t field_24_, field_28_, field_34_, field_40_, field_44_;
    int column_count_, row_count_, field_64_, field_68_;
    StringStruct item_list_;
};
struct Win { Win *win_parent_; };
Win *WinCallbackWindow;
struct Buffer {
    void copy(Buffer *, int, int, int, int, int, int);
    void write_l(char *, RECT *, unsigned int);
};
struct GraphicWin : Win, Buffer {
    struct { struct { int biWidth, biHeight; } bmiHeader; } dib_;
};
struct EditGroup : GraphicWin, Dialog { void on_redraw(); };
EditGroup *active;
int copies, writes, mode;
RECT rectangles[8]; char *labels[8];
void Buffer::copy(Buffer *to, int x, int y, int dx, int dy, int w, int h) {
    assert(to == static_cast<Buffer *>(active));
    assert(x == 11 && y == 13 && dx == 0 && dy == 0 && w == 80 && h == 60);
    ++copies;
}
void Buffer::write_l(char *text, RECT *area, unsigned int length) {
    assert(length == strlen(text));
    assert(writes < 8);
    labels[writes] = text; rectangles[writes++] = *area;
    if (writes == 1 && mode == 1) {
        active->column_count_ = active->row_count_ = 1;
    }
    if (writes == 1 && mode == 2) active->field_68_ = 40;
}
BODY
int main() {
    char a[] = "a", b[] = "bb", c[] = "ccc";
    StringStructPayload pa = {a}, pb = {b}, pc = {c};
    StringStructEntry entries[3];
    for (int i = 0; i < 3; ++i) {
        entries[i].next = &entries[(i + 1) % 3];
        entries[i].previous = &entries[(i + 2) % 3];
    }
    entries[0].payload = &pa; entries[1].payload = &pb; entries[2].payload = &pc;
    GraphicWin parent;
    for (mode = 0; mode < 3; ++mode) {
        EditGroup e; active = &e; writes = copies = 0;
        e.win_parent_ = &parent;
        e.dib_.bmiHeader.biWidth = 80; e.dib_.bmiHeader.biHeight = -60;
        e.field_24_ = 11; e.field_28_ = 13;
        e.field_34_ = 7; e.field_40_ = 5; e.field_44_ = 6;
        e.column_count_ = e.row_count_ = 2; e.field_64_ = 30; e.field_68_ = 20;
        e.item_list_.head_ = &entries[0]; e.item_list_.current_ = &entries[1];
        e.item_list_.entry_count_ = 3; e.item_list_.current_position_ = 1;
        e.on_redraw();
        assert(copies == 1 && WinCallbackWindow == &parent);
        assert(writes == (mode == 1 ? 1 : 4));
        assert(labels[0] == a);
        assert(rectangles[0].left == 12 && rectangles[0].right == 42);
        assert(rectangles[0].top == 3 && rectangles[0].bottom == 17);
        if (mode != 1) {
            assert(labels[1] == b && labels[2] == c && labels[3] == a);
            assert(rectangles[1].top == (mode == 2 ? 43 : 23));
            assert(rectangles[1].bottom == (mode == 2 ? 57 : 37));
            assert(rectangles[2].left == 42 && rectangles[2].right == 72);
            assert(e.item_list_.current_ == &entries[0]);
            assert(e.item_list_.current_position_ == 0);
        }
    }
}
"""
    fixture = fixture.replace("SEEK_BODY", seek)
    source, executable = tmp_path / "editgroup.cpp", tmp_path / "editgroup"
    source.write_text(fixture.replace("BODY", body))
    subprocess.run([compiler, "-std=c++98", "-Wall", "-Wextra", "-Werror",
                    str(source), "-o", str(executable)], check=True,
                   capture_output=True, timeout=30)
    subprocess.run([str(executable)], check=True, capture_output=True, timeout=10)
    cached_bounds = body.replace(
        "void EditGroup::on_redraw() {",
        "void EditGroup::on_redraw() {\n"
        "    const int cached_columns = Dialog::column_count_;\n"
        "    const int cached_rows = Dialog::row_count_;",
    ).replace("column < Dialog::column_count_",
              "column < cached_columns").replace(
        "row < Dialog::row_count_", "row < cached_rows")
    assert cached_bounds != body
    source.write_text(fixture.replace("BODY", cached_bounds))
    subprocess.run([compiler, "-std=c++98", "-Wall", "-Wextra", "-Werror",
                    str(source), "-o", str(executable)], check=True,
                   capture_output=True, timeout=30)
    assert subprocess.run([str(executable)], capture_output=True,
                          timeout=10).returncode != 0
