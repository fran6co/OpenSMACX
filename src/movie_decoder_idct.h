#pragma once

#include <stddef.h>
#include <stdint.h>

namespace movie_decoder_idct {

// These are the five values read from the native transform's shared constant
// area. The executable listing proves the access widths and operations, but
// not the data owner's bounds or initializer. Callers must supply values from
// an independently established state snapshot.
struct IdctConstants {
  int32_t rotation_multiplier;
  float first_difference_scale;
  float second_difference_scale;
  float cross_scale;
  float bias;
};

// Input has eight consecutive signed dwords. Output is eight consecutive
// signed dwords. This is the arithmetic body used by native 0x005CEE80.
void transform_idct_row(const int32_t *input,
                        int32_t *output,
                        const IdctConstants &constants);

// Input is one 8-dword coefficient vector from an 0x20-byte native table
// record. Output is eight coefficients written at a nine-dword block pitch,
// as observed in 0x005CED1D's stores (+0,+0x24,...,+0xFC).
void transform_idct_column(const int32_t *input,
                           int32_t *block_column,
                           const IdctConstants &constants);

// The eight input vectors are contiguous 8-dword records, matching the native
// 0x20-byte-spaced loader tables. The intermediate block has a nine-dword row
// pitch. The destination pitch is the caller's
// `stride_arg` from 0x005CEF8A, measured in dwords (native scales it by 4).
// This models the two native transform passes; it does not validate either
// input extent or destination allocation.
void transform_idct_block(const int32_t *eight_input_vectors,
                          int32_t *intermediate_block,
                          int32_t *output,
                          size_t output_pitch_dwords,
                          const IdctConstants &constants);

} // namespace movie_decoder_idct
