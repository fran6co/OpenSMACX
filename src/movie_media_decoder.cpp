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
MediaTransformRegisterContinuation
__cdecl sub_5cf150() {
    MediaCoefficientTableStorage &transform_constants =
        movie_media_coefficient_tables;
    MediaConversionTableStorage &conversion_tables =
        movie_media_conversion_tables;
    MediaTransformBitState &reader = conversion_tables.transform_bit_state;
    const BYTE *cursor = reader.cursor;
    DWORD accumulator = reader.accumulator;
    LONG available_bits = reader.available_bits;
    DWORD index_bits = 0x3f;

    // 5CF150's initial signed-byte DC term and refill, in the same register
    // widths as EBP/ESI/ECX. Its IMUL/SHR/ADC sequence is product >> 16.
    const int dc = static_cast<signed char>(static_cast<BYTE>(accumulator));
    const __int64 dc_product = static_cast<__int64>(dc) *
        static_cast<__int64>(movie_media_signed32(
            conversion_tables.block_scale_entries[0]));
    movie_media_raw_workspace_store(0,
        movie_media_int_bits(movie_media_low32(dc_product)));
    accumulator >>= 8;
    {
        const BYTE byte = *cursor++;
        available_bits = movie_media_add_ecx(available_bits, -8);
        accumulator |= static_cast<DWORD>(byte) <<
            (static_cast<unsigned int>(available_bits) & 31);
        available_bits = movie_media_add_ecx(available_bits, 8);
    }

    for (;;) {
        const unsigned int code = accumulator & 7;
        const DWORD slot = movie_media_raw_word_at(
            transform_constants.words + 5, index_bits);
        switch (code) {
        case 0:
            movie_media_raw_workspace_store(slot, 0);
            break;
        case 1: {
            unsigned int run = (accumulator & 0xff) >> 3;
            // The key-1 run reads three distinct table windows: A8[i] is
            // the first destination, A4[i] is the next destination, and
            // A0[i] becomes the next live EBX-derived destination.
            DWORD run_slot = slot;
            do {
                movie_media_raw_workspace_store(run_slot, 0);
                movie_media_raw_workspace_store(movie_media_raw_word_at(
                    transform_constants.words + 4, index_bits), 0);
                run_slot = movie_media_raw_word_at(
                    transform_constants.words + 3, index_bits);
                index_bits -= 2;
                --run;
            } while (run != 0);
            ++index_bits;

            accumulator >>= 8;
            {
                const BYTE byte = *cursor++;
                available_bits = movie_media_add_ecx(available_bits, -8);
                accumulator |= static_cast<DWORD>(byte) <<
                    (static_cast<unsigned int>(available_bits) & 31);
                available_bits = movie_media_add_ecx(available_bits, 8);
            }
            --index_bits;
            if (movie_media_signed32(index_bits) <= 0)
                return movie_media_transform_return(
                    reader, cursor, accumulator, available_bits,
                    movie_media_signed32(index_bits));
            continue;
        }
        case 2:
            movie_media_raw_workspace_store(slot,
                movie_media_raw_word_at(
                    conversion_tables.block_scale_entries, slot));
            break;
        case 3:
        case 7: {
            if ((accumulator & 0xfc) == 0xfc) {
                accumulator >>= 8;
                const BYTE byte = *cursor++;
                available_bits = movie_media_add_ecx(available_bits, -8);
                accumulator |= static_cast<DWORD>(byte) <<
                    (static_cast<unsigned int>(available_bits) & 31);
                available_bits = movie_media_add_ecx(available_bits, 8);
                const int amplitude = static_cast<signed char>(
                    static_cast<BYTE>(accumulator));
                const __int64 product = static_cast<__int64>(amplitude) *
                    static_cast<__int64>(movie_media_signed32(
                        movie_media_raw_word_at(
                            conversion_tables.block_scale_entries, slot)));
                movie_media_raw_workspace_store(slot,
                    movie_media_int_bits(movie_media_low32(product)));
            } else {
                const int signed_code = static_cast<signed char>(
                    static_cast<BYTE>(accumulator & 0xfc));
                const int amplitude = signed_code * 0x4000;
                const __int64 product = static_cast<__int64>(amplitude) *
                    static_cast<__int64>(movie_media_signed32(
                        movie_media_raw_word_at(
                            conversion_tables.block_scale_entries, slot)));
                movie_media_raw_workspace_store(slot, movie_media_int_bits(
                    movie_media_low32_after_arithmetic_shift16(product)));
            }

            accumulator >>= 8;
            {
                const BYTE byte = *cursor++;
                available_bits = movie_media_add_ecx(available_bits, -8);
                accumulator |= static_cast<DWORD>(byte) <<
                    (static_cast<unsigned int>(available_bits) & 31);
                available_bits = movie_media_add_ecx(available_bits, 8);
            }
            --index_bits;
            if (movie_media_signed32(index_bits) <= 0)
                return movie_media_transform_return(
                    reader, cursor, accumulator, available_bits,
                    movie_media_signed32(index_bits));
            continue;
        }
        case 4: {
            movie_media_raw_workspace_store(slot, 0);
            const DWORD next_slot = movie_media_raw_word_at(
                transform_constants.words + 4, index_bits);
            --index_bits;
            movie_media_raw_workspace_store(next_slot, 0);
            break;
        }
        case 5: {
            // Key 5's target 226 path differs from key 1: clear A8[i], load
            // A4[i], and pre-decrement EDI before the first run iteration.
            movie_media_raw_workspace_store(slot, 0);
            DWORD run_slot = movie_media_raw_word_at(
                transform_constants.words + 4, index_bits);
            --index_bits;
            unsigned int run = (accumulator & 0xff) >> 3;
            do {
                movie_media_raw_workspace_store(run_slot, 0);
                movie_media_raw_workspace_store(movie_media_raw_word_at(
                    transform_constants.words + 4, index_bits), 0);
                run_slot = movie_media_raw_word_at(
                    transform_constants.words + 3, index_bits);
                index_bits -= 2;
                --run;
            } while (run != 0);
            ++index_bits;

            accumulator >>= 8;
            {
                const BYTE byte = *cursor++;
                available_bits = movie_media_add_ecx(available_bits, -8);
                accumulator |= static_cast<DWORD>(byte) <<
                    (static_cast<unsigned int>(available_bits) & 31);
                available_bits = movie_media_add_ecx(available_bits, 8);
            }
            --index_bits;
            if (movie_media_signed32(index_bits) <= 0)
                return movie_media_transform_return(
                    reader, cursor, accumulator, available_bits,
                    movie_media_signed32(index_bits));
            continue;
        }
        case 6:
            movie_media_raw_workspace_store(slot, movie_media_int_bits(
                movie_media_negate32(movie_media_signed32(
                    movie_media_raw_word_at(
                        conversion_tables.block_scale_entries, slot)))));
            break;
        }

        accumulator >>= 3;
        available_bits = movie_media_add_ecx(available_bits, -3);
        if (available_bits < 8) {
            const BYTE byte = *cursor++;
            accumulator |= static_cast<DWORD>(byte) <<
                (static_cast<unsigned int>(available_bits) & 31);
            available_bits = movie_media_add_ecx(available_bits, 8);
        }
        --index_bits;
        if (movie_media_signed32(index_bits) <= 0)
            return movie_media_transform_return(
                reader, cursor, accumulator, available_bits,
                movie_media_signed32(index_bits));
    }
}

