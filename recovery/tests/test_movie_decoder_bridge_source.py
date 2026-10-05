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


def test_mapped_six_block_bridge_links_and_updates_shared_reader_state(
    tmp_path: Path,
) -> None:
    compiler = shutil.which("g++") or shutil.which("clang++")
    if compiler is None:
        return
    bridge = (SRC / "movie_media_decoder.cpp").read_text()
    wrappers = "\n\n".join(
        _function(bridge, signature)
        for signature in (
            "static void movie_decoder_idct_rows(",
            "void __fastcall sub_5d2e70(",
        )
    )
    general = (SRC / "general.cpp").read_text()
    helpers = "\n\n".join(
        _function(general, signature)
        for signature in (
            'extern "C" void __fastcall sub_5d2d20(',
            "void __fastcall sub_5d2e00(",
        )
    )
    source = r'''#include <cassert>
#include <cstdint>
#include <cstring>
#include <stdint.h>
#define __fastcall
#define __cdecl
using BYTE = uint8_t;
using DWORD = uint32_t;
using LONG = int32_t;
''' + helpers + r'''
#include "movie_decoder_huffman.h"
struct MediaDecoderProcessStorage {
  BYTE narrow_block_output_bytes[0x200];
  BYTE wide_block_output_bytes[0x400];
  DWORD selected_record_mode_3364;
  DWORD plane_allocation_gate_3368;
  DWORD decoder_table_init_gate_336c;
};
struct MediaDecoderTableStorage {
  int32_t decoded_coefficient_words[64];
  int32_t scale_source_words[64];
  int32_t coefficient_position_byte_offsets[64];
  uint32_t primary_symbol_words[512];
  uint32_t secondary_symbol_words_a[256];
  uint32_t secondary_symbol_words_b[256];
  uint32_t block_prefix_words[64];
  int32_t decoded_coefficient_scale_words[64];
  struct { const BYTE *cursor; uint32_t accumulator; int32_t available_bits; }
      frame_bit_state;
};
struct MediaConversionTableStorage { struct { DWORD words[1024]; } remap_or_lookup_storage; };
MediaDecoderProcessStorage movie_media_decoder_process_storage = {};
MediaDecoderTableStorage movie_media_decoder_tables = {};
MediaConversionTableStorage movie_media_conversion_tables = {};
void __cdecl sub_5cef8a(unsigned char *, int) {}
''' + wrappers + r'''
int main() {
  for (int i = 0; i != 512; ++i)
    movie_media_decoder_tables.primary_symbol_words[i] = 0x30;
  movie_media_decoder_tables.decoded_coefficient_scale_words[0] = 1;
  BYTE packet[64] = {};
  BYTE coefficients[1024] = {};
  BYTE output[528];
  std::memset(output, 0xa5, sizeof(output));
  movie_media_decoder_tables.frame_bit_state.cursor = packet;
  movie_media_decoder_tables.frame_bit_state.accumulator = 0;
  movie_media_decoder_tables.frame_bit_state.available_bits = 32;
  sub_5d2e70(coefficients, output, 16);
  assert(movie_media_decoder_tables.frame_bit_state.cursor == packet + 10);
  assert(movie_media_decoder_tables.frame_bit_state.available_bits == 20);
  for (int i = 0; i != 512; ++i) assert(output[i] == 0);
  for (int i = 512; i != 528; ++i) assert(output[i] == 0xa5);
}
'''
    unit = tmp_path / "mapped_bridge.cpp"
    unit.write_text(source)
    binary = tmp_path / "mapped_bridge"
    subprocess.run(
        [compiler, "-std=c++17", "-D__fastcall=", "-Wall", "-Wextra", "-Werror",
         "-pedantic", "-iquote", str(SRC),
         str(SRC / "movie_decoder_huffman.cpp"), str(unit),
         "-o", str(binary)],
        check=True,
    )
    subprocess.run([str(binary)], check=True)
