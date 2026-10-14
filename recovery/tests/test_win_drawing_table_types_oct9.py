"""Host controls for Win's borrowed drawing-table pointers."""

from pathlib import Path
import re
import shutil
import subprocess

import pytest


ROOT = Path(__file__).resolve().parents[2]
COMPILERS = [path for name in ("g++", "clang++")
             if (path := shutil.which(name)) is not None]


def _read(name):
    return (ROOT / "src" / name).read_text()


def _body(text, signature):
    start = text.index(signature)
    opening = text.index("{", start)
    depth = 0
    for end in range(opening, len(text)):
        depth += (text[end] == "{") - (text[end] == "}")
        if depth == 0:
            return text[start:end + 1]
    raise AssertionError(f"unterminated function {signature}")


def _assigned_rhs(text, signature, target):
    body = _body(text, signature)
    match = re.search(rf"{re.escape(target)}\s*=\s*(.*?);", body, re.S)
    assert match, f"missing {target} assignment in {signature}"
    return " ".join(match.group(1).split())


def _paint_table_branches():
    body = _body(_read("graphicwin.cpp"), "void GraphicWin::on_paint(RECT *area)")
    start = body.index("        if ((iFlags_ & 0x1000000) == 0) {")
    draw_end = body.index(
        "Buffer::draw(destination, WinFillColour, x, y, 1, 1);", start)
    end = body.index("\n        }", draw_end) + len("\n        }")
    return body[start:end]


HARNESS = r'''
#include <cassert>
using byte = unsigned char;
using ColorTable = byte[256];
static byte ShadowColorTable[256];
static byte RedTintColorTable[256];
static byte BlueTintColorTable[256];
static ColorTable FadeColorTables[8];
struct Buffer {
    static int copy_calls, dest_calls, multi_calls, draw_calls;
    static const void *received;
    static int copy(Buffer *, int, int, int, int, int, int) {
        ++copy_calls; return 0;
    }
    static int draw_dest(Buffer *, int, int, int, int, int, byte *table) {
        ++dest_calls; received = table; return 0;
    }
    static int draw_multi_table_dest(Buffer *, int, int, int, int, int,
                                     ColorTable *table) {
        ++multi_calls; received = table; return 0;
    }
    static int draw(Buffer *, int, int, int, int, int) {
        ++draw_calls; return 0;
    }
};
int Buffer::copy_calls, Buffer::dest_calls, Buffer::multi_calls,
    Buffer::draw_calls;
const void *Buffer::received;
static int ExpansionEnabled;
struct Rect { int left, top, right, bottom; };
static byte *multi_debug_source(byte *shadow_table) {
    byte *field_A0_ = nullptr;
    field_A0_ = MULTI_DEBUG_RHS;
    return field_A0_;
}
static byte *setup_menu_source() {
    byte *field_A0_ = nullptr;
    field_A0_ = SETUP_WIN_RHS;
    return field_A0_;
}
static ColorTable *credits_source() {
    ColorTable *multi_color_tables_ = nullptr;
    multi_color_tables_ = CREDITS_RHS;
    return multi_color_tables_;
}
struct PaintOwner {
    unsigned int iFlags_;
    byte *color_table_;
    ColorTable *multi_color_tables_;
    int WinFillColour;
    Rect outer_rect_;
    Buffer storage;
    void paint_tables() {
        Buffer *destination = &storage;
        int x = 3, y = 4;
        PAINT_TABLE_BRANCHES
    }
};
'''


@pytest.mark.parametrize("compiler", COMPILERS or [None],
                         ids=lambda value: Path(value).name if value else "no-host-cxx")
def test_actual_drawing_table_producers_and_paint_selection(tmp_path, compiler):
    if compiler is None:
        pytest.skip("host GCC or Clang required")

    win_h = _read("win.h")
    win_cpp = _read("win.cpp")
    graphic = _read("graphicwin.cpp")
    multi = _read("multidebug.cpp")
    setup = _read("setupwin.cpp")
    credits = _read("credits.cpp")

    assert re.search(r"unsigned char\s*\*color_table_\s*;", win_h)
    assert re.search(r"ColorTable\s*\*multi_color_tables_\s*;", win_h)

    multi_rhs = _assigned_rhs(
        multi, "int MultiDebug::init(Net *net, Win *window, unsigned char *shadow_table)",
        "color_table_")
    setup_rhs = _assigned_rhs(
        setup, "int SetupWin::do_menu_rightside(char *filename, int flags)",
        "color_table_")
    credits_rhs = _assigned_rhs(credits, "void Credits::exec()", "graphicWin2_.multi_color_tables_")
    assert multi_rhs == "shadow_table"
    assert setup_rhs == "ExpansionEnabled ? RedTintColorTable : BlueTintColorTable"
    assert credits_rhs == "FadeColorTables"

    harness = (HARNESS.replace("MULTI_DEBUG_RHS", multi_rhs)
               .replace("SETUP_WIN_RHS", setup_rhs)
               .replace("CREDITS_RHS", credits_rhs)
               .replace("PAINT_TABLE_BRANCHES", _paint_table_branches()))
    cases = r'''
int main() {
    assert(multi_debug_source(ShadowColorTable) == ShadowColorTable);
    ExpansionEnabled = 1;
    assert(setup_menu_source() == RedTintColorTable);
    ExpansionEnabled = 0;
    assert(setup_menu_source() == BlueTintColorTable);
    assert(credits_source() == FadeColorTables);

    PaintOwner owner{};
    owner.iFlags_ = 0x1000000;
    owner.outer_rect_ = {0, 0, 8, 9};
    owner.color_table_ = ShadowColorTable;
    owner.multi_color_tables_ = FadeColorTables;
    owner.paint_tables();
    assert(Buffer::dest_calls == 1 && Buffer::multi_calls == 0);
    assert(Buffer::received == ShadowColorTable); // A0 has priority over A4.

    owner.color_table_ = nullptr;
    owner.paint_tables();
    assert(Buffer::dest_calls == 1 && Buffer::multi_calls == 1);
    assert(Buffer::received == FadeColorTables);

    owner.multi_color_tables_ = nullptr;
    owner.paint_tables();
    assert(Buffer::draw_calls == 1); // Null table pointers keep the plain draw.

    owner.iFlags_ = 0;
    owner.color_table_ = ShadowColorTable;
    owner.paint_tables();
    assert(Buffer::copy_calls == 1); // The ordinary copy branch remains first.
}
'''
    # Lifecycle zeroing and the existing mouse-recursion null gate stay explicit.
    constructor = _body(win_cpp, "Win::Win() {")
    close = _body(win_cpp, "void Win::close() {")
    mouse = _body(win_cpp, "Win *__cdecl get_mouse_window_recurse(")
    for body in (constructor, close):
        assert re.search(r"color_table_\s*=\s*0\s*;", body)
        assert re.search(r"multi_color_tables_\s*=\s*0\s*;", body)
    assert "window->color_table_ == 0" in mouse

    source = tmp_path / "win_drawing_tables.cpp"
    source.write_text(harness + cases)
    binary = tmp_path / "win_drawing_tables"
    built = subprocess.run([compiler, "-std=c++17", "-Wall", "-Wextra",
                            "-Werror", "-Wno-unused-parameter",
                            str(source), "-o", str(binary)],
                           capture_output=True, text=True)
    assert built.returncode == 0, built.stderr
    ran = subprocess.run([str(binary)], capture_output=True, text=True)
    assert ran.returncode == 0, ran.stdout + ran.stderr