// ORIGINAL: 0x005CF2F0 sub_5cf2f0 0x005CF2F0-0x005CF5C7
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
MediaTransformRegisterContinuation __cdecl sub_5cf2f0(
    const BYTE *bit_buffer, DWORD bit_position,
                        MediaDecoderContext &context) {
  // The initial receiver is typed because the normal owner/caller provides
  // this MediaDecoderContext. After the first 5CF150 call, however, ESI is
  // replaced with that helper's cursor and the original performs raw
  // dword accesses at ESI+100/104/108. Do not claim those derived addresses
  // are another live MediaDecoderContext allocation.
  BYTE *context_bytes = reinterpret_cast<BYTE *>(&context);
  const BYTE *const luma_delta_table = movie_media_luma_delta_entries();
  const BYTE *const chroma_delta_table = movie_media_chroma_delta_entries();

  // The native caller pushes workspace addresses 9C5000/5020/5200/5220 and
  // widths 0x40, then 9C5400/5500 and widths 0x20. 5CF150 reads no stack
  // arguments but does overwrite ESI/EBP/EDI from its real shared state. The
  // typed result exposes those live outputs rather than incorrectly retaining
  // this body's old bit buffer, context, and cursor.
  MediaTransformRegisterContinuation continuation;
  DWORD predictor_word;
  std::memcpy(&predictor_word, context_bytes + 0x100, sizeof(predictor_word));
  predictor_word += static_cast<DWORD>(movie_media_decode_predictor_delta(
      bit_buffer, bit_position, luma_delta_table, 0x19));
  std::memcpy(context_bytes + 0x100, &predictor_word, sizeof(predictor_word));
  continuation = sub_5cf150();
  bit_buffer = reinterpret_cast<const BYTE *>(
      static_cast<size_t>(continuation.ebp_accumulator));
  bit_position = static_cast<DWORD>(continuation.edi_coefficient_index);
  context_bytes = const_cast<BYTE *>(continuation.esi_cursor);

  std::memcpy(&predictor_word, context_bytes + 0x100, sizeof(predictor_word));
  predictor_word += static_cast<DWORD>(movie_media_decode_predictor_delta(
      bit_buffer, bit_position, luma_delta_table, 0x19));
  std::memcpy(context_bytes + 0x100, &predictor_word, sizeof(predictor_word));
  continuation = sub_5cf150();
  bit_buffer = reinterpret_cast<const BYTE *>(
      static_cast<size_t>(continuation.ebp_accumulator));
  bit_position = static_cast<DWORD>(continuation.edi_coefficient_index);
  context_bytes = const_cast<BYTE *>(continuation.esi_cursor);

  std::memcpy(&predictor_word, context_bytes + 0x100, sizeof(predictor_word));
  predictor_word += static_cast<DWORD>(movie_media_decode_predictor_delta(
      bit_buffer, bit_position, luma_delta_table, 0x19));
  std::memcpy(context_bytes + 0x100, &predictor_word, sizeof(predictor_word));
  continuation = sub_5cf150();
  bit_buffer = reinterpret_cast<const BYTE *>(
      static_cast<size_t>(continuation.ebp_accumulator));
  bit_position = static_cast<DWORD>(continuation.edi_coefficient_index);
  context_bytes = const_cast<BYTE *>(continuation.esi_cursor);

  std::memcpy(&predictor_word, context_bytes + 0x100, sizeof(predictor_word));
  predictor_word += static_cast<DWORD>(movie_media_decode_predictor_delta(
      bit_buffer, bit_position, luma_delta_table, 0x19));
  std::memcpy(context_bytes + 0x100, &predictor_word, sizeof(predictor_word));
  continuation = sub_5cf150();
  bit_buffer = reinterpret_cast<const BYTE *>(
      static_cast<size_t>(continuation.ebp_accumulator));
  bit_position = static_cast<DWORD>(continuation.edi_coefficient_index);
  context_bytes = const_cast<BYTE *>(continuation.esi_cursor);

  std::memcpy(&predictor_word, context_bytes + 0x104, sizeof(predictor_word));
  predictor_word += static_cast<DWORD>(movie_media_decode_predictor_delta(
      bit_buffer, bit_position, chroma_delta_table, 0x18));
  std::memcpy(context_bytes + 0x104, &predictor_word, sizeof(predictor_word));
  continuation = sub_5cf150();
  bit_buffer = reinterpret_cast<const BYTE *>(
      static_cast<size_t>(continuation.ebp_accumulator));
  bit_position = static_cast<DWORD>(continuation.edi_coefficient_index);
  context_bytes = const_cast<BYTE *>(continuation.esi_cursor);

  std::memcpy(&predictor_word, context_bytes + 0x108, sizeof(predictor_word));
  predictor_word += static_cast<DWORD>(movie_media_decode_predictor_delta(
      bit_buffer, bit_position, chroma_delta_table, 0x18));
  std::memcpy(context_bytes + 0x108, &predictor_word, sizeof(predictor_word));
  continuation = sub_5cf150();
  return continuation;
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
  int signed_value;
  std::memcpy(&signed_value, &value, sizeof(signed_value));
  if (signed_value >= 0) return signed_value >> 17;
  return -1 - (int)((~value) >> 17);
}

