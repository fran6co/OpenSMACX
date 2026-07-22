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
#include "worldwin.h"

/*
Purpose: Unknown; the legacy implementation is a bare return with no body.
Original Offset: 004C45E0
Return Value: n/a
Status: Complete
*/
void WorldWin::clear_terrain() {
}

void __fastcall world_win_clear_terrain_redirect(WorldWin *self, void *) {
    self->clear_terrain();
}

/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
Original Offset: 004C3E90
Return Value: n/a
Status: Complete
*/
void WorldWin::on_left_double_click(int, int) {
}

void __fastcall world_win_on_left_double_click_redirect(WorldWin *self, void *, int a1, int a2) {
    self->on_left_double_click(a1, a2);
}
