// Shared typed movie-decoder storage and helper interfaces.
#pragma once

#include "movie_media.h"
#include "movie_decoder_records.h"
#include "movie_decoder_bit_state.h"
#include <stdint.h>
#include <stddef.h>

// Minimum transform working window reached by the 5CFAED/5D02CC schedules:
// [009C5000,009C581C). The first 0x600 bytes are block/intermediate words;
// the final 0x21C bytes are reused as strided transform output and butterfly
// input. These are access-window roles, not claims about one original
// allocation or a codec format.
struct MediaTransformWorkspace {
  int block_and_intermediate_words[384]; // 009C5000..009C55FF
  int row_and_butterfly_words[135];      // 009C5600..009C581B
};
extern MediaTransformWorkspace movie_media_transform_workspace;

// Shared coefficient/transform lookup bytes consumed by the transform-path
// reader and the six-block coefficient decoder. This source owner provides a
// selected access window beginning at 009C9594; its size reaches the highest
// proven access but does not establish the original object's upper bound or
// allocation. Indexed DWORD tables overlap byte lookups, so separate alias
// arrays would introduce unsupported ownership and indexing assumptions.
struct MediaCoefficientPredictorViewLayout {
  DWORD preceding_lookup_words[229]; // [009C9594,009C9928)
  BYTE luma_delta_entries[128];      // [009C9928,009C99A8), lookup index 0..127
  BYTE chroma_delta_entries[256];    // [009C99A8,009C9AA8), lookup index 0..255
};
struct MediaCoefficientTableStorage {
  DWORD words[325]; // selected access/initializer window [009C9594,009C9AA8)
};
extern MediaCoefficientTableStorage movie_media_coefficient_tables;
static const BYTE *movie_media_luma_delta_entries() {
  return reinterpret_cast<const BYTE *>(
      movie_media_coefficient_tables.words) +
      offsetof(MediaCoefficientPredictorViewLayout, luma_delta_entries);
}
static const BYTE *movie_media_chroma_delta_entries() {
  return reinterpret_cast<const BYTE *>(
      movie_media_coefficient_tables.words) +
      offsetof(MediaCoefficientPredictorViewLayout, chroma_delta_entries);
}

// Composite source storage for windows used by 5D35B0, 5D2E70 and the row
// packer. The first two byte windows are fully covered by observed producer
// fill widths and byte transform accesses; these are touched extents, not an
// assertion of one original allocation. The three tail dwords are separate
// observed cells at consecutive addresses; their behavioral names remain
// provisional.
struct MediaDecoderProcessStorage {
  BYTE narrow_block_output_bytes[0x200]; // [009B2D64,009B2F64)
  BYTE wide_block_output_bytes[0x400];   // [009B2F64,009B3364)
  uint32_t selected_record_mode_3364;    // [009B3364,009B3368)
  uint32_t plane_allocation_gate_3368;   // [009B3368,009B336C)
  uint32_t decoder_table_init_gate_336c; // [009B336C,009B3370)
};
extern MediaDecoderProcessStorage movie_media_decoder_process_storage;

// Composite contiguous access windows used by 5CE85C/5D35B0. Member widths
// follow observed reads/writes, including the 0x009C384C default-fill subview
// within primary_symbol_words[512]. The aggregate's outer sizeof is a source
// view of those accesses, not proof of a single original allocation boundary.
// The coefficient offset table is pinned initialized data; the other tables
// are runtime-built/used windows and need one shared definition.
typedef MovieDecoderBitState MediaDecoderFrameBitState;

struct MediaDecoderTableStorage {
  int32_t decoded_coefficient_words[64];            // [009C314C,009C324C)
  int32_t scale_source_words[64];                   // [009C324C,009C334C)
  int32_t coefficient_position_byte_offsets[64];    // [009C334C,009C344C)
  uint32_t primary_symbol_words[512];               // [009C344C,009C3C4C)
  uint32_t secondary_symbol_words_a[256];           // [009C3C4C,009C404C)
  uint32_t secondary_symbol_words_b[256];           // [009C404C,009C444C)
  uint32_t block_prefix_words[64];                  // [009C444C,009C454C)
  int32_t decoded_coefficient_scale_words[64];      // [009C454C,009C464C)
  MediaDecoderFrameBitState frame_bit_state;        // [009C464C,009C4658)
};
extern MediaDecoderTableStorage movie_media_decoder_tables;

typedef char media_transform_workspace_window_size_check[
    sizeof(MediaTransformWorkspace) == 0x81c ? 1 : -1];
typedef char media_coefficient_access_window_size_check[
    sizeof(MediaCoefficientTableStorage) == 0x514 ? 1 : -1];
typedef char media_predictor_luma_view_offset_check[
    offsetof(MediaCoefficientPredictorViewLayout, luma_delta_entries) == 0x394 ? 1 : -1];
typedef char media_predictor_chroma_view_offset_check[
    offsetof(MediaCoefficientPredictorViewLayout, chroma_delta_entries) == 0x414 ? 1 : -1];