// Normalized typed body for the block decoder at 005D0D82. The original
// fragment receives pitch/output in ECX/EDX and predictor/context/input in
// EAX/ESI/EDI/EBP from its outlined 005D0AAB caller. Its stack arrays and
// arithmetic are expressed directly here; no executable-address call or
// separate table alias is used.
// ORIGINAL: 0x005D0D82 sub_5d0d82 0x005D0D82-0x005D1278
// RECOVERY_BINDING: v1 {"evidence": ["recovery/tasks/movie-decoder-owner-oct5.json", "pinned caller/body and shared-storage review; fresh VC6 public symbol", "Complete typed VLC/range decode plus 8 row and 8 column transforms. Reuse the shared 325-DWORD accessed coefficient window and mapped sub_5d29e1/sub_5d2c0a; retain the observed fill branch and caller-supplied output pitch."], "reason": "Faithful semantic media decoder owner recovery with normalized custom-register helper interfaces. Original identities, spans and historical findings retained; no matching, runtime or game acceptance granted."}
// symbol    ?sub_5d0d82@@YAXPAUMediaDecoderContext@@KPAEH@Z
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
void __cdecl sub_5d0d82(
    MediaDecoderContext *context, DWORD predictor_value, BYTE *destination,
    int row_pitch_bytes) {
  // Model the native 0x300-byte scratch as DWORD objects. The original writes
  // coefficient words and later addresses the same frame as bytes; retaining
  // a byte array plus DWORD type-punning would make the C++ translation rely
  // on alignment and effective-type undefined behavior.
  DWORD local_storage_words[0x300 / sizeof(DWORD)];
  DWORD *const coefficients = local_storage_words;
  const DWORD dc = predictor_value * context->block_transform_weights[0];
  coefficients[0] = dc;
  for (int i = 1; i != 64; ++i) coefficients[i] = 0;

  DWORD bit_position = context->bit_position_204;
  const BYTE *const input = context->compressed_input_200;
  const DWORD *const weights = context->block_transform_weights;
  BYTE run_position = 0x3f;
  for (;;) {
    const DWORD code_word = movie_media_read_bits32(input, bit_position) >> 15;
    if (code_word < 0x20) break;

    const DWORD entry = movie_media_coefficient_entry(code_word);
    const DWORD code_length = entry >> 16;
    const BYTE category = (BYTE)(entry >> 8);
    DWORD amplitude;
    bit_position += code_length;

    if (category == 0x41) break;
    if (category > 0x41) {
      const DWORD escape_window =
          movie_media_read_bits32(input, bit_position) >> 18;
      bit_position += 14;
      run_position = (BYTE)(run_position - category - 1);
      amplitude = escape_window & 0xff;
      if (amplitude == 0) {
        amplitude = movie_media_read_bits32(input, bit_position) >> 24;
        bit_position += 8;
        if (amplitude == 0x80) {
          amplitude = movie_media_read_bits32(input, bit_position) >> 24;
          bit_position += 8;
          amplitude -= 0x100;
        } else if ((int)amplitude > 0) {
          amplitude -= 0x100;
        }
      }
    } else {
      run_position = (BYTE)(run_position - category);
      amplitude = entry & 0xff;
      const unsigned sign_index = (16 - code_length) & 31;
      if (((code_word >> sign_index) & 1) == 0)
        amplitude = 0u - amplitude;
      ++bit_position;
    }

    const DWORD coefficient_index =
        movie_media_coefficient_tables.words[6 + run_position];
    coefficients[coefficient_index] = amplitude * weights[coefficient_index];
  }
  context->bit_position_204 = bit_position;

  if (run_position == 0x3f) {
    const int fill_value = movie_media_sar17(dc);
    for (int row = 0; row != 8; ++row) {
      int *const output = reinterpret_cast<int *>(
          destination + row * row_pitch_bytes);
      for (int column = 0; column != 8; ++column)
        output[column] = fill_value;
    }
    return;
  }

  // The native row transforms write a 9-DWORD-spaced intermediate matrix;
  // the column transforms consume its columns and advance output by pitch.
  for (int row = 0; row != 8; ++row) {
    BYTE *const local_storage_bytes =
        reinterpret_cast<BYTE *>(local_storage_words);
    sub_5d29e1(reinterpret_cast<int *>(local_storage_bytes + 0x0c + row * 0x20),
               reinterpret_cast<int *>(local_storage_bytes + 0x10c + row * 4));
  }
  for (int column = 0; column != 8; ++column) {
    BYTE *const local_storage_bytes =
        reinterpret_cast<BYTE *>(local_storage_words);
    sub_5d2c0a(reinterpret_cast<int *>(destination + column * row_pitch_bytes),
               reinterpret_cast<int *>(local_storage_bytes + 0x10c + column * 0x24));
  }
}

