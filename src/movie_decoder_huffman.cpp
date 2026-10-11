#include "movie_decoder_huffman.h"

#include <cstring>

// Existing mapped product helpers. Their real bodies live in general.cpp;
// this owner must not fork the pixel-lane decoding arithmetic.
extern "C" void __fastcall sub_5d2d20(char *src, int mode, int stride,
                                      char *dst, char adjust);
void __fastcall sub_5d2e00(char *src, int byte_offset, int stride,
                           char *dst, char adjust);

namespace movie_decoder_huffman {
namespace {

uint32_t pull_word(BitState &state) {
  const uint32_t value = static_cast<uint32_t>(state.cursor[0]) |
                              (static_cast<uint32_t>(state.cursor[1]) << 8);
  state.cursor += 2;
  return value;
}

void refill_if_short(BitState &state) {
  if (state.available_bits < 0x10) {
    const int8_t old_count = static_cast<int8_t>(state.available_bits);
    state.accumulator |= pull_word(state) << ((0x10 - old_count) & 0x1f);
    state.available_bits += 0x10;
  }
}

void fill_block(uint8_t *base, int row_bytes, int fill) {
  const int32_t value = fill;
  for (int row = 0; row != 8; ++row) {
    for (int x = 0; x != 8; ++x) {
      std::memcpy(base + row * row_bytes + x * 4, &value, sizeof(value));
    }
  }
}

int32_t arithmetic_shift_right(uint32_t value, unsigned shift) {
  const int32_t signed_value = static_cast<int32_t>(value);
  if (signed_value >= 0) return signed_value >> shift;
  return -1 - static_cast<int32_t>((~value) >> shift);
}

// IMUL retains the low dword, including overflow, in the original decoder.
int32_t low32_product(int32_t left, int32_t right) {
  const uint32_t product = static_cast<uint32_t>(left) *
                           static_cast<uint32_t>(right);
  int32_t result;
  std::memcpy(&result, &product, sizeof(result));
  return result;
}

uint32_t consume_block_code(BitState &bits,
                                 const uint32_t table[64]) {
  const uint32_t entry = table[bits.accumulator >> 26];
  const unsigned count = entry & 0xff;
  bits.accumulator <<= (count & 0x1f);
  bits.available_bits -= static_cast<int32_t>(count);
  refill_if_short(bits);
  return entry;
}

void pack_rows_eight(const uint8_t *source_a,
                     const uint8_t *source_b,
                     uint32_t *destination,
                     const uint8_t *byte_remap) {
  for (int column = 0; column != 8; ++column) {
    const int a_index = 2 + column * 8;
    const int b_index = 2 + column * 4;
    const uint32_t high =
        (static_cast<uint32_t>(byte_remap[source_a[a_index]]) << 8) |
        byte_remap[source_a[a_index + 4]];
    const uint32_t low =
        (static_cast<uint32_t>(byte_remap[source_b[b_index]]) << 8) |
        byte_remap[source_b[b_index + 0x100]];
    destination[column] = (high << 16) | low;
  }
}

}  // namespace

void refill_word(BitState &state) { refill_if_short(state); }

BlockHeader decode_block_header(BitState &state, bool header_enabled,
                                const uint32_t block_huffman[64],
                                uint8_t *coefficient_plane,
                                int half_width) {
  BlockHeader result;
  result.flags = 0;
  result.delta_mode = 0;
  result.coefficient_cursor = coefficient_plane;
  if (!header_enabled) return result;
  uint32_t flags = 0;
  if ((state.accumulator & 0xc0000000u) == 0) {
    state.accumulator <<= 2;
    state.available_bits -= 2;
    refill_word(state);
  } else {
    if ((state.accumulator & 0x80000000u) != 0) {
      flags = 0x3ff;
      state.accumulator <<= 1;
      --state.available_bits;
    } else {
      flags = state.accumulator >> 24;
      state.accumulator <<= 8;
      state.available_bits -= 8;
    }
    refill_word(state);
    const uint32_t first = consume_block_code(state, block_huffman);
    const int32_t first_symbol = arithmetic_shift_right(first, 22);
    const uint32_t second = consume_block_code(state, block_huffman);
    const int32_t second_symbol = arithmetic_shift_right(second, 22);
    const int32_t position =
        second_symbol * half_width + (first_symbol >> 1);
    result.coefficient_cursor += position * 4;
    result.delta_mode = static_cast<uint32_t>(first_symbol) & 1u;
  }
  result.flags = flags;
  return result;
}

// ORIGINAL: 0x005CE85C sub_5ce85c 0x005CE85C-0x005CEA09
// RECOVERY_BINDING: v1 {"evidence": ["recovery/tasks/movie-decoder-owner-oct5.json", "pinned caller/body and shared-storage review; fresh VC6 public symbol", "Keep the semantic C++ name; body state is explicitly passed and uses the shared typed tables/bit-state owners. Do not copy executable bytes, use original-address calls, or treat a provisional owner name as native mangling."], "reason": "Faithful semantic media decoder owner recovery with normalized custom-register helper interfaces. Original identities, spans and historical findings retained; no matching, runtime or game acceptance granted."}
// symbol    ?decode_coefficients@movie_decoder_huffman@@YAHAAUState@1@@Z
// TRIED: MISMATCH #1 push/mov - the bit-reader state (accumulator, bits-available count, word cursor) that the original keeps in esi/edi/ebp for the whole function is modelled as ordinary locals written back once at the shared epilogue and at the single early-return branch; entry-frame shape differs first.
int decode_coefficients(State &state) {
  BitState &bits = state.bits;
  uint32_t accumulator = bits.accumulator << 8;
  int count = bits.available_bits - 8;
  state.fill_value = low32_product(
      arithmetic_shift_right(bits.accumulator, 24), state.scale_factor);
  state.coefficients[0] = state.fill_value;

  if (count < 0x10) {
    const uint16_t word = static_cast<uint16_t>(pull_word(bits));
    accumulator |= static_cast<uint32_t>(word)
                   << ((0x10 - static_cast<int8_t>(count)) & 0x1f);
    count += 0x10;
  }

  std::memset(state.coefficients + 1, 0, 63 * sizeof(*state.coefficients));
  int position = 1;
  const Tables &tables = *state.tables;
  for (;;) {
    uint32_t packed = tables.primary[accumulator >> 0x17];
    const uint8_t tag = static_cast<uint8_t>(packed);
    int new_count = count;
    if (static_cast<int8_t>(tag) > 9) {
      if (tag < 0x20) {
        if (tag < 0x10) {
          accumulator <<= 9;
          new_count = count - 9;
          if (new_count < 0x10) {
            const uint16_t word = static_cast<uint16_t>(pull_word(bits));
            accumulator |= static_cast<uint32_t>(word)
                           << ((0x10 - static_cast<int8_t>(new_count)) & 0x1f);
            new_count = count + 7;
          }
          packed = tables.secondary_a[accumulator >> 0x18];
        } else {
          accumulator <<= 6;
          new_count = count - 6;
          if (new_count < 0x10) {
            const uint16_t word = static_cast<uint16_t>(pull_word(bits));
            accumulator |= static_cast<uint32_t>(word)
                           << ((0x10 - static_cast<int8_t>(new_count)) & 0x1f);
            new_count = count + 10;
          }
          packed = tables.secondary_b[accumulator >> 0x18];
        }
      } else if (tag > 0x2f) {
        accumulator <<= 2;
        int final_count = count - 2;
        if (final_count < 0x10) {
          const uint16_t word = static_cast<uint16_t>(pull_word(bits));
          accumulator |= static_cast<uint32_t>(word)
                         << ((0x10 - static_cast<int8_t>(final_count)) & 0x1f);
          final_count = count + 0xe;
        }
        bits.accumulator = accumulator;
        bits.available_bits = final_count;
        return position;
      } else {
        accumulator <<= 6;
        new_count = count - 6;
        if (new_count < 0x10) {
          const uint16_t word = static_cast<uint16_t>(pull_word(bits));
          accumulator |= static_cast<uint32_t>(word)
                         << ((0x10 - static_cast<int8_t>(new_count)) & 0x1f);
          new_count = count + 10;
        }
        packed = (accumulator & 0xffff0000u) | 0x10u;
      }
    }

    count = new_count - static_cast<int>(packed & 0xff);
    const int next_position = position + static_cast<int>((packed >> 0x10) & 0x3f);
    accumulator <<= packed & 0x1f;
    if (count < 0x10) {
      const uint16_t word = static_cast<uint16_t>(pull_word(bits));
      accumulator |= static_cast<uint32_t>(word)
                     << ((0x10 - static_cast<int8_t>(count)) & 0x1f);
      count += 0x10;
    }
    const int offset = tables.coefficient_offsets[next_position];
    const int value = static_cast<int32_t>(packed) >> 0x16;
    const int scale = *reinterpret_cast<const int *>(
        reinterpret_cast<const uint8_t *>(tables.coefficient_scales) + offset);
    *reinterpret_cast<int *>(reinterpret_cast<uint8_t *>(state.coefficients) + offset) =
        value * scale;
    position = next_position + 1;
  }
}

void decode_six_blocks(SixBlockArgs &args) {
  const int half_width = args.row_width >> 1;
  BlockHeader header = decode_block_header(*args.bits, args.header_enabled,
                                           args.block_huffman,
                                           args.coefficient_plane, half_width);
  const int luma_offsets[4] = {0, 0x20, 0x200, 0x220};
  const int chroma_offsets[2] = {0, 0x100};
  const uint32_t flags[6] = {1, 2, 4, 8, 0x10, 0x20};

  for (int block = 0; block != 6; ++block) {
    const bool chroma = block >= 4;
    uint8_t *const target = chroma
        ? args.scratch.chroma_16x8 + chroma_offsets[block - 4]
        : args.scratch.luma_16x16 + luma_offsets[block];
    if ((header.flags & flags[block]) == 0) {
      args.coefficient_state->bits = *args.bits;
      const int decoded = decode_coefficients(*args.coefficient_state);
      *args.bits = args.coefficient_state->bits;
      if (decoded == 1) {
        fill_block(target, chroma ? 0x20 : 0x40,
                   args.coefficient_state->fill_value);
      } else {
        args.idct_rows(target, chroma ? 8 : 0x10);
      }
      continue;
    }

    const uint32_t code = consume_block_code(*args.bits,
                                                   args.block_huffman);
    const int extra = arithmetic_shift_right(code, 22) - 0x40;
    if (!chroma) {
      const int plane_offset = (block & 2) != 0 ? half_width * 0x20 : 0;
      const int x_offset = (block & 1) != 0 ? 0x10 : 0;
      ::sub_5d2d20(
          reinterpret_cast<char *>(header.coefficient_cursor + plane_offset +
                                   x_offset),
          static_cast<int>(header.delta_mode), half_width,
          reinterpret_cast<char *>(target), static_cast<char>(extra));
    } else {
      ::sub_5d2e00(reinterpret_cast<char *>(header.coefficient_cursor),
                   block == 4 ? 1 : 0, half_width,
                   reinterpret_cast<char *>(target),
                   static_cast<char>(extra));
    }
  }

  args.pack_rows(args.scratch.luma_16x16, args.scratch.chroma_16x8,
                 args.destination, half_width * 4);
}

void transform_sixteen(const uint8_t *source_a,
                       const uint8_t *source_b,
                       uint32_t *destination, int row_stride,
                       const uint8_t byte_remap[256]) {
  for (int tile = 0; tile != 16; ++tile) {
    pack_rows_eight(source_a, source_b, destination, byte_remap);
    source_a += 0x40;
    if ((tile & 1) != 0) source_b += 0x20;
    destination = reinterpret_cast<uint32_t *>(
        reinterpret_cast<uint8_t *>(destination) + row_stride);
  }
}

void pack_eight_pixels(const uint8_t *source_a,
                       const uint8_t *source_b,
                       uint32_t destination[8],
                       const uint8_t byte_remap[256]) {
  pack_rows_eight(source_a, source_b, destination, byte_remap);
}

}  // namespace movie_decoder_huffman
