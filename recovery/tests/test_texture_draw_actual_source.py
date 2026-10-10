"""Compile and exercise the live Texture draw source and its DDA helpers."""
from __future__ import annotations

import re
import shutil
import subprocess
import tempfile
from pathlib import Path

import pytest


ROOT = Path(__file__).resolve().parents[2]
SOURCE = ROOT / "src/texture.cpp"


PREFIX = r'''#include <assert.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
struct RECT { int left, top, right, bottom; };
struct Vert { int x, y; };
struct EdgeScan {
  int f0, counter, id, accumA, accumB, deltaA, deltaB, accumC, deltaC;
  int extraDeltaC, fracAccum, fracDelta, fracSub, accumD, deltaD;
};
class Buffer {
public:
  void *get_data() { return storage; }
  void free_data(int) { ++free_calls; }
  unsigned char storage[128] = {};
  unsigned int stride_ = 16;
  RECT rect1_ = {0, 0, 16, 8};
  int free_calls = 0;
};
static int forward_calls = 0;
class Texture {
public:
  void *pixels_ = 0;
  uint32_t iWidth_ = 8;
  void draw(Buffer *, Vert *, int *, int, Vert *) { ++forward_calls; }
  void draw(Buffer *, Vert *, int *, int, Vert *, int);
};
enum { _PC_24 = 1, _RC_CHOP = 2, _MCW_PC = 4, _MCW_RC = 8 };
int _control87(int, int) { return 0; }
const int TextureLightCeiling = 255;
const int TextureLightFloor = 0;
static void *slot_pixel_base, *slot_pixels;
static int slot_threshold, slot_threshold_2, slot_494;
static int slot_count, slot_stride;
static int slot_width, slot_clip_right, slot_clip_left, slot_clip_bottom;
static int slot_clip_top, slot_max_y, slot_row, slot_4e4, slot_4c8;
static int slot_498, slot_4f8, slot_53c, slot_4c4, slot_530;
static int slot_span_right, slot_span_left, slot_dither;
static void **blit_pixel_base = &slot_pixel_base;
static int *blit_light_threshold = &slot_threshold;
static int *blit_light_threshold_2 = &slot_threshold_2;
static int *blit_slot_494 = &slot_494;
static Vert *edge_uvb_cell = 0;
static Vert **blit_edge_uvs_b = &edge_uvb_cell;
static Vert *edge_uva_cell = 0;
static Vert **blit_edge_uvs_a = &edge_uva_cell;
static int *blit_vertex_count = &slot_count;
static int *z_cell = 0;
static int **blit_z_values = &z_cell;
static void **blit_texture_pixels = &slot_pixels;
static int *blit_buffer_stride = &slot_stride;
static int *blit_texture_width = &slot_width;
static int *blit_clip_right = &slot_clip_right;
static int *blit_clip_left = &slot_clip_left;
static int *blit_clip_bottom = &slot_clip_bottom;
static int *blit_clip_top = &slot_clip_top;
static int *blit_max_y_index = &slot_max_y;
static int *blit_row_offset = &slot_row;
static int *blit_slot_4e4 = &slot_4e4;
static int *blit_slot_4c8 = &slot_4c8;
static unsigned short light_pack_words[2] = {};
static unsigned short *blit_light_pack_a = &light_pack_words[0];
static int *blit_slot_498 = &slot_498;
static int *blit_slot_4f8 = &slot_4f8;
static int *blit_slot_53c = &slot_53c;
static int *blit_slot_4c4 = &slot_4c4;
static int *blit_slot_530 = &slot_530;
static int *blit_span_right = &slot_span_right;
static int *blit_span_left = &slot_span_left;
static int *blit_dither = &slot_dither;
static int reset_calls = 0;
static void texture_reset_fpu() { ++reset_calls; }
static uintptr_t texture_pointer_address(const void *pointer) {
  return reinterpret_cast<uintptr_t>(pointer);
}
static int texture_fixed(int value) {
  return static_cast<int>(static_cast<unsigned int>(value) << 16);
}
static int texture_difference(int a, int b) {
  return static_cast<int>(static_cast<unsigned int>(a) - static_cast<unsigned int>(b));
}
static int texture_span_step(int, int) { return 0; }
static unsigned int texture_span_pixels(int value) {
  const unsigned int pixels = static_cast<unsigned int>(value) & 0xffffu;
  return pixels - 1u < 0x8001u ? pixels : 1u;
}
static int setup_calls = 0;
static int advance_calls = 0;
static int texture_gouraud_edge(EdgeScan &edge, int, const Vert *, const Vert *,
                                const int *, int, int) {
  ++setup_calls;
  edge.counter = 2;
  edge.accumA = edge.f0 < 0 ? 0 : 0x00030000;
  edge.accumB = 0;
  edge.accumC = edge.f0 < 0 ? 1 : 4;
  edge.accumD = 0x0012f000;
  return 1;
}
static int texture_advance_gouraud_edge(EdgeScan &, const Vert *, const Vert *,
                                        const int *, int, int) {
  return advance_calls++ < 2;
}
static unsigned short texture_light_step(int, int) { return 0; }
'''


