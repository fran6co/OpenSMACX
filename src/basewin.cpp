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
#include "basewin.h"

/*
Purpose: Unknown; the legacy implementation is a bare return with no body.
Original Offset: 00408710
Return Value: n/a
Status: Complete
*/
void BaseWin::close() {
}

/*
Purpose: Unknown; the legacy implementation is a bare return with no body.
Original Offset: 00416DD0
Return Value: n/a
Status: Complete
*/
void BaseWin::UNK4() {
}

/*
Purpose: Unknown; the legacy implementation is a bare return with no body.
Original Offset: 0041AF20
Return Value: n/a
Status: Complete
*/
void BaseWin::UNK6() {
}

/*
Purpose: Unknown; the legacy implementation is a bare return with no body.
Original Offset: 0041AF30
Return Value: n/a
Status: Complete
*/
void BaseWin::UNK7() {
}

void __fastcall base_win_close_redirect(BaseWin *self, void *) {
    self->close();
}

void __fastcall base_win_unk4_redirect(BaseWin *self, void *) {
    self->UNK4();
}

void __fastcall base_win_unk6_redirect(BaseWin *self, void *) {
    self->UNK6();
}

void __fastcall base_win_unk7_redirect(BaseWin *self, void *) {
    self->UNK7();
}
