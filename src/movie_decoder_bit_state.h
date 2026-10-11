#pragma once

#include <stdint.h>

// Shared 12-byte reader shape used by the frame and Huffman decoder views.
// The owning table stores this directly at 009C464C; staging decisions belong
// to each decoder operation, not to a second representation of the fields.
struct MovieDecoderBitState {
  const uint8_t *cursor;
  uint32_t accumulator;
  int32_t available_bits;
};

