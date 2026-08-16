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
#include "font.h"
#include "palette.h"
#include "spot.h"

#include <new>
// `Buffer::init` creates the surface and clipper, and names
// IID_IDirectDrawSurface - which lives in dxguid.lib. Linked the way
// `time.h` links Winmm, rather than in CMakeLists, so the dependency
// sits beside the code that needs it.
#pragma comment(lib, "dxguid.lib")
#include <ddraw.h>

Palette **BufferPalette = reinterpret_cast<Palette **>(0x009B8174);

/*
Purpose: Construct an empty Buffer, including its Spot subobject, text state,
         and either the process palette or the legacy grayscale fallback.
ORIGINAL: 0x005D7210
// name      ??0Buffer@@QAE@XZ
// size      530 bytes
// spans     0x005D7210-0x005D740A;0x00662B50-0x00662B68
// prototype void (__thiscall ??0Buffer@@QAE@XZ)(Buffer* this)
// callers   80   call targets   2
// kind      game
// flags     sp_ready;purged_ok
// calls     0x005FA860 0x005FE560
// notes     Runtime redirect installed by DllMain after byte-signature validation
Status: Complete
Verification note: five mutation-harness survivors here are equivalent by
construction. Widening the 0x4BC loop bound writes an extra zero at 0x50C,
which the 0xFFFFFFFF store below overwrites. Widening the fallback ramp bound
writes a 257th entry at 0x4A4, where the index truncates to zero and all four
bytes land on the zero already stored there. The three adjacent stores inside
that ramp target distinct bytes with independent values, so their order is not
observable.
*/
void Buffer::construct() {
    new (&spot_) Spot();
    volatile uint32_t *const object =
        reinterpret_cast<volatile uint32_t *>(this);
    object[0x000 / 4] = BufferVtable;
    for (size_t offset = 0x4BC; offset < 0x50C; offset += 4) {
        object[offset / 4] = 0;
    }
    object[0x584 / 4] = 0;
    object[0x00C / 4] = 0;
    object[0x008 / 4] = 0;
    object[0x014 / 4] = 0;
    object[0x010 / 4] = 0;
    object[0x018 / 4] = 0;
    object[0x01C / 4] = 0;
    object[0x050 / 4] = 0;
    object[0x054 / 4] = 0;
    object[0x058 / 4] = 0;
    object[0x05C / 4] = 0;
    object[0x060 / 4] = 0;
    object[0x064 / 4] = 0;
    object[0x068 / 4] = 0;
    object[0x06C / 4] = 0;
    object[0x074 / 4] = 0;
    object[0x078 / 4] = 0;
    object[0x4A4 / 4] = 0;
    object[0x4A8 / 4] = 0;
    object[0x50C / 4] = 0xFFFFFFFFU;
    object[0x510 / 4] = 0;
    object[0x514 / 4] = 0;
    object[0x518 / 4] = 0;
    object[0x51C / 4] = 0;
    object[0x520 / 4] = *BufferResetValue520;
    object[0x524 / 4] = 0;
    object[0x530 / 4] = 0;
    object[0x52C / 4] = reinterpret_cast<uintptr_t>(*FontDefaultPtr);
    object[0x53C / 4] = 0;
    object[0x54C / 4] = 0xFFFFFFFFU;
    object[0x55C / 4] = 2;
    object[0x56C / 4] = 2;
    object[0x540 / 4] = 0xFFFFFFFFU;
    object[0x550 / 4] = 0xFFFFFFFFU;
    object[0x560 / 4] = 2;
    object[0x570 / 4] = 2;
    object[0x534 / 4] = 0;
    object[0x544 / 4] = 0xFFFFFFFFU;
    object[0x554 / 4] = 0xFFFFFFFFU;
    object[0x564 / 4] = 2;
    object[0x574 / 4] = 2;
    object[0x538 / 4] = 0;
    object[0x548 / 4] = 0xFFFFFFFFU;
    object[0x558 / 4] = 0xFFFFFFFFU;
    object[0x568 / 4] = 2;
    object[0x578 / 4] = 2;
    object[0x57C / 4] = 0;
    *reinterpret_cast<volatile uint8_t *>(
        reinterpret_cast<uint8_t *>(this) + 0x580) = 0;
    object[0x070 / 4] = 0;
    object[0x004 / 4] = 0;
    object[0x07C / 4] = 0x28;
    object[0x080 / 4] = 0;
    object[0x084 / 4] = 0;
    *reinterpret_cast<volatile uint16_t *>(
        reinterpret_cast<uint8_t *>(this) + 0x088) = 1;
    *reinterpret_cast<volatile uint16_t *>(
        reinterpret_cast<uint8_t *>(this) + 0x08A) = 8;
    object[0x08C / 4] = 0;
    object[0x090 / 4] = 0;
    object[0x094 / 4] = 0;
    object[0x098 / 4] = 0;
    object[0x09C / 4] = 0x100;
    object[0x0A0 / 4] = 0;

    Palette *const palette = *BufferPalette;
    if (palette) {
        palette->get_rgbquad(
            reinterpret_cast<RGBQUAD *>(
                reinterpret_cast<uint8_t *>(this) + 0x0A4),
            0, 0x100);
    } else {
        volatile uint8_t *entry =
            reinterpret_cast<volatile uint8_t *>(this) + 0x0A4;
        for (uint32_t index = 0; index < 0x100; ++index) {
            entry[2] = 0;
            entry[1] = 0;
            entry[0] = static_cast<uint8_t>(index);
            entry[3] = 0;
            entry += 4;
        }
    }
}

Buffer *__fastcall buffer_construct_redirect(Buffer *self, void *) {
    self->construct();
    return self;
}

