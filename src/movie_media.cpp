// Typed movie/media owners. Historical matching obligations are retained;
// decoder and converter bodies live in their mapped source owners. Their
// presence does not establish native codec equivalence or working playback.
#include "stdafx.h"
#include "movie_media.h"
#include "movie_decoder_owner.h"
#include "general.h"

#include <process.h>
#include <stdlib.h>
#include <string.h>

// Media SDK dependencies, distinct from recovered application function bodies.
// RECOVERY_IMPORT: v1 __imp__InterlockedExchange@8 KERNEL32.dll name:InterlockedExchange
// RECOVERY_IMPORT: v1 __imp__SetThreadPriority@8 KERNEL32.dll name:SetThreadPriority
// RECOVERY_IMPORT: v1 __imp__Sleep@4 KERNEL32.dll name:Sleep
// RECOVERY_IMPORT: v1 __imp__mmioClose@8 WINMM.dll name:mmioClose

// ORIGINAL: 0x005CBAE0 sub_5cbae0 0x005CBAE0-0x005CBBB6 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"reason":"Recover the runtime-owned DirectDraw overlay operation and its actual source/destination interfaces, rectangles and failure-path receiver; update_overlay is a descriptive recovered name, not an original debug-symbol claim.","evidence":["Pinned5CBAE0..5CBBB6 passes runtime in ECX and mode in EDX;5CBB6C calls Surface2 slot33(+84), the installed DDRAW.H UpdateOverlay contract, with DDOVERLAYFX size38 and DDFX at30.","MediaRuntime descriptor+0 and rectangle fields+0C/+10/+14/+18; MediaDescriptor Surface2 pointers+14/+1C and dimensions+60/+64 have existing typed producers in movie_media.cpp and UV2Player::exec.","Pinned5CBB5E..6B passes distinct source and destination RECTs; destination is0,0,width-1,height-1. Flags200/84000 mean hide/show|DDFX, not DDBLT_WAIT or rotation.","Pinned5CBB7F writes runtime queue_record_kind+4C on E_INVALIDARG;5CBB98..A0 reloads runtime->descriptor->surface_1c after UpdateOverlay before Restore, ignoring its return.","Original identity/span and seed provenance retained from git:36d19d9ef:src/stringstruct.cpp and retired seed git:117282ee:src/unrecovered/005cbae0.cpp; prior Blt and descriptor-Restore interpretation is superseded, not original behavior."]}
// symbol    ?update_overlay@MediaRuntime@@QAIHH@Z

int __fastcall MediaRuntime::update_overlay(int show) {
  DDOVERLAYFX effects;
  memset(&effects, 0, sizeof(effects));
  effects.dwSize = sizeof(effects);
  DWORD flags;
  if (show != 0) {
    flags = DDOVER_SHOW | DDOVER_DDFX;
    effects.dwDDFX = DDOVERFX_ARITHSTRETCHY;
  } else {
    flags = DDOVER_HIDE;
  }

  // The pinned call passes the runtime output rectangle as lpDestRect and
  // the whole overlay surface (0,0,width-1,height-1) as lpSrcRect.
  RECT destination_rectangle;
  destination_rectangle.top = static_cast<LONG>(output_rect_top_18);
  destination_rectangle.left = static_cast<LONG>(output_rect_left_14);
  destination_rectangle.bottom = static_cast<LONG>(output_rect_top_18 + output_rect_height_10);
  destination_rectangle.right = static_cast<LONG>(output_rect_left_14 + output_rect_extent_0c);

  MediaDescriptor *const output = descriptor;
  RECT source_rectangle;
  source_rectangle.top = 0;
  source_rectangle.left = 0;
  source_rectangle.right = static_cast<LONG>(output->output_width_64 - 1U);
  source_rectangle.bottom = static_cast<LONG>(output->output_height_60 - 1U);

  const HRESULT result = output->surface_1c->UpdateOverlay(
      &source_rectangle, output->surface_14, &destination_rectangle, flags, &effects);
  if (result != DD_OK) {
    if (result == E_INVALIDARG) {
      queue_record_kind = 1;
    } else if (result == DDERR_SURFACELOST) {
      // Original reloads both pointers after the COM callback, not a cached receiver.
      descriptor->surface_1c->Restore();
    }
    return 1;
  }
  return 0;
}

// Typed dispatcher/waiter contracts owned by this translation unit.
extern "C" int __fastcall sub_5c9ec0(MediaRuntime *runtime);
extern "C" void __fastcall sub_5ca970(MediaRuntime *runtime);
extern "C" int __fastcall sub_5ca310(MediaRuntime *runtime, int payload_bytes);
// Product-owned declarations below use the actual owner headers so C++
// linkage is not accidentally changed to C by this draft.

// Existing product body at 0x005CA230: ECX is the runtime, EDX is the
// payload byte count retained from the just-read header, and one stack
// argument selects the record kind. Preserve the product declaration's
// C++ linkage until an authorized identity migration says otherwise.
extern "C" char *__fastcall sub_5cf120(int code);

// Mapped output worker implemented below; this forward declaration supplies
// the CRT thread-entry contract, not a seed-only or placeholder dependency.
extern "C" void __cdecl sub_5cdfa0(void *runtime);

// The source-owned owner starts with the 64-entry block scale lookup at
// 0x9C5820, then preserves the 12 intervening bytes before three adjacent
// 128-DWORD color banks. The remap table follows those banks exactly.
// RECOVERY_OBJECT: v2 0x009C5820 0x009C9594 ?movie_media_conversion_tables@@3UMediaConversionTableStorage@@A 0x005CC940
MediaConversionTableStorage movie_media_conversion_tables;

// Hand-written 16.16 fixed multiply with carry rounding (imul; shl/shr; adc),
// inlined from an asm helper that returns EAX.
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

// Minimum source table read by native builders 5CC940 and 5CD2D0. Their
// indexed accesses cover 64 DWORDs at 0x691F4C..0x69204B; this does not claim
// an extent for surrounding original data.
// RECOVERY_OBJECT: v1 0x00691F4C _media_block_transform_weights
static const DWORD media_block_transform_weights[64] = {
    8192, 5906, 6270, 6967, 8192, 10426, 15137, 29692,
    5906, 4258, 4520, 5023, 5906, 7517, 10913, 21407,
    6270, 4520, 4799, 5332, 6270, 7980, 11585, 22725,
    6967, 5023, 5332, 5925, 6967, 8867, 12873, 25251,
    8192, 5906, 6270, 6967, 8192, 10426, 15137, 29692,
    10426, 7517, 7980, 8867, 10426, 13270, 19266, 37791,
    15137, 10913, 11585, 12873, 15137, 19266, 27969, 54864,
    29692, 21407, 22725, 25251, 29692, 37791, 54864, 107619,
};

// ECX, EDX plus eight stack arguments; 5CC150 supplies exactly this layout.
// ORIGINAL: 0x005CBC00 sub_5cbc00 0x005CBC00-0x005CBE2A
// RECOVERY_BINDING: v1 {"reason":"Recover packed components using full-word widths, the original negative-C saturation and the sum of all three component widths; preserve the unused stack argument and selected VC6/x86 shift behavior.","evidence":["The complete 205-instruction leaf at 5CBC00..5CBE2A reads incoming stack slots +4,+12,+16,+20,+24,+28,+32, never +8, and retains RET 0x20 for all eight stack arguments.","Pinned 5CBE00/5CBE08/5CBE0F load incoming +16/+20/+12 after accounting for the 0x14 frame and saved registers: the final test sums the B/C/A widths, not the unused +8 argument.","Pinned 5CBD45 compares rounded C against its lower bound; 5CBD47 accepts values at or above it, otherwise 5CBD49 reaches 5CBD6F MOV EBX,ECX before the signed borrow test at 5CBD71.","Pinned full-word width loads feed direct CL shifts, and per-component maxima remain live through final packing; the existing caller derives nonnegative channel popcounts with a total of 15 or 16."]}
// symbol    @sub_5cbc00@40
// kind      game
// TRIED: IDA's 8-arg __stdcall guess; disasm shows ecx/edx as real args (fastcall, 10 logical params) and Ghidra's final-sum check param_5+param_6+param_7 is wrong - raw disasm reads [esp+0x2c](param_4)+[esp+0x34](param_6)+[esp+0x38](param_7). Prologue register/stack-frame shape (#0 sub vs push) not chased.
extern "C" int __fastcall sub_5cbc00(
    int sample, int component_a, int component_b, int unused,
    int component_a_bits, int component_b_bits, int component_c_bits,
    int component_a_shift, int component_b_shift, int component_c_shift) {
  int bits = component_a_bits;
  int divisor = 1 << (8 - bits);
  int packed_a = sample / divisor;
  int rounded_a;
  int max_a;
  if (sample < 0) {
    max_a = 1 << bits;
    rounded_a = (sample - (1 << (7 - bits))) / divisor;
    if (rounded_a < 1 - max_a) rounded_a = 1 - max_a;
  } else {
    max_a = 1 << bits;
    rounded_a = ((1 << (7 - bits)) + sample) / divisor;
    if (max_a - 1 < rounded_a) rounded_a = max_a - 1;
  }

  bits = component_b_bits;
  divisor = 1 << (8 - bits);
  int packed_b = component_a / divisor;
  int rounded_b;
  int max_b;
  if (component_a < 0) {
    rounded_b = (component_a - (1 << (7 - bits))) / divisor;
    max_b = 1 << bits;
    if (rounded_b < 1 - max_b) rounded_b = 1 - max_b;
  } else {
    rounded_b = ((1 << (7 - bits)) + component_a) / divisor;
    max_b = 1 << bits;
    if (max_b - 1 < rounded_b) rounded_b = max_b - 1;
  }

  bits = component_c_bits;
  divisor = 1 << (8 - bits);
  unsigned int packed_c = (unsigned int)(component_b / divisor);
  unsigned int rounded_c;
  unsigned int max_c;
  if (component_b < 0) {
    rounded_c = (unsigned int)((component_b -
        (1 << (7 - bits))) / divisor);
    max_c = 1U << bits;
    unsigned int lower = 1U - max_c;
    if ((int)rounded_c < (int)lower) rounded_c = lower;
  } else {
    rounded_c = (unsigned int)(((1 << (7 - bits)) + component_b) /
                               divisor);
    max_c = 1U << bits;
    unsigned int upper = max_c - 1;
    if ((int)rounded_c <= (int)upper) goto combine_components;
    rounded_c = upper;
  }
combine_components:
  if ((int)rounded_c < 0) --rounded_b;
  if (rounded_b < 0) --rounded_a;
  if (rounded_a < 0) --packed_c;
  if ((int)packed_c < 0) --packed_b;
  if (packed_b < 0) --packed_a;

  // Keep each maximum alive through packing, as the pinned routine reuses
  // these values to form the component masks instead of shifting again.
  unsigned int result =
      ((max_c - 1U) & packed_c)
          << ((component_c_shift + 0x10)) |
      ((static_cast<unsigned int>(max_b) - 1U) & (unsigned int)packed_b)
          << ((component_b_shift + 0x10)) |
      ((static_cast<unsigned int>(max_a) - 1U) & (unsigned int)packed_a)
          << ((component_a_shift + 0x10)) |
      ((max_c - 1U) & rounded_c)
          << (component_c_shift) |
      ((static_cast<unsigned int>(max_b) - 1U) & (unsigned int)rounded_b)
          << (component_b_shift) |
      ((static_cast<unsigned int>(max_a) - 1U) & (unsigned int)rounded_a)
          << (component_a_shift);
  if (component_a_bits + component_b_bits + component_c_bits != 0x10)
    result |= 0x80008000;
  return (int)result;
}

// Native caller passes ECX, EDX and five stack dwords; writes all three banks.
// ORIGINAL: 0x005CC150 sub_5cc150 0x005CC150-0x005CC2A9 BYTE_EXACT
// symbol    @sub_5cc150@28
// kind      game
// TRIED: catalogue head is stdcall(a1..a5) but the body reads ecx/edx directly as real values at entry (mov ebx,edx; mov edi,ecx) - PROPOSAL: __fastcall(ecxArg, edxArg, a1..a5); ret 0x14 cleans exactly the 5 stack ints, consistent with fastcall. Call-site argument order for the 2nd/3rd loops was cross-checked against the ghidra decompile's param_1..param_7 (fastcall) naming, which also confirms two of the fixed-point angle computations per loop are dead (computed, never read by the callee).
// The original sets no return value (EAX keeps the last result); callers
// ignore it. The shared declaration stays int.
#pragma warning(disable : 4716)
extern "C" int __fastcall sub_5cc150(int ecx_arg, int edx_arg, int a1,
                                     int a2, int a3, int a4, int a5) {
  DWORD *words = movie_media_conversion_tables.packed_component_words;
  int index;
  for (index = 0; index < 128; ++index) {
    int angle = (int)(signed char)((index << 1) + 1) + 0x80;
    words[index] = (DWORD)sub_5cbc00(angle, angle, angle, ecx_arg, edx_arg,
                        a1, a2, a3, a4, a5);
  }
  for (index = 0; index < 128; ++index) {
    int first = media_fixed_mul(-0x5816, (signed char)((index << 1) + 1));
    int second = media_fixed_mul(0x1c5a1, (signed char)((index << 1) + 1));
    words[256 + index] = (DWORD)sub_5cbc00(0, first, second, 0, edx_arg,
                        a1, a2, a3, a4, a5);
  }
  for (index = 0; index < 128; ++index) {
    int first = media_fixed_mul(0x166e9, (signed char)((index << 1) + 1));
    int second = media_fixed_mul(-0xb6cf, (signed char)((index << 1) + 1));
    words[128 + index] = (DWORD)sub_5cbc00(first, second, 0, 0, edx_arg,
                        a1, a2, a3, a4, a5);
  }
}
#pragma warning(default : 4716)

// Rebuilds the same three 128-word banks with the descriptor-selected value.
// ORIGINAL: 0x005CC2B0 sub_5cc2b0 0x005CC2B0-0x005CC42C
// symbol    @sub_5cc2b0@4
// kind      game
// TRIED: `__int64` product + `(x+0x8000)>>16` for the imul/shr-5 rounded-highword idiom, instead of the raw edx:eax add/adc sequence; #1 mismatch is in the fastcall-arg-to-local store.
extern "C" void __fastcall sub_5cc2b0(int selected_component) {
  DWORD *words = movie_media_conversion_tables.packed_component_words;
  int i;
  for (i = 0; i < 128; ++i) {
    int signed_value = (int)(signed char)(i * 2 + 1);
    unsigned int first = (unsigned int)(signed_value + 0x80);
    unsigned int second = first;
    unsigned int third = first;
    int selected = selected_component;
    if ((int)first < 0) {
      second = first - 1;
      if ((int)second < 0) third = second;
      if ((int)third < 0) selected = selected_component - 1;
    }
    words[i] = ((((third & 0xff) | ((unsigned int)selected << 8)) << 8 |
                     (second & 0xff)) << 8) | (first & 0xff);
  }

  int j;
  for (j = 0; j < 128; ++j) {
    unsigned int first = (unsigned int)media_fixed_mul(-0x5816, (signed char)(j * 2 + 1));
    unsigned int second = (unsigned int)media_fixed_mul(0x1c5a1, (signed char)(j * 2 + 1));
    int selected = 0;
    unsigned int third = 0;
    unsigned int adjusted = first;
    if ((int)second < 0) adjusted = first - 1;
    if ((int)adjusted < 0) {
      third = 0xffffffffU;
      selected = -1;
    }
    words[256 + j] = ((((third & 0xff) | ((unsigned int)selected << 8)) << 8 |
                           (adjusted & 0xff)) << 8) | (second & 0xff);
  }

  int k;
  for (k = 0; k < 128; ++k) {
    signed char scaled = (signed char)(k * 2 + 1);
    unsigned int first = (unsigned int)media_fixed_mul(0x166e9, scaled);
    unsigned int second = (unsigned int)media_fixed_mul(-0xb6cf, scaled);
    int selected = 0;
    if ((int)second < 0) --first;
    if ((int)first < 0) selected = -1;
    words[128 + k] = ((((first & 0xff) | ((unsigned int)selected << 8)) << 8 |
                           (second & 0xff)) << 8);
  }
}

