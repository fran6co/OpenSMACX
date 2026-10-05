"""Promotable actual-source controls for Win's two scrollbar wheel route."""

import pathlib
import re
import shutil
import subprocess
import tempfile

import pytest

from match_recovery.source import read_file


ROOT = next(parent for parent in pathlib.Path(__file__).resolve().parents
            if (parent / "src/win.cpp").is_file())
WIN_CPP = ROOT / "src" / "win.cpp"
ADDRESS = 0x5F85E0
LENGTH = 45


def _mapped_row():
    matches = [row for row in read_file(WIN_CPP) if row.address == ADDRESS]
    assert len(matches) == 1
    row = matches[0]
    assert row.path.resolve() == WIN_CPP.resolve()
    assert row.image_spans == ((ADDRESS, ADDRESS + LENGTH),)
    assert "?on_mousewheel_up@Win@@QAEXH@Z" in row.region
    return row


def _body():
    _mapped_row()
    text = WIN_CPP.read_text()
    match = re.search(r"\bvoid\s+Win::on_mousewheel_up\s*\([^;]*?\)\s*\{",
                      text, re.S)
    assert match is not None
    opening = text.index("{", match.start())
    depth = 0
    for index in range(opening, len(text)):
        if text[index] == "{":
            depth += 1
        elif text[index] == "}":
            depth -= 1
            if depth == 0:
                return text[match.start():index + 1]
    raise AssertionError("unterminated Win::on_mousewheel_up definition")


def _fixture(body, main):
    return r'''#include <cassert>
struct Scroll {
  int id;
  static int count;
  static int ids[8];
  static int deltas[8];
  void on_mousewheel_up(int delta) {
    ids[count] = id; deltas[count] = delta; ++count;
  }
};
int Scroll::count = 0;
int Scroll::ids[8] = {};
int Scroll::deltas[8] = {};
class Win {
 public:
  Scroll *scroll_vert_;
  Scroll *scroll_horz_;
  Win() : scroll_vert_(0), scroll_horz_(0) {}
  void on_mousewheel_up(int delta);
};
''' + body + main


def _compile_and_run(body, main, should_pass=True):
    compiler = shutil.which("g++") or shutil.which("clang++")
    if compiler is None:
        pytest.skip("requires a host C++ compiler")
    with tempfile.TemporaryDirectory(prefix="osmx-win-wheelup-") as temp:
        unit = pathlib.Path(temp) / "win_wheelup.cpp"
        binary = pathlib.Path(temp) / "win_wheelup"
        unit.write_text(_fixture(body, main))
        compiled = subprocess.run(
            [compiler, "-std=c++98", "-Wall", "-Wextra", "-Werror",
             str(unit), "-o", str(binary)],
            capture_output=True, text=True, timeout=30,
        )
        assert compiled.returncode == 0, compiled.stderr
        result = subprocess.run([str(binary)], capture_output=True, text=True, timeout=30)
        if should_pass:
            assert result.returncode == 0, result.stderr
        else:
            assert result.returncode != 0
            assert "assert" in result.stderr.lower(), result.stderr


def test_native_identity_and_product_route_both_scrollbars_in_order():
    body = _body()
    main = r'''int main() {
  Scroll vertical = {1}, horizontal = {2};
  Win win; win.scroll_vert_ = &vertical; win.scroll_horz_ = &horizontal;
  win.on_mousewheel_up(-120);
  assert(Scroll::count == 2);
  assert(Scroll::ids[0] == 1 && Scroll::ids[1] == 2);
  assert(Scroll::deltas[0] == -120 && Scroll::deltas[1] == -120);
  Scroll::count = 0; win.scroll_vert_ = 0;
  win.on_mousewheel_up(120);
  assert(Scroll::count == 1 && Scroll::ids[0] == 2 &&
         Scroll::deltas[0] == 120);
  Scroll::count = 0; win.scroll_horz_ = 0;
  win.on_mousewheel_up(1);
  assert(Scroll::count == 0);
}'''
    _compile_and_run(body, main)


def test_controls_reject_route_order_and_pointer_mutants():
    body = _body()
    order_mutant = body.replace(
        "Scroll *first = scroll_vert_;", "Scroll *first = scroll_horz_;", 1
    ).replace(
        "Scroll *second = scroll_horz_;", "Scroll *second = scroll_vert_;", 1
    )
    assert order_mutant != body
    main = r'''int main() {
  Scroll vertical = {1}, horizontal = {2};
  Win win; win.scroll_vert_ = &vertical; win.scroll_horz_ = &horizontal;
  win.on_mousewheel_up(9);
  assert(Scroll::ids[0] == 1 && Scroll::ids[1] == 2);
}'''
    _compile_and_run(body, main)
    _compile_and_run(order_mutant, main, should_pass=False)

    wrong_member = body.replace(
        "Scroll *second = scroll_horz_;", "Scroll *second = scroll_vert_;", 1
    )
    assert wrong_member != body
    main = r'''int main() {
  Scroll horizontal = {2};
  Win win; win.scroll_vert_ = 0; win.scroll_horz_ = &horizontal;
  win.on_mousewheel_up(9);
  assert(Scroll::count == 1 && Scroll::ids[0] == 2);
}'''
    _compile_and_run(body, main)
    _compile_and_run(wrong_member, main, should_pass=False)
