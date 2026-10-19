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
#ifndef OPENSMACX_MAP_COORDINATES_H
#define OPENSMACX_MAP_COORDINATES_H

#include "map.h"

extern uint32_t WorldCanyonOffsetValues[12];
extern uint32_t world_ridge_offset_val[13];

/*
Purpose: Wrap an x coordinate on a round map, as world_site() and good_sensor() do it.
Original Offset: n/a
Return Value: Wrapped x
Status: Complete

xrange() at 0048BEE0 cannot stand in, for one bit. It tests the whole of
MapIsFlat while these two read the low BYTE and test bit zero
(`mov cl, byte ptr [94988Ch] / test cl, 1`). The two agree on the 0 and 1 the
game stores there and disagree on everything else, and the difference is in the
original rather than in the transcription. game.cpp's territory_xrange() carries
the same distinction for reset_territory().
*/
__forceinline static int site_xrange(int x) {
    if (!(MapIsFlat & 1)) {
        if (x >= 0) {
            if (x >= MapLongitudeBounds) {
                x -= MapLongitudeBounds;
            }
        } else {
            x += MapLongitudeBounds;
        }
    }
    return x;
}

/*
Purpose: Address the map tile world_site() scores.
Original Offset: n/a
Return Value: Pointer to map tile
Status: Complete

map_loc() cannot stand in. It takes unsigned coordinates and therefore shifts x
logically, while world_site() inlines this with `sar` on x - and its centre
tile, unlike the twenty-eight and twenty-one radius tiles, is never bounds
checked, so a negative x reaches the shift.
*/
__forceinline static Map *site_tile(int x, int y) {
    return &map_tiles()[(x >> 1) + y * (int)MapLongitude];
}

#endif // OPENSMACX_MAP_COORDINATES_H
