"""Live decoder-edge controls with mock IDCT/storage; not native ABI, codec tables or playback proof."""

from __future__ import annotations

import shutil
import subprocess
from pathlib import Path

import pytest


TEST_FILE = Path(__file__).resolve()
ROOT = next(parent for parent in TEST_FILE.parents if (parent / "src").is_dir())
SRC = ROOT / "src"
TESTS = ROOT / "recovery" / "tests"


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


def _compile_and_run(compiler: str, tmp_path: Path, sources: list[Path]) -> None:
    binary = tmp_path / "live-source-host"
    result = subprocess.run(
        [compiler, "-std=c++17", "-D__fastcall=", "-Wall", "-Wextra", "-Werror",
         "-pedantic", "-iquote", str(SRC), *map(str, sources), "-o", str(binary)],
        capture_output=True, text=True, check=False,
    )
    assert result.returncode == 0, result.stderr
    run = subprocess.run([str(binary)], capture_output=True, text=True, check=False)
    assert run.returncode == 0, run.stderr


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
def test_live_huffman_body_and_all_three_fixture_initializers(
    compiler_name: str, tmp_path: Path,
) -> None:
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(f"{compiler_name} is unavailable")

    header = (SRC / "movie_decoder_huffman.h").read_text()
    source = (SRC / "movie_decoder_huffman.cpp").read_text()
    host = (TESTS / "movie_decoder_huffman_host.cpp").read_text()
    assert "void (*pack_rows)(const uint8_t *component_rows" in header
    assert "args.pack_rows(args.scratch.luma_16x16, args.scratch.chroma_16x8," in source
    assert "transform_sixteen(args.scratch.luma_16x16" not in source
    assert host.count("idct, remap, pack_rows};") == 3
    assert "&coeff_state, idct, remap, pack_rows};" in host

    general = (SRC / "general.cpp").read_text()
    helpers = "\n\n".join(
        _function(general, sig)
        for sig in ('extern "C" void __fastcall sub_5d2d20(',
                    "void __fastcall sub_5d2e00(")
    )
    product = tmp_path / "mapped_delta_helpers.cpp"
    product.write_text("#define __fastcall\n#include <cstdint>\n" + helpers)
    _compile_and_run(
        compiler, tmp_path,
        [SRC / "movie_decoder_huffman.cpp", TESTS / "movie_decoder_huffman_host.cpp", product],
    )


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
def test_live_5d2e70_bridge_calls_mapped_5cea09_with_native_arguments(
    compiler_name: str, tmp_path: Path,
) -> None:
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(f"{compiler_name} is unavailable")
    bridge = (SRC / "movie_media_decoder.cpp").read_text()
    assert "sub_5cea09(component_rows, chroma_rows, destination, row_pitch_bytes);" in bridge
    assert "args.pack_rows = movie_decoder_pack_rows;" in bridge
    wrappers = "\n\n".join(
        _function(bridge, sig)
        for sig in ("static void movie_decoder_idct_rows(",
                    "static void movie_decoder_pack_rows(",
                    "void __fastcall sub_5d2e70(")
    )
    general = (SRC / "general.cpp").read_text()
    helpers = "\n\n".join(
        _function(general, sig)
        for sig in ('extern "C" void __fastcall sub_5d2d20(',
                    "void __fastcall sub_5d2e00(")
    )
    unit = r'''#include <cassert>
#include <cstdint>
#include <cstring>
#define __fastcall
#define __cdecl
using BYTE=uint8_t; using DWORD=uint32_t; using LONG=int32_t;
''' + helpers + r'''
#include "movie_decoder_huffman.h"
struct MediaDecoderProcessStorage {
  BYTE narrow_block_output_bytes[0x200]; BYTE wide_block_output_bytes[0x400];
  DWORD selected_record_mode_3364, plane_allocation_gate_3368, decoder_table_init_gate_336c;
};
struct MediaDecoderTableStorage {
  int32_t decoded_coefficient_words[64], scale_source_words[64], coefficient_position_byte_offsets[64];
  uint32_t primary_symbol_words[512], secondary_symbol_words_a[256], secondary_symbol_words_b[256], block_prefix_words[64];
  int32_t decoded_coefficient_scale_words[64];
  struct { const BYTE *cursor; uint32_t accumulator; int32_t available_bits; } frame_bit_state;
};
struct MediaConversionTableStorage { struct { DWORD words[1024]; } remap_or_lookup_storage; };
MediaDecoderProcessStorage movie_media_decoder_process_storage = {};
MediaDecoderTableStorage movie_media_decoder_tables = {};
MediaConversionTableStorage movie_media_conversion_tables = {};
static int pack_calls; static const BYTE *pack_component, *pack_chroma;
static BYTE *pack_destination; static int pack_pitch;
void __cdecl sub_5cea09(const BYTE *component, const BYTE *chroma, BYTE *destination, int pitch) {
  ++pack_calls; pack_component=component; pack_chroma=chroma; pack_destination=destination; pack_pitch=pitch;
  for (int row=0; row!=16; ++row) std::memset(destination+row*pitch, 0, 32);
}
void __cdecl sub_5cef8a(unsigned char *, int) {}
''' + wrappers + r'''
int main() {
  for (int i=0; i!=512; ++i) movie_media_decoder_tables.primary_symbol_words[i]=0x30;
  movie_media_decoder_tables.decoded_coefficient_scale_words[0]=1;
  BYTE packet[64]={}, coefficients[1024]={}, output[528]; std::memset(output,0xa5,sizeof(output));
  movie_media_decoder_tables.frame_bit_state.cursor=packet;
  movie_media_decoder_tables.frame_bit_state.accumulator=0;
  movie_media_decoder_tables.frame_bit_state.available_bits=32;
  sub_5d2e70(coefficients,output,16);
  assert(pack_calls==1);
  assert(pack_component==movie_media_decoder_process_storage.wide_block_output_bytes);
  assert(pack_chroma==movie_media_decoder_process_storage.narrow_block_output_bytes);
  assert(pack_destination==output && pack_pitch==32);
  assert(movie_media_decoder_tables.frame_bit_state.cursor==packet+10);
  assert(movie_media_decoder_tables.frame_bit_state.available_bits==20);
  for(int i=0;i!=512;++i) assert(output[i]==0);
  for(int i=512;i!=528;++i) assert(output[i]==0xa5);
}
'''
    host = tmp_path / "mapped_bridge.cpp"
    host.write_text(unit)
    # Compile the extracted current mapped adapter together with its globals
    # and edge capture stub. The Huffman TU and product delta helpers are live.
    binary = tmp_path / "mapped-bridge-host"
    result = subprocess.run(
        [compiler, "-std=c++17", "-D__fastcall=", "-Wall", "-Wextra", "-Werror",
         "-pedantic", "-iquote", str(SRC), str(SRC / "movie_decoder_huffman.cpp"),
         str(host), "-o", str(binary)],
        capture_output=True, text=True, check=False,
    )
    assert result.returncode == 0, result.stderr
    run = subprocess.run([str(binary)], capture_output=True, text=True, check=False)
    assert run.returncode == 0, run.stderr
