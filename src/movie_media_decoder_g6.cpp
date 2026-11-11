/*
 * OpenSMACX - an open source clone of Sid Meier's Alpha Centauri.
 * Copyright (C) 2013-2021 Brendan Casey
 *
 * OpenSMACX is free software: you can redistribute it and / or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * OpenSMACX is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with OpenSMACX. If not, see <http://www.gnu.org/licenses/>.
 */

// INFERRED BOUNDARY, not a discovered historical filename. The movie
// decoder's C functions 0x005D2E70..0x005D4231 carry Pentium Pro (/G6)
// instruction scheduling; their neighbour 0x005D2E00 measures exact only
// without /G6, so the contribution boundary lies between 0x005D2E6E and
// 0x005D2E70. These six owners were split out of movie_media_decoder.cpp
// so that they compile with the evidenced /G6 recipe. Source order follows
// image address order.
#include "stdafx.h"
#include "movie_media.h"
#include "general.h"
#include "movie_decoder_owner.h"
#include "movie_decoder_huffman.h"

#include <cstring>
#include <stdlib.h>

// Hand-written 16.16 fixed multiply with carry rounding (imul; shl/shr; adc),
// the same asm helper movie_media.cpp defines (identical inline definition).
#pragma warning(disable : 4035)
inline int media_fixed_mul(int factor, int value) {
  __asm {
    push edx
    mov edx, factor
    mov eax, value
    imul edx
    shl edx, 16
    shr eax, 16
    adc eax, edx
    pop edx
  }
}
#pragma warning(default : 4035)

// Naked owners assembled in movie_media_decoder.cpp.
extern "C" void __cdecl sub_5d271d(unsigned int *src, unsigned char *dst,
                                   int count);
extern "C" int __cdecl sub_5d427c(unsigned short *samples, unsigned char *steps,
                                  int a3);

// Existing product helper is defined in movie_media_decoder.cpp.
extern void __cdecl sub_5d2761(unsigned int *source,
                               unsigned int *destination, int count);

// WORKAROUND(decl): the image calls 0x005D2E70's coefficient decoder
// (0x005CE85C) with no argument - that hand-written routine reads the shared
// reader directly - while the shared header declares it with a State&.
static inline int movie_decode_coefficients() {
  return reinterpret_cast<int(__cdecl *)()>(
      &movie_decoder_huffman::decode_coefficients)();
}

// Mapped product helpers defined in movie_media_decoder.cpp. Their last
// argument is declared `char` there, but this caller pushes the full
// `(code >> 22) - 0x40` word (sar/sub/push, no byte narrowing).
// WORKAROUND(decl): int-typed views of the two char-argument helpers.
extern "C" void __fastcall sub_5d2d20(char *src, int mode, int stride,
                                      char *dst, int adjust);
void __fastcall sub_5d2e00(char *src, int byte_offset, int stride,
                           char *dst, char adjust);
typedef void(__fastcall *MovieChromaDelta)(char *src, int byte_offset,
                                           int stride, char *dst, int adjust);

// The six-block decoder's shared bit reader, spelled as the original's
// macros: every step reads and writes the shared reader fields directly.
// The bit count compares unsigned (jae) in every refill.
#define MOVIE_BITS_REFILL(bits)                                              \
  if (static_cast<DWORD>((bits)->available_bits) < 0x10) {                   \
    (bits)->accumulator |=                                                   \
        static_cast<DWORD>(                                                  \
            *reinterpret_cast<const unsigned short *>((bits)->cursor))       \
        << (0x10 - (bits)->available_bits);                                  \
    (bits)->available_bits += 0x10;                                          \
    (bits)->cursor += 2;                                                     \
  }
#define MOVIE_BITS_CONSUME(bits, n)                                          \
  {                                                                          \
    (bits)->accumulator <<= (n);                                             \
    (bits)->available_bits -= (n);                                           \
    MOVIE_BITS_REFILL(bits)                                                  \
  }
#define MOVIE_BLOCK_CODE(bits, out)                                          \
  {                                                                          \
    code = movie_media_decoder_tables                                        \
               .block_prefix_words[(bits)->accumulator >> 26];               \
    count = code & 0xff;                                                     \
    (bits)->accumulator <<= count;                                           \
    (bits)->available_bits -= count;                                         \
    MOVIE_BITS_REFILL(bits)                                                  \
    out = code >> 22;                                                        \
  }
