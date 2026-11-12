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

// INFERRED BOUNDARY, not a discovered historical filename. The macroblock
// decoder 0x005D2E70 and its frame driver 0x005D35B0 (its only caller) sit
// between the non-/G6 owner 0x005D2E00 (movie_media_decoder.cpp) and the /G6
// owners from 0x005D3980 (movie_media_decoder_g6.cpp). Without /G6,
// 0x005D2E70 measures 515/517 matching instructions with identical mnemonics
// against 388/517 with /G6, and 0x005D35B0's diff shrinks from 61 to 37
// unmatched lines (its prologue defers `push edi` past the gate test like
// the non-/G6 compile); 0x005D3980 instead prefers /G6 (0.96 against 0.74
// mnemonic similarity). Both therefore belong to a non-/G6 contribution,
// most likely the same unit as their neighbour 0x005D2E00; they compile here
// with that unit's recipe until the decoder unit's ownership is regrouped.
#include "stdafx.h"
#include "movie_media.h"
#include "general.h"
#include "movie_decoder_owner.h"
#include "movie_decoder_huffman.h"

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

// The six-block decoder's shared bit reader, spelled as macros: every step
// reads and writes the shared reader fields directly through the function's
// three field references (bit_cursor, bit_accumulator, bit_count). The bit
// count compares unsigned (jae) in every refill.
#define MOVIE_BITS_REFILL()                                                  \
  if (static_cast<DWORD>(bit_count) < 0x10) {                                \
    bit_accumulator |=                                                       \
        static_cast<DWORD>(                                                  \
            *reinterpret_cast<const unsigned short *>(bit_cursor))           \
        << (0x10 - bit_count);                                               \
    bit_count += 0x10;                                                       \
    bit_cursor += 2;                                                         \
  }
#define MOVIE_BITS_CONSUME(n)                                                \
  {                                                                          \
    bit_accumulator <<= (n);                                                 \
    bit_count -= (n);                                                        \
    MOVIE_BITS_REFILL()                                                      \
  }
#define MOVIE_BLOCK_CODE(out)                                                \
  {                                                                          \
    code = movie_media_decoder_tables                                        \
               .block_prefix_words[bit_accumulator >> 26];                   \
    count = code & 0xff;                                                     \
    bit_accumulator <<= count;                                               \
    bit_count -= count;                                                      \
    MOVIE_BITS_REFILL()                                                      \
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
    MOVIE_BLOCK_CODE(adjust)                                                 \
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
    MOVIE_BLOCK_CODE(adjust)                                                 \
    reinterpret_cast<MovieChromaDelta>(&sub_5d2e00)(                         \
               reinterpret_cast<char *>(coefficient_block), (plane),         \
               row_width, reinterpret_cast<char *>(block),                   \
               adjust - 0x40);                                               \
  }


// Mapped original owner 005D2E70: one 16x16 macroblock. The optional header
// (record mode) gives six block flags and a motion offset; each flagged block
// is a delta copy from the coefficient plane, each other block is decoded by
// 0x005CE85C and either filled with its DC value or inverse-transformed. The
// reader is its own object (movie_media_frame_bit_state): while it was a
// member of movie_media_decoder_tables, the variable-index block-prefix read
// aliased it and VC6 kept four refill stores the image drops (521 vs 517).
// ORIGINAL: 0x005D2E70 sub_5d2e70 0x005D2E70-0x005D35A4
// RECOVERY_BINDING: v1 {"evidence": ["recovery/tasks/movie-decoder-owner-oct5.json", "pinned caller/body and shared-storage review; fresh VC6 public symbol", "Seed extent is 0x734 bytes. Preserve the 12-byte interval [0x005D35A4,0x005D35B0) as unresolved; do not extend this binding to the next mapped function. Normalize the original ECX/EDX/stack flow to typed pointers and one stack width."], "reason": "Faithful semantic media decoder owner recovery with normalized custom-register helper interfaces. Original identities, spans and historical findings retained; no matching, runtime or game acceptance granted."}
// symbol    ?sub_5d2e70@@YIXPAE0H@Z
// TRIED: MISMATCH #0 'sub' vs 'push' (prologue itself diverges). JPEG- style Huffman bitstream decoder for 6 DCT-ish blocks (4 at stride-0x10, 2 at stride-8), each either memset from a fill value or decoded via a shared Huffman-table walk; factored the identical block body into decode_or_fill()/fill_block() and the repeated "refill bit buffer if count<0x10" idiom into a REFILL macro, both landed in this file (not a separate helper unit). Retargeted the signature from the emitter's own guessed `__stdcall(int)` to `void __fastcall(int,int,int)` to match Ghidra's ecx/edx/stack-arg reading and the `ret 4` epilogue (one stack-passed int, callee-cleaned) - did not close the prologue gap. `sub_5cef8a`/`sub_5cea09` are declared 0-arg/ unknown-arity in the scaffold but called with 2/4 args in the disassembly; bypassed via raw __stdcall function-pointer casts to the same addresses rather than editing the conflicting extern declarations.
void __fastcall sub_5d2e70(BYTE *coefficient_block,
                           BYTE *temporary_block, int row_width) {
  // WORKAROUND(type): the reader is used through one reference per field.
  // Through a pointer to the 12-byte struct (or the struct itself) VC6
  // reloads the merged bit count before the accumulator on the
  // record-mode-0 path (515/517); separate objects load the accumulator
  // first, as the image does (517/517). The original most likely declared
  // cursor, accumulator and bit count as three globals at
  // 0x009C464C/50/54; a separate-extern-globals spelling also measures exact.
  const uint8_t *&bit_cursor = movie_media_frame_bit_state.cursor;
  uint32_t &bit_accumulator = movie_media_frame_bit_state.accumulator;
  int32_t &bit_count = movie_media_frame_bit_state.available_bits;
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
  } else if ((bit_accumulator & 0xc0000000) == 0) {
    flags = 0;
    MOVIE_BITS_CONSUME(2)
  } else {
    int horizontal;
    int vertical;
    if ((bit_accumulator & 0x80000000) != 0) {
      flags = 0x3ff;
      MOVIE_BITS_CONSUME(1)
    } else {
      flags = bit_accumulator >> 24;
      MOVIE_BITS_CONSUME(8)
    }
    MOVIE_BLOCK_CODE(horizontal)
    MOVIE_BLOCK_CODE(vertical)
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
      &movie_media_frame_bit_state;
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
