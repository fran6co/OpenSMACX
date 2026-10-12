"""Bounded source controls for Buffer's coordinate box operation.

The host fixture exercises the live extracted body and its four drawing
collaborators. It is not a full Buffer, SpriteBox, ABI, or game-runtime proof.
"""
from pathlib import Path
import os
import re
import shutil
import subprocess
import tempfile

import pytest
from match_recovery.source import read_file


ROOT = next(p for p in Path(__file__).resolve().parents
            if (p / "src/buffer.cpp").is_file())
PIN = "01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf"
ORIGINAL_PREFIX = "// ORIGINAL: 0x005E3193 ?box@Buffer@@QAEHHHHHHH@Z 0x005E3193-0x005E31FC"
ORIGINAL_NAME = "?box@Buffer@@QAEHHHHHHH@Z"
TRIED = "// TRIED: falling off the end without a return (C4716 under the scoring flags, treated as an error); the near-miss is the extra \"xor eax,eax\" that \"return 0\" adds versus the original's bare \"pop\"-chain tail."
SIGNATURE = "void Buffer::box(int left, int top, int right, int bottom, int color1, int color2)"


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
    sprite = (ROOT / "src/spritebox.cpp").read_text(encoding="utf-8")
    declaration = "void box(int left, int top, int right, int bottom, int color1, int color2);"
    assert declaration in header
    original_line = next((line for line in source.splitlines()
                          if line.startswith("// ORIGINAL: 0x005E3193 ")), "")
    assert original_line.startswith(ORIGINAL_PREFIX)
    record, = (row for row in read_file(ROOT / "src/buffer.cpp")
               if row.address == 0x005E3193)
    assert record.name == ORIGINAL_NAME
    assert record.image_spans == ((0x005E3193, 0x005E31FC),)
    assert TRIED in source
    assert "RECOVERY_BINDING: v1" in source[source.index(original_line):source.index(SIGNATURE)]
    box = function_body(source, SIGNATURE)
    assert "return" not in box
    assert box.count("hline(") == 2 and box.count("vline(") == 2
    caller = function_body(sprite, "void SpriteBox::draw_item(int position)")
    call = re.search(
        r"(?ms)^\s*static_cast<Buffer \*>\(static_cast<GraphicWin \*>\(this\)\)->box\((.*?)\);",
        caller)
    assert call
    assert "sprite_x, sprite_y, sprite_x + sprite_width - 1" in call.group(1)
    return box


PRELUDE = r'''#include <assert.h>
#include <limits.h>
struct Event { int kind, a, b, c, color; };
static Event events[8];
static int count;
static void push(int kind, int a, int b, int c, int color) {
  assert(count < 8);
  events[count].kind = kind;
  events[count].a = a;
  events[count].b = b;
  events[count].c = c;
  events[count].color = color;
  ++count;
}
struct Buffer {
  void hline(int x1, int x2, int y, int color) { push(1, x1, x2, y, color); }
  void vline(int x, int y1, int y2, int color) { push(2, x, y1, y2, color); }
  void box(int left, int top, int right, int bottom, int color1, int color2);
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
  b.box(5, 7, 17, 23, -3, 0x234567);
  assert(count == 4);
  check(0, 1, 6, 17, 7, -3);
  check(1, 1, 5, 16, 23, 0x234567);
  check(2, 2, 5, 7, 22, -3);
  check(3, 2, 17, 8, 23, 0x234567);

  // Do not normalize inverted/degenerate rectangles or negative colors.
  count = 0;
  b.box(-7, -4, -7, -4, INT_MIN, -1);
  assert(count == 4);
  check(0, 1, -6, -7, -4, INT_MIN);
  check(1, 1, -7, -8, -4, -1);
  check(2, 2, -7, -4, -5, INT_MIN);
  check(3, 2, -7, -3, -4, -1);
  return 0;
}
'''


def compile_run(program: str, compiler: str):
    with tempfile.TemporaryDirectory(prefix="buffer-box-contract-") as temp:
        source = Path(temp) / "box.cpp"
        exe = Path(temp) / "box"
        source.write_text(program, encoding="utf-8")
        built = subprocess.run(
            [compiler, "-std=c++98", "-Wall", "-Wextra", "-Werror", str(source), "-o", str(exe)],
            capture_output=True, text=True, timeout=45)
        assert built.returncode == 0, built.stdout + built.stderr
        return subprocess.run([str(exe)], capture_output=True, text=True, timeout=15)


def test_coordinate_box_header_identity_and_ignored_source_caller():
    actual_source()


def test_live_box_body_order_arithmetic_and_negative_controls():
    box = actual_source()
    compilers = [name for name in ("g++", "clang++") if shutil.which(name)]
    assert compilers
    good = PRELUDE + box + MAIN
    for compiler in compilers:
        result = compile_run(good, compiler)
        assert result.returncode == 0, compiler + ": " + result.stdout + result.stderr

        wrong_edge = box.replace("bottom - 1", "bottom", 1)
        assert wrong_edge != box
        assert compile_run(PRELUDE + wrong_edge + MAIN, compiler).returncode != 0

        wrong_color = box.replace("vline(left, top, bottom - 1, color1)",
                                  "vline(left, top, bottom - 1, color2)", 1)
        assert wrong_color != box
        assert compile_run(PRELUDE + wrong_color + MAIN, compiler).returncode != 0

        swapped = box.replace(
            "    hline(left + 1, right, top, color1);\n"
            "    hline(left, right - 1, bottom, color2);",
            "    hline(left, right - 1, bottom, color2);\n"
            "    hline(left + 1, right, top, color1);", 1)
        assert swapped != box
        assert compile_run(PRELUDE + swapped + MAIN, compiler).returncode != 0


def test_pinned_box_tail_and_known_caller_do_not_consume_a_result():
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

    original = Listing(image.read(Span(0x5E3193, 0x5E31FC)), 0x5E3193).instructions
    calls = [(i.address, i.op_str) for i in original if i.mnemonic == "call"]
    assert calls == [(0x5E31B2, "0x5e1a80"), (0x5E31C8, "0x5e1a80"),
                     (0x5E31DC, "0x5e1bf0"), (0x5E31F0, "0x5e1bf0")]
    assert [(i.mnemonic, i.op_str) for i in original[-5:]] == [
        ("pop", "edi"), ("pop", "esi"), ("pop", "ebp"), ("pop", "ebx"), ("ret", "0x18")]
    last_call = next(index for index, ins in enumerate(original) if ins.address == 0x5E31F0)
    assert all(i.mnemonic != "mov" or not i.op_str.startswith("eax,")
               for i in original[last_call + 1:])

    caller = Listing(image.read(Span(0x610FE3, 0x610FFB)), 0x610FE3).instructions
    assert (caller[0].address, caller[0].mnemonic, caller[0].op_str) == (
        0x610FE3, "call", "0x5e3193")
    assert caller[1].address == 0x610FE8 and caller[1].mnemonic == "mov"
    assert not any(i.mnemonic in ("test", "cmp") and "eax" in i.op_str
                   for i in caller[1:])
