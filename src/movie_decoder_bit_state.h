#pragma once

#include <stdint.h>

// 12-byte reader shape of the Huffman decoder's State view. The frame reader
// it mirrors is not an instance of it: cursor, accumulator and bit count are
// three globals at 009C464C/50/54 (movie_decoder_owner.h), which the
// hand-written 0x005CE85C loads and stores directly.
struct MovieDecoderBitState {
  const uint8_t *cursor;
  uint32_t accumulator;
  int32_t available_bits;
};

