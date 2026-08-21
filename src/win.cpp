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
#include "general.h"
#include "temp.h"
#include "dialog.h"
#include "sounddevice.h"
#include "menu.h"
#include <ddraw.h>  // IDirectDrawSurface::GetDC / ReleaseDC in the hdc pair

const uint32_t WinPrimaryVtable = 0x0066FDD0;
const uint32_t WinSecondaryVtable = 0x0066FF30;

/*
Purpose: Construct a Win from its AutoSound subobject and the process window
         defaults, preserving every sparse write and legacy return residue.
// ORIGINAL: 0x005EB3D0 ??0Win@@QAE@XZ 0x005EB3D0-0x005EB63D
// size      621 bytes
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
    for (offset = 0x400; offset <= 0x438; offset += 4) {
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
// ORIGINAL: 0x005ED7D0 ?move@Win@@QAEHHH@Z 0x005ED7D0-0x005ED877
// size      167 bytes
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
// ORIGINAL: 0x005F7E90 ?is_visible@Win@@QAEHXZ 0x005F7E90-0x005F7EB6 BYTE_EXACT
// size      38 bytes
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
// ORIGINAL: 0x005ED240 ?client_to_screen@Win@@QAEXPAH0@Z 0x005ED240-0x005ED2C5
// size      133 bytes
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
// ORIGINAL: 0x005F1060 ?on_query_new_palette@Win@@QAEHXZ 0x005F1060-0x005F106F BYTE_EXACT
// size      15 bytes
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
// ORIGINAL: 0x005EE050 ?get_vert_pos@Win@@QAEHXZ 0x005EE050-0x005EE064 BYTE_EXACT
// size      20 bytes
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
// ORIGINAL: 0x005EE090 ?get_horz_pos@Win@@QAEHXZ 0x005EE090-0x005EE0A4 BYTE_EXACT
// size      20 bytes
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
// ORIGINAL: 0x005EE0F0 ?set_vert_paging@Win@@QAEXH@Z 0x005EE0F0-0x005EE107 BYTE_EXACT
// size      23 bytes
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
// ORIGINAL: 0x005EE110 ?set_horz_paging@Win@@QAEXH@Z 0x005EE110-0x005EE127 BYTE_EXACT
// size      23 bytes
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
// ORIGINAL: 0x005FA7E0 ?in_box@@YAHHHPAURECT@@@Z 0x005FA7E0-0x005FA814
// symbol    ?in_box@@YAHHHPBUtagRECT@@@Z
// size      52 bytes
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
// ORIGINAL: 0x005F8670 sub_5f8670 0x005F8670-0x005F869B
// symbol    ?offset_rect@@YAXPAUtagRECT@@HH@Z
// size      43 bytes
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
// ORIGINAL: 0x005F86C0 sub_5f86c0 0x005F86C0-0x005F86E6
// symbol    ?make_rect@@YAPAUtagRECT@@PAU1@HHHH@Z
// size      38 bytes
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
// ORIGINAL: 0x005FA7A0 ?in_box@@YAHHHHHHH@Z 0x005FA7A0-0x005FA7DB
// size      59 bytes
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
// ORIGINAL: 0x004BA830 ?UNK2@TutWin@@QAEXPAURECT@@PAHPAH@Z 0x004BA830-0x004BA863
// symbol    ?rect_center@@YAHPAUtagRECT@@PAH1@Z
// size      51 bytes
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
// ORIGINAL: 0x005F2CA0 ?is_dialog_focus@Win@@QAEHXZ 0x005F2CA0-0x005F2CDF
// size      63 bytes
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

int WinHdcRefCount;        // 0x009B3AB0
HDC WinSharedHdc;          // 0x009B7B2C
IDirectDrawSurface *DirectDrawSurface;  // 0x009BC498
Win *WinModalStack[4];     // 0x009B6EF8
HINSTANCE WinInstance;     // 0x009B7B14
int WinScreenWidth;        // 0x009B7B1C
int WinScreenHeight;       // 0x009B7B20
Win *WinTrackingWindow;    // 0x009B7AAC
Win *WinHoverWindow;       // 0x009B7A7C
Win *WinPointerOwner1;     // 0x009B7ACC
Win *WinPointerOwner2;     // 0x009B7AD0
Win *WinPointerOwner3;     // 0x009B7ABC
Win *WinPointerOwner4;     // 0x009B7AC0
Win *WinFocusWindow;       // 0x009B7AE0
Win *WinModalWindow;       // 0x009B7ADC
Win *WinRootWindows[64];   // 0x009B6E48
int WinRootCount;          // 0x009B7B34
int WinMouseDirect;        // 0x009B7AA4
int WinMouseScreenX;       // 0x009B6628
int WinMouseScreenY;       // 0x009B662C
int WinTrackingMode;       // 0x009B7AA8
int WinTrackingX;          // 0x009B7AB0
int WinTrackingY;          // 0x009B7AB4
Win *WinZOrderWindow;      // 0x009B7A6C
int WinZOrderCount;        // 0x009B7B30
int WinZOrderFlag;         // 0x009B7A78
void(__cdecl *WinLeftDownHook)(int x, int y);  // 0x009B7A90
int WinCursorMoved;        // 0x009B7B3C
void(__cdecl *WinMessageHook)();                            // 0x009B7A88
void(__cdecl *WinMouseHook)(HWND window, LPARAM position);  // 0x009B7A94
void(__cdecl *WinKeyHook)(WPARAM key);                      // 0x009B7A8C

/*
Purpose: Acquire the process-wide device context, taking one reference.
// ORIGINAL: 0x005EC690 ?get_hdc@Win@@QAGPAUHDC__@@XZ 0x005EC690-0x005EC6E1
// symbol    ?get_hdc@Win@@SAPAUHDC__@@XZ
// size      81 bytes
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
    if (WinHdcRefCount != 0) {
        ++WinHdcRefCount;
        return WinSharedHdc;
    }
    if (DirectDrawSurface == nullptr) {
        WinSharedHdc = GetDC(HandleMain);
    } else {
        DirectDrawSurface->GetDC(&WinSharedHdc);
    }
    // A failed acquire leaves the count at zero so the next call retries.
    if (WinSharedHdc != nullptr) {
        WinHdcRefCount = 1;
    }
    return WinSharedHdc;
}

/*
Purpose: Drop one reference to the process-wide device context, releasing it
         once the last reference is gone.
// ORIGINAL: 0x005EC6F0 ?release_hdc@Win@@QAGXXZ 0x005EC6F0-0x005EC73C BYTE_EXACT
// symbol    ?release_hdc@Win@@SAXXZ
// size      76 bytes
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
    --WinHdcRefCount;
    // The legacy body tests for exactly zero rather than at-or-below, so an
    // over-release drives the count negative and skips the release entirely.
    if (WinHdcRefCount != 0) {
        return;
    }
    if (DirectDrawSurface != nullptr) {
        DirectDrawSurface->ReleaseDC(WinSharedHdc);
        WinSharedHdc = nullptr;
        return;
    }
    ReleaseDC(HandleMain, WinSharedHdc);
    WinSharedHdc = nullptr;
}

HDC __cdecl win_get_hdc_redirect() {
    return Win::get_hdc();
}

void __cdecl win_release_hdc_redirect() {
    Win::release_hdc();
}


/*
Purpose: Select a system cursor by name and refresh the displayed cursor.
// ORIGINAL: 0x005EC7C0 ?set_cursor@Win@@QAEHH@Z 0x005EC7C0-0x005EC7FE
// size      62 bytes
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


// The globals `Win::flip` reaches, all of them past `.data`'s stored bytes
// and so zero-fill - real objects for the same reason `WinHdcRefCount` and
// `WinSharedHdc` are, and none of them named anywhere before this body came
// out of src/recovered.
//
// 0x009B7A48. Called with no arguments at the top of every flip when it is
// set. Nothing recovered installs it yet.
void (*WinFlipHook)();
// 0x009B7A54 / 0x009B7A64 / 0x00696D2C / 0x00696D30. The bubble's font, the
// colour its box is filled with, its text colour and its edge colour. The
// last two are in `.data` and both hold 255.
Font *WinBubbleFont;
int WinBubbleFillColour;
const int WinBubbleTextColour = 255;   // 0x00696D2C
const int WinBubbleEdgeColour = 255;   // 0x00696D30
// 0x009B7A2C and the pair below it: a sprite drawn over the screen after the
// bubble, at 0x009B7A34 across and 0x009B7A30 down. Its frame comes from its
// own `cTransparentIndex_` at +8.
Sprite *WinFlipSprite;
int WinFlipSpriteY;                    // 0x009B7A30
int WinFlipSpriteX;                    // 0x009B7A34
// 0x009B7AD8. Chooses how a partial flip reaches the screen: zero blits the
// whole surface, non-zero intersects the caller's rectangle first.
int WinFlipClipped;
// 0x009BC49C. The surface `DirectDrawSurface` is blitted FROM - the back
// buffer to its front.
IDirectDrawSurface *DirectDrawBackBuffer;
// 0x009BC2D0. The rectangle a clipped flip intersects against.
RECT DirectDrawClipRect;

/*
Purpose: Present the screen buffer: draw the bubble text over it, flip or
         blit the DirectDraw surface, and copy the result to the window.
// ORIGINAL: 0x005EFD20 ?flip@Win@@QAAXPAURECT@@@Z 0x005EFD20-0x005F01E7
// symbol    ?flip@Win@@SAXPAUtagRECT@@@Z
// size      1223 bytes
// callers   27   call targets   14
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005D8000 0x005DAC70 0x005DACB0 0x005DD020 0x005DD130 0x005DE8F0 0x005DFCD0 0x005E3203 0x005E3503 0x005E3563 0x005E4B4A 0x005EC6F0 0x006453E0 0x00645DD0
Return Value: n/a
Status: WIP

DERIVED FROM THE IMAGE, not ported from src/recovered/units/005efd20.cpp,
which is deleted. That unit was checked against the listing first and three
of its statements are wrong:

  it passed `BitBlt` the height as the width. The image loads
  `[0x9b7514]` and `[0x9b7510]`, NEGATES the first and pushes it as `cy`
  with the second as `cx` - so the width is `biWidth` and the height is
  `-biHeight`. The unit had them the other way round, which is a blit of
  the wrong shape every frame.

  it called the surface's slot 5 with NO arguments. The image pushes five
  plus the receiver: `Blt(nullptr, other, nullptr, DDBLT_WAIT, nullptr)`.
  The unit's own comment says the emitter's slots are nullary until someone
  fixes the signature, and nobody did.

  it never advanced the y between bubble lines - `top += 0`. The image adds
  a line height derived from the font: `unk_1_` if it is negative, and
  `height_ + unk_1_` if it is not.

A fourth thing about it is not a defect but reads as one: the block after
the bubble names its locals `top` and `right` for 0x9B6E40 and 0x9B6E3C,
which are `right` and `top`. The STORES are correct - the block undoes the
four adjustments the box made - so only the names mislead.

THREE OF ITS "GLOBALS" ARE MEMBERS. 0x009B74C0, 0x009B7510 and 0x009B7514
are `ScreenBuffer.rect2_`, `.dib_.bmiHeader.biWidth` and `.biHeight`:
`ScreenBuffer` is at 0x009B7490 and those are +0x30, +0x80 and +0x84. A
scaffold that binds globals by address cannot see that, which is most of
why it needed thirty-three of them.

THE SLOTS ARE `IDirectDrawSurface`, confirmed by the arity pushed at each
site: 5 is `Blt`, 11 is `Flip`, 17 is `GetDC`, 25 is `Lock`, 26 is
`ReleaseDC`, 32 is `Unlock`. Slot 11 being `Flip` is what this function is.
*/
void __cdecl Win::flip(RECT *area) {
    if (WinFlipHook != nullptr) {
        WinFlipHook();
    }

    if (*WinBubbleActive != 0) {
        if (area != nullptr) {
            ScreenBuffer.set_clip(area);
        }
        ScreenBuffer.box(WinBubbleRect, WinBubbleEdgeColour,
                         WinBubbleEdgeColour);
        // The box is drawn on the border and the fill inside it, so the
        // rectangle moves in by one on each side and back out below.
        WinBubbleRect->left += 1;
        WinBubbleRect->top -= 1;
        WinBubbleRect->right += 1;
        WinBubbleRect->bottom -= 1;
        ScreenBuffer.fill(WinBubbleRect, WinBubbleFillColour);
        ScreenBuffer.set_font(WinBubbleFont, nullptr, nullptr, nullptr);
        ScreenBuffer.set_text_color(WinBubbleTextColour, -1, 1, 1);

        LPSTR text = reinterpret_cast<LPSTR>(*WinBubbleActive);
        if (strchr(text, '^') == nullptr) {
            // One line, centred in the box.
            if (text != nullptr) {
                ScreenBuffer.write_cent_l(text, WinBubbleRect,
                                          static_cast<int>(strlen(text)));
            }
        } else {
            // `^` separates lines. Each is centred in turn and the
            // separator is put back so the caller's string survives.
            int y = WinBubbleRect->top + 1;
            LPSTR line = text;
            for (;;) {
                LPSTR const split = strchr(line, '^');
                if (split != nullptr) {
                    *split = '\0';
                }
                ScreenBuffer.write_cent_l(
                    line, WinBubbleRect->left, y,
                    WinBubbleRect->right - WinBubbleRect->left,
                    static_cast<int>(strlen(line)));
                y += (WinBubbleFont->unk_1_ < 0)
                    ? WinBubbleFont->line_height_
                    : WinBubbleFont->height_ + WinBubbleFont->unk_1_;
                if (split == nullptr) {
                    break;
                }
                *split = '^';
                line = split + 1;
            }
        }
    }

    WinBubbleRect->left -= 1;
    WinBubbleRect->top += 1;
    WinBubbleRect->right -= 1;
    WinBubbleRect->bottom += 1;
    if (area != nullptr) {
        ScreenBuffer.set_clip(&ScreenBuffer.rect2_);
    }

    if (WinFlipSprite != nullptr) {
        // The frame index is the sprite's own byte at +8.
        WinFlipSprite->draw(&ScreenBuffer, WinFlipSprite->cTransparentIndex_,
                            WinFlipSpriteY, WinFlipSpriteX, 1, 1);
    }

    if (DirectDrawSurface != nullptr) {
        // A clipped flip only reaches the screen when the caller's rectangle
        // meets the surface's; an unclipped one always does.
        RECT unused;
        if (WinFlipClipped == 0 || area == nullptr
            || IntersectRect(&unused, area, &DirectDrawClipRect)) {
            DirectDrawSurface->Blt(nullptr, DirectDrawBackBuffer, nullptr,
                                   DDBLT_WAIT, nullptr);
        } else {
            return;
        }
    }

    if (WinHdcRefCount != 0) {
        ++WinHdcRefCount;
    } else {
        WinSharedHdc = GetDC(HandleMain);
        if (WinSharedHdc != nullptr) {
            WinHdcRefCount = 1;
        }
    }
    if (WinSharedHdc == nullptr) {
        return;
    }

    const HDC source = ScreenBuffer.get_hdc();
    if (source == nullptr) {
        if (--WinHdcRefCount != 0) {
            return;
        }
        if (DirectDrawSurface != nullptr) {
            DirectDrawSurface->Blt(nullptr, DirectDrawBackBuffer, nullptr,
                                   DDBLT_WAIT, nullptr);
        }
        WinSharedHdc = nullptr;
        return;
    }

    ScreenBuffer.sync_to_palette(PaletteActive);

    if (BufferDirectDraw == nullptr) {
        if (WinHdcRefCount != 0) {
            ++WinHdcRefCount;
        } else if (DirectDrawSurface != nullptr) {
            DirectDrawSurface->Flip(nullptr, DDFLIP_WAIT);
            WinHdcRefCount = 1;
        } else {
            WinSharedHdc = GetDC(HandleMain);
            if (WinSharedHdc != nullptr) {
                WinHdcRefCount = 1;
            }
        }
        if (WinSharedHdc != nullptr) {
            SelectPalette(WinSharedHdc, PaletteInitialized, FALSE);
            RealizePalette(WinSharedHdc);
            Win::release_hdc();
        }
    }

    if (area == nullptr) {
        BitBlt(WinSharedHdc, 0, 0, ScreenBuffer.dib_.bmiHeader.biWidth,
               -ScreenBuffer.dib_.bmiHeader.biHeight, source, 0, 0, SRCCOPY);
    } else {
        BitBlt(WinSharedHdc, area->left, area->top,
               area->right - area->left, area->bottom - area->top,
               source, area->left, area->top, SRCCOPY);
    }

    if (--WinHdcRefCount == 0) {
        if (DirectDrawSurface != nullptr) {
            DirectDrawSurface->ReleaseDC(WinSharedHdc);
        } else {
            ReleaseDC(HandleMain, WinSharedHdc);
        }
        WinSharedHdc = nullptr;
    }
}