// Normalized source body for the six predictor/transform sequence beginning
// at 005D0AAB and falling through into 005D0D82 for the sixth block. The six
// destination offsets and row pitches are direct translations of the five
// ECX/EDX setup pairs plus the final fallthrough setup in the pinned image.
// ORIGINAL: 0x005D0AAB sub_5d0aab 0x005D0AAB-0x005D0D82
// RECOVERY_BINDING: v1 {"evidence": ["recovery/tasks/movie-decoder-owner-oct5.json", "pinned caller/body and shared-storage review; fresh VC6 public symbol", "Normalize the shared ESI/EDI/EBP/EAX state into the proven MediaRuntime and caller-owned 0x800-byte scratch. Perform six delta/predictor steps and invoke the typed 5D0D82 block body for each; preserve the sixth native fallthrough as an explicit source call edge."], "reason": "Faithful semantic media decoder owner recovery with normalized custom-register helper interfaces. Original identities, spans and historical findings retained; no matching, runtime or game acceptance granted."}
// symbol    ?sub_5d0aab@@YAXPAUMediaDecoderContext@@PAK@Z
// TRIED: no prologue/epilogue in the original (falls into 0x5D0D82, no ret) - edi/esi/ebp are register-passed by one caller sharing its frame; no standalone C parameter list can receive that, so byte-exactness is not reachable here. Modelled as static globals a companion routine would set instead.
void __cdecl sub_5d0aab(
    MediaDecoderContext *context, DWORD *scratch_words) {
  DWORD bit_position = context->bit_position_204;
  const BYTE *const input = context->compressed_input_200;
  const BYTE *const lookup_a = reinterpret_cast<const BYTE *>(
      movie_media_coefficient_tables.words) + 0x394;
  const BYTE *const lookup_b = reinterpret_cast<const BYTE *>(
      movie_media_coefficient_tables.words) + 0x414;
  const DWORD output_offsets[6] = {1, 9, 129, 137, 257, 321};

  for (int block = 0; block != 6; ++block) {
    DWORD *predictor;
    const BYTE *lookup;
    unsigned lookup_shift;
    if (block < 4) {
      predictor = &context->predictor_4f8;
      lookup = lookup_a;
      lookup_shift = 25;
    } else if (block == 4) {
      predictor = &context->predictor_4fc;
      lookup = lookup_b;
      lookup_shift = 24;
    } else {
      predictor = &context->predictor_500;
      lookup = lookup_b;
      lookup_shift = 24;
    }
    *predictor += movie_media_decode_predictor_delta(
        input, bit_position, lookup, lookup_shift);
    context->bit_position_204 = bit_position;
    sub_5d0d82(
        context, *predictor,
        reinterpret_cast<BYTE *>(scratch_words + output_offsets[block]),
        block < 4 ? 0x40 : 0x20);
    bit_position = context->bit_position_204;
  }
}

void movie_media_decode_six_coefficient_blocks(
    MediaDecoderContext *context, DWORD *scratch_words) {
  sub_5d0aab(context, scratch_words);
}

// ORIGINAL: 0x005D1466 sub_5d1466 0x005D1466-0x005D1654
// RECOVERY_BINDING: v1 {"evidence": ["recovery/tasks/movie-decoder-owner-oct5.json", "pinned caller/body and shared-storage review; fresh VC6 public symbol", "Store the advanced +0x204 cursor after composed decode, then execute sixteen 5D2037 output calls using the saved stack pitch/output arguments."], "reason": "Faithful semantic media decoder owner recovery with normalized custom-register helper interfaces. Original identities, spans and historical findings retained; no matching, runtime or game acceptance granted."}
// symbol    ?sub_5d1466@@YAXPAEHPAUMediaDecoderContext@@@Z
// TRIED: callee (sub_5d2037) takes its args in esi/edi/ebp with no stack push before `call` - no cdecl/fastcall signature the compiler can be told to use reproduces that register setup without __asm, which is barred. Landed with plausible field offsets and call count/order; prologue register save order and the per-call register loads do not match.
void __cdecl sub_5d1466(BYTE *base, int stride, MediaDecoderContext *context) {
  DWORD scratch_words[512];
  sub_5d0aab(context, scratch_words);
  for (int row = 0; row != 16; ++row) {
    int *const source = reinterpret_cast<int *>(
        reinterpret_cast<BYTE *>(scratch_words) + row * 0x40);
    int *const tables = reinterpret_cast<int *>(
        reinterpret_cast<BYTE *>(scratch_words) + 0x400 + (row / 2) * 0x20);
    sub_5d2037(source, tables,
               reinterpret_cast<unsigned int *>(base + row * stride));
  }
}

// ORIGINAL: 0x005D1842 sub_5d1842 0x005D1842-0x005D1A30
// RECOVERY_BINDING: v1 {"evidence": ["recovery/tasks/movie-decoder-owner-oct5.json", "pinned caller/body and shared-storage review; fresh VC6 public symbol", "Store the advanced +0x204 cursor after six-block decode, then execute sixteen 5CF852 row outputs."], "reason": "Faithful semantic media decoder owner recovery with normalized custom-register helper interfaces. Original identities, spans and historical findings retained; no matching, runtime or game acceptance granted."}
// symbol    ?sub_5d1842@@YAXPAEHPAUMediaDecoderContext@@@Z
// TRIED: callee (OutputPixelRow16) takes its args in esi/edi/ebp with no stack push before `call` - no cdecl/fastcall signature the compiler can be told to use reproduces that register setup without __asm, which is barred. Landed with plausible field offsets and call count/order; prologue register save order and the per-call register loads do not match.
void __cdecl sub_5d1842(BYTE *base, int stride, MediaDecoderContext *context) {
  DWORD scratch_words[512];
  sub_5d0aab(context, scratch_words);
  for (int row = 0; row != 16; ++row) {
    unsigned int *const component = reinterpret_cast<unsigned int *>(
        reinterpret_cast<BYTE *>(scratch_words) + row * 0x40);
    unsigned int *const chroma = reinterpret_cast<unsigned int *>(
        reinterpret_cast<BYTE *>(scratch_words) + 0x400 + (row / 2) * 0x20);
    OutputPixelRow16(reinterpret_cast<short *>(base + row * stride),
                     component, chroma);
  }
}