// The receiver is the embedded MediaPixelFormat subobject at descriptor +2C.
// 5CBE30 explicitly computes ECX as descriptor+0x2c before this call.
// ORIGINAL: 0x005CC450 sub_5cc450 0x005CC450-0x005CC70D
// symbol    ?sub_5cc450@MediaPixelFormat@@QAIXPAUMediaRuntime@@@Z
// kind      game
// TRIED: __thiscall member over a PixelFormat-like struct (bit counts at self+0/4/8, shifts at self+0xc/0x10/0x14, a flag at self+0x1c). Loop 1 builds three 512-entry float-derived tables (0x9c6f2c/772c/7f2c) from clamped index -0x100..0xff using six FPU constants at fixed .rdata addresses (dereferenced as plain float/double loads, not transcribed as literals since their bit patterns are unknown); loops 2-4 build three 256-entry integer shift/clamp tables (0x9c872c/8bf4/ 900c) per RGB channel from the struct fields. Ghidra's `__ftol()` no-arg calls were replaced with ordinary `(int)` casts on the FPU expressions, which is what the compiler lowers to `call __ftol` itself. Compiles and matches the first 26 instructions (all of the integer clamp/index setup); diverges at #26 (`fld` vs `lea`) once the FPU stack scheduling for the three-way `fld st(0)` duplication starts - x87 stack scheduling, not source form, and not chased further given the size of the function.
void __fastcall MediaPixelFormat::sub_5cc450(MediaRuntime *) {
  for (int value = -0x100; value < 0x100; ++value) {
    int clamped = value;
    if (clamped < -0x40) clamped = -0x40;
    else if (clamped > 0x3f) clamped = 0x3f;
    int index = (value - 0xc0) & 0x1ff;
    double shifted = clamped + 0.5f;

    movie_media_conversion_tables.contribution_byte3[index] =
        clamped * 0x800801 + 0x4883d8a9;
    int component_1_a = (int)(0.5 - shifted * 0.3441);
    int component_1_b = (int)(shifted * 1.772 + 0.5);
    movie_media_conversion_tables.contribution_byte1[index] =
        ((component_1_a & 0x1ff) << 11) + (component_1_b & 0x1ff);
    int component_0_a = (int)(0.5 - shifted * 0.7141);
    int component_0_b = (int)(shifted * 1.402 + 0.5);
    movie_media_conversion_tables.contribution_byte0[index] =
        ((component_0_a & 0x1ff) + ((component_0_b & 0x1ff) << 12)) << 11;
  }

  int output_index = 0;
  for (int red_input = -0xb1; red_input < 0x1b3;
       red_input += 2, ++output_index) {
    int clamped = red_input < 0 ? 0 : (red_input > 0xff ? 0xff : red_input);
    int sign_extension = clamped >> (-(int)red_bits + 8);
    int value = ((1 << (-(int)red_bits + 7)) + clamped) >>
                (-(int)red_bits + 8);
    int maximum = (1 << red_bits) - 1;
    if (value > maximum) value = maximum;
    int shifted_value = value << red_shift;
    movie_media_conversion_tables.blend_high[output_index] = shifted_value;
    if (total_component_bits <= 0x10)
      movie_media_conversion_tables.blend_high[output_index] =
          (sign_extension << (red_shift + 0x10)) + shifted_value;
  }

  output_index = 0;
  for (int green_input = -0x85; green_input < 0x187;
       green_input += 2, ++output_index) {
    int clamped = green_input < 0 ? 0 :
                  (green_input > 0xff ? 0xff : green_input);
    int sign_extension = clamped >> (-(int)green_bits + 8);
    int value = ((1 << (-(int)green_bits + 7)) + clamped) >>
                (-(int)green_bits + 8);
    int maximum = (1 << green_bits) - 1;
    if (value > maximum) value = maximum;
    int shifted_value = value << green_shift;
    movie_media_conversion_tables.blend_mid[output_index] = shifted_value;
    if (total_component_bits <= 0x10)
      movie_media_conversion_tables.blend_mid[output_index] =
          (sign_extension << (green_shift + 0x10)) + shifted_value;
  }

  output_index = 0;
  for (int blue_input = -0xe1; blue_input < 0x1e3;
       blue_input += 2, ++output_index) {
    int clamped = blue_input < 0 ? 0 :
                  (blue_input > 0xff ? 0xff : blue_input);
    int sign_extension = clamped >> (-(int)blue_bits + 8);
    int value = ((1 << (-(int)blue_bits + 7)) + clamped) >>
                (-(int)blue_bits + 8);
    int maximum = (1 << blue_bits) - 1;
    if (value > maximum) value = maximum;
    int shifted_value = value << blue_shift;
    movie_media_conversion_tables.blend_low[output_index] = shifted_value;
    if (total_component_bits <= 0x10)
      movie_media_conversion_tables.blend_low[output_index] =
          (sign_extension << (blue_shift + 0x10)) + shifted_value;
  }
}

// The first native 5CC940 case-3 reads and all native 5CD2D0 reads touch
// these exact 64 DWORD entries. These indexed values are a neutral source
// table, not a claim that adjacent original words share the same owner.
// RECOVERY_OBJECT: v1 0x0069204C _media_block_quant_order
static const DWORD media_block_quant_order[64] = {
    8, 16, 19, 22, 26, 27, 29, 34, 16, 16, 22, 24, 27, 29, 34, 37,
    19, 22, 26, 27, 29, 34, 34, 38, 22, 22, 26, 27, 29, 34, 37, 40,
    22, 26, 27, 29, 32, 35, 40, 48, 26, 27, 29, 32, 35, 40, 48, 58,
    26, 27, 29, 34, 38, 46, 56, 69, 27, 29, 35, 38, 46, 56, 69, 83,
};

// 64-word output table consumed by 5CC940 case 3. The three scalar literals
// below are read from the pinned initial image with their float/double widths.
// ORIGINAL: 0x005CD2D0 sub_5cd2d0 0x005CD2D0-0x005CD3A2 BYTE_EXACT
// RECOVERY_BINDING: v1 {"reason":"Preserve the original eight-by-eight coefficient walk and signed fixed-point rounding; retain the observed first-coefficient result under the existing integer ABI without asserting an original nominal return type.","evidence":["Pinned 5CD306/5CD367/5CD38E use signed IMUL into EDX:EAX; SHL EDX,16 and SHR EAX,16 combine product bits 16..47, and ADC EAX,EDX adds the carry from original product bit 15 before the DWORD store.","Pinned 5CD2E2 homes the quality-100 path's eight-row latch in the dead quality argument; the non-100 arm advances its coefficient cursor by 0x20 after eight elements until 0x100 bytes are consumed.","Pinned 5CD39A stores the final fixed-point result into destination[0], and the epilogue retains that value in EAX through RET; the mapped caller at 5CC940 does not consume it. The old fabricated zero return is not the observed register result."]}
// symbol    _sub_5cd2d0
// kind      game
// TRIED: homed into general.cpp (its real TU) 2026-09-03 and reverted for one reason:
// TRIED: `long long` (VC6 has none, use __int64); flat 64-iter loop instead of nested 8x8 do/while counted down
// The original sets no return value (EAX keeps the last result); callers
// ignore it. The shared declaration stays int.
#pragma warning(disable : 4716)
extern "C" int __cdecl sub_5cd2d0(DWORD *destination, int quality) {
  if (quality == 100) {
    for (int row = 0; row < 8; ++row) {
      for (int column = 0; column != 8; ++column) {
        const int index = row * 8 + column;
        destination[index] = (DWORD)media_fixed_mul(0x80000, (int)media_block_transform_weights[index]);
      }
    }
  } else {
    const float scale = (float)(107.0 - quality) * 0.0625f;
    for (int row = 0; row < 8; ++row) {
      for (int column = 0; column != 8; ++column) {
        const int index = row * 8 + column;
        destination[index] = (DWORD)media_fixed_mul(
            (int)((int)media_block_quant_order[index] * scale * 65536.0f),
            (int)media_block_transform_weights[index]);
      }
    }
  }
  destination[0] = (DWORD)media_fixed_mul(
      0x80000, (int)media_block_transform_weights[0]);
}
#pragma warning(default : 4716)

// ECX is IDirectDraw2*, EDX is the caller's surface output slot; the three
// stack arguments are width, height and FourCC. Native creates a FourCC
// surface, queries Surface2, applies source-blit color key {1,1}, then
// publishes the queried surface only on success.
// ORIGINAL: 0x005CC860 sub_5cc860 0x005CC860-0x005CC935 BYTE_EXACT
// symbol    @sub_5cc860@20
// kind      game
// TRIED: __stdcall(int,int,int) per IDA's guess - entry reads ecx/edx directly with no matching stack slot, so this is __fastcall with param_1/param_2 in ecx/edx. The three vtable-style calls push their receiver as an explicit stack arg rather than loading it into ecx, so they are plain function-pointer calls through *(vtable+off), not C++ virtual dispatch; exact stack slot reuse for the WSA-shaped struct (0x6c/0x1007/0x80/0x20/4) not nailed down byte-for-byte.
// LEVER: both failures share one Release tail and the result local; the
//        color key flag is DDCKEY_DESTOVERLAY (push 4), not SRCBLT.
extern "C" int __fastcall sub_5cc860(IDirectDraw2 *provider,
                                     IDirectDrawSurface2 **output,
                                     int width, int height, int fourcc) {
  DDSURFACEDESC description;
  memset(&description, 0, 0x6c);
  description.dwSize = 0x6c;
  description.dwFlags = 0x1007;
  description.dwWidth = width;
  description.dwHeight = height;
  description.ddsCaps.dwCaps = 0x80;
  description.ddpfPixelFormat.dwSize = 0x20;
  description.ddpfPixelFormat.dwFlags = DDPF_FOURCC;
  description.ddpfPixelFormat.dwFourCC = (DWORD)fourcc;

  IDirectDrawSurface2 *queried;
  IDirectDrawSurface *created;
  int result;
  if (provider->CreateSurface(&description, &created, 0) == DD_OK) {
    result = 1;
    if (created->QueryInterface(IID_IDirectDrawSurface2,
                                (void **)&queried) == DD_OK) {
      DDCOLORKEY color_key;
      color_key.dwColorSpaceLowValue = result;
      color_key.dwColorSpaceHighValue = result;
      if (queried->SetColorKey(DDCKEY_DESTOVERLAY, &color_key) == DD_OK) {
        *output = queried;
        return 0;
      }
    }
    // Both failures release the original CreateSurface result, not the
    // queried Surface2 pointer; preserve that observed reference choice.
    created->Release();
  } else {
    result = 1;
  }
  return result;
}

int __fastcall sub_5cdad0(IDirectDraw2 *, IDirectDrawSurface2 **, int, int, int);
void __cdecl sub_5cd3b0(char, char, char, char, char, char,
                        int *, unsigned int);
extern "C" void __cdecl sub_5cd560(signed char, signed char, signed char,
                                    signed char, signed char, signed char,
                                    int *, unsigned int);
extern "C" void __fastcall sub_5cd8d0(MediaRuntime *, unsigned char *,
                                      unsigned char *, unsigned char *, unsigned int);
extern "C" void __fastcall sub_5cd980(MediaRuntime *, unsigned char *,
                                      unsigned char *, unsigned char *, unsigned int);
extern "C" void __fastcall sub_5cda30(MediaRuntime *, unsigned char *,
                                      unsigned char *, unsigned char *, unsigned int);
void __cdecl sub_5cfaed(unsigned char *, BYTE *, int);
void __cdecl sub_5d02cc(unsigned char *, void *, int);
int __fastcall sub_5d35b0(MediaRuntime *, const BYTE *, BYTE *, DWORD, int);
extern "C" void __fastcall sub_5d3f50(MediaRuntime *);

// Native 16-bit block writer. Its scale/bank reads are views into the color
// setup owner: scale[0], packed bank 0, and packed banks 1/2. It indexes 128
// entries in each packed bank and writes four 16-byte-wide, eight-row blocks.
// ORIGINAL: 0x005CD560 sub_5cd560 0x005CD560-0x005CD74D
// symbol    _sub_5cd560
// kind      game
// TRIED: homed into general.cpp (its real TU) 2026-09-03 and reverted for one reason:
// TRIED: (none - lands as MISMATCH #2 movsx/mov, likely helper-function extraction changing register scheduling around the sign-extend widen)
extern "C" void __cdecl sub_5cd560(
    signed char component0, signed char component1, signed char component2,
    signed char component3, signed char component4, signed char component5,
    int *destination_words, unsigned int pitch) {
    DWORD *const words = movie_media_conversion_tables.packed_component_words;
    const int chroma =
        words[256 + ((media_fixed_mul(component4,
                      movie_media_conversion_tables.block_scale_entries[0]) >> 1) & 0x7f)] +
        words[128 + ((media_fixed_mul(component5,
                      movie_media_conversion_tables.block_scale_entries[0]) >> 1) & 0x7f)];
    unsigned short *const base = (unsigned short *)destination_words;
    union {
        DWORD word;
        unsigned short half;
    } packed;
    int fill;
    unsigned short *row;
    int outer;

    fill = words[(media_fixed_mul(component0,
                  movie_media_conversion_tables.block_scale_entries[0]) >> 1) & 0x7f] + chroma;
    row = base;
    for (outer = 8; outer != 0; --outer) {
        DWORD *const cells = (DWORD *)row;
        row += 8;
        packed.half = (unsigned short)fill;
        packed.word <<= 16;
        packed.half = (unsigned short)fill;
        cells[0] = packed.word;
        cells[1] = packed.word;
        cells[2] = packed.word;
        cells[3] = packed.word;
        row += pitch / 2 - 8;
    }

    fill = words[(media_fixed_mul(component1,
                  movie_media_conversion_tables.block_scale_entries[0]) >> 1) & 0x7f] + chroma;
    row = base + 8;
    for (outer = 8; outer != 0; --outer) {
        DWORD *const cells = (DWORD *)row;
        row += 8;
        packed.half = (unsigned short)fill;
        packed.word <<= 16;
        packed.half = (unsigned short)fill;
        cells[0] = packed.word;
        cells[1] = packed.word;
        cells[2] = packed.word;
        cells[3] = packed.word;
        row += pitch / 2 - 8;
    }

    fill = words[(media_fixed_mul(component2,
                  movie_media_conversion_tables.block_scale_entries[0]) >> 1) & 0x7f] + chroma;
    row = base + pitch * 4;
    for (outer = 8; outer != 0; --outer) {
        DWORD *const cells = (DWORD *)row;
        row += 8;
        packed.half = (unsigned short)fill;
        packed.word <<= 16;
        packed.half = (unsigned short)fill;
        cells[0] = packed.word;
        cells[1] = packed.word;
        cells[2] = packed.word;
        cells[3] = packed.word;
        row += pitch / 2 - 8;
    }

    fill = words[(media_fixed_mul(component3,
                  movie_media_conversion_tables.block_scale_entries[0]) >> 1) & 0x7f] + chroma;
    row = base + pitch * 4 + 8;
    for (outer = 8; outer != 0; --outer) {
        DWORD *const cells = (DWORD *)row;
        row += 8;
        packed.half = (unsigned short)fill;
        packed.word <<= 16;
        packed.half = (unsigned short)fill;
        cells[0] = packed.word;
        cells[1] = packed.word;
        cells[2] = packed.word;
        cells[3] = packed.word;
        row += pitch / 2 - 8;
    }
}

