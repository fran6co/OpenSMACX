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

int int_from_bits(uint32_t bits) {
    int value;
    static_assert(sizeof(value) == sizeof(bits), "Win geometry requires 32-bit int");
    memcpy(&value, &bits, sizeof(value));
    return value;
}

uint32_t midpoint(uint32_t near_edge, uint32_t far_edge) {
    const uint32_t delta = far_edge - near_edge;
    const uint32_t adjusted = delta + (delta >> 31);
    const uint32_t half = (adjusted >> 1) | (adjusted & 0x80000000U);
    return near_edge + half;
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
Purpose: Determine whether this window and every ancestor are visible.
Original Offset: 005F7E90
Status: Complete
*/
int Win::is_visible() {
    if (!(field_9C_ & 1U)) {
        return 0;
    }
    // A detached window is visible on its own; otherwise the whole parent
    // chain must be visible, which the legacy body walks recursively.
    if (win_parent_ && !win_parent_->is_visible()) {
        return 0;
    }
    return 1;
}

/*
Purpose: Translate a client-relative point into screen coordinates by walking
         the parent chain.
Original Offset: 005ED240
Status: Complete
*/
void Win::client_to_screen(int *x, int *y) {
    *x = int_from_bits(static_cast<uint32_t>(*x) + long_bits(client_rect_.left) + long_bits(outer_rect_.left));
    *y = int_from_bits(static_cast<uint32_t>(*y) + long_bits(client_rect_.top) + long_bits(outer_rect_.top));
    // Bit 5 marks a window whose coordinates are relative to its parent, so
    // the walk continues only while both that flag and a parent are present.
    if ((field_98_ & 0x20U) == 0 || !win_parent_) {
        return;
    }
    win_parent_->client_to_screen(x, y);
    // Bit 15 additionally backs out the parent's own outer origin. The legacy
    // body re-reads win_parent_ for each subtraction rather than caching it.
    if ((field_98_ & 0x8000U) == 0) {
        return;
    }
    *x = int_from_bits(static_cast<uint32_t>(*x) - long_bits(win_parent_->outer_rect_.left));
    *y = int_from_bits(static_cast<uint32_t>(*y) - long_bits(win_parent_->outer_rect_.top));
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

int __fastcall win_is_visible_redirect(Win *self, void *) {
    return self->is_visible();
}

void __fastcall win_client_to_screen_redirect(
        Win *self, void *, int *x, int *y) {
    self->client_to_screen(x, y);
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

/*
Purpose: Build a rectangle from an origin and dimensions using wrapping coordinates.
Original Offset: 005F86C0
Status: Complete
*/
RECT *__cdecl make_rect(RECT *rect, int x, int y, int width, int height) {
    volatile RECT *ordered = rect;
    const uint32_t x_bits = static_cast<uint32_t>(x);
    const uint32_t y_bits = static_cast<uint32_t>(y);
    ordered->left = long_from_bits(x_bits);
    ordered->top = long_from_bits(y_bits);
    ordered->right = long_from_bits(x_bits + static_cast<uint32_t>(width));
    ordered->bottom = long_from_bits(y_bits + static_cast<uint32_t>(height));
    return rect;
}

/*
Purpose: Determine whether a point is inside an origin-and-dimensions rectangle.
Original Offset: 005FA7A0
Status: Complete
*/
int __cdecl in_box(int x, int y, int left, int top, int width, int height) {
    if (x < left) {
        return 0;
    }
    const int right = int_from_bits(
        static_cast<uint32_t>(left) + static_cast<uint32_t>(width));
    if (x >= right || y < top) {
        return 0;
    }
    const int bottom = int_from_bits(
        static_cast<uint32_t>(top) + static_cast<uint32_t>(height));
    return y < bottom;
}

/*
Purpose: Compute a rectangle center with wrapping subtraction and truncation toward zero.
Original Offset: 004BA830
Status: Complete
*/
int __cdecl rect_center(RECT *rect, int *x, int *y) {
    volatile RECT *ordered = rect;
    const uint32_t left = static_cast<uint32_t>(ordered->left);
    const uint32_t right = static_cast<uint32_t>(ordered->right);
    const int center_x = int_from_bits(midpoint(left, right));
    memcpy(x, &center_x, sizeof(center_x));

    const uint32_t top = static_cast<uint32_t>(ordered->top);
    const uint32_t bottom = static_cast<uint32_t>(ordered->bottom);
    const int center_y = int_from_bits(midpoint(top, bottom));
    memcpy(y, &center_y, sizeof(center_y));
    return center_y;
}

int __fastcall tutwin_rect_center_redirect(
        void *, void *, RECT *rect, int *x, int *y) {
    return rect_center(rect, x, y);
}
