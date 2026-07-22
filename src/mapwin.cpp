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
#include "mapwin.h"

/*
Purpose: Unknown; the legacy implementation is a bare return with no body.
Original Offset: 00467960
Return Value: n/a
Status: Complete
*/
void MapWin::UNK3() {
}

/*
Purpose: Refresh the map's image buttons; the legacy implementation is a bare
         return, so the buttons are never rebuilt here.
Original Offset: 0046FB80
Return Value: n/a
Status: Complete
*/
void MapWin::do_image_buttons() {
}

void __fastcall map_win_unk3_redirect(MapWin *self, void *) {
    self->UNK3();
}

void __fastcall map_win_do_image_buttons_redirect(MapWin *self, void *) {
    self->do_image_buttons();
}
