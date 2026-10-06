// Host fixture; test injects the exact source-owned 5CF2F0 function body.
#include <cassert>
#include <cstring>
#include <stddef.h>
#include <stdint.h>

typedef unsigned char BYTE;
typedef uint32_t DWORD;
#define __cdecl

struct MediaDecoderContext {
  DWORD block_transform_weights[64];
  DWORD predictor_4f8;
  DWORD predictor_4fc;
  DWORD predictor_500;
};
struct MediaTransformRegisterContinuation {
  const BYTE *esi_cursor;
  DWORD ebp_accumulator;
  int32_t ecx_available_bits;
  int edi_coefficient_index;
};
struct MediaCoefficientPredictorViewLayout {
  DWORD preceding_lookup_words[229];
  BYTE luma_delta_entries[128];
  BYTE chroma_delta_entries[256];
};
struct MediaCoefficientTableStorage { DWORD words[325]; };
typedef char table_window_size_check[
    sizeof(MediaCoefficientTableStorage) == 0x514 ? 1 : -1];
typedef char luma_offset_check[
    offsetof(MediaCoefficientPredictorViewLayout, luma_delta_entries) ==
        0x394 ? 1 : -1];
typedef char chroma_offset_check[
    offsetof(MediaCoefficientPredictorViewLayout, chroma_delta_entries) ==
        0x414 ? 1 : -1];

static MediaCoefficientTableStorage movie_media_coefficient_tables;
static const BYTE *movie_media_luma_delta_entries() {
  return reinterpret_cast<const BYTE *>(movie_media_coefficient_tables.words) +
      offsetof(MediaCoefficientPredictorViewLayout, luma_delta_entries);
}
static const BYTE *movie_media_chroma_delta_entries() {
  return reinterpret_cast<const BYTE *>(movie_media_coefficient_tables.words) +
      offsetof(MediaCoefficientPredictorViewLayout, chroma_delta_entries);
}
static int transform_calls;
static BYTE host_input[32];
static MediaDecoderContext host_context;
static int host_transform_index;

static DWORD movie_media_read_bits32(const BYTE *input, DWORD bit_position) {
  const DWORD word_index = bit_position >> 5;
  const DWORD shift = bit_position & 31;
  DWORD high_word, low_word;
  std::memcpy(&high_word, input + word_index * 4, sizeof(high_word));
  std::memcpy(&low_word, input + word_index * 4 + 4, sizeof(low_word));
  if (shift == 0) return high_word;
  return (high_word << shift) | (low_word >> (32 - shift));
}
static DWORD movie_media_decode_predictor_delta(
    const BYTE *input, DWORD &bit_position, const BYTE *lookup,
    unsigned lookup_shift) {
  const BYTE code = static_cast<BYTE>(
      lookup[movie_media_read_bits32(input, bit_position) >> lookup_shift]);
  const unsigned width = code >> 4;
  bit_position += code & 0x0f;
  if (width == 0) return 0;
  const DWORD value = movie_media_read_bits32(input, bit_position) >>
                      (32 - width);
  bit_position += width;
  if (((value >> (width - 1)) & 1) != 0) return value;
  return value - ((1u << width) - 1u);
}
static MediaTransformRegisterContinuation sub_5cf150() {
  ++transform_calls;
  MediaTransformRegisterContinuation state;
  state.esi_cursor = reinterpret_cast<const BYTE *>(&host_context);
  state.ebp_accumulator = static_cast<DWORD>(
      reinterpret_cast<size_t>(host_input));
  state.ecx_available_bits = 8;
  state.edi_coefficient_index = host_transform_index;
  return state;
}

/*__ACTUAL_OWNER__*/

int main() {
  std::memset(host_input, 0, sizeof(host_input));
  std::memset(&host_context, 0, sizeof(host_context));
  std::memset(const_cast<BYTE *>(movie_media_luma_delta_entries()), 0, 128);
  std::memset(const_cast<BYTE *>(movie_media_chroma_delta_entries()), 0, 256);
  const unsigned luma_indices[4] = {90, 52, 104, 80};
  const unsigned chroma_indices[2] = {180, 180};
  for (unsigned int i = 0; i != 4; ++i)
    const_cast<BYTE *>(movie_media_luma_delta_entries())[luma_indices[i]] = 0x10;
  for (unsigned int i = 0; i != 2; ++i)
    const_cast<BYTE *>(movie_media_chroma_delta_entries())[chroma_indices[i]] = 0x10;
  host_input[3] = 0xB4;
  host_context.predictor_4f8 = 10;
  host_context.predictor_4fc = 20;
  host_context.predictor_500 = 30;
  DWORD bit_position = 0;
  host_transform_index = 0;
  transform_calls = 0;
  const MediaTransformRegisterContinuation final_state =
      sub_5cf2f0(host_input, bit_position, host_context);
  assert(final_state.edi_coefficient_index == 0);
  assert(host_context.predictor_4f8 == 14);
  assert(host_context.predictor_4fc == 21);
  assert(host_context.predictor_500 == 31);
  assert(transform_calls == 6);
}
