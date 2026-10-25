"""Host controls for the actual Caviar::clear_buffers source body.

Callbacks are bounded doubles. This does not assert the native pointer-cell
layout or runtime/allocator behavior.
"""

from pathlib import Path
import shutil
import subprocess

import pytest


ROOT = Path(__file__).resolve().parents[2]

from test_caviar_descriptor_ramps_overlay_oct9 import extract


def actual_global_and_body(source=None):
    if source is None:
        source = (ROOT / "src/caviar.cpp").read_text()
    lines = [line for line in source.splitlines()
             if line.strip().startswith("uint16_t *CaviarSceneMemory;")]
    assert len(lines) == 1
    declaration = lines[0].split("//", 1)[0].strip()
    assert declaration == "uint16_t *CaviarSceneMemory;"
    body = extract(source, "void Caviar::clear_buffers(int fill)")
    return declaration, body


FIXTURE_PREFIX = r'''#include <stdint.h>
#include <stddef.h>
#include <string.h>
struct RECT { int32_t left, top, right, bottom; };
struct Viewport { int left, top, right, bottom; };
static Viewport CaviarViewportState;
struct Scene { RECT scene_rect_; };
static Scene CaviarScene;
struct Buffer {
  void fill(int, int, int, int, int);
  void fill(int);
};
struct BufferOwner { Buffer value; Buffer &buffer() { return value; } };
static BufferOwner CaviarSceneBufferA, CaviarSceneBufferB;
struct Caviar { void clear_buffers(int); };
'''

FIXTURE_SUFFIX = r'''
static uint16_t memory_a[0x100 * 8];
static uint16_t memory_b[0x100 * 8];
static char events[16];
static int event_count;
static int mutate_after_capture;
static int a_args[5];
static int b_arg;
static uint16_t *table_rows[8];
static int table_values[8];
static int table_widths[8];
static int table_calls;

static void event(char value) { events[event_count++] = value; }
void Buffer::fill(int x, int y, int width, int height, int value) {
  event('A');
  a_args[0] = x; a_args[1] = y; a_args[2] = width;
  a_args[3] = height; a_args[4] = value;
  if (mutate_after_capture) {
    CaviarSceneMemory = memory_b;
    CaviarViewportState.left = 19;
    CaviarViewportState.top = 17;
    CaviarViewportState.right = 29;
    CaviarViewportState.bottom = 31;
  }
}
void Buffer::fill(int value) { event('B'); b_arg = value; }
static void vox_fill_colour_table(uint16_t *row, int value, int width) {
  event('T');
  table_rows[table_calls] = row;
  table_values[table_calls] = value;
  table_widths[table_calls] = width;
  ++table_calls;
}

static void reset_observers() {
  event_count = 0;
  table_calls = 0;
  mutate_after_capture = 0;
  memset(events, 0, sizeof(events));
}
'''

