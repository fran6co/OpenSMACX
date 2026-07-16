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
#include "scroll.h"

namespace {

LONG long_from_bits(uint32_t bits) {
    LONG value;
    static_assert(sizeof(value) == sizeof(bits), "Scroll geometry requires 32-bit LONG");
    memcpy(&value, &bits, sizeof(value));
    return value;
}

uint32_t read_bits(const void *object, size_t offset) {
    uint32_t value;
    memcpy(&value, static_cast<const uint8_t *>(object) + offset, sizeof(value));
    return value;
}

void write_bits(void *object, size_t offset, uint32_t value) {
    memcpy(static_cast<uint8_t *>(object) + offset, &value, sizeof(value));
}

void write_sprite(void *object, size_t offset, Sprite *sprite) {
    static_assert(sizeof(uintptr_t) == sizeof(uint32_t),
                  "Scroll sprite pointers require the 32-bit legacy ABI");
    const uintptr_t bits = reinterpret_cast<uintptr_t>(sprite);
    *reinterpret_cast<volatile uintptr_t *>(
        static_cast<uint8_t *>(object) + offset) = bits;
}

uint32_t read_volatile_bits(const void *object, size_t offset) {
    return *reinterpret_cast<const volatile uint32_t *>(
        static_cast<const uint8_t *>(object) + offset);
}

void set_sprite_triplet(void *object, size_t primary_offset,
                        size_t button_offset, Sprite *sprite1,
                        Sprite *sprite2, Sprite *sprite3) {
    write_sprite(object, primary_offset, sprite1);
    write_sprite(object, primary_offset + 4, sprite2);
    write_sprite(object, primary_offset + 8, sprite3);

    const uint32_t height = 0U - read_volatile_bits(object, 0x4C8);
    const uint32_t width = read_volatile_bits(object, 0x4C4);
    if (long_from_bits(width) > long_from_bits(height)) {
        write_sprite(object, button_offset, sprite1);
        write_sprite(object, button_offset + 4, sprite2);
        write_sprite(object, button_offset + 8, sprite3);
    }
}

uint32_t arithmetic_shift_right_one(uint32_t value) {
    return (value >> 1U) | (value & 0x80000000U);
}

uint32_t signed_min(uint32_t left, uint32_t right) {
    return long_from_bits(left) < long_from_bits(right) ? left : right;
}

#ifdef _MSC_VER
__declspec(noinline)
#else
__attribute__((noinline))
#endif
uint32_t signed_divide(uint32_t dividend_bits, uint32_t divisor_bits) {
    LONG dividend = long_from_bits(dividend_bits);
    LONG divisor = long_from_bits(divisor_bits);
    LONG quotient;
#if defined(_MSC_VER) && defined(_M_IX86)
    __asm {
        mov eax, dividend
        cdq
        idiv divisor
        mov quotient, eax
    }
#elif defined(__GNUC__) && defined(__i386__)
    LONG remainder;
    __asm__ volatile (
        "cltd\n\t"
        "idivl %3"
        : "=&a" (quotient), "=&d" (remainder)
        : "0" (dividend), "rm" (divisor)
        : "cc");
#else
    quotient = dividend / divisor;
#endif
    return static_cast<uint32_t>(quotient);
}

}  // namespace

/*
Purpose: Set the border color and reset the scrollbar thumb rectangle.
Original Offset: 00605B10
Status: Complete
*/
void Scroll::set_border_color(int color) {
    border_color_ = color;
    const bool borderless = color == -1;
    const uint32_t thickness = static_cast<uint32_t>(bar_thickness_);
    const LONG inset = borderless ? 0 : 1;
    const LONG extent = long_from_bits(borderless ? thickness : thickness - 1U);
    thumb_rect_.left = inset;
    thumb_rect_.top = inset;
    thumb_rect_.right = extent;
    thumb_rect_.bottom = extent;
}

/*
Purpose: Set the left scrollbar sprites and horizontal button sprites.
Original Offset: 00605BE0
Status: Complete
*/
void Scroll::set_sprite_left(
        Sprite *sprite1, Sprite *sprite2, Sprite *sprite3) {
    set_sprite_triplet(this, 0xA7C, 0x15BC, sprite1, sprite2, sprite3);
}

/*
Purpose: Set the right scrollbar sprites and horizontal button sprites.
Original Offset: 00605C30
Status: Complete
*/
void Scroll::set_sprite_right(
        Sprite *sprite1, Sprite *sprite2, Sprite *sprite3) {
    set_sprite_triplet(this, 0xA94, 0x2108, sprite1, sprite2, sprite3);
}