// Typed mapped worker. Local source packet bytes begin after the eight-byte
// record header; the full caller-owned payload remains the in-place output
// area used by the native block converter.
// RECOVERY_ASSOCIATED: v1 switch_targets 0x005CC940 0x005CD28C 0x005CD2A8 0x005CC985
// RECOVERY_ASSOCIATED: v1 switch_targets 0x005CC940 0x005CD2A8 0x005CD2C4 0x005CCE0D
// ORIGINAL: 0x005CC940 sub_5cc940 0x005CC940-0x005CD28C
// symbol    @sub_5cc940@8
// kind      game
// TRIED: a DirectDraw surface-format-change/blit handler (COM vtable dispatch through IDirectDrawSurface-shaped interfaces, confirmed by the 0x887601c2/DDERR_SURFACELOST-style comparisons and the guidance's own COM-dispatch note). Landed: __fastcall signature (ecx=param_1, edx=param_2, matches `mov eax,edx; mov esi,ecx` at entry), the first switch's dispatch table resolved directly from the image at 0x5CD28C (parsed the PE section table by hand since no RESOLVED JUMP TABLE section was printed for this address: index->target is 0->0x5CC9D4, 1->0x5CC9EF, 2->0x5CC98C, 3->0x5CC9A7, 4..6->0x5CC9DD), the geometry/clip-flag field arithmetic (param_1[3..6]), the COM CreateSurface/QueryInterface/TestCooperativeLevel/Restore/Lock call sequence with real vtable slot numbers (0x10/0x14/0x18/0x1c/0x20 => the emitter's own slots 5,6,13,24,25,27,32) and real arguments, and the second switch's call-out shape to the already-catalogued blit helpers (sub_5cd8d0/5cd980/5cda30/5d35b0/5d4020/5d3980/5d3df0/5d3f50). Case 3 of the second switch is a pixel-format converter that packs 1/3/6 source bytes into 32-bit BGRA-ish words via byte-granularity CONCAT31 before calling sub_5cd560/5cd3b0 (16bpp) or sub_5d02cc/5cfaed (raw copy) - left as a placeholder (TRIED) rather than guess the per-run-type byte layout without more time.
// Rewritten from the image: flags 0x400 set takes the FourCC path and clear
// the CreateSurface path (the old body had them swapped); the fill colour is
// 1; the FourCC array and CreateSurface descriptor stay uninitialized; the
// case-3 scale is (first + phase / 14) with no weight term; kinds 5-7 pass
// the packet and surface base unadjusted. Left: frame layout and register
// choice (diagnostic 0.91).
extern "C" void __fastcall sub_5cc940(MediaRuntime *runtime,
                                     MediaQueueRecord *record) {
  // Native callers 5CABE8/5CAC11/5CB460/5CB489 do not consume EAX.
  BYTE *packet = record->payload;
  const DWORD record_kind = record->field_0c;
  BYTE *surface = 0;
  BYTE *alternate = 0;
  DWORD pitch; // the image leaves it unset for other queue kinds
  DWORD fourcc_count;
  IDirectDrawSurface *created;
  RECT fill_rectangle;
  DWORD fourcc_codes[64]; // not cleared by the original
  DDBLTFX fill_effects;
  DDSURFACEDESC surface_description;
  DDSURFACEDESC locked;
  memset(&locked, 0, sizeof(locked));
  runtime->selected_record_kind_39c = record_kind;
  locked.dwSize = sizeof(locked);

  switch (record_kind) {
  case 3:
    runtime->descriptor->output_width_64 = *(SHORT *)packet;
    runtime->descriptor->output_height_60 = *(SHORT *)(packet + 2);
    runtime->record_scale_byte_048 = packet[4];
    break;
  case 4:
    runtime->descriptor->output_width_64 = *(SHORT *)packet;
    runtime->descriptor->output_height_60 = *(SHORT *)(packet + 2);
    runtime->record_scale_byte_048 = packet[4];
    runtime->record_option_040 = packet[7] & 2;
    runtime->record_option_044 = packet[7] & 1;
    break;
  case 1:
    sub_5cd750(runtime, packet);
    break;
  case 5:
  case 6:
  case 7:
    runtime->descriptor->output_width_64 = *(SHORT *)(packet + 8);
    runtime->descriptor->output_height_60 = *(SHORT *)(packet + 10);
    break;
  }

  runtime->output_rect_left_14 = 0;
  runtime->output_rect_top_18 = 0;
  runtime->output_rect_extent_0c = runtime->descriptor->output_width_64;
  runtime->output_rect_height_10 = runtime->descriptor->output_height_60;
  if ((runtime->descriptor->flags & 0x40) != 0) runtime->output_rect_extent_0c <<= 1;
  if ((runtime->descriptor->flags & 0x30) != 0) runtime->output_rect_height_10 <<= 1;
  if ((runtime->descriptor->flags & 0x04) != 0)
    runtime->output_rect_left_14 =
        (int)(runtime->surface_width_004 - runtime->output_rect_extent_0c) >> 1;
  if ((runtime->descriptor->flags & 0x08) != 0)
    runtime->output_rect_top_18 =
        (int)(runtime->surface_height_008 - runtime->output_rect_height_10) >> 1;
  if ((runtime->descriptor->flags & 0x01) != 0)
    runtime->output_rect_left_14 += runtime->descriptor->geometry_offset_x_68;
  if ((runtime->descriptor->flags & 0x02) != 0)
    runtime->output_rect_top_18 += runtime->descriptor->geometry_offset_y_6c;
  if ((runtime->descriptor->flags & 0x80) != 0)
    runtime->output_rect_extent_0c = runtime->descriptor->geometry_width_override_70;
  if ((runtime->descriptor->flags & 0x100) != 0)
    runtime->output_rect_height_10 = runtime->descriptor->geometry_height_override_74;

  if ((runtime->descriptor->flags & 0x800) == 0) {
    fourcc_count = 0x40;
    if ((runtime->descriptor->flags & 0x400) != 0) {
      if (runtime->queue_record_kind != 1 && runtime->descriptor->surface_1c == 0) {
        if (runtime->descriptor->draw_device_10->GetFourCCCodes(
                &fourcc_count, fourcc_codes) != DD_OK) {
          runtime->queue_record_kind = 1;
        } else if (runtime->queue_record_kind == 4) {
          if (sub_5cdad0(runtime->descriptor->draw_device_10,
                         &runtime->descriptor->surface_1c,
                         (int)runtime->descriptor->output_width_64,
                         (int)runtime->descriptor->output_height_60,
                         0x32595559) != 0)
            runtime->queue_record_kind = 1;
        } else if (sub_5cc860(runtime->descriptor->draw_device_10,
                              &runtime->descriptor->surface_1c,
                              (int)runtime->descriptor->output_width_64,
                              (int)runtime->descriptor->output_height_60,
                              0x32595559) != 0) {
          runtime->queue_record_kind = 1;
        } else {
          runtime->owned_heap_638 = (BYTE *)malloc(
              runtime->descriptor->output_width_64 *
              runtime->descriptor->output_height_60 * 2);
          memset(&fill_effects, 0, sizeof(fill_effects));
          fill_rectangle.left = runtime->output_rect_left_14;
          fill_rectangle.right = runtime->output_rect_extent_0c + fill_rectangle.left;
          fill_rectangle.top = runtime->output_rect_top_18;
          fill_rectangle.bottom = runtime->output_rect_height_10 + fill_rectangle.top;
          fill_effects.dwSize = sizeof(fill_effects);
          fill_effects.dwFillColor = 1;
          runtime->descriptor->surface_14->Blt(&fill_rectangle, 0, 0, 0x01000400,
                                               &fill_effects);
          runtime->output_handle_candidate = 1;
        }
      }
    } else if (runtime->descriptor->surface_1c == 0) {
      // The original sets only these fields; dwSize stays uninitialized.
      surface_description.dwFlags = 7;
      surface_description.ddsCaps.dwCaps = 0x40;
      if ((runtime->descriptor->flags & 0x180) != 0)
        surface_description.ddsCaps.dwCaps = 0x840;
      surface_description.dwWidth = runtime->descriptor->output_width_64;
      surface_description.dwHeight = runtime->descriptor->output_height_60;
      if ((runtime->descriptor->flags & 0x40) != 0)
        surface_description.dwWidth = runtime->descriptor->output_width_64 * 2;
      if (runtime->descriptor->draw_device_10->CreateSurface(
              &surface_description, &created, 0) == DD_OK) {
        created->QueryInterface(IID_IDirectDrawSurface2,
                                (void **)&runtime->descriptor->surface_1c);
        if (runtime->descriptor->draw_device_10->CreateSurface(
                &surface_description, &created, 0) == DD_OK)
          created->QueryInterface(IID_IDirectDrawSurface2,
                                  (void **)&runtime->descriptor->surface_20);
      }
    }

    if (runtime->descriptor->surface_14 != 0 &&
        (runtime->descriptor->flags & 0x02000000) != 0 &&
        runtime->descriptor->surface_14->IsLost() == DDERR_SURFACELOST &&
        runtime->descriptor->surface_14->Restore() != DD_OK)
      return;
    if (runtime->descriptor->surface_18 != 0 &&
        (runtime->descriptor->flags & 0x08000000) != 0 &&
        runtime->descriptor->surface_18->IsLost() == DDERR_SURFACELOST &&
        runtime->descriptor->surface_18->Restore() != DD_OK)
      return;
    if (runtime->descriptor->surface_1c != 0 &&
        runtime->descriptor->surface_1c->IsLost() == DDERR_SURFACELOST) {
      if (runtime->descriptor->surface_1c->Restore() != DD_OK) return;
      runtime->queue_record_kind = 2;
      runtime->output_handle_candidate = 1;
    }
    if (runtime->descriptor->surface_20 != 0 &&
        runtime->descriptor->surface_20->IsLost() == DDERR_SURFACELOST &&
        runtime->descriptor->surface_20->Restore() != DD_OK)
      return;

    switch (runtime->queue_record_kind) {
    case 1:
      if ((runtime->descriptor->flags & 0x400) != 0) {
        if ((runtime->descriptor->flags & 0x08000000) != 0) {
          if (runtime->descriptor->surface_18->Lock(0, &locked, DDLOCK_WAIT, 0) != DD_OK)
            return;
        } else if (runtime->descriptor->surface_14->Lock(0, &locked, DDLOCK_WAIT, 0) != DD_OK) {
          return;
        }
      } else if (runtime->descriptor->surface_1c->Lock(0, &locked, DDLOCK_WAIT, 0) != DD_OK) {
        return;
      }
      surface = alternate = (BYTE *)locked.lpSurface;
      pitch = (DWORD)locked.lPitch;
      break;
    case 2:
      surface = alternate = runtime->owned_heap_638;
      pitch = runtime->descriptor->output_width_64 << 1;
      break;
    case 4:
      if (runtime->descriptor->surface_1c->Lock(0, &locked, DDLOCK_WAIT, 0) != DD_OK)
        return;
      surface = alternate = (BYTE *)locked.lpSurface;
      pitch = (DWORD)locked.lPitch;
      break;
    }
  } else {
    surface = runtime->descriptor->provided_output_base_4c;
    pitch = runtime->descriptor->output_width_64 << 1;
    alternate = runtime->descriptor->provided_output_alternate_50;
    if ((runtime->descriptor->flags & 0x40) != 0) pitch <<= 1;
  }

  switch (runtime->selected_record_kind_39c) {
  case 3: {
    // Same block scale table as 5CD2D0's caller set, built inline here.
    const int complement = 100 - (int)runtime->record_scale_byte_048;
    const int first = complement * 22 / 100 + 8;
    const int step = complement * 50 / 100 + 10 - first;
    int row_phase = 0;
    for (int index = 0; index < 64;) {
      int phase = row_phase;
      for (int column = 8; column != 0; --column) {
        movie_media_conversion_tables.block_scale_entries[index] =
            (DWORD)media_fixed_mul((phase / 14 + first) << 16,
                                   (int)media_block_transform_weights[index]);
        phase += step;
        ++index;
      }
      row_phase += step;
    }

    BYTE *source;
    BYTE *row;
    if ((runtime->descriptor->flags & 0x40) != 0) {
      row = surface + runtime->output_rect_top_18 * pitch +
            runtime->output_rect_left_14 * 4;
      if ((runtime->descriptor->flags & 0x30) != 0) pitch *= 2;
      source = packet + 8;
      for (DWORD block_y = 0; block_y < runtime->descriptor->output_height_60;
           block_y += 16) {
        BYTE *block = row;
        for (DWORD block_x = 0; block_x < runtime->descriptor->output_width_64;
             block_x += 16) {
          int run = *source++;
          if (run > 12) {
            sub_5cfaed(source, block, (int)pitch);
          } else {
            switch (run) {
            case 3:
              sub_5cd3b0(source[0], source[0], source[0], source[0], source[1],
                         source[2], (int *)block, pitch);
              break;
            case 6:
              sub_5cd3b0(source[0], source[1], source[2], source[3], source[4],
                         source[5], (int *)block, pitch);
              break;
            case 12:
              sub_5cd3b0(source[0], source[2], source[4], source[6], source[8],
                         source[10], (int *)block, pitch);
              break;
            }
          }
          source += run;
          block += 0x40;
        }
        row += pitch / 4 * 64;
      }
    } else {
      row = surface + runtime->output_rect_top_18 * pitch +
            runtime->output_rect_left_14 * 2;
      if ((runtime->descriptor->flags & 0x30) != 0) pitch *= 2;
      source = packet + 8;
      for (DWORD block_y = 0; block_y < runtime->descriptor->output_height_60;
           block_y += 16) {
        BYTE *block = row;
        for (DWORD block_x = 0; block_x < runtime->descriptor->output_width_64;
             block_x += 16) {
          int run = *source++;
          if (run > 12) {
            sub_5d02cc(source, block, (int)pitch);
          } else {
            switch (run) {
            case 3:
              sub_5cd560(source[0], source[0], source[0], source[0], source[1],
                         source[2], (int *)block, pitch);
              break;
            case 6:
              sub_5cd560(source[0], source[1], source[2], source[3], source[4],
                         source[5], (int *)block, pitch);
              break;
            case 12:
              sub_5cd560(source[0], source[2], source[4], source[6], source[8],
                         source[10], (int *)block, pitch);
              break;
            }
          }
          source += run;
          block += 0x20;
        }
        row += pitch / 2 * 32;
      }
    }
    break;
  }
  case 4:
    sub_5cd2d0(runtime->decoder_context_3f8.block_transform_weights,
               (int)runtime->record_scale_byte_048);
    if (runtime->queue_record_kind != 1) {
      // A const function pointer keeps the image's real call to 5CDA30.
      void (__fastcall *const decode_blocks)(MediaRuntime *, BYTE *, BYTE *,
                                             BYTE *, unsigned int) = sub_5cda30;
      decode_blocks(runtime, packet + 8, surface, alternate, pitch);
    } else if ((runtime->descriptor->flags & 0x40) == 0 &&
               runtime->descriptor->pixel_format.total_component_bits <= 0x10) {
      BYTE *destination = surface + runtime->output_rect_top_18 * pitch +
                          runtime->output_rect_left_14 * 2;
      if ((runtime->descriptor->flags & 0x30) != 0) pitch *= 2;
      sub_5cd8d0(runtime, packet + 8, destination, alternate, pitch);
    } else {
      BYTE *destination = surface + runtime->output_rect_top_18 * pitch +
                          runtime->output_rect_left_14 * 4;
      if ((runtime->descriptor->flags & 0x30) != 0) pitch *= 2;
      sub_5cd980(runtime, packet + 8, destination, alternate, pitch);
    }
    break;
  case 1:
    sub_5d3df0(runtime, packet);
    if (runtime->descriptor->pixel_format.total_component_bits != 8)
      sub_5d3f50(runtime);
    sub_5d4020(runtime, surface, alternate, (int)pitch);
    break;
  case 2:
    sub_5d3980(runtime, packet);
    sub_5d4020(runtime, surface, alternate, (int)pitch);
    break;
  case 5:
  case 6:
  case 7:
    // The image passes the packet and surface base unadjusted.
    sub_5d35b0(runtime, packet, surface, pitch,
               (int)runtime->selected_record_kind_39c);
    break;
  }

  if ((runtime->descriptor->flags & 0x800) != 0) return;
  switch (runtime->queue_record_kind) {
  case 1:
    if ((runtime->descriptor->flags & 0x400) != 0) {
      if ((runtime->descriptor->flags & 0x08000000) != 0) {
        if (runtime->descriptor->surface_18->Unlock(0) != DD_OK)
          ++runtime->surface_loss_counter_030;
      } else if (runtime->descriptor->surface_14->Unlock(0) != DD_OK) {
        ++runtime->surface_loss_counter_02c;
      }
    } else if (runtime->descriptor->surface_1c->Unlock(0) != DD_OK) {
      ++runtime->surface_loss_counter_038;
    }
    break;
  case 4:
    if (runtime->descriptor->surface_1c->Unlock(0) != DD_OK)
      ++runtime->surface_loss_counter_038;
    break;
  }
}

/*
Purpose: Count the set bits in the low THIRTY-ONE bits of ECX.

             mov edx,ecx / xor eax,eax / xor ecx,ecx
             mov esi,1 / shl esi,cl / test edx,esi / je / inc eax
             inc ecx / cmp ecx,0x1f / jl / ret

         `cmp ecx,0x1f / jl` runs the body for bit indices 0 through 30. BIT 31
         IS NEVER TESTED, so this is not a population count - it agrees with
         one on every value with the top bit clear, which is most of them.

         ECX carries a VALUE here, not a `this` pointer, which is why this one
         does not take the (void *, void *) shape the rest of these do.

// ORIGINAL: 0x005CC430 sub_5cc430 0x005CC430-0x005CC44B FILE BYTE_EXACT
// symbol    ?media_component_mask_bits_31@@YIHI@Z
// size      27 bytes
// prototype
// callers   1   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: how many of bits 0..30 are set
Status: Complete
*/
int __fastcall media_component_mask_bits_31(uint32_t value) {
    int count = 0;
    for (int bit = 0; bit < 0x1F; ++bit) {
        if (value & (1U << bit)) {
            ++count;
        }
    }
    return count;
}