// ORIGINAL: 0x005D1654 sub_5d1654 0x005D1654-0x005D1842
// RECOVERY_BINDING: v1 {"evidence": ["recovery/tasks/movie-decoder-owner-oct5.json", "pinned caller/body and shared-storage review; fresh VC6 public symbol", "Store the advanced +0x204 cursor after six-block decode, then execute sixteen 5D1B88 output calls."], "reason": "Faithful semantic media decoder owner recovery with normalized custom-register helper interfaces. Original identities, spans and historical findings retained; no matching, runtime or game acceptance granted."}
// symbol    ?sub_5d1654@@YAXPAEHPAUMediaDecoderContext@@@Z
// TRIED: callee (sub_5d1b88) takes its args in esi/edi/ebp with no stack push before `call` - no cdecl/fastcall signature the compiler can be told to use reproduces that register setup without __asm, which is barred. Landed with plausible field offsets and call count/order; prologue register save order and the per-call register loads do not match.
void __cdecl sub_5d1654(BYTE *base, int stride, MediaDecoderContext *context) {
  DWORD scratch_words[512];
  sub_5d0aab(context, scratch_words);
  for (int row = 0; row != 16; ++row) {
    int *const source = reinterpret_cast<int *>(
        reinterpret_cast<BYTE *>(scratch_words) + row * 0x40);
    int *const tables = reinterpret_cast<int *>(
        reinterpret_cast<BYTE *>(scratch_words) + 0x400 + (row / 2) * 0x20);
    sub_5d1b88(reinterpret_cast<unsigned int *>(base + row * stride),
               source, tables);
  }
}

// ORIGINAL: 0x005D1A30 sub_5d1a30 0x005D1A30-0x005D1B88
// RECOVERY_BINDING: v1 {"evidence": ["recovery/tasks/movie-decoder-owner-oct5.json", "pinned caller/body and shared-storage review; fresh VC6 public symbol", "Store the advanced +0x204 cursor after six-block decode, then execute sixteen 5CF5C7 row outputs."], "reason": "Faithful semantic media decoder owner recovery with normalized custom-register helper interfaces. Original identities, spans and historical findings retained; no matching, runtime or game acceptance granted."}
// symbol    ?sub_5d1a30@@YAXPAEHPAUMediaDecoderContext@@@Z
// TRIED: OutputPixelRow (0x005CF5C7) is itself unrecovered and takes its row source/dest through esi/ebx/edi set by the caller before each call - a register-argument convention with no C++ expression that isn't __asm. Transcribed as 16 nullary calls in source order, matching call count/topology, not the per-call register setup.
void __cdecl sub_5d1a30(BYTE *base, int stride, MediaDecoderContext *context) {
  DWORD scratch_words[512];
  sub_5d0aab(context, scratch_words);
  for (int row = 0; row != 16; ++row) {
    unsigned int *const component = reinterpret_cast<unsigned int *>(
        reinterpret_cast<BYTE *>(scratch_words) + row * 0x40);
    unsigned int *const chroma = reinterpret_cast<unsigned int *>(
        reinterpret_cast<BYTE *>(scratch_words) + 0x400 + (row / 2) * 0x20);
    OutputPixelRow(reinterpret_cast<int *>(base + row * stride),
                   component, chroma);
  }
}

// ORIGINAL: 0x005D1278 sub_5d1278 0x005D1278-0x005D1466
// RECOVERY_BINDING: v1 {"evidence": ["recovery/tasks/movie-decoder-owner-oct5.json", "pinned caller/body and shared-storage review; fresh VC6 public symbol", "Receive the exact runtime+0x3F8 MediaDecoderContext, run the shared six-block 5D0AAB/5D0D82 decode, publish its advanced context+0x204 cursor, then execute sixteen mapped 5D2536 rows."], "reason": "Faithful semantic media decoder owner recovery with normalized custom-register helper interfaces. Original identities, spans and historical findings retained; no matching, runtime or game acceptance granted."}
// symbol    ?sub_5d1278@@YAXPAEHPAUMediaDecoderContext@@@Z
// TRIED: sub_5d2536/sub_5d0aab take no C-expressible params (register- implicit edi/esi/ebp with no prologue); the esi/edi/ebp setup before each bare call cannot be threaded through a normal argument list without __asm, so the calls below are faithful in COUNT and ORDER, not operands.
void __cdecl sub_5d1278(BYTE *base, int stride, MediaDecoderContext *context) {
  DWORD scratch_words[512];
  sub_5d0aab(context, scratch_words);
  for (int row = 0; row != 16; ++row) {
    int *const source = reinterpret_cast<int *>(
        reinterpret_cast<BYTE *>(scratch_words) + row * 0x40);
    int *const tables = reinterpret_cast<int *>(
        reinterpret_cast<BYTE *>(scratch_words) + 0x400 + (row / 2) * 0x20);
    sub_5d2536(source, tables,
               reinterpret_cast<int *>(base + row * stride));
  }
}

// Existing product owner in general.cpp; keep the one implementation.
extern int __cdecl sub_5d2761(unsigned int *source,
                              unsigned int *destination, int count);

static void *movie_decoder_allocate(size_t bytes) {
  return calloc(1, bytes);
}

static void movie_decoder_idct_rows(uint8_t *output, int stride_arg) {
  // Preserve the mapped block-builder edge. Its body calls the mapped row and
  // column owners; the row-loader continuity sentinel is a separate migration.
  sub_5cef8a(reinterpret_cast<BYTE *>(output), stride_arg);
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
  movie_decoder_huffman::BitState bits;
  bits.cursor = movie_media_decoder_tables.frame_bit_state.cursor;
  bits.accumulator = movie_media_decoder_tables.frame_bit_state.accumulator;
  bits.available_bits = movie_media_decoder_tables.frame_bit_state.available_bits;

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
  coefficient_state.bits = bits;
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
  args.bits = &bits;
  args.block_huffman = movie_media_decoder_tables.block_prefix_words;
  args.coefficient_state = &coefficient_state;
  args.idct_rows = movie_decoder_idct_rows;
  args.byte_remap = reinterpret_cast<const uint8_t *>(
      movie_media_conversion_tables.remap_or_lookup_storage.words);

  movie_decoder_huffman::decode_six_blocks(args);
  movie_media_decoder_tables.frame_bit_state.cursor = bits.cursor;
  movie_media_decoder_tables.frame_bit_state.accumulator = bits.accumulator;
  movie_media_decoder_tables.frame_bit_state.available_bits = bits.available_bits;
}

struct MovieDecoderFrameBridgeContext {
  movie_decoder_frame::BitState *bit_state;
};

