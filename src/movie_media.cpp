// Typed movie/media owners. Historical matching obligations are retained;
// the explicitly unfinished converter callees below retain their seeds and
// do not establish recovered codec behavior or working playback.
#include "stdafx.h"
#include "movie_media.h"
#include "movie_decoder_owner.h"
#include "general.h"

#include <process.h>
#include <stdlib.h>
#include <string.h>

// ORIGINAL: 0x005CBAE0 sub_5cbae0 0x005CBAE0-0x005CBBB6 FILE
// RECOVERY_BINDING: v1 {"reason":"Recover the runtime-owned DirectDraw overlay operation and its actual source/destination interfaces, rectangles and failure-path receiver; update_overlay is a descriptive recovered name, not an original debug-symbol claim.","evidence":["Pinned5CBAE0..5CBBB6 passes runtime in ECX and mode in EDX;5CBB6C calls Surface2 slot33(+84), the installed DDRAW.H UpdateOverlay contract, with DDOVERLAYFX size38 and DDFX at30.","MediaRuntime descriptor+0 and rectangle fields+0C/+10/+14/+18; MediaDescriptor Surface2 pointers+14/+1C and dimensions+60/+64 have existing typed producers in movie_media.cpp and UV2Player::exec.","Pinned5CBB5E..6B passes distinct source and destination RECTs; destination is0,0,width-1,height-1. Flags200/84000 mean hide/show|DDFX, not DDBLT_WAIT or rotation.","Pinned5CBB7F writes runtime queue_record_kind+4C on E_INVALIDARG;5CBB98..A0 reloads runtime->descriptor->surface_1c after UpdateOverlay before Restore, ignoring its return.","Original identity/span and seed provenance retained from git:36d19d9ef:src/stringstruct.cpp and retired seed git:117282ee:src/unrecovered/005cbae0.cpp; prior Blt and descriptor-Restore interpretation is superseded, not original behavior."]}
// symbol    ?update_overlay@MediaRuntime@@QAIHH@Z

int __fastcall MediaRuntime::update_overlay(int show) {
  DDOVERLAYFX effects;
  memset(&effects, 0, sizeof(effects));
  effects.dwSize = sizeof(effects);
  DWORD flags;
  if (show == 0) {
    flags = DDOVER_HIDE;
  } else {
    flags = DDOVER_SHOW | DDOVER_DDFX;
    effects.dwDDFX = DDOVERFX_ARITHSTRETCHY;
  }

  RECT source_rectangle;
  source_rectangle.left = static_cast<LONG>(output_rect_left_14);
  source_rectangle.top = static_cast<LONG>(output_rect_top_18);
  source_rectangle.right = static_cast<LONG>(output_rect_left_14 + output_rect_extent_0c);
  source_rectangle.bottom = static_cast<LONG>(output_rect_top_18 + output_rect_or_draw2_bits_10);

  MediaDescriptor *const output = descriptor;
  RECT destination_rectangle;
  destination_rectangle.left = 0;
  destination_rectangle.top = 0;
  destination_rectangle.right = static_cast<LONG>(output->output_width_64 - 1U);
  destination_rectangle.bottom = static_cast<LONG>(output->output_height_60 - 1U);

  const HRESULT result = output->surface_1c->UpdateOverlay(
      &source_rectangle, output->surface_14, &destination_rectangle, flags, &effects);
  if (result != DD_OK) {
    if (result == E_INVALIDARG) {
      queue_record_kind = 1;
      return 1;
    }
    if (result == DDERR_SURFACELOST) {
      // Original reloads both pointers after the COM callback, not a cached receiver.
      descriptor->surface_1c->Restore();
    }
    return 1;
  }
  return 0;
}

// Typed dispatcher/waiter contracts owned by this translation unit.
extern "C" int __fastcall sub_5c9ec0(MediaRuntime *runtime, void *unused);
extern "C" int __fastcall sub_5ca970(MediaRuntime *runtime, void *unused);
extern "C" int __fastcall sub_5ca310(MediaRuntime *runtime, int payload_bytes);
// Product-owned declarations below use the actual owner headers so C++
// linkage is not accidentally changed to C by this draft.

// Existing product body at 0x005CA230: ECX is the runtime, EDX is the
// payload byte count retained from the just-read header, and one stack
// argument selects the record kind. Preserve the product declaration's
// C++ linkage until an authorized identity migration says otherwise.
extern "C" char *__fastcall sub_5cf120(int code);

// Seed-only output worker. This declaration must bind to its original-mapped
// owner when that dependency is migrated; no local placeholder is provided.
extern "C" void __cdecl sub_5cdfa0(void *runtime);

// The pinned image contains exactly 92 adjacent {HRESULT, relocated string}
// records in this scanned table window. Text and order are preserved from the
// image; no claim is made about storage beyond the observed window.
struct MediaHResultText {
  HRESULT result;
  const char *text;
};

// The source-owned owner starts with the 64-entry block scale lookup at
// 0x9C5820, then preserves the 12 intervening bytes before three adjacent
// 128-DWORD color banks. The remap table follows those banks exactly.
// RECOVERY_OBJECT: v2 0x009C5820 0x009C9594 ?movie_media_conversion_tables@@3UMediaConversionTableStorage@@A 0x005CC940
MediaConversionTableStorage movie_media_conversion_tables;
static int media_rounded_high_word(int value, int multiplier);

// Minimum source table read by native builders 5CC940 and 5CD2D0. Their
// indexed accesses cover 64 DWORDs at 0x691F4C..0x69204B; this does not claim
// an extent for surrounding original data.
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

static const char media_hresult_unknown[] = "Uknown Error Code";

// ORIGINAL: 0x005CF120 sub_5cf120 0x005CF120-0x005CF144 BYTE_EXACT
// symbol    @sub_5cf120@4
// kind      game
extern "C" char *__fastcall sub_5cf120(int code) {
  for (unsigned int i = 0;
       i != sizeof(media_hresult_texts) / sizeof(media_hresult_texts[0]);
       ++i) {
    if (media_hresult_texts[i].result == code)
      return (char *)media_hresult_texts[i].text;
  }
  return (char *)media_hresult_unknown;
}

