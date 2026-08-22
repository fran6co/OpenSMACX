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
#include "pickwin.h"
#include "vector_teardown.h"

const void *const PickWinSpriteCtor = (const void *)0x005E37E0;
const void *const PickWinSpriteDtor = (const void *)0x00406850;
const void *const PickWinFlatButtonCtor = (const void *)0x00607CF0;
const void *const PickWinFlatButtonDtor = (const void *)0x00406880;

const uint32_t PickWinPrimaryVtable = 0x0066D13C;
const uint32_t PickWinBufferVtable = 0x0066D134;

/*
// ORIGINAL: 0x0048AC10 ??0PickWin@@QAE@XZ 0x0048AC10-0x0048ADB6;0x00657E30-0x00657F65
// TRIED: deriving from ConstructedGraphicWin (graphicwin.h) to move the
//   base construction ahead of the members. The ORDER IS RIGHT - this image
//   really does `call 0x5d4cf0` before its first `lea ecx, [esi + N]`, checked
//   2026-08-22 - but making the source agree took this body 21 -> 16
//   of 101. Same shape as Scroll (scroll.cpp), where the reorder alone was
//   worth 1 instruction and the payoff came from two levers underneath that it
//   made reachable; here those levers are not the vtable-immediate one, since
//   these constants already fold, and this body is structurally far off
//   (134 compiled against 101). Re-derive the ordering fix as part
//   of a dedicated pass on the whole body, not on its own.
// TRIED: register allocation - the SEH prologue agrees (7/7) then the
//            compiled body reserves an extra `sub esp, 8` and an extra
//            callee-save (ebx) the image does not; the image keeps `this` in
//            ecx/a stack slot with less spill pressure. MISMATCH, 16/101
//            instructions agree. Not chased further - a source-level rewrite
//            to shed one register's worth of pressure was not found cheaply.
// LEVER: popup_ (sizeof(Popup) == 0x537C matches exactly) and the
//        flatButton1_/2_/3_/sprites1_../4_/sprite5_ run right after it -
//        the whole prefix before the irreducible listBox_ - converted to
//        real declared members, built implicitly. 16/101 -> 21/101; still
//        MISMATCH but a real improvement, unlike the same lever tried on
//        ProdPicker and ReportIf in this batch (both went WORSE - see their
//        own files). listBox_ and everything from flatButton4_ onward stay
//        raw/explicit: listBox_ has no declared one-argument constructor
//        (out of this batch's scope), and real members declared after it
//        would be hoisted into the same implicit preamble as the ones
//        above it, ahead of listBox_'s explicit call, which the image does
//        not do.
// size      731 bytes
// prototype void (__thiscall ??0PickWin@@QAE@XZ)(PickWin* this)
// callers   1   call targets   8
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x004048A0 0x005D4CF0 0x005E37E0 0x00607CF0 0x00609DB0 0x0062B7C0 0x006456E4 0x006457C2
*/
// The construction chain is GraphicWin::construct(), not a base constructor
// call - GraphicWin's own constructor is the empty inline stub, and every
// sub-object below is placement-new'd explicitly, in the exact order the
// disassembly shows, because C++'s implicit member construction would run
// them all BEFORE this body, and GraphicWin::construct() has to be first.
//
// ListBox has no one-argument constructor declared - its own recovery
// (0x00609DB0) is out of this batch's scope - so it is called directly at
// its image address here, the same treatment pending_bodies.cpp gives every
// not-yet-promoted callee: a `reinterpret_cast` of the address, called
// immediately, compiles the image's own `call rel32` rather than the
// `call dword ptr [...]` a stored pointer would give.
PickWin::PickWin() {
    new (static_cast<GraphicWin *>(this)) GraphicWin();

    // popup_, flatButton1_..flatButton3_, sprites1_..sprites4_ and sprite5_
    // are REAL members now (see pickwin.h) and build implicitly, in
    // declaration order, before this body runs.
    typedef void(__fastcall *pending_listbox_ctor)(void *, void *, int);
    reinterpret_cast<pending_listbox_ctor>(0x00609DB0)(listBox_, nullptr, 1);

    new (flatButton4_) FlatButton();
    new (flatButton5_) FlatButton();
    new (flatButton6_) FlatButton();
    new (flatButton7_) FlatButton();
    new (flatButton8_) FlatButton();
    new (flatButton9_) FlatButton();

    VectorCtorIterator(flatButtons10_, 0xB4C, 7, PickWinFlatButtonCtor, PickWinFlatButtonDtor);

    new (buttonGroup_) ButtonGroup();

}

/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
// ORIGINAL: 0x00489B10 ?on_button_passover@PickWin@@QAEXHH@Z 0x00489B10-0x00489B13 BYTE_EXACT
// size      3 bytes
// prototype void (__thiscall ?on_button_passover@PickWin@@QAEXHH@Z)(PickWin* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void PickWin::on_button_passover(int, int) {
}

void __fastcall pick_win_on_button_passover_redirect(PickWin *self, void *, int a1, int a2) {
    self->on_button_passover(a1, a2);
}

/*
Purpose: Step the receiver back to the subobject ??_GPickWin@@UAEPAXI@Z
         expects, then forward unchanged.
// ORIGINAL: 0x0048ADF0 ??_GPickWin@@WEEE@AEPAXI@Z 0x0048ADF0-0x0048ADFB BYTE_EXACT
// symbol    ??_EPickWin@@WEEE@AEPAXI@Z
// CORRECTED from ??3PickWin@@SAXPAXI@Z
//   11 bytes, `sub ecx, 0x444; jmp 0x0048ADC0` into
//   ??_GPickWin@@UAEPAXI@Z, which executes `ret 4`; no stack access and
//   the receiver stays in ECX. `WEEE@` re-demangles to adjustor{1092}
//   and 1092 == 0x444, the constant subtracted
// size      11 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: the forwarded call's
Status: Complete
*/
