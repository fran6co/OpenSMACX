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
#include "squarelock.h"

/*
Purpose: Reset the square lock to its unset sentinels.
Original Offset: 0058FD70
Return Value: n/a
Status: Complete
*/
void SquareLock::clear() {
    flag_ = 0;
    second_ = -1;
    first_ = -1;
}

void __fastcall square_lock_clear_redirect(SquareLock *self, void *) {
    self->clear();
}
