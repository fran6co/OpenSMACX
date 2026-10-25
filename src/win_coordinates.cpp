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
#include "win.h"

// RECOVERY_IMPORT: v1 __imp__GetCursorPos@4 USER32.dll name:GetCursorPos

/*
Purpose: Refresh the cached screen metrics from the system.
// ORIGINAL: 0x005EFD00 sub_5efd00 0x005EFD00-0x005EFD1B BYTE_EXACT
// symbol    _DDInitRefreshScreenMetrics
// size      27 bytes
// callers   3   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x005EFD09 0x005EFD12
Return Value: The height, which is the second value read.
Status: Complete
*/
extern "C" int __cdecl DDInitRefreshScreenMetrics() {
    WinScreenWidth = GetSystemMetrics(SM_CXSCREEN);
    return (WinScreenHeight = GetSystemMetrics(SM_CYSCREEN));
}

// ORIGINAL: 0x005ECF20 ?screen_to_client@Win@@QAEXPAURECT@@@Z 0x005ECF20-0x005ECFD8 FILE BYTE_EXACT
// symbol    ?screen_to_client@Win@@QAEXPAUtagRECT@@@Z
// notes     the RECT overload; the coordinate-pair form shares the name
//           and marker_symbols resolves to whichever comes first.
// working copy - scaffold materialised by --work
// size      184 bytes
// prototype void (__thiscall ?screen_to_client@Win@@QAEXPAURECT@@@Z)(Win* this, RECT*)
// callers   4   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005ED2D0
void Win::screen_to_client(RECT * rect) {
    if (rect == 0) {
        return;
    }
    int dx = -(client_rect_.left + outer_rect_.left);
    int dy = -(client_rect_.top + outer_rect_.top);
    if ((iFlags_ & 0x20) && win_parent_ != 0) {
        win_parent_->screen_to_client(&dx, &dy);
        if (iFlags_ & 0x8000) {
            dx += win_parent_->outer_rect_.left;
            dy += win_parent_->outer_rect_.top;
        }
    }
    rect->left += dx;
    rect->right += dx;
    rect->top += dy;
    rect->bottom += dy;
}

// ORIGINAL: 0x005EC8A0 ?get_mouse_pos@Win@@QAEXPAHPAH@Z 0x005EC8A0-0x005EC952 FILE BYTE_EXACT
// symbol    ?get_mouse_pos@Win@@QAEXPAH0@Z
// working copy - scaffold materialised by --work
// size      178 bytes
// prototype void (__thiscall ?get_mouse_pos@Win@@QAEXPAHPAH@Z)(Win* this, int*, int*)
// callers   26   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005ED2D0
// indirect  0x005EC8C5

void Win::get_mouse_pos(int * x, int * y) {
    if (x != 0 && y != 0) {
        POINT pt;  // the artifact spelled this `struct Pt { int x, y; }`
        // 0x00669284, NOT the SetCursorPos slot this used to reach through
        // the file-scope `g`. The image calls `[0x669284]` here
        // (0x005EC8C5) and the two differ only in a relocation, which
        // the byte comparison masks - so the claim held while the call
        // was wrong.
        GetCursorPos(&pt);
        *x = pt.x;
        *y = pt.y;
        *x -= client_rect_.left + outer_rect_.left;
        *y -= client_rect_.top + outer_rect_.top;
        if ((iFlags_ & 0x20) != 0 && win_parent_ != 0) {
            // THE PARENT'S, not this window's. The image loads win_parent_
            // into ECX - the receiver register - and calls from there:
            // `mov ecx, [esi + 0xc4]` / `test ecx, ecx` straight into the
            // call. Calling it on `this` compiled the same load into EAX and
            // then an extra `mov ecx, esi` to set the receiver back.
            win_parent_->screen_to_client(x, y);
            if ((iFlags_ & 0x8000) != 0) {
                *x += win_parent_->outer_rect_.left;
                *y += win_parent_->outer_rect_.top;
            }
        }
    }
}

// ORIGINAL: 0x005ED0A0 ?nonscreen_to_client@Win@@QAEXPAURECT@@@Z 0x005ED0A0-0x005ED16A FILE BYTE_EXACT
// symbol    ?nonscreen_to_client@Win@@QAEXPAUtagRECT@@@Z
// working copy - scaffold materialised by --work
// size      202 bytes
// prototype void (__thiscall ?nonscreen_to_client@Win@@QAEXPAURECT@@@Z)(Win* this, RECT*)
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005ED2D0

