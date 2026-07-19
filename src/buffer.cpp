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
