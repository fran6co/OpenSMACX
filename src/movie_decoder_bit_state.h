#pragma once

#include <stdint.h>

// Shared 12-byte reader shape used by the frame and Huffman decoder views.
// Its one instance is the standalone movie_media_frame_bit_state object at
// 009C464C; staging decisions belong to each decoder operation, not to a
// second representation of the fields.
struct MovieDecoderBitState {
  const uint8_t *cursor;
  uint32_t accumulator;
  int32_t available_bits;
};