// TRIED: 4 -> 18 of 64, and the two changes that got it there are BEHAVIOUR
// fixes, not spellings. (1) The parent walk called `screen_to_client` on
// `this`; the image loads win_parent_ into ECX - the receiver register - and
// calls from there, exactly as the sibling client_to_screen does. (2) The
// 0x8000 adjustment sat OUTSIDE the 0x20 block, so it applied even when no
// parent walk had happened; the image's `test ah, 0x80` is between the call
// and the `jmp` that rejoins the 0x20 test's own `je` target.
//
// What is left is allocation, not shape: 64 instructions against 64, with
// `this` and `rect` in swapped registers throughout. The image pushes ecx to
// allocate one local slot and then REUSES THE ARGUMENT SLOT for localX
// (`lea edx, [esp + 0x14]`); this body keeps `rect` live instead. Converting
// the guard to an early return - the form that made client_to_screen exact -
// does not move it, so the difference is downstream of source shape.
void Win::nonscreen_to_client(RECT * rect) {
    // EARLY RETURN, matching the sibling client_to_screen that measures
    // exact. The image loads `rect` into EDI and `this` into ESI;
    // wrapping the body in `if (rect != 0)` swapped that assignment.
    if (rect == 0) {
        return;
    }
    int localX = -(client_rect_.left + outer_rect_.left);
    int localY = -(client_rect_.top + outer_rect_.top);
    if ((iFlags_ & 0x20) != 0 && win_parent_ != 0) {
        // THE PARENT'S, not this window's: the image loads win_parent_
        // into ECX - the receiver register - and calls from there
        // (`mov ecx, [esi + 0xc4]` / `test ecx, ecx` at 0x005ED0E5),
        // exactly as its sibling client_to_screen does.
        win_parent_->screen_to_client(&localX, &localY);
        // NESTED, not a sibling test. The image's `test ah, 0x80` at
        // 0x005ED104 sits between the call and the `jmp 0x5ed135` that
        // rejoins the 0x20 test's own `je` target - so it only runs when
        // the parent walk did. Its sibling client_to_screen nests it the
        // same way.
        if ((iFlags_ & 0x8000) != 0) {
            localX = localX + win_parent_->outer_rect_.left;
            localY = localY + win_parent_->outer_rect_.top;
        }
    }
    // Reuse localX/localY rather than adding `dx`/`dy` - the lever that made
    // the sibling nonclient_to_screen exact: two extra locals held one value
    // too many live and the compiler swapped `this` and `rect` between esi
    // and edi for the whole body.
    localX += outer_rect_.left;
    localY += outer_rect_.top;
    rect->left = rect->left + localX;
    rect->right = rect->right + localX;
    rect->top = rect->top + localY;
    rect->bottom = rect->bottom + localY;
}

// ORIGINAL: 0x005ED520 ?screen_to_nonclient@Win@@QAEXPAH0@Z 0x005ED520-0x005ED5BD FILE BYTE_EXACT
// working copy - scaffold materialised by --work
// size      157 bytes
// prototype void (__thiscall ?screen_to_nonclient@Win@@QAEXPAH0@Z)(Win* this, int*, int*)
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005ED2D0
void Win::screen_to_nonclient(int * x, int * y) {
    *x -= client_rect_.left + outer_rect_.left;
    *y -= client_rect_.top + outer_rect_.top;
    if ((iFlags_ & 0x20) != 0 &&
        win_parent_ != 0) {
        (win_parent_)->screen_to_client(x, y);
        if ((iFlags_ & 0x8000) != 0) {
            *x += win_parent_->outer_rect_.left;
            *y += win_parent_->outer_rect_.top;
        }
    }
    *x += outer_rect_.left;
    *y += outer_rect_.top;
}

// ORIGINAL: 0x005ECFE0 ?client_to_screen@Win@@QAEXPAURECT@@@Z 0x005ECFE0-0x005ED094 FILE BYTE_EXACT
// symbol    ?client_to_screen@Win@@QAEXPAUtagRECT@@@Z
// in place of the `reinterpret_cast<char *>(this)` raw-offset form - same
// score either way (9/56, 0.885 similar, best of ten flag sets); kept for
// the class_debt fix, not a byte-exactness gain. Plateau MISMATCH.
// working copy - scaffold materialised by --work
// size      180 bytes
// prototype void (__thiscall ?client_to_screen@Win@@QAEXPAURECT@@@Z)(Win* this, RECT*)
// callers   12   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005ED240