MAIN = r'''
int main() {
  unsigned char pixels[64] = {};
  pixels[0] = 7; pixels[10] = 250; pixels[15] = 2;
  unsigned char coast[64] = {};
  unsigned char remap[256] = {};
  coast[0] = 10; coast[10] = 11; coast[15] = 12;
  remap[10] = 0; remap[11] = 1; remap[12] = 0xe7;
  unsigned char out[3] = {};
  texture_draw_lit_span(out, pixels, 4, 0x0000f000, 0x0000e000,
                        0x00013000, 0x00015000, 0x0012f000, 0x0100, 3);
  assert(out[0] == 25 && out[1] == 0x0d && out[2] == 0x16);
  out[0] = 10; out[1] = 20; out[2] = 30;
  texture_draw_keyed_lit_span(out, pixels, 4, 0x0000f000, 0x0000e000,
                              0x00013000, 0x00015000, 0x0012f000,
                              0x0100, 3, 250);
  assert(out[0] == 25 && out[1] == 20 && out[2] == 0x16);
  out[0] = 40; out[1] = 50; out[2] = 60;
  texture_draw_coast_span(out, pixels, coast, remap, 4,
                          0x0000f000, 0x0000e000, 0x00013000,
                          0x00015000, 0x0012f000, 0x0100, 3);
  assert(out[0] == 40 && out[1] == 0x0d && out[2] == 0xe7);
  texture_draw_keyed_coast_fill_span(out, coast, remap, 4,
                                     0x0000f000, 0x0000e000,
                                     0x00013000, 0x00015000, 0x99, 3);
  assert(out[0] == 40 && out[1] == 0x99 && out[2] == 0x99);

  static Buffer buffer;
  Texture texture;
  texture.pixels_ = pixels;
  Vert vertices[4] = {{1, 0}, {4, 0}, {4, 2}, {1, 2}};
  Vert uv[4] = {{0, 0}, {3, 0}, {3, 2}, {0, 2}};
  int lights[4] = {18, 18, 18, 18};
  buffer.rect1_.bottom = 2;
  texture.draw(&buffer, vertices, lights, 4, uv, 0xaa);
  assert(buffer.storage[1] == 0xaa);
  assert(buffer.storage[buffer.stride_ + 1] == pixels[0] + 0x12);
  assert(advance_calls == 3); // second row terminates after its snapshot
  assert(reset_calls == 2);

  advance_calls = setup_calls = 0;
  buffer.rect1_.bottom = 1;
  texture.draw(&buffer, vertices, lights, 4, uv, 0xcc);
  assert(advance_calls == 2); // no edge refresh at the bottom offset
  assert(buffer.storage[1] == 0xcc);
  assert(reset_calls == 4);

  advance_calls = setup_calls = 0;
  buffer.rect1_.bottom = 0;
  texture.draw(&buffer, vertices, lights, 4, uv, 0xdd);
  assert(setup_calls == 2 && advance_calls == 0);
  assert(buffer.storage[1] == 0xcc);
  assert(reset_calls == 6);

  advance_calls = setup_calls = 0;
  buffer.stride_ = 0;
  buffer.rect1_.bottom = 8;
  texture.draw(&buffer, vertices, lights, 4, uv, 0xee);
  assert(setup_calls == 2 && advance_calls == 0); // row and bottom offsets equal
  assert(buffer.storage[1] == 0xcc);
  assert(reset_calls == 8);

  texture.draw(&buffer, vertices, lights, 4, uv, -1);
  assert(forward_calls == 1 && reset_calls == 8);
  return 0;
}
'''