/*
Purpose: Dismiss any pending bubble text and repaint the area it covered.
// ORIGINAL: 0x005F8500 ?clear_bubble_text@Win@@QAGXXZ 0x005F8500-0x005F852F
// symbol    ?clear_bubble_text@Win@@SAXXZ
// size      47 bytes
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
// ORIGINAL: 0x005EC680 ?UNK1@Win@@QAEHHHHHHHHHH@Z 0x005EC680-0x005EC685 BYTE_EXACT
// size      5 bytes
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
// ORIGINAL: 0x005ED7C0 ?UNK5@Win@@QAEHXZ 0x005ED7C0-0x005ED7C3 BYTE_EXACT
// size      3 bytes
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
// ORIGINAL: 0x005EDFE0 ?UNK6@Win@@QAEHH@Z 0x005EDFE0-0x005EDFE5 BYTE_EXACT
// size      5 bytes
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
// ORIGINAL: 0x005F6A30 ?on_set_cursor@Win@@QAEHPAXII@Z 0x005F6A30-0x005F6A38 BYTE_EXACT
// size      8 bytes
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


/*
Purpose: Record the window that receives focus by default.
// ORIGINAL: 0x005F2CE0 ?set_def_focus@Win@@QAAXPAUWin@@@Z 0x005F2CE0-0x005F2CEA BYTE_EXACT
// symbol    ?set_def_focus@Win@@SAXH@Z
// size      10 bytes
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
// ORIGINAL: 0x005EE130 ?UNK8@Win@@QAEXH@Z 0x005EE130-0x005EE157 BYTE_EXACT
// size      39 bytes
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
// ORIGINAL: 0x005EE160 ?UNK9@Win@@QAEXH@Z 0x005EE160-0x005EE187 BYTE_EXACT
// size      39 bytes
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
// ORIGINAL: 0x005EE020 ?reset_window_clip@Win@@QAEXXZ 0x005EE020-0x005EE021 BYTE_EXACT
// size      1 bytes
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
// ORIGINAL: 0x005F54B0 ?on_move@Win@@QAEXHH@Z 0x005F54B0-0x005F54B3 BYTE_EXACT
// size      3 bytes
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
// ORIGINAL: 0x005F54C0 ?on_size@Win@@QAEXIHH@Z 0x005F54C0-0x005F54C3 BYTE_EXACT
// size      3 bytes
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
// ORIGINAL: 0x005F54D0 ?on_size_nc@Win@@QAEXIHH@Z 0x005F54D0-0x005F54D3 BYTE_EXACT
// size      3 bytes
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
// ORIGINAL: 0x005F6A40 ?on_sys_command@Win@@QAEHIHH@Z 0x005F6A40-0x005F6A43 BYTE_EXACT
// size      3 bytes
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

/*
Purpose: Bring this window's palette into step with the active one, but only
         when it has fallen behind - set_active_window is skipped when the
         cached generation at 0x184 already matches the palette's at 0x400.
// ORIGINAL: 0x005F2C60 ?sync_palette@Win@@QAEXXZ 0x005F2C60-0x005F2C94
// size      52 bytes
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
// ORIGINAL: 0x005EE030 ?set_vert_pos@Win@@QAEXH@Z 0x005EE030-0x005EE047 BYTE_EXACT
// size      23 bytes
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
// ORIGINAL: 0x005EE070 ?set_horz_pos@Win@@QAEXH@Z 0x005EE070-0x005EE087 BYTE_EXACT
// size      23 bytes
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
// ORIGINAL: 0x005EE0B0 ?set_vert_range@Win@@QAEXHH@Z 0x005EE0B0-0x005EE0CC BYTE_EXACT
// size      28 bytes
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
// ORIGINAL: 0x005EE0D0 ?set_horz_range@Win@@QAEXHH@Z 0x005EE0D0-0x005EE0EC BYTE_EXACT
// size      28 bytes
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
// ORIGINAL: 0x005F2670 ?OnSetCursor@Win@@QAAHPAXPAXII@Z 0x005F2670-0x005F2676 BYTE_EXACT
// symbol    ?OnSetCursor@Win@@SAHPAX0II@Z
// size      6 bytes
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
// ORIGINAL: 0x005ECE80 ?UNK3@Win@@QAEHH@Z 0x005ECE80-0x005ECEB6
// size      54 bytes
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
// ORIGINAL: 0x005ECE20 ?is_descendant@Win@@QAEHPAVWin@@@Z 0x005ECE20-0x005ECE73
// symbol    ?is_descendant@Win@@QAEHPAV1@@Z
// size      83 bytes
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
// ORIGINAL: 0x005F8530 ?on_mousewheel_down_vert@Win@@QAEXH@Z 0x005F8530-0x005F8547 BYTE_EXACT
// size      23 bytes
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
// ORIGINAL: 0x005F8590 ?on_mousewheel_up_horz@Win@@QAEXH@Z 0x005F8590-0x005F85A7 BYTE_EXACT
// size      23 bytes
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
// ORIGINAL: 0x005EC960 ?get_lbutton_state@Win@@QAEHXZ 0x005EC960-0x005EC97B BYTE_EXACT
// size      27 bytes
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

int Win::get_lbutton_state() {
    return GetSystemMetrics(0x17) ^ (GetAsyncKeyState(1) >> 15);
}

/*
// ORIGINAL: 0x005F8550 ?on_mousewheel_down_horz@Win@@QAEXH@Z 0x005F8550-0x005F8567 BYTE_EXACT
// size      23 bytes
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
// ORIGINAL: 0x005F8570 ?on_mousewheel_up_vert@Win@@QAEXH@Z 0x005F8570-0x005F8587 BYTE_EXACT
// size      23 bytes
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


/*
Purpose: Route a left button press - find the window under the pointer, ask
         it what was hit, raise it, and either begin a drag or dispatch the
         click.
// ORIGINAL: 0x005F2330 ?OnLButtonDown@Win@@QAAXPAXJHHI@Z 0x005F2330-0x005F256B
// symbol    ?OnLButtonDown@Win@@SAXPAUHWND__@@JHHI@Z
// size      571 bytes
// prototype
// callers   2   call targets   3
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005F4EC0 0x005F6F10
//
// `QAA`, so no receiver, and it passes `&x`/`&y` - the addresses of its OWN
// parameters - to `get_mouse_window`, which rewrites them into the found
// window's coordinates. Everything after that uses the rewritten pair.
//
// The tracking codes are 2 and 10 through 17. `hit` is whatever the window's
// slot 0x12C returns, and those values start a drag instead of a click: the
// four contiguous globals at 0x009B7AA8 are the whole of the drag state, and
// `Win::do_tracking` is the only thing that reads them back.
Status: WIP
*/
void Win::OnLButtonDown(HWND window, LONG dbl, int x, int y, WPARAM keys) {
    static const size_t WinSlotHitTest = 0x12C;    // slot 75
    static const size_t WinSlotLeftDown = 0x144;   // slot 81
    typedef int (OriginalObject::*func_win_hit_test)(int, int);
    typedef void (OriginalObject::*func_win_left_down)(LONG, int, int,
                                                       WPARAM, int);

    Win *const target = get_mouse_window(&x, &y);
    if (target != nullptr) {
        if (WinMouseDirect != 0
            || (target->iFlags_ & WinFlagHitTestIndirect) != 0) {
            const int hit = (ORIGINAL(target)->*original_slot<func_win_hit_test>(
                *reinterpret_cast<uint8_t **>(target) + WinSlotHitTest))(x, y);
            if (hit != 0) {
                // WRITTEN OUT TWICE, as the original has it: the
                // parent's child list and the root list are the same
                // shuffle over different arrays, and factoring them
                // together costs 158 bytes of the 571.
                Win *const parent = target->win_parent_;
                if (parent != nullptr) {
                    if ((target->iFlags_ & WinFlagNoRaise) == 0) {
                        int index = 0;
                        while (index < parent->child_count_
                               && parent->children_[index] != target) {
                            ++index;
                        }
                        if (index < parent->child_count_) {
                            while (index > 0) {
                                parent->children_[index] = parent->children_[index - 1];
                                --index;
                            }
                            parent->children_[0] = target;
                        }
                        WinZOrderCount = 0;
                        for (int root = 0; root < WinRootCount; ++root) {
                            if (WinZOrderWindow != nullptr
                                && WinZOrderWindow == WinRootWindows[root]) {
                                WinZOrderCount = 0;
                                WinZOrderFlag = 0;
                            }
                            if ((WinRootWindows[root]->iSomeFlag_
                                 & WinFlagVisible) != 0) {
                                recurse_zorder(WinRootWindows[root]);
                            }
                        }
                    }
                } else {
                    if ((target->iFlags_ & WinFlagNoRaise) == 0) {
                        int index = 0;
                        while (index < WinRootCount
                               && WinRootWindows[index] != target) {
                            ++index;
                        }
                        if (index < WinRootCount) {
                            while (index > 0) {
                                WinRootWindows[index] = WinRootWindows[index - 1];
                                --index;
                            }
                            WinRootWindows[0] = target;
                        }
                        WinZOrderCount = 0;
                        for (int root = 0; root < WinRootCount; ++root) {
                            if (WinZOrderWindow != nullptr
                                && WinZOrderWindow == WinRootWindows[root]) {
                                WinZOrderCount = 0;
                                WinZOrderFlag = 0;
                            }
                            if ((WinRootWindows[root]->iSomeFlag_
                                 & WinFlagVisible) != 0) {
                                recurse_zorder(WinRootWindows[root]);
                            }
                        }
                    }
                }
                if (hit == 2 || (hit > 9 && hit <= 0x11)) {
                    WinTrackingWindow = target;
                    WinTrackingMode = hit;
                    WinTrackingX = x;
                    WinTrackingY = y;
                    return;
                }
            }
        }
        uint8_t *const vtable = *reinterpret_cast<uint8_t **>(target);
        (ORIGINAL(target)->*original_slot<func_win_left_down>(
            vtable + WinSlotLeftDown))(dbl, x, y, keys, WinMouseDirect);
    }
    if (WinLeftDownHook != nullptr) {
        WinLeftDownHook(x, y);
    }
    if (WinMessageHook != nullptr) {
        WinMessageHook();
    }
}

