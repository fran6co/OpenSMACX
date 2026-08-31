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
#include "filemap.h"
#include "font.h"
#include "general.h"   // mem_get, for the link labels this owns
#include "palette.h"
#include "spot.h"
#include "temp.h"      // BufferStrHeight, which the wrap family scales by
#include "alpha.h"     // Language, the write/wrap text-selection switch

#include <new>

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
// Slot 1 of the vtable - the do-nothing virtual, returning 0.
//
// NO `ORIGINAL:` MARKER, deliberately, and the gate caught the first
// attempt: an annotation here makes a SECOND claim on 0x00406B30, which
// `annotation_scan` reports as a duplicate and drops from both sides. The
// claim count fell by one - `nullsub_thunks.cpp` stopped counting - for a
// comment.
//
// It is not claimed here because it is not this file's function. `src/nullsub_thunks.cpp`
// already owns 0x00406B30 as `sub_406b30`, byte-exact, and 129 vtable slots
// across 70 classes point at it - Buffer's slot 1 among them. There is ONE
// three-byte function in the image, not 129: `/Gy` gives every function its
// own COMDAT and the linker folded every identical empty override onto a
// single copy.
//
// So the recovery of "Buffer::surface_lost" is not a second body. It is
// declaring this override so that it compiles to the same three bytes and
// folds the same way - `xor eax, eax; ret`, which is why it returns `int`
// and returns 0. Written `void` it is `ret` alone, one byte, and Buffer's
// slot 1 would stop being the function the image put there. Verified by
// compiling this unit and reading the symbol out of the object:
// `?surface_lost@Buffer@@UAEHXZ` is `33 c0 c3`, the three bytes at
// 0x00406B30.
int Buffer::surface_lost() { return 0; }

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
// body src/buffer.h



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

// 0x009B3A10, and it is a GLOBAL for one reason: the `__asm` block in
// `Buffer::fill(RECT *, int)` needs a memory location it can name, and a
// compiler-allocated local has no name assembly can write. It holds the
// bytes to skip between one filled row and the next, `stride_ - width`, and
// it is live only for the duration of that loop. The whole image references
// it from that one function and nowhere else, which is what a scratch
// location smuggled out to file scope looks like.
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

