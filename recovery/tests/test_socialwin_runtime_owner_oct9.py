"""Actual-source controls for SocialWin's typed runtime receivers."""

from pathlib import Path
import shutil
import subprocess

import pytest


ROOT = Path(__file__).resolve().parents[2]


def braced_statement(source: str, marker: str) -> str:
    start = source.index(marker)
    opening = source.index("{", start)
    depth = 0
    for end in range(opening, len(source)):
        depth += (source[end] == "{") - (source[end] == "}")
        if depth == 0:
            return source[start:end + 1]
    raise AssertionError(f"unterminated statement: {marker}")


SOURCE = (ROOT / "src/socialwin.cpp").read_text()


def test_socialwin_canvas_uses_real_main_interface_buffer_and_restores_clip():
    assert "InterfaceBuffer" not in SOURCE
    assert "InterfaceClipRect" not in SOURCE
    assert "reinterpret_cast<Buffer *>(0x007AEC64)" not in SOURCE
    for name in ("on_redraw", "draw_psych_alloc", "draw_economy_alloc",
                 "draw_labs_alloc", "draw_overview", "draw_mouseover",
                 "draw_info"):
        body = braced_statement(SOURCE, f"SocialWin::{name}(")
        assert "static_cast<Buffer *>(&MainInterfaceGlobal)" in body
    for name in ("on_redraw", "draw_psych_alloc", "draw_economy_alloc",
                 "draw_labs_alloc", "draw_overview", "draw_mouseover"):
        body = braced_statement(SOURCE, f"SocialWin::{name}(")
        assert "restore_clip()" in body


def test_language_branch_uses_source_owned_word():
    assert "extern uint32_t Language;" in SOURCE
    assert "0x009BC054" not in SOURCE
    assert SOURCE.count("if (Language != 2)") == 1


def test_map_window_virtual_base_and_null_argument_are_typed(tmp_path):
    start = SOURCE.index("            MapWin *window = MapWinTable[0];")
    end = SOURCE.index("                0, 1, -1, -1);", start) + len(
        "                0, 1, -1, -1);"
    )
    selected_window = SOURCE[start:end]
    assert "reinterpret_cast" not in selected_window

    harness = r"""
#include <cassert>
struct Buffer { int id; };
struct GraphicWin : Buffer {};
struct MapWin : virtual GraphicWin {};
MapWin *MapWinTable[8];
int TutSocWinText=19;
int calls;
GraphicWin *received;
int received_a, received_b;
struct Tutorial {
  void tut_win(GraphicWin *window, int, int a, int b, int, int, int, int) {
    ++calls; received=window; received_a=a; received_b=b;
  }
} TutorialWindow;
void run(int out_a, int out_b) {
__SELECTION__
}
int main() {
  MapWinTable[0]=0; run(3, 5); assert(calls==1 && received==0);
  MapWin map; GraphicWin *expected=static_cast<GraphicWin *>(&map);
  MapWinTable[0]=&map; run(7, 11);
  assert(calls==2 && received==expected && received_a==7 && received_b==11);
}
""".replace("__SELECTION__", selected_window)
    compiler = shutil.which("c++")
    if compiler is None:
        pytest.skip("host C++ compiler unavailable")
    source = tmp_path / "socialwin_map_receiver.cpp"
    executable = tmp_path / "socialwin_map_receiver"
    source.write_text(harness)
    subprocess.run([compiler, "-std=c++17", str(source), "-o", str(executable)],
                   check=True, capture_output=True, text=True, timeout=30)
    subprocess.run([str(executable)], check=True, capture_output=True,
                   text=True, timeout=10)


def test_language_branch_actual_source_statement(tmp_path):
    statement = braced_statement(SOURCE, "if (Language != 2)")
    harness = r"""
unsigned int Language;
int uppercase_calls;
char StringTemp[8];
void CharUpperA(char *) { ++uppercase_calls; }
void run() { __LANGUAGE_BRANCH__ }
int main() {
  Language=2; run(); if (uppercase_calls != 0) return 1;
  Language=1; run(); if (uppercase_calls != 1) return 2;
  Language=0; run(); return uppercase_calls == 2 ? 0 : 3;
}
""".replace("__LANGUAGE_BRANCH__", statement)
    compiler = shutil.which("c++")
    if compiler is None:
        pytest.skip("host C++ compiler unavailable")
    source = tmp_path / "socialwin_language.cpp"
    executable = tmp_path / "socialwin_language"
    source.write_text(harness)
    subprocess.run([compiler, "-std=c++17", str(source), "-o", str(executable)],
                   check=True, capture_output=True, text=True, timeout=30)
    subprocess.run([str(executable)], check=True, capture_output=True,
                   text=True, timeout=10)
