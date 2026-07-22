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
#include "newtechwin.h"

/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
Original Offset: 00483E90
Return Value: n/a
Status: Complete
*/
void NewTechWin::on_left_click(int, int) {
}

void __fastcall new_tech_win_on_left_click_redirect(NewTechWin *self, void *, int a1, int a2) {
    self->on_left_click(a1, a2);
}

/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns 1.
Original Offset: 00483EA0
Return Value: 1, always
Status: Complete
*/
int NewTechWin::on_key_click(int, int) {
    return 1;
}

int __fastcall new_tech_win_on_key_click_redirect(NewTechWin *self, void *, int a1, int a2) {
    return self->on_key_click(a1, a2);
}
