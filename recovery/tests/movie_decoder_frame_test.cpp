#include "movie_decoder_frame.h"

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <new>

using namespace movie_decoder_frame;

struct TestContext {
  std::uint8_t planes[3][32 * 32 * 4];
  int allocations;
  std::size_t allocation_sizes[3];
  int decode_calls;
  std::uint32_t decode_offsets[8];
  std::uint8_t *selected_base;
  std::uint8_t *coefficient_base;
  std::uint8_t **coefficient_slot;
  std::uint8_t **temporary_a_slot;
  std::uint8_t **temporary_b_slot;
  BitState *bits;
  const std::uint8_t *expected_reader_cursor;
  std::uint32_t expected_reader_accumulator;
  std::int32_t expected_reader_count;
  bool throw_on_decode;
  int packed_rows;
  std::uint32_t pack_source_offsets[40];
  std::uint32_t pack_destination_offsets[40];
  std::uint32_t pack_counts[40];
  std::uint8_t *destination_base;
};

static TestContext *active;

static void *host_calloc(std::size_t count) {
  const int index = active->allocations++;
  active->allocation_sizes[index] = count;
  std::memset(active->planes[index], 0xcc, sizeof(active->planes[index]));
  return active->planes[index];
}

static void decode_block(void *opaque, std::uint8_t *coefficient,
                         std::uint8_t *destination, int width) {
  TestContext *context = static_cast<TestContext *>(opaque);
  assert(width == 32);
  assert(*context->coefficient_slot != 0);
  assert(*context->temporary_a_slot != 0);
  assert(*context->temporary_b_slot != 0);
  assert(*context->coefficient_slot != *context->temporary_a_slot);
  assert(*context->coefficient_slot != *context->temporary_b_slot);
  assert(context->decode_calls < 8);
  context->decode_offsets[context->decode_calls] =
      static_cast<std::uint32_t>(coefficient - context->coefficient_base);
  assert(destination == context->selected_base +
                            context->decode_offsets[context->decode_calls]);
  assert(context->bits->cursor != 0);
  assert(context->bits->cursor == context->expected_reader_cursor);
  assert(context->bits->accumulator == context->expected_reader_accumulator);
  assert(context->bits->available_bits == context->expected_reader_count);
  ++context->decode_calls;
  if (context->throw_on_decode) throw 1;
}

static void pack_row(const std::uint32_t *source, std::uint32_t *destination,
                     int count) {
  const int row = active->packed_rows++;
  assert(row < 40);
  active->pack_source_offsets[row] = static_cast<std::uint32_t>(
      reinterpret_cast<const std::uint8_t *>(source) - active->selected_base);
  active->pack_destination_offsets[row] = static_cast<std::uint32_t>(
      reinterpret_cast<std::uint8_t *>(destination) - active->destination_base);
  active->pack_counts[row] = static_cast<std::uint32_t>(count);
}

static CodebookRecord record_for(int index, bool second_group) {
  CodebookRecord record;
  record.unresolved_word_4 = 0xaaaa0000u + static_cast<unsigned>(index);
  if (!second_group) {
    if ((index & 1) == 0) {
      record.word_before_cursor = 14;
      record.word_at_cursor = 0x00123456u + static_cast<unsigned>(index);
      record.selector_word_at_plus8 = 0;
    } else {
      record.word_before_cursor = 9;
      record.word_at_cursor = 0x00234567u + static_cast<unsigned>(index);
      record.selector_word_at_plus8 = 0xfc00u;
    }
  } else if ((index & 1) == 0) {
    record.word_before_cursor = 9;
    record.word_at_cursor = 0x00345678u + static_cast<unsigned>(index);
    record.selector_word_at_plus8 = 0;
  } else {
    record.word_before_cursor = 6;
    record.word_at_cursor = 0x00456789u + static_cast<unsigned>(index);
    record.selector_word_at_plus8 = 0x8000u;
  }
  return record;
}

static void test_packet_and_mode_alias_publication() {
  alignas(BitState) std::uint8_t alias_storage[96] = {};
  BitState *const alias_bits = new (alias_storage + 16) BitState;
  alias_bits->cursor = alias_storage + 40;
  alias_bits->accumulator = 0xaaaaaaaa;
  alias_bits->available_bits = 0x45674567;
  const std::uint32_t expected_accumulator = 0x45674567;
  const std::uint8_t *const packet =
      reinterpret_cast<const std::uint8_t *>(alias_bits) +
      offsetof(BitState, available_bits) - 0x10;

  std::uint8_t *coefficient = alias_storage;
  std::uint8_t *temporary_a = alias_storage + 1;
  std::uint8_t *temporary_b = alias_storage + 2;
  RuntimePlanes planes = {&coefficient, &temporary_a, &temporary_b};
  std::uint32_t allocation_gate = 1;
  std::uint32_t table_gate = 1;
  std::int32_t scale_source[64] = {};
  std::int32_t scale_output[64] = {};
  std::int32_t transform_products[64] = {};
  Tables tables = {0, 0, 0, 0, scale_source, scale_output};
  std::uint8_t destination[1] = {};
  Callbacks callbacks = {0, 0, 0};
  FrameArgs args;
  args.planes = &planes;
  args.allocation_gate = &allocation_gate;
  args.record_mode = reinterpret_cast<std::uint32_t *>(
      &alias_bits->available_bits);
  args.table_init_gate = &table_gate;
  args.width = 0;
  args.height = 1;
  args.compressed = packet;
  args.bit_state = alias_bits;
  args.destination = destination;
  args.output_pitch = 0;
  args.record_kind = 7;
  args.codebook_records = 0;
  args.transform_products = transform_products;
  args.tables = tables;
  args.decoder_context = 0;
  args.callbacks = callbacks;

  assert(decode_frame(args) == 0);
  assert(alias_bits->cursor == packet + 0x14);
  assert(alias_bits->accumulator == expected_accumulator);
  assert(alias_bits->available_bits == 1);
}

