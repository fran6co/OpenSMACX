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


def _case_block(body: str) -> str:
    start = body.index("case 3: {")
    opening = body.index("{", start)
    depth = 0
    for index in range(opening, len(body)):
        if body[index] == "{":
            depth += 1
        elif body[index] == "}":
            depth -= 1
            if depth == 0:
                return body[start:index + 1]
    raise AssertionError("unterminated case-3 block")


def _compile_and_run(source: str, tmp_path: Path, suffix: str) -> int:
    compiler = shutil.which("g++") or shutil.which("clang++")
    if compiler is None:
        return 0
    unit = tmp_path / f"movie_media_case3_{suffix}.cpp"
    unit.write_text(source)
    binary = tmp_path / f"movie_media_case3_{suffix}"
    subprocess.run(
        [compiler, "-std=c++17", "-Wall", "-Wextra", "-Werror", "-pedantic",
         str(unit), "-o", str(binary)],
        check=True,
    )
    return subprocess.run([str(binary)], check=False).returncode


def test_actual_case3_dispatch_preserves_large_run_edge(tmp_path: Path) -> None:
    text = (SRC / "movie_media.cpp").read_text()
    body = _function(text, 'extern "C" int __fastcall sub_5cc940(')
    case3 = _case_block(body)
    harness = r'''#include <cassert>
#include <cstdint>
#include <cstring>
#define __fastcall
#define BYTE unsigned char
typedef std::uint32_t DWORD;
struct MediaRuntime {
  DWORD record_scale_byte_048;
  int output_rect_top_18;
  int output_rect_left_14;
};
struct MediaDescriptor {
  DWORD output_height_60;
  DWORD output_width_64;
  DWORD flags;
};
static int large_16_calls;
static int large_32_calls;
static int short_16_calls;
static int short_32_calls;
void media_build_runtime_block_scale_table(int) {}
void sub_5d02cc(unsigned char *, void *, int) { ++large_16_calls; }
void sub_5cfaed(unsigned char *, BYTE *, int) { ++large_32_calls; }
void sub_5cd3b0(char, char, char, char, char, char, int *, unsigned int) {
  ++short_32_calls;
}
void sub_5cd560(signed char, signed char, signed char, signed char,
                signed char, signed char, int *, unsigned int) {
  ++short_16_calls;
}
static void run_case(DWORD flags, BYTE run_bytes) {
  MediaRuntime runtime_value = {};
  MediaRuntime *runtime = &runtime_value;
  MediaDescriptor d = {};
  d.output_width_64 = 16;
  d.output_height_60 = 16;
  d.flags = flags;
  BYTE compressed_storage[32] = {};
  compressed_storage[0] = run_bytes;
  BYTE *compressed = compressed_storage;
  BYTE surface_storage[4096] = {};
  BYTE *surface_source = surface_storage;
  DWORD output_pitch = 64;
  switch (3) {
''' + case3 + r'''
    case 4:
      break;
  }
}
int main() {
  run_case(0, 13);
  assert(large_16_calls == 1 && large_32_calls == 0);
  run_case(0x40, 13);
  assert(large_16_calls == 2 && large_32_calls == 0);
  run_case(0, 3);
  assert(short_16_calls == 1 && short_32_calls == 0);
  run_case(0x40, 3);
  assert(short_16_calls == 1 && short_32_calls == 1);
  run_case(0, 6);
  assert(short_16_calls == 2 && short_32_calls == 1);
  run_case(0x40, 12);
  assert(short_16_calls == 2 && short_32_calls == 2);
  for (unsigned run_bytes = 0; run_bytes <= 12; ++run_bytes) {
    if (run_bytes == 3 || run_bytes == 6 || run_bytes == 12) continue;
    run_case(0, static_cast<BYTE>(run_bytes));
  }
  assert(short_16_calls == 2 && short_32_calls == 2);
}
'''
    assert _compile_and_run(harness, tmp_path, "actual") == 0

    wrong_dispatch = (
        "sub_5d02cc((unsigned char *)compressed, output_block,\n"
        "                     (int)block_pitch);"
    )
    replacement = (
        "if (four_byte_pixels)\n"
        "            sub_5cfaed((unsigned char *)compressed, output_block,\n"
        "                       (int)block_pitch);\n"
        "          else\n"
        "            sub_5d02cc((unsigned char *)compressed, output_block,\n"
        "                       (int)block_pitch);"
    )
    assert wrong_dispatch in harness
    mutant = harness.replace(wrong_dispatch, replacement, 1)
    assert _compile_and_run(mutant, tmp_path, "wrong_large_dispatch") != 0