static void movie_decoder_decode_six_blocks(
    void *context, uint8_t *coefficient_block,
    uint8_t *temporary_block, int row_width) {
  // 5D2E70 consumes the shared frame reader.  The frame body carries that
  // same live state between rows, so synchronize it around the mapped owner.
  MovieDecoderFrameBridgeContext *bridge =
      static_cast<MovieDecoderFrameBridgeContext *>(context);
  movie_media_decoder_tables.frame_bit_state.cursor =
      bridge->bit_state->cursor;
  movie_media_decoder_tables.frame_bit_state.accumulator =
      bridge->bit_state->accumulator;
  movie_media_decoder_tables.frame_bit_state.available_bits =
      bridge->bit_state->available_bits;
  sub_5d2e70(coefficient_block, temporary_block, row_width);
  bridge->bit_state->cursor =
      movie_media_decoder_tables.frame_bit_state.cursor;
  bridge->bit_state->accumulator =
      movie_media_decoder_tables.frame_bit_state.accumulator;
  bridge->bit_state->available_bits =
      movie_media_decoder_tables.frame_bit_state.available_bits;
}

static void movie_decoder_pack_row(const uint32_t *source,
                                   uint32_t *destination, int count) {
  sub_5d2761(const_cast<unsigned int *>(source),
             reinterpret_cast<unsigned int *>(destination), count);
}

// Replace the mapped 005D35B0 sentinel in movie_media.cpp with this body.
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

  movie_decoder_frame::BitState bits;
  bits.cursor = movie_media_decoder_tables.frame_bit_state.cursor;
  bits.accumulator = movie_media_decoder_tables.frame_bit_state.accumulator;
  bits.available_bits = movie_media_decoder_tables.frame_bit_state.available_bits;

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
  MovieDecoderFrameBridgeContext bridge_context;
  bridge_context.bit_state = &bits;

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
  args.bit_state = &bits;
  args.destination = output;
  args.output_pitch = output_pitch;
  args.record_kind = record_kind;
  args.codebook_records = movie_media_decoder_static_storage.codebook_records;
  args.transform_products =
      movie_media_decoder_static_storage.transform_product_words;
  args.tables = tables;
  args.decoder_context = &bridge_context;
  args.callbacks = callbacks;

  const int result = movie_decoder_frame::decode_frame(args);
  movie_media_decoder_tables.frame_bit_state.cursor = bits.cursor;
  movie_media_decoder_tables.frame_bit_state.accumulator = bits.accumulator;
  movie_media_decoder_tables.frame_bit_state.available_bits = bits.available_bits;
  return result;
}

// Source-owned 005CF5C7 and 005CF852 row converters. Their table slices are
// the three observed 128-DWORD banks at the front of the shared conversion
// owner; the source indices select entries using bits 17..23.
// ORIGINAL: 0x005CF5C7 OutputPixelRow 0x005CF5C7-0x005CF852
// RECOVERY_BINDING: v1 {"evidence": ["recovery/tasks/movie-decoder-owner-oct5.json", "pinned caller/body and shared-storage review; fresh VC6 public symbol", "Use the single MediaConversionTableStorage owner and its observed three 128-DWORD output banks; preserve the row's 8-pair lookup order. No independent table copy."], "reason": "Faithful semantic media decoder owner recovery with normalized custom-register helper interfaces. Original identities, spans and historical findings retained; no matching, runtime or game acceptance granted."}
// symbol    ?OutputPixelRow@@YAXAAUMediaConversionTableStorage@@PAHPBI2@Z
// TRIED: homed into general.cpp (its real TU) 2026-09-03 and reverted for one reason:
// TRIED: zero-arg contract head - the body has no prologue and reads esi/edi/ebx as pre-loaded pointers with no load from any fixed address, i.e. a register-passing convention no standard C signature reproduces without __asm; recast as three explicit pointer params (diverges at instr #1, a normal push-args prologue vs none)
void __cdecl OutputPixelRow(MediaConversionTableStorage &tables,
                            int *destination,
                            const uint32_t *component_source,
                            const uint32_t *chroma_source) {
  const DWORD *component = tables.packed_component_words;
  const DWORD *chroma_blue = tables.packed_component_words + 128;
  const DWORD *chroma_red = tables.packed_component_words + 256;
  for (int pair = 0; pair < 8; ++pair) {
    const DWORD second_chroma =
        chroma_blue[(chroma_source[pair] >> 17) & 0x7f] +
        chroma_red[(chroma_source[0x40 + pair] >> 17) & 0x7f];
    destination[pair * 2] =
        component[(component_source[pair * 2] >> 17) & 0x7f] +
        second_chroma;
    destination[pair * 2 + 1] =
        component[(component_source[pair * 2 + 1] >> 17) & 0x7f] +
        second_chroma;
  }
}

// ORIGINAL: 0x005CF852 OutputPixelRow16 0x005CF852-0x005CFAED
// RECOVERY_BINDING: v1 {"evidence": ["recovery/tasks/movie-decoder-owner-oct5.json", "pinned caller/body and shared-storage review; fresh VC6 public symbol", "Use the same shared three-bank owner and lookup schedule as the 32-bit row, with the observed 16-bit destination stores."], "reason": "Faithful semantic media decoder owner recovery with normalized custom-register helper interfaces. Original identities, spans and historical findings retained; no matching, runtime or game acceptance granted."}
// symbol    ?OutputPixelRow16@@YAXAAUMediaConversionTableStorage@@PAFPBI2@Z
// TRIED: homed into general.cpp (its real TU) 2026-09-03 and reverted for one reason:
// TRIED: zero-arg contract head - same register-in/register-out shape as 0x005CF5C7; recast as three explicit pointer params (diverges at instr #1, a normal push-args prologue vs none)
void __cdecl OutputPixelRow16(MediaConversionTableStorage &tables,
                              short *destination,
                              const uint32_t *component_source,
                              const uint32_t *chroma_source) {
  const DWORD *component = tables.packed_component_words;
  const DWORD *chroma_blue = tables.packed_component_words + 128;
  const DWORD *chroma_red = tables.packed_component_words + 256;
  for (int pair = 0; pair < 8; ++pair) {
    const DWORD second_chroma =
        chroma_blue[(chroma_source[pair] >> 17) & 0x7f] +
        chroma_red[(chroma_source[0x40 + pair] >> 17) & 0x7f];
    destination[pair * 2] = static_cast<short>(
        component[(component_source[pair * 2] >> 17) & 0x7f] +
        second_chroma);
    destination[pair * 2 + 1] = static_cast<short>(
        component[(component_source[pair * 2 + 1] >> 17) & 0x7f] +
        second_chroma);
  }
}

