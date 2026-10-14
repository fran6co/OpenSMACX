from __future__ import annotations

import shutil
import subprocess
from pathlib import Path
import os
import re
import pytest


ROOT = Path(__file__).resolve().parent
SRC = ROOT.parents[1] / "src"
PINNED_SHA256 = "01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf"


def test_pinned_outer_wrappers_capture_and_publish_bitstream_state_once() -> None:
    from capstone import Cs, CS_ARCH_X86, CS_MODE_32
    from capstone.x86 import (
        X86_OP_IMM, X86_OP_MEM, X86_OP_REG,
        X86_REG_EBP, X86_REG_EDI, X86_REG_ESI, X86_REG_ESP,
    )
    from match_recovery.model import Span
    from match_recovery.pe_coff import PeImage

    image_path = Path(os.environ.get(
        "OPENSMACX_IMAGE", str(SRC.parent / ".opensmacx/game/terranx_original.exe")
    ))
    if not image_path.is_file():
        import pytest
        pytest.skip("pinned original unavailable")
    image = PeImage(image_path.read_bytes(), PINNED_SHA256)
    decoder = Cs(CS_ARCH_X86, CS_MODE_32)
    decoder.detail = True
    wrapper_specs = (
        (0x5D1278, 0x5D1466, 0x5D128B, 0x5D1291, 0x5D1297, 0x5D129C),
        (0x5D1466, 0x5D1654, 0x5D1479, 0x5D147F, 0x5D1485, 0x5D148A),
        (0x5D1654, 0x5D1842, 0x5D1667, 0x5D166D, 0x5D1673, 0x5D1678),
        (0x5D1842, 0x5D1A30, 0x5D1855, 0x5D185B, 0x5D1861, 0x5D1866),
        (0x5D1A30, 0x5D1B88, 0x5D1A3A, 0x5D1A40, 0x5D1A46, 0x5D1A4B),
    )

    def decoded(start: int, end: int):
        instructions = list(decoder.disasm(image.read(Span(start, end)), start))
        assert sum(item.size for item in instructions) == end - start
        return {item.address: item for item in instructions}

    def reg_operand(insn, index: int, register: int) -> bool:
        operand = insn.operands[index]
        return operand.type == X86_OP_REG and operand.reg == register

    def mem_operand(insn, index: int, base: int, disp: int) -> bool:
        operand = insn.operands[index]
        return (operand.type == X86_OP_MEM and operand.mem.base == base
                and operand.mem.disp == disp and operand.mem.index == 0)

    for start, end, input_at, cursor_at, call_at, store_at in wrapper_specs:
        instructions = decoded(start, end)
        input_load = instructions[input_at]
        cursor_load = instructions[cursor_at]
        decode_call = instructions[call_at]
        cursor_store = instructions[store_at]
        assert input_load.mnemonic == "mov" and reg_operand(input_load, 0, X86_REG_EBP)
        assert mem_operand(input_load, 1, X86_REG_ESI, 0x200)
        assert cursor_load.mnemonic == "mov" and reg_operand(cursor_load, 0, X86_REG_EDI)
        assert mem_operand(cursor_load, 1, X86_REG_ESI, 0x204)
        assert input_load.address < cursor_load.address < decode_call.address
        assert decode_call.mnemonic == "call" and decode_call.operands[0].type == X86_OP_IMM
        assert decode_call.operands[0].imm == 0x5D0AAB
        assert cursor_store.mnemonic == "mov" and mem_operand(cursor_store, 0, X86_REG_ESI, 0x204)
        assert reg_operand(cursor_store, 1, X86_REG_EDI)
        assert decode_call.address < cursor_store.address
        cursor_stores = [
            item for item in instructions.values()
            if item.mnemonic == "mov" and item.operands
            and item.operands[0].type == X86_OP_MEM
            and item.operands[0].mem.base == X86_REG_ESI
            and item.operands[0].mem.disp == 0x204
        ]
        assert cursor_stores == [cursor_store]
        assert all(
            item.address > cursor_store.address
            for item in instructions.values()
            if item.mnemonic == "call" and item.address != decode_call.address
        )

    # The outlined bitstream body consumes EBP/EDI register snapshots. It
    # updates predictor members per block but does not publish either captured
    # input or cursor through context+0x200/+0x204.
    for start, end in ((0x5D0AAB, 0x5D0D82), (0x5D0D82, 0x5D1278)):
        instructions = decoded(start, end)
        for item in instructions.values():
            for operand in item.operands:
                if operand.type != X86_OP_MEM or operand.mem.base != X86_REG_ESI:
                    continue
                assert operand.mem.disp != 0x200
                assert operand.mem.disp != 0x204
        if start == 0x5D0D82:
            # Normalize the three saved registers before assigning local offsets.
            saves = [instructions[a] for a in (0x5D1049, 0x5D104A, 0x5D104B)]
            assert [i.mnemonic for i in saves] == ["push"] * 3
            assert [i.operands[0].reg for i in saves] == [
                X86_REG_ESI, X86_REG_EBP, X86_REG_EDI]
            for address, register, displacement in (
                (0x5D104C, X86_REG_ESI, 0x0C),
                (0x5D1050, X86_REG_EDI, 0x10C),
                (0x5D10DF, X86_REG_ESI, 0x10C),
                (0x5D10F4, X86_REG_ESI, 0x130),
                (0x5D1172, X86_REG_ESI, 0x208),
            ):
                insn = instructions[address]
                assert insn.mnemonic == "lea"
                assert reg_operand(insn, 0, register)
                assert mem_operand(insn, 1, X86_REG_ESP, displacement)


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


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
def test_actual_six_step_predictor_and_dct_bodies(
        tmp_path: Path, compiler_name: str) -> None:
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(f"{compiler_name} unavailable")
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
        "static int movie_media_signed32(",
        "DWORD __cdecl sub_5d0d82(",
        "DWORD __cdecl sub_5d0aab(",
        "void movie_media_decode_six_coefficient_blocks(",
        "void __cdecl sub_5d1278(",
        "void __cdecl sub_5d1466(",
        "void __cdecl sub_5d1654(",
        "void __cdecl sub_5d1842(",
        "void __cdecl sub_5d1a30(",
    )
    extracted = [_function(source_text, signature) for signature in signatures]
    block_index = signatures.index("DWORD __cdecl sub_5d0d82(")
    storage_decl = "  int local_storage_words[0x300 / sizeof(int)];"
    assert extracted[block_index].count(storage_decl) == 1
    extracted[block_index] = extracted[block_index].replace(
        storage_decl, storage_decl + "\n  observed_local_storage = "
        "reinterpret_cast<std::uintptr_t>(local_storage_words);\n"
        "  static_assert(sizeof(local_storage_words) == 0x300, \"scratch extent\");\n"
        "  static_assert(std::is_same<decltype(local_storage_words), int[192]>::value, "
        "\"signed scratch element type\");", 1)
    reader = extracted[0]
    read_needle = "static DWORD movie_media_read_bits32(const BYTE *input, DWORD bit_position) {"
    assert reader.count(read_needle) == 1
    extracted[0] = reader.replace(
        read_needle,
        read_needle + "\n  if (!first_read_input) first_read_input = input;\n"
        "  else if (first_read_input != input) ++different_read_inputs;",
        1,
    )
    bodies = "\n\n".join(extracted)
    unit_source = r'''#include <cassert>
#include <cstdint>
#include <cstring>
#include <type_traits>
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
DWORD __cdecl sub_5d0d82(MediaDecoderContext *, DWORD, const BYTE *, BYTE *, int, DWORD);
static const BYTE *first_read_input;
static int different_read_inputs;
static MediaDecoderContext *active_context;
static const BYTE *replacement_input;
static bool mutate_input_on_transform;
static bool did_mutate_context_input;
static bool throw_on_transform;
static bool observe_converter_commit;
static DWORD expected_converter_cursor;
static int row_calls;
static int column_calls;
static int converter_calls;
static int first_row_value;
static int first_row_coefficient_three;
static std::uintptr_t observed_local_storage;
static int transform_pointer_mismatches;
extern "C" void __cdecl sub_5d29e1(int *source, int *destination) {
  const int row = row_calls % 8;
  const std::uintptr_t source_offset =
      reinterpret_cast<std::uintptr_t>(source) - observed_local_storage;
  const std::uintptr_t destination_offset =
      reinterpret_cast<std::uintptr_t>(destination) - observed_local_storage;
  if (source_offset != static_cast<std::uintptr_t>(row * 8 * 4)
      || destination_offset != static_cast<std::uintptr_t>((64 + row) * 4))
    ++transform_pointer_mismatches;
  if (row_calls == 0) {
    first_row_value = source[0];
    first_row_coefficient_three = source[3];
  }
  if (mutate_input_on_transform && row_calls == 0) {
    active_context->compressed_input_200 = replacement_input;
    did_mutate_context_input = true;
  }
  if (throw_on_transform) throw 7;
  ++row_calls;
  for (int i = 0; i != 8; ++i) destination[i * 9] = source[0] + i;
}
extern "C" void __cdecl sub_5d2c0a(int *destination, int *source) {
  const int column = column_calls % 8;
  const std::uintptr_t source_offset =
      reinterpret_cast<std::uintptr_t>(source) - observed_local_storage;
  if (source_offset != static_cast<std::uintptr_t>((64 + column * 9) * 4))
    ++transform_pointer_mismatches;
  ++column_calls;
  for (int i = 0; i != 8; ++i) destination[i] = source[0] + i;
}
static void observe_conversion() {
  if (observe_converter_commit) {
    assert(active_context != nullptr);
    assert(active_context->bit_position_204 == expected_converter_cursor);
  }
  ++converter_calls;
}
void __cdecl sub_5d2037(int *, int *, unsigned int *) { observe_conversion(); }
void __cdecl sub_5d2536(int *, int *, int *) { observe_conversion(); }
void __cdecl sub_5d1b88(unsigned int *, int *, int *) { observe_conversion(); }
void __cdecl OutputPixelRow16(short *, unsigned int *, unsigned int *) {
  observe_conversion();
}
void __cdecl OutputPixelRow(int *, unsigned int *, unsigned int *) {
  observe_conversion();
}
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
  assert(movie_media_signed32(0x00000000u) == 0);
  assert(movie_media_signed32(0x7fffffffu) == INT_MAX);
  assert(movie_media_signed32(0x80000000u) == INT_MIN);
  assert(movie_media_signed32(0xffffffffu) == -1);
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
  const DWORD completed_position = sub_5d0aab(
      &runtime, scratch, runtime.compressed_input_200, runtime.bit_position_204);
  assert(runtime.predictor_4f8 == 5);
  assert(runtime.predictor_4fc == 4);
  assert(runtime.predictor_500 == 6);
  assert(completed_position == 18);
  assert(runtime.bit_position_204 == 0);
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
  ac_runtime.bit_position_204 = 77;
  const DWORD block_position = sub_5d0d82(
      &ac_runtime, 0, ac_runtime.compressed_input_200, output, 64,
      0);
  assert(block_position == 1);
  if (transform_pointer_mismatches != 0) {
    std::fprintf(stderr, "FAIL: transform scratch pointer offset mismatch\n");
    return 97;
  }
  assert(ac_runtime.bit_position_204 == 77);
  assert(row_calls == 8 && column_calls == 8);
  assert(first_row_value == 0);
  assert(first_row_coefficient_three == -0x20000);

  MediaDecoderContext wrapper_runtime = {};
  wrapper_runtime.compressed_input_200 = zero_input;
  wrapper_runtime.block_transform_weights[0] = 0x20000;
  wrapper_runtime.predictor_4f8 = 2;
  BYTE wrapper_output[16 * 64] = {};
  sub_5d1466(wrapper_output, 64, &wrapper_runtime);
  assert(wrapper_runtime.bit_position_204 == 18);
  assert(wrapper_runtime.predictor_4f8 == static_cast<DWORD>(-2));
  assert(converter_calls == 16);

  // Every native outer wrapper publishes the completed cursor once before
  // its first pixel/conversion call.
  using Wrapper = void (__cdecl *)(BYTE *, int, MediaDecoderContext *);
  const Wrapper wrappers[5] = {
      sub_5d1278, sub_5d1466, sub_5d1654, sub_5d1842, sub_5d1a30};
  for (Wrapper wrapper : wrappers) {
    MediaDecoderContext state = {};
    state.compressed_input_200 = zero_input;
    state.block_transform_weights[0] = 0x20000;
    BYTE destination[16 * 64] = {};
    converter_calls = 0;
    active_context = &state;
    expected_converter_cursor = 18;
    observe_converter_commit = true;
    wrapper(destination, 64, &state);
    observe_converter_commit = false;
    assert(state.bit_position_204 == 18);
    assert(converter_calls == 16);
  }

  // The public convenience wrapper follows the same snapshot-and-single-commit
  // contract even though it has no output-conversion phase.
  BYTE block_input[32] = {};
  BYTE other_input[32] = {};
  set_msb(block_input, 3, 0x3f, 17);
  set_msb(block_input, 20, 0, 1);
  movie_media_coefficient_tables.words[228] = 0x00000101u;
  movie_media_coefficient_tables.words[212] = 0x00004100u;
  movie_media_coefficient_tables.words[68] = 3;
  MediaDecoderContext expected_convenience = {};
  expected_convenience.block_transform_weights[0] = 0x20000;
  expected_convenience.block_transform_weights[3] = 0x20000;
  DWORD expected_convenience_scratch[512] = {};
  active_context = &expected_convenience;
  mutate_input_on_transform = false;
  const DWORD expected_convenience_position = sub_5d0aab(
      &expected_convenience, expected_convenience_scratch, block_input, 0);
  MediaDecoderContext convenience = {};
  convenience.compressed_input_200 = block_input;
  convenience.block_transform_weights[0] = 0x20000;
  convenience.block_transform_weights[3] = 0x20000;
  DWORD convenience_scratch[512] = {};
  active_context = &convenience;
  row_calls = 0;
  column_calls = 0;
  first_read_input = nullptr;
  different_read_inputs = 0;
  mutate_input_on_transform = true;
  did_mutate_context_input = false;
  replacement_input = other_input;
  movie_media_decode_six_coefficient_blocks(&convenience, convenience_scratch);
  assert(did_mutate_context_input);
  assert(convenience.compressed_input_200 == other_input);
  assert(different_read_inputs == 0);
  assert(convenience.bit_position_204 == expected_convenience_position);

  MediaDecoderContext convenience_interrupted = {};
  convenience_interrupted.compressed_input_200 = block_input;
  convenience_interrupted.block_transform_weights[0] = 0x20000;
  convenience_interrupted.block_transform_weights[3] = 0x20000;
  active_context = &convenience_interrupted;
  row_calls = 0;
  column_calls = 0;
  mutate_input_on_transform = false;
  throw_on_transform = true;
  bool convenience_threw = false;
  try {
    movie_media_decode_six_coefficient_blocks(
        &convenience_interrupted, convenience_scratch);
  } catch (int value) { convenience_threw = value == 7; }
  throw_on_transform = false;
  assert(convenience_threw);
  assert(convenience_interrupted.bit_position_204 == 0);
  assert(convenience_interrupted.predictor_4f8 != 0);

  // A transform callback can alter the context's input pointer after block
  // one. The native outer wrapper captured EBP once, so all later reads must
  // still use the original input pointer. Compare against the same stream
  // without mutation for every wrapper.
  set_msb(block_input, 3, 0x3f, 17);
  set_msb(block_input, 20, 0, 1);
  for (int i = 0; i != 325; ++i) movie_media_coefficient_tables.words[i] = 0;
  movie_media_coefficient_tables.words[228] = 0x00000101u;
  movie_media_coefficient_tables.words[212] = 0x00004100u;
  movie_media_coefficient_tables.words[68] = 3;
  reinterpret_cast<BYTE *>(movie_media_coefficient_tables.words)[0x394] = 0x12;
  reinterpret_cast<BYTE *>(movie_media_coefficient_tables.words)[0x414] = 0x12;
  for (Wrapper wrapper : wrappers) {
    MediaDecoderContext baseline = {};
    baseline.compressed_input_200 = block_input;
    baseline.block_transform_weights[0] = 0x20000;
    baseline.block_transform_weights[3] = 0x20000;
    BYTE baseline_output[16 * 64] = {};
    active_context = &baseline;
    row_calls = 0;
    column_calls = 0;
    first_read_input = nullptr;
    different_read_inputs = 0;
    mutate_input_on_transform = false;
    throw_on_transform = false;
    wrapper(baseline_output, 64, &baseline);
    const DWORD baseline_position = baseline.bit_position_204;

    MediaDecoderContext changed = {};
    changed.compressed_input_200 = block_input;
    changed.block_transform_weights[0] = 0x20000;
    changed.block_transform_weights[3] = 0x20000;
    BYTE changed_output[16 * 64] = {};
    active_context = &changed;
    row_calls = 0;
    column_calls = 0;
    first_read_input = nullptr;
    different_read_inputs = 0;
    replacement_input = other_input;
    mutate_input_on_transform = true;
    did_mutate_context_input = false;
    wrapper(changed_output, 64, &changed);
    assert(did_mutate_context_input);
    assert(changed.compressed_input_200 == other_input);
    assert(different_read_inputs == 0);
    assert(changed.bit_position_204 == baseline_position);
  }

  // Throwing from a transform after a predictor update leaves the caller's
  // cursor uncommitted, exactly as the native wrapper's post-decode store.
  for (Wrapper wrapper : wrappers) {
    MediaDecoderContext interrupted = {};
    interrupted.compressed_input_200 = block_input;
    interrupted.block_transform_weights[0] = 0x20000;
    interrupted.block_transform_weights[3] = 0x20000;
    BYTE interrupted_output[16 * 64] = {};
    active_context = &interrupted;
    row_calls = 0;
    column_calls = 0;
    first_read_input = nullptr;
    different_read_inputs = 0;
    mutate_input_on_transform = false;
    throw_on_transform = true;
    bool threw = false;
    try { wrapper(interrupted_output, 64, &interrupted); }
    catch (int value) { threw = value == 7; }
    throw_on_transform = false;
    assert(threw);
    assert(interrupted.bit_position_204 == 0);
    assert(interrupted.predictor_4f8 != 0);
  }
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
    baseline = subprocess.run([str(binary)], capture_output=True, text=True)
    assert baseline.returncode == 0, baseline.stderr

    # Restoring the old unsigned storage must fail the actual-array type
    # control, not merely encounter an unrelated compiler error.
    unsigned_unit = tmp_path / "movie_decoder_unsigned_workspace.cpp"
    assert unit_source.count(storage_decl) == 1
    unsigned_unit.write_text(unit_source.replace(
        storage_decl, "  DWORD local_storage_words[0x300 / sizeof(DWORD)];", 1))
    unsigned_result = subprocess.run(
        [compiler, "-std=c++17", "-fsyntax-only", str(unsigned_unit)],
        capture_output=True, text=True)
    assert unsigned_result.returncode != 0
    assert re.search(r"static assertion failed[^\n]*signed scratch element type",
                     unsigned_result.stderr), unsigned_result.stderr

    # Compile-success mutants remain inside the scratch; require the intended
    # diagnostic rather than accepting an unrelated crash as proof.
    for index, (correct, shifted) in enumerate((
        ("local_storage_words + row * 8", "local_storage_words + 3 + row * 8"),
        ("local_storage_words + 64 + row", "local_storage_words + 67 + row"),
        ("local_storage_words + 64 + column * 9",
         "local_storage_words + 67 + column * 9"),
    )):
        assert unit_source.count(correct) == 1
        mutant_unit = tmp_path / f"movie_decoder_pointer_mutant_{index}.cpp"
        mutant_unit.write_text(unit_source.replace(correct, shifted, 1))
        mutant_binary = tmp_path / f"movie_decoder_pointer_mutant_{index}"
        subprocess.run(
            [compiler, "-std=c++17", "-Wall", "-Wextra", "-Werror",
             "-pedantic", str(mutant_unit), "-o", str(mutant_binary)], check=True)
        rejected = subprocess.run([str(mutant_binary)], capture_output=True, text=True)
        assert rejected.returncode == 97
        assert rejected.stderr == "FAIL: transform scratch pointer offset mismatch\n"

    mutants = (
        ("words[6 + run_position]", "words[7 + run_position]"),
        ("block < 4 ? 0x40 : 0x20", "block < 4 ? 0x20 : 0x20"),
        ("{1, 9, 129, 137, 257, 321}", "{2, 9, 129, 137, 257, 321}"),
        ("197 + (code >> 1)", "198 + (code >> 1)"),
        ("  if (run_position == 0x3f) {",
         "  context->bit_position_204 = bit_position;\n"
         "  if (run_position == 0x3f) {"),
        ("context, *predictor, input,",
         "context, *predictor, context->compressed_input_200,"),
        ("context->bit_position_204 = bit_position;\n"
         "  for (int row = 0; row != 16; ++row) {",
         "static_cast<void>(bit_position);\n"
         "  for (int row = 0; row != 16; ++row) {"),
    )
    for index, (before, after) in enumerate(mutants):
        assert unit_source.count(before) >= 1
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
