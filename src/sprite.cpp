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
#include "sprite.h"

#include <new>

int *SpriteMemoryUsed = reinterpret_cast<int *>(0x009B6618);
func_sprite_free *SpriteFree = (func_sprite_free *)0x00644EF2;

/*
Purpose: Initialize an empty sprite and charge its own size to the sprite
         memory total.
Original Offset: 005E37E0
Status: Complete
*/
Sprite::Sprite() {
    // The legacy body interleaves the accounting update between the first two
    // field stores and writes the type byte late, so the observable store
    // order is pinned rather than left to the compiler.
    volatile uint32_t *ordered = reinterpret_cast<volatile uint32_t *>(this);
    ordered[0x00 / 4] = 0;
    *SpriteMemoryUsed = static_cast<int>(
        static_cast<uint32_t>(*SpriteMemoryUsed) + sizeof(Sprite));
    ordered[0x04 / 4] = 0;
    ordered[0x0C / 4] = 0;
    ordered[0x10 / 4] = 0;
    ordered[0x14 / 4] = 0;
    ordered[0x18 / 4] = 0;
    ordered[0x1C / 4] = 0;
    ordered[0x20 / 4] = 0;
    ordered[0x24 / 4] = 0;
    *reinterpret_cast<volatile uint8_t *>(
        reinterpret_cast<uint8_t *>(this) + 0x08) = 9;
    ordered[0x28 / 4] = 0;
#if defined(__GNUC__) && defined(__i386__)
    // The legacy constructor leaves the instance pointer in EAX.
    __asm__ __volatile__("" : : "a"(this) : "memory");
#endif
}

Sprite *__fastcall sprite_construct_redirect(Sprite *self, void *) {
    new (self) Sprite();
    return self;
}

/*
Purpose: Release a sprite's allocations, discount its pixel memory, and clear
         every field except the type byte.
Original Offset: 005E3820
Status: Complete
*/
void Sprite::close() {
    volatile uint32_t *ordered = reinterpret_cast<volatile uint32_t *>(this);
    // The pixel buffer is only owned, accounted, and released while the
    // borrowed flag at 0x28 is clear.
    if (ordered[0x28 / 4] == 0 && ordered[0x04 / 4] != 0) {
        *SpriteMemoryUsed = static_cast<int>(
            static_cast<uint32_t>(*SpriteMemoryUsed)
            - ordered[0x14 / 4] * ordered[0x10 / 4]);
        SpriteFree(reinterpret_cast<void *>(ordered[0x04 / 4]));
        ordered[0x04 / 4] = 0;
    }
    if (ordered[0x00 / 4] != 0) {
        SpriteFree(reinterpret_cast<void *>(ordered[0x00 / 4]));
        ordered[0x00 / 4] = 0;
    }
    // The type byte at 0x08 is deliberately preserved.
    ordered[0x0C / 4] = 0;
    ordered[0x10 / 4] = 0;
    ordered[0x14 / 4] = 0;
    ordered[0x18 / 4] = 0;
    ordered[0x1C / 4] = 0;
    ordered[0x20 / 4] = 0;
    ordered[0x24 / 4] = 0;
    ordered[0x28 / 4] = 0;
}

void __fastcall sprite_close_redirect(Sprite *self, void *) {
    self->close();
}

int *SpriteDrawOriginX = reinterpret_cast<int *>(0x00696D18);
int *SpriteDrawOriginY = reinterpret_cast<int *>(0x00696D1C);
func_sprite_draw_original *SpriteDrawOriginal = (func_sprite_draw_original *)0x005E4B9A;

/*
Purpose: Draw the sprite with a temporarily substituted draw origin.
Original Offset: 005E4B4A
Return Value: Whatever the four-argument overload returns
Status: Complete with a temporary dependency on the four-argument overload
*/
int Sprite::draw(Buffer *buffer, int a, int b, int c, int x, int y) {
    // The origin is swapped for the duration of the call and restored in the
    // reverse order the legacy body uses.
    const int saved_x = *SpriteDrawOriginX;
    const int saved_y = *SpriteDrawOriginY;
    *SpriteDrawOriginX = x;
    *SpriteDrawOriginY = y;
    const int result = SpriteDrawOriginal(this, buffer, a, b, c);
    *SpriteDrawOriginY = saved_y;
    *SpriteDrawOriginX = saved_x;
    return result;
}

int __fastcall sprite_draw_redirect(
        Sprite *self, void *, Buffer *buffer, int a, int b, int c, int x, int y) {
    return self->draw(buffer, a, b, c, x, y);
}

/*
Purpose: Legacy stub; the original body returns 0 without reading its
         arguments.
Original Offset: 005E4ADA
Status: Complete
*/
int Sprite::UNK1(int, int, int, int, int, int, int) {
    return 0;
}

int __fastcall sprite_unk1_redirect(
        Sprite *self, void *, int a, int b, int c, int d, int e, int f, int g) {
    return self->UNK1(a, b, c, d, e, f, g);
}

/*
Purpose: Legacy stub; the original body returns 0 without reading its
         arguments.
Original Offset: 005E4AEA
Status: Complete
*/
int Sprite::UNK2(int, int, int, int, int) {
    return 0;
}

int __fastcall sprite_unk2_redirect(
        Sprite *self, void *, int a, int b, int c, int d, int e) {
    return self->UNK2(a, b, c, d, e);
}
