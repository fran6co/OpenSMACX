#include "movie_decoder_idct.h"

#include <cstring>

namespace movie_decoder_idct {
namespace {

uint32_t bits(int32_t value) {
  uint32_t result;
  std::memcpy(&result, &value, sizeof(result));
  return result;
}

int32_t signed_bits(uint32_t value) {
  int32_t result;
  std::memcpy(&result, &value, sizeof(result));
  return result;
}

int32_t add32(int32_t left, int32_t right) {
  return signed_bits(bits(left) + bits(right));
}

int32_t sub32(int32_t left, int32_t right) {
  return signed_bits(bits(left) - bits(right));
}

int32_t mul2(int32_t value) {
  return signed_bits(bits(value) << 1);
}

int32_t signed_high_product(int32_t left, int32_t right) {
  const int64_t product = static_cast<int64_t>(left) * right;
  const uint64_t product_bits = static_cast<uint64_t>(product);
  return signed_bits(static_cast<uint32_t>(product_bits >> 32));
}

uint32_t low_dword(double value) {
  uint64_t representation;
  std::memcpy(&representation, &value, sizeof(representation));
  return static_cast<uint32_t>(representation);
}

void transform(const int32_t *source,
               int32_t *result,
               const IdctConstants &constants) {
  const int32_t sum_low = add32(source[3], source[5]);
  const int32_t sum_high = add32(source[7], source[1]);
  const int32_t difference_a = sub32(source[5], source[3]);
  const int32_t difference_b = sub32(source[1], source[7]);

  // These expressions follow the float-coefficient x87 dataflow recorded in
  // 0x005CED1D/0x005CEE80. The original stores each expression as a double,
  // then consumes that double's low dword as an integer. This host model
  // preserves the low-dword operation, but binary64 intermediates are not a
  // claim of x87 80-bit rounding equivalence.
  const double cross =
      (static_cast<double>(difference_b) + difference_a) * constants.cross_scale;
  const double first_float =
      static_cast<double>(difference_a) * constants.first_difference_scale +
      cross + constants.bias;
  const double second_float =
      static_cast<double>(difference_b) * constants.second_difference_scale -
      cross + constants.bias;
  const int32_t first = signed_bits(low_dword(first_float));
  const int32_t second = signed_bits(low_dword(second_float));

  const int32_t rotation = mul2(signed_high_product(
      sub32(sum_high, sum_low), constants.rotation_multiplier));
  const int32_t half_a = add32(first, rotation);
  const int32_t rotated = add32(rotation, second);
  const int32_t output_b = add32(add32(second, sum_low), sum_high);

  const int32_t sum_low_2 = add32(source[4], source[0]);
  const int32_t difference_c = sub32(source[0], source[4]);
  const int32_t rotation_2 = signed_high_product(
      sub32(source[2], source[6]), constants.rotation_multiplier);
  const int32_t rotation_2_twice = mul2(rotation_2);
  const int32_t sum_high_2 =
      add32(add32(source[6], source[2]), rotation_2_twice);
  const int32_t difference_d = add32(rotation_2_twice, difference_c);
  const int32_t difference_e = sub32(difference_c, rotation_2_twice);
  const int32_t tail_a = add32(sum_high_2, sum_low_2);
  const int32_t tail_b = sub32(sum_low_2, sum_high_2);

  result[0] = add32(tail_a, output_b);
  result[1] = add32(rotated, difference_d);
  result[2] = add32(difference_e, half_a);
  result[3] = add32(tail_b, first);
  result[4] = sub32(tail_b, first);
  result[5] = sub32(difference_e, half_a);
  result[6] = sub32(difference_d, rotated);
  result[7] = sub32(tail_a, output_b);
}

} // namespace

void transform_idct_row(const int32_t *input,
                         int32_t *output,
                         const IdctConstants &constants) {
  transform(input, output, constants);
}

void transform_idct_column(const int32_t *input,
                            int32_t *block_column,
                            const IdctConstants &constants) {
  bool other_coefficients_zero = true;
  for (size_t index = 1; index != 8; ++index) {
    other_coefficients_zero = other_coefficients_zero && input[index] == 0;
  }
  if (other_coefficients_zero) {
    for (size_t row = 0; row != 8; ++row) {
      block_column[row * 9] = input[0];
    }
    return;
  }

  int32_t transformed[8];
  transform(input, transformed, constants);
  for (size_t row = 0; row != 8; ++row) {
    block_column[row * 9] = transformed[row];
  }
}

void transform_idct_block(const int32_t *eight_input_vectors,
                          int32_t *intermediate_block,
                          int32_t *output,
                          size_t output_pitch_dwords,
                          const IdctConstants &constants) {
  for (size_t column = 0; column != 8; ++column) {
    transform_idct_column(eight_input_vectors + column * 8,
                          intermediate_block + column,
                          constants);
  }

  for (size_t row = 0; row != 8; ++row) {
    transform_idct_row(intermediate_block + row * 9, output + row * output_pitch_dwords,
                       constants);
  }
}

} // namespace movie_decoder_idct
