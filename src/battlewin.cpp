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
ORIGINAL: 0x00422E10 BYTE_EXACT
// name      ?on_iface_left_click@BattleWin@@QAEXHH@Z
// size      3 bytes
// spans     0x00422E10-0x00422E13
// prototype void (__thiscall ?on_iface_left_click@BattleWin@@QAEXHH@Z)(BattleWin* this, int, int)
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void BattleWin::on_iface_left_click(int, int) {
}

/*
Purpose: Destroy the window. The whole teardown is the Time member at +8; the
         original advances `this` by 8 and tail-jumps into the Time
         destructor, and nothing else in the object is touched.
ORIGINAL: 0x00422ED0
// name      ??1BattleWin@@QAE@XZ
// size      8 bytes
// spans     0x00422ED0-0x00422ED8
// prototype void (__thiscall ??1BattleWin@@QAE@XZ)(BattleWin* this)
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
BattleWin::~BattleWin() {
    reinterpret_cast<Time *>(reinterpret_cast<uint8_t *>(this) + 8)->~Time();
}

void __fastcall battle_win_dtor_redirect(BattleWin *self, void *) {
    self->~BattleWin();
}

void __fastcall battle_win_on_iface_left_click_redirect(BattleWin *self, void *, int a1, int a2) {
    self->on_iface_left_click(a1, a2);
}

/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
ORIGINAL: 0x00422E20 BYTE_EXACT
// name      ?on_iface_right_click@BattleWin@@QAEXHH@Z
// size      3 bytes
// spans     0x00422E20-0x00422E23
// prototype void (__thiscall ?on_iface_right_click@BattleWin@@QAEXHH@Z)(BattleWin* this, int, int)
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void BattleWin::on_iface_right_click(int, int) {
}

void __fastcall battle_win_on_iface_right_click_redirect(BattleWin *self, void *, int a1, int a2) {
    self->on_iface_right_click(a1, a2);
}

/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
ORIGINAL: 0x00422E70 BYTE_EXACT
// name      ?on_iface_right_down@BattleWin@@QAEXHH@Z
// size      3 bytes
// spans     0x00422E70-0x00422E73
// prototype void (__thiscall ?on_iface_right_down@BattleWin@@QAEXHH@Z)(BattleWin* this, int, int)
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void BattleWin::on_iface_right_down(int, int) {
}

void __fastcall battle_win_on_iface_right_down_redirect(BattleWin *self, void *, int a1, int a2) {
    self->on_iface_right_down(a1, a2);
}

/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
ORIGINAL: 0x00422E80 BYTE_EXACT
// name      ?on_iface_mouse_move@BattleWin@@QAEXHH@Z
// size      3 bytes
// spans     0x00422E80-0x00422E83
// prototype void (__thiscall ?on_iface_mouse_move@BattleWin@@QAEXHH@Z)(BattleWin* this, int, int)
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void BattleWin::on_iface_mouse_move(int, int) {
}

void __fastcall battle_win_on_iface_mouse_move_redirect(BattleWin *self, void *, int a1, int a2) {
    self->on_iface_mouse_move(a1, a2);
}

/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
ORIGINAL: 0x00422E90 BYTE_EXACT
// name      ?on_iface_mouse_leave@BattleWin@@QAEXHH@Z
// size      3 bytes
// spans     0x00422E90-0x00422E93
// prototype void (__thiscall ?on_iface_mouse_leave@BattleWin@@QAEXHH@Z)(BattleWin* this, int, int)
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void BattleWin::on_iface_mouse_leave(int, int) {
}

void __fastcall battle_win_on_iface_mouse_leave_redirect(BattleWin *self, void *, int a1, int a2) {
    self->on_iface_mouse_leave(a1, a2);
}

/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
ORIGINAL: 0x00422EA0 BYTE_EXACT
// name      ?on_iface_selected@BattleWin@@QAEXHH@Z
// size      3 bytes
// spans     0x00422EA0-0x00422EA3
// prototype void (__thiscall ?on_iface_selected@BattleWin@@QAEXHH@Z)(BattleWin* this, int, int)
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void BattleWin::on_iface_selected(int, int) {
}

void __fastcall battle_win_on_iface_selected_redirect(BattleWin *self, void *, int a1, int a2) {
    self->on_iface_selected(a1, a2);
}

/*
ORIGINAL: 0x00422DC0 BYTE_EXACT
// name      ?on_iface_button_clicked@BattleWin@@QAEXH@Z
// size      26 bytes
// spans     0x00422DC0-0x00422DDA
// prototype void (__thiscall ?on_iface_button_clicked@BattleWin@@QAEXH@Z)(BattleWin* this, int)
// callers   0   call targets   2
Status: Complete
*/
void BattleWin::on_iface_button_clicked(int a1) {
    reinterpret_cast<SubInterface*>(this)->release_iface_mode();
    reinterpret_cast<MainInterface*>(0x7af234)->on_iface_button_clicked(a1);
}