/*
Purpose: Set the four fonts used by the buffer.
ORIGINAL: 0x005DAC70 BYTE_EXACT
// name      ?set_font@Buffer@@QAEHPAVFont@@PAVFont@@PAVFont@@PAVFont@@@Z
// size      64 bytes
// spans     0x005DAC70-0x005DACB0
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
ORIGINAL: 0x005DACB0 BYTE_EXACT
// name      ?set_text_color@Buffer@@QAEXHHHH@Z
// size      43 bytes
// spans     0x005DACB0-0x005DACDB
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
ORIGINAL: 0x005DACE0 BYTE_EXACT
// name      ?set_text_color2@Buffer@@QAEXHHHH@Z
// size      43 bytes
// spans     0x005DACE0-0x005DAD0B
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
ORIGINAL: 0x005DAD10 BYTE_EXACT
// name      ?set_text_color3@Buffer@@QAEXHHHH@Z
// size      43 bytes
// spans     0x005DAD10-0x005DAD3B
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
ORIGINAL: 0x005DAD40 BYTE_EXACT
// name      ?set_text_color_hyper@Buffer@@QAEXHHHH@Z
// size      43 bytes
// spans     0x005DAD40-0x005DAD6B
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
ORIGINAL: 0x005DF570 BYTE_EXACT
// name      ?init_class@Buffer@@QAAHXZ
// size      3 bytes
// spans     0x005DF570-0x005DF573
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
ORIGINAL: 0x005DF580 BYTE_EXACT
// name      ?close_class@Buffer@@QAAXXZ
// size      1 bytes
// spans     0x005DF580-0x005DF581
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

int __fastcall buffer_set_font_redirect(
    Buffer *self, void *, Font *font1, Font *font2, Font *font3, Font *font4) {
    return self->set_font(font1, font2, font3, font4);
}

void __fastcall buffer_set_text_color_redirect(
    Buffer *self, void *, int color1, int color2, int color3, int color4) {
    self->set_text_color(color1, color2, color3, color4);
}

void __fastcall buffer_set_text_color2_redirect(
    Buffer *self, void *, int color1, int color2, int color3, int color4) {
    self->set_text_color2(color1, color2, color3, color4);
}

void __fastcall buffer_set_text_color3_redirect(
    Buffer *self, void *, int color1, int color2, int color3, int color4) {
    self->set_text_color3(color1, color2, color3, color4);
}

void __fastcall buffer_set_text_color_hyper_redirect(
    Buffer *self, void *, int color1, int color2, int color3, int color4) {
    self->set_text_color_hyper(color1, color2, color3, color4);
}

int __cdecl buffer_init_class_redirect() {
    return 0;
}

void __cdecl buffer_close_class_redirect() {
}

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
ORIGINAL: 0x005E3373
// name      ?get_data@Buffer@@QAEHXZ
// size      119 bytes
// spans     0x005E3373-0x005E33EA
// prototype int (__thiscall ?get_data@Buffer@@QAEHXZ)(Buffer* this)
// callers   48   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x005E33BF
// notes     Runtime redirect installed by DllMain after byte-signature validation
Status: Complete
*/
int Buffer::get_data() {
    volatile uint32_t *ordered = reinterpret_cast<volatile uint32_t *>(this);
    void *const surface = reinterpret_cast<void *>(ordered[0x58 / 4]);
    if (!surface) {
        // Without a surface the buffer publishes its own storage, and the
        // store happens whether or not that storage exists.
        const uint32_t storage = ordered[0x54 / 4];
        ordered[0x50 / 4] = storage;
        if (!storage) {
            return 0;
        }
        ordered[0x6C / 4] = ordered[0x6C / 4] + 1;
        return static_cast<int>(storage);
    }
    const uint32_t existing = ordered[0x50 / 4];
    if (existing) {
        ordered[0x6C / 4] = ordered[0x6C / 4] + 1;
        return static_cast<int>(existing);
    }
    // The legacy body leaves the descriptor uninitialised apart from its size.
    uint8_t descriptor[SurfaceDescriptorSize];
    const uint32_t descriptor_size = SurfaceDescriptorSize;
    memcpy(descriptor, &descriptor_size, sizeof(descriptor_size));
    void **const vtable = *reinterpret_cast<void ***>(surface);
    const func_surface_lock lock = reinterpret_cast<func_surface_lock>(
        vtable[BufferSurfaceLockSlot / sizeof(void *)]);
    if (lock(surface, nullptr, descriptor, 1, nullptr) != 0) {
        return 0;
    }
    ordered[0x6C / 4] = ordered[0x6C / 4] + 1;
    uint32_t pitch;
    uint32_t data;
    memcpy(&pitch, descriptor + SurfaceDescriptorPitch, sizeof(pitch));
    memcpy(&data, descriptor + SurfaceDescriptorData, sizeof(data));
    ordered[0x4A8 / 4] = pitch;
    ordered[0x50 / 4] = data;
    return static_cast<int>(data);
}

/*
Purpose: Release acquired references to the buffer's pixel data, unlocking the
         DirectDraw surface once the last reference is dropped.
ORIGINAL: 0x005E34A3
// name      ?free_data@Buffer@@QAEXH@Z
// size      88 bytes
// spans     0x005E34A3-0x005E34FB
// prototype void (__thiscall ?free_data@Buffer@@QAEXH@Z)(Buffer* this, int)
// callers   43   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x005E34EA
// notes     Runtime redirect installed by DllMain after byte-signature validation
Status: Complete
*/
void Buffer::free_data(int count) {
    volatile uint32_t *ordered = reinterpret_cast<volatile uint32_t *>(this);
    void *const surface = reinterpret_cast<void *>(ordered[0x58 / 4]);
    const int32_t remaining = static_cast<int32_t>(
        ordered[0x6C / 4] - static_cast<uint32_t>(count));
    ordered[0x6C / 4] = static_cast<uint32_t>(remaining);
    if (!surface) {
        if (remaining <= 0) {
            ordered[0x50 / 4] = 0;
            ordered[0x6C / 4] = 0;
        }
        return;
    }
    const uint32_t data = ordered[0x50 / 4];
    if (data != 0 && remaining <= 0) {
        void **const vtable = *reinterpret_cast<void ***>(surface);
        const func_surface_unlock unlock = reinterpret_cast<func_surface_unlock>(
            vtable[BufferSurfaceUnlockSlot / sizeof(void *)]);
        unlock(surface, reinterpret_cast<void *>(data));
        ordered[0x50 / 4] = 0;
        ordered[0x6C / 4] = 0;
    }
}

