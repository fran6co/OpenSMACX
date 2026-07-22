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

/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
Original Offset: 00489B10
Return Value: n/a
Status: Complete
*/
void PickWin::on_button_passover(int, int) {
}

void __fastcall pick_win_on_button_passover_redirect(PickWin *self, void *, int a1, int a2) {
    self->on_button_passover(a1, a2);
}
