#include "movie_decoder_huffman.h"

#include <cassert>
#include <cstdint>
#include <cstring>

#ifndef __fastcall
#define __fastcall
#endif

using namespace movie_decoder_huffman;

extern "C" void __fastcall sub_5d2d20(char *src, int mode, int stride,
                                      char *dst, char adjust);
void __fastcall sub_5d2e00(char *src, int byte_offset, int stride,
                           char *dst, char adjust);

namespace {
int idct_calls;
int last_stride_arg;
std::uint8_t *last_idct_target;

const std::uint8_t *active_byte_remap;

void pack_rows(const std::uint8_t *component_rows,
               const std::uint8_t *chroma_rows,
               std::uint8_t *destination, int row_pitch_bytes) {
  transform_sixteen(component_rows, chroma_rows,
                    reinterpret_cast<std::uint32_t *>(destination),
                    row_pitch_bytes, active_byte_remap);
}

void idct(std::uint8_t *target, int stride_arg) {
  ++idct_calls;
  last_idct_target = target;
  last_stride_arg = stride_arg;
}

void test_refill_word() {
  const std::uint8_t input[] = {0x34, 0x12, 0xaa};
  BitState bits{input, 0x01020304u, 8};
  refill_word(bits);
  assert(bits.cursor == input + 2);
  assert(bits.available_bits == 24);
  assert(bits.accumulator == (0x01020304u | (0x1234u << 8)));
}

void test_block_header_paths() {
  std::uint32_t table[64];
  for (std::uint32_t &entry : table) entry = (64u << 22) | 1u;
  const std::uint8_t stream[] = {0x12, 0x34, 0x56, 0x78};
  std::uint8_t coefficient_plane[8192] = {};

  BitState disabled{stream, 0x80000000u, 32};
  BlockHeader no_header = decode_block_header(disabled, false, table,
                                               coefficient_plane, 12);
  assert(no_header.flags == 0 && no_header.delta_mode == 0);
  assert(no_header.coefficient_cursor == coefficient_plane);
  assert(disabled.accumulator == 0x80000000u && disabled.available_bits == 32);

  BitState no_flags{stream, 0x00123456u, 32};
  BlockHeader empty = decode_block_header(no_flags, true, table,
                                           coefficient_plane, 12);
  assert(empty.flags == 0 && empty.coefficient_cursor == coefficient_plane);
  assert(no_flags.accumulator == (0x00123456u << 2));
  assert(no_flags.available_bits == 30);

  BitState explicit_flags{stream, 0x40000000u, 32};
  BlockHeader explicit_header = decode_block_header(
      explicit_flags, true, table, coefficient_plane, 12);
  assert(explicit_header.flags == 0x40);
  assert(explicit_header.delta_mode == 0);
  assert(explicit_header.coefficient_cursor == coefficient_plane +
         (64 * 12 + 32) * 4);
  const std::uint8_t *const mutant_cursor_ignoring_first_symbol =
      coefficient_plane + (64 * 12) * 4;
  assert(explicit_header.coefficient_cursor !=
         mutant_cursor_ignoring_first_symbol);

  BitState escaped_flags{stream, 0xc0000000u, 32};
  BlockHeader escaped = decode_block_header(escaped_flags, true, table,
                                             coefficient_plane, 12);
  assert(escaped.flags == 0x3ff);
  assert(escaped_flags.accumulator == 0);
  assert(escaped.coefficient_cursor == coefficient_plane +
         (64 * 12 + 32) * 4);
}

void test_terminal_huffman_entry() {
  std::uint32_t primary[512] = {};
  std::uint32_t secondary_a[256] = {};
  std::uint32_t secondary_b[256] = {};
  std::int32_t offsets[64] = {};
  std::int32_t scales[64] = {};
  std::int32_t coefficients[63];
  const std::uint8_t stream[] = {0x55, 0xaa};
  primary[0] = 0x30;  // high tag class takes the terminal branch
  Tables tables{primary, secondary_a, secondary_b, offsets, scales};
  State state{{stream, 0, 32}, 0, 7, coefficients, &tables};

  const int result = decode_coefficients(state);
  assert(result == 1);
  assert(state.fill_value == 0);
  assert(state.bits.cursor == stream);
  assert(state.bits.accumulator == 0);
  assert(state.bits.available_bits == 22);
  for (int i = 0; i != 63; ++i) assert(coefficients[i] == 0);
}

void test_value_then_terminal_entry() {
  std::uint32_t primary[512] = {};
  std::uint32_t secondary_a[256] = {};
  std::uint32_t secondary_b[256] = {};
  std::int32_t offsets[64] = {};
  std::int32_t scales[64] = {};
  std::int32_t coefficients[63] = {};
  const std::uint8_t stream[] = {0, 0};
  primary[128] = 0x00410001u;  // value 1, next coefficient slot 2, one bit
  primary[256] = 0x30;         // terminal class consumes the final two bits
  offsets[2] = 4;              // slot offsets are byte offsets
  scales[1] = 5;
  Tables tables{primary, secondary_a, secondary_b, offsets, scales};
  State state{{stream, 0x00400000u, 32}, 0, 9, coefficients, &tables};

  const int result = decode_coefficients(state);
  assert(result == 3);
  assert(coefficients[1] == 5);
  assert(state.fill_value == 0);
  assert(state.bits.accumulator == 0);
  assert(state.bits.available_bits == 21);
  assert(state.bits.cursor == stream);
}

void test_six_block_native_dispatch_and_fill() {
  idct_calls = 0;
  std::uint8_t stream[16] = {};
  BitState bits{stream, 0, 32};
  std::int32_t coeffs[63] = {};
  std::uint32_t primary[512] = {};
  std::uint32_t secondary_a[256] = {};
  std::uint32_t secondary_b[256] = {};
  std::int32_t offsets[64] = {};
  std::int32_t scales[64] = {};
  primary[0] = 0x30;
  Tables tables{primary, secondary_a, secondary_b, offsets, scales};
  State coeff_state{bits, 0, 1, coeffs, &tables};
  std::uint8_t coefficient_plane[8192] = {};
  std::uint8_t luma[0x400] = {};
  std::uint8_t chroma[0x200] = {};
  std::uint8_t remap[256];
  std::uint32_t output[16 * 8] = {};
  for (int i = 0; i != 256; ++i) remap[i] = static_cast<std::uint8_t>(255 - i);

  SixBlockArgs args{coefficient_plane, reinterpret_cast<std::uint8_t *>(output),
                    16, false, {luma, chroma}, &bits, primary, &coeff_state,
                    idct, remap, pack_rows};
  active_byte_remap = remap;
  decode_six_blocks(args);
  assert(idct_calls == 0);
  const std::int32_t filled = 0;
  for (int block = 0; block != 4; ++block) {
    const int offset = block == 0 ? 0 : block == 1 ? 0x20 :
                       block == 2 ? 0x200 : 0x220;
    for (int row = 0; row != 8; ++row) {
      for (int col = 0; col != 8; ++col) {
        std::int32_t actual;
        std::memcpy(&actual, luma + offset + row * 0x40 + col * 4, 4);
        assert(actual == filled);
      }
    }
  }
  for (int block = 0; block != 2; ++block) {
    const int offset = block * 0x100;
    for (int row = 0; row != 8; ++row) {
      for (int col = 0; col != 8; ++col) {
        std::int32_t actual;
        std::memcpy(&actual, chroma + offset + row * 0x20 + col * 4, 4);
        assert(actual == filled);
      }
    }
  }
}

void test_six_block_idct_route() {
  std::uint8_t stream[16] = {};
  BitState bits{stream, 0x00400000u, 32};
  std::int32_t coeffs[63] = {};
  std::uint32_t primary[512] = {};
  std::uint32_t secondary_a[256] = {};
  std::uint32_t secondary_b[256] = {};
  std::int32_t offsets[64] = {};
  std::int32_t scales[64] = {};
  primary[128] = 0x00410001u;
  primary[256] = 0x30;
  primary[0] = 0x30;
  offsets[2] = 4;
  scales[1] = 5;
  Tables tables{primary, secondary_a, secondary_b, offsets, scales};
  State coeff_state{bits, 0, 9, coeffs, &tables};
  std::uint8_t coefficient_plane[128] = {};
  std::uint8_t luma[0x400] = {};
  std::uint8_t chroma[0x200] = {};
  std::uint8_t remap[256] = {};
  std::uint32_t output[16 * 8] = {};
  idct_calls = 0;
  last_idct_target = nullptr;
  SixBlockArgs args{coefficient_plane, reinterpret_cast<std::uint8_t *>(output),
                    16, false, {luma, chroma}, &bits, primary, &coeff_state,
                    idct, remap, pack_rows};
  active_byte_remap = remap;
  decode_six_blocks(args);
  assert(idct_calls == 1);
  assert(last_idct_target == luma);
  assert(last_stride_arg == 0x10);
}

void test_six_flagged_delta_dispatch() {
  idct_calls = 0;
  const int half_width = 16;
  const std::uint32_t code = (65u << 22) | 1u;
  std::uint32_t block_table[64];
  for (std::uint32_t &entry : block_table) entry = code;
  std::uint8_t stream[16] = {};
  BitState bits{stream, 0xc0000000u, 32};
  std::uint8_t coefficient_plane[8192];
  for (std::uint8_t &value : coefficient_plane) value = 10;
  std::int32_t coeffs[63] = {};
  std::uint32_t primary[512] = {};
  std::uint32_t secondary_a[256] = {};
  std::uint32_t secondary_b[256] = {};
  std::int32_t offsets[64] = {};
  std::int32_t scales[64] = {};
  primary[0] = 0x30;
  Tables tables{primary, secondary_a, secondary_b, offsets, scales};
  State coeff_state{bits, 0, 1, coeffs, &tables};
  std::uint8_t luma[0x400] = {};
  std::uint8_t chroma[0x200] = {};
  std::uint8_t remap[256];
  std::uint32_t output[16 * 8] = {};
  for (int i = 0; i != 256; ++i) remap[i] = static_cast<std::uint8_t>(i);
  SixBlockArgs args{coefficient_plane, reinterpret_cast<std::uint8_t *>(output),
                    half_width, true, {luma, chroma}, &bits, block_table,
                    &coeff_state, idct, remap, pack_rows};
  active_byte_remap = remap;
  decode_six_blocks(args);
  assert(idct_calls == 0);
  for (int block = 0; block != 4; ++block) {
    const int offset = block == 0 ? 0 : block == 1 ? 0x20 :
                       block == 2 ? 0x200 : 0x220;
    for (int row = 0; row != 8; ++row)
      for (int col = 0; col != 8; ++col)
        assert(luma[offset + row * 0x40 + 2 + col * 4] == 11);
  }
  for (int block = 0; block != 2; ++block) {
    const int offset = block * 0x100;
    for (int row = 0; row != 8; ++row)
      for (int col = 0; col != 8; ++col)
        assert(chroma[offset + row * 0x20 + 2 + col * 4] == 11);
  }
}

void test_sixteen_tile_progression_and_pack() {
  std::uint8_t source_a[0x40 * 16 + 64] = {};
  std::uint8_t source_b[0x20 * 8 + 0x120] = {};
  std::uint8_t remap[256];
  std::uint32_t output[16 * 10 + 8] = {};
  for (int i = 0; i != 256; ++i) remap[i] = static_cast<std::uint8_t>(i);
  for (int tile = 0; tile != 16; ++tile) {
    const int base_a = tile * 0x40;
    const int base_b = (tile / 2) * 0x20;
    for (int col = 0; col != 8; ++col) {
      source_a[base_a + 2 + col * 8] = static_cast<std::uint8_t>(tile + col);
      source_a[base_a + 6 + col * 8] = static_cast<std::uint8_t>(tile + col + 1);
      source_b[base_b + 2 + col * 4] = static_cast<std::uint8_t>(tile + col + 2);
      source_b[base_b + 0x102 + col * 4] = static_cast<std::uint8_t>(tile + col + 3);
    }
  }
  const int stride = 10 * static_cast<int>(sizeof(std::uint32_t));
  transform_sixteen(source_a, source_b, output, stride, remap);
  for (int tile = 0; tile != 16; ++tile) {
    const int base_a = tile * 0x40;
    const int base_b = (tile / 2) * 0x20;
    for (int col = 0; col != 8; ++col) {
      const std::uint32_t high =
          (static_cast<std::uint32_t>(source_a[base_a + 2 + col * 8]) << 8) |
          source_a[base_a + 6 + col * 8];
      const std::uint32_t low =
          (static_cast<std::uint32_t>(source_b[base_b + 2 + col * 4]) << 8) |
          source_b[base_b + 0x102 + col * 4];
      assert(output[tile * 10 + col] == ((high << 16) | low));
    }
    assert(output[tile * 10 + 8] == 0);
    assert(output[tile * 10 + 9] == 0);
  }
}

void test_delta_block_shapes_and_byte_wrap() {
  std::uint8_t source[8 * 64 + 64] = {};
  std::uint8_t block4[8 * 64] = {};
  std::uint8_t block2[8 * 32] = {};
  for (int row = 0; row != 8; ++row) {
    for (int column = 0; column != 64; ++column) {
      source[row * 64 + column] = static_cast<std::uint8_t>(row * 17 + column);
    }
  }
  sub_5d2d20(reinterpret_cast<char *>(source), 1, 16,
             reinterpret_cast<char *>(block4), 7);
  static const int mode_nonzero_source[8] = {2, 7, 6, 11, 10, 15, 14, 19};
  for (int row = 0; row != 8; ++row) {
    for (int col = 0; col != 8; ++col) {
      assert(block4[row * 64 + 2 + col * 4] ==
             static_cast<std::uint8_t>(source[row * 64 + mode_nonzero_source[col]] + 7));
    }
  }
  sub_5d2d20(reinterpret_cast<char *>(source), 0, 16,
             reinterpret_cast<char *>(block4), static_cast<char>(0x103));
  static const int mode_zero_source[8] = {3, 2, 7, 6, 11, 10, 15, 14};
  for (int row = 0; row != 8; ++row) {
    for (int col = 0; col != 8; ++col) {
      assert(block4[row * 64 + 2 + col * 4] ==
             static_cast<std::uint8_t>(source[row * 64 + mode_zero_source[col]] + 3));
    }
  }
  sub_5d2e00(reinterpret_cast<char *>(source), 1, 8,
             reinterpret_cast<char *>(block2), 5);
  for (int row = 0; row != 8; ++row) {
    for (int col = 0; col != 8; ++col) {
      assert(block2[row * 32 + 2 + col * 4] ==
             static_cast<std::uint8_t>(source[1 + row * 64 + col * 4] + 5));
  }
  // Mutant: using the 4x pitch (half_width*4) in the chroma helper changes
  // row one because the native 2x helper advances half_width*8 bytes.
  assert(block2[0x20 + 2] !=
         static_cast<std::uint8_t>(source[1 + 8 * 4] + 5));
  }
}
}  // namespace

int main() {
  test_refill_word();
  test_block_header_paths();
  test_terminal_huffman_entry();
  test_value_then_terminal_entry();
  test_six_block_native_dispatch_and_fill();
  test_six_block_idct_route();
  test_six_flagged_delta_dispatch();
  test_sixteen_tile_progression_and_pack();
  test_delta_block_shapes_and_byte_wrap();
}
