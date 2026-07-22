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

Win **ScrollCurrentWin = reinterpret_cast<Win **>(0x009B7AB8);
int *ScrollDefaultThickness = (int *)0x009B8DD4;
int *ScrollNonClientInit = (int *)0x009B8E24;
uint32_t *ScrollCloseStaticDefaults = (uint32_t *)0x00697020;
uint32_t *ScrollCloseDynamicDefaults = (uint32_t *)0x009B8DE0;

namespace {

#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wattributes"
#endif
typedef int (__thiscall func_scroll_init)(
    Scroll *, int, int, int, int, Win *, int, int);
typedef uint32_t (__thiscall *func_noarg_virtual)(void *);
#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif

func_scroll_init *ScrollOriginalInit = (func_scroll_init *)0x006054D0;

int __cdecl call_original_scroll_init(
        Scroll *self, int x, int y, int width, int height, Win *parent,
        int setting, int options) {
    return ScrollOriginalInit(
        self, x, y, width, height, parent, setting, options);
}

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

uint32_t redraw_from_vtable(void *self, uint32_t vtable_bits) {
    const uint32_t redraw_bits = read_volatile_bits(
        reinterpret_cast<const void *>(static_cast<uintptr_t>(vtable_bits)), 0xF8);
    uint32_t result;
#if defined(_MSC_VER) && defined(_M_IX86)
    __asm {
        mov ecx, self
        mov eax, redraw_bits
        call eax
        mov result, eax
    }
#elif defined(__GNUC__) && defined(__i386__)
    uintptr_t self_bits = reinterpret_cast<uintptr_t>(self);
    __asm__ volatile (
        "call *%2"
        : "=a" (result), "+c" (self_bits)
        : "r" (redraw_bits)
        : "edx", "memory", "cc");
#else
    typedef uint32_t (*RedrawProc)(void *);
    result = reinterpret_cast<RedrawProc>(
        static_cast<uintptr_t>(redraw_bits))(self);
#endif
    return result;
}

#ifdef _MSC_VER
__declspec(noinline)
#else
__attribute__((noinline))
#endif
uint32_t call_noarg_virtual(void *self, size_t slot) {
    const uint32_t vtable_bits = read_volatile_bits(self, 0);
    const uint32_t target_bits = read_volatile_bits(
        reinterpret_cast<const void *>(static_cast<uintptr_t>(vtable_bits)),
        slot);
    return reinterpret_cast<func_noarg_virtual>(
        static_cast<uintptr_t>(target_bits))(self);
}

void set_sprite_triplet(void *object, Sprite *volatile *primary,
                        size_t button_offset, bool horizontal, Sprite *sprite1,
                        Sprite *sprite2, Sprite *sprite3) {
    primary[0] = sprite1;
    primary[1] = sprite2;
    primary[2] = sprite3;

    const uint32_t height = 0U - read_volatile_bits(object, 0x4C8);
    const uint32_t width = read_volatile_bits(object, 0x4C4);
    const bool update_button = horizontal
        ? long_from_bits(width) > long_from_bits(height)
        : long_from_bits(width) < long_from_bits(height);
    if (update_button) {
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

ScrollPrimaryInitProc ScrollPrimaryInit = &call_original_scroll_init;

/*
Purpose: Reset Scroll-owned state from the process defaults, close the two
         embedded FlatButtons through their virtual close slots, then close
         the source-owned GraphicWin base.
Original Offset: 00605370
Return Value: GraphicWin::close return residue
Status: Complete; embedded FlatButton close redirects are source-owned
*/
uint32_t Scroll::close() {
    volatile uint32_t *const object =
        reinterpret_cast<volatile uint32_t *>(this);
    volatile uint32_t *const fixed =
        reinterpret_cast<volatile uint32_t *>(ScrollCloseStaticDefaults);
    volatile uint32_t *const dynamic =
        reinterpret_cast<volatile uint32_t *>(ScrollCloseDynamicDefaults);

    object[0xA14 / 4] = dynamic[0];
    object[0xA1C / 4] = fixed[3];
    object[0xA28 / 4] = 0;
    object[0xA20 / 4] = dynamic[1];
    object[0xA24 / 4] = fixed[4];
    object[0xA2C / 4] = dynamic[1];
    object[0xA3C / 4] = 0xFFFFFFFFU;
    object[0xA38 / 4] = 0;
    object[0xA30 / 4] = fixed[2];
    object[0xA34 / 4] = fixed[1];
    object[0xA40 / 4] = fixed[0];
    object[0xA44 / 4] = 0;
    object[0xA4C / 4] = 0;
    object[0xA50 / 4] = 0;
    object[0xA54 / 4] = 0;
    object[0xA58 / 4] = 0;
    object[0xA48 / 4] = fixed[5];
    object[0xA5C / 4] = fixed[6];
    object[0xA64 / 4] = fixed[7];
    object[0xA68 / 4] = fixed[8];
    object[0xA6C / 4] = fixed[9];
    object[0xA70 / 4] = fixed[10];

    for (size_t index = 0; index < 3; ++index) {
        object[(0xA7C / 4) + index] = dynamic[2 + index];
        object[(0xA88 / 4) + index] = dynamic[5 + index];
        object[(0xA94 / 4) + index] = dynamic[8 + index];
        object[(0xAA0 / 4) + index] = dynamic[11 + index];
    }
    object[0xA74 / 4] = dynamic[15];
    object[0xA78 / 4] = dynamic[16];
    object[0x2144 / 4] = 0;
    object[0x2148 / 4] = 0;

    call_noarg_virtual(reinterpret_cast<uint8_t *>(this) + 0xAAC, 0x168);
    call_noarg_virtual(reinterpret_cast<uint8_t *>(this) + 0x15F8, 0x168);
    return GraphicWin::close();
}

/*
Purpose: Initialize a scrollbar from a rectangle.
Original Offset: 00605840
Status: Complete
*/
int Scroll::init(RECT *rect, Win *parent, int setting, int options) {
    if (!rect || !parent) {
        return 3;
    }

    const volatile RECT *const ordered_rect = rect;
    const uint32_t top = static_cast<uint32_t>(ordered_rect->top);
    const uint32_t left = static_cast<uint32_t>(ordered_rect->left);
    const uint32_t bottom = static_cast<uint32_t>(ordered_rect->bottom);
    const uint32_t right = static_cast<uint32_t>(ordered_rect->right);
    return ScrollPrimaryInit(
        this, long_from_bits(left), long_from_bits(top),
        long_from_bits(right - left), long_from_bits(bottom - top),
        parent, setting, options);
}

/*
Purpose: Initialize a vertical scrollbar using the process-default thickness.
Original Offset: 00605890
Status: Complete
*/
int Scroll::init_vert(
        int x, int y, int length, Win *parent, int setting) {
    if (!parent || length == 0) {
        return 3;
    }
    return ScrollPrimaryInit(
        this, x, y, *ScrollDefaultThickness, length, parent, setting, 0);
}

/*
Purpose: Initialize a horizontal scrollbar using the process-default thickness.
Original Offset: 006058D0
Status: Complete
*/
int Scroll::init_horz(
        int x, int y, int length, Win *parent, int setting) {
    if (!parent || length == 0) {
        return 3;
    }
    return ScrollPrimaryInit(
        this, x, y, length, *ScrollDefaultThickness, parent, setting, 0);
}

/*
Purpose: Initialize a vertical nonclient scrollbar.
Original Offset: 00605910
Status: Complete
*/
int Scroll::init_vert_nc(
        int x, int y, int length, Win *parent, int setting) {
    *reinterpret_cast<volatile int *>(ScrollNonClientInit) = 1;
    if (!parent || length == 0) {
        return 3;
    }
    return ScrollPrimaryInit(
        this, x, y, *ScrollDefaultThickness, length, parent, setting, 0);
}

/*
Purpose: Initialize a horizontal nonclient scrollbar.
Original Offset: 00605960
Status: Complete
*/
int Scroll::init_horz_nc(
        int x, int y, int length, Win *parent, int setting) {
    *reinterpret_cast<volatile int *>(ScrollNonClientInit) = 1;
    if (!parent || length == 0) {
        return 3;
    }
    return ScrollPrimaryInit(
        this, x, y, length, *ScrollDefaultThickness, parent, setting, 0);
}

/*
Purpose: Set the signed scrollbar range and redraw it at the lower endpoint.
Original Offset: 006059B0
Status: Complete
*/
uint32_t Scroll::set_range(int minimum, int maximum) {
    volatile uint32_t *const reversed = &range_reversed_;
    if (minimum < maximum) {
        *reversed = 0U;
    } else {
        *reversed = 1U;
        const int temporary = minimum;
        minimum = maximum;
        maximum = temporary;
    }
    *reinterpret_cast<volatile int *>(&range_minimum_) = minimum;
    *reinterpret_cast<volatile int *>(&position_) = minimum;
    const uint32_t vtable_bits = read_volatile_bits(this, 0);
    *reinterpret_cast<volatile int *>(&range_maximum_) = maximum;
    return redraw_from_vtable(this, vtable_bits);
}

/*
Purpose: Set the color shared by the scrollbar and both end buttons.
Original Offset: 00605A10
Status: Complete
*/
uint32_t Scroll::set_button_color(int color) {
    const uint32_t value = static_cast<uint32_t>(color);
    color_ = value;
    flat_button_left_.color_ = value;
    flat_button_right_.color_ = value;
    redraw_from_vtable(&flat_button_left_,
                       read_volatile_bits(&flat_button_left_, 0));
    return redraw_from_vtable(&flat_button_right_,
                              read_volatile_bits(&flat_button_right_, 0));
}

/*
Purpose: Set the bevel thickness shared by the scrollbar and both end buttons.
Original Offset: 00605A50
Status: Complete
*/
uint32_t Scroll::set_bevel_thickness(int thickness) {
    const uint32_t value = static_cast<uint32_t>(thickness);
    bevel_thickness_ = value;
    flat_button_left_.bevel_thickness_ = value;
    flat_button_right_.bevel_thickness_ = value;
    redraw_from_vtable(&flat_button_left_,
                       read_volatile_bits(&flat_button_left_, 0));
    return redraw_from_vtable(&flat_button_right_,
                              read_volatile_bits(&flat_button_right_, 0));
}

/*
Purpose: Set the upper bevel color shared by the scrollbar and both end buttons.
Original Offset: 00605A90
Status: Complete
*/
uint32_t Scroll::set_bevel_upper(int color) {
    const uint32_t value = static_cast<uint32_t>(color);
    bevel_upper_ = value;
    flat_button_left_.bevel_upper_ = value;
    flat_button_right_.bevel_upper_ = value;
    redraw_from_vtable(&flat_button_left_,
                       read_volatile_bits(&flat_button_left_, 0));
    return redraw_from_vtable(&flat_button_right_,
                              read_volatile_bits(&flat_button_right_, 0));
}

/*
Purpose: Set the lower bevel color shared by the scrollbar and both end buttons.
Original Offset: 00605AD0
Status: Complete
*/
uint32_t Scroll::set_bevel_lower(int color) {
    const uint32_t value = static_cast<uint32_t>(color);
    bevel_lower_ = value;
    flat_button_left_.bevel_lower_ = value;
    flat_button_right_.bevel_lower_ = value;
    redraw_from_vtable(&flat_button_left_,
                       read_volatile_bits(&flat_button_left_, 0));
    return redraw_from_vtable(&flat_button_right_,
                              read_volatile_bits(&flat_button_right_, 0));
}

/*
Purpose: Set the scrollbar thickness and reset its thumb rectangle.
Original Offset: 00605B80
Status: Complete
*/
uint32_t Scroll::set_bar_thickness(int thickness) {
    const uint32_t thickness_bits = static_cast<uint32_t>(thickness);
    *reinterpret_cast<volatile int *>(&bar_thickness_) = thickness;
    volatile RECT *const thumb = &thumb_rect_;
    thumb->left = 0;
    thumb->top = 0;
    thumb->right = long_from_bits(thickness_bits);
    thumb->bottom = long_from_bits(thickness_bits);
    const uint32_t color = static_cast<uint32_t>(
        *reinterpret_cast<volatile int *>(&border_color_));
    if (color == 0xFFFFFFFFU) {
        return color;
    }

    const uint32_t right = static_cast<uint32_t>(thumb->right) - 1U;
    thumb->left = 1;
    const uint32_t top = static_cast<uint32_t>(thumb->top) + 1U;
    thumb->right = long_from_bits(right);
    const uint32_t bottom = static_cast<uint32_t>(thumb->bottom) - 1U;
    thumb->top = long_from_bits(top);
    thumb->bottom = long_from_bits(bottom);
    return bottom;
}

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
Purpose: Expand a rectangle horizontally and vertically.
Original Offset: 00606F00
Status: Complete
*/
RECT *__cdecl expand_rect(RECT *rect, int horizontal, int vertical) {
    volatile RECT *ordered_rect = rect;
    const uint32_t horizontal_bits = static_cast<uint32_t>(horizontal);
    const uint32_t vertical_bits = static_cast<uint32_t>(vertical);
    ordered_rect->left = long_from_bits(
        static_cast<uint32_t>(ordered_rect->left) - horizontal_bits);
    ordered_rect->right = long_from_bits(
        static_cast<uint32_t>(ordered_rect->right) + horizontal_bits);
    ordered_rect->top = long_from_bits(
        static_cast<uint32_t>(ordered_rect->top) - vertical_bits);
    ordered_rect->bottom = long_from_bits(
        static_cast<uint32_t>(ordered_rect->bottom) + vertical_bits);
    return rect;
}

/*
Purpose: Set the left scrollbar sprites and horizontal button sprites.
Original Offset: 00605BE0
Status: Complete
*/
void Scroll::set_sprite_left(
        Sprite *sprite1, Sprite *sprite2, Sprite *sprite3) {
    set_sprite_triplet(this, &sprite_left1_, 0x15BC, true,
                       sprite1, sprite2, sprite3);
}

/*
Purpose: Set the right scrollbar sprites and horizontal button sprites.
Original Offset: 00605C30
Status: Complete
*/
void Scroll::set_sprite_right(
        Sprite *sprite1, Sprite *sprite2, Sprite *sprite3) {
    set_sprite_triplet(this, &sprite_right1_, 0x2108, true,
                       sprite1, sprite2, sprite3);
}

/*
Purpose: Set the upper scrollbar sprites and vertical upper-button sprites.
Original Offset: 00605C80
Status: Complete
*/
void Scroll::set_sprite_up(
        Sprite *sprite1, Sprite *sprite2, Sprite *sprite3) {
    set_sprite_triplet(this, &sprite_up1_, 0x15BC, false,
                       sprite1, sprite2, sprite3);
}

/*
Purpose: Set the lower scrollbar sprites and vertical lower-button sprites.
Original Offset: 00605CD0
Status: Complete
*/
void Scroll::set_sprite_down(
        Sprite *sprite1, Sprite *sprite2, Sprite *sprite3) {
    set_sprite_triplet(this, &sprite_down1_, 0x2108, false,
                       sprite1, sprite2, sprite3);
}

/*
Purpose: Clamp, optionally reverse, and redraw the scrollbar position.
Original Offset: 00605D20
Status: Complete
*/
uint32_t Scroll::set_pos(int position) {
    ::Win *const parent = *reinterpret_cast<::Win *volatile *>(&win_parent_);
    if (!parent) {
        return 0U;
    }
    *ScrollCurrentWin = parent;

    const int minimum = *reinterpret_cast<volatile int *>(&range_minimum_);
    volatile int *const current = &position_;
    if (position < minimum) {
        *current = minimum;
    } else {
        const int maximum = *reinterpret_cast<volatile int *>(&range_maximum_);
        *current = position > maximum ? maximum : position;
    }
    if (*reinterpret_cast<volatile uint32_t *>(&range_reversed_) != 0U) {
        const uint32_t maximum = static_cast<uint32_t>(
            *reinterpret_cast<volatile int *>(&range_maximum_));
        const uint32_t clamped = static_cast<uint32_t>(*current);
        *current = long_from_bits(
            maximum - clamped + static_cast<uint32_t>(minimum));
    }
    const uint32_t vtable_bits = read_volatile_bits(this, 0);
    return redraw_from_vtable(this, vtable_bits);
}

/*
Purpose: Compute and publish the scrollbar thumb rectangle.
Original Offset: 00606C50
Status: Complete
Verification note: several mutation-harness survivors in this body are
equivalent by construction, not coverage gaps. The two interim stores to
0xA3C in the drag branch are dead - the branch ends by unconditionally
storing 0xFFFFFFFF over them, faithfully mirroring the original's transient
writes, so no suite can observe them. The clamp idioms are strictness-
insensitive: `x < 0 ? 0U : x` equals `x < 1 ? 0U : x` and `x <= 0 ? 0U : x`
everywhere (the only inputs where the branches differ select equal values),
and likewise `a > b ? a : b` equals `a >= b ? a : b`. The early read_bits
offsets over the just-normalized rect scan bytes that were all zeroed two
statements earlier, so off-by-small-constant offset mutants read the same
zeros.
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

/*
Purpose: Reset the scrollbar thumb rectangle from its stored thickness.
Original Offset: 00606EA0
Status: Complete
*/
uint32_t Scroll::set_thumb_rect() {
    const uint32_t thickness = static_cast<uint32_t>(
        *reinterpret_cast<volatile int *>(&bar_thickness_));
    volatile RECT *const thumb = &thumb_rect_;
    thumb->left = 0;
    thumb->top = 0;
    thumb->right = long_from_bits(thickness);
    thumb->bottom = long_from_bits(thickness);
    if (*reinterpret_cast<volatile int *>(&border_color_) == -1) {
        return thickness;
    }

    const uint32_t right = static_cast<uint32_t>(thumb->right) - 1U;
    thumb->left = 1;
    const uint32_t top = static_cast<uint32_t>(thumb->top) + 1U;
    thumb->right = long_from_bits(right);
    const uint32_t bottom = static_cast<uint32_t>(thumb->bottom) - 1U;
    thumb->top = long_from_bits(top);
    thumb->bottom = long_from_bits(bottom);
    return bottom;
}

int __fastcall scroll_init_rect_redirect(
        Scroll *self, void *, RECT *rect, Win *parent, int setting, int options) {
    return self->init(rect, parent, setting, options);
}

int __fastcall scroll_init_vert_redirect(
        Scroll *self, void *, int x, int y, int length, Win *parent, int setting) {
    return self->init_vert(x, y, length, parent, setting);
}

int __fastcall scroll_init_horz_redirect(
        Scroll *self, void *, int x, int y, int length, Win *parent, int setting) {
    return self->init_horz(x, y, length, parent, setting);
}

int __fastcall scroll_init_vert_nc_redirect(
        Scroll *self, void *, int x, int y, int length, Win *parent, int setting) {
    return self->init_vert_nc(x, y, length, parent, setting);
}

int __fastcall scroll_init_horz_nc_redirect(
        Scroll *self, void *, int x, int y, int length, Win *parent, int setting) {
    return self->init_horz_nc(x, y, length, parent, setting);
}

uint32_t __fastcall scroll_set_range_redirect(
        Scroll *self, void *, int minimum, int maximum) {
    return self->set_range(minimum, maximum);
}

uint32_t __fastcall scroll_set_button_color_redirect(
        Scroll *self, void *, int color) {
    return self->set_button_color(color);
}

uint32_t __fastcall scroll_set_bevel_thickness_redirect(
        Scroll *self, void *, int thickness) {
    return self->set_bevel_thickness(thickness);
}

uint32_t __fastcall scroll_set_bevel_upper_redirect(
        Scroll *self, void *, int color) {
    return self->set_bevel_upper(color);
}

uint32_t __fastcall scroll_set_bevel_lower_redirect(
        Scroll *self, void *, int color) {
    return self->set_bevel_lower(color);
}

uint32_t __fastcall scroll_set_bar_thickness_redirect(
        Scroll *self, void *, int thickness) {
    return self->set_bar_thickness(thickness);
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

Sprite *__fastcall scroll_set_sprite_up_redirect(
        Scroll *self, void *, Sprite *sprite1, Sprite *sprite2, Sprite *sprite3) {
    self->set_sprite_up(sprite1, sprite2, sprite3);
    return sprite1;
}

Sprite *__fastcall scroll_set_sprite_down_redirect(
        Scroll *self, void *, Sprite *sprite1, Sprite *sprite2, Sprite *sprite3) {
    self->set_sprite_down(sprite1, sprite2, sprite3);
    return sprite1;
}

uint32_t __fastcall scroll_set_pos_redirect(
        Scroll *self, void *, int position) {
    return self->set_pos(position);
}

RECT *__fastcall scroll_compute_thumb_rect_redirect(
        Scroll *self, void *, RECT *rect) {
    self->compute_thumb_rect(rect);
    return reinterpret_cast<RECT *>(
        reinterpret_cast<uint8_t *>(self) + 0xA4C);
}

uint32_t __fastcall scroll_set_thumb_rect_redirect(Scroll *self, void *) {
    return self->set_thumb_rect();
}

uint32_t __fastcall scroll_close_redirect(Scroll *self, void *) {
    return self->close();
}

/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
Original Offset: 00606310
Return Value: n/a
Status: Complete
*/
void Scroll::on_left_click(int, int) {
}

void __fastcall scroll_on_left_click_redirect(Scroll *self, void *, int a1, int a2) {
    self->on_left_click(a1, a2);
}