/*
Purpose: Decide which window a screen position belongs to, translating the
         position into that window's coordinates on the way.
// ORIGINAL: 0x005F6F10 ?get_mouse_window@Win@@QAAHPAH0@Z 0x005F6F10-0x005F7319
// symbol    ?get_mouse_window@Win@@SAPAV1@PAH0@Z
// size      1033 bytes
// prototype
// callers   8   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005ED2D0 0x005F6AB0
//
// CORRECTED from `int`, like `get_key_window` beside it: the catalogue says
// `H` and every caller treats the result as a `Win *`.
//
// The four `WinPointerOwner` globals come first, in that order, and the
// FIRST that is visible wins; one that is not is nulled on the way past.
// Owners 2 and 4 set `WinMouseDirect`, and that is what the tail reads to
// decide between returning the window and returning its `poWinBase_`.
Status: WIP
*/
Win *Win::get_mouse_window(int *x, int *y) {
    WinMouseScreenX = *x;
    WinMouseScreenY = *y;

    Win *found = nullptr;
    for (;;) {
        if (WinPointerOwner1 != nullptr) {
            if ((WinPointerOwner1->iSomeFlag_ & WinFlagVisible) != 0) {
                *x -= WinPointerOwner1->client_rect_.left + WinPointerOwner1->outer_rect_.left;
                *y -= WinPointerOwner1->client_rect_.top + WinPointerOwner1->outer_rect_.top;
                if ((WinPointerOwner1->iFlags_ & WinFlagClipToParent) != 0
                    && WinPointerOwner1->win_parent_ != nullptr) {
                    WinPointerOwner1->win_parent_->screen_to_client(x, y);
                    if ((WinPointerOwner1->iFlags_ & WinFlagParentOffset) != 0) {
                        *x += WinPointerOwner1->win_parent_->outer_rect_.left;
                        *y += WinPointerOwner1->win_parent_->outer_rect_.top;
                    }
                }
                WinMouseDirect = 0;
                found = WinPointerOwner1;
                break;
            }
            WinPointerOwner1 = nullptr;
            continue;
        }
        if (WinPointerOwner2 != nullptr) {
            if ((WinPointerOwner2->iSomeFlag_ & WinFlagVisible) != 0) {
                WinPointerOwner2->screen_to_client(x, y);
                *x += WinPointerOwner2->outer_rect_.left;
                *y += WinPointerOwner2->outer_rect_.top;
                WinMouseDirect = 1;
                found = WinPointerOwner2;
                break;
            }
            WinPointerOwner2 = nullptr;
            continue;
        }
        if (WinPointerOwner3 != nullptr) {
            if ((WinPointerOwner3->iSomeFlag_ & WinFlagVisible) != 0) {
                *x -= WinPointerOwner3->client_rect_.left + WinPointerOwner3->outer_rect_.left;
                *y -= WinPointerOwner3->client_rect_.top + WinPointerOwner3->outer_rect_.top;
                if ((WinPointerOwner3->iFlags_ & WinFlagClipToParent) != 0
                    && WinPointerOwner3->win_parent_ != nullptr) {
                    WinPointerOwner3->win_parent_->screen_to_client(x, y);
                    if ((WinPointerOwner3->iFlags_ & WinFlagParentOffset) != 0) {
                        *x += WinPointerOwner3->win_parent_->outer_rect_.left;
                        *y += WinPointerOwner3->win_parent_->outer_rect_.top;
                    }
                }
                WinMouseDirect = 0;
                found = WinPointerOwner3;
                break;
            }
            WinPointerOwner3 = nullptr;
            continue;
        }
        if (WinPointerOwner4 != nullptr) {
            if ((WinPointerOwner4->iSomeFlag_ & WinFlagVisible) != 0) {
                WinPointerOwner4->screen_to_client(x, y);
                *x += WinPointerOwner4->outer_rect_.left;
                *y += WinPointerOwner4->outer_rect_.top;
                WinMouseDirect = 1;
                found = WinPointerOwner4;
                break;
            }
            WinPointerOwner4 = nullptr;
            continue;
        }
        break;
    }

    if (found == nullptr) {
        if (WinFocusWindow != nullptr) {
            *x -= WinFocusWindow->client_rect_.left + WinFocusWindow->outer_rect_.left;
            *y -= WinFocusWindow->client_rect_.top + WinFocusWindow->outer_rect_.top;
            if ((WinFocusWindow->iFlags_ & WinFlagClipToParent) != 0
                && WinFocusWindow->win_parent_ != nullptr) {
                WinFocusWindow->win_parent_->screen_to_client(x, y);
                if ((WinFocusWindow->iFlags_ & WinFlagParentOffset) != 0) {
                    *x += WinFocusWindow->win_parent_->outer_rect_.left;
                    *y += WinFocusWindow->win_parent_->outer_rect_.top;
                }
            }
            *x += WinFocusWindow->outer_rect_.left;
            *y += WinFocusWindow->outer_rect_.top;
            found = get_mouse_window_recurse(WinFocusWindow, x, y);
            if (found == nullptr && WinModalWindow != nullptr) {
                *x = WinMouseScreenX;
                *y = WinMouseScreenY;
                WinModalWindow->screen_to_client(x, y);
                *x += WinModalWindow->outer_rect_.left;
                *y += WinModalWindow->outer_rect_.top;
                found = get_mouse_window_recurse(WinModalWindow, x, y);
            }
            if (found == nullptr) {
                // Neither subtree claimed it, so the focus window keeps it
                // and the position goes back to where it started.
                *x = WinMouseScreenX;
                *y = WinMouseScreenY;
                *x -= WinFocusWindow->client_rect_.left + WinFocusWindow->outer_rect_.left;
                *y -= WinFocusWindow->client_rect_.top + WinFocusWindow->outer_rect_.top;
                if ((WinFocusWindow->iFlags_ & WinFlagClipToParent) != 0
                    && WinFocusWindow->win_parent_ != nullptr) {
                    WinFocusWindow->win_parent_->screen_to_client(x, y);
                    if ((WinFocusWindow->iFlags_ & WinFlagParentOffset) != 0) {
                        *x += WinFocusWindow->win_parent_->outer_rect_.left;
                        *y += WinFocusWindow->win_parent_->outer_rect_.top;
                    }
                }
                WinMouseDirect = 0;
                found = WinFocusWindow;
            }
        } else {
            for (int index = 0; index < WinRootCount; ++index) {
                found = get_mouse_window_recurse(WinRootWindows[index], x, y);
                if (found != nullptr) {
                    break;
                }
            }
        }
    }

    if (found != nullptr && WinMouseDirect == 0) {
        found = found->poWinBase_;
    }
    return found;
}

