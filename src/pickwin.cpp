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
// RULED-OUT: register allocation - the SEH prologue agrees (7/7) then the
//            compiled body reserves an extra `sub esp, 8` and an extra
//            callee-save (ebx) the image does not; the image keeps `this` in
//            ecx/a stack slot with less spill pressure. MISMATCH, 16/101
//            instructions agree. Not chased further - a source-level rewrite
//            to shed one register's worth of pressure was not found cheaply.
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
    GraphicWin::construct();

    new (popup_) Popup();
    new (flatButton1_) FlatButton();
    new (flatButton2_) FlatButton();
    new (flatButton3_) FlatButton();

    VectorCtorIterator(sprites1_, 0x2C, 3, PickWinSpriteCtor, PickWinSpriteDtor);
    VectorCtorIterator(sprites2_, 0x2C, 3, PickWinSpriteCtor, PickWinSpriteDtor);
    VectorCtorIterator(sprites3_, 0x2C, 3, PickWinSpriteCtor, PickWinSpriteDtor);
    VectorCtorIterator(sprites4_, 0x2C, 3, PickWinSpriteCtor, PickWinSpriteDtor);

    new (sprite5_) Sprite();

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

    uint32_t *const object = reinterpret_cast<uint32_t *>(this);
    object[0x000 / 4] = PickWinPrimaryVtable;
    object[0x444 / 4] = PickWinBufferVtable;
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
