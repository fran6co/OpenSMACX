"""Bounded source/host controls for Buffer's two dotted-box contracts."""
from pathlib import Path
import os
import shutil
import subprocess
import tempfile

import pytest
from match_recovery.source import read_file


ROOT = next(p for p in Path(__file__).resolve().parents
            if (p / "src/buffer.cpp").is_file())
PIN = "01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf"
COORD_PREFIX = "// ORIGINAL: 0x005E3283 ?dotted_box@Buffer@@QAEHHHHHH@Z"
RECT_PREFIX = "// ORIGINAL: 0x005E32F3 ?dotted_box@Buffer@@QAEHPAURECT@@H@Z"
COORD_SIGNATURE = "void Buffer::dotted_box(int left, int top, int right, int bottom, int color)"
RECT_SIGNATURE = "int Buffer::dotted_box(RECT * a1, int a2)"


def function_body(source: str, signature: str) -> str:
    start = source.index(signature)
    opening = source.index("{", start)
    depth = 0
    for at in range(opening, len(source)):
        if source[at] == "{":
            depth += 1
        elif source[at] == "}":
            depth -= 1
            if depth == 0:
                return source[start:at + 1]
    raise AssertionError("unterminated function: " + signature)


def actual_source():
    header = (ROOT / "src/buffer.h").read_text(encoding="utf-8")
    source = (ROOT / "src/buffer.cpp").read_text(encoding="utf-8")
    assert "void dotted_box(int left, int top, int right, int bottom, int color);" in header
    assert "int dotted_box(RECT *area, int color);" in header

    coord_comment = source[source.index(COORD_PREFIX):source.index(COORD_SIGNATURE)]
    assert "prototype int (__thiscall ?dotted_box@Buffer@@QAEHHHHHH@Z)" in coord_comment
    assert "reconstructed history, not an authentic original symbol record" in coord_comment
    assert "indirect use remains unverified" in coord_comment
    assert "RECOVERY_BINDING: v1" in coord_comment
    rect_comment = source[source.index(RECT_PREFIX):source.index(RECT_SIGNATURE)]
    assert "RECOVERY_BINDING" not in rect_comment

    coord_record, = (row for row in read_file(ROOT / "src/buffer.cpp")
                     if row.address == 0x005E3283)
    rect_record, = (row for row in read_file(ROOT / "src/buffer.cpp")
                    if row.address == 0x005E32F3)
    assert coord_record.name == "?dotted_box@Buffer@@QAEHHHHHH@Z"
    assert coord_record.image_spans == ((0x005E3283, 0x005E32E4),)
    assert rect_record.name == "?dotted_box@Buffer@@QAEHPAURECT@@H@Z"
    assert rect_record.image_spans == ((0x005E32F3, 0x005E3370),)

    coord = function_body(source, COORD_SIGNATURE)
    assert "return" not in coord
    expected = (
        "dotted_hline(left + 1, right, top, color);",
        "dotted_hline(left, right - 1, bottom, color);",
        "dotted_vline(left, top, bottom - 1, color);",
        "dotted_vline(right, top + 1, bottom, color);",
    )
    assert tuple(line.strip() for line in coord.splitlines()
                 if line.strip().startswith(("dotted_hline(", "dotted_vline("))) == expected

    rect = function_body(source, RECT_SIGNATURE)
    assert "return 0x10;" in rect and "return 0;" in rect
    assert rect.count("dotted_hline(") == 2 and rect.count("dotted_vline(") == 2
    return coord, rect


PRELUDE = r'''#include <assert.h>
#include <limits.h>
struct RECT { int left, top, right, bottom; };
struct Event { int kind, a, b, c, color; };
static Event events[16];
static int count;
static void push(int kind, int a, int b, int c, int color) {
  assert(count < 16);
  events[count].kind = kind;
  events[count].a = a;
  events[count].b = b;
  events[count].c = c;
  events[count].color = color;
  ++count;
}
struct Buffer {
  void dotted_hline(int x1, int x2, int y, int color) { push(1, x1, x2, y, color); }
  void dotted_vline(int x, int y1, int y2, int color) { push(2, x, y1, y2, color); }
  void dotted_box(int left, int top, int right, int bottom, int color);
  int dotted_box(RECT *area, int color);
};
'''