/*
Purpose: The window procedure the class registers - route every input
         message to the Win the pointer or the keyboard focus is over, and
         hand everything else to DefWindowProc.
// ORIGINAL: 0x005F0650 ?window_proc@Win@@QAGJPAXIIJ@Z 0x005F0650-0x005F0E27
// symbol    ?window_proc@Win@@SGJPAUHWND__@@IIJ@Z
// size      2007 bytes
// prototype
// callers   0   call targets   11
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005EC690 0x005EC6F0 0x005EFD20 0x005F1820 0x005F2330 0x005F6A50
//           0x005F6F10 0x005F7320 0x005F7580 0x005F86A0 0x00616730
//
// CORRECTED from a NON-STATIC member. The catalogued name ends in `QAG` - a
// public member declared __stdcall, which would take a receiver and clean
// five slots - and the body ends in `ret 0x10`. Four arguments, no `this`,
// which is what a WNDPROC is; every one of its `Win::` callees is `QAA` or
// `QAG` too, and the one that is not - `do_tracking`, `QAE` - is called on
// `WinTrackingWindow` rather than on a receiver of its own. Same shape as
// `init_class` and `flip` beside it. See the QAA note on `set_display_mode`.
//
// Promoted out of src/unrecovered/005f0650.cpp, where it was a scaffold
// working copy dispatching through `VCall` slot shims and reading its
// imports out of IAT addresses. In the build it dispatches the way
// `Sound::fade` does - `original_slot` off the object's own vtable, which
// cannot disagree with the original's layout because it IS the original's
// layout - and calls BeginPaint, EndPaint, InvalidateRect, SelectPalette,
// RealizePalette, GetWindowLongA, GetDC and DefWindowProcA by name. All
// eight were checked against the import directory rather than inferred from
// argument counts.
Status: WIP
*/
// VC6's winuser.h hides `WM_MOUSEWHEEL` behind
// `_WIN32_WINNT >= 0x0400`, which this build does not set - and raising it
// would change what every other translation unit sees for one constant. The
// image handles 0x020A whatever the SDK guard says.
#ifndef WM_MOUSEWHEEL
#define WM_MOUSEWHEEL 0x020A
#endif

