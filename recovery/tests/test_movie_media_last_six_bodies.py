from __future__ import annotations

import shutil
import subprocess
from pathlib import Path
import pytest


ROOT = Path(__file__).resolve().parent
SRC = ROOT.parents[1] / "src"


def _function(source: str, signature: str) -> str:
    start = source.index(signature)
    while True:
        opening = source.index("{", start)
        declaration_end = source.find(";", start, opening)
        if declaration_end < 0:
            break
        start = source.index(signature, declaration_end + 1)
    depth = 0
    for index in range(opening, len(source)):
        if source[index] == "{":
            depth += 1
        elif source[index] == "}":
            depth -= 1
            if depth == 0:
                return source[start:index + 1]
    raise AssertionError(f"unterminated function {signature}")


def _compile_and_run(tmp_path: Path, source: str) -> None:
    compiler = shutil.which("g++") or shutil.which("clang++")
    if compiler is None:
        pytest.skip("requires a host C++ compiler")
    unit = tmp_path / "movie_media_last_six.cpp"
    unit.write_text(source)
    binary = tmp_path / "movie_media_last_six"
    subprocess.run(
        [compiler, "-std=c++17", "-Wall", "-Wextra", "-Werror",
         "-pedantic", str(unit), "-o", str(binary)],
        check=True,
    )
    subprocess.run([str(binary)], check=True, cwd=tmp_path)


def test_format_header_and_three_palette_channels_use_native_fields(
    tmp_path: Path,
) -> None:
    media = (SRC / "movie_media.cpp").read_text()
    body = _function(media, 'extern "C" void __fastcall sub_5cd750(')
    source = r'''#include <cassert>
#include <cstdint>
#include <cstring>
#define __fastcall
#define __int64 long long
using BYTE = unsigned char;
using DWORD = unsigned int;
struct MediaDescriptor {
  DWORD output_width_64;
  DWORD output_height_60;
  DWORD setup_value_78;
  BYTE palette_rgb_triplets_b0[0x300];
};
struct MediaRuntime {
  MediaDescriptor *descriptor;
  DWORD unresolved_word_618;
  DWORD unresolved_word_61c;
};
''' + body + r'''
static void put16(BYTE *p, DWORD value) {
  unsigned short word = (unsigned short)value;
  std::memcpy(p, &word, 2);
}
int main() {
  BYTE input[64] = {};
  BYTE *packet = input + 8;
  put16(packet - 2, 0x1111);
  put16(packet, 0x1234);
  put16(packet + 2, 0x0567);
  put16(packet + 4, 0x00ab);
  put16(packet + 6, 2);
  packet[12] = 200; packet[13] = 100; packet[14] = 50;
  packet[15] = 25; packet[16] = 75; packet[17] = 125;
  MediaDescriptor descriptor = {};
  descriptor.setup_value_78 = 200;
  MediaRuntime runtime = {&descriptor, 0, 0};
  sub_5cd750(&runtime, packet);
  assert(descriptor.output_width_64 == 0x1234);
  assert(descriptor.output_height_60 == 0x0567);
  assert(runtime.unresolved_word_618 == 0x00ab);
  assert(runtime.unresolved_word_61c == 2);
  assert(descriptor.palette_rgb_triplets_b0[0] == 255);
  assert(descriptor.palette_rgb_triplets_b0[1] == 200);
  assert(descriptor.palette_rgb_triplets_b0[2] == 100);
  assert(descriptor.palette_rgb_triplets_b0[3] == 50);
  assert(descriptor.palette_rgb_triplets_b0[4] == 150);
  assert(descriptor.palette_rgb_triplets_b0[5] == 250);
}
'''
    _compile_and_run(tmp_path, source)


