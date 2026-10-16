"""Host controls for MessageWin's ListBox draw-callback argument slots."""
from pathlib import Path
import shutil
import subprocess

import pytest


ROOT = Path(__file__).resolve().parents[2]
SIGNATURE = "void __cdecl MessageWin::listbox_draw("
OLD_SIGNATURE = (
    "void __cdecl MessageWin::listbox_draw(GraphicWin *target, int, int x, int y,\n"
    "                                       int row_height, int)"
)
NEW_SIGNATURE = (
    "void __cdecl MessageWin::listbox_draw(GraphicWin *target, int, int, int left,\n"
    "                                       int top, int height)"
)


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
    raise AssertionError("unterminated MessageWin::listbox_draw")


def fixture(function: str) -> str:
    return r'''#include <assert.h>
#define __cdecl
struct Buffer { int marker; };
struct WindowBase { int marker; };
struct GraphicWin : WindowBase, Buffer {};
struct Sprite {
    int transparent_index() const { return 0x5a; }
    int height() const { return 9; }
    void draw(Buffer *target, int transparent, int x, int y, int a, int b) {
        seen_target = target; seen_transparent = transparent;
        seen_x = x; seen_y = y; seen_a = a; seen_b = b;
    }
    static Buffer *seen_target;
    static int seen_transparent, seen_x, seen_y, seen_a, seen_b;
};
Buffer *Sprite::seen_target = 0;
int Sprite::seen_transparent = 0, Sprite::seen_x = 0, Sprite::seen_y = 0;
int Sprite::seen_a = 0, Sprite::seen_b = 0;
Sprite g_IFACE_GREEN_RIGHT_ARROW_SPRITE[1];
struct MessageWin { static void __cdecl listbox_draw(GraphicWin *, int, int, int, int, int); };
''' + function + r'''
int main() {
    GraphicWin target;
    target.WindowBase::marker = 17;
    target.Buffer::marker = 19;
    const int item_id = 0x1a2b;
    const int list_offset = 0x1357;
    const int left = 41, top = 83, height = 27;
    MessageWin::listbox_draw(&target, item_id, list_offset, left, top, height);
    assert(Sprite::seen_target == static_cast<Buffer *>(&target));
    assert(Sprite::seen_transparent == 0x5a);
    assert(Sprite::seen_x == left + 4);
    assert(Sprite::seen_y == top + (height - 9) / 2);
    assert(Sprite::seen_a == 1 && Sprite::seen_b == 1);
}
'''


def compile_and_run(compiler: str, tmp_path: Path, source: str):
    cpp = tmp_path / "message_list_draw.cpp"
    exe = tmp_path / "message_list_draw"
    cpp.write_text(source, encoding="utf-8")
    built = subprocess.run(
        [compiler, "-std=c++11", "-Wall", "-Wextra", str(cpp), "-o", str(exe)],
        capture_output=True, text=True, timeout=30, check=False,
    )
    assert built.returncode == 0, built.stdout + built.stderr
    return subprocess.run([str(exe)], capture_output=True, text=True,
                          timeout=10, check=False)


def test_callback_argument_order_is_pinned_by_actual_listbox_caller():
    caller = (ROOT / "src/listbox.cpp").read_text(encoding="utf-8")
    call = "draw_callback_(window, dialog.pos_to_id(index),\n                       static_cast<int>(field_18_) + ListBoxDrawOffset,\n                       row.left, row.top, row.bottom - row.top);"
    assert call in caller
    source = (ROOT / "src/messagewin.cpp").read_text(encoding="utf-8")
    header = (ROOT / "src/messagewin.h").read_text(encoding="utf-8")
    assert NEW_SIGNATURE in source
    assert "int list_offset, int left, int top,\n                                   int height" in header
    body = definition(source)
    assert "left + 4" in body and "top + (height - arrow.height()) / 2" in body
    assert "row_height" not in body and "list_offset" not in body


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
def test_actual_callback_uses_left_top_and_height_slots(compiler_name, tmp_path):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(f"{compiler_name} is unavailable")
    source = (ROOT / "src/messagewin.cpp").read_text(encoding="utf-8")
    function = definition(source)
    assert NEW_SIGNATURE in function
    result = compile_and_run(compiler, tmp_path, fixture(function))
    assert result.returncode == 0, result.stdout + result.stderr


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
def test_old_signature_mutant_misbinds_geometry_slots(compiler_name, tmp_path):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(f"{compiler_name} is unavailable")
    source = (ROOT / "src/messagewin.cpp").read_text(encoding="utf-8")
    function = definition(source)
    assert NEW_SIGNATURE in function
    mutant = function.replace(NEW_SIGNATURE, OLD_SIGNATURE, 1)
    mutant = mutant.replace("left + 4", "x + 4", 1)
    mutant = mutant.replace("top + (height - arrow.height()) / 2",
                            "y + (row_height - arrow.height()) / 2", 1)
    assert mutant != function
    result = compile_and_run(compiler, tmp_path, fixture(mutant))
    assert result.returncode != 0, "old callback argument mapping unexpectedly survived"