LRESULT __stdcall Win::window_proc(HWND window, UINT message, WPARAM wparam,
                                   LPARAM lparam) {
    // WIN'S OWN VTABLE, by the offsets the original encodes and named for
    // what each call does with them. Spelled the way `Sound::fade` spells
    // it - `original_slot` off the object's own vtable - because `Win` is
    // not polymorphic in this tree: `Win::construct` writes
    // `object[0] = WinPrimaryVtable` by hand, over the pointer
    // `auto_sound_.construct()` just put there, which is base-then-derived
    // vtable assignment done manually. Declaring these `virtual` needs
    // `Win : public AutoSound` first, and that is a change to the object
    // model rather than to this function.
    //
    // NOT `src/vtable_shim.h`'s `VCall`, which is the tree's other spelling
    // for this. That shim declares every slot nullary and CANNOT be typed:
    // `slot016` is called with no arguments in `fx.cpp`, with one in
    // `src/recovered/units/00491380.cpp`, and with four here, because slot
    // 16 of a PrefWin is not slot 16 of a Win. One class-agnostic shim can
    // never carry a signature.
    static const size_t WinSlotSysClose = 0x38;     // slot 14
    static const size_t WinSlotMouseEnter = 0x40;   // slot 16
    static const size_t WinSlotMouseLeave = 0x48;   // slot 18
    static const size_t WinSlotMouseWheel = 0x134;  // slot 77
    static const size_t WinSlotKey = 0x138;         // slot 78
    static const size_t WinSlotChar = 0x13C;        // slot 79
    static const size_t WinSlotSysKey = 0x140;      // slot 80
    static const size_t WinSlotLButtonUp = 0x148;   // slot 82
    static const size_t WinSlotRButtonDown = 0x14C; // slot 83
    static const size_t WinSlotRButtonUp = 0x150;   // slot 84

    typedef void (OriginalObject::*func_win_slot_38)();
    typedef void (OriginalObject::*func_win_enter)(int, int, WPARAM, int);
    typedef void (OriginalObject::*func_win_leave)(int, int);
    typedef void (OriginalObject::*func_win_wheel)(WPARAM, unsigned int,
                                                   int, int, int);
    typedef void (OriginalObject::*func_win_key)(WPARAM, int, int, int);
    typedef void (OriginalObject::*func_win_char)(WPARAM, int);
    typedef void (OriginalObject::*func_win_button_up)(int, int, WPARAM, int);
    typedef void (OriginalObject::*func_win_button_down)(int, int, int,
                                                         WPARAM, int);

    // THE ORIGINAL'S OWN ORDER: `cmp 0x201/ja`, `cmp 0x201/je`,
    // `cmp 0x102/ja`, `cmp 0x102/je`, then the jump table for
    // [6, 0x101]. Written as `message < WM_LBUTTONUP` the first test
    // lowers to `jae` and the second comparison against 0x201 folds
    // away, which is four instructions the image has and the rebuild
    // did not.
    if (message > WM_LBUTTONDOWN) {
        if (message > WM_MOUSEWHEEL) {
            if (message == WM_QUERYNEWPALETTE) {
                if (BufferDirectDraw != 0) {
                    return 1;
                }
                if (WinHdcRefCount == 0) {
                    if (DirectDrawSurface == nullptr) {
                        WinSharedHdc = GetDC(HandleMain);
                    } else {
                        DirectDrawSurface->GetDC(&WinSharedHdc);
                    }
                    if (WinSharedHdc == nullptr) {
                        return 0;
                    }
                    WinHdcRefCount = 1;
                } else {
                    ++WinHdcRefCount;
                }
                if (WinSharedHdc != nullptr) {
                    SelectPalette(WinSharedHdc, PaletteInitialized, FALSE);
                    RealizePalette(WinSharedHdc);
                    release_hdc();
                }
                return 0;
            }
            if (message == WM_PALETTECHANGED) {
                if (window == reinterpret_cast<HWND>(wparam)) {
                    return 0;
                }
                if (BufferDirectDraw != 0) {
                    return 0;
                }
                if (get_hdc() == nullptr) {
                    return 0;
                }
                SelectPalette(WinSharedHdc, PaletteInitialized, FALSE);
                RealizePalette(WinSharedHdc);
                release_hdc();
                return 0;
            }
            if (message == WM_USER + 1) {
                // The tick a `Time` posts to this window, `wparam` being the
                // Time itself. Every offset the original touches is a member
                // `time.h` already declares, and the layout is pinned by its
                // own `static_assert(sizeof(Time) == 0x28)`:
                //
                //   [esi + 0x1C]  tick_posted_     cleared on every tick
                //   [esi]         oneshot_state_   `test al,1`, `test al,2`
                //   [esi + 0x08]  callback1_       void(int)
                //   [esi + 0x0C]  callback2_       void(int, int)
                //   [esi + 0x14]  cb_param1_       pushed last, so it is first
                //   [esi + 0x10]  cb_param2_
                //
                // This is the half of the one-shot emulation that lives
                // outside `Time`: `SetTimer` has no one-shot mode, so a
                // `pulse()` that went through it arrives here repeating,
                // and the second tick is what stops it.
                Time *const timer = reinterpret_cast<Time *>(wparam);
                timer->tick_posted_ = 0;
                if (MsgStatus == 0) {
                    if ((timer->oneshot_state_ & TimeOneShot) != 0) {
                        if ((timer->oneshot_state_ & TimeOneShotFired) != 0) {
                            timer->stop();
                            return DefWindowProcA(window, message, wparam,
                                                  lparam);
                        }
                        timer->oneshot_state_ |= TimeOneShotFired;
                    }
                    if (Time::TimeModal == nullptr
                        || timer == Time::TimeModal) {
                        if (timer->callback1_ != nullptr) {
                            timer->callback1_(timer->cb_param1_);
                        }
                        if (timer->callback2_ != nullptr) {
                            timer->callback2_(timer->cb_param1_,
                                              timer->cb_param2_);
                        }
                    }
                }
            }
            return DefWindowProcA(window, message, wparam, lparam);
        }
        if (message == WM_MOUSEWHEEL) {
            int x = static_cast<short>(LOWORD(lparam));
            int y = static_cast<short>(HIWORD(lparam));
            Win *const over =
                get_mouse_window(&x, &y);
            if (over == nullptr) {
                return 0;
            }
            uint8_t *const vtable = *reinterpret_cast<uint8_t **>(over);
            (ORIGINAL(over)->*original_slot<func_win_wheel>(
                vtable + WinSlotMouseWheel))(wparam, static_cast<unsigned int>(wparam) >> 16,
                                 x, y, WinMouseDirect);
            return 0;
        }
        switch (message) {
        case WM_LBUTTONUP: {
            WinTrackingWindow = nullptr;
            int x = static_cast<short>(LOWORD(lparam));
            int y = static_cast<short>(HIWORD(lparam));
            Win *const over =
                get_mouse_window(&x, &y);
            if (over == nullptr) {
                return 0;
            }
            uint8_t *const vtable = *reinterpret_cast<uint8_t **>(over);
            (ORIGINAL(over)->*original_slot<func_win_button_up>(
                vtable + WinSlotLButtonUp))(x, y, wparam, WinMouseDirect);
            return 0;
        }
        case WM_LBUTTONDBLCLK:
            OnLButtonDown(window, 1, static_cast<short>(LOWORD(lparam)), static_cast<short>(HIWORD(lparam)), wparam);
            return 0;
        case WM_RBUTTONDOWN:
        case WM_RBUTTONDBLCLK: {
            int x = static_cast<short>(LOWORD(lparam));
            int y = static_cast<short>(HIWORD(lparam));
            Win *const over =
                get_mouse_window(&x, &y);
            if (over != nullptr) {
                uint8_t *const vtable = *reinterpret_cast<uint8_t **>(over);
                (ORIGINAL(over)->*original_slot<func_win_button_down>(
                    vtable + WinSlotRButtonDown))(message == WM_RBUTTONDBLCLK ? 1 : 0,
                                     x, y, wparam, WinMouseDirect);
            }
            break;
        }
        case WM_RBUTTONUP: {
            WinTrackingWindow = nullptr;
            int x = static_cast<short>(LOWORD(lparam));
            int y = static_cast<short>(HIWORD(lparam));
            Win *const over =
                get_mouse_window(&x, &y);
            if (over != nullptr) {
                uint8_t *const vtable = *reinterpret_cast<uint8_t **>(over);
                (ORIGINAL(over)->*original_slot<func_win_button_up>(
                    vtable + WinSlotRButtonUp))(x, y, wparam, WinMouseDirect);
            }
            break;
        }
        default:
            return DefWindowProcA(window, message, wparam, lparam);
        }
        if (WinMouseHook != nullptr) {
            WinMouseHook(window, lparam);
        }
    } else if (message == WM_LBUTTONDOWN) {
        OnLButtonDown(window, 0, static_cast<short>(LOWORD(lparam)),
                      static_cast<short>(HIWORD(lparam)), wparam);
        return 0;
    } else if (message > WM_CHAR) {
            switch (message) {
            case WM_SYSKEYDOWN:
            case WM_SYSKEYUP: {
                Win *const focus =
                    get_key_window();
                if (focus == nullptr) {
                    return 0;
                }
                uint8_t *const vtable = *reinterpret_cast<uint8_t **>(focus);
                (ORIGINAL(focus)->*original_slot<func_win_key>(
                    vtable + WinSlotSysKey))(wparam,
                                     message == WM_SYSKEYDOWN ? 1 : 0,
                                     static_cast<short>(LOWORD(lparam)),
                                     static_cast<int>(lparam >> 16));
                return 0;
            }
            case WM_SYSCOMMAND: {
                Win *const owner = reinterpret_cast<Win *>(
                    GetWindowLongA(window, GWL_USERDATA));
                if (owner == nullptr) {
                    return 0;
                }
                if (wparam != SC_CLOSE) {
                    DefWindowProcA(window, WM_SYSCOMMAND, wparam, lparam);
                    return 0;
                }
                *ScrollCurrentWin = owner;
                {
                    typedef void(__cdecl * func_win_closed)();
                    func_win_closed closed = *reinterpret_cast<func_win_closed *>(
                        reinterpret_cast<char *>(owner) + 0x404);
                    if (closed != nullptr) {
                        closed();
                    }
                }
                {
                    uint8_t *const vtable =
                        *reinterpret_cast<uint8_t **>(owner);
                    (ORIGINAL(owner)->*original_slot<func_win_slot_38>(
                        vtable + WinSlotSysClose))();
                }
                sub_5f86a0(*reinterpret_cast<int *>(
                    reinterpret_cast<char *>(owner) + 0x18));
                return 0;
            }
            case WM_MOUSEMOVE: {
                WinCursorMoved = 0;
                if (WinTrackingWindow != nullptr) {
                    WinTrackingWindow->do_tracking(static_cast<short>(LOWORD(lparam)),
                                                   static_cast<short>(HIWORD(lparam)));
                    return 0;
                }
                int x = static_cast<short>(LOWORD(lparam));
                int y = static_cast<short>(HIWORD(lparam));
                Win *const over =
                    get_mouse_window(&x, &y);
                update_cursor(over, 1);
                if (over == nullptr) {
                    return 0;
                }
                if (WinHoverWindow != nullptr
                    && (WinCursorMoved != 0 || over != WinHoverWindow)
                    && (WinPointerOwner3 == nullptr
                        || WinPointerOwner3 == WinHoverWindow)) {
                    uint8_t *const vtable =
                        *reinterpret_cast<uint8_t **>(WinHoverWindow);
                    (ORIGINAL(WinHoverWindow)->*original_slot<func_win_leave>(
                        vtable + WinSlotMouseLeave))(x, y);
                }
                WinHoverWindow = over;
                uint8_t *const vtable = *reinterpret_cast<uint8_t **>(over);
                (ORIGINAL(over)->*original_slot<func_win_enter>(
                    vtable + WinSlotMouseEnter))(x, y, wparam, WinMouseDirect);
                return 0;
            }
            default:
                break;
            }
            return DefWindowProcA(window, message, wparam, lparam);
    } else if (message == WM_CHAR) {
        Win *const focus = get_key_window();
        if (focus == nullptr) {
            return 0;
        }
        uint8_t *const vtable = *reinterpret_cast<uint8_t **>(focus);
        (ORIGINAL(focus)->*original_slot<func_win_char>(
            vtable + WinSlotChar))(wparam, static_cast<short>(LOWORD(lparam)));
        return 0;
    } else {
        switch (message) {
        case WM_ACTIVATE: {
            const unsigned int active = LOWORD(wparam);
            const unsigned int minimised = HIWORD(wparam);
            InvalidateRect(HandleMain, nullptr, FALSE);
            if (active == 0) {
                WinPointerOwner1 = nullptr;
                WinPointerOwner2 = nullptr;
            } else if (minimised == 0 && BufferDirectDraw == 0
                       && get_hdc() != nullptr) {
                SelectPalette(WinSharedHdc, PaletteInitialized, FALSE);
                RealizePalette(WinSharedHdc);
                release_hdc();
            }
            DefWindowProcA(window, WM_ACTIVATE,
                           (minimised << 16) | active, lparam);
            return 0;
        }
        case WM_PAINT: {
            PAINTSTRUCT paint;
            if (BeginPaint(window, &paint) == nullptr) {
                return 0;
            }
            RECT damaged = paint.rcPaint;
            update_screen(&damaged, nullptr);
            flip(&damaged);
            EndPaint(window, &paint);
            return 0;
        }
        case WM_KEYDOWN:
        case WM_KEYUP: {
            Win *const focus = get_key_window();
            if (focus != nullptr) {
                uint8_t *const vtable = *reinterpret_cast<uint8_t **>(focus);
                (ORIGINAL(focus)->*original_slot<func_win_key>(
                    vtable + WinSlotKey))(wparam,
                                     message == WM_KEYDOWN ? 1 : 0,
                                     static_cast<short>(LOWORD(lparam)),
                                     static_cast<int>(lparam >> 16));
            }
            if (message == WM_KEYDOWN && WinKeyHook != nullptr) {
                WinKeyHook(wparam);
            }
            break;
        }
        default:
            // EVERY OTHER MESSAGE IN THIS RANGE GOES TO DefWindowProc, and
            // returning 0 instead is not a near-miss: the jump table at
            // 0x005F0E28 has five arms and 248 of the 252 messages in
            // [6, 0x101] take arm 4, which is 0x005F0D11 -
            // `DefWindowProcA` with its result returned. WM_NCCREATE is one
            // of those, and a window procedure that answers 0 to WM_NCCREATE
            // makes CreateWindowEx return NULL. So does WM_CREATE, and both
            // arrive here before CreateWindowEx has returned at all.
            return DefWindowProcA(window, message, wparam, lparam);
        }
    }
    if (WinMessageHook != nullptr) {
        WinMessageHook();
    }
    return 0;
}

