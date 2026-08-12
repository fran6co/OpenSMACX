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
#include "credits.h"

/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
ORIGINAL: 0x00428D80 BYTE_EXACT
Return Value: n/a
Status: Complete
*/
void Credits::on_left_down(int, int) {
}

void __fastcall credits_on_left_down_redirect(Credits *self, void *, int a1, int a2) {
    self->on_left_down(a1, a2);
}

/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
ORIGINAL: 0x00428D90 BYTE_EXACT
Return Value: n/a
Status: Complete
*/
void Credits::on_right_down(int, int) {
}

void __fastcall credits_on_right_down_redirect(Credits *self, void *, int a1, int a2) {
    self->on_right_down(a1, a2);
}

/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
ORIGINAL: 0x00428DA0 BYTE_EXACT
Return Value: n/a
Status: Complete
*/
void Credits::on_mouse_move(int, int) {
}

void __fastcall credits_on_mouse_move_redirect(Credits *self, void *, int a1, int a2) {
    self->on_mouse_move(a1, a2);
}

/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
ORIGINAL: 0x00428DB0 BYTE_EXACT
Return Value: n/a
Status: Complete
*/
void Credits::on_button_clicked(int) {
}

void __fastcall credits_on_button_clicked_redirect(Credits *self, void *, int a1) {
    self->on_button_clicked(a1);
}