/*
Purpose: Compute and publish the scrollbar thumb rectangle.
Original Offset: 00606C50
Status: Complete
*/
void Scroll::compute_thumb_rect(RECT *rect) {
    auto *thumb = reinterpret_cast<uint8_t *>(this) + 0xA4C;
    for (size_t offset = 0; offset < sizeof(RECT); offset += sizeof(uint32_t)) {
        write_bits(rect, offset, read_bits(thumb, offset));
    }

    uint32_t left = read_bits(rect, 0);
    uint32_t right = read_bits(rect, 8) - left;
    const uint32_t top = read_bits(rect, 4);
    write_bits(rect, 8, right);
    uint32_t bottom = read_bits(rect, 12) - top;
    left = 0;
    write_bits(rect, 0, left);
    write_bits(rect, 12, bottom);
    write_bits(rect, 4, 0);
    if (read_bits(this, 0xA1C) != 0xFFFFFFFFU) {
        left = read_bits(rect, 0) + 1U;
        right = read_bits(rect, 8) + 1U;
        const uint32_t inset_top = read_bits(rect, 4) + 1U;
        bottom = read_bits(rect, 12) + 1U;
        write_bits(rect, 0, left);
        write_bits(rect, 8, right);
        write_bits(rect, 4, inset_top);
        write_bits(rect, 12, bottom);
    }

    right = read_bits(rect, 8);
    left = read_bits(rect, 0);
    const uint32_t extent = right - left;
    const uint32_t drag_coordinate = read_bits(this, 0xA3C);
    uint32_t offset;

    if (drag_coordinate == 0xFFFFFFFFU) {
        const bool no_end_buttons = (read_bits(this, 0xA14) & 2U) != 0;
        offset = no_end_buttons ? 0U : extent + 1U;
        uint32_t adjustment = 0xFFFFFFFFU - (no_end_buttons
            ? extent : extent * 3U);
        if (read_bits(this, 0xA1C) != 0xFFFFFFFFU) {
            adjustment -= 2U;
        }
        const uint32_t height = 0U - read_bits(this, 0x4C8);
        const uint32_t width = read_bits(this, 0x4C4);
        const bool horizontal = long_from_bits(width) > long_from_bits(height);
        const uint32_t axis_length = horizontal ? width : height;
        const uint32_t maximum = read_bits(this, 0xA24);
        const uint32_t minimum = read_bits(this, 0xA20);
        if (maximum != minimum) {
            const uint32_t numerator = (read_bits(this, 0xA2C) - minimum)
                * (axis_length + adjustment);
            offset += signed_divide(numerator, maximum - minimum);
        }
        if (horizontal) {
            write_bits(rect, 0, read_bits(rect, 0) + offset);
            write_bits(rect, 8, read_bits(rect, 8) + offset);
        } else {
            write_bits(rect, 4, read_bits(rect, 4) + offset);
            write_bits(rect, 12, read_bits(rect, 12) + offset);
        }
    } else {
        const bool no_end_buttons = (read_bits(this, 0xA14) & 2U) != 0;
        const uint32_t far_edge = left + extent;
        const uint32_t candidate = drag_coordinate
            + arithmetic_shift_right_one(extent);
        write_bits(this, 0xA3C, candidate);
        const uint32_t height = 0U - read_bits(this, 0x4C8);
        const uint32_t width = read_bits(this, 0x4C4);
        const bool horizontal = long_from_bits(width) > long_from_bits(height);
        const uint32_t axis_length = horizontal ? width : height;
        const uint32_t upper = axis_length - (no_end_buttons
            ? far_edge : far_edge * 2U);
        const uint32_t limited = signed_min(candidate, upper);
        if (no_end_buttons) {
            offset = long_from_bits(limited) < 0 ? 0U : limited;
        } else if (long_from_bits(far_edge) > long_from_bits(limited)) {
            offset = far_edge;
        } else {
            offset = limited;
        }
        write_bits(this, 0xA3C, offset);
        if (horizontal) {
            write_bits(rect, 0, read_bits(rect, 0) + offset);
            write_bits(rect, 8, read_bits(rect, 8) + offset);
        } else {
            write_bits(rect, 4, read_bits(rect, 4) + offset);
            write_bits(rect, 12, read_bits(rect, 12) + offset);
        }
        write_bits(this, 0xA3C, 0xFFFFFFFFU);
    }
    for (size_t coordinate = 0; coordinate < sizeof(RECT);
            coordinate += sizeof(uint32_t)) {
        write_bits(thumb, coordinate, read_bits(rect, coordinate));
    }
}

void __fastcall scroll_set_border_color_redirect(
        Scroll *self, void *, int color) {
    self->set_border_color(color);
}

Sprite *__fastcall scroll_set_sprite_left_redirect(
        Scroll *self, void *, Sprite *sprite1, Sprite *sprite2, Sprite *sprite3) {
    self->set_sprite_left(sprite1, sprite2, sprite3);
    return sprite1;
}

Sprite *__fastcall scroll_set_sprite_right_redirect(
        Scroll *self, void *, Sprite *sprite1, Sprite *sprite2, Sprite *sprite3) {
    self->set_sprite_right(sprite1, sprite2, sprite3);
    return sprite1;
}

RECT *__fastcall scroll_compute_thumb_rect_redirect(
        Scroll *self, void *, RECT *rect) {
    self->compute_thumb_rect(rect);
    return reinterpret_cast<RECT *>(
        reinterpret_cast<uint8_t *>(self) + 0xA4C);
}
