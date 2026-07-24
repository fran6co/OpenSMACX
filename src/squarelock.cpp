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
#include "map.h"

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

/*
Purpose: Release the square this lock holds and every tile in its footprint.
         Does nothing when the lock bit (0x1) is clear. When the stored
         coordinate is off the map the record is simply cleared; otherwise
         unlock_map is called on each tile of the footprint - a single tile,
         or, when the 0x4 flag is set without the 0x10 flag, a radius of 25
         tiles (81 when the 0x8 flag is also set) walked through the shared
         RadiusOffset tables and wrapped in x by xrange. The record is reset to
         its unset sentinels on the way out.
Original Offset: 0058FD90
Return Value: n/a
Status: Complete
*/
void SquareLock::unlock(int factionID) {
    if (!(flag_ & 1)) {
        return;
    }
    const int x = first_;
    const int y = second_;
    if (y >= 0 && y < *MapLatitudeBounds && x >= 0 && x < *MapLongitudeBounds) {
        int count;
        if ((flag_ & 4) && !(flag_ & 0x10)) {
            count = (flag_ & 8) ? 81 : 25;   // RadiusRange[4] or RadiusRange[2]
        } else {
            count = 1;
        }
        for (int i = 0; i < count; ++i) {
            const int nx = xrange(RadiusOffsetX[i] + first_);
            const int ny = RadiusOffsetY[i] + second_;
            if (ny >= 0 && ny < *MapLatitudeBounds &&
                nx >= 0 && nx < *MapLongitudeBounds) {
                unlock_map(nx, ny, factionID);
            }
        }
    }
    flag_ = 0;
    second_ = -1;
    first_ = -1;
}

void __fastcall square_lock_unlock_redirect(SquareLock *self, void *,
                                            int factionID) {
    self->unlock(factionID);
}
