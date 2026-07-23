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
#include <cstring>
#include "palette.h"

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

func_win_update_cursor *WinUpdateCursorOriginal =
    (func_win_update_cursor *)0x005F1820;

/*
Purpose: Select a system cursor by name and refresh the displayed cursor.
Original Offset: 005EC7C0
Return Value: No errors (0); name outside the accepted range (3)
Status: Complete with a temporary dependency on the cursor refresh
Verification note: ordering the cursor_handle_ clear against the refresh is
not observable here - the refresh is passed a null window, so it cannot read
this object's state through the seam a fixture can see. The store order is
kept as the legacy body has it rather than because a test distinguishes it.
*/
int Win::set_cursor(int name) {
    // Only the system cursor range is accepted; anything else is rejected
    // before any field is touched.
    if (name <= 0x7EFF || name >= 0x7F8B) {
        return 3;
    }
    cursor_sprite_ = nullptr;
    cursor_name_ = name;
    cursor_handle_ = nullptr;
    WinUpdateCursorOriginal(nullptr, 1);
    return 0;
}

int __fastcall win_set_cursor_redirect(Win *self, void *, int name) {
    return self->set_cursor(name);
}

int *WinBubbleActive = reinterpret_cast<int *>(0x009B7A50);
int *WinBubbleCompanion = reinterpret_cast<int *>(0x009B7A4C);
RECT *WinBubbleRect = reinterpret_cast<RECT *>(0x009B6E38);
func_win_update_screen *WinUpdateScreenOriginal =
    (func_win_update_screen *)0x005F7320;
func_win_flip *WinFlipOriginal = (func_win_flip *)0x005EFD20;

/*
Purpose: Dismiss any pending bubble text and repaint the area it covered.
Original Offset: 005F8500
Status: Complete with temporary dependencies on the screen refresh and flip
*/
void Win::clear_bubble_text() {
    // Nothing to dismiss when no bubble is pending.
    if (*WinBubbleActive == 0) {
        return;
    }
    *WinBubbleCompanion = 0;
    *WinBubbleActive = 0;
    WinUpdateScreenOriginal(WinBubbleRect, nullptr);
    WinFlipOriginal(WinBubbleRect);
}

void __cdecl win_clear_bubble_text_redirect() {
    Win::clear_bubble_text();
}

/*
Purpose: Legacy stub; the original body returns 0 without reading its
         arguments.
Original Offset: 005EC680
Status: Complete
*/
int Win::UNK1(int, int, int, int, int, int, int, int, int) {
    return 0;
}

int __fastcall win_unk1_redirect(
        Win *self, void *, int a, int b, int c, int d, int e, int f, int g, int h, int i) {
    return self->UNK1(a, b, c, d, e, f, g, h, i);
}

/*
Purpose: Legacy stub; the original body returns 0 without reading its
         arguments.
Original Offset: 005ED7C0
Status: Complete
*/
int Win::UNK5() {
    return 0;
}

int __fastcall win_unk5_redirect(
        Win *self, void *) {
    return self->UNK5();
}

/*
Purpose: Legacy stub; the original body returns 0 without reading its
         arguments.
Original Offset: 005EDFE0
Status: Complete
*/
int Win::UNK6(int) {
    return 0;
}

int __fastcall win_unk6_redirect(
        Win *self, void *, int a) {
    return self->UNK6(a);
}

/*
Purpose: Legacy stub; the original body returns 1 without reading its
         arguments.
Original Offset: 005F6A30
Status: Complete
*/
int Win::on_set_cursor(void *, unsigned int, unsigned int) {
    return 1;
}

int __fastcall win_on_set_cursor_redirect(
        Win *self, void *, void *a, unsigned int b, unsigned int c) {
    return self->on_set_cursor(a, b, c);
}

int *WinDefaultFocus = reinterpret_cast<int *>(0x009B7AEC);

/*
Purpose: Record the window that receives focus by default.
Original Offset: 005F2CE0
Status: Complete
*/
void Win::set_def_focus(int focus) {
    *WinDefaultFocus = focus;
}

void __cdecl win_set_def_focus_redirect(int focus) {
    Win::set_def_focus(focus);
}

/*
Purpose: Publish a value into both attached scrollbars' first shared slot.
Original Offset: 005EE130
Status: Complete
*/
void Win::UNK8(int value) {
    // Each scrollbar is optional and tested separately, so one attached bar
    // is still updated when the other is absent.
    if (scroll_vert_) {
        scroll_vert_->field_2144_ = static_cast<uint32_t>(value);
    }
    if (scroll_horz_) {
        scroll_horz_->field_2144_ = static_cast<uint32_t>(value);
    }
}

/*
Purpose: Publish a value into both attached scrollbars' second shared slot.
Original Offset: 005EE160
Status: Complete
*/
void Win::UNK9(int value) {
    if (scroll_vert_) {
        scroll_vert_->field_2148_ = static_cast<uint32_t>(value);
    }
    if (scroll_horz_) {
        scroll_horz_->field_2148_ = static_cast<uint32_t>(value);
    }
}