#define MOVIE_FILL_BLOCK(block, row_words)                                   \
  {                                                                          \
    const int fill = movie_media_decoder_tables.decoded_coefficient_words[0];\
    int *row = reinterpret_cast<int *>(block);                               \
    for (int rows = 8; rows != 0; --rows) {                                  \
      row[0] = fill;                                                         \
      row[1] = fill;                                                         \
      row[2] = fill;                                                         \
      row[3] = fill;                                                         \
      row[4] = fill;                                                         \
      row[5] = fill;                                                         \
      row[6] = fill;                                                         \
      row[7] = fill;                                                         \
      row += (row_words);                                                    \
    }                                                                        \
  }
#define MOVIE_LUMA_BLOCK(bit, block, source)                                 \
  if ((flags & (bit)) == 0) {                                                \
    if (movie_decode_coefficients() == 1) {                                  \
      MOVIE_FILL_BLOCK(block, 0x10)                                          \
    } else {                                                                 \
      sub_5cef8a(block, 0x10);                                               \
    }                                                                        \
  } else {                                                                   \
    MOVIE_BLOCK_CODE(bits, adjust)                                           \
    sub_5d2d20(reinterpret_cast<char *>(source), delta_mode, row_width,      \
               reinterpret_cast<char *>(block),                              \
               adjust - 0x40);                                               \
  }
#define MOVIE_CHROMA_BLOCK(bit, block, plane)                                \
  if ((flags & (bit)) == 0) {                                                \
    if (movie_decode_coefficients() == 1) {                                  \
      MOVIE_FILL_BLOCK(block, 8)                                             \
    } else {                                                                 \
      sub_5cef8a(block, 8);                                                  \
    }                                                                        \
  } else {                                                                   \
    MOVIE_BLOCK_CODE(bits, adjust)                                           \
    reinterpret_cast<MovieChromaDelta>(&sub_5d2e00)(                         \
               reinterpret_cast<char *>(coefficient_block), (plane),         \
               row_width, reinterpret_cast<char *>(block),                   \
               adjust - 0x40);                                               \
  }


// Mapped original owner 005D2E70: one 16x16 macroblock. The optional header
// (record mode) gives six block flags and a motion offset; each flagged block
// is a delta copy from the coefficient plane, each other block is decoded by
// 0x005CE85C and either filled with its DC value or inverse-transformed. The
// residual is the reader's stores: this TU reaches the reader fields through
// movie_media_decoder_tables, which aliases the block-prefix table, so VC6
// keeps four refill stores the image drops (separate reader globals measured
// 517/517 instructions in a stand-alone experiment).
// ORIGINAL: 0x005D2E70 sub_5d2e70 0x005D2E70-0x005D35A4
// RECOVERY_BINDING: v1 {"evidence": ["recovery/tasks/movie-decoder-owner-oct5.json", "pinned caller/body and shared-storage review; fresh VC6 public symbol", "Seed extent is 0x734 bytes. Preserve the 12-byte interval [0x005D35A4,0x005D35B0) as unresolved; do not extend this binding to the next mapped function. Normalize the original ECX/EDX/stack flow to typed pointers and one stack width."], "reason": "Faithful semantic media decoder owner recovery with normalized custom-register helper interfaces. Original identities, spans and historical findings retained; no matching, runtime or game acceptance granted."}
// symbol    ?sub_5d2e70@@YIXPAE0H@Z
// TRIED: MISMATCH #0 'sub' vs 'push' (prologue itself diverges). JPEG- style Huffman bitstream decoder for 6 DCT-ish blocks (4 at stride-0x10, 2 at stride-8), each either memset from a fill value or decoded via a shared Huffman-table walk; factored the identical block body into decode_or_fill()/fill_block() and the repeated "refill bit buffer if count<0x10" idiom into a REFILL macro, both landed in this file (not a separate helper unit). Retargeted the signature from the emitter's own guessed `__stdcall(int)` to `void __fastcall(int,int,int)` to match Ghidra's ecx/edx/stack-arg reading and the `ret 4` epilogue (one stack-passed int, callee-cleaned) - did not close the prologue gap. `sub_5cef8a`/`sub_5cea09` are declared 0-arg/ unknown-arity in the scaffold but called with 2/4 args in the disassembly; bypassed via raw __stdcall function-pointer casts to the same addresses rather than editing the conflicting extern declarations.
void __fastcall sub_5d2e70(BYTE *coefficient_block,
                           BYTE *temporary_block, int row_width) {
  MediaDecoderFrameBitState *const bits =
      &movie_media_decoder_tables.frame_bit_state;
  BYTE *const luma = movie_media_decoder_process_storage.wide_block_output_bytes;
  BYTE *const chroma =
      movie_media_decoder_process_storage.narrow_block_output_bytes;
  int flags;
  int delta_mode;
  int code;
  int count;
  int adjust;

  row_width >>= 1;
  if (movie_media_decoder_process_storage.selected_record_mode_3364 == 0) {
    flags = 0;
  } else if ((bits->accumulator & 0xc0000000) == 0) {
    flags = 0;
    MOVIE_BITS_CONSUME(bits, 2)
  } else {
    int horizontal;
    int vertical;
    if ((bits->accumulator & 0x80000000) != 0) {
      flags = 0x3ff;
      MOVIE_BITS_CONSUME(bits, 1)
    } else {
      flags = bits->accumulator >> 24;
      MOVIE_BITS_CONSUME(bits, 8)
    }
    MOVIE_BLOCK_CODE(bits, horizontal)
    MOVIE_BLOCK_CODE(bits, vertical)
    coefficient_block = reinterpret_cast<BYTE *>(
        reinterpret_cast<DWORD *>(coefficient_block) +
        (vertical * row_width + (horizontal >> 1)));
    delta_mode = horizontal & 1;
  }

  MOVIE_LUMA_BLOCK(1, luma, coefficient_block)
  MOVIE_LUMA_BLOCK(2, luma + 0x20, coefficient_block + 0x10)
  MOVIE_LUMA_BLOCK(4, luma + 0x200, coefficient_block + row_width * 0x20)
  MOVIE_LUMA_BLOCK(8, luma + 0x220,
                   coefficient_block + row_width * 0x20 + 0x10)
  MOVIE_CHROMA_BLOCK(0x10, chroma, 1)
  MOVIE_CHROMA_BLOCK(0x20, chroma + 0x100, 0)

  sub_5cea09(luma, chroma, temporary_block, row_width * 4);
}