def _function(source: str, pattern: str) -> str:
    match = re.search(pattern, source)
    if match is None:
        raise AssertionError(f"live source function not found: {pattern}")
    start = match.start()
    opening = source.index("{", match.end() - 1)
    depth = 0
    quote = ""
    escaped = False
    line_comment = False
    block_comment = False
    i = opening
    while i < len(source):
        ch = source[i]
        nxt = source[i + 1] if i + 1 < len(source) else ""
        if line_comment:
            if ch == "\n":
                line_comment = False
        elif block_comment:
            if ch == "*" and nxt == "/":
                block_comment = False
                i += 1
        elif quote:
            if escaped:
                escaped = False
            elif ch == "\\":
                escaped = True
            elif ch == quote:
                quote = ""
        elif ch == "/" and nxt == "/":
            line_comment = True
            i += 1
        elif ch == "/" and nxt == "*":
            block_comment = True
            i += 1
        elif ch in "\"'":
            quote = ch
        elif ch == "{":
            depth += 1
        elif ch == "}":
            depth -= 1
            if depth == 0:
                return source[start:i + 1]
        i += 1
    raise AssertionError("unterminated live source function")


@pytest.mark.parametrize("compiler_name", ["g++", "clang++"])
def test_live_texture_draw_source_and_dda(compiler_name: str) -> None:
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(f"{compiler_name} is unavailable")
    source = SOURCE.read_text(encoding="utf-8")
    helper_start = source.index("static unsigned int texture_draw_source_step(")
    helper_end = source.index("static void texture_reset_fpu()", helper_start)
    helpers = source[helper_start:helper_end]
    owner = _function(
        source,
        r"void\s+Texture::draw\(Buffer\s*\*[^,]+,\s*Vert\s*\*[^,]+,\s*"
        r"int\s*\*[^,]+,\s*int\s+[^,]+,\s*Vert\s*\*[^,]+,\s*int\s+[^)]+\)\s*\{",
    )
    assert "while (*blit_row_offset < *blit_clip_bottom)" in owner
    assert "*blit_clip_top <= *blit_row_offset" in owner
    assert "while (y < clip.bottom)" not in owner
    fill = owner.index("static_cast<unsigned char>(fill_color)")
    entry_reset = owner.index("texture_reset_fpu();")
    args = owner.index("*blit_edge_uvs_b")
    assert fill < entry_reset < args

    with tempfile.TemporaryDirectory(prefix="texture-draw-live-source-") as temp:
        cpp = Path(temp) / "live.cpp"
        binary = Path(temp) / "live"
        cpp.write_text(PREFIX + "\n" + helpers + "\n" + owner + "\n" + MAIN,
                       encoding="utf-8")
        built = subprocess.run(
            [compiler, "-std=c++11", "-Wall", "-Wextra", "-Werror", "-no-pie",
             str(cpp), "-o", str(binary)],
            text=True, capture_output=True, timeout=30,
        )
        assert built.returncode == 0, built.stdout + built.stderr
        ran = subprocess.run([str(binary)], text=True, capture_output=True,
                             timeout=10)
        assert ran.returncode == 0, ran.stdout + ran.stderr
