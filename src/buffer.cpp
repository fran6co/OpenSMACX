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
#include "stdafx.h"
#include "original_seam.h"
#include "buffer.h"
#include "graphicwin.h"
#include "filemap.h"
#include "font.h"
#include "general.h"   // mem_get, for the link labels this owns
#include "palette.h"
#include "spot.h"
#include "temp.h"      // BufferStrHeight, which the wrap family scales by
#include "alpha.h"     // Language, the write/wrap text-selection switch

#include <new>

// Patterned line rendering is still unfinished. This owning-TU body keeps
// the non-cover route-preview branch linkable without an executable-address
// call. The existing five-argument solid-line renderer remains unchanged.
// ORIGINAL: 0x005E09D0 ?line@Buffer@@QAEXHHHHHHHHH@Z 0x005E09D0-0x005E10BA
// RECOVERY_BINDING: v1 {"evidence": ["Original4672C0 calls5E09D0; src/unrecovered/005e09d0.cpp retains its original span and seed.", "The original void catalogue and ignored-result call sites support the declaration; BODY GOES HERE does not implement pattern rendering."], "reason": "The non-cover direct route renderer references this nine-argument patterned Buffer line, distinct from the recovered five-argument solid overload. Its owning-TU placeholder is explicitly unfinished and only preserves linking."}
void Buffer::line(int x1, int y1, int x2, int y2, int first_color,
                  int second_color, int pattern_on, int pattern_off, int phase) {
    // BODY GOES HERE.
}

// THE ORIGINAL'S MIN, which evaluates both arguments twice. `write_cent_l`
// calls `strlen` four times because it uses this twice, and that is not a
// transcription artefact - it is what a macro does and what the image has.
#define BUFFER_MIN(a, b) ((a) < (b) ? (a) : (b))


// The `owned_` table's length, needed by the constructor as well as by
// `init` and `close`, so it is declared ahead of all three.
static const size_t OwnedAllocationCount = 20;

/*
Purpose: Construct an empty Buffer, including its Spot subobject, text state,
         and either the process palette or a placeholder colour table.

         WRITTEN THROUGH ITS MEMBERS, and that is the whole of the change
         that matched it. Every one of the fifty-three stores here used to
         be `object[0x520 / 4] = ...` through a `volatile uint32_t *`,
         because the offsets had no names; naming the DIB block at 0x7C gave
         the last twelve of them names, and the body became ordinary C++.

         THE ONLY THING THAT HAD TO BE COPIED FROM THE IMAGE is the order of
         two pairs - 0xC before 0x8, 0x14 before 0x10. VC6 emits these stores
         in source order, so the original wrote them that way; every other
         line here is in the order a person would write it, and the compiler
         schedules them into the image's.
// ORIGINAL: 0x005D7210 ??0Buffer@@QAE@XZ 0x005D7210-0x005D740A;0x00662B50-0x00662B68 BYTE_EXACT
// size      530 bytes
// prototype void (__thiscall ??0Buffer@@QAE@XZ)(Buffer* this)
// callers   80   call targets   2
// kind      game
// flags     sp_ready;purged_ok
// calls     0x005FA860 0x005FE560
// notes     Runtime redirect installed by DllMain after byte-signature validation
Status: Complete
Verification note: three of the old mutation-harness survivors were artefacts
of the raw-offset spelling and cannot be expressed now. Widening the `owned_`
loop wrote an extra zero at 0x50C, which the -1 below overwrote; the loop is
bounded by the array now. Widening the fallback ramp wrote a 257th entry at
0x4A4, where the index truncated to zero; it is bounded by 256 now. The three
adjacent stores inside that ramp still target distinct bytes with independent
values, so their order is genuinely not observable.
*/
// Plain Buffer's null query shares the folded leaf mapped in nullsub_thunks.cpp
// at 0x00406B30. GraphicWin overrides this slot with its typed self-query;
// the compiler supplies the Buffer-secondary-base adjustor at 0x00404420.
GraphicWin *Buffer::get_graphic_win() { return nullptr; }

/*
Purpose: Return the address of the pixel at (x, y) in the buffer, locking the
         surface first when needed, and refusing coordinates outside the DIB.
// ORIGINAL: 0x005E33F3 ?get_data@Buffer@@QAEHHH@Z 0x005E33F3-0x005E349D FILE BYTE_EXACT
// size      170 bytes
// prototype int (__thiscall ?get_data@Buffer@@QAEHHH@Z)(Buffer* this, int, int)
// callers   9   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// indirect  0x005E3456
// calls     (none)
Status: Promoted 2026-08-29 from src/unrecovered/005e33f3.cpp, which proved the
claim against the opaque scaffold. The body is the two-coordinate sibling of
the in-class `get_data()` in buffer.h and reads the same members the same way:
`dib_.bmiHeader.biWidth` is the x bound, the NEGATIVE top-down `biHeight` is
the negated y bound, and the lock path stores `stride_` before
`locked_bits_`. `Sprite::extract` calls it by name five times.
*/
int Buffer::get_data(int x, int y) {
    if (x < dib_.bmiHeader.biWidth && y < -dib_.bmiHeader.biHeight) {
        int value;
        if (surface_ == nullptr) {
            locked_bits_ = dib_bits_;
            if (dib_bits_ == nullptr) {
                return 0;
            }
            ++surface_lock_count_;
            value = reinterpret_cast<int>(dib_bits_);
        } else {
            if (locked_bits_ != nullptr) {
                ++surface_lock_count_;
                value = reinterpret_cast<int>(locked_bits_);
            } else {
                DDSURFACEDESC description;
                description.dwSize = sizeof(description);
                if (surface_->Lock(nullptr, &description, DDLOCK_WAIT, nullptr) != 0) {
                    return 0;
                }
                stride_ = description.lPitch;
                ++surface_lock_count_;
                locked_bits_ = description.lpSurface;
                value = reinterpret_cast<int>(description.lpSurface);
            }
        }
        if (value != 0) {
            return stride_ * y + reinterpret_cast<int>(locked_bits_) + x;
        }
    }
    return 0;
}

Buffer::Buffer() {
    // `spot_` is constructed ahead of this body by declaration order, and
    // the vtable store the image makes here is the compiler's, not source.
    for (size_t slot = 0; slot < OwnedAllocationCount; ++slot) {
        owned_[slot] = nullptr;
    }
    palette_ = nullptr;
    field_C_ = 0;
    field_8_ = 0;
    field_14_ = 0;
    field_10_ = 0;
    field_18_ = 0;
    init_flags_ = 0;
    locked_bits_ = nullptr;
    dib_bits_ = nullptr;
    surface_ = nullptr;
    clipper_ = nullptr;
    hdc2_ = nullptr;
    hdc_ = nullptr;
    hdc_lock_count_ = 0;
    surface_lock_count_ = 0;
    previous_bitmap_ = nullptr;
    bitmap_handle_ = nullptr;
    palette_seed_ = 0;
    stride_ = 0;
    field_50C_ = -1;
    write_font_slot_ = 0;
    field_514_ = 0;
    font_slot_ = 0;
    markup_pending_ = 0;
    markup_enabled_ = BufferField520Default;
    field_524_ = 0;

    // THE FOUR TEXT STYLES, one `Font *` and four colours each. `set_text_color`
    // through `set_text_color4` write one style apiece, which is what pairs
    // `font2_` with the `color_2_val_*` row below it. Only the first gets the
    // process default font; the other three start unset, and -1 is the "no
    // colour" the drawing code tests for.
    font1_ = FontDefault;
    color_val_1_ = 0;
    color_val_2_ = -1;
    color_val_3_ = 2;
    color_val_4_ = 2;
    font2_ = nullptr;
    color_2_val_1_ = -1;
    color_2_val_2_ = -1;
    color_2_val_3_ = 2;
    color_2_val_4_ = 2;
    font3_ = nullptr;
    color_3_val_1_ = -1;
    color_3_val_2_ = -1;
    color_3_val_3_ = 2;
    color_3_val_4_ = 2;
    font4_ = nullptr;
    color_hyper_val_1_ = -1;
    color_hyper_val_2_ = -1;
    color_hyper_val_3_ = 2;
    color_hyper_val_4_ = 2;
    has_palette_ = 0;
    field_580_ = 0;
    clip_region_ = nullptr;
    poOwner_ = 0;

    // An 8-bit top-down DIB with a full 256-colour table and nothing drawn in
    // it yet; `Buffer::init` fills in the dimensions.
    dib_.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    dib_.bmiHeader.biWidth = 0;
    dib_.bmiHeader.biHeight = 0;
    dib_.bmiHeader.biPlanes = 1;
    dib_.bmiHeader.biBitCount = 8;
    dib_.bmiHeader.biCompression = BI_RGB;
    dib_.bmiHeader.biSizeImage = 0;
    dib_.bmiHeader.biXPelsPerMeter = 0;
    dib_.bmiHeader.biYPelsPerMeter = 0;
    dib_.bmiHeader.biClrUsed = 256;
    dib_.bmiHeader.biClrImportant = 0;

    if (PaletteCurrent) {
        PaletteCurrent->get_rgbquad(dib_.bmiColors, 0, 256);
    } else {
        // No palette yet: a blue ramp, so the table is at least well-formed.
        for (int entry = 0; entry < 256; ++entry) {
            dib_.bmiColors[entry].rgbRed = 0;
            dib_.bmiColors[entry].rgbGreen = 0;
            dib_.bmiColors[entry].rgbBlue = static_cast<BYTE>(entry);
            dib_.bmiColors[entry].rgbReserved = 0;
        }
    }
}

Buffer *__fastcall buffer_construct_redirect(Buffer *self, void *) {
    new (self) Buffer();
    return self;
}

/*
Purpose: Set the four fonts used by the buffer.
// ORIGINAL: 0x005DAC70 ?set_font@Buffer@@QAEHPAVFont@@PAVFont@@PAVFont@@PAVFont@@@Z 0x005DAC70-0x005DACB0 BYTE_EXACT
// symbol    ?set_font@Buffer@@QAEHPAVFont@@000@Z
// size      64 bytes
// prototype int (__thiscall ?set_font@Buffer@@QAEHPAVFont@@PAVFont@@PAVFont@@PAVFont@@@Z)(Buffer* this, Font*, Font*, Font*, Font*)
// callers   189   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// notes     Runtime redirect installed by DllMain after byte-signature validation
Return Value: No errors (0); invalid primary font (3)
Status: Complete
*/
int Buffer::set_font(Font *font1, Font *font2, Font *font3, Font *font4) {
    if (!font1) {
        return 3;
    }
    if (font1->is_initialized()) {
        font1_ = font1;
    }
    font2_ = font2;
    font3_ = font3;
    font4_ = font4;
    return 0;
}

/*
Purpose: Set the primary text colors for the four color slots.
// ORIGINAL: 0x005DACB0 ?set_text_color@Buffer@@QAEXHHHH@Z 0x005DACB0-0x005DACDB BYTE_EXACT
// size      43 bytes
// prototype void (__thiscall ?set_text_color@Buffer@@QAEXHHHH@Z)(Buffer* this, int, int, int, int)
// callers   188   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// notes     Runtime redirect installed by DllMain after byte-signature validation
Return Value: n/a
Status: Complete
*/
void Buffer::set_text_color(int color1, int color2, int color3, int color4) {
    color_val_1_ = color1;
    color_val_2_ = color2;
    color_val_3_ = color3;
    color_val_4_ = color4;
}

/*
Purpose: Set the secondary text colors for the four color slots.
// ORIGINAL: 0x005DACE0 ?set_text_color2@Buffer@@QAEXHHHH@Z 0x005DACE0-0x005DAD0B BYTE_EXACT
// size      43 bytes
// prototype void (__thiscall ?set_text_color2@Buffer@@QAEXHHHH@Z)(Buffer* this, int, int, int, int)
// callers   46   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// notes     Runtime redirect installed by DllMain after byte-signature validation
Return Value: n/a
Status: Complete
*/
void Buffer::set_text_color2(int color1, int color2, int color3, int color4) {
    color_2_val_1_ = color1;
    color_2_val_2_ = color2;
    color_2_val_3_ = color3;
    color_2_val_4_ = color4;
}

/*
Purpose: Set the tertiary text colors for the four color slots.
// ORIGINAL: 0x005DAD10 ?set_text_color3@Buffer@@QAEXHHHH@Z 0x005DAD10-0x005DAD3B BYTE_EXACT
// size      43 bytes
// prototype void (__thiscall ?set_text_color3@Buffer@@QAEXHHHH@Z)(Buffer* this, int, int, int, int)
// callers   30   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// notes     Runtime redirect installed by DllMain after byte-signature validation
Return Value: n/a
Status: Complete
*/
void Buffer::set_text_color3(int color1, int color2, int color3, int color4) {
    color_3_val_1_ = color1;
    color_3_val_2_ = color2;
    color_3_val_3_ = color3;
    color_3_val_4_ = color4;
}

/*
Purpose: Set the hyperlink text colors for the four color slots.
// ORIGINAL: 0x005DAD40 ?set_text_color_hyper@Buffer@@QAEXHHHH@Z 0x005DAD40-0x005DAD6B BYTE_EXACT
// size      43 bytes
// prototype void (__thiscall ?set_text_color_hyper@Buffer@@QAEXHHHH@Z)(Buffer* this, int, int, int, int)
// callers   20   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// notes     Runtime redirect installed by DllMain after byte-signature validation
Return Value: n/a
Status: Complete
*/
void Buffer::set_text_color_hyper(int color1, int color2, int color3, int color4) {
    color_hyper_val_1_ = color1;
    color_hyper_val_2_ = color2;
    color_hyper_val_3_ = color3;
    color_hyper_val_4_ = color4;
}

/*
Purpose: Initialize shared Buffer state; the legacy implementation has no state to initialize.
// ORIGINAL: 0x005DF570 ?init_class@Buffer@@QAAHXZ 0x005DF570-0x005DF573 BYTE_EXACT
// symbol    ?init_class@Buffer@@SAHXZ
// size      3 bytes
// prototype 
// callers   1   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// notes     Runtime redirect installed by DllMain after byte-signature validation
Return Value: No errors (0)
Status: Complete
*/
int Buffer::init_class() {
    return 0;
}

/*
Purpose: Close shared Buffer state; the legacy implementation is intentionally empty.
// ORIGINAL: 0x005DF580 ?close_class@Buffer@@QAAXXZ 0x005DF580-0x005DF581 BYTE_EXACT
// symbol    ?close_class@Buffer@@QAEXXZ
// size      1 bytes
// prototype 
// callers   1   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// notes     Runtime redirect installed by DllMain after byte-signature validation
Return Value: n/a
Status: Complete
*/
void Buffer::close_class() {
}


int __cdecl buffer_init_class_redirect() {
    return 0;
}

void __cdecl buffer_close_class_redirect() {
}

namespace {

// An RGNDATA holding exactly one rectangle: 48 bytes, which is what the
// original reserves - `Buffer::set_clip` builds the block at `esp + 0x24`
// and its last store is `esp + 0x50`, so 0x30 bytes, `RGNDATAHEADER`'s 32
// plus a `RECT`'s 16.
//
}  // namespace

namespace {

typedef long(__stdcall *func_surface_lock)(void *, void *, void *, uint32_t, void *);
typedef long(__stdcall *func_surface_unlock)(void *, void *);

// DDSURFACEDESC fields the lock path consumes.
static const size_t SurfaceDescriptorSize = 0x6C;
static const size_t SurfaceDescriptorPitch = 0x10;
static const size_t SurfaceDescriptorData = 0x24;

}  // namespace

/*
Purpose: Acquire the buffer's pixel data, locking the DirectDraw surface on the
         first reference and counting every acquisition.
// ORIGINAL: 0x005E3373 ?get_data@Buffer@@QAEHXZ 0x005E3373-0x005E33EA BYTE_EXACT
// size      119 bytes
// prototype int (__thiscall ?get_data@Buffer@@QAEHXZ)(Buffer* this)
// callers   48   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x005E33BF
// notes     Runtime redirect installed by DllMain after byte-signature validation
Status: Complete
*/
// body src/buffer.h

/*
Purpose: Release acquired references to the buffer's pixel data, unlocking the
         DirectDraw surface once the last reference is dropped.
// ORIGINAL: 0x005E34A3 ?free_data@Buffer@@QAEXH@Z 0x005E34A3-0x005E34FB BYTE_EXACT
// size      88 bytes
// prototype void (__thiscall ?free_data@Buffer@@QAEXH@Z)(Buffer* this, int)
// callers   43   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x005E34EA
// notes     Runtime redirect installed by DllMain after byte-signature validation
Status: Complete
*/
// MOVED OUT OF LINE 2026-09-06 from the in-class definition in buffer.h. The
// image CALLS free_data here and at every other site (43 of them in the
// catalogue), and an in-class definition let VC6 fold the whole body into
// each caller - render_multi_table_dest in caviar.cpp was +20 instructions
// from it alone, 76 compiled against the image's 53, and came back to 38 of
// 53 agreeing on the move. Measured across the whole tree before and after:
// 0 claims regressed in buffer.cpp, sprite.cpp, texture.cpp or caviar.cpp.
// The in-class spelling's own lesson moves with the body:
// THE SURFACE TEST COMES FIRST and the subtraction is written out in
// both arms - `cmp ecx, edi; jne` at 0x005E34AE before any arithmetic.
// Hoisted above the test it lands two instructions early and every jump
// after it moves, which is the same shape `Buffer::close` needed.
void Buffer::free_data(int count) {
    if (surface_ == nullptr) {
        surface_lock_count_ -= count;
        if (surface_lock_count_ <= 0) {
            locked_bits_ = nullptr;
            surface_lock_count_ = 0;
        }
        return;
    }
    surface_lock_count_ -= count;
    if (locked_bits_ != nullptr && surface_lock_count_ <= 0) {
        surface_->Unlock(locked_bits_);
        locked_bits_ = nullptr;
        surface_lock_count_ = 0;
    }
}


/*
Purpose: Report the line height of the buffer's primary font, falling back to
         the global default font when none is set.
// ORIGINAL: 0x005DCAB0 ?text_line_height@Buffer@@QAEHXZ 0x005DCAB0-0x005DCADB BYTE_EXACT
// LEVER: branch polarity - image falls through to the override-add case and
//        jumps AWAY to the no-override case at the end; `if (>= 0) { add }
//        return { no override }` matches, `if (< 0) return` inverted it.
// size      43 bytes
// prototype int (__thiscall ?text_line_height@Buffer@@QAEHXZ)(Buffer* this)
// callers   21   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// notes     Runtime redirect installed by DllMain after byte-signature validation
Status: Complete
*/
int Buffer::text_line_height() {
    if (!font1_) {
        font1_ = FontDefault;
    }
    const Font *const font = font1_;
    const int *const fields = reinterpret_cast<const int *>(font);
    const int height_override = fields[0x00 / 4];
    if (height_override >= 0) {
        return static_cast<int>(
            static_cast<uint32_t>(fields[0x10 / 4])
            + static_cast<uint32_t>(height_override));
    }
    // No override: the font's own line height already includes leading.
    return fields[0x0C / 4];
}


// QueryInterface output at 62CDBD, interface load/call at 62CDEF and
// release/clear at 62D100..62D13A establish this four-byte pointer slot.
// RECOVERY_OBJECT: v2 0x009BC494 0x009BC498 ?BufferDirectDraw@@3PAUIDirectDraw@@A 0x00454FC0
IDirectDraw *BufferDirectDraw;  // 0x009BC494
// 0x00696BF0, holding 1. `Buffer::Buffer` and `Buffer::close` are its
// only two references in the whole image and both READ it, so it is a
// default that the two restore into `markup_enabled_` rather than state.
//
// NOT `const`. It lives in `.data` below the zero-fill line, so the value
// is known - but the original loads it, `mov eax, [0x696bf0]`, and a `const`
// the compiler can see folds to `mov eax, 1`. A mutable global keeps the
// load, and the relocation is masked.
uint32_t BufferField520Default = 1;

// Shared fill scratch: stride minus painted width. Original 5DFE63 writes
// this DWORD and 5DFE98 reads it after each completed row; direct-address
// scanning finds no other users. Its initial image storage is zero-filled.
// Keep the observed shared storage, not a fixed-address alias or an invented
// per-Buffer member. Direct scans do not rule out computed indirect access.
// RECOVERY_OBJECT: v2 0x009B3A10 0x009B3A14 ?BufferFillRowGap@@3IA 0x005DFCD0
uint32_t BufferFillRowGap;

namespace {

typedef void (OriginalObject::*func_buffer_virtual)();

}  // namespace


/*
Purpose: Copy a region of another buffer into the same position in this one.
// ORIGINAL: 0x005D95B0 ?copy@Buffer@@QAEHPAVBuffer@@HHHH@Z 0x005D95B0-0x005D95D3 BYTE_EXACT
// symbol    ?copy@Buffer@@QAEHPAV1@HHHH@Z
// size      35 bytes
// prototype int (__thiscall ?copy@Buffer@@QAEHPAVBuffer@@HHHH@Z)(Buffer* this, Buffer*, int, int, int, int)
// callers   8   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005DFF00
Return Value: whatever the full copy returns
Status: Complete with temporary full-copy dependency

The original is nothing but an argument shuffle onto the seven-argument copy:
it pushes arg5, arg4, arg3, arg2, arg3, arg2, arg1, so the destination
coordinates are handed over a second time as the source coordinates. That
repetition is the whole content of this overload.
*/
namespace {

// 0x00696BF8. Every PCX the game ships is prefixed with this and a NUL;
// `load_pcx` steps over both when it finds them.
const char PcxAssetPrefix[] = "monkey sweats on a tuesday";

// A PCX header is 128 bytes, and the fields this decoder reads sit at fixed
// offsets in it: magic, version, encoding, bits-per-pixel, the bounding box,
// then planes and stride near the end.
const size_t PcxHeaderSize = 0x80;
const size_t PcxMagic = 0, PcxVersion = 1, PcxEncoding = 2, PcxBitsPerPixel = 3;
const size_t PcxXMin = 4, PcxYMin = 6, PcxXMax = 8, PcxYMax = 10;
const size_t PcxPlanes = 65, PcxBytesPerLine = 66;

}  // namespace

/*
Purpose: Decode a PCX image out of memory into this buffer, then install the
         256 colours that follow it and publish them through the palette.
// ORIGINAL: 0x005E2690 ?load_pcx@Buffer@@QAEHPAEKPAVPalette@@HH@Z 0x005E2690-0x005E2AF7
// TRIED: byte-exactness in this pass. Best measured is 0.22 similar
// (6-25/374 instructions across the flag sets), and the divergence is not
// one small edit: the frame is 0x100 against the image's 0xfc, `this` and
// `data` swap registers throughout (esi/edi/ebx allocated differently from
// the first instruction on).
// LEVER (measured, did not move the score): duplicated `sync_to_palette`'s
// body inline here rather than calling it, since `osmx calls` shows the
// image never reaches 0x005DE8F0 from this address - `Palette::get_rgbquad`
// and `SetDIBColorTable` are open-coded at 0x005E2A1C and 0x005E2A7C. This
// is the correct call graph (`sync_to_palette` itself stays BYTE_EXACT for
// its 24 other callers), but similarity does not move on it - best measured
// stays ~0.21-0.22 across flag sets - because the register-allocation
// divergence starts at instruction 0, before any of the inlined code is
// reached. Kept for call-graph fidelity; do not re-chase similarity from
// this alone.
// LEVER (measured 2026-08-29, moved the counts): `get_hdc` and
// `release_hdc` are ALSO open-coded by the image here - vtable GetDC at
// 0x005E2A56 and ReleaseDC at 0x005E2AB3 with both counts and the hdc2_
// publishes between - and writing both protocols out (Win::init_class
// idiom) took the compiled body 253 -> 311 instructions and agreement
// 5/374 -> 22/374, and `call_diff` stopped reporting FEWER. The remaining
// delta is frame layout (0x94 against 0xfc), not missing code.
// symbol    ?load_pcx@Buffer@@QAEHPAEKPAVPalette@@HH@Z
// size      1127 bytes
// prototype int (__thiscall ?load_pcx@Buffer@@QAEHPAEKPAVPalette@@HH@Z)(Buffer* this, unsigned int8*, unsigned int, Palette*, int, int)
// callers   2   call targets   7
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005D7670 0x005FE560 0x005FE650 0x006453E0 0x006458F0 0x00645930 0x006465F0
Return Value: 0 on success, 3 for a bad argument or an unusable header,
              7 for no pixel storage, or whatever `init` returned
Status: WIP

PROMOTED FROM src/unrecovered/005e2690.cpp, which is deleted, and it is
mostly SHORTER than that unit rather than merely renamed. Three blocks it
spelled out by hand are recovered functions here:

  the `if (surface_ == 0) ... else Lock(...)` block is `get_data`
  the `--surface_lock_count_` pair that closes it is `free_data(1)`
  the whole palette tail - the seed compare, `get_rgbquad`,
  `SetDIBColorTable`, and the `has_palette_` / `palette_` stores that end
  it - is `sync_to_palette`

The unit could not say so because none of those fields had names when it was
written: it reads `field_58_`, `field_50_`, `field_6C_`, `field_4A8_`,
`field_68_`, `field_4A4_`, and calls Lock, Unlock, GetDC and ReleaseDC
through raw vtable indices 25, 32, 17 and 26.

THE LAST TWO ARGUMENTS ARE NOT WHAT THEY ARE CALLED. The by-name overload
passes its `tgl` and `height` straight through, but this body adds the first
to every decoded pixel and uses the second as the number of palette entries
to copy - so they are a colour base and a colour count, and they are named
for that here.
*/
int Buffer::load_pcx(BYTE *data, DWORD size, Palette *palette,
                     int colour_base, int colour_count) {
    if (data == nullptr && surface_ == nullptr) {
        return 3;
    }

    // The image is 374 instructions; after open-coding `sync_to_palette`
    // AND `get_hdc`/`release_hdc` (all three are called in the call form and
    // all three are inline in the image) this tree compiles 311. The rest of
    // the gap is frame layout: the image reserves 0xfc where ours is 0x94.
    // The shipped assets carry a watermark ahead of the PCX itself.
    //
    // `strlen` THREE TIMES, not hoisted. The image calls it at 0x005E26B7,
    // 0x005E26D4 and 0x005E26E9 - once per use - and a `const size_t prefix`
    // collapses all three into one, which is two calls the image makes and we
    // did not.
    if (strncmp(reinterpret_cast<const char *>(data), PcxAssetPrefix,
                strlen(PcxAssetPrefix)) == 0) {
        data += strlen(PcxAssetPrefix) + 1;
        size -= static_cast<DWORD>(strlen(PcxAssetPrefix) + 1);
    }

    BYTE header[PcxHeaderSize];
    memcpy(header, data, sizeof(header));
    if (header[PcxMagic] != 0x0A || header[PcxEncoding] != 1) {
        return 3;
    }
    if (header[PcxVersion] < 4 && palette != nullptr) {
        return 3;
    }

    uint16_t x_min, y_min, x_max, y_max, bytes_per_line;
    memcpy(&x_min, header + PcxXMin, sizeof(x_min));
    memcpy(&y_min, header + PcxYMin, sizeof(y_min));
    memcpy(&x_max, header + PcxXMax, sizeof(x_max));
    memcpy(&y_max, header + PcxYMax, sizeof(y_max));
    memcpy(&bytes_per_line, header + PcxBytesPerLine, sizeof(bytes_per_line));
    const int width = x_max - x_min + 1;
    const int height = y_max - y_min + 1;
    const int bits_per_pixel = header[PcxBitsPerPixel];
    const int planes = header[PcxPlanes];

    GraphicWin *const owner = get_graphic_win();
    const int sized = (owner == 0)
        ? init(width, height, static_cast<int>(init_flags_), nullptr)
        : owner->resize(width, height, 1);
    if (sized != 0) {
        return sized;
    }
    if ((1 << bits_per_pixel) * planes != 256) {
        return 3;
    }

    uint8_t *pixels = reinterpret_cast<uint8_t *>(get_data());
    if (pixels == nullptr) {
        return 7;
    }

    // RLE: a byte with both top bits set is a run length and the byte after
    // it is the value; anything else is one literal pixel. `colour_base`
    // shifts every pixel into its slice of the palette.
    const int row_bytes = (8 / bits_per_pixel) * bytes_per_line * planes;
    const uint8_t *source = data + PcxHeaderSize;
    int outstanding = row_bytes * height;
    int filled = 0;
    while (outstanding != 0) {
        int run = 1;
        uint8_t value = *source++;
        if ((value & 0xC0) == 0xC0) {
            run = value & 0x3F;
            value = *source++;
        }
        memset(pixels, static_cast<uint8_t>(colour_base + value),
               static_cast<size_t>(run));
        pixels += run;
        filled += run;
        outstanding -= run;
        // A row ends short of the buffer's stride, so step over the gap and
        // give back whatever the run overshot by.
        if (row_bytes <= filled) {
            pixels += static_cast<int>(stride_) - filled;
            outstanding += filled - row_bytes;
            filled = 0;
        }
    }
    free_data(1);

    // The palette is the last 0x300 bytes of the file: 256 RGB triples,
    // stored here as BGR0 quads from `colour_base` onwards.
    const uint8_t *entry = data + size - 0x300;
    for (int index = 0; index < colour_count; ++index) {
        RGBQUAD &colour = dib_.bmiColors[colour_base + index];
        colour.rgbRed = entry[0];
        colour.rgbGreen = entry[1];
        colour.rgbBlue = entry[2];
        colour.rgbReserved = 0;
        entry += 3;
    }
    field_580_ = static_cast<uint8_t>(colour_base);

    if (palette == nullptr) {
        return 0;
    }
    palette->set_from_dib(&dib_);
    // sync_to_palette (0x005DE8F0, BYTE_EXACT elsewhere) is OPEN-CODED here
    // by the image - Palette::get_rgbquad at 0x005E2A1C and SetDIBColorTable
    // through [0x006690B4] at 0x005E2A7C are both emitted inline rather than
    // called through the standalone function - so the body is duplicated
    // rather than called.
    if (palette_seed_ != palette->seed_) {
        palette_seed_ = palette->seed_;
        RGBQUAD *const table = dib_.bmiColors;
        palette->get_rgbquad(table, 0, 0x100);
        // get_hdc and release_hdc, written out: the image inlines the whole
        // acquire protocol here (vtable GetDC at 0x005E2A56, ReleaseDC at
        // 0x005E2AB3, both counts and the hdc2_ publishes in between), and
        // the call form leaves ~60 instructions of this body unaccounted
        // for. Same idiom as Win::init_class.
        if (locked_bits_ != 0) {
            get_graphic_win();
        }
        IDirectDrawSurface *const surface = surface_;
        HDC acquired;
        if (surface == nullptr) {
            hdc2_ = hdc_;
            ++hdc_lock_count_;
            acquired = hdc_;
        } else if (hdc2_ != nullptr) {
            ++hdc_lock_count_;
            acquired = hdc2_;
        } else {
            if (surface->GetDC(&hdc2_) != 0) {
                get_graphic_win();
            }
            ++hdc_lock_count_;
            acquired = hdc2_;
        }
        if (acquired != nullptr) {
            SetDIBColorTable(hdc2_, 0, 0x100, table);
            // release_hdc(1), written out the same way.
            if (surface_ == nullptr) {
                --hdc_lock_count_;
                if (hdc_lock_count_ <= 0) {
                    hdc2_ = nullptr;
                    hdc_lock_count_ = 0;
                }
            } else {
                --hdc_lock_count_;
                if (hdc2_ != nullptr && hdc_lock_count_ <= 0) {
                    if (surface_->ReleaseDC(hdc2_) != 0) {
                        get_graphic_win();
                    }
                    hdc_lock_count_ = 0;
                    hdc2_ = nullptr;
                }
            }
        }
    }
    has_palette_ = 1;
    palette_ = palette;
    return 0;
}

int Buffer::copy(Buffer *buffer, int xCoord, int yCoord, int width,
                 int height) {
    // Through a pointer-to-member since the blitter moved in-class into
    // buffer.h (2026-09-17): a plain call lets /O2's /Ob1 fold the whole
    // 279-instruction body into this 35-byte shuffle, where the image makes
    // the call - the same measured lever the get_data sites in flic.cpp use.
    int (Buffer::*copy)(Buffer *, int, int, int, int, int, int) =
        &Buffer::copy;
    return (this->*copy)(buffer, xCoord, yCoord, xCoord, yCoord, width,
                         height);
}


/*
Purpose: Copy the region a rectangle describes out of another buffer into the
         same position in this one.
// ORIGINAL: 0x005D95E0 ?copy@Buffer@@QAEHPAVBuffer@@PAURECT@@@Z 0x005D95E0-0x005D960A BYTE_EXACT
// symbol    ?copy@Buffer@@QAEHPAV1@PAUtagRECT@@@Z
// size      42 bytes
// prototype int (__thiscall ?copy@Buffer@@QAEHPAVBuffer@@PAURECT@@@Z)(Buffer* this, Buffer*, RECT*)
// callers   8   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005DFF00
Return Value: whatever the full copy returns
Status: Complete with temporary full-copy dependency

The rectangle is half-open: the original subtracts before pushing, so the
extents are right-left and bottom-top rather than the inclusive spans. Like
the five-argument overload it hands the corner over twice, once as the
destination and once as the source.
*/
int Buffer::copy(Buffer *buffer, RECT *rect) {
    // TOP FIRST. The image loads `[eax+4]` before `[eax]`, which puts top in
    // edx and left in esi; reading left first swaps both registers and every
    // use downstream, including the argument push order.
    const int top = rect->top;
    const int left = rect->left;
    // Same pointer-to-member spelling as the five-int overload above: the
    // blitter is in-class in buffer.h now and a plain call inlines it here
    // too (279 instructions against this shuffle's 21).
    int (Buffer::*copy)(Buffer *, int, int, int, int, int, int) =
        &Buffer::copy;
    return (this->*copy)(buffer, left, top, left, top,
                         rect->right - left, rect->bottom - top);
}


/*
Purpose: Give the buffer a size and the storage behind it - a DirectDraw
         surface when the process has DirectDraw, a DIB section otherwise -
         and leave a device context selected onto it.
// ORIGINAL: 0x005D7670 ?init@Buffer@@QAEHHHHPAUExtDirectDraw@@@Z 0x005D7670-0x005D79BF
// size      847 bytes
// prototype int (__thiscall ?init@Buffer@@QAEHHHHPAUExtDirectDraw@@@Z)(Buffer* this, int, int, int, ExtDirectDraw*)
// callers   40   call targets   5
// kind      game
// flags     sp_ready;purged_ok
// calls     0x005D7470 0x005D8000 0x005FA8A0 0x00644EF2 0x006465F0
//
// `biHeight` IS NEGATIVE because the DIB is top-down, which is the whole
// reason the early-out reads `height == -dib_.bmiHeader.biHeight`.
// Every caller that reads it back negates it again - `Win::init_class`
// passes `-logo.height_` to `copy` - so the sign lives in the field.
//
// The four returns are 3 (bad argument), 1 (no device context), 0x12 (the
// surface or its clipper could not be created) and 0 (done, and also the
// early-out).
//
// THE `memset` IS A REAL CALL IN THE IMAGE - `push 0x6c; push 0; push edx;
// call _memset; add esp, 0xc` at 0x005D77C3 - and `/O2` implies `/Oi`,
// which expands ours to `rep stosd`. `#pragma function(memset)` is the only
// lever VC6 offers for that, and it is deliberately NOT used here.
//
// TRIED: the last six instructions, and they are NOT source-shaped.
// Under `/O2 /Oi- /Gy /GR- /GX` this is 280/286 with every instruction from
// 0x005D7930 to the epilogue identical and the two listings the SAME LENGTH.
// The whole divergence is one six-instruction window at 0x005D7924, where the
// image loads the counter into eax, loads hdc2_ through edi, increments, and
// stores the counter AFTER both pushes; we load into ecx and store BEFORE the
// hdc2_ load. Same instructions, different schedule and register.
//
// Seven spellings measured, all 280/286 to the instruction: `hdc_lock_count_++`,
// `+= 1`, `= hdc_lock_count_ + 1`, a read-modify-write through `int *const`,
// the HDC cached in a local before the increment, and the statements swapped
// (that one is WORSE, 276). The interesting hypothesis was aliasing - the
// image's order is the CONSERVATIVE one, which VC6 picks when the pointer has
// escaped - so `HDC *const dc = &hdc2_` was threaded through the real
// `GetDC(dc)` call so it genuinely escapes, both with and without the other
// uses rewritten as `*dc`. VC6 emits byte-identical code for all of them.
//
// `osmx.py semantic` REFUSES this, correctly: "instruction 220: mov against
// push" - the reorder means it is not allocation-only. Do not re-derive any of
// the above; the next lever, if there is one, is not in this statement.
//
// Measured both ways, because the pragma was here and had to earn its
// place: with it, similarity 0.859 and 69 instructions in position; without
// it, 0.705 and 80. NEITHER IS BYTE-EXACT, so what the pragma bought was a
// metric and not a claim - and it bought that by changing the intrinsic for
// every body below it in this file. `DDSURFACEDESC description = { 0 };`
// and `ZeroMemory` were both tried; VC6 inlines those too, identically.
Status: WIP
*/
int Buffer::init(int width, int height, int tgl, ExtDirectDraw *direct_draw) {
    // DECLARED HERE, not beside its use, and that is what the frame size
    // says: the image reserves 0x74 - `borrowed` at esp+0x10, this pointer
    // at esp+0x14, and the 0x6c descriptor from esp+0x18. Declared inside
    // the DirectDraw arm it shares a slot and the frame comes out 0x70.
    IDirectDrawSurface *created;

    const int borrowed = tgl & 4;
    if (borrowed != 0 && direct_draw == nullptr) {
        return 3;
    }
    if (width < 0 || height < 0) {
        return 3;
    }
    if (width == dib_.bmiHeader.biWidth
        && height == -dib_.bmiHeader.biHeight) {
        return 0;
    }

    close();
    spot_.init(0x28);
    link_count_ = 0;
    for (int slot = 0; slot < 20; ++slot) {
        if (owned_[slot] != nullptr) {
            free(owned_[slot]);
            owned_[slot] = nullptr;
        }
    }

    if (BufferDirectDraw == nullptr && borrowed == 0) {
        hdc_ = CreateCompatibleDC(nullptr);
        if (hdc_ == nullptr) {
            return 1;
        }
    }

    dib_.bmiHeader.biWidth = width;
    dib_.bmiHeader.biHeight = -height;

    // THE DIB ARM COMES FIRST, as the image lays it out: `CreateDIBSection`
    // and its fatal-error path are emitted before the DirectDraw block, not
    // after it. Written the other way round the same blocks come out in the
    // opposite order and 13 fewer instructions land in position.
    if (BufferDirectDraw == nullptr) {
        if (borrowed == 0) {
            bitmap_handle_ = CreateDIBSection(hdc_, &dib_, 0, &dib_bits_,
                                              nullptr, 0);
            if (bitmap_handle_ == nullptr) {
                MessageBoxA(nullptr,
                            "Unable to allocate draw-buffer; terminating program",
                            "FATAL ERROR", MB_ICONEXCLAMATION);
                exit(4);
            }
        }
    } else if (borrowed == 0) {
        DDSURFACEDESC description;
        memset(&description, 0, sizeof(description));
        description.dwSize = sizeof(description);
        description.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH;
        // IMAGE ORDER: the caps go in BEFORE the extent - `mov [esp+0x88],
        // 0x40840` precedes the width and height stores - and the width
        // precedes the height. VC6 keeps these in source order, so the
        // order here is a fact about the original and not a preference.
        // OWNDC, NOT VIDEOMEMORY. The image stores 0x40840 - OFFSCREENPLAIN
        // (0x40) | SYSTEMMEMORY (0x800) | OWNDC (0x40000) - where this had
        // 0x4840, which reads VIDEOMEMORY (0x4000) for the third flag. A
        // system-memory surface that also asks for video memory is a
        // contradiction; asking it to keep its own DC is what the buffer
        // wants, and is why `get_hdc` can hold one across calls.
        description.ddsCaps.dwCaps =
            DDSCAPS_OFFSCREENPLAIN | DDSCAPS_SYSTEMMEMORY | DDSCAPS_OWNDC;
        description.dwWidth = width;
        description.dwHeight = height;
        if (BufferDirectDraw->CreateSurface(&description, &created,
                                            nullptr) != 0) {
            return 0x12;
        }
        if (BufferDirectDraw->CreateClipper(0, &clipper_, nullptr) != 0) {
            return 0x12;
        }
        created->QueryInterface(IID_IDirectDrawSurface,
                                reinterpret_cast<void **>(&surface_));
    }
    if (borrowed != 0) {
        surface_ = direct_draw->surface;
        clipper_ = direct_draw->clipper;
    }

    // WRITTEN THROUGH THE POINTER IT PASSES. The image computes
    // `lea eax, [esi+0x30]` once, pushes it, and stores the four fields as
    // `[eax]`, `[eax+4]`, `[eax+8]`, `[eax+0xc]`. Storing to the members by
    // name lets VC6 address two of them off `esi` instead, which is one byte
    // shorter and moves every jump after it.
    RECT *const extent = &rect2_;
    extent->left = 0;
    extent->top = 0;
    extent->right = width;
    extent->bottom = height;
    set_clip(extent);

    dib_.bmiHeader.biWidth = width;
    dib_.bmiHeader.biHeight = -height;
    dib_.bmiHeader.biSizeImage = width * height;
    dib_.bmiHeader.biClrUsed = 256;

    if (BufferDirectDraw == nullptr) {
        previous_bitmap_ = SelectObject(hdc_, bitmap_handle_);
    }
    stride_ = (width + 3) & ~3;

    if (FontDefault != nullptr) {
        if (FontDefault->is_initialized()) {
            font1_ = FontDefault;
        }
        font2_ = nullptr;
        font3_ = nullptr;
        font4_ = nullptr;
    }

    if (locked_bits_ != 0) {
        get_graphic_win();
    }

    if (surface_ == nullptr) {
        hdc2_ = hdc_;
    } else if (hdc2_ == nullptr) {
        if (surface_->GetDC(&hdc2_) != 0) {
            get_graphic_win();
        }
    }

    ++hdc_lock_count_;
    SetBkMode(hdc2_, TRANSPARENT);
    // TA_BASELINE (0x18), not the three zero-valued flags. `TA_LEFT`,
    // `TA_TOP` and `TA_NOUPDATECP` are all 0, so that spelling passes 0 and
    // reads as though it said something; the image pushes 0x18, which is
    // TA_BASELINE on its own. Text placed at a baseline rather than a top
    // edge sits somewhere else on the screen - this is a behaviour
    // difference, not a spelling one.
    SetTextAlign(hdc2_, TA_BASELINE);

    if (surface_ == nullptr) {
        if (--hdc_lock_count_ <= 0) {
            hdc2_ = nullptr;
            hdc_lock_count_ = 0;
        }
    } else {
        const int remaining = --hdc_lock_count_;
        if (hdc2_ != nullptr && remaining <= 0) {
            if (surface_->ReleaseDC(hdc2_) != 0) {
                get_graphic_win();
            }
            hdc_lock_count_ = 0;
            hdc2_ = nullptr;
        }
    }

    init_flags_ = tgl;
    return 0;
}

/*
Purpose: Flood the whole buffer with one colour - through DirectDraw when
         there is a surface, and by writing the mapped bits when there is
         not.
// HAND-WRITTEN ASSEMBLY IN THE ORIGINAL, so byte-exactness is NOT reachable
// from C++ and the honest ceiling is semantic equivalence. The shipped bytes
// save and restore the FLAGS around a block - `pushf` ... `popf` - things a compiler has no reason to emit, because it owns the flags
// between setting and reading them.
//
// Found by `tools/handwritten_asm.py`. Do NOT grind spellings here, and do NOT
// answer it with `__asm`: a semantic C++ body is worth more than a byte-exact
// assembly one.
//
// ORIGINAL: 0x005DFB50 ?fill@Buffer@@QAEHH@Z 0x005DFB50-0x005DFCCD
// TRIED: byte-exactness, because the image's body is HAND-WRITTEN ASSEMBLY - a `pushf`/`cld`/`mul` block at 0x005DFC43 that saves no flags, clobbers callee-saved ebx without saving it, and uses opcodes VC6 does not emit from C++. This tree does not answer that with `__asm`. The reasoning was already written out below in prose, which the reader cannot see - so this body kept showing up as untouched and kept being re-picked. Best reached is 18/157; the frame alone is 0xE0 against 0xD0, four locals the asm block addresses directly.
// size      381 bytes
// prototype int (__thiscall ?fill@Buffer@@QAEHH@Z)(Buffer* this, int)
// callers   40   call targets   1
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x005DFCD0
//
// The three DirectDraw slots are `Blt` (5), `Lock` (25) and `Unlock` (32),
// checked against the interface; `EqualRect` is the import at 0x006692BC.
//
// CANNOT REACH BYTE-EXACT UNDER THIS PROJECT'S RULES, and the reason is in
// the image rather than in the recovery. 0x005DFC43 is an `__asm` block:
//
//     pushf                       ; nothing saves the flags around its own
//     push edi                    ; fill; nothing saves edi/esi it already
//     push esi                    ; owns; nothing emits `cld` defensively
//     cld
//     mov edi, [ebp-4]            ; dest    <- the four locals below
//     mov eax, [ebp-8]            ; width
//     mov ebx, eax                ; ...clobbering ebx, a CALLEE-SAVED
//     xor ecx, ecx                ;    register, with no save of its own
//     and ebx, 3
//     setne cl
//     shr eax, 2
//     add eax, ecx
//     mov ecx, [ebp-0x10]         ; height
//     mul ecx                     ; count = ((w+3)/4) * h
//     mov ecx, eax
//     mov dl, [ebp+8]             ; colour, broadcast to all four bytes
//     mov al, dl
//     mov ah, dl
//     shl eax, 0x10
//     mov al, dl
//     mov ah, dl
//     rep stosd
//     pop esi
//     pop edi
//     popf
//     mov esi, [ebp-0xc]          ; reload `this`, which it clobbered
//
// The loop is written in C. NOT EXCLUDED AND NOT FINISHED - a body that does
// not match yet, to be iterated on. The comment here used to say AGENTS.md
// bars `__asm` from recovered bodies; that reading looks stale, because
// `__declspec(naked)` blocks are in use in alphanet.cpp, leaf_recoveries.cpp
// and scroll.cpp today. Whether one belongs HERE is a separate question from
// whether they are allowed at all.
// EVERY REMAINING DIVERGENCE IS DOWNSTREAM OF THAT ONE DECISION, which is
// why it is worth having measured rather than argued:
//
//   * the four locals at [ebp-4], [ebp-8], [ebp-0xc], [ebp-0x10] exist only
//     because the block reads them by name - that is the 0x10 by which the
//     image's frame (0xE0) exceeds this one (0xD0);
//   * `this` is spilled at [ebp-0xc] and reloaded at 0x005DFC71 because the
//     block clobbers esi behind the allocator's back;
//   * ebx is unavailable to the allocator for the same reason, so the image
//     spells zero `push 0` / `test eax, eax` where this body, with ebx free,
//     gets `xor ebx, ebx` hoisted once and then `push ebx` / `cmp eax, ebx`.
//     One cause, eight of the twenty edits.
//
// What the C loop itself costs is NOTHING: VC6 recognises it and emits the
// same `rep stosd`. The gap is the hand-written prologue around it.
//
// Measured 119/157 mnemonics, 20 edits, against `/c /O2 /Oy- /Gy /GR- /GX` -
// the FRAMED set, which `flags frame` above already says and which the
// best-of-four report does not pick, since a frameless build ties on the
// aggregate while getting the prologue wrong.
//
// `WIP` below is the tree's word for "not BYTE_EXACT" and is the honest one,
// but it does not here mean "unfinished": there is no next edit to this body
// that raises the tier, only a rule change about `__asm`.
Status: WIP
*/
int Buffer::fill(int color) {
    // THE FRAME IS WHAT THIS FUNCTION IS ABOUT. The image reserves 0xE0:
    // DDBLTFX (0x64) at ebp-0x74, a DDSURFACEDESC (0x6C) at ebp-0xE0 - which
    // is `get_data` INLINED, not a local of its own - and four dwords above
    // them, one of which is a spill of `this` at ebp-0xc. Declaring the
    // descriptor here as well folds the two onto one slot and the prologue
    // changes shape, so it is not declared: `get_data()` brings its own.
    DDBLTFX effects;
    if (surface_ != nullptr) {
        effects.dwSize = sizeof(DDBLTFX);
        effects.dwFillColor = color;
        return surface_->Blt(nullptr, nullptr, nullptr,
                             DDBLT_COLORFILL | DDBLT_WAIT, &effects);
    }
    if (dib_bits_ == nullptr) {
        return 0;
    }
    // A buffer whose clip rectangle is not the whole buffer fills the
    // rectangle instead, through the overload that takes one.
    if (!EqualRect(&rect1_, &rect2_)) {
        return fill(&rect1_, color);
    }

    // `get_data` BY NAME, now that it is in-class and VC6 can fold it in.
    // The protocol was written out here, which is the same instructions in a
    // different order and its own DDSURFACEDESC in a different slot.
    void *const pixels = reinterpret_cast<void *>(get_data());

    // The colour in all four bytes, one dword per pixel-quad, rows of
    // `(width + 3) / 4` dwords.
    // UNSIGNED throughout: the image divides with `shr eax, 2` and
    // multiplies with `mul`. Written `int`, the same expression compiles to
    // `cdq; sbb; and; add; sar` - six instructions of sign correction for a
    // count that cannot be negative.
    const uint32_t height = static_cast<uint32_t>(rect2_.bottom);
    const uint32_t width = static_cast<uint32_t>(rect2_.right);
    const uint8_t value = static_cast<uint8_t>(color);
    const uint32_t quad = (static_cast<uint32_t>(value) << 24)
        | (static_cast<uint32_t>(value) << 16)
        | (static_cast<uint32_t>(value) << 8) | value;
    uint32_t *out = static_cast<uint32_t *>(pixels);
    const uint32_t count =
        ((width >> 2) + ((width & 3) != 0 ? 1u : 0u)) * height;
    for (uint32_t index = 0; index < count; ++index) {
        out[index] = quad;
    }

    if (surface_ == nullptr) {
        if (--surface_lock_count_ <= 0) {
            locked_bits_ = nullptr;
            surface_lock_count_ = 0;
        }
        return 0;
    }
    if (--surface_lock_count_ <= 0 && locked_bits_ != nullptr) {
        surface_->Unlock(locked_bits_);
        locked_bits_ = nullptr;
        surface_lock_count_ = 0;
    }
    return 0;
}

/*
Purpose: Fill a rectangle of this buffer with a single colour, through
         DirectDraw when there is a surface and by writing the mapped bits
         when there is not.
// HAND-WRITTEN ASSEMBLY IN THE ORIGINAL, so byte-exactness is NOT reachable
// from C++ and the honest ceiling is semantic equivalence. The shipped bytes
// save and restore the FLAGS around a block - `pushf` ... `popf` - things a compiler has no reason to emit, because it owns the flags
// between setting and reading them.
//
// Found by `tools/handwritten_asm.py`. Do NOT grind spellings here, and do NOT
// answer it with `__asm`: a semantic C++ body is worth more than a byte-exact
// assembly one.
//
// ORIGINAL: 0x005DFCD0 ?fill@Buffer@@QAEHPAURECT@@H@Z 0x005DFCD0-0x005DFEFC
// TRIED: byte-exactness, because 0x005DFE69 is a HAND-WRITTEN ASSEMBLY
// block that spends EBP as a row counter (`push ebp; mov ebp, ecx` ... `dec
// ebp; jne row`) inside a function with a real frame, and routes the
// per-row stride through a named file-scope global (`add edi,
// [0x9B3A10]`) rather than a loop-invariant a compiler would keep in a
// register. Neither is something VC6 emits from C++, so this tree does not
// answer it with `__asm`; the loop is written in C and best reached is
// 189/215 mnemonics, 24 edits, against the FRAMED flag set. Do not
// re-derive this - see the prose below for the full accounting.
// symbol    ?fill@Buffer@@QAEHPAUtagRECT@@H@Z
// size      556 bytes
// prototype int (__thiscall ?fill@Buffer@@QAEHPAURECT@@H@Z)(Buffer* this, RECT* rect, int)
// callers   8   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
//
// `IntersectRect` is the import at 0x00669338; the DirectDraw slots are
// `Blt` (5, +0x14), `Lock` (25, +0x64) and `Unlock` (32, +0x80), the same
// three the no-argument overload uses.
//
// THREE RETURN CODES, and they are the only place this class says anything
// but 0: 7 when the buffer has neither bits nor surface, 3 when the software
// path is asked to fill a null rectangle, 0 everywhere else including every
// failure.
//
// CANNOT REACH BYTE-EXACT, for the same reason as `fill(int)` and more
// plainly. 0x005DFE69 is an `__asm` block, and this one settles it beyond
// argument - it uses EBP as its row counter:
//
//     pushf / push edi / push esi / cld
//     mov edi, [ebp-0x18]        ; dest
//     mov edx, [ebp-0x1c]        ; width
//     mov ebx, edx / and ebx, 3  ; tail bytes
//     shr edx, 2                 ; whole dwords
//     mov cl, [ebp+0xc] ...      ; colour broadcast to four bytes
//     push ebp                   ; <-- THE FRAME POINTER, saved so the
//     mov ebp, ecx               ;     block can use it as a loop counter
//   row:
//     mov ecx, edx / rep stosd
//     mov ecx, ebx / rep stosb
//     add edi, [0x9B3A10]        ; the row gap, via a named global
//     dec ebp / jne row
//     pop ebp / pop esi / pop edi / popf
//     mov esi, [ebp-0x14]        ; reload `this`
//
// A compiler cannot spend EBP in a function it built a frame for, and it
// has no reason to route a loop-invariant through a file-scope global. Both
// are things only a human writing assembly does. AGENTS.md bars `__asm`
// from recovered bodies, so the loop is C and the gap is measured:
// 189 of 215 mnemonics, 24 edits, frame 0xF0 against the image's 0xEC.
//
// The C loop is not what costs it - VC6 recognises the fill and emits the
// same `rep stosd`. The 24 are the block's own bookkeeping plus one
// consequence that reaches the PROLOGUE: the image spills `this` to
// [ebp-0x14] at instruction 7 and reloads it at 0x005DFEA6 purely because
// the block clobbers ESI, and with no such pressure the allocator here
// keeps `this` in EBX and orders the first six instructions differently.
// That is edits #4, #5, #14 and #17, 300 bytes before the `__asm`.
//
// ONE THING IN THE ORIGINAL WORTH FLAGGING, left as the image has it
// because byte-exactness is the goal: the Blt path passes `&clipped` as the
// destination rectangle even when `area` is null - in which case nothing
// ever wrote to `clipped` and DirectDraw is handed an uninitialised RECT.
// Only reachable through a direct call with a null rectangle; `fill(int)`
// always passes `&rect1_`.
//
// THE `neg` IS NOT A BUG, though it was recorded as a suspected one here
// first. `biHeight` is negative - a top-down DIB - and `Buffer::init` carries both
// halves of the round trip: it stores the negation at 0x005D7758
// (`neg edi; mov [esi+0x84], edi`, and again from that same EDI at
// 0x005D788D on the DIB path), then reads it back through a second `neg`
// at 0x005D76BE-0x005D76C6 to compare against a positive height in its
// "already this size" early-out. So `-biHeight` IS the positive height, and
// `clipped.top >= -biHeight` is the ordinary bounds test its `+biWidth`
// neighbour looks like.
Status: WIP
*/
int Buffer::fill(RECT *area, int color) {
    // ALL THREE AT FUNCTION SCOPE, which the 0xEC frame requires: the image
    // lays out `clipped` at [ebp-0x10], DDBLTFX at [ebp-0x80] and
    // DDSURFACEDESC at [ebp-0xEC], never overlapping. Declared inside the
    // branches that use them their lifetimes are disjoint, VC6 folds them
    // onto shared slots, and the frame shrinks to 0x9C. One `clipped` too:
    // the two paths share the slot in the image.
    RECT clipped;
    DDBLTFX effects;
    DDSURFACEDESC description;

    if (dib_bits_ == nullptr && surface_ == nullptr) {
        return 7;
    }
    if (surface_ != nullptr) {
        effects.dwSize = sizeof(DDBLTFX);
        effects.dwFillColor = color;
        if (area != nullptr && !IntersectRect(&clipped, area, &rect1_)) {
            return 0;
        }
        // See the note above: `clipped` is uninitialised here when `area`
        // is null, exactly as the image leaves it.
        surface_->Blt(&clipped, nullptr, nullptr,
                      DDBLT_COLORFILL | DDBLT_WAIT, &effects);
        return 0;
    }

    if (area == nullptr) {
        return 3;
    }
    clipped = *area;
    if (!IntersectRect(&clipped, &clipped, &rect1_)) {
        return 0;
    }
    if (clipped.left >= dib_.bmiHeader.biWidth) {
        return 0;
    }
    // `biHeight` holds the negative, so this is the plain bounds test.
    if (clipped.top >= -dib_.bmiHeader.biHeight) {
        return 0;
    }

    void *pixels;
    if (surface_ == nullptr) {
        locked_bits_ = dib_bits_;
        pixels = dib_bits_;
        if (dib_bits_ == nullptr) {
            return 0;
        }
        ++surface_lock_count_;
    } else if (locked_bits_ != nullptr) {
        ++surface_lock_count_;
        pixels = locked_bits_;
    } else {
        description.dwSize = sizeof(DDSURFACEDESC);
        if (surface_->Lock(nullptr, &description, DDLOCK_WAIT, nullptr) != 0) {
            return 0;
        }
        ++surface_lock_count_;
        stride_ = description.lPitch;
        locked_bits_ = description.lpSurface;
        pixels = description.lpSurface;
    }
    if (pixels == nullptr) {
        return 0;
    }

    uintptr_t destination = reinterpret_cast<uintptr_t>(pixels)
        + static_cast<uintptr_t>(stride_) * static_cast<uintptr_t>(clipped.top)
        + static_cast<uintptr_t>(clipped.left);
    if (destination == 0) {
        return 0;
    }

    int rows = clipped.bottom - clipped.top;
    // UNSIGNED, so the split into whole dwords is `shr edx, 2` as the image
    // has it rather than the `sar` a signed width earns.
    const uint32_t width = static_cast<uint32_t>(clipped.right - clipped.left);
    BufferFillRowGap = stride_ - width;

    const uint8_t value = static_cast<uint8_t>(color);
    const uint32_t quad = (static_cast<uint32_t>(value) << 24)
        | (static_cast<uint32_t>(value) << 16)
        | (static_cast<uint32_t>(value) << 8) | value;
    do {
        uint8_t *tail = reinterpret_cast<uint8_t *>(destination);
        uint32_t count;
        for (count = width >> 2; count > 0; --count) {
            // x86 permits unaligned REP STOSD; a typed uint32_t store does
            // not. Copy the representation into byte-addressed pixel storage.
            memcpy(tail, &quad, sizeof(quad));
            tail += sizeof(quad);
        }
        for (count = width & 3; count > 0; --count) {
            *tail++ = value;
        }
        // REP STOSD/STOSB advance through the row before the original
        // adds its stride-minus-width gap at 0x005DFE98.
        // Keep the unused final-row advance numeric: with a nonzero left
        // edge it can lie past the backing allocation's one-past pointer.
        destination = reinterpret_cast<uintptr_t>(tail) + BufferFillRowGap;
    } while (--rows != 0);

    if (surface_ == nullptr) {
        if (--surface_lock_count_ <= 0) {
            locked_bits_ = nullptr;
            surface_lock_count_ = 0;
        }
        return 0;
    }
    --surface_lock_count_;
    if (locked_bits_ != nullptr && surface_lock_count_ <= 0) {
        surface_->Unlock(locked_bits_);
        locked_bits_ = nullptr;
        surface_lock_count_ = 0;
    }
    return 0;
}

/*
Purpose: Build a RECT from the four coordinates and forward to the RECT
         overload.
// ORIGINAL: 0x005D8240 ?fill@Buffer@@QAEHHHHHH@Z 0x005D8240-0x005D8282 BYTE_EXACT
// size      66 bytes
// prototype int (__thiscall ?fill@Buffer@@QAEHHHHHH@Z)(Buffer* this, int, int, int, int, int)
// callers   3   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005DFCD0
Return Value: whatever the RECT overload returns
Status: Complete
*/
int Buffer::fill(int left, int top, int width, int height, int color) {
    RECT area;
    SetRect(&area, left, top, left + width, top + height);
    return fill(&area, color);
}

/*
Purpose: Load a PCX file by name into this buffer, supplying a default
         extension and memory-mapping the file for the decoder.
// ORIGINAL: 0x005D7DE0 ?load_pcx@Buffer@@QAEHPBDPAVPalette@@HH@Z 0x005D7DE0-0x005D7F1D;0x00662BBC-0x00662BD1 BYTE_EXACT
// size      338 bytes
// prototype int (__thiscall ?load_pcx@Buffer@@QAEHPBDPAVPalette@@HH@Z)(Buffer* this, int8* fileName, Palette*, int, int size)
// callers   44   call targets   6
// kind      game
// flags     seh;sp_ready;purged_ok
// calls     0x00628380 0x006283E0 0x00628430 0x005E2690 0x00645470 0x006453E0
//
// TWO RANGES, and the 338 is their sum: 317 bytes of body at 0x005D7DE0 and
// a 21-byte EH funclet at 0x00662BBC. Written as one range ending at
// 0x005D7F31 - 338 counted forward from the entry - the span swallows
// `sub_5d7f20`, and the comparison then measures this body against the next
// function's prologue and can never agree.
//
// A NAME FIXER AND A FILE MAPPER; the decoding is all in the five-argument
// overload at 0x005E2690, which takes the mapped bytes and their length.
//
// The `Filemap` local is what makes this function carry an EH frame -
// `push -1; push 0x662BC7; mov eax, fs:[0]` - and it is constructed at
// instruction 10, BEFORE the null-filename test at 13. Declaring it after
// that test would move the constructor call past the early return and
// change the shape of the whole prologue, so the declaration stays first.
//
// Return codes: 3 for a null name, 6 when the file will not open, and
// otherwise whatever the decoder returns.
//
// `path` is 260 bytes - MAX_PATH - which the frame settles rather than
// guesses: `sub esp, 0x114` with the 16-byte Filemap at offset 0 and the
// buffer at offset 0x10 leaves exactly 0x104.
//
// BYTE_EXACT, 317/317, against `/c /O2 /Gy /GR- /GX`.
Status: Complete
*/

int Buffer::load_pcx(const char *filename, Palette *palette, int tgl, int height) {
    Filemap map;
    if (filename == nullptr) {
        return 3;
    }

    char path[MAX_PATH];
    path[0] = '\0';
    strcat(path, filename);
    // Supply `.pcx` unless the name already carries an extension. The scan
    // starts ON the terminator and walks down, so a dot at position 0 is
    // never reached and counts as no extension - which is what the image
    // does, `cmp eax, ecx` against the start of the buffer both before the
    // loop and after it.
    char *scan = path + strlen(filename);
    while (scan != path && *scan != '.') {
        --scan;
    }
    if (scan == path) {
        strcat(path, ".pcx");
    }

    if (map.open_read(path, TRUE) == nullptr) {
        return 6;
    }
    // A SEPARATE STATEMENT, not an argument. MSVC evaluates arguments
    // right to left, so written inline the GetFileSize call would land
    // between the `palette` and `view` pushes; the image makes it first,
    // at 0x005D7EC1, and pushes its result from EAX four instructions
    // later - which is what a local assigned before the call looks like.
    const DWORD size = GetFileSize(map.get_handle(), nullptr);
    return load_pcx(static_cast<BYTE *>(map.get_view()), size,
                    palette, tgl, height);
}

// Read the dimensions from the mapped PCX header without loading its image
// data into a Buffer. The file carries the same optional asset watermark as
// the regular PCX decoder above. The original three-argument cdecl callers
// and receiver-free body establish a static utility; the old catalogue
// spelling is retained separately from the current compiler symbol.
// ORIGINAL: 0x005E2480 ?get_pcx_dimensions@Buffer@@QAAHPBDPAHPAH@Z 0x005E2480-0x005E2690;0x00662C11-0x00662C26
// RECOVERY_BINDING: v1 {"evidence": ["source:src/unrecovered/005e2480.cpp", "pinned-image:0x00696BF8,0x00696CFC,0x00696D04,0x00696D0C"], "reason": "Home PCX header dimension decoding in Buffer using its Filemap lifecycle and source-owned format literals"}
// symbol    ?get_pcx_dimensions@Buffer@@SAHPBDPAH1@Z
int Buffer::get_pcx_dimensions(const char *filename, int *width, int *height) {
    Filemap map;
    if (filename == nullptr || width == nullptr || height == nullptr) {
        return 3;
    }

    char path[MAX_PATH];
    path[0] = '\0';
    strcat(path, filename);
    char *scan = path + strlen(filename);
    while (scan != path && *scan != '.') {
        --scan;
    }
    if (scan == path) {
        const char *extension;
        switch (Language) {
            case 1: extension = ".pcf"; break;
            case 2: extension = ".pcg"; break;
            default: extension = ".pcx"; break;
        }
        strcat(path, extension);
    }

    if (map.open_read(path, TRUE) == nullptr) {
        return 6;
    }
    GetFileSize(map.get_handle(), nullptr);

    const char *content = static_cast<const char *>(map.get_view());
    const size_t prefix_length = strlen(PcxAssetPrefix);
    if (strncmp(content, PcxAssetPrefix, prefix_length) == 0) {
        content += prefix_length + 1;
    }

    unsigned char header[0x80];
    memcpy(header, content, sizeof(header));
    if (header[0] == '\n' && header[2] == 1) {
        const int xmin = header[4] | (header[5] << 8);
        const int ymin = header[6] | (header[7] << 8);
        const int xmax = header[8] | (header[9] << 8);
        const int ymax = header[10] | (header[11] << 8);
        *width = xmax - xmin + 1;
        *height = ymax - ymin + 1;
        map.close();
        return 0;
    }

    return 3;
}

/*
Purpose: THE BLITTER. Copy a rectangle out of this buffer into `buffer`,
         clipping the read against this buffer's own extent and the write
         against the destination's clip rectangle, and handling the case
         where source and destination are the same object.
// HAND-WRITTEN ASSEMBLY IN THE ORIGINAL, so byte-exactness is NOT reachable
// from C++ and the honest ceiling is semantic equivalence. The shipped bytes
// save and restore the FLAGS around a block - `pushf` ... `popf` - things a compiler has no reason to emit, because it owns the flags
// between setting and reading them.
//
// Found by `tools/handwritten_asm.py`. Do NOT grind spellings here, and do NOT
// answer it with `__asm`: a semantic C++ body is worth more than a byte-exact
// assembly one.
//
// ORIGINAL: 0x005DFF00 ?copy@Buffer@@QAEHPAVBuffer@@HHHHHH@Z 0x005DFF00-0x005E079B
// TRIED: byte-exactness, twice over, and it was RULED OUT IN PROSE ONLY
//   until now - which is why `frontier.py --untouched` kept offering this
//   body. Both reasons are mechanical facts, not judgements.
//   (1) The original disassembly contains `pushf` and
//   `popf`. The shipped bytes save and restore FLAGS around a block, which
//   no compiler emits because it owns the flags between setting and reading
//   them, so no C++ spelling can produce these bytes.
//   (2) Measured 2026-08-22: 196 instructions against the image's 761,
//   similarity 0.255, first divergence at instruction 7. The image writes
//   the same-buffer case FOUR times - one arm per combination of "the
//   horizontal shift needs a backwards copy" and "the vertical shift needs
//   rows in reverse" - each repeating the whole clip / `get_data` /
//   pointer-compute / `free_data` sequence inline; the three largest gaps
//   are runs of 188, 199 and 89 instructions. This body collapses those
//   four arms into a row direction plus `memmove`, deliberately.
//   The honest ceiling is SEMANTIC, and `osmx semantic` refuses at this
//   instruction count. Closing it means writing the four arms back out,
//   which is a different body rather than an adjustment to this one - and
//   it still could not be byte-exact, because of (1). Do not answer it with
//   `__asm`: a semantic C++ body is worth more than a byte-exact assembly
//   one.
// symbol    ?copy@Buffer@@QAEHPAV1@HHHHHH@Z
// size      2203 bytes
// prototype int (__thiscall ?copy@Buffer@@QAEHPAVBuffer@@HHHHHH@Z)(Buffer* this, Buffer*, int, int, int, int, int, int)
// callers   51   call targets   2
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005E3373 0x005E34A3
// indirect  0x005DFFDD 0x005E0705
Return Value: 0 for a completed or fully clipped copy, 3 for a bad argument
              or a surface that would not lock, 7 for no pixel storage
Status: WIP

PROMOTED FROM src/recovered/units/005dff00.cpp, which is deleted. That unit
reached the image through raw offsets - `*(int *)(bb + 0x4a8)` for the
destination stride, `*(int *)(self + 0x84)` for the negated height - and
through two macros and four `goto` labels, because when it was written none
of those offsets had names. They all do now, and the two macros are one
call: `REL_BUF_R3` and the `LRelSurfR3` label together are `free_data`, the
no-surface arm and the surface arm, inlined by hand.

NOT BYTE-EXACT, AND FAR FROM IT: 237 instructions against the image's 761,
similarity 0.255. The gap is one deliberate decision, and it is worth
knowing before anyone tries to close it.

The image writes the same-buffer case FOUR TIMES - once per combination of
"the horizontal shift needs a backwards copy" and "the vertical shift needs
rows in reverse" - and each of the four repeats the whole clip,
`get_data`, pointer-compute and `free_data` sequence inline. That is close
to five hundred instructions of duplication, and it accounts for nearly all
of the difference: the three largest gaps against the image are runs of
188, 199 and 89 instructions.

Here the four arms collapse into a row direction and `memmove`, because
`memmove` is the C spelling of both halves of what the direction flag does.
That is one readable arm instead of four unreadable ones, and it is the
same computation. The preserved unit chose the other way - it kept all four
arms specifically so "the clip/get_data()/pointer-compute code is repeated
with the same multiplicity as the original" - and even then could not
express the reversed intra-row copy through `memcpy`, so it did not match
either.

To close this properly the four arms have to come back, with `get_data` and
`free_data` inlined at each of them, which is a different body from this
one rather than an adjustment to it.
*/
// MOVED IN-CLASS 2026-09-17, restoring the header placement the image had.
// The flic translation unit is keyed U (see the top of flic.cpp) so its
// `Buffer::copy` references mangle PAU, and no out-of-line PAV definition
// can resolve them - the original link carried both spellings because each
// spelled its own inline COMDAT, folded together by ICF. Same arrangement
// as get_data above: the body lives in buffer.h, the marker and this
// history stay here.
// body src/buffer.h

/*
Purpose: Release every resource the buffer owns and reset it to its
         constructed state.
// ORIGINAL: 0x005D7470 ?close@Buffer@@QAEXXZ 0x005D7470-0x005D7665 BYTE_EXACT
// size      501 bytes
// prototype void (__thiscall ?close@Buffer@@QAEXXZ)(Buffer* this)
// callers   44   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00644EF2
// indirect  0x005D74E6 0x005D74F1 0x005D750E 0x005D751B 0x005D7535 0x005D7542 0x005D7559 0x005D7566
// notes     Runtime redirect installed by DllMain after byte-signature validation
Status: Complete
*/
void Buffer::close() {
    // Twenty owned allocations released through the executable's allocator.
    // The member, not `this` plus 0x4BC: `Buffer::init` walks the same array
    // and the extent is proved - see `owned_` in buffer.h.
    for (size_t index = 0; index < OwnedAllocationCount; ++index) {
        if (owned_[index] != nullptr) {
            free(owned_[index]);
            owned_[index] = nullptr;
        }
    }

    if (hdc_ != nullptr) {
        hdc_lock_count_ = 0;
        surface_lock_count_ = 0;
        if (BufferDirectDraw != nullptr) {
            // THE SURFACE TEST COMES FIRST, and the decrement is written
            // out in BOTH arms rather than hoisted above them - the image is
            // `cmp [esi+0x58], ebx; jne` and only then `mov ecx, [esi+0x68];
            // dec ecx`. Hoisted, the count lands one instruction early and
            // the whole tail shifts. `Buffer::release_hdc` needed the same
            // shape. The count was just zeroed, so either decrement lands at
            // or below zero and the published data is dropped.
            if (surface_ == nullptr) {
                if (--hdc_lock_count_ <= 0) {
                    hdc2_ = nullptr;
                    hdc_lock_count_ = 0;
                }
            } else {
                const int remaining = --hdc_lock_count_;
                if (hdc2_ != nullptr && remaining <= 0) {
                    if (surface_->ReleaseDC(hdc2_) != 0) {
                        get_graphic_win();
                    }
                    hdc_lock_count_ = 0;
                    hdc2_ = nullptr;
                }
            }
        } else if ((init_flags_ & 4U) == 0) {
            if (previous_bitmap_ != 0) {
                SelectObject(hdc_, reinterpret_cast<HGDIOBJ>(previous_bitmap_));
                previous_bitmap_ = nullptr;
            }
            DeleteDC(hdc_);
            hdc2_ = nullptr;
            hdc_ = nullptr;
        }
    }

    if (bitmap_handle_ != nullptr) {
        DeleteObject(bitmap_handle_);
        bitmap_handle_ = nullptr;
    }
    if (clip_region_ != nullptr) {
        DeleteObject(clip_region_);
        clip_region_ = nullptr;
    }

    if (BufferDirectDraw != nullptr) {
        if (surface_ != nullptr) {
            surface_->Release();
        }
        if (clipper_ != nullptr) {
            clipper_->Release();
        }
    }

    surface_ = nullptr;
    clipper_ = nullptr;
    field_C_ = 0;
    field_8_ = 0;
    field_18_ = 0;
    field_14_ = 0;
    field_10_ = 0;
    palette_seed_ = 0;
    stride_ = 0;
    has_palette_ = 0;
    field_580_ = 0;
    dib_.bmiHeader.biWidth = 0;
    dib_.bmiHeader.biHeight = 0;
    locked_bits_ = 0;
    dib_bits_ = nullptr;
    field_50C_ = 0xFFFFFFFFU;
    write_font_slot_ = 0;
    field_514_ = 0;
    font_slot_ = 0;
    markup_pending_ = 0;
    markup_enabled_ = BufferField520Default;
    font1_ = FontDefault;
    // Four text slots at 0x530..0x53C with five rows 0x10 apart. The layout
    // is deliberately not uniform: the fourth slot holds 2 where the others
    // hold -1 at row two, and 0 where the others hold 2 at row four, so the
    // stores are written out rather than generated.
    color_val_1_ = 0;
    color_val_2_ = 0xFFFFFFFFU;
    color_val_3_ = 2;
    color_val_4_ = 2;
    font2_ = nullptr;
    color_2_val_1_ = 0xFFFFFFFFU;
    color_2_val_2_ = 0xFFFFFFFFU;
    color_2_val_3_ = 2;
    color_2_val_4_ = 2;
    font3_ = nullptr;
    color_3_val_1_ = 0xFFFFFFFFU;
    color_3_val_2_ = 0xFFFFFFFFU;
    color_3_val_3_ = 2;
    color_3_val_4_ = 2;
    font4_ = nullptr;
    color_hyper_val_1_ = 0xFFFFFFFFU;
    color_hyper_val_2_ = 0xFFFFFFFFU;
    color_hyper_val_3_ = 2;
    color_hyper_val_4_ = 2;
    palette_ = nullptr;
    init_flags_ = 0;
}


/*
Purpose: Destroy a Buffer by installing its virtual table, releasing every
         owned resource, and destroying the trailing Spot subobject.

         THE BODY IS `~Buffer() { close(); }`, and those three tokens are the
         whole function. The vtable store, the `Spot` at +0x4B0 and the EH
         frame that unwinds it are all things the compiler emits around a
         virtual destructor with a member of class type; the image's 20
         instructions are what VC6 writes for that source, so spelling any of
         them out by hand - as the stand-in here used to, through a volatile
         pointer and a raw 0x4B0 - both missed the match and described the
         layout twice.
// ORIGINAL: 0x005D7410 ??1Buffer@@QAE@XZ 0x005D7410-0x005D7463;0x00662B68-0x00662B80 BYTE_EXACT
// LEVER: THE BODY STAYS IN buffer.h. Measured 2026-09-03 chasing MapWin's
//            destructor (0x00420F90), which the image spells as three
//            `call ??1Buffer`s where this tree expands the inline: defining
//            it here, out of line, did make 0x00420F90 byte-exact - and
//            silenced the compiler's deleting-destructor emission in every
//            other TU, costing deleting_thunks.cpp its ??_GBuffer claim at
//            0x005E3610 (UNRESOLVED). Same body either way; the difference
//            is which call sites the compiler is willing to build it in.
// symbol    ??1Buffer@@UAE@XZ
// size      107 bytes
// prototype void (__thiscall ??1Buffer@@QAE@XZ)(Buffer* this)
// callers   87   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005D7470 0x005FA870
// notes     Runtime redirect installed by DllMain after byte-signature validation
// LEVER: OUT OF LINE (2026-09-03, was `MEASURED virtual ~Buffer() { close(); }`
//   in buffer.h). The in-class definition let /O2's /Ob1 expand the teardown
//   into every local Buffer's scope end - `close()` then `~Spot()`, plus the
//   vptr store - where the image makes one `call ??1Buffer@@QAE@XZ` at all 87
//   of its sites. Same lever as general.h's bitmask_call: the inliner has no
//   body to fold. checkbox/radiobutton/sprite init_class and palette's
//   textcolor_init all carry the expansion.
Status: Complete
*/

MEASURED Buffer::~Buffer() { close(); }

void __fastcall buffer_destructor_redirect(Buffer *self, void *) {
    // QUALIFIED, so it is the direct `call ??1Buffer@@QAE@XZ` the image makes
    // and not a dispatch through the vtable slot.
    self->Buffer::~Buffer();
}

/*
Purpose: Acquire the device context, taking one reference on the shared handle.
// ORIGINAL: 0x005E3503 ?get_hdc@Buffer@@QAEHXZ 0x005E3503-0x005E355C BYTE_EXACT
// symbol    ?get_hdc@Buffer@@QAEPAUHDC__@@XZ
// size      89 bytes
// prototype int (__thiscall ?get_hdc@Buffer@@QAEHXZ)(Buffer* this)
// callers   9   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x005E3510 0x005E3542 0x005E354D
Return Value: The device context, or zero when the surface refuses one
Status: Complete
*/

/*
Purpose: Drop the given number of device-context references, releasing the
         handle once the last one is gone.
// ORIGINAL: 0x005E3563 ?release_hdc@Buffer@@QAEXH@Z 0x005E3563-0x005E35C3 BYTE_EXACT
// size      96 bytes
// prototype void (__thiscall ?release_hdc@Buffer@@QAEXH@Z)(Buffer* this, int)
// callers   8   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x005E35AA 0x005E35B5
Status: Complete
*/


/*
Purpose: Republish a palette into the buffer's colour table and device context,
         skipping the work when the palette has not changed.
// ORIGINAL: 0x005DE8F0 ?sync_to_palette@Buffer@@QAEHPAVPalette@@@Z 0x005DE8F0-0x005DEA12 BYTE_EXACT
// size      290 bytes
// prototype int (__thiscall ?sync_to_palette@Buffer@@QAEHPAVPalette@@@Z)(Buffer* this, Palette*)
// callers   24   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005FE560
// indirect  0x005DE94E 0x005DE978 0x005DE983 0x005DE99E 0x005DE9DB 0x005DE9E6
Return Value: No errors (0); no pixel storage (7); null palette (3)
Status: Complete
Verification note: the publish branch is reachable - the fixture's surface
stub hands back a synthetic device context - and the release that follows it
is verified through the reference count and the surface's ReleaseDC. Only the
SetDIBColorTable call itself is unobservable: it is a real GDI import, and on
a synthetic handle it fails harmlessly with no effect any assertion can see,
so its four mutants survive. Reordering get_rgbquad against the acquire is
likewise equivalent, since the two touch disjoint state.
*/
int Buffer::sync_to_palette(Palette *palette) {
    if (!dib_bits_) {
        return 7;
    }
    if (!palette) {
        return 3;
    }
    // An unchanged palette costs nothing: `seed_` is the generation and
    // `palette_seed_` is the one this buffer's colour table already holds.
    if (palette_seed_ != palette->seed_) {
        palette_seed_ = palette->seed_;
        RGBQUAD *const table = dib_.bmiColors;
        palette->get_rgbquad(table, 0, 0x100);
        // THE MEMBER, NOT THE RETURNED VALUE. `get_hdc` hands back `hdc_`
        // and the image tests what it returns, but the call RELOADS it:
        //
        //   mov eax, [edi]   test eax, eax   je ...
        //   mov ecx, [edi]   push ecx   call SetDIBColorTable
        //
        // Holding the result in a local passes `eax` straight through, which
        // is one instruction shorter and moves every jump after it.
        if (get_hdc() != nullptr) {
            SetDIBColorTable(hdc2_, 0, 0x100, table);
            release_hdc(1);
        }
    }
    has_palette_ = 1;
    palette_ = palette;
    return 0;
}


/*
Purpose: Report the height of the buffer's text font, resolving the process
         default the first time it is needed.
// ORIGINAL: 0x005DCA80 ?text_height@Buffer@@QAEHXZ 0x005DCA80-0x005DCAA5 BYTE_EXACT
// LEVER: single trailing `return font1_->height_;` after the `if` (no early
//        return inside it) - two early `return`s each recompute a fresh
//        load in the non-null path, one merged return reuses the memory
//        operand the image's own non-null path re-reads for.
// size      37 bytes
// prototype int (__thiscall ?text_height@Buffer@@QAEHXZ)(Buffer* this)
// callers   5   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: The font's height
Status: Complete
*/
int Buffer::text_height() {
    // CORRECTED. This used to say the legacy body loads the default BEFORE
    // testing the cached font, and hoisted `FontDefault` accordingly. The
    // shipped bytes say the opposite: `mov eax, [ecx+0x52c] / test eax, eax /
    // jne` comes first and `mov eax, [0x9bb484]` only after the branch is not
    // taken. The hoist put the load ahead of the test and cost the match.
    if (!font1_) {
        font1_ = FontDefault;
    }
    return font1_->height_;
}


/*
Purpose: Clip the buffer to a rectangle, updating the GDI clip region and the
         DirectDraw clipper to match.
// ORIGINAL: 0x005D8000 ?set_clip@Buffer@@QAEHPAURECT@@@Z 0x005D8000-0x005D81F3 BYTE_EXACT
// symbol    ?set_clip@Buffer@@QAEHPAUtagRECT@@@Z
// size      499 bytes
// prototype int (__thiscall ?set_clip@Buffer@@QAEHPAURECT@@@Z)(Buffer* this, RECT* rect)
// callers   114   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x005D8063 0x005D8099 0x005D80BE 0x005D80C9 0x005D80E1 0x005D80EF 0x005D8100 0x005D8120 0x005D815E 0x005D8169 0x005D81D7 0x005D81E4
Return Value: No errors (0); empty intersection or region failure (1);
              null rectangle (3); no pixel storage and no surface (7)
Status: Complete
Verification note: six mutants survive, none of them an untested path.
IntersectRect and EqualRect are pure computations and are fully observed, as
is the region slot's lifecycle - the cleanup branch is driven by a case that
enters holding a region. What cannot be observed is the effect of DeleteObject
and SelectClipRgn on GDI objects a fixture cannot inspect, and the
CreateRectRgnIndirect failure path, which a real GDI call will not enter. The
`acquired` flag's assignment is order-independent against the acquire beside
it, and the trailing rectangle array's extent is a declaration size the
dispatch reads back by fixed width. The clipper and surface dispatches, the
RGNDATA contents, and the reference count are all observed.
*/
int Buffer::set_clip(RECT *rect) {
    if (!dib_bits_ && surface_ == 0) {
        return 7;
    }
    if (!rect) {
        return 3;
    }
    RECT requested = *rect;
    if (!IntersectRect(&rect1_, &rect2_, &requested)) {
        return 1;
    }

    // The image inlines the whole `get_hdc` / `release_hdc` protocol here -
    // the marker above says `calls (none)` - and both bodies are in-class in
    // `buffer.h` so that VC6 does the same. Defined in the .cpp they are
    // ordinary calls: `/O2` implies `/Ob2` and it has seen them, but it
    // declines on size.
    // `int`, not `bool`: the image tests it with `cmp dword ptr [esp+0x54],
    // ebp`. A `bool` is a byte and compiles `mov al, [esp+0x54]; test al, al`.
    int acquired;
    if (hdc2_ == nullptr) {
        acquired = 1;
        get_hdc();
    } else {
        acquired = 0;
    }
    if (hdc2_ != nullptr) {
        if (clip_region_ != nullptr) {
            DeleteObject(clip_region_);
            clip_region_ = nullptr;
        }
        // TWO CALLS, NOT ONE WITH A VARIABLE. A clip equal to the full
        // extent needs no region at all - null restores the unclipped state -
        // and the image reaches SelectClipRgn from both arms by jumping into
        // the other's argument setup: `push ebp; push eax; jmp 0x5d8120`.
        // That is a tail merge of two calls, which VC6 will do for itself;
        // written as one call on a variable it emits a test and a single
        // push instead, and the whole tail moves.
        if (EqualRect(&rect1_, &rect2_)) {
            SelectClipRgn(hdc2_, nullptr);
        } else {
            HRGN const region = CreateRectRgnIndirect(&rect1_);
            clip_region_ = region;
            if (region == nullptr) {
                // The legacy body returns here without releasing the context
                // it may have just acquired; preserved deliberately.
                return 1;
            }
            SelectClipRgn(hdc2_, region);
        }
    }
    if (acquired) {
        release_hdc(1);
    }

    if (surface_ != 0) {
        // A single-rectangle clip list: the header's bound and the one entry
        // in the rectangle array are both the clipped rectangle.
        //
        // A REAL OBJECT, not a byte array with a pointer cast over it.
        // `RGNDATA` ends in `char Buffer[1]`, so the one-rectangle form is
        // its own type - the same shape as `Dib` above, and for the same
        // reason. `sizeof(RGNDATAHEADER) + sizeof(RECT)` is 0x30, and the
        // original builds exactly that on the stack: the block starts at
        // `esp + 0x24` and its last store is `esp + 0x50`.
        struct ClipList {
            RGNDATAHEADER rdh;
            RECT rects[1];
        } clip;
        clip.rdh.dwSize = sizeof(RGNDATAHEADER);
        clip.rdh.iType = RDH_RECTANGLES;
        clip.rdh.nCount = 1;
        clip.rdh.nRgnSize = sizeof(RECT);
        clip.rdh.rcBound = rect1_;
        clip.rects[0] = rect1_;

        clipper_->SetClipList(reinterpret_cast<LPRGNDATA>(&clip), 0);
        surface_->SetClipper(clipper_);
    }
    return 0;
}

/*
Purpose: Set the clip rectangle from a position and an extent.
// ORIGINAL: 0x005D8200 ?set_clip@Buffer@@QAEHHHHH@Z 0x005D8200-0x005D823D BYTE_EXACT
// size      61 bytes
// prototype int (__thiscall ?set_clip@Buffer@@QAEHHHHH@Z)(Buffer* this, int xLeft, int yTop, int length, int width)
// callers   2   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005D8000
// indirect  0x005D8224
Return Value: whatever the RECT overload returns
Status: Complete

Promoted from src/recovered/005d8200.cpp, which is deleted. That unit reached
`SetRect` through its IAT slot at 0x00669274 - the same indirection the
image uses - because
a verification scaffold declares no headers; here it is the import by name,
and the relocation is masked either way.
*/
int Buffer::set_clip(int left, int top, int width, int height) {
    RECT area;
    SetRect(&area, left, top, left + width, top + height);
    return set_clip(&area);
}


/*
Purpose: Measure `len` bytes of a string, following the buffer's markup.
// ORIGINAL: 0x005DC7C0 ?text_width@Buffer@@QAEHPADH@Z 0x005DC7C0-0x005DCA02
// TRIED: THE HELPER SHAPE, spelled and refused. The image's scan is one
//            helper INLINED TWICE - specialised on font slot 3, the '='
//            test present in one copy (0x5DC82C loop) and absent in the
//            other (0x5DC871) - driving `len` through `&len` and spilling
//            the cursor through [ebp-8]/[ebp-0xc]. Spelled that way here as
//            a static __forceinline taking (LPSTR, int *, int) and called
//            from `if (font_slot_ == 3)`: VC6 12.00.8168 REFUSES the
//            inline in this TU (a real `call` where the image has the
//            copies), every flag set scores 2-15 of 201 and similarity
//            0.031-0.582, all below the committed 0.463. The IDENTICAL
//            shape DOES inline in a minimal TU - free or member caller,
//            /O1 and /O2, with this tree's /FI vc6_compat.h and
//            /DBUILD_DEBUG, with 400 filler functions ahead of it, and
//            with Font::width an in-class method - so the refusal is
//            specific to this TU, not to the shape. Reverted to the
//            committed loop-over-locals form; also confirmed: the
//            `switch` must keep its `static_cast<unsigned char>` - spelled
//            on the plain `char` the image's `movsx` implies, VC6 emits a
//            sub/je compare chain instead of the jump table.
// TRIED: DUPLICATING THE SCAN LOOP IN PLACE, 2026-09-03 - the other way to
//            get two specialised copies without asking VC6 to inline. Hoisted
//            the `font_slot_ == 3` test out of the loop and wrote the loop
//            twice, `=` in the token set of one copy and not the other, so
//            the image's 0x5DC82C/0x5DC871 pair has a literal source. It
//            compiles two loops, and the instruction count moves the right
//            way (149 -> 160 against the image's 201) but under /O2 - the
//            flag set every BYTE_EXACT body in this file is measured on -
//            similarity falls from 0.463 to 0.051. The 0.565 that makes the
//            sweep look like a win is /O1 /G6, a different question. Do not
//            retry this shape: the duplication is right, the surrounding
//            register plan is what the helper refusal was already blocking.
// TRIED: see the "NOT BYTE EXACT" paragraph below - the image drives
//            this scan through `&len` (a spilled frame slot reloaded every
//            iteration) and accumulates width in the dead `text` parameter
//            slot, which is an inlined-helper-over-pointers shape a
//            straight loop-over-locals translation cannot force VC6 into.
//            122 of 201 mnemonics shared at the last attempt; not
//            re-attempted this pass (578 bytes, deepest function in this
//            file).
// size      578 bytes
// prototype int (__thiscall ?text_width@Buffer@@QAEHPADH@Z)(Buffer* this, int8*, int)
// callers   16   call targets   3
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x006192F0 0x006458F0 0x00645DD0
Return Value: The measured width in pixels
Status: Semantics transcribed from the image; not yet byte exact

PROMOTED FROM src/unrecovered/005dc7c0.cpp, which reached the marker through
`*reinterpret_cast<Font **>(0x009BB484)` - that address is `FontDefault`, an
object this tree already has - and left every caller paying `call [ptr]`
against the image's `call rel32`.

The markup: `{` `[` and `$LINK<` select fonts 1, 2 and 3, `}` and `]` return
to font 0, and a doubled token (`{{`, `}}`, `[[`, `]]`) is an escape that
measures as one character in the current font. `=` opens a span that runs to
the next `>`, which is skipped entirely. Every one of those was read out of
the jump table at 0x5DCA04 and its byte index at 0x5DCA20 rather than
guessed, and the `$LINK<` literal off 0x00696C98.

NOT BYTE EXACT, and the shape says why: the image drives the scan through
`&len` - `lea edx, [ebp+0xc]` at 0x005DC817 - reloading the cursor and the
count from the frame on every iteration, and accumulates the width in the
DEAD `text` parameter slot. That is an inlined helper taking pointers, not a
loop over locals, and a straight transcription cannot make VC6 spill the same
way. 122 of 201 mnemonics shared at the last attempt.
*/
int Buffer::text_width(LPSTR text, int len) {
    if (!font1_) {
        font1_ = FontDefault;
    }
    if (!markup_enabled_) {
        return font1_->width(text, len);
    }

    LPSTR segment = text;
    int width = 0;
    for (;;) {
        // Inside a link - font slot 3 - `=` closes the run as well.
        LPSTR cursor = segment;
        while (len != 0) {
            const char letter = *cursor;
            bool token = letter == '{' || letter == '}' || letter == '[' ||
                         letter == ']' || letter == '$';
            if (font_slot_ == 3 && letter == '=') {
                token = true;
            }
            if (token) {
                break;
            }
            ++cursor;
            --len;
        }

        int next_slot = font_slot_;
        if (len != 0) {
            switch (static_cast<unsigned char>(*cursor)) {
            case '$':
                if (strncmp(cursor, "$LINK<", 6) == 0) {
                    markup_pending_ = 1;
                    next_slot = 3;
                }
                break;
            case '=':
                markup_pending_ = 2;
                break;
            case '{':
                if (cursor[1] == '{') {
                    ++cursor;
                    --len;
                } else {
                    next_slot = 1;
                }
                break;
            case '}':
                if (cursor[1] == '}') {
                    ++cursor;
                    --len;
                } else {
                    next_slot = 0;
                }
                break;
            case '[':
                if (cursor[1] == '[') {
                    ++cursor;
                    --len;
                } else {
                    next_slot = 2;
                }
                break;
            case ']':
                if (cursor[1] == ']') {
                    ++cursor;
                    --len;
                } else {
                    next_slot = 0;
                }
                break;
            default:
                break;
            }
        }

        // The four fonts are consecutive, and the image indexes them:
        // `mov ecx, [edi + eax*4 + 0x52c]`. A slot with no font measures in
        // font1_ rather than not at all.
        Font *const *const fonts = &font1_;
        Font *const chosen = fonts[font_slot_];
        const int span = static_cast<int>(cursor - segment);
        width += chosen ? chosen->width(segment, span)
                        : font1_->width(segment, span);

        if (len == 0) {
            markup_pending_ = 0;
            return width;
        }

        const int pending = markup_pending_;
        if (pending == 1) {
            cursor += 5;          // the rest of `$LINK<`
            len -= 5;
        }
        ++cursor;
        segment = cursor;
        if (pending == 2) {
            LPSTR const close = strchr(cursor, '>');
            next_slot = 0;
            if (close) {
                len -= static_cast<int>(close - cursor) + 1;
                segment = close + 1;
            }
            markup_pending_ = next_slot;
        }
        font_slot_ = next_slot;
        --len;
    }
}

/*
Purpose: Measure a null-terminated string with the buffer's text font.
// ORIGINAL: 0x005DC790 ?text_width@Buffer@@QAEHPAD@Z 0x005DC790-0x005DC7BA BYTE_EXACT
// size      42 bytes
// prototype int (__thiscall ?text_width@Buffer@@QAEHPAD@Z)(Buffer* this, int8*)
// callers   17   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005DC7C0 0x006453E0
Return Value: The measured width, or zero for a null string
Status: Complete with a temporary dependency on the measured overload
*/
int Buffer::text_width(LPSTR text) {
    if (!text) {
        return 0;
    }
    return text_width(text, static_cast<int>(strlen(text)));
}


// Draw an inclusive-endpoint segment inside the half-open active clip.
// The original acquires before rejecting a non-axis-aligned segment and
// releases on every acquired clipping exit. Clipping advances the original
// Bresenham error; starting a new line at the clipped coordinates changes
// the pixels at ties. The four octants share these two major-axis walks,
// with descending lines starting at their left (higher-Y) endpoint.
// The initial pixel precedes the endpoint check: a descending endpoint at
// the excluded bottom edge can advance beyond the segment before that one
// write (5E18B9, 5E18E4, 5E193C). Preserve this original boundary quirk.
// As with the buffer's other pixel operations, the recovered geometry
// contract requires representable coordinate/error arithmetic and addresses.
// ORIGINAL: 0x005E15F0 ?line@Buffer@@QAEXHHHHH@Z 0x005E15F0-0x005E1A7F FILE
// RECOVERY_BINDING: v1 {"evidence": ["5E15FE..5E160A inspect Buffer storage; 5E1629/5E164A call its axis primitives; 5E1A7C returns0x14. The scoped source preserves the original error accumulator, lock lifetime and descending-bottom clipping quirk."], "reason": "The five-integer void drawing member uses Buffer storage and clip fields; its original thiscall entry and vertical/horizontal callees establish the owning class and signature."}
void Buffer::line(int x1, int y1, int x2, int y2, int color) {
    if (dib_bits_ == nullptr && surface_ == nullptr) {
        return;
    }
    if (x1 == x2) {
        vline(x1, y1, y2, color);
        return;
    }
    if (y1 == y2) {
        hline(x1, x2, y1, color);
        return;
    }
    if (x2 < x1) {
        int swap = x1;
        x1 = x2;
        x2 = swap;
        swap = y1;
        y1 = y2;
        y2 = swap;
    }
    if (get_data() == 0) {
        return;
    }

    // Lock may publish a new pitch or alter the clip. Read both afterwards.
    const RECT clip = rect1_;
    const int pitch = static_cast<int>(stride_);
    unsigned char *const pixels = static_cast<unsigned char *>(locked_bits_);
    const unsigned char ink = static_cast<unsigned char>(color);
    const int dx = x2 - x1;
    const int dy = y2 > y1 ? y2 - y1 : y1 - y2;
    const int y_step = y2 > y1 ? 1 : -1;
    int x = x1;
    int y = y1;

    do {
        // These coarse tests retain the original strict comparisons; the
        // entry checks below enforce the exclusive right and bottom edges.
        if (x1 > clip.right || x2 < clip.left ||
            (y_step > 0 ? (y1 > clip.bottom || y2 < clip.top)
                        : (y1 < clip.top || y2 > clip.bottom))) {
            break;
        }
        if (dx >= dy) {
            int error = 2 * dy - dx;
            const int along = 2 * dy;
            const int diagonal = 2 * (dy - dx);
            while (x < clip.left ||
                   (y_step > 0 ? y < clip.top : y >= clip.bottom)) {
                if (error > 0) {
                    error += diagonal;
                    y += y_step;
                } else {
                    error += along;
                }
                ++x;
            }
            if (x >= clip.right ||
                (y_step > 0 ? y >= clip.bottom : y < clip.top)) {
                break;
            }
            unsigned char *pixel = pixels + y * pitch + x;
            *pixel = ink;
            const int last_x = x2 < clip.right ? x2 : clip.right - 1;
            while (x < last_x) {
                if (error > 0) {
                    y += y_step;
                    if (y_step > 0 ? y >= clip.bottom : y < clip.top) {
                        break;
                    }
                    error += diagonal;
                    pixel += y_step * pitch;
                } else {
                    error += along;
                }
                ++x;
                ++pixel;
                *pixel = ink;
            }
        } else {
            int error = 2 * dx - dy;
            const int along = 2 * dx;
            const int diagonal = 2 * (dx - dy);
            while (x < clip.left ||
                   (y_step > 0 ? y < clip.top : y >= clip.bottom)) {
                if (error > 0) {
                    error += diagonal;
                    ++x;
                } else {
                    error += along;
                }
                y += y_step;
            }
            if (x >= clip.right ||
                (y_step > 0 ? y >= clip.bottom : y < clip.top)) {
                break;
            }
            unsigned char *pixel = pixels + y * pitch + x;
            *pixel = ink;
            const int last_y = y_step > 0
                ? (y2 < clip.bottom ? y2 : clip.bottom - 1)
                : (y2 >= clip.top ? y2 : clip.top);
            while (y_step > 0 ? y < last_y : y > last_y) {
                if (error > 0) {
                    ++x;
                    if (x >= clip.right) {
                        break;
                    }
                    error += diagonal;
                    ++pixel;
                } else {
                    error += along;
                }
                y += y_step;
                pixel += y_step * pitch;
                *pixel = ink;
            }
        }
    } while (false);
    free_data(1);
}

namespace {

// RECT edges wrap as raw 32-bit values, exactly as the original's inc/dec
// instructions do; the bit-cast round trip keeps the arithmetic out of
// signed-overflow territory.
uint32_t edge_bits(LONG value) {
    uint32_t bits;
    static_assert(sizeof(bits) == sizeof(value),
                  "Buffer geometry requires 32-bit RECT fields");
    memcpy(&bits, &value, sizeof(bits));
    return bits;
}

int edge_int(uint32_t bits) {
    int value;
    static_assert(sizeof(value) == sizeof(bits),
                  "Buffer geometry requires 32-bit int");
    memcpy(&value, &bits, sizeof(value));
    return value;
}

}  // namespace

/*
Purpose: Fill one horizontal run of pixels, clipped to the buffer's rectangle.
// ORIGINAL: 0x005E1A80 ?hline@Buffer@@QAEXHHHH@Z 0x005E1A80-0x005E1BF0
// TRIED: the second null check on the computed destination address
// (`add eax, edx; test eax, eax; je`, which the image keeps at 0x005E1B83
// even though the sum of a non-null base and offset cannot be null).
// Written as a plain sum through a named `char *const dest`, `== 0`,
// `!dest`, or with the offset hoisted into its own `const int` - four
// spellings, all byte-identical - VC6 always folds the redundant `test`
// into the preceding `add`'s own flags. Best reached is 0.996 similar
// (69/131 agreeing at `/c /O2 /Oi- /Gy /GR- /GX`), one instruction short in
// 131; nothing shy of a barrier keeps the dead test and that would cost
// more than the instruction is worth. Do not re-derive this.
// size      368 bytes
// prototype void (__thiscall ?hline@Buffer@@QAEXHHHH@Z)(Buffer* this, int, int, int, int)
// kind      game
// calls     0x006465F0
Return Value: n/a
Status: Semantics transcribed from the image

PROMOTED FROM src/recovered/units/005e1a80.cpp, which read every field as
`*reinterpret_cast<int *>(&locked_bits_)` and left `Buffer::box` calling both
of these through a pointer-to-member.

The acquire and release in the middle are `get_data` and `free_data`, which
the image INLINES here and which are in-class for that reason.
*/
void Buffer::hline(int x1, int x2, int y, int color) {
    if (dib_bits_ == nullptr && surface_ == nullptr) {
        return;
    }
    if (y < rect1_.top) {
        return;
    }
    if (y >= rect1_.bottom) {
        return;
    }
    if (x1 == x2) {
        return;
    }
    if (x1 > x2) {
        x1 ^= x2;
        x2 ^= x1;
        x1 ^= x2;
    }
    if (x1 >= rect1_.right) {
        return;
    }
    if (x2 < rect1_.left) {
        return;
    }
    if (x1 < rect1_.left) {
        x1 = rect1_.left;
    }
    if (x2 >= rect1_.right) {
        x2 = rect1_.right - 1;
    }
    // The DIB's own extent, and the height is NEGATIVE - a top-down DIB - so
    // the bound is its negation.
    if (x1 >= dib_.bmiHeader.biWidth) {
        return;
    }
    if (y >= -dib_.bmiHeader.biHeight) {
        return;
    }
    if (get_data() == 0) {
        return;
    }
    // TWO NULL CHECKS, NOT ONE. The image tests the acquired pointer at
    // 0x005E1B6F and tests the COMPUTED ADDRESS again at 0x005E1B83 - a
    // second `test eax, eax; je`, four bytes, which is exactly what this
    // body was short by. The sum of a non-null base and an offset cannot be
    // null, so the check is unreachable; it is in the shipped bytes.
    // TRIED: the check IS here and VC6 still folds it away. The image
    // emits `add eax, edx; test eax, eax; je`, keeping a `test` that the add's
    // own flags already answer; ours emits `add eax, edx; je`. One instruction
    // in 131, and the body is otherwise identical - 0.996 similar, the only
    // other difference being edx against ecx, which is allocation.
    //
    // Four spellings measured, all byte-identical to each other: the sum in two
    // steps through a named base, `== 0`, `!dest`, and the offset hoisted into
    // its own `const int`. VC6 folds the comparison into the add's flags every
    // time, because it is a dead null test on a sum it can see is non-null.
    // Nothing short of a barrier would keep it, and that would cost more than
    // the instruction is worth. Do not re-derive this.
    // THE BASE IS ADDED LAST - `imul eax, ebp; add eax, ebx; add eax, edx` -
    // so the offsets sum first and the pointer joins at the end.
    char *const dest = stride_ * y + x1 + static_cast<char *>(locked_bits_);
    if (dest == nullptr) {
        return;
    }
    memset(dest, color, x2 - x1 + 1);
    free_data(1);
}

/*
Purpose: Fill one vertical run of pixels, clipped to the buffer's rectangle.
// HAND-WRITTEN ASSEMBLY IN THE ORIGINAL, so byte-exactness is NOT reachable
// from C++ and the honest ceiling here is semantic equivalence. The shipped
// bytes use the `loop` instruction, which VC6 never emits - it counts down
// with `dec`/`jne` - and it writes the colour through `ah`, which is
// a hand-assembly idiom rather than a compiler's. The earlier opcode census
// found seven such bodies in the game code; that count is historical.
//
// Do NOT grind spellings here, and do NOT write `__asm`: a semantic C++ body
// is worth more than a byte-exact assembly one. `Buffer::vline` reached 0.925
// similar before this was noticed.
// ORIGINAL: 0x005E1BF0 ?vline@Buffer@@QAEXHHHH@Z 0x005E1BF0-0x005E1D68
// size      376 bytes
// prototype void (__thiscall ?vline@Buffer@@QAEXHHHH@Z)(Buffer* this, int, int, int, int)
// kind      game
// calls     (none)
// TRIED: byte-exact is unreachable (hand-written `loop`/`ah`-write,
//   above); best measured is /O2 /Gy /GR- /Oy- /GX at 14/145 instructions,
//   0.925 similar - still MISMATCH, and this is the ceiling.
Return Value: n/a
Status: Semantics transcribed from the image

PROMOTED FROM src/recovered/units/005e1bf0.cpp. Its transcription declared
the surface descriptor as `unsigned char buf[0x28]` and then wrote 0x6c into
its size field - a forty-byte buffer told DirectDraw it was a hundred and
eight. `get_data` has it right.

THE ROW WALK IS A DO/WHILE: the image writes one byte, adds the pitch, and
tests the counter at the bottom, so a run of zero rows cannot happen - the
`y1 == y2` guard above is what makes that safe.
*/
void Buffer::vline(int x, int y1, int y2, int color) {
    if (dib_bits_ == nullptr && surface_ == nullptr) {
        return;
    }
    if (x < rect1_.left) {
        return;
    }
    if (x >= rect1_.right) {
        return;
    }
    if (y1 == y2) {
        return;
    }
    if (y1 > y2) {
        y1 ^= y2;
        y2 ^= y1;
        y1 ^= y2;
    }
    if (y1 >= rect1_.bottom) {
        return;
    }
    if (y2 < rect1_.top) {
        return;
    }
    if (y1 < rect1_.top) {
        y1 = rect1_.top;
    }
    if (y2 >= rect1_.bottom) {
        y2 = rect1_.bottom - 1;
    }
    if (x >= dib_.bmiHeader.biWidth) {
        return;
    }
    if (y1 >= -dib_.bmiHeader.biHeight) {
        return;
    }
    if (get_data() == 0) {
        return;
    }
    const int pitch = static_cast<int>(stride_);
    char *pixel = static_cast<char *>(locked_bits_) + y1 * pitch + x;
    // The same unreachable check `hline` carries, at 0x005E1CF7.
    if (pixel == nullptr) {
        return;
    }
    int rows = y2 - y1 + 1;
    do {
        *pixel = static_cast<char>(color);
        pixel += pitch;
    } while (--rows);
    free_data(1);
}


/*
Purpose: Outline a rectangle as a two-color bevel: the top and left edges in
         the first color, the bottom and right edges in the second, with the
         corner ownership split the way the emboss look requires - the top
         edge spans [left+1, right-1], the bottom [left, right-2] one row up
         from the bottom, the left column [top, bottom-2], and the right
         column [top+1, bottom-1] one column in from the right.
// ORIGINAL: 0x005E3203 ?box@Buffer@@QAEHPAURECT@@HH@Z 0x005E3203-0x005E3280 BYTE_EXACT
// symbol    ?box@Buffer@@QAEHPAUtagRECT@@HH@Z
// size      125 bytes
// prototype int (__thiscall ?box@Buffer@@QAEHPAURECT@@HH@Z)(Buffer* this, RECT*, int, int)
// callers   25   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005E1A80 0x005E1BF0
Return Value: No errors (0); null rectangle (3)
Status: Complete with temporary hline/vline dependencies

All four fields are read before the first line is drawn (0x005E321C through
0x005E3227 load top, bottom, right and left, with left stashed into the
incoming rectangle's own argument slot at 0x005E322B), so a callee that
rewrites the rectangle cannot move the remaining edges. The edge arithmetic
is the original's raw inc/dec - 32-bit wrapping, no ordering or emptiness
checks - and a reversed or degenerate rectangle is handed to the primitives
as-is. The primitives' EAX residues are discarded: the original zeroes EAX
after the last call.
*/
int Buffer::box(RECT *rect, int color1, int color2) {
    if (!rect) {
        return 3;
    }
    // PLAIN INT ARITHMETIC. The image reads the four edges into registers and
    // walks them with `dec`/`inc`; the `edge_bits`/`edge_int` round trip is a
    // pair of calls VC6 declines to inline, and there are eight of them here.
    // IMAGE ORDER: top, bottom, right, left - `mov ebp,[eax+4]`,
    // `mov esi,[eax+0xc]`, `mov edi,[eax+8]`, `mov eax,[eax]` - and the two
    // decrements come after both are loaded.
    const int top = rect->top;
    const int bottom_in = rect->bottom - 1;
    const int right_in = rect->right - 1;
    const int left = rect->left;
    hline(left + 1, right_in, top, color1);
    hline(left, right_in - 1, bottom_in, color2);
    vline(left, top, bottom_in - 1, color1);
    vline(right_in, top + 1, bottom_in, color2);
    return 0;
}


/*
Purpose: Reset the buffer's link table - reinitialise the spot list to 40
         entries, clear the count, and free the twenty owned link pointers,
         each through the executable's CRT boundary.
// ORIGINAL: 0x005DEF90 ?clear_links@Buffer@@QAEXXZ 0x005DEF90-0x005DEFD4 BYTE_EXACT
// size      68 bytes
// prototype void (__thiscall ?clear_links@Buffer@@QAEXXZ)(Buffer* this)
// callers   5   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005FA8A0 0x00644EF2
Return Value: n/a
Status: Complete
*/
void Buffer::clear_links() {
    spot_.init(0x28);
    link_count_ = 0;
    for (size_t index = 0; index < OwnedAllocationCount; ++index) {
        if (owned_[index] != nullptr) {
            free(owned_[index]);
            owned_[index] = nullptr;
        }
    }
}


/*
Purpose: Draw a string, switching fonts on markup and recording link regions.
// ORIGINAL: 0x005DCAE0 ?write_multi_font_raw_l@Buffer@@QAEHPADHHH@Z 0x005DCAE0-0x005DCE23
// TRIED: 3/288 - one extra local dword (`sub esp, 0xc` here against
//            the image's `sub esp, 8`) throws off the whole frame from
//            instruction 2 on. `text_width`'s own twin (same file) has an
//            identical shape and an identical plateau; both are the
//            deepest functions in this file (288 and 201 image
//            instructions) and neither was re-attempted this pass -
//            already explicitly marked "not yet byte exact" above with
//            its own scope note.
// size      835 bytes
// prototype int (__thiscall ?write_multi_font_raw_l@Buffer@@QAEHPADHHH@Z)(Buffer* this, int8*, int, int, int)
// callers   9   call targets   6
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005DBD00 0x006458F0 0x00645DD0 0x005D4510
Return Value: The x coordinate one past the last glyph drawn
Status: Semantics transcribed from the image; not yet byte exact

PROMOTED FROM src/unrecovered/005dcae0.cpp, which reached every field through
`markup_pending_` and left four call sites paying
`call [ptr]` against the image's `call rel32`.

THE MEASURER'S TWIN. `text_width` walks the same markup and the same four
fonts; this one draws it, and additionally OWNS THE LINK TABLE: `$LINK<`
opens a region, `=` captures its label into `owned_[link_count_ - 1]`, and each
completed run is handed to `spot_` as a hit rectangle whose height comes from
the link font - `unk_1_` when negative, `height_ + unk_1_` when not, which is
the same reading `Win::flip` needs for its bubble.

Twenty links is the ceiling, and the image says so out loud: a twenty-first
raises `MessageBoxA(0, "Maximum hypertext links exceeded", "WARNING!", 0)`
and then carries on drawing.
*/
int Buffer::write_multi_font_raw_l(LPSTR text, int x_coord, int y_coord,
                                   int len) {
    if (!markup_enabled_) {
        return write_raw_l(text, x_coord, y_coord, len);
    }

    for (;;) {
        LPSTR stop = text;
        while (len != 0) {
            const char letter = *stop;
            bool token = letter == '{' || letter == '}' || letter == '[' ||
                         letter == ']' || letter == '$';
            if (write_font_slot_ == 3 && letter == '=') {
                token = true;
            }
            if (token) {
                break;
            }
            ++stop;
            --len;
        }

        if (len == 0) {
            const int opened_at = x_coord;
            const int reached = write_raw_l(text, x_coord, y_coord,
                                            static_cast<int>(stop - text));
            // THE LINK'S HIT REGION, recorded only while a `$LINK<` run is open.
            // The height is the link font's, and `font4_` falling back to
            // `font1_` is the image's own guard - `unk_1_` when negative,
            // `height_ + unk_1_` when not.
            if (write_font_slot_ == 3 && link_count_ < 20) {
                Font *const link_font = font4_ ? font4_ : font1_;
                if (link_font) {
                        const int height = link_font->unk_1_ < 0
                            ? link_font->line_height_
                            : link_font->height_ + link_font->unk_1_;
                        spot_.add(link_count_ - 1, 0, opened_at, y_coord,
                                      reached - opened_at, height);
                }
            }
            return reached;
        }

        int next_slot;
        const char letter = *stop;
        if (letter == '$' && strncmp(stop, "$LINK<", 6) == 0) {
            markup_pending_ = 1;
            next_slot = 3;
            if (++link_count_ > 20) {
                MessageBoxA(nullptr, "Maximum hypertext links exceeded",
                            "WARNING!", 0);
            }
        } else if (letter == '=') {
            markup_pending_ = 2;
            next_slot = write_font_slot_;
        } else if (letter == '{' && stop[1] != '{') {
            next_slot = 1;
        } else if (letter == '}' && stop[1] != '}') {
            next_slot = 0;
        } else if (letter == '[' && stop[1] != '[') {
            next_slot = 2;
        } else if (letter == ']' && stop[1] != ']') {
            next_slot = 0;
        } else {
            // A doubled token is an escape: one of the pair is drawn.
            if (letter == '{' || letter == '}' || letter == '[' ||
                letter == ']') {
                ++stop;
                --len;
            }
            next_slot = write_font_slot_;
        }

        const int opened_at = x_coord;
        x_coord = write_raw_l(text, x_coord, y_coord,
                              static_cast<int>(stop - text));

        const int pending = markup_pending_;
        if (pending == 1) {
            stop += 5;          // the rest of `$LINK<`
            len -= 5;
        }
        ++stop;
        text = stop;

        if (pending == 2) {
            LPSTR const close = strchr(stop, '>');
            if (close) {
                // The label between `=` and `>` is copied out and owned.
                if (link_count_ < 20) {
                    const int label = static_cast<int>(close - text);
                    char *const kept = static_cast<char *>(mem_get(label + 1));
                    owned_[link_count_ - 1] = kept;
                    if (kept) {
                        memcpy(kept, text, label);
                        kept[label] = '\0';
                    }
                }
                len += static_cast<int>(text - close) - 1;
                text = close + 1;
            }
            next_slot = 0;
            markup_pending_ = 0;
        }

        // THE LINK'S HIT REGION, recorded only while a `$LINK<` run is open.
        // The height is the link font's, and `font4_` falling back to
        // `font1_` is the image's own guard - `unk_1_` when negative,
        // `height_ + unk_1_` when not.
        if (write_font_slot_ == 3 && link_count_ < 20) {
            Font *const link_font = font4_ ? font4_ : font1_;
            if (link_font) {
                const int height = link_font->unk_1_ < 0
                    ? link_font->line_height_
                    : link_font->height_ + link_font->unk_1_;
                spot_.add(link_count_ - 1, 0, opened_at, y_coord,
                          x_coord - opened_at, height);
            }
        }
        write_font_slot_ = next_slot;
        --len;
    }
}

/*
Purpose: Draw at most `len` characters of a string at an explicit pen
         position, clamping the count to the string's own length first.
// ORIGINAL: 0x005DCEA0 ?write_l@Buffer@@QAEHPADHHH@Z 0x005DCEA0-0x005DCF34
// LEVER: re-wrote the min(strlen(text), len) TERNARY OUT TWICE in source
//        (once for the `< 0` guard, once for the final argument) instead of
//        hoisting it into one local - `call_diff` showed FEWER strlen calls
//        than the image's four, matching Font::width's own documented
//        macro-reevaluation lever. 0/64 -> 22/64 (0.850 similar).
// TRIED: remaining 22/64 plateau is a register role swap (esi/edi both
//            hold `text`, assigned to the opposite register from the
//            image's) starting at instruction 2, before either ternary is
//            reached - a VC6 allocation choice at the very top of the
//            function, not something the macro-reevaluation fix touches.
// size      148 bytes
// prototype int (__thiscall ?write_l@Buffer@@QAEHPADHHH@Z)(Buffer* this, int8*, int, int, int)
// callers   29   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005DCAE0 0x006453E0
Return Value: The raster writer's result; the incoming x for a null string or
              an empty draw; unusable font (3, `mov eax, 3` at 0x005DCF2B)
Status: Complete with a temporary raster-writer dependency

The null-string and empty-draw exits return the incoming x rather than a
status code, which is what makes this family chainable: the caller advances
its pen by the return value, so a draw that emits nothing leaves the pen where
it was. The font guard is the only path here that yields a status code.

The original evaluates min(strlen(text), len) twice - once for the `< 0` test
at 0x005DCEE8 and once for the value it forwards - because `min` was a macro;
it calls strlen up to four times for the same reason (each `min` call
re-evaluates `strlen(text)` a second time on whichever branch takes it, since
the macro's "then" arm re-runs the expression it just compared). strlen is
pure and `len` is untouched between them, so every evaluation is the same
value - this body writes the macro's expansion out literally rather than
hoisting it, to match the image's own repeated calls. The comparison is
signed (`cmp eax, esi` / `jge`), so a negative `len` wins the min and takes
the early exit.
*/
int Buffer::write_l(LPSTR text, int x_coord, int y_coord, int len) {
    if (!text) {
        return x_coord;
    }
    if (!font1_ || !font1_->is_initialized()) {
        return 3;
    }
    if ((static_cast<int>(strlen(text)) < len ? static_cast<int>(strlen(text)) : len) < 0) {
        return x_coord;
    }
    return write_multi_font_raw_l(
        text, x_coord, y_coord,
        static_cast<int>(strlen(text)) < len ? static_cast<int>(strlen(text)) : len);
}


/*
Purpose: Draw at most `len` characters of a string flush against a
         rectangle's left edge and vertically centred on the text font.
// ORIGINAL: 0x005DCF40 ?write_l@Buffer@@QAEHPADPAURECT@@H@Z 0x005DCF40-0x005DD016
// LEVER: a real BUG in a prior pass's REASONING, not the image: the note
//        below used to argue the font1_ re-test/rebind at 0x005DCFCD is
//        unreachable (the earlier null-font guard already returned), and
//        left it untranscribed on that theory - but the image still emits
//        it regardless. Adding `if (!font1_) { font1_ = FontDefault; }`
//        back moved 0/80 -> 4/80 (68 -> 73 instructions, matching shape
//        much closer).
// TRIED: remaining gap is scheduling - the image reads all four RECT
//            fields (including the dead `rect->right`) BEFORE the font1_
//            recheck; this body's recheck (textually before the final
//            `rect->left` use) gets scheduled first instead. Not chased
//            further at this budget.
// symbol    ?write_l@Buffer@@QAEHPADPAUtagRECT@@H@Z
// size      214 bytes
// prototype int (__thiscall ?write_l@Buffer@@QAEHPADPAURECT@@H@Z)(Buffer* this, int8*, RECT*, int)
// callers   35   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005DCAE0 0x006453E0
Return Value: The raster writer's result; zero for every rejected input
Status: Complete with a temporary raster-writer dependency

Unlike its three siblings this body has a single failure exit - `xor eax, eax`
at 0x005DD00D - so a null string, a null rectangle, an unusable font and an
empty draw all return zero. It never returns the status code 3 and it never
returns the pen position. The rectangle overload of write_cent_l deliberately
does not share that convention: its font guard returns 3. That disagreement
between the two rectangle overloads is why these four bodies are transcribed
separately rather than sharing a guard helper.

The vertical centre is `top + (bottom - height - top) / 2`, a signed halving
that truncates toward zero (`cdq` / `sub eax, edx` / `sar eax, 1` at
0x005DCFEB), computed on raw 32-bit values with no ordering or emptiness
check, so a reversed or degenerate rectangle is centred as-is and the
arithmetic wraps rather than saturating. The horizontal position is
rect->left verbatim - nothing is measured, and text_width is never called.

CORRECTED. This used to say the re-test/rebind at 0x005DCFCD is
unreachable - the guard at 0x005DCF69 already returned zero for a null
font1_, and only pure strlen calls run in between - and left it
untranscribed on that theory. The image still emits it (`test edx,edx /
jne` then a real rebind from the process default at 0x009BB484), so it is
reproduced here regardless of provable unreachability: mechanical
transcription, not a dead branch this recovery gets to prune.

Verification note: the original loads rect->right at 0x005DCFB8 and spills it
at 0x005DCFC6 without ever reading it back; the dead store is not transcribed.
*/
int Buffer::write_l(LPSTR text, RECT *rect, int len) {
    if (!text) {
        return 0;
    }
    if (!rect) {
        return 0;
    }
    if (!font1_ || !font1_->is_initialized()) {
        return 0;
    }
    // FOUR STRLEN CALLS - the image's `MIN` macro, used twice, each use
    // evaluating its argument twice - same shape as write_cent_l/
    // write_cent_l(rect) below: `< 0` and `== 0` as two separate guards
    // sharing this body's one epilogue.
    int limit;
    if (BUFFER_MIN(static_cast<int>(strlen(text)), len) < 0 ||
        (limit = BUFFER_MIN(static_cast<int>(strlen(text)), len)) == 0) {
        return 0;
    }
    // LEVER: the image's call list for this body is just
    // write_multi_font_raw_l + strlen - no edge_bits/edge_int round trip
    // (image never calls anything for the vertical-centre math, it is plain
    // wrapping int arithmetic). Using rect->left/top/bottom and
    // font1_->height_ directly, without the memcpy bit-cast, drops the two
    // extra calls the image does not make.
    if (!font1_) {
        font1_ = FontDefault;
    }
    const int y_span = (rect->bottom - font1_->height_) - rect->top;
    const int y_coord = rect->top + y_span / 2;
    return write_multi_font_raw_l(text, rect->left, y_coord, limit);
}


/*
Purpose: Draw at most `len` characters of a string horizontally centred in a
         span of `width` pixels starting at an explicit pen position.
// ORIGINAL: 0x005DD020 ?write_cent_l@Buffer@@QAEHPADHHHH@Z 0x005DD020-0x005DD0D2
// size      178 bytes
// prototype int (__thiscall ?write_cent_l@Buffer@@QAEHPADHHHH@Z)(Buffer* this, int8*, int, int, int, int)
// callers   30   call targets   3
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005DC7C0 0x005DCAE0 0x006453E0
// LEVER: PIN THE SHARED EXIT WITH A GOTO. The merged `||` guard that used to
//   be here tail-merged the empty-draw return into the null-text epilogue at
//   0x005DD02D, so our `je` jumped BACKWARDS - 39/76 (0.932). Splitting it
//   and pinning a second copy of the block between the tests and `text_width`
//   with a `goto fail:` from the `< 0` guard - the same lever that fixed
//   `write_right_l` - puts our `jl 0x5dd08a` and the fall-through of
//   `test esi,esi; jne 0x5dd094` on the image's own second epilogue:
//   39/76 (0.932) -> 55/76 (0.980).
// TRIED: the last 21 instructions, and the two shapes left are both worse.
//   Computing the centring INLINE in the call argument,
//   `x_coord + (width - drawn) / 2` instead of through the `centred` local,
//   scores 50/76 - the opposite of `write_right_l`, where hoisting the
//   offset into a local was the win. The residue is interleaving the image
//   has and we do not (`push y` at 0x5DD058, before the `sar`, with `this`
//   held back to 0x5DD063 where we hoist `mov ecx,ebx` ten instructions
//   early) plus one instruction the image pays and we do not: 76 against 75.
// TRIED: the two-locals spelling of that residue, `const int offset =
//   (width - drawn) / 2;` then `x_coord + offset` in the call, and the same
//   with the add swapped (`offset + x_coord`): both 55/76, byte-identical
//   to the committed `centred` local - the image's `mov edx,eax` copy
//   before the `sar` and its late `mov eax,x` load are a register plan the
//   sibling `write_right_l` also could not reach (its note above). And
//   accumulating into the parameter, `x_coord += (width - drawn) / 2`, is
//   50/76. Committed shape stands.
Return Value: The raster writer's result; the incoming x for a null string or
              an empty draw; unusable font (3)
Status: Complete with temporary raster-writer and text-width dependencies

The centring offset is measured from the CLAMPED count: 0x005DD094 pushes the
same esi that reaches the raster writer, so a draw truncated by `len` is
centred on the truncated text. The rectangle overload below does the opposite
and measures the whole string with a fresh strlen at 0x005DD1C4. That
asymmetry is in the bytes and is the reason these two bodies are transcribed
separately rather than sharing a centring helper.

The offset is `x + (width - measured_width) / 2`, a signed halving that
truncates toward zero on raw 32-bit values, so a text wider than the span
produces a negative offset and the arithmetic wraps rather than saturating.
Nothing on any path writes to the object.

Verification note: as in write_l, the min is evaluated twice by the original
and once here, and relaxing the min's comparison to `<=` is an equivalent
mutant that will survive.
*/
int Buffer::write_cent_l(LPSTR text, int x_coord, int y_coord, int width,
                         int len) {
    if (!text) {
        return x_coord;
    }
    if (!font1_ || !font1_->is_initialized()) {
        return 3;
    }
    // FOUR STRLEN CALLS - 0x005DD04D, 0x005DD05E, 0x005DD06F, 0x005DD07C -
    // which is a `MIN` macro used TWICE, each use evaluating its argument
    // twice. The two guards are separate as well: `< 0` and `== 0`, jumping
    // to one shared epilogue at 0x005DD08A. Written as one `<= 0` test over
    // a local, this body is two calls and one branch short.
    // ONE RETURN, TWO TESTS, as in WinMain: the image's `jl` at 0x005DD06C
    // and its `jne` at 0x005DD088 both reach the SAME epilogue at
    // 0x005DD08A. Two separate `if (...) return x_coord;` statements make
    // VC6 emit that epilogue twice; a short-circuit `||` gives it one.
    int limit;
    // ONE EXIT BLOCK, PINNED BETWEEN THE TEST AND THE WORK. The image's
    // `jl 0x5dd08a` (the negative guard) and the fall-through of
    // `test esi,esi; jne 0x5dd094` (the zero guard) both land on the SAME
    // epilogue at 0x5dd08a, which sits AFTER both tests and BEFORE
    // text_width. Written as one short-circuit `||` VC6 folds that block
    // into the null-text epilogue at 0x5dd02d and jumps BACKWARDS to it
    // (39/76); this `goto` holds a second copy of it where the image has it
    // - the same lever that fixed `write_right_l`.
    if (BUFFER_MIN(static_cast<int>(strlen(text)), len) < 0) {
        goto fail;
    }
    limit = BUFFER_MIN(static_cast<int>(strlen(text)), len);
    if (limit == 0) {
    fail:
        return x_coord;
    }
    const int drawn = text_width(text, limit);
    // PLAIN INT ARITHMETIC, because that is what the image emits: `sub`,
    // then `cdq; sub eax, edx; sar edx, 1` - a signed halving that truncates
    // toward zero - then `add`. The `edge_bits`/`edge_int` round trip is a
    // pair of calls VC6 declines to inline, and five of them were the whole
    // difference in this tail.
    const int centred = x_coord + (width - drawn) / 2;
    return write_multi_font_raw_l(text, centred, y_coord, limit);
}


/*
Purpose: Draw at most `len` characters of a string centred both horizontally
         and vertically inside a rectangle.
// ORIGINAL: 0x005DD130 ?write_cent_l@Buffer@@QAEHPADPAURECT@@H@Z 0x005DD130-0x005DD24A
// TRIED: close in shape and count (109 compiled against the image's
//            106) but low `agreeing` - the remainder is a consistent
//            ebx/ebp/edi/ecx register permutation through the whole body,
//            not a structural gap. Consistent with the sibling
//            `write_cent_l(int,int,int,int)`'s own documented note that a
//            two-guard split scores WORSE here (34/76 against 39/76); not
//            re-tried. No further source-form lever found at this budget.
// TRIED (2026-08-29): the image allocates this=esi, text=edi, left=ebx,
//            len=ebp and spills top/right/bottom to 0x14/0x18/0x1c; ours
//            allocates text=ebx, left=ebp, right=edi and interleaves the
//            text load with the callee-saved pushes (divergence at
//            instruction 2). Dropping the four `const int` edge locals and
//            reading the `area.` copy fields at their uses - making the
//            frame slots the only storage, as the image's spills suggest -
//            measured IDENTICALLY 9/106: VC6 promotes the copy's fields to
//            pseudo-registers either way, so the allocation is cost-model
//            internal and did not yield to the source shape.
// symbol    ?write_cent_l@Buffer@@QAEHPADPAUtagRECT@@H@Z
// size      282 bytes
// prototype int (__thiscall ?write_cent_l@Buffer@@QAEHPADPAURECT@@H@Z)(Buffer* this, int8*, RECT*, int)
// callers   108   call targets   3
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005DC7C0 0x005DCAE0 0x006453E0
Return Value: The raster writer's result; unusable font (3); zero for a null
              string, a null rectangle or an empty draw
Status: Complete with temporary raster-writer and text-width dependencies

Three things separate this body from its three siblings, and none of them
survives being folded into a shared helper:

  - It returns 3 on the font guard (0x005DD232) but zero on every other
    rejection (0x005DD241), where the other rectangle overload returns zero
    uniformly.
  - It measures the WHOLE string: text_width is handed a fresh strlen from
    0x005DD1C4, not the clamped count the raster writer receives, so a draw
    truncated by `len` is still centred as if it were complete.
  - It re-reads font1_ after the text-width call and rebinds it from the
    process default at 0x009BB484 when it comes back null. Here that rebind is
    reachable, because the measured overload performs the identical rebind of
    its own at 0x005DC7D3 and can leave the field clear; the guard is not
    re-run, so the substituted font is used unchecked.

All four rectangle fields are loaded up front, before any call is made, so a
text-width callee that rewrites the rectangle cannot move either centre. Both
centres are signed halvings truncating toward zero over raw 32-bit values,
with no ordering or emptiness check, so reversed rectangles centre as-is and
the arithmetic wraps.

Verification note: the min is evaluated twice by the original and once here,
and relaxing the min's comparison to `<=` is an equivalent mutant.
*/
int Buffer::write_cent_l(LPSTR text, RECT *rect, int len) {
    // ONE RETURN FOR BOTH, as in the other overload: the image's two `je`s
    // reach the same epilogue. Two separate `if (...) return 0;` statements
    // make VC6 emit that epilogue twice and grow a frame around it.
    if (!text || !rect) {
        return 0;
    }
    // PLAIN INT ARITHMETIC, as in `box` and the other overload: the
    // `edge_bits`/`edge_int` round trip is a pair of calls VC6 declines to
    // inline, and there are ten of them here. The image reads the four edges
    // into registers in order and halves with `cdq; sub; sar 1`.
    // A COPY OF THE RECTANGLE, which is what the frame says: the image
    // reserves 0x10 bytes - exactly a RECT - and reads the four edges out of
    // the caller's copy into it before doing anything else.
    RECT area = *rect;
    const int left = area.left;
    const int top = area.top;
    const int right = area.right;
    const int bottom = area.bottom;
    if (!font1_ || !font1_->is_initialized()) {
        return 3;
    }
    // FIVE STRLEN CALLS in this body: the `MIN` macro used twice, each
    // evaluating its argument twice, and a fifth for the measurement below.
    // The guards are `< 0` and `== 0` reaching the same epilogue, exactly as
    // in the other overload.
    int limit;
    if (BUFFER_MIN(static_cast<int>(strlen(text)), len) < 0 ||
        (limit = BUFFER_MIN(static_cast<int>(strlen(text)), len)) == 0) {
        return 0;
    }
    // THE WHOLE STRING, not the clamped count: this overload measures with a
    // fresh strlen where the other centres on the truncated text. That
    // asymmetry is in the bytes.
    const int drawn = text_width(text, static_cast<int>(strlen(text)));
    const int x_coord = left + (right - drawn - left) / 2;
    if (!font1_) {
        font1_ = FontDefault;
    }
    const int y_coord = top + (bottom - font1_->height_ - top) / 2;
    return write_multi_font_raw_l(text, x_coord, y_coord, limit);
}


/*
// ORIGINAL: 0x005DA680 ?poly@Buffer@@QAEHPAUVert@@HH@Z 0x005DA680-0x005DA69B BYTE_EXACT
// size      27 bytes
// prototype 
// callers   1   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00626620
Status: Complete
*/
int Buffer::poly(Vert *verts, int count, int color) {
    return polygon(this, verts, count, color);
}

// ORIGINAL: 0x005DD300 ?write_right_l@Buffer@@QAEHPADHHHH@Z 0x005DD300-0x005DD3A5 FILE
// LEVER: X BEFORE Y, the reverse of what the note below used to claim. The
//        image adds `field_width - text_width` to the SECOND parameter and
//        passes that as write_multi_font_raw_l's second argument, and its
//        early exits return the second parameter untouched - and buffer.h's
//        own fact about the raster writer says the scalar writers "return
//        the incoming x". So the second parameter is the x, the third the
//        y, and the alignment offset lands on x. Two decompiled callers
//        (the homed-from archives 005ddbb0 and 004b0410, both deleted
//        with their homings) independently named the third argument `y`,
//        which agrees. Swapping
//        the two names moves the offset from the third to the second stack
//        slot, which is the whole of the semantic difference.
// LEVER: BUFFER_MIN, WHICH EVALUATES BOTH ARGUMENTS. `int w = len; if
//        (strlen(text) < len) w = strlen(text);` holds `w` in a callee-saved
//        register across the call and that is what pushes `this` out to ebp
//        (12/71, `push ebp` against the image's `push ebx`). Spelled
//        `BUFFER_MIN((int)strlen(text), len)` there is no local to keep
//        alive, the second `strlen` lands inside the macro's true arm exactly
//        where the image has it, and `this` gets ebx back.
// TRIED: nesting everything inside a single `if (font1_ && font_obj_)` block with one trailing `return a2` (matching Ghidra's shape more literally) instead of early returns - same MISMATCH #1 'push' vs 'mov' either way; the divergence is in the prologue's register push order (ebx/esi/edi count/order), not the branch structure. similarity ~0.61-0.63.
// TRIED: the last four instructions. The image loads y ([esp+0x18]) into edx
//        and x ([esp+0x14]) into eax and ends `add eax,ecx`; VC6 gives us x
//        in edx and y in eax ending `add ecx,edx`. Measured `offset +
//        x_coord` for `x_coord + offset` (65/71 both, so it is not the
//        operand order) and `int new_x = x_coord + offset` hoisted into a
//        local of its own (64/71, worse). Register plan on the two argument
//        loads, not source form.
// TRIED: confirming that once more with three spellings ranked in one run
//        against the tree's own 65/71: folding `width` away into
//        `field_width - text_width(text, len)` (65/71, byte-identical),
//        `const int new_x = x_coord + offset;` ahead of the return (64/71)
//        and accumulating into the parameter, `x_coord += field_width -
//        width;` then passing `x_coord` (61/71). Nothing reaches the image's
//        `mov edx,[esp+0x18]` / `mov eax,[esp+0x14]` / `add eax,ecx` before
//        the four pushes.
// size      165 bytes
// prototype int (__thiscall ?write_right_l@Buffer@@QAEHPADHHHH@Z)(Buffer* this, int8*, int, int, int, int)
// callers   4   call targets   3
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005DC7C0 0x005DCAE0 0x006453E0
int Buffer::write_right_l(char *text, int x_coord, int y_coord, int field_width, int len) {
    if (text == 0) {
        return x_coord;
    }
    if (font1_ == 0 || font1_->font_obj_ == 0) {
        return 3;
    }

    // THE SHARED EXIT SITS BETWEEN THE TEST AND THE WORK. The image returns
    // x_coord from ONE block at 0x005DD366 that both the negative-length
    // guard (`jl`) and the zero-length guard (fall-through) reach, and the
    // drawing code carries on after it. Spelled as two `return` statements
    // VC6 emits two epilogues and the shared block dissolves (33/71); spelled
    // as one `if (a || b)` it merges the block with the `text == 0` return
    // above and the jumps turn round and point backwards (40/71). This label
    // is what holds the block where the image has it - `fail` is not an
    // error path, it is the ordinary "nothing to draw" exit.
    if (BUFFER_MIN((int)strlen(text), len) < 0) {
        goto fail;
    }
    len = BUFFER_MIN((int)strlen(text), len);
    if (len == 0) {
    fail:
        return x_coord;
    }

    // THE OFFSET IS A LOCAL, not part of the argument expression: hoisting
    // `field_width - width` out of the call is what makes VC6 load the field
    // first and subtract the measured width, in the image's own order.
    int width = text_width(text, len);
    int offset = field_width - width;
    return write_multi_font_raw_l(text, x_coord + offset, y_coord, len);
}

// All buffer blitters borrow the same Sprite descriptor. Its real object
// definition and original construction evidence are below.
extern Sprite g_BUFFER_SPRITE;

// ORIGINAL: 0x005D8290 ?setup_buff_sprite@Buffer@@QAEXH@Z 0x005D8290-0x005D835C FILE BYTE_EXACT
// LEVER: NAMED MEMBERS AND THE REAL INTERFACE. This was `*(int *)((char *)this
//        + 0x50)` for every field and a hand-rolled `((DDUnlockFn)vtbl[0x20])
//        (field58, owner)` for the release - 14/64 (0.571). Written through
//        `locked_bits_`, `dib_bits_`, `stride_`, `dib_.bmiHeader.biWidth`,
//        `-dib_.bmiHeader.biHeight` and `surface_->Unlock(locked_bits_)`
//        exactly as the BYTE_EXACT `get_pixel` spells them: 60/64 (0.986).
// size      204 bytes
// prototype void (__thiscall ?setup_buff_sprite@Buffer@@QAEXH@Z)(Buffer* this, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x005D834B

void Buffer::setup_buff_sprite(int colour) {
    if (surface_ != nullptr) {
        return;
    }
    if (colour == -1) {
        // HONEST READ OF THE WORD. The image reads ONE byte here
        // (`xor ecx,ecx; mov cl, byte ptr [0x696d14]`), the low byte of the
        // colour word - which is how VC6 zero-extends a BYTE-typed object,
        // and WinFillColour is one now.
        colour = WinFillColour;
    }
    // THE BITS ARE PUBLISHED WHATEVER THEY ARE, and `locked_bits_` is stored
    // before the test - the same shape `get_data` and `get_pixel` have.
    LPVOID bits = dib_bits_;
    locked_bits_ = bits;
    if (bits != nullptr) {
        ++surface_lock_count_;
    }
    g_BUFFER_SPRITE.pcBits_ = static_cast<char *>(bits);
    g_BUFFER_SPRITE.iSpriteWidth2_ = stride_;
    g_BUFFER_SPRITE.iSpriteWidth_ = dib_.bmiHeader.biWidth;
    g_BUFFER_SPRITE.iSpriteHeight_ = -dib_.bmiHeader.biHeight;
    g_BUFFER_SPRITE.iWidth_ = dib_.bmiHeader.biWidth;
    g_BUFFER_SPRITE.iHeight_ = -dib_.bmiHeader.biHeight;
    g_BUFFER_SPRITE.iLeftOffset_ = 0;
    g_BUFFER_SPRITE.iTopOffset_ = 0;
    g_BUFFER_SPRITE.cTransparentIndex_ = (uint8_t)colour;
    g_BUFFER_SPRITE.fObj1Exists_ = 0;

    // The same release pair `get_pixel` ends with, in the same two arms.
    if (surface_ == nullptr) {
        const int count = --surface_lock_count_;
        if (count <= 0) {
            locked_bits_ = nullptr;
            surface_lock_count_ = 0;
        }
        return;
    }
    const int count = --surface_lock_count_;
    if (locked_bits_ != nullptr && count <= 0) {
        surface_->Unlock(locked_bits_);
        locked_bits_ = nullptr;
        surface_lock_count_ = 0;
    }
}

/*
Purpose: Release the shared Sprite descriptor's borrowed pixel pointer.
// ORIGINAL: 0x005D8360 sub_5d8360 0x005D8360-0x005D836B BYTE_EXACT
// symbol    ?release_buff_sprite@Buffer@@SAXXZ
// size      11 bytes
// prototype
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void Buffer::release_buff_sprite() {
    g_BUFFER_SPRITE.pcBits_ = 0;
}

// ORIGINAL: 0x005D84C0 ?draw@Buffer@@QAEHPAVBuffer@@HHHHH@Z 0x005D84C0-0x005D8646
// RECOVERY_BINDING: v1 {"evidence": ["Original5D84C0 ends ret18 and forwards the software path to Sprite5E4B4A;5D8370 ends ret10 and is a distinct four-argument overload. The previous six-argument forwarder selected the wrong ABI.", "Original5D84F5 calls source SetColorKey and5D854C destination Blt;5D854F returns the scale denominator. Initializer5D71D0 and cleanup5D71F0 establish the shared Sprite at9B3A50 whose fields the fallback borrows."], "reason": "Recover the six-argument Buffer draw in its owning class using real surfaces and the shared Sprite object; retain the same-span seed."}
// TRIED: none confirmed byte-exact; MISMATCH #2 'push' vs 'mov' - the two indirect vtable-slot calls (0x74 and 0x14 on the sprite pointers) push their receiver as an explicit first stack argument rather than loading it into ecx, so they are modelled as raw __stdcall function-pointer calls here rather than C++ virtual dispatch; the real calling convention on those two slots is unconfirmed.
// symbol    ?draw@Buffer@@QAEHPAV1@HHHHH@Z
// size      390 bytes
// prototype int (__thiscall ?draw@Buffer@@QAEHPAVBuffer@@HHHHH@Z)(Buffer* this, Buffer*, int, int, int, int, int)
// callers   14   call targets   1
// kind      game
// flags     sp_ready;purged_ok
// calls     0x005E4B4A
// indirect  0x005D84F5 0x005D854C 0x005D860A

// The source and destination fields at +0x58 are DirectDraw surfaces, not
// Sprite pointers. Original 0x005D84F5 sets the source color key; 0x005D854C
// blits on the destination. The shared Sprite is only the software fallback.
int Buffer::draw(Buffer *destination, int index, int x, int y,
                 int numerator, int denominator) {
    if (surface_ != 0 && destination->surface_ != 0) {
        DDCOLORKEY key;
        key.dwColorSpaceLowValue = index;
        key.dwColorSpaceHighValue = index;
        surface_->SetColorKey(DDCKEY_SRCBLT, &key);
        RECT rectangle;
        rectangle.left = x;
        rectangle.top = y;
        rectangle.right = x + dib_.bmiHeader.biWidth * numerator / denominator;
        rectangle.bottom = y - dib_.bmiHeader.biHeight * numerator / denominator;
        destination->surface_->Blt(&rectangle, surface_, 0,
                                  DDBLT_WAIT | DDBLT_KEYSRC, 0);
        // The original returns the scale denominator, not either COM result.
        return denominator;
    }
    setup_buff_sprite(index);
    int result = g_BUFFER_SPRITE.draw(destination, index, x, y, numerator, denominator);
    g_BUFFER_SPRITE.pcBits_ = 0;
    return result;
}

// ORIGINAL: 0x005D8930 ?draw_mono@Buffer@@QAEHPAVBuffer@@HHHHHH@Z 0x005D8930-0x005D8A28
// RECOVERY_BINDING: v1 {"evidence": ["Pinned248-byte wrapper receives destination then transparency, not a transparent destination pointer. Existing setup_buff_sprite owns the descriptor preamble; forwards all seven arguments to5E3E00, clears the borrowed pixel pointer and returns its result.", "Existing seed identity and extent retained against the pinned original; catalogue class/return spellings are provenance, not a substitute for emitted symbol facts or fresh measurements."], "reason": "Recover Buffer mono drawing through its actual borrowed Sprite descriptor."}
// symbol    ?draw_mono@Buffer@@QAEHPAV1@HHHHHH@Z
int Buffer::draw_mono(Buffer *destination, int transparent, int x, int y,
                      int colour, int numerator, int denominator) {
    setup_buff_sprite(transparent);
    int result = g_BUFFER_SPRITE.draw_mono(
        destination, transparent, x, y, colour, numerator, denominator);
    release_buff_sprite();
    return result;
}

// ORIGINAL: 0x005D8840 ?draw_mono@Buffer@@QAEHPAVBuffer@@HHHH@Z 0x005D8840-0x005D892E
// symbol    ?draw_mono@Buffer@@QAEHPAV1@HHHH@Z
//           The image spells the leading `Buffer *` as `PAVBuffer@@`; VC6 emits
//           the back-reference `PAV1@` for the same type, so the fact names what
//           this tree actually emits.
// size      238 bytes
// prototype int (__thiscall ?draw_mono@Buffer@@QAEHPAVBuffer@@HHHH@Z)(Buffer* this, Buffer*, int, int, int, int)
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005E3E50
// indirect  0x005D88F7
// provenance (retired seed): src/unrecovered/005d8840.cpp. Homed for COVERAGE,
// no byte claim. The seed inlined the descriptor preamble and lock pair that
// `setup_buff_sprite` owns, exactly as the seven-argument draw_mono above
// spells the same sequence; the callee is Sprite::draw_mono's fixed-size
// member at 0x005E3E50.
int Buffer::draw_mono(Buffer *target, int transparent, int x, int y,
                      int colour) {
    setup_buff_sprite(transparent);
    int result = g_BUFFER_SPRITE.draw_mono(target, transparent, x, y, colour);
    release_buff_sprite();
    return result;
}

// ORIGINAL: 0x005D8370 ?draw@Buffer@@QAEHPAVBuffer@@HHH@Z 0x005D8370-0x005D84B7
// symbol    ?draw@Buffer@@QAEHPAV1@HHH@Z
//           The image spells the leading `Buffer *` as `PAVBuffer@@`; VC6 emits
//           the back-reference `PAV1@` for the same type, so the fact names what
//           this tree actually emits.
// size      327 bytes
// prototype int (__thiscall ?draw@Buffer@@QAEHPAVBuffer@@HHH@Z)(Buffer* this, Buffer*, int, int, int)
// callers   3   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005E4B9A
// indirect  0x005D83B2 0x005D83C7 0x005D8485
// provenance (retired seed): src/unrecovered/005d8370.cpp. Homed for COVERAGE,
// no byte claim. The four-argument member of the draw family: the DirectDraw
// fast path of the landed six-argument `draw` above with the WHOLE surface as
// the rectangle - Blt(NULL, ..., DDBLT_WAIT | DDBLT_KEYSRC, NULL) - returning
// the colour-key argument the same word stored into both DDCOLORKEY fields
// (the image reloads it at 0x005D83CA), as the scaled path returns its own
// denominator; 0x10 for a null destination, and the software path through the
// shared Sprite.
int Buffer::draw(Buffer *destination, int index, int x, int y) {
    if (destination == 0) {
        return 0x10;
    }
    if (surface_ != 0 && destination->surface_ != 0) {
        DDCOLORKEY key;
        key.dwColorSpaceLowValue = index;
        key.dwColorSpaceHighValue = index;
        surface_->SetColorKey(DDCKEY_SRCBLT, &key);
        destination->surface_->Blt(0, surface_, 0, DDBLT_WAIT | DDBLT_KEYSRC, 0);
        return index;
    }
    setup_buff_sprite(index);
    int result = g_BUFFER_SPRITE.draw(destination, index, x, y);
    release_buff_sprite();
    return result;
}

// DirectDraw surface vtable slots 0x19 (Lock) and 0x20 (Unlock), reached the
// way win.cpp reaches its own: an explicit index through the interface's
// vtable. A `VCall` shim would say the same thing in bytes and cost the
// compiler-work ratchet a site for a call that is not a C++ virtual at all.
typedef int(__stdcall *DDLockFn)(void *, void *, void *, int, int);
typedef int(__stdcall *DDUnlockFn)(void *, int);

/*
Purpose: One pixel out of the surface, for the pixel-precise hit test in
         get_mouse_window_recurse. HOMED from src/unrecovered/005e2210.cpp
         on 2026-08-25 because homing that caller made this the last
         undefined symbol in the link; 15 callers tree-wide.
*/
// ORIGINAL: 0x005E2210 ?get_pixel@Buffer@@QAEHHH@Z 0x005E2210-0x005E232E FILE BYTE_EXACT
// LEVER: the pun came out and the members went in - `self + 0x58/0x80/0x84`
//        are `surface_`, `dib_.bmiHeader.biWidth`, `dib_.bmiHeader.biHeight`,
//        and the two hand-rolled vtable walks at slots 0x19/0x20 are
//        `surface_->Lock(...)` / `surface_->Unlock(...)` exactly as the
//        BYTE_EXACT `get_data`/`free_data` pair in buffer.h spells them.
//        11/107 (0.837) -> BYTE_EXACT 107/107.
// LEVER: CHAIN THE FOUR BOUNDS GUARDS. Written as four separate `if`s the
//        image's single shared epilogue becomes SIX separate
//        `pop/xor/add esp/ret 8` blocks and nothing lands in position
//        (11/107); written as one `if (a || b || c || d)` there is one
//        `return 0` for the whole guard set and VC6 merges it with the
//        first guard's into the single epilogue at 0x005E2322 -> 107/107.
//        This is the MIRROR of the "do not chain conditions the image tests
//        separately" rule: here the image DOES share one epilogue, so the
//        conditions have to share one `return`.
// symbol    ?get_pixel@Buffer@@QAEHHH@Z
// size      286 bytes
// prototype int (__thiscall ?get_pixel@Buffer@@QAEHHH@Z)(Buffer* this, int, int)
// callers   15   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
int Buffer::get_pixel(int x, int y) {
    // THE EARLY OUTS SHARE ONE EPILOGUE: the first guard keeps its own
    // `return 0` and the four bounds tests share the second, which is what
    // puts both on the single epilogue at 0x005E2322.
    if (dib_bits_ == nullptr && surface_ == nullptr) {
        return 0;
    }
    if (x < 0 || y < 0 || x >= dib_.bmiHeader.biWidth
        || y >= -dib_.bmiHeader.biHeight) {
        return 0;
    }

    // THE BITS TO READ FROM, whichever of the two storages is live. The
    // no-surface arm stores `locked_bits_` whether or not the bits exist -
    // the image emits that store before its `je` - exactly as `get_data` does.
    LPVOID bits;
    if (surface_ == nullptr) {
        locked_bits_ = dib_bits_;
        bits = dib_bits_;
        if (dib_bits_ != nullptr) {
            ++surface_lock_count_;
        }
    } else if (locked_bits_ != nullptr) {
        ++surface_lock_count_;
        bits = locked_bits_;
    } else {
        // UNINITIALISED APART FROM ITS SIZE, as `get_data`: `Lock` fills it.
        DDSURFACEDESC description;
        description.dwSize = sizeof(description);
        if (surface_->Lock(nullptr, &description, DDLOCK_WAIT, nullptr) != 0) {
            bits = nullptr;
        } else {
            stride_ = description.lPitch;
            ++surface_lock_count_;
            locked_bits_ = description.lpSurface;
            bits = description.lpSurface;
        }
    }

    unsigned char *pixel;
    if (bits == nullptr) {
        pixel = nullptr;
    } else {
        // Re-read from the member, as the image does: `stride_ * y` first,
        // then the locked base, then x.
        pixel = reinterpret_cast<unsigned char *>(locked_bits_) + stride_ * y + x;
    }

    if (surface_ == nullptr) {
        const int count = --surface_lock_count_;
        if (count <= 0) {
            locked_bits_ = nullptr;
            surface_lock_count_ = 0;
        }
    } else {
        const int count = --surface_lock_count_;
        if (locked_bits_ != nullptr && count <= 0) {
            surface_->Unlock(locked_bits_);
            locked_bits_ = nullptr;
            surface_lock_count_ = 0;
        }
    }

    if (pixel == nullptr) {
        return 0;
    }
    return *pixel;
}

/*
Purpose: Tile a source buffer across a rectangle of this one, stepping the
         source's own origin backwards by the tiling offset so the seams land
         where the caller asked for them.

         HOMED from src/unrecovered/005da860.cpp, which held this exact
         transcription. The rectangle is built, intersected with the buffer's
         own extent, and the intersection - not the request - is what the
         tiles are clipped to; the source dimensions come out of the DIB
         header, and the vertical step is the NEGATED height because a DIB's
         biHeight is stored negative (see `Buffer::init`).
// ORIGINAL: 0x005DA860 ?tile@Buffer@@QAEHPAVBuffer@@HHHHHH@Z 0x005DA860-0x005DA9EC
// symbol    ?tile@Buffer@@QAEHPAV1@HHHHHH@Z
// size      396 bytes
// prototype int (__thiscall ?tile@Buffer@@QAEHPAVBuffer@@HHHHHH@Z)(Buffer* this, Buffer*, int, int, int, int, int, int)
// callers   1   call targets   3
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005D8000 0x005DFF00
// indirect  0x005DA8E4
// TRIED: straight transcription diverges at #2 (original pushes ebx/ebp/esi/edi as one prologue block; this form only needs three, so the compiler's register save set differs) - 122/144 mnemonics still agree. Landing the closest.
// TRIED: the residual, re-measured in place 2026-09-03 after homing - the
//        image's frame is 0x24 with `this` SPILLED into it and edi holding
//        `this`; this form is 0x28 with `this` in a register and three of the
//        four callee-saved pushes sunk below the null guards. Spelled and
//        measured: `const` vs plain locals; `RECT selfRect` declared before
//        vs after the right/bottom stores (the image's store order is left,
//        top, the selfRect copy, right, bottom); and the whole body nested
//        under `if (dib_bits_ != 0 || surface_ != 0)` so the pushes stop
//        being skippable - box_sprite's own LEVER in this file. All three
//        leave 3/144 with the same first divergence, and 24 flag sets scored
//        /O2 as the best at 0.917 similar. Allocator slot preference, not a
//        source relationship this spelling reaches.
// TRIED: measured post-move 2026-09-18 (recover measure, /O2 /Gy /GR- /Oy-
//        /GX): with copy's body in-class in buffer.h, the plain loop call
//        folds the whole blitter in - 350 instructions against the image's
//        144, similarity 0.29, first divergence at offset 0 ("a different
//        program, not a different allocation"). APPLIED 2026-09-18: the call
//        goes through a pointer-to-member, the same measured lever the two
//        copy overloads above and flic.cpp's six sites use; VC6 devirtualizes
//        it to the image's direct call at 0x005DA9BB. The allocator residual
//        above (frame 0x28 vs 0x24) is unchanged by the lever.
*/
int Buffer::tile(Buffer *source, int src_x, int src_y, int x, int y, int w,
                 int h) {
    if (dib_bits_ == 0 && surface_ == 0) {
        return 7;
    }

    RECT clipRect;
    clipRect.left = x;
    clipRect.top = y;
    RECT selfRect = rect1_;
    int rectRight = w + x;
    clipRect.right = rectRight;
    clipRect.bottom = h + y;

    if (IntersectRect(&clipRect, &clipRect, &selfRect)) {
        set_clip(&clipRect);

        // The DIB's own dimensions, read out of the header the buffer was
        // built with - not fields of `Buffer`, fields of its DIB.
        int tileWidth = source->dib_.bmiHeader.biWidth;
        int tileHeight = -source->dib_.bmiHeader.biHeight;

        if (tileWidth != 0 && tileHeight != 0) {
            int tilesX = w / tileWidth;
            if (tilesX * tileWidth < w) {
                tilesX++;
            }
            int tilesY = h / tileHeight;
            if (tilesY * tileHeight < h) {
                tilesY++;
            }

            int startX = x - src_x % tileWidth;
            int startY = y - src_y % tileHeight;

            if (tilesX * tileWidth + startX < rectRight) {
                tilesX++;
            }
            if (tilesY * tileHeight + startY < h + y) {
                tilesY++;
            }

            // Through a pointer-to-member since the blitter moved in-class
            // into buffer.h (2026-09-17): a plain call here folds the whole
            // 279-instruction body into this 396-byte walker, where the
            // image makes the call at 0x005DA9BB - the same measured lever
            // the two copy overloads above and flic.cpp's sites use.
            int (Buffer::*copy)(Buffer *, int, int, int, int, int, int) =
                &Buffer::copy;
            int rowsLeft = tilesY;
            if (rowsLeft > 0) {
                int rowY = startY;
                do {
                    int colsLeft = tilesX;
                    if (colsLeft > 0) {
                        int colX = startX;
                        do {
                            (source->*copy)(this, 0, 0, colX, rowY, tileWidth,
                                            tileHeight);
                            colsLeft--;
                            colX += tileWidth;
                        } while (colsLeft != 0);
                    }
                    rowY += tileHeight;
                    rowsLeft--;
                } while (rowsLeft != 0);
            }

            set_clip(&selfRect);
        }
    }

    return 0;
}

/*
Purpose: The same tiling pass with a mask colour: every frame element is
         painted twice, once as the sprite and once through the mask, which
         is how a panel's shadow is laid down under its frame.

         HOMED from src/unrecovered/005da6c0.cpp. One argument more than
         `tile(Buffer *)` and one call different - `copy_mask` for `copy` -
         everything else is the same walk.
// ORIGINAL: 0x005DA6C0 ?tile_mask@Buffer@@QAEHPAVBuffer@@HHHHHHH@Z 0x005DA6C0-0x005DA851
// symbol    ?tile_mask@Buffer@@QAEHPAV1@HHHHHHH@Z
// size      401 bytes
// prototype int (__thiscall ?tile_mask@Buffer@@QAEHPAVBuffer@@HHHHHHH@Z)(Buffer* this, Buffer*, int, int, int, int, int, int, int)
// callers   1   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005D8000 0x005E2B00
// indirect  0x005DA744
// TRIED: same shape as 0x005DA860/0x005DA9F0 - straight transcription diverges at #2 on the prologue register-save set (fewer registers stay live in this form than the original chose).
// TRIED: the same three re-spellings measured on 0x005DA860 above (const, the
//        selfRect store order, the body nested under the null guards) leave
//        this at 4/146 with the image's 0x24 frame against 0x28 - the same
//        allocator slot preference, documented once there.
*/
int Buffer::tile_mask(Buffer *mask, int src_x, int src_y, int x, int y, int w,
                      int h, int color) {
    if (dib_bits_ == 0 && surface_ == 0) {
        return 7;
    }

    RECT clipRect;
    clipRect.left = x;
    clipRect.top = y;
    const int rectRight = w + x;
    clipRect.right = rectRight;
    clipRect.bottom = h + y;

    RECT selfRect = rect1_;

    if (IntersectRect(&clipRect, &clipRect, &selfRect)) {
        set_clip(&clipRect);

        const int tileWidth = mask->dib_.bmiHeader.biWidth;
        const int tileHeight = -mask->dib_.bmiHeader.biHeight;

        if (tileWidth != 0 && tileHeight != 0) {
            int tilesX = w / tileWidth;
            if (tilesX * tileWidth < w) {
                tilesX++;
            }
            int tilesY = h / tileHeight;
            if (tilesY * tileHeight < h) {
                tilesY++;
            }

            const int startX = x - src_x % tileWidth;
            const int startY = y - src_y % tileHeight;

            if (tilesX * tileWidth + startX < rectRight) {
                tilesX++;
            }
            if (tilesY * tileHeight + startY < h + y) {
                tilesY++;
            }

            int rowsLeft = tilesY;
            if (rowsLeft > 0) {
                int rowY = startY;
                do {
                    int colsLeft = tilesX;
                    if (colsLeft > 0) {
                        int colX = startX;
                        do {
                            mask->copy_mask(this, 0, 0, colX, rowY, tileWidth,
                                            tileHeight, color);
                            colsLeft--;
                            colX += tileWidth;
                        } while (colsLeft != 0);
                    }
                    rowY += tileHeight;
                    rowsLeft--;
                } while (rowsLeft != 0);
            }

            set_clip(&selfRect);
        }
    }

    return 0;
}

/*
Purpose: Tile a SPRITE across a rectangle. The sprite overload differs from
         the buffer one in three ways: the dimensions come off the sprite
         record rather than a DIB header, the vertical step is NOT negated,
         and each tile is a `Sprite::draw` at the transparent index rather
         than a `copy`.

         HOMED from src/unrecovered/005da9f0.cpp. `box_sprite` tiles the four
         edge strips of every frame through this one.
// ORIGINAL: 0x005DA9F0 ?tile@Buffer@@QAEHPAVSprite@@HHHHHH@Z 0x005DA9F0-0x005DAB8A
// symbol    ?tile@Buffer@@QAEHPAVSprite@@HHHHHH@Z
// size      410 bytes
// prototype int (__thiscall ?tile@Buffer@@QAEHPAVSprite@@HHHHHH@Z)(Buffer* this, Sprite*, int, int, int, int, int, int)
// callers   1   call targets   3
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005D8000 0x005EFF00
// indirect  0x005DAB78
*/

int Buffer::tile(Sprite *sprite, int src_x, int src_y, int x, int y, int w,
                 int h) {
    if (dib_bits_ == 0 && surface_ == 0) {
        return 7;
    }

    RECT clipRect;
    clipRect.left = x;
    clipRect.top = y;
    const int rectRight = w + x;
    clipRect.right = rectRight;
    clipRect.bottom = y + h;

    RECT selfRect = rect1_;

    if (IntersectRect(&clipRect, &clipRect, &selfRect)) {
        set_clip(&clipRect);

        const int tileWidth = sprite->iWidth_;
        const int tileHeight = sprite->iHeight_;

        if (tileWidth != 0 && tileHeight != 0) {
            int tilesX = w / tileWidth;
            if (tilesX * tileWidth < w) {
                tilesX++;
            }
            int tilesY = h / tileHeight;
            if (tilesY * tileHeight < h) {
                tilesY++;
            }

            const int startX = x - src_x % tileWidth;
            const int startY = y - src_y % tileHeight;

            if (tilesX * tileWidth + startX < rectRight) {
                tilesX++;
            }
            if (tilesY * tileHeight + startY < y + h) {
                tilesY++;
            }

            int rowsLeft = tilesY;
            if (rowsLeft > 0) {
                int rowY = startY;
                do {
                    int colsLeft = tilesX;
                    if (colsLeft > 0) {
                        int colX = startX;
                        do {
                            sprite->draw(this, sprite->cTransparentIndex_,
                                         colX, rowY, 1, 1);
                            colsLeft--;
                            colX += tileWidth;
                        } while (colsLeft != 0);
                    }
                    rowY += tileHeight;
                    rowsLeft--;
                } while (rowsLeft != 0);
            }

            set_clip(&selfRect);
        }
    }

    return 0;
}

/*
Purpose: Blit one rectangle from this buffer into `dest`, replacing every
         destination pixel whose current colour is the mask colour. The
         clipping is done in three stages - negative source coordinates are
         folded into the extent, the extent is clamped to the source's own
         DIB, and the surviving rectangle is intersected with the
         destination's clip - which is why the two `get_data` calls below
         are handed DIFFERENT origins.

         HOMED from src/unrecovered/005e2b00.cpp, which held this
         transcription. The direction is the non-obvious part and the
         archive's own note records it: `dest` (the argument) is the buffer
         WRITTEN and mask-tested, `this` is the one READ.
// ORIGINAL: 0x005E2B00 ?copy_mask@Buffer@@QAEHPAVBuffer@@HHHHHHH@Z 0x005E2B00-0x005E2DD3 FILE
// symbol    ?copy_mask@Buffer@@QAEHPAV1@HHHHHHH@Z
// size      723 bytes
// prototype int (__thiscall ?copy_mask@Buffer@@QAEHPAVBuffer@@HHHHHHH@Z)(Buffer* this, Buffer*, int, int, int, int, int, int, int)
// callers   2   call targets   2
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005E33F3 0x005E34A3
// indirect  0x005E2C27
// TRIED: plain nested for-loop instead of the Duff's-device-style 4x-unrolled byte-compare-copy the original lowers to; edi/esi roles are swapped from the naive reading - `a1` (the argument) is the one WRITTEN and mask-tested, `this` is the one READ; diverges at instr #0 (prologue shape)
// TRIED: the residual is the INNER LOOP, not the prologue, and it is a wall
//        of the same kind as `fill`/`copy` in this file. The image's scan is
//        a DUFF'S DEVICE - `switch (width & 3)` falling into a do/while that
//        runs `width / 4 + (width & 3 ? 1 : 0)` times - and it keeps its
//        state in fixed .data, not in locals: the case counter at 0x009B3A80,
//        the `this`-side row remainder at 0x009B3A84 and the destination-side
//        one at 0x009B37D8, all written with `mov dword ptr [abs]` and
//        incremented inside the dispatch. The dispatch itself is a chain of
//        `lea edx, <case>` / `jmp edx` computed jumps, which is not how VC6
//        lowers a switch (it builds a table), so this block is hand-written
//        assembly in the original and byte-exactness is not reachable from
//        C++. `handwritten_asm.py` does not flag it because `jmp reg` is not
//        on its opcode list. The nested loop here is the same program - the
//        direction was verified against the bytes (`edi` = dest->get_data is
//        compared and written, `esi` = get_data on `this` is read) - and the
//        three extra stride/width locals are the image's three globals.
*/
int Buffer::copy_mask(Buffer *dest, int src_x, int src_y, int x, int y, int w,
                      int h, int color) {
    if (dib_bits_ == 0 && surface_ == 0) {
        return 7;
    }
    if (dest == 0) {
        return 3;
    }

    // Fold negative source coordinates into the extent: the part of the
    // rectangle that falls above/left of the source is not drawn, and the
    // destination origin moves with it.
    if (src_x < 0) {
        w += src_x;
        x -= src_x;
        src_x = 0;
    }
    if (src_y < 0) {
        h += src_y;
        y -= src_y;
        src_y = 0;
    }

    const int source_width = dib_.bmiHeader.biWidth;
    const int source_height = -dib_.bmiHeader.biHeight;

    if (source_width < src_x) {
        return 0;
    }
    if (source_height < src_y) {
        return 0;
    }

    if (source_width < src_x + w) {
        w = source_width - src_x;
    }
    if (source_height < src_y + h) {
        h = source_height - src_y;
    }

    RECT clipped;
    clipped.left = x;
    clipped.top = y;
    clipped.right = x + w;
    clipped.bottom = y + h;

    if (!IntersectRect(&clipped, &clipped, &dest->rect1_)) {
        return 0;
    }

    // Whatever the intersection took off the left and top edges is taken off
    // the source origin too, so the two reads stay in step.
    const int inset_x = clipped.left - x;
    const int inset_y = clipped.top - y;

    unsigned char *write_ptr =
        reinterpret_cast<unsigned char *>(dest->get_data(clipped.left, clipped.top));
    if (write_ptr == 0) {
        return 3;
    }
    unsigned char *read_ptr = reinterpret_cast<unsigned char *>(
        get_data(src_x + inset_x, src_y + inset_y));
    if (read_ptr == 0) {
        return 3;
    }

    const int write_stride = dest->stride_;
    const int read_stride = stride_;
    const int rows = clipped.bottom - clipped.top;
    const int columns = clipped.right - clipped.left;
    const unsigned char mask = static_cast<unsigned char>(color);

    for (int row = 0; row < rows; row++) {
        for (int column = 0; column < columns; column++) {
            if (write_ptr[column] == mask) {
                write_ptr[column] = read_ptr[column];
            }
        }
        read_ptr += read_stride - columns;
        write_ptr += write_stride - columns;
    }

    free_data(1);
    dest->free_data(1);
    return 0;
}

/*
Purpose: Draw one framed panel: four corner sprites, four tiled edge strips
         and a tiled centre, every element optionally masked through the
         params' pattern buffer.
// ORIGINAL: 0x005E2DD3 ?box_sprite@Buffer@@QAEHPAURECT@@PAUBoxSpriteParams@@@Z 0x005E2DD3-0x005E3188
// symbol    ?box_sprite@Buffer@@QAEHPAUtagRECT@@PAUBoxSpriteParams@@@Z
// size      949 bytes
// prototype int (__thiscall ?box_sprite@Buffer@@QAEHPAURECT@@PAUBoxSpriteParams@@@Z)(Buffer* this, RECT*, BoxSpriteParams*)
// callers   127   call targets   4
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005DA6C0 0x005DA860 0x005DA9F0 0x005E4B4A
// LEVER: cTransparentIndex_ is UNSIGNED. The image reads the byte as
//        `xor eax,eax; mov al,[sprite+8]` at every site that hands it to
//        Sprite::draw as an int (here and at 0x005EFF02 in Win::flip); a
//        `char` compiles to `movsx` and cost four instructions.
// LEVER: the corner-trim widths/heights are NOT named locals. Spelling
//        `const int cw = corner0_->iWidth_; x += cw; w -= cw;` gives the
//        temporary a home slot and stores it three times; repeating the
//        expression keeps it in a register. That one change was worth 24
//        agreeing instructions and the whole frame shape (409 -> 406, which
//        is the image's own count).
// LEVER: the whole body sits inside one `if (rect != 0 && params != 0)`
//        rather than returning early from two guards. A `return 0` guard
//        makes VC6 emit a second epilogue with `xor eax,eax`; the image has
//        ONE epilogue and nothing writes eax on the guard path, because the
//        value the function returns is the centre tile's and the guards
//        simply skip the work. Bare `return;` is error C2561 and an
//        uninitialised `return result;` emits a load, so the wrapped `if` is
//        the only spelling that reaches it.
// TRIED: hoisting x/y to function scope (initialised and not), `register`,
//        dropping `const`, caching vs not caching the sprite pointer, naming
//        the corner-trim temporaries, reordering the edge insets. All leave
//        the same four instructions: in corner1 and corner2 the image spills
//        the sprite pointer into its reserved frame slot (`push ecx`), this
//        tree spills it into the dead `params` parameter slot instead - the
//        allocator's slot preference, not a source relationship, and it
//        costs one displacement on each of the two stores and their two
//        reloads.
Status: SHAPE_EXACT 402/406 - the instruction count matches at 406, and the
        four that disagree are the corner1/corner2 spill-slot choice above.
*/
int Buffer::box_sprite(RECT *rect, BoxSpriteParams *params) {
    if (rect != 0 && params != 0) {
        int x;
        int y;

        // The corners, drawn 1:1. Top-left, top-right, bottom-left, bottom-right:
        // the order of the four tests is the order the image draws them in.
        if (params->corner0_ != 0) {
            Sprite *const sprite = params->corner0_;
            x = rect->left;
            y = rect->top;
            sprite->draw(this, sprite->cTransparentIndex_, x, y, 1, 1);
            if (params->mask_ != 0) {
                tile_mask(params->mask_, x, y, x, y, sprite->iWidth_,
                          sprite->iHeight_, params->mask_color_);
            }
        }

        if (params->corner1_ != 0) {
            Sprite *const sprite = params->corner1_;
            x = rect->right - sprite->iWidth_;
            y = rect->top;
            sprite->draw(this, sprite->cTransparentIndex_, x, y, 1, 1);
            if (params->mask_ != 0) {
                tile_mask(params->mask_, x, y, x, y, sprite->iWidth_,
                          sprite->iHeight_, params->mask_color_);
            }
        }

        if (params->corner2_ != 0) {
            Sprite *const sprite = params->corner2_;
            x = rect->left;
            y = rect->bottom - sprite->iHeight_;
            sprite->draw(this, sprite->cTransparentIndex_, x, y, 1, 1);
            if (params->mask_ != 0) {
                tile_mask(params->mask_, x, y, x, y, sprite->iWidth_,
                          sprite->iHeight_, params->mask_color_);
            }
        }

        if (params->corner3_ != 0) {
            Sprite *const sprite = params->corner3_;
            x = rect->right - sprite->iWidth_;
            y = rect->bottom - sprite->iHeight_;
            sprite->draw(this, sprite->cTransparentIndex_, x, y, 1, 1);
            if (params->mask_ != 0) {
                tile_mask(params->mask_, x, y, x, y, sprite->iWidth_,
                          sprite->iHeight_, params->mask_color_);
            }
        }

        // The edges, tiled between the corners. Top, bottom, left, right: each
        // strip is inset by the two corners that stand on it. THE INSET ORDER
        // IS MEASURED - edge1 trims its width before it moves x, the other
        // three move first, and swapping them costs four instructions.
        if (params->edge0_ != 0) {
            Sprite *const sprite = params->edge0_;
            x = rect->left;
            y = rect->top;
            int w = rect->right - x;
            const int h = sprite->iHeight_;
            if (params->corner0_ != 0) {
                x += params->corner0_->iWidth_;
                w -= params->corner0_->iWidth_;
            }
            if (params->corner1_ != 0) {
                w -= params->corner1_->iWidth_;
            }
            tile(sprite, 0, 0, x, y, w, h);
            if (params->mask_ != 0) {
                tile_mask(params->mask_, x, y, x, y, w, h, params->mask_color_);
            }
        }

        if (params->edge1_ != 0) {
            Sprite *const sprite = params->edge1_;
            x = rect->left;
            y = rect->bottom - sprite->iHeight_;
            int w = rect->right - x;
            const int h = sprite->iHeight_;
            if (params->corner2_ != 0) {
                w -= params->corner2_->iWidth_;
                x += params->corner2_->iWidth_;
            }
            if (params->corner3_ != 0) {
                w -= params->corner3_->iWidth_;
            }
            tile(sprite, 0, 0, x, y, w, h);
            if (params->mask_ != 0) {
                tile_mask(params->mask_, x, y, x, y, w, h, params->mask_color_);
            }
        }

        if (params->edge2_ != 0) {
            Sprite *const sprite = params->edge2_;
            x = rect->left;
            y = rect->top;
            const int w = sprite->iWidth_;
            int h = rect->bottom - y;
            if (params->corner0_ != 0) {
                y += params->corner0_->iHeight_;
                h -= params->corner0_->iHeight_;
            }
            if (params->corner2_ != 0) {
                h -= params->corner2_->iHeight_;
            }
            tile(sprite, 0, 0, x, y, w, h);
            if (params->mask_ != 0) {
                tile_mask(params->mask_, x, y, x, y, w, h, params->mask_color_);
            }
        }

        if (params->edge3_ != 0) {
            Sprite *const sprite = params->edge3_;
            x = rect->right - sprite->iWidth_;
            y = rect->top;
            const int w = sprite->iWidth_;
            int h = rect->bottom - y;
            if (params->corner1_ != 0) {
                y += params->corner1_->iHeight_;
                h -= params->corner1_->iHeight_;
            }
            if (params->corner3_ != 0) {
                h -= params->corner3_->iHeight_;
            }
            tile(sprite, 0, 0, x, y, w, h);
            if (params->mask_ != 0) {
                tile_mask(params->mask_, x, y, x, y, w, h, params->mask_color_);
            }
        }

        // The centre, tiled from the mask buffer itself and inset by all four
        // edges. The image re-tests mask_ here rather than reusing the last
        // edge's test, and its return value is this tile's: the guards at the
        // top leave eax alone, so this is the only `return` the body needs.
        if (params->mask_ != 0) {
            int cx = rect->left;
            int cy = rect->top;
            int w = rect->right - cx;
            int h = rect->bottom - cy;
            if (params->edge2_ != 0) {
                cx += params->edge2_->iWidth_;
                w -= params->edge2_->iWidth_;
            }
            if (params->edge0_ != 0) {
                cy += params->edge0_->iHeight_;
                h -= params->edge0_->iHeight_;
            }
            if (params->edge3_ != 0) {
                w -= params->edge3_->iWidth_;
            }
            if (params->edge1_ != 0) {
                h -= params->edge1_->iHeight_;
            }
            return tile(params->mask_, cx, cy, cx, cy, w, h);
        }
    }
}

// Original initializer 0x005D71D0 constructs this Sprite at 0x009B3A50
// through 0x005E37E0 and registers its cleanup; 0x005D71F0 closes it through
// 0x005E3820. The blit setup/reset family borrows its pixel pointer rather
// than owning another allocation. Its original storage is 0x2C zero-fill bytes.
// RECOVERY_OBJECT: v2 0x009B3A50 0x009B3A7C ?g_BUFFER_SPRITE@@3VSprite@@A 0x005D8290
Sprite g_BUFFER_SPRITE;

// 0x008C6E2C and 0x008C6E30 - see the declaration in buffer.h. Zero here as
// the image's .bss is; the preferences write them before any window is drawn.
int TextColourTitleMain;
int TextColourTitleShadow;

// See the declaration in buffer.h. Copied verbatim out of the image's .rdata,
// four characters plus a NUL each, so the loader families append exactly what
// the shipped build appends.
const char BufferExtTgf[] = ".tgf";
const char BufferExtTgg[] = ".tgg";
const char BufferExtTga[] = ".tga";
const char BufferExtBmf[] = ".bmf";
const char BufferExtBmg[] = ".bmg";
const char BufferExtBmp[] = ".bmp";

// ===== MANAGED GLOBALS - real objects, homed to their domain =====
// In the shipped image these live at fixed data addresses and are
// constructed before WinMain by the CRT's dynamic-initializer walk
// (winedbg-confirmed: walker return 0x00644EEB, table at 0x682568..).
// Here the same recovered constructors run through this build's own
// startup, and the matching destructors close them at exit. Addresses of
// the ones documented individually live beside their definitions.
// Startup 44CC20 constructs this Buffer; 44CC40 destroys it. Constructor
// 5D7254 writes the final DWORD at +584; the three independent Buffer
// receivers in 462779..4627A1 have stride588, bounding its complete extent.
// RECOVERY_OBJECT: v2 0x00798668 0x00798BF0 ?g_PCX_PARSE_TEMP_BUFFER1@@3VBuffer@@A 0x00454FC0
Buffer g_PCX_PARSE_TEMP_BUFFER1;  // 0x00798668
// Original initializer 0x44CF70 and exit 0x44CFA0 use 51 elements at stride
// 0x588; the loader's end pointer is 0x76DC30. Image-initial storage is zero.
// RECOVERY_OBJECT: v2 0x0075C218 0x0076DC30 ?g_IFACE_BOX_SPRITE_BUFFERS@@3PAVBuffer@@A 0x00453A70
Buffer g_IFACE_BOX_SPRITE_BUFFERS[51];
// Original initializer/exit 0x44D140/0x44D160 use Buffer's complete 0x588
// extent (constructor's final DWORD is at +0x584), initially zero.
// RECOVERY_OBJECT: v2 0x0077AA70 0x0077AFF8 ?g_IFACE_STD_POPUPS_MIDDLE_BUFFER@@3VBuffer@@A 0x00453A70
Buffer g_IFACE_STD_POPUPS_MIDDLE_BUFFER;
Buffer g_VEHDRAW_BUFFER;  // 0x008CC298
// File-backed Sprite draw stages through this Buffer: original 0x005E4BC3,
// 0x005E4BFC and 0x005E4C09 use it for load_pcx, drawing and close.
// RECOVERY_OBJECT: v1 0x009B6080 ?g_BUFFER@@3VBuffer@@A
Buffer g_BUFFER;
// ===== MANAGED GLOBALS - real objects, homed to their domain =====
// In the shipped image these live at fixed data addresses and are
// constructed before WinMain by the CRT's dynamic-initializer walk
// (winedbg-confirmed: walker return 0x00644EEB, table at 0x682568..).
// Here the same recovered constructors run through this build's own
// startup, and the matching destructors close them at exit.

// ORIGINAL: 0x005D7B40 ?load_tga@Buffer@@QAGHPBDPAVPalette@@HH@Z 0x005D7B40-0x005D7C78;0x00662B92-0x00662BA7 FILE
// TRIED: plain `Filemap fm;` with early returns has no SEH prologue at all; wrapping in try{}catch(...){throw;} recovers most of the /GX frame shape but still diverges at #0 push/mov - same wall as 0x005D7A30's Filemap-ctor SEH frame.
// size      333 bytes
// prototype 
// callers   1   call targets   5
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00628380 0x006283E0 0x00628430 0x006453E0 0x00645470
// indirect  0x005D7C41
// placeholder - not yet decompiled
// To start: tools/decomp_status.py --work 0x005D7B40


int __stdcall Buffer::load_tga(const char * a1, Palette * a2, int a3, int a4) {
    try {
        Filemap fm;
        if (a1 == 0) {
            return 3;
        }
        char path[260];
        path[0] = 0;
        strcat(path, a1);
        char *end = path + strlen(a1);
        if (end != path) {
            do {
                if (*end == '.') {
                    if (end != path) goto has_ext;
                    break;
                }
                end--;
            } while (end != path);
        }
        switch (Language) {
            case 1: strcat(path, BufferExtTgf); break;
            case 2: strcat(path, BufferExtTgg); break;
            default: strcat(path, BufferExtTga); break;
        }
      has_ext:
        if (fm.open_read(path, 1) == 0) {
            return 6;
        }
        GetFileSize(fm.get_handle(), 0);
        return 8;
    } catch (...) {
        throw;
    }
}

// ORIGINAL: 0x005E3193 ?box@Buffer@@QAEHHHHHHH@Z 0x005E3193-0x005E31FC FILE
// TRIED: falling off the end without a return (C4716 under the scoring flags, treated as an error); the near-miss is the extra "xor eax,eax" that "return 0" adds versus the original's bare "pop"-chain tail.
// working copy - scaffold materialised by --work
// size      105 bytes
// prototype int (__thiscall ?box@Buffer@@QAEHHHHHHH@Z)(Buffer* this, int, int, int, int, int, int)
// callers   1   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005E1A80 0x005E1BF0

int Buffer::box(int a1, int a2, int a3, int a4, int a5, int a6) {
    hline(a1 + 1, a3, a2, a5);
    hline(a1, a3 - 1, a4, a6);
    vline(a1, a2, a4 - 1, a5);
    vline(a3, a2 + 1, a4, a6);
    return 0;
}


// ====================
// THE TWO DOTTED PRIMITIVES. Both are HAND-WRITTEN ASSEMBLY IN THE ORIGINAL
// (the earlier census found the shipped bytes count the run down with the
// `loop` opcode, which VC6 never emits), so byte-exactness is NOT reachable
// from C++ and the honest ceiling here is semantic equivalence. Same wall
// `vline` documented; do not grind spellings here and do not write `__asm`.
//
// The lock / unlock in the middle is `get_data` and `free_data`, exactly as
// the byte-exact `hline` uses them; the only difference is the step.

// ORIGINAL: 0x005E1D70 ?dotted_hline@Buffer@@QAEXHHHH@Z 0x005E1D70-0x005E1EE4 FILE
// size      372 bytes
// prototype void (__thiscall ?dotted_hline@Buffer@@QAEXHHHH@Z)(Buffer* this, int, int, int, int)
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x005E1E3B 0x005E1ECF
// PRESERVED UNIT - measured MISMATCH. Homed from src/recovered/units/ for
// COVERAGE: the archive could not measure it and this TU can.
void Buffer::dotted_hline(int x1, int x2, int y, int color) {
    if (dib_bits_ == nullptr && surface_ == nullptr) {
        return;
    }
    if (y < rect1_.top) {
        return;
    }
    if (y >= rect1_.bottom) {
        return;
    }
    if (x1 == x2) {
        return;
    }
    if (x1 > x2) {
        x1 ^= x2;
        x2 ^= x1;
        x1 ^= x2;
    }
    if (x1 >= rect1_.right) {
        return;
    }
    if (x2 < rect1_.left) {
        return;
    }
    if (x1 < rect1_.left) {
        x1 = rect1_.left;
    }
    if (x2 >= rect1_.right) {
        x2 = rect1_.right - 1;
    }
    if (x1 >= dib_.bmiHeader.biWidth) {
        return;
    }
    if (y >= -dib_.bmiHeader.biHeight) {
        return;
    }
    if (get_data() == 0) {
        return;
    }
    // EVERY OTHER BYTE, left to right: the run length is halved and the
    // destination pointer steps by two. This is what makes it dotted rather
    // than solid - `hline` memsets the whole run.
    char *dest = stride_ * y + x1 + static_cast<char *>(locked_bits_);
    if (dest == nullptr) {
        return;
    }
    unsigned int count = (static_cast<unsigned int>(x2 - x1) + 1) >> 1;
    do {
        *dest = static_cast<char>(color);
        dest += 2;
    } while (--count != 0);
    free_data(1);
}

// ORIGINAL: 0x005E1EF0 ?dotted_vline@Buffer@@QAEXHHHH@Z 0x005E1EF0-0x005E2070 FILE
// size      384 bytes
// prototype void (__thiscall ?dotted_vline@Buffer@@QAEXHHHH@Z)(Buffer* this, int, int, int, int)
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x005E1FBD 0x005E205B
// PRESERVED UNIT - measured MISMATCH. Homed from src/recovered/units/ for
// COVERAGE. The step is a whole ROW - `stride_ * 2` - rather than the two
// bytes `dotted_hline` steps, which is what makes the dots run downwards.
void Buffer::dotted_vline(int x, int y1, int y2, int color) {
    if (dib_bits_ == nullptr && surface_ == nullptr) {
        return;
    }
    if (x < rect1_.left) {
        return;
    }
    if (x >= rect1_.right) {
        return;
    }
    if (y1 == y2) {
        return;
    }
    if (y1 > y2) {
        y1 ^= y2;
        y2 ^= y1;
        y1 ^= y2;
    }
    if (y1 >= rect1_.bottom) {
        return;
    }
    if (y2 < rect1_.top) {
        return;
    }
    if (y1 < rect1_.top) {
        y1 = rect1_.top;
    }
    if (y2 >= rect1_.bottom) {
        y2 = rect1_.bottom - 1;
    }
    if (x >= dib_.bmiHeader.biWidth) {
        return;
    }
    if (y1 >= -dib_.bmiHeader.biHeight) {
        return;
    }
    if (get_data() == 0) {
        return;
    }
    char *dest = stride_ * y1 + x + static_cast<char *>(locked_bits_);
    if (dest == nullptr) {
        return;
    }
    unsigned int count = (static_cast<unsigned int>(y2 - y1) + 1) >> 1;
    do {
        *dest = static_cast<char>(color);
        dest += stride_ * 2;
    } while (--count != 0);
    free_data(1);
}

// ORIGINAL: 0x005E3283 ?dotted_box@Buffer@@QAEHHHHHH@Z 0x005E3283-0x005E32E4 FILE
// TRIED: nothing beyond the direct four-call transcription; diverges at #44 (pop vs xor), near the epilogue
// working copy - scaffold materialised by --work
// size      97 bytes
// prototype int (__thiscall ?dotted_box@Buffer@@QAEHHHHHH@Z)(Buffer* this, int, int, int, int, int)
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005E1D70 0x005E1EF0

int Buffer::dotted_box(int a1, int a2, int a3, int a4, int a5) {
    dotted_hline(a1 + 1, a3, a2, a5);
    dotted_hline(a1, a3 - 1, a4, a5);
    dotted_vline(a1, a2, a4 - 1, a5);
    dotted_vline(a3, a2 + 1, a4, a5);
    return 0;
}

// ORIGINAL: 0x005E32F3 ?dotted_box@Buffer@@QAEHPAURECT@@H@Z 0x005E32F3-0x005E3370 FILE
// symbol    ?dotted_box@Buffer@@QAEHPAUtagRECT@@H@Z
// working copy - scaffold materialised by --work
// size      125 bytes
// prototype int (__thiscall ?dotted_box@Buffer@@QAEHPAURECT@@H@Z)(Buffer* this, RECT*, int)
// callers   2   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005E1D70 0x005E1EF0

int Buffer::dotted_box(RECT * a1, int a2) {
    if (a1 == 0) {
        return 0x10;
    }
    int top = a1->top;
    int bottom = a1->bottom;
    int right = a1->right;
    int left = a1->left;
    --bottom;
    --right;
    dotted_hline(left + 1, right, top, a2);
    dotted_hline(left, right - 1, bottom, a2);
    dotted_vline(left, top, bottom - 1, a2);
    dotted_vline(right, top + 1, bottom, a2);
    return 0;
}

// Fixed-slot bindings carried from 005dd450.cpp

// ORIGINAL: 0x005DD450 ?write_right_l@Buffer@@QAEHPADPAURECT@@H@Z 0x005DD450-0x005DD52B FILE
// symbol    ?write_right_l@Buffer@@QAEHPADPAUtagRECT@@H@Z
// TRIED: bare `strlen` (MSVC inlines it as `repne scasb`, needs `#pragma function(strlen)` to force the real call). With the pragma the structure and call sequence match, but the register allocator puts a1/this in different registers than the original (ebp/esi here vs ebx/edi there); not reproduced. 78/85 shared.
// working copy - scaffold materialised by --work
// size      219 bytes
// prototype int (__thiscall ?write_right_l@Buffer@@QAEHPADPAURECT@@H@Z)(Buffer* this, int8*, RECT*, int)
// callers   18   call targets   3
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005DC7C0 0x005DCAE0 0x006453E0


int Buffer::write_right_l(char * a1, RECT * a2, int a3) {
    if (a1 != 0 && a2 != 0) {
        if (font1_ == 0 || font1_->font_obj_ == 0) {
            return 3;
        }
        int sVar2 = strlen(a1);
        int sVar3 = a3;
        if (sVar2 < a3) {
            sVar3 = strlen(a1);
        }
        if (sVar3 >= 0) {
            sVar3 = strlen(a1);
            if (sVar3 < a3) {
                a3 = strlen(a1);
            }
            if (a3 != 0) {
                int width = text_width(a1, a3);
                if (font1_ == 0) {
                    font1_ = reinterpret_cast<Font *>(FontDefault);
                }
                return write_multi_font_raw_l(a1, a2->right - width,
                    (a2->bottom - font1_->height_ - a2->top) / 2 + a2->top, a3);
            }
        }
    }
    return 0;
}

// ORIGINAL: 0x005DB6C0 ?write_cent_l@Buffer@@QAEHPAVFont@@PADHHHH@Z 0x005DB6C0-0x005DB71E FILE
// TRIED: direct transcription only tried once; diverges at #0 (push vs mov) in the prologue register-save pattern
// working copy - scaffold materialised by --work
// size      94 bytes
// prototype int (__thiscall ?write_cent_l@Buffer@@QAEHPAVFont@@PADHHHH@Z)(Buffer* this, Font*, int8*, int, int, int, int)
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005DD020

int Buffer::write_cent_l(Font * a1, char * a2, int a3, int a4, int a5, int a6) {
    Font *saved_font1 = font1_;
    Font *saved_font2 = font2_;
    Font *saved_font3 = font3_;
    if (a1 != 0 && a1->font_obj_ != 0) {
        font1_ = a1;
        font2_ = 0;
        font3_ = 0;
    }
    int result = write_cent_l(a2, a3, a4, a5, a6);
    font1_ = saved_font1;
    font2_ = saved_font2;
    font3_ = saved_font3;
    return result;
}

// ORIGINAL: 0x005DB870 ?write_right_l@Buffer@@QAEHPAVFont@@PADHHHH@Z 0x005DB870-0x005DB8CE FILE
// TRIED: named-local save/restore and raw-pointer-triple save/restore both match control flow and the call, but VC6 schedules the three register saves (ebx/ebp/esi) differently than the original at #0 (push vs mov) - register allocation, not logic
// working copy - scaffold materialised by --work
// size      94 bytes
// prototype int (__thiscall ?write_right_l@Buffer@@QAEHPAVFont@@PADHHHH@Z)(Buffer* this, Font*, int8*, int, int, int, int)
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005DD300

int Buffer::write_right_l(Font * a1, char * a2, int a3, int a4, int a5, int a6) {
    Font * saved_font1 = font1_;
    Font * saved_font2 = font2_;
    Font * saved_font3 = font3_;
    if (a1 != 0 && a1->font_obj_ != 0) {
        font1_ = a1;
        font2_ = 0;
        font3_ = 0;
    }
    int result = write_right_l(a2, a3, a4, a5, a6);
    font1_ = saved_font1;
    font2_ = saved_font2;
    font3_ = saved_font3;
    return result;
}

// THE FLYING-TEXT BLOCK, and the two space constants its writer joins lines
// with. Named bindings, not objects: the image holds them at these fixed
// data addresses and nothing in this build maps them, so the binding form
// (the same convention socialwin.cpp's SpaceText uses) keeps the addresses
// visible without inventing storage for them.
//
//   0x009BB484  the process default font - font.h's own FontDefault, which
//               is why no binding for it is declared here
//   0x009B33CC  the accumulator `wrap_flying` builds each line in
//   0x009B3A90  the pen x the next line is written at
//   0x009B3A94  the pen y the next line is written at
//   0x009B3A98  the height still available before the next wrap
//               (0x009B3A9C, the next slot up, is temp.h's BufferStrHeight)
//   0x00696CD8 / 0x00696CDC  two distinct one-space .rdata strings: the
//               first is what `wrap_height_flying` measures as the
//               more-text-follows indicator, the second is what `wrap_flying`
//               measures for the same purpose on its own path
static int *const FlyingTextBuf = (int *)0x009B33CC;
static int *const FlyingTextX = (int *)0x009B3A90;
static int *const FlyingTextY = (int *)0x009B3A94;
static int *const FlyingTextHeight = (int *)0x009B3A98;
// 0x00682820 - the one-space .rdata string, the same constant
// socialwin.cpp's SpaceText binding names.
// THE THREE WRITER FONT SLOTS (font1_/font2_/font3_) ARE SAVED AND RESTORED
// AS ONE BLOCK by every Font*-first wrapper in this file. The struct names the
// three members without punning them; the save and the restore stay three
// moves of three named fields, which is what the image has.
struct BufferFontTriple {
  Font *primary;
  Font *second;
  Font *third;
};

// 0x009B39D8 - where the length-limited writers leave their return code;
// `wrap`'s no-font path stores 3 there and reads it straight back.
static int *const BufferWriteStatus = (int *)0x009B39D8;
// 0x00696C90 - the "$LINK<" header token, three copies in a row in .rdata;
// `find_line_break_l` strncmp's the first six bytes of a '$' segment
// against it to recognise a link.
static const char *const MarkupLinkToken = (const char *)0x00696C90;
static int *const SpaceText = (int *)0x00682820;
static int *const SpaceTextPrompt = (int *)0x00696CD8;
static int *const SpaceTextTail = (int *)0x00696CDC;

// ORIGINAL: 0x005DD250 ?write_cent_l@Buffer@@QAEHPADHHH@Z 0x005DD250-0x005DD2F3 FILE
// working copy - scaffold materialised by --work
// size      163 bytes
// prototype int (__thiscall ?write_cent_l@Buffer@@QAEHPADHHH@Z)(Buffer* this, int8*, int, int, int)
// callers   3   call targets   3
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005DC7C0 0x005DCAE0 0x006453E0


int Buffer::write_cent_l(char * a1, int a2, int a3, int a4) {
        if (a1 == 0) {
        return a2;
    }
    int font1 = reinterpret_cast<int>(font1_);
    if (font1 != 0 && *reinterpret_cast<int *>(font1 + 8) != 0) {
        int len1 = strlen(a1);
        int len = a4;
        if (len1 < a4) {
            len = strlen(a1);
        }
        if (len >= 0) {
            int len2 = strlen(a1);
            if (len2 < a4) {
                len = strlen(a1);
            }
            if (len != 0) {
                int width = text_width(a1, len);
                return write_multi_font_raw_l(a1, a2 - (width >> 1), a3, len);
            }
        }
        return a2;
    }
    return 3;
}

// ORIGINAL: 0x005DDA00 ?wrap_cent@Buffer@@QAEHPADHHHH@Z 0x005DDA00-0x005DDAA2 FILE
// working copy - scaffold materialised by --work
// size      162 bytes
// prototype int (__thiscall ?wrap_cent@Buffer@@QAEHPADHHHH@Z)(Buffer* this, int8* lpString, int, int, int, int)
// callers   0   call targets   3
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005DC410 0x005DD020 0x006453E0

int Buffer::wrap_cent(char * a1, int a2, int a3, int a4, int a5) {
    if (a1 == 0) {
        return a3;
    }
    BufferStrHeight = 0;
    field_18_ = 0;
    int total = a3;
    char *cur = a1;
    char *brk;
    do {
        int width = a4;
        if (cur == 0) {
            brk = 0;
        } else {
            BufferStrHeight = 0;
            unsigned int len = strlen(cur);
            brk = (char *)find_line_break_l(cur, &width, (int)len);
            if (brk == 0) {
                len = strlen(cur);
            } else {
                len = (unsigned int)(brk - cur);
            }
            write_cent_l(cur, a2, total, a4, (int)len);
        }
        total = total + a5;
        field_18_ = field_18_ + 1;
        cur = brk;
    } while (brk != 0);
    return total;
}

// ORIGINAL: 0x005DD920 ?wrap_cent@Buffer@@QAEHPADHHH@Z 0x005DD920-0x005DD9F2 FILE BYTE_EXACT
// LEVER: #pragma function(strlen) stops the intrinsic `repne scasb` inlining; write_cent_l's length arg is duplicated per branch (if/else each with its own call) rather than computed into a shared `len` local first - the shared-local form left a spurious "sub" where the original had "mov,mov,mov" building each branch's own push list.
// working copy - scaffold materialised by --work
// size      210 bytes
// prototype int (__thiscall ?wrap_cent@Buffer@@QAEHPADHHH@Z)(Buffer* this, int8*, int, int, int)
// callers   13   call targets   3
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005DC410 0x005DD020 0x006453E0


int Buffer::wrap_cent(char * a1, int a2, int a3, int a4) {
    if (a1 == 0) {
        return a3;
    }

    BufferStrHeight = 0;
    field_18_ = 0;

    int total = a3;
    char *cur = a1;
    char *next;
    do {
        int width = a4;
        if (cur == 0) {
            next = 0;
        } else {
            BufferStrHeight = 0;
            unsigned int len = strlen(cur);
            next = reinterpret_cast<char *>(find_line_break_l(cur, &width, len));
            if (next != 0) {
                write_cent_l(cur, a2, total, a4, next - cur);
            } else {
                write_cent_l(cur, a2, total, a4, strlen(cur));
            }
        }
        field_18_ = field_18_ + 1;
        if (font1_ == 0) {
            font1_ = FontDefault;
        }
        Font *font = font1_;
        int delta = (font->unk_1_ >= 0) ? (font->height_ + font->unk_1_) : font->line_height_;
        total += delta;
        cur = next;
    } while (next != 0);

    return total;
}

// ORIGINAL: 0x005DDAB0 ?wrap_cent@Buffer@@QAEHPADPAURECT@@@Z 0x005DDAB0-0x005DDBA6 FILE BYTE_EXACT
// symbol    ?wrap_cent@Buffer@@QAEHPADPAUtagRECT@@@Z
//           The one difference from the image's own name: this tree's `RECT`
//           is windows.h's `typedef struct tagRECT`, so the parameter mangles
//           with the real tag. Nothing else moves - the bytes are the body
//           below, and the claim holds.
// size      246 bytes
// prototype int (__thiscall ?wrap_cent@Buffer@@QAEHPADPAURECT@@@Z)(Buffer* this, char*, RECT*)
// callers   ?   call targets   3
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x005DC410 0x005DD020 0x006453E0
//
// LOAD-BEARING SOURCE FORM - the logic below was already right at MISMATCH
// #0; four spelling changes, none of them semantic, carried it to BYTE_EXACT.
// Each was measured on its own, so the next reader can trust the order:
//
//   #0  -> #22  `#pragma function(strlen)`.  /O2 expands `strlen` to
//               `repne scasb`, which burns EDI and ECX inside the loop and
//               costs the register allocator EBP; the original calls
//               0x006453E0.
//   #22 -> #39  `int lineWidth = width;` at the TOP of the loop body rather
//               than inside the non-null arm.  This is what frees EBP: with
//               the copy in the arm, /O2 keeps an EBP frame and spills
//               `width`; hoisted, EBP holds `width` and the frame is the
//               original's one-slot `push ecx`.
//   #39 -> #61  the `write_cent_l` CALL DUPLICATED into both arms instead of
//               a shared call taking a `chunkLen` temporary.  VC6 tail-merges
//               the common `push str; mov ecx,this; call` suffix and leaves
//               the four argument pushes duplicated, which is the original's
//               shape.
//   #61 -> BYTE_EXACT  real class members (`lineCount_`, `font1_`) instead of
//               `reinterpret_cast` through `(char *)this + 0x18` / `+ 0x52c`.
//               Field accesses let VC6 prove the two do not alias, so it
//               hoists the `font1_` load above the `lineCount_` store the way
//               the original schedules it.  The cast form cannot express that.
//
// Also measured, and NOT levers: locals-at-top, locals-at-first-use, no named
// locals at all, and declaring `lineWidth` before or after the other locals -
// all four sat at exactly the divergence of the shape they were built on.

int Buffer::wrap_cent(char * text, RECT * rect) {
    if (text == 0 || rect == 0) {
        return rect->top;
    }
    BufferStrHeight = 0;
    field_18_ = 0;
    int left = rect->left;
    int width = rect->right - left;
    char *str = text;
    int yPos = rect->top;
    char *nextStr;
    for (;;) {
        int lineWidth = width;
        if (str == 0) {
            nextStr = 0;
        } else {
            BufferStrHeight = 0;
            unsigned int fullLen = strlen(str);
            nextStr = reinterpret_cast<char *>(
                find_line_break_l(str, &lineWidth, static_cast<int>(fullLen)));
            if (nextStr != 0) {
                write_cent_l(str, left, yPos, width,
                             static_cast<int>(nextStr - str));
            } else {
                write_cent_l(str, left, yPos, width,
                             static_cast<int>(strlen(str)));
            }
        }
        field_18_ = field_18_ + 1;
        if (this->font1_ == 0) {
            this->font1_ = FontDefault;
        }
        Font *font = this->font1_;
        int advance;
        if (font->unk_1_ >= 0) {
            advance = font->height_ + font->unk_1_;
        } else {
            advance = font->line_height_;
        }
        yPos = yPos + advance;
        str = nextStr;
        if (nextStr == 0) {
            break;
        }
    }
    return yPos;
}

// ORIGINAL: 0x005DB9E0 ?wrap_cent@Buffer@@QAEHPAVFont@@PADHHH@Z 0x005DB9E0-0x005DBAF5
// TRIED: only one source-form tried; original starts with `sub esp,0xc` for locals before its register pushes (no frame pointer at all - ebp is a plain callee-saved register here), which this form doesn't reproduce. Landing the closest structural translation (mnemonic_similarity 0.712).
// size      277 bytes
// prototype int (__thiscall ?wrap_cent@Buffer@@QAEHPAVFont@@PADHHH@Z)(Buffer* this, Font*, int8*, int, int, int)
// callers   1   call targets   3
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005DC410 0x005DD020 0x006453E0


int Buffer::wrap_cent(Font * a1, char * a2, int a3, int a4, int a5) {
    
    Font *savedFont = font1_;
    Font *savedB = font2_;
    Font *savedC = font3_;

    if (a1 != 0 && a1->font_obj_ != 0) {
        font1_ = a1;
        font2_ = 0;
        font3_ = 0;
    }

    int y = a4;
    if (a2 != 0) {
        BufferStrHeight = 0;
        field_18_ = 0;
        char *str = a2;
        char *breakPos;
        do {
            breakPos = 0;
            int outParam = a5;
            if (str != 0) {
                BufferStrHeight = 0;
                unsigned int len = strlen(str);
                breakPos = reinterpret_cast<char *>(find_line_break_l(str, &outParam, len));
                int lineLen;
                if (breakPos == 0) {
                    lineLen = strlen(str);
                }
                else {
                    lineLen = static_cast<int>(breakPos - str);
                }
                write_cent_l(str, a3, y, outParam, lineLen);
            }
            field_18_ += 1;
            if (font1_ == 0) {
                font1_ = FontDefault;
            }
            Font *font = font1_;
            int lineHeight;
            if (font->unk_1_ < 0) {
                lineHeight = font->line_height_;
            }
            else {
                lineHeight = font->height_ + font->unk_1_;
            }
            y += lineHeight;
            str = breakPos;
        } while (breakPos != 0);
    }

    font1_ = savedFont;
    font2_ = savedB;
    font3_ = savedC;
    return y;
}

// ORIGINAL: 0x005DBB00 ?wrap_cent@Buffer@@QAEHPAVFont@@PADHHHH@Z 0x005DBB00-0x005DBBFA FILE
// TRIED: `wrapWidth` (the find_line_break_l out-param) reset from `a5` once before the loop instead of every iteration - worse (16 edits vs 15). Kept the per-iteration reset (matches Ghidra's `param_2 = param_6` inside the do/while) even though the raw disassembly's `[esp+0x24]` write reads like a once-only init; the compiled edit count says otherwise. Missing one `push` near the top (2 saved regs where the original has 3) is the dominant remaining difference.
// size      250 bytes
// prototype int (__thiscall ?wrap_cent@Buffer@@QAEHPAVFont@@PADHHHH@Z)(Buffer* this, Font*, int8*, int, int, int, int)
// callers   0   call targets   3
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005DC410 0x005DD020 0x006453E0

int Buffer::wrap_cent(Font *a1, char *a2, int a3, int a4, int a5, int a6) {
        Font **const fontSlot = &font1_;
    Font *savedFont = font1_;
    Font *savedA = font2_;
    Font *savedB = font3_;

    if (a1 != 0 && a1->font_obj_ != 0) {
        *fontSlot = a1;
        font2_ = 0;
        font3_ = 0;
    }

    int y = a4;
    if (a2 != 0) {
        *reinterpret_cast<int *>(0x9b3a9c) = 0;
        field_18_ = 0;
        char *str = a2;
        char *brk;
        int wrapWidth;
        do {
            wrapWidth = a5;
            if (str == 0) {
                brk = 0;
            } else {
                *reinterpret_cast<int *>(0x9b3a9c) = 0;
                unsigned int len = strlen(str);
                brk = reinterpret_cast<char *>(find_line_break_l(str, &wrapWidth, len));
                if (brk == 0) {
                    len = strlen(str);
                } else {
                    len = brk - str;
                }
                write_cent_l(str, a3, y, a5, len);
            }
            y += a6;
            field_18_ = field_18_ + 1;
            str = brk;
        } while (brk != 0);
    }

    font1_ = savedFont;
    font2_ = savedA;
    font3_ = savedB;
    return y;
}

// ORIGINAL: 0x005DC360 ?wrap_height@Buffer@@QAEHPADH@Z 0x005DC360-0x005DC40A FILE
// TRIED: `if (font->field0 < 0)` compiled to jge with the arms swapped (original is jl, negative-first fallthrough); writing `if (field0 >= 0) {...} else {...}` (arms swapped to match) lifted mnemonic similarity 0.929 -> 0.976. One scheduling-only diff remains (field_14 check's loads reordered around the field_18 increment store, #49) - not chased further.
// working copy - scaffold materialised by --work
// size      170 bytes
// prototype int (__thiscall ?wrap_height@Buffer@@QAEHPADH@Z)(Buffer* this, int8*, int)
// callers   18   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005DC410 0x006453E0


int Buffer::wrap_height(char * a1, int a2) {
        if (a1 == 0) {
        return 0;
    }
    field_18_ = 0;
    BufferStrHeight = 0;
    field_14_ = field_10_;
    int total = 0;
    int width = a2;
    char *str = a1;
    do {
        int len = strlen(str);
        int lineWidth = width;
        str = reinterpret_cast<char *>(find_line_break_l(str, &lineWidth, len));
        BufferStrHeight = 0;
        if (font1_ == 0) {
            font1_ = FontDefault;
        }
        Font *font = font1_;
        int lineHeight;
        if (font->unk_1_ >= 0) {
            lineHeight = font->height_ + font->unk_1_;
        } else {
            lineHeight = font->line_height_;
        }
        total += lineHeight;
        field_18_ = field_18_ + 1;
        if (field_14_ != 0) {
            width -= field_14_;
            field_14_ = 0;
        }
    } while (str != 0);
    return total;
}

// ORIGINAL: 0x005DDEF0 ?wrap_height_flying_flush@Buffer@@QAEHXZ 0x005DDEF0-0x005DDF5F FILE BYTE_EXACT
// size      111 bytes
// prototype int (__thiscall ?wrap_height_flying_flush@Buffer@@QAEHXZ)(Buffer* this)
// callers   2   call targets   0
// kind      
// flags     
// calls     (none)
// working copy - scaffold materialised by --work

int Buffer::wrap_height_flying_flush() {
    if (BufferStrHeight != 0) {
        if (font1_ == 0) {
            font1_ = FontDefault;
        }
        Font *font = font1_;
        int height = (font->unk_1_ >= 0) ? font->height_ + font->unk_1_
                                          : font->line_height_;
        *FlyingTextY += height;
        field_18_++;
    }
    *FlyingTextHeight = 0;
    write_font_slot_ = 0;
    font_slot_ = 0;
    field_514_ = 0;
    markup_pending_ = 0;
    return *FlyingTextY;
}

// ORIGINAL: 0x005DDDE0 ?wrap_height_flying@Buffer@@QAEHPAD@Z 0x005DDDE0-0x005DDEEA
// TRIED: the `_strlen` no-arg-extern + function-pointer-cast idiom (see 0x006141C0) to avoid /O2's strlen intrinsic did not move the divergence - it is register-allocation, not a call shape. The original pushes ebx/esi/edi all in the shared prologue, before the `param_2==0` early return; this source form (an `if` straight after the first branch) defers the `push edi` past that early return since edi is unused on it. Not chased further; landing the closest structural match.
// size      266 bytes
// prototype int (__thiscall ?wrap_height_flying@Buffer@@QAEHPAD@Z)(Buffer* this, int8*)
// callers   3   call targets   3
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005DC410 0x005DC7C0 0x006453E0


int Buffer::wrap_height_flying(char * a1) {
    if (*FlyingTextHeight == 0) {
        return 0;
    }
    if (a1 != 0) {
        if (font1_ == 0) {
            font1_ = FontDefault;
        }
        char *text = a1;
        if (*text != '\0') {
            do {
                int remaining = *FlyingTextHeight - BufferStrHeight;
                text = (char *)find_line_break_l(text, &remaining, strlen(text));
                if (text == 0) {
                    break;
                }
                field_18_ = field_18_ + 1;
                if (font1_ == 0) {
                    font1_ = FontDefault;
                }
                Font *fnt = font1_;
                int lineAdvance;
                if (fnt->unk_1_ < 0) {
                    lineAdvance = fnt->line_height_;
                } else {
                    lineAdvance = fnt->height_ + fnt->unk_1_;
                }
                BufferStrHeight = 0;
                *FlyingTextY = *FlyingTextY + lineAdvance;
                if (field_14_ != 0) {
                    *FlyingTextHeight = *FlyingTextHeight - field_14_;
                    field_14_ = 0;
                }
            } while (true);
            char *prompt = (char *)0x00696CD8;
            if (prompt != 0) {
                unsigned int len = strlen(prompt);
                int width = text_width(prompt, (int)len);
                BufferStrHeight = BufferStrHeight + width;
            }
        }
    }
    return *FlyingTextY;
}

// ORIGINAL: 0x005DE1D0 ?wrap_flying_flush@Buffer@@QAEHXZ 0x005DE1D0-0x005DE2DE
// TRIED: the `_strlen` no-arg-extern lever (0x006141C0) moved the divergence from instruction 0 to instruction 5. The original reads `FlyingTextX`/`FlyingTextY` into ebx/ebp up front, before the text/font-metric checks that guard the call using them; this source form evaluates them only at the call site, which is a hoisting choice VC6 made differently. Not chased further.
// size      270 bytes
// prototype int (__thiscall ?wrap_flying_flush@Buffer@@QAEHXZ)(Buffer* this)
// callers   3   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005DCAE0 0x006453E0


int Buffer::wrap_flying_flush() {
    const char *const text = (const char *)FlyingTextBuf;
    if (text[0] != '\0') {
        unsigned int len1 = strlen(text);
        if (font1_ != 0 && font1_->font_obj_ != 0) {
            unsigned int len2 = strlen(text);
            unsigned int chosen = len1;
            if ((int)len2 < (int)len1) {
                chosen = strlen(text);
            }
            if ((int)chosen >= 0) {
                unsigned int len3 = strlen(text);
                unsigned int finalLen = len1;
                if ((int)len3 < (int)len1) {
                    finalLen = strlen(text);
                }
                if (finalLen != 0) {
                    write_multi_font_raw_l(const_cast<char *>(text), *FlyingTextX, *FlyingTextY, finalLen);
                }
            }
        }
        if (font1_ == 0) {
            font1_ = FontDefault;
        }
        Font *fnt = font1_;
        int lineAdvance = (fnt->unk_1_ < 0) ? fnt->line_height_ : fnt->height_ + fnt->unk_1_;
        *FlyingTextY = *FlyingTextY + lineAdvance;
        field_18_ = field_18_ + 1;
    }
    *FlyingTextBuf = 0;
    *FlyingTextHeight = 0;
    write_font_slot_ = 0;
    font_slot_ = 0;
    field_514_ = 0;
    markup_pending_ = 0;
    return *FlyingTextY;
}

// ORIGINAL: 0x005DDFB0 ?wrap_flying@Buffer@@QAEHPAD@Z 0x005DDFB0-0x005DE1C8 FILE
// TRIED: MISMATCH #2 'push' vs 'sub' - the original reserves its EH/this-save slot with `push ecx` before the other pushes; not chased further. Dropped the `mov eax,0x696cdc; test eax,eax; je` guard entirely (Ghidra already resolved it as always-true and folded it away; a compile-time-nonzero address literal risked our own compiler eliding the branch differently).
// working copy - scaffold materialised by --work
// size      536 bytes
// prototype int (__thiscall ?wrap_flying@Buffer@@QAEHPAD@Z)(Buffer* this, int8*)
// callers   3   call targets   6
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005DC410 0x005DC7C0 0x005DCAE0 0x006453E0 0x00645470 0x00645930

int Buffer::wrap_flying(char * a1) {
    
    if (*FlyingTextHeight == 0) {
        return 0;
    }
    if (a1 == 0) {
        return *FlyingTextY;
    }
    if (*a1 == 0) {
        return *FlyingTextY;
    }

    char *cur = a1;
    char *buf = (char *)FlyingTextBuf;
    for (;;) {
        if (buf[0] != 0) {
            strcat(buf, (const char *)SpaceText);
            unsigned int len = strlen((const char *)SpaceTextTail);
            int w = this->text_width((char *)SpaceTextTail, len);
            BufferStrHeight += w;
        }

        char *brk = cur;
        if (BufferStrHeight < *FlyingTextHeight) {
            unsigned int len2 = strlen(cur);
            int avail = *FlyingTextHeight - BufferStrHeight;
            brk = (char *)this->find_line_break_l(cur, &avail, len2);
        }
        if (brk == 0) {
            break;
        }

        unsigned int buflen = strlen(buf);
        memcpy(buf + buflen, cur, brk - cur);
        buf[buflen + (brk - cur)] = 0;

        int saved_94 = *FlyingTextY;
        int saved_90 = *FlyingTextX;
        unsigned int flen = strlen(buf);

        if ((int)font1_ != 0 && *(int *)(reinterpret_cast<int>(font1_) + 8) != 0) {
            unsigned int a = strlen(buf);
            unsigned int b = flen;
            if ((int)a < (int)b) {
                b = strlen(buf);
            }
            if ((int)b >= 0) {
                unsigned int c = strlen(buf);
                if ((int)c < (int)flen) {
                    flen = strlen(buf);
                }
                if (flen != 0) {
                    this->write_multi_font_raw_l(buf, saved_90, saved_94, flen);
                }
            }
        }

        buf[0] = 0;
        BufferStrHeight = 0;
        field_18_ += 1;
        if (font1_ == 0) {
            font1_ = FontDefault;
        }
        Font *rec = font1_;
        int add;
        if (rec->unk_1_ < 0) {
            add = rec->line_height_;
        } else {
            add = rec->height_ + rec->unk_1_;
        }
        *FlyingTextY += add;

        cur = brk;
        if (field_14_ != 0) {
            *FlyingTextHeight -= field_14_;
            *FlyingTextX += field_14_;
            field_14_ = 0;
        }
    }

    strcat(buf, cur);
    return *FlyingTextY;
}

// ORIGINAL: 0x005DB580 ?write_l@Buffer@@QAEHPAVFont@@PADHHH@Z 0x005DB580-0x005DB65F FILE BYTE_EXACT
// working copy - scaffold materialised by --work
// size      223 bytes
// prototype int (__thiscall ?write_l@Buffer@@QAEHPAVFont@@PADHHH@Z)(Buffer* this, Font*, int8*, int, int, int)
// callers   1   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005DCAE0 0x006453E0
// LEVER: SAVE THE THREE FONT SLOTS AS ONE BLOCK, not as three locals.
//        `BufferFontTriple savedFonts = *fonts;` through a pointer to the
//        slot trio is what makes the image open with
//        `lea esi,[edi+0x52c]` and load the triple off it - three saves as
//        separate `Font *` locals fold back into direct member loads and
//        the register allocation diverges from instruction 2 on (4/83).
//        Block form: 83/83. The struct names the members; nothing is punned
//        through a type that is not there.

int Buffer::write_l(Font * font, char * text, int x, int y, int maxLen) {
    BufferFontTriple *const fonts = reinterpret_cast<BufferFontTriple *>(&font1_);
    BufferFontTriple savedFonts = *fonts;
    int result;
    if (font != 0 && font->font_obj_ != 0) {
        font1_ = font;
        font2_ = 0;
        font3_ = 0;
    }
    int len = maxLen;

    if (text == 0) {
        result = x;
    } else if (fonts->primary == 0 || fonts->primary->font_obj_ == 0) {
        result = 3;
    } else {
        if (((int)strlen(text) < len ? (int)strlen(text) : len) < 0) {
            goto nothing_drawn;
        }
        len = (int)strlen(text) < len ? (int)strlen(text) : len;
        if (len == 0) {
            // Both length exits share one block in the original: the negative
            // case branches forward into the zero case's assignment.
nothing_drawn:
            result = x;
        } else {
            result = write_multi_font_raw_l(text, x, y, len);
        }
    }

    *fonts = savedFonts;
    return result;
}

// ORIGINAL: 0x005DBC50 ?wrap_right@Buffer@@QAEHPAVFont@@PADHHH@Z 0x005DBC50-0x005DBCA9 FILE
// TRIED: save/swap/restore of the triple at this+0x52c is right (matches Ghidra's field layout and the 4-arg wrap_right overload target); divergence is register-allocation scheduling in the push/pop prologue, not semantics - mnemonic_similarity 0.91, first divergence at insn #0.
// working copy - scaffold materialised by --work
// size      89 bytes
// prototype int (__thiscall ?wrap_right@Buffer@@QAEHPAVFont@@PADHHH@Z)(Buffer* this, Font*, int8*, int, int, int)
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005DDBB0

int Buffer::wrap_right(Font * a1, char * a2, int a3, int a4, int a5) {
        int *const triple = reinterpret_cast<int *>(&font1_);
    int saved0 = triple[0];
    int saved1 = triple[1];
    int saved2 = triple[2];
    if (a1 != 0 && a1->font_obj_ != 0) {
        triple[0] = reinterpret_cast<int>(a1);
        triple[1] = 0;
        triple[2] = 0;
    }
    int result = wrap_right(a2, a3, a4, a5);
    triple[0] = saved0;
    triple[1] = saved1;
    triple[2] = saved2;
    return result;
}

// ORIGINAL: 0x005DBCB0 ?wrap_right@Buffer@@QAEHPAVFont@@PADH@Z 0x005DBCB0-0x005DBCFF
// size      79 bytes
// prototype int (__thiscall ?wrap_right@Buffer@@QAEHPAVFont@@PADH@Z)(Buffer* this, Font*, int8*, int)
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005DDC90
// 0x005DBCB0  ?wrap_right@Buffer@@QAEHPAVFont@@PADH@Z  ->  ?wrap_right@Buffer@@QAEHPAVFont@@PADH@Z
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.


int Buffer::wrap_right(Font * a1, char * a2, int a3) {
    Font *savedFont = font1_;
    Font *savedFont2 = font2_;
    Font *savedFont3 = font3_;
    if (a1 != 0 && a1->font_obj_ != 0) {
        font1_ = a1;
        font2_ = 0;
        font3_ = 0;
    }
    int result = wrap_right(a2, a3);
    font1_ = savedFont;
    font2_ = savedFont2;
    font3_ = savedFont3;
    return result;
}

// ORIGINAL: 0x005DB810 ?write_cent_l@Buffer@@QAEHPAVFont@@PADHHH@Z 0x005DB810-0x005DB869 FILE
// TRIED: save/override font1_-3_ around the 4-arg overload, then restore
// working copy - scaffold materialised by --work
// size      89 bytes
// prototype int (__thiscall ?write_cent_l@Buffer@@QAEHPAVFont@@PADHHH@Z)(Buffer* this, Font*, int8*, int, int, int)
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005DD250

int Buffer::write_cent_l(Font * a1, char * a2, int a3, int a4, int a5) {
    Font *savedFont1 = font1_;
    Font *savedFont2 = font2_;
    Font *savedFont3 = font3_;
    if (a1 != 0 && a1->font_obj_ != 0) {
        font1_ = a1;
        font2_ = 0;
        font3_ = 0;
    }
    int result = write_cent_l(a2, a3, a4, a5);
    font1_ = savedFont1;
    font2_ = savedFont2;
    font3_ = savedFont3;
    return result;
}

// ORIGINAL: 0x005DB8D0 ?write_right_l@Buffer@@QAEHPAVFont@@PADHHH@Z 0x005DB8D0-0x005DB929 FILE
// TRIED: nothing beyond the direct save/swap/delegate/restore transcription; diverges at #0 (push vs mov), likely register/local allocation order
// working copy - scaffold materialised by --work
// size      89 bytes
// prototype int (__thiscall ?write_right_l@Buffer@@QAEHPAVFont@@PADHHH@Z)(Buffer* this, Font*, int8*, int, int, int)
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005DD3B0

int Buffer::write_right_l(Font * a1, char * a2, int a3, int a4, int a5) {
    Font *savedFont1 = font1_;
    Font *savedFont2 = font2_;
    Font *savedFont3 = font3_;
    if (a1 != 0 && a1->font_obj_ != 0) {
        font1_ = a1;
        font2_ = 0;
        font3_ = 0;
    }
    int result = write_right_l(a2, a3, a4, a5);
    font1_ = savedFont1;
    font2_ = savedFont2;
    font3_ = savedFont3;
    return result;
}

// ORIGINAL: 0x005DB780 ?write_cent_l@Buffer@@QAEHPAVFont@@PADHHHHH@Z 0x005DB780-0x005DB806 FILE
// working copy - scaffold materialised by --work
// size      134 bytes
// prototype int (__thiscall ?write_cent_l@Buffer@@QAEHPAVFont@@PADHHHHH@Z)(Buffer* this, Font*, int8*, int, int, int, int, int)
// callers   1   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005DD130

int Buffer::write_cent_l(Font * a1, char * a2, int a3, int a4, int a5, int a6, int a7) {
    Font *saved_font1 = font1_;
    Font *saved_font2 = font2_;
    Font *saved_font3 = font3_;
    if (a1 != 0 && a1->font_obj_ != 0) {
        font1_ = a1;
        font2_ = 0;
        font3_ = 0;
    }
    RECT rect;
    rect.left = a3;
    rect.top = a4;
    rect.right = a3 + a5;
    rect.bottom = a4 + a6;
    int result = write_cent_l(a2, &rect, a7);
    font1_ = saved_font1;
    font2_ = saved_font2;
    font3_ = saved_font3;
    return result;
}

// ORIGINAL: 0x005DD3B0 ?write_right_l@Buffer@@QAEHPADHHH@Z 0x005DD3B0-0x005DD44F FILE
// TRIED: literal Ghidra-order translation (a1==0 early-return, nested strlen/text_width) - MISMATCH #1 push/mov, original pushes ebx/esi/edi unconditionally before the a1==0 test; register-allocation shape only.
// working copy - scaffold materialised by --work
// size      159 bytes
// prototype int (__thiscall ?write_right_l@Buffer@@QAEHPADHHH@Z)(Buffer* this, int8*, int, int, int)
// callers   12   call targets   3
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005DC7C0 0x005DCAE0 0x006453E0

int Buffer::write_right_l(char * a1, int a2, int a3, int a4) {
    if (a1 == 0) {
        return a2;
    }
    if ((font1_ != 0) && (font1_->font_obj_ != 0)) {
        int len1 = strlen(a1);
        int w = a4;
        if (len1 < a4) {
            w = strlen(a1);
        }
        if (w >= 0) {
            int len2 = strlen(a1);
            if (len2 < a4) {
                a4 = strlen(a1);
            }
            if (a4 != 0) {
                int tw = text_width(a1, a4);
                return write_multi_font_raw_l(a1, a2 - tw, a3, a4);
            }
        }
        return a2;
    }
    return 3;
}

// ORIGINAL: 0x005DD0E0 ?write_cent_l@Buffer@@QAEHPADHHHHH@Z 0x005DD0E0-0x005DD123 BYTE_EXACT
// size      67 bytes
// prototype int (__thiscall ?write_cent_l@Buffer@@QAEHPADHHHHH@Z)(Buffer* this, int8*, int, int, int, int, int)
// callers   2   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005DD130
// 0x005DD0E0  ?write_cent_l@Buffer@@QAEHPADHHHHH@Z  ->  ?write_cent_l@Buffer@@QAEHPADHHHHH@Z
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.


int Buffer::write_cent_l(char *a1, int a2, int a3, int a4, int a5, int a6) {
    RECT r;
    r.left = a2;
    r.top = a3;
    r.right = a2 + a4;
    r.bottom = a3 + a5;
    return write_cent_l(a1, &r, a6);
}


// ORIGINAL: 0x005DB660 ?write_l@Buffer@@QAEHPAVFont@@PADPAURECT@@H@Z 0x005DB660-0x005DB6B4 FILE
// symbol    ?write_l@Buffer@@QAEHPAVFont@@PADPAUtagRECT@@H@Z
// size      84 bytes
// prototype int (__thiscall ?write_l@Buffer@@QAEHPAVFont@@PADPAURECT@@H@Z)(Buffer* this, Font*, int8*, RECT*, int)
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005DCF40
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x005DB660
// measured tier  MISMATCH
// divergence     0
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/005db660/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?write_l@Buffer@@QAEHPAVFont@@PADPAURECT@@H@Z  at 0x005DB660  (84 bytes)
//
// A VERIFICATION ARTIFACT, not product source: classes are opaque and
// globals are bound to fixed addresses, because both are byte-visible
// and both differ from the style src/ is written in.
//
// VC6 DIALECT - this must compile under cl 12.00.8168, which is the
// only compiler this project builds with. Avoid: auto, nullptr, constexpr,
// static_assert, enum class, range-for, lambdas, long long, <cstdint>,
// and declaring `int i` twice in one function (VC6 leaks for-scope).
// static_cast/reinterpret_cast are fine and are the right spelling.
//
// SOURCE-FORM RULES, each one learned by a fan-out agent that lost
// attempts to it. They are here rather than in a prompt so the next
// agent does not rediscover them:
//
//  * A ternary is not an if. `x ? a : b` lowers to `jne` with the arms
//    swapped; `if (x) {...}` gives `je`.
//  * VC6 NEVER hoists a same-polarity guard's body out of line. To get
//    two special cases branching FAR away, write them negated and
//    NESTED - `if (a != X) { if (a != Y) { return D; } ...; }` - not as
//    sequential guard clauses, which inline each body instead.
//  * VC6 rejects `__thiscall` on a free function pointer (C4234). For
//    an indirect virtual call use the generated VCall shim below; for a
//    thiscall function POINTER, take a member-function pointer of a
//    dummy class instead of spelling the convention.
//  * Loop form is visible: while / do-while / for lower differently,
//    and so does counting up versus down.
//  * If the original dedicates a callee-saved register to a constant
//    across the whole body (an extra `push ebx`/`push edi` in the
//    prologue), it had enough register pressure to do so. That is a
//    hard case - the tool reports a similarity ratio so you can tell a
//    near miss from a wrong body.


int Buffer::write_l(Font *a1, char *a2, RECT *a3, int a4) {
        Font *f0 = font1_;
    Font *f1 = font2_;
    Font *f2 = font3_;
    if (a1 != 0 && a1->font_obj_ != 0) {
        font1_ = a1;
        font2_ = 0;
        font3_ = 0;
    }
    int result = write_l(a2, a3, a4);
    font1_ = f0;
    font2_ = f1;
    font3_ = f2;
    return result;
}

// ORIGINAL: 0x005DB720 ?write_cent_l@Buffer@@QAEHPAVFont@@PADPAURECT@@H@Z 0x005DB720-0x005DB774 FILE
// symbol    ?write_cent_l@Buffer@@QAEHPAVFont@@PADPAUtagRECT@@H@Z
// size      84 bytes
// prototype int (__thiscall ?write_cent_l@Buffer@@QAEHPAVFont@@PADPAURECT@@H@Z)(Buffer* this, Font*, char *, RECT*, int)
// callers   6   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005DD130
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x005DB720
// measured tier  MISMATCH
// divergence     0
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/005db720/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?write_cent_l@Buffer@@QAEHPAVFont@@PADPAURECT@@H@Z  at 0x005DB720  (84 bytes)
//
// A VERIFICATION ARTIFACT, not product source: classes are opaque and
// globals are bound to fixed addresses, because both are byte-visible
// and both differ from the style src/ is written in.
//
// VC6 DIALECT - this must compile under cl 12.00.8168, which is the
// only compiler this project builds with. Avoid: auto, nullptr, constexpr,
// static_assert, enum class, range-for, lambdas, long long, <cstdint>,
// and declaring `int i` twice in one function (VC6 leaks for-scope).
// static_cast/reinterpret_cast are fine and are the right spelling.
//
// SOURCE-FORM RULES, each one learned by a fan-out agent that lost
// attempts to it. They are here rather than in a prompt so the next
// agent does not rediscover them:
//
//  * A ternary is not an if. `x ? a : b` lowers to `jne` with the arms
//    swapped; `if (x) {...}` gives `je`.
//  * VC6 NEVER hoists a same-polarity guard's body out of line. To get
//    two special cases branching FAR away, write them negated and
//    NESTED - `if (a != X) { if (a != Y) { return D; } ...; }` - not as
//    sequential guard clauses, which inline each body instead.
//  * VC6 rejects `__thiscall` on a free function pointer (C4234). For
//    an indirect virtual call use the generated VCall shim below; for a
//    thiscall function POINTER, take a member-function pointer of a
//    dummy class instead of spelling the convention.
//  * Loop form is visible: while / do-while / for lower differently,
//    and so does counting up versus down.
//  * If the original dedicates a callee-saved register to a constant
//    across the whole body (an extra `push ebx`/`push edi` in the
//    prologue), it had enough register pressure to do so. That is a
//    hard case - the tool reports a similarity ratio so you can tell a
//    near miss from a wrong body.

int Buffer::write_cent_l(Font* a1, char * a2, RECT* a3, int a4) {
    Font **p = reinterpret_cast<Font **>(reinterpret_cast<char *>(this) + 0x52c);
    Font *old0 = p[0];
    Font *old1 = p[1];
    Font *old2 = p[2];
    if (a1 != 0 && a1->font_obj_ != 0) {
        p[0] = a1;
        p[1] = 0;
        p[2] = 0;
    }
    int result = write_cent_l(a2, a3, a4);
    p[0] = old0;
    p[1] = old1;
    p[2] = old2;
    return result;
}

// ORIGINAL: 0x005DBC00 ?wrap_cent@Buffer@@QAEHPAVFont@@PADPAURECT@@@Z 0x005DBC00-0x005DBC4F FILE
// symbol    ?wrap_cent@Buffer@@QAEHPAVFont@@PADPAUtagRECT@@@Z
// size      79 bytes
// prototype int (__thiscall ?wrap_cent@Buffer@@QAEHPAVFont@@PADPAURECT@@@Z)(Buffer* this, Font*, char *, RECT*)
// callers   4   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005DDAB0
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x005DBC00
// measured tier  MISMATCH
// divergence     0
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/005dbc00/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?wrap_cent@Buffer@@QAEHPAVFont@@PADPAURECT@@@Z  at 0x005DBC00  (79 bytes)
//
// A VERIFICATION ARTIFACT, not product source: classes are opaque and
// globals are bound to fixed addresses, because both are byte-visible
// and both differ from the style src/ is written in.
//
// VC6 DIALECT - this must compile under cl 12.00.8168, which is the
// only compiler this project builds with. Avoid: auto, nullptr, constexpr,
// static_assert, enum class, range-for, lambdas, long long, <cstdint>,
// and declaring `int i` twice in one function (VC6 leaks for-scope).
// static_cast/reinterpret_cast are fine and are the right spelling.
//
// SOURCE-FORM RULES, each one learned by a fan-out agent that lost
// attempts to it. They are here rather than in a prompt so the next
// agent does not rediscover them:
//
//  * A ternary is not an if. `x ? a : b` lowers to `jne` with the arms
//    swapped; `if (x) {...}` gives `je`.
//  * VC6 NEVER hoists a same-polarity guard's body out of line. To get
//    two special cases branching FAR away, write them negated and
//    NESTED - `if (a != X) { if (a != Y) { return D; } ...; }` - not as
//    sequential guard clauses, which inline each body instead.
//  * VC6 rejects `__thiscall` on a free function pointer (C4234). For
//    an indirect virtual call use the generated VCall shim below; for a
//    thiscall function POINTER, take a member-function pointer of a
//    dummy class instead of spelling the convention.
//  * Loop form is visible: while / do-while / for lower differently,
//    and so does counting up versus down.
//  * If the original dedicates a callee-saved register to a constant
//    across the whole body (an extra `push ebx`/`push edi` in the
//    prologue), it had enough register pressure to do so. That is a
//    hard case - the tool reports a similarity ratio so you can tell a
//    near miss from a wrong body.


int Buffer::wrap_cent(Font* a1, char * a2, RECT* a3) {
        Font *save0 = font1_;
    Font *save1 = font2_;
    Font *save2 = font3_;
    if (a1 != 0 && a1->font_obj_ != 0) {
        font1_ = a1;
        font2_ = 0;
        font3_ = 0;
    }
    int result = wrap_cent(a2, a3);
    font1_ = save0;
    font2_ = save1;
    font3_ = save2;
    return result;
}

// ORIGINAL: 0x005DB990 ?wrap@Buffer@@QAEHPAVFont@@PADPAURECT@@@Z 0x005DB990-0x005DB9DF FILE
// symbol    ?wrap@Buffer@@QAEHPAVFont@@PADPAUtagRECT@@@Z
// size      79 bytes
// prototype int (__thiscall ?wrap@Buffer@@QAEHPAVFont@@PADPAURECT@@@Z)(Buffer* this, Font*, int8*, RECT*)
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005DD530
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x005DB990
// measured tier  MISMATCH
// divergence     0
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/005db990/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?wrap@Buffer@@QAEHPAVFont@@PADPAURECT@@@Z  at 0x005DB990  (79 bytes)
//
// A VERIFICATION ARTIFACT, not product source: classes are opaque and
// globals are bound to fixed addresses, because both are byte-visible
// and both differ from the style src/ is written in.
//
// The VC6 dialect limits and the source-form rules used to live here.
// They are knowledge, not scaffolding, so they now live in the agent
// system prompt (mizuchi.yaml, plugins.claude-runner.systemPrompt),
// where they can be edited without regenerating anything and are in
// context from the first token rather than behind a file read. This
// emitter computes declarations; it does not carry lessons.

int Buffer::wrap(Font *a1, char *a2, RECT *a3) {
    Font **p = &font1_;
    Font *saved0 = *p;
    Font *saved1 = font2_;
    Font *saved2 = font3_;
    if (a1 && a1->font_obj_) {
        *p = a1;
        font2_ = 0;
        font3_ = 0;
    }
    int result = wrap(a2, a3);
    *p = saved0;
    font2_ = saved1;
    font3_ = saved2;
    return result;
}

// ORIGINAL: 0x005DDDA0 ?wrap_height_flying@Buffer@@QAEHPADH@Z 0x005DDDA0-0x005DDDD2 FILE
// size      50 bytes
// prototype int (__thiscall ?wrap_height_flying@Buffer@@QAEHPADH@Z)(Buffer* this, int8*, int)
// callers   2   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005DDDE0
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x005DDDA0
// measured tier  MISMATCH
// divergence     9
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/005ddda0/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?wrap_height_flying@Buffer@@QAEHPADH@Z  at 0x005DDDA0  (50 bytes)
//
// A VERIFICATION ARTIFACT, not product source: classes are opaque and
// globals are bound to fixed addresses, because both are byte-visible
// and both differ from the style src/ is written in.
//
// VC6 DIALECT - this must compile under cl 12.00.8168, which is the
// only compiler this project builds with. Avoid: auto, nullptr, constexpr,
// static_assert, enum class, range-for, lambdas, long long, <cstdint>,
// and declaring `int i` twice in one function (VC6 leaks for-scope).
// static_cast/reinterpret_cast are fine and are the right spelling.
//
// SOURCE-FORM RULES, each one learned by a fan-out agent that lost
// attempts to it. They are here rather than in a prompt so the next
// agent does not rediscover them:
//
//  * A ternary is not an if. `x ? a : b` lowers to `jne` with the arms
//    swapped; `if (x) {...}` gives `je`.
//  * VC6 NEVER hoists a same-polarity guard's body out of line. To get
//    two special cases branching FAR away, write them negated and
//    NESTED - `if (a != X) { if (a != Y) { return D; } ...; }` - not as
//    sequential guard clauses, which inline each body instead.
//  * VC6 rejects `__thiscall` on a free function pointer (C4234). For
//    an indirect virtual call use the generated VCall shim below; for a
//    thiscall function POINTER, take a member-function pointer of a
//    dummy class instead of spelling the convention.
//  * Loop form is visible: while / do-while / for lower differently,
//    and so does counting up versus down.
//  * If the original dedicates a callee-saved register to a constant
//    across the whole body (an extra `push ebx`/`push edi` in the
//    prologue), it had enough register pressure to do so. That is a
//    hard case - the tool reports a similarity ratio so you can tell a
//    near miss from a wrong body.


int Buffer::wrap_height_flying(char *a1, int a2) {
        register char *text = a1;
    int value = field_10_;
    field_18_ = 0;
    field_14_ = value;
    markup_pending_ = 0;
    *FlyingTextY = 0;
    BufferStrHeight = 0;
    *FlyingTextHeight = a2;
    return wrap_height_flying(text);
}

// ORIGINAL: 0x005DDC90 ?wrap_right@Buffer@@QAEHPADH@Z 0x005DDC90-0x005DDD91 FILE
// TRIED: `int *a2` second parameter - the mangled name is `PADH` (char*, int), not `PADPAH`, so the second argument really is `int` and the disassembly's [eax]/[eax+4]/[eax+8] reads are the body reinterpreting that int as an `int*` internally, not a real pointer parameter.
// size      257 bytes
// prototype int (__thiscall ?wrap_right@Buffer@@QAEHPADH@Z)(Buffer* this, char*, int)
// callers   1   call targets   3
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x005DC410 0x005DD3B0 0x006453E0
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?wrap_right@Buffer@@QAEHPADH@Z  at 0x005DDC90  (257 bytes)
//
// A VERIFICATION ARTIFACT, not product source: classes are opaque and
// globals are bound to fixed addresses, because both are byte-visible
// and both differ from the style src/ is written in.
//
// The VC6 dialect limits and the source-form rules used to live here.
// They are knowledge, not scaffolding, so they now live in the agent
// system prompt (mizuchi.yaml, plugins.claude-runner.systemPrompt),
// where they can be edited without regenerating anything and are in
// context from the first token rather than behind a file read. This
// emitter computes declarations; it does not carry lessons.


int Buffer::wrap_right(char * a1, int a2) {
    int *rec = reinterpret_cast<int *>(a2);
    if (a1 != 0 && rec != 0) {
        BufferStrHeight = 0;
        int oldField10 = field_10_;
        field_18_ = 0;
        field_14_ = oldField10;
        int start = rec[0];
        int width = rec[2] - start;
        int total = rec[1];
        char *cur = a1;
        char *found;
        do {
            if (cur == 0) {
                found = 0;
            } else {
                BufferStrHeight = 0;
                int localWidth = width;
                unsigned int len = strlen(cur);
                found = reinterpret_cast<char *>(find_line_break_l(cur, &localWidth, (int)len));
                int segLen;
                if (found == 0) {
                    segLen = (int)strlen(cur);
                } else {
                    segLen = (int)(found - cur);
                }
                write_right_l(cur, start, total, segLen);
            }
            field_18_ = field_18_ + 1;
            if (font1_ == 0) {
                font1_ = FontDefault;
            }
            Font *f = font1_;
            int heightAdd;
            if (f->unk_1_ < 0) {
                heightAdd = f->line_height_;
            } else {
                heightAdd = f->height_ + f->unk_1_;
            }
            total = total + heightAdd;
            int oldField14 = field_14_;
            if (oldField14 != 0) {
                field_14_ = 0;
                width = width - oldField14;
            }
            cur = found;
        } while (found != 0);
        return total;
    }
    return rec[1];
}

// ORIGINAL: 0x005DDF60 ?wrap_flying@Buffer@@QAEHPADHHH@Z 0x005DDF60-0x005DDFA6 BYTE_EXACT
// size      70 bytes
// prototype int (__thiscall ?wrap_flying@Buffer@@QAEHPADHHH@Z)(Buffer* this, int8*, int, int, int)
// callers   3   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005DDFB0
// 0x005DDF60  ?wrap_flying@Buffer@@QAEHPADHHH@Z  ->  ?wrap_flying@Buffer@@QAEHPADHHH@Z
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.


int Buffer::wrap_flying(char *a1, int a2, int a3, int a4) {
    uint32_t saved = field_10_;
    field_18_ = 0;
    field_14_ = saved;
    *FlyingTextX = a2;
    BufferStrHeight = 0;
    *FlyingTextY = a3;
    *FlyingTextHeight = a4;
    markup_pending_ = 0;
    *reinterpret_cast<uint8_t *>(FlyingTextBuf) = 0;
    return wrap_flying(a1);
}

// ORIGINAL: 0x005E2330 ?put_pixel@Buffer@@QAEHHHH@Z 0x005E2330-0x005E2457
// TRIED: the two indirect calls are DirectDraw surface Lock/Unlock through the surface pointer stored at surface_ - stdcall COM methods, not thiscall members of this class, so they are reached through a plain typed function pointer read from the surface's own vtable rather than a member-pointer shim; this matches the original's own indirect-call instruction shape. Ruled out a named lock-descriptor struct in favour of a raw byte buffer sized 0x6c with the two fields it actually reads (lpSurface at +0x10, pitch at +0x24) addressed by offset, since the real DDSURFACEDESC layout is not declared here. Landing the closest MISMATCH; divergence starts at instruction #5, `push` in the original for the DDraw lock call setup versus `xor` in the rebuilt.
// size      295 bytes
// prototype int (__thiscall ?put_pixel@Buffer@@QAEHHHH@Z)(Buffer* this, int, int, int)
// callers   1   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x005E23C3 0x005E243F


int Buffer::put_pixel(int a1, int a2, int a3) {
  char *local_48;
  int result;
  DDSURFACEDESC description;
  void *surface;

  int cur = reinterpret_cast<int>(dib_bits_);
  if (cur == 0 && surface_ == 0) {
    return 7;
  }
  if (a1 < 0) {
    return 0;
  }
  if (a2 < 0) {
    return 0;
  }
  if (dib_.bmiHeader.biWidth <= a1) {
    return 0;
  }
  if (-dib_.bmiHeader.biHeight <= a2) {
    return 0;
  }

  surface = (void *)surface_;
  if (surface == 0) {
    locked_bits_ = (void *)cur;
    if (cur == 0) {
      goto after_write;
    }
    surface_lock_count_ = surface_lock_count_ + 1;
    local_48 = (char *)cur;
  } else if (locked_bits_ == 0) {
    typedef int(__stdcall * LockFn)(void *, void *, void *, int, int);
    LockFn lock;
    description.dwSize = sizeof(description);
    lock = (LockFn)(*(void ***)surface)[0x64 / 4];
    result = lock(surface, 0, &description, 1, 0);
    if (result != 0) {
      goto after_write;
    }
    stride_ = description.lPitch;
    surface_lock_count_ = surface_lock_count_ + 1;
    locked_bits_ = description.lpSurface;
    local_48 = (char *)locked_bits_;
  } else {
    surface_lock_count_ = surface_lock_count_ + 1;
    local_48 = (char *)locked_bits_;
  }

  if (local_48 != 0) {
    unsigned char *p = (unsigned char *)(stride_ * a2 + local_48 + a1);
    if (p != 0) {
      *p = (unsigned char)a3;
    }
  }

after_write:
  surface = (void *)surface_;
  if (surface == 0) {
    int n = surface_lock_count_ - 1;
    surface_lock_count_ = n;
    if (n < 1) {
      locked_bits_ = nullptr;
      surface_lock_count_ = 0;
      return 0;
    }
  } else {
    int n = surface_lock_count_ - 1;
    surface_lock_count_ = n;
    if (locked_bits_ != 0 && n < 1) {
      typedef int(__stdcall * UnlockFn)(void *, int);
      UnlockFn unlock = (UnlockFn)(*(void ***)surface)[0x80 / 4];
      unlock(surface, (int)(uintptr_t)locked_bits_);
      locked_bits_ = nullptr;
      surface_lock_count_ = 0;
    }
  }
  return 0;
}

// ORIGINAL: 0x005DDBB0 ?wrap_right@Buffer@@QAEHPADHHH@Z 0x005DDBB0-0x005DDC8F FILE
// TRIED: source order already matches Ghidra's (g=0; field18=0; field14=field10; y=a3); the original's `ebx/ebp` push order ahead of the field writes looks like pure register-scheduling, not reachable by reordering the C statements that produced it.
// working copy - scaffold materialised by --work
// size      223 bytes
// prototype int (__thiscall ?wrap_right@Buffer@@QAEHPADHHH@Z)(Buffer* this, int8*, int, int, int)
// callers   1   call targets   3
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005DC410 0x005DD3B0 0x006453E0

int Buffer::wrap_right(char * a1, int a2, int a3, int a4) {
        if (a1 == 0) {
        return a3;
    }
    BufferStrHeight = 0;
    field_18_ = 0;
    field_14_ = field_10_;
    int y = a3;
    char *text = a1;
    int width = a4;
    char *brk;
    do {
        a3 = width;
        if (text == 0) {
            brk = 0;
        } else {
            int len;
            BufferStrHeight = 0;
            unsigned int slen = strlen(text);
            brk = reinterpret_cast<char *>(find_line_break_l(text, &a3, slen));
            if (brk == 0) {
                len = strlen(text);
            } else {
                len = (int)(brk - text);
            }
            write_right_l(text, a2, y, len);
        }
        field_18_ = field_18_ + 1;
        if (font1_ == 0) {
            font1_ = FontDefault;
        }
        Font *font = font1_;
        int lineH;
        if (font->unk_1_ < 0) {
            lineH = font->line_height_;
        } else {
            lineH = font->height_ + font->unk_1_;
        }
        y = y + lineH;
        int f14 = field_14_;
        if (f14 != 0) {
            field_14_ = 0;
            width = width - f14;
        }
        text = brk;
    } while (brk != 0);
    return y;
}

// ORIGINAL: 0x005DD530 ?wrap@Buffer@@QAEHPADPAURECT@@@Z 0x005DD530-0x005DD730 FILE
// symbol    ?wrap@Buffer@@QAEHPADPAUtagRECT@@@Z
// TRIED: MISMATCH #2 'push' vs 'sub' - direct transcription of the Ghidra do-while, using font1_ (0x52C) and its font_obj_ (+8); the entry-block stack-frame allocation shape diverges from the original almost immediately, not chased further.
// working copy - scaffold materialised by --work
// size      512 bytes
// prototype int (__thiscall ?wrap@Buffer@@QAEHPADPAURECT@@@Z)(Buffer* this, int8*, RECT*)
// callers   7   call targets   3
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005DC410 0x005DCAE0 0x006453E0

int Buffer::wrap(char * a1, RECT * a2) {
    
    if (a1 != 0 && a2 != 0) {
        BufferStrHeight = 0;
        field_18_ = 0;
        field_14_ = field_10_;

        int x = a2->left;
        int y = a2->top;
        int width = a2->right - a2->left;
        char *str = a1;
        char *nextStr;

        do {
            int lineResult;
            if (str == 0) {
                nextStr = 0;
            } else {
                BufferStrHeight = 0;
                int breakPos = width;
                nextStr = reinterpret_cast<char *>(
                    this->find_line_break_l(str, &breakPos, static_cast<int>(strlen(str))));

                Font *font = font1_;
                bool hasFont = (font != 0) && (font->font_obj_ != 0);

                if (nextStr == 0) {
                    unsigned int len = strlen(str);
                    if (!hasFont) {
                        lineResult = 3;
                    } else {
                        unsigned int a = strlen(str);
                        unsigned int b = len;
                        if (static_cast<int>(a) < static_cast<int>(len))
                            b = strlen(str);
                        if (static_cast<int>(b) >= 0) {
                            b = strlen(str);
                            if (static_cast<int>(b) < static_cast<int>(len))
                                len = strlen(str);
                            if (len != 0)
                                lineResult = this->write_multi_font_raw_l(str, x, y, len);
                        }
                    }
                } else {
                    unsigned int len = static_cast<unsigned int>(nextStr - str);
                    if (!hasFont) {
                        *BufferWriteStatus = 3;
                        lineResult = *BufferWriteStatus;
                    } else {
                        unsigned int a = strlen(str);
                        unsigned int b = len;
                        if (static_cast<int>(a) < static_cast<int>(len))
                            b = strlen(str);
                        if (static_cast<int>(b) >= 0) {
                            b = strlen(str);
                            if (static_cast<int>(b) < static_cast<int>(len))
                                len = strlen(str);
                            if (len != 0)
                                lineResult = this->write_multi_font_raw_l(str, x, y, len);
                        }
                    }
                }
            }

            *BufferWriteStatus = lineResult;

            if (font1_ == 0)
                font1_ = reinterpret_cast<Font *>(FontDefault);

            int *fontData = reinterpret_cast<int *>(font1_);
            int lineHeight;
            if (*fontData < 0)
                lineHeight = fontData[3];
            else
                lineHeight = fontData[4] + *fontData;
            y += lineHeight;

            int carry = *reinterpret_cast<int *>(0x14);
            (*reinterpret_cast<int *>(0x18))++;
            if (carry != 0) {
                *reinterpret_cast<int *>(0x14) = 0;
                width -= carry;
                x += carry;
            }

            str = nextStr;
        } while (str != 0);

        return y;
    }

    return a2->top;
}

// ORIGINAL: 0x005DC410 ?find_line_break_l@Buffer@@QAEHPADPAHH@Z 0x005DC410-0x005DC70A FILE
// symbol    ?find_line_break_l@Buffer@@QAEPADPADPAHH@Z
// TRIED: same bracket-skip/jump-table markup dispatch as Buffer::text_width (0x005DC7C0), verified against the raw jump/byte tables at 0x5dc70c/0x5dc728 (identical shape, different literal token address 0x696c90). The four find-break-then-adjust call sites collapse to one resolve_break() helper (space-before or segment-start keeps the position, else memrchr back to the last space). Reaches MISMATCH, 147/265 mnemonics shared, first divergence at #2 (frame setup, not the parser).
// working copy - scaffold materialised by --work
// size      762 bytes
// prototype int (__thiscall ?find_line_break_l@Buffer@@QAEHPADPAHH@Z)(Buffer* this, int8*, int*, int)
// callers   15   call targets   4
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x00619370 0x00628AF0 0x006458F0 0x00645DD0


char *Buffer::find_line_break_l(char * a1, int * a2, int a3) {
    if (!a2) {
        return 0;
    }
    if (!font1_) {
        font1_ = FontDefault;
    }
    if (!markup_enabled_) {
        return font1_->find_line_break_l(a1, a2, a3);
    }

    char *segStart = a1;
    char *esi;
    int ebxMode;

scan_loop:
    {
        char *p = segStart;
        int n = a3;
        while (n != 0) {
            char c = *p;
            bool isBreak;
            if (field_514_ == 3) {
                isBreak = (c == '{' || c == '}' || c == '[' || c == ']' || c == '$' || c == '=');
            } else {
                isBreak = (c == '{' || c == '}' || c == '[' || c == ']' || c == '$');
            }
            if (isBreak) {
                break;
            }
            ++p;
            --n;
        }
        a3 = n;
        esi = p;
    }

    if (a3 == 0) {
        goto tail_exit;
    }

    {
        unsigned char ch = (unsigned char)*esi;
        switch (ch) {
        case '$':
            if (strncmp(esi, reinterpret_cast<const char *>(MarkupLinkToken), 6) == 0) {
                markup_pending_ = 1;
                ebxMode = 3;
                goto join_compute;
            }
            break;
        case '=':
            markup_pending_ = 2;
            break;
        case '{':
            if (esi[1] == '{') {
                goto consume_extra;
            }
            ebxMode = 1;
            goto join_compute;
        case '}':
            if (esi[1] == '}') {
                goto consume_extra;
            }
            ebxMode = 0;
            goto join_compute;
        case '[':
            if (esi[1] == '[') {
                goto consume_extra;
            }
            ebxMode = 2;
            goto join_compute;
        case ']':
            if (esi[1] == ']') {
                goto consume_extra;
            }
            ebxMode = 0;
            goto join_compute;
        default:
            break;
        }
    }
    ebxMode = field_514_;
    goto join_compute;

consume_extra:
    --a3;
    ++esi;
    ebxMode = field_514_;
    goto join_compute;

join_compute:
    {
        int mode = field_514_;
        Font *const *fontArray = &font1_;
        Font *segFont = fontArray[mode];
        int segLen = (int)(esi - segStart);
        char *found;
        if (segFont) {
            found = segFont->find_line_break_l(segStart, a2, segLen);
        } else {
            found = font1_->find_line_break_l(segStart, a2, segLen);
        }
        if (found != 0) {
            markup_pending_ = 0;
            return reinterpret_cast<char *>(found) - segStart + a1;
        }
    }

    {
        int markupKind = markup_pending_;
        if (markupKind == 1) {
            a3 -= 5;
            esi += 5;
        }
        ++esi;
        segStart = esi;
        if (markupKind == 2) {
            char *found2 = strchr(esi, '>');
            ebxMode = 0;
            if (found2) {
                a3 = a3 - (int)(found2 - esi) - 1;
                segStart = found2 + 1;
            }
            markup_pending_ = ebxMode;
        }
    }
    field_514_ = ebxMode;
    --a3;
    goto scan_loop;

tail_exit:
    {
        Font *const *fontArray = &font1_;
        Font *segFont = fontArray[field_514_];
        int segLen = (int)(esi - segStart);
        char *found;
        if (segFont) {
            found = segFont->find_line_break_l(segStart, a2, segLen);
        } else {
            found = font1_->find_line_break_l(segStart, a2, segLen);
        }
        markup_pending_ = 0;
        if (found == 0) {
            return 0;
        }
        return reinterpret_cast<char *>(found) - segStart + a1;
    }
}

// ORIGINAL: 0x005DE580 ?change_color@Buffer@@QAEHHH@Z 0x005DE580-0x005DE721
// TRIED: MISMATCH #7 xor/mov - get_data()/free_data() logic inlined by hand (raw uint32_t offsets, matching Buffer::get_data's own style) rather than calling the real methods, since neither is a catalogued callee of this address; register allocation differs from the original past that point.
// size      417 bytes
// prototype int (__thiscall ?change_color@Buffer@@QAEHHH@Z)(Buffer* this, int, int)
// callers   12   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x005DE5D3 0x005DE642 0x005DE70A
// To start: tools/decomp_status.py --work 0x005DE580


int Buffer::change_color(int a1, int a2) {
    volatile uint32_t *const ordered = reinterpret_cast<volatile uint32_t *>(this);

    RECT rect;
    rect.right = dib_.bmiHeader.biWidth;
    rect.bottom = -dib_.bmiHeader.biHeight;
    if (ordered[0x54 / 4] == 0 && ordered[0x58 / 4] == 0) {
        return 7;
    }
    rect.left = 0;
    rect.top = 0;
    if (!(*reinterpret_cast<int (__stdcall *const *)(RECT *, RECT *, RECT *)>(IntersectRect))(
            &rect, &rect1_, &rect)) {
        return 0;
    }
    if (rect.left >= dib_.bmiHeader.biWidth || rect.top >= -dib_.bmiHeader.biHeight) {
        return 7;
    }

    int pixel_base;
    void *const surface = reinterpret_cast<void *>(ordered[0x58 / 4]);
    if (!surface) {
        const uint32_t storage = ordered[0x54 / 4];
        ordered[0x50 / 4] = storage;
        if (!storage) {
            return 7;
        }
        ordered[0x6C / 4] = ordered[0x6C / 4] + 1;
        pixel_base = static_cast<int>(storage);
    } else {
        const uint32_t existing = ordered[0x50 / 4];
        if (existing) {
            ordered[0x6C / 4] = ordered[0x6C / 4] + 1;
            pixel_base = static_cast<int>(existing);
        } else {
            uint8_t descriptor[0x6C];
            *reinterpret_cast<uint32_t *>(descriptor) = 0x6C;
            void **const vtable = *reinterpret_cast<void ***>(surface);
            long (__stdcall *const lock)(void *, void *, void *, uint32_t, void *) =
                reinterpret_cast<long (__stdcall *)(void *, void *, void *, uint32_t, void *)>(
                    vtable[0x64 / 4]);
            if (lock(surface, 0, descriptor, 1, 0) != 0) {
                return 7;
            }
            ordered[0x6C / 4] = ordered[0x6C / 4] + 1;
            uint32_t pitch = *reinterpret_cast<uint32_t *>(descriptor + 0x10);
            uint32_t data = *reinterpret_cast<uint32_t *>(descriptor + 0x24);
            ordered[0x4A8 / 4] = pitch;
            ordered[0x50 / 4] = data;
            pixel_base = static_cast<int>(data);
        }
    }

    if (pixel_base != 0) {
        char *dest = reinterpret_cast<char *>(
            static_cast<int32_t>(ordered[0x4A8 / 4]) * rect.top + pixel_base + rect.left);
        if (dest != 0) {
            int32_t row_width = rect.right - rect.left;
            int32_t row_count = rect.bottom - rect.top;
            int32_t stride_remainder = static_cast<int32_t>(ordered[0x4A8 / 4]) - row_width;
            char *p = dest;
            for (int32_t y = row_count; y != 0; --y) {
                for (int32_t x = row_width; x != 0; --x) {
                    if (*p == static_cast<char>(a1)) {
                        *p = static_cast<char>(a2);
                    }
                    ++p;
                }
                p += stride_remainder;
            }

            void *const surface2 = reinterpret_cast<void *>(ordered[0x58 / 4]);
            const int32_t remaining = static_cast<int32_t>(ordered[0x6C / 4]) - 1;
            ordered[0x6C / 4] = static_cast<uint32_t>(remaining);
            if (!surface2) {
                if (remaining <= 0) {
                    ordered[0x50 / 4] = 0;
                    ordered[0x6C / 4] = 0;
                }
                return 0;
            }
            const uint32_t data2 = ordered[0x50 / 4];
            if (data2 != 0 && remaining <= 0) {
                void **const vtable2 = *reinterpret_cast<void ***>(surface2);
                long (__stdcall *const unlock)(void *, void *) =
                    reinterpret_cast<long (__stdcall *)(void *, void *)>(vtable2[0x80 / 4]);
                unlock(surface2, reinterpret_cast<void *>(data2));
                ordered[0x50 / 4] = 0;
                ordered[0x6C / 4] = 0;
            }
            return 0;
        }
    }
    return 7;
}

// ORIGINAL: 0x005E2070 ?change_color@Buffer@@QAEHHHHHHH@Z 0x005E2070-0x005E220D
// TRIED: clip against rect1_, then a DirectDraw Lock/search-replace/ Unlock byte fill on the locked surface (Lock/Unlock are genuinely __stdcall COM methods - `this` is pushed as an explicit arg, not passed via ecx, confirmed by the disassembly never setting ecx before either indirect call). Landed at 0.67 similarity; diverges at #8 on the width_/ height_ range-check scheduling.
// size      413 bytes
// prototype int (__thiscall ?change_color@Buffer@@QAEHHHHHHH@Z)(Buffer* this, int, int, int, int, int, int)
// callers   2   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x005E20C5 0x005E212E 0x005E21F6


int Buffer::change_color(int a1, int a2, int a3, int a4, int a5, int a6) {
    if (dib_bits_ == 0 && surface_ == 0) {
        return 7;
    }

    RECT rect;
    rect.left = a1;
    rect.top = a2;
    rect.right = a1 + a3;
    rect.bottom = a2 + a4;

    RECT clip = rect1_;
    if (!(IntersectRect)(&rect, &clip, &rect)) {
        return 0;
    }

    if (rect.left >= dib_.bmiHeader.biWidth || rect.top >= -dib_.bmiHeader.biHeight) {
        return 7;
    }

    unsigned char descBuf[0x6c];

    if (surface_ != 0) {
        if (locked_bits_ != 0) {
            surface_lock_count_++;
        } else {
            *reinterpret_cast<int *>(descBuf) = 0x6c;

            void *vtbl = *reinterpret_cast<void **>(surface_);
            DDLockFn lockFn = *reinterpret_cast<DDLockFn *>(reinterpret_cast<char *>(vtbl) + 0x64);
            long hr = lockFn(reinterpret_cast<void *>(surface_), 0, descBuf, 1, 0);
            if (hr != 0) {
                return 7;
            }

            stride_ = *reinterpret_cast<uint32_t *>(descBuf + 0x10);
            surface_lock_count_++;
            locked_bits_ = reinterpret_cast<void *>(*reinterpret_cast<uint32_t *>(descBuf + 0x24));
        }
    } else {
        uint32_t bits = reinterpret_cast<uint32_t>(dib_bits_);
        locked_bits_ = reinterpret_cast<void *>(bits);
        if (bits == 0) {
            return 7;
        }
        surface_lock_count_++;
    }

    if (locked_bits_ == nullptr) {
        return 7;
    }

    int clippedTop = rect.top;
    char *pixelPtr = stride_ * clippedTop + rect.left + static_cast<char *>(locked_bits_);

    if (pixelPtr != 0) {
        int width = rect.right - rect.left;
        int height = rect.bottom - clippedTop;
        int rowSkip = static_cast<int>(stride_) - width;

        char searchColor = static_cast<char>(a5);
        char replaceColor = static_cast<char>(a6);

        char *p = pixelPtr;
        int rowsLeft = height;
        do {
            int colsLeft = width;
            do {
                if (*p == searchColor) {
                    *p = replaceColor;
                }
                p++;
                colsLeft--;
            } while (colsLeft != 0);
            p += rowSkip;
            rowsLeft--;
        } while (rowsLeft != 0);

        if (surface_ == 0) {
            int count = surface_lock_count_ - 1;
            surface_lock_count_ = count;
            if (count > 0) {
                return 0;
            }
            locked_bits_ = nullptr;
            surface_lock_count_ = 0;
            return 0;
        }

        int count = surface_lock_count_ - 1;
        surface_lock_count_ = count;
        if (locked_bits_ == nullptr) {
            return 0;
        }
        if (count > 0) {
            return 0;
        }

        void *vtbl = *reinterpret_cast<void **>(surface_);
        DDUnlockFn unlockFn = *reinterpret_cast<DDUnlockFn *>(reinterpret_cast<char *>(vtbl) + 0x80);
        unlockFn(reinterpret_cast<void *>(surface_), reinterpret_cast<int>(locked_bits_));
        locked_bits_ = nullptr;
        surface_lock_count_ = 0;
        return 0;
    }

    return 7;
}

// Remapping snapshots its row gap before acquisition, but computes the
// first pixel using the pitch published by acquisition. Preserve that split
// even if DirectDraw changes pitch. Failure after acquisition deliberately
// does not release; this includes successful locks that return null pixels.
// Geometry differences fit signed integers. Numeric address arithmetic
// preserves native pointer-width wrap and the original zero-address check.
// ORIGINAL: 0x005DA330 ?map_colors@Buffer@@QAEHHHHHPAE@Z 0x005DA330-0x005DA4C1 FILE
// RECOVERY_BINDING: v1 {"evidence": ["5DA330..5DA4C1 has five explicit stack arguments;5DA39B snapshots stride-width before Lock,5DA41A computes first pixel using post-Lock pitch,5DA455..45A translates live indexed bytes, and only completed painting reaches5DA46D release. Failure returns7 without release; null table returns16."], "reason": "The coordinate colour remapper owns Buffer pixel acquisition and release; it reads the supplied translation table live for each byte in the exclusive clipped rectangle."}
int Buffer::map_colors(int left, int top, int right, int bottom,
                       unsigned char *table) {
    if (table == nullptr) {
        return 16;
    }
    RECT clipped;
    clipped.left = left;
    clipped.top = top;
    clipped.right = right;
    clipped.bottom = bottom;
    if (!IntersectRect(&clipped, &clipped, &rect1_)) {
        return 0;
    }
    int rows = clipped.bottom - clipped.top;
    const int width = clipped.right - clipped.left;
    const int row_gap = static_cast<int>(stride_) - width;
    if (clipped.left >= dib_.bmiHeader.biWidth
        || clipped.top >= -dib_.bmiHeader.biHeight) {
        return 7;
    }
    if (get_data() == 0) {
        return 7;
    }
    uintptr_t address = reinterpret_cast<uintptr_t>(locked_bits_)
        + static_cast<uintptr_t>(static_cast<int>(stride_))
            * static_cast<uintptr_t>(clipped.top)
        + static_cast<uintptr_t>(clipped.left);
    if (address == 0) {
        return 7;
    }
    do {
        int remaining = width;
        do {
            unsigned char *pixel = reinterpret_cast<unsigned char *>(address);
            // Do not snapshot the table: it may alias the output pixels.
            *pixel = table[*pixel];
            ++address;
        } while (--remaining != 0);
        address += static_cast<uintptr_t>(row_gap);
    } while (--rows != 0);
    free_data(1);
    return 0;
}

// ORIGINAL: 0x005DA4D0 ?map_colors@Buffer@@QAEHPAURECT@@PAE@Z 0x005DA4D0-0x005DA67D
// symbol    ?map_colors@Buffer@@QAEHPAUtagRECT@@PAE@Z
//           windows.h typedefs RECT to struct tagRECT, so the pointer mangles
//           with the real tag.
// TRIED: MISMATCH #6 mov/push - get_data()/free_data() logic inlined by hand (raw uint32_t offsets, matching Buffer::get_data's own style, as in the sibling change_color at 0x005DE580) since neither is a catalogued callee of this address; register allocation differs from the original past that point.
// size      429 bytes
// prototype int (__thiscall ?map_colors@Buffer@@QAEHPAURECT@@PAE@Z)(Buffer* this, RECT*, unsigned int8*)
// callers   2   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x005DA530 0x005DA5B5 0x005DA666


int Buffer::map_colors(RECT *area, unsigned char *table) {
    if (area == nullptr || table == nullptr) {
        return 16;
    }
    return map_colors(area->left, area->top, area->right - 1,
                      area->bottom - 1, table);
}

// ORIGINAL: 0x005DD730 ?wrap@Buffer@@QAEHPADHHH@Z 0x005DD730-0x005DD91C FILE
// size      492 bytes
// prototype int (__thiscall ?wrap@Buffer@@QAEHPADHHH@Z)(Buffer* this, int8*, int, int, int)
// callers   8   call targets   3
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005DC410 0x005DCAE0 0x006453E0


int Buffer::wrap(char * a1, int a2, int a3, int a4) {
    
    if (a1 == 0 || a4 == 0) {
        return a3;
    }

    BufferStrHeight = 0;
    field_18_ = 0;
    field_14_ = field_10_;

    char *cur = a1;
    for (;;) {
        char *nextLine;
        int wrapResult;
        int wrapLen = a4;

        if (cur == 0) {
            nextLine = 0;
            wrapResult = a2;
        } else {
            BufferStrHeight = 0;
            unsigned int len = strlen(cur);
            char *brk = reinterpret_cast<char *>(find_line_break_l(cur, &wrapLen, (int)len));
            nextLine = brk;
            if (brk == 0) {
                unsigned int len2 = strlen(cur);
                if (font1_ == 0 || font1_->font_obj_ == 0) {
                    wrapResult = 3;
                } else {
                    unsigned int lenA = strlen(cur);
                    unsigned int lenB = len2;
                    if ((int)lenA < (int)len2) {
                        lenB = strlen(cur);
                    }
                    wrapResult = a2;
                    if ((int)lenB >= 0) {
                        unsigned int lenC = strlen(cur);
                        if ((int)lenC < (int)len2) {
                            len2 = strlen(cur);
                        }
                        wrapResult = a2;
                        if (len2 != 0) {
                            wrapResult = write_multi_font_raw_l(cur, a2, a3, (int)len2);
                        }
                    }
                }
            } else {
                unsigned int segLen = (unsigned int)(brk - cur);
                if (font1_ == 0 || font1_->font_obj_ == 0) {
                    *BufferWriteStatus = 3;
                    wrapResult = *BufferWriteStatus;
                } else {
                    unsigned int lenA = strlen(cur);
                    unsigned int lenB = segLen;
                    if ((int)lenA < (int)segLen) {
                        lenB = strlen(cur);
                    }
                    if ((int)lenB >= 0) {
                        unsigned int lenC = strlen(cur);
                        if ((int)lenC < (int)segLen) {
                            segLen = strlen(cur);
                        }
                        if (segLen != 0) {
                            wrapResult = write_multi_font_raw_l(cur, a2, a3, (int)segLen);
                            goto after_wrap;
                        }
                    }
                    *BufferWriteStatus = a2;
                    wrapResult = *BufferWriteStatus;
                }
            }
        }
after_wrap:
        *BufferWriteStatus = wrapResult;
        if (font1_ == 0) {
            font1_ = FontDefault;
        }
        Font *rec = font1_;
        int adj;
        if (rec->unk_1_ < 0) {
            adj = rec->line_height_;
        } else {
            adj = rec->height_ + rec->unk_1_;
        }
        a3 = a3 + adj;
        int carry = field_14_;
        field_18_ = field_18_ + 1;
        if (carry != 0) {
            a4 = a4 - carry;
            a2 = a2 + carry;
            field_14_ = 0;
        }
        cur = nextLine;
        if (nextLine == 0) {
            return a3;
        }
    }
}

// ORIGINAL: 0x005D8A30 ?draw_dest@Buffer@@QAEHPAVBuffer@@HHHH@Z 0x005D8A30-0x005D8B1E BYTE_EXACT
// LEVER: THE BLIT DESCRIPTOR IS A SPRITE. The ten Blit slots this wrapper
//        fills are the fields of the Sprite at 0x009B3A50 (pcBits_,
//        cTransparentIndex_, iSpriteWidth2_, iSpriteWidth_, iSpriteHeight_,
//        iWidth_, iHeight_, iLeftOffset_, iTopOffset_, fObj1Exists_, in that
//        order, +0x04..+0x28), which is why the tail can call through one
//        pointer named BlitSprite. Byte-exact first time through the named
//        members; the archive's `VCall` shim and `ReleaseFn` vtable walk
//        become `surface_->Unlock(locked_bits_)` like everywhere else.
// symbol    ?draw_dest@Buffer@@QAEHPAV1@HHHH@Z
//           The image spells the leading `Buffer *` as `PAVBuffer@@`; VC6 emits
//           the back-reference `PAV1@` for the same type, so the fact names what
//           this tree actually emits. Bytes are unchanged.
// size      238 bytes
// prototype int (__thiscall ?draw_dest@Buffer@@QAEHPAVBuffer@@HHHH@Z)(Buffer* this, Buffer*, int, int, int, int)
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005E5833
// indirect  0x005D8AE7
// WAS-LEVER: same preamble as the other Buffer::draw_* siblings; the callee
// (Sprite::draw_dest) takes `unsigned char *` as its last param where this
// function's own a5 is `int`, so the tail call casts it.


int Buffer::draw_dest(Buffer * a1, int a2, int a3, int a4, int a5) {
    int transparent = a2;
    if (surface_ == 0) {
        if (a2 == -1) {
            transparent = WinFillColour;
        }
        locked_bits_ = dib_bits_;
        if (locked_bits_ != nullptr) {
            surface_lock_count_ = surface_lock_count_ + 1;
        }
        g_BUFFER_SPRITE.pcBits_ = static_cast<char *>(locked_bits_);
        g_BUFFER_SPRITE.iSpriteWidth2_ = stride_;
        g_BUFFER_SPRITE.iSpriteWidth_ = dib_.bmiHeader.biWidth;
        g_BUFFER_SPRITE.iSpriteHeight_ = -dib_.bmiHeader.biHeight;
        g_BUFFER_SPRITE.iWidth_ = dib_.bmiHeader.biWidth;
        g_BUFFER_SPRITE.iHeight_ = -dib_.bmiHeader.biHeight;
        g_BUFFER_SPRITE.iLeftOffset_ = 0;
        g_BUFFER_SPRITE.iTopOffset_ = 0;
        g_BUFFER_SPRITE.cTransparentIndex_ = (unsigned char)transparent;
        g_BUFFER_SPRITE.fObj1Exists_ = 0;

        if (surface_ == 0) {
            surface_lock_count_ = surface_lock_count_ - 1;
            if ((int)surface_lock_count_ > 0) {
                goto tail;
            }
        } else {
            surface_lock_count_ = surface_lock_count_ - 1;
            if (locked_bits_ == nullptr || (int)surface_lock_count_ > 0) {
                goto tail;
            }
            {
            surface_->Unlock(locked_bits_);
        }
        }
        locked_bits_ = nullptr;
        surface_lock_count_ = 0;
    }
tail:
    {
        int result = g_BUFFER_SPRITE.draw_dest(
            a1, a2, a3, a4, reinterpret_cast<unsigned char *>(a5));
        g_BUFFER_SPRITE.pcBits_ = 0;
        return result;
    }
}

// ORIGINAL: 0x005D93C0 ?draw_multi_table_dest@Buffer@@QAEHPAVBuffer@@HHHH@Z 0x005D93C0-0x005D94AE BYTE_EXACT
// LEVER: same shape and same landing as draw_dest above - the descriptor is
//        the Sprite at 0x009B3A50, the release is surface_->Unlock, and the
//        byte-exact form needs no spelling beyond the tree's own members.
// symbol    ?draw_multi_table_dest@Buffer@@QAEHPAV1@HHHH@Z
//           The image spells the leading `Buffer *` as `PAVBuffer@@`; VC6 emits
//           the back-reference `PAV1@` for the same type, so the fact names what
//           this tree actually emits. Bytes are unchanged.
// size      238 bytes
// prototype int (__thiscall ?draw_multi_table_dest@Buffer@@QAEHPAVBuffer@@HHHH@Z)(Buffer* this, Buffer*, int, int, int, int)
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005E8FF5
// indirect  0x005D9477
// WAS-LEVER: same preamble as the other Buffer::draw_* siblings; the callee
// (Sprite::draw_multi_table_dest) takes `unsigned char *` as its last
// param where this function's own a5 is `int`, so the tail call casts it.


int Buffer::draw_multi_table_dest(Buffer * a1, int a2, int a3, int a4, int a5) {
    int transparent = a2;
    if (surface_ == 0) {
        if (a2 == -1) {
            transparent = WinFillColour;
        }
        locked_bits_ = dib_bits_;
        if (locked_bits_ != nullptr) {
            surface_lock_count_ = surface_lock_count_ + 1;
        }
        g_BUFFER_SPRITE.pcBits_ = static_cast<char *>(locked_bits_);
        g_BUFFER_SPRITE.iSpriteWidth2_ = stride_;
        g_BUFFER_SPRITE.iSpriteWidth_ = dib_.bmiHeader.biWidth;
        g_BUFFER_SPRITE.iSpriteHeight_ = -dib_.bmiHeader.biHeight;
        g_BUFFER_SPRITE.iWidth_ = dib_.bmiHeader.biWidth;
        g_BUFFER_SPRITE.iHeight_ = -dib_.bmiHeader.biHeight;
        g_BUFFER_SPRITE.iLeftOffset_ = 0;
        g_BUFFER_SPRITE.iTopOffset_ = 0;
        g_BUFFER_SPRITE.cTransparentIndex_ = (unsigned char)transparent;
        g_BUFFER_SPRITE.fObj1Exists_ = 0;

        if (surface_ == 0) {
            surface_lock_count_ = surface_lock_count_ - 1;
            if ((int)surface_lock_count_ > 0) {
                goto tail;
            }
        } else {
            surface_lock_count_ = surface_lock_count_ - 1;
            if (locked_bits_ == nullptr || (int)surface_lock_count_ > 0) {
                goto tail;
            }
            {
            surface_->Unlock(locked_bits_);
        }
        }
        locked_bits_ = nullptr;
        surface_lock_count_ = 0;
    }
tail:
    {
        int result = g_BUFFER_SPRITE.draw_multi_table_dest(
            a1, a2, a3, a4, reinterpret_cast<unsigned char *>(a5));
        g_BUFFER_SPRITE.pcBits_ = 0;
        return result;
    }
}

// ORIGINAL: 0x005D8B20 ?draw_dest@Buffer@@QAEHPAVBuffer@@HHHHHPAE@Z 0x005D8B20-0x005D8C18 FILE
// TRIED: the archive's transcription read the TRANSPARENT INDEX out of the
//        first parameter (`(int)a1`) where the image loads `[esp+0xc]`, the
//        int AFTER the `Buffer *` - the five-argument form, which is byte
//        exact, uses the second parameter too. Correcting it took agreement
//        26/80 -> 28/80; reading WinFillColour as the uint8_t object it is,
//        rather than through a reinterpreted pointer, took it to 35/80.
//        Still MISMATCH: the branch targets move by four bytes somewhere
//        past instruction 9, which is a code-size difference, not a
//        register or ordering one. Not chased further.
// TRIED: naming the transparent index through `*reinterpret_cast<uint8_t
//        *>(WinFillColour)` costs a register the image does not spend.
// symbol    ?draw_dest@Buffer@@QAEHPAV1@HHHHHPAE@Z
//           The image spells the leading `Buffer *` as `PAVBuffer@@`; VC6 emits
//           the back-reference `PAV1@` for the same type, so the fact names what
//           this tree actually emits. Bytes are unchanged.
// TRIED: same shape as 0x005D8740/0x005D8930 - see 0x005D8740's note for what the stdcall-Unlock and opaque-field access forms rule out.
// TRIED: nothing new past that - 94.9%+ mnemonic similarity, first gap at #41 (mov vs cmp) in the NULL-surface lock-count arm, same spot as both siblings.
// working copy - scaffold materialised by --work
// size      248 bytes
// prototype int (__thiscall ?draw_dest@Buffer@@QAEHPAVBuffer@@HHHHHPAE@Z)(Buffer* this, Buffer*, int, int, int, int, int, unsigned int8*)
// callers   1   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005E6652
// indirect  0x005D8BD7


int Buffer::draw_dest(Buffer * a1, int a2, int a3, int a4, int a5, int a6, unsigned char * a7) {
    int transparentIndex = a2;
    if (surface_ == 0) {
        if (transparentIndex == -1) {
            transparentIndex = WinFillColour;
        }

        locked_bits_ = dib_bits_;
        int bits = reinterpret_cast<int>(locked_bits_);
        if (bits != 0) {
            ++surface_lock_count_;
        }
        g_BUFFER_SPRITE.pcBits_ = static_cast<char *>(locked_bits_);
        g_BUFFER_SPRITE.iSpriteWidth2_ = stride_;
        g_BUFFER_SPRITE.iSpriteWidth_ = dib_.bmiHeader.biWidth;
        g_BUFFER_SPRITE.iSpriteHeight_ = -dib_.bmiHeader.biHeight;
        g_BUFFER_SPRITE.iWidth_ = dib_.bmiHeader.biWidth;
        g_BUFFER_SPRITE.iHeight_ = -dib_.bmiHeader.biHeight;
        g_BUFFER_SPRITE.iLeftOffset_ = 0;
        g_BUFFER_SPRITE.iTopOffset_ = 0;
        g_BUFFER_SPRITE.cTransparentIndex_ = static_cast<char>(transparentIndex);
        g_BUFFER_SPRITE.fObj1Exists_ = 0;

        void *surface = reinterpret_cast<void *>(surface_);
        if (surface == 0) {
            int count = static_cast<int>(surface_lock_count_) - 1;
            surface_lock_count_ = count;
            if (count > 0) {
                goto forward;
            }
        } else {
            int count = static_cast<int>(surface_lock_count_) - 1;
            surface_lock_count_ = count;
            int cachedBits = reinterpret_cast<int>(locked_bits_);
            if (cachedBits == 0 || count > 0) {
                goto forward;
            }
            typedef void(__stdcall * UnlockFn)(void *, void *);
            UnlockFn unlock = *reinterpret_cast<UnlockFn *>(
                *reinterpret_cast<int *>(surface) + 0x80);
            unlock(surface, reinterpret_cast<void *>(cachedBits));
        }
        locked_bits_ = nullptr;
        surface_lock_count_ = 0;
    }

forward:
    {
        int result = g_BUFFER_SPRITE.draw_dest(a1, a2, a3, a4, a5, a6, a7);
        g_BUFFER_SPRITE.pcBits_ = 0;
        return result;
    }
}

// ORIGINAL: 0x005D94B0 ?draw_multi_table_dest@Buffer@@QAEHPAVBuffer@@HHHHHPAE@Z 0x005D94B0-0x005D95A8 FILE
// TRIED: the same two corrections as draw_dest above (second parameter is
//        the transparent index; WinFillColour read as the object it is),
//        26/80 -> 35/80, still MISMATCH on the same four-byte block-size
//        difference. Not chased further.
// symbol    ?draw_multi_table_dest@Buffer@@QAEHPAV1@HHHHHPAE@Z
//           The image spells the leading `Buffer *` as `PAVBuffer@@`; VC6 emits
//           the back-reference `PAV1@` for the same type, so the fact names what
//           this tree actually emits. Bytes are unchanged.
// TRIED: same shape as 0x005D8930/0x005D8B20/0x005D8EE0 - see 0x005D8740's note for the stdcall-Unlock function-pointer form.
// TRIED: nothing new past that - 94.9%+ mnemonic similarity, first gap at #41 (mov vs cmp) in the NULL-surface lock-count arm, same spot as every sibling in this family.
// working copy - scaffold materialised by --work
// size      248 bytes
// prototype int (__thiscall ?draw_multi_table_dest@Buffer@@QAEHPAVBuffer@@HHHHHPAE@Z)(Buffer* this, Buffer*, int, int, int, int, int, unsigned int8*)
// callers   4   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005E8FA5
// indirect  0x005D9567


int Buffer::draw_multi_table_dest(Buffer * a1, int a2, int a3, int a4, int a5, int a6, unsigned char * a7) {
    int transparentIndex = a2;
    if (surface_ == 0) {
        if (transparentIndex == -1) {
            transparentIndex = WinFillColour;
        }

        locked_bits_ = dib_bits_;
        int bits = reinterpret_cast<int>(locked_bits_);
        if (bits != 0) {
            ++surface_lock_count_;
        }
        g_BUFFER_SPRITE.pcBits_ = static_cast<char *>(locked_bits_);
        g_BUFFER_SPRITE.iSpriteWidth2_ = stride_;
        g_BUFFER_SPRITE.iSpriteWidth_ = dib_.bmiHeader.biWidth;
        g_BUFFER_SPRITE.iSpriteHeight_ = -dib_.bmiHeader.biHeight;
        g_BUFFER_SPRITE.iWidth_ = dib_.bmiHeader.biWidth;
        g_BUFFER_SPRITE.iHeight_ = -dib_.bmiHeader.biHeight;
        g_BUFFER_SPRITE.iLeftOffset_ = 0;
        g_BUFFER_SPRITE.iTopOffset_ = 0;
        g_BUFFER_SPRITE.cTransparentIndex_ = static_cast<char>(transparentIndex);
        g_BUFFER_SPRITE.fObj1Exists_ = 0;

        void *surface = reinterpret_cast<void *>(surface_);
        if (surface == 0) {
            int count = static_cast<int>(surface_lock_count_) - 1;
            surface_lock_count_ = count;
            if (count > 0) {
                goto forward;
            }
        } else {
            int count = static_cast<int>(surface_lock_count_) - 1;
            surface_lock_count_ = count;
            int cachedBits = reinterpret_cast<int>(locked_bits_);
            if (cachedBits == 0 || count > 0) {
                goto forward;
            }
            typedef void(__stdcall * UnlockFn)(void *, void *);
            UnlockFn unlock = *reinterpret_cast<UnlockFn *>(
                *reinterpret_cast<int *>(surface) + 0x80);
            unlock(surface, reinterpret_cast<void *>(cachedBits));
        }
        locked_bits_ = nullptr;
        surface_lock_count_ = 0;
    }

forward:
    {
        int result = g_BUFFER_SPRITE.draw_multi_table_dest(a1, a2, a3, a4, a5, a6, a7);
        g_BUFFER_SPRITE.pcBits_ = 0;
        return result;
    }
}

// ORIGINAL: 0x005D7CA0 ?load_bmp@Buffer@@QAGHPBDPAVPalette@@HH@Z 0x005D7CA0-0x005D7DD8;0x00662BA7-0x00662BBC FILE
// TRIED: same load_tga/load_bmp/load_pcx SEH-frame family as 0x005D7A30 and 0x005D7B40; try{}catch(...){throw;} is the closest known shape and still diverges at #0 push/mov.
// size      333 bytes
// prototype 
// callers   1   call targets   5
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00628380 0x006283E0 0x00628430 0x006453E0 0x00645470
// indirect  0x005D7DA1
// placeholder - not yet decompiled
// To start: tools/decomp_status.py --work 0x005D7CA0


int __stdcall Buffer::load_bmp(const char * a1, Palette * a2, int a3, int a4) {
    try {
        Filemap fm;
        if (a1 == 0) {
            return 3;
        }
        char path[260];
        path[0] = 0;
        strcat(path, a1);
        char *end = path + strlen(a1);
        if (end != path) {
            do {
                if (*end == '.') {
                    if (end != path) goto has_ext;
                    break;
                }
                end--;
            } while (end != path);
        }
        switch (Language) {
            case 1: strcat(path, BufferExtBmf); break;
            case 2: strcat(path, BufferExtBmg); break;
            default: strcat(path, BufferExtBmp); break;
        }
      has_ext:
        if (fm.open_read(path, 1) == 0) {
            return 6;
        }
        GetFileSize(fm.get_handle(), 0);
        return 8;
    } catch (...) {
        throw;
    }
}

// ORIGINAL: 0x005D79C0 ?resize@Buffer@@QAEXHH@Z 0x005D79C0-0x005D7A05 FILE BYTE_EXACT
// provenance (retired seed): git:b5a9c5dd:src/recovered/units/005d79c0.cpp
// A plain Buffer resizes its surface; a window-owned Buffer dispatches to
// its GraphicWin. The original queries the owner twice before resizing.
// calls     0x005D7670
void Buffer::resize(int width, int height) {
    if (get_graphic_win() != nullptr) {
        get_graphic_win()->resize(width, height, 1);
        return;
    }
    init(width, height, init_flags_, nullptr);
}

// ---------------------------------------------------------------------------
// COVERAGE HOMES, 2026-09-26. The class:Buffer seed-only group, transcribed
// into real member bodies (FILE markers, no byte claims) from their retired
// seeds. Each seed read its fields through fixed offsets and its surfaces
// through raw vtable-slot pushes; here that state is the named members and
// the typed interface calls the rest of this file already uses, and the
// shared lock pair is `get_data` / `free_data`.
// ---------------------------------------------------------------------------

// ORIGINAL: 0x005D7B20 ?load_tga@Buffer@@QAGHPBD@Z 0x005D7B20-0x005D7B36
// size      22 bytes
// prototype int (__stdcall ?load_tga@Buffer@@QAGHPBD@Z)(Buffer* this, int8*)
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005D7B40
// provenance (retired seed): src/recovered/units/005d7b20.cpp. Homed for
// COVERAGE, no byte claim: the palette-less loader entry hands the file name
// to the four-argument loader with the full 256-entry colour count.
int __stdcall Buffer::load_tga(const char *filename) {
    return load_tga(filename, nullptr, 0, 0x100);
}

// ORIGINAL: 0x005D7C80 ?load_bmp@Buffer@@QAGHPBD@Z 0x005D7C80-0x005D7C96
// size      22 bytes
// prototype int (__stdcall ?load_bmp@Buffer@@QAGHPBD@Z)(Buffer* this, int8*)
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005D7CA0
// provenance (retired seed): src/recovered/units/005d7c80.cpp. Homed for
// COVERAGE, no byte claim: the same forwarder shape as load_tga above.
int __stdcall Buffer::load_bmp(const char *filename) {
    return load_bmp(filename, nullptr, 0, 0x100);
}

// ORIGINAL: 0x005D9610 ?copy@Buffer@@QAEHPAVBuffer@@HHHHHHHH@Z 0x005D9610-0x005D98EA
// symbol    ?copy@Buffer@@QAEHPAV1@HHHHHHHH@Z
//           The image spells the leading `Buffer *` as `PAVBuffer@@`; VC6 emits
//           the back-reference `PAV1@` for the same type, so the fact names what
//           this tree actually emits.
// size      730 bytes
// prototype int (__thiscall ?copy@Buffer@@QAEHPAVBuffer@@HHHHHHHH@Z)(Buffer* this, Buffer*, int, int, int, int, int, int, int, int)
// callers   4   call targets   3
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005E3503 0x005E3563 0x005FE560
// indirect  0x005D9637 0x005D966F 0x005D967A 0x005D96A6 0x005D96D2 0x005D96DD 0x005D9732 0x005D973D 0x005D97A5 0x005D9817 0x005D986D 0x005D98BC 0x005D98C7
// provenance (retired seed): src/unrecovered/005d9610.cpp. Homed for COVERAGE,
// no byte claim. GDI StretchBlt between the two buffers' device contexts: each
// context comes from `get_hdc` and goes back through `release_hdc`, and each
// colour table goes through the five-line palette publish `sync_to_palette`
// owns - inlined twice here because the blit touches two buffers, which is
// how buffer.h already describes this family.
int Buffer::copy(Buffer *destination, int src_x, int src_y, int src_w,
                 int src_h, int dest_x, int dest_y, int dest_w, int dest_h) {
    if (destination == 0) {
        return 0x10;
    }
    HDC destination_dc = destination->get_hdc();
    if (destination_dc == 0) {
        return 7;
    }
    HDC source_dc = get_hdc();
    if (source_dc == 0) {
        destination->release_hdc(1);
        return 7;
    }

    if (dib_bits_ != 0 && PaletteActive != 0) {
        if (palette_seed_ != PaletteActive->seed_) {
            palette_seed_ = PaletteActive->seed_;
            RGBQUAD *const table = dib_.bmiColors;
            PaletteActive->get_rgbquad(table, 0, 0x100);
            if (get_hdc() != 0) {
                SetDIBColorTable(hdc2_, 0, 0x100, table);
                release_hdc(1);
            }
        }
        has_palette_ = 1;
        palette_ = PaletteActive;
    }
    if (destination->dib_bits_ != 0 && PaletteActive != 0) {
        if (destination->palette_seed_ != PaletteActive->seed_) {
            destination->palette_seed_ = PaletteActive->seed_;
            RGBQUAD *const table = destination->dib_.bmiColors;
            PaletteActive->get_rgbquad(table, 0, 0x100);
            if (destination->get_hdc() != 0) {
                SetDIBColorTable(destination->hdc2_, 0, 0x100, table);
                destination->release_hdc(1);
            }
        }
        destination->has_palette_ = 1;
        destination->palette_ = PaletteActive;
    }

    StretchBlt(destination_dc, dest_x, dest_y, dest_w, dest_h, source_dc,
               src_x, src_y, src_w, src_h, SRCCOPY);

    destination->release_hdc(1);
    release_hdc(1);
    return 0;
}

// The per-entry callback `write_strings_height` consults with each id and
// payload pair before measuring it. The pointer lives in the buffer word the
// header still names `field_C_`.
typedef void(__cdecl *WriteStringsCallback)(int, int);

// ORIGINAL: 0x005DAD70 ?write_strings_height@Buffer@@QAEHPAUStringList@@HH@Z 0x005DAD70-0x005DB03E
// symbol    ?write_strings_height@Buffer@@QAEHPAVStringList@@HH@Z
//           The image spells the list `PAUStringList@@`; this tree declares
//           `class StringList`, and the first visible tag declaration fixes
//           VC6's mangling key, so the object holds `PAVStringList@@` - the
//           same type, spelled the way this tree emits it.
// size      718 bytes
// prototype int (__thiscall ?write_strings_height@Buffer@@QAEHPAUStringList@@HH@Z)(Buffer* this, StringList*, int, int)
// callers   4   call targets   11
// kind      game
// flags     sp_ready;purged_ok
// calls     0x00402500 0x00402530 0x005DC360 0x005DC410 0x005DCAB0 0x005DDDA0 0x005DDDE0 0x005DDEF0 0x005E3630 0x005E3650 0x006453E0
// indirect  0x005DADCB 0x005DAF78
// provenance (retired seed): src/unrecovered/005dad70.cpp. Homed for COVERAGE,
// no byte claim. Measures a StringList without drawing it. The entry payload's
// metadata word picks the per-entry path - 3 is a bare font advance, 2 a
// manual find_line_break_l walk, anything else the flying writer's own
// measurement - and `width` is stored as the wrap width the flying family
// reads back.
int Buffer::write_strings_height(StringList *strings, int width, int flags) {
    if (strings == 0) {
        return 0;
    }
    if ((flags & 2) == 0 && strings->entry_count_ - 1 >= 0) {
        strings->current_position_ = 0;
        strings->current_ = strings->head_;
    }

    // The payload node's metadata word (+8) and text (+4) are what
    // StringStructPayload declares; `current_entry` walks to the current one.
    int payload = strings->head_ != 0 ? strings->current_entry()->field_8_ : 0;

    if (field_C_ != 0) {
        const int id = strings->head_ != 0 ? strings->current_->id : 0;
        reinterpret_cast<WriteStringsCallback>(field_C_)(id, payload);
    }

    int total_height = 0;
    if (payload == 3) {
        if (font1_ == 0) {
            font1_ = FontDefault;
        }
        total_height = font1_->line_advance();
        field_18_ = 0;
        field_14_ = field_10_;
        markup_pending_ = 0;
        *FlyingTextHeight = width;
        *FlyingTextY = 0;
        BufferStrHeight = 0;
        wrap_height_flying(nullptr);
    } else if (payload == 2) {
        char *text = strings->head_ != 0 ? strings->current_entry()->text : 0;
        if (text != 0) {
            field_18_ = 0;
            BufferStrHeight = 0;
            field_14_ = field_10_;
            int remaining_width = width;
            do {
                const int len = strlen(text);
                int break_width = remaining_width;
                text = find_line_break_l(text, &break_width, len);
                BufferStrHeight = 0;
                total_height += text_line_height();
                field_18_ = field_18_ + 1;
                if (field_14_ != 0) {
                    remaining_width -= field_14_;
                    field_14_ = 0;
                }
            } while (text != 0);
        }
        field_18_ = 0;
        field_14_ = field_10_;
        markup_pending_ = 0;
        BufferStrHeight = 0;
        *FlyingTextHeight = width;
        *FlyingTextY = 0;
        wrap_height_flying(nullptr);
    } else {
        const int id = strings->head_ != 0 ? strings->current_->id : 0;
        char *text = strings->head_ != 0 ? strings->current_entry()->text : 0;
        field_18_ = 0;
        field_14_ = field_10_;
        markup_pending_ = 0;
        BufferStrHeight = 0;
        *FlyingTextHeight = width;
        *FlyingTextY = 0;
        wrap_height_flying(text);
    }

    if (strings->current_position_ < strings->entry_count_ - 1) {
        do {
            strings->next_entry();
            if (strings->current_ != 0) {
                strings->current_entry();
            }

            const int line_payload =
                strings->head_ != 0 ? strings->current_entry()->field_8_ : 0;

            if (field_C_ != 0) {
                const int id =
                    strings->head_ != 0 ? strings->current_->id : 0;
                reinterpret_cast<WriteStringsCallback>(field_C_)(id,
                                                                 line_payload);
            }

            if (strings->head_ == 0 || line_payload == 0) {
                wrap_height_flying(strings->current_text());
            } else {
                total_height += wrap_height_flying_flush();

                if (line_payload == 3) {
                    total_height += text_line_height();
                    wrap_height_flying(nullptr, width);
                } else if (line_payload == 2) {
                    total_height += wrap_height(strings->current_text(), width);
                    wrap_height_flying(nullptr, width);
                } else {
                    wrap_height_flying(strings->current_text(), width);
                }
            }
        } while (strings->current_position_ < strings->entry_count_ - 1);
    }

    return wrap_height_flying_flush() + total_height;
}

// ORIGINAL: 0x005DB930 ?write_right_l@Buffer@@QAEHPAUFont@@PADPAURECT@@H@Z 0x005DB930-0x005DB984
// symbol    ?write_right_l@Buffer@@QAEHPAVFont@@PADPAUtagRECT@@H@Z
//           The image spells the rectangle `PAURECT@@`; windows.h declares
//           `struct tagRECT` behind the RECT define, so the object holds
//           `PAUtagRECT@@` - the same type, spelled the way this tree emits it.
// size      84 bytes
// prototype int (__thiscall ?write_right_l@Buffer@@QAEHPAVFont@@PADPAURECT@@H@Z)(Buffer* this, Font*, char*, RECT*, int)
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005DD450
// provenance (retired seed): src/recovered/units/005db930.cpp. Homed for
// COVERAGE, no byte claim. The RECT member of the Font-first family, spelled
// exactly like its coordinate-vector sibling above: the three writer font
// slots overridden while the four-argument RECT writer runs.
int Buffer::write_right_l(Font *font, char *text, RECT *area, int len) {
    Font *saved_font1 = font1_;
    Font *saved_font2 = font2_;
    Font *saved_font3 = font3_;
    if (font != 0 && font->font_obj_ != 0) {
        font1_ = font;
        font2_ = 0;
        font3_ = 0;
    }
    int result = write_right_l(text, area, len);
    font1_ = saved_font1;
    font2_ = saved_font2;
    font3_ = saved_font3;
    return result;
}

// ORIGINAL: 0x005DE730 ?change_color@Buffer@@QAEHPAURECT@@HH@Z 0x005DE730-0x005DE8EB
// symbol    ?change_color@Buffer@@QAEHPAUtagRECT@@HH@Z
//           The image spells the rectangle `PAURECT@@`; windows.h declares
//           `struct tagRECT` behind the RECT define, so the object holds
//           `PAUtagRECT@@` - the same type, spelled the way this tree emits it.
// size      443 bytes
// prototype int (__thiscall ?change_color@Buffer@@QAEHPAURECT@@HH@Z)(Buffer* this, RECT*, int, int)
// callers   1   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x005DE79D 0x005DE80C 0x005DE8D4
// provenance (retired seed): src/unrecovered/005de730.cpp. Homed for COVERAGE,
// no byte claim: the rectangle remap of the scalar `change_color` overloads,
// over the same IntersectRect clip and `get_data`/`free_data` lock pair the
// blitters use, replacing `find_color` with `replace_color` in place.
int Buffer::change_color(RECT *area, int find_color, int replace_color) {
    if (area == 0) {
        return 3;
    }
    if (dib_bits_ == 0 && surface_ == 0) {
        return 7;
    }

    RECT clipped;
    clipped.left = area->left;
    clipped.top = area->top;
    clipped.right = clipped.left + (area->right - area->left);
    clipped.bottom = clipped.top + (area->bottom - area->top);
    if (!IntersectRect(&clipped, &rect1_, &clipped)) {
        return 0;
    }
    if (!(clipped.left < dib_.bmiHeader.biWidth
          && clipped.top < -dib_.bmiHeader.biHeight)) {
        return 7;
    }
    if (get_data() == 0) {
        return 7;
    }

    uint8_t *pixel = static_cast<uint8_t *>(locked_bits_)
        + stride_ * clipped.top + clipped.left;
    if (pixel == 0) {
        return 7;
    }
    const int width = clipped.right - clipped.left;
    const int height = clipped.bottom - clipped.top;
    const int remainder = static_cast<int>(stride_) - width;
    const uint8_t find = static_cast<uint8_t>(find_color);
    const uint8_t replace = static_cast<uint8_t>(replace_color);
    for (int row = 0; row < height; ++row) {
        for (int col = 0; col < width; ++col) {
            if (*pixel == find) {
                *pixel = replace;
            }
            ++pixel;
        }
        pixel += remainder;
    }

    free_data(1);
    return 0;
}

// ORIGINAL: 0x005DF590 ?write_pcx@Buffer@@QAEHPAD@Z 0x005DF590-0x005DF907;0x00662BFC-0x00662C11
// size      908 bytes
// prototype int (__thiscall ?write_pcx@Buffer@@QAEHPAD@Z)(Buffer* this, char*)
// callers   2   call targets   8
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00628380 0x006283E0 0x00628650 0x00628810 0x00628AF0 0x006453E0 0x00645470 0x00645930
// indirect  0x005DF668 0x005DF716 0x005DF8C0
// provenance (retired seed): src/unrecovered/005df590.cpp. Homed for COVERAGE,
// no byte claim. A type-5 eight-bit PCX writer: 0x80-byte header, the DIB rows
// RLE-run-encoded through `stride_`, one 0x0c separator, then the 256 palette
// entries as the RGB triplets the DIB stores BGRX. `.pcx` is appended when the
// caller's name carries no extension, and 4 reports a file that would not
// open. The `Filemap` local is what carries the EH frame, as in load_pcx.
int Buffer::write_pcx(char *filename) {
    Filemap map;

    char name[300];
    name[0] = '\0';
    strcat(name, filename);
    if (memrchr(name, name + strlen(name), '.') == 0) {
        strcat(name, ".pcx");
    }

    if (get_data() == 0) {
        return 3;
    }

    const int width = dib_.bmiHeader.biWidth;
    const int height = dib_.bmiHeader.biHeight;
    unsigned char *view = static_cast<unsigned char *>(map.create(
        name, static_cast<uint32_t>(width * height * -2 + 0x80), 1));
    if (view != 0) {
        unsigned char header[0x80];
        header[0] = 10;  // the ZSoft manufacturer byte
        header[1] = 5;   // version 3.0: a 256-entry palette follows
        header[2] = 1;   // run-length encoded
        header[3] = 8;   // eight bits per plane
        *reinterpret_cast<short *>(header + 4) = 0;
        *reinterpret_cast<short *>(header + 6) = 0;
        *reinterpret_cast<short *>(header + 8) = static_cast<short>(width) - 1;
        *reinterpret_cast<short *>(header + 0xA) =
            -1 - static_cast<short>(height);
        *reinterpret_cast<short *>(header + 0xC) = 0;
        *reinterpret_cast<short *>(header + 0xE) = 0;
        header[0x40] = 0;
        header[0x41] = 1;
        *reinterpret_cast<short *>(header + 0x42) =
            static_cast<short>(stride_);
        *reinterpret_cast<short *>(header + 0x44) = 0;
        *reinterpret_cast<short *>(header + 0x46) = 0;
        *reinterpret_cast<short *>(header + 0x48) = 0;
        memcpy(view, header, 0x80);

        const uint8_t *bits = static_cast<const uint8_t *>(locked_bits_);
        unsigned char *out = view + 0x80;
        int rows_left = -height;
        if (height != 0 && rows_left >= 0) {
            do {
                unsigned int col = 0;
                const int row_width = stride_;
                if (row_width != 0) {
                    do {
                        const uint8_t value = bits[col];
                        unsigned int run = 1;
                        uint8_t count = 1;
                        if (value == bits[col + 1]) {
                            unsigned int probe = col + 1;
                            do {
                                if (static_cast<unsigned int>(row_width) <= probe
                                    || count > 0x3e) {
                                    break;
                                }
                                count = count + 1;
                                run = count;
                                probe = count + col;
                            } while (value == bits[probe]);
                            if (count < 2) {
                                if (value > 0x3f) {
                                    *out++ =
                                        static_cast<uint8_t>(count | 0xc0);
                                }
                            } else {
                                *out++ = static_cast<uint8_t>(count | 0xc0);
                            }
                        } else {
                            if (value > 0x3f) {
                                *out++ = static_cast<uint8_t>(count | 0xc0);
                            }
                        }
                        *out++ = value;
                        col += run;
                    } while (col < static_cast<unsigned int>(row_width));
                }
                bits += stride_;
                --rows_left;
            } while (rows_left != 0);
        }
        *out++ = 0xc;

        // PCX wants RGB triplets; the DIB table is BGRX, so each entry is
        // written red, green, blue past the blue byte it starts on.
        const RGBQUAD *table = dib_.bmiColors;
        for (int entry = 0; entry < 0x100; ++entry) {
            out[0] = table[entry].rgbRed;
            out[1] = table[entry].rgbGreen;
            out[2] = table[entry].rgbBlue;
            out += 3;
        }

        free_data(1);
        map.close(out);
        return 0;
    }

    free_data(1);
    return 4;
}

// ORIGINAL: 0x005DF910 ?write_bmp@Buffer@@QAEHPAD@Z 0x005DF910-0x005DFB48
// size      568 bytes
// prototype int (__thiscall ?write_bmp@Buffer@@QAEHPAD@Z)(Buffer* this, char*)
// callers   0   call targets   4
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00634BB0 0x00645470 0x00645598 0x0064603F
// indirect  0x005DF9AD 0x005DFB1C
// provenance (retired seed): src/unrecovered/005df910.cpp. Homed for COVERAGE,
// no byte claim. An uncompressed 8-bit BMP: ".bmp" appended unconditionally,
// the 14-byte file header and 40-byte info header, the DIB's own colour table,
// then the rows bottom-up (`biHeight` is stored negative). 1 reports a file
// that would not open.
int Buffer::write_bmp(char *filename) {
    char name[0xB0];
    name[0] = '\0';
    strcat(name, filename);
    strcat(name, ".bmp");
    FILE *file = env_open(name, "w+b");
    if (file == 0) {
        return 1;
    }

    if (get_data() == 0) {
        fclose(file);
        return 3;
    }

    const int width = dib_.bmiHeader.biWidth;
    const int height = dib_.bmiHeader.biHeight;

    BITMAPFILEHEADER file_header;
    BITMAPINFOHEADER info_header;
    file_header.bfType = 0x4d42;  // 'B','M'
    file_header.bfSize = static_cast<DWORD>(0x436 - stride_ * height);
    file_header.bfReserved1 = 0;
    file_header.bfReserved2 = 0;
    file_header.bfOffBits = 0x436;

    info_header.biSize = 0x28;
    info_header.biWidth = width;
    info_header.biHeight = -height;
    info_header.biPlanes = 1;
    info_header.biBitCount = 8;
    info_header.biCompression = BI_RGB;
    info_header.biSizeImage = -(width * height);
    info_header.biXPelsPerMeter = 0;
    info_header.biYPelsPerMeter = 0;
    info_header.biClrUsed = 0x100;
    info_header.biClrImportant = 0x100;

    fwrite(&file_header, 0xe, 1, file);
    fwrite(&info_header, 0x28, 1, file);
    fwrite(dib_.bmiColors, 0x400, 1, file);

    // The LAST row first: `height` is negative, so the start sits
    // (rows - 1) rows into the pixel block and walks backwards.
    uint8_t *row = static_cast<uint8_t *>(locked_bits_)
        - (height + 1) * static_cast<int>(stride_);
    int rows = -height;
    if (height != 0 && rows > -1) {
        do {
            fwrite(row, width, 1, file);
            row -= stride_;
            rows--;
        } while (rows != 0);
    }

    free_data(1);
    fclose(file);
    return 0;
}

// ORIGINAL: 0x005E10C0 ?hline@Buffer@@QAEXHHHHHHHH@Z 0x005E10C0-0x005E134E
// size      654 bytes
// prototype void (__thiscall ?hline@Buffer@@QAEXHHHHHHHH@Z)(Buffer* this, int, int, int, int, int, int, int, int)
// callers   1   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x005E11DF 0x005E1339
// provenance (retired seed): src/recovered/units/005e10c0.cpp. Homed for
// COVERAGE, no byte claim. The clipped horizontal writer with a dash pattern
// beside it: two colours, an on length, an off length and a phase, the same
// five words the nine-argument `line` names. Reversing the run re-expresses
// the phase against the swapped on/off lengths.
void Buffer::hline(int x1, int x2, int y, int first_color, int second_color,
                   int pattern_on, int pattern_off, int phase) {
    if (dib_bits_ == nullptr && surface_ == nullptr) {
        return;
    }
    if (y < rect1_.top || y >= rect1_.bottom) {
        return;
    }
    if (x1 == x2) {
        return;
    }
    if (first_color == -1 && second_color == -1) {
        return;
    }
    if (x2 < x1) {
        int swap = x1;
        x1 = x2;
        x2 = swap;
        swap = first_color;
        first_color = second_color;
        second_color = swap;
        phase = (pattern_off - phase) + pattern_on;
        swap = pattern_on;
        pattern_on = pattern_off;
        pattern_off = swap;
    }
    if (x1 >= rect1_.right) {
        return;
    }
    if (x2 < rect1_.left) {
        return;
    }
    if (x1 < rect1_.left) {
        x1 = rect1_.left;
    }
    if (x2 >= rect1_.right) {
        x2 = rect1_.right - 1;
    }
    if (x1 >= dib_.bmiHeader.biWidth) {
        return;
    }
    if (y >= -dib_.bmiHeader.biHeight) {
        return;
    }
    if (get_data() == 0) {
        return;
    }

    uint8_t *bits = static_cast<uint8_t *>(locked_bits_) + stride_ * y + x1;
    if (bits == nullptr) {
        return;
    }

    int phase_value = phase % (pattern_on + pattern_off);
    if (phase_value < pattern_on) {
        phase_value = phase_value - pattern_on;
    } else {
        phase_value = (pattern_on - phase_value) + pattern_off;
    }
    const int negated_on = -pattern_on;
    int count = (x2 - x1) + 1;

    if (first_color == -1) {
        // Only the second colour paints; the pattern marks the gaps.
        do {
            if (phase_value < 0) {
                ++phase_value;
                if (phase_value == 0) {
                    phase_value = pattern_off;
                }
            } else {
                *bits = static_cast<uint8_t>(second_color);
                --phase_value;
                if (phase_value == 0) {
                    phase_value = negated_on;
                }
            }
            ++bits;
        } while (--count != 0);
    } else if (second_color == -1) {
        do {
            if (phase_value < 0) {
                *bits = static_cast<uint8_t>(first_color);
                ++phase_value;
                if (phase_value == 0) {
                    phase_value = pattern_off;
                }
            } else {
                --phase_value;
                if (phase_value == 0) {
                    phase_value = negated_on;
                }
            }
            ++bits;
        } while (--count != 0);
    } else {
        do {
            if (phase_value < 0) {
                *bits = static_cast<uint8_t>(first_color);
                ++phase_value;
                if (phase_value == 0) {
                    phase_value = pattern_off;
                }
            } else {
                *bits = static_cast<uint8_t>(second_color);
                --phase_value;
                if (phase_value == 0) {
                    phase_value = negated_on;
                }
            }
            ++bits;
        } while (--count != 0);
    }

    free_data(1);
}


// Original entries 5D8650, 5D8FE0 and 5D91D0 have the same Buffer receiver,
// field accesses and shared Sprite publisher as draw_dest_source at 5D8DF0.
// These are actual Buffer members, not an invented subclass or a second
// Sprite layout. Existing Buffer friendship grants access to the real owner.
// draw_unk2 is provisional; its callee at 5E4AEA is an original zero-return
// drawing hook. The destination remains a genuine Buffer pointer throughout.
// ORIGINAL: 0x005D8650 sub_5d8650 0x005D8650-0x005D873E FILE BYTE_EXACT
// symbol    ?draw_unk2@Buffer@@QAEHPAV1@HHHH@Z
// size      238 bytes
int Buffer::draw_unk2(Buffer *source, int transparent_in, int x, int y,
                           int table) {
    int transparent = transparent_in;
    if (surface_ == 0) {
        if (transparent_in == -1) {
            transparent = WinFillColour;
        }
        locked_bits_ = dib_bits_;
        if (locked_bits_ != nullptr) {
            surface_lock_count_ = surface_lock_count_ + 1;
        }
        g_BUFFER_SPRITE.pcBits_ = static_cast<char *>(locked_bits_);
        g_BUFFER_SPRITE.iSpriteWidth2_ = stride_;
        g_BUFFER_SPRITE.iSpriteWidth_ = dib_.bmiHeader.biWidth;
        g_BUFFER_SPRITE.iSpriteHeight_ = -dib_.bmiHeader.biHeight;
        g_BUFFER_SPRITE.iWidth_ = dib_.bmiHeader.biWidth;
        g_BUFFER_SPRITE.iHeight_ = -dib_.bmiHeader.biHeight;
        g_BUFFER_SPRITE.iLeftOffset_ = 0;
        g_BUFFER_SPRITE.iTopOffset_ = 0;
        g_BUFFER_SPRITE.cTransparentIndex_ = (unsigned char)transparent;
        g_BUFFER_SPRITE.fObj1Exists_ = 0;

        if (surface_ == 0) {
            surface_lock_count_ = surface_lock_count_ - 1;
            if ((int)surface_lock_count_ > 0) {
                goto tail;
            }
        } else {
            surface_lock_count_ = surface_lock_count_ - 1;
            if (locked_bits_ == nullptr || (int)surface_lock_count_ > 0) {
                goto tail;
            }
            surface_->Unlock(locked_bits_);
        }
        locked_bits_ = nullptr;
        surface_lock_count_ = 0;
    }
tail:
    {
        int result = g_BUFFER_SPRITE.draw_noop(source,
                                          transparent_in, x, y, table);
        g_BUFFER_SPRITE.pcBits_ = 0;
        return result;
    }
}


// ORIGINAL: 0x005D8FE0 sub_5d8fe0 0x005D8FE0-0x005D90CE FILE BYTE_EXACT
// symbol    ?draw@Buffer@@QAEHPAV1@HHHPAE@Z
// size      238 bytes
int Buffer::draw(Buffer *source, int transparent_in, int x, int y,
                           unsigned char *table) {
    int transparent = transparent_in;
    if (surface_ == 0) {
        if (transparent_in == -1) {
            transparent = WinFillColour;
        }
        locked_bits_ = dib_bits_;
        if (locked_bits_ != nullptr) {
            surface_lock_count_ = surface_lock_count_ + 1;
        }
        g_BUFFER_SPRITE.pcBits_ = static_cast<char *>(locked_bits_);
        g_BUFFER_SPRITE.iSpriteWidth2_ = stride_;
        g_BUFFER_SPRITE.iSpriteWidth_ = dib_.bmiHeader.biWidth;
        g_BUFFER_SPRITE.iSpriteHeight_ = -dib_.bmiHeader.biHeight;
        g_BUFFER_SPRITE.iWidth_ = dib_.bmiHeader.biWidth;
        g_BUFFER_SPRITE.iHeight_ = -dib_.bmiHeader.biHeight;
        g_BUFFER_SPRITE.iLeftOffset_ = 0;
        g_BUFFER_SPRITE.iTopOffset_ = 0;
        g_BUFFER_SPRITE.cTransparentIndex_ = (unsigned char)transparent;
        g_BUFFER_SPRITE.fObj1Exists_ = 0;

        if (surface_ == 0) {
            surface_lock_count_ = surface_lock_count_ - 1;
            if ((int)surface_lock_count_ > 0) {
                goto tail;
            }
        } else {
            surface_lock_count_ = surface_lock_count_ - 1;
            if (locked_bits_ == nullptr || (int)surface_lock_count_ > 0) {
                goto tail;
            }
            surface_->Unlock(locked_bits_);
        }
        locked_bits_ = nullptr;
        surface_lock_count_ = 0;
    }
tail:
    {
        int result = g_BUFFER_SPRITE.draw(source, transparent_in, x, y, table);
        g_BUFFER_SPRITE.pcBits_ = 0;
        return result;
    }
}


// ORIGINAL: 0x005D91D0 sub_5d91d0 0x005D91D0-0x005D92BE FILE BYTE_EXACT
// symbol    ?draw_dest_unk2@Buffer@@QAEHPAV1@HHHPAE@Z
// size      238 bytes
int Buffer::draw_dest_unk2(Buffer *source, int transparent_in, int x, int y,
                           unsigned char *table) {
    int transparent = transparent_in;
    if (surface_ == 0) {
        if (transparent_in == -1) {
            transparent = WinFillColour;
        }
        locked_bits_ = dib_bits_;
        if (locked_bits_ != nullptr) {
            surface_lock_count_ = surface_lock_count_ + 1;
        }
        g_BUFFER_SPRITE.pcBits_ = static_cast<char *>(locked_bits_);
        g_BUFFER_SPRITE.iSpriteWidth2_ = stride_;
        g_BUFFER_SPRITE.iSpriteWidth_ = dib_.bmiHeader.biWidth;
        g_BUFFER_SPRITE.iSpriteHeight_ = -dib_.bmiHeader.biHeight;
        g_BUFFER_SPRITE.iWidth_ = dib_.bmiHeader.biWidth;
        g_BUFFER_SPRITE.iHeight_ = -dib_.bmiHeader.biHeight;
        g_BUFFER_SPRITE.iLeftOffset_ = 0;
        g_BUFFER_SPRITE.iTopOffset_ = 0;
        g_BUFFER_SPRITE.cTransparentIndex_ = (unsigned char)transparent;
        g_BUFFER_SPRITE.fObj1Exists_ = 0;

        if (surface_ == 0) {
            surface_lock_count_ = surface_lock_count_ - 1;
            if ((int)surface_lock_count_ > 0) {
                goto tail;
            }
        } else {
            surface_lock_count_ = surface_lock_count_ - 1;
            if (locked_bits_ == nullptr || (int)surface_lock_count_ > 0) {
                goto tail;
            }
            surface_->Unlock(locked_bits_);
        }
        locked_bits_ = nullptr;
        surface_lock_count_ = 0;
    }
tail:
    {
        int result = g_BUFFER_SPRITE.draw_dest_unk2(source, transparent_in, x,
                                                    y, table);
        g_BUFFER_SPRITE.pcBits_ = 0;
        return result;
    }
}

// ORIGINAL: 0x005E1350 ?vline@Buffer@@QAEXHHHHHHHH@Z 0x005E1350-0x005E15E7
// size      663 bytes
// prototype void (__thiscall ?vline@Buffer@@QAEXHHHHHHHH@Z)(Buffer* this, int, int, int, int, int, int, int, int)
// callers   1   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x005E1471 0x005E15D2
// provenance (retired seed): src/recovered/units/005e1350.cpp. Homed for
// COVERAGE, no byte claim. The patterned `hline` above with the run walking
// down a whole `stride_` per pixel instead of one byte.
void Buffer::vline(int x, int y1, int y2, int first_color, int second_color,
                   int pattern_on, int pattern_off, int phase) {
    if (dib_bits_ == nullptr && surface_ == nullptr) {
        return;
    }
    if (x < rect1_.left || x >= rect1_.right) {
        return;
    }
    if (y1 == y2) {
        return;
    }
    if (first_color == -1 && second_color == -1) {
        return;
    }
    if (y2 < y1) {
        int swap = y1;
        y1 = y2;
        y2 = swap;
        swap = first_color;
        first_color = second_color;
        second_color = swap;
        phase = (pattern_off - phase) + pattern_on;
        swap = pattern_on;
        pattern_on = pattern_off;
        pattern_off = swap;
    }
    if (y1 >= rect1_.bottom) {
        return;
    }
    if (rect1_.top > y2) {
        return;
    }
    if (y1 < rect1_.top) {
        y1 = rect1_.top;
    }
    if (y2 >= rect1_.bottom) {
        y2 = rect1_.bottom - 1;
    }
    if (x >= dib_.bmiHeader.biWidth) {
        return;
    }
    if (y1 >= -dib_.bmiHeader.biHeight) {
        return;
    }
    if (get_data() == 0) {
        return;
    }

    uint8_t *bits = static_cast<uint8_t *>(locked_bits_) + stride_ * y1 + x;
    if (bits == nullptr) {
        return;
    }

    int phase_value = phase % (pattern_on + pattern_off);
    if (phase_value < pattern_on) {
        phase_value = phase_value - pattern_on;
    } else {
        phase_value = (pattern_on - phase_value) + pattern_off;
    }
    const int negated_on = -pattern_on;
    int count = (y2 - y1) + 1;

    if (first_color == -1) {
        do {
            if (phase_value < 0) {
                ++phase_value;
                if (phase_value == 0) {
                    phase_value = pattern_off;
                }
            } else {
                *bits = static_cast<uint8_t>(second_color);
                --phase_value;
                if (phase_value == 0) {
                    phase_value = negated_on;
                }
            }
            bits += stride_;
        } while (--count != 0);
    } else if (second_color == -1) {
        do {
            if (phase_value < 0) {
                *bits = static_cast<uint8_t>(first_color);
                ++phase_value;
                if (phase_value == 0) {
                    phase_value = pattern_off;
                }
            } else {
                --phase_value;
                if (phase_value == 0) {
                    phase_value = negated_on;
                }
            }
            bits += stride_;
        } while (--count != 0);
    } else {
        do {
            if (phase_value < 0) {
                *bits = static_cast<uint8_t>(first_color);
                ++phase_value;
                if (phase_value == 0) {
                    phase_value = pattern_off;
                }
            } else {
                *bits = static_cast<uint8_t>(second_color);
                --phase_value;
                if (phase_value == 0) {
                    phase_value = negated_on;
                }
            }
            bits += stride_;
        } while (--count != 0);
    }

    free_data(1);
}