void Win::client_to_screen(RECT * rect) {
    if (rect == 0) {
        return;
    }
    int xAdj = client_rect_.left + outer_rect_.left;
    int yAdj = client_rect_.top + outer_rect_.top;
    // NO `owner` LOCAL. The image tests win_parent_ in ECX - the receiver
    // register - and calls straight from there; binding it to a local put it
    // in EBX and cost a `push ebx` plus a `mov ecx, ebx` at the call. And the
    // image RELOADS it afterwards (`mov esi, [esi + 0xc4]` at 0x005ED044)
    // rather than keeping it live, so the local was not what the original
    // wrote either.
    if ((iFlags_ & 0x20U) != 0 && win_parent_ != 0) {
        win_parent_->client_to_screen(&xAdj, &yAdj);
        if ((iFlags_ & 0x8000U) != 0) {
            xAdj -= win_parent_->outer_rect_.left;
            yAdj -= win_parent_->outer_rect_.top;
        }
    }
    rect->left += xAdj;
    rect->right += xAdj;
    rect->top += yAdj;
    rect->bottom += yAdj;
}

// ORIGINAL: 0x005ED170 ?nonclient_to_screen@Win@@QAEXPAURECT@@@Z 0x005ED170-0x005ED236 FILE BYTE_EXACT
// symbol    ?nonclient_to_screen@Win@@QAEXPAUtagRECT@@@Z
// working copy - scaffold materialised by --work
// size      198 bytes
// prototype void (__thiscall ?nonclient_to_screen@Win@@QAEXPAURECT@@@Z)(Win* this, RECT*)
// callers   1   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005ED240

void Win::nonclient_to_screen(RECT * rect) {
    if (rect == 0) {
        return;
    }
    int x = client_rect_.left + outer_rect_.left;
    int y = client_rect_.top + outer_rect_.top;
    if ((iFlags_ & 0x20) != 0 && win_parent_ != 0) {
        // THE PARENT'S: `mov ecx, [esi + 0xc4]` / `test ecx, ecx` at
        // 0x005ED1B1 puts win_parent_ in the receiver register.
        win_parent_->client_to_screen(&x, &y);
        if ((iFlags_ & 0x8000) != 0) {
            x -= win_parent_->outer_rect_.left;
            y -= win_parent_->outer_rect_.top;
        }
    }
    // LEVER: reuse x and y, do NOT introduce `dx`/`dy`. Two extra locals
    // held one value too many live and the compiler swapped `this` and
    // `rect` between esi and edi for the whole body - 39 of 62 with every
    // instruction otherwise in place. Subtracting into x/y and adding those
    // is the same arithmetic with one fewer live value, and it is exact.
    x -= outer_rect_.left;
    y -= outer_rect_.top;
    rect->left += x;
    rect->right += x;
    rect->top += y;
    rect->bottom += y;
}

/*
Purpose: Determine whether a point is inside a rectangle using Win32 edge semantics.
// ORIGINAL: 0x005FA7E0 ?in_box@@YAHHHPAURECT@@@Z 0x005FA7E0-0x005FA814 BYTE_EXACT
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

// LEVER: guard clauses, not one `&&` chain. The image is four independent
// tests, each with its OWN `xor eax,eax; ret` on failure - not a shared
// early-return block - and the last comparison returns straight out of a
// `setl` with no branch at all. A single boolean expression folds the four
// tests into shared control flow the image does not have.
*/
BOOL __cdecl in_box(int x, int y, const RECT *rect) {
    if (x < rect->left) {
        return FALSE;
    }
    if (x >= rect->right) {
        return FALSE;
    }
    if (y < rect->top) {
        return FALSE;
    }
    return y < rect->bottom;
}

/*
Purpose: Determine whether a point is inside an origin-and-dimensions rectangle.
// ORIGINAL: 0x005FA7A0 ?in_box@@YAHHHHHHH@Z 0x005FA7A0-0x005FA7DB BYTE_EXACT
// LEVER: four SEPARATE early-return guards (`if (x<left) return 0; ... if (x>=right) return 0; if (y<top) return 0; ...`) instead of the merged `x>=right || y<top`, and plain signed `+` instead of the `int_from_bits`/`uint32_t` bit-cast - the image never combines the two mid checks and never needs the cast. 0/25 -> BYTE_EXACT.
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
    const int right = left + width;
    if (x >= right) {
        return 0;
    }
    if (y < top) {
        return 0;
    }
    const int bottom = top + height;
    return y < bottom;
}
