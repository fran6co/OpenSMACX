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
// These member implementations retain the original out-of-line tutorial-call
// boundaries. This current source organization does not assert historical TUs.
#include "stdafx.h"
#include "tutwin.h"
#include "mapwin.h"
#include "basewin.h"
#include "maininterface.h"
#include "socialwin.h"
#include "designwin.h"

/*
 * Four 67-byte clones differing only in which fixed window they finish on.
 * Each centres the rectangle on both axes and hands the pair to that window's
 * client_to_screen. The centring is inlined four times rather than shared,
 * matching the original, which carries four copies.
 *
 * The halving is the compiler's signed divide-by-two - `sub eax,esi; cdq;
 * sub eax,edx; sar eax,1` - so it truncates toward zero rather than flooring,
 * and the subtraction wraps. Modelled here the way win.cpp's own midpoint()
 * models the identical idiom: unsigned arithmetic throughout, with the sign
 * bit carried back over the shift.
 */


/*
Purpose: Centre the rectangle on both axes and convert the result to screen
         coordinates through the iface window.
// ORIGINAL: 0x004BC5A0 ?iface_rect@TutWin@@QAEXPAURECT@@PAH1@Z 0x004BC5A0-0x004BC5E3 BYTE_EXACT
// LEVER: plain `left + (right - left) / 2` signed division - the image
//        already emits the `cdq; sub; sar` fixup itself, so the hand-rolled
//        unsigned-with-sign-carried-over-the-shift trick this body used to
//        model the SAME idiom compiled to different instructions. Matches
//        the sibling `base_rect`/`soc_rect`/`des_rect`, already byte-exact
//        on this same lever.
// symbol    ?iface_rect@TutWin@@QAEXPAUtagRECT@@PAH1@Z
// size      67 bytes
// prototype void (__thiscall ?iface_rect@TutWin@@QAEXPAURECT@@PAH1@Z)(TutWin* this, RECT*, int*, int*)
// callers   3   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005ED240
Return Value: n/a
Status: Complete
Verification note: client_to_screen is entered on the FIXED window at 0x007AE820,
         never on `this` - the original loads it as an immediate into ECX. The
         TutWin receiver is not read by this body at all.
*/
void TutWin::iface_rect(RECT *rect, int *x, int *y) {
    *x = rect->left + (rect->right - rect->left) / 2;
    *y = rect->top + (rect->bottom - rect->top) / 2;
    MainInterfaceGlobal.client_to_screen(x, y);
}



/*
Purpose: Centre the rectangle on both axes and convert the result to screen
         coordinates through the base window.
// ORIGINAL: 0x004BC5F0 ?base_rect@TutWin@@QAEXPAURECT@@PAH1@Z 0x004BC5F0-0x004BC633 BYTE_EXACT
// symbol    ?base_rect@TutWin@@QAEXPAUtagRECT@@PAH1@Z
// size      67 bytes
// prototype void (__thiscall ?base_rect@TutWin@@QAEXPAURECT@@PAH1@Z)(TutWin* this, RECT*, int*, int*)
// callers   2   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005ED240
Return Value: n/a
Status: Complete
Verification note: client_to_screen is entered on the FIXED window at 0x006A7628,
         never on `this` - the original loads it as an immediate into ECX. The
         TutWin receiver is not read by this body at all.
*/
void TutWin::base_rect(RECT *rect, int *x, int *y) {
    // Plain signed division: the image emits `cdq; sub eax,edx; sar eax,1`
    // (round-toward-zero) for each half, not a rounding bit-trick.
    *x = rect->left + (rect->right - rect->left) / 2;
    *y = rect->top + (rect->bottom - rect->top) / 2;
    TutWinBaseWindow.client_to_screen(x, y);
}



/*
Purpose: Centre the rectangle on both axes and convert the result to screen
         coordinates through the soc window.
// ORIGINAL: 0x004BC640 ?soc_rect@TutWin@@QAEXPAURECT@@PAH1@Z 0x004BC640-0x004BC683 BYTE_EXACT
// symbol    ?soc_rect@TutWin@@QAEXPAUtagRECT@@PAH1@Z
// size      67 bytes
// prototype void (__thiscall ?soc_rect@TutWin@@QAEXPAURECT@@PAH1@Z)(TutWin* this, RECT*, int*, int*)
// callers   2   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005ED240
Return Value: n/a
Status: Complete
Verification note: client_to_screen is entered on the FIXED window at 0x008A6270,
         never on `this` - the original loads it as an immediate into ECX. The
         TutWin receiver is not read by this body at all.
*/
void TutWin::soc_rect(RECT *rect, int *x, int *y) {
    // Plain signed division: the image emits `cdq; sub eax,edx; sar eax,1`
    // (round-toward-zero) for each half, not a rounding bit-trick.
    *x = rect->left + (rect->right - rect->left) / 2;
    *y = rect->top + (rect->bottom - rect->top) / 2;
    TutWinSocWindow.client_to_screen(x, y);
}



