// Typed movie-decoder helpers and source-owned storage views. The generated
// initializer includes are supplied by the configured project preparation
// hook under the ignored build cache.
#include "stdafx.h"
#include "movie_media.h"
#include "general.h"
#include "movie_decoder_owner.h"
#include "movie_decoder_frame.h"
#include "movie_decoder_huffman.h"

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
