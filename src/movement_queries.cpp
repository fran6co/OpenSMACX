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
// Endpoint movement and territory queries. Original translation-unit filename is unknown.
#include "stdafx.h"
#include "general.h"
#include "alpha.h"
#include "map.h"
#include "veh.h"

/*
Purpose: Determine whether specified unit can physically reach the destination coordinates.
// ORIGINAL: 0x0056B320 ?get_there@@YAHHHH@Z 0x0056B320-0x0056B47D
// size      349 bytes
// prototype int (__cdecl ?get_there@@YAHHHH@Z)(int vehID, int xCoord, int yCoord)
// callers   1   call targets   5
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x004E3A50 0x0050DDC0 0x0050DE50 0x0050E030 0x0050E160
// TRIED: best 17/147, 0.062 similar (best flag set /O2 /Gy /GR- /Oy-
//            /GX) - `call_diff` shows this tree making 6 calls against the
//            image's 7, missing exactly `port_to_port` (0x0050E160). Its
//            marker (0x0050E160, map.h) already notes the image keeps a REAL
//            call to port_to_port here (and in veh.cpp's valid_patrol) while
//            inlining it whole at naval_base/convoy - a genuine per-call-site
//            split the image itself makes. `port_to_port` is `MEASURED
//            inline` in map.h because THAT lever is what fixed naval_base and
//            convoy; at get_there's call site VC6 chooses to inline it too
//            (confirmed by dumping this tree's own compiled listing - no
//            `call` opcode for it, and the prologue reserves `sub esp, 0x10`
//            where the image's is a single `push ecx`, consistent with the
//            inlined body's extra locals). VC6 6.0 has no
//            `__declspec(noinline)` to force the split per call site, and
//            reverting port_to_port to non-inline would regress the
//            already-recorded naval_base/convoy fixes - out of this batch's
//            reach without touching shared map.h behavior other claims
//            depend on. Not chased further.
Return Value: Can unit reach tile? true/false
Status: Complete
*/
BOOL __cdecl get_there(int veh_id, int x_dst, int y_dst) {
    uint32_t triad = get_triad(veh_id);
    if (triad == TRIAD_AIR) {
        return true;
    }
    int x_src = Vehs[veh_id].x; 
    int y_src = Vehs[veh_id].y;
    uint32_t region_src = region_at(x_src, y_src);
    uint32_t region_dst = region_at(x_dst, y_dst);
    if (!triad) { // TRIAD_LAND
        return (region_src == region_dst);
    }
    int base_id_src = base_at(x_src, y_src);
    int base_id_dst = base_at(x_dst, y_dst);
    if (base_id_dst >= 0) {
        return (base_id_src < 0) ? base_on_sea(base_id_dst, region_src)
            : port_to_port(base_id_src, base_id_dst);
    }
    if (is_ocean(x_dst, y_dst)) {
        return (base_id_src < 0) ? (region_src == region_dst) 
            : base_on_sea(base_id_src, region_dst);
    }
    return (base_id_src < 0) ? sea_coast(region_dst, region_src)
        : port_to_coast(base_id_src, region_dst);
}

/*
Purpose: Return the first neighboring tile index that satisfies the coast/border checks, or 0.
         Earlier recovery prose treated the point-B comparison and integer result as unnecessary;
         the pinned function reads both regions and returns the successful neighbor index.
// ORIGINAL: 0x0056B480 ?coast_or_border@@YAHHHHHH@Z 0x0056B480-0x0056B5A9 BYTE_EXACT
// size      297 bytes
// prototype int (__cdecl ?coast_or_border@@YAHHHHHH@Z)(int xCoordPtA, int yCoordPtA, int xCoordPtB, int yCoordPtB, int factionID)
// callers   1   call targets   1
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x004E3EF0
Return Value: First qualifying neighbor index (1..8), or zero if none qualifies.
Status: Complete
*/
// Historical TRIED hypothesis below is contradicted by the pinned assembly: it checks ownership
// before rereading point A's region.
// TRIED: best 7/110, 0.826 similar (best flag set /O2 /Gy /GR- /GX; call
//            count already matches the image at 2, both to whose_territory).
//            The remaining gap looks like the `region_a != region_b` OR-term
//            being evaluated ahead of the `whose_territory(...)` call in the
//            image (a cheap loop-invariant compare scheduled before an
//            expensive call) - swapping the source's OR order to match
//            scored WORSE (0.780), so that is not the lever. Same
//            xrange/on_map/is_ocean-loop register-scheduling plateau as the
//            rest of this file otherwise. Not chased further.
int __cdecl coast_or_border(int x_point_a, int y_point_a, int x_point_b,
                            int y_point_b, int faction_id) {
    if (whose_territory(faction_id, x_point_a, y_point_a, NULL, false) != (int)faction_id) {
        return 0; // faction doesn't control point A
    }
    uint32_t region_b = region_at(x_point_b, y_point_b);
    for (int i = 1; i < 9; i++) {
        int x_radius = xrange(x_point_a + RadiusOffsetX[i]);
        int y_radius = y_point_a + RadiusOffsetY[i];
        if (on_map(x_radius, y_radius) && (is_ocean(x_radius, y_radius)
                || whose_territory(faction_id, x_radius, y_radius, NULL, false) != (int)faction_id
                || region_at(x_point_a, y_point_a) != region_b)) {
            return i;
        }
    }
    return 0;
}