// `JackalClass` is at 0x00696DC8 and again at 0x00696DD4 - the image holds
// two copies, one for the registration and one for the creation, and the
// operand is relocated either way, so the literal is the honest spelling.
static const char WinClassName[] = "JackalClass";

/*
// ORIGINAL: 0x005F01F0 ?init_class@Win@@QAAHPAD@Z 0x005F01F0-0x005F04D4;0x00662CCC-0x00662CE1
// symbol    ?init_class@Win@@SAHPAD@Z
// size      761 bytes
// prototype
// callers   1   call targets   7
// kind      game
// flags     sp_ready;purged_ok
// calls     0x005D7210 0x005D7410 0x005D7670 0x005D7DE0 0x005DFB50 0x005DFF00 0x005EFD20
// indirect  0x005F0236 0x005F0263 0x005F0273 0x005F028E 0x005F02D0 0x005F02D9 0x005F030A
//           0x005F036F 0x005F0375 0x005F039A 0x005F03A9 0x005F03BD 0x005F03E3 0x005F03F5
//           0x005F0498
//
// Promoted 2026-08-15 from src/unrecovered/005f01f0.cpp to retire its
// pending_bodies forwarder.
//
// WRITTEN AGAINST THE REAL HEADERS. It used to reach every Win32 import
// through a hand-written function-pointer typedef and its IAT slot address
// - `(*(FnRegisterClassA *)g_0066929c)(&wndclass)` - and every global
// through `static int *const g_009b7b14 = (int *)0x009B7B14`. That is what
// a MEASUREMENT SCAFFOLD has to do, because a scaffold includes no
// <windows.h> and declares no project header; this file is in the build and
// has both. So the imports are called by name, `WNDCLASSA` is the real
// struct, the surface is an `IDirectDrawSurface` whose `GetDC`/`ReleaseDC`
// are the slots the offsets 0x44 and 0x68 were reaching, and the globals
// are objects declared in win.h. Every one of those operands is relocated
// and the comparison masks relocations, so none of it costs a byte.
//
// LEFT AT 88.7%, first divergence #7, and the divergence is a PROLOGUE
// PUSH: the image saves esi and edi up front, this build saves only esi
// there.
//
// NOT "one register short", which is what this note used to say. Both
// builds use two callee-saved registers and both cache the
// `GetSystemMetrics` import slot - `call edi` twice here, `call esi` twice
// there. What differs is WHICH register holds the zero. The image keeps it
// in edi from function entry, so edi is live before the `return 1` that
// `RegisterClassA` failing takes and has to be saved in the prologue. This
// build keeps the zero in esi and gives edi to the import slot, whose live
// range starts AFTER that early return - so VC6 shrink-wraps it and emits
// `push edi` at the point of first use, thirty instructions in. Every
// instruction after the prologue is then four bytes off in the frame, which
// is why positional agreement collapses to 12 of 203 while similarity is
// 0.887.
//
// So the question is not how to spend a second register. It is how to make
// VC6 assign the constant zero to the register it saves first, and that is
// allocation order, which no spelling tried here reaches.
//
// RULED OUT: the flag sets are byte-identical here, so it is not the frame
// pointer or /O1 register pressure; declaring `wndclass` before `logo` does
// not move the frame; and caching the import slot in a local changes
// nothing, because this build already caches it.
//
// FIXED: the four `WinModalStack` stores now precede `GetModuleHandleA`,
// which is where the image has them - it stores all four from a zeroed eax
// and only then makes the call. Worth 0.882 -> 0.887, and it is what the
// image does regardless of what it is worth.
Status: Complete
*/
int __cdecl Win::init_class(LPSTR window_name) {
    WNDCLASSA wndclass;

    // The splash buffer. Its destructor is what puts this function in an EH
    // frame: `push -1` at entry is the try level, and the `mov [level], 0`
    // after the constructor is VC6 saying the object is now live.
    Buffer logo;

    // A MEMSET, NOT FOUR STORES, on the evidence of the register the zero
    // lands in. The image materialises TWO zeros - `xor eax, eax` for these
    // four global stores and `xor edi, edi` for everything after the
    // GetModuleHandleA call that clobbers eax. Four separate assignments make
    // VC6 reuse the one callee-saved zero it already has, and the whole body
    // shifts; this reproduces the pair. 12 of 203 instructions to 16.
    memset(WinModalStack, 0, sizeof(WinModalStack));
    WinInstance = GetModuleHandleA(nullptr);

    wndclass.style = CS_VREDRAW | CS_HREDRAW | CS_DBLCLKS | CS_OWNDC;  // 0x2B
    wndclass.lpfnWndProc = &Win::window_proc;
    wndclass.cbClsExtra = 0;
    wndclass.cbWndExtra = 0;
    wndclass.hInstance = WinInstance;
    wndclass.hIcon = LoadIconA(nullptr, IDI_APPLICATION);
    wndclass.hCursor = nullptr;
    wndclass.hbrBackground = static_cast<HBRUSH>(GetStockObject(BLACK_BRUSH));
    wndclass.lpszMenuName = nullptr;
    wndclass.lpszClassName = WinClassName;

    // TWO COPIES OF ONE STRING, which is why both calls can be handed
    // `WinClassName`. The image passes 0x696DC8 here and 0x696DD4 to
    // CreateWindowExA, and both hold "JackalClass" - VC6 pooled neither, so
    // the literal appears twice. Read out of the image rather than assumed:
    // the operands are relocations, so the byte comparison masks them and
    // could not have told the difference either way.
    if (RegisterClassA(&wndclass) == 0) {
        return 1;
    }

    WinScreenWidth = GetSystemMetrics(SM_CXSCREEN);
    WinScreenHeight = GetSystemMetrics(SM_CYSCREEN);
    HandleMain = CreateWindowExA(
        WS_EX_APPWINDOW, WinClassName, window_name, WS_POPUP,
        0, 0, WinScreenWidth, WinScreenHeight,
        nullptr, nullptr, WinInstance, nullptr);

    if (HandleMain == nullptr) {
        return 1;
    }

    // The shared device context, acquired and released around the palette
    // set-up. Written out rather than calling `Win::get_hdc` and
    // `Win::release_hdc` because the original has the whole protocol inline
    // here - a call would be one instruction where the image has thirty.
    if (WinHdcRefCount == 0) {
        if (DirectDrawSurface == nullptr) {
            WinSharedHdc = GetDC(HandleMain);
        } else {
            DirectDrawSurface->GetDC(&WinSharedHdc);
        }
        if (WinSharedHdc != nullptr) {
            WinHdcRefCount = 1;
        }
    } else {
        ++WinHdcRefCount;
    }

    SetBkMode(WinSharedHdc, TRANSPARENT);
    SetSystemPaletteUse(WinSharedHdc, SYSPAL_NOSTATIC);
    SelectPalette(WinSharedHdc, PaletteInitialized, FALSE);

    --WinHdcRefCount;
    if (WinHdcRefCount == 0) {
        if (DirectDrawSurface == nullptr) {
            ReleaseDC(HandleMain, WinSharedHdc);
        } else {
            DirectDrawSurface->ReleaseDC(WinSharedHdc);
        }
        WinSharedHdc = nullptr;
    }

    ScreenBuffer.init(WinScreenWidth, WinScreenHeight, 0, 0);
    ScreenBuffer.fill(0);
    if (logo.load_pcx("logo.pcx", PaletteActive, 10, 0xEC) == 0) {
        logo.copy(&ScreenBuffer, 0, 0,
                  (ScreenBuffer.dib_.bmiHeader.biWidth
                   - logo.dib_.bmiHeader.biWidth) / 2,
                  (logo.dib_.bmiHeader.biHeight
                   - ScreenBuffer.dib_.bmiHeader.biHeight) / 2,
                  logo.dib_.bmiHeader.biWidth,
                  -logo.dib_.bmiHeader.biHeight);
    }

    if ((JackalInitFlags & 4) == 0) {
        ShowWindow(HandleMain, SW_SHOW);
        Win::flip(0);
    }

    return 0;
}
