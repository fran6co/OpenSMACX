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
#include "netwin.h"

/*
Purpose: Unknown; the legacy implementation is a bare return with no body.
// ORIGINAL: 0x0047F920 ?UNK1@NetWin@@QAEXXZ 0x0047F920-0x0047F921 BYTE_EXACT
// size      1 bytes
// prototype void (__thiscall ?UNK1@NetWin@@QAEXXZ)(NetWin* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void NetWin::UNK1() {
}

/*
Purpose: Unknown; the legacy implementation is a bare return with no body.
// ORIGINAL: 0x0047F930 ?UNK2@NetWin@@QAEXXZ 0x0047F930-0x0047F931 BYTE_EXACT
// size      1 bytes
// prototype void (__thiscall ?UNK2@NetWin@@QAEXXZ)(NetWin* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void NetWin::UNK2() {
}

/*
Purpose: Unknown; the legacy implementation is a bare return with no body.
// ORIGINAL: 0x004834D0 ?alloc_slots@NetWin@@QAEXXZ 0x004834D0-0x004834D1 BYTE_EXACT
// size      1 bytes
// prototype void (__thiscall ?alloc_slots@NetWin@@QAEXXZ)(NetWin* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void NetWin::alloc_slots() {
}

void __fastcall net_win_unk1_redirect(NetWin *self, void *) {
    self->UNK1();
}

void __fastcall net_win_unk2_redirect(NetWin *self, void *) {
    self->UNK2();
}

void __fastcall net_win_alloc_slots_redirect(NetWin *self, void *) {
    self->alloc_slots();
}

/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns 1.
// ORIGINAL: 0x0047B750 ?on_key_click@NetWin@@QAEHHH@Z 0x0047B750-0x0047B758 BYTE_EXACT
// size      8 bytes
// prototype int (__thiscall ?on_key_click@NetWin@@QAEHHH@Z)(NetWin* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: 1, always
Status: Complete
*/
int NetWin::on_key_click(int, int) {
    return 1;
}

int __fastcall net_win_on_key_click_redirect(NetWin *self, void *, int a1, int a2) {
    return self->on_key_click(a1, a2);
}

/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
// ORIGINAL: 0x0047F940 ?on_mouse_move@NetWin@@QAEXHH@Z 0x0047F940-0x0047F943 BYTE_EXACT
// size      3 bytes
// prototype void (__thiscall ?on_mouse_move@NetWin@@QAEXHH@Z)(NetWin* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void NetWin::on_mouse_move(int, int) {
}

void __fastcall net_win_on_mouse_move_redirect(NetWin *self, void *, int a1, int a2) {
    self->on_mouse_move(a1, a2);
}

/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
// ORIGINAL: 0x0047F950 ?on_mouse_leave@NetWin@@QAEXHH@Z 0x0047F950-0x0047F953 BYTE_EXACT
// size      3 bytes
// prototype void (__thiscall ?on_mouse_leave@NetWin@@QAEXHH@Z)(NetWin* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void NetWin::on_mouse_leave(int, int) {
}

void __fastcall net_win_on_mouse_leave_redirect(NetWin *self, void *, int a1, int a2) {
    self->on_mouse_leave(a1, a2);
}

/*
Purpose: Clear the dword at 0x178 and write a five-byte pattern over the start
         of the object.

             mov [eax+0x178],ecx   ; ecx is zero
             mov byte [eax],0xFF   / [eax+1],0 / [eax+2],0
             mov byte [eax+3],0xFF / [eax+4],2

         THE FIRST FIVE BYTES ARE INSIDE THE GraphicWin BASE, and that was
         checked rather than assumed. ??0NetWin@@QAE@XZ does `mov esi,ecx` then
         calls GraphicWin's constructor with ecx UNADJUSTED, which by the
         base-versus-member rule in AGENTS.md makes GraphicWin a base at offset
         zero - its Spot and StringBox members follow at 0xD34 and 0xD40, on
         `this` plus an offset. The modelled inheritance is right.

         So this really does write over the object's first five bytes, which
         the model attributes to Win's leading AutoSound. 0xFF,00,00,0xFF then
         2 reads like a colour and a mode rather than a pointer, and that
         appearance is what made this look like a modelling error; it is not.
         The behaviour is transcribed as the original has it, byte for byte and
         in the original's order - 0x178 first - rather than corrected to what
         it seems like it ought to be.
// ORIGINAL: 0x00483820 ?UNK5@NetWin@@QAEXXZ 0x00483820-0x0048383C
// size      28 bytes
// prototype void (__thiscall ?UNK5@NetWin@@QAEXXZ)(NetWin* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void NetWin::UNK5() {
    uint8_t *const self = reinterpret_cast<uint8_t *>(this);
    *reinterpret_cast<uint32_t *>(self + 0x178) = 0;
    self[0] = 0xFF;
    self[1] = 0;
    self[2] = 0;
    self[3] = 0xFF;
    self[4] = 2;
}

void __fastcall net_win_unk5_redirect(NetWin *self, void *) {
    self->UNK5();
}


// ---------------------------------------------------------------------------
// DEFINED IN THE HEADER, CLAIMED HERE.
//
// These pieces are written in-class so the image's own inlining reproduces -
// a constructor or destructor the compiler is expected to fold into its
// caller. A marker cannot live beside them: `decomp.reader` globs `*.cpp`
// and `*.c`, and a comparison compiles a TRANSLATION UNIT, so a marker in a
// header could be neither read nor measured. VC6 emits each of them into
// this unit's object as its own COMDAT anyway, which is what the comparison
// pulls out, and the `body` fact says where to go to edit one.
//
// The ratchet still covers the header: this unit includes it, so breaking an
// in-class body here fails the claim below. Measured, not assumed.
// ---------------------------------------------------------------------------

/*
// ORIGINAL: 0x00481C50 ??0NetWin@@QAE@XZ 0x00481C50-0x00481D08;0x00657491-0x006574E9
// size      272 bytes
// prototype void (__thiscall ??0NetWin@@QAE@XZ)(NetWin* this)
// callers   1   call targets   6
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005D4CF0 0x005FA860 0x00607CF0 0x0060E670 0x00614E50 0x00629110
*/

// The six subobjects (GraphicWin base, then spot_/stringBox_/editBox_/
// checkBox_/flatButton1_/flatButton2_, in declaration order) construct via
// the implicit base+member-init sequence; 0x005D4CF0 is `?construct@
// GraphicWin@@QAEXXZ`, a construct() method (see basebutton.cpp's own
// `?construct@BaseButton@@QAEXXZ`), which the trivial `GraphicWin() { ; }`
// base ctor cannot reach implicitly, so it is called explicitly here.
//
// RULED-OUT: byte-exact is not reachable from this marker alone.
// StringBox()/CheckBox() are still trivial stubs (0x00629110/0x0060E670
// unfilled) and EditBox() has no declared constructor at all - none of the
// three are among this pass's seven targets, so the image's calls to them
// cannot be reproduced from here. CheckBox's own call also pushes an extra
// `1` (a virtually-inherited GraphicWin's hidden most-derived flag, added
// automatically by the compiler once CheckBox::CheckBox() takes it - not
// reachable while checkBox_'s constructor is the current no-arg stub).
NetWin::NetWin() {
    volatile uint32_t *const object =
        reinterpret_cast<volatile uint32_t *>(this);
    object[0x000 / 4] = 0x0066CCEC;
    object[0x444 / 4] = 0x0066CCE4;
    object[0x772C / 4] = 0;
    object[0x7730 / 4] = 0;
}
