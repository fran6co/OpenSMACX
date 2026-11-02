"""Compile the actual MessageWin::init body to check its local Buffer lifetime."""
from pathlib import Path
import shutil
import subprocess

import pytest


ROOT = Path(__file__).resolve().parents[2]
SIGNATURE = "void MessageWin::init()"
FONT_CALL = "font_.init(DefaultFontFace, 12, 0);"
BUFFER_DECL = "Buffer scratch;"


def definition(text: str) -> str:
    start = text.index(SIGNATURE)
    opening = text.index("{", start)
    depth = 0
    for index in range(opening, len(text)):
        if text[index] == "{":
            depth += 1
        elif text[index] == "}":
            depth -= 1
            if depth == 0:
                return text[start:index + 1]
    raise AssertionError("unterminated MessageWin::init")


def fixture(function: str) -> str:
    return r'''#include <assert.h>
#include <string>
#include <string>
#define __cdecl

static std::string events;
static int throw_from_font = 0;
static void record(char event) { events += event; }

struct RECT { long left, top, right, bottom; };
struct Heap {};
struct MainInterface { RECT message_bounds_; };
static MainInterface MainInterfaceGlobal = {{11, 12, 111, 212}};
static int MessageWinMessageCount = 99;
static const char *DefaultFontFace = "default";

struct GraphicWin {};
struct Buffer {
    Buffer() { record('B'); }
    ~Buffer() { record('D'); }
    void close() { record('C'); }
};
struct Font {
    void init(const char *, int, int) {
        record('F');
        if (throw_from_font) throw 17;
    }
};
struct Dialog {
    int field_38_, field_3C_, field_6C_, field_4C_, field_48_, field_34_, field_B4_;
    int rect_size_init_calls_;
    int rect_heap_init_calls_;
    Heap *rect_heap_argument_;
    Dialog() : rect_size_init_calls_(0), rect_heap_init_calls_(0),
        rect_heap_argument_(0) {}
    void close() { record('L'); }
    void init(RECT *, int) { ++rect_size_init_calls_; record('I'); }
    void init(RECT *, Heap *heap) {
        ++rect_heap_init_calls_;
        rect_heap_argument_ = heap;
        record('H');
    }
    void set_dialog_font(Font *, Font *, Font *) { record('P'); }
    void set_dialog_text_color(int, int, int, int) { record('1'); }
    void set_dialog_text_color2(int, int, int, int) { record('2'); }
    void set_dialog_text_color3(int, int, int, int) { record('3'); }
    void attach(MainInterface *, long, long, int) { record('A'); }
};
struct ListBox : Dialog {
    typedef void (*Draw)(GraphicWin *, int, int, int, int, int);
    typedef int (*Write)(GraphicWin *, int, int, RECT *, char *);
    typedef int (*Fill)(GraphicWin *, int, int, RECT *);
    void set_message_initial_state() { record('S'); }
    void set_callbacks(Draw draw, Write write, Fill fill) {
        assert(draw != 0 && write != 0 && fill != 0);
        record('K');
    }
};
struct MessageWin {
    Font font_;
    RECT bounds_;
    ListBox listBox_;
    static void __cdecl listbox_draw(GraphicWin *, int, int, int, int, int) {}
    static int __cdecl listbox_write(GraphicWin *, int, int, RECT *, char *) { return 0; }
    static int __cdecl listbox_fill(GraphicWin *, int, int, RECT *) { return 0; }
    void init();
};
''' + function + r'''
int main() {
    MessageWin window;
    events.clear();
    window.init();
    assert(events == "BFCLHPS123KAD");
    assert(window.listBox_.rect_heap_init_calls_ == 1);
    assert(window.listBox_.rect_heap_argument_ == 0);
    assert(window.listBox_.rect_size_init_calls_ == 0);
    assert(window.bounds_.left == 11 && window.bounds_.top == 12);
    assert(window.bounds_.right == 111 && window.bounds_.bottom == 212);
    assert(MessageWinMessageCount == 0);

    events.clear();
    throw_from_font = 1;
    try {
        window.init();
        assert(!"Font::init should throw in this control");
    } catch (int value) {
        assert(value == 17);
    }
    assert(events == "BFD");
}
'''


def compile_and_run(compiler: str, tmp_path: Path, source: str):
    cpp = tmp_path / "messagewin_init_lifetime.cpp"
    exe = tmp_path / "messagewin_init_lifetime"
    cpp.write_text(source, encoding="utf-8")
    built = subprocess.run(
        [compiler, "-std=c++98", "-Wall", "-Wextra", str(cpp), "-o", str(exe)],
        capture_output=True, text=True, timeout=30, check=False,
    )
    assert built.returncode == 0, built.stdout + built.stderr
    return subprocess.run([str(exe)], capture_output=True, text=True,
                          timeout=10, check=False)


def test_actual_messagewin_init_constructs_buffer_before_font_and_closes_later():
    source = (ROOT / "src/messagewin.cpp").read_text(encoding="utf-8")
    body = definition(source)
    assert BUFFER_DECL in body and FONT_CALL in body
    assert body.index(BUFFER_DECL) < body.index(FONT_CALL)
    assert body.index(FONT_CALL) < body.index("scratch.close();")
    assert "// ORIGINAL: 0x00471D50" in source


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
def test_actual_init_order_and_unwind_under_host_compilers(compiler_name, tmp_path):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(f"{compiler_name} is unavailable")
    source = (ROOT / "src/messagewin.cpp").read_text(encoding="utf-8")
    body = definition(source)
    result = compile_and_run(compiler, tmp_path, fixture(body))
    assert result.returncode == 0, result.stdout + result.stderr


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
def test_declaration_after_font_init_mutant_is_rejected(compiler_name, tmp_path):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(f"{compiler_name} is unavailable")
    source = (ROOT / "src/messagewin.cpp").read_text(encoding="utf-8")
    body = definition(source)
    correct_order = BUFFER_DECL + "\n    " + FONT_CALL
    reversed_order = FONT_CALL + "\n    " + BUFFER_DECL
    mutant = body.replace(correct_order, reversed_order, 1)
    assert mutant != body
    result = compile_and_run(compiler, tmp_path, fixture(mutant))
    assert result.returncode != 0, "post-font Buffer construction unexpectedly passed"


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
def test_integer_zero_overload_mutant_is_rejected(compiler_name, tmp_path):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(f"{compiler_name} is unavailable")
    source = (ROOT / "src/messagewin.cpp").read_text(encoding="utf-8")
    body = definition(source)
    correct = "listBox_.Dialog::init(&bounds_, static_cast<Heap *>(0));"
    mutant = body.replace(correct, "listBox_.Dialog::init(&bounds_, 0);", 1)
    assert mutant != body
    result = compile_and_run(compiler, tmp_path, fixture(mutant))
    assert result.returncode != 0, "integer-zero overload mutant unexpectedly passed"