#undef MOVIE_CHROMA_BLOCK
#undef MOVIE_LUMA_BLOCK
#undef MOVIE_FILL_BLOCK
#undef MOVIE_BLOCK_CODE
#undef MOVIE_BITS_CONSUME
#undef MOVIE_BITS_REFILL

// Earlier recovery replaced the mapped 005D35B0 sentinel in movie_media.cpp.
// Its argument roles come from the native call-site stack/register flow; the
// current provisional integer-pointer signature is not carried forward.
// ORIGINAL: 0x005D35B0 sub_5d35b0 0x005D35B0-0x005D3980
// RECOVERY_BINDING: v1 {"evidence": ["recovery/tasks/movie-decoder-owner-oct5.json", "pinned caller/body and shared-storage review; fresh VC6 public symbol", "Native caller supplies runtime in ECX, compressed packet in EDX, then destination, pitch, and record kind on stack; do not preserve provisional integer-pointer casts or emitted mangling as ABI evidence."], "reason": "Faithful semantic media decoder owner recovery with normalized custom-register helper interfaces. Original identities, spans and historical findings retained; no matching, runtime or game acceptance granted."}
// symbol    ?sub_5d35b0@@YIHPAUMediaRuntime@@PBEPAEKH@Z
// TRIED: __fastcall(param_1,param_2) + 3 stack args (IDA's 3-arg __stdcall guess missed the two register params); the imul/shl/shr/adc fixed-point-multiply idiom is written as `__int64` products (VC6 keyword, not the forbidden `long long`) rather than replicating the exact carry sequence. MISMATCH #7, not chased.
int __fastcall sub_5d35b0(MediaRuntime *runtime,
                          const BYTE *compressed, BYTE *output,
                          DWORD output_pitch, int record_kind) {
  if (movie_media_decoder_process_storage.plane_allocation_gate_3368 == 0) {
    movie_media_decoder_process_storage.plane_allocation_gate_3368 = 1;
    runtime->decoder_coefficient_plane_654 = static_cast<BYTE *>(
        calloc(1, runtime->descriptor->output_width_64 *
                      runtime->descriptor->output_height_60 * 4));
    runtime->decoder_temporary_plane_a_658 = static_cast<BYTE *>(
        calloc(1, runtime->descriptor->output_width_64 *
                      runtime->descriptor->output_height_60 * 4));
    runtime->decoder_temporary_plane_b_65c = static_cast<BYTE *>(
        calloc(1, runtime->descriptor->output_width_64 *
                      runtime->descriptor->output_height_60 * 4));
  }

  const int record_mode = record_kind != 5;
  const int bias = static_cast<signed char>(compressed[0x0d]);
  if (movie_media_decoder_process_storage.decoder_table_init_gate_336c == 0) {
    uint32_t *const primary = movie_media_decoder_tables.primary_symbol_words;
    uint32_t *const secondary_a =
        movie_media_decoder_tables.secondary_symbol_words_a;
    uint32_t *const secondary_b =
        movie_media_decoder_tables.secondary_symbol_words_b;
    uint32_t *const block_prefix = movie_media_decoder_tables.block_prefix_words;
    int i;
    primary[0] = 0x0f;
    for (i = 1; i < 8; ++i) primary[i] = 0x1f;
    for (i = 8; i < 16; ++i) primary[i] = 0x2f;
    for (i = 256; i < 384; ++i) primary[i] = 0x3f;

    const MediaDecoderCodebookRecord *const records =
        movie_media_decoder_static_storage.codebook_records;
    for (i = 0; i < 94; ++i) {
      const int length = static_cast<int>(records[i].word_before_cursor);
      const int code = static_cast<int>(records[i].word_at_cursor);
      const int selector = static_cast<int>(records[i].selector_word_at_plus8);
      if ((selector & 0xfc00) != 0) {
        const int index = selector >> 7;
        const int count = 1 << (9 - length);
        const int value = (((code & 0xfc00) | (code << 16)) << 6) | length;
        for (int j = 0; j < count; ++j) primary[index + j] = value;
      } else {
        const int index = selector >> 2;
        const int count = 1 << (0x0e - length);
        const int value =
            (((code & 0xfc00) | (code << 16)) << 6) | (length - 6);
        for (int j = 0; j < count; ++j) secondary_b[index + j] = value;
      }
    }
    for (i = 94; i < 222; ++i) {
      const int length = static_cast<int>(records[i].word_before_cursor) + 8;
      const int code = static_cast<int>(records[i].word_at_cursor);
      const int selector = static_cast<int>(records[i].selector_word_at_plus8);
      if ((selector & 0x8000) == 0) {
        const int index = selector >> 7;
        const int count = 1 << (0x11 - length);
        const int value =
            (((code & 0xfc00) | (code << 16)) << 6) | (length - 9);
        for (int j = 0; j < count; ++j) secondary_a[index + j] = value;
      } else {
        const int index = selector >> 10;
        const int count = 1 << (0x0e - length);
        const int value =
            (((code & 0xfc00) | (code << 16)) << 6) | (length - 6);
        for (int j = 0; j < count; ++j) secondary_b[index + j] = value;
      }
    }

    for (i = 0; i < 32; ++i) block_prefix[i] = 1;
    uint32_t *prefix = block_prefix + 32;
    uint32_t step;
    for (step = 0x400000; step < 0x4400000; step += 0x400000)
      *prefix++ = step | 6;
    step = 0xfc000000;
    movie_media_decoder_process_storage.decoder_table_init_gate_336c = 1;
    *prefix = step | 6;
  }

  MediaDecoderFrameBitState *const bits =
      &movie_media_decoder_tables.frame_bit_state;
  const DWORD accumulator =
      (*reinterpret_cast<const unsigned short *>(compressed + 0x10) << 16) |
      *reinterpret_cast<const unsigned short *>(compressed + 0x12);
  compressed += 0x14;
  bits->available_bits = 0x20;
  bits->accumulator = accumulator;
  const int first_factor =
      movie_media_decoder_static_storage.transform_product_words[0] << 15;
  bits->cursor = compressed;
  movie_media_decoder_process_storage.selected_record_mode_3364 = record_mode;
  int32_t *const scale_source = movie_media_decoder_tables.scale_source_words;
  int32_t *const scale_output =
      movie_media_decoder_tables.decoded_coefficient_scale_words;
  scale_output[0] = media_fixed_mul(
      first_factor, movie_media_decoder_tables.scale_source_words[0]);
  for (int k = 1; k < 64; ++k) {
    scale_output[k] = media_fixed_mul(
        movie_media_decoder_static_storage.transform_product_words[k] * bias
            << 12,
        scale_source[k]);
  }

  DWORD *selected = reinterpret_cast<DWORD *>(
      record_kind == 7 ? runtime->decoder_temporary_plane_b_65c
                       : runtime->decoder_temporary_plane_a_658);
  BYTE *destination = output;
  for (DWORD y = 0; y < runtime->descriptor->output_height_60; y += 0x10) {
    for (DWORD x = 0; x < runtime->descriptor->output_width_64; x += 0x10) {
      const DWORD width = runtime->descriptor->output_width_64;
      const DWORD half = (width * y + x) >> 1;
      sub_5d2e70(reinterpret_cast<BYTE *>(
                     reinterpret_cast<DWORD *>(
                         runtime->decoder_coefficient_plane_654) + half),
                 reinterpret_cast<BYTE *>(selected + half), width);
    }
  }
  for (DWORD row = 0; row < runtime->descriptor->output_height_60 - 1; ++row) {
    sub_5d2761(reinterpret_cast<unsigned int *>(selected),
               reinterpret_cast<unsigned int *>(destination),
               runtime->descriptor->output_width_64 >> 1);
    destination += output_pitch;
    selected += runtime->descriptor->output_width_64 >> 1;
  }

  if (record_kind != 7) {
    BYTE *const coefficient = runtime->decoder_coefficient_plane_654;
    runtime->decoder_coefficient_plane_654 =
        runtime->decoder_temporary_plane_a_658;
    runtime->decoder_temporary_plane_a_658 = coefficient;
  }
  return 0;
}