typedef char media_predictor_views_size_check[
    sizeof(MediaCoefficientPredictorViewLayout) == 0x514 ? 1 : -1];
typedef char media_decoder_process_access_window_size_check[
    sizeof(MediaDecoderProcessStorage) == 0x60c ? 1 : -1];
typedef char media_decoder_table_access_window_size_check[
    sizeof(MediaDecoderTableStorage) == 0x150c ? 1 : -1];
typedef char media_decoder_frame_bit_state_size_check[
    sizeof(MediaDecoderFrameBitState) == 0x0c ? 1 : -1];

// Selected statically initialized/accessed table window. Its source aggregate
// size is required for the local type, but does not independently prove the
// original allocation boundary.
struct MediaDecoderStaticStorage {
  MediaDecoderCodebookRecord codebook_records[222];
  int32_t transform_product_words[65];
};
extern MediaDecoderStaticStorage movie_media_decoder_static_storage;

// Minimum shared IDCT scratch/control access window. Pointer representation
// remains a source-owned 32-bit cell read and written through memcpy.
struct MediaDecoderIdctStorage {
  DWORD output_pitch_bytes;
  DWORD current_output_pointer_bits;
  DWORD butterfly_fp_scratch_words[4];
  DWORD constant_words[5];
  int intermediate_words[72];
};
extern MediaDecoderIdctStorage movie_media_idct_storage;

typedef char media_decoder_static_initializer_window_size_check[
    sizeof(MediaDecoderStaticStorage) == 0xee4 ? 1 : -1];
typedef char media_decoder_idct_access_window_size_check[
    sizeof(MediaDecoderIdctStorage) == 0x14c ? 1 : -1];
typedef char media_decoder_pointer_slot_width_check[
    sizeof(void *) == sizeof(DWORD) ? 1 : -1];

// Native accesses establish these two dwords inside MediaRuntime's currently
// opaque +0x610..+0x637 interval. Proposed member delta (offsets from runtime):
//   +0x610..617 remain unresolved
//   DWORD transform_extent_618; DWORD transform_count_61c;
//   +0x620..637 remain unresolved
// This proposal does not claim semantics beyond the observed stores/loop test.

void __cdecl OutputPixelRow(MediaConversionTableStorage &tables,
                            int *destination,
                            const uint32_t *component_source,
                            const uint32_t *chroma_source);
void __cdecl OutputPixelRow16(MediaConversionTableStorage &tables,
                              short *destination,
                              const uint32_t *component_source,
                              const uint32_t *chroma_source);

// Normalized body of the 16-call row packer at 005CEA09.  The original
// routine seeds ESI/EDI/EBP from these explicit pointer arguments and then
// advances them through sixteen calls to the adjacent 005CEB12 helper.
void __cdecl sub_5cea09(const BYTE *component_rows,
                        const BYTE *chroma_rows,
                        BYTE *destination, int row_pitch_bytes);

// Shared concrete consumers used by the two native macroblock decoders.
void movie_media_transform_block_pair(int phase);
void movie_media_write_macroblock_rows(unsigned char *destination,
                                       int row_pitch_bytes,
                                       int sixteen_bit);

// Semantic decoder bodies normalized from the shared-register 5D0AAB/5D0D82
// flow. Six block outputs occupy observed interleaved positions in the one
// caller-owned 0x800-byte scratch frame.
void movie_media_decode_six_coefficient_blocks(
    MediaDecoderContext *context, DWORD *scratch_words);
DWORD __cdecl sub_5d0aab(MediaDecoderContext *context, DWORD *scratch_words,
                         const BYTE *input, DWORD bit_position);
DWORD __cdecl sub_5d0d82(MediaDecoderContext *context, DWORD predictor_value,
                         const BYTE *input, BYTE *destination,
                         int row_pitch_bytes, DWORD bit_position);

extern "C" void __cdecl sub_5d29e1(int *source, int *destination);
extern "C" void __cdecl sub_5d2c0a(int *destination, int *source);

// Existing product helper ABIs. Their definitions stay in the current owners.
struct MediaTransformRegisterContinuation {
  const BYTE *esi_cursor;       // 005CF150's shared reader cursor on return
  DWORD ebp_accumulator;        // shared bit accumulator returned in EBP
  LONG ecx_available_bits;     // shared bit count returned in ECX
  int edi_coefficient_index;   // local transform index returned in EDI
};
MediaTransformRegisterContinuation __cdecl sub_5cf150();
MediaTransformRegisterContinuation __cdecl sub_5cf2f0(
    const BYTE *bit_buffer, DWORD bit_position,
    MediaDecoderContext &context);
extern "C" void __cdecl sub_5d29e1(int *source, int *destination);
extern "C" void __cdecl sub_5d2b20(int *destination, int *source);

void __cdecl sub_5cfaed(unsigned char *compressed, BYTE *destination,
                        int row_pitch_bytes);
void __cdecl sub_5d02cc(unsigned char *compressed, void *destination,
                        int row_pitch_bytes);

extern "C" void __fastcall sub_5cd750(MediaRuntime *runtime,
                                      BYTE *packet);
