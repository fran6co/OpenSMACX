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

const uint32_t WinPrimaryVtable = 0x0066FDD0;
const uint32_t WinSecondaryVtable = 0x0066FF30;
uint32_t *WinStaticDefaults = reinterpret_cast<uint32_t *>(0x00696D34);
uint32_t *WinDynamicDefaults = reinterpret_cast<uint32_t *>(0x009B7AF0);

/*
Purpose: Construct a Win from its AutoSound subobject and the process window
         defaults, preserving every sparse write and legacy return residue.
Original Offset: 005EB3D0
Status: Complete
*/
void Win::construct() {
    auto_sound_.construct();
    volatile uint32_t *const object =
        reinterpret_cast<volatile uint32_t *>(this);
    volatile const uint32_t *const fixed = WinStaticDefaults;
    volatile const uint32_t *const dynamic = WinDynamicDefaults;

    object[0x0C8 / 4] = WinSecondaryVtable;
    object[0x0CC / 4] = 0;
    object[0x0D0 / 4] = 0;
    object[0x0D4 / 4] = 0;
    object[0x0D8 / 4] = 0;
    object[0x0DC / 4] = 0;
    object[0x000 / 4] = WinPrimaryVtable;
    object[0x0A8 / 4] = reinterpret_cast<uintptr_t>(this);
    object[0x3FC / 4] = 0;
    object[0x09C / 4] = 0;
    object[0x0A0 / 4] = 0;
    object[0x0A4 / 4] = 0;
    object[0x0AC / 4] = 0;
    object[0x0B0 / 4] = 0;
    object[0x134 / 4] = 0;
    object[0x138 / 4] = 0;
    object[0x188 / 4] = 0;
    object[0x18C / 4] = 0;
    object[0x190 / 4] = 0;
    object[0x194 / 4] = 0;
    object[0x198 / 4] = 0;
    object[0x184 / 4] = 0;
    object[0x0C4 / 4] = 0;
    object[0x0F0 / 4] = 0;
    object[0x19C / 4] = 0;
    object[0x12C / 4] = 0;
    object[0x130 / 4] = 1;
    object[0x0FC / 4] = dynamic[0];
    object[0x100 / 4] = fixed[0];
    object[0x114 / 4] = fixed[1];
    object[0x104 / 4] = dynamic[2];
    object[0x108 / 4] = fixed[2];
    object[0x10C / 4] = fixed[3];
    object[0x110 / 4] = fixed[4];
    object[0x118 / 4] = fixed[5];
    object[0x11C / 4] = fixed[6];
    object[0x120 / 4] = fixed[7];
    object[0x124 / 4] = dynamic[3];
    object[0x128 / 4] = fixed[8];
    object[0x0F8 / 4] = dynamic[1];
    object[0x0E0 / 4] = 0;
    object[0x0E4 / 4] = 0;
    object[0x0E8 / 4] = 0;
    object[0x0EC / 4] = 0;
    object[0x43C / 4] = 0;
    object[0x440 / 4] = 0;
    object[0x0F4 / 4] = 0;
    object[0x15C / 4] = 0;
    object[0x160 / 4] = 0;
    object[0x164 / 4] = 0;
    object[0x168 / 4] = 0;
    for (size_t offset = 0x13C; offset <= 0x158; offset += 4) {
        object[offset / 4] = 0;
    }
    object[0x0B4 / 4] = 0;
    object[0x0C0 / 4] = 0;
    object[0x0BC / 4] = 0;
    object[0x0B8 / 4] = 0;
    object[0x16C / 4] = 0;
    object[0x170 / 4] = 0;
    object[0x098 / 4] = 0;
    for (size_t offset = 0x400; offset <= 0x438; offset += 4) {
        object[offset / 4] = 0;
    }
    object[0x174 / 4] = 1;
    object[0x178 / 4] = 1;
    object[0x17C / 4] = 1;
    object[0x180 / 4] = 1;
    object[0x1A0 / 4] = 2;
#if defined(__GNUC__) && defined(__i386__)
    __asm__ __volatile__("" : : "a"(this) : "memory");
#endif
}

Win *__fastcall win_construct_redirect(Win *self, void *) {
    self->construct();
    return self;
}

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

