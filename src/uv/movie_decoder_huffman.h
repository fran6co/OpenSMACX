#pragma once

#include <stddef.h>
#include <stdint.h>
#include "movie_decoder_bit_state.h"

namespace movie_decoder_huffman {

// Index and availability are kept separate: the original state contains a
// left-aligned accumulator and a signed count of valid high bits.
typedef ::MovieDecoderBitState BitState;

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
  int32_t *coefficients; // 64-word window: DC at 0, AC coefficients at 1..63
  const Tables *tables;
};

// Typed body for original 0x005CE85C. Decodes the coefficient
// sequence until the terminal table class is met. Keep the semantic C++ name;
// the native writer owns the original mapping and emitted linker identity.
int decode_coefficients(State &state);

}  // namespace movie_decoder_huffman
