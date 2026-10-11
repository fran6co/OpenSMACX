#pragma once

#include <stddef.h>
#include <stdint.h>
#include "movie_decoder_bit_state.h"
#include "movie_decoder_records.h"

namespace movie_decoder_frame {

typedef MediaDecoderCodebookRecord CodebookRecord;

struct Tables {
  uint32_t *primary;       // 512 DWORD owner at 009C344C
  uint32_t *secondary_a;   // 256 DWORD owner at 009C3C4C
  uint32_t *secondary_b;   // 256 DWORD owner at 009C404C
  uint32_t *block_prefix;  // 64 DWORD owner at 009C444C
  int32_t *scale_source;   // 64 DWORD owner at 009C324C
  int32_t *scale_output;   // 64 DWORD owner at 009C454C
};

struct RuntimePlanes {
  uint8_t **coefficient;
  uint8_t **temporary_a;
  uint8_t **temporary_b;
};

typedef ::MovieDecoderBitState BitState;

struct Callbacks {
  void *(*calloc_bytes)(size_t bytes);
  void (*decode_six_blocks)(void *decoder_context,
                            uint8_t *coefficient_block,
                            uint8_t *destination_block, int width);
  void (*pack_plane_row)(const uint32_t *source,
                         uint32_t *destination, int count);
};

struct FrameArgs {
  RuntimePlanes *planes;
  uint32_t *allocation_gate;
  uint32_t *record_mode;
  uint32_t *table_init_gate;
  uint32_t width;
  uint32_t height;
  const uint8_t *compressed;
  BitState *bit_state;
  uint8_t *destination;
  uint32_t output_pitch;
  int record_kind;
  const CodebookRecord *codebook_records; // 94 + 128 records
  const int32_t *transform_products; // 0x696938 then 0x69693C..+0xFC
  Tables tables;
  void *decoder_context; // typed adapter owns the table/state/scratch views
  Callbacks callbacks;
};

// Exact native table-writing loops, parameterized by source-owned records and
// destination-owned views. The untouched table bytes retain their preexisting
// initialized representation; this routine does not synthesize defaults.
void initialize_codebooks(Tables &tables, const CodebookRecord records[222]);

// Frame decoder shared by the mapped 005D35B0 owner and its typed callbacks.
int decode_frame(FrameArgs &args);

} // namespace movie_decoder_frame