// Native receiver is MediaRuntime; its first pointer is MediaDescriptor*.
// ORIGINAL: 0x005CBE30 sub_5cbe30 0x005CBE30-0x005CC14A
// symbol    ?sub_5cbe30@MediaRuntime@@QAEXXZ
// kind      game
// TRIED: MISMATCH #9 lea/push - thiscall member with local array frame layout differs from the original's; register/stack allocation for the two 91-int DDCAPS scratch buffers does not reproduce the original's `sub esp, 0x364` + rep-stosd shape byte-for-byte.
// LEVER: `descriptor->` re-read at each use except one cached pointer for the
//        pixel-format block; red/green counts written out, the helper called
//        for blue (so this caller must not inline it), signed shift masks.
//        The image passes this runtime in EDX to the fastcall sub_5cc450.
#pragma inline_depth(0)
void MediaRuntime::sub_5cbe30() {
  if (descriptor->stream_gate_80 != 0) return;

  if (descriptor->draw_device_10 == 0 && descriptor->draw_query_interface_0c != 0) {
    release_interface_10 = 1;
    descriptor->draw_query_interface_0c->QueryInterface(
        IID_IDirectDraw2, (void **)&descriptor->draw_device_10);
  }


  int result_kind;
  if ((descriptor->flags & 0x00000200) != 0) {
    DDCAPS caps_a;
    DDCAPS caps_b;
    memset(&caps_a, 0, sizeof(caps_a));
    memset(&caps_b, 0, sizeof(caps_b));
    caps_a.dwSize = 0x16c;
    caps_b.dwSize = 0x16c;
    if (descriptor->draw_device_10->GetCaps(&caps_a, &caps_b) == DD_OK) {
      if ((descriptor->capability_options_08 & 2) != 0 &&
          (caps_a.dwCaps & 0x2000) != 0 && (caps_a.dwCaps & 0x4000) != 0) {
        result_kind = 2;
      } else if ((descriptor->capability_options_08 & 4) != 0 &&
                 (caps_a.dwCaps & 0x100) != 0 &&
                 (caps_a.dwCaps & 0x200) != 0) {
        result_kind = 4;
      } else {
        result_kind = 1;
      }
    } else {
      result_kind = 1;
    }
  } else {
    result_kind = 1;
  }
  queue_record_kind = result_kind;
  descriptor->surface_1c = 0;
  descriptor->surface_20 = 0;

  if ((descriptor->flags & 0x00000800) == 0) {
    DDSURFACEDESC surface_description;
    memset(&surface_description, 0, sizeof(surface_description));
    surface_description.dwSize = sizeof(surface_description);
    if (descriptor->surface_14->GetSurfaceDesc(&surface_description) == DD_OK) {
      surface_width_004 = surface_description.dwWidth;
      surface_height_008 = surface_description.dwHeight;
    }

    MediaDescriptor *d = descriptor;
    if (d->surface_14 != 0) {
      DDPIXELFORMAT pixel_format;
      memset(&pixel_format, 0, sizeof(pixel_format));
      pixel_format.dwSize = sizeof(pixel_format);
      d->surface_14->GetPixelFormat(&pixel_format);

      if (pixel_format.dwRGBBitCount == 8) {
        d->pixel_format.total_component_bits = 8;
      } else {
        int bit;
        int red_bits = 0;
        for (bit = 0; bit < 0x1f; ++bit) {
          if ((pixel_format.dwRBitMask & (1U << bit)) != 0) ++red_bits;
        }
        d->pixel_format.red_bits = red_bits;
        int green_bits = 0;
        for (bit = 0; bit < 0x1f; ++bit) {
          if ((pixel_format.dwGBitMask & (1U << bit)) != 0) ++green_bits;
        }
        d->pixel_format.green_bits = green_bits;
        d->pixel_format.blue_bits =
            (DWORD)media_component_mask_bits_31(pixel_format.dwBBitMask);

        // Signed masks: the image shifts them with `sar`.
        int red_shift = 0;
        int red_mask = (int)pixel_format.dwRBitMask;
        while ((red_mask & 1) == 0) {
          red_mask >>= 1;
          ++red_shift;
        }
        d->pixel_format.red_shift = red_shift;

        int green_shift = 0;
        int green_mask = (int)pixel_format.dwGBitMask;
        while ((green_mask & 1) == 0) {
          green_mask >>= 1;
          ++green_shift;
        }
        d->pixel_format.green_shift = green_shift;

        int blue_shift = 0;
        int blue_mask = (int)pixel_format.dwBBitMask;
        while ((blue_mask & 1) == 0) {
          blue_mask >>= 1;
          ++blue_shift;
        }
        d->pixel_format.blue_shift = blue_shift;

        d->pixel_format.total_component_bits =
            d->pixel_format.red_bits + d->pixel_format.blue_bits +
            d->pixel_format.green_bits;
        d->pixel_format.combined_component_mask =
            pixel_format.dwRBitMask | pixel_format.dwGBitMask |
            pixel_format.dwBBitMask;
      }
    }

    if (descriptor->pixel_format.total_component_bits == 8 &&
        (descriptor->flags & 0x04000000) == 0) {
      descriptor->palette_entries_28 = (BYTE *)malloc(0x400);
      descriptor->draw_device_10->CreatePalette(
          0x44, (LPPALETTEENTRY)descriptor->palette_entries_28, &descriptor->palette_24, 0);
      memset(descriptor->palette_entries_28, 0, 0x400);
      descriptor->surface_14->SetPalette(descriptor->palette_24);
    }
  }

  if ((descriptor->flags & 0x00100000) == 0) descriptor->setup_value_78 = 100;

  if (descriptor->pixel_format.total_component_bits == 0x10 ||
      descriptor->pixel_format.total_component_bits == 0x0f) {
    sub_5cc150(0xff, (int)descriptor->pixel_format.red_bits,
               (int)descriptor->pixel_format.green_bits,
               (int)descriptor->pixel_format.blue_bits,
               (int)descriptor->pixel_format.red_shift,
               (int)descriptor->pixel_format.green_shift,
               (int)descriptor->pixel_format.blue_shift);
  } else {
    sub_5cc2b0(0xff);
  }
  descriptor->pixel_format.sub_5cc450(this);

  for (int input = -0x200; input < 0x200; ++input) {
    int clamped = input;
    if (clamped < -0x80) clamped = -0x80;
    else if (clamped > 0x7f) clamped = 0x7f;
    unsigned int index = (unsigned int)(input - 0x180) & 0x3ff;
    movie_media_conversion_tables.remap_or_lookup_storage.words[index] =
        (DWORD)(clamped + 0x80);
  }
  teardown_gate_3c = 1;
}
#pragma inline_depth(8)

// Caller/native receiver evidence identifies this as a MediaRuntime method.
// ORIGINAL: 0x005CE2B0 sub_5ce2b0 0x005CE2B0-0x005CE2F0 BYTE_EXACT
// symbol    ?sub_5ce2b0@MediaRuntime@@QAEHXZ
// kind      game
int MediaRuntime::sub_5ce2b0() {
  InterlockedExchange(&sync_word_3b0, 1);
  uintptr_t thread = _beginthread(sub_5cdfa0, 0, this);
  beginthread_result_3b4 = (DWORD)thread;
  SetThreadPriority((HANDLE)thread, 2);
  return beginthread_result_3b4 == 0;
}

// ORIGINAL: 0x005CE2F0 sub_5ce2f0 0x005CE2F0-0x005CE336 BYTE_EXACT
// symbol    ?sub_5ce2f0@MediaRuntime@@QAEHXZ
// kind      game
int MediaRuntime::sub_5ce2f0() {
  if (beginthread_result_3b4 != 0) {
    InterlockedExchange(&sync_word_3b0, 0);
    LONG observed = InterlockedExchange(&sync_word_3b0, 0);
    if (observed == 0) {
      do {
        Sleep(10);
        observed = InterlockedExchange(&sync_word_3b0, 0);
      } while (observed == 0);
    }
    return 0;
  }
  return 1;
}

// CRT worker entry. Its active fields are the pending output ring and the
// DirectSound buffer interface already established by the descriptor owner.
// ORIGINAL: 0x005CDFA0 sub_5cdfa0 0x005CDFA0-0x005CE2B0
// symbol    _sub_5cdfa0
// kind      game
// TRIED: the brief's parameterless contract - disasm reads one cdecl stack arg at entry+4 (matches IDA's void*) and never touches ecx, so this takes a void* param, not zero args. COM slots 4/11/19/20 on the interface at *(*self+0x5c) are exactly IDirectSoundBuffer::GetCurrentPosition/Lock/Unlock/Restore; ignored Ghidra's InterlockedExchange/Increment/Decrement names for the four 0x6690xx/0x669100 calls - those are wrong guesses at an unresolved fixed address, not real symbols.
// The worker re-reads every shared cursor at each use (the image reloads a
// field straight after storing it), so it sees the runtime as volatile;
// the COM calls go through a plain alias (their loads schedule freely).
// Cursor comparisons and the modulo are signed.
extern "C" void __cdecl sub_5cdfa0(void *argument) {
  volatile MediaRuntime *r = (volatile MediaRuntime *)argument;
  MediaRuntime *self = (MediaRuntime *)argument;
  Sleep(10);
  if (InterlockedExchange(const_cast<LONG *>(&r->sync_word_3b0), 1) != 0) {
    do {
      if (r->buffer_setup_finished != 0) {
        DWORD write_position;
        if (r->synthetic_cursor_mode != 0) {
          r->synthetic_playback_cursor += 0x372;
          if ((LONG)r->synthetic_playback_cursor >= (LONG)r->playback_buffer_bytes)
            r->synthetic_playback_cursor -= r->playback_buffer_bytes;
          write_position = r->synthetic_playback_cursor;
        } else {
          DWORD safe_write_position;
          self->descriptor->play_buffer->GetCurrentPosition(&write_position,
                                                         &safe_write_position);
        }

        LONG available;
        if ((LONG)r->playback_cursor < (LONG)write_position)
          available = (LONG)(write_position - r->playback_cursor);
        else
          available = (LONG)(write_position - r->playback_cursor +
                             r->playback_buffer_bytes);
        if (available >= (LONG)((MediaQueueRecord *)r->output_records)[r->output_consumer_cursor % 24].field_04) {
          volatile MediaQueueRecord *record =
              &r->output_records[r->output_consumer_cursor % 24];
          LPVOID first, second;
          DWORD first_bytes, second_bytes;
          if (r->pending_output_records != 0) {
            if (r->synthetic_cursor_mode == 0) {
              HRESULT result = self->descriptor->play_buffer->Lock(
                  r->playback_cursor, record->field_04, &first, &first_bytes,
                  &second, &second_bytes, 0);
              while (result != DS_OK) {
                self->descriptor->play_buffer->Restore();
                result = self->descriptor->play_buffer->Lock(
                    r->playback_cursor, record->field_04, &first, &first_bytes,
                    &second, &second_bytes, 0);
              }
              memcpy(first, record->payload, first_bytes);
              if (second_bytes != 0)
                memcpy(second, record->payload + first_bytes, second_bytes);
              self->descriptor->play_buffer->Unlock(first, first_bytes, second, second_bytes);
            }
            r->playback_cursor =
                (LONG)(r->playback_cursor + record->field_04) %
                (LONG)r->playback_buffer_bytes;
            InterlockedIncrement(const_cast<LONG *>(&r->output_consumer_cursor));
            InterlockedExchange(const_cast<LONG *>(&r->output_lock_word),
                                (LONG)record->field_08);
            InterlockedDecrement(const_cast<LONG *>(&r->pending_output_records));
            record->payload = 0;
            record->field_04 = 0;
          } else {
            if (r->synthetic_cursor_mode == 0
                && self->descriptor->play_buffer->Lock(
                       r->playback_cursor, (DWORD)available, &first,
                       &first_bytes, &second, &second_bytes, 0) == DS_OK) {
              memset(first, 0, first_bytes);
              if (second_bytes != 0) memset(second, 0, second_bytes);
              self->descriptor->play_buffer->Unlock(first, first_bytes, second, second_bytes);
            }
            r->playback_cursor =
                (LONG)(r->playback_cursor + (DWORD)available) %
                (LONG)r->playback_buffer_bytes;
            InterlockedIncrement(const_cast<LONG *>(&r->output_lock_word));
          }
        }
      }
      InterlockedIncrement(const_cast<LONG *>(&r->worker_wakeup_counter));
      Sleep(10);
    } while (InterlockedExchange(const_cast<LONG *>(&r->sync_word_3b0), 1) != 0);
  }
  _endthread();
}

// ORIGINAL: 0x005CB050 sub_5cb050 0x005CB050-0x005CB220 BYTE_EXACT
// symbol    ?initialize_stream@MediaRuntime@@SIHPADPAU1@@Z
// kind      game
// TRIED: sub_5c9ec0 is catalogued nullary cdecl but the disassembly sets `ecx = esi` (this=a2) before every call - a free __thiscall declaration is C4234, so it is reached through the Dummy member-function-pointer trick. The four mmio* Windows multimedia calls go through fixed-address function-pointer globals rather than by name, since the catalogue does not model them. Landing the closest control-flow-faithful form (divergence starts at instruction #6, on the mmio-open guard).
int __fastcall MediaRuntime::initialize_stream(LPSTR filename,
                                               MediaRuntime *runtime) {
  MediaRuntime &r = *runtime;
  *reinterpret_cast<volatile LONG *>(&r.stream_eof) = 0;
  DWORD signature;  // not initialized by the original; a short read keeps stack garbage
  if ((r.descriptor->flags & 0x00080000) == 0)
    r.descriptor->stream = mmioOpenA(filename, 0, 0);
  if (r.descriptor->stream == 0) return 1;
  if (mmioSetBuffer(r.descriptor->stream, 0, 0x4000, 0) != 0) return 1;
  mmioRead(r.descriptor->stream, (HPSTR)&signature, 4);
  mmioSeek(r.descriptor->stream, -4, SEEK_CUR);

  r.descriptor->stream_gate_7c = signature == 0x44414553 ||
                                 signature == 0x6c484353 ||
                                 signature == 0x684e5331;

  for (int i = 0; i < 24; ++i) r.output_records[i].payload = 0;
  for (int k = 0; k < 24; ++k) r.read_records[k].payload = 0;

  r.input_ring_base = (BYTE *)calloc(1, 1200000);
  r.input_ring_current = r.input_ring_base;
  if (r.input_ring_base == 0) return 2;
  r.decoded_ring_base = (BYTE *)calloc(1, 300000);
  r.decoded_ring_current = r.decoded_ring_base;
  if (r.decoded_ring_base == 0) return 2; // native path leaves first block

  DWORD *input_words = (DWORD *)r.input_ring_base;
  for (int input_count = 300000; input_count != 0; --input_count)
    *input_words++ = 0;
  DWORD *decoded_words = (DWORD *)r.decoded_ring_base;
  for (int output_count = 75000; output_count != 0; --output_count)
    *decoded_words++ = 0;

  r.read_counter = 0;
  r.output_counter = 0;
  r.read_consumer_cursor = 0;
  r.read_producer_cursor = 0;

  r.pending_read_records = 0;
  r.pending_output_records = 0;
  r.channel_state = 4;

  if (r.descriptor->stream_gate_7c != 0) {
    while (r.pending_output_records < 0x15) {
      if (sub_5c9ec0(runtime) == -1) break;
      if (sub_5c9ec0(runtime) == -1) break;
    }
  } else {
    while (r.pending_read_records < 0x14) {
      if (sub_5c9ec0(runtime) == -1) break;
    }
  }
  r.descriptor->stream_gate_80 =
      (*reinterpret_cast<volatile LONG *>(&r.read_counter) == 0);
  return 0;
}

// One TLV field: p is at the tag; steps over it and the length byte, then
// accumulates that many big-endian value bytes. count keeps the length.
static inline int media_read_field(BYTE *&p, unsigned int &count) {
  ++p;
  unsigned int remaining = *p++;
  count = remaining;
  int value = 0;
  while (remaining--) value = (value << 8) + *p++;
  return value;
}

static inline int media_read_signed_field(BYTE *&p) {
  unsigned int count;
  int value = media_read_field(p, count);
  if (count == 1) {
    if (value > 0x7f) value -= 0x100;
  } else if (count == 2) {
    if (value > 0x7fff) value -= 0x10000;
  } else if (count == 3) {
    if (value > 0x7fffff) value -= 0x1000000;
  }
  return value;
}

// RECOVERY_ASSOCIATED: v1 switch_targets 0x005CA6C0 0x005CA95C 0x005CA96C 0x005CA770
// ORIGINAL: 0x005CA6C0 sub_5ca6c0 0x005CA6C0-0x005CA95B
// symbol    ?parse_initial_format@MediaRuntime@@QAIHH@Z
// kind      game
// TRIED: NOT a real parameter - Ghidra's `char param_1` is actually buf[4], a byte read from the just-mmioRead'd local buffer, because Ghidra doesn't track the alloca'd 0xc000 frame. Modeled instead as a __thiscall member (ecx->edi is `this`, no stack arg) with a large local `char buf[0xc000]` (reproduces the `mov eax,0xc000; call __alloca_probe` prologue for free) parsing a TLV byte stream: skip name fields until 0xfd, then dispatch tags 0x81/0x82/0x83/0x84 (big- endian length-prefixed sign-extended ints stored to this+0x5c/0x54/ 0x60/0x58) until tag 0x8a. Reordering local decls to match the original's `lea eax,[esp]`-before-prologue-pushes scheduling did not change the codegen; first divergence stays at instruction #2 (original computes the buffer pointer before saving edi, rebuilt pushes edi first) - register scheduling, not source form.
// The jump table at 0x5CA95C stores tag 0x83 to +0x58 (playback rate) and
// 0x84 to +0x60; the TRIED note above has those two swapped.
// Left: the image adjusts with sub reg,imm (ours add -imm) and swaps
// ecx/edx in the skip loops.
int __fastcall MediaRuntime::parse_initial_format(int byte_count) {
  BYTE bytes[0xc000];
  LONG read = mmioRead(descriptor->stream, (HPSTR)bytes, byte_count);
  if (read == 0) return -1;
  if (read == -1) return -2;

  format_parameter_5c = 0x10;
  format_parameter_54 = 1;
  playback_frequency = 0x5622;
  format_dispatch_60 = 0;

  unsigned int count;
  BYTE *p = bytes + 4;
  while (*p != 0xfd) media_read_field(p, count);

  ++p;
  for (int tag = *p; tag != 0x8a; tag = *p) {
    switch (tag) {
    case 0x81: format_parameter_5c = (DWORD)media_read_signed_field(p); break;
    case 0x82: format_parameter_54 = (DWORD)media_read_signed_field(p); break;
    case 0x83: playback_frequency = (DWORD)media_read_signed_field(p); break;
    case 0x84: format_dispatch_60 = (DWORD)media_read_signed_field(p); break;
    default: media_read_field(p, count); break;
    }
  }
  return 0;
}

