from __future__ import annotations

import shutil
import subprocess
import re
import os
from pathlib import Path

import pytest

ROOT = Path(__file__).resolve().parent
SRC = ROOT.parents[1] / "src"


def test_pinned_idct_wrapper_advances_output_only_between_row_calls() -> None:
    from capstone import Cs, CS_ARCH_X86, CS_MODE_32
    from capstone.x86 import X86_OP_IMM, X86_OP_MEM
    from match_recovery.model import Span
    from match_recovery.pe_coff import PeImage

    path = Path(os.environ.get("OPENSMACX_IMAGE",
                              str(SRC.parent / ".opensmacx/game/terranx_original.exe")))
    if not path.is_file():
        pytest.skip("pinned original unavailable")
    image = PeImage(path.read_bytes(),
                    "01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf")
    decoder = Cs(CS_ARCH_X86, CS_MODE_32)
    decoder.detail = True
    instructions = list(decoder.disasm(image.read(Span(0x5CF048, 0x5CF11D)), 0x5CF048))
    assert sum(i.size for i in instructions) == 0x5CF11D - 0x5CF048
    calls = [i for i in instructions if i.mnemonic == "call"
             and i.operands[0].type == X86_OP_IMM
             and i.operands[0].imm == 0x5CEE80]
    stores = [i for i in instructions if i.mnemonic == "mov"
              and i.operands[0].type == X86_OP_MEM
              and not i.operands[0].mem.base and not i.operands[0].mem.index
              and i.operands[0].mem.disp == 0x9C3004]
    assert len(calls) == len(stores) == 8  # Initial pointer and seven advances.
    assert calls[0].address == 0x5CF04D
    assert calls[-1].address == 0x5CF111
    assert calls[-1].address + calls[-1].size == 0x5CF116
    assert stores[0].address < calls[0].address
    for previous, current in zip(calls, calls[1:]):
        assert sum(previous.address < s.address < current.address for s in stores) == 1
    assert not any(s.address > calls[-1].address for s in stores)
    assert instructions[-1].mnemonic == "ret"


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


def _without_comments(source: str) -> str:
    return re.sub(r"/\*.*?\*/|//[^\n]*", "", source, flags=re.DOTALL)


def test_mapped_idct_bodies_compile_and_execute_from_product_owner(
    tmp_path: Path,
) -> None:
    compiler = shutil.which("g++") or shutil.which("clang++")
    if compiler is None:
        return
    decoder = _without_comments((SRC / "movie_media_decoder.cpp").read_text())
    signatures = (
        "static int *movie_media_idct_output_pointer()",
        "static void movie_media_idct_set_output_pointer(int *value)",
        "static float movie_media_idct_float_constant(int index)",
        "static movie_decoder_idct::IdctConstants movie_media_idct_constants()",
        "static DWORD movie_media_int_bits(int value)",
        "static int movie_media_signed32(DWORD value)",
        "static int movie_media_add32(int left, int right)",
        "static int movie_media_sub32(int left, int right)",
        "static int movie_media_mul2(int value)",
        "static int movie_media_signed_high_product(int left, int right)",
        "void __cdecl sub_5ced1d(int *esi_ptr)",
        "void __cdecl sub_5cee80(int *src)",
        "void __cdecl sub_5cef8a(unsigned char *row_base, int stride_arg)",
    )
    owners = "\n\n".join(_function(decoder, signature) for signature in signatures)
    owner_header = (SRC / "movie_decoder_owner.h").read_text()
    assert "extern MediaDecoderTableStorage movie_media_decoder_tables;" in owner_header
    source = r'''#include <cassert>
#include <cstdint>
#include <cstring>
#define __cdecl
using BYTE = std::uint8_t;
using DWORD = std::uint32_t;
using LONG = std::int32_t;
using __int64 = long long;
struct MediaDecoderTableStorage { int32_t decoded_coefficient_words[64]; };
struct MediaDecoderIdctStorage {
  DWORD output_pitch_bytes;
  // Host-only widening of the source's DWORD pointer representation.
  std::uintptr_t current_output_pointer_bits;
  DWORD butterfly_fp_scratch_words[4];
  DWORD constant_words[5];
  int intermediate_words[72];
};
MediaDecoderTableStorage movie_media_decoder_tables = {};
MediaDecoderIdctStorage movie_media_idct_storage = {};
#include "movie_decoder_idct.h"
''' + owners + r'''
int main() {
  const float scale_a = 0.5411961f;
  const float scale_b = 1.306563f;
  const float scale_cross = 0.7071068f;
  const float bias = 0.25f;
  movie_media_idct_storage.constant_words[0] = 0x5a82799a;
  std::memcpy(&movie_media_idct_storage.constant_words[1], &scale_a, sizeof(scale_a));
  std::memcpy(&movie_media_idct_storage.constant_words[2], &scale_b, sizeof(scale_b));
  std::memcpy(&movie_media_idct_storage.constant_words[3], &scale_cross, sizeof(scale_cross));
  std::memcpy(&movie_media_idct_storage.constant_words[4], &bias, sizeof(bias));
  for (int i = 0; i != 64; ++i)
    movie_media_decoder_tables.decoded_coefficient_words[i] = i * 3 + 1;
  BYTE output[16 * 9 * sizeof(int)];
  std::memset(output, 0xa5, sizeof(output));
  sub_5cef8a(output, 16);
  assert(movie_media_idct_storage.output_pitch_bytes == 64);
  BYTE *final_pointer = 0;
  std::memcpy(&final_pointer,
      &movie_media_idct_storage.current_output_pointer_bits, sizeof(final_pointer));
  assert(final_pointer == output + 7 * 64);
  for (int row = 0; row != 8; ++row) {
    bool changed = false;
    for (int column = 0; column != 8; ++column)
      changed = changed || reinterpret_cast<int *>(output)[row * 16 + column] !=
                            static_cast<int>(0xa5a5a5a5u);
    assert(changed);
    for (int column = 8; column != 16; ++column)
      assert(reinterpret_cast<int *>(output)[row * 16 + column] ==
             static_cast<int>(0xa5a5a5a5u));
  }
  assert(movie_media_idct_storage.butterfly_fp_scratch_words[1] != 0 ||
         movie_media_idct_storage.butterfly_fp_scratch_words[3] != 0);
}
'''
    unit = tmp_path / "actual_mapped_idct.cpp"
    unit.write_text(source)
    binary = tmp_path / "actual_mapped_idct"
    subprocess.run(
        [compiler, "-std=c++17", "-Wall", "-Wextra", "-Werror",
         "-pedantic", "-iquote", str(SRC), str(SRC / "movie_decoder_idct.cpp"),
         str(unit), "-o", str(binary)],
        check=True,
    )
    subprocess.run([str(binary)], check=True)
