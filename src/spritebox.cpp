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
#include "spritebox.h"

/*
Purpose: Unknown; the legacy implementation is a constant return that returns.
Original Offset: 00611040
Return Value: n/a
Status: Complete
*/
void SpriteBox::on_mouse_move(int, int) {
}

/*
Purpose: Unknown; the legacy implementation is a constant return that returns.
Original Offset: 00611050
Return Value: n/a
Status: Complete
*/
void SpriteBox::on_mouse_leave(int, int) {
}

void __fastcall sprite_box_on_mouse_move_redirect(SpriteBox *self, void *, int a1, int a2) {
    self->on_mouse_move(a1, a2);
}

void __fastcall sprite_box_on_mouse_leave_redirect(SpriteBox *self, void *, int a1, int a2) {
    self->on_mouse_leave(a1, a2);
}
