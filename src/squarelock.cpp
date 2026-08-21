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
// ORIGINAL: 0x0058FD70 ?clear@SquareLock@@QAEXXZ 0x0058FD70-0x0058FD85 BYTE_EXACT
// size      21 bytes
// prototype void (__thiscall ?clear@SquareLock@@QAEXXZ)(SquareLock* this)
// callers   0   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void SquareLock::clear() {
    flag = 0;
    second = -1;
    first = -1;
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
// ORIGINAL: 0x0058FD90 ?unlock@SquareLock@@QAEXH@Z 0x0058FD90-0x0058FE77 BYTE_EXACT
// size      231 bytes
// prototype void (__thiscall ?unlock@SquareLock@@QAEXH@Z)(SquareLock* this, int)
// callers   4   call targets   1
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x00591CF0
Return Value: n/a
Status: Complete
*/
void SquareLock::unlock(int factionID) {
    if (!(flag & 1)) {
        return;
    }
    const int x = first;
    const int y = second;
    if (y >= 0 && y < MapLatitudeBounds && x >= 0 && x < MapLongitudeBounds) {
        int count;
        if ((flag & 4) && !(flag & 0x10)) {
            count = (flag & 8) ? 81 : 25;   // RadiusRange[4] or RadiusRange[2]
        } else {
            count = 1;
        }
        for (int i = 0; i < count; ++i) {
            const int nx = xrange(RadiusOffsetX[i] + first);
            const int ny = RadiusOffsetY[i] + second;
            if (ny >= 0 && ny < MapLatitudeBounds &&
                nx >= 0 && nx < MapLongitudeBounds) {
                unlock_map(nx, ny, factionID);
            }
        }
    }
    flag = 0;
    second = -1;
    first = -1;
}

void __fastcall square_lock_unlock_redirect(SquareLock *self, void *,
                                            int factionID) {
    self->unlock(factionID);
}

/*
Purpose: Take a square and every tile in its footprint for a faction. The
         coordinate and the flags (with the lock bit 0x1 forced on) are stored
         in the record first, unconditionally. An off-map coordinate then stops
         there. Otherwise lock_map is called on each footprint tile - a single
         tile, or a radius of 25 (0x4 set, 0x10 clear) or 81 (also 0x8 set)
         tiles walked through the shared RadiusOffset tables and wrapped in x by
         xrange; if any tile is already held by another faction the attempt is
         abandoned at that tile.
// ORIGINAL: 0x0058FE80 ?lock@SquareLock@@QAEHHHHH@Z 0x0058FE80-0x0058FF68
// size      232 bytes
// prototype BOOL (__thiscall ?lock@SquareLock@@QAEHHHHH@Z)(SquareLock* this, int factionID, int flags, int xCoord, int yCoord)
// callers   4   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00591C90
Return Value: 1 when a footprint tile is already locked by another faction,
              0 otherwise (including an off-map coordinate)
Status: Complete
*/
int SquareLock::lock(int factionID, int flags, int x, int y) {
    first = x;
    second = y;
    flag = flags | 1;
    if (y < 0 || y >= MapLatitudeBounds || x < 0 || x >= MapLongitudeBounds) {
        return 0;
    }
    int count;
    if ((flag & 4) && !(flag & 0x10)) {
        count = (flag & 8) ? 81 : 25;   // RadiusRange[4] or RadiusRange[2]
    } else {
        count = 1;
    }
    for (int i = 0; i < count; ++i) {
        const int nx = xrange(RadiusOffsetX[i] + first);
        const int ny = RadiusOffsetY[i] + second;
        if (ny >= 0 && ny < MapLatitudeBounds &&
            nx >= 0 && nx < MapLongitudeBounds) {
            if (lock_map(nx, ny, factionID)) {
                return 1;
            }
        }
    }
    return 0;
}

int __fastcall square_lock_lock_redirect(SquareLock *self, void *, int factionID,
                                         int flags, int x, int y) {
    return self->lock(factionID, flags, x, y);
}
