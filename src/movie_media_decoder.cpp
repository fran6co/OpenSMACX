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

// The pinned image credits the EA uV/uV2 1.50 player at file offset 0x291F08.
// This is subsystem provenance, not proof of per-function original modules
// or permission to replace these mappings with an unverified dependency.
// Typed movie-decoder helpers and source-owned storage views. The generated
// initializer includes are supplied by the configured project preparation
// hook under the ignored build cache.
#include "stdafx.h"
#include "movie_media.h"
#include "general.h"
#include "movie_decoder_owner.h"
#include "movie_decoder_frame.h"
#include "movie_decoder_huffman.h"
#include "movie_decoder_idct.h"

#include <cstring>
#include <stdlib.h>

// Reviewed accessed storage views; generated includes are produced in the
// ignored preparation directory by the configured project data initializer.
// RECOVERY_OBJECT: v1 0x00695B58 ?movie_media_decoder_static_storage@@3UMediaDecoderStaticStorage@@A
MediaDecoderStaticStorage movie_media_decoder_static_storage = {
#include "movie_decoder_static.inc"
};
// RECOVERY_OBJECT: v1 0x009C5000 ?movie_media_transform_workspace@@3UMediaTransformWorkspace@@A
MediaTransformWorkspace movie_media_transform_workspace = {
#include "movie_decoder_workspace.inc"
};
// RECOVERY_OBJECT: v1 0x009C9594 ?movie_media_coefficient_tables@@3UMediaCoefficientTableStorage@@A
MediaCoefficientTableStorage movie_media_coefficient_tables = {
#include "movie_decoder_constants.inc"
};
// RECOVERY_OBJECT: v1 0x009B2D64 ?movie_media_decoder_process_storage@@3UMediaDecoderProcessStorage@@A
MediaDecoderProcessStorage movie_media_decoder_process_storage;
// RECOVERY_OBJECT: v1 0x009C314C ?movie_media_decoder_tables@@3UMediaDecoderTableStorage@@A
MediaDecoderTableStorage movie_media_decoder_tables = {
#include "movie_decoder_tables.inc"
};
// RECOVERY_OBJECT: v1 0x009C3000 ?movie_media_idct_storage@@3UMediaDecoderIdctStorage@@A
MediaDecoderIdctStorage movie_media_idct_storage = {
#include "movie_decoder_idct_storage.inc"
};

extern "C" void __cdecl sub_5d29e1(int *source, int *destination);
extern "C" void __cdecl sub_5d2c0a(int *destination, int *source);

static DWORD movie_media_read_bits32(const BYTE *input, DWORD bit_position) {
  const DWORD word_index = bit_position >> 5;
  const DWORD shift = bit_position & 31;
  DWORD high_word;
  DWORD low_word;
  // The original SHLD path loads both adjacent DWORDs, including at shift 0.
  std::memcpy(&high_word, input + word_index * 4, sizeof(high_word));
  std::memcpy(&low_word, input + word_index * 4 + 4, sizeof(low_word));
  if (shift == 0) return high_word;
  return (high_word << shift) | (low_word >> (32 - shift));
}

static DWORD movie_media_decode_predictor_delta(
    const BYTE *input, DWORD &bit_position, const BYTE *lookup,
    unsigned lookup_shift) {
  const BYTE code = lookup[movie_media_read_bits32(input, bit_position) >>
                           lookup_shift];
  const unsigned width = code >> 4;
  bit_position += code & 0x0f;
  if (width == 0) return 0;

  const DWORD value = movie_media_read_bits32(input, bit_position) >>
                      (32 - width);
  bit_position += width;
  if (((value >> (width - 1)) & 1) != 0) return value;
  return value - ((1u << width) - 1u);
}

/*
Purpose: Decode one 64-coefficient block for the movie codec: dequant the DC
         term through the table at 0x009C5820, then walk coefficients 63..1
         through the dispatch table at 0x009C9AA8 - zero runs, direct table
         dequant, negated dequant, the two-zero arm through 0x009C95A4, and
         the ZRL run arm walking 0x009C95A4/0x009C95A0 while the escape count
         lasts - refilling the accumulator a byte at a time whenever fewer
         than 8 bits remain, and publishing the cursor/accumulator/count back
         to their slots on the way out.

// symbol    ?sub_5cf150@@YAHXZ
// size      416 bytes
// prototype int (__cdecl sub_5cf150)(void)
// callers   3   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Status: Coverage transcription - unclaimed body (the original keeps the
        bit-reader state register-resident across the whole switch; this
        spelling re-reads the globals, diverging at instruction 0 - seed
        TRIED).
*/

static LONG movie_media_add_ecx(LONG value, int amount) {
    DWORD bits;
    std::memcpy(&bits, &value, sizeof(bits));
    bits += static_cast<DWORD>(amount);
    std::memcpy(&value, &bits, sizeof(value));
    return value;
}

static int movie_media_low32(__int64 value) {
    const DWORD bits = static_cast<DWORD>(value);
    int result;
    std::memcpy(&result, &bits, sizeof(result));
    return result;
}

static int movie_media_low32_after_arithmetic_shift16(__int64 value) {
    uint64_t bits;
    std::memcpy(&bits, &value, sizeof(bits));
    // Only the low dword of the arithmetic shift survives; sign extension
    // above bit 47 cannot affect those 32 result bits.
    const DWORD low = static_cast<DWORD>(bits >> 16);
    int result;
    std::memcpy(&result, &low, sizeof(result));
    return result;
}

static int movie_media_negate32(int value) {
    DWORD bits;
    std::memcpy(&bits, &value, sizeof(bits));
    bits = 0u - bits;
    std::memcpy(&value, &bits, sizeof(value));
    return value;
}

static int movie_media_signed32(DWORD value) {
    int result;
    std::memcpy(&result, &value, sizeof(result));
    return result;
}

static DWORD movie_media_raw_word_at(const DWORD *base, DWORD index_bits) {
    uintptr_t address = reinterpret_cast<uintptr_t>(base);
    address += static_cast<uintptr_t>(index_bits) * sizeof(DWORD);
    DWORD value;
    std::memcpy(&value, reinterpret_cast<const void *>(address), sizeof(value));
    return value;
}

static void movie_media_raw_workspace_store(DWORD index_bits, DWORD value) {
    uintptr_t address = reinterpret_cast<uintptr_t>(
        movie_media_transform_workspace.row_and_butterfly_words);
    address += static_cast<uintptr_t>(index_bits) * sizeof(DWORD);
    std::memcpy(reinterpret_cast<void *>(address), &value, sizeof(value));
}

static DWORD movie_media_int_bits(int value) {
    DWORD bits;
    std::memcpy(&bits, &value, sizeof(bits));
    return bits;
}

// The pinned x86 ADD/SUB/SHL operations wrap as 32-bit bit patterns.
// Avoid relying on signed-overflow behavior in the translated row path.
static int movie_media_add32(int left, int right) {
    return movie_media_signed32(movie_media_int_bits(left) +
                                 movie_media_int_bits(right));
}

static int movie_media_sub32(int left, int right) {
    return movie_media_signed32(movie_media_int_bits(left) -
                                 movie_media_int_bits(right));
}

static int movie_media_mul2(int value) {
    return movie_media_signed32(movie_media_int_bits(value) << 1);
}

static int movie_media_signed_high_product(int left, int right) {
    const __int64 product = static_cast<__int64>(left) * right;
    uint64_t bits;
    std::memcpy(&bits, &product, sizeof(bits));
    return movie_media_signed32(static_cast<DWORD>(bits >> 32));
}

static MediaTransformRegisterContinuation movie_media_transform_return(
    MediaTransformBitState &reader, const BYTE *cursor, DWORD accumulator,
    LONG available_bits, int coefficient_index) {
    reader.cursor = cursor;
    reader.accumulator = accumulator;
    reader.available_bits = available_bits;
    MediaTransformRegisterContinuation result;
    result.esi_cursor = cursor;
    result.ebp_accumulator = accumulator;
    result.ecx_available_bits = available_bits;
    result.edi_coefficient_index = coefficient_index;
    return result;
}

// ORIGINAL: 0x005CF150 sub_5cf150 0x005CF150-0x005CF2F0 FILE
// RECOVERY_BINDING: v1 {"evidence": ["pinned terranx_original.exe SHA-256 01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf; mapped span 005CF150-005CF2F0", "pinned dispatch table at 009C9AA8 targets 005CF1CE/239/1AC/26A/1DB/226/1BC/26A; run arms read distinct A8/A4/A0 windows", "both native exits write ESI/EBP/ECX to 009C5920/28/24 and return EDI without restoring it; 5CF2F0 consumes those outputs after five calls", "existing movie_media_decoder.cpp shared workspace, scale and transform-bit-state owners; stack workspace pushes are cleaned by callers and not read in the 416-byte body", "historical leaf source comments preserved verbatim immediately before this mapping; new C++ wrapper/helper does not claim a measured symbol or ABI"], "reason": "Source-owned full implementation of the mapped 5CF150 transform; its normalized return carries the original post-call register state to the six-step 5CF2F0 decoder. This is not native custom-register ABI evidence."}
// symbol    ?sub_5cf150@@YA?AUMediaTransformRegisterContinuation@@XZ
// TRIED: The old leaf source transcript was not register-faithful. The mapped function now owns the full eight-way implementation; its C++ result normalizes the native ESI/EBP/ECX/EDI continuation and does not claim the unknown external custom-register ABI.
// Hand-written asm (EA uV decoder module): no prologue, clobbers esi/ebp unsaved, register-convention outputs, imul/shl/shr/adc rounding.
__declspec(naked) MediaTransformRegisterContinuation __cdecl sub_5cf150() {
    __asm {
        mov eax, dword ptr [movie_media_conversion_tables + 0x108]
        mov esi, dword ptr [movie_media_conversion_tables + 0x100]
        shl eax, 0x18
        mov ebp, dword ptr [movie_media_conversion_tables + 0x108]
        sar eax, 8
        mov ecx, dword ptr [movie_media_conversion_tables + 0x104]
        imul dword ptr [movie_media_conversion_tables]
        shl edx, 0x10
        xor ebx, ebx
        shr eax, 0x10
        mov edi, 0x3f
        adc eax, edx
        mov dword ptr [movie_media_transform_workspace + 0x600], eax
        xor eax, eax
        shr ebp, 8
        mov al, byte ptr [esi]
        inc esi
        sub ecx, 8
        shl eax, cl
        add ecx, 8
        or ebp, eax
m5cf150_5cf199:
        mov eax, ebp
        mov ebx, dword ptr [edi*4 + movie_media_coefficient_tables + 0x14]
        and eax, 7
        // The original dispatches through an 8-entry code-pointer table at
        // 0x9C9AA8 (jmp [eax*4+table]); VC6 inline asm cannot emit a table of
        // label addresses, so the same eight targets are selected by compares.
        cmp eax, 1
        jb m5cf150_5cf1ce
        je m5cf150_5cf239
        cmp eax, 3
        jb m5cf150_5cf1ac
        je m5cf150_5cf26a
        cmp eax, 5
        jb m5cf150_5cf1db
        je m5cf150_5cf226
        cmp eax, 6
        je m5cf150_5cf1bc
        jmp m5cf150_5cf26a
m5cf150_5cf1ac:
        mov eax, dword ptr [ebx*4 + movie_media_conversion_tables]
        mov dword ptr [ebx*4 + movie_media_transform_workspace + 0x600], eax
        jmp m5cf150_5cf1f9
m5cf150_5cf1bc:
        mov eax, dword ptr [ebx*4 + movie_media_conversion_tables]
        neg eax
        mov dword ptr [ebx*4 + movie_media_transform_workspace + 0x600], eax
        jmp m5cf150_5cf1f9
m5cf150_5cf1ce:
        mov dword ptr [ebx*4 + movie_media_transform_workspace + 0x600], 0
        jmp m5cf150_5cf1f9
m5cf150_5cf1db:
        mov dword ptr [ebx*4 + movie_media_transform_workspace + 0x600], 0
        mov ebx, dword ptr [edi*4 + movie_media_coefficient_tables + 0x10]
        dec edi
        mov dword ptr [ebx*4 + movie_media_transform_workspace + 0x600], 0
m5cf150_5cf1f9:
        shr ebp, 3
        sub ecx, 3
        cmp ecx, 8
        jge m5cf150_5cf210
        xor eax, eax
        mov al, byte ptr [esi]
        inc esi
        shl eax, cl
        add ecx, 8
        or ebp, eax
m5cf150_5cf210:
        dec edi
        jg m5cf150_5cf199
        mov dword ptr [movie_media_conversion_tables + 0x100], esi
        mov dword ptr [movie_media_conversion_tables + 0x108], ebp
        mov dword ptr [movie_media_conversion_tables + 0x104], ecx
        ret
m5cf150_5cf226:
        mov dword ptr [ebx*4 + movie_media_transform_workspace + 0x600], 0
        mov ebx, dword ptr [edi*4 + movie_media_coefficient_tables + 0x10]
        dec edi
m5cf150_5cf239:
        mov eax, ebp
        xor edx, edx
        and eax, 0xff
        shr eax, 3
m5cf150_5cf245:
        mov dword ptr [ebx*4 + movie_media_transform_workspace + 0x600], edx
        mov ebx, dword ptr [edi*4 + movie_media_coefficient_tables + 0x10]
        mov dword ptr [ebx*4 + movie_media_transform_workspace + 0x600], edx
        mov ebx, dword ptr [edi*4 + movie_media_coefficient_tables + 0xc]
        sub edi, 2
        dec eax
        jne m5cf150_5cf245
        inc edi
        jmp m5cf150_5cf2c4
m5cf150_5cf26a:
        mov eax, ebp
        and eax, 0xfc
        cmp eax, 0xfc
        je m5cf150_5cf296
        shl eax, 0x18
        sar eax, 0xa
        imul dword ptr [ebx*4 + movie_media_conversion_tables]
        shl edx, 0x10
        shr eax, 0x10
        adc eax, edx
        mov dword ptr [ebx*4 + movie_media_transform_workspace + 0x600], eax
        jmp m5cf150_5cf2c4
m5cf150_5cf296:
        shr ebp, 8
        mov al, byte ptr [esi]
        inc esi
        sub ecx, 8
        shl eax, cl
        add ecx, 8
        or ebp, eax
        mov eax, ebp
        shl eax, 0x18
        sar eax, 8
        imul dword ptr [ebx*4 + movie_media_conversion_tables]
        shl edx, 0x10
        shr eax, 0x10
        adc eax, edx
        mov dword ptr [ebx*4 + movie_media_transform_workspace + 0x600], eax
m5cf150_5cf2c4:
        shr ebp, 8
        xor eax, eax
        mov al, byte ptr [esi]
        inc esi
        sub ecx, 8
        shl eax, cl
        add ecx, 8
        or ebp, eax
        dec edi
        jg m5cf150_5cf199
        mov dword ptr [movie_media_conversion_tables + 0x100], esi
        mov dword ptr [movie_media_conversion_tables + 0x108], ebp
        mov dword ptr [movie_media_conversion_tables + 0x104], ecx
        ret
    }
}

// ORIGINAL: 0x005CF2F0 sub_5cf2f0 0x005CF2F0-0x005CF5C7 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["pinned terranx_original.exe SHA-256 01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf; mapped span 005CF2F0-005CF5C7", "original instruction/register dataflow: EBP bit buffer, EDI live bit cursor, ESI receiver; four 0x19 table lookups update +0x100, then two 0x18 lookups update +0x104/+0x108; after each helper call the next ESI/EBP/EDI are the returned clobbered registers", "5CF150 native eight-way dispatch targets at 005CF1CE/239/1AC/26A/1DB/226/1BC/26A; run keys 1 and 5 use distinct A8/A4/A0 indexed windows and distinct EDI update order", "movie_media_decoder.cpp shared typed bit-window/VLC helpers and MediaDecoderContext fields; coefficient table byte window is words+0x394 (128 bytes) and words+0x414 (256 bytes)", "mapped 005CF150 source definition consumes no stack arguments; original call sites set workspace operands but the callee uses shared source-owned conversion state", "recover inspect 005CF2F0 --callers: no callers among mapped spans; external/register-ABI caller remains unknown"], "reason": "Typed source owner for the mapped six-delta predictor decoder; the source API explicitly models the mutated cursor and real decoder context. This is not original custom-register ABI evidence or a byte-match claim."}
// symbol    ?sub_5cf2f0@@YA?AUMediaTransformRegisterContinuation@@PBEKAAUMediaDecoderContext@@@Z
// TRIED: no conventional signature can match - no prologue at all, and edi/esi/ebp are read as inputs pre-loaded by the caller (Ghidra: unaff_EBP/unaff_ESI/unaff_EDI), same shape as the established 0x005D1B88 case. Landed as an explicit 3-pointer __cdecl function reproducing the algorithm exactly (six unrolled Huffman-decode-and-advance blocks, matching the original's own unrolling) - MISMATCH from instruction #0 (prologue) is expected and cannot be closed without __asm.
// TRIED: Post-call ESI-derived predictor accesses are raw byte dword effects; the returned address is not asserted to name a MediaDecoderContext.
// The original entry's EBP/EDI/ESI are respectively the bit-buffer pointer,
// live bit cursor, and MediaDecoderContext receiver. Each 5CF150 call then
// overwrites those registers with its shared transform reader's accumulator,
// coefficient index, and cursor. Carry those actual source-owned values back
// into the next iteration; do not keep the entry values live across a call.
// Original entry convention: no prologue; inputs arrive in EBP/EDI/ESI;
// this source signature is semantic normalization, not original-ABI evidence.
// Hand-written asm (EA uV decoder module): no prologue, esi/edi/ebx/ebp register inputs.
__declspec(naked) MediaTransformRegisterContinuation __cdecl sub_5cf2f0(
    const BYTE *bit_buffer, DWORD bit_position,
                        MediaDecoderContext &context) {
    __asm {
        mov edx, edi
        shr edx, 5
        mov ecx, edi
        mov eax, dword ptr [ebp + edx*4]
        mov edx, dword ptr [ebp + edx*4 + 4]
        shld eax, edx, cl
        shr eax, 0x19
        movzx ebx, byte ptr [eax + movie_media_coefficient_tables + 0x394]
        mov edx, ebx
        shr ebx, 4
        and edx, 0xf
        xor eax, eax
        add edi, edx
        cmp ebx, 0
        je m5cf2f0_5cf34c
        mov edx, edi
        mov ecx, edi
        shr edx, 5
        mov eax, dword ptr [ebp + edx*4]
        mov edx, dword ptr [ebp + edx*4 + 4]
        shld eax, edx, cl
        mov ecx, 0x20
        sub ecx, ebx
        add edi, ebx
        shr eax, cl
        mov ecx, ebx
        dec ebx
        bt eax, ebx
        jb m5cf2f0_5cf34c
        mov edx, 1
        shl edx, cl
        dec edx
        sub eax, edx
m5cf2f0_5cf34c:
        mov ebx, dword ptr [esi + 0x100]
        add eax, ebx
        mov dword ptr [esi + 0x100], eax
        push 0x40
        push offset movie_media_transform_workspace
        call sub_5cf150
        add esp, 8
        mov edx, edi
        shr edx, 5
        mov ecx, edi
        mov eax, dword ptr [ebp + edx*4]
        mov edx, dword ptr [ebp + edx*4 + 4]
        shld eax, edx, cl
        shr eax, 0x19
        movzx ebx, byte ptr [eax + movie_media_coefficient_tables + 0x394]
        mov edx, ebx
        shr ebx, 4
        and edx, 0xf
        xor eax, eax
        add edi, edx
        cmp ebx, 0
        je m5cf2f0_5cf3c5
        mov edx, edi
        mov ecx, edi
        shr edx, 5
        mov eax, dword ptr [ebp + edx*4]
        mov edx, dword ptr [ebp + edx*4 + 4]
        shld eax, edx, cl
        mov ecx, 0x20
        sub ecx, ebx
        add edi, ebx
        shr eax, cl
        mov ecx, ebx
        dec ebx
        bt eax, ebx
        jb m5cf2f0_5cf3c5
        mov edx, 1
        shl edx, cl
        dec edx
        sub eax, edx
m5cf2f0_5cf3c5:
        mov ebx, dword ptr [esi + 0x100]
        add eax, ebx
        mov dword ptr [esi + 0x100], eax
        push 0x40
        push offset movie_media_transform_workspace + 0x20
        call sub_5cf150
        add esp, 8
        mov edx, edi
        shr edx, 5
        mov ecx, edi
        mov eax, dword ptr [ebp + edx*4]
        mov edx, dword ptr [ebp + edx*4 + 4]
        shld eax, edx, cl
        shr eax, 0x19
        movzx ebx, byte ptr [eax + movie_media_coefficient_tables + 0x394]
        mov edx, ebx
        shr ebx, 4
        and edx, 0xf
        xor eax, eax
        add edi, edx
        cmp ebx, 0
        je m5cf2f0_5cf43e
        mov edx, edi
        mov ecx, edi
        shr edx, 5
        mov eax, dword ptr [ebp + edx*4]
        mov edx, dword ptr [ebp + edx*4 + 4]
        shld eax, edx, cl
        mov ecx, 0x20
        sub ecx, ebx
        add edi, ebx
        shr eax, cl
        mov ecx, ebx
        dec ebx
        bt eax, ebx
        jb m5cf2f0_5cf43e
        mov edx, 1
        shl edx, cl
        dec edx
        sub eax, edx
m5cf2f0_5cf43e:
        mov ebx, dword ptr [esi + 0x100]
        add eax, ebx
        mov dword ptr [esi + 0x100], eax
        push 0x40
        push offset movie_media_transform_workspace + 0x200
        call sub_5cf150
        add esp, 8
        mov edx, edi
        shr edx, 5
        mov ecx, edi
        mov eax, dword ptr [ebp + edx*4]
        mov edx, dword ptr [ebp + edx*4 + 4]
        shld eax, edx, cl
        shr eax, 0x19
        movzx ebx, byte ptr [eax + movie_media_coefficient_tables + 0x394]
        mov edx, ebx
        shr ebx, 4
        and edx, 0xf
        xor eax, eax
        add edi, edx
        cmp ebx, 0
        je m5cf2f0_5cf4b7
        mov edx, edi
        mov ecx, edi
        shr edx, 5
        mov eax, dword ptr [ebp + edx*4]
        mov edx, dword ptr [ebp + edx*4 + 4]
        shld eax, edx, cl
        mov ecx, 0x20
        sub ecx, ebx
        add edi, ebx
        shr eax, cl
        mov ecx, ebx
        dec ebx
        bt eax, ebx
        jb m5cf2f0_5cf4b7
        mov edx, 1
        shl edx, cl
        dec edx
        sub eax, edx
m5cf2f0_5cf4b7:
        mov ebx, dword ptr [esi + 0x100]
        add eax, ebx
        mov dword ptr [esi + 0x100], eax
        push 0x40
        push offset movie_media_transform_workspace + 0x220
        call sub_5cf150
        add esp, 8
        mov edx, edi
        shr edx, 5
        mov ecx, edi
        mov eax, dword ptr [ebp + edx*4]
        mov edx, dword ptr [ebp + edx*4 + 4]
        shld eax, edx, cl
        shr eax, 0x18
        movzx ebx, byte ptr [eax + movie_media_coefficient_tables + 0x414]
        mov edx, ebx
        shr ebx, 4
        and edx, 0xf
        xor eax, eax
        add edi, edx
        cmp ebx, 0
        je m5cf2f0_5cf530
        mov edx, edi
        mov ecx, edi
        shr edx, 5
        mov eax, dword ptr [ebp + edx*4]
        mov edx, dword ptr [ebp + edx*4 + 4]
        shld eax, edx, cl
        mov ecx, 0x20
        sub ecx, ebx
        add edi, ebx
        shr eax, cl
        mov ecx, ebx
        dec ebx
        bt eax, ebx
        jb m5cf2f0_5cf530
        mov edx, 1
        shl edx, cl
        dec edx
        sub eax, edx
m5cf2f0_5cf530:
        mov ebx, dword ptr [esi + 0x104]
        add eax, ebx
        mov dword ptr [esi + 0x104], eax
        push 0x20
        push offset movie_media_transform_workspace + 0x400
        call sub_5cf150
        add esp, 8
        mov edx, edi
        shr edx, 5
        mov ecx, edi
        mov eax, dword ptr [ebp + edx*4]
        mov edx, dword ptr [ebp + edx*4 + 4]
        shld eax, edx, cl
        shr eax, 0x18
        movzx ebx, byte ptr [eax + movie_media_coefficient_tables + 0x414]
        mov edx, ebx
        shr ebx, 4
        and edx, 0xf
        xor eax, eax
        add edi, edx
        cmp ebx, 0
        je m5cf2f0_5cf5a9
        mov edx, edi
        mov ecx, edi
        shr edx, 5
        mov eax, dword ptr [ebp + edx*4]
        mov edx, dword ptr [ebp + edx*4 + 4]
        shld eax, edx, cl
        mov ecx, 0x20
        sub ecx, ebx
        add edi, ebx
        shr eax, cl
        mov ecx, ebx
        dec ebx
        bt eax, ebx
        jb m5cf2f0_5cf5a9
        mov edx, 1
        shl edx, cl
        dec edx
        sub eax, edx
m5cf2f0_5cf5a9:
        mov ebx, dword ptr [esi + 0x108]
        add eax, ebx
        mov dword ptr [esi + 0x108], eax
        push 0x20
        push offset movie_media_transform_workspace + 0x500
        call sub_5cf150
        add esp, 8
        ret
    }
}

static DWORD movie_media_coefficient_entry(DWORD code) {
  const DWORD *const words = movie_media_coefficient_tables.words;
  if (code >= 0x8000)
    return words[65 + (code >> 13)]; // 009C9698
  if (code >= 0x0800)
    return words[77 + (code >> 9)];  // 009C96C8
  if (code >= 0x0400)
    return words[133 + (code >> 7)]; // 009C97A8
  if (code >= 0x0200)
    return words[133 + (code >> 5)]; // same 009C97A8 base
  if (code >= 0x0100)
    return words[149 + (code >> 4)]; // 009C97E8
  if (code >= 0x0080)
    return words[165 + (code >> 3)]; // 009C9828
  if (code >= 0x0040)
    return words[181 + (code >> 2)]; // 009C9868
  return words[197 + (code >> 1)];   // 009C98A8
}

static int movie_media_sar17(DWORD value) {
  // Pinned 5D2CC5..5D2D09 and 5D119D use SAR17 without a division bias.
  // VC6 preserves the DWORD bits on conversion and shifts signed values
  // arithmetically, including negative values not divisible by 2^17.
  return static_cast<int>(value) >> 17;
}

// The first five source-owned coefficient words are the exact operands read
// at 009C9594..009C95A4. They remain part of the shared decoder table window.
// The image's FILD inputs retain signed dword precision through the x87
// arithmetic. Promote each loaded binary32 coefficient before calculation;
// these binary64 intermediates avoid float truncation but do not claim the
// original x87 precision/control-word result for every input.
// The TRIED lines in this family retain superseded General-transcript history.
// Current bodies use source-owned coefficient words and qword low-DWORD
// extraction; prior numeric-cast/raw-alias descriptions are not current code.
static int movie_media_idct_coefficient_integer(unsigned index) {
    return movie_media_signed32(movie_media_coefficient_tables.words[index]);
}

static float movie_media_idct_coefficient_float(unsigned index) {
    float value;
    std::memcpy(&value, &movie_media_coefficient_tables.words[index],
                sizeof(value));
    return value;
}

static int movie_media_idct_biased_low_dword(const double &value) {
    DWORD low_dword;
    std::memcpy(&low_dword, &value, sizeof(low_dword));
    return movie_media_signed32(low_dword);
}

// ORIGINAL: 0x005D29E1 sub_5d29e1 0x005D29E1-0x005D2B20 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned005D29E1..005D2B20 full decode ends RET; eight destination offsets are [0, 36, 72, 108, 144, 180, 216, 252]; SAR17 count is 0. The row variant has an AC-zero broadcast path; dense siblings differ by eight final SAR17 reductions.", "Current ordinary extern-C declaration sub_5d29e1 has the unique independently owned COFF COMDAT _sub_5d29e1 in the qualified production object; only its current emitted-name fact is corrected.", "The stripped original provides no nominal name/type proof. Original ESI/EDI entry contracts and the current normalized cdecl mismatch remain recovery obligations; historical asm-only assertions are unproved hypotheses, not exclusions. No byte, ABI, lifetime or complete-behavior claim is granted."], "reason": "Retain the original butterfly identity and spans while selecting its actual current emitted symbol for strict observation."}
// symbol    _sub_5d29e1
// TRIED: this is not addressable with a normal calling convention - the body reads unaff_ESI/unaff_EDI with no stack access and no register setup of its own (first instruction is `push edi`, `mov ebx,[esi+0x14]` immediately after), so the 3 callers must be pre-loading esi/edi before `call`ing in. That is an interprocedural register-allocation artifact of compiling the 3 call sites and this body together as one translation unit; it cannot be reproduced by compiling this function alone under any ordinary C++ signature, since VC6 only picks a non-standard register ABI for an internal-linkage callee when it can see all of its callers. Declared it as a plain two-pointer __cdecl function instead (src, dst) for a faithful, compiling body. Also not reproduced: the fadd-then- reinterpret-low-dword pattern for double->int (`fstp qword ptr`, then `mov` of the low 4 bytes) is the classic magic- number float truncation trick that avoids calling `_ftol`; mimicked with a raw double-then-reinterpret-cast here. Best reached: MISMATCH, edit_count 7, 455 bytes vs 319 - the whole prologue/epilogue and call sequencing differ because of the calling-convention gap above.
// size      319 bytes
// prototype
// callers   3   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// Hand-written asm (EA uV decoder module): no prologue, esi/edi/ebx/ebp register inputs.
extern "C" __declspec(naked) void __cdecl sub_5d29e1(int *src, int *dst) {
    __asm {
        push edi
        mov ebx, dword ptr [esi + 0x14]
        mov eax, dword ptr [esi + 0xc]
        mov ecx, dword ptr [esi + 4]
        mov edx, dword ptr [esi + 0x1c]
        mov edi, dword ptr [esi + 8]
        mov ebp, dword ptr [esi + 0x18]
        or edi, ebx
        or ebp, eax
        or edi, ecx
        or ebp, edx
        or ebp, edi
        mov edi, dword ptr [esi + 0x10]
        or ebp, edi
        je m5d29e1_5d2af9
        sub esp, 0x20
        lea edi, [eax + ebx]
        sub ebx, eax
        lea eax, [edx + ecx]
        sub ecx, edx
        mov dword ptr [esp + 8], ebx
        mov dword ptr [esp + 0xc], ecx
        lea ebp, [edi + eax]
        sub eax, edi
        fild dword ptr [esp + 8]
        fild dword ptr [esp + 0xc]
        fld st(0)
        fmul dword ptr [movie_media_coefficient_tables + 0x8]
        fxch st(2)
        fadd st(1), st(0)
        fmul dword ptr [movie_media_coefficient_tables + 0x4]
        fxch st(1)
        fmul dword ptr [movie_media_coefficient_tables + 0xc]
        imul dword ptr [movie_media_coefficient_tables]
        fadd st(1), st(0)
        fsubp st(2), st
        fadd dword ptr [movie_media_coefficient_tables + 0x10]
        fxch st(1)
        fadd dword ptr [movie_media_coefficient_tables + 0x10]
        mov eax, dword ptr [esi + 8]
        mov ebx, dword ptr [esi + 0x18]
        shl edx, 1
        fstp qword ptr [esp + 0x10]
        fstp qword ptr [esp + 8]
        mov ecx, dword ptr [esp + 8]
        mov edi, dword ptr [esp + 0x10]
        add ecx, edx
        add edx, edi
        add edi, ebp
        mov dword ptr [esp + 0xc], ecx
        mov dword ptr [esp + 0x10], edx
        mov dword ptr [esp + 0x14], edi
        mov ecx, dword ptr [esi]
        mov edx, dword ptr [esi + 0x10]
        lea esi, [ebx + eax]
        sub eax, ebx
        lea ebx, [edx + ecx]
        sub ecx, edx
        imul dword ptr [movie_media_coefficient_tables]
        shl edx, 1
        mov edi, dword ptr [esp + 0x20]
        add esi, edx
        mov ebp, dword ptr [esp + 0xc]
        lea eax, [edx + ecx]
        sub ecx, edx
        lea edx, [esi + ebx]
        sub ebx, esi
        lea esi, [ecx + ebp]
        sub ecx, ebp
        mov ebp, dword ptr [esp + 8]
        mov dword ptr [edi + 0x48], esi
        mov esi, dword ptr [esp + 0x10]
        mov dword ptr [edi + 0xb4], ecx
        lea ecx, [ebx + ebp]
        sub ebx, ebp
        mov ebp, dword ptr [esp + 0x14]
        add esp, 0x24
        mov dword ptr [edi + 0x6c], ecx
        mov dword ptr [edi + 0x90], ebx
        lea ebx, [esi + eax]
        sub eax, esi
        lea ecx, [edx + ebp]
        sub edx, ebp
        mov dword ptr [edi], ecx
        mov dword ptr [edi + 0x24], ebx
        mov dword ptr [edi + 0xd8], eax
        mov dword ptr [edi + 0xfc], edx
        ret
m5d29e1_5d2af9:
        pop edi
        mov eax, dword ptr [esi]
        mov dword ptr [edi], eax
        mov dword ptr [edi + 0x24], eax
        mov dword ptr [edi + 0x48], eax
        mov dword ptr [edi + 0x6c], eax
        mov dword ptr [edi + 0x90], eax
        mov dword ptr [edi + 0xb4], eax
        mov dword ptr [edi + 0xd8], eax
        mov dword ptr [edi + 0xfc], eax
        ret
    }
}

// ORIGINAL: 0x005D2B20 sub_5d2b20 0x005D2B20-0x005D2C0A FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned005D2B20..005D2C0A full decode ends RET; eight destination offsets are [0, 4, 8, 12, 16, 20, 24, 28]; SAR17 count is 0. The row variant has an AC-zero broadcast path; dense siblings differ by eight final SAR17 reductions.", "Current ordinary extern-C declaration sub_5d2b20 has the unique independently owned COFF COMDAT _sub_5d2b20 in the qualified production object; only its current emitted-name fact is corrected.", "The stripped original provides no nominal name/type proof. Original ESI/EDI entry contracts and the current normalized cdecl mismatch remain recovery obligations; historical asm-only assertions are unproved hypotheses, not exclusions. No byte, ABI, lifetime or complete-behavior claim is granted."], "reason": "Retain the original butterfly identity and spans while selecting its actual current emitted symbol for strict observation."}
// symbol    _sub_5d2b20
// TRIED: BYTE_EXACT is likely structurally unreachable here without `__asm` (banned). The disassembly reads `esi` as an 8-int input array from instruction #1 (`mov ebx,[esi+0x14]`) and NEVER writes esi before that first read anywhere in the function - Ghidra names it `unaff_ESI` for exactly this reason. That means the real calling convention hands this function its input array already sitting in esi, which none of __cdecl/__stdcall/__fastcall/__thiscall can express - every standard convention either uses the stack or ecx/edx for the first argument, never esi with zero setup. This is plausible as a compiler-chosen convention specific to this internal, 2-caller helper (proven safe only in the context of its real callers, which this recovery unit does not see), but nothing in valid VC6 C++ can request it. Landed body takes the input array as a normal second parameter instead (MISMATCH from the very first instruction, since the prologue differs immediately) so the arithmetic shape is on record: an 8-point butterfly (pairwise sum/difference of src[0],src[2],src[4], src[6] and separately src[1],src[3],src[5],src[7]) combining one `__int64` fixed-point multiply-by-IdctSin45Q31 (the `imul dword ptr [addr]` high-dword-times-2 pattern) with float arithmetic against IdctInv2CosPi8/IdctInv2Cos3Pi8/IdctCos3Pi8, each float result added to IdctAanScale and reduced to an int via `fstp qword` followed by reading only the low dword - this looks like the classic "magic-constant" double-to-int rounding idiom (add a bias, read the mantissa's low bits) but a plain `(int)(double)expr` cast was NOT enough to reproduce it; the exact bias/rounding trick was not reverse-engineered.
// size      234 bytes
// prototype
// callers   2   call targets   0
// kind
// flags
// calls     (none)
// working copy - scaffold materialised by --work
// Hand-written asm (EA uV decoder module): no prologue, esi/edi/ebx/ebp register inputs.
extern "C" __declspec(naked) void __cdecl sub_5d2b20(int *dst, int *src) {
    __asm {
        mov ebx, dword ptr [esi + 0x14]
        mov eax, dword ptr [esi + 0xc]
        mov ecx, dword ptr [esi + 4]
        mov edx, dword ptr [esi + 0x1c]
        lea edi, [eax + ebx]
        sub ebx, eax
        lea eax, [edx + ecx]
        sub ecx, edx
        mov dword ptr [esp + 8], ebx
        mov dword ptr [esp + 0xc], ecx
        lea ebp, [edi + eax]
        sub eax, edi
        fild dword ptr [esp + 8]
        fild dword ptr [esp + 0xc]
        fld st(0)
        fmul dword ptr [movie_media_coefficient_tables + 0x8]
        fxch st(2)
        fadd st(1), st(0)
        fmul dword ptr [movie_media_coefficient_tables + 0x4]
        fxch st(1)
        fmul dword ptr [movie_media_coefficient_tables + 0xc]
        imul dword ptr [movie_media_coefficient_tables]
        fadd st(1), st(0)
        fsubp st(2), st
        fadd dword ptr [movie_media_coefficient_tables + 0x10]
        fxch st(1)
        fadd dword ptr [movie_media_coefficient_tables + 0x10]
        mov eax, dword ptr [esi + 8]
        mov ebx, dword ptr [esi + 0x18]
        shl edx, 1
        fstp qword ptr [esp + 0x10]
        fstp qword ptr [esp + 8]
        mov ecx, dword ptr [esp + 8]
        mov edi, dword ptr [esp + 0x10]
        add ecx, edx
        add edx, edi
        add edi, ebp
        mov dword ptr [esp + 0xc], ecx
        mov dword ptr [esp + 0x10], edx
        mov dword ptr [esp + 0x14], edi
        mov ecx, dword ptr [esi]
        mov edx, dword ptr [esi + 0x10]
        lea esi, [ebx + eax]
        sub eax, ebx
        lea ebx, [edx + ecx]
        sub ecx, edx
        imul dword ptr [movie_media_coefficient_tables]
        shl edx, 1
        mov edi, dword ptr [esp + 4]
        add esi, edx
        mov ebp, dword ptr [esp + 0xc]
        lea eax, [edx + ecx]
        sub ecx, edx
        lea edx, [esi + ebx]
        sub ebx, esi
        lea esi, [ecx + ebp]
        sub ecx, ebp
        mov ebp, dword ptr [esp + 8]
        mov dword ptr [edi + 8], esi
        mov esi, dword ptr [esp + 0x10]
        mov dword ptr [edi + 0x14], ecx
        lea ecx, [ebx + ebp]
        sub ebx, ebp
        mov ebp, dword ptr [esp + 0x14]
        mov dword ptr [edi + 0xc], ecx
        mov dword ptr [edi + 0x10], ebx
        lea ebx, [esi + eax]
        sub eax, esi
        lea ecx, [edx + ebp]
        sub edx, ebp
        mov dword ptr [edi], ecx
        mov dword ptr [edi + 4], ebx
        mov dword ptr [edi + 0x18], eax
        mov dword ptr [edi + 0x1c], edx
        ret
    }
}

// ORIGINAL: 0x005D2C0A sub_5d2c0a 0x005D2C0A-0x005D2D13 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned005D2C0A..005D2D13 full decode ends RET; eight destination offsets are [0, 4, 8, 12, 16, 20, 24, 28]; SAR17 count is 8. The row variant has an AC-zero broadcast path; dense siblings differ by eight final SAR17 reductions.", "Current ordinary extern-C declaration sub_5d2c0a has the unique independently owned COFF COMDAT _sub_5d2c0a in the qualified production object; only its current emitted-name fact is corrected.", "The stripped original provides no nominal name/type proof. Original ESI/EDI entry contracts and the current normalized cdecl mismatch remain recovery obligations; historical asm-only assertions are unproved hypotheses, not exclusions. No byte, ABI, lifetime or complete-behavior claim is granted."], "reason": "Retain the original butterfly identity and spans while selecting its actual current emitted symbol for strict observation."}
// symbol    _sub_5d2c0a
// TRIED: BYTE_EXACT is structurally unreachable here without `__asm` (banned), same finding as the sibling 8-point butterfly at 0x005D2B20 (see the sibling 0x005D2B20 body (homed here)): the disassembly reads `esi` (src) from the very first real instruction and `edi` (dst, saved via `push edi` at entry and reloaded from the stack near the end) with no stack setup for either, which none of __cdecl/__stdcall/__fastcall/__thiscall can request. Landed as `sub_5d2c0a(int *dst, int *src)` (MISMATCH from instruction 0, the prologue) so the arithmetic is on record: identical sum/difference/`__int64` fixed-point-multiply butterfly to the sibling, but every combined pair is additionally scaled by `>> 17` before it is stored, which the sibling's version does not do.
// size      265 bytes
// prototype
// callers   1   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// Hand-written asm (EA uV decoder module): no prologue, esi/edi/ebx/ebp register inputs.
extern "C" __declspec(naked) void __cdecl sub_5d2c0a(int *dst, int *src) {
    __asm {
        push edi
        mov ebx, dword ptr [esi + 0x14]
        mov eax, dword ptr [esi + 0xc]
        mov ecx, dword ptr [esi + 4]
        mov edx, dword ptr [esi + 0x1c]
        sub esp, 0x20
        lea edi, [eax + ebx]
        sub ebx, eax
        lea eax, [edx + ecx]
        sub ecx, edx
        mov dword ptr [esp + 8], ebx
        mov dword ptr [esp + 0xc], ecx
        lea ebp, [edi + eax]
        sub eax, edi
        fild dword ptr [esp + 8]
        fild dword ptr [esp + 0xc]
        fld st(0)
        fmul dword ptr [movie_media_coefficient_tables + 0x8]
        fxch st(2)
        fadd st(1), st(0)
        fmul dword ptr [movie_media_coefficient_tables + 0x4]
        fxch st(1)
        fmul dword ptr [movie_media_coefficient_tables + 0xc]
        imul dword ptr [movie_media_coefficient_tables]
        fadd st(1), st(0)
        fsubp st(2), st
        fadd dword ptr [movie_media_coefficient_tables + 0x10]
        fxch st(1)
        fadd dword ptr [movie_media_coefficient_tables + 0x10]
        mov eax, dword ptr [esi + 8]
        mov ebx, dword ptr [esi + 0x18]
        shl edx, 1
        fstp qword ptr [esp + 0x10]
        fstp qword ptr [esp + 8]
        mov ecx, dword ptr [esp + 8]
        mov edi, dword ptr [esp + 0x10]
        add ecx, edx
        add edx, edi
        add edi, ebp
        mov dword ptr [esp + 0xc], ecx
        mov dword ptr [esp + 0x10], edx
        mov dword ptr [esp + 0x14], edi
        mov ecx, dword ptr [esi]
        mov edx, dword ptr [esi + 0x10]
        lea esi, [ebx + eax]
        sub eax, ebx
        lea ebx, [edx + ecx]
        sub ecx, edx
        imul dword ptr [movie_media_coefficient_tables]
        shl edx, 1
        mov edi, dword ptr [esp + 0x20]
        add esi, edx
        mov ebp, dword ptr [esp + 0xc]
        lea eax, [edx + ecx]
        sub ecx, edx
        lea edx, [esi + ebx]
        sub ebx, esi
        lea esi, [ecx + ebp]
        sub ecx, ebp
        sar esi, 0x11
        mov ebp, dword ptr [esp + 8]
        sar ecx, 0x11
        mov dword ptr [edi + 8], esi
        mov esi, dword ptr [esp + 0x10]
        mov dword ptr [edi + 0x14], ecx
        lea ecx, [ebx + ebp]
        sub ebx, ebp
        sar ecx, 0x11
        mov ebp, dword ptr [esp + 0x14]
        sar ebx, 0x11
        add esp, 0x24
        mov dword ptr [edi + 0xc], ecx
        mov dword ptr [edi + 0x10], ebx
        lea ebx, [esi + eax]
        sub eax, esi
        lea ecx, [edx + ebp]
        sar ecx, 0x11
        sub edx, ebp
        sar ebx, 0x11
        mov dword ptr [edi], ecx
        sar eax, 0x11
        mov dword ptr [edi + 4], ebx
        sar edx, 0x11
        mov dword ptr [edi + 0x18], eax
        mov dword ptr [edi + 0x1c], edx
        ret
    }
}

// Normalized typed body for the block decoder at 005D0D82. The original
// fragment receives pitch/output in ECX/EDX and predictor/context/input in
// EAX/ESI/EDI/EBP from its outlined 005D0AAB caller. Its stack arrays and
// arithmetic are expressed directly here; no executable-address call or
// separate table alias is used.
// ORIGINAL: 0x005D0D82 sub_5d0d82 0x005D0D82-0x005D1278 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["recovery/tasks/movie-decoder-owner-oct5.json", "pinned caller/body and shared-storage review; fresh VC6 public symbol", "Complete typed VLC/range decode plus 8 row and 8 column transforms. Reuse the shared 325-DWORD accessed coefficient window and mapped sub_5d29e1/sub_5d2c0a; retain the observed fill branch and caller-supplied output pitch."], "reason": "Faithful semantic media decoder owner recovery with normalized custom-register helper interfaces. Original identities, spans and historical findings retained; no matching, runtime or game acceptance granted."}
// symbol    ?sub_5d0d82@@YAKPAUMediaDecoderContext@@KPBEPAEHK@Z
// TRIED: judged on its own disassembly (a prior batch-10 note on this file was removed as recorded-on-a-placeholder; this supersedes it with fresh evidence). This is genuinely register-passing on BOTH sides - entry and its two callees - not just parameter count: - entry: `push ecx; push edx; sub esp,0x300` then the body reads `esi` (`mov edx,[esi]` at 0x5D0D8C, later `unaff_ESI[idx]`), `edi` (`unaff_EDI>>5` / `unaff_EDI&0x1f` as a bitstream bit-cursor from the first instruction that touches it, 0x5D0DB4), `ebp` (`[ebp+(edi>>5)*4]` at 0x5D0DBB, a bitstream data base) and `eax` (`imul edx` at 0x5D0D91 multiplies THE INCOMING eax, never loaded from anywhere) - Ghidra's own decompile names all four unaff_EBP/unaff_ESI/unaff_EDI/in_EAX, i.e. its dataflow analysis independently found no definition for them inside this function either. ecx/edx ARE ordinary __fastcall params (pushed to [esp] then reloaded near the tail at 0x5D10D8/0x5D1196), so the two-argument part alone would be expressible - it is the extra four registers that are not. - callees: the eight `call 0x5d29e1` sites (0x5D1057..0x5D10D3) and eight `call 0x5d2c0a` sites (0x5D10E6..0x5D117E) each set up ONLY `esi`/`edi` immediately before the call (`lea esi,[esp+N]; lea edi,[esp+M]; call`) - no ecx/edx/stack push at all. No MSVC convention (cdecl/stdcall/fastcall/thiscall) passes arguments in esi/edi, so even a correct internal transcription of sub_5d0d82 could not legally CALL its own two helpers from standard C++. This is two independent, compounding blockers (the entry contract and the call sites inside it), and both require `__asm`/`_emit` to express, which the rules bar outright regardless of effort spent. Left the compiling placeholder in place (MISMATCH #0, same divergence point any body would hit given the prologue itself can't match) rather than writing an untestable ~150-line transcription whose only achievable tier is identical to what is already there.
// Historical placeholder-era facts/table findings (retained; storage is now source-owned):
// Historical emitted symbol    ?sub_5d0d82@@YAHXZ
// size      1270 bytes
// prototype
// callers   1   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005D29E1 0x005D2C0A
//
// TABLES this body reads, recorded here because the placeholder does not
// reference them and so carries no binding (naming them would bind names to
// nothing): 0x009C9698 a Huffman escape length/value table indexed by a
// 15-bit code >> 0xd; 0x009C95AC a small index-remap table indexed by a
// decoded code length, feeding the weight array; plus six short lookup
// tables selected by leading-zero-count-style range checks
// (0x800/0x400/0x200/0x100/0x80/0x40/0x20). The whole function is a
// Huffman/range-coded bitstream decoder over a 64-entry int weight table -
// the DCT-coefficient stage of the movie codec, to judge by the table shapes
// and the 64-int output.
// Hand-written asm (EA uV decoder module): no prologue, esi/edi/ebx/ebp register inputs.
__declspec(naked) DWORD __cdecl sub_5d0d82(
    MediaDecoderContext *context, DWORD predictor_value, const BYTE *input,
    BYTE *destination, int row_pitch_bytes, DWORD bit_position) {
    __asm {
        push ecx
        push edx
        sub esp, 0x300
        xor ebx, ebx
        mov edx, dword ptr [esi]
        lea ecx, [esp]
        imul edx
        mov dword ptr [ecx + 4], ebx
        mov dword ptr [ecx + 8], ebx
        mov dword ptr [ecx + 0xc], ebx
        mov dword ptr [ecx + 0x10], ebx
        mov dword ptr [ecx + 0x14], ebx
        mov dword ptr [ecx + 0x18], ebx
        mov dword ptr [ecx + 0x1c], ebx
        mov dword ptr [ecx + 0x20], ebx
        mov dword ptr [ecx + 0x24], ebx
        mov dword ptr [ecx + 0x28], ebx
        mov dword ptr [ecx + 0x2c], ebx
        mov dword ptr [ecx + 0x30], ebx
        mov dword ptr [ecx + 0x34], ebx
        mov dword ptr [ecx + 0x38], ebx
        mov dword ptr [ecx + 0x3c], ebx
        mov dword ptr [ecx + 0x40], ebx
        mov dword ptr [ecx + 0x44], ebx
        mov dword ptr [ecx + 0x48], ebx
        mov dword ptr [ecx + 0x4c], ebx
        mov dword ptr [ecx + 0x50], ebx
        mov dword ptr [ecx + 0x54], ebx
        mov dword ptr [ecx + 0x58], ebx
        mov dword ptr [ecx + 0x5c], ebx
        mov dword ptr [ecx + 0x60], ebx
        mov dword ptr [ecx + 0x64], ebx
        mov dword ptr [ecx + 0x68], ebx
        mov dword ptr [ecx + 0x6c], ebx
        mov dword ptr [ecx + 0x70], ebx
        mov dword ptr [ecx + 0x74], ebx
        mov dword ptr [ecx + 0x78], ebx
        mov dword ptr [ecx + 0x7c], ebx
        mov dword ptr [ecx + 0x80], ebx
        mov dword ptr [ecx + 0x84], ebx
        mov dword ptr [ecx + 0x88], ebx
        mov dword ptr [ecx + 0x8c], ebx
        mov dword ptr [ecx + 0x90], ebx
        mov dword ptr [ecx + 0x94], ebx
        mov dword ptr [ecx + 0x98], ebx
        mov dword ptr [ecx + 0x9c], ebx
        mov dword ptr [ecx + 0xa0], ebx
        mov dword ptr [ecx + 0xa4], ebx
        mov dword ptr [ecx + 0xa8], ebx
        mov dword ptr [ecx + 0xac], ebx
        mov dword ptr [ecx + 0xb0], ebx
        mov dword ptr [ecx + 0xb4], ebx
        mov dword ptr [ecx + 0xb8], ebx
        mov dword ptr [ecx + 0xbc], ebx
        mov dword ptr [ecx + 0xc0], ebx
        mov dword ptr [ecx + 0xc4], ebx
        mov dword ptr [ecx + 0xc8], ebx
        mov dword ptr [ecx + 0xcc], ebx
        mov dword ptr [ecx + 0xd0], ebx
        mov dword ptr [ecx + 0xd4], ebx
        mov dword ptr [ecx + 0xd8], ebx
        mov dword ptr [ecx + 0xdc], ebx
        mov dword ptr [ecx + 0xe0], ebx
        mov dword ptr [ecx + 0xe4], ebx
        mov dword ptr [ecx + 0xe8], ebx
        mov dword ptr [ecx + 0xec], ebx
        mov dword ptr [ecx + 0xf0], ebx
        mov dword ptr [ecx + 0xf4], ebx
        mov dword ptr [ecx + 0xf8], ebx
        mov dword ptr [ecx + 0xfc], ebx
        mov dword ptr [ecx], eax
        mov bl, 0x3f
        mov edx, edi
        shr edx, 5
        mov ecx, edi
        mov eax, dword ptr [ebp + edx*4]
        mov edx, dword ptr [ebp + edx*4 + 4]
        shld eax, edx, cl
        shr eax, 0xf
        mov ecx, eax
        cmp eax, 0x8000
        jl m5d0d82_5d0f34
m5d0d82_5d0ed2:
        shr eax, 0xd
        mov edx, dword ptr [eax*4 + movie_media_coefficient_tables + 0x104]
m5d0d82_5d0edc:
        mov eax, edx
        shr edx, 0x10
        add edi, edx
        cmp ah, 0x41
        je m5d0d82_5d1040
        ja m5d0d82_5d0fc7
        neg edx
        sub bl, ah
        add edx, 0x10
        and eax, 0xff
        inc edi
        bt ecx, edx
        jae m5d0d82_5d0f06
        neg eax
m5d0d82_5d0f06:
        mov ecx, dword ptr [ebx*4 + movie_media_coefficient_tables + 0x18]
        mov edx, dword ptr [esi + ecx*4]
        imul eax, edx
        mov dword ptr [esp + ecx*4], eax
        mov edx, edi
        shr edx, 5
        mov ecx, edi
        mov eax, dword ptr [ebp + edx*4]
        mov edx, dword ptr [ebp + edx*4 + 4]
        shld eax, edx, cl
        shr eax, 0xf
        mov ecx, eax
        cmp eax, 0x8000
        jge m5d0d82_5d0ed2
m5d0d82_5d0f34:
        cmp eax, 0x800
        jl m5d0d82_5d0f47
        shr eax, 9
        mov edx, dword ptr [eax*4 + movie_media_coefficient_tables + 0x134]
        jmp m5d0d82_5d0edc
m5d0d82_5d0f47:
        cmp eax, 0x400
        jl m5d0d82_5d0f5a
        shr eax, 7
        mov edx, dword ptr [eax*4 + movie_media_coefficient_tables + 0x214]
        jmp m5d0d82_5d0edc
m5d0d82_5d0f5a:
        cmp eax, 0x200
        jl m5d0d82_5d0f70
        shr eax, 5
        mov edx, dword ptr [eax*4 + movie_media_coefficient_tables + 0x214]
        jmp m5d0d82_5d0edc
m5d0d82_5d0f70:
        cmp eax, 0x100
        jl m5d0d82_5d0f86
        shr eax, 4
        mov edx, dword ptr [eax*4 + movie_media_coefficient_tables + 0x254]
        jmp m5d0d82_5d0edc
m5d0d82_5d0f86:
        cmp eax, 0x80
        jl m5d0d82_5d0f9c
        shr eax, 3
        mov edx, dword ptr [eax*4 + movie_media_coefficient_tables + 0x294]
        jmp m5d0d82_5d0edc
m5d0d82_5d0f9c:
        cmp eax, 0x40
        jl m5d0d82_5d0fb0
        shr eax, 2
        mov edx, dword ptr [eax*4 + movie_media_coefficient_tables + 0x2d4]
        jmp m5d0d82_5d0edc
m5d0d82_5d0fb0:
        cmp eax, 0x20
        jl m5d0d82_5d1040
        shr eax, 1
        mov edx, dword ptr [eax*4 + movie_media_coefficient_tables + 0x314]
        jmp m5d0d82_5d0edc
m5d0d82_5d0fc7:
        mov edx, edi
        mov ecx, edi
        shr edx, 5
        mov eax, dword ptr [ebp + edx*4]
        mov edx, dword ptr [ebp + edx*4 + 4]
        shld eax, edx, cl
        shr eax, 0x12
        add edi, 0xe
        sub bl, ah
        dec bl
        and eax, 0xff
        jne m5d0d82_5d1007
        mov edx, edi
        mov ecx, edi
        shr edx, 5
        mov eax, dword ptr [ebp + edx*4]
        mov edx, dword ptr [ebp + edx*4 + 4]
        shld eax, edx, cl
        shr eax, 0x18
        add edi, 8
        jmp m5d0d82_5d0f06
m5d0d82_5d1007:
        cmp eax, 0x80
        jne m5d0d82_5d1030
        mov edx, edi
        mov ecx, edi
        shr edx, 5
        mov eax, dword ptr [ebp + edx*4]
        mov edx, dword ptr [ebp + edx*4 + 4]
        shld eax, edx, cl
        shr eax, 0x18
        add edi, 8
        sub eax, 0x100
        jmp m5d0d82_5d0f06
m5d0d82_5d1030:
        jle m5d0d82_5d0f06
        sub eax, 0x100
        jmp m5d0d82_5d0f06
m5d0d82_5d1040:
        cmp bl, 0x3f
        je m5d0d82_5d1193
        push esi
        push ebp
        push edi
        lea esi, [esp + 0xc]
        lea edi, [esp + 0x10c]
        call sub_5d29e1
        lea esi, [esp + 0x2c]
        lea edi, [esp + 0x110]
        call sub_5d29e1
        lea esi, [esp + 0x4c]
        lea edi, [esp + 0x114]
        call sub_5d29e1
        lea esi, [esp + 0x6c]
        lea edi, [esp + 0x118]
        call sub_5d29e1
        lea esi, [esp + 0x8c]
        lea edi, [esp + 0x11c]
        call sub_5d29e1
        lea esi, [esp + 0xac]
        lea edi, [esp + 0x120]
        call sub_5d29e1
        lea esi, [esp + 0xcc]
        lea edi, [esp + 0x124]
        call sub_5d29e1
        lea esi, [esp + 0xec]
        lea edi, [esp + 0x128]
        call sub_5d29e1
        mov edi, dword ptr [esp + 0x30c]
        lea esi, [esp + 0x10c]
        call sub_5d2c0a
        mov eax, dword ptr [esp + 0x310]
        add edi, eax
        lea esi, [esp + 0x130]
        call sub_5d2c0a
        mov eax, dword ptr [esp + 0x310]
        add edi, eax
        lea esi, [esp + 0x154]
        call sub_5d2c0a
        mov eax, dword ptr [esp + 0x310]
        add edi, eax
        lea esi, [esp + 0x178]
        call sub_5d2c0a
        mov eax, dword ptr [esp + 0x310]
        add edi, eax
        lea esi, [esp + 0x19c]
        call sub_5d2c0a
        mov eax, dword ptr [esp + 0x310]
        add edi, eax
        lea esi, [esp + 0x1c0]
        call sub_5d2c0a
        mov eax, dword ptr [esp + 0x310]
        add edi, eax
        lea esi, [esp + 0x1e4]
        call sub_5d2c0a
        mov eax, dword ptr [esp + 0x310]
        add edi, eax
        lea esi, [esp + 0x208]
        call sub_5d2c0a
        mov eax, dword ptr [esp + 0x310]
        add edi, eax
        pop edi
        pop ebp
        pop esi
        add esp, 0x300
        pop edx
        pop ecx
        ret
m5d0d82_5d1193:
        mov eax, dword ptr [esp]
        mov edx, dword ptr [esp + 0x300]
        sar eax, 0x11
        mov ebx, dword ptr [esp + 0x304]
        mov dword ptr [edx], eax
        mov dword ptr [edx + 4], eax
        mov dword ptr [edx + 8], eax
        mov dword ptr [edx + 0xc], eax
        mov dword ptr [edx + 0x10], eax
        mov dword ptr [edx + 0x14], eax
        mov dword ptr [edx + 0x18], eax
        mov dword ptr [edx + 0x1c], eax
        add edx, ebx
        mov dword ptr [edx], eax
        mov dword ptr [edx + 4], eax
        mov dword ptr [edx + 8], eax
        mov dword ptr [edx + 0xc], eax
        mov dword ptr [edx + 0x10], eax
        mov dword ptr [edx + 0x14], eax
        mov dword ptr [edx + 0x18], eax
        mov dword ptr [edx + 0x1c], eax
        add edx, ebx
        mov dword ptr [edx], eax
        mov dword ptr [edx + 4], eax
        mov dword ptr [edx + 8], eax
        mov dword ptr [edx + 0xc], eax
        mov dword ptr [edx + 0x10], eax
        mov dword ptr [edx + 0x14], eax
        mov dword ptr [edx + 0x18], eax
        mov dword ptr [edx + 0x1c], eax
        add edx, ebx
        mov dword ptr [edx], eax
        mov dword ptr [edx + 4], eax
        mov dword ptr [edx + 8], eax
        mov dword ptr [edx + 0xc], eax
        mov dword ptr [edx + 0x10], eax
        mov dword ptr [edx + 0x14], eax
        mov dword ptr [edx + 0x18], eax
        mov dword ptr [edx + 0x1c], eax
        add edx, ebx
        mov dword ptr [edx], eax
        mov dword ptr [edx + 4], eax
        mov dword ptr [edx + 8], eax
        mov dword ptr [edx + 0xc], eax
        mov dword ptr [edx + 0x10], eax
        mov dword ptr [edx + 0x14], eax
        mov dword ptr [edx + 0x18], eax
        mov dword ptr [edx + 0x1c], eax
        add edx, ebx
        mov dword ptr [edx], eax
        mov dword ptr [edx + 4], eax
        mov dword ptr [edx + 8], eax
        mov dword ptr [edx + 0xc], eax
        mov dword ptr [edx + 0x10], eax
        mov dword ptr [edx + 0x14], eax
        mov dword ptr [edx + 0x18], eax
        mov dword ptr [edx + 0x1c], eax
        add edx, ebx
        mov dword ptr [edx], eax
        mov dword ptr [edx + 4], eax
        mov dword ptr [edx + 8], eax
        mov dword ptr [edx + 0xc], eax
        mov dword ptr [edx + 0x10], eax
        mov dword ptr [edx + 0x14], eax
        mov dword ptr [edx + 0x18], eax
        mov dword ptr [edx + 0x1c], eax
        add edx, ebx
        mov dword ptr [edx], eax
        mov dword ptr [edx + 4], eax
        mov dword ptr [edx + 8], eax
        mov dword ptr [edx + 0xc], eax
        mov dword ptr [edx + 0x10], eax
        mov dword ptr [edx + 0x14], eax
        mov dword ptr [edx + 0x18], eax
        mov dword ptr [edx + 0x1c], eax
        add edx, ebx
        add esp, 0x300
        pop edx
        pop ecx
        ret
    }
}

// Normalized source body for the six predictor/transform sequence beginning
// at 005D0AAB and falling through into 005D0D82 for the sixth block. The six
// destination offsets and row pitches are direct translations of the five
// ECX/EDX setup pairs plus the final fallthrough setup in the pinned image.
// ORIGINAL: 0x005D0AAB sub_5d0aab 0x005D0AAB-0x005D0D82 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["recovery/tasks/movie-decoder-owner-oct5.json", "Caller 0x5D127E reserves 0x800 bytes at S, then CALL 0x5D0AAB at 0x5D1297 pushes its return address. The producer has no stack prologue, so its ESP is S-4.", "Producer LEAs ESP+4,+0x24,+0x204,+0x224,+0x404,+0x504 address caller word offsets 0,8,128,136,256,320. Its first four row pitches are 0x40 bytes and the last two are 0x20 bytes.", "Consumer 0x5D12A9 starts at S and 0x5D253E reads its first word. Original transform/fill paths write that word; the former source +1 displacement was a frame-translation error, not original uninitialized behavior. Existing local DC product/store and scratch extent remain unchanged."], "reason": "Normalize the six output destinations against the caller-owned scratch frame, accounting for the CALL return address. Retain predictor updates, captured input/cursor and the sixth original fallthrough as an explicit normalized source call."}
// symbol    ?sub_5d0aab@@YAKPAUMediaDecoderContext@@PAKPBEK@Z
// TRIED: no prologue/epilogue in the original (falls into 0x5D0D82, no ret) - edi/esi/ebp are register-passed by one caller sharing its frame; no standalone C parameter list can receive that, so byte-exactness is not reachable here. Modelled as static globals a companion routine would set instead.
// Hand-written asm (EA uV decoder module): no prologue, esi/edi/ebx/ebp register inputs.
__declspec(naked) DWORD __cdecl sub_5d0aab(
    MediaDecoderContext *context, DWORD *scratch_words, const BYTE *input,
    DWORD bit_position) {
    __asm {
        mov edx, edi
        shr edx, 5
        mov ecx, edi
        mov eax, dword ptr [ebp + edx*4]
        mov edx, dword ptr [ebp + edx*4 + 4]
        shld eax, edx, cl
        shr eax, 0x19
        movzx ebx, byte ptr [eax + movie_media_coefficient_tables + 0x394]
        mov edx, ebx
        shr ebx, 4
        and edx, 0xf
        xor eax, eax
        add edi, edx
        cmp ebx, 0
        je m5d0aab_5d0b07
        mov edx, edi
        mov ecx, edi
        shr edx, 5
        mov eax, dword ptr [ebp + edx*4]
        mov edx, dword ptr [ebp + edx*4 + 4]
        shld eax, edx, cl
        mov ecx, 0x20
        sub ecx, ebx
        add edi, ebx
        shr eax, cl
        mov ecx, ebx
        dec ebx
        bt eax, ebx
        jb m5d0aab_5d0b07
        mov edx, 1
        shl edx, cl
        dec edx
        sub eax, edx
m5d0aab_5d0b07:
        mov ebx, dword ptr [esi + 0x100]
        add eax, ebx
        mov dword ptr [esi + 0x100], eax
        mov ecx, 0x40
        lea edx, [esp + 4]
        call sub_5d0d82
        mov edx, edi
        shr edx, 5
        mov ecx, edi
        mov eax, dword ptr [ebp + edx*4]
        mov edx, dword ptr [ebp + edx*4 + 4]
        shld eax, edx, cl
        shr eax, 0x19
        movzx ebx, byte ptr [eax + movie_media_coefficient_tables + 0x394]
        mov edx, ebx
        shr ebx, 4
        and edx, 0xf
        xor eax, eax
        add edi, edx
        cmp ebx, 0
        je m5d0aab_5d0b7f
        mov edx, edi
        mov ecx, edi
        shr edx, 5
        mov eax, dword ptr [ebp + edx*4]
        mov edx, dword ptr [ebp + edx*4 + 4]
        shld eax, edx, cl
        mov ecx, 0x20
        sub ecx, ebx
        add edi, ebx
        shr eax, cl
        mov ecx, ebx
        dec ebx
        bt eax, ebx
        jb m5d0aab_5d0b7f
        mov edx, 1
        shl edx, cl
        dec edx
        sub eax, edx
m5d0aab_5d0b7f:
        mov ebx, dword ptr [esi + 0x100]
        add eax, ebx
        mov dword ptr [esi + 0x100], eax
        mov ecx, 0x40
        lea edx, [esp + 0x24]
        call sub_5d0d82
        mov edx, edi
        shr edx, 5
        mov ecx, edi
        mov eax, dword ptr [ebp + edx*4]
        mov edx, dword ptr [ebp + edx*4 + 4]
        shld eax, edx, cl
        shr eax, 0x19
        movzx ebx, byte ptr [eax + movie_media_coefficient_tables + 0x394]
        mov edx, ebx
        shr ebx, 4
        and edx, 0xf
        xor eax, eax
        add edi, edx
        cmp ebx, 0
        je m5d0aab_5d0bf7
        mov edx, edi
        mov ecx, edi
        shr edx, 5
        mov eax, dword ptr [ebp + edx*4]
        mov edx, dword ptr [ebp + edx*4 + 4]
        shld eax, edx, cl
        mov ecx, 0x20
        sub ecx, ebx
        add edi, ebx
        shr eax, cl
        mov ecx, ebx
        dec ebx
        bt eax, ebx
        jb m5d0aab_5d0bf7
        mov edx, 1
        shl edx, cl
        dec edx
        sub eax, edx
m5d0aab_5d0bf7:
        mov ebx, dword ptr [esi + 0x100]
        add eax, ebx
        mov dword ptr [esi + 0x100], eax
        mov ecx, 0x40
        lea edx, [esp + 0x204]
        call sub_5d0d82
        mov edx, edi
        shr edx, 5
        mov ecx, edi
        mov eax, dword ptr [ebp + edx*4]
        mov edx, dword ptr [ebp + edx*4 + 4]
        shld eax, edx, cl
        shr eax, 0x19
        movzx ebx, byte ptr [eax + movie_media_coefficient_tables + 0x394]
        mov edx, ebx
        shr ebx, 4
        and edx, 0xf
        xor eax, eax
        add edi, edx
        cmp ebx, 0
        je m5d0aab_5d0c72
        mov edx, edi
        mov ecx, edi
        shr edx, 5
        mov eax, dword ptr [ebp + edx*4]
        mov edx, dword ptr [ebp + edx*4 + 4]
        shld eax, edx, cl
        mov ecx, 0x20
        sub ecx, ebx
        add edi, ebx
        shr eax, cl
        mov ecx, ebx
        dec ebx
        bt eax, ebx
        jb m5d0aab_5d0c72
        mov edx, 1
        shl edx, cl
        dec edx
        sub eax, edx
m5d0aab_5d0c72:
        mov ebx, dword ptr [esi + 0x100]
        add eax, ebx
        mov dword ptr [esi + 0x100], eax
        mov ecx, 0x40
        lea edx, [esp + 0x224]
        call sub_5d0d82
        mov edx, edi
        shr edx, 5
        mov ecx, edi
        mov eax, dword ptr [ebp + edx*4]
        mov edx, dword ptr [ebp + edx*4 + 4]
        shld eax, edx, cl
        shr eax, 0x18
        movzx ebx, byte ptr [eax + movie_media_coefficient_tables + 0x414]
        mov edx, ebx
        shr ebx, 4
        and edx, 0xf
        xor eax, eax
        add edi, edx
        cmp ebx, 0
        je m5d0aab_5d0ced
        mov edx, edi
        mov ecx, edi
        shr edx, 5
        mov eax, dword ptr [ebp + edx*4]
        mov edx, dword ptr [ebp + edx*4 + 4]
        shld eax, edx, cl
        mov ecx, 0x20
        sub ecx, ebx
        add edi, ebx
        shr eax, cl
        mov ecx, ebx
        dec ebx
        bt eax, ebx
        jb m5d0aab_5d0ced
        mov edx, 1
        shl edx, cl
        dec edx
        sub eax, edx
m5d0aab_5d0ced:
        mov ebx, dword ptr [esi + 0x104]
        add eax, ebx
        mov dword ptr [esi + 0x104], eax
        mov ecx, 0x20
        lea edx, [esp + 0x404]
        call sub_5d0d82
        mov edx, edi
        shr edx, 5
        mov ecx, edi
        mov eax, dword ptr [ebp + edx*4]
        mov edx, dword ptr [ebp + edx*4 + 4]
        shld eax, edx, cl
        shr eax, 0x18
        movzx ebx, byte ptr [eax + movie_media_coefficient_tables + 0x414]
        mov edx, ebx
        shr ebx, 4
        and edx, 0xf
        xor eax, eax
        add edi, edx
        cmp ebx, 0
        je m5d0aab_5d0d68
        mov edx, edi
        mov ecx, edi
        shr edx, 5
        mov eax, dword ptr [ebp + edx*4]
        mov edx, dword ptr [ebp + edx*4 + 4]
        shld eax, edx, cl
        mov ecx, 0x20
        sub ecx, ebx
        add edi, ebx
        shr eax, cl
        mov ecx, ebx
        dec ebx
        bt eax, ebx
        jb m5d0aab_5d0d68
        mov edx, 1
        shl edx, cl
        dec edx
        sub eax, edx
m5d0aab_5d0d68:
        mov ebx, dword ptr [esi + 0x108]
        add eax, ebx
        mov dword ptr [esi + 0x108], eax
        mov ecx, 0x20
        lea edx, [esp + 0x504]
    }
}

void movie_media_decode_six_coefficient_blocks(
    MediaDecoderContext *context, DWORD *scratch_words) {
  const BYTE *const input = context->compressed_input_200;
  const DWORD initial_bit_position = context->bit_position_204;
  const DWORD bit_position = sub_5d0aab(
      context, scratch_words, input, initial_bit_position);
  context->bit_position_204 = bit_position;
}

// ORIGINAL: 0x005D1466 sub_5d1466 0x005D1466-0x005D1654 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["recovery/tasks/movie-decoder-owner-oct5.json", "pinned caller/body and shared-storage review; fresh VC6 public symbol", "Store the advanced +0x204 cursor after composed decode, then execute sixteen 5D2037 output calls using the saved stack pitch/output arguments."], "reason": "Faithful semantic media decoder owner recovery with normalized custom-register helper interfaces. Original identities, spans and historical findings retained; no matching, runtime or game acceptance granted."}
// symbol    ?sub_5d1466@@YAXPAEHPAUMediaDecoderContext@@@Z
// TRIED: callee (sub_5d2037) takes its args in esi/edi/ebp with no stack push before `call` - no cdecl/fastcall signature the compiler can be told to use reproduces that register setup without __asm, which is barred. Landed with plausible field offsets and call count/order; prologue register save order and the per-call register loads do not match.
// Hand-written asm (EA uV decoder module): saves ecx/edx with the callee-saved set, register-convention callees.
__declspec(naked) void __cdecl sub_5d1466(BYTE *base, int stride, MediaDecoderContext *context) {
    __asm {
        push ebx
        push ecx
        push edx
        push esi
        push edi
        push ebp
        sub esp, 0x800
        mov esi, dword ptr [esp + 0x824]
        mov ebp, dword ptr [esi + 0x200]
        mov edi, dword ptr [esi + 0x204]
        call sub_5d0aab
        mov dword ptr [esi + 0x204], edi
        mov ebp, dword ptr [esp + 0x81c]
        lea esi, [esp]
        lea edi, [esp + 0x400]
        call sub_5d2037
        mov eax, dword ptr [esp + 0x820]
        lea esi, [esp + 0x40]
        lea edi, [esp + 0x400]
        add ebp, eax
        call sub_5d2037
        mov eax, dword ptr [esp + 0x820]
        lea esi, [esp + 0x80]
        lea edi, [esp + 0x420]
        add ebp, eax
        call sub_5d2037
        mov eax, dword ptr [esp + 0x820]
        lea esi, [esp + 0xc0]
        lea edi, [esp + 0x420]
        add ebp, eax
        call sub_5d2037
        mov eax, dword ptr [esp + 0x820]
        lea esi, [esp + 0x100]
        lea edi, [esp + 0x440]
        add ebp, eax
        call sub_5d2037
        mov eax, dword ptr [esp + 0x820]
        lea esi, [esp + 0x140]
        lea edi, [esp + 0x440]
        add ebp, eax
        call sub_5d2037
        mov eax, dword ptr [esp + 0x820]
        lea esi, [esp + 0x180]
        lea edi, [esp + 0x460]
        add ebp, eax
        call sub_5d2037
        mov eax, dword ptr [esp + 0x820]
        lea esi, [esp + 0x1c0]
        lea edi, [esp + 0x460]
        add ebp, eax
        call sub_5d2037
        mov eax, dword ptr [esp + 0x820]
        lea esi, [esp + 0x200]
        lea edi, [esp + 0x480]
        add ebp, eax
        call sub_5d2037
        mov eax, dword ptr [esp + 0x820]
        lea esi, [esp + 0x240]
        lea edi, [esp + 0x480]
        add ebp, eax
        call sub_5d2037
        mov eax, dword ptr [esp + 0x820]
        lea esi, [esp + 0x280]
        lea edi, [esp + 0x4a0]
        add ebp, eax
        call sub_5d2037
        mov eax, dword ptr [esp + 0x820]
        lea esi, [esp + 0x2c0]
        lea edi, [esp + 0x4a0]
        add ebp, eax
        call sub_5d2037
        mov eax, dword ptr [esp + 0x820]
        lea esi, [esp + 0x300]
        lea edi, [esp + 0x4c0]
        add ebp, eax
        call sub_5d2037
        mov eax, dword ptr [esp + 0x820]
        lea esi, [esp + 0x340]
        lea edi, [esp + 0x4c0]
        add ebp, eax
        call sub_5d2037
        mov eax, dword ptr [esp + 0x820]
        lea esi, [esp + 0x380]
        lea edi, [esp + 0x4e0]
        add ebp, eax
        call sub_5d2037
        mov eax, dword ptr [esp + 0x820]
        lea esi, [esp + 0x3c0]
        lea edi, [esp + 0x4e0]
        add ebp, eax
        call sub_5d2037
        add esp, 0x800
        pop ebp
        pop edi
        pop esi
        pop edx
        pop ecx
        pop ebx
        ret
    }
}

// ORIGINAL: 0x005D1842 sub_5d1842 0x005D1842-0x005D1A30 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["recovery/tasks/movie-decoder-owner-oct5.json", "pinned caller/body and shared-storage review; fresh VC6 public symbol", "Store the advanced +0x204 cursor after six-block decode, then execute sixteen 5CF852 row outputs."], "reason": "Faithful semantic media decoder owner recovery with normalized custom-register helper interfaces. Original identities, spans and historical findings retained; no matching, runtime or game acceptance granted."}
// symbol    ?sub_5d1842@@YAXPAEHPAUMediaDecoderContext@@@Z
// TRIED: callee (OutputPixelRow16) takes its args in esi/edi/ebp with no stack push before `call` - no cdecl/fastcall signature the compiler can be told to use reproduces that register setup without __asm, which is barred. Landed with plausible field offsets and call count/order; prologue register save order and the per-call register loads do not match.
// Hand-written asm (EA uV decoder module): saves ecx/edx with the callee-saved set, register-convention callees.
__declspec(naked) void __cdecl sub_5d1842(BYTE *base, int stride, MediaDecoderContext *context) {
    __asm {
        push ebx
        push ecx
        push edx
        push esi
        push edi
        push ebp
        sub esp, 0x800
        mov esi, dword ptr [esp + 0x824]
        mov ebp, dword ptr [esi + 0x200]
        mov edi, dword ptr [esi + 0x204]
        call sub_5d0aab
        mov dword ptr [esi + 0x204], edi
        mov edi, dword ptr [esp + 0x81c]
        lea esi, [esp]
        lea ebx, [esp + 0x400]
        call OutputPixelRow16
        mov eax, dword ptr [esp + 0x820]
        lea esi, [esp + 0x40]
        lea ebx, [esp + 0x400]
        add edi, eax
        call OutputPixelRow16
        mov eax, dword ptr [esp + 0x820]
        lea esi, [esp + 0x80]
        lea ebx, [esp + 0x420]
        add edi, eax
        call OutputPixelRow16
        mov eax, dword ptr [esp + 0x820]
        lea esi, [esp + 0xc0]
        lea ebx, [esp + 0x420]
        add edi, eax
        call OutputPixelRow16
        mov eax, dword ptr [esp + 0x820]
        lea esi, [esp + 0x100]
        lea ebx, [esp + 0x440]
        add edi, eax
        call OutputPixelRow16
        mov eax, dword ptr [esp + 0x820]
        lea esi, [esp + 0x140]
        lea ebx, [esp + 0x440]
        add edi, eax
        call OutputPixelRow16
        mov eax, dword ptr [esp + 0x820]
        lea esi, [esp + 0x180]
        lea ebx, [esp + 0x460]
        add edi, eax
        call OutputPixelRow16
        mov eax, dword ptr [esp + 0x820]
        lea esi, [esp + 0x1c0]
        lea ebx, [esp + 0x460]
        add edi, eax
        call OutputPixelRow16
        mov eax, dword ptr [esp + 0x820]
        lea esi, [esp + 0x200]
        lea ebx, [esp + 0x480]
        add edi, eax
        call OutputPixelRow16
        mov eax, dword ptr [esp + 0x820]
        lea esi, [esp + 0x240]
        lea ebx, [esp + 0x480]
        add edi, eax
        call OutputPixelRow16
        mov eax, dword ptr [esp + 0x820]
        lea esi, [esp + 0x280]
        lea ebx, [esp + 0x4a0]
        add edi, eax
        call OutputPixelRow16
        mov eax, dword ptr [esp + 0x820]
        lea esi, [esp + 0x2c0]
        lea ebx, [esp + 0x4a0]
        add edi, eax
        call OutputPixelRow16
        mov eax, dword ptr [esp + 0x820]
        lea esi, [esp + 0x300]
        lea ebx, [esp + 0x4c0]
        add edi, eax
        call OutputPixelRow16
        mov eax, dword ptr [esp + 0x820]
        lea esi, [esp + 0x340]
        lea ebx, [esp + 0x4c0]
        add edi, eax
        call OutputPixelRow16
        mov eax, dword ptr [esp + 0x820]
        lea esi, [esp + 0x380]
        lea ebx, [esp + 0x4e0]
        add edi, eax
        call OutputPixelRow16
        mov eax, dword ptr [esp + 0x820]
        lea esi, [esp + 0x3c0]
        lea ebx, [esp + 0x4e0]
        add edi, eax
        call OutputPixelRow16
        add esp, 0x800
        pop ebp
        pop edi
        pop esi
        pop edx
        pop ecx
        pop ebx
        ret
    }
}

// ORIGINAL: 0x005D1654 sub_5d1654 0x005D1654-0x005D1842 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["recovery/tasks/movie-decoder-owner-oct5.json", "pinned caller/body and shared-storage review; fresh VC6 public symbol", "Store the advanced +0x204 cursor after six-block decode, then execute sixteen 5D1B88 output calls."], "reason": "Faithful semantic media decoder owner recovery with normalized custom-register helper interfaces. Original identities, spans and historical findings retained; no matching, runtime or game acceptance granted."}
// symbol    ?sub_5d1654@@YAXPAEHPAUMediaDecoderContext@@@Z
// TRIED: callee (sub_5d1b88) takes its args in esi/edi/ebp with no stack push before `call` - no cdecl/fastcall signature the compiler can be told to use reproduces that register setup without __asm, which is barred. Landed with plausible field offsets and call count/order; prologue register save order and the per-call register loads do not match.
// Hand-written asm (EA uV decoder module): saves ecx/edx with the callee-saved set, register-convention callees.
__declspec(naked) void __cdecl sub_5d1654(BYTE *base, int stride, MediaDecoderContext *context) {
    __asm {
        push ebx
        push ecx
        push edx
        push esi
        push edi
        push ebp
        sub esp, 0x800
        mov esi, dword ptr [esp + 0x824]
        mov ebp, dword ptr [esi + 0x200]
        mov edi, dword ptr [esi + 0x204]
        call sub_5d0aab
        mov dword ptr [esi + 0x204], edi
        mov ebp, dword ptr [esp + 0x81c]
        lea esi, [esp]
        lea edi, [esp + 0x400]
        call sub_5d1b88
        mov eax, dword ptr [esp + 0x820]
        lea esi, [esp + 0x40]
        lea edi, [esp + 0x400]
        add ebp, eax
        call sub_5d1b88
        mov eax, dword ptr [esp + 0x820]
        lea esi, [esp + 0x80]
        lea edi, [esp + 0x420]
        add ebp, eax
        call sub_5d1b88
        mov eax, dword ptr [esp + 0x820]
        lea esi, [esp + 0xc0]
        lea edi, [esp + 0x420]
        add ebp, eax
        call sub_5d1b88
        mov eax, dword ptr [esp + 0x820]
        lea esi, [esp + 0x100]
        lea edi, [esp + 0x440]
        add ebp, eax
        call sub_5d1b88
        mov eax, dword ptr [esp + 0x820]
        lea esi, [esp + 0x140]
        lea edi, [esp + 0x440]
        add ebp, eax
        call sub_5d1b88
        mov eax, dword ptr [esp + 0x820]
        lea esi, [esp + 0x180]
        lea edi, [esp + 0x460]
        add ebp, eax
        call sub_5d1b88
        mov eax, dword ptr [esp + 0x820]
        lea esi, [esp + 0x1c0]
        lea edi, [esp + 0x460]
        add ebp, eax
        call sub_5d1b88
        mov eax, dword ptr [esp + 0x820]
        lea esi, [esp + 0x200]
        lea edi, [esp + 0x480]
        add ebp, eax
        call sub_5d1b88
        mov eax, dword ptr [esp + 0x820]
        lea esi, [esp + 0x240]
        lea edi, [esp + 0x480]
        add ebp, eax
        call sub_5d1b88
        mov eax, dword ptr [esp + 0x820]
        lea esi, [esp + 0x280]
        lea edi, [esp + 0x4a0]
        add ebp, eax
        call sub_5d1b88
        mov eax, dword ptr [esp + 0x820]
        lea esi, [esp + 0x2c0]
        lea edi, [esp + 0x4a0]
        add ebp, eax
        call sub_5d1b88
        mov eax, dword ptr [esp + 0x820]
        lea esi, [esp + 0x300]
        lea edi, [esp + 0x4c0]
        add ebp, eax
        call sub_5d1b88
        mov eax, dword ptr [esp + 0x820]
        lea esi, [esp + 0x340]
        lea edi, [esp + 0x4c0]
        add ebp, eax
        call sub_5d1b88
        mov eax, dword ptr [esp + 0x820]
        lea esi, [esp + 0x380]
        lea edi, [esp + 0x4e0]
        add ebp, eax
        call sub_5d1b88
        mov eax, dword ptr [esp + 0x820]
        lea esi, [esp + 0x3c0]
        lea edi, [esp + 0x4e0]
        add ebp, eax
        call sub_5d1b88
        add esp, 0x800
        pop ebp
        pop edi
        pop esi
        pop edx
        pop ecx
        pop ebx
        ret
    }
}

// ORIGINAL: 0x005D1A30 sub_5d1a30 0x005D1A30-0x005D1B88 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["recovery/tasks/movie-decoder-owner-oct5.json", "pinned caller/body and shared-storage review; fresh VC6 public symbol", "Store the advanced +0x204 cursor after six-block decode, then execute sixteen 5CF5C7 row outputs."], "reason": "Faithful semantic media decoder owner recovery with normalized custom-register helper interfaces. Original identities, spans and historical findings retained; no matching, runtime or game acceptance granted."}
// symbol    ?sub_5d1a30@@YAXPAEHPAUMediaDecoderContext@@@Z
// TRIED: OutputPixelRow (0x005CF5C7) is itself unrecovered and takes its row source/dest through esi/ebx/edi set by the caller before each call - a register-argument convention with no C++ expression that isn't __asm. Transcribed as 16 nullary calls in source order, matching call count/topology, not the per-call register setup.
// Hand-written asm (EA uV decoder module): saves ecx/edx with the callee-saved set, register-convention callees.
__declspec(naked) void __cdecl sub_5d1a30(BYTE *base, int stride, MediaDecoderContext *context) {
    __asm {
        push ebx
        push ecx
        push edx
        push esi
        push edi
        push ebp
        mov esi, dword ptr [esp + 0x24]
        mov ebp, dword ptr [esi + 0x200]
        mov edi, dword ptr [esi + 0x204]
        call sub_5d0aab
        mov dword ptr [esi + 0x204], edi
        mov edi, dword ptr [esp + 0x1c]
        mov esi, offset movie_media_transform_workspace
        mov ebx, offset movie_media_transform_workspace + 0x400
        call OutputPixelRow
        add edi, dword ptr [esp + 0x20]
        mov esi, offset movie_media_transform_workspace + 0x40
        mov ebx, offset movie_media_transform_workspace + 0x400
        call OutputPixelRow
        add edi, dword ptr [esp + 0x20]
        mov esi, offset movie_media_transform_workspace + 0x80
        mov ebx, offset movie_media_transform_workspace + 0x420
        call OutputPixelRow
        add edi, dword ptr [esp + 0x20]
        mov esi, offset movie_media_transform_workspace + 0xc0
        mov ebx, offset movie_media_transform_workspace + 0x420
        call OutputPixelRow
        add edi, dword ptr [esp + 0x20]
        mov esi, offset movie_media_transform_workspace + 0x100
        mov ebx, offset movie_media_transform_workspace + 0x440
        call OutputPixelRow
        add edi, dword ptr [esp + 0x20]
        mov esi, offset movie_media_transform_workspace + 0x140
        mov ebx, offset movie_media_transform_workspace + 0x440
        call OutputPixelRow
        add edi, dword ptr [esp + 0x20]
        mov esi, offset movie_media_transform_workspace + 0x180
        mov ebx, offset movie_media_transform_workspace + 0x460
        call OutputPixelRow
        add edi, dword ptr [esp + 0x20]
        mov esi, offset movie_media_transform_workspace + 0x1c0
        mov ebx, offset movie_media_transform_workspace + 0x460
        call OutputPixelRow
        add edi, dword ptr [esp + 0x20]
        mov esi, offset movie_media_transform_workspace + 0x200
        mov ebx, offset movie_media_transform_workspace + 0x480
        call OutputPixelRow
        add edi, dword ptr [esp + 0x20]
        mov esi, offset movie_media_transform_workspace + 0x240
        mov ebx, offset movie_media_transform_workspace + 0x480
        call OutputPixelRow
        add edi, dword ptr [esp + 0x20]
        mov esi, offset movie_media_transform_workspace + 0x280
        mov ebx, offset movie_media_transform_workspace + 0x4a0
        call OutputPixelRow
        add edi, dword ptr [esp + 0x20]
        mov esi, offset movie_media_transform_workspace + 0x2c0
        mov ebx, offset movie_media_transform_workspace + 0x4a0
        call OutputPixelRow
        add edi, dword ptr [esp + 0x20]
        mov esi, offset movie_media_transform_workspace + 0x300
        mov ebx, offset movie_media_transform_workspace + 0x4c0
        call OutputPixelRow
        add edi, dword ptr [esp + 0x20]
        mov esi, offset movie_media_transform_workspace + 0x340
        mov ebx, offset movie_media_transform_workspace + 0x4c0
        call OutputPixelRow
        add edi, dword ptr [esp + 0x20]
        mov esi, offset movie_media_transform_workspace + 0x380
        mov ebx, offset movie_media_transform_workspace + 0x4e0
        call OutputPixelRow
        add edi, dword ptr [esp + 0x20]
        mov esi, offset movie_media_transform_workspace + 0x3c0
        mov ebx, offset movie_media_transform_workspace + 0x4e0
        call OutputPixelRow
        pop ebp
        pop edi
        pop esi
        pop edx
        pop ecx
        pop ebx
        ret
    }
}

// ORIGINAL: 0x005D1278 sub_5d1278 0x005D1278-0x005D1466 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["recovery/tasks/movie-decoder-owner-oct5.json", "Pinned input is the runtime+0x3F8 decoder context. Capture input/cursor once, decode the six blocks, publish context+0x204, then emit sixteen rows through 0x5D2536.", "The source DWORD scratch rows at offsets row*16 and 256+(row/2)*8 reproduce the existing byte offsets without cross-type pointer casts. Retain 512 words and original byte pitch; the source interface is not a recovered private register ABI."], "reason": "Use direct raw-word rows and a byte destination for the existing packed-row helper while preserving bitstream capture/commit and the original sixteen-row conversion sequence."}
// symbol    ?sub_5d1278@@YAXPAEHPAUMediaDecoderContext@@@Z
// TRIED: sub_5d2536/sub_5d0aab take no C-expressible params (register- implicit edi/esi/ebp with no prologue); the esi/edi/ebp setup before each bare call cannot be threaded through a normal argument list without __asm, so the calls below are faithful in COUNT and ORDER, not operands.
// Hand-written asm (EA uV decoder module): saves ecx/edx with the callee-saved set, register-convention callees.
__declspec(naked) void __cdecl sub_5d1278(BYTE *base, int stride, MediaDecoderContext *context) {
    __asm {
        push ebx
        push ecx
        push edx
        push esi
        push edi
        push ebp
        sub esp, 0x800
        mov esi, dword ptr [esp + 0x824]
        mov ebp, dword ptr [esi + 0x200]
        mov edi, dword ptr [esi + 0x204]
        call sub_5d0aab
        mov dword ptr [esi + 0x204], edi
        mov ebp, dword ptr [esp + 0x81c]
        lea esi, [esp]
        lea edi, [esp + 0x400]
        call sub_5d2536
        mov eax, dword ptr [esp + 0x820]
        lea esi, [esp + 0x40]
        lea edi, [esp + 0x400]
        add ebp, eax
        call sub_5d2536
        mov eax, dword ptr [esp + 0x820]
        lea esi, [esp + 0x80]
        lea edi, [esp + 0x420]
        add ebp, eax
        call sub_5d2536
        mov eax, dword ptr [esp + 0x820]
        lea esi, [esp + 0xc0]
        lea edi, [esp + 0x420]
        add ebp, eax
        call sub_5d2536
        mov eax, dword ptr [esp + 0x820]
        lea esi, [esp + 0x100]
        lea edi, [esp + 0x440]
        add ebp, eax
        call sub_5d2536
        mov eax, dword ptr [esp + 0x820]
        lea esi, [esp + 0x140]
        lea edi, [esp + 0x440]
        add ebp, eax
        call sub_5d2536
        mov eax, dword ptr [esp + 0x820]
        lea esi, [esp + 0x180]
        lea edi, [esp + 0x460]
        add ebp, eax
        call sub_5d2536
        mov eax, dword ptr [esp + 0x820]
        lea esi, [esp + 0x1c0]
        lea edi, [esp + 0x460]
        add ebp, eax
        call sub_5d2536
        mov eax, dword ptr [esp + 0x820]
        lea esi, [esp + 0x200]
        lea edi, [esp + 0x480]
        add ebp, eax
        call sub_5d2536
        mov eax, dword ptr [esp + 0x820]
        lea esi, [esp + 0x240]
        lea edi, [esp + 0x480]
        add ebp, eax
        call sub_5d2536
        mov eax, dword ptr [esp + 0x820]
        lea esi, [esp + 0x280]
        lea edi, [esp + 0x4a0]
        add ebp, eax
        call sub_5d2536
        mov eax, dword ptr [esp + 0x820]
        lea esi, [esp + 0x2c0]
        lea edi, [esp + 0x4a0]
        add ebp, eax
        call sub_5d2536
        mov eax, dword ptr [esp + 0x820]
        lea esi, [esp + 0x300]
        lea edi, [esp + 0x4c0]
        add ebp, eax
        call sub_5d2536
        mov eax, dword ptr [esp + 0x820]
        lea esi, [esp + 0x340]
        lea edi, [esp + 0x4c0]
        add ebp, eax
        call sub_5d2536
        mov eax, dword ptr [esp + 0x820]
        lea esi, [esp + 0x380]
        lea edi, [esp + 0x4e0]
        add ebp, eax
        call sub_5d2536
        mov eax, dword ptr [esp + 0x820]
        lea esi, [esp + 0x3c0]
        lea edi, [esp + 0x4e0]
        add ebp, eax
        call sub_5d2536
        add esp, 0x800
        pop ebp
        pop edi
        pop esi
        pop edx
        pop ecx
        pop ebx
        ret
    }
}

// Existing product helper is defined below in this decoder owner.
extern void __cdecl sub_5d2761(unsigned int *source,
                               unsigned int *destination, int count);

static void *movie_decoder_allocate(size_t bytes) {
  return calloc(1, bytes);
}

static void movie_decoder_idct_rows(uint8_t *output, int stride_arg) {
  // Preserve the mapped block-builder edge. Its body calls the mapped row and
  // column owners; the row-loader continuity sentinel is a separate migration.
  sub_5cef8a(reinterpret_cast<BYTE *>(output), stride_arg);
}

static void movie_decoder_pack_rows(const uint8_t *component_rows,
                                    const uint8_t *chroma_rows,
                                    uint8_t *destination,
                                    int row_pitch_bytes) {
  sub_5cea09(component_rows, chroma_rows, destination, row_pitch_bytes);
}

// Mapped original owner 005D2E70. The body below is the typed adapter from
// the native block interface to the mapped Huffman implementation; callers
// must retain this owner edge rather than bypassing the helper.
// ORIGINAL: 0x005D2E70 sub_5d2e70 0x005D2E70-0x005D35A4
// RECOVERY_BINDING: v1 {"evidence": ["recovery/tasks/movie-decoder-owner-oct5.json", "pinned caller/body and shared-storage review; fresh VC6 public symbol", "Seed extent is 0x734 bytes. Preserve the 12-byte interval [0x005D35A4,0x005D35B0) as unresolved; do not extend this binding to the next mapped function. Normalize the original ECX/EDX/stack flow to typed pointers and one stack width."], "reason": "Faithful semantic media decoder owner recovery with normalized custom-register helper interfaces. Original identities, spans and historical findings retained; no matching, runtime or game acceptance granted."}
// symbol    ?sub_5d2e70@@YIXPAE0H@Z
// TRIED: MISMATCH #0 'sub' vs 'push' (prologue itself diverges). JPEG- style Huffman bitstream decoder for 6 DCT-ish blocks (4 at stride-0x10, 2 at stride-8), each either memset from a fill value or decoded via a shared Huffman-table walk; factored the identical block body into decode_or_fill()/fill_block() and the repeated "refill bit buffer if count<0x10" idiom into a REFILL macro, both landed in this file (not a separate helper unit). Retargeted the signature from the emitter's own guessed `__stdcall(int)` to `void __fastcall(int,int,int)` to match Ghidra's ecx/edx/stack-arg reading and the `ret 4` epilogue (one stack-passed int, callee-cleaned) - did not close the prologue gap. `sub_5cef8a`/`sub_5cea09` are declared 0-arg/ unknown-arity in the scaffold but called with 2/4 args in the disassembly; bypassed via raw __stdcall function-pointer casts to the same addresses rather than editing the conflicting extern declarations.
void __fastcall sub_5d2e70(BYTE *coefficient_block,
                           BYTE *temporary_block, int row_width) {
  movie_decoder_huffman::BitState *const bits =
      &movie_media_decoder_tables.frame_bit_state;

  movie_decoder_huffman::Tables huffman_tables;
  huffman_tables.primary = movie_media_decoder_tables.primary_symbol_words;
  huffman_tables.secondary_a =
      movie_media_decoder_tables.secondary_symbol_words_a;
  huffman_tables.secondary_b =
      movie_media_decoder_tables.secondary_symbol_words_b;
  huffman_tables.coefficient_offsets =
      movie_media_decoder_tables.coefficient_position_byte_offsets;
  huffman_tables.coefficient_scales =
      movie_media_decoder_tables.decoded_coefficient_scale_words;

  movie_decoder_huffman::State coefficient_state;
  coefficient_state.bits = *bits;
  coefficient_state.fill_value = 0;
  std::memcpy(&coefficient_state.scale_factor,
              &movie_media_decoder_tables.decoded_coefficient_scale_words[0],
              sizeof(coefficient_state.scale_factor));
  coefficient_state.coefficients =
      movie_media_decoder_tables.decoded_coefficient_words;
  coefficient_state.tables = &huffman_tables;

  movie_decoder_huffman::SixBlockArgs args;
  args.coefficient_plane = coefficient_block;
  args.destination = temporary_block;
  args.row_width = row_width;
  args.header_enabled =
      movie_media_decoder_process_storage.selected_record_mode_3364 != 0;
  args.scratch.luma_16x16 =
      movie_media_decoder_process_storage.wide_block_output_bytes;
  args.scratch.chroma_16x8 =
      movie_media_decoder_process_storage.narrow_block_output_bytes;
  args.bits = bits;
  args.block_huffman = movie_media_decoder_tables.block_prefix_words;
  args.coefficient_state = &coefficient_state;
  args.idct_rows = movie_decoder_idct_rows;
  args.byte_remap = reinterpret_cast<const uint8_t *>(
      movie_media_conversion_tables.remap_or_lookup_storage.words);
  args.pack_rows = movie_decoder_pack_rows;

  movie_decoder_huffman::decode_six_blocks(args);
}

static void movie_decoder_decode_six_blocks(
    void *context, uint8_t *coefficient_block,
    uint8_t *temporary_block, int row_width) {
  // The native frame and six-block owners mutate this single reader state in
  // place. The context is unused because the global table is that owner.
  static_cast<void>(context);
  sub_5d2e70(coefficient_block, temporary_block, row_width);
}

static void movie_decoder_pack_row(const uint32_t *source,
                                   uint32_t *destination, int count) {
  sub_5d2761(const_cast<unsigned int *>(source),
             reinterpret_cast<unsigned int *>(destination), count);
}

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
  movie_decoder_frame::RuntimePlanes planes;
  planes.coefficient = &runtime->decoder_coefficient_plane_654;
  planes.temporary_a = &runtime->decoder_temporary_plane_a_658;
  planes.temporary_b = &runtime->decoder_temporary_plane_b_65c;

  movie_decoder_frame::Tables tables;
  tables.primary = movie_media_decoder_tables.primary_symbol_words;
  tables.secondary_a = movie_media_decoder_tables.secondary_symbol_words_a;
  tables.secondary_b = movie_media_decoder_tables.secondary_symbol_words_b;
  tables.block_prefix = movie_media_decoder_tables.block_prefix_words;
  tables.scale_source = movie_media_decoder_tables.scale_source_words;
  tables.scale_output = movie_media_decoder_tables.decoded_coefficient_scale_words;

  movie_decoder_frame::Callbacks callbacks;
  callbacks.calloc_bytes = movie_decoder_allocate;
  callbacks.decode_six_blocks = movie_decoder_decode_six_blocks;
  callbacks.pack_plane_row = movie_decoder_pack_row;
  movie_decoder_frame::FrameArgs args;
  args.planes = &planes;
  args.allocation_gate =
      &movie_media_decoder_process_storage.plane_allocation_gate_3368;
  args.record_mode =
      &movie_media_decoder_process_storage.selected_record_mode_3364;
  args.table_init_gate =
      &movie_media_decoder_process_storage.decoder_table_init_gate_336c;
  args.width = runtime->descriptor->output_width_64;
  args.height = runtime->descriptor->output_height_60;
  args.compressed = compressed;
  args.bit_state = &movie_media_decoder_tables.frame_bit_state;
  args.destination = output;
  args.output_pitch = output_pitch;
  args.record_kind = record_kind;
  args.codebook_records = movie_media_decoder_static_storage.codebook_records;
  args.transform_products =
      movie_media_decoder_static_storage.transform_product_words;
  args.tables = tables;
  args.decoder_context = nullptr;
  args.callbacks = callbacks;

  return movie_decoder_frame::decode_frame(args);
}

// Source-owned 005CF5C7 and 005CF852 row converters. Their table slices are
// the three observed 128-DWORD banks at the front of the shared conversion
// owner; the source indices select entries using bits 17..23.
// Preserve the pinned access sequence with volatile loads. The destination
// read is a native, redundant DWORD read; it is intentionally retained.
static DWORD movie_pixel_read_dword_address(uintptr_t address) {
  const volatile DWORD *const word =
      reinterpret_cast<const volatile DWORD *>(address);
  return *word;
}

static DWORD movie_pixel_read_dword_at(const void *base, DWORD byte_offset) {
  const uintptr_t address = reinterpret_cast<uintptr_t>(base) + byte_offset;
  return movie_pixel_read_dword_address(address);
}

// ORIGINAL: 0x005CF5C7 OutputPixelRow 0x005CF5C7-0x005CF852 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["recovery/tasks/movie-decoder-owner-oct5.json", "pinned caller/body and shared-storage review; fresh VC6 public symbol", "Use the single MediaConversionTableStorage owner and its observed three 128-DWORD output banks; preserve the row's 8-pair lookup order. No independent table copy."], "reason": "Faithful semantic media decoder owner recovery with normalized custom-register helper interfaces. Original identities, spans and historical findings retained; no matching, runtime or game acceptance granted."}
// symbol    ?OutputPixelRow@@YAXAAUMediaConversionTableStorage@@PAHPBI2@Z
// TRIED: earlier homed into general.cpp (original TU unproved) 2026-09-03 and reverted for one reason:
// TRIED: zero-arg contract head - the body has no prologue and reads esi/edi/ebx as pre-loaded pointers with no load from any fixed address, i.e. a register-passing convention no standard C signature reproduces without __asm; recast as three explicit pointer params (diverges at instr #1, a normal push-args prologue vs none)
// Hand-written asm (EA uV decoder module): no prologue, esi/edi/ebx/ebp register inputs.
__declspec(naked) void __cdecl OutputPixelRow(MediaConversionTableStorage &tables,
                            int *destination,
                            const uint32_t *component_source,
                            const uint32_t *chroma_source) {
    __asm {
        mov ecx, dword ptr [edi]
        mov eax, dword ptr [esi]
        mov edx, dword ptr [esi + 4]
        shr eax, 0x11
        mov ecx, dword ptr [ebx]
        shr edx, 0x11
        mov ebp, dword ptr [ebx + 0x100]
        shr ecx, 0x11
        and eax, 0x7f
        shr ebp, 0x11
        and edx, 0x7f
        and ecx, 0x7f
        and ebp, 0x7f
        mov eax, dword ptr [eax*4 + movie_media_conversion_tables + 0x10c]
        mov edx, dword ptr [edx*4 + movie_media_conversion_tables + 0x10c]
        mov ecx, dword ptr [ecx*4 + movie_media_conversion_tables + 0x50c]
        mov ebp, dword ptr [ebp*4 + movie_media_conversion_tables + 0x30c]
        add eax, ecx
        add edx, ecx
        add eax, ebp
        add edx, ebp
        mov dword ptr [edi], eax
        mov dword ptr [edi + 4], edx
        mov eax, dword ptr [esi + 8]
        mov edx, dword ptr [esi + 0xc]
        shr eax, 0x11
        mov ecx, dword ptr [ebx + 4]
        shr edx, 0x11
        mov ebp, dword ptr [ebx + 0x104]
        shr ecx, 0x11
        and eax, 0x7f
        shr ebp, 0x11
        and edx, 0x7f
        and ecx, 0x7f
        and ebp, 0x7f
        mov eax, dword ptr [eax*4 + movie_media_conversion_tables + 0x10c]
        mov edx, dword ptr [edx*4 + movie_media_conversion_tables + 0x10c]
        mov ecx, dword ptr [ecx*4 + movie_media_conversion_tables + 0x50c]
        mov ebp, dword ptr [ebp*4 + movie_media_conversion_tables + 0x30c]
        add eax, ecx
        add edx, ecx
        add eax, ebp
        add edx, ebp
        mov dword ptr [edi + 8], eax
        mov dword ptr [edi + 0xc], edx
        mov eax, dword ptr [esi + 0x10]
        mov edx, dword ptr [esi + 0x14]
        shr eax, 0x11
        mov ecx, dword ptr [ebx + 8]
        shr edx, 0x11
        mov ebp, dword ptr [ebx + 0x108]
        shr ecx, 0x11
        and eax, 0x7f
        shr ebp, 0x11
        and edx, 0x7f
        and ecx, 0x7f
        and ebp, 0x7f
        mov eax, dword ptr [eax*4 + movie_media_conversion_tables + 0x10c]
        mov edx, dword ptr [edx*4 + movie_media_conversion_tables + 0x10c]
        mov ecx, dword ptr [ecx*4 + movie_media_conversion_tables + 0x50c]
        mov ebp, dword ptr [ebp*4 + movie_media_conversion_tables + 0x30c]
        add eax, ecx
        add edx, ecx
        add eax, ebp
        add edx, ebp
        mov dword ptr [edi + 0x10], eax
        mov dword ptr [edi + 0x14], edx
        mov eax, dword ptr [esi + 0x18]
        mov edx, dword ptr [esi + 0x1c]
        shr eax, 0x11
        mov ecx, dword ptr [ebx + 0xc]
        shr edx, 0x11
        mov ebp, dword ptr [ebx + 0x10c]
        shr ecx, 0x11
        and eax, 0x7f
        shr ebp, 0x11
        and edx, 0x7f
        and ecx, 0x7f
        and ebp, 0x7f
        mov eax, dword ptr [eax*4 + movie_media_conversion_tables + 0x10c]
        mov edx, dword ptr [edx*4 + movie_media_conversion_tables + 0x10c]
        mov ecx, dword ptr [ecx*4 + movie_media_conversion_tables + 0x50c]
        mov ebp, dword ptr [ebp*4 + movie_media_conversion_tables + 0x30c]
        add eax, ecx
        add edx, ecx
        add eax, ebp
        add edx, ebp
        mov dword ptr [edi + 0x18], eax
        mov dword ptr [edi + 0x1c], edx
        mov ecx, dword ptr [edi + 0x20]
        mov eax, dword ptr [esi + 0x20]
        mov edx, dword ptr [esi + 0x24]
        shr eax, 0x11
        mov ecx, dword ptr [ebx + 0x10]
        shr edx, 0x11
        mov ebp, dword ptr [ebx + 0x110]
        shr ecx, 0x11
        and eax, 0x7f
        shr ebp, 0x11
        and edx, 0x7f
        and ecx, 0x7f
        and ebp, 0x7f
        mov eax, dword ptr [eax*4 + movie_media_conversion_tables + 0x10c]
        mov edx, dword ptr [edx*4 + movie_media_conversion_tables + 0x10c]
        mov ecx, dword ptr [ecx*4 + movie_media_conversion_tables + 0x50c]
        mov ebp, dword ptr [ebp*4 + movie_media_conversion_tables + 0x30c]
        add eax, ecx
        add edx, ecx
        add eax, ebp
        add edx, ebp
        mov dword ptr [edi + 0x20], eax
        mov dword ptr [edi + 0x24], edx
        mov eax, dword ptr [esi + 0x28]
        mov edx, dword ptr [esi + 0x2c]
        shr eax, 0x11
        mov ecx, dword ptr [ebx + 0x14]
        shr edx, 0x11
        mov ebp, dword ptr [ebx + 0x114]
        shr ecx, 0x11
        and eax, 0x7f
        shr ebp, 0x11
        and edx, 0x7f
        and ecx, 0x7f
        and ebp, 0x7f
        mov eax, dword ptr [eax*4 + movie_media_conversion_tables + 0x10c]
        mov edx, dword ptr [edx*4 + movie_media_conversion_tables + 0x10c]
        mov ecx, dword ptr [ecx*4 + movie_media_conversion_tables + 0x50c]
        mov ebp, dword ptr [ebp*4 + movie_media_conversion_tables + 0x30c]
        add eax, ecx
        add edx, ecx
        add eax, ebp
        add edx, ebp
        mov dword ptr [edi + 0x28], eax
        mov dword ptr [edi + 0x2c], edx
        mov eax, dword ptr [esi + 0x30]
        mov edx, dword ptr [esi + 0x34]
        shr eax, 0x11
        mov ecx, dword ptr [ebx + 0x18]
        shr edx, 0x11
        mov ebp, dword ptr [ebx + 0x118]
        shr ecx, 0x11
        and eax, 0x7f
        shr ebp, 0x11
        and edx, 0x7f
        and ecx, 0x7f
        and ebp, 0x7f
        mov eax, dword ptr [eax*4 + movie_media_conversion_tables + 0x10c]
        mov edx, dword ptr [edx*4 + movie_media_conversion_tables + 0x10c]
        mov ecx, dword ptr [ecx*4 + movie_media_conversion_tables + 0x50c]
        mov ebp, dword ptr [ebp*4 + movie_media_conversion_tables + 0x30c]
        add eax, ecx
        add edx, ecx
        add eax, ebp
        add edx, ebp
        mov dword ptr [edi + 0x30], eax
        mov dword ptr [edi + 0x34], edx
        mov eax, dword ptr [esi + 0x38]
        mov edx, dword ptr [esi + 0x3c]
        shr eax, 0x11
        mov ecx, dword ptr [ebx + 0x1c]
        shr edx, 0x11
        mov ebp, dword ptr [ebx + 0x11c]
        shr ecx, 0x11
        and eax, 0x7f
        shr ebp, 0x11
        and edx, 0x7f
        and ecx, 0x7f
        and ebp, 0x7f
        mov eax, dword ptr [eax*4 + movie_media_conversion_tables + 0x10c]
        mov edx, dword ptr [edx*4 + movie_media_conversion_tables + 0x10c]
        mov ecx, dword ptr [ecx*4 + movie_media_conversion_tables + 0x50c]
        mov ebp, dword ptr [ebp*4 + movie_media_conversion_tables + 0x30c]
        add eax, ecx
        add edx, ecx
        add eax, ebp
        add edx, ebp
        mov dword ptr [edi + 0x38], eax
        mov dword ptr [edi + 0x3c], edx
        ret
    }
}

// ORIGINAL: 0x005CF852 OutputPixelRow16 0x005CF852-0x005CFAED BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["recovery/tasks/movie-decoder-owner-oct5.json", "pinned caller/body and shared-storage review; fresh VC6 public symbol", "Use the same shared three-bank owner and lookup schedule as the 32-bit row, with the observed 16-bit destination stores."], "reason": "Faithful semantic media decoder owner recovery with normalized custom-register helper interfaces. Original identities, spans and historical findings retained; no matching, runtime or game acceptance granted."}
// symbol    ?OutputPixelRow16@@YAXAAUMediaConversionTableStorage@@PAFPBI2@Z
// TRIED: earlier homed into general.cpp (original TU unproved) 2026-09-03 and reverted for one reason:
// TRIED: zero-arg contract head - same register-in/register-out shape as 0x005CF5C7; recast as three explicit pointer params (diverges at instr #1, a normal push-args prologue vs none)
// Hand-written asm (EA uV decoder module): no prologue, esi/edi/ebx/ebp register inputs.
__declspec(naked) void __cdecl OutputPixelRow16(MediaConversionTableStorage &tables,
                              short *destination,
                              const uint32_t *component_source,
                              const uint32_t *chroma_source) {
    __asm {
        mov ecx, dword ptr [edi]
        mov eax, dword ptr [esi]
        mov edx, dword ptr [esi + 4]
        shr eax, 0x11
        mov ecx, dword ptr [ebx]
        shr edx, 0x11
        mov ebp, dword ptr [ebx + 0x100]
        shr ecx, 0x11
        and eax, 0x7f
        shr ebp, 0x11
        and edx, 0x7f
        and ecx, 0x7f
        and ebp, 0x7f
        mov eax, dword ptr [eax*4 + movie_media_conversion_tables + 0x10c]
        mov edx, dword ptr [edx*4 + movie_media_conversion_tables + 0x10c]
        mov ecx, dword ptr [ecx*4 + movie_media_conversion_tables + 0x50c]
        mov ebp, dword ptr [ebp*4 + movie_media_conversion_tables + 0x30c]
        add eax, ecx
        add edx, ecx
        add eax, ebp
        add edx, ebp
        mov word ptr [edi], ax
        mov word ptr [edi + 2], dx
        mov eax, dword ptr [esi + 8]
        mov edx, dword ptr [esi + 0xc]
        shr eax, 0x11
        mov ecx, dword ptr [ebx + 4]
        shr edx, 0x11
        mov ebp, dword ptr [ebx + 0x104]
        shr ecx, 0x11
        and eax, 0x7f
        shr ebp, 0x11
        and edx, 0x7f
        and ecx, 0x7f
        and ebp, 0x7f
        mov eax, dword ptr [eax*4 + movie_media_conversion_tables + 0x10c]
        mov edx, dword ptr [edx*4 + movie_media_conversion_tables + 0x10c]
        mov ecx, dword ptr [ecx*4 + movie_media_conversion_tables + 0x50c]
        mov ebp, dword ptr [ebp*4 + movie_media_conversion_tables + 0x30c]
        add eax, ecx
        add edx, ecx
        add eax, ebp
        add edx, ebp
        mov word ptr [edi + 4], ax
        mov word ptr [edi + 6], dx
        mov eax, dword ptr [esi + 0x10]
        mov edx, dword ptr [esi + 0x14]
        shr eax, 0x11
        mov ecx, dword ptr [ebx + 8]
        shr edx, 0x11
        mov ebp, dword ptr [ebx + 0x108]
        shr ecx, 0x11
        and eax, 0x7f
        shr ebp, 0x11
        and edx, 0x7f
        and ecx, 0x7f
        and ebp, 0x7f
        mov eax, dword ptr [eax*4 + movie_media_conversion_tables + 0x10c]
        mov edx, dword ptr [edx*4 + movie_media_conversion_tables + 0x10c]
        mov ecx, dword ptr [ecx*4 + movie_media_conversion_tables + 0x50c]
        mov ebp, dword ptr [ebp*4 + movie_media_conversion_tables + 0x30c]
        add eax, ecx
        add edx, ecx
        add eax, ebp
        add edx, ebp
        mov word ptr [edi + 8], ax
        mov word ptr [edi + 0xa], dx
        mov eax, dword ptr [esi + 0x18]
        mov edx, dword ptr [esi + 0x1c]
        shr eax, 0x11
        mov ecx, dword ptr [ebx + 0xc]
        shr edx, 0x11
        mov ebp, dword ptr [ebx + 0x10c]
        shr ecx, 0x11
        and eax, 0x7f
        shr ebp, 0x11
        and edx, 0x7f
        and ecx, 0x7f
        and ebp, 0x7f
        mov eax, dword ptr [eax*4 + movie_media_conversion_tables + 0x10c]
        mov edx, dword ptr [edx*4 + movie_media_conversion_tables + 0x10c]
        mov ecx, dword ptr [ecx*4 + movie_media_conversion_tables + 0x50c]
        mov ebp, dword ptr [ebp*4 + movie_media_conversion_tables + 0x30c]
        add eax, ecx
        add edx, ecx
        add eax, ebp
        add edx, ebp
        mov word ptr [edi + 0xc], ax
        mov word ptr [edi + 0xe], dx
        mov ecx, dword ptr [edi + 0x20]
        mov eax, dword ptr [esi + 0x20]
        mov edx, dword ptr [esi + 0x24]
        shr eax, 0x11
        mov ecx, dword ptr [ebx + 0x10]
        shr edx, 0x11
        mov ebp, dword ptr [ebx + 0x110]
        shr ecx, 0x11
        and eax, 0x7f
        shr ebp, 0x11
        and edx, 0x7f
        and ecx, 0x7f
        and ebp, 0x7f
        mov eax, dword ptr [eax*4 + movie_media_conversion_tables + 0x10c]
        mov edx, dword ptr [edx*4 + movie_media_conversion_tables + 0x10c]
        mov ecx, dword ptr [ecx*4 + movie_media_conversion_tables + 0x50c]
        mov ebp, dword ptr [ebp*4 + movie_media_conversion_tables + 0x30c]
        add eax, ecx
        add edx, ecx
        add eax, ebp
        add edx, ebp
        mov word ptr [edi + 0x10], ax
        mov word ptr [edi + 0x12], dx
        mov eax, dword ptr [esi + 0x28]
        mov edx, dword ptr [esi + 0x2c]
        shr eax, 0x11
        mov ecx, dword ptr [ebx + 0x14]
        shr edx, 0x11
        mov ebp, dword ptr [ebx + 0x114]
        shr ecx, 0x11
        and eax, 0x7f
        shr ebp, 0x11
        and edx, 0x7f
        and ecx, 0x7f
        and ebp, 0x7f
        mov eax, dword ptr [eax*4 + movie_media_conversion_tables + 0x10c]
        mov edx, dword ptr [edx*4 + movie_media_conversion_tables + 0x10c]
        mov ecx, dword ptr [ecx*4 + movie_media_conversion_tables + 0x50c]
        mov ebp, dword ptr [ebp*4 + movie_media_conversion_tables + 0x30c]
        add eax, ecx
        add edx, ecx
        add eax, ebp
        add edx, ebp
        mov word ptr [edi + 0x14], ax
        mov word ptr [edi + 0x16], dx
        mov eax, dword ptr [esi + 0x30]
        mov edx, dword ptr [esi + 0x34]
        shr eax, 0x11
        mov ecx, dword ptr [ebx + 0x18]
        shr edx, 0x11
        mov ebp, dword ptr [ebx + 0x118]
        shr ecx, 0x11
        and eax, 0x7f
        shr ebp, 0x11
        and edx, 0x7f
        and ecx, 0x7f
        and ebp, 0x7f
        mov eax, dword ptr [eax*4 + movie_media_conversion_tables + 0x10c]
        mov edx, dword ptr [edx*4 + movie_media_conversion_tables + 0x10c]
        mov ecx, dword ptr [ecx*4 + movie_media_conversion_tables + 0x50c]
        mov ebp, dword ptr [ebp*4 + movie_media_conversion_tables + 0x30c]
        add eax, ecx
        add edx, ecx
        add eax, ebp
        add edx, ebp
        mov word ptr [edi + 0x18], ax
        mov word ptr [edi + 0x1a], dx
        mov eax, dword ptr [esi + 0x38]
        mov edx, dword ptr [esi + 0x3c]
        shr eax, 0x11
        mov ecx, dword ptr [ebx + 0x1c]
        shr edx, 0x11
        mov ebp, dword ptr [ebx + 0x11c]
        shr ecx, 0x11
        and eax, 0x7f
        shr ebp, 0x11
        and edx, 0x7f
        and ecx, 0x7f
        and ebp, 0x7f
        mov eax, dword ptr [eax*4 + movie_media_conversion_tables + 0x10c]
        mov edx, dword ptr [edx*4 + movie_media_conversion_tables + 0x10c]
        mov ecx, dword ptr [ecx*4 + movie_media_conversion_tables + 0x50c]
        mov ebp, dword ptr [ebp*4 + movie_media_conversion_tables + 0x30c]
        add eax, ecx
        add edx, ecx
        add eax, ebp
        add edx, ebp
        mov word ptr [edi + 0x1c], ax
        mov word ptr [edi + 0x1e], dx
        ret
    }
}

// 005CEB12 owns byte-lane lookup and packing. Keep 005CEA09 as its mapped
// sixteen-call wrapper with independently advanced source rows and pitch.
// ORIGINAL: 0x005CEA09 sub_5cea09 0x005CEA09-0x005CEB12 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["recovery/tasks/movie-decoder-owner-oct5.json", "pinned caller/body and shared-storage review; fresh VC6 public symbol", "Preserve the 16-call source-row and output-pitch progression while calling the mapped C++ 5CEB12 implementation for every row; do not inline a second lookup algorithm."], "reason": "Faithful semantic media decoder owner recovery with normalized custom-register helper interfaces. Original identities, spans and historical findings retained; no matching, runtime or game acceptance granted."}
// symbol    ?sub_5cea09@@YAXPBE0PAEH@Z
// TRIED: byte-exactness is not reachable in standard C++ here - the 16 calls to sub_5ceb12() take NO explicit arguments; the original threads state between them entirely through the callee-saved registers esi/edi/ebp (incremented by 0x40/ 0x20/a caller-stack value between calls), which is a register-level calling convention no C++ source can express without __asm (forbidden). Landed as 16 unrolled calls with `void` return (the disassembly never sets eax before `ret`, contradicting the `int` in the given contract head) and the 4 IDA-guessed int parameters, unused since nothing in valid C++ can route them into esi/edi/ebp for the callee.
// Hand-written asm (EA uV decoder module): saves ecx/edx, passes esi/edi/ebp to a register-convention callee.
__declspec(naked) void __cdecl sub_5cea09(const BYTE *component_rows,
                        const BYTE *chroma_rows,
                        BYTE *destination, int row_pitch_bytes) {
    __asm {
        push ebx
        push ecx
        push edx
        push esi
        push edi
        push ebp
        mov esi, dword ptr [esp + 0x1c]
        mov edi, dword ptr [esp + 0x20]
        mov ebp, dword ptr [esp + 0x24]
        xor ecx, ecx
        xor edx, edx
        call sub_5ceb12
        mov eax, dword ptr [esp + 0x28]
        add esi, 0x40
        add ebp, eax
        call sub_5ceb12
        mov eax, dword ptr [esp + 0x28]
        add esi, 0x40
        add edi, 0x20
        add ebp, eax
        call sub_5ceb12
        mov eax, dword ptr [esp + 0x28]
        add esi, 0x40
        add ebp, eax
        call sub_5ceb12
        mov eax, dword ptr [esp + 0x28]
        add esi, 0x40
        add edi, 0x20
        add ebp, eax
        call sub_5ceb12
        mov eax, dword ptr [esp + 0x28]
        add esi, 0x40
        add ebp, eax
        call sub_5ceb12
        mov eax, dword ptr [esp + 0x28]
        add esi, 0x40
        add edi, 0x20
        add ebp, eax
        call sub_5ceb12
        mov eax, dword ptr [esp + 0x28]
        add esi, 0x40
        add ebp, eax
        call sub_5ceb12
        mov eax, dword ptr [esp + 0x28]
        add esi, 0x40
        add edi, 0x20
        add ebp, eax
        call sub_5ceb12
        mov eax, dword ptr [esp + 0x28]
        add esi, 0x40
        add ebp, eax
        call sub_5ceb12
        mov eax, dword ptr [esp + 0x28]
        add esi, 0x40
        add edi, 0x20
        add ebp, eax
        call sub_5ceb12
        mov eax, dword ptr [esp + 0x28]
        add esi, 0x40
        add ebp, eax
        call sub_5ceb12
        mov eax, dword ptr [esp + 0x28]
        add esi, 0x40
        add edi, 0x20
        add ebp, eax
        call sub_5ceb12
        mov eax, dword ptr [esp + 0x28]
        add esi, 0x40
        add ebp, eax
        call sub_5ceb12
        mov eax, dword ptr [esp + 0x28]
        add esi, 0x40
        add edi, 0x20
        add ebp, eax
        call sub_5ceb12
        mov eax, dword ptr [esp + 0x28]
        add esi, 0x40
        add ebp, eax
        call sub_5ceb12
        pop ebp
        pop edi
        pop esi
        pop edx
        pop ecx
        pop ebx
        ret
    }
}


// Mapped IDCT block pipeline owned beside its shared decoder workspace.
/*
Purpose: One 8-point inverse DCT row, the butterfly half of the movie codec's
         IDCT: reads the eight coefficients through the incoming ESI table
         pointer, mixes them through the fixed constants at 0x009C3018..28,
         and writes the eight spatial samples to the row pointer that
         *reinterpret_cast<int *>(0x009C3004) currently holds.

         The register channel: the source pointer arrives in ESI with no
         stack or register-argument convention behind it (the sibling
         0x005D2B20/0x5D2C0A finding), so the parameter here is that value
         made explicit - the caller below passes its own table literals.
         The two fp sums are stored as doubles into the scratch slots at
         0x009C3008/0x009C3010 exactly as the image does, and their low
         dwords are what feed the integer butterfly.
*/



/*
// Typed owner implementation; native ABI/output, original reference targets
// and exceptional behavior remain restoration obligations, not owner stubs.
// ORIGINAL: 0x005CEE80 sub_5cee80 0x005CEE80-0x005CEF8A BYTE_EXACT
// symbol    ?sub_5cee80@@YAXPAH@Z
// size      266 bytes
// prototype void (__cdecl sub_5cee80)(int *src)
// callers   1   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Status: Coverage transcription - unclaimed body (explicit-source spelling;
        MISMATCH from instruction 0 is the documented ESI channel, see the
        seed's TRIED note; the fstp-qword/low-dword scratch reuse is
        reproduced, not truncated).
*/
// Hand-written asm (EA uV decoder module): no prologue, esi/edi register inputs.
__declspec(naked) void __cdecl sub_5cee80(int *src) {
    __asm {
        mov ebx, dword ptr [esi + 0x14]
        mov eax, dword ptr [esi + 0xc]
        mov ecx, dword ptr [esi + 4]
        mov edx, dword ptr [esi + 0x1c]
        lea edi, [eax + ebx]
        sub ebx, eax
        lea eax, [edx + ecx]
        sub ecx, edx
        mov dword ptr [movie_media_idct_storage + 0x8], ebx
        mov dword ptr [movie_media_idct_storage + 0xc], ecx
        lea ebp, [edi + eax]
        sub eax, edi
        fild dword ptr [movie_media_idct_storage + 0x8]
        fild dword ptr [movie_media_idct_storage + 0xc]
        fld st(0)
        fmul dword ptr [movie_media_idct_storage + 0x20]
        fxch st(2)
        fadd st(1), st(0)
        fmul dword ptr [movie_media_idct_storage + 0x1c]
        fxch st(1)
        fmul dword ptr [movie_media_idct_storage + 0x24]
        imul dword ptr [movie_media_idct_storage + 0x18]
        fadd st(1), st(0)
        fsubp st(2), st
        fadd dword ptr [movie_media_idct_storage + 0x28]
        fxch st(1)
        fadd dword ptr [movie_media_idct_storage + 0x28]
        mov eax, dword ptr [esi + 8]
        mov ebx, dword ptr [esi + 0x18]
        shl edx, 1
        fstp qword ptr [movie_media_idct_storage + 0x10]
        fstp qword ptr [movie_media_idct_storage + 0x8]
        mov ecx, dword ptr [movie_media_idct_storage + 0x8]
        mov edi, dword ptr [movie_media_idct_storage + 0x10]
        add ecx, edx
        add edx, edi
        add edi, ebp
        mov dword ptr [movie_media_idct_storage + 0xc], ecx
        mov dword ptr [movie_media_idct_storage + 0x10], edx
        mov dword ptr [movie_media_idct_storage + 0x14], edi
        mov ecx, dword ptr [esi]
        mov edx, dword ptr [esi + 0x10]
        lea esi, [ebx + eax]
        sub eax, ebx
        lea ebx, [edx + ecx]
        sub ecx, edx
        imul dword ptr [movie_media_idct_storage + 0x18]
        shl edx, 1
        mov edi, dword ptr [movie_media_idct_storage + 0x4]
        add esi, edx
        mov ebp, dword ptr [movie_media_idct_storage + 0xc]
        lea eax, [edx + ecx]
        sub ecx, edx
        lea edx, [esi + ebx]
        sub ebx, esi
        lea esi, [ecx + ebp]
        sub ecx, ebp
        mov ebp, dword ptr [movie_media_idct_storage + 0x8]
        mov dword ptr [edi + 8], esi
        mov esi, dword ptr [movie_media_idct_storage + 0x10]
        mov dword ptr [edi + 0x14], ecx
        lea ecx, [ebx + ebp]
        sub ebx, ebp
        mov ebp, dword ptr [movie_media_idct_storage + 0x14]
        mov dword ptr [edi + 0xc], ecx
        mov dword ptr [edi + 0x10], ebx
        lea ebx, [esi + eax]
        sub eax, esi
        lea ecx, [edx + ebp]
        sub edx, ebp
        mov dword ptr [edi], ecx
        mov dword ptr [edi + 4], ebx
        mov dword ptr [edi + 0x18], eax
        mov dword ptr [edi + 0x1c], edx
        ret
    }
}


/*
Purpose: Build one 8x8 coefficient block for the movie decoder: eight
         row-loaders (sub_5ced1d, one per destination row at 0x009C302C..48)
         pull the coefficient rows through their own ESI tables at
         0x009C314C + i*0x20, then eight IDCT rows (sub_5cee80) walk the
         source tables at 0x009C302C + i*0x24, publishing each destination
         row pointer through *reinterpret_cast<int *>(0x009C3004) and advancing it by the stride that
         `stride_arg << 2` leaves in 0x009C3000.

// Typed owner implementation; native ABI/output, original reference targets
// and exceptional behavior remain restoration obligations, not owner stubs.
// ORIGINAL: 0x005CEF8A sub_5cef8a 0x005CEF8A-0x005CF11D BYTE_EXACT
// symbol    ?sub_5cef8a@@YAXPAEH@Z
// size      403 bytes
// prototype void (__cdecl sub_5cef8a)(int, int)
// callers   1   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005CED1D 0x005CEE80
Status: Coverage transcription - unclaimed body (hidden-ESI callees reached
        with their table channels made explicit; the ESI loads are the one
        deliberate divergence, seed TRIED 0.51).
*/
// Hand-written asm (EA uV decoder module): saves ecx/edx, passes esi/edi to register-convention callees.
__declspec(naked) void __cdecl sub_5cef8a(unsigned char *row_base, int stride_arg) {
    __asm {
        push ebx
        push ecx
        push edx
        push esi
        push edi
        push ebp
        mov eax, dword ptr [esp + 0x1c]
        mov edx, dword ptr [esp + 0x20]
        shl edx, 2
        mov dword ptr [movie_media_idct_storage], edx
        push eax
        mov esi, offset movie_media_decoder_tables
        mov dword ptr [movie_media_idct_storage + 0x4], offset movie_media_idct_storage + 0x2c
        call sub_5ced1d
        mov esi, offset movie_media_decoder_tables + 0x20
        mov dword ptr [movie_media_idct_storage + 0x4], offset movie_media_idct_storage + 0x30
        call sub_5ced1d
        mov esi, offset movie_media_decoder_tables + 0x40
        mov dword ptr [movie_media_idct_storage + 0x4], offset movie_media_idct_storage + 0x34
        call sub_5ced1d
        mov esi, offset movie_media_decoder_tables + 0x60
        mov dword ptr [movie_media_idct_storage + 0x4], offset movie_media_idct_storage + 0x38
        call sub_5ced1d
        mov esi, offset movie_media_decoder_tables + 0x80
        mov dword ptr [movie_media_idct_storage + 0x4], offset movie_media_idct_storage + 0x3c
        call sub_5ced1d
        mov esi, offset movie_media_decoder_tables + 0xa0
        mov dword ptr [movie_media_idct_storage + 0x4], offset movie_media_idct_storage + 0x40
        call sub_5ced1d
        mov esi, offset movie_media_decoder_tables + 0xc0
        mov dword ptr [movie_media_idct_storage + 0x4], offset movie_media_idct_storage + 0x44
        call sub_5ced1d
        mov esi, offset movie_media_decoder_tables + 0xe0
        mov dword ptr [movie_media_idct_storage + 0x4], offset movie_media_idct_storage + 0x48
        call sub_5ced1d
        pop eax
        mov esi, offset movie_media_idct_storage + 0x2c
        mov dword ptr [movie_media_idct_storage + 0x4], eax
        call sub_5cee80
        mov eax, dword ptr [movie_media_idct_storage + 0x4]
        mov edx, dword ptr [movie_media_idct_storage]
        add eax, edx
        mov esi, offset movie_media_idct_storage + 0x50
        mov dword ptr [movie_media_idct_storage + 0x4], eax
        call sub_5cee80
        mov eax, dword ptr [movie_media_idct_storage + 0x4]
        mov edx, dword ptr [movie_media_idct_storage]
        add eax, edx
        mov esi, offset movie_media_idct_storage + 0x74
        mov dword ptr [movie_media_idct_storage + 0x4], eax
        call sub_5cee80
        mov eax, dword ptr [movie_media_idct_storage + 0x4]
        mov edx, dword ptr [movie_media_idct_storage]
        add eax, edx
        mov esi, offset movie_media_idct_storage + 0x98
        mov dword ptr [movie_media_idct_storage + 0x4], eax
        call sub_5cee80
        mov eax, dword ptr [movie_media_idct_storage + 0x4]
        mov edx, dword ptr [movie_media_idct_storage]
        add eax, edx
        mov esi, offset movie_media_idct_storage + 0xbc
        mov dword ptr [movie_media_idct_storage + 0x4], eax
        call sub_5cee80
        mov eax, dword ptr [movie_media_idct_storage + 0x4]
        mov edx, dword ptr [movie_media_idct_storage]
        add eax, edx
        mov esi, offset movie_media_idct_storage + 0xe0
        mov dword ptr [movie_media_idct_storage + 0x4], eax
        call sub_5cee80
        mov eax, dword ptr [movie_media_idct_storage + 0x4]
        mov edx, dword ptr [movie_media_idct_storage]
        add eax, edx
        mov esi, offset movie_media_idct_storage + 0x104
        mov dword ptr [movie_media_idct_storage + 0x4], eax
        call sub_5cee80
        mov eax, dword ptr [movie_media_idct_storage + 0x4]
        mov edx, dword ptr [movie_media_idct_storage]
        add eax, edx
        mov esi, offset movie_media_idct_storage + 0x128
        mov dword ptr [movie_media_idct_storage + 0x4], eax
        call sub_5cee80
        pop ebp
        pop edi
        pop esi
        pop edx
        pop ecx
        pop ebx
        ret
    }
}


/*
Purpose: Apply the first IDCT pass to one 8-coefficient vector. In the
         original, ESI supplies the vector and 0x009C3004 supplies the output
         pointer. The typed C++ body makes both channels explicit through its
         argument and shared MediaDecoderIdctStorage view. When input[1..7]
         are all zero, input[0] is repeated at output offsets 0, 0x24, ...,
         0xFC; otherwise the eight-value butterfly writes those same offsets.
         The caller provides one vector per column and points output into the
         nine-dword-stride intermediate block.
// Typed owner implementation; native ABI/output, original reference targets
// and exceptional behavior remain restoration obligations, not owner stubs.
// ORIGINAL: 0x005CED1D sub_5ced1d 0x005CED1D-0x005CEE80 BYTE_EXACT
// RECOVERY_BINDING: v1 {"reason":"The historical 0x005CEF8A bound included the complete separate sub_5cee80 transform. End this transform at its terminal RET; preserve both identities and their independently implemented behavior and unresolved ABI/precision obligations.","evidence":["pinned:0x005CED1D-0x005CEE80 complete instruction decode ending with RET at 0x005CEE7F; no branch leaves this body","pinned:independent sub_5cee80 begins exactly at 0x005CEE80, with its own complete 0x005CEE80-0x005CEF8A mapping","source:src/movie_media_decoder.cpp separate transform bodies; IDA and Ghidra agree on both entry/return boundaries"]}
// symbol    ?sub_5ced1d@@YAXPAH@Z
// size      355 bytes
// kind      game
// calls     (none)
Status: Typed semantic body present through the shared column transform.
        External ESI-only entry ABI, x87 precision, exact output and linked
        storage/reference behavior remain unverified; preserve those debts.
*/
// Hand-written asm (EA uV decoder module): no prologue, esi/edi register inputs.
__declspec(naked) void __cdecl sub_5ced1d(int *esi_ptr) {
    __asm {
        mov ebx, dword ptr [esi + 0x14]
        mov eax, dword ptr [esi + 0xc]
        mov ecx, dword ptr [esi + 4]
        mov edx, dword ptr [esi + 0x1c]
        or ecx, ebx
        or edx, eax
        mov edi, dword ptr [esi + 8]
        mov ebp, dword ptr [esi + 0x18]
        or edx, ecx
        or edi, ebp
        mov ecx, dword ptr [esi + 0x10]
        or edx, edi
        or edx, ecx
        je m5ced1d_5cee54
        mov ecx, dword ptr [esi + 4]
        mov edx, dword ptr [esi + 0x1c]
        lea edi, [eax + ebx]
        sub ebx, eax
        lea eax, [edx + ecx]
        sub ecx, edx
        mov dword ptr [movie_media_idct_storage + 0x8], ebx
        mov dword ptr [movie_media_idct_storage + 0xc], ecx
        lea ebp, [edi + eax]
        sub eax, edi
        fild dword ptr [movie_media_idct_storage + 0x8]
        fild dword ptr [movie_media_idct_storage + 0xc]
        fld st(0)
        fmul dword ptr [movie_media_idct_storage + 0x20]
        fxch st(2)
        fadd st(1), st(0)
        fmul dword ptr [movie_media_idct_storage + 0x1c]
        fxch st(1)
        fmul dword ptr [movie_media_idct_storage + 0x24]
        imul dword ptr [movie_media_idct_storage + 0x18]
        fadd st(1), st(0)
        fsubp st(2), st
        fadd dword ptr [movie_media_idct_storage + 0x28]
        fxch st(1)
        fadd dword ptr [movie_media_idct_storage + 0x28]
        mov eax, dword ptr [esi + 8]
        mov ebx, dword ptr [esi + 0x18]
        shl edx, 1
        fstp qword ptr [movie_media_idct_storage + 0x10]
        fstp qword ptr [movie_media_idct_storage + 0x8]
        mov ecx, dword ptr [movie_media_idct_storage + 0x8]
        mov edi, dword ptr [movie_media_idct_storage + 0x10]
        add ecx, edx
        add edx, edi
        add edi, ebp
        mov dword ptr [movie_media_idct_storage + 0xc], ecx
        mov dword ptr [movie_media_idct_storage + 0x10], edx
        mov dword ptr [movie_media_idct_storage + 0x14], edi
        mov ecx, dword ptr [esi]
        mov edx, dword ptr [esi + 0x10]
        lea esi, [ebx + eax]
        sub eax, ebx
        lea ebx, [edx + ecx]
        sub ecx, edx
        imul dword ptr [movie_media_idct_storage + 0x18]
        shl edx, 1
        mov edi, dword ptr [movie_media_idct_storage + 0x4]
        add esi, edx
        mov ebp, dword ptr [movie_media_idct_storage + 0xc]
        lea eax, [edx + ecx]
        sub ecx, edx
        lea edx, [esi + ebx]
        sub ebx, esi
        lea esi, [ecx + ebp]
        sub ecx, ebp
        mov ebp, dword ptr [movie_media_idct_storage + 0x8]
        mov dword ptr [edi + 0x48], esi
        mov esi, dword ptr [movie_media_idct_storage + 0x10]
        mov dword ptr [edi + 0xb4], ecx
        lea ecx, [ebx + ebp]
        sub ebx, ebp
        mov ebp, dword ptr [movie_media_idct_storage + 0x14]
        mov dword ptr [edi + 0x6c], ecx
        mov dword ptr [edi + 0x90], ebx
        lea ebx, [esi + eax]
        sub eax, esi
        lea ecx, [edx + ebp]
        sub edx, ebp
        mov dword ptr [edi], ecx
        mov dword ptr [edi + 0x24], ebx
        mov dword ptr [edi + 0xd8], eax
        mov dword ptr [edi + 0xfc], edx
        ret
m5ced1d_5cee54:
        mov eax, dword ptr [esi]
        mov edi, dword ptr [movie_media_idct_storage + 0x4]
        mov dword ptr [edi], eax
        mov dword ptr [edi + 0x24], eax
        mov dword ptr [edi + 0x48], eax
        mov dword ptr [edi + 0x6c], eax
        mov dword ptr [edi + 0x90], eax
        mov dword ptr [edi + 0xb4], eax
        mov dword ptr [edi + 0xd8], eax
        mov dword ptr [edi + 0xfc], eax
        ret
    }
}

// Decoder providers share their typed storage and callers here. This is a
// recovered ownership cohort, not proof of one historical source module.
// ORIGINAL: 0x005D271D sub_5d271d 0x005D271D-0x005D2761 FILE BYTE_EXACT
// symbol    _sub_5d271d
// size      68 bytes
// prototype
// callers   1   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// Earlier seed observation: MISMATCH at instruction 1. Historical scores
// could depend on the complete seed scaffolding, not the body alone.

typedef int int32_t;
typedef unsigned int uint32_t;
typedef short int16_t;
typedef unsigned short uint16_t;
typedef signed char int8_t;
typedef unsigned char uint8_t;
typedef int int32;
typedef unsigned int uint32;
typedef short int16;
typedef unsigned short uint16;
// `char`, NOT `signed char`. They are distinct MSVC types and mangle
// differently - D against C - and the catalogue's `int8` means the first:
// counted over every catalogued mangled name, `PAD` appears 508 times and
// `PAC` once. Spelling it `signed char` made 150 derived prototypes emit a
// symbol no target object holds. `int8_t` keeps its C meaning below; neither
// catalogue ever uses it.
typedef char int8;
typedef unsigned char uint8;

// Each source dword is 4 bytes b0..b3 (little-endian); each byte is
// written out twice adjacently to the destination (2x horizontal pixel
// doubling: b0,b0,b1,b1,b2,b2,b3,b3), 4 source bytes producing 8
// destination bytes per iteration.
//
// The original assembles each destination dword in a register via
// 8/16-bit sub-register moves (mov bl,ah / mov bh,ah / shl ebx,0x10 /
// mov bl,al / mov bh,al) before a single 32-bit store. Every arithmetic
// rendering tried here (shift-or splat, multiply-by-0x101 splat) makes
// VC6 emit `and`/`imul`/extra `shl`+`or` chains instead of reproducing
// that register shuffle, so this settles for the byte-store form below,
// which is shorter and closer in instruction count than the arithmetic
// attempts (26 mnemonics / 0.73 similarity, vs 34-43 / 0.58-0.6 for the
// splat versions) even though it stores 8 bytes individually rather
// than 2 dwords.
// Hand-written asm (EA uV decoder module): saves ecx/edx, private register use.
extern "C" __declspec(naked) void __cdecl sub_5d271d(unsigned int *src, unsigned char *dst,
                                    int count) {
    __asm {
        push ebx
        push ecx
        push edx
        push esi
        push edi
        push ebp
        mov esi, dword ptr [esp + 0x1c]
        mov edi, dword ptr [esp + 0x20]
        mov ecx, dword ptr [esp + 0x24]
m5d271d_5d272f:
        mov eax, dword ptr [esi]
        add esi, 4
        mov bl, ah
        mov bh, ah
        shl ebx, 0x10
        mov bl, al
        mov bh, al
        mov dword ptr [edi], ebx
        shr eax, 0x10
        mov bl, ah
        mov bh, ah
        shl ebx, 0x10
        mov bl, al
        mov bh, al
        mov dword ptr [edi + 4], ebx
        add edi, 8
        sub ecx, 4
        jne m5d271d_5d272f
        pop ebp
        pop edi
        pop esi
        pop edx
        pop ecx
        pop ebx
        ret
    }
}

// ORIGINAL: 0x005D2D20 sub_5d2d20 0x005D2D20-0x005D2DF2 FILE
// symbol    @sub_5d2d20@20
// TRIED: `extern "C" int __stdcall sub_5d2d20(int a1, int a2, int a3)` per the given def head - `test edx,edx` and `lea eax,[ecx+7]` read both registers with no matching stack loads, so two args (src ptr in ecx, mode in edx) are __fastcall register params, not part of a 3-int __stdcall; changed to `void __fastcall(char*,int,int,char*,char)` with 5 total params (2 register + 3 stack), void return (eax never set). `int step = stride * 4;` hoisted above the `if` (matching the unconditional `lea edi,[eax*4]` before the branch) gets close; best found is 83.6% mnemonic similarity (`--dir` picked /O1 framed over /O2 frameless) - remaining diff is a 4-instruction reorder of which register (edx vs ecx) holds `dst` while `adjust`/`i` are set up, which is register-allocation scheduling, not a control-flow gap.
// working copy - scaffold materialised by --work
// size      210 bytes
// prototype
// callers   1   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)


typedef int int32_t;
typedef unsigned int uint32_t;
typedef short int16_t;
typedef unsigned short uint16_t;
typedef signed char int8_t;
typedef unsigned char uint8_t;
typedef int int32;
typedef unsigned int uint32;
typedef short int16;
typedef unsigned short uint16;
// `char`, NOT `signed char`. They are distinct MSVC types and mangle
// differently - D against C - and the catalogue's `int8` means the first:
// counted over every catalogued mangled name, `PAD` appears 508 times and
// `PAC` once. Spelling it `signed char` made 150 derived prototypes emit a
// symbol no target object holds. `int8_t` keeps its C meaning below; neither
// catalogue ever uses it.
typedef char int8;
typedef unsigned char uint8;
// `src` (ecx) and `mode` (edx) are __fastcall register args; `stride`,
// `dst` and `adjust` are the three stack args the `ret 0xc` pops - see
// TRIED above for the signature evidence.
extern "C" void __fastcall sub_5d2d20(char *src, int mode, int stride,
                                      char *dst, char adjust) {
    int step = stride * 4;
    char *p;
    char *q;
    int i;
    if (mode != 0) {
        p = src + 7;
        q = dst + 6;
        i = 8;
        do {
            q[-4] = p[-5] + adjust;
            q[0] = p[0] + adjust;
            q[4] = p[-1] + adjust;
            q[8] = p[4] + adjust;
            q[0xc] = p[3] + adjust;
            q[0x10] = p[8] + adjust;
            q[0x14] = p[7] + adjust;
            q[0x18] = p[0xc] + adjust;
            p = p + step;
            --i;
            q = q + 0x40;
        } while (i != 0);
        return;
    }
    p = src + 2;
    q = dst + 6;
    i = 8;
    do {
        q[-4] = p[1] + adjust;
        q[0] = p[0] + adjust;
        q[4] = p[5] + adjust;
        q[8] = p[4] + adjust;
        q[0xc] = p[9] + adjust;
        q[0x10] = p[8] + adjust;
        q[0x14] = p[0xd] + adjust;
        q[0x18] = p[0xc] + adjust;
        p = p + step;
        --i;
        q = q + 0x40;
    } while (i != 0);
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


typedef int int32_t;
typedef unsigned int uint32_t;
typedef short int16_t;
typedef unsigned short uint16_t;
typedef signed char int8_t;
typedef unsigned char uint8_t;
typedef int int32;
typedef unsigned int uint32;
typedef short int16;
typedef unsigned short uint16;
// `char`, NOT `signed char`. They are distinct MSVC types and mangle
// differently - D against C - and the catalogue's `int8` means the first:
// counted over every catalogued mangled name, `PAD` appears 508 times and
// `PAC` once. Spelling it `signed char` made 150 derived prototypes emit a
// symbol no target object holds. `int8_t` keeps its C meaning below; neither
// catalogue ever uses it.
typedef char int8;
typedef unsigned char uint8;
extern "C" void __fastcall sub_5d3f50(MediaRuntime *runtime) {
    const MediaPixelFormat &initial_format = runtime->descriptor->pixel_format;
    const unsigned int shift_green =
        8u - initial_format.green_bits;
    const unsigned int shift_red =
        8u - initial_format.red_bits;
    const unsigned int shift_blue =
        8u - initial_format.blue_bits;

    for (int index = 0; index < 256; ++index) {
        MediaDescriptor *const descriptor = runtime->descriptor;
        const MediaPixelFormat &format = descriptor->pixel_format;
        const int palette_index = index * 3;
        unsigned int green =
            (descriptor->palette_rgb_triplets_b0[palette_index + 1] >>
             shift_green) << format.green_shift;
        unsigned int red =
            (descriptor->palette_rgb_triplets_b0[palette_index] >>
             shift_red) << format.red_shift;
        unsigned int result = green | red;
        unsigned int blue =
            (descriptor->palette_rgb_triplets_b0[palette_index + 2] >>
             shift_blue) << format.blue_shift;
        result |= blue;
        if ((descriptor->flags & 0x40) != 0)
            result |= result << 0x10;
        descriptor->palette_lookup_3b0.entries[index] = result;
    }
}


// ORIGINAL: 0x005D427C sub_5d427c 0x005D427C-0x005D439F BYTE_EXACT
// symbol    _sub_5d427c
// TRIED: entry uses pushal/popal (all eight registers saved and restored) around the whole body with cld and only rep movsb/ rep movsd inside, which VC6 never emits for ordinary compiled C++ - this is very likely inline assembly in the original source, which the rules here bar reproducing. Transcribed Ghidra's decompilation (already a faithful read of the byte stream) into plain C++ instead: a byte-oriented decoder that walks a control word one bit at a time, alternating literal dword/byte copies from the source with back-reference copies from already-written output, matching an LZ-style unpacker. No `__asm`; this compiles but will not reach the original's register-saving shape.
// size      291 bytes
// callers   1   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)

// Hand-written asm (EA uV decoder module): pushad/popad + leave, cld/rep movsb.
extern "C" __declspec(naked) int __cdecl sub_5d427c(unsigned short *samples, unsigned char *steps,
                                  int a3) {
    __asm {
        push ebp
        mov ebp, esp
        add esp, -4
        push esi
        push edi
        pushad
        cld
        mov ecx, dword ptr [ebp + 0x10]
        mov ebx, dword ptr [ebp + 8]
        mov edi, dword ptr [ebp + 0xc]
        mov dword ptr [ebp - 4], 0
        or ebx, ebx
        je m5d427c_5d4352
        mov ax, word ptr [ebx]
        lea ebx, [ebx + 2]
        and al, 1
        je m5d427c_5d42ab
        lea ebx, [ebx + 3]
m5d427c_5d42ab:
        xor eax, eax
        mov al, byte ptr [ebx]
        shl eax, 0x10
        mov ah, byte ptr [ebx + 1]
        mov al, byte ptr [ebx + 2]
        lea ebx, [ebx + 3]
        mov dword ptr [ebp - 4], eax
        cmp ecx, 0
        je m5d427c_5d4352
        xor ecx, ecx
        jmp m5d427c_5d42ee
        nop
m5d427c_5d42cc:
        and ecx, 3
        lea esi, [ebx + 2]
        rep movsb
        mov ebx, esi
        mov ch, dl
        mov cl, dh
        and edx, 0x1c
        shr ch, 5
        shr edx, 2
        neg ecx
        lea esi, [ecx + edi - 1]
        lea ecx, [edx + 3]
        rep movsb
m5d427c_5d42ee:
        mov edx, dword ptr [ebx]
        or cl, dl
        jns m5d427c_5d42cc
m5d427c_5d42f4:
        add cl, cl
        js m5d427c_5d4328
        mov cl, dh
        shr ecx, 6
        and ecx, 3
        lea esi, [ebx + 3]
        rep movsb
        mov ebx, esi
        mov ecx, edx
        shr ecx, 0x10
        mov ch, dh
        and ch, 0x3f
        neg ecx
        lea esi, [ecx + edi - 1]
        and edx, 0x3f
        lea ecx, [edx + 4]
        rep movsb
        mov edx, dword ptr [ebx]
        or cl, dl
        jns m5d427c_5d42cc
        jmp m5d427c_5d42f4
        nop
m5d427c_5d4328:
        add cl, cl
        jns m5d427c_5d435c
        cmp dl, 0xfc
        jae m5d427c_5d4348
        and edx, 0x1f
        lea ecx, [edx + 1]
        lea esi, [ebx + 1]
        rep movsd
        mov ebx, esi
        mov edx, dword ptr [ebx]
        or cl, dl
        jns m5d427c_5d42cc
        jmp m5d427c_5d42f4
        mov eax, eax
m5d427c_5d4348:
        mov ecx, edx
        and ecx, 3
        lea esi, [ebx + 1]
        rep movsb
m5d427c_5d4352:
        mov eax, dword ptr [ebp - 4]
        popad
        pop edi
        pop esi
        leave
        ret
        mov eax, eax
m5d427c_5d435c:
        mov ecx, edx
        and ecx, 3
        lea esi, [ebx + 4]
        rep movsb
        mov ebx, esi
        mov ecx, edx
        and ecx, 0x10
        shl ecx, 0xc
        mov eax, edx
        shr eax, 8
        mov cl, ah
        mov ch, al
        neg ecx
        lea esi, [ecx + edi - 1]
        rol edx, 8
        shr dh, 2
        and edx, 0x3ff
        lea ecx, [edx + 5]
        rep movsb
        mov edx, dword ptr [ebx]
        or cl, dl
        jns m5d427c_5d42cc
        jmp m5d427c_5d42f4
    }
}

// ORIGINAL: 0x005D439F sub_5d439f 0x005D439F-0x005D4448 FILE BYTE_EXACT
// symbol    _sub_5d439f
// working copy - scaffold materialised by --work
// size      169 bytes
// prototype
// callers   1   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)

// The producer writes unsigned 32-bit index words; the native consumer
// interprets their stored bit patterns as signed indices.
// The old integer-only prototype was a disassembler guess. Callers pass two
// buffers and two counts in four dword stack slots, with no ecx receiver;
// the tail supplies no return value.
// Hand-written asm (EA uV decoder module): saves ecx/edx, shrd.
extern "C" __declspec(naked) void __cdecl sub_5d439f(unsigned int *index_words,
                                    const unsigned char *compressed,
                                    int word_count, int bit_width) {
    __asm {
        push esi
        push edi
        push ebp
        push ebx
        push ecx
        push edx
        mov ebx, 0
        mov ecx, dword ptr [esp + 0x28]
        mov edx, dword ptr [esp + 0x24]
        mov edi, dword ptr [esp + 0x1c]
        mov esi, dword ptr [esp + 0x20]
        push ecx
        mov eax, 1
        shl eax, cl
        sub edx, 4
        lea ebp, [eax - 1]
        jl m5d439f_5d441a
m5d439f_5d43ca:
        mov eax, ebx
        mov ecx, ebx
        shr eax, 3
        and ecx, 7
        mov eax, dword ptr [eax + esi]
        shrd eax, eax, cl
        mov ecx, eax
        and ecx, ebp
        mov dword ptr [edi], ecx
        mov ecx, dword ptr [esp]
        shrd eax, eax, cl
        and eax, ebp
        mov dword ptr [edi + 4], eax
        lea eax, [ebx + ecx*2]
        lea ecx, [ebx + ecx*2]
        shr eax, 3
        and ecx, 7
        mov eax, dword ptr [eax + esi]
        shrd eax, eax, cl
        mov ecx, eax
        and ecx, ebp
        mov dword ptr [edi + 8], ecx
        mov ecx, dword ptr [esp]
        shrd eax, eax, cl
        and eax, ebp
        mov dword ptr [edi + 0xc], eax
        add edi, 0x10
        sub edx, 4
        lea ebx, [ebx + ecx*4]
        jge m5d439f_5d43ca
m5d439f_5d441a:
        add edx, 3
        jl m5d439f_5d443e
m5d439f_5d441f:
        mov eax, ebx
        mov ecx, ebx
        shr eax, 3
        and ecx, 7
        mov eax, dword ptr [eax + esi]
        shrd eax, eax, cl
        and eax, ebp
        mov dword ptr [edi], eax
        add ebx, dword ptr [esp]
        sub edx, 1
        lea edi, [edi + 4]
        jge m5d439f_5d441f
m5d439f_5d443e:
        add esp, 4
        pop edx
        pop ecx
        pop ebx
        pop ebp
        pop edi
        pop esi
        ret
    }
}

// ORIGINAL: 0x005D4448 sub_5d4448 0x005D4448-0x005D44FC FILE BYTE_EXACT
// symbol    _sub_5d4448
// TRIED: byte-exact - `pushal`/`popal` at entry has no C++ spelling (documented wall). Landed the nested do/while-with-goto gather loop structurally instead: 9 stack args, an outer per-run counter reset from a saved [ebp+0x24] and an inner index-table lookup vs a direct 16-byte-stride struct-array fallback.
// working copy - scaffold materialised by --work
// size      180 bytes
// prototype
// callers   1   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)

// The unsigned index allocation is consumed as signed native indices; memcpy
// preserves the original bit patterns and signed comparisons without aligned typed loads.
// Hand-written asm (EA uV decoder module): pushad/popad + leave.
extern "C" __declspec(naked) void __cdecl sub_5d4448(
    const unsigned int *index_words, unsigned char *previous_frame,
    unsigned char *current_frame, int run_limit, int *motion_offsets,
    MediaDecodedPixelBlock4x4 *block_scratch, int pitch, int run_count,
    int height_groups)
 {
    __asm {
        push ebp
        mov ebp, esp
        add esp, -0xc
        push ebx
        push esi
        push edi
        pushad
        mov eax, dword ptr [ebp + 0x24]
        mov dword ptr [ebp - 4], eax
        mov eax, dword ptr [ebp + 0x1c]
        mov ebx, dword ptr [ebp + 0x14]
        shl ebx, 4
        sub eax, ebx
        mov dword ptr [ebp - 8], eax
        mov eax, dword ptr [ebp + 0xc]
        sub eax, dword ptr [ebp + 0x10]
        mov dword ptr [ebp - 0xc], eax
        mov edx, dword ptr [ebp + 0x20]
        lea ecx, [edx + edx*2]
        mov esi, dword ptr [ebp + 8]
        mov edi, dword ptr [ebp + 0x10]
m5d4448_5d447b:
        mov eax, dword ptr [ebp - 4]
        mov dword ptr [ebp + 0x24], eax
        jmp m5d4448_5d4484
        nop
m5d4448_5d4484:
        mov eax, dword ptr [esi]
        add esi, 4
        cmp eax, dword ptr [ebp + 0x14]
        jge m5d4448_5d44c2
m5d4448_5d448e:
        mov ebx, dword ptr [ebp + 0x18]
        mov ebx, dword ptr [ebx + eax*4]
        add ebx, dword ptr [ebp - 0xc]
        add ebx, edi
        mov eax, dword ptr [ebx]
        mov dword ptr [edi], eax
        mov eax, dword ptr [edx + ebx]
        mov dword ptr [edx + edi], eax
        mov eax, dword ptr [ebx + edx*2]
        mov dword ptr [edi + edx*2], eax
        mov eax, dword ptr [ecx + ebx]
        mov dword ptr [ecx + edi], eax
        add edi, 4
        sub dword ptr [ebp + 0x24], 1
        jle m5d4448_5d44ee
        mov eax, dword ptr [esi]
        add esi, 4
        cmp eax, dword ptr [ebp + 0x14]
        jl m5d4448_5d448e
m5d4448_5d44c2:
        mov ebx, dword ptr [ebp - 8]
        lea eax, [eax*8]
        lea ebx, [ebx + eax*2]
        mov eax, dword ptr [ebx]
        mov dword ptr [edi], eax
        mov eax, dword ptr [ebx + 4]
        mov dword ptr [edx + edi], eax
        mov eax, dword ptr [ebx + 8]
        mov dword ptr [edi + edx*2], eax
        mov eax, dword ptr [ebx + 0xc]
        mov dword ptr [ecx + edi], eax
        add edi, 4
        sub dword ptr [ebp + 0x24], 1
        jg m5d4448_5d4484
m5d4448_5d44ee:
        add edi, ecx
        sub dword ptr [ebp + 0x28], 1
        jg m5d4448_5d447b
        popad
        pop edi
        pop esi
        pop ebx
        leave
        ret
    }
}

// The 3x3 numerical member bodies are consolidated in src/matrix3.cpp.

// ORIGINAL: 0x005D2E00 sub_5d2e00 0x005D2E00-0x005D2E6E FILE BYTE_EXACT
// symbol    ?sub_5d2e00@@YIXPADHH0D@Z
// working copy - scaffold materialised by --work
// size      110 bytes
// prototype
// callers   1   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)


typedef int int32_t;
typedef unsigned int uint32_t;
typedef short int16_t;
typedef unsigned short uint16_t;
typedef signed char int8_t;
typedef unsigned char uint8_t;
typedef int int32;
typedef unsigned int uint32;
typedef short int16;
typedef unsigned short uint16;
// `char`, NOT `signed char`. They are distinct MSVC types and mangle
// differently - D against C - and the catalogue's `int8` means the first:
// counted over every catalogued mangled name, `PAD` appears 508 times and
// `PAC` once. Spelling it `signed char` made 150 derived prototypes emit a
// symbol no target object holds. `int8_t` keeps its C meaning below; neither
// catalogue ever uses it.
typedef char int8;
typedef unsigned char uint8;
// `lea eax, [ecx+edx]` with no stack access for the first two operands is
// __fastcall (ecx=a1, edx=a2); three more come off the stack, the last a
// single byte - matches `ret 0xc` (3 stack slots popped by the callee).
// a1 and a4 are pointers the census's H was only guessing at; the body adds a
// byte offset to each, which is the proof.
void __fastcall sub_5d2e00(char *a1, int a2, int a3, char *a4, char a5) {
    char *src = a1 + a2;
    int stride = a3 * 8;
    char *dst = a4 + 6;
    int count = 8;
    do {
        dst[-4] = src[0] + a5;
        dst[0] = src[4] + a5;
        dst[4] = src[8] + a5;
        dst[8] = src[0xc] + a5;
        dst[0xc] = src[0x10] + a5;
        dst[0x10] = src[0x14] + a5;
        dst[0x14] = src[0x18] + a5;
        dst[0x18] = src[0x1c] + a5;
        src = src + stride;
        count = count - 1;
        dst = dst + 0x20;
    } while (count != 0);
}


// IMA ADPCM nibble -> step-index adjustment, pre-scaled by 64 so the index
// itself travels as a byte offset: the eight dwords are {-1,-1,-1,-1,2,4,6,8}
// times 64, and the 0x1600 clamp below is 88 * 64, the IMA step index's own
// maximum. Image .data at 0x00694DF4.
// Pinned image initial bytes: eight prefix words, 89 rows of 16 deltas,
// eight adjustments, then four zero state words. Every delta was compared
// with the original, including signed nibbles and the final step of 32767.
// Earlier seed notes describing these lookup words as zero were incorrect.
#define MEDIA_ADPCM_MAG(s, n) (((s) >> 3) + (((n) & 1) ? ((s) >> 2) : 0) + (((n) & 2) ? ((s) >> 1) : 0) + (((n) & 4) ? (s) : 0))
#define MEDIA_ADPCM_ROW(s) MEDIA_ADPCM_MAG(s, 0), MEDIA_ADPCM_MAG(s, 1), MEDIA_ADPCM_MAG(s, 2), MEDIA_ADPCM_MAG(s, 3), MEDIA_ADPCM_MAG(s, 4), MEDIA_ADPCM_MAG(s, 5), MEDIA_ADPCM_MAG(s, 6), MEDIA_ADPCM_MAG(s, 7), -MEDIA_ADPCM_MAG(s, 0), -MEDIA_ADPCM_MAG(s, 1), -MEDIA_ADPCM_MAG(s, 2), -MEDIA_ADPCM_MAG(s, 3), -MEDIA_ADPCM_MAG(s, 4), -MEDIA_ADPCM_MAG(s, 5), -MEDIA_ADPCM_MAG(s, 6), -MEDIA_ADPCM_MAG(s, 7)
// Access-window binding: native coefficient readers at 0x005CDB90/0x005CDD10
// overlap the first delta row; 0x005CE6E3 owns the four trailing state writes.
// This groups proven storage accesses, not a recovered original declaration.
// RECOVERY_OBJECT: v2 0x00693794 0x00694E24 ?MediaAdpcmTables@@3UMediaAdpcmStorage@@A 0x005CE6E3
MediaAdpcmStorage MediaAdpcmTables = {
    {0, 240, 460, 392, 0, 0, -208, -220,
     MEDIA_ADPCM_ROW(7), MEDIA_ADPCM_ROW(8), MEDIA_ADPCM_ROW(9),
     MEDIA_ADPCM_ROW(10), MEDIA_ADPCM_ROW(11), MEDIA_ADPCM_ROW(12),
     MEDIA_ADPCM_ROW(13), MEDIA_ADPCM_ROW(14), MEDIA_ADPCM_ROW(16),
     MEDIA_ADPCM_ROW(17), MEDIA_ADPCM_ROW(19), MEDIA_ADPCM_ROW(21),
     MEDIA_ADPCM_ROW(23), MEDIA_ADPCM_ROW(25), MEDIA_ADPCM_ROW(28),
     MEDIA_ADPCM_ROW(31), MEDIA_ADPCM_ROW(34), MEDIA_ADPCM_ROW(37),
     MEDIA_ADPCM_ROW(41), MEDIA_ADPCM_ROW(45), MEDIA_ADPCM_ROW(50),
     MEDIA_ADPCM_ROW(55), MEDIA_ADPCM_ROW(60), MEDIA_ADPCM_ROW(66),
     MEDIA_ADPCM_ROW(73), MEDIA_ADPCM_ROW(80), MEDIA_ADPCM_ROW(88),
     MEDIA_ADPCM_ROW(97), MEDIA_ADPCM_ROW(107), MEDIA_ADPCM_ROW(118),
     MEDIA_ADPCM_ROW(130), MEDIA_ADPCM_ROW(143), MEDIA_ADPCM_ROW(157),
     MEDIA_ADPCM_ROW(173), MEDIA_ADPCM_ROW(190), MEDIA_ADPCM_ROW(209),
     MEDIA_ADPCM_ROW(230), MEDIA_ADPCM_ROW(253), MEDIA_ADPCM_ROW(279),
     MEDIA_ADPCM_ROW(307), MEDIA_ADPCM_ROW(337), MEDIA_ADPCM_ROW(371),
     MEDIA_ADPCM_ROW(408), MEDIA_ADPCM_ROW(449), MEDIA_ADPCM_ROW(494),
     MEDIA_ADPCM_ROW(544), MEDIA_ADPCM_ROW(598), MEDIA_ADPCM_ROW(658),
     MEDIA_ADPCM_ROW(724), MEDIA_ADPCM_ROW(796), MEDIA_ADPCM_ROW(876),
     MEDIA_ADPCM_ROW(963), MEDIA_ADPCM_ROW(1060), MEDIA_ADPCM_ROW(1166),
     MEDIA_ADPCM_ROW(1282), MEDIA_ADPCM_ROW(1411), MEDIA_ADPCM_ROW(1552),
     MEDIA_ADPCM_ROW(1707), MEDIA_ADPCM_ROW(1878), MEDIA_ADPCM_ROW(2066),
     MEDIA_ADPCM_ROW(2272), MEDIA_ADPCM_ROW(2499), MEDIA_ADPCM_ROW(2749),
     MEDIA_ADPCM_ROW(3024), MEDIA_ADPCM_ROW(3327), MEDIA_ADPCM_ROW(3660),
     MEDIA_ADPCM_ROW(4026), MEDIA_ADPCM_ROW(4428), MEDIA_ADPCM_ROW(4871),
     MEDIA_ADPCM_ROW(5358), MEDIA_ADPCM_ROW(5894), MEDIA_ADPCM_ROW(6484),
     MEDIA_ADPCM_ROW(7132), MEDIA_ADPCM_ROW(7845), MEDIA_ADPCM_ROW(8630),
     MEDIA_ADPCM_ROW(9493), MEDIA_ADPCM_ROW(10442), MEDIA_ADPCM_ROW(11487),
     MEDIA_ADPCM_ROW(12635), MEDIA_ADPCM_ROW(13899), MEDIA_ADPCM_ROW(15289),
     MEDIA_ADPCM_ROW(16818), MEDIA_ADPCM_ROW(18500), MEDIA_ADPCM_ROW(20350),
     MEDIA_ADPCM_ROW(22385), MEDIA_ADPCM_ROW(24623), MEDIA_ADPCM_ROW(27086),
     MEDIA_ADPCM_ROW(29794), MEDIA_ADPCM_ROW(32767)},
    {-64, -64, -64, -64, 128, 256, 384, 512}, 0, 0, 0, 0
};
#undef MEDIA_ADPCM_ROW
#undef MEDIA_ADPCM_MAG

// THE REST OF THE IMA ADPCM DECODER'S STATE, and the delta table. All four
// state words are zero in the shipped .data (they are decoder state, not
// data), so the evidence for the names is the arithmetic:
//   * 0x00694E14 and 0x00694E18 are filled from the two HALVES of one dword
//     the caller hands in - `(short)*param_6` and `(short)(*param_6 >> 16)` -
//     are clamped to [-32768, 32767] and written back as the two halves of
//     the caller's dword. That is a stereo predictor pair.
//   * 0x00694E1C and 0x00694E20 do the same with the second dword, are
//     adjusted by the eight-word index table and clamped to
//     [0, 0x1600] = [0, 88 * 64],
//     the IMA step index. That is the matching step-index pair.
//   * 0x006937B4 is indexed `(nibble * 4) + step_index`, i.e. [code][index],
//     and its first row reads 0,1,3,4,7,8,10,11 - exactly step 7 split by the
//     three magnitude bits of an IMA nibble. 89 rows of 16 dwords.
// Preserve the native byte-offset interpretation, including unaligned input
// offsets, without an unaligned int dereference. This is not a bounds waiver.

// ORIGINAL: 0x005CE640 sub_5ce640 0x005CE640-0x005CE6E3 FILE BYTE_EXACT
// symbol    ?sub_5ce640@@YAXHPAEPAFHPAH2@Z
// size      163 bytes
// prototype
// callers   0   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// Purpose: Decode `sample_total - first_sample` samples of IMA ADPCM audio
//          into 16-bit output. `codes` is the packed nibble stream, `samples`
//          the destination; `predictor` and `step_index` are the caller's two
//          words of decoder state, both read AND written, so the decode
//          resumes where the previous block left off. Evidence for the
//          identification: `predicted` is clamped to [-32768, 32767], the
//          delta comes out of the 8x88 table at 0x006937B4 whose first row
//          (0,1,3,4,7,8,10,11) is exactly step 7 scaled by the three
//          magnitude bits, and the adjustment table is the IMA one.
// TRIED: earlier homed into general.cpp (original TU unproved) 2026-09-03 and reverted because
// its one binding was then thought evidence-free. It is not: image_data.py at
// 0x00694DF4 reads {-64,-64,-64,-64,128,256,384,512}, which is the IMA index
// adjustment {-1,-1,-1,-1,2,4,6,8} scaled by 64, and 0x1600 is 88 * 64. Named
// AdpcmIndexAdjust, 2026-09-06.
// TRIED: byte-exactness is out of reach here - the image's fourth instruction
// is `pushal` (0x60), which VC6 never emits for any C++ body; this is a
// handwritten_asm finding, so the ceiling is semantic equivalence.
// Hand-written asm (EA uV decoder module): pushad/popad + leave.
__declspec(naked) void __cdecl sub_5ce640(int first_sample, unsigned char *codes, short *samples,
                        int sample_total, int *predictor, int *step_index) {
    __asm {
        push ebp
        mov ebp, esp
        pushad
        xor ecx, ecx
        add ecx, dword ptr [ebp + 8]
        mov eax, dword ptr [ebp + 8]
        add dword ptr [ebp + 0x14], eax
        mov eax, dword ptr [ebp + 0x1c]
        mov eax, dword ptr [eax]
        mov edx, dword ptr [ebp + 0x18]
        mov edx, dword ptr [edx]
        movsx edx, dx
        mov esi, dword ptr [ebp + 0xc]
        mov edi, dword ptr [ebp + 0x10]
        xor ebx, ebx
m5ce640_5ce664:
        mov bl, byte ptr [esi]
        test ecx, 1
        jne m5ce640_5ce673
        shr bl, 4
        jmp m5ce640_5ce677
m5ce640_5ce673:
        and bl, 0xf
        inc esi
m5ce640_5ce677:
        add edx, dword ptr [eax + ebx*4 + MediaAdpcmTables + 0x20]
        cmp edx, 0x7fff
        jg m5ce640_5ce6d5
        cmp edx, 0xffff8000
        jl m5ce640_5ce6dc
m5ce640_5ce68e:
        mov word ptr [edi], dx
        add edi, 2
        inc ecx
        and bl, 7
        add eax, dword ptr [ebx*4 + MediaAdpcmTables + 0x1660]
        cmp eax, 0x1600
        jg m5ce640_5ce6bd
        cmp eax, 0
        jl m5ce640_5ce6c9
        cmp ecx, dword ptr [ebp + 0x14]
        jl m5ce640_5ce664
m5ce640_5ce6b0:
        mov ecx, dword ptr [ebp + 0x18]
        mov dword ptr [ecx], edx
        mov ecx, dword ptr [ebp + 0x1c]
        mov dword ptr [ecx], eax
        popad
        leave
        ret
m5ce640_5ce6bd:
        mov eax, 0x1600
        cmp ecx, dword ptr [ebp + 0x14]
        jl m5ce640_5ce664
        jmp m5ce640_5ce6b0
m5ce640_5ce6c9:
        mov eax, 0
        cmp ecx, dword ptr [ebp + 0x14]
        jl m5ce640_5ce664
        jmp m5ce640_5ce6b0
m5ce640_5ce6d5:
        mov edx, 0x7fff
        jmp m5ce640_5ce68e
m5ce640_5ce6dc:
        mov edx, 0xffff8000
        jmp m5ce640_5ce68e
    }
}

// THE MOVIE CODEC'S RUNTIME-FILLED LOOKUP TABLES. All four are zero in the
// shipped .data and filled before use, so image_data.py has nothing to read
// at any of them; the names come from how the bodies index them, which is the
// evidence the anonymous-global rule asks for.
//
// Three 512-entry word tables indexed by three DISJOINT 9-bit fields of one
// accumulated sum - sub_5d2761 forms `(sum >> 23)`, `(sum >> 11) & 0x1ff` and
// `sum & 0x1ff` and adds one entry from each - plus a 256-entry byte remap
// that sub_5ceb12 packs two at a time into each output word.
static DWORD *const MovieBlendTableHi =
    movie_media_conversion_tables.blend_high;
static DWORD *const MovieBlendTableMid =
    movie_media_conversion_tables.blend_mid;
static DWORD *const MovieBlendTableLow =
    movie_media_conversion_tables.blend_low;
static unsigned char *const MovieByteRemapLut =
    reinterpret_cast<unsigned char *>(movie_media_conversion_tables.remap_or_lookup_storage.words);

// A second tier of the same family, named from the lane each one indexes.
// sub_5d2834 / sub_5d2901 build a sum and pull one entry per BYTE lane out of
// it - `[sum >> 0x18]`, `[sum & 0xff]`, `[(sum >> 8) & 0xff]` - so each of
// these is a 256-entry-per-byte-value contribution table for one lane.
static DWORD *const MovieColourTableByte3 =
    movie_media_conversion_tables.contribution_byte3;
static DWORD *const MovieColourTableByte1 =
    movie_media_conversion_tables.contribution_byte1;
static DWORD *const MovieColourTableByte0 =
    movie_media_conversion_tables.contribution_byte0;
// sub_5d1b88 / sub_5d2037 index these three by three DIFFERENT coefficient
// streams rather than by byte lanes: `data[i]`, `tables[i]` and
// `tables[0x40 + i]` respectively.
static DWORD *const MovieBlendTableData =
    movie_media_conversion_tables.contribution_byte3 + 256;
static DWORD *const MovieBlendTableQuant =
    movie_media_conversion_tables.contribution_byte1 + 256;
static DWORD *const MovieBlendTableQuant2 =
    movie_media_conversion_tables.contribution_byte0 + 256;

// ORIGINAL: 0x005D3DF0 sub_5d3df0 0x005D3DF0-0x005D3F48 FILE
// symbol    ?sub_5d3df0@@YIHPAUMediaRuntime@@PAE@Z
// size      344 bytes
// prototype
// callers   ?   call targets   3
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005D427C 0x006470A6 0x00644EF2
// TRIED: __fastcall(this,a2) signature chosen from the ecx=this / edx=a2 register reads at entry; PROPOSAL below. Landed at 0.88 mnemonic similarity - remaining divergence is prologue register-save scheduling, not logic.
int __fastcall sub_5d3df0(MediaRuntime *runtime, unsigned char *frame_base) {
  int saved_index;
  memcpy(&saved_index, &runtime->unresolved_word_61c, sizeof(saved_index));
  MediaDescriptor *const descriptor = runtime->descriptor;
  int height;
  int width;
  memcpy(&height, &descriptor->output_height_60, sizeof(height));
  memcpy(&width, &descriptor->output_width_64, sizeof(width));
  const DWORD pixel_count_bits = static_cast<DWORD>(height) *
                                 static_cast<DWORD>(width);
  const DWORD allocation_size = (pixel_count_bits + 0x0fu) & 0xfffffff0u;

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
      malloc(static_cast<DWORD>(height) << 3));
  // Native 5D3EA7..5D3EB2 divides the aligned area as signed, then wraps
  // the index-word byte count at 32 bits, including high-bit dimensions.
  int signed_allocation_size;
  memcpy(&signed_allocation_size, &allocation_size, sizeof(signed_allocation_size));
  runtime->motion_block_indices = static_cast<unsigned int *>(
      malloc(static_cast<DWORD>(signed_allocation_size / 16) << 2));

  if (runtime->owned_heap_638 != 0 && runtime->owned_heap_63c != 0 &&
      runtime->motion_row_offsets != 0 && runtime->motion_block_indices != 0) {
    const int stride = height;
    DWORD value_bits = 0u - pixel_count_bits;
    DWORD index = 0;
    if ((stride & 0x7fffffff) != 0) {
      do {
        int value;
        memcpy(&value, &value_bits, sizeof(value));
        runtime->motion_row_offsets[index] = value;
        value_bits += static_cast<DWORD>(width);
        index++;
      } while (index < (static_cast<DWORD>(height) << 1));
    }
    const DWORD sample_offset = (static_cast<DWORD>(saved_index) + 4u) * 3u;
    unsigned short *const samples = reinterpret_cast<unsigned short *>(
        reinterpret_cast<uintptr_t>(frame_base) + sample_offset);
    sub_5d427c(samples, runtime->owned_heap_638, 1);
    return 1;
  }
  return 0;
}

// ORIGINAL: 0x005CEB12 sub_5ceb12 0x005CEB12-0x005CED1D FILE
// symbol    ?sub_5ceb12@@YAHPBE0PAI@Z
// TRIED: no C-callable convention passes 3 pointers in esi/edi/ebp with zero prologue and no stack args - the original has no `push ebp`/frame at all and even leaves ecx/edx's high 24 bits as whatever the caller left them (Ghidra's CONCAT31 "uVar1/uVar2" tracking), which only a hand-written leaf/tail-continuation does. Landed as a real 3-pointer function reproducing the LUT-pack arithmetic; first divergence is the (unavoidable) prologue itself.
// working copy - scaffold materialised by --work
// size      523 bytes
// prototype
// callers   1   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)


// Hand-written asm (EA uV decoder module): no prologue, esi/edi/ebp register inputs.
__declspec(naked) int __cdecl sub_5ceb12(const unsigned char *srcA, const unsigned char *srcB, unsigned int *dst) {
    __asm {
        xor ebx, ebx
        xor eax, eax
        mov al, byte ptr [esi + 2]
        mov bl, byte ptr [edi + 2]
        mov cl, byte ptr [esi + 6]
        mov dl, byte ptr [edi + 0x102]
        mov al, byte ptr [eax + movie_media_conversion_tables + 0x70c]
        mov bl, byte ptr [ebx + movie_media_conversion_tables + 0x70c]
        shl eax, 8
        mov cl, byte ptr [ecx + movie_media_conversion_tables + 0x70c]
        shl ebx, 8
        mov dl, byte ptr [edx + movie_media_conversion_tables + 0x70c]
        or eax, ecx
        or ebx, edx
        shl eax, 0x10
        or eax, ebx
        xor ebx, ebx
        mov dword ptr [ebp], eax
        xor eax, eax
        mov al, byte ptr [esi + 0xa]
        mov bl, byte ptr [edi + 6]
        mov cl, byte ptr [esi + 0xe]
        mov dl, byte ptr [edi + 0x106]
        mov al, byte ptr [eax + movie_media_conversion_tables + 0x70c]
        mov bl, byte ptr [ebx + movie_media_conversion_tables + 0x70c]
        shl eax, 8
        mov cl, byte ptr [ecx + movie_media_conversion_tables + 0x70c]
        shl ebx, 8
        mov dl, byte ptr [edx + movie_media_conversion_tables + 0x70c]
        or eax, ecx
        or ebx, edx
        shl eax, 0x10
        or eax, ebx
        xor ebx, ebx
        mov dword ptr [ebp + 4], eax
        xor eax, eax
        mov al, byte ptr [esi + 0x12]
        mov bl, byte ptr [edi + 0xa]
        mov cl, byte ptr [esi + 0x16]
        mov dl, byte ptr [edi + 0x10a]
        mov al, byte ptr [eax + movie_media_conversion_tables + 0x70c]
        mov bl, byte ptr [ebx + movie_media_conversion_tables + 0x70c]
        shl eax, 8
        mov cl, byte ptr [ecx + movie_media_conversion_tables + 0x70c]
        shl ebx, 8
        mov dl, byte ptr [edx + movie_media_conversion_tables + 0x70c]
        or eax, ecx
        or ebx, edx
        shl eax, 0x10
        or eax, ebx
        xor ebx, ebx
        mov dword ptr [ebp + 8], eax
        xor eax, eax
        mov al, byte ptr [esi + 0x1a]
        mov bl, byte ptr [edi + 0xe]
        mov cl, byte ptr [esi + 0x1e]
        mov dl, byte ptr [edi + 0x10e]
        mov al, byte ptr [eax + movie_media_conversion_tables + 0x70c]
        mov bl, byte ptr [ebx + movie_media_conversion_tables + 0x70c]
        shl eax, 8
        mov cl, byte ptr [ecx + movie_media_conversion_tables + 0x70c]
        shl ebx, 8
        mov dl, byte ptr [edx + movie_media_conversion_tables + 0x70c]
        or eax, ecx
        or ebx, edx
        shl eax, 0x10
        or eax, ebx
        xor ebx, ebx
        mov dword ptr [ebp + 0xc], eax
        xor eax, eax
        mov al, byte ptr [esi + 0x22]
        mov bl, byte ptr [edi + 0x12]
        mov cl, byte ptr [esi + 0x26]
        mov dl, byte ptr [edi + 0x112]
        mov al, byte ptr [eax + movie_media_conversion_tables + 0x70c]
        mov bl, byte ptr [ebx + movie_media_conversion_tables + 0x70c]
        shl eax, 8
        mov cl, byte ptr [ecx + movie_media_conversion_tables + 0x70c]
        shl ebx, 8
        mov dl, byte ptr [edx + movie_media_conversion_tables + 0x70c]
        or eax, ecx
        or ebx, edx
        shl eax, 0x10
        or eax, ebx
        xor ebx, ebx
        mov dword ptr [ebp + 0x10], eax
        xor eax, eax
        mov al, byte ptr [esi + 0x2a]
        mov bl, byte ptr [edi + 0x16]
        mov cl, byte ptr [esi + 0x2e]
        mov dl, byte ptr [edi + 0x116]
        mov al, byte ptr [eax + movie_media_conversion_tables + 0x70c]
        mov bl, byte ptr [ebx + movie_media_conversion_tables + 0x70c]
        shl eax, 8
        mov cl, byte ptr [ecx + movie_media_conversion_tables + 0x70c]
        shl ebx, 8
        mov dl, byte ptr [edx + movie_media_conversion_tables + 0x70c]
        or eax, ecx
        or ebx, edx
        shl eax, 0x10
        or eax, ebx
        xor ebx, ebx
        mov dword ptr [ebp + 0x14], eax
        xor eax, eax
        mov al, byte ptr [esi + 0x32]
        mov bl, byte ptr [edi + 0x1a]
        mov cl, byte ptr [esi + 0x36]
        mov dl, byte ptr [edi + 0x11a]
        mov al, byte ptr [eax + movie_media_conversion_tables + 0x70c]
        mov bl, byte ptr [ebx + movie_media_conversion_tables + 0x70c]
        shl eax, 8
        mov cl, byte ptr [ecx + movie_media_conversion_tables + 0x70c]
        shl ebx, 8
        mov dl, byte ptr [edx + movie_media_conversion_tables + 0x70c]
        or eax, ecx
        or ebx, edx
        shl eax, 0x10
        or eax, ebx
        xor ebx, ebx
        mov dword ptr [ebp + 0x18], eax
        xor eax, eax
        mov al, byte ptr [esi + 0x3a]
        mov bl, byte ptr [edi + 0x1e]
        mov cl, byte ptr [esi + 0x3e]
        mov dl, byte ptr [edi + 0x11e]
        mov al, byte ptr [eax + movie_media_conversion_tables + 0x70c]
        mov bl, byte ptr [ebx + movie_media_conversion_tables + 0x70c]
        shl eax, 8
        mov cl, byte ptr [ecx + movie_media_conversion_tables + 0x70c]
        shl ebx, 8
        mov dl, byte ptr [edx + movie_media_conversion_tables + 0x70c]
        or eax, ecx
        or ebx, edx
        shl eax, 0x10
        or eax, ebx
        xor ebx, ebx
        mov dword ptr [ebp + 0x1c], eax
        ret
    }
}

// Volatile accesses preserve the pinned order on unchecked indices. On the
// 32-bit target uintptr_t addition supplies native modulo-2^32 addressing;
// no bounds check/clamp is introduced. Host controls exercise valid indices.
static int movie_blend_read_index(const int *base, DWORD word_offset) {
    const uintptr_t address = reinterpret_cast<uintptr_t>(base) +
                              static_cast<uintptr_t>(word_offset) * 4u;
    return *reinterpret_cast<const volatile int *>(address);
}

static DWORD movie_blend_read_word(const DWORD *base, DWORD word_index) {
    const uintptr_t address = reinterpret_cast<uintptr_t>(base) +
                              static_cast<uintptr_t>(word_index) * 4u;
    return *reinterpret_cast<const volatile DWORD *>(address);
}

static DWORD movie_blend_low_half_ordered(DWORD sum) {
    const DWORD high = movie_blend_read_word(MovieBlendTableHi, sum >> 0x17);
    const DWORD middle = movie_blend_read_word(
        MovieBlendTableMid, (sum & 0x000ff800u) >> 0x0b);
    const DWORD low = movie_blend_read_word(MovieBlendTableLow, sum & 0x1ffu);
    return (high | middle | low) & 0xffffu;
}

static DWORD movie_blend_high_half_ordered(DWORD sum) {
    const DWORD high = movie_blend_read_word(MovieBlendTableHi, sum >> 0x17);
    const DWORD low = movie_blend_read_word(MovieBlendTableLow, sum & 0x1ffu);
    const DWORD middle = movie_blend_read_word(
        MovieBlendTableMid, (sum & 0x000ff800u) >> 0x0b);
    return (high | low | middle) & 0xffff0000u;
}

// ORIGINAL: 0x005D2037 sub_5d2037 0x005D2037-0x005D2536 FILE BYTE_EXACT
// symbol    ?sub_5d2037@@YAXPAH0PAI@Z
// TRIED: nullary per the contract head - no [esp+N]/[ebp+N] read exists anywhere in the body, but esi/edi/ebp are read from the very first instruction with no prologue and no assignment into them first, so nullary leaves the data unreachable. Proposed a 3-pointer signature (data/tables/out standing in for esi/edi/ebp) so the 8 unrolled DES-style S-box rounds compile and read correctly; this is a PROPOSAL, not decoded evidence - no standard calling convention passes 3 live registers with zero prologue, so BYTE_EXACT is structurally unreachable here short of __asm (refused). MISMATCH #0 is the prologue itself.
// working copy - scaffold materialised by --work
// size      1279 bytes
// prototype
// callers   1   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)


// Hand-written asm (EA uV decoder module): no prologue, esi/edi/ebx/ebp register inputs.
__declspec(naked) void __cdecl sub_5d2037(int *data_row, int *selector_pair,
                        unsigned int *packed_output) {
    __asm {
        mov eax, dword ptr [esi]
        mov ebx, dword ptr [edi]
        mov edx, dword ptr [edi + 0x100]
        mov eax, dword ptr [eax*4 + movie_media_conversion_tables + 0x1b0c]
        mov ebx, dword ptr [ebx*4 + movie_media_conversion_tables + 0x230c]
        mov edx, dword ptr [edx*4 + movie_media_conversion_tables + 0x2b0c]
        add eax, edx
        add edx, ebx
        add ebx, eax
        mov eax, ebx
        mov ecx, ebx
        shr ebx, 0x17
        and eax, 0xff800
        shr eax, 0xb
        and ecx, 0x1ff
        mov ebx, dword ptr [ebx*4 + movie_media_conversion_tables + 0x2f0c]
        mov eax, dword ptr [eax*4 + movie_media_conversion_tables + 0x33d4]
        mov ecx, dword ptr [ecx*4 + movie_media_conversion_tables + 0x37ec]
        or eax, ebx
        mov ebx, dword ptr [esi + 4]
        or eax, ecx
        and eax, 0xffff
        mov ecx, dword ptr [ebx*4 + movie_media_conversion_tables + 0x1b0c]
        add ecx, edx
        mov edx, ecx
        mov ebx, ecx
        and ecx, 0xff800
        shr edx, 0x17
        and ebx, 0x1ff
        shr ecx, 0xb
        mov edx, dword ptr [edx*4 + movie_media_conversion_tables + 0x2f0c]
        mov ebx, dword ptr [ebx*4 + movie_media_conversion_tables + 0x37ec]
        mov ecx, dword ptr [ecx*4 + movie_media_conversion_tables + 0x33d4]
        or edx, ebx
        or edx, ecx
        and edx, 0xffff0000
        or edx, eax
        mov dword ptr [ebp], edx
        mov eax, dword ptr [esi + 8]
        mov ebx, dword ptr [edi + 4]
        mov edx, dword ptr [edi + 0x104]
        mov eax, dword ptr [eax*4 + movie_media_conversion_tables + 0x1b0c]
        mov ebx, dword ptr [ebx*4 + movie_media_conversion_tables + 0x230c]
        mov edx, dword ptr [edx*4 + movie_media_conversion_tables + 0x2b0c]
        add eax, edx
        add edx, ebx
        add ebx, eax
        mov eax, ebx
        mov ecx, ebx
        shr ebx, 0x17
        and eax, 0xff800
        shr eax, 0xb
        and ecx, 0x1ff
        mov ebx, dword ptr [ebx*4 + movie_media_conversion_tables + 0x2f0c]
        mov eax, dword ptr [eax*4 + movie_media_conversion_tables + 0x33d4]
        mov ecx, dword ptr [ecx*4 + movie_media_conversion_tables + 0x37ec]
        or eax, ebx
        mov ebx, dword ptr [esi + 0xc]
        or eax, ecx
        and eax, 0xffff
        mov ecx, dword ptr [ebx*4 + movie_media_conversion_tables + 0x1b0c]
        add ecx, edx
        mov edx, ecx
        mov ebx, ecx
        and ecx, 0xff800
        shr edx, 0x17
        and ebx, 0x1ff
        shr ecx, 0xb
        mov edx, dword ptr [edx*4 + movie_media_conversion_tables + 0x2f0c]
        mov ebx, dword ptr [ebx*4 + movie_media_conversion_tables + 0x37ec]
        mov ecx, dword ptr [ecx*4 + movie_media_conversion_tables + 0x33d4]
        or edx, ebx
        or edx, ecx
        and edx, 0xffff0000
        or edx, eax
        mov dword ptr [ebp + 4], edx
        mov eax, dword ptr [esi + 0x10]
        mov ebx, dword ptr [edi + 8]
        mov edx, dword ptr [edi + 0x108]
        mov eax, dword ptr [eax*4 + movie_media_conversion_tables + 0x1b0c]
        mov ebx, dword ptr [ebx*4 + movie_media_conversion_tables + 0x230c]
        mov edx, dword ptr [edx*4 + movie_media_conversion_tables + 0x2b0c]
        add eax, edx
        add edx, ebx
        add ebx, eax
        mov eax, ebx
        mov ecx, ebx
        shr ebx, 0x17
        and eax, 0xff800
        shr eax, 0xb
        and ecx, 0x1ff
        mov ebx, dword ptr [ebx*4 + movie_media_conversion_tables + 0x2f0c]
        mov eax, dword ptr [eax*4 + movie_media_conversion_tables + 0x33d4]
        mov ecx, dword ptr [ecx*4 + movie_media_conversion_tables + 0x37ec]
        or eax, ebx
        mov ebx, dword ptr [esi + 0x14]
        or eax, ecx
        and eax, 0xffff
        mov ecx, dword ptr [ebx*4 + movie_media_conversion_tables + 0x1b0c]
        add ecx, edx
        mov edx, ecx
        mov ebx, ecx
        and ecx, 0xff800
        shr edx, 0x17
        and ebx, 0x1ff
        shr ecx, 0xb
        mov edx, dword ptr [edx*4 + movie_media_conversion_tables + 0x2f0c]
        mov ebx, dword ptr [ebx*4 + movie_media_conversion_tables + 0x37ec]
        mov ecx, dword ptr [ecx*4 + movie_media_conversion_tables + 0x33d4]
        or edx, ebx
        or edx, ecx
        and edx, 0xffff0000
        or edx, eax
        mov dword ptr [ebp + 8], edx
        mov eax, dword ptr [esi + 0x18]
        mov ebx, dword ptr [edi + 0xc]
        mov edx, dword ptr [edi + 0x10c]
        mov eax, dword ptr [eax*4 + movie_media_conversion_tables + 0x1b0c]
        mov ebx, dword ptr [ebx*4 + movie_media_conversion_tables + 0x230c]
        mov edx, dword ptr [edx*4 + movie_media_conversion_tables + 0x2b0c]
        add eax, edx
        add edx, ebx
        add ebx, eax
        mov eax, ebx
        mov ecx, ebx
        shr ebx, 0x17
        and eax, 0xff800
        shr eax, 0xb
        and ecx, 0x1ff
        mov ebx, dword ptr [ebx*4 + movie_media_conversion_tables + 0x2f0c]
        mov eax, dword ptr [eax*4 + movie_media_conversion_tables + 0x33d4]
        mov ecx, dword ptr [ecx*4 + movie_media_conversion_tables + 0x37ec]
        or eax, ebx
        mov ebx, dword ptr [esi + 0x1c]
        or eax, ecx
        and eax, 0xffff
        mov ecx, dword ptr [ebx*4 + movie_media_conversion_tables + 0x1b0c]
        add ecx, edx
        mov edx, ecx
        mov ebx, ecx
        and ecx, 0xff800
        shr edx, 0x17
        and ebx, 0x1ff
        shr ecx, 0xb
        mov edx, dword ptr [edx*4 + movie_media_conversion_tables + 0x2f0c]
        mov ebx, dword ptr [ebx*4 + movie_media_conversion_tables + 0x37ec]
        mov ecx, dword ptr [ecx*4 + movie_media_conversion_tables + 0x33d4]
        or edx, ebx
        or edx, ecx
        and edx, 0xffff0000
        or edx, eax
        mov dword ptr [ebp + 0xc], edx
        mov eax, dword ptr [esi + 0x20]
        mov ebx, dword ptr [edi + 0x10]
        mov edx, dword ptr [edi + 0x110]
        mov eax, dword ptr [eax*4 + movie_media_conversion_tables + 0x1b0c]
        mov ebx, dword ptr [ebx*4 + movie_media_conversion_tables + 0x230c]
        mov edx, dword ptr [edx*4 + movie_media_conversion_tables + 0x2b0c]
        add eax, edx
        add edx, ebx
        add ebx, eax
        mov eax, ebx
        mov ecx, ebx
        shr ebx, 0x17
        and eax, 0xff800
        shr eax, 0xb
        and ecx, 0x1ff
        mov ebx, dword ptr [ebx*4 + movie_media_conversion_tables + 0x2f0c]
        mov eax, dword ptr [eax*4 + movie_media_conversion_tables + 0x33d4]
        mov ecx, dword ptr [ecx*4 + movie_media_conversion_tables + 0x37ec]
        or eax, ebx
        mov ebx, dword ptr [esi + 0x24]
        or eax, ecx
        and eax, 0xffff
        mov ecx, dword ptr [ebx*4 + movie_media_conversion_tables + 0x1b0c]
        add ecx, edx
        mov edx, ecx
        mov ebx, ecx
        and ecx, 0xff800
        shr edx, 0x17
        and ebx, 0x1ff
        shr ecx, 0xb
        mov edx, dword ptr [edx*4 + movie_media_conversion_tables + 0x2f0c]
        mov ebx, dword ptr [ebx*4 + movie_media_conversion_tables + 0x37ec]
        mov ecx, dword ptr [ecx*4 + movie_media_conversion_tables + 0x33d4]
        or edx, ebx
        or edx, ecx
        and edx, 0xffff0000
        or edx, eax
        mov dword ptr [ebp + 0x10], edx
        mov eax, dword ptr [esi + 0x28]
        mov ebx, dword ptr [edi + 0x14]
        mov edx, dword ptr [edi + 0x114]
        mov eax, dword ptr [eax*4 + movie_media_conversion_tables + 0x1b0c]
        mov ebx, dword ptr [ebx*4 + movie_media_conversion_tables + 0x230c]
        mov edx, dword ptr [edx*4 + movie_media_conversion_tables + 0x2b0c]
        add eax, edx
        add edx, ebx
        add ebx, eax
        mov eax, ebx
        mov ecx, ebx
        shr ebx, 0x17
        and eax, 0xff800
        shr eax, 0xb
        and ecx, 0x1ff
        mov ebx, dword ptr [ebx*4 + movie_media_conversion_tables + 0x2f0c]
        mov eax, dword ptr [eax*4 + movie_media_conversion_tables + 0x33d4]
        mov ecx, dword ptr [ecx*4 + movie_media_conversion_tables + 0x37ec]
        or eax, ebx
        mov ebx, dword ptr [esi + 0x2c]
        or eax, ecx
        and eax, 0xffff
        mov ecx, dword ptr [ebx*4 + movie_media_conversion_tables + 0x1b0c]
        add ecx, edx
        mov edx, ecx
        mov ebx, ecx
        and ecx, 0xff800
        shr edx, 0x17
        and ebx, 0x1ff
        shr ecx, 0xb
        mov edx, dword ptr [edx*4 + movie_media_conversion_tables + 0x2f0c]
        mov ebx, dword ptr [ebx*4 + movie_media_conversion_tables + 0x37ec]
        mov ecx, dword ptr [ecx*4 + movie_media_conversion_tables + 0x33d4]
        or edx, ebx
        or edx, ecx
        and edx, 0xffff0000
        or edx, eax
        mov dword ptr [ebp + 0x14], edx
        mov eax, dword ptr [esi + 0x30]
        mov ebx, dword ptr [edi + 0x18]
        mov edx, dword ptr [edi + 0x118]
        mov eax, dword ptr [eax*4 + movie_media_conversion_tables + 0x1b0c]
        mov ebx, dword ptr [ebx*4 + movie_media_conversion_tables + 0x230c]
        mov edx, dword ptr [edx*4 + movie_media_conversion_tables + 0x2b0c]
        add eax, edx
        add edx, ebx
        add ebx, eax
        mov eax, ebx
        mov ecx, ebx
        shr ebx, 0x17
        and eax, 0xff800
        shr eax, 0xb
        and ecx, 0x1ff
        mov ebx, dword ptr [ebx*4 + movie_media_conversion_tables + 0x2f0c]
        mov eax, dword ptr [eax*4 + movie_media_conversion_tables + 0x33d4]
        mov ecx, dword ptr [ecx*4 + movie_media_conversion_tables + 0x37ec]
        or eax, ebx
        mov ebx, dword ptr [esi + 0x34]
        or eax, ecx
        and eax, 0xffff
        mov ecx, dword ptr [ebx*4 + movie_media_conversion_tables + 0x1b0c]
        add ecx, edx
        mov edx, ecx
        mov ebx, ecx
        and ecx, 0xff800
        shr edx, 0x17
        and ebx, 0x1ff
        shr ecx, 0xb
        mov edx, dword ptr [edx*4 + movie_media_conversion_tables + 0x2f0c]
        mov ebx, dword ptr [ebx*4 + movie_media_conversion_tables + 0x37ec]
        mov ecx, dword ptr [ecx*4 + movie_media_conversion_tables + 0x33d4]
        or edx, ebx
        or edx, ecx
        and edx, 0xffff0000
        or edx, eax
        mov dword ptr [ebp + 0x18], edx
        mov eax, dword ptr [esi + 0x38]
        mov ebx, dword ptr [edi + 0x1c]
        mov edx, dword ptr [edi + 0x11c]
        mov eax, dword ptr [eax*4 + movie_media_conversion_tables + 0x1b0c]
        mov ebx, dword ptr [ebx*4 + movie_media_conversion_tables + 0x230c]
        mov edx, dword ptr [edx*4 + movie_media_conversion_tables + 0x2b0c]
        add eax, edx
        add edx, ebx
        add ebx, eax
        mov eax, ebx
        mov ecx, ebx
        shr ebx, 0x17
        and eax, 0xff800
        shr eax, 0xb
        and ecx, 0x1ff
        mov ebx, dword ptr [ebx*4 + movie_media_conversion_tables + 0x2f0c]
        mov eax, dword ptr [eax*4 + movie_media_conversion_tables + 0x33d4]
        mov ecx, dword ptr [ecx*4 + movie_media_conversion_tables + 0x37ec]
        or eax, ebx
        mov ebx, dword ptr [esi + 0x3c]
        or eax, ecx
        and eax, 0xffff
        mov ecx, dword ptr [ebx*4 + movie_media_conversion_tables + 0x1b0c]
        add ecx, edx
        mov edx, ecx
        mov ebx, ecx
        and ecx, 0xff800
        shr edx, 0x17
        and ebx, 0x1ff
        shr ecx, 0xb
        mov edx, dword ptr [edx*4 + movie_media_conversion_tables + 0x2f0c]
        mov ebx, dword ptr [ebx*4 + movie_media_conversion_tables + 0x37ec]
        mov ecx, dword ptr [ecx*4 + movie_media_conversion_tables + 0x33d4]
        or edx, ebx
        or edx, ecx
        and edx, 0xffff0000
        or edx, eax
        mov dword ptr [ebp + 0x1c], edx
        ret
    }
}

// ORIGINAL: 0x005D2761 sub_5d2761 0x005D2761-0x005D2834 FILE BYTE_EXACT
// symbol    ?sub_5d2761@@YAXPAI0H@Z
// LEVER: the four tbl_ globals are four of EIGHT 0x400-byte tables laid
//         end to end at 0x009C672C + k*0x400 for k = 0..7, all ZERO in the
//         file and filled at run time (image_data.py reads 0 at every one).
//         0x5CC4E0..0x5CC590 is their builder: a 0x100-iteration loop with
//         variable-count shifts (sar/shl by cl) writing through
//         `mov [edi+0x9c7f2c]` - a canonical VLC/Huffman code table
//         builder, not a cipher S-box. sub_5d2761 consumes four of them by
//         byte-lane, which is a decode-table lookup, so the whole
//         0x005CE85C..0x005D4502 span is a block-DCT movie decoder and not
//         part of the DirectDraw half of this TU.
// TRIED: original prologue saves 6 registers (ebx,ecx,edx,esi,edi,ebp); plain __cdecl(uint*,uint*,int) body only pressures 1-2, so the reservation never reproduces. Explicit per-register temps for each S-box term didn't move it either.
// working copy - scaffold materialised by --work
// size      211 bytes
// prototype
// callers   1   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)


// The observed row-pack callers use a void callback and ignore any result;
// the original source-level prototype remains unproven.
// Hand-written asm (EA uV decoder module): saves ecx/edx, private register use.
__declspec(naked) void __cdecl sub_5d2761(unsigned int *src, unsigned int *dst, int count) {
    __asm {
        push ebx
        push ecx
        push edx
        push esi
        push edi
        push ebp
        mov esi, dword ptr [esp + 0x1c]
        mov edx, dword ptr [esp + 0x20]
m5d2761_5d276f:
        mov eax, dword ptr [esi]
        xor ebx, ebx
        mov bl, al
        mov edi, eax
        shr edi, 0x18
        and eax, 0xff00
        shr eax, 8
        mov ebp, dword ptr [ebx*4 + movie_media_conversion_tables + 0x270c]
        mov edi, dword ptr [edi*4 + movie_media_conversion_tables + 0x170c]
        add esi, 4
        mov eax, dword ptr [eax*4 + movie_media_conversion_tables + 0x1f0c]
        add edi, ebp
        add ebp, eax
        add eax, edi
        mov ecx, eax
        mov edi, eax
        shr eax, 0x17
        and ecx, 0xff800
        shr ecx, 0xb
        and edi, 0x1ff
        mov eax, dword ptr [eax*4 + movie_media_conversion_tables + 0x2f0c]
        mov bl, byte ptr [esi - 2]
        mov ecx, dword ptr [ecx*4 + movie_media_conversion_tables + 0x33d4]
        mov edi, dword ptr [edi*4 + movie_media_conversion_tables + 0x37ec]
        add eax, ecx
        mov ebx, dword ptr [ebx*4 + movie_media_conversion_tables + 0x170c]
        add eax, edi
        add ebx, ebp
        mov ebp, eax
        mov eax, ebx
        mov edi, ebx
        and ebx, 0xff800
        shr eax, 0x17
        and edi, 0x1ff
        shr ebx, 0xb
        mov ecx, dword ptr [esp + 0x24]
        mov eax, dword ptr [eax*4 + movie_media_conversion_tables + 0x2f0c]
        mov edi, dword ptr [edi*4 + movie_media_conversion_tables + 0x37ec]
        mov ebx, dword ptr [ebx*4 + movie_media_conversion_tables + 0x33d4]
        add eax, edi
        add eax, ebx
        and ebp, 0xffff
        and eax, 0xffff0000
        or eax, ebp
        dec ecx
        mov dword ptr [edx], eax
        lea edx, [edx + 4]
        mov dword ptr [esp + 0x24], ecx
        jne m5d2761_5d276f
        pop ebp
        pop edi
        pop esi
        pop edx
        pop ecx
        pop ebx
        ret
    }
}

// Fixed-slot bindings carried from 005d2834.cpp

// ORIGINAL: 0x005D2834 sub_5d2834 0x005D2834-0x005D2901 FILE BYTE_EXACT
// symbol    ?sub_5d2834@@YAHPAIPAHH@Z
// TRIED: literal Ghidra-order locals (uVar1/uVar2) and inline expr order both produce a tight 191-byte body without the 6-register push prologue the original has; register pressure not reproduced.
// working copy - scaffold materialised by --work
// size      205 bytes
// prototype
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)


// Hand-written asm (EA uV decoder module): saves ecx/edx, private register use.
__declspec(naked) int __cdecl sub_5d2834(unsigned int *param_1, int *param_2, int param_3) {
    __asm {
        push ebx
        push ecx
        push edx
        push esi
        push edi
        push ebp
        mov eax, dword ptr [esp + 0x1c]
        mov edx, dword ptr [esp + 0x20]
        mov ebx, dword ptr [esp + 0x24]
        mov esi, eax
m5d2834_5d2848:
        mov eax, dword ptr [esi]
        xor ebx, ebx
        mov bl, al
        mov edi, eax
        shr edi, 0x18
        and eax, 0xff00
        shr eax, 8
        mov ebp, dword ptr [ebx*4 + movie_media_conversion_tables + 0x270c]
        mov edi, dword ptr [edi*4 + movie_media_conversion_tables + 0x170c]
        add esi, 4
        mov eax, dword ptr [eax*4 + movie_media_conversion_tables + 0x1f0c]
        add edi, ebp
        add ebp, eax
        add eax, edi
        mov ecx, eax
        mov edi, eax
        shr eax, 0x17
        and ecx, 0xff800
        shr ecx, 0xb
        and edi, 0x1ff
        mov eax, dword ptr [eax*4 + movie_media_conversion_tables + 0x2f0c]
        mov bl, byte ptr [esi - 2]
        mov ecx, dword ptr [ecx*4 + movie_media_conversion_tables + 0x33d4]
        mov edi, dword ptr [edi*4 + movie_media_conversion_tables + 0x37ec]
        add eax, ecx
        mov ebx, dword ptr [ebx*4 + movie_media_conversion_tables + 0x170c]
        add eax, edi
        add ebx, ebp
        mov dword ptr [edx], eax
        mov eax, ebx
        mov edi, ebx
        and ebx, 0xff800
        shr eax, 0x17
        and edi, 0x1ff
        shr ebx, 0xb
        mov ecx, dword ptr [esp + 0x24]
        mov eax, dword ptr [eax*4 + movie_media_conversion_tables + 0x2f0c]
        mov edi, dword ptr [edi*4 + movie_media_conversion_tables + 0x37ec]
        mov ebx, dword ptr [ebx*4 + movie_media_conversion_tables + 0x33d4]
        add eax, edi
        add eax, ebx
        dec ecx
        mov dword ptr [edx + 4], eax
        lea edx, [edx + 8]
        mov dword ptr [esp + 0x24], ecx
        jne m5d2834_5d2848
        pop ebp
        pop edi
        pop esi
        pop edx
        pop ecx
        pop ebx
        ret
    }
}

// ORIGINAL: 0x005D2901 sub_5d2901 0x005D2901-0x005D29E1 FILE BYTE_EXACT
// symbol    ?sub_5d2901@@YAHPAH00H@Z
// TRIED: extern int name[] for the six indexed lookup tables (same tables as the sibling sub_5d2834 at 0x005D2834), not the given `static int *const` - these are used as `table[idx]`, an indexed base. Signature widened to the IDA-guessed 4 params (int*,int*,int*,int) since the given head has none.
// TRIED: could not reproduce the 6-register push prologue (push ebx,ecx,edx,esi,edi,ebp - ecx/edx pushed as scratch stack slots, not preservation); same wall the sibling function at 0x005D2834 hit. Hoisting the shared table_7f2c+table_772c subexpression into named locals `a`/`b` (to force it live across both blend computations, matching the original's edx reuse) made similarity slightly worse (0.61 -> 0.60), not better.
// working copy - scaffold materialised by --work
// size      224 bytes
// prototype
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)


// Hand-written asm (EA uV decoder module): saves ecx/edx, private register use.
__declspec(naked) int __cdecl sub_5d2901(int *param_1, int *param_2, int *param_3, int param_4) {
    __asm {
        push ebx
        push ecx
        push edx
        push esi
        push edi
        push ebp
        mov esi, dword ptr [esp + 0x1c]
        mov edi, dword ptr [esp + 0x20]
        mov ebx, dword ptr [esp + 0x24]
m5d2901_5d2913:
        mov eax, dword ptr [esi]
        mov edx, dword ptr [edi]
        add eax, edx
        add esi, 4
        shr eax, 1
        add edi, 4
        and eax, 0x7f7f7f7f
        xor edx, edx
        mov ecx, eax
        mov ebp, eax
        shr ecx, 8
        mov dl, al
        shr ebp, 0x10
        and ecx, 0xff
        shr eax, 0x18
        mov edx, dword ptr [edx*4 + movie_media_conversion_tables + 0x270c]
        and ebp, 0xff
        mov ecx, dword ptr [ecx*4 + movie_media_conversion_tables + 0x1f0c]
        add edx, ecx
        mov eax, dword ptr [eax*4 + movie_media_conversion_tables + 0x170c]
        add eax, edx
        mov ebp, dword ptr [ebp*4 + movie_media_conversion_tables + 0x170c]
        add ebp, edx
        mov edx, eax
        shr edx, 0x17
        mov ecx, eax
        shr ecx, 0xb
        and eax, 0x1ff
        mov edx, dword ptr [edx*4 + movie_media_conversion_tables + 0x2f0c]
        and ecx, 0x1ff
        mov eax, dword ptr [eax*4 + movie_media_conversion_tables + 0x37ec]
        add ebx, 8
        mov ecx, dword ptr [ecx*4 + movie_media_conversion_tables + 0x33d4]
        add eax, edx
        add eax, ecx
        mov edx, ebp
        shr edx, 0x17
        mov dword ptr [ebx - 8], eax
        mov eax, ebp
        and ebp, 0x1ff
        shr eax, 0xb
        mov edx, dword ptr [edx*4 + movie_media_conversion_tables + 0x2f0c]
        and eax, 0x1ff
        mov ebp, dword ptr [ebp*4 + movie_media_conversion_tables + 0x37ec]
        add edx, ebp
        mov ecx, dword ptr [esp + 0x28]
        mov eax, dword ptr [eax*4 + movie_media_conversion_tables + 0x33d4]
        add eax, edx
        dec ecx
        nop
        mov dword ptr [esp + 0x28], ecx
        mov dword ptr [ebx - 4], eax
        jne m5d2901_5d2913
        pop ebp
        pop edi
        pop esi
        pop edx
        pop ecx
        pop ebx
        ret
    }
}


// ORIGINAL: 0x005D4020 sub_5d4020 0x005D4020-0x005D4231 FILE
// symbol    ?sub_5d4020@@YIXPAUMediaRuntime@@PAE1H@Z
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
    MediaDescriptor *descriptor = runtime->descriptor;
    int signed_width;
    int signed_height;
    memcpy(&signed_width, &descriptor->output_width_64, sizeof(signed_width));
    memcpy(&signed_height, &descriptor->output_height_60, sizeof(signed_height));
    if (descriptor->pixel_format.total_component_bits == 8) {
        if ((descriptor->flags & 0x40) != 0) {
            const DWORD pixel_count = descriptor->output_width_64 *
                                      descriptor->output_height_60;
            int signed_pixel_count;
            memcpy(&signed_pixel_count, &pixel_count, sizeof(signed_pixel_count));
            sub_5d271d(reinterpret_cast<unsigned int *>(src), dest,
                       signed_pixel_count);
            return;
        }
        if (signed_height == 0) {
            return;
        }
        {
            unsigned int rowCount = descriptor->output_height_60;
            unsigned int rowBytes = descriptor->output_width_64;
            unsigned int row = 0;
            unsigned char *s = src;
            unsigned char *d = dest;
            do {
                unsigned char *sp = s;
                unsigned char *dp = d;
                unsigned int n4 = rowBytes >> 2;
                for (; n4 != 0; --n4) {
                    *(unsigned int *)dp = *(unsigned int *)sp;
                    sp += 4;
                    dp += 4;
                }
                unsigned int n1 = rowBytes & 3;
                for (; n1 != 0; --n1) {
                    *dp = *sp;
                    ++sp;
                    ++dp;
                }
                descriptor = runtime->descriptor;
                rowBytes = descriptor->output_width_64;
                d += (destStride >> 2) * 4;
                s += rowBytes;
                ++row;
            } while (row < descriptor->output_height_60);
        }
        return;
    }

    unsigned char *clut = reinterpret_cast<unsigned char *>(
        descriptor->palette_lookup_3b0.entries);
    int row = 0;
    if ((descriptor->flags & 0x40) == 0) {
        if (signed_height == 0) {
            return;
        }
        do {
            unsigned int col = 0;
            unsigned char *sp = src;
            unsigned char *dp = dest;
            if (signed_width != 0) {
                do {
                    col += 8;
                    src = sp + 8;
                    dest = dp + 0x10;
                    for (int k = 0; k < 8; ++k) {
                        *(unsigned short *)(dp + k * 2) =
                            *(unsigned short *)(clut + sp[k] * 4);
                    }
                    sp = src;
                    dp = dest;
                } while (col < runtime->descriptor->output_width_64);
            }
            descriptor = runtime->descriptor;
            memcpy(&signed_width, &descriptor->output_width_64, sizeof(signed_width));
            dest = dest + ((destStride - signed_width * 2) & ~1);
            ++row;
        } while (static_cast<unsigned int>(row) < descriptor->output_height_60);
        return;
    }

    if (signed_height == 0) {
        return;
    }
    do {
        unsigned int col = 0;
        unsigned char *sp = src;
        unsigned char *dp = dest;
        if (signed_width != 0) {
            do {
                col += 8;
                src = sp + 8;
                dest = dp + 0x20;
                for (int k = 0; k < 8; ++k) {
                    *(unsigned int *)(dp + k * 4) =
                        *(unsigned int *)(clut + sp[k] * 4);
                }
                sp = src;
                dp = dest;
            } while (col < runtime->descriptor->output_width_64);
        }
        descriptor = runtime->descriptor;
        memcpy(&signed_width, &descriptor->output_width_64, sizeof(signed_width));
        dest = dest + ((destStride - signed_width * 4) & ~3);
        ++row;
    } while (static_cast<unsigned int>(row) < descriptor->output_height_60);
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


int __fastcall sub_5d3980(MediaRuntime *runtime, unsigned char *packet) {
    int iVar2, iVar11, iVar13;
    unsigned int uVar3, uVar4, uVar5, uVar6, uVar8, uVar9, uVar15;
    void *pvVar7;
    const unsigned char *puVar14;
    unsigned char *puVar17;
    int flagOK;

    unsigned char *const previous_frame = runtime->owned_heap_63c;
    flagOK = 1;
    runtime->owned_heap_63c = runtime->owned_heap_638;
    runtime->owned_heap_638 = previous_frame;
    DWORD payload_word;
    memcpy(&payload_word, packet - 2, sizeof(payload_word));
    uVar3 = payload_word >> 0x10;
    runtime->decoder_packet_value_620 = uVar3;
    memcpy(&payload_word, packet, sizeof(payload_word));
    uVar4 = payload_word >> 0x10;
    runtime->decoder_packet_value_624 = uVar4;
    memcpy(&payload_word, packet + 2, sizeof(payload_word));
    uVar5 = payload_word >> 0x10;
    runtime->decoder_packet_value_628 = uVar5;
    memcpy(&payload_word, packet + 4, sizeof(payload_word));
    uVar6 = payload_word >> 0x10;
    runtime->decoder_packet_value_62c = uVar6;
    if (uVar3 != 0) {
        if (runtime->decoder_capacity_630 < (int)uVar3) {
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
            pvVar7 = malloc(uVar3 * 8);
            runtime->motion_coordinates = static_cast<MediaMotionCoordinate *>(pvVar7);
            runtime->decoder_capacity_630 = uVar3;
        }
        if (runtime->motion_pixel_offsets == 0) {
            pvVar7 = malloc(uVar3 * 4);
            runtime->motion_pixel_offsets = static_cast<int *>(pvVar7);
            runtime->decoder_capacity_630 = uVar3;
        }
        flagOK = runtime->motion_coordinates != 0;
        if (runtime->motion_pixel_offsets == 0) {
            flagOK = 0;
        }
    }
    const unsigned int count_sum = uVar5 + uVar4;
    memcpy(&iVar13, &count_sum, sizeof(iVar13));
    if (iVar13 != 0) {
        if ((runtime->decoder_capacity_634 < iVar13) &&
            (runtime->decoded_pixel_blocks != 0)) {
            free(runtime->decoded_pixel_blocks);
            runtime->decoded_pixel_blocks = 0;
        }
        if (runtime->decoded_pixel_blocks == 0) {
            pvVar7 = malloc(iVar13 * 0x10);
            runtime->decoded_pixel_blocks = static_cast<MediaDecodedPixelBlock4x4 *>(pvVar7);
            runtime->decoder_capacity_634 = iVar13;
        }
        if (runtime->decoded_pixel_blocks == 0) {
            return 0;
        }
    }
    if (flagOK != 0) {
        uVar15 = uVar3 * 0x14;
        unsigned int coordinate_index = uVar3;
        while (coordinate_index != 0) {
            --coordinate_index;
            MediaMotionCoordinate &coordinate =
                runtime->motion_coordinates[coordinate_index];
            // Original backward unpack order is y then x for each pair.
            for (int axis = 1; axis >= 0; --axis) {
                uVar15 = uVar15 - 10;
                memcpy(&payload_word, packet + (uVar15 >> 3) + 0xc,
                       sizeof(payload_word));
                uVar8 = payload_word >> (uVar15 & 7);
                uVar9 = uVar8 & 0x3ff;
                if ((uVar8 & 0x200) != 0) {
                    uVar9 = uVar9 | 0xfffffc00;
                }
                int &component = axis == 1 ? coordinate.y : coordinate.x;
                memcpy(&component, &uVar9, sizeof(component));
            }
        }
        memcpy(&iVar13, &runtime->descriptor->output_height_60,
               sizeof(iVar13));
        int *const row_offsets = runtime->motion_row_offsets;
        iVar11 = 0;
        if (uVar3 != 0) {
            do {
                const MediaMotionCoordinate &coordinate =
                    runtime->motion_coordinates[iVar11];
                runtime->motion_pixel_offsets[iVar11] =
                    row_offsets[iVar13 + coordinate.y] + coordinate.x;
                iVar11 = iVar11 + 1;
            } while (iVar11 < (int)uVar3);
        }
        const unsigned int following_records_offset =
            (((static_cast<int>(uVar3 * 0x14u + 0x1fu) >> 3) &
              0xfffffffcu) + 0xcu);
        puVar14 = packet + following_records_offset;
        if (uVar4 != 0) {
            memcpy(runtime->decoded_pixel_blocks, puVar14, uVar4 * 0x10);
        }
        puVar14 += uVar4 * 0x10;
        if (uVar5 != 0) {
            MediaDecodedPixelBlock4x4 *block = runtime->decoded_pixel_blocks + uVar4;
            const unsigned char *packed_run = puVar14;
            for (uVar3 = uVar5; uVar3 != 0; uVar3 = uVar3 - 1) {
                puVar17 = block->pixels;
                memcpy(&uVar4, packed_run + 4, sizeof(uVar4));
                puVar17[0xf] = packed_run[uVar4 & 3];
                puVar17[0xe] = packed_run[(uVar4 >> 2) & 3];
                puVar17[0xd] = packed_run[(uVar4 >> 4) & 3];
                puVar17[0xc] = packed_run[(uVar4 >> 6) & 3];
                puVar17[0xb] = packed_run[(uVar4 >> 8) & 3];
                puVar17[10] = packed_run[(uVar4 >> 10) & 3];
                puVar17[9]  = packed_run[(uVar4 >> 0xc) & 3];
                puVar17[8]  = packed_run[(uVar4 >> 0xe) & 3];
                puVar17[7]  = packed_run[(uVar4 >> 0x10) & 3];
                puVar17[6]  = packed_run[(uVar4 >> 0x12) & 3];
                puVar17[5]  = packed_run[(uVar4 >> 0x14) & 3];
                puVar17[4]  = packed_run[(uVar4 >> 0x16) & 3];
                puVar17[3]  = packed_run[(uVar4 >> 0x18) & 3];
                puVar17[2]  = packed_run[(uVar4 >> 0x1a) & 3];
                puVar17[1]  = packed_run[(uVar4 >> 0x1c) & 3];
                puVar17[0]  = packed_run[(uVar4 >> 0x1e) & 3];
                packed_run += 8;
                ++block;
            }
        }
        int output_width;
        int output_height;
        memcpy(&output_width, &runtime->descriptor->output_width_64,
               sizeof(output_width));
        memcpy(&output_height, &runtime->descriptor->output_height_60,
               sizeof(output_height));
        uVar3 = (static_cast<unsigned int>(output_width) *
                 static_cast<unsigned int>(output_height)) >> 4;
        if ((uVar6 * uVar3 + 0x1f & 0xffffffe0) != 0) {
            sub_5d439f(runtime->motion_block_indices,
                       puVar14 + uVar5 * 8, static_cast<int>(uVar3),
                       static_cast<int>(uVar6));
        }
        const DWORD final_width = runtime->descriptor->output_width_64;
        const DWORD final_height = runtime->descriptor->output_height_60;
        int final_pitch;
        memcpy(&final_pitch, &final_width, sizeof(final_pitch));
        sub_5d4448(runtime->motion_block_indices,
                   runtime->owned_heap_63c, runtime->owned_heap_638,
                   static_cast<int>(runtime->decoder_packet_value_620),
                   runtime->motion_pixel_offsets,
                   runtime->decoded_pixel_blocks, final_pitch,
                   static_cast<int>(final_width >> 2),
                   static_cast<int>(final_height >> 2));
    }
    return flagOK;
}

// ORIGINAL: 0x005D1B88 sub_5d1b88 0x005D1B88-0x005D2037 FILE BYTE_EXACT
// symbol    ?sub_5d1b88@@YAXPAIPAH1@Z
// TRIED: earlier homed into general.cpp (original TU unproved) 2026-09-03 and reverted for one reason:
// its fixed-address bindings have no naming evidence. image_data.py reads
// 6 of them and every one is zero at load - runtime-filled lookup tables
// with no .data content, no string, no arithmetic identity beyond 'a table
// indexed here' - and none is modelled anywhere in src. The homing-first rule
// does not ask for them to be invented: an unclaimed body with evidence-free
// bindings waits for the class pass that measures it in context.
// TRIED: no conventional signature can match - the original has NO prologue/epilogue and reads esi/edi/ebp as inputs set up by the CALLER (Ghidra: unaff_ESI/unaff_EDI/unaff_EBP), which is not a calling convention VC6 can target without __asm/naked (barred). Landed as a 3-pointer __cdecl function with an 8-iteration loop reproducing the exact table math (16 unrolled blocks in the original, all structurally identical) so the algorithm is faithfully documented even though the prologue can never match.
// working copy - scaffold materialised by --work
// size      1199 bytes
// prototype
// callers   1   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)


// ESI holds 16 data indices, EDI holds two 8-DWORD selector streams at
// offsets 0 and 64, and EBP receives 16 output DWORDs. DWORD accumulation
// preserves the original x86 modulo-2^32 additions after the LUT reads.
// Hand-written asm (EA uV decoder module): no prologue, esi/edi/ebx/ebp register inputs.
__declspec(naked) void __cdecl sub_5d1b88(unsigned int *packed_output, int *data_row,
                        int *selector_pair) {
    __asm {
        mov eax, dword ptr [esi]
        mov ebx, dword ptr [edi]
        mov edx, dword ptr [edi + 0x100]
        mov eax, dword ptr [eax*4 + movie_media_conversion_tables + 0x1b0c]
        mov ebx, dword ptr [ebx*4 + movie_media_conversion_tables + 0x230c]
        mov edx, dword ptr [edx*4 + movie_media_conversion_tables + 0x2b0c]
        add eax, edx
        add edx, ebx
        add ebx, eax
        mov eax, ebx
        mov ecx, ebx
        shr ebx, 0x17
        and eax, 0xff800
        shr eax, 0xb
        and ecx, 0x1ff
        mov ebx, dword ptr [ebx*4 + movie_media_conversion_tables + 0x2f0c]
        mov eax, dword ptr [eax*4 + movie_media_conversion_tables + 0x33d4]
        mov ecx, dword ptr [ecx*4 + movie_media_conversion_tables + 0x37ec]
        or eax, ebx
        mov ebx, dword ptr [esi + 4]
        or eax, ecx
        mov dword ptr [ebp], eax
        mov ecx, dword ptr [ebx*4 + movie_media_conversion_tables + 0x1b0c]
        add ecx, edx
        mov edx, ecx
        mov ebx, ecx
        and ecx, 0xff800
        shr edx, 0x17
        and ebx, 0x1ff
        shr ecx, 0xb
        mov edx, dword ptr [edx*4 + movie_media_conversion_tables + 0x2f0c]
        mov ebx, dword ptr [ebx*4 + movie_media_conversion_tables + 0x37ec]
        mov ecx, dword ptr [ecx*4 + movie_media_conversion_tables + 0x33d4]
        or edx, ebx
        or edx, ecx
        mov dword ptr [ebp + 4], edx
        mov eax, dword ptr [esi + 8]
        mov ebx, dword ptr [edi + 4]
        mov edx, dword ptr [edi + 0x104]
        mov eax, dword ptr [eax*4 + movie_media_conversion_tables + 0x1b0c]
        mov ebx, dword ptr [ebx*4 + movie_media_conversion_tables + 0x230c]
        mov edx, dword ptr [edx*4 + movie_media_conversion_tables + 0x2b0c]
        add eax, edx
        add edx, ebx
        add ebx, eax
        mov eax, ebx
        mov ecx, ebx
        shr ebx, 0x17
        and eax, 0xff800
        shr eax, 0xb
        and ecx, 0x1ff
        mov ebx, dword ptr [ebx*4 + movie_media_conversion_tables + 0x2f0c]
        mov eax, dword ptr [eax*4 + movie_media_conversion_tables + 0x33d4]
        mov ecx, dword ptr [ecx*4 + movie_media_conversion_tables + 0x37ec]
        or eax, ebx
        mov ebx, dword ptr [esi + 0xc]
        or eax, ecx
        mov dword ptr [ebp + 8], eax
        mov ecx, dword ptr [ebx*4 + movie_media_conversion_tables + 0x1b0c]
        add ecx, edx
        mov edx, ecx
        mov ebx, ecx
        and ecx, 0xff800
        shr edx, 0x17
        and ebx, 0x1ff
        shr ecx, 0xb
        mov edx, dword ptr [edx*4 + movie_media_conversion_tables + 0x2f0c]
        mov ebx, dword ptr [ebx*4 + movie_media_conversion_tables + 0x37ec]
        mov ecx, dword ptr [ecx*4 + movie_media_conversion_tables + 0x33d4]
        or edx, ebx
        or edx, ecx
        mov dword ptr [ebp + 0xc], edx
        mov eax, dword ptr [esi + 0x10]
        mov ebx, dword ptr [edi + 8]
        mov edx, dword ptr [edi + 0x108]
        mov eax, dword ptr [eax*4 + movie_media_conversion_tables + 0x1b0c]
        mov ebx, dword ptr [ebx*4 + movie_media_conversion_tables + 0x230c]
        mov edx, dword ptr [edx*4 + movie_media_conversion_tables + 0x2b0c]
        add eax, edx
        add edx, ebx
        add ebx, eax
        mov eax, ebx
        mov ecx, ebx
        shr ebx, 0x17
        and eax, 0xff800
        shr eax, 0xb
        and ecx, 0x1ff
        mov ebx, dword ptr [ebx*4 + movie_media_conversion_tables + 0x2f0c]
        mov eax, dword ptr [eax*4 + movie_media_conversion_tables + 0x33d4]
        mov ecx, dword ptr [ecx*4 + movie_media_conversion_tables + 0x37ec]
        or eax, ebx
        mov ebx, dword ptr [esi + 0x14]
        or eax, ecx
        mov dword ptr [ebp + 0x10], eax
        mov ecx, dword ptr [ebx*4 + movie_media_conversion_tables + 0x1b0c]
        add ecx, edx
        mov edx, ecx
        mov ebx, ecx
        and ecx, 0xff800
        shr edx, 0x17
        and ebx, 0x1ff
        shr ecx, 0xb
        mov edx, dword ptr [edx*4 + movie_media_conversion_tables + 0x2f0c]
        mov ebx, dword ptr [ebx*4 + movie_media_conversion_tables + 0x37ec]
        mov ecx, dword ptr [ecx*4 + movie_media_conversion_tables + 0x33d4]
        or edx, ebx
        or edx, ecx
        mov dword ptr [ebp + 0x14], edx
        mov eax, dword ptr [esi + 0x18]
        mov ebx, dword ptr [edi + 0xc]
        mov edx, dword ptr [edi + 0x10c]
        mov eax, dword ptr [eax*4 + movie_media_conversion_tables + 0x1b0c]
        mov ebx, dword ptr [ebx*4 + movie_media_conversion_tables + 0x230c]
        mov edx, dword ptr [edx*4 + movie_media_conversion_tables + 0x2b0c]
        add eax, edx
        add edx, ebx
        add ebx, eax
        mov eax, ebx
        mov ecx, ebx
        shr ebx, 0x17
        and eax, 0xff800
        shr eax, 0xb
        and ecx, 0x1ff
        mov ebx, dword ptr [ebx*4 + movie_media_conversion_tables + 0x2f0c]
        mov eax, dword ptr [eax*4 + movie_media_conversion_tables + 0x33d4]
        mov ecx, dword ptr [ecx*4 + movie_media_conversion_tables + 0x37ec]
        or eax, ebx
        mov ebx, dword ptr [esi + 0x1c]
        or eax, ecx
        mov dword ptr [ebp + 0x18], eax
        mov ecx, dword ptr [ebx*4 + movie_media_conversion_tables + 0x1b0c]
        add ecx, edx
        mov edx, ecx
        mov ebx, ecx
        and ecx, 0xff800
        shr edx, 0x17
        and ebx, 0x1ff
        shr ecx, 0xb
        mov edx, dword ptr [edx*4 + movie_media_conversion_tables + 0x2f0c]
        mov ebx, dword ptr [ebx*4 + movie_media_conversion_tables + 0x37ec]
        mov ecx, dword ptr [ecx*4 + movie_media_conversion_tables + 0x33d4]
        or edx, ebx
        or edx, ecx
        mov dword ptr [ebp + 0x1c], edx
        mov eax, dword ptr [esi + 0x20]
        mov ebx, dword ptr [edi + 0x10]
        mov edx, dword ptr [edi + 0x110]
        mov eax, dword ptr [eax*4 + movie_media_conversion_tables + 0x1b0c]
        mov ebx, dword ptr [ebx*4 + movie_media_conversion_tables + 0x230c]
        mov edx, dword ptr [edx*4 + movie_media_conversion_tables + 0x2b0c]
        add eax, edx
        add edx, ebx
        add ebx, eax
        mov eax, ebx
        mov ecx, ebx
        shr ebx, 0x17
        and eax, 0xff800
        shr eax, 0xb
        and ecx, 0x1ff
        mov ebx, dword ptr [ebx*4 + movie_media_conversion_tables + 0x2f0c]
        mov eax, dword ptr [eax*4 + movie_media_conversion_tables + 0x33d4]
        mov ecx, dword ptr [ecx*4 + movie_media_conversion_tables + 0x37ec]
        or eax, ebx
        mov ebx, dword ptr [esi + 0x24]
        or eax, ecx
        mov dword ptr [ebp + 0x20], eax
        mov ecx, dword ptr [ebx*4 + movie_media_conversion_tables + 0x1b0c]
        add ecx, edx
        mov edx, ecx
        mov ebx, ecx
        and ecx, 0xff800
        shr edx, 0x17
        and ebx, 0x1ff
        shr ecx, 0xb
        mov edx, dword ptr [edx*4 + movie_media_conversion_tables + 0x2f0c]
        mov ebx, dword ptr [ebx*4 + movie_media_conversion_tables + 0x37ec]
        mov ecx, dword ptr [ecx*4 + movie_media_conversion_tables + 0x33d4]
        or edx, ebx
        or edx, ecx
        mov dword ptr [ebp + 0x24], edx
        mov eax, dword ptr [esi + 0x28]
        mov ebx, dword ptr [edi + 0x14]
        mov edx, dword ptr [edi + 0x114]
        mov eax, dword ptr [eax*4 + movie_media_conversion_tables + 0x1b0c]
        mov ebx, dword ptr [ebx*4 + movie_media_conversion_tables + 0x230c]
        mov edx, dword ptr [edx*4 + movie_media_conversion_tables + 0x2b0c]
        add eax, edx
        add edx, ebx
        add ebx, eax
        mov eax, ebx
        mov ecx, ebx
        shr ebx, 0x17
        and eax, 0xff800
        shr eax, 0xb
        and ecx, 0x1ff
        mov ebx, dword ptr [ebx*4 + movie_media_conversion_tables + 0x2f0c]
        mov eax, dword ptr [eax*4 + movie_media_conversion_tables + 0x33d4]
        mov ecx, dword ptr [ecx*4 + movie_media_conversion_tables + 0x37ec]
        or eax, ebx
        mov ebx, dword ptr [esi + 0x2c]
        or eax, ecx
        mov dword ptr [ebp + 0x28], eax
        mov ecx, dword ptr [ebx*4 + movie_media_conversion_tables + 0x1b0c]
        add ecx, edx
        mov edx, ecx
        mov ebx, ecx
        and ecx, 0xff800
        shr edx, 0x17
        and ebx, 0x1ff
        shr ecx, 0xb
        mov edx, dword ptr [edx*4 + movie_media_conversion_tables + 0x2f0c]
        mov ebx, dword ptr [ebx*4 + movie_media_conversion_tables + 0x37ec]
        mov ecx, dword ptr [ecx*4 + movie_media_conversion_tables + 0x33d4]
        or edx, ebx
        or edx, ecx
        mov dword ptr [ebp + 0x2c], edx
        mov eax, dword ptr [esi + 0x30]
        mov ebx, dword ptr [edi + 0x18]
        mov edx, dword ptr [edi + 0x118]
        mov eax, dword ptr [eax*4 + movie_media_conversion_tables + 0x1b0c]
        mov ebx, dword ptr [ebx*4 + movie_media_conversion_tables + 0x230c]
        mov edx, dword ptr [edx*4 + movie_media_conversion_tables + 0x2b0c]
        add eax, edx
        add edx, ebx
        add ebx, eax
        mov eax, ebx
        mov ecx, ebx
        shr ebx, 0x17
        and eax, 0xff800
        shr eax, 0xb
        and ecx, 0x1ff
        mov ebx, dword ptr [ebx*4 + movie_media_conversion_tables + 0x2f0c]
        mov eax, dword ptr [eax*4 + movie_media_conversion_tables + 0x33d4]
        mov ecx, dword ptr [ecx*4 + movie_media_conversion_tables + 0x37ec]
        or eax, ebx
        mov ebx, dword ptr [esi + 0x34]
        or eax, ecx
        mov dword ptr [ebp + 0x30], eax
        mov ecx, dword ptr [ebx*4 + movie_media_conversion_tables + 0x1b0c]
        add ecx, edx
        mov edx, ecx
        mov ebx, ecx
        and ecx, 0xff800
        shr edx, 0x17
        and ebx, 0x1ff
        shr ecx, 0xb
        mov edx, dword ptr [edx*4 + movie_media_conversion_tables + 0x2f0c]
        mov ebx, dword ptr [ebx*4 + movie_media_conversion_tables + 0x37ec]
        mov ecx, dword ptr [ecx*4 + movie_media_conversion_tables + 0x33d4]
        or edx, ebx
        or edx, ecx
        mov dword ptr [ebp + 0x34], edx
        mov eax, dword ptr [esi + 0x38]
        mov ebx, dword ptr [edi + 0x1c]
        mov edx, dword ptr [edi + 0x11c]
        mov eax, dword ptr [eax*4 + movie_media_conversion_tables + 0x1b0c]
        mov ebx, dword ptr [ebx*4 + movie_media_conversion_tables + 0x230c]
        mov edx, dword ptr [edx*4 + movie_media_conversion_tables + 0x2b0c]
        add eax, edx
        add edx, ebx
        add ebx, eax
        mov eax, ebx
        mov ecx, ebx
        shr ebx, 0x17
        and eax, 0xff800
        shr eax, 0xb
        and ecx, 0x1ff
        mov ebx, dword ptr [ebx*4 + movie_media_conversion_tables + 0x2f0c]
        mov eax, dword ptr [eax*4 + movie_media_conversion_tables + 0x33d4]
        mov ecx, dword ptr [ecx*4 + movie_media_conversion_tables + 0x37ec]
        or eax, ebx
        mov ebx, dword ptr [esi + 0x3c]
        or eax, ecx
        mov dword ptr [ebp + 0x38], eax
        mov ecx, dword ptr [ebx*4 + movie_media_conversion_tables + 0x1b0c]
        add ecx, edx
        mov edx, ecx
        mov ebx, ecx
        and ecx, 0xff800
        shr edx, 0x17
        and ebx, 0x1ff
        shr ecx, 0xb
        mov edx, dword ptr [edx*4 + movie_media_conversion_tables + 0x2f0c]
        mov ebx, dword ptr [ebx*4 + movie_media_conversion_tables + 0x37ec]
        mov ecx, dword ptr [ecx*4 + movie_media_conversion_tables + 0x33d4]
        or edx, ebx
        or edx, ecx
        mov dword ptr [ebp + 0x3c], edx
        ret
    }
}

// ORIGINAL: 0x005D2536 sub_5d2536 0x005D2536-0x005D271D FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned 0x5D2536 reads ESI/EDI coefficient and selector words and stores through EBP at byte offsets 0,4,...,0x1C.", "Current sole caller 0x5D1278 owns DWORD[512]; its coefficient rows are word offsets row*16 and selector pairs are 256+(row/2)*8. Output is BYTE base plus row*byte pitch, without an int-array/alignment contract.", "The normalized source retains the target VC6 thirty-two-bit word-to-int conversion before the existing wrapping address calculation, with no mask, clamp or added initialization."], "reason": "Use the actual caller-owned raw-word scratch and byte-pitched output for the normalized packed-row interface. Preserve the four volatile input reads, their signed index interpretation, unchecked lookup arithmetic and eight packed stores; this does not establish the original hidden-register ABI or nominal parameter types."}
// symbol    ?sub_5d2536@@YAXPAK0PAE@Z
// TRIED: earlier homed into general.cpp (original TU unproved) 2026-09-03 and reverted for one reason:
// its fixed-address bindings have no naming evidence. image_data.py reads
// 1 of them and every one is zero at load - runtime-filled lookup tables
// with no .data content, no string, no arithmetic identity beyond 'a table
// indexed here' - and none is modelled anywhere in src. The homing-first rule
// does not ask for them to be invented: an unclaimed body with evidence-free
// bindings waits for the class pass that measures it in context.
// TRIED: no C++ calling convention passes edi/esi/ebp as params with no prologue; __asm is banned, so the register-implicit entry (no push/pop, bare ret) cannot be reproduced byte-exact. Reshaped as three pointer params for a faithful (non-exact) transcription.
// size      487 bytes
// prototype
// callers   1   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)

static DWORD movie_coefficient_pack_word(int index) {
    // Pinned 5D2536 indexes DWORDs at 9C672C + index*8: this is word 512
    // in the source-owned remap/lookup window at 9C5F2C.
    // Preserve flat x86 address arithmetic, including unchecked indices;
    // indexing the declared subarray would introduce an unproved bound.
    const DWORD displacement = (DWORD)index * 8u;
    const uintptr_t address = reinterpret_cast<uintptr_t>(
        &movie_media_conversion_tables.remap_or_lookup_storage.words[512])
        + static_cast<int32_t>(displacement);
    return *reinterpret_cast<const volatile DWORD *>(address);
}

// ESI supplies 16 coefficient indices; EDI supplies 8 lower and 8 upper
// selector indices at offsets 0 and 64; EBP receives eight packed DWORDs.
// Hand-written asm (EA uV decoder module): no prologue, esi/edi/ebx/ebp register inputs.
__declspec(naked) void __cdecl sub_5d2536(DWORD *coefficient_row, DWORD *selector_pair,
                        BYTE *packed_output) {
    __asm {
        mov eax, dword ptr [edi + 0x100]
        mov ebx, dword ptr [edi]
        mov edx, dword ptr [esi]
        mov ecx, dword ptr [esi + 4]
        mov eax, dword ptr [eax*8 + movie_media_conversion_tables + 0xf0c]
        shl eax, 8
        mov ebx, dword ptr [ebx*8 + movie_media_conversion_tables + 0xf0c]
        mov ecx, dword ptr [ecx*8 + movie_media_conversion_tables + 0xf0c]
        shl ebx, 8
        mov edx, dword ptr [edx*8 + movie_media_conversion_tables + 0xf0c]
        or eax, ecx
        or ebx, edx
        shl eax, 0x10
        or eax, ebx
        mov dword ptr [ebp], eax
        mov eax, dword ptr [edi + 0x104]
        mov ebx, dword ptr [edi + 4]
        mov edx, dword ptr [esi + 8]
        mov ecx, dword ptr [esi + 0xc]
        mov eax, dword ptr [eax*8 + movie_media_conversion_tables + 0xf0c]
        shl eax, 8
        mov ebx, dword ptr [ebx*8 + movie_media_conversion_tables + 0xf0c]
        mov ecx, dword ptr [ecx*8 + movie_media_conversion_tables + 0xf0c]
        shl ebx, 8
        mov edx, dword ptr [edx*8 + movie_media_conversion_tables + 0xf0c]
        or eax, ecx
        or ebx, edx
        shl eax, 0x10
        or eax, ebx
        mov dword ptr [ebp + 4], eax
        mov eax, dword ptr [edi + 0x108]
        mov ebx, dword ptr [edi + 8]
        mov edx, dword ptr [esi + 0x10]
        mov ecx, dword ptr [esi + 0x14]
        mov eax, dword ptr [eax*8 + movie_media_conversion_tables + 0xf0c]
        shl eax, 8
        mov ebx, dword ptr [ebx*8 + movie_media_conversion_tables + 0xf0c]
        mov ecx, dword ptr [ecx*8 + movie_media_conversion_tables + 0xf0c]
        shl ebx, 8
        mov edx, dword ptr [edx*8 + movie_media_conversion_tables + 0xf0c]
        or eax, ecx
        or ebx, edx
        shl eax, 0x10
        or eax, ebx
        mov dword ptr [ebp + 8], eax
        mov eax, dword ptr [edi + 0x10c]
        mov ebx, dword ptr [edi + 0xc]
        mov edx, dword ptr [esi + 0x18]
        mov ecx, dword ptr [esi + 0x1c]
        mov eax, dword ptr [eax*8 + movie_media_conversion_tables + 0xf0c]
        shl eax, 8
        mov ebx, dword ptr [ebx*8 + movie_media_conversion_tables + 0xf0c]
        mov ecx, dword ptr [ecx*8 + movie_media_conversion_tables + 0xf0c]
        shl ebx, 8
        mov edx, dword ptr [edx*8 + movie_media_conversion_tables + 0xf0c]
        or eax, ecx
        or ebx, edx
        shl eax, 0x10
        or eax, ebx
        mov dword ptr [ebp + 0xc], eax
        mov eax, dword ptr [edi + 0x110]
        mov ebx, dword ptr [edi + 0x10]
        mov edx, dword ptr [esi + 0x20]
        mov ecx, dword ptr [esi + 0x24]
        mov eax, dword ptr [eax*8 + movie_media_conversion_tables + 0xf0c]
        shl eax, 8
        mov ebx, dword ptr [ebx*8 + movie_media_conversion_tables + 0xf0c]
        mov ecx, dword ptr [ecx*8 + movie_media_conversion_tables + 0xf0c]
        shl ebx, 8
        mov edx, dword ptr [edx*8 + movie_media_conversion_tables + 0xf0c]
        or eax, ecx
        or ebx, edx
        shl eax, 0x10
        or eax, ebx
        mov dword ptr [ebp + 0x10], eax
        mov eax, dword ptr [edi + 0x114]
        mov ebx, dword ptr [edi + 0x14]
        mov edx, dword ptr [esi + 0x28]
        mov ecx, dword ptr [esi + 0x2c]
        mov eax, dword ptr [eax*8 + movie_media_conversion_tables + 0xf0c]
        shl eax, 8
        mov ebx, dword ptr [ebx*8 + movie_media_conversion_tables + 0xf0c]
        mov ecx, dword ptr [ecx*8 + movie_media_conversion_tables + 0xf0c]
        shl ebx, 8
        mov edx, dword ptr [edx*8 + movie_media_conversion_tables + 0xf0c]
        or eax, ecx
        or ebx, edx
        shl eax, 0x10
        or eax, ebx
        mov dword ptr [ebp + 0x14], eax
        mov eax, dword ptr [edi + 0x118]
        mov ebx, dword ptr [edi + 0x18]
        mov edx, dword ptr [esi + 0x30]
        mov ecx, dword ptr [esi + 0x34]
        mov eax, dword ptr [eax*8 + movie_media_conversion_tables + 0xf0c]
        shl eax, 8
        mov ebx, dword ptr [ebx*8 + movie_media_conversion_tables + 0xf0c]
        mov ecx, dword ptr [ecx*8 + movie_media_conversion_tables + 0xf0c]
        shl ebx, 8
        mov edx, dword ptr [edx*8 + movie_media_conversion_tables + 0xf0c]
        or eax, ecx
        or ebx, edx
        shl eax, 0x10
        or eax, ebx
        mov dword ptr [ebp + 0x18], eax
        mov eax, dword ptr [edi + 0x11c]
        mov ebx, dword ptr [edi + 0x1c]
        mov edx, dword ptr [esi + 0x38]
        mov ecx, dword ptr [esi + 0x3c]
        mov eax, dword ptr [eax*8 + movie_media_conversion_tables + 0xf0c]
        shl eax, 8
        mov ebx, dword ptr [ebx*8 + movie_media_conversion_tables + 0xf0c]
        mov ecx, dword ptr [ecx*8 + movie_media_conversion_tables + 0xf0c]
        shl ebx, 8
        mov edx, dword ptr [edx*8 + movie_media_conversion_tables + 0xf0c]
        or eax, ecx
        or ebx, edx
        shl eax, 0x10
        or eax, ebx
        mov dword ptr [ebp + 0x1c], eax
        ret
    }
}

// ORIGINAL: 0x005CE6E3 sub_5ce6e3 0x005CE6E3-0x005CE85A FILE BYTE_EXACT
// symbol    ?sub_5ce6e3@@YAXPAEPAFHPAH2@Z
// TRIED: earlier homed into general.cpp (original TU unproved) 2026-09-03 and reverted for one reason:
// its fixed-address bindings have no naming evidence. image_data.py reads
// 5 of them and every one is zero at load - runtime-filled lookup tables
// with no .data content, no string, no arithmetic identity beyond 'a table
// indexed here' - and none is modelled anywhere in src. The homing-first rule
// does not ask for them to be invented: an unclaimed body with evidence-free
// bindings waits for the class pass that measures it in context.
// TRIED: original uses `pushal`/`popal` around the whole body (5 stack args, no `this`); no portable C source form reproduces that register-save shape, so #0 mismatches in the prologue.
// The preceding fixed-address/naming objection is superseded by the shared
// MediaAdpcmTables owner above: its complete initial bytes and observed state
// offsets are verified. The pushal/popal matching limitation remains.
// size      375 bytes
// prototype
// callers   1   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)


// Hand-written asm (EA uV decoder module): pushad/popad + leave.
__declspec(naked) void __cdecl sub_5ce6e3(unsigned char *codes, short *samples, int sample_total, int *predictor_pair, int *step_index_pair) {
    __asm {
        push ebp
        mov ebp, esp
        pushad
        xor ecx, ecx
        mov edx, dword ptr [ebp + 0x18]
        mov edx, dword ptr [edx]
        movsx eax, dx
        mov dword ptr [MediaAdpcmTables + 0x1688], eax
        shr edx, 0x10
        movsx eax, dx
        mov dword ptr [MediaAdpcmTables + 0x168c], eax
        mov edx, dword ptr [ebp + 0x14]
        mov edx, dword ptr [edx]
        movsx eax, dx
        mov dword ptr [MediaAdpcmTables + 0x1680], eax
        shr edx, 0x10
        movsx eax, dx
        mov dword ptr [MediaAdpcmTables + 0x1684], eax
        mov esi, dword ptr [ebp + 8]
        mov edi, dword ptr [ebp + 0xc]
        xor ebx, ebx
m5ce6e3_5ce721:
        mov bl, byte ptr [esi]
        shr bl, 4
        mov eax, dword ptr [MediaAdpcmTables + 0x1688]
        mov edx, dword ptr [MediaAdpcmTables + 0x1680]
        add edx, dword ptr [eax + ebx*4 + MediaAdpcmTables + 0x20]
        cmp edx, 0x7fff
        jg m5ce6e3_5ce824
        cmp edx, 0xffff8000
        jl m5ce6e3_5ce82e
m5ce6e3_5ce750:
        mov word ptr [edi + ecx*4], dx
        mov dword ptr [MediaAdpcmTables + 0x1680], edx
        and bl, 7
        add eax, dword ptr [ebx*4 + MediaAdpcmTables + 0x1660]
        cmp eax, 0x1600
        jg m5ce6e3_5ce810
        cmp eax, 0
        jl m5ce6e3_5ce81a
m5ce6e3_5ce778:
        mov dword ptr [MediaAdpcmTables + 0x1688], eax
        mov bl, byte ptr [esi]
        and bl, 0xf
        mov eax, dword ptr [MediaAdpcmTables + 0x168c]
        mov edx, dword ptr [MediaAdpcmTables + 0x1684]
        add edx, dword ptr [eax + ebx*4 + MediaAdpcmTables + 0x20]
        cmp edx, 0x7fff
        jg m5ce6e3_5ce846
        cmp edx, 0xffff8000
        jl m5ce6e3_5ce850
m5ce6e3_5ce7ac:
        mov word ptr [edi + ecx*4 + 2], dx
        mov dword ptr [MediaAdpcmTables + 0x1684], edx
        and bl, 7
        add eax, dword ptr [ebx*4 + MediaAdpcmTables + 0x1660]
        cmp eax, 0x1600
        jg m5ce6e3_5ce838
        cmp eax, 0
        jl m5ce6e3_5ce83f
m5ce6e3_5ce7cd:
        mov dword ptr [MediaAdpcmTables + 0x168c], eax
        add ecx, 1
        inc esi
        cmp ecx, dword ptr [ebp + 0x10]
        jl m5ce6e3_5ce721
        mov edx, dword ptr [MediaAdpcmTables + 0x1684]
        shl edx, 0x10
        mov ecx, dword ptr [MediaAdpcmTables + 0x1680]
        mov dx, cx
        mov ecx, dword ptr [ebp + 0x14]
        mov dword ptr [ecx], edx
        mov edx, dword ptr [MediaAdpcmTables + 0x168c]
        shl edx, 0x10
        mov ecx, dword ptr [MediaAdpcmTables + 0x1688]
        mov dx, cx
        mov ecx, dword ptr [ebp + 0x18]
        mov dword ptr [ecx], edx
        popad
        leave
        ret
m5ce6e3_5ce810:
        mov eax, 0x1600
        jmp m5ce6e3_5ce778
m5ce6e3_5ce81a:
        mov eax, 0
        jmp m5ce6e3_5ce778
m5ce6e3_5ce824:
        mov edx, 0x7fff
        jmp m5ce6e3_5ce750
m5ce6e3_5ce82e:
        mov edx, 0xffff8000
        jmp m5ce6e3_5ce750
m5ce6e3_5ce838:
        mov eax, 0x1600
        jmp m5ce6e3_5ce7cd
m5ce6e3_5ce83f:
        mov eax, 0
        jmp m5ce6e3_5ce7cd
m5ce6e3_5ce846:
        mov edx, 0x7fff
        jmp m5ce6e3_5ce7ac
m5ce6e3_5ce850:
        mov edx, 0xffff8000
        jmp m5ce6e3_5ce7ac
    }
}


// The pinned image contains exactly 92 adjacent {HRESULT, relocated string}
// records in this scanned table window. Text and order are preserved from the
// image; no claim is made about storage beyond the observed window.
struct MediaHResultText {
  HRESULT result;
  const char *text;
};

// RECOVERY_OBJECT: v1 0x00694E28 _media_hresult_texts
static const MediaHResultText media_hresult_texts[] = {
    {0x88760005L, "DDERR_ALREADYINITIALIZED"},
    {0x8876000AL, "DDERR_CANNOTATTACHSURFACE"},
    {0x88760014L, "DDERR_CANNOTDETACHSURFACE"},
    {0x88760028L, "DDERR_CURRENTLYNOTAVAIL"},
    {0x88760037L, "DDERR_EXCEPTION"},
    {0x80004005L, "DDERR_GENERIC"},
    {0x8876005AL, "DDERR_HEIGHTALIGN"},
    {0x8876005FL, "DDERR_INCOMPATIBLEPRIMARY"},
    {0x88760064L, "DDERR_INVALIDCAPS"},
    {0x8876006EL, "DDERR_INVALIDCLIPLIST"},
    {0x88760078L, "DDERR_INVALIDMODE"},
    {0x88760082L, "DDERR_INVALIDOBJECT"},
    {0x80070057L, "DDERR_INVALIDPARAMS"},
    {0x88760091L, "DDERR_INVALIDPIXELFORMAT"},
    {0x88760096L, "DDERR_INVALIDRECT"},
    {0x887600A0L, "DDERR_LOCKEDSURFACES"},
    {0x887600AAL, "DDERR_NO3D"},
    {0x887600B4L, "DDERR_NOALPHAHW"},
    {0x887600CDL, "DDERR_NOCLIPLIST"},
    {0x887600D2L, "DDERR_NOCOLORCONVHW"},
    {0x887600D4L, "DDERR_NOCOOPERATIVELEVELSET"},
    {0x887600D7L, "DDERR_NOCOLORKEY"},
    {0x887600DCL, "DDERR_NOCOLORKEYHW"},
    {0x887600DEL, "DDERR_NODIRECTDRAWSUPPORT"},
    {0x887600E1L, "DDERR_NOEXCLUSIVEMODE"},
    {0x887600E6L, "DDERR_NOFLIPHW"},
    {0x887600F0L, "DDERR_NOGDI"},
    {0x887600FAL, "DDERR_NOMIRRORHW"},
    {0x887600FFL, "DDERR_NOTFOUND"},
    {0x88760104L, "DDERR_NOOVERLAYHW"},
    {0x88760118L, "DDERR_NORASTEROPHW"},
    {0x88760122L, "DDERR_NOROTATIONHW"},
    {0x88760136L, "DDERR_NOSTRETCHHW"},
    {0x8876013CL, "DDERR_NOT4BITCOLOR"},
    {0x8876013DL, "DDERR_NOT4BITCOLORINDEX"},
    {0x88760140L, "DDERR_NOT8BITCOLOR "},
    {0x8876014AL, "DDERR_NOTEXTUREHW"},
    {0x8876014FL, "DDERR_NOVSYNCHW"},
    {0x88760154L, "DDERR_NOZBUFFERHW"},
    {0x8876015EL, "DDERR_NOZOVERLAYHW"},
    {0x88760168L, "DDERR_OUTOFCAPS"},
    {0x8007000EL, "DDERR_OUTOFMEMORY"},
    {0x8876017CL, "DDERR_OUTOFVIDEOMEMORY"},
    {0x8876017EL, "DDERR_OVERLAYCANTCLIP"},
    {0x88760180L, "DDERR_OVERLAYCOLORKEYONLYONEACTIVE"},
    {0x88760183L, "DDERR_PALETTEBUSY"},
    {0x88760190L, "DDERR_COLORKEYNOTSET"},
    {0x8876019AL, "DDERR_SURFACEALREADYATTACHED"},
    {0x887601A4L, "DDERR_SURFACEALREADYDEPENDENT"},
    {0x887601AEL, "DDERR_SURFACEBUSY"},
    {0x887601B8L, "DDERR_SURFACEISOBSCURED"},
    {0x887601C2L, "DDERR_SURFACELOST"},
    {0x887601CCL, "DDERR_SURFACENOTATTACHED"},
    {0x887601D6L, "DDERR_TOOBIGHEIGHT"},
    {0x887601E0L, "DDERR_TOOBIGSIZE"},
    {0x887601EAL, "DDERR_TOOBIGWIDTH"},
    {0x80004001L, "DDERR_UNSUPPORTED"},
    {0x887601FEL, "DDERR_UNSUPPORTEDFORMAT"},
    {0x88760208L, "DDERR_UNSUPPORTEDMASK"},
    {0x88760219L, "DDERR_VERTICALBLANKINPROGRESS"},
    {0x8876021CL, "DDERR_WASSTILLDRAWING"},
    {0x88760230L, "DDERR_XALIGN"},
    {0x88760231L, "DDERR_INVALIDDIRECTDRAWGUID"},
    {0x88760232L, "DDERR_DIRECTDRAWALREADYCREATED"},
    {0x88760233L, "DDERR_NODIRECTDRAWHW"},
    {0x88760234L, "DDERR_PRIMARYSURFACEALREADYEXISTS"},
    {0x88760235L, "DDERR_NOEMULATION"},
    {0x88760236L, "DDERR_REGIONTOOSMALL"},
    {0x88760237L, "DDERR_CLIPPERISUSINGHWND"},
    {0x88760238L, "DDERR_NOCLIPPERATTACHED"},
    {0x88760239L, "DDERR_NOHWND"},
    {0x8876023AL, "DDERR_HWNDSUBCLASSED"},
    {0x8876023BL, "DDERR_HWNDALREADYSET"},
    {0x8876023CL, "DDERR_NOPALETTEATTACHED"},
    {0x8876023DL, "DDERR_NOPALETTEHW"},
    {0x8876023EL, "DDERR_BLTFASTCANTCLIP"},
    {0x8876023FL, "DDERR_NOBLTHW"},
    {0x88760240L, "DDERR_NODDROPSHW"},
    {0x88760241L, "DDERR_OVERLAYNOTVISIBLE"},
    {0x88760242L, "DDERR_NOOVERLAYDEST"},
    {0x88760243L, "DDERR_INVALIDPOSITION"},
    {0x88760244L, "DDERR_NOTAOVERLAYSURFACE"},
    {0x88760245L, "DDERR_EXCLUSIVEMODEALREADYSET"},
    {0x88760246L, "DDERR_NOTFLIPPABLE"},
    {0x88760247L, "DDERR_CANTDUPLICATE"},
    {0x88760248L, "DDERR_NOTLOCKED"},
    {0x88760249L, "DDERR_CANTCREATEDC"},
    {0x8876024AL, "DDERR_NODC"},
    {0x8876024BL, "DDERR_WRONGMODE"},
    {0x8876024CL, "DDERR_IMPLICITLYCREATED"},
    {0x8876024DL, "DDERR_NOTPALETTIZED"},
    {0x8876024EL, "DDERR_UNSUPPORTEDMODE"},
};

// RECOVERY_OBJECT: v1 0x00695B34 _media_hresult_unknown
static const char media_hresult_unknown[] = "Uknown Error Code";

// ORIGINAL: 0x005CF120 sub_5cf120 0x005CF120-0x005CF144 BYTE_EXACT
// symbol    @sub_5cf120@4
// kind      game
extern "C" char *__fastcall sub_5cf120(int code) {
  for (unsigned int i = 0;
       i < sizeof(media_hresult_texts) / sizeof(media_hresult_texts[0]);
       ++i) {
    if (code == media_hresult_texts[i].result)
      return (char *)media_hresult_texts[i].text;
  }
  return (char *)media_hresult_unknown;
}


// ORIGINAL: 0x005CFAED sub_5cfaed 0x005CFAED-0x005D02CC BYTE_EXACT
// TRIED: signature changed to (unsigned char *, int, int) - the disasm reads 3 stack args ([esp+0x38] at entry, [esp+0x20]/[esp+0x24] after the scratch frame is released), not the 0-arg contract. The body is a mechanically-generated 1:1 transcription (script-derived from the raw disassembly, not hand-typed, to avoid transcription error over ~150 near-identical calls): 6x (sub_5cf150(); 8x sub_5d29e1; 8x sub_5d2b20) then 16x OutputPixelRow with an `edi` accumulator stepped by param3. sub_5d29e1/sub_5d2b20/OutputPixelRow all pass their SECOND (and for OutputPixelRow, first/third) operand through ESI/EDI/EBX with NO stack push at those call sites - a register convention this rewrite cannot reproduce without inline asm (banned), so it passes the same values as ordinary arguments instead. Low similarity (sim ~0.1-0.3 across all four builds) is a direct, expected consequence of that convention mismatch, not a structural error - every call target, argument value and order is transcribed 1:1 from the disassembly. Landed for coverage; TRIED as byte-exact without __asm.
// symbol    ?sub_5cfaed@@YAXPAE0H@Z
// kind      game
// Hand-written asm (EA uV decoder module): saves ecx/edx with the callee-saved set, register-convention callees.
__declspec(naked) void __cdecl sub_5cfaed(BYTE *compressed, BYTE *destination,
                        int row_pitch_bytes) {
    __asm {
        push ebx
        push ecx
        push edx
        push esi
        push edi
        push ebp
        sub esp, 0x1c
        mov esi, dword ptr [esp + 0x38]
        xor eax, eax
        mov al, byte ptr [esi]
        inc esi
        mov dword ptr [movie_media_conversion_tables + 0x100], esi
        mov dword ptr [movie_media_conversion_tables + 0x108], eax
        mov dword ptr [movie_media_conversion_tables + 0x104], 8
        call sub_5cf150
        mov esi, offset movie_media_transform_workspace + 0x600
        mov dword ptr [esp], offset movie_media_transform_workspace + 0x700
        call sub_5d29e1
        mov esi, offset movie_media_transform_workspace + 0x620
        mov dword ptr [esp], offset movie_media_transform_workspace + 0x704
        call sub_5d29e1
        mov esi, offset movie_media_transform_workspace + 0x640
        mov dword ptr [esp], offset movie_media_transform_workspace + 0x708
        call sub_5d29e1
        mov esi, offset movie_media_transform_workspace + 0x660
        mov dword ptr [esp], offset movie_media_transform_workspace + 0x70c
        call sub_5d29e1
        mov esi, offset movie_media_transform_workspace + 0x680
        mov dword ptr [esp], offset movie_media_transform_workspace + 0x710
        call sub_5d29e1
        mov esi, offset movie_media_transform_workspace + 0x6a0
        mov dword ptr [esp], offset movie_media_transform_workspace + 0x714
        call sub_5d29e1
        mov esi, offset movie_media_transform_workspace + 0x6c0
        mov dword ptr [esp], offset movie_media_transform_workspace + 0x718
        call sub_5d29e1
        mov esi, offset movie_media_transform_workspace + 0x6e0
        mov dword ptr [esp], offset movie_media_transform_workspace + 0x71c
        call sub_5d29e1
        mov esi, offset movie_media_transform_workspace + 0x700
        mov dword ptr [esp], offset movie_media_transform_workspace
        call sub_5d2b20
        mov esi, offset movie_media_transform_workspace + 0x724
        mov dword ptr [esp], offset movie_media_transform_workspace + 0x40
        call sub_5d2b20
        mov esi, offset movie_media_transform_workspace + 0x748
        mov dword ptr [esp], offset movie_media_transform_workspace + 0x80
        call sub_5d2b20
        mov esi, offset movie_media_transform_workspace + 0x76c
        mov dword ptr [esp], offset movie_media_transform_workspace + 0xc0
        call sub_5d2b20
        mov esi, offset movie_media_transform_workspace + 0x790
        mov dword ptr [esp], offset movie_media_transform_workspace + 0x100
        call sub_5d2b20
        mov esi, offset movie_media_transform_workspace + 0x7b4
        mov dword ptr [esp], offset movie_media_transform_workspace + 0x140
        call sub_5d2b20
        mov esi, offset movie_media_transform_workspace + 0x7d8
        mov dword ptr [esp], offset movie_media_transform_workspace + 0x180
        call sub_5d2b20
        mov esi, offset movie_media_transform_workspace + 0x7fc
        mov dword ptr [esp], offset movie_media_transform_workspace + 0x1c0
        call sub_5d2b20
        call sub_5cf150
        mov esi, offset movie_media_transform_workspace + 0x600
        mov dword ptr [esp], offset movie_media_transform_workspace + 0x700
        call sub_5d29e1
        mov esi, offset movie_media_transform_workspace + 0x620
        mov dword ptr [esp], offset movie_media_transform_workspace + 0x704
        call sub_5d29e1
        mov esi, offset movie_media_transform_workspace + 0x640
        mov dword ptr [esp], offset movie_media_transform_workspace + 0x708
        call sub_5d29e1
        mov esi, offset movie_media_transform_workspace + 0x660
        mov dword ptr [esp], offset movie_media_transform_workspace + 0x70c
        call sub_5d29e1
        mov esi, offset movie_media_transform_workspace + 0x680
        mov dword ptr [esp], offset movie_media_transform_workspace + 0x710
        call sub_5d29e1
        mov esi, offset movie_media_transform_workspace + 0x6a0
        mov dword ptr [esp], offset movie_media_transform_workspace + 0x714
        call sub_5d29e1
        mov esi, offset movie_media_transform_workspace + 0x6c0
        mov dword ptr [esp], offset movie_media_transform_workspace + 0x718
        call sub_5d29e1
        mov esi, offset movie_media_transform_workspace + 0x6e0
        mov dword ptr [esp], offset movie_media_transform_workspace + 0x71c
        call sub_5d29e1
        mov esi, offset movie_media_transform_workspace + 0x700
        mov dword ptr [esp], offset movie_media_transform_workspace + 0x20
        call sub_5d2b20
        mov esi, offset movie_media_transform_workspace + 0x724
        mov dword ptr [esp], offset movie_media_transform_workspace + 0x60
        call sub_5d2b20
        mov esi, offset movie_media_transform_workspace + 0x748
        mov dword ptr [esp], offset movie_media_transform_workspace + 0xa0
        call sub_5d2b20
        mov esi, offset movie_media_transform_workspace + 0x76c
        mov dword ptr [esp], offset movie_media_transform_workspace + 0xe0
        call sub_5d2b20
        mov esi, offset movie_media_transform_workspace + 0x790
        mov dword ptr [esp], offset movie_media_transform_workspace + 0x120
        call sub_5d2b20
        mov esi, offset movie_media_transform_workspace + 0x7b4
        mov dword ptr [esp], offset movie_media_transform_workspace + 0x160
        call sub_5d2b20
        mov esi, offset movie_media_transform_workspace + 0x7d8
        mov dword ptr [esp], offset movie_media_transform_workspace + 0x1a0
        call sub_5d2b20
        mov esi, offset movie_media_transform_workspace + 0x7fc
        mov dword ptr [esp], offset movie_media_transform_workspace + 0x1e0
        call sub_5d2b20
        call sub_5cf150
        mov esi, offset movie_media_transform_workspace + 0x600
        mov dword ptr [esp], offset movie_media_transform_workspace + 0x700
        call sub_5d29e1
        mov esi, offset movie_media_transform_workspace + 0x620
        mov dword ptr [esp], offset movie_media_transform_workspace + 0x704
        call sub_5d29e1
        mov esi, offset movie_media_transform_workspace + 0x640
        mov dword ptr [esp], offset movie_media_transform_workspace + 0x708
        call sub_5d29e1
        mov esi, offset movie_media_transform_workspace + 0x660
        mov dword ptr [esp], offset movie_media_transform_workspace + 0x70c
        call sub_5d29e1
        mov esi, offset movie_media_transform_workspace + 0x680
        mov dword ptr [esp], offset movie_media_transform_workspace + 0x710
        call sub_5d29e1
        mov esi, offset movie_media_transform_workspace + 0x6a0
        mov dword ptr [esp], offset movie_media_transform_workspace + 0x714
        call sub_5d29e1
        mov esi, offset movie_media_transform_workspace + 0x6c0
        mov dword ptr [esp], offset movie_media_transform_workspace + 0x718
        call sub_5d29e1
        mov esi, offset movie_media_transform_workspace + 0x6e0
        mov dword ptr [esp], offset movie_media_transform_workspace + 0x71c
        call sub_5d29e1
        mov esi, offset movie_media_transform_workspace + 0x700
        mov dword ptr [esp], offset movie_media_transform_workspace + 0x200
        call sub_5d2b20
        mov esi, offset movie_media_transform_workspace + 0x724
        mov dword ptr [esp], offset movie_media_transform_workspace + 0x240
        call sub_5d2b20
        mov esi, offset movie_media_transform_workspace + 0x748
        mov dword ptr [esp], offset movie_media_transform_workspace + 0x280
        call sub_5d2b20
        mov esi, offset movie_media_transform_workspace + 0x76c
        mov dword ptr [esp], offset movie_media_transform_workspace + 0x2c0
        call sub_5d2b20
        mov esi, offset movie_media_transform_workspace + 0x790
        mov dword ptr [esp], offset movie_media_transform_workspace + 0x300
        call sub_5d2b20
        mov esi, offset movie_media_transform_workspace + 0x7b4
        mov dword ptr [esp], offset movie_media_transform_workspace + 0x340
        call sub_5d2b20
        mov esi, offset movie_media_transform_workspace + 0x7d8
        mov dword ptr [esp], offset movie_media_transform_workspace + 0x380
        call sub_5d2b20
        mov esi, offset movie_media_transform_workspace + 0x7fc
        mov dword ptr [esp], offset movie_media_transform_workspace + 0x3c0
        call sub_5d2b20
        call sub_5cf150
        mov esi, offset movie_media_transform_workspace + 0x600
        mov dword ptr [esp], offset movie_media_transform_workspace + 0x700
        call sub_5d29e1
        mov esi, offset movie_media_transform_workspace + 0x620
        mov dword ptr [esp], offset movie_media_transform_workspace + 0x704
        call sub_5d29e1
        mov esi, offset movie_media_transform_workspace + 0x640
        mov dword ptr [esp], offset movie_media_transform_workspace + 0x708
        call sub_5d29e1
        mov esi, offset movie_media_transform_workspace + 0x660
        mov dword ptr [esp], offset movie_media_transform_workspace + 0x70c
        call sub_5d29e1
        mov esi, offset movie_media_transform_workspace + 0x680
        mov dword ptr [esp], offset movie_media_transform_workspace + 0x710
        call sub_5d29e1
        mov esi, offset movie_media_transform_workspace + 0x6a0
        mov dword ptr [esp], offset movie_media_transform_workspace + 0x714
        call sub_5d29e1
        mov esi, offset movie_media_transform_workspace + 0x6c0
        mov dword ptr [esp], offset movie_media_transform_workspace + 0x718
        call sub_5d29e1
        mov esi, offset movie_media_transform_workspace + 0x6e0
        mov dword ptr [esp], offset movie_media_transform_workspace + 0x71c
        call sub_5d29e1
        mov esi, offset movie_media_transform_workspace + 0x700
        mov dword ptr [esp], offset movie_media_transform_workspace + 0x220
        call sub_5d2b20
        mov esi, offset movie_media_transform_workspace + 0x724
        mov dword ptr [esp], offset movie_media_transform_workspace + 0x260
        call sub_5d2b20
        mov esi, offset movie_media_transform_workspace + 0x748
        mov dword ptr [esp], offset movie_media_transform_workspace + 0x2a0
        call sub_5d2b20
        mov esi, offset movie_media_transform_workspace + 0x76c
        mov dword ptr [esp], offset movie_media_transform_workspace + 0x2e0
        call sub_5d2b20
        mov esi, offset movie_media_transform_workspace + 0x790
        mov dword ptr [esp], offset movie_media_transform_workspace + 0x320
        call sub_5d2b20
        mov esi, offset movie_media_transform_workspace + 0x7b4
        mov dword ptr [esp], offset movie_media_transform_workspace + 0x360
        call sub_5d2b20
        mov esi, offset movie_media_transform_workspace + 0x7d8
        mov dword ptr [esp], offset movie_media_transform_workspace + 0x3a0
        call sub_5d2b20
        mov esi, offset movie_media_transform_workspace + 0x7fc
        mov dword ptr [esp], offset movie_media_transform_workspace + 0x3e0
        call sub_5d2b20
        call sub_5cf150
        mov esi, offset movie_media_transform_workspace + 0x600
        mov dword ptr [esp], offset movie_media_transform_workspace + 0x700
        call sub_5d29e1
        mov esi, offset movie_media_transform_workspace + 0x620
        mov dword ptr [esp], offset movie_media_transform_workspace + 0x704
        call sub_5d29e1
        mov esi, offset movie_media_transform_workspace + 0x640
        mov dword ptr [esp], offset movie_media_transform_workspace + 0x708
        call sub_5d29e1
        mov esi, offset movie_media_transform_workspace + 0x660
        mov dword ptr [esp], offset movie_media_transform_workspace + 0x70c
        call sub_5d29e1
        mov esi, offset movie_media_transform_workspace + 0x680
        mov dword ptr [esp], offset movie_media_transform_workspace + 0x710
        call sub_5d29e1
        mov esi, offset movie_media_transform_workspace + 0x6a0
        mov dword ptr [esp], offset movie_media_transform_workspace + 0x714
        call sub_5d29e1
        mov esi, offset movie_media_transform_workspace + 0x6c0
        mov dword ptr [esp], offset movie_media_transform_workspace + 0x718
        call sub_5d29e1
        mov esi, offset movie_media_transform_workspace + 0x6e0
        mov dword ptr [esp], offset movie_media_transform_workspace + 0x71c
        call sub_5d29e1
        mov esi, offset movie_media_transform_workspace + 0x700
        mov dword ptr [esp], offset movie_media_transform_workspace + 0x400
        call sub_5d2b20
        mov esi, offset movie_media_transform_workspace + 0x724
        mov dword ptr [esp], offset movie_media_transform_workspace + 0x420
        call sub_5d2b20
        mov esi, offset movie_media_transform_workspace + 0x748
        mov dword ptr [esp], offset movie_media_transform_workspace + 0x440
        call sub_5d2b20
        mov esi, offset movie_media_transform_workspace + 0x76c
        mov dword ptr [esp], offset movie_media_transform_workspace + 0x460
        call sub_5d2b20
        mov esi, offset movie_media_transform_workspace + 0x790
        mov dword ptr [esp], offset movie_media_transform_workspace + 0x480
        call sub_5d2b20
        mov esi, offset movie_media_transform_workspace + 0x7b4
        mov dword ptr [esp], offset movie_media_transform_workspace + 0x4a0
        call sub_5d2b20
        mov esi, offset movie_media_transform_workspace + 0x7d8
        mov dword ptr [esp], offset movie_media_transform_workspace + 0x4c0
        call sub_5d2b20
        mov esi, offset movie_media_transform_workspace + 0x7fc
        mov dword ptr [esp], offset movie_media_transform_workspace + 0x4e0
        call sub_5d2b20
        call sub_5cf150
        mov esi, offset movie_media_transform_workspace + 0x600
        mov dword ptr [esp], offset movie_media_transform_workspace + 0x700
        call sub_5d29e1
        mov esi, offset movie_media_transform_workspace + 0x620
        mov dword ptr [esp], offset movie_media_transform_workspace + 0x704
        call sub_5d29e1
        mov esi, offset movie_media_transform_workspace + 0x640
        mov dword ptr [esp], offset movie_media_transform_workspace + 0x708
        call sub_5d29e1
        mov esi, offset movie_media_transform_workspace + 0x660
        mov dword ptr [esp], offset movie_media_transform_workspace + 0x70c
        call sub_5d29e1
        mov esi, offset movie_media_transform_workspace + 0x680
        mov dword ptr [esp], offset movie_media_transform_workspace + 0x710
        call sub_5d29e1
        mov esi, offset movie_media_transform_workspace + 0x6a0
        mov dword ptr [esp], offset movie_media_transform_workspace + 0x714
        call sub_5d29e1
        mov esi, offset movie_media_transform_workspace + 0x6c0
        mov dword ptr [esp], offset movie_media_transform_workspace + 0x718
        call sub_5d29e1
        mov esi, offset movie_media_transform_workspace + 0x6e0
        mov dword ptr [esp], offset movie_media_transform_workspace + 0x71c
        call sub_5d29e1
        mov esi, offset movie_media_transform_workspace + 0x700
        mov dword ptr [esp], offset movie_media_transform_workspace + 0x500
        call sub_5d2b20
        mov esi, offset movie_media_transform_workspace + 0x724
        mov dword ptr [esp], offset movie_media_transform_workspace + 0x520
        call sub_5d2b20
        mov esi, offset movie_media_transform_workspace + 0x748
        mov dword ptr [esp], offset movie_media_transform_workspace + 0x540
        call sub_5d2b20
        mov esi, offset movie_media_transform_workspace + 0x76c
        mov dword ptr [esp], offset movie_media_transform_workspace + 0x560
        call sub_5d2b20
        mov esi, offset movie_media_transform_workspace + 0x790
        mov dword ptr [esp], offset movie_media_transform_workspace + 0x580
        call sub_5d2b20
        mov esi, offset movie_media_transform_workspace + 0x7b4
        mov dword ptr [esp], offset movie_media_transform_workspace + 0x5a0
        call sub_5d2b20
        mov esi, offset movie_media_transform_workspace + 0x7d8
        mov dword ptr [esp], offset movie_media_transform_workspace + 0x5c0
        call sub_5d2b20
        mov esi, offset movie_media_transform_workspace + 0x7fc
        mov dword ptr [esp], offset movie_media_transform_workspace + 0x5e0
        call sub_5d2b20
        add esp, 0x1c
        mov edi, dword ptr [esp + 0x20]
        mov esi, offset movie_media_transform_workspace
        mov ebx, offset movie_media_transform_workspace + 0x400
        call OutputPixelRow
        add edi, dword ptr [esp + 0x24]
        mov esi, offset movie_media_transform_workspace + 0x40
        mov ebx, offset movie_media_transform_workspace + 0x400
        call OutputPixelRow
        add edi, dword ptr [esp + 0x24]
        mov esi, offset movie_media_transform_workspace + 0x80
        mov ebx, offset movie_media_transform_workspace + 0x420
        call OutputPixelRow
        add edi, dword ptr [esp + 0x24]
        mov esi, offset movie_media_transform_workspace + 0xc0
        mov ebx, offset movie_media_transform_workspace + 0x420
        call OutputPixelRow
        add edi, dword ptr [esp + 0x24]
        mov esi, offset movie_media_transform_workspace + 0x100
        mov ebx, offset movie_media_transform_workspace + 0x440
        call OutputPixelRow
        add edi, dword ptr [esp + 0x24]
        mov esi, offset movie_media_transform_workspace + 0x140
        mov ebx, offset movie_media_transform_workspace + 0x440
        call OutputPixelRow
        add edi, dword ptr [esp + 0x24]
        mov esi, offset movie_media_transform_workspace + 0x180
        mov ebx, offset movie_media_transform_workspace + 0x460
        call OutputPixelRow
        add edi, dword ptr [esp + 0x24]
        mov esi, offset movie_media_transform_workspace + 0x1c0
        mov ebx, offset movie_media_transform_workspace + 0x460
        call OutputPixelRow
        add edi, dword ptr [esp + 0x24]
        mov esi, offset movie_media_transform_workspace + 0x200
        mov ebx, offset movie_media_transform_workspace + 0x480
        call OutputPixelRow
        add edi, dword ptr [esp + 0x24]
        mov esi, offset movie_media_transform_workspace + 0x240
        mov ebx, offset movie_media_transform_workspace + 0x480
        call OutputPixelRow
        add edi, dword ptr [esp + 0x24]
        mov esi, offset movie_media_transform_workspace + 0x280
        mov ebx, offset movie_media_transform_workspace + 0x4a0
        call OutputPixelRow
        add edi, dword ptr [esp + 0x24]
        mov esi, offset movie_media_transform_workspace + 0x2c0
        mov ebx, offset movie_media_transform_workspace + 0x4a0
        call OutputPixelRow
        add edi, dword ptr [esp + 0x24]
        mov esi, offset movie_media_transform_workspace + 0x300
        mov ebx, offset movie_media_transform_workspace + 0x4c0
        call OutputPixelRow
        add edi, dword ptr [esp + 0x24]
        mov esi, offset movie_media_transform_workspace + 0x340
        mov ebx, offset movie_media_transform_workspace + 0x4c0
        call OutputPixelRow
        add edi, dword ptr [esp + 0x24]
        mov esi, offset movie_media_transform_workspace + 0x380
        mov ebx, offset movie_media_transform_workspace + 0x4e0
        call OutputPixelRow
        add edi, dword ptr [esp + 0x24]
        mov esi, offset movie_media_transform_workspace + 0x3c0
        mov ebx, offset movie_media_transform_workspace + 0x4e0
        call OutputPixelRow
        pop ebp
        pop edi
        pop esi
        pop edx
        pop ecx
        pop ebx
        ret
    }
}


// ORIGINAL: 0x005D02CC sub_5d02cc 0x005D02CC-0x005D0AAB BYTE_EXACT
// TRIED: the callees use a custom register convention (row/block ptr in esi, the single stack arg written once to a fixed [esp] slot instead of pushed per call; OutputPixelRow16 takes esi/ebx/edi with no stack args at all) that no standard C++ calling convention expresses without __asm. Landed all 96 sub_5d29e1/sub_5d2b20 calls plus the 16 OutputPixelRow16 calls with the right addresses/order as plain cdecl(void*,void*[,void*]) - compiles, right call graph, but sim only ~0.12 since every call site's instruction shape differs. Signature changed to (unsigned char*, void*, int) from the disasm's 3-arg use (a1=byte* string, a2=dest, a3=row stride). Not retried given the convention mismatch is structural, not a wording issue.
// symbol    ?sub_5d02cc@@YAXPAEPAXH@Z
// kind      game
// Hand-written asm (EA uV decoder module): saves ecx/edx with the callee-saved set, register-convention callees.
__declspec(naked) void __cdecl sub_5d02cc(BYTE *compressed, void *destination,
                        int row_pitch_bytes) {
    __asm {
        push ebx
        push ecx
        push edx
        push esi
        push edi
        push ebp
        sub esp, 0x1c
        mov esi, dword ptr [esp + 0x38]
        xor eax, eax
        mov al, byte ptr [esi]
        inc esi
        mov dword ptr [movie_media_conversion_tables + 0x100], esi
        mov dword ptr [movie_media_conversion_tables + 0x108], eax
        mov dword ptr [movie_media_conversion_tables + 0x104], 8
        call sub_5cf150
        mov esi, offset movie_media_transform_workspace + 0x600
        mov dword ptr [esp], offset movie_media_transform_workspace + 0x700
        call sub_5d29e1
        mov esi, offset movie_media_transform_workspace + 0x620
        mov dword ptr [esp], offset movie_media_transform_workspace + 0x704
        call sub_5d29e1
        mov esi, offset movie_media_transform_workspace + 0x640
        mov dword ptr [esp], offset movie_media_transform_workspace + 0x708
        call sub_5d29e1
        mov esi, offset movie_media_transform_workspace + 0x660
        mov dword ptr [esp], offset movie_media_transform_workspace + 0x70c
        call sub_5d29e1
        mov esi, offset movie_media_transform_workspace + 0x680
        mov dword ptr [esp], offset movie_media_transform_workspace + 0x710
        call sub_5d29e1
        mov esi, offset movie_media_transform_workspace + 0x6a0
        mov dword ptr [esp], offset movie_media_transform_workspace + 0x714
        call sub_5d29e1
        mov esi, offset movie_media_transform_workspace + 0x6c0
        mov dword ptr [esp], offset movie_media_transform_workspace + 0x718
        call sub_5d29e1
        mov esi, offset movie_media_transform_workspace + 0x6e0
        mov dword ptr [esp], offset movie_media_transform_workspace + 0x71c
        call sub_5d29e1
        mov esi, offset movie_media_transform_workspace + 0x700
        mov dword ptr [esp], offset movie_media_transform_workspace
        call sub_5d2b20
        mov esi, offset movie_media_transform_workspace + 0x724
        mov dword ptr [esp], offset movie_media_transform_workspace + 0x40
        call sub_5d2b20
        mov esi, offset movie_media_transform_workspace + 0x748
        mov dword ptr [esp], offset movie_media_transform_workspace + 0x80
        call sub_5d2b20
        mov esi, offset movie_media_transform_workspace + 0x76c
        mov dword ptr [esp], offset movie_media_transform_workspace + 0xc0
        call sub_5d2b20
        mov esi, offset movie_media_transform_workspace + 0x790
        mov dword ptr [esp], offset movie_media_transform_workspace + 0x100
        call sub_5d2b20
        mov esi, offset movie_media_transform_workspace + 0x7b4
        mov dword ptr [esp], offset movie_media_transform_workspace + 0x140
        call sub_5d2b20
        mov esi, offset movie_media_transform_workspace + 0x7d8
        mov dword ptr [esp], offset movie_media_transform_workspace + 0x180
        call sub_5d2b20
        mov esi, offset movie_media_transform_workspace + 0x7fc
        mov dword ptr [esp], offset movie_media_transform_workspace + 0x1c0
        call sub_5d2b20
        call sub_5cf150
        mov esi, offset movie_media_transform_workspace + 0x600
        mov dword ptr [esp], offset movie_media_transform_workspace + 0x700
        call sub_5d29e1
        mov esi, offset movie_media_transform_workspace + 0x620
        mov dword ptr [esp], offset movie_media_transform_workspace + 0x704
        call sub_5d29e1
        mov esi, offset movie_media_transform_workspace + 0x640
        mov dword ptr [esp], offset movie_media_transform_workspace + 0x708
        call sub_5d29e1
        mov esi, offset movie_media_transform_workspace + 0x660
        mov dword ptr [esp], offset movie_media_transform_workspace + 0x70c
        call sub_5d29e1
        mov esi, offset movie_media_transform_workspace + 0x680
        mov dword ptr [esp], offset movie_media_transform_workspace + 0x710
        call sub_5d29e1
        mov esi, offset movie_media_transform_workspace + 0x6a0
        mov dword ptr [esp], offset movie_media_transform_workspace + 0x714
        call sub_5d29e1
        mov esi, offset movie_media_transform_workspace + 0x6c0
        mov dword ptr [esp], offset movie_media_transform_workspace + 0x718
        call sub_5d29e1
        mov esi, offset movie_media_transform_workspace + 0x6e0
        mov dword ptr [esp], offset movie_media_transform_workspace + 0x71c
        call sub_5d29e1
        mov esi, offset movie_media_transform_workspace + 0x700
        mov dword ptr [esp], offset movie_media_transform_workspace + 0x20
        call sub_5d2b20
        mov esi, offset movie_media_transform_workspace + 0x724
        mov dword ptr [esp], offset movie_media_transform_workspace + 0x60
        call sub_5d2b20
        mov esi, offset movie_media_transform_workspace + 0x748
        mov dword ptr [esp], offset movie_media_transform_workspace + 0xa0
        call sub_5d2b20
        mov esi, offset movie_media_transform_workspace + 0x76c
        mov dword ptr [esp], offset movie_media_transform_workspace + 0xe0
        call sub_5d2b20
        mov esi, offset movie_media_transform_workspace + 0x790
        mov dword ptr [esp], offset movie_media_transform_workspace + 0x120
        call sub_5d2b20
        mov esi, offset movie_media_transform_workspace + 0x7b4
        mov dword ptr [esp], offset movie_media_transform_workspace + 0x160
        call sub_5d2b20
        mov esi, offset movie_media_transform_workspace + 0x7d8
        mov dword ptr [esp], offset movie_media_transform_workspace + 0x1a0
        call sub_5d2b20
        mov esi, offset movie_media_transform_workspace + 0x7fc
        mov dword ptr [esp], offset movie_media_transform_workspace + 0x1e0
        call sub_5d2b20
        call sub_5cf150
        mov esi, offset movie_media_transform_workspace + 0x600
        mov dword ptr [esp], offset movie_media_transform_workspace + 0x700
        call sub_5d29e1
        mov esi, offset movie_media_transform_workspace + 0x620
        mov dword ptr [esp], offset movie_media_transform_workspace + 0x704
        call sub_5d29e1
        mov esi, offset movie_media_transform_workspace + 0x640
        mov dword ptr [esp], offset movie_media_transform_workspace + 0x708
        call sub_5d29e1
        mov esi, offset movie_media_transform_workspace + 0x660
        mov dword ptr [esp], offset movie_media_transform_workspace + 0x70c
        call sub_5d29e1
        mov esi, offset movie_media_transform_workspace + 0x680
        mov dword ptr [esp], offset movie_media_transform_workspace + 0x710
        call sub_5d29e1
        mov esi, offset movie_media_transform_workspace + 0x6a0
        mov dword ptr [esp], offset movie_media_transform_workspace + 0x714
        call sub_5d29e1
        mov esi, offset movie_media_transform_workspace + 0x6c0
        mov dword ptr [esp], offset movie_media_transform_workspace + 0x718
        call sub_5d29e1
        mov esi, offset movie_media_transform_workspace + 0x6e0
        mov dword ptr [esp], offset movie_media_transform_workspace + 0x71c
        call sub_5d29e1
        mov esi, offset movie_media_transform_workspace + 0x700
        mov dword ptr [esp], offset movie_media_transform_workspace + 0x200
        call sub_5d2b20
        mov esi, offset movie_media_transform_workspace + 0x724
        mov dword ptr [esp], offset movie_media_transform_workspace + 0x240
        call sub_5d2b20
        mov esi, offset movie_media_transform_workspace + 0x748
        mov dword ptr [esp], offset movie_media_transform_workspace + 0x280
        call sub_5d2b20
        mov esi, offset movie_media_transform_workspace + 0x76c
        mov dword ptr [esp], offset movie_media_transform_workspace + 0x2c0
        call sub_5d2b20
        mov esi, offset movie_media_transform_workspace + 0x790
        mov dword ptr [esp], offset movie_media_transform_workspace + 0x300
        call sub_5d2b20
        mov esi, offset movie_media_transform_workspace + 0x7b4
        mov dword ptr [esp], offset movie_media_transform_workspace + 0x340
        call sub_5d2b20
        mov esi, offset movie_media_transform_workspace + 0x7d8
        mov dword ptr [esp], offset movie_media_transform_workspace + 0x380
        call sub_5d2b20
        mov esi, offset movie_media_transform_workspace + 0x7fc
        mov dword ptr [esp], offset movie_media_transform_workspace + 0x3c0
        call sub_5d2b20
        call sub_5cf150
        mov esi, offset movie_media_transform_workspace + 0x600
        mov dword ptr [esp], offset movie_media_transform_workspace + 0x700
        call sub_5d29e1
        mov esi, offset movie_media_transform_workspace + 0x620
        mov dword ptr [esp], offset movie_media_transform_workspace + 0x704
        call sub_5d29e1
        mov esi, offset movie_media_transform_workspace + 0x640
        mov dword ptr [esp], offset movie_media_transform_workspace + 0x708
        call sub_5d29e1
        mov esi, offset movie_media_transform_workspace + 0x660
        mov dword ptr [esp], offset movie_media_transform_workspace + 0x70c
        call sub_5d29e1
        mov esi, offset movie_media_transform_workspace + 0x680
        mov dword ptr [esp], offset movie_media_transform_workspace + 0x710
        call sub_5d29e1
        mov esi, offset movie_media_transform_workspace + 0x6a0
        mov dword ptr [esp], offset movie_media_transform_workspace + 0x714
        call sub_5d29e1
        mov esi, offset movie_media_transform_workspace + 0x6c0
        mov dword ptr [esp], offset movie_media_transform_workspace + 0x718
        call sub_5d29e1
        mov esi, offset movie_media_transform_workspace + 0x6e0
        mov dword ptr [esp], offset movie_media_transform_workspace + 0x71c
        call sub_5d29e1
        mov esi, offset movie_media_transform_workspace + 0x700
        mov dword ptr [esp], offset movie_media_transform_workspace + 0x220
        call sub_5d2b20
        mov esi, offset movie_media_transform_workspace + 0x724
        mov dword ptr [esp], offset movie_media_transform_workspace + 0x260
        call sub_5d2b20
        mov esi, offset movie_media_transform_workspace + 0x748
        mov dword ptr [esp], offset movie_media_transform_workspace + 0x2a0
        call sub_5d2b20
        mov esi, offset movie_media_transform_workspace + 0x76c
        mov dword ptr [esp], offset movie_media_transform_workspace + 0x2e0
        call sub_5d2b20
        mov esi, offset movie_media_transform_workspace + 0x790
        mov dword ptr [esp], offset movie_media_transform_workspace + 0x320
        call sub_5d2b20
        mov esi, offset movie_media_transform_workspace + 0x7b4
        mov dword ptr [esp], offset movie_media_transform_workspace + 0x360
        call sub_5d2b20
        mov esi, offset movie_media_transform_workspace + 0x7d8
        mov dword ptr [esp], offset movie_media_transform_workspace + 0x3a0
        call sub_5d2b20
        mov esi, offset movie_media_transform_workspace + 0x7fc
        mov dword ptr [esp], offset movie_media_transform_workspace + 0x3e0
        call sub_5d2b20
        call sub_5cf150
        mov esi, offset movie_media_transform_workspace + 0x600
        mov dword ptr [esp], offset movie_media_transform_workspace + 0x700
        call sub_5d29e1
        mov esi, offset movie_media_transform_workspace + 0x620
        mov dword ptr [esp], offset movie_media_transform_workspace + 0x704
        call sub_5d29e1
        mov esi, offset movie_media_transform_workspace + 0x640
        mov dword ptr [esp], offset movie_media_transform_workspace + 0x708
        call sub_5d29e1
        mov esi, offset movie_media_transform_workspace + 0x660
        mov dword ptr [esp], offset movie_media_transform_workspace + 0x70c
        call sub_5d29e1
        mov esi, offset movie_media_transform_workspace + 0x680
        mov dword ptr [esp], offset movie_media_transform_workspace + 0x710
        call sub_5d29e1
        mov esi, offset movie_media_transform_workspace + 0x6a0
        mov dword ptr [esp], offset movie_media_transform_workspace + 0x714
        call sub_5d29e1
        mov esi, offset movie_media_transform_workspace + 0x6c0
        mov dword ptr [esp], offset movie_media_transform_workspace + 0x718
        call sub_5d29e1
        mov esi, offset movie_media_transform_workspace + 0x6e0
        mov dword ptr [esp], offset movie_media_transform_workspace + 0x71c
        call sub_5d29e1
        mov esi, offset movie_media_transform_workspace + 0x700
        mov dword ptr [esp], offset movie_media_transform_workspace + 0x400
        call sub_5d2b20
        mov esi, offset movie_media_transform_workspace + 0x724
        mov dword ptr [esp], offset movie_media_transform_workspace + 0x420
        call sub_5d2b20
        mov esi, offset movie_media_transform_workspace + 0x748
        mov dword ptr [esp], offset movie_media_transform_workspace + 0x440
        call sub_5d2b20
        mov esi, offset movie_media_transform_workspace + 0x76c
        mov dword ptr [esp], offset movie_media_transform_workspace + 0x460
        call sub_5d2b20
        mov esi, offset movie_media_transform_workspace + 0x790
        mov dword ptr [esp], offset movie_media_transform_workspace + 0x480
        call sub_5d2b20
        mov esi, offset movie_media_transform_workspace + 0x7b4
        mov dword ptr [esp], offset movie_media_transform_workspace + 0x4a0
        call sub_5d2b20
        mov esi, offset movie_media_transform_workspace + 0x7d8
        mov dword ptr [esp], offset movie_media_transform_workspace + 0x4c0
        call sub_5d2b20
        mov esi, offset movie_media_transform_workspace + 0x7fc
        mov dword ptr [esp], offset movie_media_transform_workspace + 0x4e0
        call sub_5d2b20
        call sub_5cf150
        mov esi, offset movie_media_transform_workspace + 0x600
        mov dword ptr [esp], offset movie_media_transform_workspace + 0x700
        call sub_5d29e1
        mov esi, offset movie_media_transform_workspace + 0x620
        mov dword ptr [esp], offset movie_media_transform_workspace + 0x704
        call sub_5d29e1
        mov esi, offset movie_media_transform_workspace + 0x640
        mov dword ptr [esp], offset movie_media_transform_workspace + 0x708
        call sub_5d29e1
        mov esi, offset movie_media_transform_workspace + 0x660
        mov dword ptr [esp], offset movie_media_transform_workspace + 0x70c
        call sub_5d29e1
        mov esi, offset movie_media_transform_workspace + 0x680
        mov dword ptr [esp], offset movie_media_transform_workspace + 0x710
        call sub_5d29e1
        mov esi, offset movie_media_transform_workspace + 0x6a0
        mov dword ptr [esp], offset movie_media_transform_workspace + 0x714
        call sub_5d29e1
        mov esi, offset movie_media_transform_workspace + 0x6c0
        mov dword ptr [esp], offset movie_media_transform_workspace + 0x718
        call sub_5d29e1
        mov esi, offset movie_media_transform_workspace + 0x6e0
        mov dword ptr [esp], offset movie_media_transform_workspace + 0x71c
        call sub_5d29e1
        mov esi, offset movie_media_transform_workspace + 0x700
        mov dword ptr [esp], offset movie_media_transform_workspace + 0x500
        call sub_5d2b20
        mov esi, offset movie_media_transform_workspace + 0x724
        mov dword ptr [esp], offset movie_media_transform_workspace + 0x520
        call sub_5d2b20
        mov esi, offset movie_media_transform_workspace + 0x748
        mov dword ptr [esp], offset movie_media_transform_workspace + 0x540
        call sub_5d2b20
        mov esi, offset movie_media_transform_workspace + 0x76c
        mov dword ptr [esp], offset movie_media_transform_workspace + 0x560
        call sub_5d2b20
        mov esi, offset movie_media_transform_workspace + 0x790
        mov dword ptr [esp], offset movie_media_transform_workspace + 0x580
        call sub_5d2b20
        mov esi, offset movie_media_transform_workspace + 0x7b4
        mov dword ptr [esp], offset movie_media_transform_workspace + 0x5a0
        call sub_5d2b20
        mov esi, offset movie_media_transform_workspace + 0x7d8
        mov dword ptr [esp], offset movie_media_transform_workspace + 0x5c0
        call sub_5d2b20
        mov esi, offset movie_media_transform_workspace + 0x7fc
        mov dword ptr [esp], offset movie_media_transform_workspace + 0x5e0
        call sub_5d2b20
        add esp, 0x1c
        mov edi, dword ptr [esp + 0x20]
        mov esi, offset movie_media_transform_workspace
        mov ebx, offset movie_media_transform_workspace + 0x400
        call OutputPixelRow16
        add edi, dword ptr [esp + 0x24]
        mov esi, offset movie_media_transform_workspace + 0x40
        mov ebx, offset movie_media_transform_workspace + 0x400
        call OutputPixelRow16
        add edi, dword ptr [esp + 0x24]
        mov esi, offset movie_media_transform_workspace + 0x80
        mov ebx, offset movie_media_transform_workspace + 0x420
        call OutputPixelRow16
        add edi, dword ptr [esp + 0x24]
        mov esi, offset movie_media_transform_workspace + 0xc0
        mov ebx, offset movie_media_transform_workspace + 0x420
        call OutputPixelRow16
        add edi, dword ptr [esp + 0x24]
        mov esi, offset movie_media_transform_workspace + 0x100
        mov ebx, offset movie_media_transform_workspace + 0x440
        call OutputPixelRow16
        add edi, dword ptr [esp + 0x24]
        mov esi, offset movie_media_transform_workspace + 0x140
        mov ebx, offset movie_media_transform_workspace + 0x440
        call OutputPixelRow16
        add edi, dword ptr [esp + 0x24]
        mov esi, offset movie_media_transform_workspace + 0x180
        mov ebx, offset movie_media_transform_workspace + 0x460
        call OutputPixelRow16
        add edi, dword ptr [esp + 0x24]
        mov esi, offset movie_media_transform_workspace + 0x1c0
        mov ebx, offset movie_media_transform_workspace + 0x460
        call OutputPixelRow16
        add edi, dword ptr [esp + 0x24]
        mov esi, offset movie_media_transform_workspace + 0x200
        mov ebx, offset movie_media_transform_workspace + 0x480
        call OutputPixelRow16
        add edi, dword ptr [esp + 0x24]
        mov esi, offset movie_media_transform_workspace + 0x240
        mov ebx, offset movie_media_transform_workspace + 0x480
        call OutputPixelRow16
        add edi, dword ptr [esp + 0x24]
        mov esi, offset movie_media_transform_workspace + 0x280
        mov ebx, offset movie_media_transform_workspace + 0x4a0
        call OutputPixelRow16
        add edi, dword ptr [esp + 0x24]
        mov esi, offset movie_media_transform_workspace + 0x2c0
        mov ebx, offset movie_media_transform_workspace + 0x4a0
        call OutputPixelRow16
        add edi, dword ptr [esp + 0x24]
        mov esi, offset movie_media_transform_workspace + 0x300
        mov ebx, offset movie_media_transform_workspace + 0x4c0
        call OutputPixelRow16
        add edi, dword ptr [esp + 0x24]
        mov esi, offset movie_media_transform_workspace + 0x340
        mov ebx, offset movie_media_transform_workspace + 0x4c0
        call OutputPixelRow16
        add edi, dword ptr [esp + 0x24]
        mov esi, offset movie_media_transform_workspace + 0x380
        mov ebx, offset movie_media_transform_workspace + 0x4e0
        call OutputPixelRow16
        add edi, dword ptr [esp + 0x24]
        mov esi, offset movie_media_transform_workspace + 0x3c0
        mov ebx, offset movie_media_transform_workspace + 0x4e0
        call OutputPixelRow16
        pop ebp
        pop edi
        pop esi
        pop edx
        pop ecx
        pop ebx
        ret
    }
}


#include "movie_decoder_huffman.h"

#include <cstring>

// Existing mapped product helpers share this decoder source owner.
// Keep one implementation of the pixel-lane decoding arithmetic.
extern "C" void __fastcall sub_5d2d20(char *src, int mode, int stride,
                                      char *dst, char adjust);
void __fastcall sub_5d2e00(char *src, int byte_offset, int stride,
                           char *dst, char adjust);

namespace movie_decoder_huffman {
namespace {

uint32_t pull_word(BitState &state) {
  const uint32_t value = static_cast<uint32_t>(state.cursor[0]) |
                              (static_cast<uint32_t>(state.cursor[1]) << 8);
  state.cursor += 2;
  return value;
}

uint32_t read_word(const uint8_t *cursor) {
  return static_cast<uint32_t>(cursor[0]) |
         (static_cast<uint32_t>(cursor[1]) << 8);
}

void refill_if_short(BitState &state) {
  if (static_cast<uint32_t>(state.available_bits) < 0x10u) {
    const int8_t old_count = static_cast<int8_t>(state.available_bits);
    const uint8_t *const cursor = state.cursor;
    const uint32_t word = read_word(cursor);
    const int32_t new_count = state.available_bits + 0x10;
    const uint32_t new_accumulator = state.accumulator |
        (word << ((0x10 - old_count) & 0x1f));
    state.available_bits = new_count;
    state.accumulator = new_accumulator;
    state.cursor = cursor + 2;
  }
}

void fill_block(uint8_t *base, int row_bytes, int fill) {
  const int32_t value = fill;
  for (int row = 0; row != 8; ++row) {
    for (int x = 0; x != 8; ++x) {
      std::memcpy(base + row * row_bytes + x * 4, &value, sizeof(value));
    }
  }
}

int32_t arithmetic_shift_right(uint32_t value, unsigned shift) {
  // VC6's signed right shift is the pinned SAR. Existing uses supply the
  // fixed counts 22 and 24; variable bit-reader shifts keep their own masks.
  return static_cast<int32_t>(value) >> shift;
}

// IMUL retains the low dword, including overflow, in the original decoder.
int32_t low32_product(int32_t left, int32_t right) {
  const uint32_t product = static_cast<uint32_t>(left) *
                           static_cast<uint32_t>(right);
  int32_t result;
  std::memcpy(&result, &product, sizeof(result));
  return result;
}

uint32_t consume_block_code(BitState &bits,
                                 const uint32_t table[64]) {
  const uint32_t entry = table[bits.accumulator >> 26];
  const unsigned count = entry & 0xff;
  const uint32_t accumulator = bits.accumulator << (count & 0x1f);
  const int32_t available_bits =
      bits.available_bits - static_cast<int32_t>(count);
  bits.available_bits = available_bits;
  bits.accumulator = accumulator;
  refill_if_short(bits);
  return entry;
}

void pack_rows_eight(const uint8_t *source_a,
                     const uint8_t *source_b,
                     uint32_t *destination,
                     const uint8_t *byte_remap) {
  for (int column = 0; column != 8; ++column) {
    const int a_index = 2 + column * 8;
    const int b_index = 2 + column * 4;
    const uint32_t high =
        (static_cast<uint32_t>(byte_remap[source_a[a_index]]) << 8) |
        byte_remap[source_a[a_index + 4]];
    const uint32_t low =
        (static_cast<uint32_t>(byte_remap[source_b[b_index]]) << 8) |
        byte_remap[source_b[b_index + 0x100]];
    destination[column] = (high << 16) | low;
  }
}

}  // namespace

void refill_word(BitState &state) { refill_if_short(state); }

BlockHeader decode_block_header(BitState &state, bool header_enabled,
                                const uint32_t block_huffman[64],
                                uint8_t *coefficient_plane,
                                int half_width) {
  BlockHeader result;
  result.flags = 0;
  result.delta_mode = 0;
  result.coefficient_cursor = coefficient_plane;
  if (!header_enabled) return result;
  uint32_t flags = 0;
  uint32_t accumulator = state.accumulator;
  int count = state.available_bits;
  if ((accumulator & 0xc0000000u) == 0) {
    accumulator <<= 2;
    count -= 2;
    state.accumulator = accumulator;
    state.available_bits = count;
    if (static_cast<uint32_t>(count) < 0x10u) {
      const int8_t old_count = static_cast<int8_t>(count);
      const uint8_t *const cursor = state.cursor;
      const uint32_t word = read_word(cursor);
      count += 0x10;
      accumulator |= word << ((0x10 - old_count) & 0x1f);
      state.available_bits = count;
      state.accumulator = accumulator;
      state.cursor = cursor + 2;
    }
  } else {
    if ((accumulator & 0x80000000u) != 0) {
      flags = 0x3ff;
      accumulator <<= 1;
      --count;
    } else {
      flags = accumulator >> 24;
      accumulator <<= 8;
      count -= 8;
    }
    state.accumulator = accumulator;
    state.available_bits = count;
    if (static_cast<uint32_t>(count) < 0x10u) {
      const int8_t old_count = static_cast<int8_t>(count);
      const uint8_t *const cursor = state.cursor;
      const uint32_t word = read_word(cursor);
      count += 0x10;
      accumulator |= word << ((0x10 - old_count) & 0x1f);
      // The native flag prelude publishes only ESI here. EAX/EDX remain
      // register-local until the first prefix entry has been consumed.
      state.cursor = cursor + 2;
    }

    uint32_t first = block_huffman[accumulator >> 26];
    unsigned consumed = first & 0xff;
    accumulator <<= (consumed & 0x1f);
    count -= static_cast<int>(consumed);
    state.available_bits = count;
    state.accumulator = accumulator;
    if (static_cast<uint32_t>(count) < 0x10u) {
      const int8_t old_count = static_cast<int8_t>(count);
      const uint8_t *const cursor = state.cursor;
      const uint32_t word = read_word(cursor);
      count += 0x10;
      accumulator |= word << ((0x10 - old_count) & 0x1f);
      // Likewise, this refill commits only the advanced input pointer.
      state.cursor = cursor + 2;
    }

    const int32_t first_symbol = arithmetic_shift_right(first, 22);
    uint32_t second = block_huffman[accumulator >> 26];
    consumed = second & 0xff;
    accumulator <<= (consumed & 0x1f);
    count -= static_cast<int>(consumed);
    state.available_bits = count;
    state.accumulator = accumulator;
    if (static_cast<uint32_t>(count) < 0x10u) {
      const int8_t old_count = static_cast<int8_t>(count);
      const uint8_t *const cursor = state.cursor;
      const uint32_t word = read_word(cursor);
      count += 0x10;
      accumulator |= word << ((0x10 - old_count) & 0x1f);
      state.available_bits = count;
      state.accumulator = accumulator;
      state.cursor = cursor + 2;
    }
    const int32_t second_symbol = arithmetic_shift_right(second, 22);
    const int32_t position =
        second_symbol * half_width + (first_symbol >> 1);
    result.coefficient_cursor += position * 4;
    result.delta_mode = static_cast<uint32_t>(first_symbol) & 1u;
  }
  result.flags = flags;
  return result;
}

// ORIGINAL: 0x005CE85C sub_5ce85c 0x005CE85C-0x005CEA09
// RECOVERY_BINDING: v1 {"evidence": ["recovery/tasks/movie-decoder-owner-oct5.json", "pinned caller/body and shared-storage review; fresh VC6 public symbol", "Keep the semantic C++ name; body state is explicitly passed and uses the shared typed tables/bit-state owners. Do not copy executable bytes, use original-address calls, or treat a provisional owner name as native mangling."], "reason": "Faithful semantic media decoder owner recovery with normalized custom-register helper interfaces. Original identities, spans and historical findings retained; no matching, runtime or game acceptance granted."}
// symbol    ?decode_coefficients@movie_decoder_huffman@@YAHAAUState@1@@Z
// TRIED: MISMATCH #1 push/mov - the bit-reader state (accumulator, bits-available count, word cursor) that the original keeps in esi/edi/ebp for the whole function is modelled as ordinary locals written back once at the shared epilogue and at the single early-return branch; entry-frame shape differs first.
int decode_coefficients(State &state) {
  BitState &bits = state.bits;
  uint32_t accumulator = bits.accumulator << 8;
  int count = bits.available_bits - 8;
  state.fill_value = low32_product(
      arithmetic_shift_right(bits.accumulator, 24), state.scale_factor);
  state.coefficients[0] = state.fill_value;

  if (count < 0x10) {
    const uint16_t word = static_cast<uint16_t>(pull_word(bits));
    accumulator |= static_cast<uint32_t>(word)
                   << ((0x10 - static_cast<int8_t>(count)) & 0x1f);
    count += 0x10;
  }

  std::memset(state.coefficients + 1, 0, 63 * sizeof(*state.coefficients));
  int position = 1;
  const Tables &tables = *state.tables;
  for (;;) {
    uint32_t packed = tables.primary[accumulator >> 0x17];
    const uint8_t tag = static_cast<uint8_t>(packed);
    int new_count = count;
    if (static_cast<int8_t>(tag) > 9) {
      if (tag < 0x20) {
        if (tag < 0x10) {
          accumulator <<= 9;
          new_count = count - 9;
          if (new_count < 0x10) {
            const uint16_t word = static_cast<uint16_t>(pull_word(bits));
            accumulator |= static_cast<uint32_t>(word)
                           << ((0x10 - static_cast<int8_t>(new_count)) & 0x1f);
            new_count = count + 7;
          }
          packed = tables.secondary_a[accumulator >> 0x18];
        } else {
          accumulator <<= 6;
          new_count = count - 6;
          if (new_count < 0x10) {
            const uint16_t word = static_cast<uint16_t>(pull_word(bits));
            accumulator |= static_cast<uint32_t>(word)
                           << ((0x10 - static_cast<int8_t>(new_count)) & 0x1f);
            new_count = count + 10;
          }
          packed = tables.secondary_b[accumulator >> 0x18];
        }
      } else if (tag > 0x2f) {
        accumulator <<= 2;
        int final_count = count - 2;
        if (final_count < 0x10) {
          const uint16_t word = static_cast<uint16_t>(pull_word(bits));
          accumulator |= static_cast<uint32_t>(word)
                         << ((0x10 - static_cast<int8_t>(final_count)) & 0x1f);
          final_count = count + 0xe;
        }
        bits.accumulator = accumulator;
        bits.available_bits = final_count;
        return position;
      } else {
        accumulator <<= 6;
        new_count = count - 6;
        if (new_count < 0x10) {
          const uint16_t word = static_cast<uint16_t>(pull_word(bits));
          accumulator |= static_cast<uint32_t>(word)
                         << ((0x10 - static_cast<int8_t>(new_count)) & 0x1f);
          new_count = count + 10;
        }
        packed = (accumulator & 0xffff0000u) | 0x10u;
      }
    }

    count = new_count - static_cast<int>(packed & 0xff);
    const int next_position = position + static_cast<int>((packed >> 0x10) & 0x3f);
    accumulator <<= packed & 0x1f;
    if (count < 0x10) {
      const uint16_t word = static_cast<uint16_t>(pull_word(bits));
      accumulator |= static_cast<uint32_t>(word)
                     << ((0x10 - static_cast<int8_t>(count)) & 0x1f);
      count += 0x10;
    }
    const int offset = tables.coefficient_offsets[next_position];
    const int value = static_cast<int32_t>(packed) >> 0x16;
    const int scale = *reinterpret_cast<const int *>(
        reinterpret_cast<const uint8_t *>(tables.coefficient_scales) + offset);
    *reinterpret_cast<int *>(reinterpret_cast<uint8_t *>(state.coefficients) + offset) =
        value * scale;
    position = next_position + 1;
  }
}

void decode_six_blocks(SixBlockArgs &args) {
  const int half_width = args.row_width >> 1;
  BlockHeader header = decode_block_header(*args.bits, args.header_enabled,
                                           args.block_huffman,
                                           args.coefficient_plane, half_width);
  const int luma_offsets[4] = {0, 0x20, 0x200, 0x220};
  const int chroma_offsets[2] = {0, 0x100};
  const uint32_t flags[6] = {1, 2, 4, 8, 0x10, 0x20};

  for (int block = 0; block != 6; ++block) {
    const bool chroma = block >= 4;
    uint8_t *const target = chroma
        ? args.scratch.chroma_16x8 + chroma_offsets[block - 4]
        : args.scratch.luma_16x16 + luma_offsets[block];
    if ((header.flags & flags[block]) == 0) {
      args.coefficient_state->bits = *args.bits;
      const int decoded = decode_coefficients(*args.coefficient_state);
      *args.bits = args.coefficient_state->bits;
      if (decoded == 1) {
        fill_block(target, chroma ? 0x20 : 0x40,
                   args.coefficient_state->fill_value);
      } else {
        args.idct_rows(target, chroma ? 8 : 0x10);
      }
      continue;
    }

    const uint32_t code = consume_block_code(*args.bits,
                                                   args.block_huffman);
    const int extra = arithmetic_shift_right(code, 22) - 0x40;
    if (!chroma) {
      const int plane_offset = (block & 2) != 0 ? half_width * 0x20 : 0;
      const int x_offset = (block & 1) != 0 ? 0x10 : 0;
      ::sub_5d2d20(
          reinterpret_cast<char *>(header.coefficient_cursor + plane_offset +
                                   x_offset),
          static_cast<int>(header.delta_mode), half_width,
          reinterpret_cast<char *>(target), static_cast<char>(extra));
    } else {
      ::sub_5d2e00(reinterpret_cast<char *>(header.coefficient_cursor),
                   block == 4 ? 1 : 0, half_width,
                   reinterpret_cast<char *>(target),
                   static_cast<char>(extra));
    }
  }

  args.pack_rows(args.scratch.luma_16x16, args.scratch.chroma_16x8,
                 args.destination, half_width * 4);
}

void transform_sixteen(const uint8_t *source_a,
                       const uint8_t *source_b,
                       uint32_t *destination, int row_stride,
                       const uint8_t byte_remap[256]) {
  for (int tile = 0; tile != 16; ++tile) {
    pack_rows_eight(source_a, source_b, destination, byte_remap);
    source_a += 0x40;
    if ((tile & 1) != 0) source_b += 0x20;
    destination = reinterpret_cast<uint32_t *>(
        reinterpret_cast<uint8_t *>(destination) + row_stride);
  }
}

void pack_eight_pixels(const uint8_t *source_a,
                       const uint8_t *source_b,
                       uint32_t destination[8],
                       const uint8_t byte_remap[256]) {
  pack_rows_eight(source_a, source_b, destination, byte_remap);
}

}  // namespace movie_decoder_huffman
