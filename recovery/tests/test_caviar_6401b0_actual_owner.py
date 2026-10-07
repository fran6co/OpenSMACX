"""Current-source controls for the complete 0x6401B0 helper and callers."""
from pathlib import Path
import shutil
import subprocess
import tempfile

from match_recovery.includes import COMMENTS_OR_STRINGS

ROOT = next(parent for parent in Path(__file__).resolve().parents
            if (parent / "src/caviar.cpp").is_file())
SOURCE = ROOT / "src/caviar.cpp"
KERNELS = ROOT / "src/caviar_raster_kernels.inc"


def extract(text: str, signature: str) -> str:
    masked = COMMENTS_OR_STRINGS.sub(
        lambda match: "".join("\n" if ch == "\n" else " " for ch in match[0]),
        text)
    start = masked.find(signature)
    while start >= 0:
        opening = masked.find("{", start)
        semicolon = masked.find(";", start)
        if opening >= 0 and (semicolon < 0 or opening < semicolon):
            break
        start = masked.find(signature, start + len(signature))
    assert start >= 0 and opening >= 0, "missing actual definition: " + signature
    depth = 0
    for index in range(opening, len(masked)):
        if masked[index] == "{":
            depth += 1
        elif masked[index] == "}":
            depth -= 1
            if depth == 0:
                return text[start:index + 1]
    raise AssertionError("unclosed actual body: " + signature)


HARNESS = r'''#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#define __cdecl
struct CaviarFixedPointProjectionMatrix {
  int16_t word_00_, word_02_, word_04_, word_06_, word_08_, word_0A_;
  int32_t dword_0C_, dword_10_, dword_14_;
};
CaviarFixedPointProjectionMatrix CaviarFixedPointProjection = {};
'''

MAIN = r'''
static void check(bool okay, const char *message) {
  if (!okay) { fprintf(stderr, "FAIL %s\n", message); exit(1); }
}
// Independent oracle: a 64-bit dot product, explicit modulo reduction and
// floor division, then enumerate corners instead of repeating the live walk.
static int reduce_reference(int64_t value, bool low_word) {
  uint32_t biased = (uint32_t)(value + 128);
  int64_t signed_value = biased;
  if (biased >= 0x80000000u) signed_value -= 0x100000000LL;
  int reduced = (int)(signed_value / 256);
  if (signed_value < 0 && signed_value % 256) --reduced;
  if (low_word) {
    reduced = (int)((uint32_t)reduced % 65536u);
    if (reduced >= 32768) reduced -= 65536;
  }
  return reduced;
}
static void reference_box(const short matrix[6], const short centre[3],
                          const short extent[3], int result[6]) {
  result[0] = result[1] = 32767;
  result[2] = result[3] = -32767;
  for (int axis = 0; axis < 2; ++axis) {
    const short *row = matrix + axis * 3;
    const int origin = reduce_reference((int64_t)row[0] * centre[0]
                     + (int64_t)row[1] * centre[1]
                     + (int64_t)row[2] * centre[2], false);
    int edges[3];
    for (int j = 0; j < 3; ++j)
      edges[j] = reduce_reference((int64_t)row[j] * extent[j], true);
    for (int corner = 0; corner < 8; ++corner) {
      int position = origin;
      for (int j = 0; j < 3; ++j)
        if (corner & (1 << j)) position += edges[j];
      if (position < result[axis]) result[axis] = position;
      if (position > result[axis + 2]) result[axis + 2] = position;
    }
    result[axis] -= 2;
    result[axis + 2] += 2;
  }
  result[4] = result[5] = 0;
  if (result[2] >= 0 && result[3] >= 0) {
    result[4] = result[2] - result[0] + 1;
    result[5] = result[3] - result[1] + 1;
  }
}
static short next_short(uint32_t &state) {
  state = state * 1664525u + 1013904223u;
  const int low = (int)(state >> 16);
  return (short)(low < 32768 ? low : low - 65536);
}
int main() {
  CaviarFixedPointProjection.word_00_ = 256;
  CaviarFixedPointProjection.word_02_ = -256;
  CaviarFixedPointProjection.word_04_ = 128;
  CaviarFixedPointProjection.word_06_ = -128;
  CaviarFixedPointProjection.word_08_ = 256;
  CaviarFixedPointProjection.word_0A_ = 256;
  short centre[3] = {300, 200, 150};
  short extent[3] = {10, -20, 30};
  int rect[6] = {};
  sub_6401b0(centre, extent, rect);
  check(rect[0] == 173 && rect[1] == 173 && rect[2] == 222 &&
        rect[3] == 232 && rect[4] == 50 && rect[5] == 60,
        "varied matrix, negative extent, eight-corner min/max and inset");

  CaviarFixedPointProjection.word_00_ = 256;
  CaviarFixedPointProjection.word_02_ = 0;
  CaviarFixedPointProjection.word_04_ = 0;
  CaviarFixedPointProjection.word_06_ = 0;
  CaviarFixedPointProjection.word_08_ = 256;
  CaviarFixedPointProjection.word_0A_ = 0;
  short zero[3] = {0, 0, 0};
  short zero_extent[3] = {0, 0, 0};
  sub_6401b0(zero, zero_extent, rect);
  check(rect[0] == -2 && rect[1] == -2 && rect[2] == 2 &&
        rect[3] == 2 && rect[4] == 5 && rect[5] == 5,
        "zero extent retains initialized corner and padded dimensions");
  uint32_t state = 0x6401b0u;
  for (int trial = 0; trial < 4096; ++trial) {
    short matrix[6], input[3], sizes[3];
    for (int j = 0; j < 6; ++j) matrix[j] = next_short(state);
    for (int j = 0; j < 3; ++j) {
      input[j] = next_short(state);
      sizes[j] = next_short(state);
    }
    if (trial < 8) {
      for (int j = 0; j < 6; ++j) matrix[j] = -32768;
      for (int j = 0; j < 3; ++j) {
        input[j] = trial & (1 << j) ? 32767 : -32768;
        sizes[j] = input[j];
      }
    }
    CaviarFixedPointProjection.word_00_ = matrix[0];
    CaviarFixedPointProjection.word_02_ = matrix[1];
    CaviarFixedPointProjection.word_04_ = matrix[2];
    CaviarFixedPointProjection.word_06_ = matrix[3];
    CaviarFixedPointProjection.word_08_ = matrix[4];
    CaviarFixedPointProjection.word_0A_ = matrix[5];
    int expected[6];
    reference_box(matrix, input, sizes, expected);
    sub_6401b0(input, sizes, rect);
    for (int j = 0; j < 6; ++j)
      check(rect[j] == expected[j], "full-width signed inputs versus corner oracle");
  }
  return 0;
}
'''