// The function is a stream-record dispatcher. This name is a source-owner
// candidate only; FourCC values and record-kind values stay numeric because
// no codec/library attribution is established by these branches.
// ORIGINAL: 0x005C9EC0 sub_5c9ec0 0x005C9EC0-0x005CA230 BYTE_EXACT
// symbol    @sub_5c9ec0@4
// kind      game
// TRIED: sub_5ca230/5ca310/5ca6c0 are __thiscall members of the object this function receives in ecx (mov ecx,esi before each direct call); the three `// indirect` sites are all the same cached mmioRead(HMMIO,HPSTR,LONG) stdcall pointer loaded from g_0066936c once and called through a register.
// TRIED: transcribed the fourcc/cch big-endian header decode as explicit byte shifts (buf[0..3], buf[4..7]) and the branch tree as goto-labelled ifs mirroring Ghidra 1:1; compiles and is structurally faithful but only 0.44 mnemonic similarity (996 vs 876 bytes) - did not reverse-engineer the exact esp-relative overlapping-load pattern the original uses to build the two big-endian dwords, nor match the alloca'd buffer's exact size.
// The retained continuity mapping reaches 0x005CA230; the old seed
// ended at 0x005CA22C. Preserve this historical extent discrepancy
// rather than silently treating either as fresh boundary proof.
extern "C" int __fastcall sub_5c9ec0(MediaRuntime *runtime) {
  BYTE header_and_payload[0xc00c];
  LONG read = mmioRead(runtime->descriptor->stream,
                       (HPSTR)header_and_payload, 8);
  if (read == 0) {
    runtime->stream_eof = 1;
    return -1;
  }
  if (read == -1 || read != 8) return -2;

  // Both header words are assembled least-significant byte first.
  DWORD payload_bytes =
      ((DWORD)header_and_payload[7] << 24) |
      ((DWORD)header_and_payload[6] << 16) |
      ((DWORD)header_and_payload[5] << 8) |
      (DWORD)header_and_payload[4];
  payload_bytes -= 8;
  DWORD type =
      ((DWORD)header_and_payload[3] << 24) |
      ((DWORD)header_and_payload[2] << 16) |
      ((DWORD)header_and_payload[1] << 8) |
      (DWORD)header_and_payload[0];

  switch (type) {
  case 0x66325655: // bytes: 66 32 56 55
  case 0x54514970: // bytes: 54 51 49 70
    return runtime->sub_5ca230((int)payload_bytes, 4);
  case 0x6544414d:
  case 0x4d414465:
    return runtime->sub_5ca230((int)payload_bytes, 7);
  case 0x66564754:
  case 0x54475666:
    return runtime->sub_5ca230((int)payload_bytes, 2);
  case 0x6b44414d:
  case 0x4d41446b:
    return runtime->sub_5ca230((int)payload_bytes, 5);
  case 0x6656554d:
  case 0x54475170:
    return runtime->sub_5ca230((int)payload_bytes, 3);
  case 0x684e5331:
    return runtime->parse_initial_format((int)payload_bytes);
  case 0x6b564754:
  case 0x5447566b:
    return runtime->sub_5ca230((int)payload_bytes, 1);
  case 0x6d44414d:
  case 0x4d41446d:
    return runtime->sub_5ca230((int)payload_bytes, 6);
  case 0x6c484353:
  case 0x5343486c:
    return runtime->parse_initial_format((int)payload_bytes);
  case 0x43444e53:
  case 0x534e4443: {
    if (runtime->descriptor->stream_gate_7c == 0) return -2;
    return sub_5ca310(runtime, (int)payload_bytes);
  }
  case 0x644e5331:
  case 0x5343446c:
  case 0x6c444353: {
    if (runtime->descriptor->stream_gate_7c == 0) return -2;
    return sub_5ca310(runtime, (int)payload_bytes);
  }
  case 0x44414553: {
    read = mmioRead(runtime->descriptor->stream,
                    (HPSTR)(header_and_payload + 0x0c),
                    (LONG)payload_bytes);
    if (read == 0) {
      runtime->stream_eof = 1;
      return -1;
    }
    if (read == -1) return -2;
    runtime->format_parameter_5c = 0x10;
    runtime->format_parameter_54 = 2;
    runtime->playback_frequency = 0x5622;
    runtime->format_dispatch_60 = 0x0a;
    return 0;
  }
  case 0x6c434353:
  case 0x6c4c4353:
  case 0x5343436c:
  case 0x53434c6c: {
    (void)mmioRead(runtime->descriptor->stream,
                   (HPSTR)(header_and_payload + 0x0c),
                   (LONG)payload_bytes);
    return 4;
  }
  case 0x6c454353:
  case 0x654e5331:
  case 0x444e4553:
    return -1;
  default:
    return -2;
  }
}

// 5CDB90/5CDD10's coefficient views overlap each other and the beginning of
// the existing general.cpp IMA delta table. All are views into the one
// source-owned MediaAdpcmTables POD declared in general.h; never duplicate
// the initial 20 words or read the executable's fixed addresses.
// Macros, not `int *const` locals: VC6 reads such a pointer from storage
// where the image indexes the table address directly.
#define media_decoder_lookup_a (MediaAdpcmTables.lookup_and_delta_words)
#define media_decoder_lookup_b (MediaAdpcmTables.lookup_and_delta_words + 4)

// ORIGINAL: 0x005CDB90 sub_5cdb90 0x005CDB90-0x005CDD0C
// symbol    _sub_5cdb90
// kind      game
// TRIED: homed into general.cpp (its real TU) 2026-09-03 and reverted for one reason:
// TRIED: `extern "C" ... sub_5cdb90()` fallback head; the disasm's no-`push ebp` esp-relative frame with 6 stack reads matches IDA's 6-arg guess, so the signature was changed to (int, int*, int*, int*, int, unsigned int*). #0 mismatch is the `sub esp,0x10` prologue vs the compiler's own frame shape.
extern "C" void __cdecl sub_5cdb90(int count, int *cursor, int *predictor,
                                   int *accumulator, int input_base,
                                   unsigned int *output_cursor) {
  BYTE *input = (BYTE *)(input_base + (*cursor * 15) / 28);
  int previous = *predictor;
  int value = *accumulator;
  short *output = (short *)*output_cursor;
  if (count > 0) {
    unsigned int blocks = ((unsigned int)count + 0x1b) / 0x1c;
    do {
      *cursor += 0x1c;
      int shift = (*input & 0xf) + 8;
      unsigned int code = *input >> 4;
      int coefficient_a = media_decoder_lookup_b[code];
      int coefficient_b = media_decoder_lookup_a[code];
      short *end = output + 0x38;
      ++input;
      while (output < end) {
        value = (((int)((unsigned int)(*input >> 4) << 0x1c) >> shift) +
                 coefficient_a * value + 0x80 + coefficient_b * previous) >> 8;
        if (value < -0x8000) value = -0x8000;
        else if (value > 0x7fff) value = 0x7fff;
        output[0] = (short)value;
        output[1] = (short)value;
        previous = (((int)((unsigned int)*input << 0x1c) >> shift) +
                    value * coefficient_b + 0x80 + coefficient_a * previous) >> 8;
        if (previous < -0x8000) previous = -0x8000;
        else if (previous > 0x7fff) previous = 0x7fff;
        output[2] = (short)previous;
        output[3] = (short)previous;
        output += 4;
        ++input;
      }
      --blocks;
    } while (blocks != 0);
  }
  *predictor = previous;
  *accumulator = value;
  *output_cursor = (unsigned int)output;
}

// ORIGINAL: 0x005CDD10 sub_5cdd10 0x005CDD10-0x005CDF93
// symbol    _sub_5cdd10
// kind      game
// TRIED: homed into general.cpp (its real TU) 2026-09-03 and reverted for one reason:
// TRIED: literal Ghidra-shaped transcription (8 int/ptr params, void return, do/while outer + for inner, saturating clamps) compiles and is logically faithful but MISMATCHes at instruction #0: original has no `push ebp; mov ebp,esp` frame (ebp is a plain register-allocated variable across the whole function, /Oy-style), so register allocation diverges from the very first instruction. Not chased further - large heavily-optimized leaf arithmetic function, register scheduling not source-form.
// Each block is a two-byte header (filter codes, then shifts) and 14 byte
// pairs; a pair's first byte holds the first left/right samples (high/low
// nibble), its second byte the next two. Arguments 3/4 are the left
// channel's (predictor, accumulator), 5/6 the right's, as for sub_5cdb90.
extern "C" void __cdecl sub_5cdd10(int count, int *cursor,
                                   int *left_predictor, int *left_accumulator,
                                   int *right_predictor, int *right_accumulator,
                                   int input_base, unsigned int *output_cursor) {
  BYTE *input = (BYTE *)(input_base + (*cursor * 30) / 28);
  short *output = (short *)*output_cursor;
  int left_prev = *left_predictor;
  int left_last = *left_accumulator;
  int right_prev = *right_predictor;
  int right_last = *right_accumulator;
  if (count > 0) {
    unsigned int blocks = ((unsigned int)count + 0x1b) / 0x1c;
    do {
      *cursor += 0x1c;
      input += 2;
      unsigned int hi = input[-2] >> 4, lo = input[-2] & 0xf;
      int hi_a = media_decoder_lookup_a[hi], hi_b = media_decoder_lookup_b[hi];
      int lo_a = media_decoder_lookup_a[lo], lo_b = media_decoder_lookup_b[lo];
      int shift_lo = (input[-1] & 0xf) + 8, shift_hi = (input[-1] >> 4) + 8;
      short *end = output + 0x38;
      while (output < end) {
        int sample_left = ((((int)((*input & 0xf0) << 0x18)) >> shift_hi) +
                           hi_b * left_last + hi_a * left_prev + 0x80) >> 8;
        int sample_right = (((int)((unsigned int)*input << 0x1c) >> shift_lo) +
                            lo_b * right_last + lo_a * right_prev + 0x80) >> 8;
        if (sample_left < -0x8000) sample_left = -0x8000;
        else if (sample_left > 0x7fff) sample_left = 0x7fff;
        if (sample_right < -0x8000) sample_right = -0x8000;
        else if (sample_right > 0x7fff) sample_right = 0x7fff;
        output[0] = (short)sample_left;
        output[1] = (short)sample_right;
        left_last = sample_left;
        right_last = sample_right;
        int next_left = ((((int)((input[1] & 0xf0) << 0x18)) >> shift_hi) +
                         sample_left * hi_a + hi_b * left_prev + 0x80) >> 8;
        int next_right = (((int)((unsigned int)input[1] << 0x1c) >> shift_lo) +
                          sample_right * lo_a + lo_b * right_prev + 0x80) >> 8;
        if (next_left < -0x8000) next_left = -0x8000;
        else if (next_left > 0x7fff) next_left = 0x7fff;
        if (next_right < -0x8000) next_right = -0x8000;
        else if (next_right > 0x7fff) next_right = 0x7fff;
        left_prev = next_left;
        output[2] = (short)next_left;
        output[3] = (short)next_right;
        output += 4;
        input += 2;
        right_prev = next_right;
      }
      --blocks;
    } while (blocks != 0);
  }
  *left_predictor = left_prev;
  *left_accumulator = left_last;
  *right_predictor = right_prev;
  *right_accumulator = right_last;
  *output_cursor = (unsigned int)output;
}

// Register evidence: runtime in ECX, encoded byte count in EDX.
// ORIGINAL: 0x005CA310 sub_5ca310 0x005CA310-0x005CA6BC
// symbol    @sub_5ca310@8
// kind      game
// TRIED: __fastcall(ecx,edx) two-arg signature + large local buffer for __alloca_probe reaches MISMATCH #3 (prologue); several output scratch locals (sub_5cdd10/5cdb90/5ce6e3 args) use fresh locals rather than the exact original stack slot, since those callees are opaque and their slot placement doesn't affect this body.
extern "C" int __fastcall sub_5ca310(MediaRuntime *r, int byte_count) {
  BYTE input[0xc008];
  int output_bytes;
  if (r->output_counter == 0) {
    MediaDescriptor *seek_descriptor = r->descriptor;
    if ((seek_descriptor->flags & 0x00008000) != 0)
      r->channel_seek_position =
          mmioSeek(seek_descriptor->stream, 0, SEEK_CUR) - 8;
  }
  MediaDescriptor *read_descriptor = r->descriptor;
  LONG read = mmioRead(read_descriptor->stream,
                       (HPSTR)(input + 0x14), byte_count);
  if (read == 0) { r->stream_eof = 1; return -1; }
  if (read == -1) return -2;

  if (r->format_dispatch_60 == 0) {
    output_bytes = (int)(*(DWORD *)(input + 0x18) * 4);
    if (r->decoded_ring_current + output_bytes >= r->decoded_ring_base + 0x493e0)
      r->decoded_ring_current = r->decoded_ring_base;
    int words = (output_bytes + ((output_bytes >> 31) & 3)) >> 2;
    if (r->format_parameter_54 == 2) {
      for (int i = 0; i < words; ++i)
        ((DWORD *)r->decoded_ring_current)[i] = ((DWORD *)(input + 0x1c))[i];
    } else {
      for (int i = 0; i < words; ++i) {
        short sample = ((short *)(input + 0x1c))[i];
        *(short *)(r->decoded_ring_current + i * 4) = sample;
        *(short *)(r->decoded_ring_current + i * 4 + 2) = sample;
      }
    }
    byte_count = output_bytes;
  } else if (r->format_dispatch_60 == 7) {
    r->decoder_state_3d8 = *(short *)(input + 0x1c);
    r->decoder_state_3e0 = *(short *)(input + 0x1e);
    r->decoder_state_3dc = *(short *)(input + 0x20);
    r->decoder_state_3e4 = *(short *)(input + 0x22);
    int encoded = byte_count - 0xc;
    int cursor = 0;
    unsigned int output = (unsigned int)r->decoded_ring_current;
    if (r->format_parameter_54 == 2) {
      int count = encoded / 30;
      output_bytes = count * 0x70;
      if (r->decoded_ring_current + output_bytes >= r->decoded_ring_base + 0x493e0)
        r->decoded_ring_current = r->decoded_ring_base;
      output = (unsigned int)r->decoded_ring_current;
      sub_5cdd10(count, &cursor, &r->decoder_state_3d8,
                 &r->decoder_state_3e0, &r->decoder_state_3dc,
                 &r->decoder_state_3e4, (int)(input + 0x24), &output);
      byte_count = output_bytes;
    } else {
      int count = encoded / 15;
      output_bytes = count * 0x70;
      if (r->decoded_ring_current + output_bytes >= r->decoded_ring_base + 0x493e0)
        r->decoded_ring_current = r->decoded_ring_base;
      output = (unsigned int)r->decoded_ring_current;
      sub_5cdb90(count, &cursor, &r->decoder_state_3d8,
                 &r->decoder_state_3e0, (int)(input + 0x24), &output);
      byte_count = output_bytes;
    }
  } else if (r->format_dispatch_60 == 0x0a) {
    output_bytes = byte_count * 4;
    if (r->decoded_ring_current + output_bytes >= r->decoded_ring_base + 0x493e0)
      r->decoded_ring_current = r->decoded_ring_base;
    sub_5ce6e3(input + 0x14, (short *)r->decoded_ring_current, byte_count,
               &r->decoder_state_3e8, &r->decoder_state_3f0);
    byte_count = output_bytes;
  } else {
    byte_count = *(int *)(input + 0x14);
  }

  if (r->channel_state == 2) r->descriptor->stream_gate_80 = 1;
  if (r->descriptor->stream_gate_80 != 0)
    InterlockedIncrement(&r->read_counter);
  int slot = r->output_producer_cursor % 24;
  r->output_records[slot].payload = r->decoded_ring_current;
  r->output_records[slot].field_04 = (DWORD)byte_count;
  r->output_records[slot].field_08 = (DWORD)r->read_counter;
  r->output_records[slot].field_0c = (DWORD)r->output_counter;
  r->decoded_ring_current += byte_count;
  InterlockedIncrement(&r->output_producer_cursor);
  InterlockedIncrement((LONG *)&r->pending_output_records);
  InterlockedIncrement(&r->output_counter);
  return 2;
}

