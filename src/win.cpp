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
#include "win.h"

namespace {

LONG long_from_bits(uint32_t bits) {
    LONG value;
    static_assert(sizeof(value) == sizeof(bits), "Win geometry requires 32-bit LONG");
    memcpy(&value, &bits, sizeof(value));
    return value;
}

uint32_t long_bits(LONG value) {
    uint32_t bits;
    memcpy(&bits, &value, sizeof(bits));
    return bits;
}

void move_rect(RECT &rect, int x, int y) {
    const uint32_t x_bits = static_cast<uint32_t>(x);
    const uint32_t y_bits = static_cast<uint32_t>(y);
    const uint32_t dx = x_bits - long_bits(rect.left);
    const uint32_t dy = y_bits - long_bits(rect.top);
    rect.left = long_from_bits(long_bits(rect.left) + dx);
    rect.top = long_from_bits(long_bits(rect.top) + dy);
    rect.right = long_from_bits(long_bits(rect.right) + dx);
    rect.bottom = long_from_bits(long_bits(rect.bottom) + dy);
}

}  // namespace

/*
Purpose: Move the active window rectangle while preserving its dimensions.
Original Offset: 005ED7D0
Status: Complete
*/
int Win::move(int x, int y) {
    move_rect((field_9C_ & 2U) ? client_rect_ : outer_rect_, x, y);
    return 0;
}

/*
Purpose: Set vertical scrollbar paging when a scrollbar is attached.
Original Offset: 005EE0F0
Status: Complete
*/
void Win::set_vert_paging(int paging) {
    if (scroll_vert_) {
        scroll_vert_->paging_ = paging;
    }
}

/*
Purpose: Set horizontal scrollbar paging when a scrollbar is attached.
Original Offset: 005EE110
Status: Complete
*/
void Win::set_horz_paging(int paging) {
    if (scroll_horz_) {
        scroll_horz_->paging_ = paging;
    }
}

int __fastcall win_move_redirect(Win *self, void *, int x, int y) {
    return self->move(x, y);
}

void __fastcall win_set_vert_paging_redirect(Win *self, void *, int paging) {
    self->set_vert_paging(paging);
}

void __fastcall win_set_horz_paging_redirect(Win *self, void *, int paging) {
    self->set_horz_paging(paging);
}

/*
Purpose: Determine whether a point is inside a rectangle using Win32 edge semantics.
Original Offset: 005FA7E0
Return Value: Is the point inside the rectangle? true/false
Status: Complete
*/
BOOL __cdecl in_box(int x, int y, const RECT *rect) {
    return x >= rect->left && x < rect->right && y >= rect->top && y < rect->bottom;
}