// ORIGINAL: 0x005D3980 sub_5d3980 0x005D3980-0x005D3DF0 FILE
// symbol    ?sub_5d3980@@YIHPAUMediaRuntime@@PAE@Z
// TRIED: __fastcall(int *, unsigned int *) direct transcription of the Ghidra decompile (malloc/free node-buffer growth, 10-bit signed coefficient unpack, memcpy raw blocks, 2-bit nibble pixel expansion, two helper calls); diverges at insn #4 in the prologue register-save order, not in control flow
// working copy - scaffold materialised by --work
// size      1136 bytes
// prototype
// callers   1   call targets   4
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005D439F 0x005D4448 0x00644EF2 0x006470A6


// Original inline-assembly reader (VC6 never emits this neg/lea/shr form):
// loads the DWORD that ends at p+2 and keeps its high word, i.e. the
// little-endian 16-bit value at p. ECX is saved around the block.
static __forceinline DWORD movie_packet_read16(const unsigned char *p) {
    __asm {
        push ecx
        mov eax, p
        mov ecx, 2
        mov eax, dword ptr [eax + ecx - 4]
        neg ecx
        lea ecx, [ecx*8 + 0x20]
        shr eax, cl
        pop ecx
    }
}

int __fastcall sub_5d3980(MediaRuntime *runtime, unsigned char *packet) {
    int flagOK = 1;
    unsigned char *const previous_frame = runtime->owned_heap_63c;
    runtime->owned_heap_63c = runtime->owned_heap_638;
    runtime->owned_heap_638 = previous_frame;

    const unsigned char *p = packet;
    const DWORD motion_count = movie_packet_read16(p);
    runtime->decoder_packet_value_620 = motion_count;
    p = packet + 2;
    DWORD raw_count = movie_packet_read16(p);
    runtime->decoder_packet_value_624 = raw_count;
    p = packet + 4;
    const DWORD packed_count = movie_packet_read16(p);
    runtime->decoder_packet_value_628 = packed_count;
    p = packet + 6;
    const DWORD index_width = movie_packet_read16(p);
    runtime->decoder_packet_value_62c = index_width;

    if (motion_count != 0) {
        if (static_cast<int>(motion_count) > runtime->decoder_capacity_630) {
            if (runtime->motion_coordinates != 0) {
                free(runtime->motion_coordinates);
                runtime->motion_coordinates = 0;
                runtime->decoder_capacity_630 = 0;
            }
            if (runtime->motion_pixel_offsets != 0) {
                free(runtime->motion_pixel_offsets);
                runtime->motion_pixel_offsets = 0;
            }
        }
        if (runtime->motion_coordinates == 0) {
            runtime->motion_coordinates = static_cast<MediaMotionCoordinate *>(
                malloc(motion_count * 8));
            runtime->decoder_capacity_630 = motion_count;
        }
        if (runtime->motion_pixel_offsets == 0) {
            runtime->motion_pixel_offsets = static_cast<int *>(
                malloc(motion_count * 4));
            runtime->decoder_capacity_630 = motion_count;
        }
        if (runtime->motion_coordinates == 0) flagOK = 0;
        if (runtime->motion_pixel_offsets == 0) flagOK = 0;
    }

    int block_count = packed_count + raw_count;
    if (block_count != 0) {
        if (block_count > runtime->decoder_capacity_634 &&
            runtime->decoded_pixel_blocks != 0) {
            free(runtime->decoded_pixel_blocks);
            runtime->decoded_pixel_blocks = 0;
        }
        if (runtime->decoded_pixel_blocks == 0) {
            runtime->decoded_pixel_blocks =
                static_cast<MediaDecodedPixelBlock4x4 *>(malloc(block_count << 4));
            runtime->decoder_capacity_634 = block_count;
        }
        if (runtime->decoded_pixel_blocks == 0) return 0;
    }

    if (flagOK != 0) {
        const int header_bytes =
            (static_cast<int>(motion_count * 20 + 0x1f) >> 3) & ~3;
        const DWORD component_count = motion_count * 2;
        const unsigned char *data = packet + 0xc;
        DWORD bit_position = component_count * 10 - 10;
        int *component = reinterpret_cast<int *>(runtime->motion_coordinates) +
                         component_count;
        for (DWORD remaining = component_count; remaining > 0; --remaining) {
            int value = *reinterpret_cast<const DWORD *>(
                            data + (bit_position >> 3)) >> (bit_position & 7);
            bit_position -= 10;
            value &= 0x3ff;
            if (value & 0x200) value |= 0xfffffc00;
            *--component = value;
        }

        const int *const rows = runtime->motion_row_offsets +
                                runtime->descriptor->output_height_60;
        for (int i = 0; i < static_cast<int>(motion_count); ++i) {
            runtime->motion_pixel_offsets[i] =
                rows[runtime->motion_coordinates[i].y] +
                runtime->motion_coordinates[i].x;
        }

        data += header_bytes;
        raw_count <<= 4;
        if (raw_count != 0) {
            memcpy(runtime->decoded_pixel_blocks, data, raw_count);
        }
        const unsigned char *const packed_data = data + raw_count;
        const DWORD packed_bytes = packed_count * 8;
        if (packed_bytes != 0) {
            unsigned char *block =
                reinterpret_cast<unsigned char *>(runtime->decoded_pixel_blocks) +
                raw_count;
            const unsigned char *run = packed_data;
            for (int k = 0; k < static_cast<int>(packed_count); ++k) {
                DWORD selectors = *reinterpret_cast<const DWORD *>(run + 4);
                block[0xf] = run[selectors & 3];
                selectors >>= 2;
                block[0xe] = run[selectors & 3];
                selectors >>= 2;
                block[0xd] = run[selectors & 3];
                selectors >>= 2;
                block[0xc] = run[selectors & 3];
                selectors >>= 2;
                block[0xb] = run[selectors & 3];
                selectors >>= 2;
                block[0xa] = run[selectors & 3];
                selectors >>= 2;
                block[9] = run[selectors & 3];
                selectors >>= 2;
                block[8] = run[selectors & 3];
                selectors >>= 2;
                block[7] = run[selectors & 3];
                selectors >>= 2;
                block[6] = run[selectors & 3];
                selectors >>= 2;
                block[5] = run[selectors & 3];
                selectors >>= 2;
                block[4] = run[selectors & 3];
                selectors >>= 2;
                block[3] = run[selectors & 3];
                selectors >>= 2;
                block[2] = run[selectors & 3];
                selectors >>= 2;
                block[1] = run[selectors & 3];
                selectors >>= 2;
                block[0] = run[selectors & 3];
                run += 8;
                block += 0x10;
            }
        }

        const DWORD blocks = (runtime->descriptor->output_width_64 *
                              runtime->descriptor->output_height_60) >> 4;
        if ((index_width * blocks + 0x1f) & 0xffffffe0) {
            sub_5d439f(runtime->motion_block_indices, packed_data + packed_bytes,
                       blocks, index_width);
        }
        sub_5d4448(runtime->motion_block_indices, runtime->owned_heap_63c,
                   runtime->owned_heap_638,
                   runtime->decoder_packet_value_620,
                   runtime->motion_pixel_offsets,
                   runtime->decoded_pixel_blocks,
                   runtime->descriptor->output_width_64,
                   runtime->descriptor->output_width_64 >> 2,
                   runtime->descriptor->output_height_60 >> 2);
    }
    return flagOK;
}