// ORIGINAL: 0x005CA970 sub_5ca970 0x005CA970-0x005CAAA1 BYTE_EXACT
// symbol    @sub_5ca970@4
// kind      game
extern "C" void __fastcall sub_5ca970(MediaRuntime *runtime) {
  MediaDescriptor &d = *runtime->descriptor;
  LONG no_handle = -1;
  if (runtime->channel_state == no_handle) {
    if ((d.flags & 0x00008000) != 0) {
      Sleep(0);
      mmioSeek(runtime->descriptor->stream, runtime->channel_seek_position, SEEK_SET);
      runtime->channel_state = sub_5c9ec0(runtime);
    } else {
      runtime->channel_state = no_handle;
    }
  }

  if (runtime->channel_state == no_handle) return;
  if (runtime->pending_output_records >= 0x16 ||
      runtime->pending_read_records >= 0x16)
    return;

  int result = sub_5c9ec0(runtime);
  runtime->channel_poll_result = result;
  switch (result) {
  case -1:
    if ((runtime->descriptor->flags & 0x00008000) != 0) {
      Sleep(0);
      mmioSeek(runtime->descriptor->stream, runtime->channel_seek_position, SEEK_SET);
      runtime->channel_state = sub_5c9ec0(runtime);
    } else {
      runtime->channel_state = no_handle;
    }
    return;

  case 1:
    if (runtime->descriptor->stream_gate_7c != 0) {
      runtime->channel_state = sub_5c9ec0(runtime);
      if (runtime->channel_state == 1) runtime->descriptor->stream_gate_7c = 0;
    }
    return;

  case 2:
    if (runtime->descriptor->stream_gate_80 == 0 ||
        runtime->pending_output_records < 0x16)
      runtime->channel_state = sub_5c9ec0(runtime);
    return;

  default:
    return;
  }
}

// ORIGINAL: 0x005CE340 sub_5ce340 0x005CE340-0x005CE44B
// symbol    ?setup_sound_device@MediaRuntime@@QAEHXZ
// kind      game
// LEVER: THIS IS DIRECTDRAW, NOT DIRECTSOUND - the earlier note's owner guess was wrong on the API. The four indirect calls are IDirectDraw vtable slots on the object at [ebx]+0x54: +0x08 Release, +0x10 GetCaps, +0x18 SetCooperativeLevel (push 2 = DDSCL_FULLSCREEN| DDSCL_EXCLUSIVE, push edi = the HWND at [ebx]+0x9c). The direct call at 0x5CE376 is 0x6501F0, the import thunk for DDRAW.dll's DirectDrawCreate (IAT 0x00669018, tools/iat_names.py) - so the 'field at +4' whose 0x800000 bit gates the body is a DirectDraw capability dword, and [ebx]+0x3cc is the 'DirectDraw is up' flag. The owner is therefore a video/window object in the Win family, not a sound one; a naming pass should start from win.h's DDInit, whose surf_ field holds the same IDirectDraw.
// TRIED: not chased to byte-exact - a DirectSound setup routine with four indirect (COM vtable, __stdcall) calls through an unidentified owner object (fields at +4, +0x54, +0x58, +0x9c, none matching a class already in src/). MISMATCH from instruction 0 (267 vs 253 bytes); landed for the control flow and call-order shape - each `if`/`else` branch and the shared "zero the slot" merge point match the disassembly's block structure, including the fact that a successful SetCooperativeLevel-equivalent (slot 6) skips both the Release-equivalent (slot 2) call AND the slot zeroing.
// The preceding DirectDraw LEVER interpretation is contradicted by the
// pinned PE: 0x005CE376 calls 0x006501F0, which jumps through 0x00669034,
// the DSOUND.dll DirectSoundCreate import. The old 0x00669018 identification
// was wrong. Preserve that seed history, but use the actual SDK interface.
int MediaRuntime::setup_sound_device() {
  MediaDescriptor *entry_descriptor = descriptor;
  if ((entry_descriptor->flags & 0x00800000) == 0) return 0;

  IDirectSound **device = &entry_descriptor->sound_device;
  if (*device == 0) {
    release_sound_device = 1;
    HWND window = entry_descriptor->window;
    if (DirectSoundCreate(0, device, 0) != DS_OK) {
      *device = 0;
    } else if ((*device)->SetCooperativeLevel(window, DSSCL_PRIORITY) != DS_OK) {
      (*device)->Release();
      *device = 0;
    }
  }

  MediaDescriptor *current_descriptor = descriptor;
  if (current_descriptor->sound_device != 0) {
    DSCAPS device_caps;
    memset(&device_caps, 0, 0x60);
    device_caps.dwSize = 0x60;
    descriptor->sound_device->GetCaps(&device_caps);
    emulated_device = (device_caps.dwFlags & DSCAPS_EMULDRIVER) != 0;

    if ((descriptor->flags & 0x00400000) != 0) {
      IDirectSoundBuffer *source_buffer = descriptor->source_buffer_58;
      if (source_buffer != 0) {
        DSBCAPS source_caps;
        memset(&source_caps, 0, 0x14);
        source_caps.dwSize = 0x14;
        source_buffer->GetCaps(&source_caps);
        source_buffer_bytes = source_caps.dwBufferBytes;
        return 0;
      }
    }
    source_buffer_bytes = 0x8000;
    return 0;
  }
  current_descriptor->source_buffer_58 = 0;
  return 0;
}

// ORIGINAL: 0x005CE4B0 sub_5ce4b0 0x005CE4B0-0x005CE61A
// symbol    ?create_playback_buffer@MediaRuntime@@QAEHXZ
// kind      game
// TRIED: `ecx` does real work with no stack access at entry (reads `[ecx]` before any `[ebp+..]`), so this is a __thiscall member - written as `ClassX::sub_5ce4b0()`, same precedent as 0x004E3300's ClassX::sub_4e3300. The six indirect calls are DirectDraw-COM-shaped (self pushed explicitly alongside the args, not via ECX), spelled with `__stdcall` function-pointer typedefs per call site rather than a shared VCall shim, since two calls land on offset 0xc through DIFFERENT vtables (one via `[this+0x54]`, two via `this->[0]->0x5c`) with different arities (4 args vs 2). Local DDSURFACEDESC/DDPIXELFORMAT-like scratch structs are modelled as plain structs, not the overlapping/self-referential stack layout the original build (a nested pointer field pointing back into the same frame) - not chased further. 0.83 mnemonic similarity; first divergence at #3 is the prologue register-save order, which follows from local-variable layout this body does not reproduce.
int MediaRuntime::create_playback_buffer() {
  if (descriptor->sound_device != 0) {
    DSBUFFERDESC buffer_desc;
    memset(&buffer_desc, 0, 0x14);
    buffer_desc.dwSize = 0x14;
    buffer_desc.dwFlags = 0x000100e0;
    buffer_desc.dwBufferBytes = source_buffer_bytes;

    WAVEFORMATEX format;
    buffer_desc.lpwfxFormat = &format;
    memset(&format, 0, sizeof(format));
    format.wFormatTag = WAVE_FORMAT_PCM;
    format.nChannels = 2;
    format.nSamplesPerSec = playback_frequency;
    format.nBlockAlign = 4;
    format.nAvgBytesPerSec = playback_frequency << 2;
    format.wBitsPerSample = 16;
    if (descriptor->sound_device->CreateSoundBuffer(&buffer_desc, &descriptor->play_buffer, 0) == DS_OK) {
      descriptor->play_buffer->SetFrequency(playback_frequency);

      DSBCAPS created_caps;
      memset(&created_caps, 0, 0x14);
      created_caps.dwSize = 0x14;
      if ((descriptor->flags & 0x2000) != 0) descriptor->play_buffer->SetVolume(descriptor->volume_argument);
      if (descriptor->play_buffer->GetCaps(&created_caps) == DS_OK) {
        synthetic_cursor_mode = 0;
        playback_buffer_bytes = created_caps.dwBufferBytes;
        playback_cursor = 0;
        if (descriptor->play_buffer->Play(0, 0, DSBPLAY_LOOPING) == DS_OK) {
          buffer_setup_finished = 1;
          return 0;
        }
      }
      descriptor->play_buffer->Release();
    }
  }
  descriptor->play_buffer = 0;
  synthetic_cursor_mode = 1;
  synthetic_playback_cursor = 0;
  playback_buffer_bytes = 0x8000;
  playback_cursor = 0;
  buffer_setup_finished = 1;
  return 0;
}

// ORIGINAL: 0x005CE450 sub_5ce450 0x005CE450-0x005CE4A3 BYTE_EXACT
// symbol    ?release_session_interfaces@MediaRuntime@@QAEHXZ
// kind      game
int MediaRuntime::release_session_interfaces() {
  const DWORD descriptor_flags = descriptor->flags;
  if ((descriptor_flags & 0x00800000) != 0) {
    if (release_optional_buffer != 0) {
      MediaDescriptor *source_owner = descriptor;
      if (source_owner->source_buffer_58 != 0) {
        IDirectSoundBuffer *source_buffer = source_owner->source_buffer_58;
        source_buffer->Release();
        // Native 5CE475 reloads runtime->descriptor after Release.
        descriptor->source_buffer_58 = 0;
      }
    }
    if (release_sound_device != 0) {
      // This load follows the optional-buffer Release and therefore observes
      // any descriptor replacement performed by that call.
      MediaDescriptor *device_owner = descriptor;
      IDirectSound **sound_device_slot = &device_owner->sound_device;
      if (*sound_device_slot != 0) {
        IDirectSound *sound_device = *sound_device_slot;
        sound_device->Release();
        *sound_device_slot = 0;
      }
    }
  }
  return 0;
}

// ORIGINAL: 0x005CE620 sub_5ce620 0x005CE620-0x005CE63F BYTE_EXACT
// symbol    ?release_playback_buffer@MediaRuntime@@QAEHXZ
// kind      game
int MediaRuntime::release_playback_buffer() {
  MediaDescriptor *playback_owner = descriptor;
  if (playback_owner->play_buffer != 0) {
    IDirectSoundBuffer *playback_buffer = playback_owner->play_buffer;
    playback_buffer->Release();
    // Native 5CE632 reloads runtime->descriptor before the 5CE634 store.
    descriptor->play_buffer = 0;
  }
  return 0;
}

// ORIGINAL: 0x005CAE00 sub_5cae00 0x005CAE00-0x005CAE79 BYTE_EXACT
// symbol    ?sub_5cae00@MediaRuntime@@QAEHXZ
// kind      game
// `inline`: sub_5cae80 inlines this body; sub_5caab0 calls it.
inline int MediaRuntime::sub_5cae00() {
  MediaDescriptor *stream_owner = descriptor;
  if ((stream_owner->flags & 0x00080000) == 0 && stream_owner->stream != 0)
    mmioClose(stream_owner->stream, 0);
  if (input_ring_base != 0) free(input_ring_base);
  if (decoded_ring_base != 0) free(decoded_ring_base);
  for (int i = 0; i != 24; ++i) read_records[i].payload = 0;
  // Native 5CAE58 reloads runtime->descriptor after close/free and record
  // clearing; the gate is read from that current descriptor.
  MediaDescriptor *output_owner = descriptor;
  if (output_owner->stream_gate_7c != 0)
    for (int j = 0; j != 24; ++j) output_records[j].payload = 0;
  return 0;
}

// ORIGINAL: 0x005CC710 sub_5cc710 0x005CC710-0x005CC856 BYTE_EXACT
// symbol    ?sub_5cc710@MediaRuntime@@QAEXXZ
// kind      game
// TRIED: the four vtable-slot-2 calls read `push piVar1; mov ecx, [piVar1]; call dword ptr [ecx+8]` - a plain function-pointer call taking one explicit pointer argument, NOT a thiscall dispatch (the receiver never lands in ecx before the call), so these are cdecl function-pointer casts, not virtual calls through an introduced class. `sub_5cbae0` similarly cannot be called by name (catalogued unmangled with no CRT_SIGNATURES entry, so declfix leaves it nullary - C2660 direct, C2733 redeclared) and the real call passes `this` in ecx and a literal 0 in edx with no stack push, i.e. __fastcall(void*, int); routed through a __fastcall function-pointer cast. Best reached: MISMATCH, edit_count 4, 338 bytes vs 326, mnemonic similarity ~0.98.
void MediaRuntime::sub_5cc710() {
  if (teardown_gate_3c != 0) {
    if (decoded_pixel_blocks != 0) { free(decoded_pixel_blocks); decoded_pixel_blocks = 0; }
    if (motion_coordinates != 0) { free(motion_coordinates); motion_coordinates = 0; }
    if (motion_pixel_offsets != 0) { free(motion_pixel_offsets); motion_pixel_offsets = 0; }
    if (owned_heap_638 != 0) { free(owned_heap_638); owned_heap_638 = 0; }
    if (owned_heap_63c != 0) { free(owned_heap_63c); owned_heap_63c = 0; }
    if (motion_row_offsets != 0) { free(motion_row_offsets); motion_row_offsets = 0; }
    if (motion_block_indices != 0) { free(motion_block_indices); motion_block_indices = 0; }

    if (queue_record_kind == 2) update_overlay(0);

    if (descriptor->palette_24 != 0 && (descriptor->flags & 0x04000000) == 0) {
      descriptor->palette_24->Release();
      descriptor->palette_24 = 0;
    }
    if (descriptor->surface_20 != 0 && (descriptor->flags & 0x00000800) == 0) {
      descriptor->surface_20->Release();
      descriptor->surface_20 = 0;
    }
    if (descriptor->surface_1c != 0 && (descriptor->flags & 0x00000800) == 0) {
      descriptor->surface_1c->Release();
      descriptor->surface_1c = 0;
    }
    if (release_interface_10 != 0) {
      descriptor->draw_device_10->Release();
      descriptor->draw_device_10 = 0;
      release_interface_10 = 0;
    }
    teardown_gate_3c = 0;
  }
}

// ORIGINAL: 0x005CB6C0 sub_5cb6c0 0x005CB6C0-0x005CBAD1
// symbol    ?sub_5cb6c0@MediaRuntime@@QAEXXZ
// kind      game
// TRIED: the generated VCall shim (thiscall dispatch) - every indirect call here pushes the receiver explicitly before `call [reg+off]` (confirmed on all 16 call sites), so COM-style raw vtable-slot casts were used instead; case1/case2/case4 bodies transcribed from the disassembly (Ghidra under-reports some arg counts, e.g. slot 0x14/0x58/0x80 calls, corrected from raw asm). Diverges at instruction #0 (prologue register allocation).
void MediaRuntime::sub_5cb6c0() {
  DWORD flags = descriptor->flags;
  BYTE *palette_entries = descriptor->palette_entries_28;
  if ((flags & 0x00000800) != 0) return;

  switch (queue_record_kind) {
  case 1: {
    if ((flags & 0x00000400) != 0) break;
    IDirectDrawSurface2 *target =
        (flags & 0x08000000) != 0 ? descriptor->surface_18 : descriptor->surface_14;
    if (descriptor->pixel_format.total_component_bits == 8) {
      BYTE *out = palette_entries;
      for (int i = 0; i < 0x300; i += 3, out += 4) {
        out[0] = descriptor->palette_rgb_triplets_b0[i];
        out[1] = descriptor->palette_rgb_triplets_b0[i + 1];
        out[2] = descriptor->palette_rgb_triplets_b0[i + 2];
        out[3] = 0;
      }
    }

    RECT source;
    source.left = 0;
    source.top = 0;
    source.right = (LONG)(descriptor->output_width_64 - 1);
    source.bottom = (LONG)(descriptor->output_height_60 - 1);
    RECT destination;
    destination.left = (LONG)output_rect_left_14;
    destination.top = (LONG)output_rect_top_18;
    destination.right = (LONG)(output_rect_left_14 + output_rect_extent_0c);
    destination.bottom = (LONG)(output_rect_top_18 + output_rect_height_10);
    if (target->IsLost() == DDERR_SURFACELOST && target->Restore() != DD_OK)
      return;

    DWORD row;
    if ((descriptor->flags & 0x00000020) != 0) {
      for (row = 0; row < descriptor->output_height_60; row += 2) {
        destination.top = (LONG)(output_rect_top_18 + row);
        destination.bottom = (LONG)(output_rect_top_18 + row + 1);
        source.top = (LONG)(row >> 1);
        source.bottom = (LONG)((row >> 1) + 1);
        target->Blt(&destination, descriptor->surface_1c, &source, DDBLT_WAIT, 0);
      }
    } else if ((descriptor->flags & 0x00000010) != 0) {
      for (row = 0; row < descriptor->output_height_60; row += 2) {
        destination.top = (LONG)(output_rect_top_18 + row);
        destination.bottom = (LONG)(output_rect_top_18 + row + 1);
        source.top = (LONG)(row >> 1);
        source.bottom = (LONG)((row >> 1) + 1);
        target->Blt(&destination, descriptor->surface_1c, &source, DDBLT_WAIT, 0);
        destination.top = (LONG)(row + output_rect_top_18 + 1);
        destination.bottom = (LONG)(row + output_rect_top_18 + 2);
        target->Blt(&destination, descriptor->surface_1c, &source, DDBLT_WAIT, 0);
      }
    } else {
      target->Blt(&destination, descriptor->surface_1c, &source, DDBLT_WAIT, 0);
    }
    break;
  }
  case 2:
    if (descriptor->surface_1c != 0) {
      if (output_handle_candidate != 0)
        output_handle_candidate = (DWORD)update_overlay(1);
      DDSURFACEDESC locked;
      memset(&locked, 0, sizeof(locked));
      locked.dwSize = sizeof(locked);
      if (descriptor->surface_1c->Lock(0, &locked, DDLOCK_WAIT, 0) == DD_OK) {
        DWORD *destination_row = (DWORD *)locked.lpSurface;
        DWORD row_dwords = descriptor->output_width_64 * 2 / 4;
        DWORD *source_row = (DWORD *)owned_heap_638;
        int destination_pitch = locked.lPitch / 4;
        for (DWORD row = 0; row < descriptor->output_height_60;
             ++row, destination_row += destination_pitch, source_row += row_dwords) {
          for (DWORD i = 0; i < row_dwords; i += 8) {
            destination_row[i] = source_row[i];
            destination_row[i + 1] = source_row[i + 1];
            destination_row[i + 2] = source_row[i + 2];
            destination_row[i + 3] = source_row[i + 3];
            destination_row[i + 4] = source_row[i + 4];
            destination_row[i + 5] = source_row[i + 5];
            destination_row[i + 6] = source_row[i + 6];
            destination_row[i + 7] = source_row[i + 7];
          }
        }
        descriptor->surface_1c->Unlock(0);
      }
    }
    break;
  case 4: {
    if ((flags & 0x00001000) != 0)
      descriptor->draw_device_10->WaitForVerticalBlank(DDWAITVB_BLOCKBEGIN, 0);
    RECT source;
    source.left = 0;
    source.top = 0;
    source.right = (LONG)(descriptor->output_width_64 - 1);
    source.bottom = (LONG)(descriptor->output_height_60 - 1);
    RECT destination;
    destination.left = (LONG)output_rect_left_14;
    destination.top = (LONG)output_rect_top_18;
    destination.right = (LONG)(output_rect_left_14 + output_rect_extent_0c);
    destination.bottom = (LONG)(output_rect_top_18 + output_rect_height_10);
    if (descriptor->surface_1c->IsLost() == DDERR_SURFACELOST &&
        descriptor->surface_1c->Restore() != DD_OK)
      return;
    if (descriptor->surface_14->Blt(&destination, descriptor->surface_1c,
                                    &source, DDBLT_WAIT, 0) != DD_OK)
      queue_record_kind = 1;
    break;
  }
  }

  if ((descriptor->flags & 0x08000000) != 0 && queue_record_kind == 1) {
    if (descriptor->pixel_format.total_component_bits == 8) {
      descriptor->surface_14->Flip(0, DDFLIP_WAIT);
      descriptor->palette_24->SetEntries(0, 0, 0x100,
                                         (LPPALETTEENTRY)palette_entries);
      return;
    }
    HRESULT result = descriptor->surface_14->Flip(0, DDFLIP_WAIT);
    if (result != DD_OK) OutputDebugStringA(sub_5cf120(result));
  }
}

