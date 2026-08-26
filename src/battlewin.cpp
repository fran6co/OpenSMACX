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
#include "battlewin.h"
#include "maininterface.h"
#include "subinterface.h"
#include "time.h"

/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
// ORIGINAL: 0x00422E10 ?on_iface_left_click@BattleWin@@QAEXHH@Z 0x00422E10-0x00422E13 BYTE_EXACT
// size      3 bytes
// prototype void (__thiscall ?on_iface_left_click@BattleWin@@QAEXHH@Z)(BattleWin* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void BattleWin::on_iface_left_click(int, int) {
}

/*
Purpose: Destroy the window. The whole teardown is the Time member at +8; the
         original advances `this` by 8 and tail-jumps into the Time
         destructor, and nothing else in the object is touched.
// ORIGINAL: 0x00422ED0 ??1BattleWin@@QAE@XZ 0x00422ED0-0x00422ED8 BYTE_EXACT
// size      8 bytes
// prototype void (__thiscall ??1BattleWin@@QAE@XZ)(BattleWin* this)
// callers   0   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
BattleWin::~BattleWin() {
    reinterpret_cast<Time *>(reinterpret_cast<uint8_t *>(this) + 8)->~Time();
}



/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
// ORIGINAL: 0x00422E20 ?on_iface_right_click@BattleWin@@QAEXHH@Z 0x00422E20-0x00422E23 BYTE_EXACT
// size      3 bytes
// prototype void (__thiscall ?on_iface_right_click@BattleWin@@QAEXHH@Z)(BattleWin* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void BattleWin::on_iface_right_click(int, int) {
}


/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
// ORIGINAL: 0x00422E70 ?on_iface_right_down@BattleWin@@QAEXHH@Z 0x00422E70-0x00422E73 BYTE_EXACT
// size      3 bytes
// prototype void (__thiscall ?on_iface_right_down@BattleWin@@QAEXHH@Z)(BattleWin* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void BattleWin::on_iface_right_down(int, int) {
}


/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
// ORIGINAL: 0x00422E80 ?on_iface_mouse_move@BattleWin@@QAEXHH@Z 0x00422E80-0x00422E83 BYTE_EXACT
// size      3 bytes
// prototype void (__thiscall ?on_iface_mouse_move@BattleWin@@QAEXHH@Z)(BattleWin* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void BattleWin::on_iface_mouse_move(int, int) {
}


/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
// ORIGINAL: 0x00422E90 ?on_iface_mouse_leave@BattleWin@@QAEXHH@Z 0x00422E90-0x00422E93 BYTE_EXACT
// size      3 bytes
// prototype void (__thiscall ?on_iface_mouse_leave@BattleWin@@QAEXHH@Z)(BattleWin* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void BattleWin::on_iface_mouse_leave(int, int) {
}


/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
// ORIGINAL: 0x00422EA0 ?on_iface_selected@BattleWin@@QAEXHH@Z 0x00422EA0-0x00422EA3 BYTE_EXACT
// size      3 bytes
// prototype void (__thiscall ?on_iface_selected@BattleWin@@QAEXHH@Z)(BattleWin* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void BattleWin::on_iface_selected(int, int) {
}


/*
// ORIGINAL: 0x00422DC0 ?on_iface_button_clicked@BattleWin@@QAEXH@Z 0x00422DC0-0x00422DDA BYTE_EXACT
// size      26 bytes
// prototype void (__thiscall ?on_iface_button_clicked@BattleWin@@QAEXH@Z)(BattleWin* this, int)
// callers   0   call targets   2
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x0045D170 0x0045D380
Status: Complete
*/
void BattleWin::on_iface_button_clicked(int a1) {
    reinterpret_cast<SubInterface*>(this)->release_iface_mode();
    reinterpret_cast<MainInterface*>(0x7af234)->on_iface_button_clicked(a1);
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
// ORIGINAL: 0x00422EE0 ??0BattleWin@@QAE@XZ 0x00422EE0-0x00422EFB BYTE_EXACT
// body      src/battlewin.cpp
// size      27 bytes
// prototype void (__thiscall ??0BattleWin@@QAE@XZ)(BattleWin* this)
// callers   1   call targets   1
// kind      game
// flags     sp_ready;purged_ok
// calls     0x006161D0
*/
// THE BASE VTABLE FIRST, THE DERIVED ONE LAST, with the Time member at +8
// constructed in between - the image computes the Time receiver
// (`lea ecx, [esi + 8]`) before either store. Time is held as storage in
// battlewin.h, so the construction is an explicit qualified call, the idiom
// basebutton.cpp's TRIED notes score against placement new's null guards.
BattleWin::BattleWin() {
    Time *const time = reinterpret_cast<Time *>(time_storage_);
    vtable_ = 0x0066A6E4;
    time->Time::Time();
    vtable_ = 0x0066AA44;
}