// ORIGINAL: 0x005D3DF0 sub_5d3df0 0x005D3DF0-0x005D3F48 FILE
// symbol    ?sub_5d3df0@@YIHPAUMediaRuntime@@PAE@Z
// size      344 bytes
// prototype
// callers   ?   call targets   3
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005D427C 0x006470A6 0x00644EF2
// TRIED: __fastcall(this,a2) signature chosen from the ecx=this / edx=a2 register reads at entry; PROPOSAL below. Landed at 0.88 mnemonic similarity - remaining divergence is prologue register-save scheduling, not logic.
// REWRITTEN 2026-11-07 as plain field arithmetic (0.905). The `result` local
//        gives the image's `mov eax, ebx` return and stack-homed frame_base;
//        the residual is scheduling (xor ebx and mov eax,1 placement).
int __fastcall sub_5d3df0(MediaRuntime *runtime, unsigned char *frame_base) {
  int result = 0;
  const DWORD saved_index = runtime->unresolved_word_61c;
  // The image divides the aligned area as signed (cdq) below.
  const int allocation_size =
      (runtime->descriptor->output_width_64 *
           runtime->descriptor->output_height_60 + 0x0f) & 0xfffffff0;

  if (runtime->owned_heap_638 != 0) {
    free(runtime->owned_heap_638);
    runtime->owned_heap_638 = 0;
  }
  if (runtime->owned_heap_63c != 0) {
    free(runtime->owned_heap_63c);
    runtime->owned_heap_63c = 0;
  }
  if (runtime->motion_row_offsets != 0) {
    free(runtime->motion_row_offsets);
    runtime->motion_row_offsets = 0;
  }
  if (runtime->motion_block_indices != 0) {
    free(runtime->motion_block_indices);
    runtime->motion_block_indices = 0;
  }

  runtime->owned_heap_638 = static_cast<unsigned char *>(malloc(allocation_size));
  runtime->owned_heap_63c = static_cast<unsigned char *>(malloc(allocation_size));
  runtime->motion_row_offsets = static_cast<int *>(
      malloc(runtime->descriptor->output_height_60 << 3));
  runtime->motion_block_indices = static_cast<unsigned int *>(
      malloc(allocation_size / 16 << 2));

  if (runtime->owned_heap_638 != 0 && runtime->owned_heap_63c != 0 &&
      runtime->motion_row_offsets != 0 && runtime->motion_block_indices != 0) {
    int value = -static_cast<int>(runtime->descriptor->output_width_64 *
                                  runtime->descriptor->output_height_60);
    for (DWORD index = 0;
         index < runtime->descriptor->output_height_60 * 2; ++index) {
      runtime->motion_row_offsets[index] = value;
      value += runtime->descriptor->output_width_64;
    }
    sub_5d427c(reinterpret_cast<unsigned short *>(
                   frame_base + (saved_index + 4) * 3),
               runtime->owned_heap_638, 1);
    result = 1;
  }
  return result;
}