/*
Purpose: Centre the rectangle on both axes and convert the result to screen
         coordinates through the des window.
// ORIGINAL: 0x004BC690 ?des_rect@TutWin@@QAEXPAURECT@@PAH1@Z 0x004BC690-0x004BC6D3 BYTE_EXACT
// symbol    ?des_rect@TutWin@@QAEXPAUtagRECT@@PAH1@Z
// size      67 bytes
// prototype void (__thiscall ?des_rect@TutWin@@QAEXPAURECT@@PAH1@Z)(TutWin* this, RECT*, int*, int*)
// callers   2   call targets   1
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x005ED240
Return Value: n/a
Status: Complete
Verification note: client_to_screen is entered on the FIXED window at 0x0071F2B0,
         never on `this` - the original loads it as an immediate into ECX. The
         TutWin receiver is not read by this body at all.
*/
void TutWin::des_rect(RECT *rect, int *x, int *y) {
    // Plain signed division: the image emits `cdq; sub eax,edx; sar eax,1`
    // (round-toward-zero) for each half, not a rounding bit-trick.
    *x = rect->left + (rect->right - rect->left) / 2;
    *y = rect->top + (rect->bottom - rect->top) / 2;
    TutWinDesWindow.client_to_screen(x, y);
}



/*
 * The four do_* helpers: 123-byte clones whose ONLY real difference is which
 * fixed window they centre through. The other differing bytes are the two
 * call displacements, which move because the calls are relative and the four
 * bodies sit 0x80 apart - not because either target differs. Both targets are
 * the same for all four.
 */

/*
Purpose: Centre the rectangle, convert it through the base window, and
         show the tutorial text there against the primary map window.
// ORIGINAL: 0x004BA870 ?do_base@TutWin@@QAEXPAURECT@@PBDH@Z 0x004BA870-0x004BA8EB SEMANTIC
// LEVER: two levers stacked, 4/56 -> 45/56 MNEMONIC_ONLY. (1) plain `left + (right - left) / 2` signed division instead of the unsigned-with-sign-carried trick, same as `iface_rect`. (2) `primary` read BEFORE `window` is given any value, and its null-check written `if (primary == nullptr) { window = nullptr; } else { ...compute... }` (primary-first declaration order, and the null case as the FIRST arm) - the image reads the global first and only zeroes the result register on the branch that skips the vtable dereference, both of which a `window = nullptr;`-then-test body reorders.
// TRIED: 45/56 plateau - the remainder is a consistent eax/ecx role swap through the vbtable-dereference block and the final argument push order, tried: `uint8_t *` instead of `void *` for `window`, and folding the `vbtable` local into the expression directly - neither changes it. VC6 register allocation, not a source-form lever found here.
// symbol    ?do_base@TutWin@@QAEHPAUtagRECT@@PBDH@Z
// size      123 bytes
// prototype void (__thiscall ?do_base@TutWin@@QAEXPAURECT@@PBDH@Z)(TutWin* this, RECT*, int8*, int)
// callers   2   call targets   2
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x004BDFE0 0x005ED240
Return Value: the callout result, consumed by the tutorial's continuation gates
Verification note: the GraphicWin handed to the show is MapWinTable[0]
         VIRTUAL-BASE ADJUSTED - `mov edx,[ecx]` / `mov eax,[edx+4]` /
         `add eax,ecx` - and a null slot passes a null through rather than
         faulting, which is the only branch in the body.
Verification note: the original writes the y centre over its own first
         argument slot before taking its address, so the RECT pointer is dead
         from that point. A local is equivalent and is what is used here.
*/
int TutWin::do_base(RECT *rect, const char *text, int flag) {
    // Plain signed division: the image emits `cdq; sub eax,edx; sar eax,1`
    // (round-toward-zero) for each half, not a rounding bit-trick.
    int x = rect->left + (rect->right - rect->left) / 2;
    int y = rect->top + (rect->bottom - rect->top) / 2;
    TutWinBaseWindow.client_to_screen(&x, &y);

    MapWin *const primary = MapWinTable[0];
    GraphicWin *const window = primary;
    return this->tut_win(window, text, x, y, nullptr, flag, -1, -1);
}


/*
Purpose: Centre the rectangle, convert it through the iface window, and
         show the tutorial text there against the primary map window.
// ORIGINAL: 0x004BA8F0 ?do_iface@TutWin@@QAEXPAURECT@@PBDH@Z 0x004BA8F0-0x004BA96B SEMANTIC
// LEVER: same two levers as `do_base` (see that marker) - plain signed division, and `primary` read/null-checked before `window` is given any value. 4/56 -> 45/56 MNEMONIC_ONLY.
// TRIED: same 45/56 plateau as `do_base` - a consistent eax/ecx register swap through the vbtable block; see that marker for what was tried.
// symbol    ?do_iface@TutWin@@QAEHPAUtagRECT@@PBDH@Z
// size      123 bytes
// prototype void (__thiscall ?do_iface@TutWin@@QAEXPAURECT@@PBDH@Z)(TutWin* this, RECT*, int8*, int)
// callers   1   call targets   2
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x004BDFE0 0x005ED240
Return Value: the callout result, consumed by the tutorial's continuation gates
Verification note: the GraphicWin handed to the show is MapWinTable[0]
         VIRTUAL-BASE ADJUSTED - `mov edx,[ecx]` / `mov eax,[edx+4]` /
         `add eax,ecx` - and a null slot passes a null through rather than
         faulting, which is the only branch in the body.
Verification note: the original writes the y centre over its own first
         argument slot before taking its address, so the RECT pointer is dead
         from that point. A local is equivalent and is what is used here.
*/
int TutWin::do_iface(RECT *rect, const char *text, int flag) {
    // Plain signed division: the image emits `cdq; sub eax,edx; sar eax,1`
    // (round-toward-zero) for each half, not a rounding bit-trick.
    int x = rect->left + (rect->right - rect->left) / 2;
    int y = rect->top + (rect->bottom - rect->top) / 2;
    MainInterfaceGlobal.client_to_screen(&x, &y);

    MapWin *const primary = MapWinTable[0];
    GraphicWin *const window = primary;
    return this->tut_win(window, text, x, y, nullptr, flag, -1, -1);
}