MAIN = r'''
static void check(int index, int kind, int a, int b, int c, int color) {
  assert(events[index].kind == kind);
  assert(events[index].a == a && events[index].b == b &&
         events[index].c == c && events[index].color == color);
}
int main() {
  Buffer b;
  b.dotted_box(5, 7, 17, 23, -3);
  assert(count == 4);
  check(0, 1, 6, 17, 7, -3);
  check(1, 1, 5, 16, 23, -3);
  check(2, 2, 5, 7, 22, -3);
  check(3, 2, 17, 8, 23, -3);

  // Preserve the original unnormalized arithmetic for degenerate coordinates.
  count = 0;
  b.dotted_box(-7, -4, -7, -4, INT_MIN);
  assert(count == 4);
  check(0, 1, -6, -7, -4, INT_MIN);
  check(1, 1, -7, -8, -4, INT_MIN);
  check(2, 2, -7, -4, -5, INT_MIN);
  check(3, 2, -7, -3, -4, INT_MIN);

  RECT area = {5, 7, 17, 23};
  count = 0;
  assert(b.dotted_box(0, -3) == 0x10);
  assert(count == 0);
  assert(b.dotted_box(&area, -3) == 0);
  assert(count == 4);
  check(0, 1, 6, 16, 7, -3);
  check(1, 1, 5, 15, 22, -3);
  check(2, 2, 5, 7, 21, -3);
  check(3, 2, 16, 8, 22, -3);
  return 0;
}
'''


def compile_run(program: str, compiler: str):
    with tempfile.TemporaryDirectory(prefix="buffer-dotted-box-contract-") as temp:
        source = Path(temp) / "dotted_box.cpp"
        exe = Path(temp) / "dotted_box"
        source.write_text(program, encoding="utf-8")
        built = subprocess.run(
            [compiler, "-std=c++98", "-Wall", "-Wextra", "-Werror",
             str(source), "-o", str(exe)],
            capture_output=True, text=True, timeout=45)
        assert built.returncode == 0, built.stdout + built.stderr
        return subprocess.run([str(exe)], capture_output=True, text=True, timeout=15)


def test_coordinate_void_and_rect_int_declarations_and_source_contracts():
    actual_source()


def test_live_overloads_preserve_each_result_and_drawing_contract():
    coord, rect = actual_source()
    compilers = [name for name in ("g++", "clang++") if shutil.which(name)]
    assert compilers
    good = PRELUDE + coord + "\n" + rect + "\n" + MAIN
    for compiler in compilers:
        result = compile_run(good, compiler)
        assert result.returncode == 0, compiler + ": " + result.stdout + result.stderr

        wrong_argument = coord.replace("right - 1, bottom, color",
                                       "right, bottom, color", 1)
        assert wrong_argument != coord
        assert compile_run(PRELUDE + wrong_argument + "\n" + rect + "\n" + MAIN,
                          compiler).returncode != 0

        swapped = coord.replace(
            "    dotted_hline(left + 1, right, top, color);\n"
            "    dotted_hline(left, right - 1, bottom, color);",
            "    dotted_hline(left, right - 1, bottom, color);\n"
            "    dotted_hline(left + 1, right, top, color);", 1)
        assert swapped != coord
        assert compile_run(PRELUDE + swapped + "\n" + rect + "\n" + MAIN,
                          compiler).returncode != 0


def test_pinned_bodies_distinguish_coordinate_epilogue_from_rect_status():
    from match_recovery.model import Span
    from match_recovery.pe_coff import PeImage
    from match_recovery.source.asm import Listing

    explicit = os.environ.get("OPENSMACX_IMAGE")
    candidates = [Path(explicit)] if explicit else [
        parent / ".opensmacx/game/terranx_original.exe" for parent in (ROOT, *ROOT.parents)]
    image_path = next((path for path in candidates if path.is_file()), None)
    if image_path is None:
        pytest.skip("pinned original image is unavailable")
    image = PeImage(image_path.read_bytes(), PIN)

    coord = Listing(image.read(Span(0x5E3283, 0x5E32E4)), 0x5E3283).instructions
    calls = [(i.address, i.op_str) for i in coord if i.mnemonic == "call"]
    assert calls == [(0x5E329E, "0x5e1d70"), (0x5E32B4, "0x5e1d70"),
                     (0x5E32C8, "0x5e1ef0"), (0x5E32D8, "0x5e1ef0")]
    last_call = next(index for index, ins in enumerate(coord) if ins.address == 0x5E32D8)
    assert [(i.mnemonic, i.op_str) for i in coord[last_call + 1:]] == [
        ("pop", "edi"), ("pop", "esi"), ("pop", "ebp"), ("pop", "ebx"), ("ret", "0x14")]
    assert not any(i.mnemonic in ("xor", "mov", "test") and i.op_str.startswith("eax")
                   for i in coord[last_call + 1:])

    rect = Listing(image.read(Span(0x5E32F3, 0x5E3370)), 0x5E32F3).instructions
    assert (0x5E32FE, "mov", "eax, 0x10") in [
        (i.address, i.mnemonic, i.op_str) for i in rect]
    assert (0x5E336A, "xor", "eax, eax") in [
        (i.address, i.mnemonic, i.op_str) for i in rect]
    assert [(i.mnemonic, i.op_str) for i in rect[-6:]] == [
        ("pop", "edi"), ("pop", "esi"), ("pop", "ebp"), ("xor", "eax, eax"),
        ("pop", "ebx"), ("ret", "8")]