def definitions(source: str) -> list[str]:
    return [extract(source, signature) for signature in (
        "static int32_t caviar_i32_from_bits(",
        "static uint32_t caviar_sar8_bits(",
        "static int32_t caviar_fixed8_low16(",
        "static int32_t caviar_add_i32(",
        "static int32_t caviar_sub_i32(",
        "int32_t fixed8(",
        "void __cdecl sub_6401b0(",
    )]


def compile_run(compiler: str, bodies: list[str], expected: bool) -> None:
    with tempfile.TemporaryDirectory(prefix="caviar-6401b0-owner-") as folder:
        source = Path(folder) / "actual.cpp"
        binary = Path(folder) / "actual"
        source.write_text(HARNESS + "\n".join(bodies) + MAIN,
                          encoding="utf-8")
        built = subprocess.run(
            [compiler, "-std=c++98", "-Wall", "-Wextra", "-Werror",
             str(source), "-o", str(binary)], capture_output=True, text=True)
        assert built.returncode == 0, built.stdout + built.stderr
        run = subprocess.run([str(binary)], capture_output=True, text=True)
        assert (run.returncode == 0) == expected, run.stdout + run.stderr


def test_installed_body_gcc_clang() -> None:
    bodies = definitions(SOURCE.read_text(encoding="utf-8"))
    compilers = [shutil.which(name) for name in ("g++", "clang++")]
    assert all(compilers), "GCC and Clang required"
    for compiler in compilers:
        compile_run(compiler, bodies, True)


def test_actual_caller_closure_and_argument_contracts() -> None:
    source = SOURCE.read_text(encoding="utf-8")
    kernels = KERNELS.read_text(encoding="utf-8")
    projection = extract(source, "void __cdecl sub_6401b0(")
    assert "const short *centre" in projection
    assert "const short *extent" in projection
    assert "int *out_rect" in projection

    flat = extract(kernels, "static int caviar_flat_raster_box(")
    item = extract(kernels, "static int caviar_raster_item_box(")
    part = extract(source, "void __cdecl vox_draw_part(")
    shadow = extract(source, "void __cdecl vox_draw_shadow(")
    clip = extract(source, "void __cdecl caviar_clip_shape(")
    assert "sub_6401b0(&shape->offset_x_," in flat
    assert "&shape->axis_x_, box" in flat
    assert "sub_6401b0(delta, item->extent_, box)" in item
    assert "caviar_flat_raster_box(" in part and "caviar_raster_item_box(" in part
    assert "caviar_flat_raster_box(" in shadow and "caviar_raster_item_box(" in shadow
    assert clip.count("sub_6401b0(") == 2

    header = (ROOT / "src/caviar.h").read_text(encoding="utf-8")
    assert "struct CvrShape" in header
    assert "int16_t axis_x_" in header and "int16_t offset_x_" in header


def test_current_body_mutants_fail_oracle() -> None:
    source = SOURCE.read_text(encoding="utf-8")
    bodies = definitions(source)
    method_index = len(bodies) - 1
    mutations = (
        ("x_walk = caviar_sub_i32(x_walk, extent0_x);",
         "x_walk = caviar_add_i32(x_walk, extent0_x);"),
        ("word_00_ * (int32_t)centre[0]",
         "word_02_ * (int32_t)centre[0]"),
        ("out_rect[4] = (right - (x_walk - 2)) + 1;",
         "out_rect[4] = (right - (x_walk - 2)) + 2;"),
    )
    compilers = [shutil.which(name) for name in ("g++", "clang++")]
    assert all(compilers), "GCC and Clang required"
    for old, new in mutations:
        mutant = list(bodies)
        assert old in mutant[method_index]
        mutant[method_index] = mutant[method_index].replace(old, new, 1)
        for compiler in compilers:
            try:
                compile_run(compiler, mutant, False)
            except AssertionError as error:
                raise AssertionError(old) from error
