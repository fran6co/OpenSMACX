#include "movie_decoder_frame.h"
#include "movie_decoder_huffman.h"
#include "movie_decoder_idct.h"

#include <cassert>
#include <cstdint>
#include <cstring>

using namespace movie_decoder_frame;

extern int sub_5d2761(unsigned int *source, unsigned int *destination,
                      int count);

struct CombinedContext {
  std::uint32_t primary[512];
  std::uint32_t secondary_a[256];
  std::uint32_t secondary_b[256];
  std::uint32_t block_prefix[64];
  std::int32_t coefficient_offsets[64];
  std::int32_t coefficient_scales[64];
  std::int32_t coefficient_words[64];
  std::int32_t scale_source[64];
  std::int32_t scale_output[64];
  std::int32_t transform_products[65];
  std::int32_t idct_scratch_words[4];
  std::uint8_t remap[256];
  std::uint8_t luma_scratch[0x400];
  std::uint8_t chroma_scratch[0x200];
  std::int32_t idct_intermediate[135];
  movie_decoder_huffman::BitState bit_state;
  movie_decoder_frame::BitState frame_bit_state;
  movie_decoder_huffman::Tables huffman_tables;
  movie_decoder_idct::IdctConstants idct_constants;
  std::uint8_t allocated_planes[3][16 * 16 * 4];
  std::size_t allocation_sizes[3];
  int allocations;
  int block_calls;
  int row_transform_calls;
  int blended_rows;
  std::uint8_t *selected_plane;
  std::uint8_t *output_base;
};

static CombinedContext *active_context;

static void *tracked_calloc(std::size_t bytes) {
  const int index = active_context->allocations++;
  assert(index < 3);
  active_context->allocation_sizes[index] = bytes;
  std::memset(active_context->allocated_planes[index], 0,
              sizeof(active_context->allocated_planes[index]));
  return active_context->allocated_planes[index];
}

static void idct_rows(std::uint8_t *output, int stride_arg) {
  ++active_context->row_transform_calls;
  movie_decoder_idct::transform_idct_block(
      active_context->coefficient_words,
      active_context->idct_intermediate,
      reinterpret_cast<std::int32_t *>(output),
      static_cast<std::size_t>(stride_arg), active_context->idct_constants,
      active_context->idct_scratch_words);
}

static void pack_rows(const std::uint8_t *component_rows,
                      const std::uint8_t *chroma_rows,
                      std::uint8_t *destination, int row_pitch_bytes) {
  movie_decoder_huffman::transform_sixteen(
      component_rows, chroma_rows, reinterpret_cast<std::uint32_t *>(destination),
      row_pitch_bytes, active_context->remap);
}

static void decode_six_blocks(void *opaque, std::uint8_t *coefficient_block,
                              std::uint8_t *destination_block, int width) {
  CombinedContext *context = static_cast<CombinedContext *>(opaque);
  ++context->block_calls;
  context->bit_state.cursor = context->frame_bit_state.cursor;
  context->bit_state.accumulator = context->frame_bit_state.accumulator;
  context->bit_state.available_bits = context->frame_bit_state.available_bits;
  movie_decoder_huffman::State coefficient_state;
  coefficient_state.bits = context->bit_state;
  coefficient_state.fill_value = 0;
  coefficient_state.scale_factor = context->scale_output[0];
  coefficient_state.coefficients = context->coefficient_words;
  coefficient_state.tables = &context->huffman_tables;

  movie_decoder_huffman::SixBlockArgs args;
  args.coefficient_plane = coefficient_block;
  args.destination = destination_block;
  args.row_width = width;
  args.header_enabled = false;
  args.scratch.luma_16x16 = context->luma_scratch;
  args.scratch.chroma_16x8 = context->chroma_scratch;
  args.bits = &context->bit_state;
  args.block_huffman = context->block_prefix;
  args.coefficient_state = &coefficient_state;
  args.idct_rows = idct_rows;
  args.byte_remap = context->remap;
  args.pack_rows = pack_rows;
  movie_decoder_huffman::decode_six_blocks(args);
  context->frame_bit_state.cursor = context->bit_state.cursor;
  context->frame_bit_state.accumulator = context->bit_state.accumulator;
  context->frame_bit_state.available_bits = context->bit_state.available_bits;
}

static void blend_plane_row(const std::uint32_t *source,
                            std::uint32_t *destination, int count) {
  ++active_context->blended_rows;
  sub_5d2761(const_cast<unsigned int *>(source),
             reinterpret_cast<unsigned int *>(destination), count);
}

