"""Actual-source controls for Texture's two 0x9BB494 byte consumers."""
from __future__ import annotations

from pathlib import Path
import re
import shutil
import subprocess

import pytest

from match_recovery.includes import COMMENTS_OR_STRINGS
from texture_direct_storage_host import direct_storage_prelude


ROOT = next(parent for parent in Path(__file__).resolve().parents
            if (parent / "src/texture.cpp").is_file())
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
// TEXTURE_DIRECT_STORAGE_PRELUDE
static int reset_calls;
static int edge_advance_calls;
static int setup_edge_calls;
static int mutate_scratch494_at_call;
static int mutate_scratch494_at_setup;
static unsigned char scratch494_mutation;
static int forward_calls;
static int mutation_enabled;
static void texture_reset_fpu() { ++reset_calls; }
enum { _PC_24 = 1, _RC_CHOP = 2, _MCW_PC = 4, _MCW_RC = 8 };
static int _control87(int, int) { return 0; }
static int texture_fixed(int value) {
  return static_cast<int>(static_cast<unsigned int>(value) << 16);
}
static int texture_difference(int a, int b) {
  return static_cast<int>(static_cast<unsigned int>(a) -
                          static_cast<unsigned int>(b));
}
static int texture_span_step(int delta, int width) { return delta / width; }
static unsigned short texture_light_step(int, int) { return 0; }
static unsigned int texture_span_pixels(int difference) {
  const unsigned int pixels = static_cast<unsigned int>(difference) & 0xffffu;
  return pixels - 1u < 0x8001u ? pixels : 1u;
}
class Buffer {
public:
  unsigned char storage[128];
  unsigned int stride_;
  RECT rect1_;
  int get_calls, free_calls;
  unsigned char acquired_byte, released_byte;
  Buffer() : stride_(16), get_calls(0), free_calls(0),
             acquired_byte(0), released_byte(0) {
    memset(storage, 0xEE, sizeof(storage));
    rect1_.left = 0; rect1_.top = 0; rect1_.right = 3; rect1_.bottom = 3;
  }
  void *get_data() {
    ++get_calls;
    acquired_byte = TextureRasterScratch494;
    return storage;
  }
  void free_data(int value) {
    assert(value == 1);
    ++free_calls;
    released_byte = TextureRasterScratch494;
  }
};
class Texture {
public:
  unsigned char *pixels_;
  unsigned int iWidth_;
  unsigned int iHeight_;
  Texture(unsigned char *pixels) : pixels_(pixels), iWidth_(4), iHeight_(4) {}
  void draw_color_key(Buffer *, Vert *, int, Vert *);
  void draw_color_key(Buffer *, Vert *, int, Vert *, int);
  void draw2(Buffer *, Vert *, int *, int, Vert *, int, int);
  void draw_below_cutoff(Buffer *, Vert *, int *, int, Vert *, int);
  static int setup_edge(EdgeScan *, int);
};
void Texture::draw_color_key(Buffer *, Vert *, int, Vert *) { ++forward_calls; }
void Texture::draw_below_cutoff(Buffer *, Vert *, int *, int, Vert *, int) {
  ++forward_calls;
}
static int texture_gouraud_edge(EdgeScan &edge, int, const Vert *, const Vert *,
                                const int *, int, int) {
  edge.counter = 3;
  edge.accumA = edge.f0 < 0 ? 0 : 3 << 16;
  edge.accumB = 0;
  edge.accumC = edge.f0 < 0 ? 0 : 3;
  edge.accumD = 5 << 16;
  return 1;
}
static int texture_advance_gouraud_edge(EdgeScan &, const Vert *, const Vert *,
                                        const int *, int, int) {
  ++edge_advance_calls;
  if (mutation_enabled && edge_advance_calls == mutate_scratch494_at_call)
    TextureRasterScratch494 = scratch494_mutation;
  return edge_advance_calls <= 4;
}
/* ACTUAL_BODIES */
'''


def _function(source: str, pattern: str) -> str:
    masked = COMMENTS_OR_STRINGS.sub(
        lambda match: re.sub(r"[^\n]", " ", match[0]), source)
    match = re.search(pattern, masked)
    assert match is not None, pattern
    opening = masked.index("{", match.end() - 1)
    depth = 0
    for end in range(opening, len(masked)):
        if masked[end] == "{":
            depth += 1
        elif masked[end] == "}":
            depth -= 1
            if depth == 0:
                start = masked.rfind("\n", 0, match.start()) + 1
                return source[start:end + 1]
    raise AssertionError("unterminated function: " + pattern)


DRAW_COLOR_KEY = (
    r"void\s+Texture::draw_color_key\s*\(\s*Buffer\s*\*[^,]+,\s*"
    r"Vert\s*\*[^,]+,\s*int\s+[^,]+,\s*Vert\s*\*[^,]+,\s*"
    r"int\s+[^)]+\)\s*\{"
)
DRAW2 = (
    r"void\s+Texture::draw2\s*\(\s*Buffer\s*\*[^,]+,\s*"
    r"Vert\s*\*[^,]+,\s*int\s*\*[^,]+,\s*int\s+[^,]+,\s*"
    r"Vert\s*\*[^,]+,\s*int\s+[^,]+,\s*int\s+[^)]+\)\s*\{"
)


def live_translation_unit() -> str:
    source = SOURCE.read_text(encoding="utf-8")
    edge_step = _function(source, r"static void edge_step\(EdgeScan &e\) \{")
    uv_edge_step = _function(
        source, r"static void texture_uv_edge_step\(EdgeScan &edge\) \{")
    setup_edge = _function(
        source, r"int __cdecl Texture::setup_edge\s*\(EdgeScan\s*\*a1,\s*int\s+a2\)\s*\{")
    draw_color_key = _function(source, DRAW_COLOR_KEY)
    draw_color_key = draw_color_key.replace(
        "Texture::setup_edge(", "observe_setup_edge(")
    observer = r'''