def test_block_dispatchers_preserve_branch_order_and_row_steps(
    tmp_path: Path,
) -> None:
    media = (SRC / "movie_media.cpp").read_text()
    bodies = "\n".join(
        _function(media, signature)
        for signature in (
            'extern "C" void __fastcall sub_5cd8d0(',
            'extern "C" void __fastcall sub_5cd980(',
            'extern "C" void __fastcall sub_5cda30(',
        )
    )
    source = r'''#include <cassert>
#include <cstdint>
#include <cstring>
#define __fastcall
#define __cdecl
using BYTE = unsigned char;
using DWORD = unsigned int;
struct MediaDescriptor { DWORD output_height_60, output_width_64; };
struct MediaDecoderContext {
  DWORD block_transform_weights[64];
  DWORD predictor_4f8, predictor_4fc, predictor_500;
  BYTE unresolved_original_storage_10c_1ff[0xf4];
  const BYTE *compressed_input_200;
  DWORD bit_position_204;
};
struct MediaRuntime {
  MediaDescriptor *descriptor;
  DWORD record_option_040;
  MediaDecoderContext decoder_context_3f8;
};
struct Call { BYTE *address; int pitch; void *context; int variant; } calls[32];
int call_count;
void __cdecl sub_5d1466(BYTE *p, int s, MediaDecoderContext *c) { calls[call_count++]={p,s,c,1}; }
void __cdecl sub_5d1842(BYTE *p, int s, MediaDecoderContext *c) { calls[call_count++]={p,s,c,2}; }
void __cdecl sub_5d1654(BYTE *p, int s, MediaDecoderContext *c) { calls[call_count++]={p,s,c,3}; }
void __cdecl sub_5d1a30(BYTE *p, int s, MediaDecoderContext *c) { calls[call_count++]={p,s,c,4}; }
void __cdecl sub_5d1278(BYTE *p, int s, MediaDecoderContext *c) { calls[call_count++]={p,s,c,5}; }
''' + bodies + r'''
int main() {
  MediaDescriptor descriptor = {32, 32};
  BYTE compressed[128] = {}, destination[4096] = {};
  MediaRuntime runtime = {};
  runtime.descriptor = &descriptor;
  runtime.record_option_040 = 1;
  runtime.decoder_context_3f8.predictor_4f8 =
      runtime.decoder_context_3f8.predictor_4fc =
      runtime.decoder_context_3f8.predictor_500 = 9;
  runtime.decoder_context_3f8.bit_position_204 = 9;
  sub_5cd8d0(&runtime, compressed, destination, compressed, 64);
  assert(runtime.decoder_context_3f8.compressed_input_200 == compressed &&
         runtime.decoder_context_3f8.bit_position_204 == 0);
  assert(runtime.decoder_context_3f8.predictor_4f8 == 0 &&
         runtime.decoder_context_3f8.predictor_4fc == 0 &&
         runtime.decoder_context_3f8.predictor_500 == 0);
  assert(call_count == 4);
  assert(calls[0].address == destination);
  assert(calls[1].address == destination + 32);
  assert(calls[2].address == destination + 1024);
  assert(calls[3].address == destination + 1056);
  assert(calls[0].variant == 1);
  assert(calls[0].pitch == 64);
  assert(calls[0].context == &runtime.decoder_context_3f8);

  call_count = 0;
  runtime.record_option_040 = 0;
  sub_5cd980(&runtime, compressed, destination, compressed, 64);
  assert(call_count == 4);
  assert(calls[0].address == destination);
  assert(calls[1].address == destination + 64);
  assert(calls[2].address == destination + 1024);
  assert(calls[3].address == destination + 1088);
  assert(calls[0].variant == 4);

  call_count = 0;
  sub_5cda30(&runtime, compressed, destination, compressed, 64);
  assert(call_count == 4);
  assert(calls[0].address == destination);
  assert(calls[1].address == destination + 32);
  assert(calls[2].address == destination + 1024);
  assert(calls[3].address == destination + 1056);
  assert(calls[0].variant == 5);
  assert(runtime.decoder_context_3f8.compressed_input_200 == compressed);
}
'''
    _compile_and_run(tmp_path, source)