// 005CEB12 owns byte-lane lookup and packing. Keep 005CEA09 as its mapped
// sixteen-call wrapper with independently advanced source rows and pitch.
// ORIGINAL: 0x005CEA09 sub_5cea09 0x005CEA09-0x005CEB12
// RECOVERY_BINDING: v1 {"evidence": ["recovery/tasks/movie-decoder-owner-oct5.json", "pinned caller/body and shared-storage review; fresh VC6 public symbol", "Preserve the 16-call source-row and output-pitch progression while calling the mapped C++ 5CEB12 implementation for every row; do not inline a second lookup algorithm."], "reason": "Faithful semantic media decoder owner recovery with normalized custom-register helper interfaces. Original identities, spans and historical findings retained; no matching, runtime or game acceptance granted."}
// symbol    ?sub_5cea09@@YAXPBE0PAEH@Z
// TRIED: byte-exactness is not reachable in standard C++ here - the 16 calls to sub_5ceb12() take NO explicit arguments; the original threads state between them entirely through the callee-saved registers esi/edi/ebp (incremented by 0x40/ 0x20/a caller-stack value between calls), which is a register-level calling convention no C++ source can express without __asm (forbidden). Landed as 16 unrolled calls with `void` return (the disassembly never sets eax before `ret`, contradicting the `int` in the given contract head) and the 4 IDA-guessed int parameters, unused since nothing in valid C++ can route them into esi/edi/ebp for the callee.
void __cdecl sub_5cea09(const BYTE *component_rows,
                        const BYTE *chroma_rows,
                        BYTE *destination, int row_pitch_bytes) {
  for (int row = 0; row < 16; ++row) {
    sub_5ceb12(component_rows + row * 0x40,
               chroma_rows + (row / 2) * 0x20,
               reinterpret_cast<unsigned int *>(
                   destination + row * row_pitch_bytes));
  }
}

void __cdecl OutputPixelRow(int *destination,
                            unsigned int *component_source,
                            unsigned int *chroma_source) {
  OutputPixelRow(movie_media_conversion_tables, destination,
                 component_source, chroma_source);
}

void __cdecl OutputPixelRow16(short *destination,
                              unsigned int *component_source,
                              unsigned int *chroma_source) {
  OutputPixelRow16(movie_media_conversion_tables, destination,
                   component_source, chroma_source);
}

static int *movie_media_block_words(int index) {
  return movie_media_transform_workspace.block_and_intermediate_words + index;
}

static int *movie_media_row_words(int index) {
  return movie_media_transform_workspace.row_and_butterfly_words + index;
}

void movie_media_transform_block_pair(int phase) {
  const int group_base[6] = {0, 0, 128, 128, 256, 256};
  const int half_block[6] = {0, 8, 0, 8, 0, 8};

  sub_5cf150();
  for (int row = 0; row != 8; ++row) {
    sub_5d29e1(movie_media_row_words(row * 8),
               movie_media_row_words(64 + row));
  }
  for (int column = 0; column != 8; ++column) {
    sub_5d2b20(movie_media_block_words(group_base[phase] +
                                       half_block[phase] + column * 16),
               movie_media_row_words(64 + column * 9));
  }
}