static int observe_setup_edge(EdgeScan *edge, int index) {
  ++setup_edge_calls;
  if (mutation_enabled && setup_edge_calls == mutate_scratch494_at_setup)
    TextureRasterScratch494 = scratch494_mutation;
  return Texture::setup_edge(edge, index);
}
'''
    bodies = (edge_step + "\n\n" + uv_edge_step + "\n\n" + setup_edge + "\n\n" + observer + "\n\n" + draw_color_key
              + "\n\n" + _function(source, DRAW2))
    assert "TextureRasterScratch494" in bodies
    prefix = PREFIX.replace("// TEXTURE_DIRECT_STORAGE_PRELUDE",
                            direct_storage_prelude(source), 1)
    prefix = prefix.replace("static int reset_calls;", "static int fixed_div(int, int);\nstatic int reset_calls;")
    prefix = prefix.replace("static void texture_reset_fpu()", "#define __cdecl\nstatic void texture_reset_fpu()")
    return prefix.replace("/* ACTUAL_BODIES */", bodies, 1) + r'''
static int fixed_div(int numerator, int denominator) {
  return static_cast<int>((static_cast<int64_t>(numerator) * 65536) / denominator);
}
'''


MAIN = r'''
static unsigned char scratch_byte() {
  return TextureRasterScratch494;
}
static void reset_case() {
  TextureRasterScratch494 = 0x11;
  TextureRasterKey = 0xFF;
  reset_calls = edge_advance_calls = setup_edge_calls = forward_calls = 0;
  mutate_scratch494_at_call = 5;
  mutate_scratch494_at_setup = 3;
  scratch494_mutation = 0xB6;
  mutation_enabled = 1;
}
static void setup_shape(Vert *vertices, Vert *uv) {
  vertices[0].x = 0; vertices[0].y = 0;
  vertices[1].x = 3; vertices[1].y = 0;
  vertices[2].x = 3; vertices[2].y = 3;
  vertices[3].x = 0; vertices[3].y = 3;
  uv[0].x = 0; uv[0].y = 0;
  uv[1].x = 3; uv[1].y = 0;
  uv[2].x = 3; uv[2].y = 3;
  uv[3].x = 0; uv[3].y = 3;
}
static void test_color_key_scratch_rows() {
  reset_case();
  unsigned char pixels[64] = {};
  pixels[0] = 0xFF; pixels[1] = 0x12; pixels[2] = 0x13;
  // The extracted real setup_edge advances V on each screen row. Repeat the
  // keyed/sample pattern in all three source rows so these assertions isolate
  // shared fill-color changes rather than sampling unseeded texels.
  pixels[4] = 0xFF; pixels[5] = 0x12; pixels[6] = 0x13;
  pixels[8] = 0xFF; pixels[9] = 0x12; pixels[10] = 0x13;
  Vert vertices[4], uv[4]; setup_shape(vertices, uv);
  Buffer buffer;
  Texture texture(pixels);
  texture.draw_color_key(&buffer, vertices, 4, uv, 0x1A5);
  assert(buffer.get_calls == 1 && buffer.free_calls == 1);
  assert(buffer.acquired_byte == 0x11 && buffer.released_byte == 0xB6);
  assert(scratch_byte() == 0xB6);
  assert(buffer.storage[0] == 0xEE);
  assert(buffer.storage[1] == 0xA5 && buffer.storage[2] == 0xA5);
  assert(buffer.storage[16] == 0xEE);
  assert(buffer.storage[17] == 0x12 && buffer.storage[18] == 0x13);
  assert(buffer.storage[32] == 0xEE);
  assert(buffer.storage[33] == 0xB6 && buffer.storage[34] == 0xB6);
}
static void test_draw2_scratch_rows() {
  reset_case();
  unsigned char pixels[64] = {};
  // The extracted real setup_edge advances V on each screen row. Seed every
  // sampled source row so this case isolates shared fill-color changes.
  pixels[0] = pixels[1] = pixels[2] = pixels[4] = pixels[5] = pixels[6] =
      pixels[8] = pixels[9] = pixels[10] = 20;
  Vert vertices[4], uv[4]; setup_shape(vertices, uv);
  int zvals[4] = {0, 0, 0, 0};
  Buffer buffer;
  Texture texture(pixels);
  texture.draw2(&buffer, vertices, zvals, 4, uv, 10, 0x1A5);
  assert(buffer.get_calls == 1 && buffer.free_calls == 1);
  assert(buffer.acquired_byte == 0x11 && buffer.released_byte == 0xB6);
  assert(scratch_byte() == 0xB6);
  assert(buffer.storage[0] == 0xA5 && buffer.storage[1] == 0xA5);
  assert(buffer.storage[16] == 25);
  assert(buffer.storage[32] == 0xB6 && buffer.storage[33] == 0xB6);
}
static void test_sentinel_paths_do_not_store_or_acquire() {
  reset_case();
  mutation_enabled = 0;
  Buffer buffer;
  unsigned char pixels[64] = {};
  Vert vertices[4], uv[4]; setup_shape(vertices, uv);
  int zvals[4] = {0, 0, 0, 0};
  Texture texture(pixels);
  const int reset_before = reset_calls;
  texture.draw_color_key(&buffer, vertices, 4, uv, -1);
  texture.draw2(&buffer, vertices, zvals, 4, uv, 10, -1);
  assert(forward_calls == 2);
  assert(buffer.get_calls == 0 && buffer.free_calls == 0);
  assert(reset_calls == reset_before);
  assert(TextureRasterScratch494 == 0x11);
}
int main() {
  test_color_key_scratch_rows();
  test_draw2_scratch_rows();
  test_sentinel_paths_do_not_store_or_acquire();
  return 0;
}
'''


@pytest.mark.parametrize("compiler_name", ["g++", "clang++"])
def test_actual_scratch_consumers(compiler_name: str, tmp_path: Path) -> None:
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(f"{compiler_name} is unavailable")
    source = live_translation_unit() + MAIN
    cpp = tmp_path / (compiler_name + ".cpp")
    exe = tmp_path / compiler_name
    cpp.write_text(source, encoding="utf-8")
    built = subprocess.run(
        [compiler, "-std=c++98", "-Wall", "-Wextra", "-Werror",
         "-Wno-unused-function", "-Wno-unused-variable", "-no-pie",
         str(cpp), "-o", str(exe)], cwd=ROOT, capture_output=True,
        text=True, timeout=60)
    assert built.returncode == 0, built.stdout + built.stderr
    run = subprocess.run([str(exe)], cwd=ROOT, capture_output=True,
                         text=True, timeout=30)
    assert run.returncode == 0, run.stdout + run.stderr


@pytest.mark.parametrize(("old", "new"), [
    ("alternate =\n                        TextureRasterScratch494;",
     "alternate = static_cast<unsigned char>(color);"),
    ("const unsigned char fill =\n                            TextureRasterScratch494;",
     "const unsigned char fill = static_cast<unsigned char>(fillColor);"),
])
def test_argument_snapshot_mutants_fail(old: str, new: str,
                                        tmp_path: Path) -> None:
    compiler = shutil.which("g++")
    if compiler is None:
        pytest.skip("GCC host control unavailable")
    source = live_translation_unit()
    assert source.count(old) == 1
    source = source.replace(old, new, 1) + MAIN
    cpp = tmp_path / "mutant.cpp"
    exe = tmp_path / "mutant"
    cpp.write_text(source, encoding="utf-8")
    built = subprocess.run(
        [compiler, "-std=c++98", "-Wall", "-Wextra", "-no-pie",
         str(cpp), "-o", str(exe)], cwd=ROOT, capture_output=True,
        text=True, timeout=60)
    assert built.returncode == 0, built.stdout + built.stderr
    run = subprocess.run([str(exe)], cwd=ROOT, capture_output=True,
                         text=True, timeout=30)
    assert run.returncode != 0, "argument-snapshot mutant escaped controls"
