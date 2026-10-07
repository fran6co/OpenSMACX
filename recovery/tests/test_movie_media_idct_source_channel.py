"""Actual-source IDCT source-channel and output-stride controls; not native ABI proof."""
from __future__ import annotations

import shutil
import subprocess
from pathlib import Path

import pytest


ROOT = next(p for p in Path(__file__).resolve().parents if (p / "src/movie_media_decoder.cpp").is_file())
DECODER = ROOT / "src/movie_media_decoder.cpp"


def definition(source: str, signature: str) -> str:
    start = source.index(signature)
    opening = source.index("{", start)
    depth = 0
    for end in range(opening, len(source)):
        if source[end] == "{":
            depth += 1
        elif source[end] == "}":
            depth -= 1
            if depth == 0:
                return source[start : end + 1]
    raise AssertionError(f"unterminated definition {signature}")


def source_unit(mutant: str | None = None) -> str:
    decoder = DECODER.read_text(encoding="utf-8")
    signatures = (
        "static int *movie_media_idct_output_pointer()",
        "static movie_decoder_idct::IdctConstants movie_media_idct_constants()",
        "void __cdecl sub_5ced1d(int *esi_ptr)",
    )
    bodies = [definition(decoder, signature) for signature in signatures]
    if mutant == "source_channel":
        needle = "reinterpret_cast<const int32_t *>(esi_ptr)"
        assert needle in bodies[2]
        bodies[2] = bodies[2].replace(
            needle,
            "reinterpret_cast<const int32_t *>(esi_ptr ? movie_media_idct_output_pointer() : esi_ptr)",
            1,
        )
    elif mutant == "destination_stride":
        needle = "dst[row * 9] = transformed[row];"
        assert bodies[2].count(needle) == 2
        bodies[2] = bodies[2].replace(needle, "dst[row * 8] = transformed[row];")

    harness = r'''#include <cassert>
#include <cstdint>
#include <cstring>
#define __cdecl
using BYTE = std::uint8_t;
using DWORD = std::uint32_t;
using LONG = std::int32_t;
struct MediaDecoderIdctStorage {
  DWORD output_pitch_bytes;
  std::uintptr_t current_output_pointer_bits;
  DWORD butterfly_fp_scratch_words[4];
  DWORD constant_words[5];
  int intermediate_words[72];
};
MediaDecoderIdctStorage movie_media_idct_storage = {};
#include "movie_decoder_idct.h"
''' + "\n".join(bodies[:2]) + r'''
static void movie_media_idct_set_output_pointer(int *value) {
  movie_media_idct_storage.current_output_pointer_bits =
      reinterpret_cast<std::uintptr_t>(value);
}
''' + bodies[2] + r'''
int main() {
  int source[8] = {17, 0, 0, 0, 0, 0, 0, 0};
  int destination[72];
  for (int i = 0; i != 72; ++i) destination[i] = -91;
  movie_media_idct_set_output_pointer(destination);
  sub_5ced1d(source);
  for (int row = 0; row != 8; ++row) {
    assert(destination[row * 9] == 17);
    for (int gap = 1; gap != 9; ++gap)
      assert(destination[row * 9 + gap] == -91);
  }

  // A nonzero vector takes the full butterfly and preserves the same sparse
  // 9-dword destination pattern. Values are not used as x87 equivalence proof.
  for (int i = 0; i != 8; ++i) source[i] = i + 1;
  for (int i = 0; i != 72; ++i) destination[i] = -91;
  sub_5ced1d(source);
  bool wrote = false;
  for (int row = 0; row != 8; ++row) {
    wrote = wrote || destination[row * 9] != -91;
    for (int gap = 1; gap != 9; ++gap)
      assert(destination[row * 9 + gap] == -91);
  }
  assert(wrote);
}
'''
    return harness


def compile_and_run(compiler: str, tmp_path: Path, unit: str, *, success: bool) -> None:
    source = tmp_path / "actual_5ced1d.cpp"
    binary = tmp_path / "actual_5ced1d"
    source.write_text(unit, encoding="utf-8")
    subprocess.run(
        [compiler, "-std=c++17", "-Wall", "-Wextra", "-Werror", "-pedantic",
         "-iquote", str(ROOT / "src"), str(ROOT / "src/movie_decoder_idct.cpp"),
         str(source), "-o", str(binary)],
        check=True,
        capture_output=True,
        text=True,
    )
    ran = subprocess.run([str(binary)], capture_output=True, text=True, timeout=10)
    if success:
        assert ran.returncode == 0, ran.stdout + ran.stderr
    else:
        assert ran.returncode != 0, "mutated installed-source control unexpectedly passed"


@pytest.mark.parametrize("compiler_name", ["g++", "clang++"])
def test_installed_5ced1d_body_and_source_channel_mutant(compiler_name, tmp_path):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(f"{compiler_name} unavailable")
    compile_and_run(compiler, tmp_path, source_unit(), success=True)
    compile_and_run(
        compiler, tmp_path, source_unit("source_channel"), success=False
    )


@pytest.mark.parametrize("compiler_name", ["g++", "clang++"])
def test_installed_transform_stride_mutant(compiler_name, tmp_path):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(f"{compiler_name} unavailable")
    compile_and_run(
        compiler, tmp_path, source_unit("destination_stride"), success=False
    )
