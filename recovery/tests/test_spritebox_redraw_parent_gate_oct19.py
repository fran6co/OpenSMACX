"""Exercise the literal SpriteBox redraw body against parent/list callbacks.

This is a host behavior control for source ordering and live list bounds.  Its
small fixture is not an ABI/layout or native compiler claim.
"""

from pathlib import Path
import shutil
import subprocess

import pytest


ROOT = Path(__file__).resolve().parents[2]


def extract_body(text: str) -> str:
    signature = "void SpriteBox::on_redraw()"
    assert text.count(signature) == 1
    start = text.index(signature)
    opening = text.index("{", start)
    depth = 0
    for index in range(opening, len(text)):
        if text[index] == "{":
            depth += 1
        elif text[index] == "}":
            depth -= 1
            if depth == 0:
                return text[start:index + 1]
    raise AssertionError("unterminated SpriteBox::on_redraw")


HARNESS = r'''
#include <cassert>

struct SpriteBox;
typedef void (*CopyHook)(SpriteBox *);
typedef void (*DrawHook)(SpriteBox *, int);

static int events[32];
static int event_count;
static int draw_indices[16];
static int draw_count;
static CopyHook copy_hook;
static DrawHook draw_hook;

struct Win {};

struct GraphicWin : Win {
    void *copy_receiver;
    void *copy_source;
    int copy_x, copy_y, copy_dest_x, copy_dest_y;
    int copy_width, copy_height, copy_calls;

    GraphicWin()
        : copy_receiver(0), copy_source(0), copy_x(0), copy_y(0),
          copy_dest_x(0), copy_dest_y(0), copy_width(0), copy_height(0),
          copy_calls(0) {}

    void copy(SpriteBox *source, int x, int y, int dest_x, int dest_y,
              int width, int height);
};

struct Dialog {
    int window_x_;
    int window_y_;
};

struct BitmapInfoHeader {
    int biWidth;
    int biHeight;
};

struct Dib {
    BitmapInfoHeader bmiHeader;
};

struct SpriteList {
    int entry_count_;
};

struct SpriteBox : GraphicWin, Dialog {
    Win *win_parent_;
    SpriteList list_;
    Dib dib_;
    int draw_calls;

    SpriteBox()
        : win_parent_(0), draw_calls(0) {
        list_.entry_count_ = 0;
        window_x_ = 0;
        window_y_ = 0;
        dib_.bmiHeader.biWidth = 0;
        dib_.bmiHeader.biHeight = 0;
    }

    void on_redraw();
    void draw_item(int index);
};

void GraphicWin::copy(SpriteBox *source, int x, int y, int dest_x,
                      int dest_y, int width, int height) {
    events[event_count++] = 1;
    copy_receiver = this;
    copy_source = source;
    copy_x = x;
    copy_y = y;
    copy_dest_x = dest_x;
    copy_dest_y = dest_y;
    copy_width = width;
    copy_height = height;
    ++copy_calls;
    if (copy_hook != 0) copy_hook(source);
}

void SpriteBox::draw_item(int index) {
    events[event_count++] = 2;
    draw_indices[draw_count++] = index;
    ++draw_calls;
    if (draw_hook != 0) draw_hook(this, index);
}

BODY

static void clear_events() {
    event_count = 0;
    draw_count = 0;
}

static void set_count_from_copy(SpriteBox *box) {
    box->list_.entry_count_ = 2;
}

static int resize_on_index = -1;
static int resize_to = 0;
static void resize_from_draw(SpriteBox *box, int index) {
    if (index == resize_on_index) box->list_.entry_count_ = resize_to;
}

int main() {
    SpriteBox box;
    GraphicWin parent;
    box.window_x_ = 37;
    box.window_y_ = -12;
    box.dib_.bmiHeader.biWidth = 43;
    box.dib_.bmiHeader.biHeight = 29;

    // A null parent suppresses both blitting and row painting, even for a
    // positive live count.
    clear_events();
    box.win_parent_ = 0;
    box.list_.entry_count_ = 4;
    box.on_redraw();
    assert(event_count == 0 && draw_count == 0 && box.draw_calls == 0);
    assert(parent.copy_calls == 0);

    // The parent receives the blit first, from this box, with the Dialog
    // scroll origin and signed source height; rows then run in index order.
    clear_events();
    box.win_parent_ = &parent;
    box.list_.entry_count_ = 3;
    box.draw_calls = parent.copy_calls = 0;
    copy_hook = 0;
    draw_hook = 0;
    box.on_redraw();
    assert(event_count == 4 && events[0] == 1 && events[1] == 2 &&
           events[2] == 2 && events[3] == 2);
    assert(parent.copy_calls == 1 && parent.copy_receiver == &parent);
    assert(parent.copy_source == &box);
    assert(parent.copy_x == 37 && parent.copy_y == -12);
    assert(parent.copy_dest_x == 0 && parent.copy_dest_y == 0);
    assert(parent.copy_width == 43 && parent.copy_height == -29);
    assert(draw_count == 3 && draw_indices[0] == 0 && draw_indices[1] == 1 &&
           draw_indices[2] == 2);

    // Empty and negative counts still copy the parent surface, but draw no
    // rows because the original condition is strictly greater than zero.
    for (int count = 0; count >= -2; --count) {
        clear_events();
        box.list_.entry_count_ = count;
        box.draw_calls = parent.copy_calls = 0;
        box.on_redraw();
        assert(event_count == 1 && events[0] == 1);
        assert(parent.copy_calls == 1 && draw_count == 0 && box.draw_calls == 0);
    }

    // The count is first read after the copy callback returns.
    clear_events();
    box.list_.entry_count_ = 0;
    box.draw_calls = parent.copy_calls = 0;
    copy_hook = set_count_from_copy;
    box.on_redraw();
    copy_hook = 0;
    assert(event_count == 3 && events[0] == 1 && events[1] == 2 &&
           events[2] == 2);
    assert(draw_count == 2 && draw_indices[0] == 0 && draw_indices[1] == 1);

    // A draw callback may shrink the live bound; the next condition observes
    // it instead of completing the original count snapshot.
    clear_events();
    box.list_.entry_count_ = 5;
    box.draw_calls = parent.copy_calls = 0;
    draw_hook = resize_from_draw;
    resize_on_index = 0;
    resize_to = 1;
    box.on_redraw();
    assert(draw_count == 1 && draw_indices[0] == 0);

    // It may also grow the bound, so additional rows are visited in order.
    clear_events();
    box.list_.entry_count_ = 2;
    box.draw_calls = parent.copy_calls = 0;
    resize_on_index = 0;
    resize_to = 4;
    box.on_redraw();
    draw_hook = 0;
    assert(draw_count == 4 && draw_indices[0] == 0 && draw_indices[1] == 1 &&
           draw_indices[2] == 2 && draw_indices[3] == 3);
    return 0;
}
'''