static void test_bias_byte_precedes_codebook_initialization() {
  alignas(std::uint32_t) std::uint8_t storage[0x900] = {};
  const std::uint8_t *const compressed = storage + 3;
  std::uint32_t *const primary =
      reinterpret_cast<std::uint32_t *>(storage + 16);
  primary[0] = 0x11223380u;
  std::uint32_t secondary_a[256] = {};
  std::uint32_t secondary_b[256] = {};
  std::uint32_t block_prefix[64] = {};
  std::int32_t scale_source[64] = {};
  std::int32_t scale_output[64] = {};
  std::int32_t transform_products[64] = {};
  CodebookRecord records[222];
  for (int i = 0; i != 94; ++i) records[i] = record_for(i, false);
  for (int i = 0; i != 128; ++i) records[94 + i] = record_for(i, true);
  scale_source[1] = 1;
  transform_products[1] = 1;

  std::uint8_t *coefficient = storage;
  std::uint8_t *temporary_a = storage + 1;
  std::uint8_t *temporary_b = storage + 2;
  RuntimePlanes planes = {&coefficient, &temporary_a, &temporary_b};
  std::uint32_t allocation_gate = 1;
  std::uint32_t table_gate = 0;
  std::uint32_t record_mode = 0;
  BitState bits = {0, 0, 0};
  Tables tables = {primary, secondary_a, secondary_b, block_prefix,
                   scale_source, scale_output};
  std::uint8_t destination[1] = {};
  Callbacks callbacks = {0, 0, 0};
  FrameArgs args;
  args.planes = &planes;
  args.allocation_gate = &allocation_gate;
  args.record_mode = &record_mode;
  args.table_init_gate = &table_gate;
  args.width = 0;
  args.height = 1;
  args.compressed = compressed;
  args.bit_state = &bits;
  args.destination = destination;
  args.output_pitch = 0;
  args.record_kind = 5;
  args.codebook_records = records;
  args.transform_products = transform_products;
  args.tables = tables;
  args.decoder_context = 0;
  args.callbacks = callbacks;

  assert(static_cast<std::uint8_t>(compressed[0x0d]) == 0x80);
  assert(decode_frame(args) == 0);
  assert(primary[0] == 0x0fu);
  // Bias is captured as signed -128 before the codebook initializer overwrites
  // its aliased source byte with 0x0f; applying the latter would yield zero.
  assert(scale_output[1] == -8);
}

