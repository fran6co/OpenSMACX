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
    new (static_cast<GraphicWin *>(this)) GraphicWin();

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


/*
Purpose: Step the receiver back to the subobject ??_GReportWin@@UAEPAXI@Z
         expects, then forward unchanged.
// ORIGINAL: 0x004AD870 ??_GReportWin@@WEEE@AEPAXI@Z 0x004AD870-0x004AD87B BYTE_EXACT
// symbol    ??_EReportWin@@WEEE@AEPAXI@Z
// CORRECTED from ??3ReportWin@@SAXPAXI@Z
//   11 bytes, `sub ecx, 0x444; jmp 0x004AD840` into
//   ??_GReportWin@@UAEPAXI@Z, which executes `ret 4`; no stack access
//   and the receiver stays in ECX. `WEEE@` re-demangles to
//   adjustor{1092} and 1092 == 0x444, the constant subtracted
// size      11 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
Return Value: the forwarded call's
Status: Complete
*/

/*
Purpose: Advance the satellite orbital overlay one tick: clear the orbit
         rectangle, integrate the sixteen satellites' positions from their
         velocities and the ReportWin's own step scalar at 0xEE30, decode
         the next Flic frame under them, composite it through the second
         surface buffer, then fill one 2x2 marker per live satellite and
         publish the orbit rectangle.

// ORIGINAL: 0x0049FE40 ?sat_anim@ReportWin@@QAEXXZ 0x0049FE40-0x0049FFB7 FILE
// size      375 bytes
// prototype void (__thiscall ?sat_anim@ReportWin@@QAEXXZ)(ReportWin* this)
// callers   3   call targets   6
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005D5350 0x005D5440 0x005D5930 0x005D8370 0x0062A070 0x006458C8
// Homed from src/recovered/units/0049fe40.cpp. The four float arrays live
// in field_F938_'s storage (0xF938..0xFAB8): position pairs at 0xF938/
// 0xF978, scratch results at 0xF9B8/0xF9F8, velocities at 0xFA38, the
// live-satellite flags at 0xFA78; the loop cursor sits just before
// field_E2B0_ at 0xE29C. The 0xEE30 scalar is field_EE30_ (see the layout
// note above: it is NOT the Flic, which starts at 0xEE34), and the frame
// counters the decode reads are the Flic's own at 0xF3E0/0xF3E4.
Status: Coverage transcription - unclaimed body
*/
void ReportWin::sat_anim() {
    unsigned char *const self = reinterpret_cast<unsigned char *>(this);
    int *const index = reinterpret_cast<int *>(self + 0xE29C);

    fill(reinterpret_cast<RECT *>(self + 0xE2B0), 0x41);

    *index = 0;
    do {
        int i = *index;
        reinterpret_cast<float *>(self + 0xF9B8)[i] =
            reinterpret_cast<float *>(self + 0xF9B8)[i] -
            reinterpret_cast<float *>(self + 0xFA38)[i] *
                reinterpret_cast<float *>(self + 0xF938)[i];

        i = *index;
        reinterpret_cast<float *>(self + 0xF9F8)[i] =
            reinterpret_cast<float *>(self + 0xF9F8)[i] -
            reinterpret_cast<float *>(self + 0xFA38)[i] *
                reinterpret_cast<float *>(self + 0xF978)[i];

        i = *index;
        reinterpret_cast<float *>(self + 0xF938)[i] =
            reinterpret_cast<float *>(self + 0xF9B8)[i] * field_EE30_ +
            reinterpret_cast<float *>(self + 0xF938)[i];

        i = *index;
        reinterpret_cast<float *>(self + 0xF978)[i] =
            reinterpret_cast<float *>(self + 0xF9F8)[i] * field_EE30_ +
            reinterpret_cast<float *>(self + 0xF978)[i];

        *index = *index + 1;
    } while (*index < 0x10);

    reinterpret_cast<Flic *>(self + 0xEE34)->decode_frame(
        *reinterpret_cast<int *>(self + 0xF3E0),
        *reinterpret_cast<int *>(self + 0xF3E4));

    reinterpret_cast<Buffer *>(self + 0xE8A8)->draw(
        reinterpret_cast<Buffer *>(self + 0x444), 9,
        *reinterpret_cast<int *>(self + 0xE2A0),
        *reinterpret_cast<int *>(self + 0xE2A4));

    *index = 0;
    do {
        if (reinterpret_cast<int *>(self + 0xFA78)[*index] != 0) {
            const int marker_y =
                static_cast<int>(
                    reinterpret_cast<float *>(self + 0xF978)[*index]) +
                *reinterpret_cast<int *>(self + 0xE2AC);
            const int marker_x =
                static_cast<int>(
                    reinterpret_cast<float *>(self + 0xF938)[*index]) +
                *reinterpret_cast<int *>(self + 0xE2A8);
            fill(marker_x, marker_y, 2, 2, 0xFF);
        }
        *index = *index + 1;
    } while (*index < 0x10);

    soft_update(reinterpret_cast<RECT *>(self + 0xE2B0));
}
