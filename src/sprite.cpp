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