def test_macroblock_bodies_initialize_shared_reader_and_emit_six_phases(
    tmp_path: Path,
) -> None:
    owner = (SRC / "movie_media_decoder.cpp").read_text()
    media = (SRC / "movie_media.cpp").read_text()
    general = (SRC / "general.cpp").read_text()
    helpers = "\n".join(
        _function(owner, signature)
        for signature in (
            "static DWORD movie_pixel_read_dword_address(",
            "static DWORD movie_pixel_read_dword_at(",
            "void __cdecl OutputPixelRow(MediaConversionTableStorage &tables,",
            "void __cdecl OutputPixelRow16(MediaConversionTableStorage &tables,",
            "void __cdecl sub_5cea09(",
            "void __cdecl OutputPixelRow(int *destination,",
            "void __cdecl OutputPixelRow16(short *destination,",
            "static int *movie_media_block_words(",
            "static int *movie_media_row_words(",
            "void movie_media_transform_block_pair(",
            "void movie_media_write_macroblock_rows(",
        )
    )
    packer = _function(general, "int __cdecl sub_5ceb12(")
    bodies = "\n".join(
        _function(media, signature)
        for signature in (
            "void __cdecl sub_5cfaed(",
            "void __cdecl sub_5d02cc(BYTE *compressed, void *destination,",
        )
    )
    source = r'''#include <cassert>
#include <cstdint>
#include <cstring>
#define __cdecl
#define DWORD unsigned int
using BYTE = unsigned char;
struct MediaTransformBitState { const BYTE *cursor; int available_bits; DWORD accumulator; };
struct MediaConversionTableStorage {
  BYTE prefix[0x100]; MediaTransformBitState transform_bit_state;
  DWORD packed_component_words[384];
  struct { DWORD words[1024]; } remap_or_lookup_storage;
};
struct MediaTransformWorkspace { int block_and_intermediate_words[384]; int row_and_butterfly_words[135]; };
MediaConversionTableStorage movie_media_conversion_tables = {};
MediaTransformWorkspace movie_media_transform_workspace = {};
int reader_calls, dct_rows, dct_columns;
static unsigned char *const MovieByteRemapLut =
    reinterpret_cast<unsigned char *>(movie_media_conversion_tables.remap_or_lookup_storage.words);
void __cdecl sub_5cf150() { ++reader_calls; }
extern "C" void __cdecl sub_5d29e1(int *src, int *dst) {
  int call = dct_rows;
  ++dct_rows;
  assert(src == movie_media_transform_workspace.row_and_butterfly_words + (call % 8) * 8);
  assert(dst == movie_media_transform_workspace.row_and_butterfly_words + 64 + (call % 8));
}
extern "C" void __cdecl sub_5d2b20(int *dst, int *src) {
  const int group_base[6] = {0, 0, 128, 128, 256, 256};
  const int half_block[6] = {0, 8, 0, 8, 0, 8};
  const int phase = dct_columns / 8;
  const int column = dct_columns % 8;
  assert(phase >= 0 && phase < 6);
  assert(src == movie_media_transform_workspace.row_and_butterfly_words + 64 + column * 9);
  assert(dst == movie_media_transform_workspace.block_and_intermediate_words +
                group_base[phase] + half_block[phase] + column * 16);
  ++dct_columns;
}
''' + packer + helpers + bodies + r'''
int main() {
  BYTE compressed[16] = {0x93};
  BYTE output[4096] = {};
  sub_5cfaed(compressed, output, 64);
  assert(movie_media_conversion_tables.transform_bit_state.cursor == compressed + 1);
  assert(movie_media_conversion_tables.transform_bit_state.available_bits == 8);
  assert(movie_media_conversion_tables.transform_bit_state.accumulator == 0x93);
  assert(reader_calls == 6 && dct_rows == 48 && dct_columns == 48);
  assert(movie_media_conversion_tables.packed_component_words[0] == 0);
  assert(movie_media_conversion_tables.packed_component_words[128] == 0);

  // Exercise the real row converters with one nonzero table row and source
  // indexes. Signed workspace words are the corresponding unsigned int view.
  for (int i = 0; i < 384; ++i)
    movie_media_conversion_tables.packed_component_words[i] = i * i + 17;
  movie_media_transform_workspace.block_and_intermediate_words[0] = 3 << 17;
  movie_media_transform_workspace.block_and_intermediate_words[1] = 4 << 17;
  movie_media_transform_workspace.block_and_intermediate_words[256] = 5 << 17;
  movie_media_transform_workspace.block_and_intermediate_words[320] = 6 << 17;
  int pixels[256] = {};
  movie_media_write_macroblock_rows(reinterpret_cast<BYTE *>(pixels), 64, 0);
  assert(pixels[0] == (3 * 3 + (256 + 5) * (256 + 5) +
                       (128 + 6) * (128 + 6) + 3 * 17));
  assert(pixels[1] == (4 * 4 + (256 + 5) * (256 + 5) +
                       (128 + 6) * (128 + 6) + 3 * 17));

  BYTE component_rows[16 * 0x40] = {};
  BYTE chroma_rows[8 * 0x20 + 0x120] = {};
  BYTE packed[16 * 32] = {};
  for (int i = 0; i < 256; ++i) MovieByteRemapLut[i] = (BYTE)i;
  component_rows[2] = 1; component_rows[6] = 2;
  chroma_rows[2] = 3; chroma_rows[0x102] = 4;
  sub_5cea09(component_rows, chroma_rows, packed, 32);
  unsigned int packed_word = 0;
  std::memcpy(&packed_word, packed, sizeof(packed_word));
  assert(packed_word == 0x01020304u);

  reader_calls = dct_rows = dct_columns = 0;
  sub_5d02cc(compressed, output, 64);
  assert(movie_media_conversion_tables.transform_bit_state.cursor == compressed + 1);
  assert(movie_media_conversion_tables.transform_bit_state.available_bits == 8);
  assert(movie_media_conversion_tables.transform_bit_state.accumulator == 0x93);
  assert(reader_calls == 6 && dct_rows == 48 && dct_columns == 48);
}
'''
    _compile_and_run(tmp_path, source)
    original = "const int group_base[6] = {0, 0, 128, 128, 256, 256};"
    # Change the production helper only, leaving the independently spelled
    # expected arguments in the DCT stub intact.
    assert source.count(original) == 2
    head, tail = source.rsplit(original, 1)
    mutant = head + "const int group_base[6] = {0, 0, 0, 0, 0, 0};" + tail
    with pytest.raises(subprocess.CalledProcessError):
        _compile_and_run(tmp_path, mutant)
