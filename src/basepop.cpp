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
#include "basepop.h"

/*
Purpose: Update either base-popup coordinate unless its keep-current sentinel is supplied.
Original Offset: 00601B80
Return Value: n/a
Status: Complete
*/
void BasePop::set_loc(int x, int y) {
    if (x != 0x2000) {
        loc_a_ = x;
    }
    if (y != 0x2000) {
        loc_b_ = y;
    }
}

void __fastcall base_pop_set_loc_redirect(BasePop *self, void *, int x, int y) {
    self->set_loc(x, y);
}