// Slot 1 of a Buffer's vtable answers 0 here and an object elsewhere: a
// subclass that owns its own surface returns it, and this decoder asks that
// object to size itself instead of calling `Buffer::init`. Nothing in this
// tree recovers the subclass, so all that is known about the object is the
// slot being called - which is what this declares and no more.
struct SurfaceOwner {
    virtual int slot0();
    virtual int slot1();
    virtual int slot2();
    virtual int resize(int width, int height, int flags);
};

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

    const int owner = surface_lost();
    const int sized = (owner == 0)
        ? init(width, height, static_cast<int>(init_flags_), nullptr)
        : reinterpret_cast<SurfaceOwner *>(owner)->resize(width, height, 1);
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
            surface_lost();
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
                surface_lost();
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
                        surface_lost();
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
    return copy(buffer, xCoord, yCoord, xCoord, yCoord, width, height);
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
    return copy(buffer, left, top, left, top,
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
        surface_lost();
    }

    if (surface_ == nullptr) {
        hdc2_ = hdc_;
    } else if (hdc2_ == nullptr) {
        if (surface_->GetDC(&hdc2_) != 0) {
            surface_lost();
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
                surface_lost();
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

    uint8_t *destination = static_cast<uint8_t *>(pixels)
        + stride_ * clipped.top + clipped.left;
    if (destination == nullptr) {
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
        uint32_t *whole = reinterpret_cast<uint32_t *>(destination);
        for (uint32_t count = width >> 2; count > 0; --count) {
            *whole++ = quad;
        }
        uint8_t *tail = reinterpret_cast<uint8_t *>(whole);
        for (count = width & 3; count > 0; --count) {
            *tail++ = value;
        }
        destination += BufferFillRowGap;
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
//   (1) `uv run tools/handwritten_asm.py` lists this address for `pushf` and
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
int Buffer::copy(Buffer *buffer, int xCoord, int yCoord, int wx, int wy,
                 int width, int height) {
    if (dib_bits_ == nullptr && surface_ == nullptr) {
        return 7;
    }
    if (buffer == nullptr) {
        return 3;
    }

    // A negative read position shrinks the extent and shifts the write
    // position by the overhang, then clamps: the standard blit clip.
    if (xCoord < 0) {
        width += xCoord;
        wx -= xCoord;
        xCoord = 0;
    }
    if (yCoord < 0) {
        height += yCoord;
        wy -= yCoord;
        yCoord = 0;
    }

    // `biHeight` is stored negative, so `-biHeight` is the row count.
    if (xCoord > dib_.bmiHeader.biWidth
        || yCoord > -dib_.bmiHeader.biHeight) {
        return 0;
    }
    if (xCoord + width > dib_.bmiHeader.biWidth) {
        width = dib_.bmiHeader.biWidth - xCoord;
    }
    if (yCoord + height > -dib_.bmiHeader.biHeight) {
        height = -(yCoord + dib_.bmiHeader.biHeight);
    }

    // The write rectangle, clipped against the destination's own clip rect.
    RECT clipped;
    clipped.left = wx;
    clipped.top = wy;
    clipped.right = wx + width;
    clipped.bottom = wy + height;
    if (!IntersectRect(&clipped, &clipped, &buffer->rect1_)) {
        return 0;
    }

    // Whatever the intersection moved the write rect by, the read position
    // moves with it; the extent comes from the intersected rect.
    xCoord += clipped.left - wx;
    yCoord += clipped.top - wy;
    width = clipped.right - clipped.left;
    height = clipped.bottom - clipped.top;

    if (clipped.left >= buffer->dib_.bmiHeader.biWidth
        || clipped.top >= -buffer->dib_.bmiHeader.biHeight
        || buffer->get_data() == 0) {
        return 3;
    }
    uint8_t *const write = static_cast<uint8_t *>(buffer->locked_bits_)
        + buffer->stride_ * clipped.top + clipped.left;
    if (write == nullptr) {
        return 3;
    }

    if (xCoord >= dib_.bmiHeader.biWidth
        || yCoord >= -dib_.bmiHeader.biHeight
        || get_data() == 0) {
        buffer->free_data(1);
        return 3;
    }
    uint8_t *const read = static_cast<uint8_t *>(locked_bits_)
        + stride_ * yCoord + xCoord;
    if (read == nullptr) {
        buffer->free_data(1);
        return 3;
    }

    if (buffer != this) {
        // Different objects cannot overlap, so rows go in either order.
        uint8_t *destination = write;
        const uint8_t *source = read;
        for (int row = 0; row < height; ++row) {
            memcpy(destination, source, static_cast<size_t>(width));
            source += stride_;
            destination += buffer->stride_;
        }
    } else {
        // SAME BUFFER, so the two rectangles can overlap. Rows are walked
        // from whichever end keeps the read ahead of the write, and each row
        // is moved rather than copied because the horizontal shift can
        // overlap too.
        const int step = (wy < yCoord) ? 1 : -1;
        const int first = (wy < yCoord) ? 0 : height - 1;
        uint8_t *destination = write + first * static_cast<int>(buffer->stride_);
        uint8_t *source = read + first * static_cast<int>(stride_);
        for (int row = 0; row < height; ++row) {
            memmove(destination, source, static_cast<size_t>(width));
            destination += step * static_cast<int>(buffer->stride_);
            source += step * static_cast<int>(stride_);
        }
    }

    free_data(1);
    buffer->free_data(1);
    return 0;
}

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
                        surface_lost();
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


const uint32_t BufferVtable = 0x0066FDBC;

/*
Purpose: Destroy a Buffer by installing its virtual table, releasing every
         owned resource, and destroying the trailing Spot subobject.

         THE BODY IS `~Buffer() { close(); }` IN THE HEADER, and those three
         tokens are the whole function. The vtable store, the `Spot` at
         +0x4B0 and the EH frame that unwinds it are all things the compiler
         emits around a virtual destructor with a member of class type; the
         image's 20 instructions are what VC6 writes for that source, so
         spelling any of them out by hand - as the stand-in here used to,
         through a volatile pointer and a raw 0x4B0 - both missed the match
         and described the layout twice.
// ORIGINAL: 0x005D7410 ??1Buffer@@QAE@XZ 0x005D7410-0x005D7463;0x00662B68-0x00662B80 BYTE_EXACT
// symbol    ??1Buffer@@UAE@XZ
// body      src/buffer.h
// size      107 bytes
// prototype void (__thiscall ??1Buffer@@QAE@XZ)(Buffer* this)
// callers   87   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005D7470 0x005FA870
// notes     Runtime redirect installed by DllMain after byte-signature validation
Status: Complete
*/

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
// a hand-assembly idiom rather than a compiler's. Found by
// `tools/handwritten_asm.py`, which scans every unclaimed body for opcodes
// this compiler does not generate; there are seven in the game code.
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
opens a region, `=` captures its label into `owned_[link_count_]`, and each
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
                    owned_[link_count_] = kept;
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
int Buffer::poly(Vert *verts, int a2, int a3) {
    return polygon(this, verts, a2, a3);
}

/*
 * A forwarder, not a recovery. ?polygon@@YAHPAUBuffer@@PAUVert@@HH@Z at
 * 0x00626620 is 736 bytes and undecoded, but Buffer::poly calls it, so the
 * DLL has to resolve the symbol. No `Original Offset:` line by design: that
 * annotation marks a recovered body and is indexed by address.
 *
 * The address is cast inside the body rather than at file scope so nothing is
 * dereferenced before main() - tools/verify_no_load_time_addresses.py exists
 * because a file-scope seam through an unmapped address took the process down
 * before the first line of it ran.
 *
 * auto_inline(off) is load-bearing. OPENSMACX_NOINLINE expands to NOTHING on
 * VC6 (src/vc6_compat.h gates __declspec(noinline) on _MSC_VER > 1200 and VC6
 * IS 1200), and at /Ob2 cl folds a one-line forwarder into a caller in the
 * same translation unit even when it is defined after it, because codegen is
 * deferred to end of TU. Buffer::poly is a recovered body; inlining this into
 * it would replace its `call rel32` and cost its byte match.
 */
#pragma auto_inline(off)
int __cdecl polygon(Buffer *buffer, Vert *verts, int a3, int a4) {
    typedef int(__cdecl * func_polygon)(Buffer *, Vert *, int, int);
    return reinterpret_cast<func_polygon>(0x00626620)(buffer, verts, a3, a4);
}
#pragma auto_inline(on)

// ORIGINAL: 0x005DD300 ?write_right_l@Buffer@@QAEHPADHHHH@Z 0x005DD300-0x005DD3A5 FILE
// LEVER: X BEFORE Y, the reverse of what the note below used to claim. The
//        image adds `field_width - text_width` to the SECOND parameter and
//        passes that as write_multi_font_raw_l's second argument, and its
//        early exits return the second parameter untouched - and buffer.h's
//        own fact about the raster writer says the scalar writers "return
//        the incoming x". So the second parameter is the x, the third the
//        y, and the alignment offset lands on x. Two decompiled callers
//        (src/unrecovered/005ddbb0.cpp, src/recovered/units/004b0410.cpp)
//        independently name the third argument `y`, which agrees. Swapping
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

// THE BLIT DESCRIPTOR AT 0x009B3A54.. - REAL STORAGE, ten zero-initialised
// globals (the image's slots are zero-fill past the carried bytes). They were
// `static int *const BlitSourceBits = (int *)0x009B3A54` and nine neighbours
// naming terranx.exe data that is unmapped in a standalone build. IS NAMED
// FROM ANOTHER BODY'S OWN PARAMETERS: src/unrecovered/005d92c0.cpp writes the
// same ten addresses from locals it has already named - `bits`, `*pWidth`,
// `-*pHeight`, `transparentIndex` - so the names are evidence rather than
// inference. The three that stay anonymous (0x70, 0x74, 0x78) are written as
// literal 0 in BOTH bodies, which says nothing about what they hold. The
// transparent index is a BYTE-typed object: the image stores it with
// `mov byte ptr [0x9b3a58], cl`, which is how VC6 writes a uint8_t global.
// Every store keeps its folded `[disp32]` form, the displacement relocated.
int BlitSourceBits;        // 0x009B3A54
uint8_t BlitTransparentIndex;  // 0x009B3A58, written one byte wide
static int BlitSourceField4A8;  // 0x009B3A5C
static int BlitSourceWidth;     // 0x009B3A60
static int BlitSourceNegHeight;  // 0x009B3A64
static int BlitClipWidth;       // 0x009B3A68
static int BlitClipNegHeight;   // 0x009B3A6C
static int BlitSourceField70;          // 0x009B3A70
static int BlitSourceField74;          // 0x009B3A74
static int BlitSourceField78;          // 0x009B3A78

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
    BlitSourceBits = reinterpret_cast<int>(bits);
    BlitSourceField4A8 = stride_;
    BlitSourceWidth = dib_.bmiHeader.biWidth;
    BlitSourceNegHeight = -dib_.bmiHeader.biHeight;
    BlitClipWidth = dib_.bmiHeader.biWidth;
    BlitClipNegHeight = -dib_.bmiHeader.biHeight;
    BlitSourceField70 = 0;
    BlitSourceField74 = 0;
    BlitTransparentIndex = (uint8_t)colour;
    BlitSourceField78 = 0;

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

// ====================
Sprite g_BUFFER_SPRITE;

// ===== MANAGED GLOBALS - real objects, homed to their domain =====
// In the shipped image these live at fixed data addresses and are
// constructed before WinMain by the CRT's dynamic-initializer walk
// (winedbg-confirmed: walker return 0x00644EEB, table at 0x682568..).
// Here the same recovered constructors run through this build's own
// startup, and the matching destructors close them at exit. Addresses of
// the ones documented individually live beside their definitions.
Buffer g_PCX_PARSE_TEMP_BUFFER1;  // 0x00798668
Buffer g_IFACE_BOX_SPRITE_BUFFERS[51];  // 0x0075C218, 0x588 stride
Buffer g_IFACE_STD_POPUPS_MIDDLE_BUFFER;  // 0x0077AA70
Buffer g_VEHDRAW_BUFFER;  // 0x008CC298
Buffer g_BUFFER;  // 0x009B6080
// ===== MANAGED GLOBALS - real objects, homed to their domain =====
// In the shipped image these live at fixed data addresses and are
// constructed before WinMain by the CRT's dynamic-initializer walk
// (winedbg-confirmed: walker return 0x00644EEB, table at 0x682568..).
// Here the same recovered constructors run through this build's own
// startup, and the matching destructors close them at exit.
