#include "movie_decoder_frame.h"

#include <cstring>

namespace movie_decoder_frame {
namespace {

static int32_t signed_word(uint32_t value) {
  int32_t result;
  std::memcpy(&result, &value, sizeof(result));
  return result;
}

static int32_t arithmetic_shift_right(uint32_t value,
                                          unsigned shift) {
  const int32_t signed_value = signed_word(value);
  if (signed_value >= 0) return signed_value >> shift;
  return -1 - static_cast<int32_t>((~value) >> shift);
}

static int32_t fixed_product_16(int32_t left,
                                    int32_t right) {
  const int64_t product =
      static_cast<int64_t>(left) * static_cast<int64_t>(right);
  uint64_t representation;
  std::memcpy(&representation, &product, sizeof(representation));
  const uint32_t high_word = static_cast<uint32_t>(representation >> 32);
  const uint32_t low_word = static_cast<uint32_t>(representation);
  const uint32_t combined = (low_word >> 16) + (high_word << 16) +
                                 ((low_word >> 15) & 1u);
  return signed_word(combined);
}

static uint16_t read_u16(const uint8_t *bytes) {
  return static_cast<uint16_t>(bytes[0]) |
         static_cast<uint16_t>(bytes[1] << 8);
}

static uint32_t repeated_count(int32_t shift) {
  const unsigned count_shift = static_cast<unsigned>(shift) & 31u;
  return uint32_t(1) << count_shift;
}

static uint32_t pack_primary_value(uint32_t code,
                                        uint32_t length) {
  return (((code & 0xfc00u) | (code << 16)) << 6) | length;
}

static void write_repeat(uint32_t *destination, uint32_t value,
                         uint32_t count) {
  for (uint32_t i = 0; i != count; ++i) destination[i] = value;
}

static void initialize_scale_outputs(FrameArgs &args, int8_t record_bias,
                                     int32_t first_scaled) {
  args.tables.scale_output[0] =
      fixed_product_16(args.tables.scale_source[0], first_scaled);

  for (int offset = 0; offset < 0xfc; offset += 4) {
    const int scale_index = 1 + offset / 4;
    const uint32_t wrapped =
        static_cast<uint32_t>(args.transform_products[scale_index]) *
        static_cast<uint32_t>(static_cast<int32_t>(record_bias)) *
        0x1000u;
    const int32_t right = signed_word(wrapped);
    args.tables.scale_output[scale_index] = fixed_product_16(
        args.tables.scale_source[scale_index], right);
  }
}

} // namespace

void initialize_codebooks(Tables &tables, const CodebookRecord records[222]) {
  tables.primary[0] = 0x0f;
  write_repeat(tables.primary + 1, 0x1f, 7);
  write_repeat(tables.primary + 8, 0x2f, 8);
  write_repeat(tables.primary + 256, 0x3f, 128);

  for (int short_record_index = 0; short_record_index != 94;
       ++short_record_index) {
    const CodebookRecord &record = records[short_record_index];
    const uint32_t prefix = record.word_before_cursor;
    const uint32_t code = record.word_at_cursor;
    const uint32_t selector = record.selector_word_at_plus8;
    const int8_t signed_prefix = static_cast<int8_t>(prefix);
    if ((selector & 0xfc00u) == 0) {
      const uint32_t count = repeated_count(0x0e - signed_prefix);
      const uint32_t value = pack_primary_value(code, prefix - 6u);
      if (static_cast<int32_t>(count) > 0)
        write_repeat(tables.secondary_b + arithmetic_shift_right(selector, 2),
                     value, count);
    } else {
      const uint32_t count = repeated_count(9 - signed_prefix);
      const uint32_t value = pack_primary_value(code, prefix);
      if (static_cast<int32_t>(count) > 0)
        write_repeat(tables.primary + arithmetic_shift_right(selector, 7),
                     value, count);
    }
  }

  for (int long_record_index = 0; long_record_index != 128;
       ++long_record_index) {
    const CodebookRecord &record = records[94 + long_record_index];
    const uint32_t prefix = record.word_before_cursor;
    const uint32_t code = record.word_at_cursor;
    const uint32_t selector = record.selector_word_at_plus8;
    const int8_t adjusted_prefix =
        static_cast<int8_t>(static_cast<int8_t>(prefix) + 8);
    if ((selector & 0x8000u) == 0) {
      const uint32_t count = repeated_count(0x11 - adjusted_prefix);
      const uint32_t value = pack_primary_value(code, prefix - 1u);
      if (static_cast<int32_t>(count) > 0)
        write_repeat(tables.secondary_a + arithmetic_shift_right(selector, 7),
                     value, count);
    } else {
      const uint32_t count = repeated_count(0x0e - adjusted_prefix);
      const uint32_t value = pack_primary_value(code, prefix + 2u);
      if (static_cast<int32_t>(count) > 0)
        write_repeat(tables.secondary_b + arithmetic_shift_right(selector, 10),
                     value, count);
    }
  }

  write_repeat(tables.block_prefix, 1, 32);
  uint32_t value = 0x00400000;
  for (int prefix_index = 32; value < 0x04400000;
       ++prefix_index, value += 0x00400000)
    tables.block_prefix[prefix_index] = value | 6u;
}

int decode_frame(FrameArgs &args) {
  if (*args.allocation_gate == 0) {
    *args.allocation_gate = 1;
    const uint32_t byte_count = args.width * args.height * 4u;
    *args.planes->coefficient = static_cast<uint8_t *>(
        args.callbacks.calloc_bytes(byte_count));
    *args.planes->temporary_a = static_cast<uint8_t *>(
        args.callbacks.calloc_bytes(byte_count));
    *args.planes->temporary_b = static_cast<uint8_t *>(
        args.callbacks.calloc_bytes(byte_count));
  }

  const uint32_t record_mode_value = args.record_kind != 5;
  const uint32_t table_init_gate = *args.table_init_gate;
  const int8_t record_bias =
      static_cast<int8_t>(args.compressed[0x0d]);
  if (table_init_gate == 0) {
    initialize_codebooks(args.tables, args.codebook_records);
    *args.table_init_gate = 1;
    args.tables.block_prefix[48] = 0xfc000006u;
  }

  // The original reads both packet words and computes the cursor before it
  // publishes the three shared reader fields, in count/accumulator/cursor
  // order. Keep the record-mode store after those publications as well.
  const uint32_t packet_accumulator =
      (static_cast<uint32_t>(read_u16(args.compressed + 0x10)) << 16) |
      read_u16(args.compressed + 0x12);
  const uint8_t *const packet_cursor = args.compressed + 0x14;
  args.bit_state->available_bits = 0x20;
  args.bit_state->accumulator = packet_accumulator;
  const int32_t first_scaled = signed_word(
      static_cast<uint32_t>(args.transform_products[0]) << 15);
  args.bit_state->cursor = packet_cursor;
  *args.record_mode = record_mode_value;

  initialize_scale_outputs(args, record_bias, first_scaled);
  uint8_t *selected = args.record_kind == 7
      ? *args.planes->temporary_b : *args.planes->temporary_a;

  for (uint32_t y = 0; y < args.height; y += 0x10) {
    for (uint32_t x = 0; x < args.width; x += 0x10) {
      const uint32_t pixel_index = args.width * y + x;
      const uint32_t byte_offset = ((pixel_index >> 1) << 2);
      args.callbacks.decode_six_blocks(args.decoder_context,
                                       *args.planes->coefficient + byte_offset,
                                       selected + byte_offset,
                                       static_cast<int>(args.width));
    }
  }

  if (args.height != 1) {
    const uint32_t words_per_row = args.width >> 1;
    uint8_t *source = selected;
    uint8_t *destination = args.destination;
    for (uint32_t row = 0; row < args.height - 1; ++row) {
      args.callbacks.pack_plane_row(
          reinterpret_cast<const uint32_t *>(source),
          reinterpret_cast<uint32_t *>(destination),
          static_cast<int>(words_per_row));
      source += words_per_row * 4u;
      destination += args.output_pitch;
    }
  }

  if (args.record_kind != 7) {
    uint8_t *const old_coefficient = *args.planes->coefficient;
    *args.planes->coefficient = *args.planes->temporary_a;
    *args.planes->temporary_a = old_coefficient;
  }
  return 0;
}

} // namespace movie_decoder_frame
