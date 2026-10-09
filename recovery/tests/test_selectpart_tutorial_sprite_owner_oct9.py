"""Pin the SelectPart calls to the genuine tutorial BaseWin Sprite member."""

from __future__ import annotations

import shutil
import subprocess
from pathlib import Path

import pytest

from match_recovery.source import read_text


ROOT = Path(__file__).resolve().parents[2]


def _definition(source: str, signature: str) -> str:
    start = source.index(signature)
    opening = source.index("{", start)
    depth = 0
    quote = None
    escaped = False
    line_comment = False
    block_comment = False
    index = opening
    while index < len(source):
        char = source[index]
        following = source[index + 1] if index + 1 < len(source) else ""
        if line_comment:
            if char == "\n":
                line_comment = False
        elif block_comment:
            if char == "*" and following == "/":
                block_comment = False
                index += 1
        elif quote:
            if escaped:
                escaped = False
            elif char == "\\":
                escaped = True
            elif char == quote:
                quote = None
        elif char == "/" and following == "/":
            line_comment = True
            index += 1
        elif char == "/" and following == "*":
            block_comment = True
            index += 1
        elif char in ('"', "'"):
            quote = char
        elif char == "{":
            depth += 1
        elif char == "}":
            depth -= 1
            if depth == 0:
                return source[start : index + 1]
        index += 1
    raise AssertionError(f"unterminated definition: {signature}")


def _statement(body: str, marker: str) -> str:
    start = body.index(marker)
    end = body.index(";", start) + 1
    return body[start:end]


def test_selectpart_sprite_calls_use_the_existing_typed_member(tmp_path: Path) -> None:
    source_path = ROOT / "src/selectpartwin.cpp"
    source = source_path.read_text()
    basewin_header = (ROOT / "src/basewin.h").read_text()
    tutwin_header = (ROOT / "src/tutwin.h").read_text()
    basewin_source = (ROOT / "src/basewin.cpp").read_text()
    assert "#include \"tutwin.h\"" in source
    assert "SelectPartTile" not in source
    assert "0x006B33F8" not in source
    assert "reinterpret_cast<Sprite *>" not in source
    assert "Sprite sprite6_;  // 0xBDD0" in basewin_header
    assert "extern BaseWin TutWinBaseWindow;    // 0x006A7628" in tutwin_header
    assert "sprite6_.extract(&assets, WinFillColour, 439, 64, 20, 20, 0);" in basewin_source
    assert 0x006A7628 + 0xBDD0 == 0x006B33F8

    records = {record.address: record for record in read_text(source, "src/selectpartwin.cpp")}
    assert records[0x00433F70].image_spans[0] == (0x00433F70, 0x0043446F)
    assert records[0x00433D60].image_spans[0] == (0x00433D60, 0x00433F1D)

    exec_body = _definition(source, "int SelectPartWin::exec(")
    draw_body = _definition(source, "int SelectPartWin::draw_part(")
    exec_call = _statement(exec_body, "TutWinBaseWindow.sprite6_.draw(")
    tile_call = _statement(
        draw_body,
        "static_cast<Buffer *>(this)\n            ->tile(&TutWinBaseWindow.sprite6_",
    )

    compiler = shutil.which("g++") or shutil.which("clang++")
    if compiler is None:
        pytest.skip("host C++ compiler unavailable")
    fixture = r'''
#include <cassert>
struct Buffer;
struct Sprite {
    int draw_calls = 0;
    int draw_transparent = 0;
    int draw_args[4] = {};
    int tile_calls = 0;
    Buffer *draw_destination = nullptr;
    int tile_args[6] = {};
    int draw(Buffer *destination, int transparent, int x, int y,
             int width, int height) {
        ++draw_calls;
        draw_destination = destination;
        draw_transparent = transparent;
        draw_args[0] = x; draw_args[1] = y;
        draw_args[2] = width; draw_args[3] = height;
        return 0;
    }
    int transparent_index() const { return 17; }
};
struct Buffer {
    Sprite *tile_source = nullptr;
    int tile_calls = 0;
    int tile_args[6] = {};
    int tile(Sprite *source, int x0, int y0, int x1, int y1,
             int width, int height) {
        ++tile_calls;
        tile_source = source;
        tile_args[0] = x0; tile_args[1] = y0;
        tile_args[2] = x1; tile_args[3] = y1;
        tile_args[4] = width; tile_args[5] = height;
        return 0;
    }
};
struct BaseWin { Sprite sprite6_; } TutWinBaseWindow;
struct SelectPartWin : Buffer {
    void draw_tile(int x0, int y0) {
'''+tile_call+r'''
    }
};
void draw_backdrop() {
    Buffer backdrop;
'''+exec_call+r'''
    assert(TutWinBaseWindow.sprite6_.draw_calls == 1);
    assert(TutWinBaseWindow.sprite6_.draw_destination == &backdrop);
    assert(TutWinBaseWindow.sprite6_.draw_transparent == 17);
    assert(TutWinBaseWindow.sprite6_.draw_args[0] == 0);
    assert(TutWinBaseWindow.sprite6_.draw_args[1] == 0);
    assert(TutWinBaseWindow.sprite6_.draw_args[2] == 1);
    assert(TutWinBaseWindow.sprite6_.draw_args[3] == 1);
}
int main() {
    draw_backdrop();
    SelectPartWin window;
    window.draw_tile(23, 41);
    assert(window.tile_calls == 1);
    assert(window.tile_source == &TutWinBaseWindow.sprite6_);
    assert(window.tile_args[0] == 23 && window.tile_args[1] == 41);
    assert(window.tile_args[2] == 23 && window.tile_args[3] == 41);
    assert(window.tile_args[4] == 0x69 && window.tile_args[5] == 0x5B);
}
'''
    cpp = tmp_path / "selectpart_sprite_owner.cpp"
    binary = tmp_path / "selectpart_sprite_owner"
    cpp.write_text(fixture)
    subprocess.run(
        [compiler, "-std=c++11", "-Wall", "-Wextra", "-Werror", str(cpp), "-o", str(binary)],
        check=True,
        timeout=30,
    )
    subprocess.run([str(binary)], check=True, timeout=30)

