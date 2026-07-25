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
#include "planwin.h"
#include "mapwin.h"

/*
Purpose: Clear the plan window's line count.
Original Offset: 0048AF20
Return Value: n/a
Status: Complete
*/
void PlanWin::clear_lines() {
    field_21FF8_ = 0;
}

void __fastcall plan_win_clear_lines_redirect(PlanWin *self, void *) {
    self->clear_lines();
}

/*
Purpose: Close the plan window by dropping its blink state and running the
         close it inherits from MapWin.
Original Offset: 0048BC50
Return Value: n/a
Status: Complete

The original clears the field and tail-jumps into MapWin::close with `this`
untouched, which is a plain base-class call: the MapWin subobject opens a
PlanWin at offset 0. It cannot be spelled as inheritance here because MapWin
holds its virtual base as a member at MapWin's own 0x21A6C, while a PlanWin
puts that base at 0x22050 - but MapWin::close never uses the member, reaching
the base through the vbtable instead, so it reads the right one either way.
*/
void PlanWin::close() {
    field_21A68_ = 0;
    reinterpret_cast<MapWin *>(this)->close();
}

void __fastcall plan_win_close_redirect(PlanWin *self, void *) {
    self->close();
}
