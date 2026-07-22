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

/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
Original Offset: 00422E10
Return Value: n/a
Status: Complete
*/
void BattleWin::on_iface_left_click(int, int) {
}

void __fastcall battle_win_on_iface_left_click_redirect(BattleWin *self, void *, int a1, int a2) {
    self->on_iface_left_click(a1, a2);
}

/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
Original Offset: 00422E20
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
Original Offset: 00422E70
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
Original Offset: 00422E80
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
Original Offset: 00422E90
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
Original Offset: 00422EA0
Return Value: n/a
Status: Complete
*/
void BattleWin::on_iface_selected(int, int) {
}

void __fastcall battle_win_on_iface_selected_redirect(BattleWin *self, void *, int a1, int a2) {
    self->on_iface_selected(a1, a2);
}