// ORIGINAL: 0x005D3F50 sub_5d3f50 0x005D3F50-0x005D4014 FILE
// symbol    @sub_5d3f50@4
// TRIED: signature cdecl() -> fastcall(int *param_1); evidence: ecx read directly at entry (dereferenced once for an object pointer, re-read every loop iteration), no stack access, plain `ret` (no callee-pop). Dropped Ghidra's explicit `& 0x1f` shift masks per the known lever (extra `and`); still MISMATCH at #4, likely register-allocation/prologue shape for the three precomputed shift amounts.
// working copy - scaffold materialised by --work
// size      196 bytes
// prototype
// callers   1   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)


extern "C" void __fastcall sub_5d3f50(MediaRuntime *runtime) {
    const unsigned int shift_red = 8 - runtime->descriptor->pixel_format.red_bits;
    const unsigned int shift_green = 8 - runtime->descriptor->pixel_format.green_bits;
    const unsigned int shift_blue = 8 - runtime->descriptor->pixel_format.blue_bits;

    for (int index = 0; index < 256; ++index) {
        MediaDescriptor *const d = runtime->descriptor;
        unsigned int result = (d->palette_rgb_triplets_b0[index * 3 + 1] >> shift_green) << d->pixel_format.green_shift;
        result |= (d->palette_rgb_triplets_b0[index * 3] >> shift_red) << d->pixel_format.red_shift;
        result |= (d->palette_rgb_triplets_b0[index * 3 + 2] >> shift_blue) << d->pixel_format.blue_shift;
        if (d->flags & 0x40)
            result |= result << 0x10;
        d->palette_lookup_3b0.entries[index] = result;
    }
}

