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
// decoder's C functions 0x005D3980..0x005D4231 carry Pentium Pro (/G6)
// instruction scheduling (0x005D3DF0, 0x005D3F50 and 0x005D4020 match only
// with /G6; 0x005D3980 is closer with it). The preceding macroblock decoder
// 0x005D2E70 and its driver 0x005D35B0 measure closer without /G6 and stay
// in movie_media_decoder.cpp, so the contribution boundary lies at
// 0x005D3980. These owners were split out of movie_media_decoder.cpp so that
// they compile with the evidenced /G6 recipe. Source order follows image
// address order.
#include "stdafx.h"
#include "movie_media.h"
#include "general.h"
#include "movie_decoder_owner.h"
#include "movie_decoder_huffman.h"

#include <cstring>
#include <stdlib.h>

// Naked owners assembled in movie_media_decoder.cpp.
extern "C" void __cdecl sub_5d271d(unsigned int *src, unsigned char *dst,
                                   int count);
extern "C" int __cdecl sub_5d427c(unsigned short *samples, unsigned char *steps,
                                  int a3);

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

// ORIGINAL: 0x005D3DF0 sub_5d3df0 0x005D3DF0-0x005D3F48 FILE BYTE_EXACT
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

// ORIGINAL: 0x005D3F50 sub_5d3f50 0x005D3F50-0x005D4014 FILE BYTE_EXACT
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

// ORIGINAL: 0x005D4020 sub_5d4020 0x005D4020-0x005D4231 FILE BYTE_EXACT
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
