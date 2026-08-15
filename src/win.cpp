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
ORIGINAL: 0x005EB3D0
// name      ??0Win@@QAE@XZ
// size      621 bytes
// spans     0x005EB3D0-0x005EB63D
// prototype void (__thiscall ??0Win@@QAE@XZ)(Win* this)
// callers   1   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x0062BA80
// notes     Runtime redirect installed by DllMain after byte-signature validation
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
ORIGINAL: 0x005ED7D0
// name      ?move@Win@@QAEHHH@Z
// size      167 bytes
// spans     0x005ED7D0-0x005ED877
// prototype int (__thiscall ?move@Win@@QAEHHH@Z)(Win* this, int, int)
// callers   15   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// notes     Runtime redirect installed by DllMain after byte-signature validation
Status: Complete
*/
int Win::move(int x, int y) {
    move_rect((iSomeFlag_ & 2U) ? client_rect_ : outer_rect_, x, y);
    return 0;
}

/*
Purpose: Determine whether this window and every ancestor are visible.
ORIGINAL: 0x005F7E90 BYTE_EXACT
// name      ?is_visible@Win@@QAEHXZ
// size      38 bytes
// spans     0x005F7E90-0x005F7EB6
// prototype int (__thiscall ?is_visible@Win@@QAEHXZ)(Win* this)
// callers   120   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005F7E90
// notes     Runtime redirect installed by DllMain after byte-signature validation
Status: Complete
*/
int Win::is_visible() {
    if (!(iSomeFlag_ & 1U)) {
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
ORIGINAL: 0x005ED240
// name      ?client_to_screen@Win@@QAEXPAH0@Z
// size      133 bytes
// spans     0x005ED240-0x005ED2C5
// prototype void (__thiscall ?client_to_screen@Win@@QAEXPAH0@Z)(Win* this, int*, int*)
// callers   71   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005ED240
Status: Complete
*/
void Win::client_to_screen(int *x, int *y) {
    *x = int_from_bits(static_cast<uint32_t>(*x) + long_bits(client_rect_.left) + long_bits(outer_rect_.left));
    *y = int_from_bits(static_cast<uint32_t>(*y) + long_bits(client_rect_.top) + long_bits(outer_rect_.top));
    // Bit 5 marks a window whose coordinates are relative to its parent, so
    // the walk continues only while both that flag and a parent are present.
    if ((iFlags_ & 0x20U) == 0 || !win_parent_) {
        return;
    }
    win_parent_->client_to_screen(x, y);
    // Bit 15 additionally backs out the parent's own outer origin. The legacy
    // body re-reads win_parent_ for each subtraction rather than caching it.
    if ((iFlags_ & 0x8000U) == 0) {
        return;
    }
    *x = int_from_bits(static_cast<uint32_t>(*x) - long_bits(win_parent_->outer_rect_.left));
    *y = int_from_bits(static_cast<uint32_t>(*y) - long_bits(win_parent_->outer_rect_.top));
}

/*
Purpose: Announce this window as the one the palette should follow, then report
         that the message was handled.

             push ecx / call Palette::set_active_window / add esp, 4
             mov eax, 1 / ret

         `this` is passed as the only argument and the caller cleans it, which
         is the cdecl convention the callee's mangled name declares.
ORIGINAL: 0x005F1060 BYTE_EXACT
// name      ?on_query_new_palette@Win@@QAEHXZ
// size      15 bytes
// spans     0x005F1060-0x005F106F
// prototype int (__thiscall ?on_query_new_palette@Win@@QAEHXZ)(Win* this)
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005FE4F0
Return Value: 1, always
Status: Complete
Verification note: the call to Palette::set_active_window is NOT observed by
         any suite, and tools/mutate_and_verify.py reports dropping it as a
         surviving mutant. That is accurate rather than a coverage failure to
         chase: set_active_window is a recovered EMPTY body with no rebindable
         seam, so removing the call is behaviourally identical today and no
         fixture can distinguish them. The call is kept because it is what the
         original encodes. If set_active_window ever gains a body, this
         function needs a seam and a fixture that counts the call.
*/
int Win::on_query_new_palette() {
    Palette::set_active_window(this);
    return 1;
}

int __fastcall win_on_query_new_palette_redirect(Win *self, void *) {
    return self->on_query_new_palette();
}

/*
Purpose: Read the vertical scroll bar's current position.
ORIGINAL: 0x005EE050
// name      ?get_vert_pos@Win@@QAEHXZ
// size      20 bytes
// spans     0x005EE050-0x005EE064
// prototype int (__thiscall ?get_vert_pos@Win@@QAEHXZ)(Win* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: The scroll's position, or 0 when the window has no vertical
              scroll bar. The null check is the original's, not a guard added
              here: it loads the pointer, tests it, and branches to `xor eax,
              eax` rather than dereferencing.
Status: Complete
*/
int Win::get_vert_pos() {
    return scroll_vert_ ? scroll_vert_->position_ : 0;
}

int __fastcall win_get_vert_pos_redirect(Win *self, void *) {
    return self->get_vert_pos();
}

/*
Purpose: Read the horizontal scroll bar's current position.
ORIGINAL: 0x005EE090
// name      ?get_horz_pos@Win@@QAEHXZ
// size      20 bytes
// spans     0x005EE090-0x005EE0A4
// prototype int (__thiscall ?get_horz_pos@Win@@QAEHXZ)(Win* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: The scroll's position, or 0 when the window has no horizontal
              scroll bar. Identical to get_vert_pos above but reading the
              other pointer; the two differ only in which member they load,
              which is exactly what the fixtures have to separate.
Status: Complete
*/
int Win::get_horz_pos() {
    return scroll_horz_ ? scroll_horz_->position_ : 0;
}

int __fastcall win_get_horz_pos_redirect(Win *self, void *) {
    return self->get_horz_pos();
}

/*
Purpose: Set vertical scrollbar paging when a scrollbar is attached.
ORIGINAL: 0x005EE0F0
// name      ?set_vert_paging@Win@@QAEXH@Z
// size      23 bytes
// spans     0x005EE0F0-0x005EE107
// prototype void (__thiscall ?set_vert_paging@Win@@QAEXH@Z)(Win* this, int)
// callers   2   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// notes     Runtime redirect installed by DllMain after byte-signature validation
Status: Complete
*/
void Win::set_vert_paging(int paging) {
    if (scroll_vert_) {
        scroll_vert_->paging_ = paging;
    }
}

/*
Purpose: Set horizontal scrollbar paging when a scrollbar is attached.
ORIGINAL: 0x005EE110
// name      ?set_horz_paging@Win@@QAEXH@Z
// size      23 bytes
// spans     0x005EE110-0x005EE127
// prototype void (__thiscall ?set_horz_paging@Win@@QAEXH@Z)(Win* this, int)
// callers   2   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// notes     Runtime redirect installed by DllMain after byte-signature validation
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
ORIGINAL: 0x005FA7E0
// name      ?in_box@@YAHHHPAURECT@@@Z
// size      52 bytes
// spans     0x005FA7E0-0x005FA814
// prototype 
// callers   24   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// notes     Runtime redirect installed by DllMain after byte-signature validation
Return Value: Is the point inside the rectangle? true/false
Status: Complete
*/
BOOL __cdecl in_box(int x, int y, const RECT *rect) {
    return x >= rect->left && x < rect->right && y >= rect->top && y < rect->bottom;
}

/*
Purpose: Slide a rectangle by a delta on each axis, the wrapping counterpart of
         the Win32 OffsetRect. Its neighbour make_rect below builds rectangles
         with the same wrapping arithmetic, and this one is entered the same
         way: `ret` with no operand and the caller's `add esp, 0xc` at
         0x005EDC92, so __cdecl with three arguments. EAX still holds the
         rectangle pointer at the return because nothing overwrites it after
         the load at 0x005F8670, but that is a residue and not a value: the
         one caller clobbers EAX two instructions later at 0x005EDC95.
ORIGINAL: 0x005F8670
// name      sub_5f8670
// size      43 bytes
// spans     0x005F8670-0x005F869B
// prototype 
// callers   1   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Status: Complete
Verification note: the original writes left, right, top, bottom in that
         interleaved order, reloading the y delta between the right and top
         stores. The four fields cannot alias, so a mutant that swaps two
         adjacent stores is an equivalent mutant no assertion can separate;
         the order is pinned through `volatile` only so the emitted stores
         keep matching the original, exactly as make_rect does.
*/
void __cdecl offset_rect(RECT *rect, int dx, int dy) {
    volatile RECT *ordered = rect;
    const uint32_t x_bits = static_cast<uint32_t>(dx);
    const uint32_t y_bits = static_cast<uint32_t>(dy);
    ordered->left = long_from_bits(long_bits(ordered->left) + x_bits);
    ordered->right = long_from_bits(long_bits(ordered->right) + x_bits);
    ordered->top = long_from_bits(long_bits(ordered->top) + y_bits);
    ordered->bottom = long_from_bits(long_bits(ordered->bottom) + y_bits);
}

/*
Purpose: Build a rectangle from an origin and dimensions using wrapping coordinates.
ORIGINAL: 0x005F86C0
// name      sub_5f86c0
// size      38 bytes
// spans     0x005F86C0-0x005F86E6
// prototype 
// callers   2   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
// notes     Runtime redirect installed by DllMain after byte-signature validation
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
ORIGINAL: 0x005FA7A0
// name      ?in_box@@YAHHHHHHH@Z
// size      59 bytes
// spans     0x005FA7A0-0x005FA7DB
// prototype 
// callers   1   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// notes     Runtime redirect installed by DllMain after byte-signature validation
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
ORIGINAL: 0x004BA830
// name      ?UNK2@TutWin@@QAEXPAURECT@@PAHPAH@Z
// size      51 bytes
// spans     0x004BA830-0x004BA863
// prototype void (__thiscall ?UNK2@TutWin@@QAEXPAURECT@@PAHPAH@Z)(TutWin* this, RECT*, int*, int*)
// callers   0   call targets   0
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     (none)
// notes     Runtime redirect installed by DllMain after byte-signature validation
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
ORIGINAL: 0x005F2CA0
// name      ?is_dialog_focus@Win@@QAEHXZ
// size      63 bytes
// spans     0x005F2CA0-0x005F2CDF
// prototype int (__thiscall ?is_dialog_focus@Win@@QAEHXZ)(Win* this)
// callers   7   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: Holds focus (1); does not (0)
Status: Complete
*/
int Win::is_dialog_focus() {
    if ((iFlags_ & 0x1000U) != 0) {
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

typedef long(__stdcall *func_win_surface_slot)(void *, void *);

constexpr size_t WinSurfaceGetDCSlot = 0x44;
constexpr size_t WinSurfaceReleaseDCSlot = 0x68;

void *win_surface_slot(void *object, size_t offset) {
    void **const vtable = *reinterpret_cast<void ***>(object);
    return vtable[offset / sizeof(void *)];
}

}  // namespace

/*
Purpose: Acquire the process-wide device context, taking one reference.
ORIGINAL: 0x005EC690
// name      ?get_hdc@Win@@QAGPAUHDC__@@XZ
// size      81 bytes
// spans     0x005EC690-0x005EC6E1
// prototype 
// callers   7   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x005EC6B6 0x005EC6C7
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
ORIGINAL: 0x005EC6F0
// name      ?release_hdc@Win@@QAGXXZ
// size      76 bytes
// spans     0x005EC6F0-0x005EC73C
// prototype 
// callers   9   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x005EC710 0x005EC72B
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
ORIGINAL: 0x005EC7C0
// name      ?set_cursor@Win@@QAEHH@Z
// size      62 bytes
// spans     0x005EC7C0-0x005EC7FE
// prototype int (__thiscall ?set_cursor@Win@@QAEHH@Z)(Win* this, int nCursorName)
// callers   13   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005F1820
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
ORIGINAL: 0x005F8500
// name      ?clear_bubble_text@Win@@QAGXXZ
// size      47 bytes
// spans     0x005F8500-0x005F852F
// prototype 
// callers   15   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005EFD20 0x005F7320
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
ORIGINAL: 0x005EC680 BYTE_EXACT
// name      ?UNK1@Win@@QAEHHHHHHHHHH@Z
// size      5 bytes
// spans     0x005EC680-0x005EC685
// prototype int (__thiscall ?UNK1@Win@@QAEHHHHHHHHHH@Z)(Win* this, int, int, int, int, int, int, int, int, int)
// callers   1   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
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
ORIGINAL: 0x005ED7C0 BYTE_EXACT
// name      ?UNK5@Win@@QAEHXZ
// size      3 bytes
// spans     0x005ED7C0-0x005ED7C3
// prototype int (__thiscall ?UNK5@Win@@QAEHXZ)(Win* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
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
ORIGINAL: 0x005EDFE0 BYTE_EXACT
// name      ?UNK6@Win@@QAEHH@Z
// size      5 bytes
// spans     0x005EDFE0-0x005EDFE5
// prototype int (__thiscall ?UNK6@Win@@QAEHH@Z)(Win* this, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
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
ORIGINAL: 0x005F6A30 BYTE_EXACT
// name      ?on_set_cursor@Win@@QAEHPAXII@Z
// size      8 bytes
// spans     0x005F6A30-0x005F6A38
// prototype int (__thiscall ?on_set_cursor@Win@@QAEHPAXII@Z)(Win* this, void*, unsigned int, unsigned int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
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
ORIGINAL: 0x005F2CE0
// name      ?set_def_focus@Win@@QAAXPAUWin@@@Z
// size      10 bytes
// spans     0x005F2CE0-0x005F2CEA
// prototype 
// callers   3   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
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
ORIGINAL: 0x005EE130
// name      ?UNK8@Win@@QAEXH@Z
// size      39 bytes
// spans     0x005EE130-0x005EE157
// prototype void (__thiscall ?UNK8@Win@@QAEXH@Z)(Win* this, int)
// callers   0   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
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
ORIGINAL: 0x005EE160
// name      ?UNK9@Win@@QAEXH@Z
// size      39 bytes
// spans     0x005EE160-0x005EE187
// prototype void (__thiscall ?UNK9@Win@@QAEXH@Z)(Win* this, int)
// callers   0   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
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
ORIGINAL: 0x005EE020 BYTE_EXACT
// name      ?reset_window_clip@Win@@QAEXXZ
// size      1 bytes
// spans     0x005EE020-0x005EE021
// prototype void (__thiscall ?reset_window_clip@Win@@QAEXXZ)(Win* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
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
ORIGINAL: 0x005F54B0 BYTE_EXACT
// name      ?on_move@Win@@QAEXHH@Z
// size      3 bytes
// spans     0x005F54B0-0x005F54B3
// prototype void (__thiscall ?on_move@Win@@QAEXHH@Z)(Win* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
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
ORIGINAL: 0x005F54C0 BYTE_EXACT
// name      ?on_size@Win@@QAEXIHH@Z
// size      3 bytes
// spans     0x005F54C0-0x005F54C3
// prototype void (__thiscall ?on_size@Win@@QAEXIHH@Z)(Win* this, unsigned int, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
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
ORIGINAL: 0x005F54D0 BYTE_EXACT
// name      ?on_size_nc@Win@@QAEXIHH@Z
// size      3 bytes
// spans     0x005F54D0-0x005F54D3
// prototype void (__thiscall ?on_size_nc@Win@@QAEXIHH@Z)(Win* this, unsigned int, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
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
ORIGINAL: 0x005F6A40 BYTE_EXACT
// name      ?on_sys_command@Win@@QAEHIHH@Z
// size      3 bytes
// spans     0x005F6A40-0x005F6A43
// prototype int (__thiscall ?on_sys_command@Win@@QAEHIHH@Z)(Win* this, unsigned int, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
#pragma warning(push)
#pragma warning(disable : 4716)
// The mangled name says `int`; the original is `ret 0xc` at 0x005F6A40
// and never sets EAX. `return 0;` would emit `xor eax, eax` and add two
// bytes, so falling off the end IS the behaviour.
int Win::on_sys_command(unsigned int, int, int) {
}
#pragma warning(pop)

int __fastcall win_on_sys_command_redirect(Win *self, void *, unsigned int a1, int a2, int a3) {
    return self->on_sys_command(a1, a2, a3);
}

// The active palette lives at a fixed address; the same seam basebutton.cpp
// uses. Its most-recently-set window generation counter is at 0x400.
Palette **WinActivePalette = reinterpret_cast<Palette **>(0x009B8180);

/*
Purpose: Bring this window's palette into step with the active one, but only
         when it has fallen behind - set_active_window is skipped when the
         cached generation at 0x184 already matches the palette's at 0x400.
ORIGINAL: 0x005F2C60
// name      ?sync_palette@Win@@QAEXXZ
// size      52 bytes
// spans     0x005F2C60-0x005F2C94
// prototype void (__thiscall ?sync_palette@Win@@QAEXXZ)(Win* this)
// callers   5   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005FE4F0
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
ORIGINAL: 0x005EE030 BYTE_EXACT
// name      ?set_vert_pos@Win@@QAEXH@Z
// size      23 bytes
// spans     0x005EE030-0x005EE047
// prototype void (__thiscall ?set_vert_pos@Win@@QAEXH@Z)(Win* this, int)
// callers   3   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00605D20
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
ORIGINAL: 0x005EE070 BYTE_EXACT
// name      ?set_horz_pos@Win@@QAEXH@Z
// size      23 bytes
// spans     0x005EE070-0x005EE087
// prototype void (__thiscall ?set_horz_pos@Win@@QAEXH@Z)(Win* this, int)
// callers   3   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00605D20
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
ORIGINAL: 0x005EE0B0 BYTE_EXACT
// name      ?set_vert_range@Win@@QAEXHH@Z
// size      28 bytes
// spans     0x005EE0B0-0x005EE0CC
// prototype void (__thiscall ?set_vert_range@Win@@QAEXHH@Z)(Win* this, int, int)
// callers   2   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x006059B0
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
ORIGINAL: 0x005EE0D0 BYTE_EXACT
// name      ?set_horz_range@Win@@QAEXHH@Z
// size      28 bytes
// spans     0x005EE0D0-0x005EE0EC
// prototype void (__thiscall ?set_horz_range@Win@@QAEXHH@Z)(Win* this, int, int)
// callers   2   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x006059B0
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

/*
Purpose: Window cursor-set hook; the legacy implementation returns 1 to report
         it handled the message and does nothing else.
ORIGINAL: 0x005F2670 BYTE_EXACT
// name      ?OnSetCursor@Win@@QAAHPAXPAXII@Z
// size      6 bytes
// spans     0x005F2670-0x005F2676
// prototype 
// callers   1   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: 1, always
Status: Complete
*/
int Win::OnSetCursor(void *, void *, unsigned int, unsigned int) {
    return 1;
}

int __cdecl win_onsetcursor_redirect(void *a1, void *a2, unsigned int a3, unsigned int a4) {
    return Win::OnSetCursor(a1, a2, a3, a4);
}

/*
Purpose: Report whether a value is present in the window's id table - the
         array at 0x1A4 with its count at 0x3FC. A zero value and an empty
         table both report absent.
ORIGINAL: 0x005ECE80
// name      ?UNK3@Win@@QAEHH@Z
// size      54 bytes
// spans     0x005ECE80-0x005ECEB6
// prototype int (__thiscall ?UNK3@Win@@QAEHH@Z)(Win* this, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: 1 when the value is in the table, 0 otherwise
Status: Complete
*/
int Win::UNK3(int value) {
    if (value == 0) {
        return 0;
    }
    int32_t count;
    std::memcpy(&count, reinterpret_cast<uint8_t *>(this) + 0x3FC, sizeof(count));
    uint8_t *const table = reinterpret_cast<uint8_t *>(this) + 0x1A4;
    for (int index = 0; index < count; ++index) {
        int32_t entry;
        std::memcpy(&entry, table + index * 4, sizeof(entry));
        if (entry == value) {
            return 1;
        }
    }
    return 0;
}

int __fastcall win_unk3_redirect(Win *self, void *, int value) {
    return self->UNK3(value);
}

/*
Purpose: Report whether a window is anywhere below this one in the child
         tree, searching depth first. A direct child counts, and so does any
         descendant of one.
ORIGINAL: 0x005ECE20
// name      ?is_descendant@Win@@QAEHPAVWin@@@Z
// size      83 bytes
// spans     0x005ECE20-0x005ECE73
// prototype int (__thiscall ?is_descendant@Win@@QAEHPAVWin@@@Z)(Win* this, Win*)
// callers   4   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005ECE20
Return Value: 1 when the candidate is a descendant, 0 otherwise
Status: Complete
Verification note: hoisting the count out of the loop is an EQUIVALENT mutant
         and will survive the sweep. The original reloads it every iteration,
         so this transcription does too, but the whole call tree here is
         read-only - nothing reachable from is_descendant writes to any
         object - so no execution can observe the reload. It is kept because
         it is what the bytes do, not because a test pins it. If this function
         ever gains a seam, that changes and the reload becomes testable.
         The absent null check on children_[index] is likewise faithful: the
         original dereferences the slot unconditionally at 0x005ECE2E, so a
         null slot faults in both.
*/
int Win::is_descendant(Win *candidate) {
    if (!candidate) {
        return 0;
    }
    int count = child_count_;
    if (count <= 0) {
        return 0;
    }
    for (int index = 0; ; ++index) {
        Win *const child = children_[index];
        if (child == candidate) {
            return 1;
        }
        if (child->is_descendant(candidate)) {
            return 1;
        }
        // Re-read, do NOT hoist. The original reloads the count at 0x005ECE50
        // after every recursive call returns, so a recursion that adds or
        // removes children is seen by the very next iteration. A loop-invariant
        // count is the obvious tidy-up and it changes behaviour.
        count = child_count_;
        if (index + 1 >= count) {
            return 0;
        }
    }
}

int __fastcall win_is_descendant_redirect(Win *self, void *, Win *candidate) {
    return self->is_descendant(candidate);
}

/*
ORIGINAL: 0x005F8530 BYTE_EXACT
// name      ?on_mousewheel_down_vert@Win@@QAEXH@Z
// size      23 bytes
// spans     0x005F8530-0x005F8547
// prototype void (__thiscall ?on_mousewheel_down_vert@Win@@QAEXH@Z)(Win* this, int)
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00606320
Status: Complete
*/
void Win::on_mousewheel_down_vert(int a1) {
    Scroll *scroll = scroll_vert_;
    if (scroll) {
        scroll->on_mousewheel_down(a1);
    }
}

/*
ORIGINAL: 0x005F8590 BYTE_EXACT
// name      ?on_mousewheel_up_horz@Win@@QAEXH@Z
// size      23 bytes
// spans     0x005F8590-0x005F85A7
// prototype void (__thiscall ?on_mousewheel_up_horz@Win@@QAEXH@Z)(Win* this, int)
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00606440
Status: Complete
*/
void Win::on_mousewheel_up_horz(int a1) {
    Scroll *scroll = scroll_horz_;
    if (scroll) {
        scroll->on_mousewheel_up(a1);
    }
}

/*
Purpose: Report the left mouse button state, honouring a swapped-buttons
         system setting.
ORIGINAL: 0x005EC960 BYTE_EXACT
// name      ?get_lbutton_state@Win@@QAEHXZ
// size      27 bytes
// spans     0x005EC960-0x005EC97B
// prototype int (__thiscall ?get_lbutton_state@Win@@QAEHXZ)(Win* this)
// callers   2   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x005EC963 0x005EC971
Return Value: nonzero while the logical left button is down
Status: Complete
*/
// ONE expression. Naming either result as a local is still correct C++, but
// VC6 then defers the `movsx` that widens the SHORT until after the second
// call instead of folding it straight after the first.
// DECLARED HERE THOUGH <windows.h> ALREADY DECLARES THEM, and not by
// oversight. These lines sit inside the region the byte match EXTRACTS: a
// body is lifted out and compiled in a standalone scaffold that includes no
// Windows headers at all, so without them the unit is `C2065: undeclared
// identifier` and the verdict is NO_COMPILE. Measured by deleting the
// GetSystemMetrics line: 0x005EC960 goes from BYTE_EXACT to NO_COMPILE.
//
// They are matching redeclarations, not redefinitions - identical to
// winuser.h's, so the build is indifferent and the measurement is not. If
// this grows past the two functions here, the place to fix it is the
// scaffold's PRELUDE in tools/emit_translation_unit.py rather than more
// lines like these.
extern "C" __declspec(dllimport) short __stdcall GetAsyncKeyState(int key);
extern "C" __declspec(dllimport) int __stdcall GetSystemMetrics(int index);

int Win::get_lbutton_state() {
    return GetSystemMetrics(0x17) ^ (GetAsyncKeyState(1) >> 15);
}

/*
ORIGINAL: 0x005F8550 BYTE_EXACT
// name      ?on_mousewheel_down_horz@Win@@QAEXH@Z
// size      23 bytes
// spans     0x005F8550-0x005F8567
// prototype void (__thiscall ?on_mousewheel_down_horz@Win@@QAEXH@Z)(Win* this, int)
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00606320
Status: Complete
*/
void Win::on_mousewheel_down_horz(int a1) {
    Scroll *scroll = scroll_horz_;
    if (scroll) {
        scroll->on_mousewheel_down(a1);
    }
}

/*
ORIGINAL: 0x005F8570 BYTE_EXACT
// name      ?on_mousewheel_up_vert@Win@@QAEXH@Z
// size      23 bytes
// spans     0x005F8570-0x005F8587
// prototype void (__thiscall ?on_mousewheel_up_vert@Win@@QAEXH@Z)(Win* this, int)
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00606440
Status: Complete
*/
void Win::on_mousewheel_up_vert(int a1) {
    Scroll *scroll = scroll_vert_;
    if (scroll) {
        scroll->on_mousewheel_up(a1);
    }
}

// Seams for the four unrecovered Win handlers, and the definitions that give
// the direct calls from the derived windows something to link against. Kept
// at the end of the file so nothing above - every body here carries an
// Original Offset the census reads by line number - moves.
func_win_hide WinOriginalHide = original_method<func_win_hide>(0x005EDCD0);
func_win_on_mouse_move WinOriginalOnMouseMove =
    original_method<func_win_on_mouse_move>(0x005F6320);
func_win_on_nc_hittest WinOriginalOnNcHittest =
    original_method<func_win_on_nc_hittest>(0x005F5AD0);
func_win_release_modal WinOriginalReleaseModal =
    original_method<func_win_release_modal>(0x005EE280);

void Win::hide() {
    (ORIGINAL(this)->*WinOriginalHide)();
}

void Win::on_mouse_move(int a1, int a2, unsigned int a3, int a4) {
    (ORIGINAL(this)->*WinOriginalOnMouseMove)(a1, a2, a3, a4);
}

int Win::on_nc_hittest(int a1, int a2) {
    return (ORIGINAL(this)->*WinOriginalOnNcHittest)(a1, a2);
}

void Win::release_modal() {
    (ORIGINAL(this)->*WinOriginalReleaseModal)();
}