void __fastcall win_unk8_redirect(Win *self, void *, int value) {
    self->UNK8(value);
}

void __fastcall win_unk9_redirect(Win *self, void *, int value) {
    self->UNK9(value);
}

/*
Purpose: Reset the window clip; the legacy implementation is a bare return.
Original Offset: 005EE020
Return Value: n/a
Status: Complete
*/
void Win::reset_window_clip() {
}

void __fastcall win_reset_window_clip_redirect(Win *self, void *) {
    self->reset_window_clip();
}

/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
Original Offset: 005F54B0
Return Value: n/a
Status: Complete
*/
void Win::on_move(int, int) {
}

void __fastcall win_on_move_redirect(Win *self, void *, int a1, int a2) {
    self->on_move(a1, a2);
}

/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
Original Offset: 005F54C0
Return Value: n/a
Status: Complete
*/
void Win::on_size(unsigned int, int, int) {
}

void __fastcall win_on_size_redirect(Win *self, void *, unsigned int a1, int a2, int a3) {
    self->on_size(a1, a2, a3);
}

/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
Original Offset: 005F54D0
Return Value: n/a
Status: Complete
*/
void Win::on_size_nc(unsigned int, int, int) {
}

void __fastcall win_on_size_nc_redirect(Win *self, void *, unsigned int a1, int a2, int a3) {
    self->on_size_nc(a1, a2, a3);
}

/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
Original Offset: 005F6A40
Return Value: n/a
Status: Complete
*/
void Win::on_sys_command(unsigned int, int, int) {
}

void __fastcall win_on_sys_command_redirect(Win *self, void *, unsigned int a1, int a2, int a3) {
    self->on_sys_command(a1, a2, a3);
}

// The active palette lives at a fixed address; the same seam basebutton.cpp
// uses. Its most-recently-set window generation counter is at 0x400.
Palette **WinActivePalette = reinterpret_cast<Palette **>(0x009B8180);

/*
Purpose: Bring this window's palette into step with the active one, but only
         when it has fallen behind - set_active_window is skipped when the
         cached generation at 0x184 already matches the palette's at 0x400.
Original Offset: 005F2C60
Return Value: n/a
Status: Complete
*/
void Win::sync_palette() {
    Palette *const active = *WinActivePalette;
    uint32_t generation;
    std::memcpy(&generation,
                reinterpret_cast<uint8_t *>(active) + 0x400, sizeof(generation));
    if (field_184_ == generation) {
        return;
    }
    Palette::set_active_window(this);
    std::memcpy(&generation,
                reinterpret_cast<uint8_t *>(*WinActivePalette) + 0x400,
                sizeof(generation));
    field_184_ = generation;
}

void __fastcall win_sync_palette_redirect(Win *self, void *) {
    self->sync_palette();
}

/*
Purpose: Forward a scrollbar position to the vertical Scroll the window owns,
         doing nothing when that scrollbar is absent.
Original Offset: 005EE030
Return Value: n/a
Status: Complete
*/
void Win::set_vert_pos(int position) {
    if (scroll_vert_) {
        scroll_vert_->set_pos(position);
    }
}

/*
Purpose: Forward a scrollbar position to the horizontal Scroll the window owns,
         doing nothing when that scrollbar is absent.
Original Offset: 005EE070
Return Value: n/a
Status: Complete
*/
void Win::set_horz_pos(int position) {
    if (scroll_horz_) {
        scroll_horz_->set_pos(position);
    }
}

/*
Purpose: Forward a scrollbar range to the vertical Scroll the window owns,
         doing nothing when that scrollbar is absent.
Original Offset: 005EE0B0
Return Value: n/a
Status: Complete
*/
void Win::set_vert_range(int minimum, int maximum) {
    if (scroll_vert_) {
        scroll_vert_->set_range(minimum, maximum);
    }
}

/*
Purpose: Forward a scrollbar range to the horizontal Scroll the window owns,
         doing nothing when that scrollbar is absent.
Original Offset: 005EE0D0
Return Value: n/a
Status: Complete
*/
void Win::set_horz_range(int minimum, int maximum) {
    if (scroll_horz_) {
        scroll_horz_->set_range(minimum, maximum);
    }
}

void __fastcall win_set_vert_pos_redirect(Win *self, void *, int position) {
    self->set_vert_pos(position);
}

void __fastcall win_set_horz_pos_redirect(Win *self, void *, int position) {
    self->set_horz_pos(position);
}

void __fastcall win_set_vert_range_redirect(Win *self, void *, int minimum, int maximum) {
    self->set_vert_range(minimum, maximum);
}

void __fastcall win_set_horz_range_redirect(Win *self, void *, int minimum, int maximum) {
    self->set_horz_range(minimum, maximum);
}