// ECX, EDX plus eight stack arguments; 5CC150 supplies exactly this layout.
// ORIGINAL: 0x005CBC00 sub_5cbc00 0x005CBC00-0x005CBE2A
// symbol    @sub_5cbc00@40
// kind      game
// TRIED: IDA's 8-arg __stdcall guess; disasm shows ecx/edx as real args (fastcall, 10 logical params) and Ghidra's final-sum check param_5+param_6+param_7 is wrong - raw disasm reads [esp+0x2c](param_4)+[esp+0x34](param_6)+[esp+0x38](param_7). Prologue register/stack-frame shape (#0 sub vs push) not chased.
extern "C" int __fastcall sub_5cbc00(
    int sample, int component_a, int component_b, int combined_bits,
    int component_a_bits, int component_b_bits, int component_c_bits,
    int component_a_shift, int component_b_shift, int component_c_shift) {
  unsigned char bits = (unsigned char)component_a_bits;
  int divisor = 1 << ((8 - bits) & 0x1f);
  int packed_a = sample / divisor;
  int rounded_a;
  if (sample < 0) {
    int max_a = 1 << (bits & 0x1f);
    rounded_a = (sample - (1 << ((7 - bits) & 0x1f))) / divisor;
    if (rounded_a < 1 - max_a) rounded_a = 1 - max_a;
  } else {
    int max_a = 1 << (bits & 0x1f);
    rounded_a = ((1 << ((7 - bits) & 0x1f)) + sample) / divisor;
    if (max_a - 1 < rounded_a) rounded_a = max_a - 1;
  }

  bits = (unsigned char)component_b_bits;
  divisor = 1 << ((8 - bits) & 0x1f);
  int packed_b = component_a / divisor;
  int rounded_b;
  if (component_a < 0) {
    rounded_b = (component_a - (1 << ((7 - bits) & 0x1f))) / divisor;
    int max_b = 1 << (bits & 0x1f);
    if (rounded_b < 1 - max_b) rounded_b = 1 - max_b;
  } else {
    rounded_b = ((1 << ((7 - bits) & 0x1f)) + component_a) / divisor;
    int max_b = 1 << (bits & 0x1f);
    if (max_b - 1 < rounded_b) rounded_b = max_b - 1;
  }

  bits = (unsigned char)component_c_bits;
  divisor = 1 << ((8 - bits) & 0x1f);
  unsigned int packed_c = (unsigned int)(component_b / divisor);
  unsigned int rounded_c;
  if (component_b < 0) {
    rounded_c = (unsigned int)((component_b -
        (1 << ((7 - bits) & 0x1f))) / divisor);
    unsigned int max_c = 1U << (bits & 0x1f);
    unsigned int lower = 1U - max_c;
    if ((int)lower <= (int)rounded_c) goto combine_components;
  } else {
    rounded_c = (unsigned int)(((1 << ((7 - bits) & 0x1f)) + component_b) /
                               divisor);
    unsigned int max_c = 1U << (bits & 0x1f);
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

  unsigned int result =
      (((1U << (component_c_bits & 0x1f)) - 1U) & packed_c)
          << ((component_c_shift + 0x10) & 0x1f) |
      (((1U << (component_b_bits & 0x1f)) - 1U) & (unsigned int)packed_b)
          << ((component_b_shift + 0x10) & 0x1f) |
      (((1U << (component_a_bits & 0x1f)) - 1U) & (unsigned int)packed_a)
          << ((component_a_shift + 0x10) & 0x1f) |
      (((1U << (component_c_bits & 0x1f)) - 1U) & rounded_c)
          << (component_c_shift & 0x1f) |
      (((1U << (component_b_bits & 0x1f)) - 1U) & (unsigned int)rounded_b)
          << (component_b_shift & 0x1f) |
      (((1U << (component_a_bits & 0x1f)) - 1U) & (unsigned int)rounded_a)
          << (component_a_shift & 0x1f);
  if (combined_bits + component_b_bits + component_c_bits != 0x10)
    result |= 0x80008000;
  return (int)result;
}

// Native caller passes ECX, EDX and five stack dwords; writes all three banks.
// ORIGINAL: 0x005CC150 sub_5cc150 0x005CC150-0x005CC2A9
// symbol    @sub_5cc150@28
// kind      game
// TRIED: catalogue head is stdcall(a1..a5) but the body reads ecx/edx directly as real values at entry (mov ebx,edx; mov edi,ecx) - PROPOSAL: __fastcall(ecxArg, edxArg, a1..a5); ret 0x14 cleans exactly the 5 stack ints, consistent with fastcall. Call-site argument order for the 2nd/3rd loops was cross-checked against the ghidra decompile's param_1..param_7 (fastcall) naming, which also confirms two of the fixed-point angle computations per loop are dead (computed, never read by the callee).
extern "C" int __fastcall sub_5cc150(int ecx_arg, int edx_arg, int a1,
                                     int a2, int a3, int a4, int a5) {
  int result = 0;
  signed char index = 0;
  DWORD *out = movie_media_conversion_tables.packed_component_words;
  do {
    int angle = (int)(signed char)((index << 1) + 1) + 0x80;
    result = sub_5cbc00(angle, angle, angle, ecx_arg, edx_arg,
                        a1, a2, a3, a4, a5);
    *out++ = (DWORD)result;
    ++index;
  } while ((out - movie_media_conversion_tables.packed_component_words) < 128);

  index = 0;
  out = movie_media_conversion_tables.packed_component_words + 256;
  do {
    signed char scaled = (signed char)((index << 1) + 1);
    int angle = (int)(((__int64)scaled * 116129) >> 16);
    result = sub_5cbc00(0, angle, angle, 0, edx_arg,
                        a1, a2, a3, a4, a5);
    *out++ = (DWORD)result;
    ++index;
  } while ((out - (movie_media_conversion_tables.packed_component_words + 256)) < 128);

  index = 0;
  out = movie_media_conversion_tables.packed_component_words + 128;
  do {
    signed char scaled = (signed char)((index << 1) + 1);
    int angle = media_rounded_high_word((int)scaled, -0xb6cf);
    result = sub_5cbc00(0, angle, angle, 0, edx_arg,
                        a1, a2, a3, a4, a5);
    *out++ = (DWORD)result;
    ++index;
  } while ((out - (movie_media_conversion_tables.packed_component_words + 128)) < 128);
  return result;
}

static int media_rounded_high_word(int value, int multiplier) {
  __int64 product = (__int64)value * multiplier;
  return (int)((product + 0x8000) >> 16);
}

// Rebuilds the same three 128-word banks with the descriptor-selected value.
// ORIGINAL: 0x005CC2B0 sub_5cc2b0 0x005CC2B0-0x005CC42C
// symbol    @sub_5cc2b0@4
// kind      game
// TRIED: `__int64` product + `(x+0x8000)>>16` for the imul/shr-5 rounded-highword idiom, instead of the raw edx:eax add/adc sequence; #1 mismatch is in the fastcall-arg-to-local store.
extern "C" void __fastcall sub_5cc2b0(int selected_component) {
  DWORD *out = movie_media_conversion_tables.packed_component_words;
  signed char index = 0;
  do {
    int signed_value = (int)(signed char)(index * 2 + 1);
    unsigned int first = (unsigned int)(signed_value + 0x80);
    unsigned int second = first;
    unsigned int third = first;
    int selected = selected_component;
    if ((int)first < 0) {
      second = (unsigned int)(signed_value + 0x7f);
      if ((int)second < 0) third = second;
      if ((int)third < 0) selected = selected_component - 1;
    }
    *out++ = ((((third & 0xff) | ((unsigned int)selected << 8)) << 8 |
               (second & 0xff)) << 8) | (first & 0xff);
    ++index;
  } while ((out - movie_media_conversion_tables.packed_component_words) < 128);

  index = 0;
  out = movie_media_conversion_tables.packed_component_words + 256;
  do {
    int scaled = (int)(signed char)(index * 2 + 1);
    unsigned int first = (unsigned int)media_rounded_high_word(scaled, -0x5816);
    unsigned int second = (unsigned int)media_rounded_high_word(scaled, 0x1c5a1);
    int selected = 0;
    unsigned int third = 0;
    if ((int)second < 0) --first;
    if ((int)first < 0) {
      third = 0xffffffffU;
      selected = -1;
    }
    *out++ = ((((third & 0xff) | ((unsigned int)selected << 8)) << 8 |
               (first & 0xff)) << 8) | (second & 0xff);
    ++index;
  } while ((out - (movie_media_conversion_tables.packed_component_words + 256)) < 128);

  index = 0;
  out = movie_media_conversion_tables.packed_component_words + 128;
  do {
    int scaled = (int)(signed char)(index * 2 + 1);
    unsigned int first = (unsigned int)media_rounded_high_word(scaled, 0x166e9);
    unsigned int second = (unsigned int)media_rounded_high_word(scaled, -0xb6cf);
    int selected = 0;
    if ((int)second < 0) --first;
    if ((int)first < 0) selected = -1;
    *out++ = ((((first & 0xff) | ((unsigned int)selected << 8)) << 8 |
               (second & 0xff)) << 8);
    ++index;
  } while ((out - (movie_media_conversion_tables.packed_component_words + 128)) < 128);
}

// The receiver is the embedded MediaPixelFormat subobject at descriptor +2C.
// 5CBE30 explicitly computes ECX as descriptor+0x2c before this call.
// ORIGINAL: 0x005CC450 sub_5cc450 0x005CC450-0x005CC70D
// symbol    ?sub_5cc450@MediaPixelFormat@@QAEXXZ
// kind      game
// TRIED: __thiscall member over a PixelFormat-like struct (bit counts at self+0/4/8, shifts at self+0xc/0x10/0x14, a flag at self+0x1c). Loop 1 builds three 512-entry float-derived tables (0x9c6f2c/772c/7f2c) from clamped index -0x100..0xff using six FPU constants at fixed .rdata addresses (dereferenced as plain float/double loads, not transcribed as literals since their bit patterns are unknown); loops 2-4 build three 256-entry integer shift/clamp tables (0x9c872c/8bf4/ 900c) per RGB channel from the struct fields. Ghidra's `__ftol()` no-arg calls were replaced with ordinary `(int)` casts on the FPU expressions, which is what the compiler lowers to `call __ftol` itself. Compiles and matches the first 26 instructions (all of the integer clamp/index setup); diverges at #26 (`fld` vs `lea`) once the FPU stack scheduling for the three-way `fld st(0)` duplication starts - x87 stack scheduling, not source form, and not chased further given the size of the function.
void MediaPixelFormat::sub_5cc450() {
  for (int value = -0x100; value < 0x100; ++value) {
    int clamped = value;
    if (clamped < -0x40) clamped = -0x40;
    else if (clamped > 0x3f) clamped = 0x3f;
    unsigned int index = (unsigned int)(value - 0xc0) & 0x1ff;
    double shifted = (double)clamped + 0.5;

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
    if (maximum < value) value = maximum;
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
    if (maximum < value) value = maximum;
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
    if (maximum < value) value = maximum;
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
static const DWORD media_block_quant_order[64] = {
    8, 16, 19, 22, 26, 27, 29, 34, 16, 16, 22, 24, 27, 29, 34, 37,
    19, 22, 26, 27, 29, 34, 34, 38, 22, 22, 26, 27, 29, 34, 37, 40,
    22, 26, 27, 29, 32, 35, 40, 48, 26, 27, 29, 32, 35, 40, 48, 58,
    26, 27, 29, 34, 38, 46, 56, 69, 27, 29, 35, 38, 46, 56, 69, 83,
};

// 64-word output table consumed by 5CC940 case 3. The three scalar literals
// below are read from the pinned initial image with their float/double widths.
// ORIGINAL: 0x005CD2D0 sub_5cd2d0 0x005CD2D0-0x005CD3A2
// symbol    _sub_5cd2d0
// kind      game
// TRIED: homed into general.cpp (its real TU) 2026-09-03 and reverted for one reason:
// TRIED: `long long` (VC6 has none, use __int64); flat 64-iter loop instead of nested 8x8 do/while counted down
extern "C" int __cdecl sub_5cd2d0(DWORD *destination, int quality) {
  if (quality == 100) {
    for (int i = 0; i != 64; ++i)
      destination[i] = media_block_transform_weights[i] * 8;
  } else {
    const float scale = (107.0f - (float)quality) * 0.0625f;
    for (int i = 0; i != 64; ++i) {
      const int quantized = (int)((double)media_block_quant_order[i] *
                                  (double)scale * 65536.0);
      destination[i] = (DWORD)(((__int64)media_block_transform_weights[i] *
                                quantized) >> 16);
    }
  }
  destination[0] = media_block_transform_weights[0] * 8;
  return 0;
}

// ECX is IDirectDraw2*, EDX is the caller's surface output slot; the three
// stack arguments are width, height and FourCC. Native creates a FourCC
// surface, queries Surface2, applies source-blit color key {1,1}, then
// publishes the queried surface only on success.
// ORIGINAL: 0x005CC860 sub_5cc860 0x005CC860-0x005CC935
// symbol    @sub_5cc860@20
// kind      game
// TRIED: __stdcall(int,int,int) per IDA's guess - entry reads ecx/edx directly with no matching stack slot, so this is __fastcall with param_1/param_2 in ecx/edx. The three vtable-style calls push their receiver as an explicit stack arg rather than loading it into ecx, so they are plain function-pointer calls through *(vtable+off), not C++ virtual dispatch; exact stack slot reuse for the WSA-shaped struct (0x6c/0x1007/0x80/0x20/4) not nailed down byte-for-byte.
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

  IDirectDrawSurface *created = 0;
  if (provider->CreateSurface(&description, &created, 0) != DD_OK) return 1;

  IDirectDrawSurface2 *queried = 0;
  if (created->QueryInterface(IID_IDirectDrawSurface2,
                              (void **)&queried) != DD_OK) {
    created->Release();
    return 1;
  }

  DDCOLORKEY color_key;
  color_key.dwColorSpaceLowValue = 1;
  color_key.dwColorSpaceHighValue = 1;
  if (queried->SetColorKey(DDCKEY_SRCBLT, &color_key) != DD_OK) {
    // The native failure path releases the original CreateSurface result,
    // not the queried Surface2 pointer. Preserve that observed reference
    // choice; do not normalize it to a more conventional COM cleanup.
    created->Release();
    return 1;
  }
  *output = queried;
  return 0;
}

int __fastcall sub_5cdad0(IDirectDraw2 *, IDirectDrawSurface2 **, int, int, int);
void __cdecl sub_5cd3b0(char, char, char, char, char, char,
                        int *, unsigned int);
extern "C" void __cdecl sub_5cd560(signed char, signed char, signed char,
                                    signed char, signed char, signed char,
                                    int *, unsigned int);
extern "C" void __fastcall sub_5cd8d0(MediaRuntime *, unsigned char *,
                                      unsigned char *, int, unsigned int);
extern "C" void __fastcall sub_5cd980(MediaRuntime *, unsigned char *,
                                      unsigned char *, int, unsigned int);
extern "C" void __fastcall sub_5cda30(MediaRuntime *, unsigned char *,
                                      unsigned char *, int, unsigned int);
void __cdecl sub_5cfaed(unsigned char *, BYTE *, int);
void __cdecl sub_5d02cc(unsigned char *, void *, int);
int __fastcall sub_5d35b0(MediaRuntime *, const BYTE *, BYTE *, DWORD, int);
extern "C" void __fastcall sub_5d3f50(int *);

// Native 16-bit block writer. Its scale/bank reads are views into the color
// setup owner: scale[0], packed bank 0, and packed banks 1/2. It indexes 128
// entries in each packed bank and writes four 16-byte-wide, eight-row blocks.
static int media_scale_signed_component(signed char component) {
  return (int)(((__int64)movie_media_conversion_tables.block_scale_entries[0] *
                (int)component) >> 16);
}

static void media_write_16bit_block(unsigned int pitch, BYTE *destination,
                                    unsigned short pixel) {
  const DWORD packed = ((DWORD)pixel << 16) | pixel;
  const unsigned int row_advance = (pitch & ~1U) - 0x10;
  for (int row = 0; row != 8; ++row) {
    memcpy(destination + 0x00, &packed, sizeof(packed));
    memcpy(destination + 0x04, &packed, sizeof(packed));
    memcpy(destination + 0x08, &packed, sizeof(packed));
    memcpy(destination + 0x0c, &packed, sizeof(packed));
    destination += 0x10 + row_advance;
  }
}

// ORIGINAL: 0x005CD560 sub_5cd560 0x005CD560-0x005CD74D
// symbol    _sub_5cd560
// kind      game
// TRIED: homed into general.cpp (its real TU) 2026-09-03 and reverted for one reason:
// TRIED: (none - lands as MISMATCH #2 movsx/mov, likely helper-function extraction changing register scheduling around the sign-extend widen)
extern "C" void __cdecl sub_5cd560(
    signed char component0, signed char component1, signed char component2,
    signed char component3, signed char component4, signed char component5,
    int *destination_words, unsigned int pitch) {
  DWORD *const bank0 = movie_media_conversion_tables.packed_component_words;
  DWORD *const bank1 = bank0 + 128;
  DWORD *const bank2 = bank0 + 256;
  const int index5 = (media_scale_signed_component(component5) >> 1) & 0x7f;
  const int index4 = (media_scale_signed_component(component4) >> 1) & 0x7f;
  const int baseline = (int)bank1[index5] + (int)bank2[index4];
  BYTE *const base = (BYTE *)destination_words;

  const int index0 = (media_scale_signed_component(component0) >> 1) & 0x7f;
  const int index1 = (media_scale_signed_component(component1) >> 1) & 0x7f;
  const int index2 = (media_scale_signed_component(component2) >> 1) & 0x7f;
  const int index3 = (media_scale_signed_component(component3) >> 1) & 0x7f;
  media_write_16bit_block(pitch, base,
      (unsigned short)(bank0[index0] + baseline));
  media_write_16bit_block(pitch, base + 0x10,
      (unsigned short)(bank0[index1] + baseline));
  media_write_16bit_block(pitch, base + pitch * 8,
      (unsigned short)(bank0[index2] + baseline));
  media_write_16bit_block(pitch, base + pitch * 8 + 0x10,
      (unsigned short)(bank0[index3] + baseline));
}

static int media_divide_toward_zero(int numerator, int denominator) {
  return numerator / denominator;
}

static SHORT media_read_signed_word(const BYTE *bytes) {
  SHORT value;
  memcpy(&value, bytes, sizeof(value));
  return value;
}

static void media_build_runtime_block_scale_table(int quality) {
  const int complement = 100 - quality;
  const int first = media_divide_toward_zero(complement * 22, 100) + 8;
  const int step = media_divide_toward_zero(complement * 50, 100) + 10 - first;
  for (int row = 0; row != 8; ++row) {
    int phase = row * step;
    for (int column = 0; column != 8; ++column) {
      const int index = row * 8 + column;
      const int adjustment = media_divide_toward_zero(
          (int)media_block_transform_weights[index] + phase, 7);
      const int scale = first + adjustment;
      movie_media_conversion_tables.block_scale_entries[index] =
          media_block_transform_weights[index] * (DWORD)scale;
      phase += step;
    }
  }
}

// Typed mapped worker. Local source packet bytes begin after the eight-byte
// record header; the full caller-owned payload remains the in-place output
// area used by the native block converter.
// ORIGINAL: 0x005CC940 sub_5cc940 0x005CC940-0x005CD28C
// symbol    @sub_5cc940@8
// kind      game
// TRIED: a DirectDraw surface-format-change/blit handler (COM vtable dispatch through IDirectDrawSurface-shaped interfaces, confirmed by the 0x887601c2/DDERR_SURFACELOST-style comparisons and the guidance's own COM-dispatch note). Landed: __fastcall signature (ecx=param_1, edx=param_2, matches `mov eax,edx; mov esi,ecx` at entry), the first switch's dispatch table resolved directly from the image at 0x5CD28C (parsed the PE section table by hand since no RESOLVED JUMP TABLE section was printed for this address: index->target is 0->0x5CC9D4, 1->0x5CC9EF, 2->0x5CC98C, 3->0x5CC9A7, 4..6->0x5CC9DD), the geometry/clip-flag field arithmetic (param_1[3..6]), the COM CreateSurface/QueryInterface/TestCooperativeLevel/Restore/Lock call sequence with real vtable slot numbers (0x10/0x14/0x18/0x1c/0x20 => the emitter's own slots 5,6,13,24,25,27,32) and real arguments, and the second switch's call-out shape to the already-catalogued blit helpers (sub_5cd8d0/5cd980/5cda30/5d35b0/5d4020/5d3980/5d3df0/5d3f50). Case 3 of the second switch is a pixel-format converter that packs 1/3/6 source bytes into 32-bit BGRA-ish words via byte-granularity CONCAT31 before calling sub_5cd560/5cd3b0 (16bpp) or sub_5d02cc/5cfaed (raw copy) - left as a placeholder (TRIED) rather than guess the per-run-type byte layout without more time.
extern "C" int __fastcall sub_5cc940(MediaRuntime *runtime,
                                     MediaQueueRecord *record) {
  // The historical case-3 placeholder note above is superseded by the
  // explicit 3/6/12-byte short-run paths and >12-byte dispatch below.
  // 5D35B0 and its separate downstream renderer remain unfinished.
  MediaDescriptor &d = *runtime->descriptor;
  BYTE *packet = record->payload;
  const DWORD record_kind = record->field_0c;
  runtime->selected_record_kind_39c = record_kind;

  switch (record_kind) {
  case 1:
    sub_5cd750(runtime, packet);
    break;
  case 3:
    d.output_width_64 = (DWORD)(LONG)media_read_signed_word(packet + 0);
    d.output_height_60 = (DWORD)(LONG)media_read_signed_word(packet + 2);
    runtime->record_scale_byte_048 = packet[4];
    break;
  case 4:
    d.output_width_64 = (DWORD)(LONG)media_read_signed_word(packet + 0);
    d.output_height_60 = (DWORD)(LONG)media_read_signed_word(packet + 2);
    runtime->record_scale_byte_048 = packet[4];
    runtime->record_option_040 = packet[7] & 2;
    runtime->record_option_044 = packet[7] & 1;
    break;
  case 5:
  case 6:
  case 7:
    d.output_width_64 = (DWORD)(LONG)media_read_signed_word(packet + 8);
    d.output_height_60 = (DWORD)(LONG)media_read_signed_word(packet + 10);
    break;
  }

  runtime->output_rect_left_14 = 0;
  runtime->output_rect_top_18 = 0;
  runtime->output_rect_extent_0c = d.output_width_64;
  runtime->output_rect_or_draw2_bits_10 = d.output_height_60;
  if ((d.flags & 0x40) != 0) runtime->output_rect_extent_0c <<= 1;
  if ((d.flags & 0x30) != 0)
    runtime->output_rect_or_draw2_bits_10 <<= 1;
  if ((d.flags & 0x04) != 0)
    runtime->output_rect_left_14 =
        (int)(runtime->surface_width_004 - runtime->output_rect_extent_0c) >> 1;
  if ((d.flags & 0x08) != 0)
    runtime->output_rect_top_18 =
        (int)(runtime->surface_height_008 -
              runtime->output_rect_or_draw2_bits_10) >> 1;
  if ((d.flags & 0x01) != 0)
    runtime->output_rect_left_14 += d.geometry_offset_x_68;
  if ((d.flags & 0x02) != 0)
    runtime->output_rect_top_18 += d.geometry_offset_y_6c;
  if ((d.flags & 0x80) != 0)
    runtime->output_rect_extent_0c = d.geometry_width_override_70;
  if ((d.flags & 0x100) != 0)
    runtime->output_rect_or_draw2_bits_10 = d.geometry_height_override_74;

  BYTE *surface_source = 0;
  DWORD output_pitch = 0;
  DDSURFACEDESC locked;
  memset(&locked, 0, 0x6c);
  locked.dwSize = 0x6c;

  if ((d.flags & 0x800) != 0) {
    surface_source = d.provided_output_base_4c;
    output_pitch = d.provided_output_pitch_50;
  } else if ((d.flags & 0x400) != 0) {
    if (d.surface_1c == 0) {
      DDSURFACEDESC surface_description;
      memset(&surface_description, 0, 0x6c);
      surface_description.dwSize = 0x6c;
      surface_description.dwFlags = 7;
      surface_description.dwWidth = d.output_width_64;
      surface_description.dwHeight = d.output_height_60;
      if ((d.flags & 0x180) != 0) surface_description.ddsCaps.dwCaps = 0x840;
      else surface_description.ddsCaps.dwCaps = 0x40;
      if ((d.flags & 0x40) != 0) surface_description.dwWidth *= 2;

      IDirectDrawSurface *created = 0;
      if (d.draw_device_10->CreateSurface(&surface_description, &created, 0) == DD_OK) {
        created->QueryInterface(IID_IDirectDrawSurface2,
                                (void **)&d.surface_1c);
        if (d.draw_device_10->CreateSurface(&surface_description, &created, 0) == DD_OK)
          created->QueryInterface(IID_IDirectDrawSurface2,
                                  (void **)&d.surface_20);
      }
    }
  } else if (runtime->queue_record_kind != 1 && d.surface_1c == 0) {
    DWORD fourcc_count = 0x40;
    DWORD fourcc_codes[64];
    memset(fourcc_codes, 0, sizeof(fourcc_codes));
    if (d.draw_device_10->GetFourCCCodes(&fourcc_count, fourcc_codes) == DD_OK) {
      if (runtime->queue_record_kind == 4) {
        if (sub_5cdad0(d.draw_device_10,
                       &d.surface_1c,
                       (int)d.output_width_64, (int)d.output_height_60,
                       0x32595559) != 0)
          runtime->queue_record_kind = 1;
      } else if (sub_5cc860(d.draw_device_10, &d.surface_1c,
                            (int)d.output_width_64,
                            (int)d.output_height_60, 0x32595559) != 0) {
        runtime->queue_record_kind = 1;
      } else {
        runtime->owned_heap_638 = (BYTE *)malloc(
            d.output_width_64 * d.output_height_60 * 2);
        RECT fill_rectangle;
        fill_rectangle.left = runtime->output_rect_left_14;
        fill_rectangle.top = runtime->output_rect_top_18;
        fill_rectangle.right = fill_rectangle.left + runtime->output_rect_extent_0c;
        fill_rectangle.bottom = fill_rectangle.top +
            runtime->output_rect_or_draw2_bits_10;
        DDBLTFX fill_effects;
        memset(&fill_effects, 0, 0x64);
        fill_effects.dwSize = 0x64;
        fill_effects.dwFillColor = 0;
        d.surface_14->Blt(&fill_rectangle, 0, 0, 0x01000400, &fill_effects);
        runtime->output_handle_candidate = 1;
      }
    } else {
      runtime->queue_record_kind = 1;
    }
  }

  IDirectDrawSurface2 *cooperative_surfaces[4] = {
      d.surface_14, d.surface_18, d.surface_1c, d.surface_20};
  const DWORD restore_flags[4] = {0x02000000, 0x08000000, 0, 0};
  for (int i = 0; i != 4; ++i) {
    IDirectDrawSurface2 *surface = cooperative_surfaces[i];
    if (surface == 0) continue;
    if (i < 2 && (d.flags & restore_flags[i]) == 0) continue;
    if (surface->IsLost() == DDERR_SURFACELOST) {
      if (surface->Restore() != DD_OK) return 0;
      if (i == 2) {
        runtime->queue_record_kind = 2;
        runtime->output_handle_candidate = 1;
      }
    }
  }

  const BYTE *compressed = packet + 8;
  BYTE *pixel_payload = packet;
  if (runtime->queue_record_kind == 1) {
    IDirectDrawSurface2 *lock_target;
    if ((d.flags & 0x400) == 0) lock_target = d.surface_1c;
    else if ((d.flags & 0x08000000) == 0) lock_target = d.surface_14;
    else lock_target = d.surface_18;
    if (lock_target->Lock(0, &locked, DDLOCK_WAIT, 0) != DD_OK) return 0;
    surface_source = (BYTE *)locked.lpSurface;
    output_pitch = (DWORD)locked.lPitch;
  } else if (runtime->queue_record_kind == 2) {
    surface_source = runtime->owned_heap_638;
    output_pitch = d.output_width_64 << 1;
  } else if (runtime->queue_record_kind == 4) {
    if (d.surface_1c->Lock(0, &locked, DDLOCK_WAIT, 0) != DD_OK) return 0;
    surface_source = (BYTE *)locked.lpSurface;
    output_pitch = (DWORD)locked.lPitch;
  }

  switch (runtime->selected_record_kind_39c) {
  case 1:
    sub_5d3df0((int *)runtime, (int)pixel_payload);
    if (d.pixel_format.total_component_bits != 8)
      sub_5d3f50((int *)runtime);
    sub_5d4020(runtime, surface_source, (int)surface_source,
               (int)output_pitch);
    break;
  case 2:
    sub_5d3980((int *)runtime, (unsigned int *)pixel_payload);
    sub_5d4020(runtime, surface_source, (int)surface_source,
               (int)output_pitch);
    break;
  case 3: {
    media_build_runtime_block_scale_table((int)runtime->record_scale_byte_048);
    const bool four_byte_pixels = (d.flags & 0x40) != 0;
    const unsigned int block_step = four_byte_pixels ? 0x40 : 0x20;
    const DWORD block_pitch = output_pitch;
    BYTE *output_row = surface_source +
        runtime->output_rect_top_18 * output_pitch +
        runtime->output_rect_left_14 * (four_byte_pixels ? 4 : 2);
    for (DWORD block_y = 0; block_y < d.output_height_60; block_y += 16) {
      BYTE *output_block = output_row;
      for (DWORD block_x = 0; block_x < d.output_width_64; block_x += 16) {
        const unsigned int run_bytes = *compressed++;
        if (run_bytes > 12) {
          // The pinned >12 branch has one direct edge, to 5D02CC, regardless
          // of the output-format flag used for block stepping below.
          sub_5d02cc((unsigned char *)compressed, output_block,
                     (int)block_pitch);
        } else if (run_bytes == 3) {
          if (four_byte_pixels)
            sub_5cd3b0((char)compressed[0], (char)compressed[0],
                       (char)compressed[0], (char)compressed[0],
                       (char)compressed[1], (char)compressed[2],
                       (int *)output_block, block_pitch);
          else
            sub_5cd560((signed char)compressed[0], (signed char)compressed[0],
                       (signed char)compressed[0], (signed char)compressed[0],
                       (signed char)compressed[1], (signed char)compressed[2],
                       (int *)output_block, block_pitch);
        } else if (run_bytes == 6) {
          if (four_byte_pixels)
            sub_5cd3b0((char)compressed[0], (char)compressed[1],
                       (char)compressed[2], (char)compressed[3],
                       (char)compressed[4], (char)compressed[5],
                       (int *)output_block, block_pitch);
          else
            sub_5cd560((signed char)compressed[0], (signed char)compressed[1],
                       (signed char)compressed[2], (signed char)compressed[3],
                       (signed char)compressed[4], (signed char)compressed[5],
                       (int *)output_block, block_pitch);
        } else if (run_bytes == 12) {
          if (four_byte_pixels)
            sub_5cd3b0((char)compressed[0], (char)compressed[2],
                       (char)compressed[4], (char)compressed[6],
                       (char)compressed[8], (char)compressed[10],
                       (int *)output_block, block_pitch);
          else
            sub_5cd560((signed char)compressed[0], (signed char)compressed[2],
                       (signed char)compressed[4], (signed char)compressed[6],
                       (signed char)compressed[8], (signed char)compressed[10],
                       (int *)output_block, block_pitch);
        }
        compressed += run_bytes;
        output_block += block_step;
      }
      output_row += (block_pitch / (four_byte_pixels ? 4 : 2)) * block_step;
    }
    break;
  }
  case 4:
    sub_5cd2d0(runtime->decoder_context_3f8.block_transform_weights,
               (int)runtime->record_scale_byte_048);
    if (runtime->queue_record_kind == 1) {
      if ((d.flags & 0x40) == 0 && d.pixel_format.total_component_bits < 0x11) {
        DWORD pitch = output_pitch;
        if ((d.flags & 0x30) != 0) pitch *= 2;
        BYTE *destination = surface_source +
            runtime->output_rect_top_18 * output_pitch +
            runtime->output_rect_left_14 * 2;
        sub_5cd8d0(runtime, (BYTE *)compressed, destination,
                   (int)runtime->output_rect_extent_0c, pitch);
      } else {
        DWORD pitch = output_pitch;
        if ((d.flags & 0x30) != 0) pitch *= 2;
        BYTE *destination = surface_source +
            runtime->output_rect_top_18 * output_pitch +
            runtime->output_rect_left_14 * 4;
        sub_5cd980(runtime, (BYTE *)compressed, destination,
                   (int)runtime->output_rect_extent_0c, pitch);
      }
    } else {
      sub_5cda30(runtime, (BYTE *)compressed, surface_source, 0,
                 output_pitch);
    }
    break;
  case 5:
  case 6:
  case 7: {
    BYTE *destination = surface_source +
        runtime->output_rect_top_18 * output_pitch +
        runtime->output_rect_left_14 *
            (((d.flags & 0x40) == 0 &&
              d.pixel_format.total_component_bits <= 0x10) ? 2 : 4);
    sub_5d35b0(runtime, compressed, destination, output_pitch,
               runtime->selected_record_kind_39c);
    break;
  }
  }

  if ((d.flags & 0x800) == 0) {
    if (runtime->queue_record_kind == 1) {
      if ((d.flags & 0x400) == 0) {
        if (d.surface_1c->IsLost() != DD_OK)
          ++runtime->surface_loss_counter_038;
      } else if ((d.flags & 0x08000000) == 0) {
        if (d.surface_14->IsLost() != DD_OK) {
          ++runtime->surface_loss_counter_02c;
          return 0;
        }
      } else if (d.surface_18->IsLost() != DD_OK) {
        ++runtime->surface_loss_counter_030;
        return 0;
      }
    } else if (runtime->queue_record_kind == 4 &&
               d.surface_1c->IsLost() != DD_OK) {
      ++runtime->surface_loss_counter_038;
      return 0;
    }

    if (runtime->queue_record_kind == 1) {
      IDirectDrawSurface2 *unlock_target;
      if ((d.flags & 0x400) == 0) unlock_target = d.surface_1c;
      else if ((d.flags & 0x08000000) == 0) unlock_target = d.surface_14;
      else unlock_target = d.surface_18;
      if (unlock_target->Unlock(0) != DD_OK) {
        if (unlock_target == d.surface_14) ++runtime->surface_loss_counter_02c;
        else if (unlock_target == d.surface_18) ++runtime->surface_loss_counter_030;
        else ++runtime->surface_loss_counter_038;
        return 0;
      }
    } else if (runtime->queue_record_kind == 4 &&
               d.surface_1c->Unlock(0) != DD_OK) {
      ++runtime->surface_loss_counter_038;
      return 0;
    }
  }
  return 0;
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
void MediaRuntime::sub_5cbe30() {
  MediaDescriptor &d = *descriptor;
  if (d.stream_gate_80 != 0) return;

  if (d.draw_device_10 == 0 && d.draw_query_interface_0c != 0) {
    release_interface_10 = 1;
    d.draw_query_interface_0c->QueryInterface(
        IID_IDirectDraw2, (void **)&d.draw_device_10);
  }

  int result_kind = 1;
  if ((d.flags & 0x00000200) != 0) {
    DDCAPS caps_a;
    DDCAPS caps_b;
    memset(&caps_a, 0, sizeof(caps_a));
    memset(&caps_b, 0, sizeof(caps_b));
    caps_a.dwSize = 0x16c;
    caps_b.dwSize = 0x16c;
    if (d.draw_device_10->GetCaps(&caps_a, &caps_b) == DD_OK) {
      if ((d.capability_options_08 & 2) != 0 &&
          (caps_a.dwCaps & 0x2000) != 0 && (caps_a.dwCaps & 0x4000) != 0) {
        result_kind = 2;
      } else if ((d.capability_options_08 & 4) != 0 &&
                 (caps_a.dwCaps & 0x100) != 0 &&
                 (caps_a.dwCaps & 0x200) != 0) {
        result_kind = 4;
      }
    }
  }
  queue_record_kind = result_kind;
  d.surface_1c = 0;
  d.surface_20 = 0;

  if ((d.flags & 0x00000800) == 0) {
    DDSURFACEDESC surface_description;
    memset(&surface_description, 0, sizeof(surface_description));
    surface_description.dwSize = sizeof(surface_description);
    if (d.surface_14->GetSurfaceDesc(&surface_description) == DD_OK) {
      surface_width_004 = surface_description.dwWidth;
      surface_height_008 = surface_description.dwHeight;
    }

    if (d.surface_14 != 0) {
      DDPIXELFORMAT pixel_format;
      memset(&pixel_format, 0, sizeof(pixel_format));
      pixel_format.dwSize = sizeof(pixel_format);
      d.surface_14->GetPixelFormat(&pixel_format);

      if (pixel_format.dwRGBBitCount == 8) {
        d.pixel_format.total_component_bits = 8;
      } else {
        int red_bits = 0;
        int green_bits = 0;
        for (int bit = 0; bit < 0x1f; ++bit) {
          if ((pixel_format.dwRBitMask & (1U << bit)) != 0) ++red_bits;
          if ((pixel_format.dwGBitMask & (1U << bit)) != 0) ++green_bits;
        }
        d.pixel_format.red_bits = red_bits;
        d.pixel_format.green_bits = green_bits;
        d.pixel_format.blue_bits =
            (DWORD)media_component_mask_bits_31(pixel_format.dwBBitMask);

        int red_shift = 0;
        DWORD red_mask = pixel_format.dwRBitMask;
        while ((red_mask & 1) == 0) {
          red_mask >>= 1;
          ++red_shift;
        }
        d.pixel_format.red_shift = red_shift;

        int green_shift = 0;
        DWORD green_mask = pixel_format.dwGBitMask;
        while ((green_mask & 1) == 0) {
          green_mask >>= 1;
          ++green_shift;
        }
        d.pixel_format.green_shift = green_shift;

        int blue_shift = 0;
        DWORD blue_mask = pixel_format.dwBBitMask;
        while ((blue_mask & 1) == 0) {
          blue_mask >>= 1;
          ++blue_shift;
        }
        d.pixel_format.blue_shift = blue_shift;

        d.pixel_format.total_component_bits =
            d.pixel_format.red_bits + d.pixel_format.blue_bits +
            d.pixel_format.green_bits;
        d.pixel_format.combined_component_mask =
            pixel_format.dwRBitMask | pixel_format.dwGBitMask |
            pixel_format.dwBBitMask;
      }
    }

    if (d.pixel_format.total_component_bits == 8 &&
        (d.flags & 0x04000000) == 0) {
      d.palette_entries_28 = (BYTE *)malloc(0x400);
      d.draw_device_10->CreatePalette(
          0x44, (LPPALETTEENTRY)d.palette_entries_28, &d.palette_24, 0);
      memset(d.palette_entries_28, 0, 0x400);
      d.surface_14->SetPalette(d.palette_24);
    }
  }

  if ((d.flags & 0x00100000) == 0) d.setup_value_78 = 100;

  if (d.pixel_format.total_component_bits == 0x10 ||
      d.pixel_format.total_component_bits == 0x0f) {
    sub_5cc150(0xff, (int)d.pixel_format.red_bits,
               (int)d.pixel_format.green_bits,
               (int)d.pixel_format.blue_bits,
               (int)d.pixel_format.red_shift,
               (int)d.pixel_format.green_shift,
               (int)d.pixel_format.blue_shift);
  } else {
    sub_5cc2b0(0xff);
  }
  d.pixel_format.sub_5cc450();

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
extern "C" void __cdecl sub_5cdfa0(void *argument) {
  MediaRuntime *r = (MediaRuntime *)argument;
  MediaDescriptor &d = *r->descriptor;
  Sleep(10);
  if (InterlockedExchange(&r->sync_word_3b0, 1) != 0) {
    do {
      if (r->buffer_setup_finished != 0) {
        DWORD write_position;
        if (r->synthetic_cursor_mode != 0) {
          r->synthetic_playback_cursor += 0x372;
          if (r->synthetic_playback_cursor >= r->playback_buffer_bytes)
            r->synthetic_playback_cursor -= r->playback_buffer_bytes;
          write_position = r->synthetic_playback_cursor;
        } else {
          DWORD play_position, safe_write_position;
          d.play_buffer->GetCurrentPosition(&play_position,
                                            &safe_write_position);
          write_position = play_position;
        }

        LONG available;
        if (r->playback_cursor < write_position)
          available = (LONG)(write_position - r->playback_cursor);
        else
          available = (LONG)(write_position - r->playback_cursor +
                             r->playback_buffer_bytes);
        MediaQueueRecord &record =
            r->output_records[r->output_consumer_cursor % 24];
        if (available >= (LONG)record.field_04) {
          if (r->pending_output_records != 0) {
            if (r->synthetic_cursor_mode == 0) {
              LPVOID first, second;
              DWORD first_bytes, second_bytes;
              HRESULT result = d.play_buffer->Lock(
                  r->playback_cursor, record.field_04, &first, &first_bytes,
                  &second, &second_bytes, 0);
              while (result != DS_OK) {
                d.play_buffer->Restore();
                result = d.play_buffer->Lock(
                    r->playback_cursor, record.field_04, &first, &first_bytes,
                    &second, &second_bytes, 0);
              }
              memcpy(first, record.payload, first_bytes);
              if (second_bytes != 0)
                memcpy(second, record.payload + first_bytes, second_bytes);
              d.play_buffer->Unlock(first, first_bytes, second, second_bytes);
            }
            r->playback_cursor =
                (r->playback_cursor + record.field_04) %
                r->playback_buffer_bytes;
            InterlockedIncrement(&r->output_consumer_cursor);
            InterlockedExchange(&r->output_lock_word, (LONG)record.field_08);
            InterlockedDecrement(&r->pending_output_records);
            record.payload = 0;
            record.field_04 = 0;
          } else if (r->synthetic_cursor_mode == 0) {
            LPVOID first, second;
            DWORD first_bytes, second_bytes;
            if (d.play_buffer->Lock(r->playback_cursor, (DWORD)available,
                                    &first, &first_bytes, &second,
                                    &second_bytes, 0) == DS_OK) {
              memset(first, 0, first_bytes);
              if (second_bytes != 0) memset(second, 0, second_bytes);
              d.play_buffer->Unlock(first, first_bytes, second, second_bytes);
            }
            r->playback_cursor =
                (r->playback_cursor + (DWORD)available) %
                r->playback_buffer_bytes;
            InterlockedExchange(&r->output_lock_word, 0);
          } else {
            r->playback_cursor =
                (r->playback_cursor + (DWORD)available) %
                r->playback_buffer_bytes;
            InterlockedExchange(&r->output_lock_word, 0);
          }
        }
      }
      InterlockedIncrement(&r->worker_wakeup_counter);
      Sleep(10);
    } while (InterlockedExchange(&r->sync_word_3b0, 1) != 0);
  }
  _endthread();
}

// ORIGINAL: 0x005CB050 sub_5cb050 0x005CB050-0x005CB220
// symbol    ?initialize_stream@MediaRuntime@@SIHPADPAU1@@Z
// kind      game
// TRIED: sub_5c9ec0 is catalogued nullary cdecl but the disassembly sets `ecx = esi` (this=a2) before every call - a free __thiscall declaration is C4234, so it is reached through the Dummy member-function-pointer trick. The four mmio* Windows multimedia calls go through fixed-address function-pointer globals rather than by name, since the catalogue does not model them. Landing the closest control-flow-faithful form (divergence starts at instruction #6, on the mmio-open guard).
int __fastcall MediaRuntime::initialize_stream(LPSTR filename,
                                               MediaRuntime *runtime) {
  MediaRuntime &r = *runtime;
  r.stream_eof = 0;
  DWORD signature = 0;
  if ((r.descriptor->flags & 0x00080000) == 0)
    r.descriptor->stream = mmioOpenA(filename, 0, 0);
  if (r.descriptor->stream == 0) return 1;
  if (mmioSetBuffer(r.descriptor->stream, 0, 0x4000, 0) != 0) return 1;
  mmioRead(r.descriptor->stream, (HPSTR)&signature, 4);
  mmioSeek(r.descriptor->stream, -4, SEEK_CUR);

  if (signature == 0x44414553 || signature == 0x6c484353 ||
      signature == 0x684e5331)
    r.descriptor->stream_gate_7c = 1;
  else
    r.descriptor->stream_gate_7c = 0;

  for (int i = 0; i != 24; ++i) {
    r.output_records[i].payload = 0;
    r.read_records[i].payload = 0;
  }

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

  if (r.descriptor->stream_gate_7c == 0) {
    while (r.pending_read_records < 0x14 && sub_5c9ec0(runtime, 0) != -1) {
    }
  } else {
    while (r.pending_output_records < 0x15 &&
           sub_5c9ec0(runtime, 0) != -1 &&
           sub_5c9ec0(runtime, 0) != -1) {
    }
  }
  r.descriptor->stream_gate_80 = (r.read_counter == 0);
  return 0;
}

// ORIGINAL: 0x005CA6C0 sub_5ca6c0 0x005CA6C0-0x005CA95B
// symbol    ?parse_initial_format@MediaRuntime@@QAIHH@Z
// kind      game
// TRIED: NOT a real parameter - Ghidra's `char param_1` is actually buf[4], a byte read from the just-mmioRead'd local buffer, because Ghidra doesn't track the alloca'd 0xc000 frame. Modeled instead as a __thiscall member (ecx->edi is `this`, no stack arg) with a large local `char buf[0xc000]` (reproduces the `mov eax,0xc000; call __alloca_probe` prologue for free) parsing a TLV byte stream: skip name fields until 0xfd, then dispatch tags 0x81/0x82/0x83/0x84 (big- endian length-prefixed sign-extended ints stored to this+0x5c/0x54/ 0x60/0x58) until tag 0x8a. Reordering local decls to match the original's `lea eax,[esp]`-before-prologue-pushes scheduling did not change the codegen; first divergence stays at instruction #2 (original computes the buffer pointer before saving edi, rebuilt pushes edi first) - register scheduling, not source form.
int __fastcall MediaRuntime::parse_initial_format(int byte_count) {
  BYTE bytes[0xc000];
  HMMIO stream = descriptor->stream;
  LONG read = mmioRead(stream, (HPSTR)bytes, byte_count);
  if (read == 0) return -1;
  if (read == -1) return -2;

  format_parameter_5c = 0x10;
  format_parameter_54 = 1;
  playback_frequency = 0x5622;
  format_dispatch_60 = 0;

  BYTE *cursor = bytes + 4;
  while (*cursor != 0xfd) {
    BYTE *length_byte = cursor + 1;
    cursor += 2;
    unsigned int skip = *length_byte;
    while (skip != 0) {
      ++cursor;
      --skip;
    }
  }

  BYTE *element = cursor + 1;
  BYTE tag = *element;
  for (;;) {
    if (tag == 0x8a) return 0;

    if (tag == 0x81 || tag == 0x82 || tag == 0x83 || tag == 0x84) {
      unsigned int count = element[1];
      BYTE *value_bytes = element + 2;
      DWORD raw = 0;
      unsigned int remaining = count;
      while (remaining != 0) {
        raw = (raw << 8) | *value_bytes++;
        --remaining;
      }
      int value = (int)raw;
      if (count == 1 && raw > 0x7f) value -= 0x100;
      else if (count == 2 && raw > 0x7fff) value -= 0x10000;
      else if (count == 3 && raw > 0x7fffff) value -= 0x1000000;

      if (tag == 0x81) format_parameter_5c = value;
      else if (tag == 0x82) format_parameter_54 = value;
      else if (tag == 0x83) format_dispatch_60 = value;
      else playback_frequency = (DWORD)value;
      element = value_bytes;
    } else {
      unsigned int skip = element[1];
      element += 2;
      while (skip != 0) {
        ++element;
        --skip;
      }
    }

    tag = *element;
  }
}

// The function is a stream-record dispatcher. This name is a source-owner
// candidate only; FourCC values and record-kind values stay numeric because
// no codec/library attribution is established by these branches.
// ORIGINAL: 0x005C9EC0 sub_5c9ec0 0x005C9EC0-0x005CA230
// symbol    @sub_5c9ec0@8
// kind      game
// TRIED: sub_5ca230/5ca310/5ca6c0 are __thiscall members of the object this function receives in ecx (mov ecx,esi before each direct call); the three `// indirect` sites are all the same cached mmioRead(HMMIO,HPSTR,LONG) stdcall pointer loaded from g_0066936c once and called through a register.
// TRIED: transcribed the fourcc/cch big-endian header decode as explicit byte shifts (buf[0..3], buf[4..7]) and the branch tree as goto-labelled ifs mirroring Ghidra 1:1; compiles and is structurally faithful but only 0.44 mnemonic similarity (996 vs 876 bytes) - did not reverse-engineer the exact esp-relative overlapping-load pattern the original uses to build the two big-endian dwords, nor match the alloca'd buffer's exact size.
// The retained continuity mapping reaches 0x005CA230; the old seed
// ended at 0x005CA22C. Preserve this historical extent discrepancy
// rather than silently treating either as fresh boundary proof.
extern "C" int __fastcall sub_5c9ec0(MediaRuntime *runtime, void *) {
  BYTE header_and_payload[0xc000];
  LONG read = mmioRead(runtime->descriptor->stream,
                       (HPSTR)header_and_payload, 8);
  if (read == 0) {
    runtime->stream_eof = 1;
    return -1;
  }
  if (read == -1 || read != 8) return -2;

  DWORD payload_bytes =
      ((DWORD)header_and_payload[4] << 24) |
      ((DWORD)header_and_payload[5] << 16) |
      ((DWORD)header_and_payload[6] << 8) |
      (DWORD)header_and_payload[7];
  payload_bytes -= 8;
  DWORD type =
      ((DWORD)header_and_payload[0] << 24) |
      ((DWORD)header_and_payload[1] << 16) |
      ((DWORD)header_and_payload[2] << 8) |
      (DWORD)header_and_payload[3];

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
  case 0x6b564754:
  case 0x5447566b:
    return runtime->sub_5ca230((int)payload_bytes, 1);
  case 0x6d44414d:
  case 0x4d41446d:
    return runtime->sub_5ca230((int)payload_bytes, 6);
  case 0x684e5331:
  case 0x6c484353:
  case 0x5343486c:
    return runtime->parse_initial_format((int)payload_bytes);
  case 0x43444e53:
  case 0x534e4443:
  case 0x644e5331:
  case 0x5343446c:
  case 0x6c444353:
    if (runtime->descriptor->stream_gate_7c == 0) return -2;
    return sub_5ca310(runtime, (int)payload_bytes);
  case 0x44414553: {
    read = mmioRead(runtime->descriptor->stream,
                    (HPSTR)(header_and_payload + 0x18),
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
                   (HPSTR)(header_and_payload + 0x18),
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
static int *const media_decoder_lookup_a =
    MediaAdpcmTables.lookup_and_delta_words;
static int *const media_decoder_lookup_b =
    MediaAdpcmTables.lookup_and_delta_words + 4;

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
      do {
        ++input;
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
      } while (output < end);
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
extern "C" void __cdecl sub_5cdd10(int count, int *cursor,
                                   int *left_predictor, int *right_predictor,
                                   int *left_accumulator, int *right_accumulator,
                                   int input_base, unsigned int *output_cursor) {
  short *output = (short *)*output_cursor;
  BYTE *input = (BYTE *)(input_base + (*cursor * 30) / 28);
  int left_predictor_state = *left_predictor;
  int right_predictor_state = *right_predictor;
  int left_accumulator_state = *left_accumulator;
  int right_accumulator_state = *right_accumulator;
  if (count > 0) {
    unsigned int blocks = ((unsigned int)count + 0x1b) / 0x1c;
    do {
      *cursor += 0x1c;
      unsigned int hi = *input >> 4, lo = *input & 0xf;
      int hi_a = media_decoder_lookup_a[hi], hi_b = media_decoder_lookup_b[hi];
      int lo_a = media_decoder_lookup_a[lo], lo_b = media_decoder_lookup_b[lo];
      int shift_hi = (input[1] >> 4) + 8, shift_lo = (input[1] & 0xf) + 8;
      short *end = output + 0x38;
      do {
        ++input;
        int sample_left = (((int)((*input & 0xf0) << 0x18) >> shift_hi) +
                           hi_b * left_accumulator_state + 0x80 +
                           hi_a * left_predictor_state) >> 8;
        int sample_right = (((int)((unsigned int)*input << 0x1c) >> shift_lo) +
                            lo_b * right_accumulator_state + 0x80 +
                            lo_a * right_predictor_state) >> 8;
        if (sample_left < -0x8000) sample_left = -0x8000;
        else if (sample_left > 0x7fff) sample_left = 0x7fff;
        if (sample_right < -0x8000) sample_right = -0x8000;
        else if (sample_right > 0x7fff) sample_right = 0x7fff;
        int next_left = (((int)((input[1] & 0xf0) << 0x18) >> shift_hi) +
                         sample_left * hi_a + 0x80 +
                         hi_b * left_predictor_state) >> 8;
        int next_right = (((int)((unsigned int)input[1] << 0x1c) >> shift_lo) +
                          sample_right * lo_a + 0x80 +
                          lo_b * right_predictor_state) >> 8;
        if (next_left < -0x8000) next_left = -0x8000;
        else if (next_left > 0x7fff) next_left = 0x7fff;
        if (next_right < -0x8000) next_right = -0x8000;
        else if (next_right > 0x7fff) next_right = 0x7fff;
        output[0] = (short)sample_left;
        output[1] = (short)sample_right;
        output[2] = (short)next_left;
        output[3] = (short)next_right;
        left_predictor_state = next_left;
        right_predictor_state = next_right;
        left_accumulator_state = sample_left;
        right_accumulator_state = sample_right;
        output += 4;
      } while (output < end);
      --blocks;
    } while (blocks != 0);
  }
  *left_predictor = left_predictor_state;
  *right_predictor = right_predictor_state;
  *left_accumulator = left_accumulator_state;
  *right_accumulator = right_accumulator_state;
  *output_cursor = (unsigned int)output;
}

// Register evidence: runtime in ECX, encoded byte count in EDX.
// ORIGINAL: 0x005CA310 sub_5ca310 0x005CA310-0x005CA6BC
// symbol    @sub_5ca310@8
// kind      game
// TRIED: __fastcall(ecx,edx) two-arg signature + large local buffer for __alloca_probe reaches MISMATCH #3 (prologue); several output scratch locals (sub_5cdd10/5cdb90/5ce6e3 args) use fresh locals rather than the exact original stack slot, since those callees are opaque and their slot placement doesn't affect this body.
extern "C" int __fastcall sub_5ca310(MediaRuntime *r, int byte_count) {
  BYTE input[0xc008];
  MediaDescriptor &d = *r->descriptor;
  int output_bytes;
  if (r->output_counter == 0 && (d.flags & 0x00008000) != 0)
    r->channel_seek_position = mmioSeek(d.stream, 0, SEEK_CUR) - 8;
  LONG read = mmioRead(d.stream, (HPSTR)(input + 0x14), byte_count);
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
      sub_5cdd10(count, &cursor, (int *)&r->decoder_state_3d8,
                 (int *)&r->decoder_state_3e0, (int *)&r->decoder_state_3dc,
                 (int *)&r->decoder_state_3e4, (int)(input + 0x24), &output);
      byte_count = output_bytes;
    } else {
      int count = encoded / 15;
      output_bytes = count * 0x70;
      if (r->decoded_ring_current + output_bytes >= r->decoded_ring_base + 0x493e0)
        r->decoded_ring_current = r->decoded_ring_base;
      output = (unsigned int)r->decoded_ring_current;
      sub_5cdb90(count, &cursor, (int *)&r->decoder_state_3d8,
                 (int *)&r->decoder_state_3e0, (int)(input + 0x24), &output);
      byte_count = output_bytes;
    }
  } else if (r->format_dispatch_60 == 0x0a) {
    output_bytes = byte_count * 4;
    if (r->decoded_ring_current + output_bytes >= r->decoded_ring_base + 0x493e0)
      r->decoded_ring_current = r->decoded_ring_base;
    int predictors[2] = {(int)r->decoder_state_3e8, (int)r->decoder_state_3ec};
    int indices[2] = {(int)r->decoder_state_3f0, (int)r->decoder_state_3f4};
    sub_5ce6e3(input + 0x14, (short *)r->decoded_ring_current, byte_count,
               predictors, indices);
    r->decoder_state_3e8 = predictors[0]; r->decoder_state_3ec = predictors[1];
    r->decoder_state_3f0 = indices[0]; r->decoder_state_3f4 = indices[1];
    byte_count = output_bytes;
  } else {
    byte_count = *(int *)(input + 0x14);
  }

  if (r->channel_state == 2) d.stream_gate_80 = 1;
  if (d.stream_gate_80 != 0) InterlockedIncrement(&r->read_counter);
  int slot = r->output_producer_cursor % 24;
  r->output_records[slot].payload = r->decoded_ring_current;
  r->output_records[slot].field_04 = (DWORD)byte_count;
  r->output_records[slot].field_08 = (DWORD)r->read_counter;
  r->output_records[slot].field_0c = (DWORD)r->output_counter;
  r->decoded_ring_current += byte_count;
  InterlockedIncrement(&r->output_producer_cursor);
  InterlockedIncrement(&r->pending_output_records);
  InterlockedIncrement(&r->output_counter);
  return 2;
}

// ORIGINAL: 0x005CA970 sub_5ca970 0x005CA970-0x005CAAA1
// symbol    @sub_5ca970@8
// kind      game
extern "C" int __fastcall sub_5ca970(MediaRuntime *runtime, void *) {
  MediaDescriptor &d = *runtime->descriptor;
  LONG handle = runtime->channel_state;
  if (handle == -1) {
    if ((d.flags & 0x00008000) != 0) {
      Sleep(0);
      mmioSeek(d.stream, runtime->channel_seek_position, SEEK_SET);
      handle = sub_5c9ec0(runtime, 0);
    } else {
      handle = -1;
    }
    runtime->channel_state = handle;
  }

  if (handle == -1) return 0;
  if (runtime->pending_output_records >= 0x16 ||
      runtime->pending_read_records >= 0x16)
    return 0;

  int result = sub_5c9ec0(runtime, 0);
  runtime->channel_poll_result = result;
  if (result == -1) {
    if ((d.flags & 0x00008000) != 0) {
      Sleep(0);
      mmioSeek(d.stream, runtime->channel_seek_position, SEEK_SET);
      runtime->channel_state = sub_5c9ec0(runtime, 0);
    } else {
      runtime->channel_state = -1;
    }
    return 0;
  }

  if (result == 1) {
    if (d.stream_gate_7c != 0) {
      runtime->channel_state = sub_5c9ec0(runtime, 0);
      if (runtime->channel_state == 1) d.stream_gate_7c = 0;
    }
    return 0;
  }

  if (result == 2 &&
      (d.stream_gate_80 == 0 || runtime->pending_output_records < 0x16))
    runtime->channel_state = sub_5c9ec0(runtime, 0);
  return 0;
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
  MediaDescriptor &d = *descriptor;
  if ((d.flags & 0x00800000) == 0) return 0;

  if (d.sound_device == 0) {
    release_sound_device = 1; // set before DirectSoundCreate attempt
    if (DirectSoundCreate(0, &d.sound_device, 0) != DS_OK) {
      d.sound_device = 0;
    } else if (d.sound_device->SetCooperativeLevel(
                   d.window, DSSCL_PRIORITY) != DS_OK) {
      d.sound_device->Release();
      d.sound_device = 0;
    }
  }
  if (d.sound_device == 0) {
    d.unresolved_source_or_rate_58 = 0;
    return 0;
  }

  DSCAPS device_caps;
  memset(&device_caps, 0, 0x60);
  device_caps.dwSize = 0x60;
  d.sound_device->GetCaps(&device_caps); // result ignored by native body
  emulated_device = (device_caps.dwFlags & DSCAPS_EMULDRIVER) != 0;
  IDirectSoundBuffer *source_buffer =
      reinterpret_cast<IDirectSoundBuffer *>(
          d.unresolved_source_or_rate_58);
  if ((d.flags & 0x00400000) != 0 && source_buffer != 0) {
    DSBCAPS source_caps;
    memset(&source_caps, 0, 0x14);
    source_caps.dwSize = 0x14;
    source_buffer->GetCaps(&source_caps); // result ignored
    source_buffer_bytes = source_caps.dwBufferBytes;
  } else {
    source_buffer_bytes = 0x8000;
  }
  return 0;
}

// ORIGINAL: 0x005CE4B0 sub_5ce4b0 0x005CE4B0-0x005CE61A
// symbol    ?create_playback_buffer@MediaRuntime@@QAEHXZ
// kind      game
// TRIED: `ecx` does real work with no stack access at entry (reads `[ecx]` before any `[ebp+..]`), so this is a __thiscall member - written as `ClassX::sub_5ce4b0()`, same precedent as 0x004E3300's ClassX::sub_4e3300. The six indirect calls are DirectDraw-COM-shaped (self pushed explicitly alongside the args, not via ECX), spelled with `__stdcall` function-pointer typedefs per call site rather than a shared VCall shim, since two calls land on offset 0xc through DIFFERENT vtables (one via `[this+0x54]`, two via `this->[0]->0x5c`) with different arities (4 args vs 2). Local DDSURFACEDESC/DDPIXELFORMAT-like scratch structs are modelled as plain structs, not the overlapping/self-referential stack layout the original build (a nested pointer field pointing back into the same frame) - not chased further. 0.83 mnemonic similarity; first divergence at #3 is the prologue register-save order, which follows from local-variable layout this body does not reproduce.
int MediaRuntime::create_playback_buffer() {
  MediaDescriptor &d = *descriptor;
  if (d.sound_device != 0) {
    WAVEFORMATEX format;
    memset(&format, 0, sizeof(format));
    format.wFormatTag = WAVE_FORMAT_PCM;
    format.nChannels = 2;
    format.nSamplesPerSec = d.unresolved_source_or_rate_58;
    format.nAvgBytesPerSec = d.unresolved_source_or_rate_58 << 2;
    format.nBlockAlign = 4;
    format.wBitsPerSample = 16;

    DSBUFFERDESC buffer_desc;
    memset(&buffer_desc, 0, 0x14);
    buffer_desc.dwSize = 0x14;
    buffer_desc.dwFlags = 0x000100e0;
    buffer_desc.dwBufferBytes = source_buffer_bytes;
    buffer_desc.dwReserved = 0;
    buffer_desc.lpwfxFormat = &format;
    if (d.sound_device->CreateSoundBuffer(&buffer_desc, &d.play_buffer, 0) == DS_OK) {
      d.play_buffer->SetFrequency(d.unresolved_source_or_rate_58);
      if ((d.flags & 0x2000) != 0) d.play_buffer->SetVolume(d.volume_argument);

      DSBCAPS created_caps;
      memset(&created_caps, 0, 0x14);
      created_caps.dwSize = 0x14;
      if (d.play_buffer->GetCaps(&created_caps) == DS_OK) {
        synthetic_cursor_mode = 0;
        playback_buffer_bytes = created_caps.dwBufferBytes;
        playback_cursor = 0;
        if (d.play_buffer->Play(0, 0, DSBPLAY_LOOPING) == DS_OK) {
          buffer_setup_finished = 1;
          return 0;
        }
      }
      d.play_buffer->Release();
    }
  }
  d.play_buffer = 0;
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
  MediaDescriptor &d = *descriptor;
  if ((d.flags & 0x00800000) != 0) {
    if (release_optional_buffer != 0 &&
        d.unresolved_source_or_rate_58 != 0) {
      IDirectSoundBuffer *source_buffer =
          reinterpret_cast<IDirectSoundBuffer *>(
              d.unresolved_source_or_rate_58);
      source_buffer->Release();
      d.unresolved_source_or_rate_58 = 0;
    }
    if (release_sound_device != 0 && d.sound_device != 0) {
      d.sound_device->Release();
      d.sound_device = 0;
    }
  }
  return 0;
}

// ORIGINAL: 0x005CE620 sub_5ce620 0x005CE620-0x005CE63F BYTE_EXACT
// symbol    ?release_playback_buffer@MediaRuntime@@QAEHXZ
// kind      game
int MediaRuntime::release_playback_buffer() {
  MediaDescriptor &d = *descriptor;
  if (d.play_buffer != 0) {
    d.play_buffer->Release();
    d.play_buffer = 0;
  }
  return 0;
}

// ORIGINAL: 0x005CAE00 sub_5cae00 0x005CAE00-0x005CAE79 BYTE_EXACT
// symbol    ?sub_5cae00@MediaRuntime@@QAEHXZ
// kind      game
int MediaRuntime::sub_5cae00() {
  MediaDescriptor &d = *descriptor;
  if ((d.flags & 0x00080000) == 0 && d.stream != 0)
    mmioClose(d.stream, 0);
  if (input_ring_base != 0) free(input_ring_base);
  if (decoded_ring_base != 0) free(decoded_ring_base);
  for (int i = 0; i != 24; ++i) read_records[i].payload = 0;
  if (d.stream_gate_7c != 0)
    for (int j = 0; j != 24; ++j) output_records[j].payload = 0;
  return 0;
}

// ORIGINAL: 0x005CC710 sub_5cc710 0x005CC710-0x005CC856
// symbol    ?sub_5cc710@MediaRuntime@@QAEXXZ
// kind      game
// TRIED: the four vtable-slot-2 calls read `push piVar1; mov ecx, [piVar1]; call dword ptr [ecx+8]` - a plain function-pointer call taking one explicit pointer argument, NOT a thiscall dispatch (the receiver never lands in ecx before the call), so these are cdecl function-pointer casts, not virtual calls through an introduced class. `sub_5cbae0` similarly cannot be called by name (catalogued unmangled with no CRT_SIGNATURES entry, so declfix leaves it nullary - C2660 direct, C2733 redeclared) and the real call passes `this` in ecx and a literal 0 in edx with no stack push, i.e. __fastcall(void*, int); routed through a __fastcall function-pointer cast. Best reached: MISMATCH, edit_count 4, 338 bytes vs 326, mnemonic similarity ~0.98.
void MediaRuntime::sub_5cc710() {
  if (teardown_gate_3c != 0) {
    if (owned_heap_648 != 0) { free(owned_heap_648); owned_heap_648 = 0; }
    if (owned_heap_64c != 0) { free(owned_heap_64c); owned_heap_64c = 0; }
    if (owned_heap_650 != 0) { free(owned_heap_650); owned_heap_650 = 0; }
    if (owned_heap_638 != 0) { free(owned_heap_638); owned_heap_638 = 0; }
    if (owned_heap_63c != 0) { free(owned_heap_63c); owned_heap_63c = 0; }
    if (owned_heap_644 != 0) { free(owned_heap_644); owned_heap_644 = 0; }
    if (owned_heap_640 != 0) { free(owned_heap_640); owned_heap_640 = 0; }

    if (queue_record_kind == 2 && output_handle_candidate != 0)
      output_handle_candidate = (DWORD)update_overlay(0);

    DWORD flags = descriptor->flags;
    if (descriptor->palette_24 != 0 && (flags & 0x04000000) == 0) {
      descriptor->palette_24->Release();
      descriptor->palette_24 = 0;
    }
    if (descriptor->surface_20 != 0 && (flags & 0x00000800) == 0) {
      descriptor->surface_20->Release();
      descriptor->surface_20 = 0;
    }
    if (descriptor->surface_1c != 0 && (flags & 0x00000800) == 0) {
      descriptor->surface_1c->Release();
      descriptor->surface_1c = 0;
    }
    if (release_interface_10 != 0) {
      if (descriptor->draw_device_10 != 0) {
        descriptor->draw_device_10->Release();
        descriptor->draw_device_10 = 0;
      }
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
  MediaDescriptor &d = *descriptor;
  DWORD flags = d.flags;
  DWORD saved_output = output_handle_candidate;
  if ((flags & 0x00000800) != 0) return;

  if (queue_record_kind == 1) {
    if ((flags & 0x00000400) == 0) {
      IDirectDrawSurface2 *target =
          (flags & 0x08000000) != 0 ? d.surface_18 : d.surface_14;
      if (target != 0) {
        if (d.pixel_format.total_component_bits == 8 &&
            d.palette_entries_28 != 0) {
          BYTE *out = d.palette_entries_28;
          for (int i = 0; i < 0x300; i += 3) {
            *out++ = d.palette_rgb_triplets_b0[i];
            *out++ = d.palette_rgb_triplets_b0[i + 1];
            *out++ = d.palette_rgb_triplets_b0[i + 2];
            *out++ = 0;
          }
        }

        HRESULT status = d.surface_1c->IsLost();
        if (status == DDERR_SURFACELOST && d.surface_1c->Restore() != DD_OK)
          return;

        RECT destination;
        destination.left = (LONG)output_rect_left_14;
        destination.top = (LONG)output_rect_top_18;
        destination.right = (LONG)(output_rect_left_14 + output_rect_extent_0c);
        destination.bottom = (LONG)(output_rect_top_18 +
                                     output_rect_or_draw2_bits_10);
        RECT source;
        source.left = 0;
        source.top = 0;
        source.right = (LONG)d.output_width_64 - 1;
        source.bottom = (LONG)d.output_height_60 - 1;

        if ((flags & 0x00000020) == 0) {
          if ((flags & 0x00000010) == 0) {
            target->Blt(&destination, d.surface_1c, &source,
                        DDBLT_ASYNC, 0);
          } else if (d.output_height_60 != 0) {
            for (DWORD row = 0; row < d.output_height_60; row += 2) {
              RECT row_destination = destination;
              RECT row_source = source;
              row_destination.top = (LONG)(output_rect_top_18 + row);
              row_destination.bottom = row_destination.top + 1;
              row_source.top = (LONG)(row >> 1);
              row_source.bottom = row_source.top + 1;
              target->Blt(&row_destination, d.surface_1c, &row_source,
                          DDBLT_ASYNC, 0);

              RECT second_destination = row_destination;
              second_destination.top += 1;
              second_destination.bottom = second_destination.top + 1;
              target->Blt(&second_destination, d.surface_1c, &row_source,
                          DDBLT_ASYNC, 0);
            }
          }
        } else if (d.output_height_60 != 0) {
          for (DWORD row = 0; row < d.output_height_60; row += 2) {
            RECT row_destination = destination;
            RECT row_source = source;
            row_destination.top = (LONG)(output_rect_top_18 + row);
            row_destination.bottom = row_destination.top + 1;
            row_source.top = (LONG)(row >> 1);
            row_source.bottom = row_source.top + 1;
            target->Blt(&row_destination, d.surface_1c, &row_source,
                        DDBLT_ASYNC, 0);
          }
        }
      }
    }
  } else if (queue_record_kind == 2) {
    if (d.surface_1c != 0) {
      if (saved_output != 0)
        output_handle_candidate = (DWORD)update_overlay(1);

      DDSURFACEDESC locked;
      memset(&locked, 0, 0x6c);
      locked.dwSize = 0x6c;
      if (d.surface_1c->Lock(0, &locked, DDLOCK_WAIT, 0) != DD_OK) return;
      DWORD group_count = (d.output_width_64 & 0x7fffffff) >> 1;
      BYTE *source_row = owned_heap_638;
      BYTE *destination_row = (BYTE *)locked.lpSurface;
      DWORD groups = (group_count + 7) >> 3;
      for (DWORD row = 0; row < d.output_height_60; ++row) {
        BYTE *source = source_row;
        BYTE *destination = destination_row;
        for (DWORD group = 0; group < groups; ++group) {
          DWORD *src = (DWORD *)source;
          DWORD *dst = (DWORD *)destination;
          dst[0] = src[0]; dst[1] = src[1];
          dst[2] = src[2]; dst[3] = src[3];
          dst[4] = src[4]; dst[5] = src[5];
          dst[6] = src[6]; dst[7] = src[7];
          source += 0x20;
          destination += 0x20;
        }
        source_row += group_count * 4;
        destination_row += group_count * 4;
      }
      d.surface_1c->Unlock(0);
    }
  } else if (queue_record_kind == 4) {
    IDirectDraw2 *draw_device =
        reinterpret_cast<IDirectDraw2 *>(output_rect_or_draw2_bits_10);
    if ((flags & 0x00001000) != 0 && draw_device != 0)
      draw_device->WaitForVerticalBlank(DDWAITVB_BLOCKBEGIN, 0);

    if (d.surface_1c != 0 && d.surface_14 != 0) {
      HRESULT status = d.surface_1c->IsLost();
      if (status == DDERR_SURFACELOST && d.surface_1c->Restore() != DD_OK)
        return;

      RECT destination;
      destination.left = (LONG)output_rect_left_14;
      destination.top = (LONG)output_rect_top_18;
      destination.right = (LONG)(output_rect_left_14 + output_rect_extent_0c);
      destination.bottom = (LONG)(output_rect_top_18 +
                                   output_rect_or_draw2_bits_10);
      RECT source;
      source.left = 0;
      source.top = 0;
      source.right = (LONG)d.output_width_64 - 1;
      source.bottom = (LONG)d.output_height_60 - 1;
      if (d.surface_14->Blt(&destination, d.surface_1c, &source,
                            DDBLT_ASYNC, 0) != DD_OK)
        queue_record_kind = 1;
    }
  }

  if ((flags & 0x08000000) != 0 && queue_record_kind == 1) {
    if (d.pixel_format.total_component_bits == 8) {
      if (d.surface_14 != 0) d.surface_14->Flip(0, DDFLIP_WAIT);
      if (d.palette_24 != 0 && d.palette_entries_28 != 0)
        d.palette_24->SetEntries(0, 0, 0x100,
                                 (LPPALETTEENTRY)d.palette_entries_28);
      return;
    }
    if (d.surface_14 != 0) {
      HRESULT result = d.surface_14->Flip(0, DDFLIP_WAIT);
      if (result != DD_OK) OutputDebugStringA(sub_5cf120(result));
    }
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
         11, then swaps the pointer-shaped +0x4c and DWORD +0x50 values.

         The original reloads [ECX] between stores in both groups. The typed
         owner keeps a descriptor reload at each corresponding point.

// ORIGINAL: 0x005CBBC0 sub_5cbbc0 0x005CBBC0-0x005CBBF5 FILE SEMANTIC
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
    // bit 11. The latter deliberately exchanges pointer bits and a DWORD.
    MediaDescriptor *view = descriptor;
    if (view->surface_20 != 0 && queue_record_kind == 1) {
        IDirectDrawSurface2 *previous = view->surface_1c;
        view->surface_1c = view->surface_20;
        view = descriptor;
        view->surface_20 = previous;
    }
    view = descriptor;
    if ((view->flags & 0x800U) != 0) {
        const DWORD previous_pitch = view->provided_output_pitch_50;
        const DWORD previous_base = static_cast<DWORD>(
            reinterpret_cast<uintptr_t>(view->provided_output_base_4c));
        view->provided_output_pitch_50 = previous_base;
        view = descriptor;
        view->provided_output_base_4c = reinterpret_cast<BYTE *>(
            static_cast<uintptr_t>(previous_pitch));
    }
}

// ORIGINAL: 0x005CB220 sub_5cb220 0x005CB220-0x005CB60B
// symbol    ?sub_5cb220@MediaDescriptor@@QAEXXZ
// kind      game
// TRIED: __thiscall member (class Sub5cb220Obj) with `this` reassigned to *(int**)this, matching Ghidra's structure closely; BOTH indirect calls (slots 9 and 12) are COM-style (interface pushed as an explicit stack arg, not passed in ecx) despite the brief's emitter flagging only slot 9 - confirmed by the raw asm's "push eax; call [ecx+0x30]" shape for slot 12 too, per LEVER 3. MISMATCH #0 (frame layout), not chased further.
void MediaDescriptor::sub_5cb220() {
  MediaRuntime *r = runtime;
  // Keep construction before either native cleanup jump target can be
  // reached; the slot is assigned immediately before its first read.
  MediaQueueRecord *record = 0;
  if ((flags & 0x00030000) == 0x00030000)
    SetThreadPriority(GetCurrentThread(), (int)priority);
  if (r->sub_5ce2b0() != 0) goto finish_worker;

  progress_value = 0;
  InterlockedExchange(&state, 1);
  r->start_ticks = GetTickCount();
worker_loop:
  for (;;) {
    LONG observed = InterlockedExchange(&state, 2);
    if (observed == 3) {
      InterlockedExchange(&state, 3);
      do {
        Sleep(20);
        observed = InterlockedExchange(&state, 3);
      } while (observed == 3);
      InterlockedExchange(&state, observed);
    }
    if (observed == 4) goto stop_worker;
    if (observed == 2) break;
  }

  Sleep(0);
  if (r->channel_state == -1) {
    if ((flags & 0x00008000) == 0) {
      r->channel_state = -1;
    } else {
      Sleep(0);
      mmioSeek(stream, r->channel_seek_position, SEEK_SET);
      r->channel_state = sub_5c9ec0(r, 0);
    }
    if (r->channel_state != -1) goto process_stream;
  } else {
process_stream:
    if (r->pending_output_records < 0x16 && r->pending_read_records < 0x16) {
      int result = sub_5c9ec0(r, 0);
      r->channel_poll_result = result;
      if (result == -1) {
        if ((flags & 0x00008000) == 0) {
          r->channel_state = -1;
        } else {
          Sleep(0);
          mmioSeek(stream, r->channel_seek_position, SEEK_SET);
          r->channel_state = sub_5c9ec0(r, 0);
        }
      } else if (result == 1) {
        if (stream_gate_7c != 0) {
          r->channel_state = sub_5c9ec0(r, 0);
          if (r->channel_state == 1) stream_gate_7c = 0;
        }
      } else if (result == 2 &&
                 (stream_gate_80 == 0 || r->pending_output_records < 0x16)) {
        r->channel_state = sub_5c9ec0(r, 0);
      }
    }
  }

  if (r->buffer_setup_finished == 0 && stream_gate_7c != 0) {
    r->create_playback_buffer();
    r->output_lock_word = 0;
  }
  record = &r->read_records[r->read_consumer_cursor % 24];
  if (record->payload != 0) {
    if (progress_value < 3) {
      sub_5cc940(r, record);
      r->sub_5cb6c0();
    } else {
      DWORD kind = record->field_0c;
      if (kind == 2 || kind == 1 || kind == 6 || kind == 5)
        sub_5cc940(r, record);
    }
    if ((flags & 0x00040000) != 0 && callback != 0 && callback() != 0)
      goto stop_worker;
    record->payload = 0;
    record->field_04 = 0;
    InterlockedIncrement(&r->read_consumer_cursor);
    InterlockedDecrement(&r->pending_read_records);
    r->sub_5cbbc0();
  }

  InterlockedIncrement(&progress_counter);
  if (r->pending_read_records == 0) {
    if (stream_gate_7c == 0 || (flags & 0x00800000) == 0)
      goto stop_worker;
    if (r->pending_output_records == 0) {
      if (play_buffer == 0) goto stop_worker;
      DWORD status = 0;
      play_buffer->GetStatus(&status);
      if ((status & 0x4) != 0) play_buffer->Play(0, 0, 0);
      if ((status & 0x1) == 0) goto stop_worker;
    }
  }
  if (stream_gate_7c != 0) {
    int lag = (int)r->output_lock_word - 5;
    LONG base = progress_counter;
    int attempts = 0;
    while (base - lag >= 1 && attempts <= 0x96) {
      Sleep(0);
      Sleep(3);
      lag = (int)r->output_lock_word - 5;
      ++attempts;
    }
    progress_value = lag - base;
    if (progress_value < 0) progress_value = 0;
  }
  goto worker_loop;

stop_worker:
  r->end_ticks = GetTickCount();
  r->sub_5ce2f0();
finish_worker:
  r->release_playback_buffer();
  InterlockedExchange(&state, 5);
}

// ORIGINAL: 0x005CAE80 sub_5cae80 0x005CAE80-0x005CB046
// symbol    _sub_5cae80
// kind      game
// TRIED: 0x005CB050's scaffolded prototype takes only one fastcall param, but the real callee reads its object from EDX (`mov esi, edx` at its own entry) with ECX dead - called here through an explicit fastcall(LPSTR, int*) function-pointer cast rather than by name, since a second extern "C" declaration for the same symbol conflicts. The four thiscall callees (sub_5ce340/sub_5cbe30/sub_5cb220/sub_5ce450/ sub_5cc710) are similarly unreachable by name (VC6 C4234 on a free __thiscall) and go through the Dummy member-function- pointer trick. Landing the closest control-flow-faithful form (divergence starts at instruction #12, inside the calloc/zero preamble).
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
  InterlockedExchange(&descriptor->state, 0);
  if (descriptor == 0) return 0; // native check follows earlier dereferences

  runtime->protocol_gate = 1; // before either sync or async branch
  if ((descriptor->flags & 0x00010000) != 0) {
    uintptr_t thread = _beginthread(sub_5caab0, 0, descriptor);
    descriptor->beginthread_result = (DWORD)thread;
    if (thread == (uintptr_t)-1) return 0; // native path does not free runtime
    if (InterlockedExchange(&descriptor->state, 1) != 2) {
      do {
        Sleep(20);
      } while (InterlockedExchange(&descriptor->state, 1) != 2);
    }
    InterlockedExchange(&descriptor->state, 2);
    return descriptor;
  }

  descriptor->sub_5cb220();
  if (runtime->protocol_gate != 0) {
    descriptor->elapsed_ticks = runtime->end_ticks - runtime->start_ticks;
    if ((descriptor->flags & 0x00080000) == 0 && descriptor->stream != 0)
      mmioClose(descriptor->stream, 0);
    if (runtime->input_ring_base != 0) free(runtime->input_ring_base);
    if (runtime->decoded_ring_base != 0) free(runtime->decoded_ring_base);
    for (int i = 0; i != 24; ++i) runtime->read_records[i].payload = 0;
    if (descriptor->stream_gate_7c != 0)
      for (int j = 0; j != 24; ++j) runtime->output_records[j].payload = 0;
    runtime->release_session_interfaces();
    runtime->sub_5cc710();
    InterlockedExchange(&descriptor->state, 6);
    InterlockedExchange((LONG *)&descriptor->runtime, 0);
    free(runtime);
  }
  return 0;
}

// ORIGINAL: 0x005CB610 sub_5cb610 0x005CB610-0x005CB652
// symbol    _sub_5cb610
// kind      game
extern "C" int __cdecl sub_5cb610(MediaDescriptor *d) {
  if (d->runtime != 0) {
    if (d->runtime->protocol_gate != 0) return d->state != 6;
    if (d->state == 2 || d->state == 3) return 1;
  }
  return 0;
}

// ORIGINAL: 0x005CB660 sub_5cb660 0x005CB660-0x005CB6BF
// symbol    _sub_5cb660
// kind      game
extern "C" int __cdecl sub_5cb660(MediaDescriptor *d) {
  DWORD expected = 6;
  MediaRuntime *runtime = d->runtime;
  if (runtime != 0 && runtime->protocol_gate == 0) expected = 5;
  InterlockedExchange(&d->state, 4);
  while (d->state != (LONG)expected) Sleep(10);
  return 0;
}

// ORIGINAL: 0x005CAAB0 sub_5caab0 0x005CAAB0-0x005CADF9
// symbol    _sub_5caab0
// kind      game
// TRIED: nullary extern "C" __cdecl per the contract head - IDA's own guess (void*) matches the disassembly's [esp+0x14] read, so landed as sub_5caab0(void *param); this is a _beginthread worker (GetCurrentThread/SetThreadPriority/InterlockedXxx/ Sleep/GetTickCount/_endthread). sub_5cXXXXX helpers retyped __fastcall(void*[, void*]) to match their ecx/edx-only call sites (no stack pushes); slots 9/12 are COM-style (explicit self push). Reaches #10 (mismatch inside the flag mask/store sequence); not pursued further given the size.
extern "C" void __cdecl sub_5caab0(void *argument) {
  MediaDescriptor *d = (MediaDescriptor *)argument;
  MediaRuntime *r = d->runtime;
  DWORD background_flag = d->flags & 0x00010000;
  if ((d->flags & 0x00030000) == 0x00030000)
    SetThreadPriority(GetCurrentThread(), (int)d->priority);

  if (r->sub_5ce2b0() == 0) {
    d->progress_value = 0;
    InterlockedExchange(&d->state, 1);
    r->start_ticks = GetTickCount();
thread_loop:
    ;
    MediaQueueRecord *record = 0;
    for (;;) {
      LONG observed = InterlockedExchange(&d->state, 2);
      record = &r->read_records[r->read_consumer_cursor % 24];
      if (observed == 3) {
        InterlockedExchange(&d->state, 3);
        do {
          Sleep(20);
          observed = InterlockedExchange(&d->state, 3);
        } while (observed == 3);
        InterlockedExchange(&d->state, observed);
      }
      if (observed == 4) goto stop_thread;
      if (observed == 2) break;
    }

    Sleep(0);
    sub_5ca970(r, 0);
    if (r->buffer_setup_finished == 0 && d->stream_gate_7c != 0) {
      r->create_playback_buffer();
      r->output_lock_word = 0;
    }
    if (record->payload != 0) {
      if (d->progress_value < 3) {
        sub_5cc940(r, record);
        r->sub_5cb6c0();
      } else {
        DWORD kind = record->field_0c;
        if (kind == 2 || kind == 1 || kind == 6 || kind == 5)
          sub_5cc940(r, record);
      }
      if ((d->flags & 0x00040000) != 0 && d->callback != 0 &&
          d->callback() != 0)
        goto stop_thread;
      record->payload = 0;
      record->field_04 = 0;
      InterlockedIncrement(&r->read_consumer_cursor);
      InterlockedDecrement(&r->pending_read_records);
      r->sub_5cbbc0();
    }

    InterlockedIncrement(&d->progress_counter);
    if (r->pending_read_records == 0) {
      if (d->stream_gate_7c == 0 || (d->flags & 0x00800000) == 0)
        goto stop_thread;
      if (r->pending_output_records == 0) {
        if (d->play_buffer == 0) goto stop_thread;
        DWORD status = 0;
        d->play_buffer->GetStatus(&status);
        if ((status & 0x4) != 0) d->play_buffer->Play(0, 0, 0);
        if ((status & 0x1) == 0) goto stop_thread;
      }
    }
    if (d->stream_gate_7c != 0) {
      int lag = (int)r->output_lock_word - 5;
      LONG base = d->progress_counter;
      int attempts = 0;
      while (base - lag >= 1 && attempts <= 0x96) {
        Sleep(0);
        Sleep(3);
        lag = (int)r->output_lock_word - 5;
        ++attempts;
      }
      d->progress_value = lag - base;
      if (d->progress_value < 0) d->progress_value = 0;
    }
    goto thread_loop;
  }

  goto release_playback;
stop_thread:
  r->end_ticks = GetTickCount();
  r->sub_5ce2f0();
release_playback:
  r->release_playback_buffer();
  InterlockedExchange(&d->state, 5);
  if (r->protocol_gate != 0) {
    d->elapsed_ticks = r->end_ticks - r->start_ticks;
    r->sub_5cae00();
    r->release_session_interfaces();
    r->sub_5cc710();
    InterlockedExchange(&d->state, 6);
    InterlockedExchange((LONG *)&d->runtime, 0);
    free(r);
  }
  if (background_flag != 0) _endthread();
}

// ORIGINAL: 0x005CD750 sub_5cd750 0x005CD750-0x005CD8C5
// TRIED: `extern "C" int __cdecl sub_5cd750()` fallback head; the entry `mov esi,ecx / mov [ebp-4],edx` with no stack reads is __fastcall(int*, uint*), so the signature was changed.
// symbol    @sub_5cd750@8
// kind      game
extern "C" void __fastcall sub_5cd750(MediaRuntime *runtime,
                                      BYTE *packet) {
  MediaDescriptor &descriptor = *runtime->descriptor;
  DWORD word;

  // The native loads are unaligned dwords followed by SHR 16.  Preserve the
  // complete access widths while expressing them without unaligned aliasing.
  memcpy(&word, packet - 2, sizeof(word));
  descriptor.output_width_64 = word >> 16;
  memcpy(&word, packet, sizeof(word));
  descriptor.output_height_60 = word >> 16;
  memcpy(&word, packet + 2, sizeof(word));
  runtime->unresolved_word_618 = word >> 16;
  memcpy(&word, packet + 4, sizeof(word));
  runtime->unresolved_word_61c = word >> 16;

  DWORD row = 0;
  if (runtime->unresolved_word_61c != 0) {
    const BYTE *sample = packet + 13;
    DWORD palette_index = 0;
    while (row < runtime->unresolved_word_61c) {
      const DWORD scale = descriptor.setup_value_78;
      const DWORD first = (DWORD)sample[-1] * scale;
      const DWORD second = (DWORD)sample[0] * scale;
      const DWORD third = (DWORD)sample[1] * scale;

      // Keep the original unsigned constant-divide source expression.  The
      // pinned VC6 lowering is MUL by 0x51EB851F followed by SHR 5.
      DWORD first_percent = first / 100;
      DWORD second_percent = second / 100;
      DWORD third_percent = third / 100;
      if (first_percent > 0xff) first_percent = 0xff;
      if (second_percent > 0xff) second_percent = 0xff;
      if (third_percent > 0xff) third_percent = 0xff;

      descriptor.palette_rgb_triplets_b0[palette_index] = (BYTE)first_percent;
      descriptor.palette_rgb_triplets_b0[palette_index + 1] = (BYTE)second_percent;
      descriptor.palette_rgb_triplets_b0[palette_index + 2] = (BYTE)third_percent;
      ++row;
      palette_index += 3;
      sample += 3;
    }
  }
}

// ORIGINAL: 0x005CD8D0 sub_5cd8d0 0x005CD8D0-0x005CD97B
// TRIED: the given `__stdcall(int,int,int)` head - disasm reads edx with no prior init (a real 2nd register param) and `push ecx` at entry is pure stack reservation, not `this` preservation; rewrote as `__fastcall(int*,int,int,int,uint)` matching Ghidra's 5-param read, with differently-named cdecl externs for the two nullary-scaffolded callees. MISMATCH #4 push/mov, prologue register shape only.
// symbol    @sub_5cd8d0@20
// kind      game
extern "C" void __fastcall sub_5cd8d0(MediaRuntime *runtime,
                                      BYTE *compressed, BYTE *destination,
                                      int unused, unsigned int pitch) {
  // The second fastcall argument arrives in EDX and is retained in +5F8.
  // The unused fourth logical argument remains present for the native stack
  // cleanup and is deliberately not repurposed.
  MediaDescriptor &descriptor = *runtime->descriptor;
  runtime->decoder_context_3f8.predictor_500 = 0;
  runtime->decoder_context_3f8.predictor_4fc = 0;
  runtime->decoder_context_3f8.predictor_4f8 = 0;
  runtime->decoder_context_3f8.compressed_input_200 = compressed;
  runtime->decoder_context_3f8.bit_position_204 = 0;
  (void)unused;
  DWORD block_y = 0;
  DWORD row_step = (pitch >> 1) << 5;
  BYTE *row_destination = destination;
  while (block_y < descriptor.output_height_60) {
    DWORD block_x = 0;
    BYTE *block_destination = row_destination;
    while (block_x < descriptor.output_width_64) {
      if (runtime->record_option_040 != 0) {
        sub_5d1466(block_destination, (int)pitch,
                   &runtime->decoder_context_3f8);
      } else {
        sub_5d1842(block_destination, (int)pitch,
                   &runtime->decoder_context_3f8);
      }
      block_x += 0x10;
      block_destination += 0x20;
    }
    row_destination += row_step;
    block_y += 0x10;
  }
}

// ORIGINAL: 0x005CD980 sub_5cd980 0x005CD980-0x005CDA2C
// symbol    @sub_5cd980@20
// kind      game
extern "C" void __fastcall sub_5cd980(MediaRuntime *runtime,
                                      BYTE *compressed, BYTE *destination,
                                      int unused, unsigned int pitch) {
  MediaDescriptor &descriptor = *runtime->descriptor;
  runtime->decoder_context_3f8.predictor_500 = 0;
  runtime->decoder_context_3f8.predictor_4fc = 0;
  runtime->decoder_context_3f8.predictor_4f8 = 0;
  runtime->decoder_context_3f8.compressed_input_200 = compressed;
  runtime->decoder_context_3f8.bit_position_204 = 0;
  (void)unused;
  DWORD block_y = 0;
  DWORD row_step = (pitch >> 2) << 6;
  BYTE *row_destination = destination;
  while (block_y < descriptor.output_height_60) {
    DWORD block_x = 0;
    BYTE *block_destination = row_destination;
    while (block_x < descriptor.output_width_64) {
      if (runtime->record_option_040 != 0) {
        sub_5d1654(block_destination, (int)pitch,
                   &runtime->decoder_context_3f8);
      } else {
        sub_5d1a30(block_destination, (int)pitch,
                   &runtime->decoder_context_3f8);
      }
      block_x += 0x10;
      block_destination += 0x40;
    }
    row_destination += row_step;
    block_y += 0x10;
  }
}

// ORIGINAL: 0x005CDA30 sub_5cda30 0x005CDA30-0x005CDAC4
// TRIED: nothing forced the `push ecx`-as-spill-slot / `pop ecx` prologue trick the original uses for one local; plain locals compile 6 bytes shorter (142 vs 148), diverging early
// symbol    @sub_5cda30@20
// kind      game
extern "C" void __fastcall sub_5cda30(MediaRuntime *runtime,
                                      BYTE *compressed, BYTE *destination,
                                      int unused, unsigned int pitch) {
  MediaDescriptor &descriptor = *runtime->descriptor;
  runtime->decoder_context_3f8.compressed_input_200 = compressed;
  runtime->decoder_context_3f8.bit_position_204 = 0;
  runtime->decoder_context_3f8.predictor_500 = 0;
  runtime->decoder_context_3f8.predictor_4fc = 0;
  runtime->decoder_context_3f8.predictor_4f8 = 0;
  (void)unused;
  DWORD block_y = 0;
  DWORD row_step = (pitch >> 1) << 5;
  BYTE *row_destination = destination;
  while (block_y < descriptor.output_height_60) {
    DWORD block_x = 0;
    BYTE *block_destination = row_destination;
    while (block_x < descriptor.output_width_64) {
      sub_5d1278(block_destination, (int)pitch,
                 &runtime->decoder_context_3f8);
      block_x += 0x10;
      block_destination += 0x20;
    }
    row_destination += row_step;
    block_y += 0x10;
  }
}

// ORIGINAL: 0x005CFAED sub_5cfaed 0x005CFAED-0x005D02CC
// TRIED: signature changed to (unsigned char *, int, int) - the disasm reads 3 stack args ([esp+0x38] at entry, [esp+0x20]/[esp+0x24] after the scratch frame is released), not the 0-arg contract. The body is a mechanically-generated 1:1 transcription (script-derived from the raw disassembly, not hand-typed, to avoid transcription error over ~150 near-identical calls): 6x (sub_5cf150(); 8x sub_5d29e1; 8x sub_5d2b20) then 16x OutputPixelRow with an `edi` accumulator stepped by param3. sub_5d29e1/sub_5d2b20/OutputPixelRow all pass their SECOND (and for OutputPixelRow, first/third) operand through ESI/EDI/EBX with NO stack push at those call sites - a register convention this rewrite cannot reproduce without inline asm (banned), so it passes the same values as ordinary arguments instead. Low similarity (sim ~0.1-0.3 across all four builds) is a direct, expected consequence of that convention mismatch, not a structural error - every call target, argument value and order is transcribed 1:1 from the disassembly. Landed for coverage; TRIED as byte-exact without __asm.
// symbol    ?sub_5cfaed@@YAXPAE0H@Z
// kind      game
void __cdecl sub_5cfaed(BYTE *compressed, BYTE *destination,
                        int row_pitch_bytes) {
  MediaTransformBitState &reader =
      movie_media_conversion_tables.transform_bit_state;
  reader.cursor = compressed + 1;
  reader.accumulator = *compressed;
  reader.available_bits = 8;

  for (int phase = 0; phase != 6; ++phase)
    movie_media_transform_block_pair(phase);
  movie_media_write_macroblock_rows(destination, row_pitch_bytes, 0);
}

// ORIGINAL: 0x005D02CC sub_5d02cc 0x005D02CC-0x005D0AAB
// TRIED: the callees use a custom register convention (row/block ptr in esi, the single stack arg written once to a fixed [esp] slot instead of pushed per call; OutputPixelRow16 takes esi/ebx/edi with no stack args at all) that no standard C++ calling convention expresses without __asm. Landed all 96 sub_5d29e1/sub_5d2b20 calls plus the 16 OutputPixelRow16 calls with the right addresses/order as plain cdecl(void*,void*[,void*]) - compiles, right call graph, but sim only ~0.12 since every call site's instruction shape differs. Signature changed to (unsigned char*, void*, int) from the disasm's 3-arg use (a1=byte* string, a2=dest, a3=row stride). Not retried given the convention mismatch is structural, not a wording issue.
// symbol    ?sub_5d02cc@@YAXPAEPAXH@Z
// kind      game
void __cdecl sub_5d02cc(BYTE *compressed, void *destination,
                        int row_pitch_bytes) {
  MediaTransformBitState &reader =
      movie_media_conversion_tables.transform_bit_state;
  reader.cursor = compressed + 1;
  reader.accumulator = *compressed;
  reader.available_bits = 8;

  for (int phase = 0; phase != 6; ++phase)
    movie_media_transform_block_pair(phase);
  movie_media_write_macroblock_rows(
      reinterpret_cast<unsigned char *>(destination), row_pitch_bytes, 1);
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
  const DWORD current = static_cast<DWORD>(reinterpret_cast<uintptr_t>(input_ring_current));
  const DWORD base = static_cast<DWORD>(reinterpret_cast<uintptr_t>(input_ring_base));
  if (static_cast<DWORD>(current + static_cast<DWORD>(size)) >=
      static_cast<DWORD>(base + 1200000U)) input_ring_current = input_ring_base;

  const LONG read = mmioRead(descriptor->stream,
                             reinterpret_cast<HPSTR>(input_ring_current), size);
  if (read == 0) { stream_eof = 1; return -1; }
  if (read == -1) return -2;

  const int slot = read_producer_cursor % 0x18;
  read_records[slot].payload = input_ring_current;
  read_records[slot].field_0c = static_cast<DWORD>(kind);
  read_records[slot].field_08 = static_cast<DWORD>(read_counter);
  input_ring_current = reinterpret_cast<BYTE *>(
      static_cast<uintptr_t>(static_cast<DWORD>(
          reinterpret_cast<uintptr_t>(input_ring_current) + static_cast<DWORD>(size))));
  InterlockedIncrement(&read_producer_cursor);
  InterlockedIncrement(&pending_read_records);
  InterlockedIncrement(&read_counter);
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
// ORIGINAL: 0x005CD3B0 sub_5cd3b0 0x005CD3B0-0x005CD556
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
    MediaConversionTableStorage &conversion_tables =
        movie_media_conversion_tables;
    const int scale = conversion_tables.block_scale_entries[0];
    __int64 product;
    int chroma_a;
    int chroma_b;
    int chroma;
    unsigned int row_step;
    int luma_1;
    int luma_2;
    int luma_3;
    int luma_4;

    // The image's scale idiom rounds half up before the sar 1: add bit 15
    // of the product to the shifted-down word.
    product = static_cast<__int64>(scale) *
              static_cast<__int64>(static_cast<int>(param_6));
    chroma_a = ((static_cast<int>(product >> 16) +
                 static_cast<int>((product >> 15) & 1)) >>
                1);
    product = static_cast<__int64>(scale) *
              static_cast<__int64>(static_cast<int>(param_5));
    chroma_b = ((static_cast<int>(product >> 16) +
                 static_cast<int>((product >> 15) & 1)) >>
                1);
    chroma = conversion_tables.packed_component_words[128 + (chroma_a & 0x7f)] +
             conversion_tables.packed_component_words[256 + (chroma_b & 0x7f)];

    row_step = stride_arg & 0xfffffffcu;

    product = static_cast<__int64>(scale) *
              static_cast<__int64>(static_cast<int>(param_1));
    luma_1 = conversion_tables.packed_component_words[((static_cast<int>(product >> 16) +
                                                   static_cast<int>((product >> 15) & 1)) >>
                                                  1) &
                                                 0x7f];
    {
        int *row = plane;
        int outer = 8;
        do {
            int *out = row;
            for (int inner = 8; inner != 0; --inner) {
                *out = luma_1 + chroma;
                ++out;
            }
            row = reinterpret_cast<int *>(reinterpret_cast<char *>(row) +
                                          row_step);
            --outer;
        } while (outer != 0);
    }

    product = static_cast<__int64>(scale) *
              static_cast<__int64>(static_cast<int>(param_2));
    luma_2 = conversion_tables.packed_component_words[((static_cast<int>(product >> 16) +
                                                   static_cast<int>((product >> 15) & 1)) >>
                                                  1) &
                                                 0x7f];
    {
        int *row = plane + 8;
        int outer = 8;
        do {
            int *out = row;
            for (int inner = 8; inner != 0; --inner) {
                *out = luma_2 + chroma;
                ++out;
            }
            row = reinterpret_cast<int *>(reinterpret_cast<char *>(row) +
                                          row_step);
            --outer;
        } while (outer != 0);
    }

    product = static_cast<__int64>(scale) *
              static_cast<__int64>(static_cast<int>(param_3));
    luma_3 = conversion_tables.packed_component_words[((static_cast<int>(product >> 16) +
                                                   static_cast<int>((product >> 15) & 1)) >>
                                                  1) &
                                                 0x7f];
    {
        int *row = plane + stride_arg * 2;
        int outer = 8;
        do {
            int *out = row;
            for (int inner = 8; inner != 0; --inner) {
                *out = luma_3 + chroma;
                ++out;
            }
            row = reinterpret_cast<int *>(reinterpret_cast<char *>(row) +
                                          row_step);
            --outer;
        } while (outer != 0);
    }

    product = static_cast<__int64>(scale) *
              static_cast<__int64>(static_cast<int>(param_4));
    luma_4 = conversion_tables.packed_component_words[((static_cast<int>(product >> 16) +
                                                   static_cast<int>((product >> 15) & 1)) >>
                                                  1) &
                                                 0x7f];
    {
        int *row = plane + stride_arg * 2 + 8;
        int outer = 8;
        do {
            int *out = row;
            for (int inner = 8; inner != 0; --inner) {
                *out = luma_4 + chroma;
                ++out;
            }
            row = reinterpret_cast<int *>(reinterpret_cast<char *>(row) +
                                          row_step);
            --outer;
        } while (outer != 0);
    }
}
/*
Purpose: Create the codec's draw/audio surface through COM: QueryInterface
         slot 6 on the provider with a 0x6c-byte format block (0x1007 tag,
         0x4040 tail, 0x20/4 pair and the caller's fifth argument at its
         end), then QueryInterface slot 0 of the created object for the
         interface at 0x0066FC28 - success stores the interface through the
         out pointer and returns 0, any failure Releases (slot 2) the created
         object and returns 1.

// Typed owner implementation; native ABI/output, original reference targets
// and exceptional behavior remain restoration obligations, not owner stubs.
// ORIGINAL: 0x005CDAD0 sub_5cdad0 0x005CDAD0-0x005CDB83
// symbol    ?sub_5cdad0@@YIHPAUIDirectDraw2@@PAPAUIDirectDrawSurface2@@HHH@Z
// size      179 bytes
// prototype int (__fastcall sub_5cdad0)(int *, int *, int, int, int)
// callers   1   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x005CDB3E 0x005CDB56 0x005CDB73
Status: Coverage transcription - unclaimed body (prologue register-save order
        diverges at instruction 3; seed TRIED).
*/
int __fastcall sub_5cdad0(IDirectDraw2 *provider,
                          IDirectDrawSurface2 **out_slot,
                          int width, int height, int format_tail) {
  DDSURFACEDESC description;
  memset(&description, 0, 0x6c);
  description.dwSize = 0x6c;
  description.dwFlags = 0x1007;
  description.dwHeight = static_cast<DWORD>(height);
  description.dwWidth = static_cast<DWORD>(width);
  description.ddpfPixelFormat.dwSize = 0x20;
  description.ddpfPixelFormat.dwFlags = DDPF_FOURCC;
  description.ddpfPixelFormat.dwFourCC = static_cast<DWORD>(format_tail);
  description.ddsCaps.dwCaps = 0x4040;

  IDirectDrawSurface *created = 0;
  if (provider->CreateSurface(&description, &created, 0) == DD_OK) {
    IDirectDrawSurface2 *queried = 0;
    if (created->QueryInterface(IID_IDirectDrawSurface2,
                                reinterpret_cast<void **>(&queried)) == DD_OK) {
      *out_slot = queried;
      return 0; // Preserve the original success-path reference/leak behavior.
    }
    created->Release();
  }
  return 1;
}
