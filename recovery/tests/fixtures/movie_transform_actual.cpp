// Host fixture; test injects the exact source-owned 5CF150 definition block.
#include <cassert>
#include <cstring>
#include <stdint.h>

typedef unsigned char BYTE;
typedef uint32_t DWORD;
typedef int32_t LONG;
#define __cdecl
#define __int64 long long

struct MediaTransformWorkspace {
  int block_and_intermediate_words[384];
  int row_and_butterfly_words[135];
};
struct MediaTransformBitState {
  const BYTE *cursor;
  DWORD accumulator;
  LONG available_bits;
};
struct MediaCoefficientTableStorage { DWORD words[325]; };
struct MediaConversionTableStorage {
  DWORD block_scale_entries[64];
  MediaTransformBitState transform_bit_state;
};
struct MediaTransformRegisterContinuation {
  const BYTE *esi_cursor;
  DWORD ebp_accumulator;
  LONG ecx_available_bits;
  int edi_coefficient_index;
};
static MediaTransformWorkspace movie_media_transform_workspace;
static MediaCoefficientTableStorage movie_media_coefficient_tables;
static MediaConversionTableStorage movie_media_conversion_tables;

/*__ACTUAL_OWNER__*/

static void initialize_tables() {
  std::memset(&movie_media_transform_workspace, 0,
              sizeof(movie_media_transform_workspace));
  std::memset(&movie_media_coefficient_tables, 0,
              sizeof(movie_media_coefficient_tables));
  for (int i = 0; i != 135; ++i)
    movie_media_transform_workspace.row_and_butterfly_words[i] = 0x77777777;
  for (int i = 0; i != 64; ++i)
    movie_media_conversion_tables.block_scale_entries[i] = i + 1;
}

static MediaTransformRegisterContinuation run_with_code(unsigned code) {
  static BYTE bytes[96];
  std::memset(bytes, 0, sizeof(bytes));
  movie_media_conversion_tables.transform_bit_state.cursor = bytes;
  movie_media_conversion_tables.transform_bit_state.accumulator = code << 8;
  movie_media_conversion_tables.transform_bit_state.available_bits = 8;
  for (DWORD i = 0; i != 64; ++i) {
    movie_media_coefficient_tables.words[5 + i] = i;
    movie_media_coefficient_tables.words[4 + i] = i;
    movie_media_coefficient_tables.words[3 + i] = i;
  }
  movie_media_coefficient_tables.words[4 + 63] = 64;
  movie_media_coefficient_tables.words[3 + 63] = 66;
  movie_media_coefficient_tables.words[4 + 62] = 65;
  movie_media_coefficient_tables.words[3 + 62] = 67;
  return sub_5cf150();
}

static void check_one_run(unsigned key) {
  initialize_tables();
  const MediaTransformRegisterContinuation state =
      run_with_code(key == 1 ? 9 : 13);
  DWORD expected[135];
  for (int i = 0; i != 135; ++i) expected[i] = 0x77777777;
  expected[0] = 0;

  DWORD index = 63;
  const DWORD current = movie_media_coefficient_tables.words[5 + index];
  expected[current] = 0;
  if (key == 1) {
    expected[movie_media_coefficient_tables.words[4 + index]] = 0;
    index -= 2;
    ++index;
  } else {
    DWORD ebx = movie_media_coefficient_tables.words[4 + index];
    --index;
    expected[ebx] = 0;
    expected[movie_media_coefficient_tables.words[4 + index]] = 0;
    ebx = movie_media_coefficient_tables.words[3 + index];
    (void)ebx; // loaded as the next native EBX; this one-element run ends.
    index -= 2;
    ++index;
  }
  --index; // shared 005CF2C4 extended refill decrements EDI.
  while (index > 0) {
    expected[movie_media_coefficient_tables.words[5 + index]] = 0;
    --index; // zero-filled input takes the native common zero arm.
  }
  assert(state.edi_coefficient_index == 0);
  for (int i = 0; i != 135; ++i)
    assert(movie_media_transform_workspace.row_and_butterfly_words[i] ==
           static_cast<int>(expected[i]));
}

int main() {
  const DWORD codes[] = {0, 2, 4, 6, 3, 7, 9, 13, 0xff};
  for (unsigned int i = 0; i != sizeof(codes) / sizeof(codes[0]); ++i) {
    initialize_tables();
    const MediaTransformRegisterContinuation state = run_with_code(codes[i]);
    assert(state.esi_cursor != 0);
    assert(state.edi_coefficient_index <= 0);
    assert(state.ecx_available_bits >= 8);
    assert(movie_media_conversion_tables.transform_bit_state.cursor ==
           state.esi_cursor);
    assert(movie_media_conversion_tables.transform_bit_state.accumulator ==
           state.ebp_accumulator);
    assert(movie_media_conversion_tables.transform_bit_state.available_bits ==
           state.ecx_available_bits);
  }
  check_one_run(1);
  check_one_run(5);
}
