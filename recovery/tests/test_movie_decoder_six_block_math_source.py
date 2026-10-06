from __future__ import annotations

import shutil
import subprocess
from pathlib import Path


ROOT = Path(__file__).resolve().parent
SRC = ROOT.parents[1] / "src"


def _function(source: str, signature: str) -> str:
    start = source.index(signature)
    opening = source.index("{", start)
    depth = 0
    for index in range(opening, len(source)):
        if source[index] == "{":
            depth += 1
        elif source[index] == "}":
            depth -= 1
            if depth == 0:
                return source[start:index + 1]
    raise AssertionError(f"unterminated function {signature}")


def test_actual_six_step_predictor_and_dct_bodies(tmp_path: Path) -> None:
    compiler = shutil.which("g++") or shutil.which("clang++")
    if compiler is None:
        return
    source_text = (SRC / "movie_media_decoder.cpp").read_text()
    owner_header = (SRC / "movie_decoder_owner.h").read_text()
    byte_view = _function(owner_header, "struct MediaCoefficientPredictorViewLayout") + ";\n"
    byte_accessors = "\n".join(
        _function(owner_header, signature)
        for signature in ("static const BYTE *movie_media_luma_delta_entries(",
                          "static const BYTE *movie_media_chroma_delta_entries("))
    signatures = (
        "static DWORD movie_media_read_bits32",
        "static DWORD movie_media_decode_predictor_delta",
        "static DWORD movie_media_coefficient_entry",
        "static int movie_media_sar17",
        "void __cdecl sub_5d0d82(",
        "void __cdecl sub_5d0aab(",
        "void __cdecl sub_5d1466(",
    )
    bodies = "\n\n".join(_function(source_text, signature)
                           for signature in signatures)
    unit_source = r'''#include <cassert>
#include <cstdint>
#include <cstring>
#include <cstddef>
#include <climits>
#include <cstdio>
#define __cdecl
typedef std::uint8_t BYTE;
typedef std::uint32_t DWORD;
struct MediaDecoderContext {
  DWORD block_transform_weights[64];
  DWORD predictor_4f8;
  DWORD predictor_4fc;
  DWORD predictor_500;
  BYTE unresolved_original_storage_10c_1ff[0xf4];
  const BYTE *compressed_input_200;
  DWORD bit_position_204;
};
struct MediaCoefficientTableStorage { DWORD words[325]; };
MediaCoefficientTableStorage movie_media_coefficient_tables = {};
void __cdecl sub_5d0d82(MediaDecoderContext *, DWORD, BYTE *, int);
static int row_calls;
static int column_calls;
static int converter_calls;
static int first_row_value;
extern "C" void __cdecl sub_5d29e1(int *source, int *destination) {
  if (row_calls == 0) first_row_value = source[0];
  ++row_calls;
  for (int i = 0; i != 8; ++i) destination[i * 9] = source[0] + i;
}
extern "C" void __cdecl sub_5d2c0a(int *destination, int *source) {
  ++column_calls;
  for (int i = 0; i != 8; ++i) destination[i] = source[0] + i;
}
void __cdecl sub_5d2037(int *, int *, unsigned int *) { ++converter_calls; }
''' + byte_view + byte_accessors + "\n" + bodies + r'''
static void set_msb(BYTE *buffer, unsigned position, unsigned value,
                    unsigned width) {
  for (unsigned bit = 0; bit != width; ++bit) {
    const unsigned set = (value >> (width - bit - 1)) & 1;
    const unsigned absolute = position + bit;
    const unsigned byte_index = (absolute & ~31u) / 8 +
                                3 - ((absolute & 31u) >> 3);
    if (set) buffer[byte_index] |=
        (BYTE)(1u << (7 - (absolute & 7)));
  }
}

int main() {
  BYTE reader_input[24];
  for (int i = 0; i != 24; ++i)
    reader_input[i] = (BYTE)(i * 37 + 11);
  for (unsigned start = 0; start != 128; ++start) {
    DWORD expected = 0;
    for (unsigned bit = 0; bit != 32; ++bit) {
      const unsigned absolute = start + bit;
      DWORD word;
      std::memcpy(&word, reader_input + (absolute >> 5) * 4,
                  sizeof(word));
      expected = (expected << 1) |
          ((word >> (31 - (absolute & 31))) & 1u);
    }
    assert(movie_media_read_bits32(reader_input, start) == expected);
  }

  BYTE zero_input[32] = {};
  MediaDecoderContext runtime = {};
  runtime.compressed_input_200 = zero_input;
  runtime.block_transform_weights[0] = 0x20000;
  runtime.predictor_4f8 = 9;
  runtime.predictor_4fc = 5;
  runtime.predictor_500 = 7;
  const BYTE *lookup_a = reinterpret_cast<const BYTE *>(
      movie_media_coefficient_tables.words) + 0x394;
  const BYTE *lookup_b = reinterpret_cast<const BYTE *>(
      movie_media_coefficient_tables.words) + 0x414;
  assert(movie_media_luma_delta_entries() == lookup_a);
  assert(movie_media_chroma_delta_entries() == lookup_b);
  reinterpret_cast<BYTE *>(movie_media_coefficient_tables.words)[0x394] = 0x12;
  reinterpret_cast<BYTE *>(movie_media_coefficient_tables.words)[0x414] = 0x12;
  DWORD scratch[512];
  for (int i = 0; i != 512; ++i) scratch[i] = 0x5a5a5a5au;
  sub_5d0aab(&runtime, scratch);
  assert(runtime.predictor_4f8 == 5);
  assert(runtime.predictor_4fc == 4);
  assert(runtime.predictor_500 == 6);
  assert(runtime.bit_position_204 == 18);
  const int starts[6] = {1, 9, 129, 137, 257, 321};
  const int fills[6] = {8, 7, 6, 5, 4, 6};
  for (int block = 0; block != 6; ++block) {
    const int pitch_words = block < 4 ? 16 : 8;
    for (int row = 0; row != 8; ++row) {
      for (int column = 0; column != 8; ++column)
        if ((int)scratch[starts[block] + row * pitch_words + column] !=
            fills[block]) {
          std::fprintf(stderr, "block %d row %d column %d got %d expected %d\\n",
                       block, row, column,
                       (int)scratch[starts[block] + row * pitch_words + column],
                       fills[block]);
          return 2;
        }
    }
  }
  assert(row_calls == 0 && column_calls == 0);

  MediaDecoderContext ac_runtime = {};
  BYTE ac_input[32] = {};
  ac_runtime.compressed_input_200 = ac_input;
  set_msb(ac_input, 0, 0x3f, 17);
  set_msb(ac_input, 17, 0, 1);
  movie_media_coefficient_tables.words[228] = 0x00000101u;
  movie_media_coefficient_tables.words[212] = 0x00004100u;
  movie_media_coefficient_tables.words[68] = 3;
  ac_runtime.block_transform_weights[3] = 0x20000;
  BYTE output[8 * 64];
  std::memset(output, 0xa5, sizeof(output));
  sub_5d0d82(&ac_runtime, 0, output, 64);
  assert(ac_runtime.bit_position_204 == 1);
  assert(row_calls == 8 && column_calls == 8);
  assert(first_row_value == -0x20000);

  MediaDecoderContext wrapper_runtime = {};
  wrapper_runtime.compressed_input_200 = zero_input;
  wrapper_runtime.block_transform_weights[0] = 0x20000;
  wrapper_runtime.predictor_4f8 = 2;
  BYTE wrapper_output[16 * 64] = {};
  sub_5d1466(wrapper_output, 64, &wrapper_runtime);
  assert(wrapper_runtime.bit_position_204 == 18);
  assert(wrapper_runtime.predictor_4f8 == static_cast<DWORD>(-2));
  assert(converter_calls == 16);
}
'''
    unit = tmp_path / "movie_decoder_six_blocks.cpp"
    unit.write_text(unit_source)
    binary = tmp_path / "movie_decoder_six_blocks"
    subprocess.run(
        [compiler, "-std=c++17", "-Wall", "-Wextra", "-Werror",
         "-pedantic", str(unit), "-o", str(binary)],
        check=True,
    )
    subprocess.run([str(binary)], check=True)

    mutants = (
        ("words[6 + run_position]", "words[7 + run_position]"),
        ("block < 4 ? 0x40 : 0x20", "block < 4 ? 0x20 : 0x20"),
        ("{1, 9, 129, 137, 257, 321}", "{2, 9, 129, 137, 257, 321}"),
        ("197 + (code >> 1)", "198 + (code >> 1)"),
    )
    for index, (before, after) in enumerate(mutants):
        assert before in unit_source
        mutated_source = unit_source.replace(before, after, 1)
        mutated_unit = tmp_path / f"movie_decoder_mutant_{index}.cpp"
        mutated_unit.write_text(mutated_source)
        mutated_binary = tmp_path / f"movie_decoder_mutant_{index}"
        subprocess.run(
            [compiler, "-std=c++17", "-Wall", "-Wextra", "-Werror",
             "-pedantic", str(mutated_unit), "-o", str(mutated_binary)],
            check=True,
        )
        assert subprocess.run([str(mutated_binary)], check=False).returncode != 0