MAIN = r'''
int main() {
  Caviar renderer;

  // Captures a row pointer and rectangle before Buffer callbacks mutate both
  // globals. Two table rows retain the original 0x100-word stride.
  reset_observers();
  CaviarSceneMemory = memory_a;
  CaviarViewportState.left = 2; CaviarViewportState.top = 1;
  CaviarViewportState.right = 5; CaviarViewportState.bottom = 3;
  CaviarScene.scene_rect_ = {7, 8, 9, 10};
  mutate_after_capture = 1;
  renderer.clear_buffers(0);
  if (event_count != 4 || events[0] != 'A' || events[1] != 'B' ||
      events[2] != 'T' || events[3] != 'T') return 21;
  if (a_args[0] != 2 || a_args[1] != 1 || a_args[2] != 3 ||
      a_args[3] != 2 || a_args[4] != 9 || b_arg != 0) return 22;
  if (table_calls != 2 || table_rows[0] != &memory_a[0x100 + 2] ||
      table_rows[1] != &memory_a[0x200 + 2] ||
      table_values[0] != 0xFFFF || table_values[1] != 0xFFFF ||
      table_widths[0] != 3 || table_widths[1] != 3) return 23;
  if (CaviarSceneMemory != memory_b || CaviarViewportState.left != 0 ||
      CaviarViewportState.top != 0 || CaviarViewportState.right != 0 ||
      CaviarViewportState.bottom != 0) return 24;
  if (CaviarScene.scene_rect_.left != 0 || CaviarScene.scene_rect_.top != 0 ||
      CaviarScene.scene_rect_.right != 0 || CaviarScene.scene_rect_.bottom != 0)
    return 25;

  // Nonzero fill still fills A then B but skips the colour table and reset.
  reset_observers();
  CaviarSceneMemory = memory_a;
  CaviarViewportState.left = 1; CaviarViewportState.top = 2;
  CaviarViewportState.right = 4; CaviarViewportState.bottom = 4;
  CaviarScene.scene_rect_ = {11, 12, 13, 14};
  renderer.clear_buffers(7);
  if (event_count != 2 || events[0] != 'A' || events[1] != 'B' ||
      table_calls != 0 || a_args[0] != 1 || a_args[1] != 2 ||
      a_args[2] != 3 || a_args[3] != 2 || a_args[4] != 9 || b_arg != 0)
    return 26;
  if (CaviarViewportState.left != 1 || CaviarViewportState.top != 2 ||
      CaviarViewportState.right != 4 || CaviarViewportState.bottom != 4 ||
      CaviarScene.scene_rect_.left != 11 || CaviarScene.scene_rect_.top != 12 ||
      CaviarScene.scene_rect_.right != 13 || CaviarScene.scene_rect_.bottom != 14)
    return 27;

  // A missing shared table gates both surface fills and every reset.
  reset_observers();
  CaviarSceneMemory = 0;
  CaviarViewportState.left = 3; CaviarViewportState.top = 2;
  CaviarViewportState.right = 6; CaviarViewportState.bottom = 5;
  CaviarScene.scene_rect_ = {1, 2, 3, 4};
  renderer.clear_buffers(0);
  if (event_count != 0 || table_calls != 0 ||
      CaviarViewportState.left != 3 || CaviarViewportState.top != 2 ||
      CaviarViewportState.right != 6 || CaviarViewportState.bottom != 5 ||
      CaviarScene.scene_rect_.left != 1 || CaviarScene.scene_rect_.top != 2 ||
      CaviarScene.scene_rect_.right != 3 || CaviarScene.scene_rect_.bottom != 4)
    return 28;

  // Zero and negative row spans forward the captured rectangle to A/B,
  // skip the table loop, and still reset viewport and scene rectangles.
  for (int delta = 0; delta >= -1; --delta) {
    reset_observers();
    CaviarSceneMemory = memory_a;
    CaviarViewportState.left = 1; CaviarViewportState.top = 2;
    CaviarViewportState.right = 4; CaviarViewportState.bottom = 2 + delta;
    CaviarScene.scene_rect_ = {4, 3, 2, 1};
    renderer.clear_buffers(0);
    if (event_count != 2 || events[0] != 'A' || events[1] != 'B' ||
        table_calls != 0 || a_args[0] != 1 || a_args[1] != 2 ||
        a_args[2] != 3 || a_args[3] != delta || a_args[4] != 9)
      return 29;
    if (CaviarViewportState.left != 0 || CaviarViewportState.top != 0 ||
        CaviarViewportState.right != 0 || CaviarViewportState.bottom != 0 ||
        CaviarScene.scene_rect_.left != 0 || CaviarScene.scene_rect_.top != 0 ||
        CaviarScene.scene_rect_.right != 0 || CaviarScene.scene_rect_.bottom != 0)
      return 30;
  }
  return 0;
}
'''


def make_program(source=None):
    declaration, body = actual_global_and_body(source)
    return FIXTURE_PREFIX + declaration + "\n" + FIXTURE_SUFFIX + "\n" + body + "\n" + MAIN


def compile_run(tmp_path, compiler, source, stem):
    cpp = tmp_path / (stem + ".cpp")
    exe = tmp_path / stem
    cpp.write_text(make_program(source))
    built = subprocess.run(
        [compiler, "-std=c++11", "-Wall", "-Wextra", "-Werror", str(cpp), "-o", str(exe)],
        capture_output=True, text=True, timeout=30,
    )
    assert built.returncode == 0, built.stdout + built.stderr
    return subprocess.run([str(exe)], capture_output=True, text=True, timeout=10)


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
def test_actual_clear_buffers_order_stride_live_pointer_and_resets(tmp_path, compiler_name):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(f"{compiler_name} is unavailable")
    result = compile_run(tmp_path, compiler, None, "clear_buffers_actual")
    assert result.returncode == 0, result.stdout + result.stderr


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
def test_wrong_row_stride_mutant_is_rejected(tmp_path, compiler_name):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(f"{compiler_name} is unavailable")
    source = (ROOT / "src/caviar.cpp").read_text()
    anchor = "row += 0x100;"
    assert source.count(anchor) == 1
    damaged = source.replace(anchor, "row += col_span;", 1)
    result = compile_run(tmp_path, compiler, damaged, "clear_buffers_stride_mutant")
    assert result.returncode == 23, result.stdout + result.stderr


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
def test_reloaded_global_pointer_mutant_is_rejected(tmp_path, compiler_name):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(f"{compiler_name} is unavailable")
    source = (ROOT / "src/caviar.cpp").read_text()
    anchor = "row += 0x100;"
    assert source.count(anchor) == 1
    damaged = source.replace(
        anchor,
        "row = CaviarSceneMemory + (height + 1) * 0x100 + width;",
        1,
    )
    result = compile_run(tmp_path, compiler, damaged, "clear_buffers_global_pointer_mutant")
    assert result.returncode == 23, result.stdout + result.stderr