/*
Purpose: After a read-record event, update the associated media descriptor.

             mov eax,[ecx] / mov edx,[eax+0x20] / test edx,edx / je second
             cmp [ecx+0x4c],1 / jne second
             mov esi,[eax+0x1c] / mov [eax+0x1c],edx / mov eax,[ecx]
             mov [eax+0x20],esi
     second: mov eax,[ecx] / mov edx,[eax+4] / test dh,8 / je end
             mov esi,[eax+0x4c] / mov edx,[eax+0x50] / mov [eax+0x50],esi
             mov ecx,[ecx] / mov [ecx+0x4c],edx / ret

         ECX is MediaRuntime* and [ECX] is its MediaDescriptor*. The first
         swap is gated by descriptor+0x20 being nonzero and runtime+0x4c
         equaling one; the second independently tests descriptor flags bit
         11, then swaps the alternating output-buffer pointers at +0x4c/+0x50.

         The original reloads [ECX] between stores in both groups. The typed
         owner keeps a descriptor reload at each corresponding point.

// ORIGINAL: 0x005CBBC0 sub_5cbbc0 0x005CBBC0-0x005CBBF5 FILE BYTE_EXACT
// TRIED: plateaus at MNEMONIC_ONLY 13/22 - re-reading `load32(node, 0x20)`
//            inside the guard instead of caching it in `moved` scores the
//            same (measured); the divergence is edx-vs-esi register choice
//            at the very first load, before any branch, so it is register
//            allocation rather than a source shape this tree controls.
// symbol    ?sub_5cbbc0@MediaRuntime@@QAEXXZ
// size      53 bytes
// prototype
// callers   2   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void MediaRuntime::sub_5cbbc0() {
    // The first word of MediaRuntime is its descriptor pointer. Pinned
    // 0x5CBBC0 swaps descriptor +0x1c/+0x20 only when queue kind is 1,
    // then independently swaps +0x4c/+0x50 when descriptor flags contain
    // bit 11. Both are output buffers: the next conversion writes through
    // +4c after this swap. Neither address is a pitch or a capacity.
    MediaDescriptor *view = descriptor;
    if (view->surface_20 != 0 && queue_record_kind == 1) {
        IDirectDrawSurface2 *previous = view->surface_1c;
        view->surface_1c = view->surface_20;
        view = descriptor;
        view->surface_20 = previous;
    }
    view = descriptor;
    if ((view->flags & 0x800U) != 0) {
        BYTE *previous_alternate = view->provided_output_alternate_50;
        view->provided_output_alternate_50 = view->provided_output_base_4c;
        view = descriptor;
        view->provided_output_base_4c = previous_alternate;
    }
}

// ORIGINAL: 0x005CB220 sub_5cb220 0x005CB220-0x005CB60B
// symbol    ?sub_5cb220@MediaDescriptor@@QAEXXZ
// kind      game
// TRIED: __thiscall member (class Sub5cb220Obj) with `this` reassigned to *(int**)this, matching Ghidra's structure closely; BOTH indirect calls (slots 9 and 12) are COM-style (interface pushed as an explicit stack arg, not passed in ecx) despite the brief's emitter flagging only slot 9 - confirmed by the raw asm's "push eax; call [ecx+0x30]" shape for slot 12 too, per LEVER 3. MISMATCH #0 (frame layout), not chased further.
#pragma inline_depth(0)
void MediaDescriptor::sub_5cb220() {
  // Every field goes through r->descriptor, reloaded at each use.
  MediaRuntime *r = runtime;
  if ((r->descriptor->flags & 0x00030000) == 0x00030000)
    SetThreadPriority(GetCurrentThread(), (int)r->descriptor->priority);
  if (r->sub_5ce2b0() != 0) goto finish_worker;

  r->descriptor->progress_value = 0;
  InterlockedExchange(&r->descriptor->state, 1);
  r->start_ticks = GetTickCount();
  for (;;) {
    LONG observed = InterlockedExchange(&r->descriptor->state, 2);
    MediaQueueRecord *record = &r->read_records[r->read_consumer_cursor % 24];
    if (observed == 3) {
      InterlockedExchange(&r->descriptor->state, observed);
      do {
        Sleep(20);
        observed = InterlockedExchange(&r->descriptor->state, 3);
      } while (observed == 3);
      InterlockedExchange(&r->descriptor->state, observed);
    }
    if (observed == 4) break;
    if (observed != 2) continue;

    Sleep(0);
    if (r->channel_state == -1) {
      if ((r->descriptor->flags & 0x00008000) != 0) {
        Sleep(0);
        mmioSeek(r->descriptor->stream, r->channel_seek_position, SEEK_SET);
        r->channel_state = sub_5c9ec0(r);
      } else {
        r->channel_state = -1;
      }
      if (r->channel_state == -1) goto stream_done;
    }
    if (r->pending_output_records < 0x16 &&
        r->pending_read_records < 0x16) {
      int result = sub_5c9ec0(r);
      r->channel_poll_result = result;
      switch (result) {
      case -1:
        if ((r->descriptor->flags & 0x00008000) != 0) {
          Sleep(0);
          mmioSeek(r->descriptor->stream, r->channel_seek_position, SEEK_SET);
          r->channel_state = sub_5c9ec0(r);
        } else {
          r->channel_state = -1;
        }
        break;
      case 1:
        if (r->descriptor->stream_gate_7c != 0) {
          r->channel_state = sub_5c9ec0(r);
          if (r->channel_state == 1) r->descriptor->stream_gate_7c = 0;
        }
        break;
      case 2:
        if (r->descriptor->stream_gate_80 == 0 ||
            r->pending_output_records < 0x16)
          r->channel_state = sub_5c9ec0(r);
        break;
      }
    }
  stream_done:
    if (r->buffer_setup_finished == 0 &&
        r->descriptor->stream_gate_7c != 0) {
      r->create_playback_buffer();
      r->output_lock_word = 0;
    }
    if (record->payload != 0) {
      if (r->descriptor->progress_value <= 2) {
        sub_5cc940(r, record);
        r->sub_5cb6c0();
      } else {
        DWORD kind = record->field_0c;
        if (kind == 2 || kind == 1 || kind == 6 || kind == 5)
          sub_5cc940(r, record);
      }
      if ((r->descriptor->flags & 0x00040000) != 0 && r->descriptor->callback != 0 &&
          r->descriptor->callback() != 0)
        break;
      record->payload = 0;
      record->field_04 = 0;
      InterlockedIncrement(&r->read_consumer_cursor);
      InterlockedDecrement((LONG *)&r->pending_read_records);
      r->sub_5cbbc0();
    }

    InterlockedIncrement(&r->descriptor->progress_counter);
    if (r->pending_read_records == 0) {
      if (r->descriptor->stream_gate_7c == 0 ||
          (r->descriptor->flags & 0x00800000) == 0)
        break;
      if (r->pending_output_records == 0) {
        IDirectSoundBuffer *buffer = r->descriptor->play_buffer;
        if (buffer == 0) break;
        DWORD status;
        buffer->GetStatus(&status);
        if ((status & 0x4) != 0) r->descriptor->play_buffer->Play(0, 0, 0);
        if ((status & 0x1) == 0) break;
      }
    }
    if (r->descriptor->stream_gate_7c != 0) {
      // The image keeps both values in stack slots and reloads them.
      int attempts = 0;
      volatile int lag = (int)r->output_lock_word - 5;
      volatile LONG base = r->descriptor->progress_counter;
      while (base - lag >= 1) {
        Sleep(0);
        Sleep(3);
        lag = (int)r->output_lock_word - 5;
        if (attempts++ > 0x96) break;
      }
      r->descriptor->progress_value = lag - base;
      if (r->descriptor->progress_value < 0) r->descriptor->progress_value = 0;
    }
  }

  r->end_ticks = GetTickCount();
  r->sub_5ce2f0();
finish_worker:
  r->release_playback_buffer();
  InterlockedExchange(&r->descriptor->state, 5);
}
#pragma inline_depth(8)

// ORIGINAL: 0x005CAE80 sub_5cae80 0x005CAE80-0x005CB046
// symbol    _sub_5cae80
// kind      game
// TRIED: 0x005CB050's scaffolded prototype takes only one fastcall param, but the real callee reads its object from EDX (`mov esi, edx` at its own entry) with ECX dead - called here through an explicit fastcall(LPSTR, int*) function-pointer cast rather than by name, since a second extern "C" declaration for the same symbol conflicts. The four thiscall callees (sub_5ce340/sub_5cbe30/sub_5cb220/sub_5ce450/ sub_5cc710) are similarly unreachable by name (VC6 C4234 on a free __thiscall) and go through the Dummy member-function- pointer trick. Landing the closest control-flow-faithful form (divergence starts at instruction #12, inside the calloc/zero preamble).
// TRIED: 0.994 left: the image's free path falls into the shared `return 0`
//        exit block; VC6 here duplicates it (xor before the pops). Nested
//        if/else, goto-done and an inverted init test measured no better.
extern "C" MediaDescriptor *__cdecl sub_5cae80(
    LPSTR filename, HWND window, MediaDescriptor *descriptor) {
  MediaRuntime *runtime = (MediaRuntime *)calloc(1, 0x660);
  if (runtime == 0) return 0;
  memset(runtime, 0, 0x660); // native body explicitly repeats calloc zeroing
  runtime->descriptor = descriptor;
  descriptor->window = window;
  if (MediaRuntime::initialize_stream(filename, runtime) != 0) {
    free(runtime);
    return 0;
  }
  descriptor->runtime = runtime;
  runtime->setup_sound_device();
  runtime->sub_5cbe30();
  InterlockedExchange(&runtime->descriptor->state, 0);
  if (descriptor == 0) return 0; // native check follows earlier dereferences

  descriptor->runtime->protocol_gate = 1; // before either sync or async branch
  if ((descriptor->flags & 0x00010000) != 0) {
    uintptr_t thread = _beginthread(sub_5caab0, 0, descriptor);
    descriptor->beginthread_result = (DWORD)thread;
    if (thread == (uintptr_t)-1) return 0; // native path does not free runtime
    LONG *state = &descriptor->state;
    if (InterlockedExchange(state, 1) != 2) {
      do {
        Sleep(20);
      } while (InterlockedExchange(state, 1) != 2);
    }
    InterlockedExchange(state, 2);
    return descriptor;
  }

  // Synchronous playback. The image reads the background flag through the
  // runtime's descriptor before the play loop (0x005CAF5B), inlines
  // sub_5cae00, and ends a background caller's thread like sub_5caab0.
  runtime = descriptor->runtime;
  DWORD background_flag = runtime->descriptor->flags & 0x00010000;
  descriptor->sub_5cb220();
  if (runtime->protocol_gate != 0) {
    MediaRuntime *r = descriptor->runtime;
    r->descriptor->elapsed_ticks = r->end_ticks - r->start_ticks;
    r->sub_5cae00();
    r->release_session_interfaces();
    r->sub_5cc710();
    InterlockedExchange(&r->descriptor->state, 6);
    InterlockedExchange((LONG *)&r->descriptor->runtime, 0);
    free(r);
  }
  if (background_flag != 0) _endthread();
  return 0;
}

// ORIGINAL: 0x005CB610 sub_5cb610 0x005CB610-0x005CB652 BYTE_EXACT
// symbol    _sub_5cb610
// kind      game
extern "C" int __cdecl sub_5cb610(MediaDescriptor *d) {
  if (d->runtime != 0) {
    const volatile LONG &observed_state = d->state;
    if (d->runtime->protocol_gate != 0) return observed_state != 6;
    if (observed_state == 2 || observed_state == 3) return 1;
  }
  return 0;
}

// ORIGINAL: 0x005CB660 sub_5cb660 0x005CB660-0x005CB6BF BYTE_EXACT
// symbol    _sub_5cb660
// kind      game
extern "C" int __cdecl sub_5cb660(MediaDescriptor *d) {
  MediaRuntime *runtime = d->runtime;
  DWORD protocol_gate = 0;
  if (runtime != 0) protocol_gate = runtime->protocol_gate;
  InterlockedExchange(&d->state, 4);
  const volatile LONG &observed_state = d->state;
  if (protocol_gate != 0) {
    while (observed_state != 6) Sleep(10);
  } else {
    while (observed_state != 5) Sleep(10);
  }
  return 0;
}

static __forceinline void media_worker_loop(MediaDescriptor *self) {
  MediaRuntime *r = self->runtime;
  int (MediaRuntime::*const start_output)() = &MediaRuntime::sub_5ce2b0;
  int (MediaRuntime::*const stop_output)() = &MediaRuntime::sub_5ce2f0;
  int (MediaRuntime::*const release_buffer)() = &MediaRuntime::release_playback_buffer;
  void (MediaRuntime::*const swap_buffers)() = &MediaRuntime::sub_5cbbc0;
  if ((r->descriptor->flags & 0x00030000) == 0x00030000)
    SetThreadPriority(GetCurrentThread(), (int)r->descriptor->priority);

  if ((r->*start_output)() == 0) {
    r->descriptor->progress_value = 0;
    InterlockedExchange(&r->descriptor->state, 1);
    r->start_ticks = GetTickCount();
    for (;;) {
      LONG observed = InterlockedExchange(&r->descriptor->state, 2);
      MediaQueueRecord *record = &r->read_records[r->read_consumer_cursor % 24];
      if (observed == 3) {
        InterlockedExchange(&r->descriptor->state, observed);
        do {
          Sleep(20);
          observed = InterlockedExchange(&r->descriptor->state, 3);
        } while (observed == 3);
        InterlockedExchange(&r->descriptor->state, observed);
      }
      if (observed == 4) break;
      if (observed != 2) continue;

      Sleep(0);
      sub_5ca970(r);
      if (r->buffer_setup_finished == 0 &&
          r->descriptor->stream_gate_7c != 0) {
        r->create_playback_buffer();
        r->output_lock_word = 0;
      }
      if (record->payload != 0) {
        if (r->descriptor->progress_value <= 2) {
          sub_5cc940(r, record);
          r->sub_5cb6c0();
        } else {
          DWORD kind = record->field_0c;
          if (kind == 2 || kind == 1 || kind == 6 || kind == 5)
            sub_5cc940(r, record);
        }
        if ((r->descriptor->flags & 0x00040000) != 0 && r->descriptor->callback != 0 &&
            r->descriptor->callback() != 0)
          break;
        record->payload = 0;
        record->field_04 = 0;
        InterlockedIncrement(&r->read_consumer_cursor);
        InterlockedDecrement((LONG *)&r->pending_read_records);
        (r->*swap_buffers)();
      }

      InterlockedIncrement(&r->descriptor->progress_counter);
      if (r->pending_read_records == 0) {
        if (r->descriptor->stream_gate_7c == 0 ||
            (r->descriptor->flags & 0x00800000) == 0)
          break;
        if (r->pending_output_records == 0) {
          IDirectSoundBuffer *buffer = r->descriptor->play_buffer;
          if (buffer == 0) break;
          DWORD status;
          buffer->GetStatus(&status);
          if ((status & 0x4) != 0) r->descriptor->play_buffer->Play(0, 0, 0);
          if ((status & 0x1) == 0) break;
        }
      }
      if (r->descriptor->stream_gate_7c != 0) {
        // The image keeps both values in stack slots and reloads them.
        int attempts = 0;
        volatile int lag = (int)r->output_lock_word - 5;
        volatile LONG base = r->descriptor->progress_counter;
        while (base - lag >= 1) {
          Sleep(0);
          Sleep(3);
          lag = (int)r->output_lock_word - 5;
          if (attempts++ > 0x96) break;
        }
        r->descriptor->progress_value = lag - base;
        if (r->descriptor->progress_value < 0) r->descriptor->progress_value = 0;
      }
    }
    r->end_ticks = GetTickCount();
    (r->*stop_output)();
  }

  (r->*release_buffer)();
  InterlockedExchange(&r->descriptor->state, 5);
}

// ORIGINAL: 0x005CAAB0 sub_5caab0 0x005CAAB0-0x005CADF9
// symbol    _sub_5caab0
// kind      game
// TRIED: nullary extern "C" __cdecl per the contract head - IDA's own guess (void*) matches the disassembly's [esp+0x14] read, so landed as sub_5caab0(void *param); this is a _beginthread worker (GetCurrentThread/SetThreadPriority/InterlockedXxx/ Sleep/GetTickCount/_endthread). sub_5cXXXXX helpers retyped __fastcall(void*[, void*]) to match their ecx/edx-only call sites (no stack pushes); slots 9/12 are COM-style (explicit self push). Reaches #10 (mismatch inside the flag mask/store sequence); not pursued further given the size.
// Every callee is a real call in the image, sub_5cae00 included.
#pragma inline_depth(1)
extern "C" void __cdecl sub_5caab0(void *argument) {
  // Same shape as MediaDescriptor::sub_5cb220: fields go through
  // r->descriptor, reloaded at each use.
  MediaRuntime *r = ((MediaDescriptor *)argument)->runtime;
  DWORD background_flag = r->descriptor->flags & 0x00010000;
  media_worker_loop((MediaDescriptor *)argument);
  if (r->protocol_gate != 0) {
    MediaRuntime *owner = ((MediaDescriptor *)argument)->runtime;
    owner->descriptor->elapsed_ticks = owner->end_ticks - owner->start_ticks;
    int (MediaRuntime::*const finish_session)() = &MediaRuntime::sub_5cae00;
    (owner->*finish_session)();
    owner->release_session_interfaces();
    owner->sub_5cc710();
    InterlockedExchange(&owner->descriptor->state, 6);
    InterlockedExchange((LONG *)&owner->descriptor->runtime, 0);
    free(owner);
  }
  if (background_flag != 0) _endthread();
}
#pragma inline_depth(8)

// ORIGINAL: 0x005CD750 sub_5cd750 0x005CD750-0x005CD8C5
// TRIED: `extern "C" int __cdecl sub_5cd750()` fallback head; the entry `mov esi,ecx / mov [ebp-4],edx` with no stack reads is __fastcall(int*, uint*), so the signature was changed.
// symbol    @sub_5cd750@8
// kind      game
// Hand-written unaligned word read (asm evidence: push/pop ecx around the
// block and the neg/lea [8*ecx+0x20] shift count are not VC6 output). It
// loads the dword ending at p+2 and shifts the low bytes out, returning EAX;
// inlined under /O2. The asm reads a local copy of the argument.
#pragma warning(disable : 4035)
inline DWORD media_read_word(const BYTE *bytes) {
  const BYTE *p = bytes;
  __asm {
    push ecx
    mov eax, p
    mov ecx, 2
    mov eax, [eax+ecx-4]
    neg ecx
    lea ecx, [8*ecx+0x20]
    shr eax, cl
    pop ecx
  }
}
#pragma warning(default : 4035)

extern "C" void __fastcall sub_5cd750(MediaRuntime *runtime,
                                      BYTE *packet) {
  runtime->descriptor->output_width_64 = media_read_word(packet);
  runtime->descriptor->output_height_60 = media_read_word(packet + 2);
  runtime->unresolved_word_618 = media_read_word(packet + 4);
  runtime->unresolved_word_61c = media_read_word(packet + 6);

  for (int row = 0; row < (int)runtime->unresolved_word_61c; ++row) {
    const DWORD scale = runtime->descriptor->setup_value_78;
    DWORD first = (DWORD)packet[row * 3 + 12] * scale / 100;
    DWORD second = (DWORD)packet[row * 3 + 13] * scale / 100;
    DWORD third = (DWORD)packet[row * 3 + 14] * scale / 100;
    if (first > 0xff) first = 0xff;
    if (second > 0xff) second = 0xff;
    if (third > 0xff) third = 0xff;
    runtime->descriptor->palette_rgb_triplets_b0[row * 3] = (BYTE)first;
    runtime->descriptor->palette_rgb_triplets_b0[row * 3 + 1] = (BYTE)second;
    runtime->descriptor->palette_rgb_triplets_b0[row * 3 + 2] = (BYTE)third;
  }
}

// ORIGINAL: 0x005CD8D0 sub_5cd8d0 0x005CD8D0-0x005CD97B BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned 0x5cd8d0 reloads runtime descriptor after each block helper before the width bound and after each row before the height bound; runtime +0x40 is reread for every block.", "The destination flows to packed byte/pixel writers without a disjointness contract. Output can therefore replace the runtime descriptor pointer or format option; no in-tree deliberate alias is asserted.", "Preserve the original helper branches, unchecked accesses, predictor/input initialization, sixteen-pixel steps and this format's distinct block/row byte strides."], "reason": "Retain live descriptor bounds and per-block format selection across caller-owned pixel output, matching the original sibling decoder loop rather than freezing the owner."}
// TRIED: the given `__stdcall(int,int,int)` head - disasm reads edx with no prior init (a real 2nd register param) and `push ecx` at entry is pure stack reservation, not `this` preservation; rewrote as `__fastcall(int*,int,int,int,uint)` matching Ghidra's 5-param read, with differently-named cdecl externs for the two nullary-scaffolded callees. MISMATCH #4 push/mov, prologue register shape only.
// symbol    @sub_5cd8d0@20
// kind      game
extern "C" void __fastcall sub_5cd8d0(MediaRuntime *runtime,
                                      BYTE *compressed, BYTE *destination,
                                      BYTE *unused, unsigned int pitch) {
  // The second fastcall argument arrives in EDX and is retained in +5F8.
  // The unused fourth logical argument remains present for the native stack
  // cleanup and is deliberately not repurposed.
  runtime->decoder_context_3f8.predictor_500 = 0;
  runtime->decoder_context_3f8.predictor_4fc = 0;
  runtime->decoder_context_3f8.predictor_4f8 = 0;
  runtime->decoder_context_3f8.compressed_input_200 = compressed;
  runtime->decoder_context_3f8.bit_position_204 = 0;
  (void)unused;
  BYTE *row_destination = destination;
  for (DWORD block_y = 0; block_y < runtime->descriptor->output_height_60;
       block_y += 0x10) {
    BYTE *block_destination = row_destination;
    for (DWORD block_x = 0; block_x < runtime->descriptor->output_width_64;
         block_x += 0x10) {
      if (runtime->record_option_040 != 0) {
        sub_5d1466(block_destination, (int)pitch,
                   &runtime->decoder_context_3f8);
      } else {
        sub_5d1842(block_destination, (int)pitch,
                   &runtime->decoder_context_3f8);
      }
      block_destination += 0x20;
    }
    row_destination += pitch / 2 * 32;
  }
}

// ORIGINAL: 0x005CD980 sub_5cd980 0x005CD980-0x005CDA2C BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned 0x5cd980 reloads runtime descriptor after each block helper before the width bound and after each row before the height bound; runtime +0x40 is reread for every block.", "The destination flows to packed byte/pixel writers without a disjointness contract. Output can therefore replace the runtime descriptor pointer or format option; no in-tree deliberate alias is asserted.", "Preserve the original helper branches, unchecked accesses, predictor/input initialization, sixteen-pixel steps and this format's distinct block/row byte strides."], "reason": "Retain live descriptor bounds and per-block format selection across caller-owned pixel output, matching the original sibling decoder loop rather than freezing the owner."}
// symbol    @sub_5cd980@20
// kind      game
extern "C" void __fastcall sub_5cd980(MediaRuntime *runtime,
                                      BYTE *compressed, BYTE *destination,
                                      BYTE *unused, unsigned int pitch) {
  runtime->decoder_context_3f8.predictor_500 = 0;
  runtime->decoder_context_3f8.predictor_4fc = 0;
  runtime->decoder_context_3f8.predictor_4f8 = 0;
  runtime->decoder_context_3f8.compressed_input_200 = compressed;
  runtime->decoder_context_3f8.bit_position_204 = 0;
  (void)unused;
  BYTE *row_destination = destination;
  for (DWORD block_y = 0; block_y < runtime->descriptor->output_height_60;
       block_y += 0x10) {
    BYTE *block_destination = row_destination;
    for (DWORD block_x = 0; block_x < runtime->descriptor->output_width_64;
         block_x += 0x10) {
      if (runtime->record_option_040 != 0) {
        sub_5d1654(block_destination, (int)pitch,
                   &runtime->decoder_context_3f8);
      } else {
        sub_5d1a30(block_destination, (int)pitch,
                   &runtime->decoder_context_3f8);
      }
      block_destination += 0x40;
    }
    row_destination += pitch / 4 * 64;
  }
}

// ORIGINAL: 0x005CDA30 sub_5cda30 0x005CDA30-0x005CDAC4
// RECOVERY_BINDING: v1 {"evidence": ["Pinned 0x5CDA30 reloads runtime descriptor after each 0x5D1278 call before the width test and after the inner loop before the height test.", "The packed-output helper 0x5D2536 writes through the supplied byte destination using memcpy. The flags 0x800 path in 0x5CC940 takes caller-owned provided_output_base_4c without a no-alias contract, so output can alter descriptor storage or the runtime descriptor pointer.", "No in-tree caller is shown deliberately creating that alias; preserve its defined source effects without asserting that normal playback exercises it. Context initialization, pitch/block increments and unchecked dereferences remain unchanged."], "reason": "Retain live descriptor bounds around each pixel-block decode instead of caching a descriptor reference across output stores. This preserves the original permitted alias effects and unchecked loop behavior."}
// TRIED: nothing forced the `push ecx`-as-spill-slot / `pop ecx` prologue trick the original uses for one local; plain locals compile 6 bytes shorter (142 vs 148), diverging early
// symbol    @sub_5cda30@20
// kind      game
extern "C" void __fastcall sub_5cda30(MediaRuntime *runtime,
                                      BYTE *compressed, BYTE *destination,
                                      BYTE *unused, unsigned int pitch) {
  runtime->decoder_context_3f8.compressed_input_200 = compressed;
  runtime->decoder_context_3f8.bit_position_204 = 0;
  runtime->decoder_context_3f8.predictor_500 = 0;
  runtime->decoder_context_3f8.predictor_4fc = 0;
  runtime->decoder_context_3f8.predictor_4f8 = 0;
  (void)unused;
  BYTE *row_destination = destination;
  for (DWORD block_y = 0; block_y < runtime->descriptor->output_height_60;
       block_y += 0x10) {
    BYTE *block_destination = row_destination;
    for (DWORD block_x = 0; block_x < runtime->descriptor->output_width_64;
         block_x += 0x10) {
      sub_5d1278(block_destination, (int)pitch,
                 &runtime->decoder_context_3f8);
      block_destination += 0x20;
    }
    row_destination += pitch / 2 * 32;
  }
}




/*
Purpose: Advance the mmio ring buffer by one read: when the pending offset
         plus this request would run past the 1200000-byte window the offset
         wraps to the window base, mmioRead fills the caller's buffer at the
         current offset, a 0/−1 result reports EOF/error through the +0x398
         flag slot, and a real read publishes (offset, size, sibling count)
         into the 24-entry history ring at +0x1e4 before the offset advances
         and the three counters (ring cursor at +0x37c, sibling at +0x380,
         history count at +0x388) go up through InterlockedIncrement.

// Typed owner implementation; native ABI/output, original reference targets
// and exceptional behavior remain restoration obligations, not owner stubs.
// ORIGINAL: 0x005CA230 sub_5ca230 0x005CA230-0x005CA308
// symbol    ?sub_5ca230@MediaRuntime@@QAIHHH@Z
// size      216 bytes
// prototype int (__fastcall sub_5ca230)(int *ring, int size, int kind)
// callers   1   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x005CA25F 0x005CA2ED 0x005CA2F6 0x005CA2F9
Status: Coverage transcription - unclaimed body (the entry `push ecx` dummy
        slot idiom ahead of the callee-saved pushes is not reproducible,
        seed TRIED).
*/
int __fastcall MediaRuntime::sub_5ca230(int size, int kind) {
  if (input_ring_current + size >= input_ring_base + 1200000)
    input_ring_current = input_ring_base;

  const LONG read = mmioRead(descriptor->stream,
                             reinterpret_cast<HPSTR>(input_ring_current), size);
  if (read == 0) { stream_eof = 1; return -1; }
  if (read == -1) return -2;

  LONG *cursor = &read_producer_cursor;
  read_records[*cursor % 0x18].payload = input_ring_current;
  read_records[*cursor % 0x18].field_0c = static_cast<DWORD>(kind);
  LONG *counter = &read_counter;
  read_records[*cursor % 0x18].field_08 = static_cast<DWORD>(*counter);
  input_ring_current += size;
  InterlockedIncrement(cursor);
  InterlockedIncrement((LONG *)&pending_read_records);
  InterlockedIncrement(counter);
  return 1;
}