int __fastcall buffer_get_data_redirect(Buffer *self, void *) {
    return self->get_data();
}

void __fastcall buffer_free_data_redirect(Buffer *self, void *, int count) {
    self->free_data(count);
}

/*
Purpose: Report the line height of the buffer's primary font, falling back to
         the global default font when none is set.
ORIGINAL: 0x005DCAB0
// name      ?text_line_height@Buffer@@QAEHXZ
// size      43 bytes
// spans     0x005DCAB0-0x005DCADB
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
        font1_ = *FontDefaultPtr;
    }
    const Font *const font = font1_;
    const int *const fields = reinterpret_cast<const int *>(font);
    const int height_override = fields[0x00 / 4];
    if (height_override < 0) {
        // No override: the font's own line height already includes leading.
        return fields[0x0C / 4];
    }
    return static_cast<int>(
        static_cast<uint32_t>(fields[0x10 / 4])
        + static_cast<uint32_t>(height_override));
}

int __fastcall buffer_text_line_height_redirect(Buffer *self, void *) {
    return self->text_line_height();
}

IDirectDraw *BufferDirectDraw;  // 0x009BC494
Font *BufferDefaultFont;        // 0x009BB484
uint32_t *BufferResetValue520 = (uint32_t *)0x00696BF0;
func_sprite_free *BufferFree = (func_sprite_free *)0x00644EF2;

namespace {

typedef long(__stdcall *func_surface_unlock_slot)(void *, void *);
typedef long(__stdcall *func_surface_get_dc_slot)(void *, void *);
typedef long(__stdcall *func_clipper_set_list_slot)(void *, void *, unsigned long);
typedef long(__stdcall *func_surface_set_clipper_slot)(void *, void *);
typedef long(__stdcall *func_surface_release_dc_slot)(void *, void *);
typedef unsigned long(__stdcall *func_com_release)(void *);
typedef void (OriginalObject::*func_buffer_virtual)();

static const size_t OwnedAllocationBase = 0x4BC;
static const size_t OwnedAllocationCount = 20;
static const size_t SurfaceReleaseSlot = 0x08;
static const size_t BufferVirtualSlot = 0x04;
static const size_t SurfaceGetDCSlot = 0x44;
static const size_t ClipperSetClipListSlot = 0x1C;
static const size_t SurfaceSetClipperSlot = 0x70;
static const size_t SurfaceReleaseDCSlot = 0x68;

void *slot(void *object, size_t offset) {
    void **const vtable = *reinterpret_cast<void ***>(object);
    return vtable[offset / sizeof(void *)];
}

}  // namespace

func_buffer_copy_full BufferCopyFull = original_method<func_buffer_copy_full>(0x005DFF00);

/*
Purpose: Copy a region of another buffer into the same position in this one.
ORIGINAL: 0x005D95B0
// name      ?copy@Buffer@@QAEHPAVBuffer@@HHHH@Z
// size      35 bytes
// spans     0x005D95B0-0x005D95D3
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
int Buffer::copy(Buffer *buffer, int xCoord, int yCoord, int width,
                 int height) {
    return (ORIGINAL(this)->*BufferCopyFull)(buffer, xCoord, yCoord, xCoord, yCoord, width, height);
}

int __fastcall buffer_copy_redirect(Buffer *self, void *, Buffer *buffer,
                                    int xCoord, int yCoord, int width,
                                    int height) {
    return self->copy(buffer, xCoord, yCoord, width, height);
}

/*
Purpose: Copy the region a rectangle describes out of another buffer into the
         same position in this one.
ORIGINAL: 0x005D95E0
// name      ?copy@Buffer@@QAEHPAVBuffer@@PAURECT@@@Z
// size      42 bytes
// spans     0x005D95E0-0x005D960A
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
    const int left = rect->left;
    const int top = rect->top;
    return (ORIGINAL(this)->*BufferCopyFull)(buffer, left, top, left, top, rect->right - left, rect->bottom - top);
}

int __fastcall buffer_copy_rect_redirect(Buffer *self, void *, Buffer *buffer,
                                         RECT *rect) {
    return self->copy(buffer, rect);
}

// Surface setup and image load reached by the promoted window init_class
// bodies. Their own bodies are not yet recovered, so each forwards into the
// original image through the seam. The seven-argument copy reuses
// BufferCopyFull, already bound above to 0x005DFF00.
typedef int (OriginalObject::*func_buffer_init)(int, int, int, ExtDirectDraw *);
typedef int (OriginalObject::*func_buffer_fill)(int);
typedef int (OriginalObject::*func_buffer_load_pcx)(const char *, Palette *, int, int);
static func_buffer_init BufferInitOriginal = original_method<func_buffer_init>(0x005D7670);
static func_buffer_fill BufferFillOriginal = original_method<func_buffer_fill>(0x005DFB50);
static func_buffer_load_pcx BufferLoadPcxOriginal = original_method<func_buffer_load_pcx>(0x005D7DE0);

/*
Purpose: Give the buffer a size and the storage behind it - a DirectDraw
         surface when the process has DirectDraw, a DIB section otherwise -
         and leave a device context selected onto it.
ORIGINAL: 0x005D7670
// name      ?init@Buffer@@QAEHHHHPAUExtDirectDraw@@@Z
// size      847 bytes
// spans     0x005D7670-0x005D79BF
// prototype int (__thiscall ?init@Buffer@@QAEHHHHPAUExtDirectDraw@@@Z)(Buffer* this, int, int, int, ExtDirectDraw*)
// callers   40   call targets   5
// kind      game
// flags     sp_ready;purged_ok
// calls     0x005D7470 0x005D8000 0x005FA8A0 0x00644EF2 0x006465F0
//
// `height_` HOLDS THE NEGATIVE, which is what makes the early-out read
// oddly: `width == width_ && height == -height_` is "already this size".
// Every caller that reads it back negates it again - `Win::init_class`
// passes `-logo.height_` to `copy` - so the sign lives in the field.
//
// The four returns are 3 (bad argument), 1 (no device context), 0x12 (the
// surface or its clipper could not be created) and 0 (done, and also the
// early-out).
Status: WIP
*/
// `/O2` implies `/Oi`, which expands the `memset` below to `rep stosd`; the
// image has `call _memset` at 0x005D77C3. `stdafx.h` already pins the four
// string routines this way and this is the same lever, kept local because
// only this body needs it.
#pragma function(memset)

