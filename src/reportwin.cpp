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
#include "reportwin.h"
#include "menu.h"
#include "win.h"
#include "vector_teardown.h"

const void *const ReportWinFlatButtonCtor = (const void *)0x00607CF0;
const void *const ReportWinFlatButtonDtor = (const void *)0x00406880;

const uint32_t ReportWinPrimaryVtable = 0x0066D754;
const uint32_t ReportWinBufferVtable = 0x0066D74C;

/*
Purpose: Construct every sub-object in image order - a FlatButton[7] run, a
         Spot, a ListBox(1), nine individually-constructed FlatButtons split
         by two ButtonGroups, a Scroll, three Times, two Buffers and a Flic -
         then install ReportWin's own vftables.
// ORIGINAL: 0x004AD6B0 ??0ReportWin@@QAE@XZ 0x004AD6B0-0x004AD838;0x006592F0-0x00659419
// TRIED: deriving from ConstructedGraphicWin (graphicwin.h) to move the
//   base construction ahead of the members. The ORDER IS RIGHT - this image
//   really does `call 0x5d4cf0` before its first `lea ecx, [esi + N]`, checked
//   2026-08-22 - but making the source agree took this body 14 -> 10
//   of 87. Same shape as Scroll (scroll.cpp), where the reorder alone was
//   worth 1 instruction and the payoff came from two levers underneath that it
//   made reachable; here those levers are not the vtable-immediate one, since
//   these constants already fold, and this body is structurally far off
//   (149 compiled against 87). Re-derive the ordering fix as part
//   of a dedicated pass on the whole body, not on its own.
// TRIED: register allocation - the SEH prologue agrees (7/7) then the
//            compiled body reserves an extra `sub esp, 8` the image does
//            not. MISMATCH, 9/87 instructions agree.
// LEVER: flatButtons1_ and spot_ (the whole prefix before the irreducible
//        listBox_) converted to real declared members, built implicitly.
//        9/87 -> 14/87 agreeing instructions - still MISMATCH, real
//        improvement. Everything from listBox_ onward stays raw/explicit:
//        listBox_ has no declared one-argument constructor and Flic's own
//        recovery is out of this batch's scope, and real members declared
//        after either would be hoisted into the same implicit preamble
//        ahead of them, which the image does not do.
// size      689 bytes
// prototype void (__thiscall ??0ReportWin@@QAE@XZ)(ReportWin* this)
// callers   1   call targets   11
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x005D4CF0 0x005D7210 0x005FA860 0x006051D0 0x00607CF0 0x00609DB0 0x006161D0 0x00629D60 0x0062B7C0 0x006456E4 0x006457C2
*/
// ListBox has no one-argument constructor declared and Flic's own recovery
// (0x00629D60) is out of this batch's scope, so both are called directly at
// their image address, the same treatment pending_bodies.cpp gives every
// not-yet-promoted callee.
ReportWin::ReportWin() {
    GraphicWin::construct();

    // flatButtons1_ and spot_ are real members now (see reportwin.h) and
    // build implicitly, in declaration order, before this body runs.

    typedef void(__fastcall *pending_listbox_ctor)(void *, void *, int);
    reinterpret_cast<pending_listbox_ctor>(0x00609DB0)(listBox_, nullptr, 1);

    new (flatButton2_) FlatButton();
    new (flatButton3_) FlatButton();
    new (buttonGroup1_) ButtonGroup();
    new (flatButton4_) FlatButton();
    new (flatButton5_) FlatButton();
    new (flatButton6_) FlatButton();
    new (buttonGroup2_) ButtonGroup();
    new (flatButton7_) FlatButton();
    new (flatButton8_) FlatButton();
    new (flatButton9_) FlatButton();
    new (scroll_) Scroll();
    new (time1_) Time();
    new (time2_) Time();
    new (time3_) Time();
    new (buffer1_) Buffer();
    new (buffer2_) Buffer();

    typedef void(__fastcall *pending_flic_ctor)(void *, void *);
    reinterpret_cast<pending_flic_ctor>(0x00629D60)(flic_, nullptr);

    uint32_t *const object = reinterpret_cast<uint32_t *>(this);
    object[0x000 / 4] = ReportWinPrimaryVtable;
    object[0x444 / 4] = ReportWinBufferVtable;
}

/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
// ORIGINAL: 0x004980F0 ?on_mouse_move@ReportWin@@QAEXHH@Z 0x004980F0-0x004980F3 BYTE_EXACT
// size      3 bytes
// prototype void (__thiscall ?on_mouse_move@ReportWin@@QAEXHH@Z)(ReportWin* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void ReportWin::on_mouse_move(int, int) {
}

void __fastcall report_win_on_mouse_move_redirect(ReportWin *self, void *, int a1, int a2) {
    self->on_mouse_move(a1, a2);
}

/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
// ORIGINAL: 0x00498100 ?on_mouse_leave@ReportWin@@QAEXHH@Z 0x00498100-0x00498103 BYTE_EXACT
// symbol    ?on_mouse_leave@ReportWin@@UAEXHH@Z
// size      3 bytes
// prototype void (__thiscall ?on_mouse_leave@ReportWin@@QAEXHH@Z)(ReportWin* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void ReportWin::on_mouse_leave(int, int) {
}

void __fastcall report_win_on_mouse_leave_redirect(ReportWin *self, void *, int a1, int a2) {
    self->on_mouse_leave(a1, a2);
}


