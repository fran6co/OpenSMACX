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

void __fastcall scroll_set_border_color_redirect(
        Scroll *self, void *, int color) {
    self->set_border_color(color);
}
