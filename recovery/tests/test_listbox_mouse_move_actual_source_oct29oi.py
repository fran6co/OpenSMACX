"""Actual-source host control for the pinned ListBox hover traversal.

The complete installed member is compiled against small typed linked-list and
window doubles. The control checks the pinned cursor side effects, span walk,
grid hover redraw order and selected-row clearing. It does not execute the
malformed null-payload path or claim target ABI/runtime equivalence.
"""
from __future__ import annotations

from pathlib import Path
import shutil
import subprocess

import pytest

from test_listbox_rendering_actual_source import extract_function


ROOT = Path(__file__).resolve().parents[2]
SOURCE = ROOT / "src/listbox.cpp"
SIGNATURE = "void ListBox::on_mouse_move(int x, int y)"

HARNESS = r'''#include <cassert>
#include <cstdint>
#include <stdlib.h>

struct StringStructPayload {
    unsigned int field_C_;
};

struct StringStructEntry {
    StringStructPayload *payload;
    StringStructEntry *next;
    StringStructEntry *previous;
    StringStructPayload *string_payload() { return payload; }
};

struct StringList {
    int entry_count_;
    StringStructEntry *head_;
    StringStructEntry *current_;
    int current_position_;
};

struct Dialog {
    StringList item_list_;
    int selected_position_;
    uint32_t field_F0_;
    int column_count_;
    int row_count_;
    int field_38_;
    int field_40_;
    int field_44_;
    int field_64_;
    int field_68_;
};

struct GraphicWin {
    int updates;
    GraphicWin() : updates(0) {}
    void soft_update() { ++updates; }
};

static int ListBoxDrawOffset;

struct ListBox : GraphicWin, Dialog {
    unsigned int field_18_;
    int draws[8];
    int draw_count;
    ListBox() : field_18_(0), draw_count(0) {
        item_list_.entry_count_ = 0;
        item_list_.head_ = 0;
        item_list_.current_ = 0;
        item_list_.current_position_ = 0;
        selected_position_ = -1;
        field_F0_ = 0xFFFFFFFFu;
        column_count_ = 1;
        row_count_ = 1;
        field_38_ = field_40_ = field_44_ = 0;
        field_64_ = field_68_ = 10;
    }
    void draw_item(int index) { draws[draw_count++] = index; }
    void on_mouse_move(int x, int y);
};

ACTUAL_BODY

static StringStructPayload payloads[8];
static StringStructEntry entries[8];

static void setup_list(ListBox &box, int count, const unsigned int *spans) {
    box.item_list_.entry_count_ = count;
    box.item_list_.head_ = count == 0 ? 0 : &entries[0];
    box.item_list_.current_ = count == 0 ? 0 : &entries[0];
    box.item_list_.current_position_ = 0;
    for (int i = 0; i < count; ++i) {
        payloads[i].field_C_ = spans[i];
        entries[i].payload = &payloads[i];
        entries[i].next = &entries[(i + 1) % count];
        entries[i].previous = &entries[(i + count - 1) % count];
    }
}

int main() {
    const unsigned int spans[] = {1, 2, 1, 1, 1, 1, 1, 1};

    // The variable-span walk consumes row 0, then identifies row 1 at y=20.
    ListBox first;
    setup_list(first, 3, spans);
    first.field_18_ = 0;
    first.on_mouse_move(900, 20);
    assert(first.field_F0_ == 1 && first.item_list_.current_position_ == 1);
    assert(first.draw_count == 1 && first.draws[0] == 1 && first.updates == 1);

    // Starting at top 1 positions the cursor at top-1, then advances to row 1.
    ListBox scrolled;
    setup_list(scrolled, 3, spans);
    scrolled.field_18_ = 1;
    scrolled.on_mouse_move(900, 10);
    assert(scrolled.field_F0_ == 1 && scrolled.item_list_.current_position_ == 1);
    assert(scrolled.draw_count == 1 && scrolled.draws[0] == 1 && scrolled.updates == 1);

    // Starting at the last row walks through the circular list and wraps the
    // cursor, but the computed ordinal is out of range and causes no redraw.
    ListBox wrapped;
    setup_list(wrapped, 3, spans);
    wrapped.field_18_ = 2;
    wrapped.field_F0_ = 7;
    wrapped.on_mouse_move(900, 10);
    assert(wrapped.item_list_.current_ == &entries[0]);
    assert(wrapped.item_list_.current_position_ == 0);
    assert(wrapped.field_F0_ == 7 && wrapped.draw_count == 0 && wrapped.updates == 0);

    // Grid mode redraws the old hover before the new one, then updates once.
    ListBox grid;
    setup_list(grid, 8, spans);
    grid.column_count_ = 2;
    grid.row_count_ = 2;
    grid.field_18_ = 0;
    grid.field_64_ = 10;
    grid.field_68_ = 10;
    grid.field_F0_ = 3;
    grid.on_mouse_move(20, 10);
    assert(grid.field_F0_ == 5 && grid.draw_count == 2);
    assert(grid.draws[0] == 3 && grid.draws[1] == 5 && grid.updates == 1);

    // Hovering the selected row clears the old hover and redraws only it.
    ListBox selected;
    setup_list(selected, 8, spans);
    selected.column_count_ = 2;
    selected.row_count_ = 2;
    selected.field_64_ = 10;
    selected.field_68_ = 10;
    selected.selected_position_ = 1;
    selected.field_F0_ = 4;
    selected.on_mouse_move(0, 10);
    assert(selected.field_F0_ == 0xFFFFFFFFu && selected.draw_count == 1);
    assert(selected.draws[0] == 4 && selected.updates == 1);

    // Empty lists and negative y offsets return before touching the cursor.
    ListBox empty;
    setup_list(empty, 0, spans);
    empty.item_list_.current_ = &entries[2];
    empty.item_list_.current_position_ = 2;
    empty.on_mouse_move(0, 0);
    assert(empty.item_list_.current_ == &entries[2]);
    assert(empty.item_list_.current_position_ == 2 && empty.draw_count == 0);

    ListBox above;
    setup_list(above, 3, spans);
    above.item_list_.current_ = &entries[2];
    above.item_list_.current_position_ = 2;
    above.on_mouse_move(0, -1);
    assert(above.item_list_.current_ == &entries[2]);
    assert(above.item_list_.current_position_ == 2 && above.draw_count == 0);
}
'''


