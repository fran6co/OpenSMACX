#pragma once

#include <stddef.h>
#include <stdint.h>

namespace movie_decoder_huffman {

// Index and availability are kept separate: the original state contains a
// left-aligned accumulator and a signed count of valid high bits.
struct BitState {
  const uint8_t *cursor;
  uint32_t accumulator;
  int32_t available_bits;
};

struct Tables {
  const uint32_t *primary;
  const uint32_t *secondary_a;
  const uint32_t *secondary_b;
  const int32_t *coefficient_offsets;
  const int32_t *coefficient_scales;
};

struct State {
  BitState bits;
  int32_t fill_value;
  int32_t scale_factor;
  int32_t *coefficients;
  const Tables *tables;
};

// `word_cursor` is kept as bytes to avoid alignment and aliasing assumptions.
// This operation matches the word-refill path in 0x005CE85C.
void refill_word(BitState &state);

struct BlockHeader {
  uint32_t flags;
  uint32_t delta_mode;
  uint8_t *coefficient_cursor;
};

// Header/flag prelude at entry to 0x005D2E70. Besides flags, the two codebook
// entries advance the coefficient cursor and choose the first four block
// sampling phase.
BlockHeader decode_block_header(BitState &state, bool header_enabled,
                                const uint32_t block_huffman[64],
                                uint8_t *coefficient_plane,
                                int half_width);

// Typed body for original 0x005CE85C. Decodes the coefficient
// sequence until the terminal table class is met. Keep the semantic C++ name;
// the native writer owns the original mapping and emitted linker identity.
int decode_coefficients(State &state);

struct ScratchPlanes {
  uint8_t *luma_16x16;
  uint8_t *chroma_16x8;
};

struct SixBlockArgs {
  uint8_t *coefficient_plane;
  uint8_t *destination;
  int row_width;
  bool header_enabled;
  ScratchPlanes scratch;
  BitState *bits;
  const uint32_t *block_huffman;
  State *coefficient_state;
  void (*idct_rows)(uint8_t *row_base, int stride_arg);
  const uint8_t *byte_remap;
  void (*pack_rows)(const uint8_t *component_rows,
                    const uint8_t *chroma_rows,
                    uint8_t *destination, int row_pitch_bytes);
};

// A typed host representation of the observed six-block path in 0x005D2E70.
// Its actual caller supplies the current coefficient plane, output pointer,
// and width. The remaining decoder tables/state are explicit borrowed views.
void decode_six_blocks(SixBlockArgs &args);

// Normalized C++ interface for 0x005CEA09. Its caller pushes these four
// arguments. The wrapper advances source A by 0x40 bytes per tile, source B
// by 0x20 bytes every second tile, and destination by row_stride each tile.
void transform_sixteen(const uint8_t *source_a,
                       const uint8_t *source_b,
                       uint32_t *destination, int row_stride,
                       const uint8_t byte_remap[256]);

// Explicit C++ form of the work done by the sixteen hidden-register calls to
// 0x005CEB12. This is also the per-tile operation used by transform_sixteen.
void pack_eight_pixels(const uint8_t *source_a,
                       const uint8_t *source_b,
                       uint32_t destination[8],
                       const uint8_t byte_remap[256]);

}  // namespace movie_decoder_huffman
