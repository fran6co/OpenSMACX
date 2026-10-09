"""Actual-source regression controls for FameWin's native award highlight."""
from pathlib import Path
import shutil
import subprocess

import pytest


ROOT = Path(__file__).resolve().parents[2]
SIGNATURE = "void FameWin::on_redraw() {"
BLOCK_START = "for (int banner_award_index = 0; banner_award_index < 6;"


def source_award_block() -> str:
    source = (ROOT / "src/famewin.cpp").read_text()
    body_start = source.index(SIGNATURE)
    start = source.index(BLOCK_START, body_start)
    opening = source.index("{", start)
    depth = 0
    for pos in range(opening, len(source)):
        if source[pos] == "{":
            depth += 1
        elif source[pos] == "}":
            depth -= 1
            if depth == 0:
                return source[start : pos + 1]
    raise AssertionError("unterminated FameWin award-highlight loop")


HARNESS = r"""
#include <cassert>
#include <cstdio>
#include <cstring>

char StringTemp[128];
const int award_labels[6] = {0, 1, 2, 3, 4, 5};
const char *label_get(int) { return "Award"; }
void famewin_clear_text() { StringTemp[0] = '\0'; }
void famewin_append_label(int index) { std::strcat(StringTemp, label_get(index)); }

struct HallOfFameRecord { unsigned int award_flags; };
struct FameWin;
struct Buffer {
  FameWin *owner;
  int text_width_calls;
  int fill_extents[2];
  int write_x, write_y, write_width, write_length;
  char written[128];
  Buffer() : owner(0), text_width_calls(0), write_x(0), write_y(0),
      write_width(0), write_length(0) { written[0] = '\0'; }
  int text_width(char *);
  int write_cent_l(char *text, int x, int y, int width, int length) {
    write_x = x; write_y = y; write_width = width; write_length = length;
    std::strncpy(written, text, sizeof(written) - 1);
    written[sizeof(written) - 1] = '\0';
    return 0;
  }
};
struct FameWin {
  int field_A24_, field_A28_;
  int fill_x, fill_y, fill_width, fill_height, fill_color, fills;
  FameWin() : field_A24_(500), field_A28_(100), fill_x(0), fill_y(0),
      fill_width(0), fill_height(0), fill_color(0), fills(0) {}
  void fill(int x, int y, int width, int height, int color) {
    ++fills;
    fill_x = x; fill_y = y; fill_width = width;
    fill_height = height; fill_color = color;
    // Make source read timing observable after the fill call as well.
    field_A24_ = 800;
    field_A28_ = 300;
  }
  void draw_award(HallOfFameRecord *record, Buffer *canvas) {
AWARD_BLOCK
  }
};
int Buffer::text_width(char *) {
  ++text_width_calls;
  if (text_width_calls == 1) {
    owner->field_A28_ = 200;
    return 31;
  }
  assert(text_width_calls == 2);
  owner->field_A24_ = 700;
  return 50;
}

int main() {
  HallOfFameRecord record = {1};
  FameWin window;
  Buffer canvas;
  canvas.owner = &window;
  window.draw_award(&record, &canvas);
  assert(canvas.text_width_calls == 2);
  assert(window.fills == 1);
  // First width 31 supplies the native extent 31+0x14; the second width 50
  // supplies x after its callback mutates field_A24_.
  assert(window.fill_x == 700 - 50 / 2 + 0x186);
  assert(window.fill_y == 200 + 0x0F);
  assert(window.fill_width == 31 + 0x14);
  assert(window.fill_height == 0x15 && window.fill_color == 0x41);
  // The native write reloads both fields after fill and uses strlen, not the
  // preceding text-width return, as its final argument.
  assert(canvas.write_x == 800 && canvas.write_y == 300 + 0x10);
  assert(canvas.write_width == 0x320 && canvas.write_length == 5);
  assert(!std::strcmp(canvas.written, "Award"));
}
"""


def run_host_control(tmp_path, block: str, *, should_pass: bool) -> None:
    compiler = shutil.which("g++") or shutil.which("clang++")
    if not compiler:
        pytest.skip("host C++ compiler unavailable")
    harness = HARNESS.replace("AWARD_BLOCK", block)
    executable = tmp_path / "famewin-award-highlight-control"
    built = subprocess.run(
        [compiler, "-std=c++17", "-x", "c++", "-", "-o", str(executable)],
        input=harness,
        text=True,
        capture_output=True,
    )
    assert built.returncode == 0, built.stderr
    result = subprocess.run([str(executable)], capture_output=True)
    assert (result.returncode == 0) is should_pass, result.stderr.decode()


def test_actual_award_highlight_stages_native_arguments_and_rereads(tmp_path):
    run_host_control(tmp_path, source_award_block(), should_pass=True)


def test_award_highlight_mutants_are_observed(tmp_path):
    block = source_award_block()
    old_extent_and_color = block.replace(
        "fill(fill_x, fill_y, fill_extent, 0x15, 0x41);",
        "fill(fill_x, fill_y, 0x41, 0x15, 0);",
    )
    assert old_extent_and_color != block
    run_host_control(tmp_path, old_extent_and_color, should_pass=False)

    hoisted_y = block.replace(
        "const int fill_extent = canvas->text_width(StringTemp) + 0x14;\n"
        "      const int fill_y = static_cast<int>(field_A28_) + 0x0F;",
        "const int fill_y = static_cast<int>(field_A28_) + 0x0F;\n"
        "      const int fill_extent = canvas->text_width(StringTemp) + 0x14;",
    )
    assert hoisted_y != block
    run_host_control(tmp_path, hoisted_y, should_pass=False)

    stale_x = block.replace(
        "const int text_width = canvas->text_width(StringTemp);\n"
        "      const int fill_x =\n"
        "          static_cast<int>(field_A24_) - text_width / 2 + 0x186;",
        "const int staged_A24 = static_cast<int>(field_A24_);\n"
        "      const int text_width = canvas->text_width(StringTemp);\n"
        "      const int fill_x = staged_A24 - text_width / 2 + 0x186;",
    )
    assert stale_x != block
    run_host_control(tmp_path, stale_x, should_pass=False)
