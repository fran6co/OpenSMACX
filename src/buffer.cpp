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
#include "buffer.h"
#include "font.h"

/*
Purpose: Set the four fonts used by the buffer.
Original Offset: 005DAC70
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
Original Offset: 005DACB0
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
Original Offset: 005DACE0
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
Original Offset: 005DAD10
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
Original Offset: 005DAD40
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
Original Offset: 005DF570
Return Value: No errors (0)
Status: Complete
*/
int Buffer::init_class() {
    return 0;
}

/*
Purpose: Close shared Buffer state; the legacy implementation is intentionally empty.
Original Offset: 005DF580
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

#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wattributes"
#endif
typedef long(__stdcall *func_surface_lock)(void *, void *, void *, uint32_t, void *);
typedef long(__stdcall *func_surface_unlock)(void *, void *);
#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif

// DDSURFACEDESC fields the lock path consumes.
constexpr size_t SurfaceDescriptorSize = 0x6C;
constexpr size_t SurfaceDescriptorPitch = 0x10;
constexpr size_t SurfaceDescriptorData = 0x24;

}  // namespace

/*
Purpose: Acquire the buffer's pixel data, locking the DirectDraw surface on the
         first reference and counting every acquisition.
Original Offset: 005E3373
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
Original Offset: 005E34A3
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
Original Offset: 005DCAB0
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

int *BufferDirectDrawActive = (int *)0x009BC494;
uint32_t *BufferResetValue520 = (uint32_t *)0x00696BF0;
func_sprite_free *BufferFree = (func_sprite_free *)0x00644EF2;

namespace {

#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wattributes"
#endif
typedef long(__stdcall *func_surface_unlock_slot)(void *, void *);
typedef unsigned long(__stdcall *func_com_release)(void *);
typedef void(__thiscall *func_buffer_virtual)(void *);
#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif

constexpr size_t OwnedAllocationBase = 0x4BC;
constexpr size_t OwnedAllocationCount = 20;
constexpr size_t SurfaceReleaseSlot = 0x08;
constexpr size_t BufferVirtualSlot = 0x04;

void *slot(void *object, size_t offset) {
    void **const vtable = *reinterpret_cast<void ***>(object);
    return vtable[offset / sizeof(void *)];
}

}  // namespace

/*
Purpose: Release every resource the buffer owns and reset it to its
         constructed state.
Original Offset: 005D7470
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
        if (*BufferDirectDrawActive != 0) {
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
                        reinterpret_cast<func_buffer_virtual>(
                            slot(this, BufferVirtualSlot))(this);
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

    for (size_t offset : {size_t(0x78), size_t(0x70)}) {
        if (ordered[offset / 4] != 0) {
            DeleteObject(reinterpret_cast<HGDIOBJ>(ordered[offset / 4]));
            ordered[offset / 4] = 0;
        }
    }

    if (*BufferDirectDrawActive != 0) {
        for (size_t offset : {size_t(0x58), size_t(0x5C)}) {
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