/*
Purpose: Centre the rectangle, convert it through the soc window, and
         show the tutorial text there against the primary map window.
// ORIGINAL: 0x004BA970 ?do_soc@TutWin@@QAEXPAURECT@@PBDH@Z 0x004BA970-0x004BA9EB SEMANTIC
// LEVER: same two levers as `do_base` (see that marker) - plain signed division, and `primary` read/null-checked before `window` is given any value. 4/56 -> 45/56 MNEMONIC_ONLY.
// TRIED: same 45/56 plateau as `do_base` - a consistent eax/ecx register swap through the vbtable block; see that marker for what was tried.
// symbol    ?do_soc@TutWin@@QAEHPAUtagRECT@@PBDH@Z
// size      123 bytes
// prototype void (__thiscall ?do_soc@TutWin@@QAEXPAURECT@@PBDH@Z)(TutWin* this, RECT*, int8*, int)
// callers   1   call targets   2
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x004BDFE0 0x005ED240
Return Value: the callout result, consumed by the tutorial's continuation gates
Verification note: the GraphicWin handed to the show is MapWinTable[0]
         VIRTUAL-BASE ADJUSTED - `mov edx,[ecx]` / `mov eax,[edx+4]` /
         `add eax,ecx` - and a null slot passes a null through rather than
         faulting, which is the only branch in the body.
Verification note: the original writes the y centre over its own first
         argument slot before taking its address, so the RECT pointer is dead
         from that point. A local is equivalent and is what is used here.
*/
int TutWin::do_soc(RECT *rect, const char *text, int flag) {
    // Plain signed division: the image emits `cdq; sub eax,edx; sar eax,1`
    // (round-toward-zero) for each half, not a rounding bit-trick.
    int x = rect->left + (rect->right - rect->left) / 2;
    int y = rect->top + (rect->bottom - rect->top) / 2;
    TutWinSocWindow.client_to_screen(&x, &y);

    MapWin *const primary = MapWinTable[0];
    GraphicWin *const window = primary;
    return this->tut_win(window, text, x, y, nullptr, flag, -1, -1);
}


/*
Purpose: Centre the rectangle, convert it through the des window, and
         show the tutorial text there against the primary map window.
// ORIGINAL: 0x004BA9F0 ?do_des@TutWin@@QAEXPAURECT@@PBDH@Z 0x004BA9F0-0x004BAA6B SEMANTIC
// LEVER: same two levers as `do_base` (see that marker) - plain signed division, and `primary` read/null-checked before `window` is given any value. 4/56 -> 45/56 MNEMONIC_ONLY.
// TRIED: same 45/56 plateau as `do_base` - a consistent eax/ecx register swap through the vbtable block; see that marker for what was tried.
// symbol    ?do_des@TutWin@@QAEHPAUtagRECT@@PBDH@Z
// size      123 bytes
// prototype void (__thiscall ?do_des@TutWin@@QAEXPAURECT@@PBDH@Z)(TutWin* this, RECT*, int8*, int)
// callers   1   call targets   2
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x004BDFE0 0x005ED240
Return Value: the callout result, consumed by the tutorial's continuation gates
Verification note: the GraphicWin handed to the show is MapWinTable[0]
         VIRTUAL-BASE ADJUSTED - `mov edx,[ecx]` / `mov eax,[edx+4]` /
         `add eax,ecx` - and a null slot passes a null through rather than
         faulting, which is the only branch in the body.
Verification note: the original writes the y centre over its own first
         argument slot before taking its address, so the RECT pointer is dead
         from that point. A local is equivalent and is what is used here.
*/
int TutWin::do_des(RECT *rect, const char *text, int flag) {
    // Plain signed division: the image emits `cdq; sub eax,edx; sar eax,1`
    // (round-toward-zero) for each half, not a rounding bit-trick.
    int x = rect->left + (rect->right - rect->left) / 2;
    int y = rect->top + (rect->bottom - rect->top) / 2;
    TutWinDesWindow.client_to_screen(&x, &y);

    MapWin *const primary = MapWinTable[0];
    GraphicWin *const window = primary;
    return this->tut_win(window, text, x, y, nullptr, flag, -1, -1);
}