/*
Purpose: Report whether this window holds the dialog focus, either directly or
         as its parent's current focus target.
Original Offset: 005F2CA0
Return Value: Holds focus (1); does not (0)
Status: Complete
*/
int Win::is_dialog_focus() {
    if ((field_98_ & 0x1000U) != 0) {
        return 1;
    }
    Win *const parent = win_parent_;
    if (parent) {
        // An empty focus list reads as no focus rather than dereferencing the
        // list pointer, which the legacy body leaves untouched in that case.
        const uintptr_t focused = (parent->field_CC_ == 0)
            ? 0U
            : *reinterpret_cast<const uintptr_t *>(
                  static_cast<uintptr_t>(parent->field_D0_) + 4);
        if (focused == reinterpret_cast<uintptr_t>(this)) {
            return 1;
        }
    }
    return 0;
}

int __fastcall win_is_dialog_focus_redirect(Win *self, void *) {
    return self->is_dialog_focus();
}

int *WinHdcRefCount = reinterpret_cast<int *>(0x009B3AB0);
HDC *WinSharedHdc = reinterpret_cast<HDC *>(0x009B7B2C);
void **WinHdcSurface = reinterpret_cast<void **>(0x009BC498);
// The same window handle temp.cpp binds as HandleMain; bound here too so
// the device-context protocol carries its own rebindable dependency.
HWND *WinHdcWindow = reinterpret_cast<HWND *>(0x009B7B28);

namespace {

#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wattributes"
#endif
typedef long(__stdcall *func_win_surface_slot)(void *, void *);
#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif

constexpr size_t WinSurfaceGetDCSlot = 0x44;
constexpr size_t WinSurfaceReleaseDCSlot = 0x68;

void *win_surface_slot(void *object, size_t offset) {
    void **const vtable = *reinterpret_cast<void ***>(object);
    return vtable[offset / sizeof(void *)];
}

}  // namespace

/*
Purpose: Acquire the process-wide device context, taking one reference.
Original Offset: 005EC690
Return Value: The shared device context, or zero when none could be obtained
Status: Complete
*/
HDC Win::get_hdc() {
    // A context already held is simply counted again; only the first
    // reference actually acquires one.
    if (*WinHdcRefCount != 0) {
        ++*WinHdcRefCount;
        return *WinSharedHdc;
    }
    void *const surface = *WinHdcSurface;
    if (!surface) {
        *WinSharedHdc = GetDC(*WinHdcWindow);
    } else {
        reinterpret_cast<func_win_surface_slot>(
            win_surface_slot(surface, WinSurfaceGetDCSlot))(
                surface, WinSharedHdc);
    }
    // A failed acquire leaves the count at zero so the next call retries.
    if (*WinSharedHdc != nullptr) {
        *WinHdcRefCount = 1;
    }
    return *WinSharedHdc;
}

/*
Purpose: Drop one reference to the process-wide device context, releasing it
         once the last reference is gone.
Original Offset: 005EC6F0
Status: Complete
Verification note: the two surviving mutants both concern the ReleaseDC call
on the no-surface path. It is a real GDI import whose effect no fixture can
inspect, so neither dropping it nor clearing the handle before it is
observable. Everything around it is covered: the reference count, the
surface release and its arguments, the handle being cleared, and the
exactly-zero test that makes an over-release drive the count negative and
skip the release entirely.
*/
void Win::release_hdc() {
    --*WinHdcRefCount;
    // The legacy body tests for exactly zero rather than at-or-below, so an
    // over-release drives the count negative and skips the release entirely.
    if (*WinHdcRefCount != 0) {
        return;
    }
    void *const surface = *WinHdcSurface;
    if (surface) {
        reinterpret_cast<func_win_surface_slot>(
            win_surface_slot(surface, WinSurfaceReleaseDCSlot))(
                surface, *WinSharedHdc);
        *WinSharedHdc = nullptr;
        return;
    }
    ReleaseDC(*WinHdcWindow, *WinSharedHdc);
    *WinSharedHdc = nullptr;
}

HDC __cdecl win_get_hdc_redirect() {
    return Win::get_hdc();
}

void __cdecl win_release_hdc_redirect() {
    Win::release_hdc();
}