void movie_media_write_macroblock_rows(unsigned char *destination,
                                       int row_pitch_bytes,
                                       int sixteen_bit) {
  for (int row = 0; row != 16; ++row) {
    int *component = movie_media_block_words(row * 16);
    int *chroma = movie_media_block_words(256 + (row / 2) * 8);
    unsigned char *output = destination + row * row_pitch_bytes;
    if (sixteen_bit) {
      OutputPixelRow16(movie_media_conversion_tables,
                       reinterpret_cast<short *>(output),
                       reinterpret_cast<const uint32_t *>(component),
                       reinterpret_cast<const uint32_t *>(chroma));
    } else {
      OutputPixelRow(movie_media_conversion_tables,
                     reinterpret_cast<int *>(output),
                     reinterpret_cast<const uint32_t *>(component),
                     reinterpret_cast<const uint32_t *>(chroma));
    }
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

static int *movie_media_idct_output_pointer() {
    int *result;
    std::memcpy(&result,
                &movie_media_idct_storage.current_output_pointer_bits,
                sizeof(result));
    return result;
}

static void movie_media_idct_set_output_pointer(int *value) {
    std::memcpy(&movie_media_idct_storage.current_output_pointer_bits,
                &value, sizeof(value));
}

static float movie_media_idct_float_constant(int index) {
    float value;
    std::memcpy(&value, &movie_media_idct_storage.constant_words[index],
                sizeof(value));
    return value;
}

static movie_decoder_idct::IdctConstants movie_media_idct_constants() {
    movie_decoder_idct::IdctConstants values;
    std::memcpy(&values.rotation_multiplier,
                &movie_media_idct_storage.constant_words[0], sizeof(LONG));
    std::memcpy(&values.first_difference_scale,
                &movie_media_idct_storage.constant_words[1], sizeof(float));
    std::memcpy(&values.second_difference_scale,
                &movie_media_idct_storage.constant_words[2], sizeof(float));
    std::memcpy(&values.cross_scale,
                &movie_media_idct_storage.constant_words[3], sizeof(float));
    std::memcpy(&values.bias, &movie_media_idct_storage.constant_words[4],
                sizeof(float));
    return values;
}



/*
// RECOVERY_ROLE: SCAFFOLD Current typed movie/media owner implementation; original receiver, shared storage, and call ordering retained. Native ABI/output and exceptional behavior remain explicit obligations.
// ORIGINAL: 0x005CEE80 sub_5cee80 0x005CEE80-0x005CEF8A
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
void __cdecl sub_5cee80(int *src) {
    int *dst;
    int sum_lo;
    int sum_hi;
    float diff_a;
    float diff_b;
    float mid;
    int rot;
    int out_a;
    int out_b;

    dst = movie_media_idct_output_pointer();
    sum_lo = src[3] + src[5];
    sum_hi = src[7] + src[1];
    diff_a = static_cast<float>(src[5] - src[3]);
    diff_b = static_cast<float>(src[1] - src[7]);
    mid = (diff_b + diff_a) * movie_media_idct_float_constant(3);
    rot = static_cast<int>((static_cast<__int64>(sum_hi - sum_lo) *
                            static_cast<__int64>(movie_media_idct_storage.constant_words[0])) >>
                           32) *
          2;
    // The image rounds these two sums to double inside the x87 stack and
    // fstp qword's them into the scratch slots; the INTEGER pipeline then
    // re-reads the slots' LOW dwords, so the bit pattern - not the numeric
    // value - is what flows into the eight writes.
    const double scratch_a = static_cast<double>(
        diff_a * movie_media_idct_float_constant(1) + mid +
        movie_media_idct_float_constant(4));
    const double scratch_b = static_cast<double>(
        (diff_b * movie_media_idct_float_constant(2) - mid) +
        movie_media_idct_float_constant(4));
    std::memcpy(&movie_media_idct_storage.butterfly_fp_scratch_words[0],
                &scratch_a, sizeof(scratch_a));
    std::memcpy(&movie_media_idct_storage.butterfly_fp_scratch_words[2],
                &scratch_b, sizeof(scratch_b));
    out_a = static_cast<int>(
        movie_media_idct_storage.butterfly_fp_scratch_words[0]);
    out_b = static_cast<int>(
        movie_media_idct_storage.butterfly_fp_scratch_words[2]);
    int sum_lo2;
    int diff_c;
    int rot2;
    int rot2x2;
    int sum_hi2;
    int diff_d;
    int diff_e;
    int tail_a;
    int tail_b;
    int half_a;

    half_a = out_a + rot;
    rot = rot + out_b;
    out_b = out_b + sum_lo + sum_hi;

    sum_lo2 = src[4] + src[0];
    diff_c = src[0] - src[4];
    rot2 = static_cast<int>((static_cast<__int64>(src[2] - src[6]) *
                             static_cast<__int64>(movie_media_idct_storage.constant_words[0])) >>
                            32);
    rot2x2 = rot2 * 2;
    sum_hi2 = src[6] + src[2] + rot2x2;
    diff_d = rot2x2 + diff_c;
    diff_e = diff_c + rot2 * -2;
    tail_a = sum_hi2 + sum_lo2;
    tail_b = sum_lo2 - sum_hi2;

    dst[2] = diff_e + half_a;
    dst[5] = diff_e - half_a;
    dst[3] = tail_b + out_a;
    dst[4] = tail_b - out_a;
    dst[0] = tail_a + out_b;
    dst[1] = rot + diff_d;
    dst[6] = diff_d - rot;
    dst[7] = tail_a - out_b;
}


/*
Purpose: Build one 8x8 coefficient block for the movie decoder: eight
         row-loaders (sub_5ced1d, one per destination row at 0x009C302C..48)
         pull the coefficient rows through their own ESI tables at
         0x009C314C + i*0x20, then eight IDCT rows (sub_5cee80) walk the
         source tables at 0x009C302C + i*0x24, publishing each destination
         row pointer through *reinterpret_cast<int *>(0x009C3004) and advancing it by the stride that
         `stride_arg << 2` leaves in 0x009C3000.

// RECOVERY_ROLE: SCAFFOLD Current typed movie/media owner implementation; original receiver, shared storage, and call ordering retained. Native ABI/output and exceptional behavior remain explicit obligations.
// ORIGINAL: 0x005CEF8A sub_5cef8a 0x005CEF8A-0x005CF11D
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
void __cdecl sub_5cef8a(unsigned char *row_base, int stride_arg) {
    movie_media_idct_storage.output_pitch_bytes =
        static_cast<DWORD>(stride_arg << 2);
    for (int column = 0; column != 8; ++column) {
        int *const destination =
            &movie_media_idct_storage.intermediate_words[column];
        movie_media_idct_set_output_pointer(destination);
        int *const input =
            movie_media_decoder_tables.decoded_coefficient_words + column * 8;
        sub_5ced1d(input);
    }

    movie_media_idct_set_output_pointer(reinterpret_cast<int *>(row_base));
    for (int row = 0; row != 8; ++row) {
        int *const source =
            &movie_media_idct_storage.intermediate_words[row * 9];
        sub_5cee80(source);
        BYTE *destination = reinterpret_cast<BYTE *>(movie_media_idct_output_pointer());
        destination += movie_media_idct_storage.output_pitch_bytes;
        movie_media_idct_set_output_pointer(reinterpret_cast<int *>(destination));
    }
}


/*
Purpose: The row-loader half of the same block build (coefficient fetch
         through the incoming ESI table into the row pointer *reinterpret_cast<int *>(0x009C3004)
         holds). Body still unrecovered - the seed stays in the catalogue;
         this continuity definition exists so the block builder above links.
// RECOVERY_ROLE: SCAFFOLD Current typed movie/media owner implementation; original receiver, shared storage, and call ordering retained. Native ABI/output and exceptional behavior remain explicit obligations.
// ORIGINAL: 0x005CED1D sub_5ced1d 0x005CED1D-0x005CEF8A
// symbol    ?sub_5ced1d@@YAXPAH@Z
// size      621 bytes
// kind      game
// calls     (none)
Status: Continuity placeholder - real body pending, seed preserved.
*/
void __cdecl sub_5ced1d(int *esi_ptr) {
    movie_decoder_idct::IdctConstants constants = movie_media_idct_constants();
    movie_decoder_idct::transform_idct_column(
        reinterpret_cast<const int32_t *>(esi_ptr),
        reinterpret_cast<int32_t *>(movie_media_idct_output_pointer()), constants);
}