int main() {
  CombinedContext context;
  std::memset(&context, 0, sizeof(context));
  active_context = &context;

  // Each primary entry is the observed terminal tag class, with count 0x30.
  // This bounded fixture exercises coefficient-state return and block fill
  // without asserting validity for arbitrary compressed input.
  for (int i = 0; i != 512; ++i) context.primary[i] = 0x30;
  for (int i = 0; i != 256; ++i) {
    context.secondary_a[i] = 0x30;
    context.secondary_b[i] = 0x30;
  }
  for (int i = 0; i != 256; ++i) context.remap[i] = static_cast<std::uint8_t>(i);
  context.huffman_tables.primary = context.primary;
  context.huffman_tables.secondary_a = context.secondary_a;
  context.huffman_tables.secondary_b = context.secondary_b;
  context.huffman_tables.coefficient_offsets = context.coefficient_offsets;
  context.huffman_tables.coefficient_scales = context.coefficient_scales;
  context.idct_constants.rotation_multiplier = 0;
  context.idct_constants.first_difference_scale = 0.0f;
  context.idct_constants.second_difference_scale = 0.0f;
  context.idct_constants.cross_scale = 0.0f;
  context.idct_constants.bias = 0.0f;

  CodebookRecord records[222];
  std::memset(records, 0, sizeof(records)); // not read: initialized table gate is set
  std::uint8_t *coefficient_plane = 0;
  std::uint8_t *temporary_plane_a = 0;
  std::uint8_t *temporary_plane_b = 0;
  RuntimePlanes planes = {&coefficient_plane, &temporary_plane_a,
                          &temporary_plane_b};
  std::uint32_t allocation_gate = 0;
  std::uint32_t record_mode = 0;
  std::uint32_t table_gate = 1;
  std::uint8_t compressed[256];
  std::memset(compressed, 0, sizeof(compressed));
  std::uint8_t output[16 * 64];
  std::memset(output, 0x7f, sizeof(output));
  context.output_base = output;

  Callbacks callbacks = {tracked_calloc, decode_six_blocks, blend_plane_row};
  Tables tables = {context.primary, context.secondary_a, context.secondary_b,
                   context.block_prefix, context.scale_source,
                   context.scale_output};
  FrameArgs frame;
  frame.planes = &planes;
  frame.allocation_gate = &allocation_gate;
  frame.record_mode = &record_mode;
  frame.table_init_gate = &table_gate;
  frame.width = 16;
  frame.height = 16;
  frame.compressed = compressed;
  frame.bit_state = &context.frame_bit_state;
  frame.destination = output;
  frame.output_pitch = 64;
  frame.record_kind = 7;
  frame.codebook_records = records;
  frame.transform_products = context.transform_products;
  frame.tables = tables;
  frame.decoder_context = &context;
  frame.callbacks = callbacks;

  assert(decode_frame(frame) == 0);
  assert(allocation_gate == 1 && table_gate == 1 && record_mode == 1);
  assert(context.allocations == 3);
  assert(context.allocation_sizes[0] == 16u * 16u * 4u);
  assert(context.block_calls == 1);
  assert(context.row_transform_calls == 0); // terminal coefficient entries use fill path
  assert(context.blended_rows == 15);
  assert(context.frame_bit_state.cursor == compressed + 0x14 + 10);
  assert(context.frame_bit_state.available_bits == 20);
  assert(temporary_plane_b == context.allocated_planes[2]);
  assert(temporary_plane_a == context.allocated_planes[1]);
  for (int row = 0; row != 15; ++row) {
    for (int byte = 0; byte != 32; ++byte)
      assert(output[row * 64 + byte] == 0);
    for (int byte = 32; byte != 64; ++byte)
      assert(output[row * 64 + byte] == 0x7f);
  }
  for (int byte = 15 * 64; byte != 16 * 64; ++byte)
    assert(output[byte] == 0x7f);

  // The same linked host binary also executes the actual typed IDCT body
  // directly; this is a sparse zero fixture, not a rounding-parity claim.
  std::int32_t input[64] = {};
  std::int32_t intermediate[135] = {};
  std::int32_t idct_output[64] = {};
  movie_decoder_idct::transform_idct_block(
      input, intermediate, idct_output, 8, context.idct_constants,
      context.idct_scratch_words);
  for (int i = 0; i != 64; ++i) assert(idct_output[i] == 0);
  return 0;
}