int Buffer::init(int width, int height, int tgl, ExtDirectDraw *direct_draw) {
    // Buffer's own virtual slot 1, called on `this` whenever the surface or
    // its device context could not be had. The BASE implementation is
    // `sub_406b30` - `xor eax, eax; ret` - so what it means is whatever the
    // derived windows do with it; nothing here establishes more.
    static const size_t BufferSlotSurfaceLost = 4;
    typedef void (OriginalObject::*func_buffer_surface_lost)();

    const int borrowed = tgl & 4;
    if (borrowed != 0 && direct_draw == nullptr) {
        return 3;
    }
    if (width < 0 || height < 0) {
        return 3;
    }
    if (width == static_cast<int>(width_)
        && height == -static_cast<int>(height_)) {
        return 0;
    }

    close();
    spot_.init(0x28);
    field_4AC_ = 0;
    for (int slot = 0; slot < 20; ++slot) {
        if (cached_[slot] != nullptr) {
            free(cached_[slot]);
            cached_[slot] = nullptr;
        }
    }

    if (BufferDirectDraw == nullptr && borrowed == 0) {
        hdc_ = CreateCompatibleDC(nullptr);
        if (hdc_ == nullptr) {
            return 1;
        }
    }

    width_ = width;
    height_ = -height;

    if (BufferDirectDraw != nullptr) {
        if (borrowed == 0) {
            DDSURFACEDESC description;
            memset(&description, 0, sizeof(description));
            description.dwSize = sizeof(description);
            description.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH;
            description.dwHeight = height;
            description.dwWidth = width;
            description.ddsCaps.dwCaps =
                DDSCAPS_OFFSCREENPLAIN | DDSCAPS_SYSTEMMEMORY | DDSCAPS_VIDEOMEMORY;
            IDirectDrawSurface *created = nullptr;
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
    } else if (borrowed == 0) {
        bitmap_handle_ = CreateDIBSection(hdc_, bitmap_info_, 0, ppv_bits_,
                                          nullptr, 0);
        if (bitmap_handle_ == nullptr) {
            MessageBoxA(nullptr,
                        "Unable to allocate draw-buffer; terminating program",
                        "FATAL ERROR", MB_ICONEXCLAMATION);
            exit(4);
        }
    }
    if (borrowed != 0) {
        surface_ = direct_draw->surface;
        clipper_ = direct_draw->clipper;
    }

    rect2_.left = 0;
    rect2_.top = 0;
    rect2_.right = width;
    rect2_.bottom = height;
    set_clip(&rect2_);

    width_ = width;
    height_ = -height;
    field_90_ = width * height;
    field_9C_ = 0x100;

    if (BufferDirectDraw == nullptr) {
        field_74_ = reinterpret_cast<uint32_t>(
            SelectObject(hdc_, bitmap_handle_));
    }
    field_4A8_ = (width + 3) & ~3;

    if (BufferDefaultFont != nullptr) {
        if (BufferDefaultFont->is_initialized()) {
            font1_ = BufferDefaultFont;
        }
        font2_ = nullptr;
        font3_ = nullptr;
        font4_ = nullptr;
    }

    if (field_50_ != 0) {
        (ORIGINAL(this)->*original_slot<func_buffer_surface_lost>(
            *reinterpret_cast<uint8_t **>(this) + BufferSlotSurfaceLost))();
    }

    if (surface_ == nullptr) {
        hdc2_ = hdc_;
    } else if (hdc2_ == nullptr) {
        if (surface_->GetDC(&hdc2_) != 0) {
            (ORIGINAL(this)->*original_slot<func_buffer_surface_lost>(
                *reinterpret_cast<uint8_t **>(this) + BufferSlotSurfaceLost))();
        }
    }

    ++hdc_lock_count_;
    SetBkMode(hdc2_, TRANSPARENT);
    SetTextAlign(hdc2_, TA_LEFT | TA_TOP | TA_NOUPDATECP);

    if (surface_ == nullptr) {
        if (--hdc_lock_count_ <= 0) {
            hdc2_ = nullptr;
            hdc_lock_count_ = 0;
        }
    } else {
        const int remaining = --hdc_lock_count_;
        if (hdc2_ != nullptr && remaining <= 0) {
            if (surface_->ReleaseDC(hdc2_) != 0) {
                (ORIGINAL(this)->*original_slot<func_buffer_surface_lost>(
                    *reinterpret_cast<uint8_t **>(this) + BufferSlotSurfaceLost))();
            }
            hdc_lock_count_ = 0;
            hdc2_ = nullptr;
        }
    }

    field_1C_ = tgl;
    return 0;
}

int Buffer::fill(int color) {
    return (ORIGINAL(this)->*BufferFillOriginal)(color);
}

int Buffer::load_pcx(const char *filename, Palette *palette, int tgl, int height) {
    return (ORIGINAL(this)->*BufferLoadPcxOriginal)(filename, palette, tgl, height);
}

int Buffer::copy(Buffer *buffer, int xCoord, int yCoord, int width, int height,
                 int src_width, int src_height) {
    return (ORIGINAL(this)->*BufferCopyFull)(buffer, xCoord, yCoord, width,
                                             height, src_width, src_height);
}

/*
Purpose: Release every resource the buffer owns and reset it to its
         constructed state.
ORIGINAL: 0x005D7470
// name      ?close@Buffer@@QAEXXZ
// size      501 bytes
// spans     0x005D7470-0x005D7665
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
    volatile uint32_t *ordered = reinterpret_cast<volatile uint32_t *>(this);

    // Twenty owned allocations released through the executable's allocator.
    for (size_t index = 0; index < OwnedAllocationCount; ++index) {
        volatile uint32_t *const entry =
            ordered + (OwnedAllocationBase / 4) + index;
        if (*entry != 0) {
            BufferFree(reinterpret_cast<void *>(*entry));
            *entry = 0;
        }
    }

    if (ordered[0x64 / 4] != 0) {
        ordered[0x68 / 4] = 0;
        ordered[0x6C / 4] = 0;
        if (BufferDirectDraw != 0) {
            void *const surface = reinterpret_cast<void *>(ordered[0x58 / 4]);
            // The reference count was just zeroed, so this decrement always
            // lands at or below zero and the published data is dropped.
            const int32_t remaining =
                static_cast<int32_t>(ordered[0x68 / 4]) - 1;
            ordered[0x68 / 4] = static_cast<uint32_t>(remaining);
            if (!surface) {
                if (remaining <= 0) {
                    ordered[0x60 / 4] = 0;
                    ordered[0x68 / 4] = 0;
                }
            } else {
                const uint32_t data = ordered[0x60 / 4];
                if (data != 0 && remaining <= 0) {
                    const long result =
                        reinterpret_cast<func_surface_unlock_slot>(
                            slot(surface, 0x68))(
                                surface, reinterpret_cast<void *>(data));
                    if (result != 0) {
                        (ORIGINAL(this)->*original_method<func_buffer_virtual>(reinterpret_cast<unsigned long>(slot(this, BufferVirtualSlot))))();
                    }
                    ordered[0x68 / 4] = 0;
                    ordered[0x60 / 4] = 0;
                }
            }
        } else if ((ordered[0x1C / 4] & 4U) == 0) {
            HDC device = reinterpret_cast<HDC>(ordered[0x64 / 4]);
            if (ordered[0x74 / 4] != 0) {
                SelectObject(device, reinterpret_cast<HGDIOBJ>(ordered[0x74 / 4]));
                ordered[0x74 / 4] = 0;
            }
            DeleteDC(device);
            ordered[0x60 / 4] = 0;
            ordered[0x64 / 4] = 0;
        }
    }

    size_t offset_cases[] = {size_t(0x78), size_t(0x70)};
    for (size_t offset_index = 0;
         offset_index < sizeof(offset_cases) / sizeof(offset_cases[0]);
         ++offset_index) {
        size_t offset = offset_cases[offset_index];
        if (ordered[offset / 4] != 0) {
            DeleteObject(reinterpret_cast<HGDIOBJ>(ordered[offset / 4]));
            ordered[offset / 4] = 0;
        }
    }

    if (BufferDirectDraw != 0) {
        size_t release_offset_cases[] = {size_t(0x58), size_t(0x5C)};
        for (size_t release_offset_index = 0;
             release_offset_index
                 < sizeof(release_offset_cases) / sizeof(release_offset_cases[0]);
             ++release_offset_index) {
            size_t offset = release_offset_cases[release_offset_index];
            void *const object = reinterpret_cast<void *>(ordered[offset / 4]);
            if (object) {
                reinterpret_cast<func_com_release>(
                    slot(object, SurfaceReleaseSlot))(object);
            }
        }
    }

    ordered[0x58 / 4] = 0;
    ordered[0x5C / 4] = 0;
    ordered[0x0C / 4] = 0;
    ordered[0x08 / 4] = 0;
    ordered[0x18 / 4] = 0;
    ordered[0x14 / 4] = 0;
    ordered[0x10 / 4] = 0;
    ordered[0x4A4 / 4] = 0;
    ordered[0x4A8 / 4] = 0;
    ordered[0x57C / 4] = 0;
    *reinterpret_cast<volatile uint8_t *>(
        reinterpret_cast<uint8_t *>(this) + 0x580) = 0;
    ordered[0x80 / 4] = 0;
    ordered[0x84 / 4] = 0;
    ordered[0x50 / 4] = 0;
    ordered[0x54 / 4] = 0;
    ordered[0x50C / 4] = 0xFFFFFFFFU;
    ordered[0x510 / 4] = 0;
    ordered[0x514 / 4] = 0;
    ordered[0x518 / 4] = 0;
    ordered[0x51C / 4] = 0;
    ordered[0x520 / 4] = *BufferResetValue520;
    ordered[0x52C / 4] = reinterpret_cast<uint32_t>(*FontDefaultPtr);
    // Four text slots at 0x530..0x53C with five rows 0x10 apart. The layout
    // is deliberately not uniform: the fourth slot holds 2 where the others
    // hold -1 at row two, and 0 where the others hold 2 at row four, so the
    // stores are written out rather than generated.
    ordered[0x53C / 4] = 0;
    ordered[0x54C / 4] = 0xFFFFFFFFU;
    ordered[0x55C / 4] = 2;
    ordered[0x56C / 4] = 2;
    ordered[0x530 / 4] = 0;
    ordered[0x540 / 4] = 0xFFFFFFFFU;
    ordered[0x550 / 4] = 0xFFFFFFFFU;
    ordered[0x560 / 4] = 2;
    ordered[0x570 / 4] = 2;
    ordered[0x534 / 4] = 0;
    ordered[0x544 / 4] = 0xFFFFFFFFU;
    ordered[0x554 / 4] = 0xFFFFFFFFU;
    ordered[0x564 / 4] = 2;
    ordered[0x574 / 4] = 2;
    ordered[0x538 / 4] = 0;
    ordered[0x548 / 4] = 0xFFFFFFFFU;
    ordered[0x558 / 4] = 0xFFFFFFFFU;
    ordered[0x568 / 4] = 2;
    ordered[0x578 / 4] = 2;
    ordered[0x584 / 4] = 0;
    ordered[0x1C / 4] = 0;
}

void __fastcall buffer_close_redirect(Buffer *self, void *) {
    self->close();
}

const uint32_t BufferVtable = 0x0066FDBC;

/*
Purpose: Destroy a Buffer by installing its virtual table, releasing every
         owned resource, and destroying the trailing Spot subobject.
ORIGINAL: 0x005D7410
// name      ??1Buffer@@QAE@XZ
// size      107 bytes
// spans     0x005D7410-0x005D7463;0x00662B68-0x00662B80
// prototype void (__thiscall ??1Buffer@@QAE@XZ)(Buffer* this)
// callers   87   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005D7470 0x005FA870
// notes     Runtime redirect installed by DllMain after byte-signature validation
Status: Complete
*/
void Buffer::destroy() {
    volatile uint32_t *ordered = reinterpret_cast<volatile uint32_t *>(this);
    ordered[0] = BufferVtable;
    close();
    // The Spot subobject sits immediately before the owned allocation table.
    Spot *const spot = reinterpret_cast<Spot *>(
        reinterpret_cast<uint8_t *>(this) + 0x4B0);
    spot->~Spot();
}

void __fastcall buffer_destructor_redirect(Buffer *self, void *) {
    self->destroy();
}

/*
Purpose: Acquire the device context, taking one reference on the shared handle.
ORIGINAL: 0x005E3503
// name      ?get_hdc@Buffer@@QAEHXZ
// size      89 bytes
// spans     0x005E3503-0x005E355C
// prototype int (__thiscall ?get_hdc@Buffer@@QAEHXZ)(Buffer* this)
// callers   9   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x005E3510 0x005E3542 0x005E354D
Return Value: The device context, or zero when the surface refuses one
Status: Complete
*/
HDC Buffer::get_hdc() {
    if (field_50_ != 0) {
        (ORIGINAL(this)->*original_method<func_buffer_virtual>(reinterpret_cast<unsigned long>(slot(this, BufferVirtualSlot))))();
    }
    void *const surface = reinterpret_cast<void *>(surface_);
    // Without a surface the buffer owns its context directly, so acquiring is
    // just publishing the stored handle and counting the reference.
    if (!surface) {
        hdc2_ = hdc_;
        ++hdc_lock_count_;
        return hdc_;
    }
    if (hdc2_ != nullptr) {
        ++hdc_lock_count_;
        return hdc2_;
    }
    const long result = reinterpret_cast<func_surface_get_dc_slot>(
        slot(surface, SurfaceGetDCSlot))(surface, &hdc2_);
    if (result != 0) {
        (ORIGINAL(this)->*original_method<func_buffer_virtual>(reinterpret_cast<unsigned long>(slot(this, BufferVirtualSlot))))();
    }
    ++hdc_lock_count_;
    return hdc2_;
}

/*
Purpose: Drop the given number of device-context references, releasing the
         handle once the last one is gone.
ORIGINAL: 0x005E3563
// name      ?release_hdc@Buffer@@QAEXH@Z
// size      96 bytes
// spans     0x005E3563-0x005E35C3
// prototype void (__thiscall ?release_hdc@Buffer@@QAEXH@Z)(Buffer* this, int)
// callers   8   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x005E35AA 0x005E35B5
Status: Complete
*/
void Buffer::release_hdc(int count) {
    void *const surface = reinterpret_cast<void *>(surface_);
    // The count is subtracted rather than decremented, so a caller may return
    // several references at once; the handle drops only at or below zero.
    const int remaining = static_cast<int>(hdc_lock_count_) - count;
    hdc_lock_count_ = static_cast<uint32_t>(remaining);
    if (!surface) {
        if (remaining < 1) {
            hdc2_ = nullptr;
            hdc_lock_count_ = 0;
        }
        return;
    }
    if (hdc2_ == nullptr || remaining >= 1) {
        return;
    }
    const long result = reinterpret_cast<func_surface_release_dc_slot>(
        slot(surface, SurfaceReleaseDCSlot))(surface, hdc2_);
    if (result != 0) {
        (ORIGINAL(this)->*original_method<func_buffer_virtual>(reinterpret_cast<unsigned long>(slot(this, BufferVirtualSlot))))();
    }
    hdc_lock_count_ = 0;
    hdc2_ = nullptr;
}

HDC __fastcall buffer_get_hdc_redirect(Buffer *self, void *) {
    return self->get_hdc();
}

void __fastcall buffer_release_hdc_redirect(Buffer *self, void *, int count) {
    self->release_hdc(count);
}

/*
Purpose: Republish a palette into the buffer's colour table and device context,
         skipping the work when the palette has not changed.
ORIGINAL: 0x005DE8F0
// name      ?sync_to_palette@Buffer@@QAEHPAVPalette@@@Z
// size      290 bytes
// spans     0x005DE8F0-0x005DEA12
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
    if (!ppv_bits_) {
        return 7;
    }
    if (!palette) {
        return 3;
    }
    // field_4A4_ caches the palette generation tag and sits immediately after
    // the 256-entry table it guards, so an unchanged palette costs nothing.
    if (field_4A4_ != palette->seed_) {
        field_4A4_ = palette->seed_;
        RGBQUAD *const table = reinterpret_cast<RGBQUAD *>(dib_);
        palette->get_rgbquad(table, 0, 0x100);
        const HDC device = get_hdc();
        if (device != nullptr) {
            SetDIBColorTable(device, 0, 0x100, table);
            release_hdc(1);
        }
    }
    field_57C_ = 1;
    field_584_ = reinterpret_cast<uint32_t>(palette);
    return 0;
}

int __fastcall buffer_sync_to_palette_redirect(
        Buffer *self, void *, Palette *palette) {
    return self->sync_to_palette(palette);
}

/*
Purpose: Report the height of the buffer's text font, resolving the process
         default the first time it is needed.
ORIGINAL: 0x005DCA80
// name      ?text_height@Buffer@@QAEHXZ
// size      37 bytes
// spans     0x005DCA80-0x005DCAA5
// prototype int (__thiscall ?text_height@Buffer@@QAEHXZ)(Buffer* this)
// callers   5   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: The font's height
Status: Complete
*/
int Buffer::text_height() {
    // The legacy body loads the default before testing the cached font, so the
    // fallback it returns is the value it just stored.
    Font *const fallback = *FontDefaultPtr;
    if (!font1_) {
        font1_ = fallback;
        return fallback->height_;
    }
    return font1_->height_;
}

int __fastcall buffer_text_height_redirect(Buffer *self, void *) {
    return self->text_height();
}

/*
Purpose: Clip the buffer to a rectangle, updating the GDI clip region and the
         DirectDraw clipper to match.
ORIGINAL: 0x005D8000
// name      ?set_clip@Buffer@@QAEHPAURECT@@@Z
// size      499 bytes
// spans     0x005D8000-0x005D81F3
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
    if (!ppv_bits_ && surface_ == 0) {
        return 7;
    }
    if (!rect) {
        return 3;
    }
    RECT requested = *rect;
    if (!IntersectRect(&rect1_, &rect2_, &requested)) {
        return 1;
    }

    // The context is acquired only when none is already held, and released
    // again only in that case.
    bool acquired = false;
    if (hdc2_ == nullptr) {
        acquired = true;
        get_hdc();
    }
    if (hdc2_ != nullptr) {
        if (field_70_ != nullptr) {
            DeleteObject(field_70_);
            field_70_ = nullptr;
        }
        HRGN region = nullptr;
        // A clip equal to the full extent needs no region at all; passing null
        // to SelectClipRgn restores the unclipped state.
        if (!EqualRect(&rect1_, &rect2_)) {
            region = CreateRectRgnIndirect(&rect1_);
            field_70_ = region;
            if (region == nullptr) {
                // The legacy body returns here without releasing the context
                // it may have just acquired; preserved deliberately.
                return 1;
            }
        }
        SelectClipRgn(hdc2_, region);
    }
    if (acquired) {
        release_hdc(1);
    }

    if (surface_ != 0) {
        // A single-rectangle RGNDATA: the header's bound and the one entry in
        // the rectangle array are both the clipped rectangle.
        struct ClipRegionData {
            RGNDATAHEADER header;
            RECT rects[1];
        } region_data;
        region_data.header.dwSize = sizeof(RGNDATAHEADER);
        region_data.header.iType = RDH_RECTANGLES;
        region_data.header.nCount = 1;
        region_data.header.nRgnSize = sizeof(RECT);
        region_data.header.rcBound = rect1_;
        region_data.rects[0] = rect1_;

        void *const clipper = reinterpret_cast<void *>(clipper_);
        reinterpret_cast<func_clipper_set_list_slot>(
            slot(clipper, ClipperSetClipListSlot))(clipper, &region_data, 0);
        void *const surface = reinterpret_cast<void *>(surface_);
        reinterpret_cast<func_surface_set_clipper_slot>(
            slot(surface, SurfaceSetClipperSlot))(surface, clipper);
    }
    return 0;
}

int __fastcall buffer_set_clip_redirect(Buffer *self, void *, RECT *rect) {
    return self->set_clip(rect);
}

func_buffer_text_width_measured BufferTextWidthMeasured =
    original_method<func_buffer_text_width_measured>(0x005DC7C0);

/*
Purpose: Measure a null-terminated string with the buffer's text font.
ORIGINAL: 0x005DC790
// name      ?text_width@Buffer@@QAEHPAD@Z
// size      42 bytes
// spans     0x005DC790-0x005DC7BA
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
    return (ORIGINAL(this)->*BufferTextWidthMeasured)(text, strlen(text));
}

int __fastcall buffer_text_width_redirect(Buffer *self, void *, LPSTR text) {
    return self->text_width(text);
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

func_buffer_line BufferHLine = original_method<func_buffer_line>(0x005E1A80);
func_buffer_line BufferVLine = original_method<func_buffer_line>(0x005E1BF0);

/*
Purpose: Outline a rectangle as a two-color bevel: the top and left edges in
         the first color, the bottom and right edges in the second, with the
         corner ownership split the way the emboss look requires - the top
         edge spans [left+1, right-1], the bottom [left, right-2] one row up
         from the bottom, the left column [top, bottom-2], and the right
         column [top+1, bottom-1] one column in from the right.
ORIGINAL: 0x005E3203
// name      ?box@Buffer@@QAEHPAURECT@@HH@Z
// size      125 bytes
// spans     0x005E3203-0x005E3280
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
    const uint32_t left = edge_bits(rect->left);
    const uint32_t top = edge_bits(rect->top);
    const uint32_t right_in = edge_bits(rect->right) - 1U;
    const uint32_t bottom_in = edge_bits(rect->bottom) - 1U;
    (ORIGINAL(this)->*BufferHLine)(edge_int(left + 1U), edge_int(right_in), edge_int(top), color1);
    (ORIGINAL(this)->*BufferHLine)(edge_int(left), edge_int(right_in - 1U), edge_int(bottom_in), color2);
    (ORIGINAL(this)->*BufferVLine)(edge_int(left), edge_int(top), edge_int(bottom_in - 1U), color1);
    (ORIGINAL(this)->*BufferVLine)(edge_int(right_in), edge_int(top + 1U), edge_int(bottom_in), color2);
    return 0;
}

int __fastcall buffer_box_redirect(Buffer *self, void *, RECT *rect,
                                   int color1, int color2) {
    return self->box(rect, color1, color2);
}

/*
Purpose: Reset the buffer's link table - reinitialise the spot list to 40
         entries, clear the count, and free the twenty owned link pointers,
         each through the executable's CRT boundary.
ORIGINAL: 0x005DEF90
// name      ?clear_links@Buffer@@QAEXXZ
// size      68 bytes
// spans     0x005DEF90-0x005DEFD4
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
    field_4AC_ = 0;
    // `lea edi, [esi + 0x4bc]` at 0x005D747B - the array itself, not a
    // pointer stored there, which is what the cast used to assume.
    void **const links = cached_;
    for (size_t index = 0; index < 20; ++index) {
        if (links[index]) {
            BufferFree(links[index]);
            links[index] = nullptr;
        }
    }
}

void __fastcall buffer_clear_links_redirect(Buffer *self, void *) {
    self->clear_links();
}

func_buffer_write_multi_font_raw_l BufferWriteMultiFontRawL =
    original_method<func_buffer_write_multi_font_raw_l>(0x005DCAE0);

/*
Purpose: Draw at most `len` characters of a string at an explicit pen
         position, clamping the count to the string's own length first.
ORIGINAL: 0x005DCEA0
// name      ?write_l@Buffer@@QAEHPADHHH@Z
// size      148 bytes
// spans     0x005DCEA0-0x005DCF34
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
it calls strlen up to four times for the same reason. strlen is pure and `len`
is untouched between them, so the single evaluation here is the same value in
every role. The comparison is signed (`cmp eax, esi` / `jge`), so a negative
`len` wins the min and takes the early exit.

Verification note: `(measured < len)` and `(measured <= len)` select the same
value when the two are equal, so a comparison mutant that only relaxes the
strictness of this min is equivalent by construction and will survive.
*/
int Buffer::write_l(LPSTR text, int x_coord, int y_coord, int len) {
    if (!text) {
        return x_coord;
    }
    if (!font1_ || !font1_->is_initialized()) {
        return 3;
    }
    const int measured = static_cast<int>(strlen(text));
    const int limit = (measured < len) ? measured : len;
    if (limit <= 0) {
        return x_coord;
    }
    return (ORIGINAL(this)->*BufferWriteMultiFontRawL)(text, x_coord, y_coord, limit);
}

int __fastcall buffer_write_l_redirect(Buffer *self, void *, LPSTR text,
                                       int x_coord, int y_coord, int len) {
    return self->write_l(text, x_coord, y_coord, len);
}

/*
Purpose: Draw at most `len` characters of a string flush against a
         rectangle's left edge and vertically centred on the text font.
ORIGINAL: 0x005DCF40
// name      ?write_l@Buffer@@QAEHPADPAURECT@@H@Z
// size      214 bytes
// spans     0x005DCF40-0x005DD016
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

Verification note: the original re-tests font1_ at 0x005DCFCD and rebinds it
from the process default at 0x009BB484 when null. That rebind cannot execute
here - the guard at 0x005DCF69 already returned zero for a null font1_, and
only pure strlen calls run in between - so it is not transcribed. The
rectangle overload of write_cent_l carries the identical rebind and there it
IS reachable, because the intervening text-width call can clear the field.

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
    const int measured = static_cast<int>(strlen(text));
    const int limit = (measured < len) ? measured : len;
    if (limit <= 0) {
        return 0;
    }
    const uint32_t left = edge_bits(rect->left);
    const uint32_t top = edge_bits(rect->top);
    const uint32_t bottom = edge_bits(rect->bottom);
    const int y_span = edge_int(bottom - edge_bits(font1_->height_) - top);
    const int y_coord = edge_int(top + edge_bits(y_span / 2));
    return (ORIGINAL(this)->*BufferWriteMultiFontRawL)(text, edge_int(left), y_coord, limit);
}

int __fastcall buffer_write_l_rect_redirect(Buffer *self, void *, LPSTR text,
                                            RECT *rect, int len) {
    return self->write_l(text, rect, len);
}

/*
Purpose: Draw at most `len` characters of a string horizontally centred in a
         span of `width` pixels starting at an explicit pen position.
ORIGINAL: 0x005DD020
// name      ?write_cent_l@Buffer@@QAEHPADHHHH@Z
// size      178 bytes
// spans     0x005DD020-0x005DD0D2
// prototype int (__thiscall ?write_cent_l@Buffer@@QAEHPADHHHH@Z)(Buffer* this, int8*, int, int, int, int)
// callers   30   call targets   3
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005DC7C0 0x005DCAE0 0x006453E0
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
    const int measured = static_cast<int>(strlen(text));
    const int limit = (measured < len) ? measured : len;
    if (limit <= 0) {
        return x_coord;
    }
    const int drawn =
        (ORIGINAL(this)->*BufferTextWidthMeasured)(text, static_cast<size_t>(limit));
    const int x_span = edge_int(edge_bits(width) - edge_bits(drawn));
    const int centred = edge_int(edge_bits(x_coord) + edge_bits(x_span / 2));
    return (ORIGINAL(this)->*BufferWriteMultiFontRawL)(text, centred, y_coord, limit);
}

int __fastcall buffer_write_cent_l_redirect(Buffer *self, void *, LPSTR text,
                                            int x_coord, int y_coord,
                                            int width, int len) {
    return self->write_cent_l(text, x_coord, y_coord, width, len);
}

/*
Purpose: Draw at most `len` characters of a string centred both horizontally
         and vertically inside a rectangle.
ORIGINAL: 0x005DD130
// name      ?write_cent_l@Buffer@@QAEHPADPAURECT@@H@Z
// size      282 bytes
// spans     0x005DD130-0x005DD24A
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
    if (!text) {
        return 0;
    }
    if (!rect) {
        return 0;
    }
    const uint32_t left = edge_bits(rect->left);
    const uint32_t top = edge_bits(rect->top);
    const uint32_t right = edge_bits(rect->right);
    const uint32_t bottom = edge_bits(rect->bottom);
    if (!font1_ || !font1_->is_initialized()) {
        return 3;
    }
    const int measured = static_cast<int>(strlen(text));
    const int limit = (measured < len) ? measured : len;
    if (limit <= 0) {
        return 0;
    }
    const int drawn =
        (ORIGINAL(this)->*BufferTextWidthMeasured)(text, static_cast<size_t>(measured));
    const int x_span = edge_int(right - edge_bits(drawn) - left);
    const int x_coord = edge_int(left + edge_bits(x_span / 2));
    if (!font1_) {
        font1_ = *FontDefaultPtr;
    }
    const int y_span = edge_int(bottom - edge_bits(font1_->height_) - top);
    const int y_coord = edge_int(top + edge_bits(y_span / 2));
    return (ORIGINAL(this)->*BufferWriteMultiFontRawL)(text, x_coord, y_coord, limit);
}

int __fastcall buffer_write_cent_l_rect_redirect(Buffer *self, void *,
                                                 LPSTR text, RECT *rect,
                                                 int len) {
    return self->write_cent_l(text, rect, len);
}

/*
ORIGINAL: 0x005DA680 BYTE_EXACT
// name      ?poly@Buffer@@QAEHPAUVert@@HH@Z
// size      27 bytes
// spans     0x005DA680-0x005DA69B
// prototype 
// callers   1   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00626620
Status: Complete
*/
int Buffer::poly(Vert *a1, int a2, int a3) {
    return polygon(this, a1, a2, a3);
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