int main() {
  test_packet_and_mode_alias_publication();
  test_bias_byte_precedes_codebook_initialization();
  std::uint32_t primary[512];
  std::uint32_t secondary_a[256];
  std::uint32_t secondary_b[256];
  std::uint32_t prefix[64];
  std::int32_t scale_source[64];
  std::int32_t scale_output[64];
  CodebookRecord records[222];
  std::int32_t transform_products[64];
  std::uint8_t compressed[64];
  std::uint8_t output[32 * 32 * 4];
  std::memset(primary, 0xa5, sizeof(primary));
  std::memset(secondary_a, 0xa5, sizeof(secondary_a));
  std::memset(secondary_b, 0xa5, sizeof(secondary_b));
  std::memset(prefix, 0xa5, sizeof(prefix));
  for (int i = 0; i != 64; ++i) {
    scale_source[i] = i * 0x1000 - 0x7000;
    scale_output[i] = 0x55555555;
    transform_products[i] = i * 0x101 + 3;
  }
  for (int i = 0; i != 94; ++i) records[i] = record_for(i, false);
  for (int i = 0; i != 128; ++i) records[94 + i] = record_for(i, true);
  for (int i = 0; i != 64; ++i) compressed[i] = static_cast<std::uint8_t>(i * 13 + 7);
  std::memset(output, 0x77, sizeof(output));

  std::uint8_t *coefficient_plane = 0;
  std::uint8_t *temporary_plane_a = 0;
  std::uint8_t *temporary_plane_b = 0;
  RuntimePlanes planes = {&coefficient_plane, &temporary_plane_a,
                          &temporary_plane_b};
  std::uint32_t allocation_gate = 0;
  std::uint32_t record_mode = 0;
  std::uint32_t table_gate = 0;
  BitState bits = {0, 0, 0};
  TestContext context;
  std::memset(&context, 0, sizeof(context));
  context.bits = &bits;
  context.expected_reader_cursor = compressed + 0x14;
  context.expected_reader_accumulator =
      (static_cast<std::uint32_t>(compressed[0x10] | (compressed[0x11] << 8)) << 16) |
      static_cast<std::uint32_t>(compressed[0x12] | (compressed[0x13] << 8));
  context.expected_reader_count = 0x20;
  context.throw_on_decode = false;
  context.coefficient_slot = &coefficient_plane;
  context.temporary_a_slot = &temporary_plane_a;
  context.temporary_b_slot = &temporary_plane_b;
  context.destination_base = output;
  active = &context;
  Callbacks callbacks = {host_calloc, decode_block, pack_row};
  Tables tables = {primary, secondary_a, secondary_b, prefix,
                   scale_source, scale_output};
  FrameArgs args;
  args.planes = &planes;
  args.allocation_gate = &allocation_gate;
  args.record_mode = &record_mode;
  args.table_init_gate = &table_gate;
  args.width = 32;
  args.height = 32;
  args.compressed = compressed;
  args.bit_state = &bits;
  args.destination = output;
  args.output_pitch = 128;
  args.record_kind = 5;
  args.codebook_records = records;
  args.transform_products = transform_products;
  args.tables = tables;
  args.decoder_context = &context;
  args.callbacks = callbacks;
  context.selected_base = context.planes[1];
  context.coefficient_base = context.planes[0];

  assert(decode_frame(args) == 0);
  assert(allocation_gate == 1 && table_gate == 1 && record_mode == 0);
  assert(context.allocations == 3);
  assert(context.allocation_sizes[0] == sizeof(context.planes[0]));
  assert(context.allocation_sizes[1] == sizeof(context.planes[1]));
  assert(context.allocation_sizes[2] == sizeof(context.planes[2]));
  assert(coefficient_plane == context.planes[1]);
  assert(temporary_plane_a == context.planes[0]);
  assert(temporary_plane_b == context.planes[2]);
  assert(bits.cursor == compressed + 0x14);
  assert(bits.accumulator ==
         (static_cast<std::uint32_t>(compressed[0x10] | (compressed[0x11] << 8)) << 16 |
          static_cast<std::uint32_t>(compressed[0x12] | (compressed[0x13] << 8))));
  assert(bits.available_bits == 0x20);
  assert(context.decode_calls == 4);
  assert(context.decode_offsets[0] == 0);
  assert(context.decode_offsets[1] == 32);
  assert(context.decode_offsets[2] == 1024);
  assert(context.decode_offsets[3] == 1056);
  assert(context.packed_rows == 31);
  for (int row = 0; row != 31; ++row) {
    assert(context.pack_source_offsets[row] == static_cast<unsigned>(row * 64));
    assert(context.pack_destination_offsets[row] == static_cast<unsigned>(row * 128));
    assert(context.pack_counts[row] == 16);
  }
  assert(primary[0] == 0x0f && primary[1] == 0x1f && primary[7] == 0x1f);
  assert(primary[8] == 0x2f && primary[15] == 0x2f);
  assert(primary[256] == 0x3f && primary[383] == 0x3f);
  assert(primary[504] != 0xa5a5a5a5u);
  assert(secondary_a[0] != 0xa5a5a5a5u);
  assert(secondary_b[0] != 0xa5a5a5a5u);
  assert(prefix[0] == 1 && prefix[31] == 1);
  assert(prefix[32] == 0x00400006 && prefix[47] == 0x04000006);
  assert(prefix[48] == 0xfc000006u && prefix[49] == 0xa5a5a5a5u);
  assert(scale_output[0] != 0x55555555);
  assert(scale_output[63] != 0x55555555);

  // Second invocation exercises allocation/table one-time gates and kind-7
  // plane selection/swap suppression without resetting process-owned state.
  context.decode_calls = 0;
  context.packed_rows = 0;
  context.selected_base = context.planes[2];
  context.coefficient_base = coefficient_plane;
  args.record_kind = 7;
  args.destination = output;
  assert(decode_frame(args) == 0);
  assert(context.allocations == 3 && table_gate == 1 && allocation_gate == 1);
  assert(record_mode == 1);
  assert(coefficient_plane == context.planes[1]);
  assert(temporary_plane_a == context.planes[0]);
  assert(context.decode_calls == 4 && context.packed_rows == 31);

  // A failing downstream callback sees and leaves the packet reader published.
  context.decode_calls = 0;
  context.throw_on_decode = true;
  args.record_kind = 5;
  bits.cursor = 0;
  bits.accumulator = 0xdeadbeefu;
  bits.available_bits = -7;
  bool caught = false;
  try {
    decode_frame(args);
  } catch (int value) {
    assert(value == 1);
    caught = true;
  }
  assert(caught && context.decode_calls == 1);
  assert(record_mode == 0);
  assert(bits.cursor == compressed + 0x14);
  assert(bits.accumulator == context.expected_reader_accumulator);
  assert(bits.available_bits == 0x20);

  return 0;
}
