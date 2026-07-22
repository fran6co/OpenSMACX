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
Original Offset: 0047F920
Return Value: n/a
Status: Complete
*/
void NetWin::UNK1() {
}

/*
Purpose: Unknown; the legacy implementation is a bare return with no body.
Original Offset: 0047F930
Return Value: n/a
Status: Complete
*/
void NetWin::UNK2() {
}

/*
Purpose: Unknown; the legacy implementation is a bare return with no body.
Original Offset: 004834D0
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
Original Offset: 0047B750
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
Original Offset: 0047F940
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
Original Offset: 0047F950
Return Value: n/a
Status: Complete
*/
void NetWin::on_mouse_leave(int, int) {
}

void __fastcall net_win_on_mouse_leave_redirect(NetWin *self, void *, int a1, int a2) {
    self->on_mouse_leave(a1, a2);
}
