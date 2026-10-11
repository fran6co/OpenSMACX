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
    body = _function(text, 'extern "C" void __fastcall sub_5cc940(')
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
static BYTE *seen_destinations[8];
static unsigned seen_pitches[8], seen_count;
static void capture(void *destination, unsigned pitch) {
  assert(seen_count < 8);
  seen_destinations[seen_count] = static_cast<BYTE *>(destination);
  seen_pitches[seen_count++] = pitch;
}
void media_build_runtime_block_scale_table(int) {}
void sub_5d02cc(unsigned char *, void *d, int p) { ++large_16_calls; capture(d,p); }
void sub_5cfaed(unsigned char *, BYTE *d, int p) { ++large_32_calls; capture(d,p); }
void sub_5cd3b0(char, char, char, char, char, char, int *d, unsigned int p) {
  ++short_32_calls;
  capture(d,p);
}
void sub_5cd560(signed char, signed char, signed char, signed char,
                signed char, signed char, int *d, unsigned int p) {
  ++short_16_calls;
  capture(d,p);
}
static void run_case(DWORD flags, BYTE run_bytes, DWORD height=16) {
  seen_count=0;
  MediaRuntime runtime_value = {};
  runtime_value.output_rect_top_18=3;
  runtime_value.output_rect_left_14=2;
  MediaRuntime *runtime = &runtime_value;
  MediaDescriptor d = {};
  d.output_width_64 = 16;
  d.output_height_60 = height;
  d.flags = flags;
  BYTE compressed_storage[32] = {};
  compressed_storage[0] = run_bytes;
  compressed_storage[1+run_bytes] = run_bytes;
  BYTE *compressed = compressed_storage;
  BYTE surface_storage[8192] = {};
  BYTE *surface_source = surface_storage;
  DWORD output_pitch = 64;
  switch (3) {
''' + case3 + r'''
    case 4:
      break;
  }
  if (run_bytes > 12 || run_bytes == 3 || run_bytes == 6 || run_bytes == 12) {
    const unsigned pitch=(flags&0x30)?128:64;
    assert(seen_count==height/16);
    for(unsigned row=0;row<seen_count;++row) {
      assert(seen_pitches[row]==pitch);
      // Origin uses the undoubled stride; only block/row stepping scales.
      assert(seen_destinations[row]==surface_storage+3*64+2*((flags&0x40)?4:2)+row*pitch*16);
    }
  } else assert(seen_count==0);
}
int main() {
  run_case(0, 13);
  assert(large_16_calls == 1 && large_32_calls == 0);
  run_case(0x40, 13);
  assert(large_16_calls == 1 && large_32_calls == 1);
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
  for (DWORD pixel_mode : {DWORD(0),DWORD(0x40)})
    for (DWORD scale : {DWORD(0),DWORD(0x10),DWORD(0x20),DWORD(0x30)}) {
      run_case(pixel_mode|scale,13,32);
      run_case(pixel_mode|scale,3,32);
    }
}
'''
    harness = '#include <initializer_list>\n' + harness
    assert _compile_and_run(harness, tmp_path, "actual") == 0

    for label, before, after in (
        ('wrong_large_dispatch','sub_5cfaed((unsigned char *)compressed','sub_5d02cc((unsigned char *)compressed'),
        ('missing_scaled_stride','if ((d.flags & 0x30) != 0) block_pitch <<= 1;','/* missing scaling */'),
        ('scaled_origin','runtime->output_rect_top_18 * output_pitch','runtime->output_rect_top_18 * (output_pitch * ((d.flags & 0x30) ? 2 : 1))'),
    ):
        assert before in harness
        assert _compile_and_run(harness.replace(before,after,1),tmp_path,label)!=0