// Transferred typed media owners. The historical seed prose and ORIGINAL
// records below remain attached to their complete C++ implementations.


/*
Purpose: Fill one 8x8-block-pair frame fragment with the flat color the six
         packed char channels name: each channel is scaled through the word
         at 0x009C5820 with the image's round-half-up idiom -
         ((product >> 16) + bit 15 of the product) >> 1, then & 0x7f - to
         index the luma table, and channels 6 and 5 likewise index the two
         chroma tables whose sum is shared by every block. The luma planes
         are written at `plane` and `plane`+8 ints, the chroma plane at
         `plane` + 2*stride ints, each as 8 rows of 8 ints stepped by the
         stride (masked to whole ints).

// Typed owner implementation; native ABI/output, original reference targets
// and exceptional behavior remain restoration obligations, not owner stubs.
// ORIGINAL: 0x005CD3B0 sub_5cd3b0 0x005CD3B0-0x005CD556 BYTE_EXACT
// symbol    ?sub_5cd3b0@@YAXDDDDDDPAHI@Z
// size      422 bytes
// prototype void (__cdecl sub_5cd3b0)(char, char, char, char, char, char, int *, unsigned int)
// callers   1   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
Status: Coverage transcription - unclaimed body (register allocation for the
        four fill loops differs past instruction 3; seed TRIED).
*/
void __cdecl sub_5cd3b0(char param_1, char param_2, char param_3, char param_4,
                        char param_5, char param_6, int *plane,
                        unsigned int stride_arg) {
    DWORD *const words = movie_media_conversion_tables.packed_component_words;
    const int chroma =
        words[256 + ((media_fixed_mul(param_5,
                      movie_media_conversion_tables.block_scale_entries[0]) >> 1) & 0x7f)] +
        words[128 + ((media_fixed_mul(param_6,
                      movie_media_conversion_tables.block_scale_entries[0]) >> 1) & 0x7f)];
    int fill;
    int *row;
    int outer;

    fill = words[(media_fixed_mul(param_1,
                  movie_media_conversion_tables.block_scale_entries[0]) >> 1) & 0x7f] + chroma;
    row = plane;
    for (outer = 8; outer != 0; --outer) {
        for (int inner = 8; inner != 0; --inner) *row++ = fill;
        row += stride_arg / 4 - 8;
    }

    fill = words[(media_fixed_mul(param_2,
                  movie_media_conversion_tables.block_scale_entries[0]) >> 1) & 0x7f] + chroma;
    row = plane + 8;
    for (outer = 8; outer != 0; --outer) {
        for (int inner = 8; inner != 0; --inner) *row++ = fill;
        row += stride_arg / 4 - 8;
    }

    fill = words[(media_fixed_mul(param_3,
                  movie_media_conversion_tables.block_scale_entries[0]) >> 1) & 0x7f] + chroma;
    row = plane + stride_arg * 2;
    for (outer = 8; outer != 0; --outer) {
        for (int inner = 8; inner != 0; --inner) *row++ = fill;
        row += stride_arg / 4 - 8;
    }

    fill = words[(media_fixed_mul(param_4,
                  movie_media_conversion_tables.block_scale_entries[0]) >> 1) & 0x7f] + chroma;
    row = plane + stride_arg * 2 + 8;
    for (outer = 8; outer != 0; --outer) {
        for (int inner = 8; inner != 0; --inner) *row++ = fill;
        row += stride_arg / 4 - 8;
    }
}
