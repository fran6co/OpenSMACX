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


def test_mapped_transform_reader_updates_its_own_source_storage(
    tmp_path: Path,
) -> None:
    compiler = shutil.which("g++") or shutil.which("clang++")
    if compiler is None:
        return
    leaf = (SRC / "leaf_recoveries.cpp").read_text()
    body = _function(leaf, "int __cdecl sub_5cf150()")
    source = r'''#include <cassert>
#include <cstdint>
#include <cstring>
#define __cdecl
#define __int64 long long
typedef unsigned char BYTE;
typedef unsigned int DWORD;
typedef int LONG;
struct MediaTransformBitState {
  const BYTE *cursor;
  LONG available_bits;
  DWORD accumulator;
};
struct MediaConversionTableStorage {
  DWORD block_scale_entries[64];
  MediaTransformBitState transform_bit_state;
  DWORD packed_component_words[384];
};
struct MediaTransformWorkspace {
  int block_and_intermediate_words[384];
  int row_and_butterfly_words[135];
};
struct MediaCoefficientTableStorage { DWORD words[325]; };
MediaConversionTableStorage movie_media_conversion_tables = {};
MediaTransformWorkspace movie_media_transform_workspace = {};
MediaCoefficientTableStorage movie_media_coefficient_tables = {};
''' + body + r'''
int main() {
  BYTE packet[128] = {};
  for (int i = 0; i != 135; ++i)
    movie_media_transform_workspace.row_and_butterfly_words[i] = 0x5a5a5a5a;
  for (int i = 0; i != 64; ++i)
        movie_media_coefficient_tables.words[5 + i] = static_cast<DWORD>(i);
  for (int i = 0; i != 384; ++i)
    movie_media_conversion_tables.packed_component_words[i] = 0x12340000u + i;
  MediaTransformBitState &state = movie_media_conversion_tables.transform_bit_state;
  state.cursor = packet;
  state.available_bits = 32;
  state.accumulator = 0;
  assert(sub_5cf150() == 0);
  assert(state.cursor == packet + 22);
  assert(state.available_bits == 11);
  assert(state.accumulator == 0);
  for (int i = 0; i != 64; ++i)
    assert(movie_media_transform_workspace.row_and_butterfly_words[i] == 0);
  for (int i = 64; i != 135; ++i)
    assert(movie_media_transform_workspace.row_and_butterfly_words[i] == 0x5a5a5a5a);
  for (int i = 0; i != 384; ++i)
    assert(movie_media_conversion_tables.packed_component_words[i] == 0x12340000u + i);
}
'''
    unit = tmp_path / "mapped_transform_reader.cpp"
    unit.write_text(source)
    binary = tmp_path / "mapped_transform_reader"
    subprocess.run(
        [compiler, "-std=c++17", "-Wall", "-Wextra", "-Werror",
         "-pedantic", str(unit), "-o", str(binary)],
        check=True,
    )
    subprocess.run([str(binary)], check=True)