// ORIGINAL: 0x005D4020 sub_5d4020 0x005D4020-0x005D4231 FILE
// symbol    ?sub_5d4020@@YIXPAUMediaRuntime@@PAE1H@Z
// REWRITTEN 2026-11-07 from the image (0.93): for-loops, intrinsic memcpy
//        rows, flags&0x40 picks the dword path first. Residual: the image
//        homes dest on the stack in the 8-bit path; here it stays in edx.
// TRIED: not chased past the first instruction (MISMATCH #0 'push' vs 'sub', 62% mnemonic similarity) - __fastcall(this, dest, int, int) reproduces the `ret 8` pop count and the edx-as-second- register-arg evidence (no `mov ebp,esp` frame, `[esp+0x10]` live from entry with no stack spill) but the prologue's register-save order still differs. Structurally follows the raw disassembly's three shapes (mode==8 raw copy via sub_5d271d, else 2-byte-per-pixel CLUT unrolled x8, else 4-byte-per-pixel CLUT unrolled x8).
// working copy - scaffold materialised by --work
// size      529 bytes
// prototype
// callers   1   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005D271D


void __fastcall sub_5d4020(MediaRuntime *runtime, unsigned char *dest,
                           unsigned char *unusedStackArg, int destStride) {
    unsigned char *src = runtime->owned_heap_638;
    DWORD row;
    (void)unusedStackArg;
    if (runtime->descriptor->pixel_format.total_component_bits == 8) {
        unsigned char *d8 = dest;
        if ((runtime->descriptor->flags & 0x40) != 0) {
            sub_5d271d(reinterpret_cast<unsigned int *>(src), d8,
                       static_cast<int>(runtime->descriptor->output_width_64 *
                                        runtime->descriptor->output_height_60));
            return;
        }
        for (row = 0; row < runtime->descriptor->output_height_60; ++row) {
            memcpy(d8, src, runtime->descriptor->output_width_64);
            src += runtime->descriptor->output_width_64;
            d8 += (destStride >> 2) * 4;
        }
        return;
    }

    const DWORD *const clut = runtime->descriptor->palette_lookup_3b0.entries;
    if ((runtime->descriptor->flags & 0x40) != 0) {
        DWORD *out = reinterpret_cast<DWORD *>(dest);
        for (row = 0; row < runtime->descriptor->output_height_60; ++row) {
            for (DWORD column = 0;
                 column < runtime->descriptor->output_width_64; column += 8) {
                out[0] = clut[src[0]];
                out[1] = clut[src[1]];
                out[2] = clut[src[2]];
                out[3] = clut[src[3]];
                out[4] = clut[src[4]];
                out[5] = clut[src[5]];
                out[6] = clut[src[6]];
                out[7] = clut[src[7]];
                src += 8;
                out += 8;
            }
            out += (destStride - runtime->descriptor->output_width_64 * 4) >> 2;
        }
        return;
    }

    unsigned short *out = reinterpret_cast<unsigned short *>(dest);
    for (row = 0; row < runtime->descriptor->output_height_60; ++row) {
        for (DWORD column = 0;
             column < runtime->descriptor->output_width_64; column += 8) {
            out[0] = static_cast<unsigned short>(clut[src[0]]);
            out[1] = static_cast<unsigned short>(clut[src[1]]);
            out[2] = static_cast<unsigned short>(clut[src[2]]);
            out[3] = static_cast<unsigned short>(clut[src[3]]);
            out[4] = static_cast<unsigned short>(clut[src[4]]);
            out[5] = static_cast<unsigned short>(clut[src[5]]);
            out[6] = static_cast<unsigned short>(clut[src[6]]);
            out[7] = static_cast<unsigned short>(clut[src[7]]);
            src += 8;
            out += 8;
        }
        out += (destStride - runtime->descriptor->output_width_64 * 2) >> 1;
    }
}