def _body() -> str:
    return extract_function(SOURCE.read_text(encoding="utf-8"), SIGNATURE)


def _compile_and_run(tmp_path: Path, compiler: str, body: str,
                     name: str) -> subprocess.CompletedProcess[str]:
    cpp = tmp_path / f"{name}.cpp"
    exe = tmp_path / name
    cpp.write_text(HARNESS.replace("ACTUAL_BODY", body), encoding="utf-8")
    subprocess.run(
        [compiler, "-std=c++11", "-Wall", "-Wextra", "-Werror",
         str(cpp), "-o", str(exe)],
        check=True, capture_output=True, text=True, timeout=60,
    )
    return subprocess.run([str(exe)], capture_output=True, text=True, timeout=20)


@pytest.mark.parametrize("compiler_name", ["g++", "clang++"])
def test_actual_mouse_move_preserves_pinned_cursor_and_hover_flow(
        tmp_path: Path, compiler_name: str) -> None:
    compiler = shutil.which(compiler_name)
    if not compiler:
        pytest.skip(f"{compiler_name} unavailable")
    result = _compile_and_run(tmp_path, compiler, _body(), f"actual-{compiler_name}")
    assert result.returncode == 0, result.stderr


def test_wrap_and_payload_span_mutants_are_rejected(tmp_path: Path) -> None:
    compiler = shutil.which("g++")
    if not compiler:
        pytest.skip("g++ unavailable")
    body = _body()
    wrap = """if (current_position == items.entry_count_) {
                        items.current_position_ = 0;
                    }"""
    assert body.count(wrap) == 1
    no_wrap = body.replace(wrap, "if (false) { items.current_position_ = 0; }")
    result = _compile_and_run(tmp_path, compiler, no_wrap, "no-wrap")
    assert result.returncode != 0, "removing cursor ordinal wrap survived the control"

    span = "span = static_cast<int>(payload->field_C_);"
    assert body.count(span) == 1
    unit_span = body.replace(span, "span = payload != 0 ? 1 : 0;")
    result = _compile_and_run(tmp_path, compiler, unit_span, "unit-span")
    assert result.returncode != 0, "ignoring payload spans survived the control"