def compile_run(tmp_path: Path, compiler: str, body: str, name: str,
                succeeds: bool) -> None:
    source = tmp_path / (name + ".cpp")
    executable = tmp_path / name
    source.write_text(HARNESS.replace("BODY", body), encoding="utf-8")
    built = subprocess.run(
        [compiler, "-std=c++98", "-Wall", "-Wextra", "-Werror",
         str(source), "-o", str(executable)],
        text=True, capture_output=True, timeout=30,
    )
    assert built.returncode == 0, built.stdout + built.stderr
    result = subprocess.run([str(executable)], text=True, capture_output=True,
                            timeout=10)
    assert (result.returncode == 0) == succeeds, result.stdout + result.stderr


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
def test_spritebox_redraw_parent_gate_and_live_rows(tmp_path, compiler_name):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(f"{compiler_name} unavailable")

    body = extract_body((ROOT / "src/spritebox.cpp").read_text(encoding="utf-8"))
    compile_run(tmp_path, compiler, body, "spritebox_redraw_source", True)

    row_guard = """        if (list_.entry_count_ > 0) {
            int index = 0;
            do {
                draw_item(index);
                ++index;
            } while (index < list_.entry_count_);
        }
    }
}"""
    row_outside_parent = """    }
    if (list_.entry_count_ > 0) {
        int index = 0;
        do {
            draw_item(index);
            ++index;
        } while (index < list_.entry_count_);
    }
}"""
    assert body.count(row_guard) == 1

    cached_count = body.replace("list_.entry_count_", "cached_count")
    assert body.count("list_.entry_count_") == 2
    assert cached_count.count("cached_count") == 2
    cached_count = cached_count.replace(
        "void SpriteBox::on_redraw() {",
        "void SpriteBox::on_redraw() {\n    const int cached_count = list_.entry_count_;",
        1,
    )
    assert cached_count.count("list_.entry_count_") == 1
    assert cached_count.count("cached_count") == 3

    copy_call = """        static_cast<GraphicWin *>(parent)->copy(
            this, Dialog::window_x_, Dialog::window_y_, 0, 0,
            dib_.bmiHeader.biWidth, -dib_.bmiHeader.biHeight);"""
    assert body.count(copy_call) == 1

    mutants = (
        (body.replace(row_guard, row_outside_parent, 1), "row-outside-parent"),
        (cached_count, "cached-entry-count"),
        (body.replace(copy_call, "        // omitted parent copy", 1),
         "missing-parent-copy"),
        (body.replace("static_cast<GraphicWin *>(parent)->copy(",
                       "static_cast<GraphicWin *>(this)->copy(", 1),
         "wrong-copy-receiver"),
    )
    for mutant, name in mutants:
        compile_run(tmp_path, compiler, mutant, name, False)

