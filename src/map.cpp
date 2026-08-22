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
#include "temp.h"
#include "map.h"
#include "alpha.h"
#include "base.h"
#include "faction.h"
#include "game.h"
#include "path.h"
#include "random.h"
#include "strings.h"
#include "technology.h"
#include "veh.h"
#include "log.h"
#include "mapwin.h"
#include "worldwin.h"

int MapLongitudeBounds;  // 0x00949870
int MapLatitudeBounds;  // 0x00949874
uint32_t MapRandSeed;  // 0x00949878
int MapSeaLevel;  // 0x0094987C
int MapSeaLevelCouncil;  // 0x00949880
uint32_t MapArea;  // 0x00949884
uint32_t MapAreaSqRoot;  // 0x00949888
BOOL MapIsFlat;  // 0x0094988C
int MapLandmarkCount;  // 0x00949890
int MapAbstractLongBounds;  // 0x0094A294
uint32_t MapAbstractLatBounds;  // 0x0094A298
uint32_t MapAbstractArea;  // 0x0094A29C
uint32_t MapSizePlanet;  // 0x0094A2A0
uint32_t MapOceanCoverage;  // 0x0094A2A4
uint32_t MapLandCoverage;  // 0x0094A2A8
uint32_t MapErosiveForces;  // 0x0094A2AC
uint32_t MapPlanetaryOrbit;  // 0x0094A2B0
uint32_t MapCloudCover;  // 0x0094A2B4
uint32_t MapNativeLifeForms;  // 0x0094A2B8
// EIGHT OF THEM, one per player: the two sites below clear
// `sizeof(int) * MaxPlayerNum` bytes, which is what the extent is.
int MapBaseSubmergedCount[8];  // 0x009B2290
int MapBaseIdClosestSubmergedVeh[8];  // 0x009B22BC
uint32_t BrushVal1;  // 0x009B22B0
uint32_t BrushVal2;  // 0x009B22B8
uint32_t WorldBuildVal1;  // 0x009B22B4

uint32_t MapLongitude;  // 0x0068FAF0 // default set to 1
LPCSTR MapExtension = "MP";

/*
Purpose: Check whether the coordinates are on the map.
// ORIGINAL: 0x004712A0 ?on_map@@YAHHH@Z 0x004712A0-0x004712CC BYTE_EXACT
// size      44 bytes
// prototype BOOL (__cdecl ?on_map@@YAHHH@Z)(int xCoord, int yCoord)
// callers   7   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
Return Value: Are the coordinates on the map? true/false
Status: Complete
*/
// BODY IN map.h, as `MEASURED inline`: the image writes it out at
// some call sites and calls it at others, and a .cpp definition is only ever
// one of those. The marker stays here because that is where the catalogue
// reads it.


/*
Purpose: Bounds check and handling of x coordinate for round maps.
// ORIGINAL: 0x0048BEE0 ?xrange@@YAHH@Z 0x0048BEE0-0x0048BF05 BYTE_EXACT
// LEVER: `!(MapIsFlat & 1)`, not `!MapIsFlat` - the image reads the low BYTE
//   and tests bit zero, matching every other MapIsFlat site in the tree
//   (game.cpp, veh.cpp, terraforming.cpp already do this). Also, the guard's
//   two arms are written `if (x < 0) {...} else if (x >= bounds) {...}`, NOT
//   `if (x >= 0) {...} else {...}` - same condition, but the image's jge
//   falls through to the negative-x arm, which only the swapped polarity
//   reproduces. Fixed xrange itself plus two callers, `site_radius` (this
//   batch) and `SquareLock::unlock` (bonus, out of batch).
// size      37 bytes
// prototype 
// callers   4   call targets   0
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     (none)
Return Value: X coordinate
Status: Complete
*/
// BODY IN map.h, as `MEASURED inline`: the image writes it out at
// some call sites and calls it at others, and a .cpp definition is only ever
// one of those. The marker stays here because that is where the catalogue
// reads it.


/*
Purpose: Check who owns a tile. Optional parameter to get closest base.
// ORIGINAL: 0x004E3EF0 ?whose_territory@@YAHHHHPAHH@Z 0x004E3EF0-0x004E3F9D
// TRIED: hoisting `Map *tile = map_loc(x, y)` before the territory read
//            (compiler already CSEs it, identical bytes); flattening the
//            nested owner==faction_id check into an early-return guard
//            clause. Both plateau at 0.944 sim / 34 of 74 instructions;
//            divergence starts in the prologue (image keeps x in edi, this
//            tree picks ebx) - a pure register-allocation difference.
// size      173 bytes
// prototype int (__cdecl ?whose_territory@@YAHHHHPAHH@Z)(int factionID, int xCoord, int yCoord, int* baseID, int ignoreComm)
// callers   40   call targets   1
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x004E3D50
Return Value: Faction id of the territory's owner or -1 if no owner or unknown faction
Status: Complete
*/
int __cdecl whose_territory(int faction_id, int x, int y, int *base_id, 
                            BOOL ignore_comm) {
    int owner = (int)map_loc(x, y)->territory;
    if (owner <= 0) {
        return -1; // no owner
    }
    if ((uint32_t)owner != faction_id) {
        if (!ignore_comm && !(GameState & STATE_OMNISCIENT_VIEW)
            && has_treaty(faction_id, owner, DTREATY_COMMLINK | DTREATY_UNK_8000000)
                != (DTREATY_COMMLINK | DTREATY_UNK_8000000)) {
            return -1; // owner unknown to faction
        }
        if (base_id) {
            *base_id = base_find(x, y, -1, region_at(x, y), -1, -1);
        }
    }
    return owner;
}

/*
Purpose: Find closest base to territory owned by another faction not at war with specified faction.
// ORIGINAL: 0x004E3FA0 ?base_territory@@YAHHHH@Z 0x004E3FA0-0x004E4017
// size      119 bytes
// prototype int (__cdecl ?base_territory@@YAHHHH@Z)(int factionID, int xCoord, int yCoord)
// callers   2   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x004E3EF0
Return Value: Base id or -1
Status: Complete
*/
// LEVER: `faction_id != (uint32_t)owner`, not `(uint32_t)owner != faction_id`
//        - the image's `cmp esi, eax` (faction_id, owner) matches the
//        operands in that order; took the best flag set (/O2 /Gy /GR- /Oy-
//        /GX) from 42/50 to 43/50, 0.960 similar.
// TRIED: remaining gap is `is_human()` (faction.h) reading
//            `FactionsStatus[0]` as `mov edx, dword ptr [addr]; and edx,
//            0xff` in the image against this tree's `xor edx, edx; mov dl,
//            byte ptr [addr]` - out of this batch's scope (faction.h, not
//            map.cpp/map.h), and identical at both is_human() call sites
//            here, so nothing in base_territory's own source shape can move
//            it. Not chased further.
int __cdecl base_territory(int faction_id, int x, int y) {
    int base_id;
    int owner = whose_territory(faction_id, x, y, &base_id, false);
    if (owner >= 0 && faction_id != (uint32_t)owner && (is_human(faction_id) || is_human(owner))
        && !has_treaty(faction_id, owner, DTREATY_VENDETTA)) {
        return base_id;
    }
    return -1;
}

/*
Purpose: For the specified tile, calculate the quality of the terrain.
// ORIGINAL: 0x004ECB90 ?crappy@@YAHHH@Z 0x004ECB90-0x004ECBE1
// size      81 bytes
// prototype int (__cdecl ?crappy@@YAHHH@Z)(int xCoord, int yCoord)
// callers   0   call targets   0
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     (none)
Return Value: Quality of terrain, lower is better (0-2)
Status: Complete
*/
int __cdecl crappy(int x, int y) {
    uint32_t poor_quality = 0;
    uint32_t rainfall = climate_at(x, y);
    if (rainfall == RAINFALL_ARID) {
        poor_quality = 1; // neither moist or rainy
    }
    uint32_t rocky = rocky_at(x, y);
    if (rocky == TERRAIN_BIT_ROCKY) {
        poor_quality++; // rocky
    } else if (rocky == ROCKINESS_FLAT && rainfall < RAINFALL_RAINY) {
        poor_quality++; // flat, moist or arid
    }
    return poor_quality;
}

/*
Purpose: Take the absolute distance between two points as parameters to calculate how far out they
         radiate. This is mainly used to determine proximity or how far away the two points are from
         each other in a rough circle shape (see RadiusOffsetX[]/RadiusOffsetY[]).
// ORIGINAL: 0x004F8090 ?vector_dist@@YAHHH@Z 0x004F8090-0x004F80CB BYTE_EXACT
// size      59 bytes
// prototype int (__cdecl ?vector_dist@@YAHHH@Z)(int xCoord, int yCoord)
// callers   5   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00644F3A
Return Value: Distance radius
Status: Complete
*/
// BODY IN map.h, as `MEASURED inline`: the image has it as a real function
// at ITS OWN 5 call sites AND inlines it whole at the 4-arg vector_dist's
// call site (0x005A5910) - a .cpp definition can only ever be one of those.

/*
Purpose: Take two points and calculate how far out they radiate. This is mainly used to determine
         proximity or how far away the two points are from each other in a rough circle shape.
// ORIGINAL: 0x005A5910 ?vector_dist@@YAHHHHH@Z 0x005A5910-0x005A5987
// LEVER: `osmx calls` showed 4 abs() calls, 0 game calls in the image; the
//        2-arg `vector_dist` (0x004F8090) was declared/defined out-of-line
//        only, so calling it here compiled a real local call. Moved it to
//        map.h as `MEASURED inline` (same idiom as the many single-field
//        setters in this file) - it stays a real out-of-line BYTE_EXACT
//        function at its own 5 call sites AND inlines whole here, matching
//        the image's call count exactly.
// TRIED: `__forceinline` instead of `inline` - identical score, so the
//            call-count fix, not the inlining strength, was the lever.
//            Remaining gap (14/50 raw, 0.929 similar, best of every flag
//            set) is x_dist's inner subtraction: the image caches
//            x_point_b in edi before subtracting, this tree reads it
//            straight from the stack slot - same register-caching family
//            as base_on_sea/base_coast, not fixable from source shape.
// size      119 bytes
// prototype 
// callers   1   call targets   1
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x00644F3A
Return Value: Distance radius
Status: Complete
*/
int __cdecl vector_dist(int x_point_a, int y_point_a, int x_point_b, int y_point_b) {
    return vector_dist(x_dist(x_point_a, x_point_b), abs(y_point_a - y_point_b));
}

/*
Purpose: Check whether there is a path between two regions. It seems to only take into account land
         destinations. TODO: Revisit in the future when Continent/Path is complete.
// ORIGINAL: 0x0050DDC0 ?sea_coast@@YAHHH@Z 0x0050DDC0-0x0050DE00 BYTE_EXACT
// size      64 bytes
// prototype int (__cdecl ?sea_coast@@YAHHH@Z)(int regionDst, int regionSrc)
// callers   2   call targets   1
// kind      game
// flags     frame;sp_ready
// calls     0x0050BA00
Return Value: Is there a path? true/false
Status: Complete
*/
BOOL __cdecl sea_coast(int region_dst, int region_src) {
    int offset;
    int mask;
    bitmask(region_src & RegionBounds, &offset, &mask);
    return (Continents[region_dst].sea_coasts[offset] & mask) != 0;
}

/*
Purpose: Count the number of paths from the source region. It seems to only take into account land 
         source and destination ranges. TODO: Revisit in the future when Continent/Path is complete.
// ORIGINAL: 0x0050DE00 ?sea_coasts@@YAHH@Z 0x0050DE00-0x0050DE49 BYTE_EXACT
// size      73 bytes
// prototype 
// callers   1   call targets   1
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x0050BA00
Return Value: Sea coasts valid path count
Status: Complete
*/
int __cdecl sea_coasts(int region_src) {
    uint32_t sea_coast_count = 0;
    for (int i = 1; i < RegionBounds; i++) {
        // sea_coast() (0x0050DDC0, BYTE_EXACT as its own out-of-line
        // function) is hand-inlined here: the image writes its body out at
        // this call site (bitmask() is the only call it keeps), rather than
        // calling 0x0050DDC0.
        int offset;
        int mask;
        bitmask_call(region_src & RegionBounds, &offset, &mask);
        if ((Continents[i].sea_coasts[offset] & mask) != 0) {
            sea_coast_count++;
        }
    }
    return sea_coast_count;
}

/*
Purpose: Check to see whether base is within a one tile radius of a sea tile with specified region.
         If you pass a land region (<63) as the 2nd parameter, it is possible to get collision
         behavior due to region bounding. TODO: Revisit in the future to see whether to remove them.
// ORIGINAL: 0x0050DE50 ?base_on_sea@@YAHHH@Z 0x0050DE50-0x0050DF28
// TRIED: --all-flags's winner (23/81, the default set) already beats every other set;
//            first divergence is the prologue's two globals (MapIsFlat at 0x94988c,
//            MapLongitudeBounds at 0x949870) loading into ebx/edi in the opposite order
//            from the image - same register-allocation swap seen on base_coast
//            (0x0050DF30) and the rest of this xrange/on_map-loop family
//            (bonus_at, goody_at already carry the same note).
// size      216 bytes
// prototype int (__cdecl ?base_on_sea@@YAHHH@Z)(int baseID, int region)
// callers   10   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
Return Value: Is base connected to specified sea region? true/false
Status: Complete
*/
BOOL __cdecl base_on_sea(int base_id, int region_sea) {
    region_sea &= RegionBounds;
    if (region_sea >= RegionBounds) { // change to equals since already bounded?
        return false; // skips poles (land or ocean)
    }
    int x = Bases[base_id].x;
    int y = Bases[base_id].y;
    for (uint32_t i = 0; i < 8; i++) {
        int x_radius = xrange(x + RadiusBaseX[i]);
        int y_radius = y + RadiusBaseY[i];
        if (on_map(x_radius, y_radius) && is_ocean(x_radius, y_radius)
            && (region_at(x_radius, y_radius) & RegionBounds) == region_sea) {
            return true;
        }
    }
    return false;
}

/*
Purpose: Determine the ocean region for coastal bases. There is an issue if a base is connected to
         more than one ocean region, it will only return the last checked region based on the base
         radius clockwise order. This could cause the AI to make incorrect calculations for what to
         prioritize building at the base. An example could be a base that is connected to a small
         water body as well as the much larger ocean. Depending on where these bodies of water are
         positioned, the AI might make assumptions not to prioritize building naval units. Also,
         the Continents compare logic isn't used by anything. This might be the root cause of
         outlined bug. TODO: Revisit in the future once more is known about Continent structure.
// ORIGINAL: 0x0050DF30 ?base_coast@@YAHH@Z 0x0050DF30-0x0050E021
// TRIED: 42/81 plateau, same xrange/on_map-loop family as base_on_sea
//            (0x0050DE50). First divergence is inside the inlined `xrange`:
//            the image loads MapLongitudeBounds ONCE and reuses that
//            register for both the `+=` and `-=` wrap arms, this tree
//            reloads it from memory for the `-=` arm, shifting every
//            address after it by 4 bytes and cascading the mismatch
//            through the rest of the loop body (which otherwise agrees,
//            per `listing_diff`: only 6 runs differ across 81
//            instructions). Tried hand-inlining `xrange`/`on_map` instead
//            of calling them - no change, confirming this is `xrange`'s own
//            shared register-allocation ceiling, not something local to
//            this call site.
// size      241 bytes
// prototype int (__cdecl ?base_coast@@YAHH@Z)(int baseID)
// callers   3   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
Return Value: Ocean region or -1 if landlocked
Status: Complete
*/
int __cdecl base_coast(int base_id) {
    int region = -1;
    int val = 0;
    int x = Bases[base_id].x;
    int y = Bases[base_id].y;
    for (uint32_t i = 0; i < 8; i++) { // is_coast()
        int x_radius = xrange(x + RadiusBaseX[i]);
        int y_radius = y + RadiusBaseY[i];
        if (on_map(x_radius, y_radius) && is_ocean(x_radius, y_radius)) {
            region = region_at(x_radius, y_radius);
            int compare = (region >= 127) ? 1 : Continents[region].tile_count;
            if (compare >= val) {
                val = compare; // value isn't used?
            }
            i += (2 - (i & 1)); // skips adjacent tiles
        }
    }
    return region;
}

/*
Purpose: Check to see if a port base shares a common body of water with destination coastal region.
// ORIGINAL: 0x0050E030 ?port_to_coast@@YAHHH@Z 0x0050E030-0x0050E15A
// size      298 bytes
// prototype int (__cdecl ?port_to_coast@@YAHHH@Z)(int baseID, int region)
// callers   2   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x0050BA00
Return Value: Is port and coastal region accessible by water to each other? true/false
Status: Complete
*/
// TRIED: best 77/103, 0.922 similar (best flag set /O2 /Gy /GR- /Oy-
//            /GX), MISMATCH at every flag set. The remaining gaps are all
//            `region_at`'s inlined `map_loc()->region` read: the image
//            schedules `xor ecx, ecx` (or the byte-index add) before loading
//            the map_tiles() pointer, this tree always loads the pointer
//            first - same register-scheduling plateau as alt_at/alt_detail_at
//            (0x00500150/0x00500180) and abstract_at (0x00591210), inherited
//            here because `region_at` is called four times over the body.
//            Not chased further.
BOOL __cdecl port_to_coast(int base_id, int region) {
    int x = Bases[base_id].x;
    int y = Bases[base_id].y;
    if (region_at(x, y) == region) {
        return true;
    }
    for (uint32_t i = 0; i < 8; i++) { // is_coast()
        int x_radius = xrange(x + RadiusBaseX[i]);
        int y_radius = y + RadiusBaseY[i];
        if (on_map(x_radius, y_radius) && is_ocean(x_radius, y_radius)) {
            // sea_coast() (0x0050DDC0, BYTE_EXACT as its own out-of-line
            // function) is hand-inlined here: the image writes its body out
            // at this call site (bitmask() is the only call it keeps),
            // rather than calling 0x0050DDC0.
            int offset;
            int mask;
            bitmask_call(region_at(x_radius, y_radius) & RegionBounds, &offset, &mask);
            if ((Continents[region].sea_coasts[offset] & mask) != 0) {
                return true;
            }
            i += (2 - (i & 1)); // skips adjacent tiles
        }
    }
    return false;
}

/*
Purpose: Check to see if two port bases share a common body of water determined by region.
// ORIGINAL: 0x0050E160 ?port_to_port@@YAHHH@Z 0x0050E160-0x0050E306
// LEVER: call_diff showed this tree calling base_on_sea where the image makes
//        no calls at all - the image inlines base_on_sea's whole body into
//        the loop instead. Manually inlining it (guard clause on
//        `region_sea >= RegionBounds`, then the destination-base radius
//        loop) took this from 5/141 to 12/141 instructions agreeing
//        (0.587 -> 0.785 similarity); still MISMATCH, the remaining gap
//        looks like register/stack-slot allocation (image reserves
//        `sub esp, 0x18`, this tree `0x10`) rather than another structural
//        difference.
// TRIED: the same inline rewritten as early-return guard clauses
//            (`if (!on_map(...)) continue;` etc.) instead of nested `if` -
//            scored identically (12/141), so the remaining gap is not
//            branch polarity.
// size      422 bytes
// prototype int (__cdecl ?port_to_port@@YAHHH@Z)(int baseIDSrc, int baseIDDst)
// callers   7   call targets   0
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     (none)
// LEVER (2026-08-21): moved to map.h as `MEASURED inline`. naval_base (0x0050E3C0) and
//        convoy (0x0050E5C0) call_diff MORE by exactly one call each on port_to_port -
//        the image inlines this whole 422-byte body at both, while still keeping a
//        real call at get_there and veh.cpp's valid_patrol (both already call_diff-clean
//        and stayed that way after this move).
Return Value: Are both ports accessible by water to each other? true/false
Status: Complete
*/
// BODY IN map.h, as `MEASURED inline`: the image writes it out at
// some call sites and calls it at others, and a .cpp definition is only ever
// one of those. The marker stays here because that is where the catalogue
// reads it.

/*
Purpose: Determine if a base has access to ports or more than one coastal region. This helps
         prioritize whether naval transports should be built.
// ORIGINAL: 0x0050E310 ?transport_base@@YAHH@Z 0x0050E310-0x0050E3BD
// LEVER: catalogue lists `sea_coasts` as callers=1, and this wasn't it - the
//        image hand-inlines `sea_coasts`'s whole loop at this call site
//        (same shape as `sea_coasts`'s own comment about `sea_coast()`)
//        instead of calling 0x0050DE00. Went 2/67 -> 42/67 (0.889 similar)
//        inlining that loop instead of calling `sea_coasts(region)`.
// TRIED: the remaining gap is is_ocean's own known plateau - the image
//            loads map_tiles() (0x94a30c) earlier, before the Bases[].x/y
//            reads, than this tree schedules it. Same root cause as
//            is_ocean's standalone note, not something local to this site.
// size      173 bytes
// prototype int (__cdecl ?transport_base@@YAHH@Z)(int baseID)
// callers   1   call targets   2
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x0050BA00 0x0050DF30
Return Value: Should base build naval transports? true/false
Status: Complete
*/
BOOL __cdecl transport_base(int base_id) {
    int region = base_coast(base_id);
    if (region < 0) {
        return false;
    }
    if (is_ocean(Bases[base_id].x, Bases[base_id].y)) {
        return true;
    }
    // `sea_coasts` (0x0050DE00, BYTE_EXACT as its own out-of-line function)
    // is hand-inlined here: the image writes its whole loop body out at this
    // call site (bitmask() is the only call it keeps), rather than calling
    // 0x0050DE00.
    uint32_t sea_coast_count = 0;
    for (int i = 1; i < RegionBounds; i++) {
        int offset;
        int mask;
        bitmask_call(region & RegionBounds, &offset, &mask);
        if ((Continents[i].sea_coasts[offset] & mask) != 0) {
            sea_coast_count++;
        }
    }
    return sea_coast_count > 1;
}

/*
Purpose: Determine if there are other faction's ports in the vicinity of the specified base.
// ORIGINAL: 0x0050E3C0 ?naval_base@@YAHH@Z 0x0050E3C0-0x0050E5BE
// TRIED: call count already matches the image (1 call, to base_coast;
//            port_to_port makes 0 calls here, same as the image - `call_diff`
//            agrees). The plateau (11/164, 0.894 similar, best of every flag
//            set) is inherited from `port_to_port`'s own already-documented
//            MISMATCH ceiling (12/141, register/stack-slot allocation, not
//            branch shape) - here it is inlined a SECOND time, inside the
//            `BaseCurrentCount` loop, compounding the same divergence.
// size      510 bytes
// prototype int (__cdecl ?naval_base@@YAHH@Z)(int baseID)
// callers   1   call targets   1
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x0050DF30
Return Value: Does base have a strategic naval importance? true/false
Status: Complete
*/
BOOL __cdecl naval_base(int base_id) {
    if (base_coast(base_id) < 0 || BaseCurrentCount <= 0) {
        return false;
    }
    uint32_t faction_id = Bases[base_id].faction_id_current;
    for (int i = 0; i < BaseCurrentCount; i++) {
        if (faction_id != Bases[i].faction_id_current) {
            if (port_to_port(base_id, i)) {
                return true;
            }
        }
    }
    return false;
}

/*
Purpose: Determine if specified unit can set up a convoy route with specified base.
// ORIGINAL: 0x0050E5C0 ?convoy@@YAHHH@Z 0x0050E5C0-0x0050E81C
// TRIED: call count already matches the image (0 calls - `port_to_port`
//            fully inlines, same as the image; `call_diff` agrees). Best of
//            every flag set is the default (18/200, 0.514 similar) - same
//            inherited `port_to_port` inlining plateau as `naval_base`,
//            not something specific to this call site's own control flow.
// size      604 bytes
// prototype int (__cdecl ?convoy@@YAHHH@Z)(int vehID, int baseID)
// callers   1   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
Return Value: Is a convoy route possible? true/false
Status: Complete
*/
BOOL __cdecl convoy(int veh_id, int base_id) {
    int home_base_id = Vehs[veh_id].home_base_id;
    if (home_base_id < 0 || base_id == (uint32_t)home_base_id) {
        return false;
    }
    uint32_t triad = get_triad(veh_id);
    if (triad == TRIAD_AIR) {
        return true;
    }
    uint32_t region_base = region_at(Bases[base_id].x, Bases[base_id].y);
    if (region_at(Bases[home_base_id].x, Bases[home_base_id].y) == region_base
        && ((region_base >= 64) == (triad == TRIAD_SEA))) {
        return true; // same region, by land or sea
    }
    if (triad) {
        return port_to_port(base_id, home_base_id); // sea
    }
    return false;
}

/*
Purpose: Validate region bounds. Bad regions include: 0, 63, 64, 127, 128.
// ORIGINAL: 0x005591C0 ?bad_reg@@YAHH@Z 0x005591C0-0x005591DD BYTE_EXACT
// size      29 bytes
// prototype 
// callers   1   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
Return Value: Is region bad? true/false
Status: Complete
*/
// BODY IN map.h, as `MEASURED inline`: the image writes it out at
// some call sites and calls it at others, and a .cpp definition is only ever
// one of those. The marker stays here because that is where the catalogue
// reads it.


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
Purpose: Determine whether point A is a coast or border tile. It seems that the point B check is
         effectively disabled since usage of this function passes same coordinates for both points.
         Modified return value to boolean rather than returning i. It is always treated as boolean
         and makes a lot more sense than returning the iterator position.
// ORIGINAL: 0x0056B480 ?coast_or_border@@YAHHHHHH@Z 0x0056B480-0x0056B5A9
// size      297 bytes
// prototype int (__cdecl ?coast_or_border@@YAHHHHHH@Z)(int xCoordPtA, int yCoordPtA, int xCoordPtB, int yCoordPtB, int factionID)
// callers   1   call targets   1
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x004E3EF0
Return Value: Is point A considered a border or coast? true/false
Status: Complete
*/
// TRIED: best 7/110, 0.826 similar (best flag set /O2 /Gy /GR- /GX; call
//            count already matches the image at 2, both to whose_territory).
//            The remaining gap looks like the `region_a != region_b` OR-term
//            being evaluated ahead of the `whose_territory(...)` call in the
//            image (a cheap loop-invariant compare scheduled before an
//            expensive call) - swapping the source's OR order to match
//            scored WORSE (0.780), so that is not the lever. Same
//            xrange/on_map/is_ocean-loop register-scheduling plateau as the
//            rest of this file otherwise. Not chased further.
BOOL __cdecl coast_or_border(int x_point_a, int y_point_a, int x_point_b, 
                             int y_point_b, int faction_id) {
    if ((int)faction_id != whose_territory(faction_id, x_point_a, y_point_a, NULL, false)) {
        return false; // faction doesn't control point A
    }
    uint32_t region_a = region_at(x_point_a, y_point_a);
    uint32_t region_b = region_at(x_point_b, y_point_b);
    for (uint32_t i = 1; i < 9; i++) {
        int x_radius = xrange(x_point_a + RadiusOffsetX[i]);
        int y_radius = y_point_a + RadiusOffsetY[i];
        if (on_map(x_radius, y_radius) && (is_ocean(x_radius, y_radius)
                || whose_territory(faction_id, x_radius, y_radius, NULL, false) != (int)faction_id
                || region_a != region_b)) { // not sure about reason for this
            return true; // modified to boolean rather than returning i
        }
    }
    return false;
}


/*
Purpose: Set the temperature for the specified tile.
// ORIGINAL: 0x00591AD0 ?temp_set@@YAXHHH@Z 0x00591AD0-0x00591B05 BYTE_EXACT
// size      53 bytes
// prototype 
// callers   4   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl temp_set(int x, int y, int temperature) {
    Map *tile = map_loc(x, y);
    tile->climate &= 0xF8;
    tile->climate |= temperature & 7;
}


/*
Purpose: Set the rainfall (climate) for the specified tile.
// ORIGINAL: 0x00591A80 ?climate_set@@YAXHHH@Z 0x00591A80-0x00591ACE BYTE_EXACT
// size      78 bytes
// prototype void (__cdecl ?climate_set@@YAXHHH@Z)(int xCoord, int yCoord, int climate)
// callers   2   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl climate_set(int x, int y, int rainfall) {
    Map *tile = map_loc(x, y);
    tile->climate &= 0xE7;
    tile->climate |= (rainfall & 3) << 3;
    tile->bit2 |= 0x400000; // TODO: identify value
    UnkBitfield1 |= 1; // TODO: identify global + value
}

/*
Purpose: Calculate the elevation of the specified tile.
// ORIGINAL: 0x005919C0 ?elev_at@@YAHHH@Z 0x005919C0-0x00591A77
// LEVER: two fixes got this from 37/67 to 57/67 (0.985 similar). (1) A
//        shared `Map *tile` read once for both `contour` and `climate`,
//        instead of calling `alt_detail_at`/`alt_at` as separate accessors
//        that each recompute map_loc(x,y). (2) BRANCH POLARITY: the ternary
//        `contour <= threshold ? 10 : modulo` put the short `+10` arm in
//        the fall-through and jumped to the long modulo block, backwards
//        from the image (which falls through to the modulo work and jumps
//        to +10). Rewriting as `if (contour > threshold) { modulo } else
//        { +10 }` matches. `contour` must be `int`: as `uint32_t` the
//        comparison compiled `ja`/unsigned, the image's `jle` is signed.
// TRIED: the remaining divergence is a single MapSeaLevel global load
//            (`mov edi, [0x94987c]`) that the image schedules AFTER the
//            contour byte-read and this tree schedules one slot earlier,
//            regardless of source statement order - tried an intermediate
//            `diff` local isolating the two subtractions, no effect. Same
//            instruction count (67/67) either way, just one load's position.
// size      183 bytes
// prototype int (__cdecl ?elev_at@@YAHHH@Z)(int xCoord, int yCoord)
// callers   1   call targets   0
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     (none)
Return Value: Elevation (bounded to: -3000 to 3500)
Status: Complete
*/
int __cdecl elev_at(int x, int y) {
    Map *const tile = map_loc(x, y);
    int contour = tile->contour;
    int elev = 50 * (contour - ElevDetail[3] - MapSeaLevel);
    if (contour > (int)ElevDetail[tile->climate >> 5]) {
        elev += (x * 113 + y * 217 + MapSeaLevel * 301) % 50;
    } else {
        elev += 10;
    }
    return range(elev, -3000, 3500);
}

/*
Purpose: Calculate the natural altitude of the specified tile.
// ORIGINAL: 0x005918A0 ?alt_natural@@YAHHH@Z 0x005918A0-0x005918EF
// TRIED: same index-fold plateau as alt_at/alt_detail_at - the inlined
//            map_loc(x,y)->contour read (via alt_detail_at) schedules the
//            width-load/imul/shift with ecx/eax swapped from the image's
//            allocation; best 4/28 (0.618) across all flag sets, default
//            flags give 3/28 (0.793 similar, MISMATCH). Not chased further.
// size      79 bytes
// prototype 
// callers   1   call targets   0
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     (none)
Return Value: Natural altitude on a scale from 0 (ocean trench) to 6 (mountain tops)
Status: Complete
*/
// BODY IN map.h, as `MEASURED inline`: the image calls it as a real function
// from world_linearize_contours but inlines it in alt_set_both, and a .cpp
// definition can only ever be one of those.

/*
Purpose: Set both the altitude and natural altitude for the specified tile. The altitude_natural
         parameter can be between 0 to 9.
// ORIGINAL: 0x005918F0 ?alt_set_both@@YAXHHH@Z 0x005918F0-0x005919C0
// LEVER: call_diff had this 3-calls-vs-2 (MORE): `alt_natural` moved to
//   `MEASURED inline` in map.h (image calls it for real from
//   world_linearize_contours but inlines it here) and the `rnd()` call
//   replaced with its own body, since a cross-TU call can never inline.
//   0.513 -> 0.843 similar. TRIED beyond that: `Map*`/pointer hoists for
//   AltNatural[altitude_natural] (no change); rewriting alt_natural's
//   `while (cond && natural) natural--;` as a decrement-then-clamp loop to
//   chase the image's pointer-walk shape (made both worse, 0.843->0.817).
// size      208 bytes
// prototype void (__cdecl ?alt_set_both@@YAXHHH@Z)(int xCoord, int yCoord, int altitude)
// callers   2   call targets   2
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x00591290 0x0064601D
Return Value: n/a
Status: Complete
*/
void __cdecl alt_set_both(int x, int y, int altitude_natural) {
    alt_set(x, y, altitude_natural);
    if (alt_natural(x, y) != altitude_natural) {
        // `rnd()`'s own body, written out: it is a real function used
        // elsewhere (general.cpp), so a cross-TU call to it cannot inline -
        // the image's `call 0x0064601D` is straight to the CRT `rand`.
        uint32_t *const base = &AltNatural[altitude_natural];
        int bounds = base[1] - base[0];
        int roll = (bounds - 1 > 0) ? rand() % bounds : 0;
        alt_put_detail(x, y, (uint8_t)(base[0] + MapSeaLevel + roll));
    }
}

/*
Purpose: Get the bit shifted (down) altitude of the specified tile.
// ORIGINAL: 0x00500150 ?alt_at@@YAHHH@Z 0x00500150-0x0050017B
// TRIED: best 12/16, 0.875 similar (MISMATCH) at every flag set - image
//            schedules `xor ecx,ecx` (the future zero-extend register) right
//            after computing the tile index, before loading the map_tiles()
//            pointer into edx; this tree always loads the pointer first and
//            zeroes the OTHER register later for the byte-load. Tried: a
//            local `Map *tile` before the field read; a local `uint32_t
//            climate` read before the shift. Neither moved the schedule -
//            same plateau shape as abstract_at's register-tie, but here the
//            mnemonics themselves differ (xor vs mov), not just the operand,
//            so it does not qualify for `semantic` either.
// size      43 bytes
// prototype int (__cdecl ?alt_at@@YAHHH@Z)(int xCoord, int yCoord)
// callers   1   call targets   0
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     (none)
Return Value: Altitude
Status: Complete
*/
// BODY IN map.h, as `MEASURED inline`: the image writes it out at
// some call sites and calls it at others, and a .cpp definition is only ever
// one of those. The marker stays here because that is where the catalogue
// reads it.
// TRIED: best MNEMONIC_ONLY, 10/12 instructions, 1.000 similar (best flag
//            set /O2 /Gy /GR- /Oy- /GX). Same index-fold plateau as
//            alt_at/alt_detail_at: the image computes `edx + eax` then
//            indexes `[ecx + edx]` (map_tiles()-style base loaded before the
//            offset add), this tree's `(x >> 1) + y * (bounds >> 1)` compiles
//            `eax + ecx` then `[eax + edx]` - operands and registers both
//            swapped from a single array-index expression. Not chased
//            further; register-allocation-only, not a source-shape fix.



/*
Purpose: Get the altitude details of the specified tile.
// ORIGINAL: 0x00500180 ?alt_detail_at@@YAHHH@Z 0x00500180-0x005001A9
// TRIED: same plateau as `alt_at` - image schedules `xor ecx,ecx` before
//            loading the map_tiles() pointer into edx; this tree loads the
//            pointer into ecx first and zeroes edx later for the byte-load.
//            Best 11/15, MISMATCH at every flag set.
// size      41 bytes
// prototype int (__cdecl ?alt_detail_at@@YAHHH@Z)(int xCoord, int yCoord)
// callers   1   call targets   0
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     (none)
Return Value: Altitude detail
Status: Complete
*/
// BODY IN map.h, as `MEASURED inline`: the image writes it out at
// some call sites and calls it at others, and a .cpp definition is only ever
// one of those. The marker stays here because that is where the catalogue
// reads it.


/*
Purpose: Set the altitude details for the specified tile.
// ORIGINAL: 0x00591260 ?alt_put_detail@@YAXHHH@Z 0x00591260-0x00591288 SEMANTIC
// TRIED: shared plateau with region_set (identical shape). Image loads
//            the byte parameter into cl BEFORE loading map_tiles(); the
//            compiler always loads map_tiles() first regardless of source
//            order. Tried: a cast-only local, a `Map *tile = map_loc(x,y);`
//            temp before the store, both no-op - same 11/14 MNEMONIC_ONLY,
//            same swapped pair of loads.
// size      40 bytes
// prototype void (__cdecl ?alt_put_detail@@YAXHHH@Z)(int xCoord, int yCoord, int detail)
// callers   5   call targets   0
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
// BODY IN map.h, as `MEASURED inline`: the image writes it out at
// some call sites and calls it at others, and a .cpp definition is only ever
// one of those. The marker stays here because that is where the catalogue
// reads it.



/*
Purpose: Set the faction owner for the specified tile.
// ORIGINAL: 0x00591B10 ?owner_set@@YAXHHH@Z 0x00591B10-0x00591B48
// TRIED: direct `tile->val2` read/write (no pointer hoist) - worse (0.857 vs 0.895).
//            Rewriting the RHS as the explicit XOR-merge identity
//            (`*field ^ ((*field ^ faction_id) & 0xF)`, which is what the image's
//            `xor/and/xor` sequence computes for `(*field & 0xF0) | (faction_id & 0xF)`)
//            scored identically to the plain OR-of-ANDs (0.895/8 of 20) - the compiler
//            already finds that transform on its own. The image factors the record-index
//            multiply as `lea x11` + implicit SIB scale-4 (`[ecx+eax*4+2]`); this tree's
//            imul-by-0x2c/lea-plus-ecx form is the same "multiply factoring" plateau as
//            whose_territory, not something the RHS shape changes.
// size      56 bytes
// prototype void (__cdecl ?owner_set@@YAXHHH@Z)(int xCoord, int yCoord, int factionID)
// callers   13   call targets   0
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl owner_set(int x, int y, int faction_id) {
    // Through a pointer to the field; see `code_set` for why.
    uint8_t *const field = &map_loc(x, y)->val2;
    *field = static_cast<uint8_t>((*field & 0xF0) | (faction_id & 0xF));
}

/*
Purpose: Set the site for the specified tile.
// ORIGINAL: 0x00591B50 ?site_set@@YAXHHH@Z 0x00591B50-0x00591B86 SEMANTIC
// TRIED: SHAPE_EXACT 17/19 plateau. Image folds the +2 field offset
//            into the READ's addressing mode but recomputes a bare tile
//            pointer (no offset) for the WRITE, applying +2 as the store's
//            own displacement instead - the field-pointer form folds +2
//            into a single lea reused for both, one byte longer at that
//            lea and everything after shifts. Tried: a `Map *tile` local
//            with two independent `->val2` accesses (worse, 15/19 and a
//            full reorder of the shl/and), an intermediate `uint8_t v`
//            local, re-calling `map_loc(x,y)` a second time for the write,
//            and swapping the OR operand order - all equal or worse.
// size      54 bytes
// prototype void (__cdecl ?site_set@@YAXHHH@Z)(int xCoord, int yCoord, int site)
// callers   9   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl site_set(int x, int y, int site) {
    // Through a pointer to the field; see `code_set` for why.
    uint8_t *const field = &map_loc(x, y)->val2;
    *field = static_cast<uint8_t>((*field & 0x0F) | (site << 4));
}

/*
Purpose: Get the region of the specified tile.
// ORIGINAL: 0x00500220 ?region_at@@YAHHH@Z 0x00500220-0x00500249
// TRIED: same plateau as `alt_at` - image schedules `xor ecx,ecx` before
//            loading the map_tiles() pointer into edx; this tree loads the
//            pointer into ecx first and zeroes edx later for the byte-load.
//            Tried a local `Map *tile` before the field read - no change.
//            Best 11/15, MISMATCH at every flag set.
// size      41 bytes
// prototype int (__cdecl ?region_at@@YAHHH@Z)(int xCoord, int yCoord)
// callers   2   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
Return Value: Region
Status: Complete
*/
// BODY IN map.h, as `MEASURED inline`: the image writes it out at
// some call sites and calls it at others, and a .cpp definition is only ever
// one of those. The marker stays here because that is where the catalogue
// reads it.


/*
Purpose: Set the region for the specified tile.
// ORIGINAL: 0x00591B90 ?region_set@@YAXHHH@Z 0x00591B90-0x00591BB8 SEMANTIC
// TRIED: same plateau as alt_put_detail - byte-parameter load and
//            map_tiles() pointer load are swapped by the compiler
//            regardless of source order. Tried a `Map *tile` temp before
//            the store; no-op, same 11/14 MNEMONIC_ONLY.
// size      40 bytes
// prototype void (__cdecl ?region_set@@YAXHHH@Z)(int xCoord, int yCoord, int region)
// callers   2   call targets   0
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
// BODY IN map.h, as `MEASURED inline`: the image writes it out at
// some call sites and calls it at others, and a .cpp definition is only ever
// one of those. The marker stays here because that is where the catalogue
// reads it.



/*
Purpose: Set the using faction id for the specified tile.
// ORIGINAL: 0x00591C10 ?using_set@@YAXHHH@Z 0x00591C10-0x00591C48
// TRIED: 12/20 plateau. Image reads tile->val3 TWICE (once via the
//   pre-offset [ecx+eax*4+5] load, once again via [eax+5] after computing
//   the plain tile pointer); this tree's `&=0xF8; |=faction&7;` compiles the
//   same XOR-merge trick but CSEs the two reads into one, one instruction
//   short. Tried: single `(v&0xF8)|(f&7)` assignment, explicit
//   `v ^= (v^f)&7`, splitting map_loc() per statement, a `uint8_t v = ...`
//   temp, casts to uint8_t/uint32_t on either side - all equal or worse.
// size      56 bytes
// prototype void (__cdecl ?using_set@@YAXHHH@Z)(int xCoord, int yCoord, int factionID)
// callers   4   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl using_set(int x, int y, int faction_id) {
    Map *tile = map_loc(x, y);
    tile->val3 &= 0xF8;
    tile->val3 |= faction_id & 7;
}


/*
Purpose: Set the lock faction id for the specified tile.
// ORIGINAL: 0x00591C50 ?lock_set@@YAHHHH@Z 0x00591C50-0x00591C88 BYTE_EXACT
// symbol    ?lock_set@@YAXHHH@Z
// size      56 bytes
// prototype int (__cdecl ?lock_set@@YAHHHH@Z)(int xCoord, int yCoord, int factionID)
// callers   0   call targets   0
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     (none)
// LEVER: `callers 0` - lock_map (0x00591C90, its only source-level caller) call_diff'd
//        MORE by exactly one call on this. Moved to map.h as `MEASURED inline`.
Return Value: n/a
Status: Complete
*/
// BODY IN map.h, as `MEASURED inline`: the image writes it out at
// some call sites and calls it at others, and a .cpp definition is only ever
// one of those. The marker stays here because that is where the catalogue
// reads it.

/*
Purpose: Lock the specified tile for the faction id.
// ORIGINAL: 0x00591C90 ?lock_map@@YAHHHH@Z 0x00591C90-0x00591CE2 BYTE_EXACT
// size      82 bytes
// prototype int (__cdecl ?lock_map@@YAHHHH@Z)(int xCoord, int yCoord, int factionID)
// callers   1   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
Return Value: True if already locked by another faction, otherwise false
Status: Complete
*/
BOOL __cdecl lock_map(int x, int y, int faction_id) {
    uint32_t lock_id = lock_at(x, y);
    if (lock_id != faction_id) {
        if (lock_id) {
            return true;
        }
        lock_set(x, y, faction_id);
    }
    return false;
}

/*
Purpose: Unlock the specified tile for faction id.
// ORIGINAL: 0x00591CF0 ?unlock_map@@YAXHHH@Z 0x00591CF0-0x00591D29 BYTE_EXACT
// LEVER: the `Map *tile = map_loc(x, y);` local must be declared INSIDE the
//        if-block, after the lock_at() check, not hoisted above it - the
//        image computes the tile pointer only on the taken branch.
// size      57 bytes
// prototype void (__cdecl ?unlock_map@@YAXHHH@Z)(int xCoord, int yCoord, int factionID)
// callers   1   call targets   0
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl unlock_map(int x, int y, int faction_id) {
    if (lock_at(x, y) == faction_id) {
        Map *tile = map_loc(x, y);
        tile->val3 &= 0xC7;
    }
}


/*
Purpose: Set the rockiness for the specified tile.
// ORIGINAL: 0x00591BC0 ?rocky_set@@YAXHHH@Z 0x00591BC0-0x00591C0E
// LEVER: `uint8_t *const field = &tile->val3;` plus a local for the masked read, same idiom
//            as owner_set/site_set - the image reads `val3` and masks it BEFORE it evaluates
//            `rocky << 6`, where `tile->val3 = (tile->val3 & 0x3F) | (rocky << 6)` in one
//            statement (either operand order) evaluated the shift first regardless. 0.920 ->
//            0.960 best-across-flags similarity.
// TRIED: delaying the `val3` store until after computing `tile->bit2 | 0x400000` into a
//            local (matching the image's read-both-then-store-both order for the two fields)
//            made it WORSE (0.960 -> 0.885) - the local for `bit2` changes the image's
//            `mov ecx,[eax+0xc]/or ecx,.../mov [eax+0xc],ecx` register-folded shape into
//            something that costs more elsewhere. Left storing `val3` immediately.
// size      78 bytes
// prototype void (__cdecl ?rocky_set@@YAXHHH@Z)(int xCoord, int yCoord, int rocky)
// callers   11   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl rocky_set(int x, int y, int rocky) {
    Map *tile = map_loc(x, y);
    uint8_t *const field = &tile->val3;
    uint8_t masked = *field & 0x3F;
    *field = masked | (rocky << 6);
    tile->bit2 |= 0x400000; // TODO: identify value
    UnkBitfield1 |= 1; // TODO: identify variable + value
}

/*
Purpose: Get the bit of the specified tile.
// ORIGINAL: 0x005001B0 ?bit_at@@YAHHH@Z 0x005001B0-0x005001D5 BYTE_EXACT
// size      37 bytes
// prototype int (__cdecl ?bit_at@@YAHHH@Z)(int xCoord, int yCoord)
// callers   1   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
Return Value: Bitfield
Status: Complete
*/

/*
Purpose: Set the bit for the specified tile.
// ORIGINAL: 0x00591D30 ?bit_put@@YAXHHH@Z 0x00591D30-0x00591D58 SEMANTIC
// size      40 bytes
// prototype void (__cdecl ?bit_put@@YAXHHH@Z)(int xCoord, int yCoord, int bit)
// callers   1   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
// BODY IN map.h, as `MEASURED inline`: the image writes it out at
// some call sites and calls it at others, and a .cpp definition is only ever
// one of those. The marker stays here because that is where the catalogue
// reads it.
// TRIED: best MNEMONIC_ONLY (needs /Oy- to even get the ebp-frame; with
//            it, 12/14 instructions, all mnemonics agree). The image loads
//            the `bit` parameter into ecx BEFORE the map_tiles() pointer into
//            edx, this tree's `map_loc(x, y)->bit = bit;` always loads the
//            pointer first and `bit` second. Tried: a local `int new_bit =
//            bit;` before the store, and a local `Map *tile = map_loc(x, y);`
//            before it - both compiled byte-identical to the plain form, so
//            the scheduling is fixed by the compiler regardless of source
//            shape. Not chased further.


/*
Purpose: Set or unset bit for the specified tile.
// ORIGINAL: 0x00591D60 ?bit_set@@YAXHHHH@Z 0x00591D60-0x00591DA2 BYTE_EXACT
// LEVER: the image computes the tile's `&bit` address ONCE, unconditionally, before testing `set` - this tree's `if (set) map_loc(x,y)->bit |= bit; else map_loc(x,y)->bit &= ~bit;` let the compiler test `set` FIRST and compute the address separately in each arm instead. Hoisting `uint32_t *const field = &map_loc(x, y)->bit;` above the `if` (see map.h) is BYTE_EXACT.
// size      66 bytes
// prototype void (__cdecl ?bit_set@@YAXHHHH@Z)(int xCoord, int yCoord, int bit, int)
// callers   32   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl bit_set(int x, int y, int bit, BOOL set) {
    uint32_t *const field = &map_loc(x, y)->bit;
    if (set) {
        *field |= bit;
    } else {
        *field &= ~bit;
    }
}



/*
Purpose: Set or unset bit2 for the specified tile.
// ORIGINAL: 0x00591DB0 ?bit2_set@@YAXHHHH@Z 0x00591DB0-0x00591DF2 BYTE_EXACT
// LEVER: was `MEASURED inline` in map.h with no comment recording why; world_borehole (0x005C7020) and siblings call_diff'd FEWER because the image keeps this as a real call there. Un-inlined to a plain map.cpp function like bit_set/code_set; `osmx check` stayed at 0 REGRESSED and world_borehole's call count now matches.
// size      66 bytes
// prototype void (__cdecl ?bit2_set@@YAXHHHH@Z)(int xCoord, int yCoord, int bit2, int)
// callers   19   call targets   0
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl bit2_set(int x, int y, int bit2, BOOL set) {
    uint32_t *const field = &map_loc(x, y)->bit2; // same LEVER as bit_set: fold the offset once.
    if (set) {
        *field |= bit2;
    } else {
        *field &= ~bit2;
    }
}



/*
Purpose: Set the code for the specified tile which keeps track of tile sequence order for landmarks.
// ORIGINAL: 0x00591E00 ?code_set@@YAXHHH@Z 0x00591E00-0x00591E45 BYTE_EXACT
// size      69 bytes
// prototype void (__cdecl ?code_set@@YAXHHH@Z)(int xCoord, int yCoord, int)
// callers   16   call targets   0
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl code_set(int x, int y, int code) {
    // THROUGH A POINTER TO THE FIELD. The image folds the member offset into
    // the address computation - `lea eax, [ecx + eax*4 + 0xc]` at 0x00591E29 -
    // and reading and writing `tile->bit2` twice puts the 0xc on the `mov`
    // instead. Taking `&tile->bit2` once is what produces the image's lea.
    uint32_t *const field = &map_loc(x, y)->bit2;
    *field = (*field & 0xFFFFFF) | (code << 24);
    UnkBitfield1 |= 4; // TODO: identify variable + value
}

/*
Purpose: Synchronize the actual tile bit with the faction visible bit.
// ORIGINAL: 0x00591E50 ?synch_bit@@YAXHHH@Z 0x00591E50-0x00591E82 BYTE_EXACT
// LEVER: `Map *tile = map_loc(x, y);` shared between the read of `tile->bit`
//        and the write to `tile->bit_visible[...]`, instead of two separate
//        `map_loc()`/`bit_at()` calls, so the address is computed once.
// size      50 bytes
// prototype void (__cdecl ?synch_bit@@YAXHHH@Z)(int xCoord, int yCoord, int factionID)
// callers   24   call targets   0
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
// BODY IN map.h, as `MEASURED inline`: the image writes it out at
// some call sites and calls it at others, and a .cpp definition is only ever
// one of those. The marker stays here because that is where the catalogue
// reads it.


/*
Purpose: Determine the tile's mineral count that translates to rockiness.
// ORIGINAL: 0x00591F00 ?minerals_at@@YAHHH@Z 0x00591F00-0x00592007
// LEVER: three fixes, 2/113 -> 13/113 (0.529 similar). (1) SIGNED / 2 IS NOT
//        >> 1: the image's `avg` computation carries the `cdq;sub;sar`
//        round-toward-zero fixup, so `avg` (and the `x/2`, `avg/2` subterms
//        of val1) must be written `/2`, not `>>1`. (2) `alt_at(x, y)` is
//        read by the image LAST, right before the switch, off a register
//        that survives the whole body untouched - moved the call to just
//        before `type` and read through a saved `origX` so the mutated `x`
//        (`x -= avg`) does not corrupt it. (3) The jump-table dump at
//        0x592008/0x592018 shows cases 1 and 2 of the outer `switch(type)`
//        are themselves compare-decrement chains over `val2`, i.e. nested
//        `switch(val2)` in the source, not the ternary chains that were
//        here - only `case 3`'s ternary reproduces its target's
//        test/cmp shape untouched.
// TRIED: not chased past this MISMATCH plateau - 263-byte body, three
//            jump tables, register-allocation-sensitive scheduling of the
//            early `push esi`/`push edi` pair. --all-flags agrees (best is
//            the same /O2 /Ob0 /Gy /GR- /Oy- /GX set already in use).
// size      263 bytes
// prototype int (__cdecl ?minerals_at@@YAHHH@Z)(int xCoord, int yCoord)
// callers   2   call targets   0
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     (none)
Return Value: 0 (Flat), 1 (Rolling), 2 (Rocky)
Status: Complete
*/
int __cdecl minerals_at(int x, int y) {
    if (!y || y == (MapLatitudeBounds - 1)) {
        return 2; // poles
    }
    int origX = x;
    int avg = (x + y) / 2;
    x -= avg;
    int val1 = (x / 2) + MapRandSeed + (x - (x % 2)) + (avg - (avg % 2));
    int val2 = (val1 - 2 * (x & 1) - (avg & 1)) & 3;
    int alt = alt_at(origX, y);
    int type = abs(alt - ALT_SHORE_LINE);
    if (alt < ALT_SHORE_LINE) {
        type--;
    }
    switch (type) {
      case 0: {
        switch (val2) {
          case 0:
            return 1;
          case 1:
          case 3:
            return 0;
          case 2:
            return ((val1 & 4) != 0) + 1; // 1 or 2
          default:
            return ~val2 & 1;
        }
      }
      case 1:
        switch (val2) {
          case 0: return 0;
          case 1: return 1;
          case 2: return 1;
          default: return 2;
        }
      case 2:
        switch (val2) {
          case 0: return 0;
          case 1: return 1;
          default: return 2;
        }
      case 3:
        return (val2 < 0 || val2 > 1) ? 2 : 1;
      default:
        return ~val2 & 1;
    }
}

/*
Purpose: Determine if the tile has a resource bonus. While the last parameter is unused, it's set to
         1 by two calls inside world_site(). Otherwise, all other calls have it set to 0.
// ORIGINAL: 0x00592030 ?bonus_at@@YAHHHH@Z 0x00592030-0x00592135
// TRIED (body is in map.h): the image reads the climate byte (alt_at) before the bit
//            dword (bit_at) at the shared `site_tile`-style address, while map.h's body
//            declares `bit` before `alt`. Swapping the declaration order to match the
//            image's read order made it WORSE (25/117 -> 17/117), so the declaration
//            order already in the header is the closer one; the remaining gap is the
//            image folding the bit-field load into the SIB-addressed `mov` where this
//            tree materialises the tile address into a register first - the same
//            register-allocation plateau as the rest of this family.
// LEVER: `alt` and `chk` typed `int` instead of `uint32_t` - the image compares
//            them against ALT_OCEAN_SHELF/ALT_SHORE_LINE with `jl`/`jge` and divides
//            `chk` with `idiv`, both signed forms; the tree's `uint32_t` locals were
//            producing `jb`/`jae`/`div`. 25/117 (0.800 similar) -> 20/117 (0.856
//            similar) across --all-flags.
// TRIED: `avg` typed `int` to match - the image's one remaining `avg >> 2` uses
//            `sar`, which `int` does reproduce, but it costs agreement elsewhere and
//            the best-flags similarity drops to 0.825. Left `uint32_t`.
// size      261 bytes
// prototype int (__cdecl ?bonus_at@@YAHHHH@Z)(int xCoord, int yCoord, int unkVal)
// callers   17   call targets   0
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     (none)
Return Value: 0 (no bonus), 1 (nutrient), 2 (mineral), 3 (energy)
Status: Complete
*/
// BODY IN map.h, as `MEASURED inline`: seven bodies call it where the
// image calls nothing - `call_diff` names it - and 0x00592030 is a real
// body of its own, so a .cpp definition is only one of the two.
// TRIED: a non-inline `bonus_at_call` forwarder, routed through from
//            crop_yield/mine_yield/energy_yield (base.cpp) so bonus_at(x,y,0)
//            would emit `call 0x592030` there - measured NO improvement to
//            the best-across-flags similarity for any of the three (each
//            tied exactly with not using it: crop_yield 0.292, mine_yield
//            0.388, energy_yield 0.137), because their winning flag set is
//            /Ob0-based and bonus_at's body already does not get folded in
//            there regardless of the forwarder. Not added.

/*
Purpose: Determine if the tile has a supply pod and if so what type.
// ORIGINAL: 0x00592140 ?goody_at@@YAHHH@Z 0x00592140-0x00592248
// TRIED: --all-flags's winner (39/114, the default set) already beats every other set
//            by a wide margin; the divergence is in the prologue - the image delays
//            loading x (`mov esi,[ebp+8]`) until after the `y * MapLongitude` multiply and
//            the callee-saved pushes, this tree loads x into edx immediately before the
//            multiply. Same register/instruction-scheduling plateau as bonus_at and the
//            rest of this family.
// size      264 bytes
// prototype int (__cdecl ?goody_at@@YAHHH@Z)(int xCoord, int yCoord)
// callers   14   call targets   0
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     (none)
Return Value: 0 (no supply pod), 1 (standard supply pod), 2 (unity pod?)
Status: Complete
*/
int __cdecl goody_at(int x, int y) {
    uint32_t bit = bit_at(x, y);
    if (bit & (BIT_SUPPLY_REMOVE | BIT_MONOLITH)) {
        return 0; // nothing, supply pod already opened or monolith
    }
    if (GameRules & RULES_NO_UNITY_SCATTERING) {
        return (bit & (BIT_UNK_4000000 | BIT_UNK_8000000)) ? 2 : 0; // ?
    }
    if (bit & BIT_SUPPLY_POD) {
        return 1; // supply pod
    }
    if (!MapRandSeed) {
        return 0; // nothing
    }
    uint32_t avg = (x + y) >> 1;
    int x_diff = x - avg;
    uint32_t cmp = (avg & 3) + 4 * (x_diff & 3);
    if (!is_ocean(x, y)
        && cmp == ((-5 * (avg >> 2) - 3 * (x_diff >> 2) + MapRandSeed) & 0xF)) {
        return 2;
    }
    return cmp == ((11 * (avg / 4) + 61 * (x_diff / 4) + MapRandSeed + 8) & 0x1F); // 0 or 1
}

/*
Purpose: Clear the map's site values in a radius from the tile.
// ORIGINAL: 0x00592400 ?site_radius@@YAXHHH@Z 0x00592400-0x00592480 BYTE_EXACT
// LEVER: fixed by fixing `xrange()` itself (see 0x0048BEE0, same file) -
//   `MapIsFlat & 1` and the swapped if/else polarity. No change needed here.
// size      128 bytes
// prototype void (__cdecl ?site_radius@@YAXHHH@Z)(int xCoord, int yCoord, int)
// callers   2   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl site_radius(int x, int y, int UNUSED(unk_val)) {
    for (int i = 0; i < 21; i++) {
        int x_radius = xrange(x + RadiusOffsetX[i]);
        int y_radius = y + RadiusOffsetY[i];
        if (on_map(x_radius, y_radius)) {
            Map *tile = map_loc(x_radius, y_radius);
            tile->val2 &= 0x0F;
        }
    }
}

/*
Purpose: Search for the first landmark found within the radius range of the specified tile.
// ORIGINAL: 0x00592550 ?find_landmark@@YAHHHH@Z 0x00592550-0x005925F8
// size      168 bytes
// prototype 
// callers   5   call targets   0
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     (none)
// LEVER: kill_landmark (0x005926F0), the only source-level caller, call_diff'd MORE
//        by exactly one call on this. Moved to map.h as `MEASURED inline`.
Return Value: Landmark offset or -1 if none found
Status: Complete
*/
// BODY IN map.h, as `MEASURED inline`: the image writes it out at
// some call sites and calls it at others, and a .cpp definition is only ever
// one of those. The marker stays here because that is where the catalogue
// reads it.

/*
Purpose: Set up a new landmark with the provided name at the specified tile.
// ORIGINAL: 0x00592600 ?new_landmark@@YAHHHPAD@Z 0x00592600-0x0059264B
// TRIED: strcpy_s(dst,32,src) vs strcpy(dst,src) - vc6_compat.h's strcpy_s already
//            inlines to a plain strcpy call, so this was a source-clarity change only; both
//            spellings already made the single call the image makes (osmx calls confirms 1
//            call to 0x00645460 either way). call_diff's "0 vs 1" was a false positive.
// TRIED: remaining mismatch is esi vs eax register allocation for MapLandmarkCount, a
//            stack-frame plateau per prior pass's note, not a source shape
// symbol    ?new_landmark@@YAHHHPBD@Z
// size      75 bytes
// prototype int (__cdecl ?new_landmark@@YAHHHPAD@Z)(int xCoord, int yCoord, int8* name)
// callers   18   call targets   1
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x00645460
Return Value: Landmark offset or -1 if max landmark count is reached
Status: Complete
*/
int __cdecl new_landmark(int x, int y, LPCSTR name) {
    int landmark_offset = MapLandmarkCount;
    if (landmark_offset >= MaxLandmarkNum) {
        return -1;
    }
    MapLandmarkCount += 1;
    Landmark *lm = &MapLandmark[landmark_offset];
    lm->x = x;
    lm->y = y;
    strcpy(lm->name, name);
    return landmark_offset;
}

/*
Purpose: Check whether the specified faction has permission to name a landmark on the provided tile.
// ORIGINAL: 0x00592650 ?valid_landmark@@YAHHHH@Z 0x00592650-0x005926EF BYTE_EXACT
// size      159 bytes
// prototype int (__cdecl ?valid_landmark@@YAHHHH@Z)(int xCoord, int yCoord, int factionID)
// callers   2   call targets   2
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x004E3B80 0x004E3EF0
Return Value: Does the faction have control of the tile to set a landmark? true/false
Status: Complete
*/
BOOL __cdecl valid_landmark(int x, int y, int faction_id) {
    int terr_faction_id = IsMultiplayerNet ? map_loc(x, y)->territory 
        : whose_territory(faction_id, x, y, NULL, false);
    if (terr_faction_id == faction_id) {
        return true;
    }
    if (terr_faction_id > 0) {
        return false;
    }
    int base_id = base_find(x, y);
    return base_id < 0 ? true : Bases[base_id].faction_id_current == faction_id;
}

/*
Purpose: Remove the landmark at the specified tile.
// ORIGINAL: 0x005926F0 ?kill_landmark@@YAXHH@Z 0x005926F0-0x005927D0
// LEVER: the image's shift-down is a per-element `MapLandmark[i] =
//        MapLandmark[i+1]` loop (struct assignment of `Landmark`, 40 bytes,
//        compiles to `mov ecx,0xa; rep movsd` per iteration) - NOT the
//        single bulk `memcpy_s` a prior pass had substituted for "the
//        original loop". Writing the loop out took this from MISMATCH
//        5/86 (0.851) to MNEMONIC_ONLY 60/86 (1.000 similar).
// size      224 bytes
// prototype void (__cdecl ?kill_landmark@@YAXHH@Z)(int xCoord, int yCoord)
// callers   3   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl kill_landmark(int x, int y) {
    int landmark_to_kill = find_landmark(x, y, 1);
    if (landmark_to_kill >= 0) {
        for (int i = landmark_to_kill; i < MapLandmarkCount - 1; i++) {
            MapLandmark[i] = MapLandmark[i + 1];
        }
        MapLandmarkCount -= 1;
    }
}

/*
Purpose: Check if coordinates are considered near or on coast. Radius (excludes actual coordinates)
         can either be all the squares directly around the coordinates or same as Base '+' radius.
// ORIGINAL: 0x004E49D0 ?is_coast@@YAHHHH@Z 0x004E49D0-0x004E4A91
// TRIED: 9/76, 0.84 similar plateau across every flag set. The image
//            uses TWO induction variables for the loop (an unscaled count
//            in [ebp-4] and a byte offset in edi, incremented in lockstep)
//            and defers loading `MapIsFlat` to AFTER the "loop won't run"
//            guard; this tree's -O2 always collapses to one induction
//            variable and hoists the `MapIsFlat` read before that guard,
//            regardless of whether the source calls `xrange`/`on_map` or
//            inlines their logic by hand (tried both, identical score).
//            `is_ocean`'s own comparison also narrows to a byte `cmp/jb`
//            here where the image keeps the 32-bit `cmp edx,0x60`/`jl` -
//            same root cause as is_ocean's own standalone plateau.
// size      193 bytes
// prototype int (__cdecl ?is_coast@@YAHHHH@Z)(int xCoord, int yCoord, int isBaseRadius)
// callers   10   call targets   0
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     (none)
Return Value: Is tile coast? true/false
Status: Complete
*/
BOOL __cdecl is_coast(int x, int y, BOOL is_base_radius) {
    uint32_t radius = is_base_radius ? 21 : 9;
    for (uint32_t i = 1; i < radius; i++) {
        int x_radius = x + RadiusOffsetX[i];
        if (!(MapIsFlat & 1)) {
            if (x_radius < 0) {
                x_radius += MapLongitudeBounds;
            } else if (x_radius >= (int)MapLongitudeBounds) {
                x_radius -= MapLongitudeBounds;
            }
        }
        int y_radius = y + RadiusOffsetY[i];
        if (on_map(x_radius, y_radius) && is_ocean(x_radius, y_radius)) {
            return true; // modified original that would return i, all calls check return as boolean
        }
    }
    return false;
}

/*
Purpose: Check whether the specified tile is part of an ocean.
// ORIGINAL: 0x005001E0 ?is_ocean@@YAHHH@Z 0x005001E0-0x00500211
// TRIED: same root plateau as `alt_at` (image schedules `xor ecx,ecx`
//            before loading the map_tiles() pointer; this tree loads the
//            pointer first). Downstream that also means the image has a
//            spare zeroed register (`eax`) to build the boolean with
//            `setl al`, while this tree materialises it with `sbb eax,eax;
//            neg eax` instead - a consequence of the same register-tie, not
//            an independent divergence. Best 9/18, 0.800 similar, MISMATCH.
// size      49 bytes
// prototype int (__cdecl ?is_ocean@@YAHHH@Z)(int xCoord, int yCoord)
// callers   3   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
Return Value: Is tile ocean? true/false
Status: Complete
*/

/*
Purpose: Get the owner of the specified tile if there is a unit in it.
// ORIGINAL: 0x00500250 ?veh_who@@YAHHH@Z 0x00500250-0x00500288 BYTE_EXACT
// size      56 bytes
// prototype int (__cdecl ?veh_who@@YAHHH@Z)(int xCoord, int yCoord)
// callers   3   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
Return Value: Owner (faction id) or -1
Status: Complete
*/
// BODY IN map.h, as `MEASURED inline`: the image writes it out at
// some call sites and calls it at others, and a .cpp definition is only ever
// one of those. The marker stays here because that is where the catalogue
// reads it.


/*
Purpose: Rebuild the Map's unit related values.
// ORIGINAL: 0x00532A90 ?rebuild_vehicle_bits@@YAXXZ 0x00532A90-0x00532B63
// TRIED: bit_set/bit_at/owner_set with (x,y) never spilled a 3rd stack
//            slot (image is `sub esp, 0xc`) - a raw `Map *tile` walked by
//            sizeof(Map), re-reading tile->bit at each of its three sites
//            rather than caching it in a local, reaches the same frame size
//            and 27/74 agreeing. A cached `int bit`/`uint32_t bit` local
//            reused across the sites undersizes the frame to `sub esp, 8`
//            and drops to 3/74 - the image does NOT cache this value in a
//            register across the veh_id search loop. Remaining mismatch
//            past instruction 3 is the same eax/ecx invariant-load swap as
//            0x00532B70 (tried y-before-tile, no change) - register
//            allocation, not a source shape.
// size      211 bytes
// prototype 
// callers   3   call targets   0
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl rebuild_vehicle_bits() {
    Map *tile = map_tiles();
    for (int y = 0; y < MapLatitudeBounds; y++) {
        for (int x = y & 1; x < MapLongitudeBounds; x += 2, tile++) {
            tile->bit &= ~BIT_VEH_IN_TILE;
            for (int veh_id = 0; veh_id < VehCurrentCount; veh_id++) {
                if (Vehs[veh_id].x == x && Vehs[veh_id].y == y) {
                    tile->bit |= BIT_VEH_IN_TILE;
                    if (!(tile->bit & BIT_BASE_IN_TILE)) {
                        tile->val2 = (tile->val2 & 0xF0) | (Vehs[veh_id].faction_id & 0xF);
                    }
                    break;
                }
            }
        }
    }
}

/*
Purpose: Rebuild the Map's base related values.
// ORIGINAL: 0x00532B70 ?rebuild_base_bits@@YAXXZ 0x00532B70-0x00532C2B
// TRIED: bit_set/owner_set called with (x,y) recomputed map_loc each time
//            and never reduced to the image's single walking pointer (0/66
//            agreeing). A raw `Map *tile` incremented by `sizeof(Map)` once
//            per x-step - since consecutive same-parity tiles are contiguous
//            and a row boundary lands on the next tile exactly - reaches
//            22/66: the whole body matches except the first two invariant
//            loads (MapLatitudeBounds, MapTiles) landing in eax/ecx swapped
//            from the image's ecx/eax. Tried: y declared before tile, a
//            named `int lat = MapLatitudeBounds` local, and a while-loop in
//            place of the for - none change which register either load gets.
//            Register-allocation plateau, not a source shape.
// size      187 bytes
// prototype
// callers   3   call targets   0
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl rebuild_base_bits() {
    Map *tile = map_tiles();
    for (int y = 0; y < MapLatitudeBounds; y++) {
        for (int x = y & 1; x < MapLongitudeBounds; x += 2, tile++) {
            tile->bit &= ~BIT_BASE_IN_TILE;
            for (int base_id = 0; base_id < BaseCurrentCount; base_id++) {
                if (Bases[base_id].x == x && Bases[base_id].y == y) {
                    tile->bit |= BIT_BASE_IN_TILE;
                    tile->val2 = (tile->val2 & 0xF0) | (Bases[base_id].faction_id_current & 0xF);
                    break;
                }
            }
        }
    }
}

/*
Purpose: Calculate the distance between two x coordinates with handling for round maps.
// ORIGINAL: 0x00579790 ?x_dist@@YAHHH@Z 0x00579790-0x005797C7
// size      55 bytes
// prototype 
// callers   1   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00644F3A
Return Value: Distance
Status: Complete
*/
// BODY IN map.h, as `MEASURED inline`: the image writes it out at
// some call sites and calls it at others, and a .cpp definition is only ever
// one of those. The marker stays here because that is where the catalogue
// reads it.
// LEVER: `!(MapIsFlat & 1)`, not `!MapIsFlat` - same idiom as xrange
//        (0x0048BEE0) - took the best flag set (/O2 /Oi- /Gy /GR- /Oy- /GX)
//        from 14/21 to 16/21 (still 0.923 similar - the raw count moved, the
//        rounded ratio didn't).
// TRIED: remaining gap (best flag set, 18 instructions here vs the
//            image's 21) is a trailing `mov ecx, eax` the image emits right
//            before `pop ebp; ret` that this tree's `return dist;` never
//            produces - same register-caching family as vector_dist
//            (0x005A5910)'s note on this same function. Not chased further.


/*
Purpose: Check whether a faction can see the specified tile.
// ORIGINAL: 0x00579840 ?is_known@@YAHHHH@Z 0x00579840-0x00579896
// size      86 bytes
// prototype int (__cdecl ?is_known@@YAHHHH@Z)(int xCoord, int yCoord, int factionID)
// callers   1   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// TRIED: 16 of 30 at the best flag set (/O2 /Gy /GR- /Oy- /GX), 32
//            compiled instructions against the image's 30. The two extra are
//            `push ebx`/`pop ebx`: the image computes `1 << faction_id` AFTER
//            the tile-index `lea` chain, when edx is dead, and reuses ecx for
//            map_tiles() (`mov edx, 1; shl edx, cl; mov ecx, [0x94a30c];
//            test byte ptr [ecx + eax*4 + 4], dl`); VC6 here hoists the shift
//            ahead of that chain, where eax/ecx/edx are all live, so the mask
//            lands in a callee-saved register and costs the save/restore.
// TRIED: nothing moves that scheduling decision. Measured, all 16 of 30
//            and all 32 instructions: a `mask`/`vis`/`tile`/`flags` local, a
//            `(uint8_t)` cast on either operand, `!!(...)`, an explicit
//            `!= 0`, extra parentheses, and swapping the `&` operands
//            (`(1 << faction_id) & ...->visibility`).
// TRIED: splitting the `||` is WORSE, not better - the mirror of
//            map_write/map_read. A guard clause (`if (A) return true; return
//            B;`) scores 0 of 30, and so does the ternary form; the image
//            shares one `mov eax, 1; pop ebp; ret` tail between both tests
//            and falls through to `xor eax, eax` only once.
// TRIED: the equivalent bit test `(map_loc(x, y)->visibility >>
//            faction_id) & 1` scores HIGHER, 25 of 30 with the instruction
//            count matching at 30, because `shr al, cl; test al, 1` needs no
//            second register - but it emits a shift of the VALUE where the
//            image shifts the MASK, so it trades one divergence for another
//            and still reaches no tier. Left as the image's own mask test.
Return Value: Is tile visible/known to faction? true/false
Status: Complete
*/
BOOL __cdecl is_known(int x, int y, int faction_id) {
    return (PlayersData[faction_id].flags & PFLAG_MAP_REVEALED
        || map_loc(x, y)->visibility & (1 << faction_id));
}

/*
Purpose: If a base exists, get the owner of the specified tile.
// ORIGINAL: 0x005798A0 ?base_who@@YAHHH@Z 0x005798A0-0x005798D8 BYTE_EXACT
// size      56 bytes
// prototype 
// callers   2   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
Return Value: Owner (faction id) or -1
Status: Complete
*/
// BODY IN map.h, as `MEASURED inline`: six bodies call it where the image
// calls nothing - `call_diff` names it - and 0x005798A0 is a real body.

/*
Purpose: If a base or unit exists, get the owner of the specified tile.
// ORIGINAL: 0x005798E0 ?anything_at@@YAHHH@Z 0x005798E0-0x00579918 BYTE_EXACT
// size      56 bytes
// prototype int (__cdecl ?anything_at@@YAHHH@Z)(int xCoord, int yCoord)
// callers   1   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
Return Value: Owner (faction id) or -1
Status: Complete
*/
int __cdecl anything_at(int x, int y) {
    Map *tile = map_loc(x, y);
    if (tile->bit & (BIT_VEH_IN_TILE | BIT_BASE_IN_TILE)) {
        // `int`, as in veh_who: the image's test is `jl`.
        int owner = tile->val2 & 0xF;
        if (owner < MaxPlayerNum) {
            return owner;
        }
    }
    return -1;
}

/*
Purpose: Shutdown allocated map variables.
// ORIGINAL: 0x00590E90 ?map_shutdown@@YAXXZ 0x00590E90-0x00590EC5 BYTE_EXACT
// size      53 bytes
// prototype 
// callers   3   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00644EF2
// LEVER: map_read (0x00591130), one of its three callers, call_diff'd FEWER by
//        exactly one call - the image inlines this whole body there. Moved to
//        map.h as `MEASURED inline`; map_init's call site (already call_diff-clean)
//        was unaffected.
Return Value: n/a
Status: Complete
*/
// BODY IN map.h, as `MEASURED inline`: the image writes it out at
// some call sites and calls it at others, and a .cpp definition is only ever
// one of those. The marker stays here because that is where the catalogue
// reads it.

/*
Purpose: Initialize map variables.
// ORIGINAL: 0x00590ED0 ?map_init@@YAXXZ 0x00590ED0-0x00591036
// LEVER: `sprintf_s(MapFilePath, 80, "maps\\%s.%s", label_get(676), MapExtension)`
//        was ONE call the image never makes - it builds the path by hand into
//        StringTemp ("no C++ library" idiom): StringTemp[0]=0; strcat "maps\\";
//        strcat label_get(676); strcat "."; then strcpy StringTemp into
//        MapFilePath and strcat MapExtension onto that. Took this from 5/105
//        (0.745 similar) to 25/105 (0.919 similar). TRIED beyond this
//        point: the image pins ebx=0/edi=1 in registers across the whole
//        function (push edi; mov edi,1 at entry; cmp eax,ebx instead of
//        test eax,eax at every null-check; mov eax,edi at the return-true
//        tail) - a VC6 register-allocation heuristic, not a source shape;
//        no rewrite of the two null-checks (`!= 0`, ternary, guard clause)
//        changed which register VC6 picked.
// symbol    ?map_init@@YAHXZ
// size      358 bytes
// prototype 
// callers   6   call targets   7
// kind      game
// flags     sp_ready;purged_ok
// calls     0x00471240 0x005D4510 0x006169A0 0x006290E0 0x00644EF2 0x00645460 0x00645470
Return Value: n/a
Status: Complete
*/
BOOL __cdecl map_init() {
    StringTemp[0] = 0;
    strcat(StringTemp, "maps\\");
    strcat(StringTemp, label_get(676));
    strcat(StringTemp, ".");
    strcpy((LPSTR)MapFilePath, StringTemp);
    strcat((LPSTR)MapFilePath, MapExtension);
    MapLongitude = MapLongitudeBounds / 2;
    MapArea = MapLongitude * MapLatitudeBounds;
    MapAreaSqRoot = quick_root(MapArea);
    map_tiles() = 0;
    map_tiles() = (Map *)mem_get(MapArea * sizeof(Map));
    if (map_tiles()) {
        MapAbstractLongBounds = (MapLongitudeBounds + 4) / 5;
        MapAbstractLatBounds = (MapLatitudeBounds + 4) / 5;
        MapAbstractArea = MapAbstractLatBounds * ((MapAbstractLongBounds + 1) / 2);
        MapAbstract() = (uint8_t *)mem_get(MapAbstractArea);
        if (MapAbstract()) {
            mapwin_terrain_fixup();
            return false;
        }
    }
    map_shutdown();
    return true;
}

/*
Purpose: Reset the map to a blank state. Doesn't wipe unk_1 and territory fields.
// ORIGINAL: 0x00591040 ?map_wipe@@YAXXZ 0x00591040-0x005910B0
// LEVER: `Map *tile = map_tiles();` walked with `tile++` each iteration,
//        instead of indexing `map_tiles()[i]` (which re-reads the global
//        every field access AND fails to fold the `ZeroMemory` call to a
//        `rep stosd`, forcing a real call) - matches the image's running
//        pointer (`edx`, `add edx, 0x2c`). Field order also fixed to the
//        image's `bit` BEFORE `val3`, and `bit2` folded into one
//        `ZeroMemory` spanning `bit2`+`bit_visible` (the image zeroes both
//        in the same `rep stosd`, `bit2` is never stored separately). `int i`
//        instead of `uint32_t i`: the image's loop-bound compare is `jl`
//        signed. Together: 9/38 (0.682) -> 30/38 (0.921), 38/38 total
//        instructions, same count as the image.
// TRIED: declaring `tile` before `MapRandSeed = random(...)+1` (to chase
//            the image scheduling the `map_tiles()` load between the
//            `random()` call and consuming its result) - worse, 20/38 (0.895).
// size      112 bytes
// prototype 
// callers   3   call targets   1
// kind      game
// flags     sp_ready;purged_ok
// calls     0x00625810
Return Value: n/a
Status: Complete
*/
void __cdecl map_wipe() {
    MapSeaLevel = 0;
    MapSeaLevelCouncil = 0;
    MapLandmarkCount = 0;
    MapRandSeed = random(0, 0x7FFF) + 1;
    Map *tile = map_tiles();
    for (int i = 0; i < (int)MapArea; i++) {
        tile->climate = ALT_BIT_OCEAN;
        tile->contour = 20;
        tile->val2 = 0xF;
        tile->region = 0;
        tile->visibility = 0;
        tile->bit = 0;
        tile->val3 = 0;
        ZeroMemory(&tile->bit2, sizeof(tile->bit2) + sizeof(tile->bit_visible));
        tile++;
    }
}

/*
Purpose: Write map data to a file.
// ORIGINAL: 0x005910B0 ?map_write@@YAHPAUFILE@@@Z 0x005910B0-0x00591125 BYTE_EXACT
// symbol    ?map_write@@YAHPAU_iobuf@@@Z
// size      117 bytes
// prototype int (__cdecl ?map_write@@YAHPAUFILE@@@Z)(FILE* file)
// callers   2   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x0064603F
Return Value: Did an error occur? true/false
Status: Complete
*/
// TRIED: call_diff names the image's target as `__fwrite`
// (src/recovered/0064603f.cpp, 0x0064603F, a CRT-internal
// `__lock_file`/`_fwrite`/`__unlock_file` wrapper, already BYTE_EXACT there
// but NOT a build input) rather than the public `fwrite()`. Neither
// `__fwrite` nor its own callees (`__lock_file`, `__unlock_file`, the raw
// `_fwrite`) are exported by the linked CRT import libraries - `extern "C"`
// forward declarations of any of them fail LNK2001, and defining local
// bodies for all three (to promote the artifact properly) is a bigger
// change than this pass's scope. Left calling the public `fwrite()`.
// LEVER: the image does NOT `&&`-chain the three calls into one condition -
//        the first two are separate `if (!fwrite(...)) return true;` early
//        returns, and the LAST is a direct `return !fwrite(...);` (branchless
//        `neg;sbb;inc` boolean, no third branch at all). Matching that
//        shape is BYTE_EXACT.
BOOL __cdecl map_write(FILE *map_file) {
    if (!fwrite(&MapLongitudeBounds, 2724, 1, map_file)) {
        return true;
    }
    if (!fwrite(map_tiles(), MapArea * sizeof(Map), 1, map_file)) {
        return true;
    }
    return !fwrite(MapAbstract(), MapAbstractArea, 1, map_file);
}

/*
Purpose: Read the map data from a file and write it into memory.
// ORIGINAL: 0x00591130 ?map_read@@YAHPAUFILE@@@Z 0x00591130-0x00591208 BYTE_EXACT
// LEVER: same shape as map_write (0x005910B0) - the image does NOT `||`-chain the last two `fread`s into one condition; each is its own `if (!fread(...)) return true;`. BYTE_EXACT on that alone.
// symbol    ?map_read@@YAHPAU_iobuf@@@Z
// size      216 bytes
// prototype int (__cdecl ?map_read@@YAHPAUFILE@@@Z)(FILE* file)
// callers   2   call targets   4
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00590ED0 0x00592940 0x00644EF2 0x00646178
Return Value: Did an error occur? true/false
Status: Complete
*/
BOOL __cdecl map_read(FILE *map_file) {
    map_shutdown();
    if (!fread(&MapLongitudeBounds, 2724, 1, map_file)) {
        return true;
    }
    map_tiles() = 0;
    MapAbstract() = 0;
    if (map_init()) {
        return true;
    }
    if (!fread(map_tiles(), MapArea * sizeof(Map), 1, map_file)) {
        return true;
    }
    if (!fread(MapAbstract(), MapAbstractArea, 1, map_file)) {
        return true;
    }
    fixup_landmarks();
    return false;
}

/*
Purpose: Get the region value for the specified tile.
// ORIGINAL: 0x00591210 ?abstract_at@@YAHHH@Z 0x00591210-0x00591230 SEMANTIC
// symbol    ?abstract_at@@YAEHH@Z
// size      32 bytes
// prototype int (__cdecl ?abstract_at@@YAHHH@Z)(int xCoord, int yCoord)
// callers   1   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// TRIED: MNEMONIC_ONLY 10 of 12, and it is the same one-instruction
//            plateau abstract_set (0x00591230) records. The image sums the
//            ROW into the base register and leaves the column in its own
//            (`add edx, eax; mov al, byte ptr [ecx + edx]`); VC6 -O2 always
//            pre-sums the two index terms instead (`add eax, ecx; mov al,
//            byte ptr [eax + edx]`), whatever the source says.
// TRIED: six spellings, every one MNEMONIC_ONLY with the divergence at
//            the same instruction 8 - the committed `(x >> 1) + y * mult`,
//            the reversed `y * mult + (x >> 1)`, an explicit `uint8_t *row`
//            local, `row` plus a separate `col` local, a `col` local alone,
//            `row += ...` as a statement, and the pointer-arithmetic
//            `*(MapAbstract() + ... + ...)` form.
Return Value: Abstract value (region)
Status: Complete
*/
// BODY IN map.h, as `MEASURED inline`: the image writes it out at
// some call sites and calls it at others, and a .cpp definition is only ever
// one of those. The marker stays here because that is where the catalogue
// reads it.


/*
Purpose: Set the region value for the specified tile.
// ORIGINAL: 0x00591230 ?abstract_set@@YAXHHE@Z 0x00591230-0x00591253 SEMANTIC
// TRIED: same plateau as abstract_at/alt_at. Image computes a row
//            pointer (base + y*(bounds>>1)) explicitly, then indexes it by
//            x>>1 in the final SIB store; VC6 -O2 always pre-sums the two
//            index terms into one register first regardless of source shape.
//            Tried: (x>>1)+y*mult, y*mult+(x>>1), an explicit `row` pointer
//            local, and a separate `col=x>>1` local before the row - all
//            identical 10/13 MNEMONIC_ONLY, same first divergence.
// size      35 bytes
// prototype void (__cdecl ?abstract_set@@YAXHHE@Z)(int xCoord, int yCoord, unsigned int8 val)
// callers   1   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
// BODY IN map.h, as `MEASURED inline`: the image writes it out at
// some call sites and calls it at others, and a .cpp definition is only ever
// one of those. The marker stays here because that is where the catalogue
// reads it.


/*
Purpose: Quickly check for unit related zone of control conflicts. If a ZOC conflict is found, store
         the coordinates of the tile inside ZOC pointers.
// ORIGINAL: 0x00593830 ?quick_zoc@@YAXHHHHHPAH0@Z 0x00593830-0x005939FC
// TRIED: 12/152, 0.873 similar (best flag set /Oi-) - call count already
//            matches the image (`call_diff` agrees). First divergence is
//            is_ocean's own known plateau (`xor ecx,ecx` before the
//            map_tiles() pointer load in the image, this tree loads the
//            pointer first) - same root cause as is_ocean's standalone note.
// symbol    ?quick_zoc@@YAXIIIHHPAH0@Z
// size      460 bytes
// prototype void (__cdecl ?quick_zoc@@YAXHHHHHPAH0@Z)(int xCoordSrc, int yCoordSrc, int factionID, int xCoordDst, int yCoordDst, int* xCoordZoc, int* yCoordZoc)
// callers   1   call targets   1
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x00644F3A
Return Value: n/a
Status: Complete
*/
void __cdecl quick_zoc(uint32_t x_src, uint32_t y_src, uint32_t faction_id, int x_dst, int y_dst, 
                       int *x_zoc, int *y_zoc) {
    BOOL is_src_ocean = is_ocean(x_src, y_src);
    int search_zoc = -1;
    for (uint32_t i = 0; i < 8; i++) {
        int x_radius = xrange(x_src + RadiusBaseX[i]);
        int y_radius = y_src + RadiusBaseY[i];
        if (on_map(x_radius, y_radius)) {
            int owner = veh_who(x_radius, y_radius);
            if (owner >= 0 && (uint32_t)owner != faction_id 
                && is_ocean(x_radius, y_radius) == is_src_ocean
                && !has_treaty(faction_id, owner, DTREATY_PACT)) {
                // image inlines BOTH vector_dist(x,y,x,y) and vector_dist(x,y) at this
                // one call site (4 direct `abs()` calls, no call to either helper), while
                // both keep real callers elsewhere - so the body is duplicated here rather
                // than marking either `inline` and disturbing those other sites.
                int dist_x = x_dist(x_radius, x_dst);
                int dist_y = abs((int)y_radius - y_dst);
                int abs_dist_x = abs(dist_x);
                int abs_dist_y = abs(dist_y);
                int largest = abs_dist_x;
                if (abs_dist_x <= abs_dist_y) {
                    largest = abs_dist_y;
                }
                int smallest = abs_dist_x;
                if (abs_dist_x >= abs_dist_y) {
                    smallest = abs_dist_y;
                }
                int proximity = largest - ((((abs_dist_y + abs_dist_x) >> 1) - smallest + 1) >> 1);
                if (proximity >= search_zoc) {
                    search_zoc = proximity;
                    *x_zoc = x_radius;
                    *y_zoc = y_radius;
                }
            }
        }
    }
}

/*
Purpose: Determine if the specified offsets are within the range radius.
// ORIGINAL: 0x005A65A0 ?radius_move@@YAHHHH@Z 0x005A65A0-0x005A65D0 BYTE_EXACT
// size      48 bytes
// prototype int (__cdecl ?radius_move@@YAHHHH@Z)(int xCoord, int yCoord, int range)
// callers   1   call targets   0
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     (none)
Return Value: Range radius, otherwise -1 if not within range
Status: Complete
*/
// BODY IN map.h, as `MEASURED inline`: the image writes it out at
// some call sites and calls it at others, and a .cpp definition is only ever
// one of those. The marker stays here because that is where the catalogue
// reads it.


/*
Purpose: Determine if the specified two tiles are within the range radius of each other.
// ORIGINAL: 0x005A65D0 ?radius_move@@YAHHHHHH@Z 0x005A65D0-0x005A662C
// size      92 bytes
// prototype int (__cdecl ?radius_move@@YAHHHHHH@Z)(int xCoordSrc, int yCoordSrc, int xCoordDst, int yCoordDst, int range)
// callers   4   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// LEVER: two fixes, 0.709 -> 0.902 similar (3/40 raw, best flag set /O2 /Gy
//        /GR- /Oy- /GX). (1) BUG IN THIS RECOVERY (not the original): the
//        wraparound test compared against the precomputed `MapLongitude`
//        global and adjusted by `MapLongitudeBounds` - a mismatched pair. The
//        image reads ONLY `MapLongitudeBounds` (0x949870) here, computing its
//        own `/2` inline via `cdq; sub; sar` for the threshold and using the
//        UNHALVED value for the adjustment - the exact shape compass_move
//        (0x005A6630) already documents, with `MapLongitudeBounds` where
//        compass_move uses `MapLongitude`. Rewriting both threshold checks as
//        `-((int)MapLongitudeBounds / 2)` / `((int)MapLongitudeBounds / 2)`
//        and the adjustment as `MapLongitudeBounds` fixed the division shape.
//        (2) Declaring `int y_radius_off = y_dst - y_src;` BEFORE
//        `x_radius_off` (rather than evaluating `y_dst - y_src` inline at the
//        tail call) matches the image computing the y-difference early,
//        interleaved with the x-difference - moved 0.800 -> 0.902.
// TRIED: remaining gap is an esi/ecx register swap running through the
//            whole body - same register-allocation plateau as the rest of
//            this file's xrange/on_map-loop family. Not chased further.
Return Value: Range radius, otherwise -1 if not within range
Status: Complete
*/
int __cdecl radius_move(int x_src, int y_src, int x_dst, int y_dst, int range) {
    int y_radius_off = y_dst - y_src;
    int x_radius_off = x_dst - x_src;
    if (x_radius_off < -((int)MapLongitudeBounds / 2)) {
        x_radius_off += MapLongitudeBounds;
    }
    if (x_radius_off > ((int)MapLongitudeBounds / 2)) {
        x_radius_off -= MapLongitudeBounds;
    }
    return radius_move(x_radius_off, y_radius_off, range);
}

/*
Purpose: Determine if the specified two tiles are within the radius directionally of each other.
// ORIGINAL: 0x005A6630 ?compass_move@@YAHHHHH@Z 0x005A6630-0x005A66E0 BYTE_EXACT
// LEVER: this body had been copy-adapted from radius_move's wraparound test (compare against MapLongitude, adjust by MapLongitudeBounds), but the image never loads MapLongitudeBounds here at all - it computes `MapLongitude / 2` (a signed `cdq; sub; sar` halving, done twice) as the threshold AND adjusts by the un-halved `MapLongitude` itself. Matching that literally (`x_radius_off < -(ML/2)` / `+= ML`, and the mirrored upper bound) is BYTE_EXACT.
// size      176 bytes
// prototype int (__cdecl ?compass_move@@YAHHHHH@Z)(int xCoordSrc, int yCoordSrc, int xCoordDst, int yCoordDst)
// callers   1   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
Return Value: Range radius, otherwise -1 if not within range
Status: Complete
*/
int __cdecl compass_move(int x_src, int y_src, int x_dst, int y_dst) {
    int x_radius_off = x_dst - x_src;
    if (x_radius_off < -((int)MapLongitude / 2)) {
        x_radius_off += MapLongitude;
    }
    if (x_radius_off > ((int)MapLongitude / 2)) {
        x_radius_off -= MapLongitude;
    }
    int y_radius_off = y_dst - y_src;
    int direction_x = (x_radius_off > 0) ? 1 : (x_radius_off >= 0) - 1;
    int direction_y = (y_radius_off > 0) ? 1 : (y_radius_off >= 0) - 1;
    for (int i = 0; i < 9; i++) {
        if (direction_x == (RadiusBaseX[i] > 0 ? 1 : (RadiusBaseX[i] >= 0) - 1)
            && direction_y == (RadiusBaseY[i] > 0 ? 1 : (RadiusBaseY[i] >= 0) - 1)) {
            return i;
        }
    }
    return -1;
}

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
Purpose: Check whether there is a sensor available in the specified tile.
// ORIGINAL: 0x005BF010 ?is_sensor@@YAHHH@Z 0x005BF010-0x005BF12D
// LEVER: a prior pass's comment read "removed unnecessary duplicate calculation of distX",
//        caching `x_dist(x, Bases[base_id].x)` into a local before the `!dist_x || dist_x
//        == 2` check. The image calls `x_dist` (0x00644F3A, abs()) TWICE - once per side of
//        the `||`, with no CSE across it - while `dist_y` (a plain `abs()`, not `x_dist`) IS
//        computed once and cached, exactly as this tree already had it. Restoring the
//        second `x_dist` call moved the compiled instruction count from 73 toward the
//        image's 102 (73 -> 87).
// TRIED: remaining divergence starts in the prologue - `mov ecx,[ebp+0xc]` (image) vs
//            this tree spilling y to ebx (`push ebx; mov ebx,[ebp+0xc]`) - the same
//            register-allocation plateau as the rest of this family.
// size      285 bytes
// prototype int (__cdecl ?is_sensor@@YAHHH@Z)(int xCoord, int yCoord)
// callers   8   call targets   3
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x004E3B80 0x0050BA00 0x00644F3A
Return Value: 0 (no sensor), 1 (sensor array via terraforming), 2 (Geosynchronous Survey Pod)
Status: Complete
*/
int __cdecl is_sensor(int x, int y) {
    if (bit_at(x, y) & BIT_SENSOR_ARRAY) {
        return 1; // Sensor Array built in tile
    }
    int base_id = base_find(x, y);
    if (base_id != -1) {
        if (!x_dist(x, Bases[base_id].x) || x_dist(x, Bases[base_id].x) == 2) {
            int dist_y = abs((int)y - Bases[base_id].y);
            if (!dist_y || dist_y == 2) {
                if (has_fac_built(FAC_GEOSYNC_SURVEY_POD, base_id)) {
                    return 2; // Geosynchronous Survey Pod
                }
            }
        }
    }
    return 0; // No sensor found
}

/*
Purpose: Check whether a sensor array is worth building on the specified tile.
// ORIGINAL: 0x00564EB0 ?good_sensor@@YAHHHH@Z 0x00564EB0-0x00565157
// size      679 bytes
// prototype int (__cdecl ?good_sensor@@YAHHHH@Z)(int factionID, int xCoord, int yCoord)
// callers   2   call targets   5
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x004E3A50 0x004E3EF0 0x00592030 0x005B9F20 0x005BF010
Return Value: Is the tile a good sensor site? true/false
Status: Complete

TRIED: best 14/242, 0.419 similar (best flag set /O2 /Gy /GR- /Oy- /GX).
`has_tech` (0x005B9F20, technology.h) is `MEASURED inline` and gets inlined
whole at this call site, where the image keeps a real call - dumping this
tree's own compiled listing shows 7 call opcodes against the image's 9, the
two missing being has_tech's. Same cross-file per-call-site conflict as
get_there/port_to_port (0x0056B320, this file) - technology.h's `inline` is
what fixes has_tech's OTHER callers, and reverting it here is out of this
batch's reach (map.cpp/map.h only) without regressing those. The remaining
gap otherwise is the usual xrange/on_map/is_ocean-loop register-scheduling
plateau documented across this file. Not chased further.

Three questions in order, and every one of them can answer no on its own.

WHO OWNS IT. The tile must already be the faction's own territory, and if it is fungus at
ALT_BIT_OCEAN_SHELF or above the faction must hold Rules->tech_improve_fungus_sqr - the same
technology that lets it work a fungus square at all, at offset 0x3C of RulesBasic.

IS IT WANTED. The 25 tiles of the surrounding radius are scanned. Any one of them that already
carries a sensor standing on the faction's own territory answers no outright, which is what stops
sensors being stacked on top of each other. Then, for the NINE innermost tiles only, three
independent reasons to want one are collected as bits of a mask:

  1 - the tile is not the faction's own territory. A sensor on the border is worth having.
  2 - one of the nine holds a base of this faction.
  4 - that base has no sensor anywhere in its own 25-tile radius.

A mask of zero answers no. Nothing here weighs the three against each other; any one will do, and
2 without 4 is reachable because the base's radius is larger than the nine tiles just searched.

Note that bit 2 is set once per qualifying base and bit 4 is decided per base, so a second base
without cover sets 4 even when the first one had it. The original tests the flags only for being
nonzero, so this is a distinction without a difference, and it is reproduced as written.

IS THE TILE FREE. A resource bonus, a base, a monolith, a condenser, a thermal borehole, a mine or
a solar collector each answer no, because the sensor would displace something already there. What
is left answers yes when it is ROCKY, or when it is arid - neither RAINFALL_RAINY nor
RAINFALL_MOIST - which are the tiles worth least as terrain. Anything else answers yes only if it
is fungus at ALT_BIT_OCEAN_SHELF or above, and no otherwise.

Verification note: the sweep against recovery-gameplay-tests kills 23 of 31 valid mutants twice
over, and the eight survivors are two proven equivalences rather than untested behaviour.

BIT 4 CANNOT CHANGE THE ANSWER, and neither can the 25-tile search that decides it. Seven
survivors are that one fact: the `25 -> 0`, `0 -> 1` and `< -> <=` mutants of the inner loop, the
inverted territory comparison inside it, the dropped `base_covered = true`, the dropped
`reasons |= 4` and its `4 -> 0`. The proof is short. `reasons` is written in exactly three places
and read in exactly one, `if (!reasons)`. Bit 4 is set only inside the block that has already
executed `reasons |= 2` two statements earlier, so every state in which bit 4 could be set is
already nonzero, and nothing distinguishes 6 from 2. The whole cover search is therefore dead as
far as the return value goes - in the original as much as here, since this is a transcription of
what 0x00564FEE through 0x00565076 does.

It is kept because it is what the original executes, and because it is not quite side-effect
free: is_sensor falls through to base_find, which writes BaseFindDist. That write is overwritten
again by every later on-map iteration of the OUTER loop, so it survives the call only when the
base sits in the inner nine and every remaining radius tile is off the map - which no fixture
here builds and which changes nothing this function returns either way.

The eighth survivor is bonus_at's third argument. The parameter is declared UNUSED in
map.cpp:00592030's recovery and read nowhere in its body, so 0 and 1 are the same call. The
original passes 0 and so does this.
*/
int __cdecl good_sensor(int faction_id, int x, int y) {
    if (whose_territory(faction_id, x, y, NULL, false) != faction_id) {
        return false;
    }
    Map *tile = map_loc(x, y);
    if (tile->bit & BIT_FUNGUS && (tile->climate & 0xE0) >= ALT_BIT_OCEAN_SHELF
        && !has_tech(Rules->tech_improve_fungus_sqr, faction_id)) {
        return false;
    }
    int reasons = 0;
    for (int i = 0; i < 25; i++) {
        int x_radius = site_xrange(x + RadiusOffsetX[i]);
        int y_radius = y + RadiusOffsetY[i];
        if (!on_map(x_radius, y_radius)) {
            continue;
        }
        if (is_sensor(x_radius, y_radius)
            && whose_territory(faction_id, x_radius, y_radius, NULL, false) == faction_id) {
            return false;
        }
        if (i >= 9) {   // the inner ring only, from here down
            continue;
        }
        int base_id = base_at(x_radius, y_radius);
        if (base_id >= 0 && Bases[base_id].faction_id_current == faction_id) {
            reasons |= 2;
            BOOL base_covered = false;
            for (int j = 0; j < 25; j++) {
                int x_base = site_xrange(x_radius + RadiusOffsetX[j]);
                int y_base = y_radius + RadiusOffsetY[j];
                if (!on_map(x_base, y_base)) {
                    continue;
                }
                if (is_sensor(x_base, y_base)
                    && whose_territory(faction_id, x_base, y_base, NULL, false) == faction_id) {
                    base_covered = true;
                    break;
                }
            }
            if (!base_covered) {
                reasons |= 4;
            }
        }
        if (whose_territory(faction_id, x_radius, y_radius, NULL, false) != faction_id) {
            reasons |= 1;
        }
    }
    if (!reasons) {
        return false;
    }
    if (bonus_at(x, y, 0)) {
        return false;
    }
    // base_who(x, y) inlined by hand: `site` is already map_loc(x, y), and the
    // image makes no call here at all (9 calls total, call_diff confirms) even
    // though base_who is `MEASURED inline` and does inline at its OTHER call
    // sites - this one specific site is where this tree's compiler keeps it real.
    Map *site = map_loc(x, y);
    if (site->bit & BIT_BASE_IN_TILE && (site->val2 & 0xF) < MaxPlayerNum) {
        return false;
    }
    uint32_t bit = site->bit;
    if (bit & (BIT_MONOLITH | BIT_CONDENSER | BIT_THERMAL_BORE)) {
        return false;
    }
    if (bit & (BIT_MINE | BIT_SOLAR_TIDAL)) {
        return false;
    }
    if ((site->val3 & 0xC0) > TERRAIN_BIT_ROLLING) {
        return true;   // rocky
    }
    if (!(site->climate & (RAINFALL_RAINY | RAINFALL_MOIST))) {
        return true;   // arid
    }
    if (!(bit & BIT_FUNGUS)) {
        return false;
    }
    return (site->climate & 0xE0) >= ALT_BIT_OCEAN_SHELF;
}

/*
Purpose: Check if faction controls the initial tile (code offset 0) of the Manifold Nexus.
// ORIGINAL: 0x005BF130 ?has_temple@@YAHH@Z 0x005BF130-0x005BF1EE
// LEVER: needs /Oy- (the marker's own `frame` flag) to even form the ebp-frame
//        prologue the image has; that alone was 3/70 -> 51/70. Caching
//        `uint32_t bit2 = map_loc(x, y)->bit2;` once and testing
//        `bit2 & MASK` / `!(bit2 >> 24)` against the LOCAL (rather than
//        `bit2_at(x, y)` / `code_at(x, y)` as two separate inline calls) let
//        VC6 fold the `*sizeof(Map)` scale into the load's own SIB addressing
//        the way the image does, taking it to 65/70, 0.986 similar.
//        TRIED: caching a `Map *tile` pointer instead of the `bit2` value
//        (65->21/70 - the pointer materialises with its own `lea` first,
//        which the image never does); swapping the final visibility test's
//        `&` operand order (no change, still 65/70) - the one remaining
//        differing run (`map_loc(x, y)->visibility & (1 << faction_id)`,
//        register choice only: esi/edx/eax vs image's eax/ecx/edx for the
//        same three values) did not move under either.
// size      190 bytes
// prototype int (__cdecl ?has_temple@@YAHH@Z)(int factionID)
// callers   1   call targets   1
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x004E3EF0
Return Value: Does faction control Nexus? true/false
Status: Complete
*/
BOOL __cdecl has_temple(int faction_id) {
    for (int y = 0; y < MapLatitudeBounds; y++) {
        for (int x = y & 1; x < MapLongitudeBounds; x += 2) {
            uint32_t bit2 = map_loc(x, y)->bit2;
            if ((bit2 & (BIT2_UNK_80000000 | BIT2_NEXUS)) == BIT2_NEXUS
                && !(bit2 >> 24)
                && faction_id == whose_territory(faction_id, x, y, NULL, false)
                && map_loc(x, y)->visibility & (1 << faction_id)) { // tile visible
                return true;
            }
        }
    }
    return false;
}

/*
Purpose: Handle setting the world altitude.
// ORIGINAL: 0x005C2020 ?world_alt_set@@YAXHHHH@Z 0x005C2020-0x005C2374
// TRIED: MISMATCH plateau, not chased to BYTE_EXACT (852-byte function,
//   282 image instructions, best 48/282 similar 0.298 at /c /O2 /Gy /GR-
//   /Oy- /GX). call_diff agrees on call count. The compiled body runs 396
//   instructions against the image's 282 from early on (RadiusRange/
//   RadiusOffsetX array reads land in a different register, and the loop
//   guard around `alt_at` pulls in two extra real calls the image's copy
//   doesn't show at that point) - a deep restructuring, not a 2-3 candidate
//   fix, so left at the pre-existing body.
// size      852 bytes
// prototype void (__cdecl ?world_alt_set@@YAXHHHH@Z)(int xCoord, int yCoord, int altitude, int isSetBoth)
// callers   23   call targets   4
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x00591290 0x005918F0 0x00591B10 0x00592030
Return Value: n/a
Status: Complete - testing
*/
void __cdecl world_alt_set(int x, int y, int altitude, BOOL is_set_both) {
    ZeroMemory(MapBaseSubmergedCount, sizeof(int) * MaxPlayerNum);
    memset(MapBaseIdClosestSubmergedVeh, -1, sizeof(int) * MaxPlayerNum);
    if (on_map(x, y)) {
        is_set_both ? alt_set_both(x, y, altitude) : alt_set(x, y, altitude);
    }
    BOOL has_set_alt;
    for (int i = 1, alt = altitude - 1; i < altitude; i++, alt--) {
        has_set_alt = false;
        for (int j = RadiusRange[i]; j < RadiusRange[i + 1]; j++) {
            int x_radius = xrange(x + RadiusOffsetX[j]);
            int y_radius = y + RadiusOffsetY[j];
            if (on_map(x_radius, y_radius) && alt_at(x_radius, y_radius) < alt) {
                if (is_set_both) {
                    alt_set_both(x_radius, y_radius, alt);
                    if (anything_at(x_radius, y_radius) < 0 && !bonus_at(x_radius, y_radius, 0)) {
                        owner_set(x_radius, y_radius, -1);
                    }
                } else {
                    alt_set(x_radius, y_radius, alt);
                }
                has_set_alt = true;
            }
        }
        if (!has_set_alt) {
            break;
        }
    }
    int max_alt = 8 - altitude;
    for (i = 1, alt = i + altitude; i < max_alt; i++, alt++) {
        has_set_alt = false;
        for (int j = RadiusRange[i]; j < RadiusRange[i + 1]; j++) {
            int x_radius = xrange(x + RadiusOffsetX[j]);
            int y_radius = y + RadiusOffsetY[j];
            if (on_map(x_radius, y_radius) && alt_at(x_radius, y_radius) < alt) {
                if (is_set_both) {
                    alt_set_both(x_radius, y_radius, alt);
                    if (anything_at(x_radius, y_radius) < 0 && !bonus_at(x_radius, y_radius, 0)) {
                        owner_set(x_radius, y_radius, -1);
                    }
                } else {
                    alt_set(x_radius, y_radius, alt);
                }
                has_set_alt = true;
            }
        }
        if (!has_set_alt) {
            break;
        }
    }
}

/*
Purpose: Handle raising the altitude of the specified tile.
// ORIGINAL: 0x005C2380 ?world_raise_alt@@YAXHH@Z 0x005C2380-0x005C23DE BYTE_EXACT
// LEVER: the image `inc eax`s BEFORE the push, so the argument is `altitude + 1`, not the
//            original value - `altitude++` (post-increment) pushed the pre-increment value;
//            `++altitude` fixed it. Also `int altitude`, not `uint32_t`: the image's `< 6`
//            compare is `jge`/signed, `uint32_t` gave `jae`. Both together: 0.961 -> BYTE_EXACT.
// size      94 bytes
// prototype void (__cdecl ?world_raise_alt@@YAXHH@Z)(int xCoord, int yCoord)
// callers   4   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005C2020
Return Value: n/a
Status: Complete - testing
*/
void __cdecl world_raise_alt(int x, int y) {
    if (on_map(x, y)) {
        int altitude = alt_at(x, y);
        if (altitude < 6) {
            world_alt_set(x, y, ++altitude, true);
        }
    }
}

/*
Purpose: Handle lowering the altitude of the specified tile.
// ORIGINAL: 0x005C23E0 ?world_lower_alt@@YAXHH@Z 0x005C23E0-0x005C243B BYTE_EXACT
// LEVER: same as world_raise_alt (0x005C2380) - the image `dec eax`s BEFORE the push, so
//            the pushed argument is `altitude - 1`; `altitude--` (post-decrement) pushed the
//            pre-decrement value, `--altitude` fixed it. `int altitude`, not `uint32_t`, to
//            match.
// size      91 bytes
// prototype 
// callers   3   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005C2020
Return Value: n/a
Status: Complete - testing
*/
void __cdecl world_lower_alt(int x, int y) {
    if (on_map(x, y)) {
        int altitude = alt_at(x, y);
        if (altitude) {
            world_alt_set(x, y, --altitude, true);
        }
    }
}

/*
Purpose: Set up the brush for creating world terrain.
// ORIGINAL: 0x005C2440 ?brush@@YAXHHH@Z 0x005C2440-0x005C27E1
// TRIED: MISMATCH plateau, not chased to BYTE_EXACT (929-byte function,
//   304 image instructions, best 8/304 similar 0.370 at /c /O1 /Ob0 /Gy
//   /GR- /Oy- /GX). call_diff agrees on call count. Diverges from the first
//   loop iteration on: which register RadiusOffsetX[i]/y_radius land in
//   (matches the file's known ebx/edi plateau), and the `on_map()` +
//   `alt_at()` guard is ordered/interleaved differently than this tree's
//   compile produces - a deep restructuring, not a 2-3 candidate fix, so
//   left at the pre-existing body.
// size      929 bytes
// prototype void (__cdecl ?brush@@YAXHHH@Z)(int xCoord, int yCoord, int altitude)
// callers   1   call targets   3
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x0046AEF0 0x00591AD0 0x005C2020
Return Value: n/a
Status: Complete - testing
*/
void __cdecl brush(int x, int y, int altitude) {
    BOOL use_draw_radius = false;
    for (int i = 0; i < 4; i++) {
        if (i != 2) {
            int x_radius = xrange(x + RadiusOffsetX[i]);
            int y_radius = y + RadiusOffsetY[i];
            if (on_map(x_radius, y_radius) && alt_at(x_radius, y_radius) != altitude) {
                BOOL skip = false;
                for (uint32_t j = 0; j < 8; j++) {
                    int x_rad_base = xrange(x_radius + RadiusBaseX[j]);
                    int y_rad_base = y_radius + RadiusBaseY[j];
                    if (on_map(x_rad_base, y_rad_base) && !is_ocean(x_rad_base, y_rad_base)
                        && !region_at(x_rad_base, y_rad_base)) {
                        skip = true;
                    }
                }
                if (!skip) {
                    if (altitude != 3) {
                        for (uint32_t j = 0, k = 0; j < 7; j += 3, k += 2) {
                            int x_rad_base = xrange(x_radius + RadiusBaseX[k + 1]);
                            int y_rad_base = y_radius + RadiusBaseY[k + 1];
                            if (on_map(x_rad_base, y_rad_base)
                                && altitude_at(x_rad_base, y_rad_base) == ALT_BIT_SHORE_LINE) {
                                int x_rad_base2 = xrange(x_radius + RadiusBaseX[(k + 2) % 8]);
                                int y_rad_base2 = y_radius + RadiusBaseY[(k + 2) % 8];
                                int x_rad_base3 = xrange(x_radius + RadiusBaseX[k % 8]);
                                int y_rad_base3 = y_radius + RadiusBaseY[k % 8];
                                if (on_map(x_rad_base2, y_rad_base2) 
                                    && on_map(x_rad_base3, y_rad_base3)
                                    && altitude_at(x_rad_base2, y_rad_base2) < ALT_BIT_SHORE_LINE
                                    && altitude_at(x_rad_base3, y_rad_base3) < ALT_BIT_SHORE_LINE) {
                                    skip = true;
                                    break;
                                }
                            }
                        }
                    }
                    if (!skip) {
                        region_set(x_radius, y_radius, 1);
                        use_draw_radius |= alt_at(x_radius, y_radius) != altitude;
                        world_alt_set(x_radius, y_radius, altitude, true);
                        if (GameState & STATE_OMNISCIENT_VIEW && altitude > 3) {
                            temp_set(x_radius, y_radius, 1);
                        }
                        BrushVal1 += 1;
                        BrushVal2 += 1;
                    }
                }
            }
        }
    }
    if (GameState & STATE_OMNISCIENT_VIEW && use_draw_radius) {
        draw_radius(x, y, 2, 2);
    }
}

/*
Purpose: Paint land to assist in the creation of the world terrain.
// ORIGINAL: 0x005C27F0 ?paint_land@@YAXHHHH@Z 0x005C27F0-0x005C28E6
// LEVER: two fixes, 0.849 -> 0.904 similar (9/89). (1) `x_rad_base`/
//        `y_rad_base` are a CUMULATIVE random walk - the image's inlined
//        `xrange()` adds `RadiusBaseX[offset]` to the PREVIOUS `esi`
//        (`x_rad_base`), and `edi` (`y_rad_base`) is a plain running
//        `add edi, RadiusBaseY[offset]` with no reset - not an offset from
//        the fixed `x`/`y` parameters every iteration, which is a BUG this
//        recovery had (paints a single-tile-radius dot around the start
//        instead of wandering). (2) `offset` as `int`: the image's
//        `rand() % 8` carries the full signed-modulo fixup
//        (`and 0x80000007; jns; dec; or 0xfffffff8; inc`), which `uint32_t`
//        drops to a plain `and eax, 7`.
// TRIED: not chased past this MISMATCH plateau - remaining gap starts in
//            the prologue frame-size (image reserves 3 stack dwords, this
//            tree keeps `unk_val` in a register alone).
// size      246 bytes
// prototype void (__cdecl ?paint_land@@YAXHHHH@Z)(int xCoord, int yCoord, int altitude, int radius)
// callers   2   call targets   2
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005C2440 0x0064601D
Return Value: n/a
Status: Complete - testing
*/
void __cdecl paint_land(int x, int y, int altitude, int radius) {
    BrushVal2 = 0;
    for (int i = 0; i < 2000 && BrushVal2 < radius; i++) {
        int search = -1;
        uint32_t unk_val = 0;
        int x_rad_base = x;
        int y_rad_base = y;
        do {
            brush(x_rad_base, y_rad_base, altitude);
            int offset = rand() % 8 | 1;
            if (offset == search) {
                if (++unk_val > ((MapLandCoverage * MapLandCoverage) + 2)) {
                    offset = (offset - (MapLandCoverage * MapLandCoverage) + unk_val - 2) % 8;
                }
            } else {
                search = offset;
                unk_val = 0;
            }
            x_rad_base = xrange(x_rad_base + RadiusBaseX[offset]);
            y_rad_base += RadiusBaseY[offset];
            i++;
        } while (i < 2000 && on_map(x_rad_base, y_rad_base) && BrushVal2 < radius);
    }
}

/*
Purpose: Build out the map continents.
// ORIGINAL: 0x005C28F0 ?build_continent@@YAXH@Z 0x005C28F0-0x005C2B3C
// LEVER: `osmx calls` shows exactly 2 game calls (paint_land,
//        do_all_non_input) plus rand() itself - `rnd(bounds, NULL)` is
//        cross-TU and can never inline, so all three call sites had to be
//        rewritten as rnd's own body (`(bounds - 1 > 0) ? rand() % bounds :
//        0`), the same idiom as alt_set_both. The region-zero loop also
//        moved to the `Map *tile` pointer-walk idiom (map_wipe's lever).
//        0.658 -> 0.698 similar (13/201).
// TRIED: not chased past this MISMATCH plateau - 588-byte body, deeply
//            nested ratio/radius conditionals; the remaining gap starts in
//            the very first loop's own scheduling (image loads MapArea into
//            ecx before zeroing eax, this tree the other order).
// size      588 bytes
// prototype
// callers   1   call targets   3
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005C27F0 0x005FCB20 0x0064601D
Return Value: n/a
Status: Complete - testing
*/
void __cdecl build_continent(int size) {
    Map *tile = map_tiles();
    for (uint32_t i = 0; i < MapArea; i++) {
        tile->region = 0;
        tile++;
    }
    int coverage = MapLandCoverage;
    if (coverage && BrushVal1 >= WorldBuildVal1) {
        coverage--;
    }
    int radius = WorldBuilder->continent_mod * coverage * coverage + WorldBuilder->continent_base;
    int x;
    int y;
    int count = 0;
    do {
        int x_bound = MapLongitudeBounds - (MapIsFlat * 8);
        x = ((x_bound - 1 > 0) ? rand() % x_bound : 0) + MapIsFlat * 4;
        int y_bound = MapLatitudeBounds - 8;
        y = ((y_bound - 1 > 0) ? rand() % y_bound : 0) + 4;
        if (x & 1) {
            x--;
        }
        if (y & 1) {
            x++; // validate whether this should be y and/or reasoning for x
        }
    } while (count++ < 100 && alt_at(x, y) >= ALT_BIT_SHORE_LINE);
    if (alt_at(x, y) < ALT_BIT_SHORE_LINE) {
        uint32_t ratio = (size * 3200) / MapArea;
        if (ratio > WorldBuilder->cont_size_ratio5 || BrushVal1 >= WorldBuildVal1) {
            radius /= 4;
            if (BrushVal1 >= WorldBuildVal1 && ratio > WorldBuilder->cont_size_ratio5) {
                radius = rand() % 3 + 1;
            }
        } else if (ratio <= WorldBuilder->cont_size_ratio4) {
            if (ratio <= WorldBuilder->cont_size_ratio3) {
                if (ratio <= WorldBuilder->cont_size_ratio2) {
                    if (ratio < WorldBuilder->cont_size_ratio1 && MapLandCoverage > 1) {
                        radius += radius / 2;
                    }
                } else {
                    int r_bound = 4 - MapLandCoverage;
                    if ((r_bound - 1 > 0) ? rand() % r_bound : 0) {
                        radius /= 2;
                    }
                }
            } else {
                radius /= 2;
            }
        } else {
            radius /= 3;
        }
        if (BrushVal1 < WorldBuildVal1 && radius < 4) {
            radius = 4;
        }
        paint_land(x, y, ALT_SHORE_LINE, radius);
        do_all_non_input();
    }
}

/*
Purpose: Build out the map hills.
// ORIGINAL: 0x005C2B40 ?build_hills@@YAXH@Z 0x005C2B40-0x005C2CA8
// LEVER: same as build_continent (0x005C28F0) - `osmx calls` shows only
//        paint_land/do_all_non_input plus rand() itself, so the two
//        `rnd(bounds, NULL)` cross-TU calls were rewritten as rnd's own
//        body. 0.677 -> 0.743 similar.
// TRIED: not chased past this MISMATCH plateau - 360-byte body, same
//            prologue frame-size gap as build_continent.
// size      360 bytes
// prototype
// callers   1   call targets   3
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005C27F0 0x005FCB20 0x0064601D
Return Value: n/a
Status: Complete - testing
*/
void __cdecl build_hills(int altitude) {
    int x;
    int y;
    int i = 0;
    BOOL keep_going = true;
    do {
        int x_bound = MapLongitudeBounds - (MapIsFlat * 8);
        x = ((x_bound - 1 > 0) ? rand() % x_bound : 0) + MapIsFlat * 4;
        int y_bound = MapLatitudeBounds - 8;
        y = ((y_bound - 1 > 0) ? rand() % y_bound : 0) + 4;
        if (x & 1) {
            x--;
        }
        if (y & 1) {
            x++; // should this have been y?
        }
        i += 2;
        if (i > 100) {
            return;
        }
        for (uint32_t j = 0; j < 9; j++) {
            int x_radius = xrange(x + RadiusOffsetX[j]);
            int y_radius = y + RadiusOffsetY[j];
            if (on_map(x_radius, y_radius) && alt_at(x_radius, y_radius) < (altitude - 1)) {
                break;
            }
        }
        keep_going = false;
        int plat_mod = WorldBuilder->plateau_mod * (2 - MapOceanCoverage);
        if (altitude != ALT_1_LEVEL_ABOVE_SEA) {
            plat_mod /= 4;
        }
        uint32_t plat_base = WorldBuilder->plateau_base;
        if (MapOceanCoverage >= 2) {
            plat_base /= 2;
        }
        paint_land(x, y, altitude, plat_mod + plat_base);
        do_all_non_input();
    } while (keep_going); // better way to set up outer loop?
}

/*
Purpose: Build out the world river beds.
// ORIGINAL: 0x005C3680 ?world_riverbeds@@YAXXZ 0x005C3680-0x005C38AF
// LEVER: `osmx calls` shows only ONE game call (bit_set) plus rand() itself
//        - both `rnd(bounds, NULL)` sites are cross-TU and were rewritten
//        as rnd's own body (same idiom as build_continent/build_hills), and
//        the riverbed-clearing loop moved to the `Map *tile` pointer-walk
//        idiom (map_wipe's lever). 0.711 -> 0.784 similar.
// TRIED: not chased past this MISMATCH plateau - same `is_ocean`
//            map_tiles-load-timing family ceiling as the standalone
//            `is_ocean`/`port_to_port` plateau this batch's sibling pass
//            already documented, on top of a 559-byte body.
// size      559 bytes
// prototype
// callers   1   call targets   2
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x00591D60 0x0064601D
Return Value: n/a
Status: Complete - testing
*/
void __cdecl world_riverbeds() {
    Map *tile = map_tiles();
    for (uint32_t i = 0; i < MapArea; i++) {
        tile->bit &= ~(BIT_RIVERBED);
        tile++;
    }
    uint32_t riverbed_count = 0;
    uint32_t max_riverbeds = (MapArea * ((4 - MapOceanCoverage) * (WorldBuilder->rivers_base
        + MapCloudCover * WorldBuilder->rivers_rain_mod) / 3)) / 3200;
    for (i = 0; i < 4000 && riverbed_count < max_riverbeds; i++) {
        int x_bound = MapLongitudeBounds;
        int x = (x_bound - 1 > 0) ? rand() % x_bound : 0;
        int y_bound = MapLatitudeBounds;
        int y = (y_bound - 1 > 0) ? rand() % y_bound : 0;
        if (x & 1) {
            x--;
        }
        if (y & 1) {
            x++;
        }
        if (!is_ocean(x, y) && !(bit_at(x, y) & BIT_RIVERBED)) {
            uint32_t search_alt = 0;
            int x_search = -1;
            int y_search = -1;
            for (int j = 0; j < RadiusRange[4]; j++) {
                int x_radius = xrange(x + RadiusOffsetX[j]);
                int y_radius = y + RadiusOffsetY[j];
                if (on_map(x_radius, y_radius)) {
                    if (bit_at(x_radius, y_radius) & BIT_RIVERBED) {
                        break; // exit if riverbed already in search area
                    }
                    if (j < RadiusRange[3]) { // limit scope of where riverbeds can be in search
                        uint32_t alt = alt_at(x_radius, y_radius);
                        if (alt > search_alt) {
                            search_alt = alt;
                            x_search = x_radius; // replaced storing j offset with radius values
                            y_search = y_radius;
                        }
                    }
                }
            }
            if (x_search >= 0) {
                bit_set(x_search, y_search, BIT_RIVERBED, true);
                riverbed_count++;
            }
        }
    }
}

/*
Purpose: Determine if there are any issues with how the world continents are set up.
// ORIGINAL: 0x005C40F0 ?world_validate@@YAHXZ 0x005C40F0-0x005C416B
// LEVER: three fixes, 0.609 -> 0.914 similar (20/52). (1) `val1`/`val2`/
//        `tile_count` as `int`, not `uint32_t` - the image's final
//        threshold uses a SIGNED div-by-3 magic-constant `imul` and a
//        SIGNED `sar` div-by-2, both lost under unsigned. (2) the
//        accumulate step rewritten `if (tile_count >= val2) {val1=val2;
//        val2=tile_count;} else if (tile_count >= val1) {val1=tile_count;}`
//        - logically the same as the nested `if (tile_count < val2)` this
//        had, but matches the image's actual `jl`/fall-through order.
//        (3) BRANCH POLARITY on the ternary: the image's fall-through
//        (no jump) computes `val2/2` and jumps away for
//        `MapLandCoverage==1`'s `(val2*2)/3` - `if (MapLandCoverage != 1)
//        {val2/2} else {(val2*2)/3}` matches; the textual `==1` order did
//        not.
// TRIED: remaining gap is an esi/edi register swap between `val1` and
//            the `region` loop counter, present at every flag set tried;
//            swapping `val1`/`val2` declaration order made it one
//            instruction worse (19/52), not better.
// size      123 bytes
// prototype 
// callers   1   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x0059C790
Return Value: Are there any issues with the world continents? true/false
Status: Complete - testing
*/
BOOL __cdecl world_validate() {
    Paths->continents();
    int val1 = 0; // rename to better var descriptions
    int val2 = 0; // rename to better var descriptions
    for (uint32_t region = 1; region < MaxRegionLandNum; region++) {
        if (!bad_reg(region)) {
            int tile_count = Continents[region].tile_count;
            if (tile_count >= val2) {
                val1 = val2;
                val2 = tile_count;
            } else if (tile_count >= val1) {
                val1 = tile_count;
            }
        }
    }
    int threshold;
    if (MapLandCoverage != 1) {
        threshold = val2 / 2;
    } else {
        threshold = (val2 * 2) / 3;
    }
    return val1 < threshold;
}

/*
Purpose: Set up the world temperature.
// ORIGINAL: 0x005C4170 ?world_temperature@@YAXXZ 0x005C4170-0x005C4401
// TRIED: MISMATCH plateau, not chased to BYTE_EXACT (657-byte function,
//   226 image instructions). LEVER that DID help: `MapLatitudeBounds /
//   WorldBuilder->solar_energy` (and the three siblings) is `int / uint32_t`,
//   which promotes to an UNSIGNED `div`; the image uses signed `idiv` at all
//   four, so each RHS is cast `(int)`. Raised best similarity 0.611 -> 0.654
//   (best flags /c /O2 /Ob0 /Gy /GR- /GX). Remaining gap starts at the
//   `bit2_at`/`code_at` pair in the loop guard: the image inlines both, this
//   tree's compile keeps them as real calls at this nesting depth - an
//   inlining-heuristic difference, not chased further.
// size      657 bytes
// prototype 
// callers   2   call targets   4
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x00591AD0 0x005FCB20 0x006257E0 0x00625810
Return Value: n/a
Status: Complete - testing
*/
void __cdecl world_temperature() {
    random_reseed(MapRandSeed + 17);
    // SIGNED DIVISION: `WorldBuilder`'s fields are `uint32_t`, and dividing
    // `int / uint32_t` promotes to an UNSIGNED `div`; the image uses `idiv`
    // at all four, so the RHS is cast back to `int` to match.
    int temp_heat = MapLatitudeBounds / (int)WorldBuilder->solar_energy;
    int thermal_banding = MapLatitudeBounds / (int)WorldBuilder->thermal_band;
    int thermal_deviance = MapLatitudeBounds / (int)WorldBuilder->thermal_deviance;
    int global_warming = MapLatitudeBounds / (int)WorldBuilder->global_warming;
    for (int y = 0; y < MapLatitudeBounds; y++) {
        for (int x = y & 1; x < MapLongitudeBounds; x += 2) {
            if ((bit2_at(x, y) & (BIT2_UNK_80000000 | BIT2_CRATER)) != BIT2_CRATER
                || code_at(x, y) >= 21) {
                int rand_seed = random(0, thermal_deviance * 2);
                int rand_orbit = random(0, MapPlanetaryOrbit + 1);
                int val1 = (MapLatitudeBounds / 2) - rand_seed - y + thermal_deviance;
                if (val1 < 0) { // abs?
                    val1 = rand_seed - (MapLatitudeBounds / 2) - thermal_deviance + y;
                }
                int val2 = (thermal_banding / 2 + (val1 - temp_heat * (MapPlanetaryOrbit - 1)
                    - MapSeaLevelCouncil * global_warming) * 2) / thermal_banding;
                int temperature = (val2 > 2) ? ((val2 <= 9) + 1) : 3;
                int alt = alt_at(x, y);
                for (int i = 0; i < 8; i++) {
                    int x_radius = xrange(x + RadiusOffsetX[i]);
                    int y_radius = y + RadiusOffsetY[i];
                    if (on_map(x_radius, y_radius) && bit_at(x_radius, y_radius) & BIT_THERMAL_BORE
                        && (alt <= ALT_1_LEVEL_ABOVE_SEA || alt_at(x_radius, y_radius) >= alt)) {
                        temperature++;
                        break;
                    }
                }
                if (temperature > 1) {
                    if (alt <= ALT_2_LEVELS_ABOVE_SEA) {
                        // == ALT_2_LEVELS_ABOVE_SEA, simplify?
                        if (alt > ALT_1_LEVEL_ABOVE_SEA && !rand_orbit) {
                            temperature--;
                        }
                    } else if (MapPlanetaryOrbit < 2 || !random(0, 2)) {
                        temperature--;
                    }
                }
                temp_set(x, y, (uint8_t)range(temperature - 1, 0, 2));
            }
        }
        do_all_non_input();
    }
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

/*
Purpose: Score the specified tile as a site for a new base.
// ORIGINAL: 0x005C4FD0 ?world_site@@YAHHHH@Z 0x005C4FD0-0x005C55B5
// TRIED: site_tile/site_xrange were being emitted as real calls; __forceinline on both
//            (site_tile at 2423, site_xrange at 1730) brought the call count from 9 down to
//            the image's 3 (2x bonus_at, 1x goody_at) but did not reach BYTE_EXACT.
// TRIED: remaining mismatch is register allocation across a 1509-byte body, the prior
//            pass's noted plateau for this family - not chased further.
// size      1509 bytes
// prototype 
// callers   8   call targets   2
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x00592030 0x00592140
Return Value: Site value 0 (unusable) or 1-15
Status: Complete

Two rings. The outer one, radius offsets 21 to 48, only counts how crowded the
neighbourhood already is; the inner one, the twenty-one tiles of a base's own
production radius, is what is actually scored. `is_ocean_site` says which kind
of site is wanted, and it is compared against the centre tile's own altitude
rather than being normalised - a caller passing anything but 0 or 1 never
matches.

The original accumulates a rainfall total across the inner ring into a stack
local at ebp-0x30 - zeroed at 0x005C50C5, read at 0x005C5327, stored back at
0x005C5339 - and never reads it again. It is a dead local, not observable from
outside the frame, and it is not transcribed.
*/
int __cdecl world_site(int x, int y, BOOL is_ocean_site) {
    int crowding = 0;
    for (int i = 21; i < 49; i++) {
        int x_radius = site_xrange(x + RadiusOffsetX[i]);
        int y_radius = y + RadiusOffsetY[i];
        if (y_radius < 0 || y_radius >= MapLatitudeBounds || x_radius < 0
            || x_radius >= MapLongitudeBounds) {
            continue;
        }
        Map *tile = site_tile(x_radius, y_radius);
        if (!(tile->bit & BIT_BASE_IN_TILE)) {
            continue;
        }
        crowding++;
        if (i >= 25) {
            continue; // the first four count double, and can count quadruple
        }
        crowding++;
        if ((site_tile(x, y)->climate & 0xE0) < ALT_BIT_SHORE_LINE
            && (tile->climate & 0xE0) < ALT_BIT_SHORE_LINE) {
            crowding += 2;
        }
    }
    int special = 0;       // bonus resources, landmarks and pods
    int rich_terrain = 0;  // monolith, forest, or rainy rolling land above sea
    int fair_terrain = 0;  // rainy rolling land that is none of those
    int ocean_count = 0;
    int deep_count = 0;    // ocean below the shelf
    int river_tiles = 0;
    int fungus_tiles = 0;
    int rocky_tiles = 0;
    BOOL big_continent = false;
    /*
    * The original's frame aliases this pointer onto the slot that held the
    * MapIsFlat copy, and the epilogue reads it whether or not the loop ever
    * assigned it. With every tile of the radius out of bounds that is a
    * dereference of 0 or 1. Reproduced rather than guarded: the two are the
    * same program, and the caller that could reach it is off the map already.
    */
    Map *radius_tile = reinterpret_cast<Map *>(
        static_cast<uintptr_t>(MapIsFlat & 1));
    for (i = 0; i < 21; i++) {
        int x_radius = site_xrange(x + RadiusOffsetX[i]);
        int y_radius = y + RadiusOffsetY[i];
        if (y_radius < 0 || y_radius >= MapLatitudeBounds || x_radius < 0
            || x_radius >= MapLongitudeBounds) {
            continue;
        }
        radius_tile = site_tile(x_radius, y_radius);
        int altitude = radius_tile->climate & 0xE0;
        if (altitude < ALT_BIT_SHORE_LINE) {
            ocean_count++;
            if (altitude < ALT_BIT_OCEAN_SHELF) {
                deep_count++;
            }
            // Only the nine innermost tiles decide whether this is a big land.
            if (i < 9 && Continents[radius_tile->region].tile_count >= 50) {
                big_continent = true;
            }
            if (!i || !bonus_at(x_radius, y_radius, 1)) {
                continue;
            }
            Map *bonus_tile = site_tile(x_radius, y_radius);
            if ((bonus_tile->bit & BIT_FUNGUS)
                && (int)(bonus_tile->climate & 0xE0) >= ALT_BIT_OCEAN_SHELF) {
                continue;
            }
            special++;
            continue;
        }
        int rockiness = radius_tile->val3 >> 6;
        if (rockiness && i && rockiness > 1) {
            rocky_tiles++;
        }
        if (!(radius_tile->bit & BIT_FUNGUS)) {
            if (bonus_at(x_radius, y_radius, 1)) {
                special += i ? 2 : -1;
            }
            uint32_t bit2 = site_tile(x_radius, y_radius)->bit2;
            // The top byte is the landmark's tile sequence code; past nine the
            // tile is too far along the landmark to be worth anything.
            if ((bit2 & (BIT2_UNK_80000000 | BIT2_VOLCANO)) == BIT2_VOLCANO
                && (bit2 & 0xFF000000) < 0x09000000) {
                special += i ? 1 : -1;
            }
            if ((bit2 & (BIT2_UNK_80000000 | BIT2_CRATER)) == BIT2_CRATER
                && (bit2 & 0xFF000000) < 0x09000000) {
                special += i ? 2 : -1;
            }
            if ((bit2 & (BIT2_UNK_80000000 | BIT2_URANIUM)) == BIT2_URANIUM) {
                special += i ? 1 : -3;
            }
            if ((bit2 & (BIT2_UNK_80000000 | BIT2_JUNGLE)) == BIT2_JUNGLE
                && (int)(radius_tile->climate & 0xE0) >= ALT_BIT_SHORE_LINE) {
                special += 2;
            }
            if (i && goody_at(x_radius, y_radius)) {
                special++;
            }
        }
        int elevation = (radius_tile->climate >> 5) - 2;
        if (elevation < 0) {
            elevation = 0;
        }
        int rainfall = (radius_tile->climate >> 3) & 3;
        if (rockiness == ROCKINESS_ROCKY
            || (radius_tile->bit2 & (BIT2_UNK_80000000 | BIT2_VOLCANO))
                == BIT2_VOLCANO) {
            rainfall = 0;
        }
        uint32_t bit = radius_tile->bit;
        if (bit & BIT_FUNGUS) {
            fungus_tiles++;
        }
        if (bit & BIT_BASE_RADIUS) {
            continue; // already worked by somebody
        }
        BOOL is_rich = (bit & (BIT_MONOLITH | BIT_FOREST)) != 0;
        if (!is_rich && rainfall && rockiness == ROCKINESS_ROLLING) {
            if (elevation && !(bit & BIT_FUNGUS)) {
                is_rich = true;
            } else {
                fair_terrain += i ? 1 : -1;
                if (rainfall > rockiness && !(bit & BIT_FUNGUS)) {
                    fair_terrain += i ? 1 : -1;
                }
            }
        }
        if (is_rich) {
            rich_terrain += i ? 1 : -1;
            if ((bit & BIT_MONOLITH) && !(bit & BIT_FOREST)
                && (rainfall > 1 || elevation > 1)) {
                rich_terrain += i ? 1 : -2;
            }
        }
        if (bit & BIT_RIVER) {
            river_tiles++;
        }
    }
    int score = river_tiles / 4 + special;
    if (big_continent) {
        score += is_ocean_site ? -2 : 4;
    }
    if (radius_tile->bit & (BIT_UNK_4000 | BIT_UNK_40000000)) {
        score += 4;
    }
    int water = (ocean_count - big_continent) / 2 + deep_count / 2;
    score += is_ocean_site ? water : -water;
    score += (rich_terrain * 3 + 6) / 4;
    score += (fair_terrain + 1) / 3;
    if (TurnCurrentNum > 150) {
        score += fungus_tiles / -3;
    }
    Map *tile = site_tile(x, y);
    int altitude = tile->climate & 0xE0;
    int is_ocean = (altitude < ALT_BIT_SHORE_LINE) ? 1 : 0;
    score -= rocky_tiles / 4;
    score -= crowding * ((is_ocean != is_ocean_site) ? 2 : 1);
    if (((tile->bit & BIT_FUNGUS) && altitude >= ALT_BIT_OCEAN_SHELF)
        || (tile->bit & BIT_MONOLITH)) {
        return 0;
    }
    if ((tile->val3 & 0xC0) > 0x40 && altitude >= ALT_BIT_SHORE_LINE) {
        return 0; // rocky land cannot hold a base
    }
    if (is_ocean != is_ocean_site && crowding >= 4) {
        return 0; // the wrong element, and no room for it either
    }
    if (score < 1) {
        return 1;
    }
    return (score > 15) ? 15 : score;
}

/*
Purpose: Analysis of the world map.
// ORIGINAL: 0x005C55C0 ?world_analysis@@YAXXZ 0x005C55C0-0x005C58B8
// LEVER: call_diff had this MORE by 3 calls: the inner loop's three `bit_set(x, y, ...)`
//            calls were real calls where the image writes bit_set's body out inline and
//            reuses ONE tile pointer for all three (the image keeps a running `Map*` in
//            ebx, bumped by sizeof(Map) once per x-loop iteration, instead of recomputing
//            map_loc(x, y) at each site) - replaced with `uint32_t *const tile_bit =
//            &map_loc(x, y)->bit;` hoisted once, then `*tile_bit`. Also fixed `i >= 32` /
//            `i == 32` to `i >= 8` / `i == 8`: the image's `cmp esi, 0x20` is the SAME
//            loop counter strength-reduced to a byte offset for RadiusOffsetX/Y addressing
//            (esi = 4*i), so the comparison the source made was against `i`, not against
//            that scaled induction variable - at `i < 20` the old `>= 32` could never be
//            true, so the region_radius/BIT_UNK_4000 branch was dead code. 0.454 -> 0.484
//            best-across-flags similarity; call_diff now agrees (0 vs 1 became 1 vs 1).
// TRIED (not attempted further): the remaining gap is the `imul 0x2c` vs
//            `lea`+SIB-scale-4 Map-stride plateau this file's other bodies already hit,
//            compounded by a prologue register-scheduling difference (ebx/edi assignment
//            order) at the very first divergence - both are the documented
//            register-allocation plateau for this family, not a source-shape defect.
// size      760 bytes
// prototype 
// callers   1   call targets   1
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x005FCB20
Return Value: n/a
Status: Complete - testing
*/
void __cdecl world_analysis() {
    for (uint32_t i = 0; i < MaxContinentNum; i++) {
        Continents[i].open_terrain = 0;
    }
    for (int y = 0; y < MapLatitudeBounds; y++) {
        for (int x = y & 1; x < MapLongitudeBounds; x += 2) {
            uint32_t region = region_at(x, y);
            BOOL is_ocean_tile = is_ocean(x, y);
            if (!is_ocean_tile && climate_at(x, y) != RAINFALL_ARID 
                && rocky_at(x, y) < TERRAIN_BIT_ROCKY
                && (!(bit_at(x, y) & BIT_FUNGUS || altitude_at(x, y) < ALT_BIT_OCEAN_SHELF))) {
                Continents[region].open_terrain++;
            }
            uint32_t *const tile_bit = &map_loc(x, y)->bit; // bit_set, inlined: the image writes
            *tile_bit &= ~(BIT_UNK_40000000 | BIT_UNK_4000); // it out here, not a call
            if (!is_ocean_tile) {                                  // correctly for all bit_*
                int search_val = -1;
                uint32_t count_val1 = 0;
                uint32_t count_val2 = 0;
                for (uint32_t i = 0; i < 20; i++) {
                    int x_radius = xrange(x + RadiusOffsetX[i + 1]);
                    int y_radius = y + RadiusOffsetY[i + 1];
                    if (on_map(x_radius, y_radius)) {
                        BOOL is_ocean_radius = is_ocean(x_radius, y_radius);
                        if (i >= 8) {
                            if (is_ocean_radius != search_val || i == 8) {
                                search_val = is_ocean_radius;
                                count_val1++;
                            }
                            uint32_t region_radius;
                            if (y_radius > 4 && y_radius < ((int)MapLatitudeBounds - 4)
                                && (region_radius = region_at(x_radius, y_radius),
                                    region_radius < MaxRegionLandNum && region != region_radius
                                    && Continents[region].tile_count > 40
                                    && Continents[region_radius].tile_count > 40)) {
                                *tile_bit |= BIT_UNK_4000; // bit_set, inlined
                            }
                        } else {
                            if (is_ocean_radius != search_val) {
                                search_val = is_ocean_radius;
                                count_val2++;
                                if (count_val2 >= 4 && Continents[region].tile_count >= 80) {
                                    *tile_bit |= BIT_UNK_40000000; // bit_set, inlined
                                }
                            }
                        }
                    }
                }
                if (count_val1 < 4) {
                    *tile_bit &= ~BIT_UNK_40000000; // bit_set, inlined
                }
            }
        }
        do_all_non_input();
    }
    for (i = 0; i < MapArea; i++) {
        map_tiles()[i].val2 &= 0xF; // clear map sites
    }
}

// Two fixed-address flags world_climate reads, no established identity
// beyond what this one function shows.
uint32_t WorldClimateSkipTerrainClear;  // 0x0068F21C
uint32_t WorldClimateSkipTerritoryReset;  // 0x009B22EC
// The process-wide WorldWin, torn down/built at this same fixed address
// elsewhere (atexit_thunks.cpp, init_thunks.cpp).
WorldWin *const WorldClimateWorldWin = (WorldWin *)0x008E9F60;

/*
Purpose: Regenerate the world's climate: shorelines, temperature, rivers,
         rainfall, continents, then analysis; reset territory unless told
         not to; reseed the random generator from the clock; and clear
         terrain on every live map window (plus the world overview window)
         unless told not to.
// ORIGINAL: 0x005C5A30 ?world_climate@@YAXXZ 0x005C5A30-0x005C5ADB BYTE_EXACT
// size      171 bytes
// prototype void (__cdecl ?world_climate@@YAXXZ)()
// callers   18   call targets   12
// kind      game
// flags     sp_ready;purged_ok
// calls     0x0046FD90 0x004C45E0 0x0051E760 0x00523DD0 0x0059C790 0x005C38B0
//           0x005C3F70 0x005C4170 0x005C4470 0x005C55C0 0x006257E0 0x006262F0
// indirect  0x005C5A7B
Return Value: n/a
Status: Complete
*/
void __cdecl world_climate() {
    do_checksums(2);
    log_say(reinterpret_cast<LPCSTR>(0x00691E2C), 0, 0, 0);
    world_shorelines();
    world_temperature();
    world_rivers();
    world_rainfall();
    Paths->continents();
    world_analysis();
    if (WorldClimateSkipTerritoryReset == 0) {
        reset_territory();
    }

    random_reseed(timeGetTime());

    if (WorldClimateSkipTerrainClear == 0) {
        int cursor = reinterpret_cast<int>(MapWinTable);
        const int end = cursor + static_cast<int>(MapWinTableSlots) * 4;
        do {
            MapWin *const window = *reinterpret_cast<MapWin *const *>(cursor);
            if (*reinterpret_cast<int *>(
                    reinterpret_cast<char *>(window) + 0x1DD74) != 0) {
                window->clear_terrain();
            }
            cursor += 4;
        } while (cursor < end);
        WorldClimateWorldWin->clear_terrain();
    }

    log_say(reinterpret_cast<LPCSTR>(0x00691E4C), 0, 0, 0);
    do_checksums(3);
}

/*
Purpose: Set the default altitude details for the specified tile.
// ORIGINAL: 0x005C58C0 ?world_alt_put_detail@@YAXHH@Z 0x005C58C0-0x005C58DB
// size      27 bytes
// prototype void (__cdecl ?world_alt_put_detail@@YAXHH@Z)(int xCoord, int yCoord)
// callers   0   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00591260
Return Value: n/a
Status: Complete
*/

/*
Purpose: Set up the world polar caps.
// ORIGINAL: 0x005C58E0 ?world_polar_caps@@YAXXZ 0x005C58E0-0x005C5A2F
// size      335 bytes
// prototype 
// callers   2   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00591260 0x0064601D
// TRIED: call_diff's "4 vs 10" undercounts by excluding rand() as hidden; `osmx calls
//            --all` already shows 6 alt_put_detail + 4 rand = 10, matching the image. The
//            remaining mismatch is the image omitting the ebp frame this tree keeps under
//            every flag set tried - a stack-frame plateau, not a source shape.
Return Value: n/a
Status: Complete - testing
*/
void __cdecl world_polar_caps() {
    for (uint8_t x = 0; x < MapLongitudeBounds; x += 2) {
        world_alt_put_detail(x, 0);
        world_alt_put_detail(x - 1, MapLatitudeBounds - 1);
    }
    uint32_t bounds = MapLongitudeBounds / 16;
    for (uint32_t i = 0; i < bounds; i++) {
        world_alt_put_detail(rnd(MapLongitude, NULL) * 2, 0);
        world_alt_put_detail(rnd(MapLongitude, NULL) * 2 + 1, 1);
        world_alt_put_detail(rnd(MapLongitude, NULL) * 2 + 1, MapLatitudeBounds - 1);
        world_alt_put_detail(rnd(MapLongitude, NULL) * 2, MapLatitudeBounds - 2);
    }
}

/*
Purpose: Set up the world contours.
// ORIGINAL: 0x005C5AE0 ?world_linearize_contours@@YAXXZ 0x005C5AE0-0x005C5BC3
// LEVER: BUG IN THIS RECOVERY (not the original): `alt_nat +
//        (alt_nat >= ALT_3_LEVELS_ABOVE_SEA) ? 4 : 1` - `+` binds tighter
//        than `?:`, so this parsed as `(alt_nat + (alt_nat >= ...)) ? 4 :
//        1`, collapsing the array index to the plain constant 4 or 1. The
//        image indexes `ElevDetail` at `[eax*4 + 0x68fb24]` /
//        `[eax*4 + 0x68fb30]` - i.e. `alt_nat + 1` or `alt_nat + 4`,
//        register-relative on `alt_nat`. Parenthesizing the ternary
//        (`alt_nat + ((alt_nat >= ALT_3_LEVELS_ABOVE_SEA) ? 4 : 1)`) fixes
//        the semantics; 0.690 -> 0.705 similar (best flags /O2 /Ob0).
// TRIED: not chased past this MISMATCH plateau - remaining gap starts
//            in the prologue frame-size (image reserves 2 stack dwords for
//            the divide's intermediates, this tree's compile picks a
//            different spill count).
// size      227 bytes
// prototype
// callers   2   call targets   3
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00591260 0x005918A0 0x005FCB20
Return Value: n/a
Status: Complete - testing
*/
void __cdecl world_linearize_contours() {
    for (int y = 0; y < MapLatitudeBounds; y++) {
        for (int x = y & 1; x < MapLongitudeBounds; x += 2) {
            uint32_t alt_nat = alt_natural(x, y);
            alt_put_detail(x, y, (uint8_t)((((ElevDetail[alt_nat
                + ((alt_nat >= ALT_3_LEVELS_ABOVE_SEA) ? 4 : 1)] - ElevDetail[alt_nat])
                * (alt_detail_at(x, y) - AltNatural[alt_nat]))
                / (AltNatural[alt_nat + 1] - AltNatural[alt_nat])) + ElevDetail[alt_nat]));
        }
        do_all_non_input();
    }
    // `memcpy`, and the ARRAYS, not their addresses. The image ends with
    // `mov ecx, 0xb; mov esi, 0x68fb20; mov edi, 0x68fb4c; rep movsd` - the
    // intrinsic, copying ElevDetail over AltNatural. `memcpy_s` emits a call
    // with four arguments, and `&AltNatural` wrote over the POINTER rather
    // than the array it names.
    memcpy(AltNatural, ElevDetail, sizeof(uint32_t) * 11);
}

/*
Purpose: Determine if the specified tile is near a landmark.
// ORIGINAL: 0x005C5BD0 ?near_landmark@@YAHHH@Z 0x005C5BD0-0x005C5C61
// size      145 bytes
// prototype BOOL (__cdecl ?near_landmark@@YAHHH@Z)(int xCoord, int yCoord)
// callers   1   call targets   0
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     (none)
// TRIED: call shape already matches - `osmx calls` shows 0 calls, same as the image,
//            so xrange/on_map/code_at are already inlined here. Best flag set still only
//            reaches 17/56 agreeing instructions; the divergence starts at the prologue's
//            register assignment (ebx loaded from MapIsFlat before the frame push order this
//            tree emits), the stack-frame/register-allocation plateau noted for this family.
// LEVER (2026-08-21): moved to map.h as `MEASURED inline`. world_unity (0x005C7750) and
//            world_geothermal (0x005C83B0) call_diff'd off by exactly one call each, and
//            in both cases it was this: a real call to near_landmark where the image has
//            it inlined. world_borehole (0x005C7020) already inlined it either way, so
//            this is additive there, and `osmx check` stayed at 0 REGRESSED.
Return Value: Is the tile near a landmark? true/false
Status: Complete
*/
// BODY IN map.h, as `MEASURED inline`: the image writes it out at
// some call sites and calls it at others, and a .cpp definition is only ever
// one of those. The marker stays here because that is where the catalogue
// reads it.

/*
Purpose: Setup the 'Garland Crater' landmark.
// ORIGINAL: 0x005C5C70 ?world_crater@@YAXHH@Z 0x005C5C70-0x005C5EEF
// size      639 bytes
// prototype void (__cdecl ?world_crater@@YAXHH@Z)(int xCoord, int yCoord)
// callers   4   call targets   9
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x00591AD0 0x00591BC0 0x00591D60 0x00591DB0 0x00591E00 0x00592600 0x005C2020 0x006169A0 0x0064601D
// LEVER: two fixes, 0.703 -> 0.779 similar (28/221 raw). (1) `rnd(bounds,
//        NULL)` WRITTEN OUT as rnd's own body (0x00579770's ternary) at both
//        call sites - the image never calls rnd(), only rand() - matching
//        the image's call count. (2) rnd's ternary compiles with INVERTED
//        branch polarity from the image's; an `if (bounds - 1 <= 0) { x = 0;
//        } else { x = rand() % bounds; }` guard clause (zero branch inline,
//        compute branch out-of-line via `jg`) reproduces the image's
//        fall-through instead. Also: `int lon_bounds = MapLongitudeBounds;`
//        declared once, used in both the second guard's condition and its
//        `rand() % lon_bounds` - matches the image reusing one register for
//        both, where the bare global re-read three times.
// TRIED: same xrange/on_map two-induction-variable loop family already
//            documented across this file (bonus_at, goody_at, base_on_sea,
//            world_borehole) - the prologue's on_map inline and the loop's
//            register scheduling (which global gets reloaded early during a
//            call's latency slot) do not follow from source shape. Not
//            chased past this MISMATCH plateau.
Return Value: n/a
Status: Complete - testing
*/
void __cdecl world_crater(int x, int y) {
    uint32_t loc_attempts = 0;
    if (!on_map(x, y)) {
        do {
            do {
                int lat_bounds = MapLatitudeBounds - 16;
                int rnd_y;
                if (lat_bounds - 1 <= 0) {
                    rnd_y = 0;
                } else {
                    rnd_y = rand() % lat_bounds;
                }
                y = rnd_y + 8;
                int lon_bounds = MapLongitudeBounds;
                int x_seed;
                if (lon_bounds - 1 <= 0) {
                    x_seed = 0;
                } else {
                    x_seed = rand() % lon_bounds;
                }
                x = ((x_seed ^ y) & 1) ^ x_seed;
                if (++loc_attempts >= 1000) {
                    return;
                }
            } while (is_ocean(x, y));
        } while (near_landmark(x, y));
        if (loc_attempts >= 1000) { // redundant check? would have already returned, remove?
            return;
        }
    }
    world_alt_set(x, y, ALT_OCEAN_TRENCH, true);
    world_alt_set(x, y, ALT_3_LEVELS_ABOVE_SEA, true);
    world_alt_set(x, y, ALT_SHORE_LINE, true);
    world_alt_set(x, y, ALT_OCEAN_SHELF, true);
    world_alt_set(x, y, ALT_SHORE_LINE, true);
    uint32_t temperature = temp_at(x, y);
    for (int i = 0; i < RadiusRange[3]; i++) {
        int x_radius = xrange(x + RadiusOffsetX[i]);
        int y_radius = y + RadiusOffsetY[i];
        if (on_map(x_radius, y_radius)) {
            bit_set(x_radius, y_radius, BIT_RIVERBED, false);
            bit2_set(x_radius, y_radius, BIT2_CRATER, true);
            code_set(x_radius, y_radius, i);
            if (i < 21) {
                rocky_set(x_radius, y_radius, ROCKINESS_ROLLING);
                temp_set(x_radius, y_radius, (uint8_t)temperature);
            }
        }
    }
    new_landmark(x, y, StringTable->get((int)Natural[LM_CRATER].name));
}

/*
Purpose: Setup the 'Monsoon Jungle' landmark.
// ORIGINAL: 0x005C5EF0 ?world_monsoon@@YAXHH@Z 0x005C5EF0-0x005C6200
// size      784 bytes
// prototype void (__cdecl ?world_monsoon@@YAXHH@Z)(int xCoord, int yCoord)
// callers   3   call targets   8
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x004E49D0 0x00591DB0 0x00591E00 0x00592600 0x005C2020 0x005C4470 0x006169A0 0x0064601D
// LEVER: same shape as world_crater (0x005C5C70) - `rnd(MapLongitudeBounds,
//        NULL)` written out as an `int lon_bounds = MapLongitudeBounds; if
//        (lon_bounds - 1 <= 0) { x_seed = 0; } else { x_seed = rand() %
//        lon_bounds; }` guard clause, matching the image's call count (no
//        real call to rnd()) and fall-through polarity. Best flag set (/O2
//        /Gy /GR- /Oy- /GX) moved 0.398 -> 0.455 similar.
// TRIED: same xrange/on_map two-induction-variable loop family plateau as
//            world_crater - the on_map prologue and the loop's register
//            scheduling do not follow from source shape. Not chased past
//            this MISMATCH plateau (784-byte body, 260 image instructions).
Return Value: n/a
Status: Complete - testing
*/
void __cdecl world_monsoon(int x, int y) {
    world_rainfall();
    uint32_t loc_attempts = 0;
    if (!on_map(x, y)) {
        do {
            uint32_t land_count;
            do {
                y = MapLatitudeBounds / 2 + rand() % 4 - 2;
                int lon_bounds = MapLongitudeBounds;
                int x_seed;
                if (lon_bounds - 1 <= 0) {
                    x_seed = 0;
                } else {
                    x_seed = rand() % lon_bounds;
                }
                x = ((x_seed ^ y) & 1) ^ x_seed;
                land_count = 0;
                for (int i = 0; i < RadiusRange[5]; i++) {
                    int x_radius = xrange(x + RadiusOffsetX[i]);
                    int y_radius = y + RadiusOffsetY[i];
                    if (on_map(x_radius, y_radius) && !is_ocean(x_radius, y_radius)) {
                        land_count++;
                    }
                }
                if (++loc_attempts >= 1000) {
                    return;
                }
            } while (is_ocean(x, y) || !is_coast(x, y, true) || land_count < 40
                || climate_at(x, y) <= RAINFALL_MOIST);
        } while (near_landmark(x, y));
        if (loc_attempts >= 1000) { // redundant check? would have already returned, remove?
            return;
        }
    }
    for (int i = 0; i < RadiusRange[5]; i++) {
        int x_radius = xrange(x + RadiusOffsetX[i]);
        int y_radius = y + RadiusOffsetY[i];
        if (abs(RadiusOffsetY[i]) <= 8 &&  on_map(x_radius, y_radius)) {
            if (i < 21 && is_ocean(x_radius, y_radius)) {
                world_alt_set(x_radius, y_radius, ALT_SHORE_LINE, true);
            }
            bit2_set(x_radius, y_radius, LM_JUNGLE, true);
            code_set(x_radius, y_radius, i);
        }
    }
    new_landmark(x, y, StringTable->get((int)Natural[LM_JUNGLE].name));
}

/*
Purpose: Setup the 'New Sargasso' landmark.
// ORIGINAL: 0x005C6200 ?world_sargasso@@YAXHH@Z 0x005C6200-0x005C6498
// size      664 bytes
// prototype void (__cdecl ?world_sargasso@@YAXHH@Z)(int xCoord, int yCoord)
// callers   3   call targets   7
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x00591D60 0x00591DB0 0x00591E00 0x00592600 0x005C2020 0x006169A0 0x0064601D
// LEVER: same shape as world_crater (0x005C5C70) - both `rnd(bounds,
//        NULL)` calls written out as guard clauses matching the image's
//        call count and fall-through polarity. Best flag set (/O2 /Gy
//        /GR- /Oy- /GX) moved 1/220 -> 26/220, 0.591 similar.
// TRIED: same xrange/on_map two-induction-variable loop family
//            plateau as world_crater. Not chased past this MISMATCH
//            plateau.
Return Value: n/a
Status: Complete - testing
*/
void __cdecl world_sargasso(int x, int y) {
    uint32_t loc_attempts = 0;
    BOOL has_skipped = false;
    if (!on_map(x, y)) {
        do {
            do {
                int lat_bounds = MapLatitudeBounds - 16;
                int rnd_y;
                if (lat_bounds - 1 <= 0) {
                    rnd_y = 0;
                } else {
                    rnd_y = rand() % lat_bounds;
                }
                y = rnd_y + 8;
                int lon_bounds = MapLongitudeBounds;
                int x_seed;
                if (lon_bounds - 1 <= 0) {
                    x_seed = 0;
                } else {
                    x_seed = rand() % lon_bounds;
                }
                x = ((x_seed ^ y) & 1) ^ x_seed;
                if (++loc_attempts >= 1000) {
                    return;
                }
            } while (!is_ocean(x, y));
        } while (near_landmark(x, y));
        if (loc_attempts >= 1000) { // redundant check? would have already returned, remove?
            return;
        }
    }
    for (int i = 0; i < RadiusRange[4]; i++) {
        int x_radius = xrange(x + RadiusOffsetX[i]);
        int y_radius = y + RadiusOffsetY[i];
        if (on_map(x_radius, y_radius) && (is_ocean(x_radius, y_radius) || i < 21)) {
            world_alt_set(x_radius, y_radius, ALT_OCEAN_SHELF, true);
            bit2_set(x_radius, y_radius, BIT2_SARGASSO, true);
            code_set(x_radius, y_radius, i);
            if (i <= 4) {
                bit_set(x_radius, y_radius, BIT_SUPPLY_POD, true);
            }
            if ((has_skipped || i < RadiusRange[2] || rand() % 24) 
                && (i < RadiusRange[3] || rand() % 3)) {
                bit_set(x_radius, y_radius, BIT_FUNGUS, true);
            } else {
                has_skipped = true;
            }
        }
    }
    new_landmark(x, y, StringTable->get((int)Natural[LM_SARGASSO].name));
}

/*
Purpose: Setup 'The Ruins' landmark.
// ORIGINAL: 0x005C64A0 ?world_ruin@@YAXHH@Z 0x005C64A0-0x005C6734
// size      660 bytes
// prototype void (__cdecl ?world_ruin@@YAXHH@Z)(int xCoord, int yCoord)
// callers   3   call targets   7
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x00591D60 0x00591DB0 0x00591E00 0x00592600 0x005C2020 0x006169A0 0x0064601D
// LEVER: same shape as world_crater (0x005C5C70) - both `rnd(bounds,
//        NULL)` calls written out as guard clauses matching the image's
//        call count and fall-through polarity. Best flag set (/O2 /Gy
//        /GR- /Oy- /GX) moved 1/221 -> 8/221, 0.422 similar.
// TRIED: same xrange/on_map two-induction-variable loop family
//            plateau as world_crater. Not chased past this MISMATCH
//            plateau.
Return Value: n/a
Status: Complete - testing
*/
void __cdecl world_ruin(int x, int y) {
    uint32_t loc_attempts = 0;
    if (!on_map(x, y)) {
        do {
            do {
                int lat_bounds = MapLatitudeBounds - 16;
                int rnd_y;
                if (lat_bounds - 1 <= 0) {
                    rnd_y = 0;
                } else {
                    rnd_y = rand() % lat_bounds;
                }
                y = rnd_y + 8;
                int lon_bounds = MapLongitudeBounds;
                int x_seed;
                if (lon_bounds - 1 <= 0) {
                    x_seed = 0;
                } else {
                    x_seed = rand() % lon_bounds;
                }
                x = ((x_seed ^ y) & 1) ^ x_seed;
                if (++loc_attempts >= 1000) {
                    return;
                }
            } while (is_ocean(x, y));
        } while (near_landmark(x, y));
        if (loc_attempts >= 1000) { // redundant check? would have already returned, remove?
            return;
        }
    }
    for (int i = 0; i < RadiusRange[2]; i++) {
        int x_radius = xrange(x + RadiusOffsetX[i]);
        int y_radius = y + RadiusOffsetY[i];
        if (on_map(x_radius, y_radius)) {
            if (i < RadiusRange[2] || !is_ocean(x_radius, y_radius)) { // is_ocean check pointless?
                world_alt_set(x_radius, y_radius, ALT_SHORE_LINE, true);
            }
            if (is_ocean(x_radius, y)) {
                if (i && i < 9) {
                    bit_set(x_radius, y_radius, BIT_MONOLITH, true); // in ocean tiles?
                }
            } else {
                bit_set(x_radius, y, BIT_FUNGUS | BIT_SUPPLY_REMOVE, true);
                if (!i) {
                    bit_set(x_radius, y_radius, BIT_FUNGUS, false);
                } else if (i < 9) {
                    bit_set(x_radius, y_radius, BIT_MONOLITH, true);
                }
            }
            bit2_set(x_radius, y_radius, BIT2_RUINS, true);
            code_set(x_radius, y_radius, i);
        }
    }
    new_landmark(x, y, StringTable->get((int)Natural[LM_RUINS].name));
}

/*
Purpose: Setup the 'Great Dunes' landmark.
// ORIGINAL: 0x005C6740 ?world_dune@@YAXHH@Z 0x005C6740-0x005C69DB
// size      667 bytes
// prototype void (__cdecl ?world_dune@@YAXHH@Z)(int xCoord, int yCoord)
// callers   3   call targets   9
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x00591BC0 0x00591D60 0x00591DB0 0x00591E00 0x00592600 0x005C2020 0x005C4470 0x006169A0 0x0064601D
// LEVER: same shape as world_crater (0x005C5C70), plus its own
//        `half_vert_bounds` local already matched rnd's `bounds`
//        parameter - written out as an `int dune_bounds = half_vert_bounds;`
//        guard clause (Y) and the usual `lon_bounds` guard clause (X). Best
//        flag set (/O2 /Gy /GR- /Oy- /GX) moved 5/228 -> 9/228, 0.559
//        similar.
// TRIED: same xrange/on_map two-induction-variable loop family
//            plateau as world_crater, plus world_rainfall()'s own call at
//            the top. Not chased past this MISMATCH plateau.
Return Value: n/a
Status: Complete - testing
*/
void __cdecl world_dune(int x, int y) {
    world_rainfall();
    uint32_t loc_attempts = 0;
    uint32_t half_vert_bounds = MapLatitudeBounds / 2;
    if (!on_map(x, y)) {
        do {
            do {
                int dune_bounds = half_vert_bounds;
                int rnd_y;
                if (dune_bounds - 1 <= 0) {
                    rnd_y = 0;
                } else {
                    rnd_y = rand() % dune_bounds;
                }
                y = rnd_y + half_vert_bounds - MapLatitudeBounds / 4;
                int lon_bounds = MapLongitudeBounds;
                int x_seed;
                if (lon_bounds - 1 <= 0) {
                    x_seed = 0;
                } else {
                    x_seed = rand() % lon_bounds;
                }
                x = ((x_seed ^ y) & 1) ^ x_seed;
                if (++loc_attempts >= 1000) {
                    return;
                }
            } while (is_ocean(x, y) || climate_at(x, y) != RAINFALL_ARID);
        } while (near_landmark(x, y));
        if (loc_attempts >= 1000) { // redundant check? would have already returned, remove?
            return;
        }
    }
    for (int i = 0; i < RadiusRange[2]; i++) {
        int x_radius = xrange(x + RadiusOffsetX[i]);
        int y_radius = y + RadiusOffsetY[i];
        if (on_map(x_radius, y_radius) && (!is_ocean(x_radius, y_radius) || i < 9 || rand() % 3)) {
            if (i == 2 || i == 7) {
                world_alt_set(x_radius, y_radius, ALT_1_LEVEL_ABOVE_SEA, true);
            } else {
                world_alt_set(x_radius, y_radius, ALT_SHORE_LINE, true);
            }
            bit_set(x_radius, y_radius, BIT_FUNGUS, false);
            bit2_set(x_radius, y_radius, LM_DUNES, true);
            rocky_set(x_radius, y_radius, ROCKINESS_FLAT);
            code_set(x_radius, y_radius, i);
        }
    }
    new_landmark(x, y + 2, StringTable->get((int)Natural[LM_DUNES].name));
}

/*
Purpose: Setup the 'Uranium Flats' landmark.
// ORIGINAL: 0x005C69E0 ?world_diamond@@YAXHH@Z 0x005C69E0-0x005C6C31
// size      593 bytes
// prototype void (__cdecl ?world_diamond@@YAXHH@Z)(int xCoord, int yCoord)
// callers   3   call targets   8
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00591BC0 0x00591D60 0x00591DB0 0x00591E00 0x00592600 0x005C2020 0x006169A0 0x0064601D
// LEVER: same shape as world_crater (0x005C5C70) - both `rnd(bounds,
//        NULL)` calls written out as guard clauses matching the image's
//        call count and fall-through polarity. Best flag set (/O2 /Gy
//        /GR- /Oy- /GX) moved 7/200 -> 14/200, 0.887 similar.
// TRIED: same xrange/on_map two-induction-variable loop family
//            plateau as world_crater. Not chased past this MISMATCH
//            plateau.
Return Value: n/a
Status: Complete - testing
*/
void __cdecl world_diamond(int x, int y) {
    uint32_t loc_attempts = 0;
    if (!on_map(x, y)) {
        do {
            do {
                int lat_bounds = MapLatitudeBounds - 16;
                int rnd_y;
                if (lat_bounds - 1 <= 0) {
                    rnd_y = 0;
                } else {
                    rnd_y = rand() % lat_bounds;
                }
                y = rnd_y + 8;
                int lon_bounds = MapLongitudeBounds;
                int x_seed;
                if (lon_bounds - 1 <= 0) {
                    x_seed = 0;
                } else {
                    x_seed = rand() % lon_bounds;
                }
                x = ((x_seed ^ y) & 1) ^ x_seed;
                if (++loc_attempts >= 1000) {
                    return;
                }
            } while (is_ocean(x, y));
        } while (near_landmark(x, y));
        if (loc_attempts >= 1000) { // redundant check? would have already returned, remove?
            return;
        }
    }
    for (uint32_t i = 0; i < 21; i++) {
        int x_radius = xrange(x + RadiusOffsetX[i]);
        int y_radius = y + RadiusOffsetY[i];
        if (on_map(x_radius, y_radius)) {
            if (i < 9 || !is_ocean(x_radius, y_radius) || rand() % 3) {
                world_alt_set(x_radius, y_radius, ALT_SHORE_LINE, true);
                bit2_set(x_radius, y_radius, BIT2_URANIUM, true);
                rocky_set(x_radius, y_radius, ROCKINESS_FLAT);
                bit_set(x_radius, y_radius, BIT_FUNGUS, false);
                code_set(x_radius, y_radius, i);
            }
        }
    }
    new_landmark(x, y, StringTable->get((int)Natural[LM_URANIUM].name));
}

/*
Purpose: Setup the 'Freshwater Sea' landmark.
// ORIGINAL: 0x005C6C40 ?world_fresh@@YAXHH@Z 0x005C6C40-0x005C6DA4
// size      356 bytes
// prototype void (__cdecl ?world_fresh@@YAXHH@Z)(int xCoord, int yCoord)
// callers   3   call targets   3
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00591DB0 0x00592600 0x006169A0
// LEVER: `bit2_set(x_it, y_it, BIT2_FRESH, true)`, not `LM_FRESH` - the image
//        pushes 0x80 (BIT2_FRESH), not 7 (LM_FRESH's enum index), same
//        LM_ vs BIT2_ mixup already fixed at world_geothermal (0x005C83B0).
//        Confirmed the constant now matches byte-for-byte (`push 0x80` on
//        both sides at the bit2_set call site) but the overall score does
//        not move (still 4/125, 0.667 similar, best flag set /O2 /Gy /GR-
//        /Oy- /GX) - the earlier structural mismatch (see TRIED) already
//        dominates the comparison.
// TRIED: the image places the `bit2_set`/`x_search` update block AFTER a
//            second nested branch this tree's straight if/else-if puts it
//            before - looks like the compiler restructured the two mutually
//            exclusive branches into a different instruction order than a
//            literal transcription of the nested if/else-if produces. Not
//            chased further; same xrange/on_map-loop family register-
//            scheduling plateau as this file's other worldbuilder bodies
//            besides.
Return Value: n/a
Status: Complete - testing
*/
void __cdecl world_fresh(int x, int y) {
    uint32_t region;
    if (on_map(x, y)) {
        region = region_at(x, y);
        if (!is_ocean(x, y)) {
            return;
        }
    } else {
        uint32_t tile_search = 0;
        int region_search = -1;
        for (uint32_t i = MaxLandmarkNum; i < MaxContinentNum; i++) {
            uint32_t tile_count = Continents[i].tile_count;
            if (tile_count >= 3 && tile_count <= 32 && tile_count >= tile_search) {
                tile_search = tile_count;
                region_search = i;
            }
        }
        if (region_search < 0) {
            return;
        }
        region = region_search;
    }
    int x_search = -1;
    BOOL has_set_landmark = false;
    for (int y_it = MapLatitudeBounds - 1; y_it >= 0 ; y_it--) {
        for (int x_it = y_it & 1; x_it < MapLongitudeBounds; x_it += 2) {
            if (region_at(x_it, y_it) == region) {
                bit2_set(x_it, y_it, BIT2_FRESH, true);
                if (x_search < 0) {
                    x_search = x_it;
                }
            } else if(!has_set_landmark && x_search >= 0) {
                int x_fresh = (x_search + x_it - 2) / 2;
                new_landmark(((x_fresh ^ y_it) & 1) ^ x_fresh, y_it,
                    StringTable->get((int)Natural[LM_FRESH].name));
                has_set_landmark = true;
            }
        }
    }
}

/*
Purpose: Setup the 'Mount Planet' landmark.
// ORIGINAL: 0x005C6DB0 ?world_volcano@@YAXHHH@Z 0x005C6DB0-0x005C7016
// size      614 bytes
// prototype void (__cdecl ?world_volcano@@YAXHHH@Z)(int xCoord, int yCoord, int)
// callers   4   call targets   8
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x00591BC0 0x00591D30 0x00591DB0 0x00591E00 0x00592600 0x005C2020 0x006169A0 0x0064601D
// LEVER: same shape as world_crater (0x005C5C70) - both `rnd(bounds, NULL)`
//        calls written out as guard clauses (`if (bounds - 1 <= 0) { x = 0;
//        } else { x = rand() % bounds; }`), matching the image's call count
//        and fall-through polarity. Needs /Oy- (the marker's own `frame`
//        flag) for the comparison to even mean anything - best flag set
//        (/O2 /Gy /GR- /Oy- /GX) is 20/206, 0.633 similar; without /Oy- the
//        fuzzy similarity score is misleadingly higher (0.868) because the
//        frame-omitted form shares more MNEMONICS despite matching 0 bytes
//        from instruction 0.
// TRIED: same xrange/on_map two-induction-variable loop family plateau as
//            world_crater/world_monsoon. Not chased past this MISMATCH
//            plateau (614-byte body, 206 image instructions).
Return Value: n/a
Status: Complete - testing
*/
void __cdecl world_volcano(int x, int y, BOOL is_not_landmark) {
    uint32_t loc_attempts = 0;
    if (!on_map(x, y)) {
        do {
            do {
                int lat_bounds = MapLatitudeBounds - 16;
                int rnd_y;
                if (lat_bounds - 1 <= 0) {
                    rnd_y = 0;
                } else {
                    rnd_y = rand() % lat_bounds;
                }
                y = rnd_y + 8;
                int lon_bounds = MapLongitudeBounds;
                int x_seed;
                if (lon_bounds - 1 <= 0) {
                    x_seed = 0;
                } else {
                    x_seed = rand() % lon_bounds;
                }
                x = ((x_seed ^ y) & 1) ^ x_seed;
                if (++loc_attempts >= 1000) {
                    return;
                }
            } while (is_ocean(x, y));
        } while (near_landmark(x, y));
        if (loc_attempts >= 1000) { // redundant check? would have already returned, remove?
            return;
        }
    }
    world_alt_set(x, y, ALT_BIT_OCEAN_TRENCH, true); // purpose??
    world_alt_set(x, y, ALT_3_LEVELS_ABOVE_SEA, true);
    for (int i = 0; i < RadiusRange[1]; i++) {
        int x_radius = xrange(x + RadiusOffsetX[i]);
        int y_radius = y + RadiusOffsetY[i];
        if (on_map(x_radius, y_radius)) {
            bit2_set(x_radius, y_radius, BIT2_VOLCANO, true);
            code_set(x_radius, y_radius, i);
            uint32_t bit = bit_at(x_radius, y_radius) & ~(BIT_SUPPLY_POD | BIT_THERMAL_BORE
                | BIT_ECH_MIRROR | BIT_CONDENSER | BIT_SOIL_ENRICHER | BIT_FARM | BIT_RIVERBED
                | BIT_SOLAR_TIDAL | BIT_FUNGUS | BIT_MINE | BIT_MAGTUBE | BIT_ROAD);
            if (!i) {
                bit &= ~(BIT_UNK_4000000 | BIT_UNK_8000000);
            }
            bit |= BIT_SUPPLY_REMOVE;
            bit_put(x_radius, y_radius, bit);
            rocky_set(x_radius, y_radius, ROCKINESS_ROCKY);
        }
    }
    MountPlanetX = x;
    MountPlanetY = y;
    if (!is_not_landmark) {
        new_landmark(x, y, StringTable->get((int)Natural[LM_VOLCANO].name));
    }
}

/*
Purpose: Setup the 'Borehole Cluster' landmark. Added to SMAC in 3.0 patch.
// ORIGINAL: 0x005C7020 ?world_borehole@@YAXHH@Z 0x005C7020-0x005C753B
// size      1307 bytes
// prototype void (__cdecl ?world_borehole@@YAXHH@Z)(int xCoord, int yCoord)
// callers   3   call targets   8
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x004712A0 0x00591D60 0x00591DB0 0x00591E00 0x00592600 0x005C2020 0x006169A0 0x0064601D
// indirect  0x005C7145
// TRIED (2026-08-21): call_diff says 12 calls here vs the image's 14 (missing
//            on_map/bit_set/bit2_set/code_set instances). on_map, bit_set, bit2_set and
//            code_set are all `MEASURED inline` in map.h already - each is called from
//            dozens of sites across map.cpp, and the image itself keeps SOME of those
//            calls real while inlining others per call site (on_map has `callers 1` on
//            its OWN listing even though this function alone names it 5 times in source).
//            Un-inlining any of them here would touch every OTHER caller map.h shares
//            them with; `osmx check` has no way to hold those fixed while this one
//            function moves. Tried un-inlining near_landmark (its ORIGINAL marker, right
//            above world_crater below) on the same theory - zero effect, reverted there.
//            Left as MISMATCH; the register-allocation plateau this file already notes
//            for xrange/on_map loop bodies applies here too once the call shape is fixed.
Return Value: n/a
Status: Complete - testing
*/
void __cdecl world_borehole(int x, int y) {
    uint32_t loc_attempts = 0;
    if (!on_map(x, y)) {
        do {
            do {
                y = rnd(MapLatitudeBounds - 16, NULL) + 8;
                int x_seed = rnd(MapLongitudeBounds, NULL);
                x = ((x_seed ^ y) & 1) ^ x_seed;
                if (++loc_attempts >= 1000) {
                    return;
                }
            } while (is_ocean(x, y));
        } while (near_landmark(x, y));
        if (loc_attempts >= 1000) { // redundant check? would have already returned, remove?
            return;
        }
    }
    uint32_t seed = timeGetTime();
    int val1 = 8;
    int val2 = ((seed % 8) / 3) + 5;
    int val3 = 3 - ((seed % 8) % 3);
    uint32_t val4 = (seed / 8) % 4;
    if (val4 & 2) {
        val2--;
        val3--;
        if (val4 & 1) {
            val2++;
            val1 = 8;
            val3++;
        } else {
            val2--;
            val1 = 6;
            val3--;
        }
        val1 = (val1 + 8) % 8 + 1;
        val2 = (val2 + 8) % 8 + 1;
        val3 = (val3 + 8) % 8 + 1;
    }
    for (int i = 0; i < RadiusRange[1]; i++) {
        int x_radius = xrange(x + RadiusOffsetX[i]);
        int y_radius = y + RadiusOffsetY[i];
        if (on_map(x_radius, y_radius)) {
            world_alt_set(x_radius, y_radius, ALT_SHORE_LINE, true);
            bit_set(x_radius, y_radius, BIT_SUPPLY_REMOVE, true);
            if (i == val1 || i == val2 || i == val3) {
                bit_set(x_radius, y_radius, BIT_THERMAL_BORE, true);
                bit2_set(x_radius, y_radius, LM_BOREHOLE, true);
                code_set(x_radius, y_radius, i);
            }
        }
    }
    int x_offset_val1 = xrange(x + RadiusOffsetX[val1]); // removed from loop
    int y_offset_val1 = y + RadiusOffsetY[val1]; // removed from loop
    for (i = 0; i < 8; i++) {
        int x_radius = xrange(x_offset_val1 + RadiusOffsetX[i + 1]);
        int y_radius = y_offset_val1 + RadiusOffsetY[i + 1];
        if (on_map(x_radius, y_radius) && is_ocean(x_radius, y_radius)) {
            world_alt_set(x_radius, y_radius, ALT_SHORE_LINE, true);
        }
    }
    int x_offset_val2 = xrange(x + RadiusOffsetX[val2]); // removed from loop
    int y_offset_val2 = y + RadiusOffsetY[val2]; // removed from loop
    for (i = 0; i < 8; i++) {
        int x_radius = xrange(x_offset_val2 + RadiusOffsetX[i + 1]);
        int y_radius = y_offset_val2 + RadiusOffsetY[i + 1];
        if (on_map(x_radius, y_radius) && is_ocean(x_radius, y_radius)) {
            world_alt_set(x_radius, y_radius, ALT_SHORE_LINE, true);
        }
    }
    int x_offset_val3 = xrange(x + RadiusOffsetX[val3]); // removed from loop
    int y_offset_val3 = y + RadiusOffsetY[val3]; // removed from loop
    for (i = 0; i < 8; i++) {
        int x_radius = xrange(x_offset_val3 + RadiusOffsetX[i + 1]);
        int y_radius = y_offset_val3 + RadiusOffsetY[i + 1];
        if (on_map(x_radius, y_radius) && is_ocean(x_radius, y_radius)) {
            world_alt_set(x_radius, y_radius, ALT_SHORE_LINE, true);
        }
    }
    bit2_set(x, y, LM_BOREHOLE, true);
    new_landmark(x, y, StringTable->get((int)Natural[LM_BOREHOLE].name));
}

/*
Purpose: Setup 'The Manifold Nexus' landmark. Added to SMAC in 4.0 patch.
// ORIGINAL: 0x005C7540 ?world_temple@@YAXHH@Z 0x005C7540-0x005C7749
// size      521 bytes
// prototype void (__cdecl ?world_temple@@YAXHH@Z)(int xCoord, int yCoord)
// callers   3   call targets   7
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x00591D60 0x00591DB0 0x00591E00 0x00592600 0x005C2020 0x006169A0 0x0064601D
// LEVER: same shape as world_crater (0x005C5C70) - both `rnd(bounds,
//        NULL)` calls written out as guard clauses matching the image's
//        call count and fall-through polarity. Best flag set (/O2 /Gy
//        /GR- /Oy- /GX) moved 11/175 -> 12/175, 0.864 similar.
// TRIED: same xrange/on_map two-induction-variable loop family
//            plateau as world_crater. Not chased past this MISMATCH
//            plateau.
Return Value: n/a
Status: Complete - testing
*/
void __cdecl world_temple(int x, int y) {
    uint32_t loc_attempts = 0;
    if (!on_map(x, y)) {
        do {
            do {
                int lat_bounds = MapLatitudeBounds - 16;
                int rnd_y;
                if (lat_bounds - 1 <= 0) {
                    rnd_y = 0;
                } else {
                    rnd_y = rand() % lat_bounds;
                }
                y = rnd_y + 8;
                int lon_bounds = MapLongitudeBounds;
                int x_seed;
                if (lon_bounds - 1 <= 0) {
                    x_seed = 0;
                } else {
                    x_seed = rand() % lon_bounds;
                }
                x = ((x_seed ^ y) & 1) ^ x_seed;
                if (++loc_attempts >= 1000) {
                    return;
                }
            } while (is_ocean(x, y));
        } while (near_landmark(x, y));
        if (loc_attempts >= 1000) { // redundant check? would have already returned, remove?
            return;
        }
    }
    for (int i = 0; i < RadiusRange[1]; i++) {
        int x_radius = xrange(x + RadiusOffsetX[i]);
        int y_radius = y + RadiusOffsetY[i];
        if (on_map(x_radius, y_radius)) {
            world_alt_set(x_radius, y_radius, ALT_SHORE_LINE, true);
            bit_set(x_radius, y_radius, BIT_SUPPLY_REMOVE, true);
            bit2_set(x_radius, y_radius, LM_NEXUS, true);
            code_set(x_radius, y_radius, i);
        }
    }
    new_landmark(x, y, StringTable->get((int)Natural[LM_NEXUS].name));
}

/*
Purpose: Setup the 'Unity Wreckage' landmark (SMACX only).
// ORIGINAL: 0x005C7750 ?world_unity@@YAXHH@Z 0x005C7750-0x005C7A75
// size      805 bytes
// prototype void (__cdecl ?world_unity@@YAXHH@Z)(int xCoord, int yCoord)
// callers   3   call targets   8
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x00591BC0 0x00591D60 0x00591DB0 0x00591E00 0x00592600 0x005C2020 0x006169A0 0x0064601D
// LEVER: same shape as world_crater (0x005C5C70) - both `rnd(bounds,
//        NULL)` calls (inside the `if (ExpansionEnabled)` guard) written
//        out as guard clauses matching the image's call count and
//        fall-through polarity. Best flag set (/O2 /Gy /GR- /Oy- /GX)
//        moved 3/279 -> 41/279 raw agreeing (0.216 similar - this
//        function's `x--/y--` then `x+=2/y+=2` then `x--/y--` re-walk of
//        the same radius loop three times gives the register allocator
//        much more to disagree about than its siblings).
// TRIED: same xrange/on_map two-induction-variable loop family
//            plateau as world_crater, repeated three times over. Not
//            chased past this MISMATCH plateau (805-byte body, 279 image
//            instructions).
Return Value: n/a
Status: Complete - testing
*/
void __cdecl world_unity(int x, int y) {
    if (ExpansionEnabled) {
        uint32_t loc_attempts = 0;
        if (!on_map(x, y)) {
            do {
                do {
                    int lat_bounds = MapLatitudeBounds - 16;
                    int rnd_y;
                    if (lat_bounds - 1 <= 0) {
                        rnd_y = 0;
                    } else {
                        rnd_y = rand() % lat_bounds;
                    }
                    y = rnd_y + 8;
                    int lon_bounds = MapLongitudeBounds;
                    int x_seed;
                    if (lon_bounds - 1 <= 0) {
                        x_seed = 0;
                    } else {
                        x_seed = rand() % lon_bounds;
                    }
                    x = ((x_seed ^ y) & 1) ^ x_seed;
                    if (++loc_attempts >= 1000) {
                        return;
                    }
                } while (is_ocean(x, y));
            } while (near_landmark(x, y));
            if (loc_attempts >= 1000) { // redundant check? would have already returned, remove?
                return;
            }
        }
        x--;
        y--;
        for (int i = 0; i < RadiusRange[2]; i++) {
            int x_radius = xrange(x + RadiusOffsetX[i]);
            int y_radius = y + RadiusOffsetY[i];
            if (on_map(x_radius, y_radius)) {
                world_alt_set(x_radius, y_radius, ALT_1_LEVEL_ABOVE_SEA, true);
            }
        }
        x += 2;
        y += 2;
        for (i = 0; i < RadiusRange[2]; i++) {
            int x_radius = xrange(x + RadiusOffsetX[i]);
            int y_radius = y + RadiusOffsetY[i];
            if (on_map(x_radius, y_radius)) {
                world_alt_set(x_radius, y_radius, ALT_1_LEVEL_ABOVE_SEA, true);
            }
        }
        x--;
        y--;
        for (i = 0; i < RadiusRange[2]; i++) {
            int x_radius = xrange(x + RadiusOffsetX[i]);
            int y_radius = y + RadiusOffsetY[i];
            if (on_map(x_radius, y_radius)) {
                rocky_set(x_radius, y_radius, ROCKINESS_FLAT);
                bit_set(x_radius, y_radius, BIT_RIVERBED, false);
                bit_set(x_radius, y_radius, BIT_FUNGUS, false);
                bit2_set(x_radius, y_radius, LM_UNITY, true);
                code_set(x_radius, y_radius, i);
                if (!i || i == 8 || i == 10 || i == 19) {
                    bit_set(x_radius, y_radius, BIT_SUPPLY_POD, true);
                } else {
                    bit_set(x_radius, y_radius, BIT_SUPPLY_REMOVE, true);
                }
            }
        }
        new_landmark(x, y, StringTable->get((int)Natural[LM_UNITY].name));
    }
}

/*
Purpose: Setup the 'Fossil Ridge' landmark (SMACX only).
// ORIGINAL: 0x005C7A80 ?world_fossil@@YAXHH@Z 0x005C7A80-0x005C7CA1
// size      545 bytes
// prototype void (__cdecl ?world_fossil@@YAXHH@Z)(int xCoord, int yCoord)
// callers   3   call targets   6
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00591DB0 0x00591E00 0x00592600 0x005C2020 0x006169A0 0x0064601D
// LEVER: same shape as world_crater (0x005C5C70) - both `rnd(bounds,
//        NULL)` calls (inside the `if (ExpansionEnabled)` guard) written
//        out as guard clauses matching the image's call count and
//        fall-through polarity. Best flag set (/O2 /Gy /GR- /Oy- /GX)
//        moved 1/183 -> 31/183, 0.850 similar.
// TRIED: same xrange/on_map two-induction-variable loop family
//            plateau as world_crater. Not chased past this MISMATCH
//            plateau.
Return Value: n/a
Status: Complete - testing
*/
void __cdecl world_fossil(int x, int y) {
    if (ExpansionEnabled) { // added check similar to Unity Wreckage
        uint32_t loc_attempts = 0;
        if (!on_map(x, y)) {
            do {
                do {
                    int lat_bounds = MapLatitudeBounds - 16;
                    int rnd_y;
                    if (lat_bounds - 1 <= 0) {
                        rnd_y = 0;
                    } else {
                        rnd_y = rand() % lat_bounds;
                    }
                    y = rnd_y + 8;
                    int lon_bounds = MapLongitudeBounds;
                    int x_seed;
                    if (lon_bounds - 1 <= 0) {
                        x_seed = 0;
                    } else {
                        x_seed = rand() % lon_bounds;
                    }
                    x = ((x_seed ^ y) & 1) ^ x_seed;
                    if (++loc_attempts >= 1000) {
                        return;
                    }
                } while (!is_ocean(x, y));
            } while (near_landmark(x, y));
            if (loc_attempts >= 1000) { // redundant check? would have already returned, remove?
                return;
            }
        }
        for (uint32_t i = 0; i < 6; i++) {
            int x_radius = xrange(x + RadiusOffsetX[i]);
            int y_radius = y + RadiusOffsetY[i];
            if (on_map(x_radius, y_radius) && is_ocean(x_radius, y_radius)) {
                world_alt_set(x_radius, y_radius, ALT_OCEAN, true);
                bit2_set(x_radius, y_radius, LM_FOSSIL, true);
                code_set(x_radius, y_radius, i);
            }
        }
        new_landmark(x, y, StringTable->get((int)Natural[LM_FOSSIL].name));
    }
}

/*
Purpose: Setup the 'Nessus Canyon' landmark.
// ORIGINAL: 0x005C7CB0 ?world_canyon@@YAXHH@Z 0x005C7CB0-0x005C7F38
// size      648 bytes
// prototype void (__cdecl ?world_canyon@@YAXHH@Z)(int xCoord, int yCoord)
// callers   1   call targets   6
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x00591DB0 0x00591E00 0x00592600 0x005C2020 0x006169A0 0x0064601D
Return Value: n/a
Status: Complete - testing
*/
void __cdecl world_canyon(int x, int y) {
    uint32_t loc_attempts = 0;
    if (!on_map(x, y)) {
        do {
            do {
                y = rnd(MapLatitudeBounds - 16, NULL) + 8;
                int x_seed = rnd(MapLongitudeBounds, NULL);
                x = ((x_seed ^ y) & 1) ^ x_seed;
                if (++loc_attempts >= 1000) {
                    return;
                }
            } while (is_ocean(x, y));
        } while (near_landmark(x, y));
        if (loc_attempts >= 1000) { // redundant check? would have already returned, remove?
            return;
        }
    } 
    const uint32_t WorldCanyonOffsetValues[12] = { 26, 27, 9, 1, 2, 0, 3, 4, 17, 23, 36, 35 };
    for (uint32_t i = 0; i < 12; i++) {
        int x_radius = xrange(x + RadiusOffsetX[WorldCanyonOffsetValues[i]]);
        int y_radius = y + RadiusOffsetY[WorldCanyonOffsetValues[i]];
        if (on_map(x_radius, y_radius)) {
            world_alt_set(x_radius, y_radius, ALT_1_LEVEL_ABOVE_SEA, true);
            world_alt_set(x_radius, y_radius, ALT_SHORE_LINE, true);
            bit2_set(x_radius, y_radius, LM_CANYON, true); // rolled these two into single loop with
            code_set(x_radius, y_radius, WorldCanyonOffsetValues[i]); // the above world_alt_set()
        }
    }
    new_landmark(x, y + 2, StringTable->get((int)Natural[LM_CANYON].name));
}

/*
Purpose: Setup the 'Sunny Mesa' landmark.
// ORIGINAL: 0x005C7F40 ?world_mesa@@YAXHH@Z 0x005C7F40-0x005C8148
// size      520 bytes
// prototype void (__cdecl ?world_mesa@@YAXHH@Z)(int xCoord, int yCoord)
// callers   3   call targets   6
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00591DB0 0x00591E00 0x00592600 0x005C2020 0x006169A0 0x0064601D
// LEVER: same shape as world_crater (0x005C5C70) - both `rnd(bounds,
//        NULL)` calls written out as guard clauses matching the image's
//        call count and fall-through polarity. Best flag set (/O2 /Gy
//        /GR- /Oy- /GX) moved 1/175 -> 12/175, 0.851 similar.
// TRIED: same xrange/on_map two-induction-variable loop family
//            plateau as world_crater. Not chased past this MISMATCH
//            plateau.
Return Value: n/a
Status: Complete - testing
*/
void __cdecl world_mesa(int x, int y) {
    uint32_t loc_attempts = 0;
    if (!on_map(x, y)) {
        do {
            do {
                int lat_bounds = MapLatitudeBounds - 16;
                int rnd_y;
                if (lat_bounds - 1 <= 0) {
                    rnd_y = 0;
                } else {
                    rnd_y = rand() % lat_bounds;
                }
                y = rnd_y + 8;
                int lon_bounds = MapLongitudeBounds;
                int x_seed;
                if (lon_bounds - 1 <= 0) {
                    x_seed = 0;
                } else {
                    x_seed = rand() % lon_bounds;
                }
                x = ((x_seed ^ y) & 1) ^ x_seed;
                if (++loc_attempts >= 1000) {
                    return;
                }
            } while (is_ocean(x, y));
        } while (near_landmark(x, y));
        if (loc_attempts >= 1000) { // redundant check? would have already returned, remove?
            return;
        }
    }
    for (int i = 0; i < RadiusRange[3]; i++) {
        int x_radius = xrange(x + RadiusOffsetX[i]);
        int y_radius = y + RadiusOffsetY[i];
        if (on_map(x_radius, y_radius)) {
            world_alt_set(x_radius, y_radius, (i < RadiusRange[2]) + ALT_SHORE_LINE, true);
            bit2_set(x_radius, y_radius, LM_MESA, true);
            code_set(x_radius, y_radius, i);
        }
    }
    new_landmark(x, y + 2, StringTable->get((int)Natural[LM_MESA].name));
}

/*
Purpose: Setup the 'Pholus Ridge' landmark.
// ORIGINAL: 0x005C8150 ?world_ridge@@YAXHH@Z 0x005C8150-0x005C83AB
// size      603 bytes
// prototype void (__cdecl ?world_ridge@@YAXHH@Z)(int xCoord, int yCoord)
// callers   3   call targets   6
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x00591DB0 0x00591E00 0x00592600 0x005C2020 0x006169A0 0x0064601D
// LEVER: same shape as world_crater (0x005C5C70) - both `rnd(bounds,
//        NULL)` calls written out as guard clauses matching the image's
//        call count and fall-through polarity. Best flag set (/O2 /Gy
//        /GR- /Oy- /GX) moved 4/199 -> 5/199, 0.699 similar.
// TRIED: same xrange/on_map two-induction-variable loop family
//            plateau as world_crater. Not chased past this MISMATCH
//            plateau.
Return Value: n/a
Status: Complete - testing
*/
void __cdecl world_ridge(int x, int y) {
    uint32_t loc_attempts = 0;
    if (!on_map(x, y)) {
        do {
            do {
                int lat_bounds = MapLatitudeBounds - 16;
                int rnd_y;
                if (lat_bounds - 1 <= 0) {
                    rnd_y = 0;
                } else {
                    rnd_y = rand() % lat_bounds;
                }
                y = rnd_y + 8;
                int lon_bounds = MapLongitudeBounds;
                int x_seed;
                if (lon_bounds - 1 <= 0) {
                    x_seed = 0;
                } else {
                    x_seed = rand() % lon_bounds;
                }
                x = ((x_seed ^ y) & 1) ^ x_seed;
                if (++loc_attempts >= 1000) {
                    return;
                }
            } while (is_ocean(x, y));
        } while (near_landmark(x, y));
        if (loc_attempts >= 1000) { // redundant check? would have already returned, remove?
            return;
        }
    }
    const uint32_t world_ridge_offset_val[13] = { 47, 44, 24, 20, 8, 7, 0, 5, 4, 17, 23, 35, 455 };
    for (uint32_t i = 0; i < 12; i++) {
        int x_radius = xrange(x + RadiusOffsetX[world_ridge_offset_val[i]]);
        int y_radius = y + RadiusOffsetY[world_ridge_offset_val[i]];
        if (on_map(x_radius, y_radius)) {
            world_alt_set(x_radius, y_radius, ALT_1_LEVEL_ABOVE_SEA, true);
            bit2_set(x_radius, y_radius, LM_RIDGE, true); // rolled these two into single loop with
            code_set(x_radius, y_radius, world_ridge_offset_val[i]); // the above world_alt_set()
        }
    }
    new_landmark(x, y + 2, StringTable->get((int)Natural[LM_RIDGE].name));
}

/*
Purpose: Setup the 'Geothermal Shallows' landmark.
// ORIGINAL: 0x005C83B0 ?world_geothermal@@YAXHH@Z 0x005C83B0-0x005C8627
// size      631 bytes
// prototype void (__cdecl ?world_geothermal@@YAXHH@Z)(int xCoord, int yCoord)
// callers   3   call targets   6
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00591DB0 0x00591E00 0x00592600 0x005C2020 0x006169A0 0x0064601D
// LEVER: same shape as world_crater (0x005C5C70) - both `rnd(bounds,
//        NULL)` calls written out as guard clauses matching the image's
//        call count and fall-through polarity. Best flag set (/O2 /Gy
//        /GR- /Oy- /GX) moved 3/207 -> 12/207, 0.492 similar.
// TRIED: same xrange/on_map two-induction-variable loop family
//            plateau as world_crater. Not chased past this MISMATCH
//            plateau.
Return Value: n/a
Status: Complete - testing
*/
void __cdecl world_geothermal(int x, int y) {
    uint32_t loc_attempts = 0;
    if (!on_map(x, y)) {
        do {
            do {
                int lat_bounds = MapLatitudeBounds - 16;
                int rnd_y;
                if (lat_bounds - 1 <= 0) {
                    rnd_y = 0;
                } else {
                    rnd_y = rand() % lat_bounds;
                }
                y = rnd_y + 8;
                int lon_bounds = MapLongitudeBounds;
                int x_seed;
                if (lon_bounds - 1 <= 0) {
                    x_seed = 0;
                } else {
                    x_seed = rand() % lon_bounds;
                }
                x = ((x_seed ^ y) & 1) ^ x_seed;
                if (++loc_attempts >= 1000) {
                    return;
                }
            } while (!is_ocean(x, y));
        } while (near_landmark(x, y));
        if (loc_attempts >= 1000) { // redundant check? would have already returned, remove?
            return;
        }
    }
    // LEVER: this is the ONLY calls block - the image has exactly one
    // world_alt_set/bit2_set/code_set per iteration, gated by BOTH conditions below,
    // with a plain `has_skipped = true;` (no calls at all) on the else side. The prior
    // recovery invented a second, unconditional set of the same three calls after the
    // if/else, which the disassembly does not have - `uv run tools/call_diff.py
    // 0x005C83B0` is what caught it (this tree made 13 calls against the image's 9),
    // and walking 0x005C83B0's raw bytes end to end confirmed the single-block shape.
    // The bit2_set argument is BIT2_GEOTHERMAL (0x400), not LM_GEOTHERMAL (10) - the
    // image pushes 0x400.
    BOOL has_skipped = false;
    for (int i = 0; i < RadiusRange[2]; i++) {
        int x_radius = xrange(x + RadiusOffsetX[i]);
        int y_radius = y + RadiusOffsetY[i];
        if (on_map(x_radius, y_radius) && (is_ocean(x_radius, y_radius) || i < 9)) {
            if ((has_skipped || !i || rand() % 25) && (i < 9 || rand() % 3)) {
                world_alt_set(x_radius, y_radius, ALT_OCEAN_SHELF, true);
                bit2_set(x_radius, y_radius, BIT2_GEOTHERMAL, true);
                code_set(x_radius, y_radius, i);
            } else {
                has_skipped = true;
            }
        }
    }
    new_landmark(x, y, StringTable->get((int)Natural[LM_GEOTHERMAL].name));
}

/*
Purpose: Set up all the landmarks.
// ORIGINAL: 0x005C8630 ?world_landmarks@@YAXXZ 0x005C8630-0x005C86DC
// size      172 bytes
// prototype 
// callers   0   call targets   15
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005C5C70 0x005C5EF0 0x005C6200 0x005C64A0 0x005C6740 0x005C69E0 0x005C6C40 0x005C6DB0 0x005C7020 0x005C7540 0x005C7750 0x005C7A80 0x005C7F40 0x005C8150 0x005C83B0
Return Value: n/a
Status: Complete - testing
*/
void __cdecl world_landmarks() {
    world_monsoon(-1, -1);
    world_crater(-1, -1);
    world_volcano(-1, -1, false);
    world_mesa(-1, -1);
    world_ridge(-1, -1);
    world_diamond(-1, -1);
    world_ruin(-1, -1);
    world_unity(-1, -1); // removed redundant ExpansionEnabled check
    world_fossil(-1, -1); // moved ExpansionEnabled check to inside function
    world_temple(-1, -1);
    world_borehole(-1, -1);
    world_sargasso(-1, -1);
    world_dune(-1, -1);
    world_fresh(-1, -1);
    world_geothermal(-1, -1);
    world_canyon(-1, -1); // Change: add "Nessus Canyon" to random map generation
    fixup_landmarks();
}

/*
Purpose: Check for any type of zone of control conflicts (base and/or unit).
// ORIGINAL: 0x005C89F0 ?zoc_any@@YAHHHH@Z 0x005C89F0-0x005C8AC0
// TRIED: same xrange/on_map-loop family ceiling as zoc_veh (0x005C8AC0,
//            right below) and base_on_sea - best 3/82, 0.786 similar across
//            every flag set. Not chased further.
// size      208 bytes
// prototype int (__cdecl ?zoc_any@@YAHHHH@Z)(int xCoord, int yCoord, int factionID)
// callers   5   call targets   0
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     (none)
Return Value: If ZOC, faction id + 1; Otherwise, 0 (however return seems to be treated as boolean)
Status: Complete
*/
int __cdecl zoc_any(int x, int y, int faction_id) {
    for (uint32_t i = 0; i < 8; i++) {
        int x_radius = xrange(x + RadiusBaseX[i]);
        int y_radius = y + RadiusBaseY[i];
        if (on_map(x_radius, y_radius)) {
            int owner = anything_at(x_radius, y_radius);
            if (owner >= 0 && (uint32_t)owner != faction_id
                && !has_treaty(faction_id, owner, DTREATY_PACT)) {
                return owner + 1;
            }
        }
    }
    return 0;
}

/*
Purpose: Check for unit related zone of control conflicts.
// ORIGINAL: 0x005C8AC0 ?zoc_veh@@YAHHHH@Z 0x005C8AC0-0x005C8B97
// TRIED: 13/80, 0.969 similar - best of every flag set. Same
//            xrange/on_map-loop family ceiling as base_on_sea (0x0050DE50):
//            the image caches MapIsFlat/MapLongitudeBounds into registers
//            in the prologue, ahead of the loop; this tree's -O2 loads them
//            one instruction later and via a different pair of registers.
// size      215 bytes
// prototype int (__cdecl ?zoc_veh@@YAHHHH@Z)(int xCoord, int yCoord, int factionID)
// callers   4   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
Return Value: If ZOC, faction id + 1; Otherwise, 0 (however return seems to be treated as boolean)
Status: Complete
*/
int __cdecl zoc_veh(int x, int y, int faction_id) {
    uint32_t ret = 0;
    for (uint32_t i = 0; i < 8; i++) {
        int x_radius = xrange(x + RadiusBaseX[i]);
        int y_radius = y + RadiusBaseY[i];
        if (on_map(x_radius, y_radius)) {
            int owner = veh_who(x_radius, y_radius);
            if (owner >= 0 && (uint32_t)owner != faction_id
                && !has_treaty(faction_id, owner, DTREATY_PACT)) {
                owner++;
                if (ret <= (uint32_t)owner) {
                    ret = owner; // any point in continuing after 1st instance of ZOC is found?
                }
            }
        }
    }
    return ret;
}

/*
Purpose: Check for unit related zone of control conflicts taking into account land or ocean.
// ORIGINAL: 0x005C8BA0 ?zoc_sea@@YAHHHH@Z 0x005C8BA0-0x005C8D36
// TRIED: 14/133, 0.908 similar - best of every flag set. First
//            divergence is is_ocean's own known plateau (`xor ecx,ecx`
//            before the map_tiles() pointer load in the image, this tree
//            loads the pointer first) - same root cause as is_ocean's
//            standalone note, not something local to this call site.
// size      406 bytes
// prototype int (__cdecl ?zoc_sea@@YAHHHH@Z)(int xCoord, int yCoord, int factionID)
// callers   5   call targets   1
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x005BFE90
Return Value: If ZOC, faction id + 1; Otherwise, 0 (however return seems to be treated as boolean)
Status: Complete
*/
int __cdecl zoc_sea(int x, int y, int faction_id) {
    BOOL is_ocean_tile = is_ocean(x, y);
    for (uint32_t i = 0; i < 8; i++) {
        int x_radius = xrange(x + RadiusBaseX[i]);
        int y_radius = y + RadiusBaseY[i];
        if (on_map(x_radius, y_radius)) {
            int owner = veh_who(x_radius, y_radius);
            if (owner >= 0 && (uint32_t)owner != faction_id
                && is_ocean(x_radius, y_radius) == is_ocean_tile
                && !has_treaty(faction_id, owner, DTREATY_PACT)) {
                for (int veh_id = veh_at(x_radius, y_radius); veh_id >= 0;
                    veh_id = Vehs[veh_id].next_veh_id_stack) {
                    if (Vehs[veh_id].faction_id != faction_id
                        && (Vehs[veh_id].visibility & (1 << faction_id)
                            || (!IsMultiplayerNet && !(Vehs[veh_id].flags & VFLAG_INVISIBLE)))) {
                        return owner + 1;
                    }
                }
            }
        }
    }
    return 0;
}

/*
Purpose: Check for unit related zone of control conflicts when moving to a non-base tile.
// ORIGINAL: 0x005C8D40 ?zoc_move@@YAHHHH@Z 0x005C8D40-0x005C8D96 BYTE_EXACT
// size      86 bytes
// prototype int (__cdecl ?zoc_move@@YAHHHH@Z)(int xCoord, int yCoord, int factionID)
// callers   6   call targets   1
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x005C8BA0
Return Value: If ZOC, faction id + 1; Otherwise, 0 (however return seems to be treated as boolean)
Status: Complete
*/
int __cdecl zoc_move(int x, int y, int faction_id) {
    int owner;
    if (!(bit_at(x, y) & BIT_BASE_IN_TILE)
        || ((owner = owner_at(x, y)), owner >= 8 || owner < 0)) {
        return zoc_sea(x, y, faction_id);
    }
    return 0;
}


/*
Purpose: Shore-edge detail for one corner of a tile whose scan found the coast.
Original Offset: n/a
Return Value: Detail value for the corner
Status: Complete

MSVC emitted each of the four occurrences as a four-entry jump table guarded by
an unsigned `cmp corner, 3 / ja`, with the out-of-range arm falling through to
the shared `return detail` after `detail` had already been loaded with 60. A
corner outside 0..3 is therefore 60 rather than undefined, and a negative one
takes the same arm because the compare is unsigned.

The four tables hold only two distinct values between them, 57 at 00462684 and
67 at 0046259F, and re-indexed by the direction the scan was looking in they
collapse to one rule: coast above the corner reads 67, coast below it reads 57.
That is the same 60 +7 / -3 shading the corner arm applies arithmetically.
*/
__forceinline static int alt_shore_detail(int corner, int west, int north, int east, int south) {
    switch (corner) {
      case 0:
        return west;
      case 1:
        return north;
      case 2:
        return east;
      case 3:
        return south;
    }
    return 60;
}

/*
Purpose: Interpolate the rendered altitude detail at one point of a tile's
         terrain polygon, so that the map renderer can slope the tile towards
         its neighbours and break the contour at the water's edge.
// ORIGINAL: 0x00462190 ?alt_get_ocean_detail@@YAHHHHH@Z 0x00462190-0x00462699
// LEVER: call_diff --all flagged this body as making 1 call the image does not
//        (`0 vs 1 - MORE`) - `alt_shore_detail`'s switch has a jump table, which VC6's
//        automatic /Ob2 inlining did not fold at all four call sites without an explicit
//        hint. `__forceinline` on `alt_shore_detail` brought call_diff back to agreement
//        (image and tree both make 0 direct calls, 4 indirect jumps apiece) and moved
//        6/431 agreeing instructions to 9/431.
// TRIED: remaining mismatch starts at the prologue - image is `sub esp, 0xc` (3
//            spill slots), this tree is `sub esp, 8` (2) - the same "image spills a
//            local the tree keeps in a register" shape as 0x00532A90/0x00532B70, on a
//            431-instruction body. Not chased further at this size.
// size      1289 bytes
// prototype 
// callers   1   call targets   0
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     (none)
Return Value: Altitude detail, 0 to 79; 0 for a tile off the map
Status: Complete

`corner` selects one of the tile's four diamond corners - 0 west, 1 north,
2 east, 3 south - and `point` selects which point of the polygon near that
corner is being asked about:

    point 0   the tile's own centre, so its own contour byte unaltered
    point 1   the edge midpoint on one side of the corner
    point 2   the corner itself
    point 3   the edge midpoint on the other side

and the value is the mean of the tiles that touch that point: one for the
centre, two for an edge midpoint, four for a corner. Every term is clamped to
0..79 before it is summed, and the mean is an arithmetic shift rather than a
division because every term is then non-negative.

The corner case applies the relief bias that gives the coast its lip: +7 when
the tile above the corner is at ALT_BIT_SHORE_LINE or above, -3 when the tile
below it is. Those two are CornerOffset[corner][1] and [corner][3], the same
two tiles the two edge midpoints average against.

The centre and edge-midpoint cases instead look for a shoreline crossing. The
tile's own altitude decides which way round the test goes, and a crossing
returns one of two fixed details, 57 or 67, chosen by corner - so a polygon
point that straddles the waterline is pinned to a constant instead of being
interpolated.

An off-map neighbour contributes nothing but does not shrink the divisor, so a
point on the edge of the map reads at half or a quarter depth. That is the
original's arithmetic, not an omission here.

Verification note: THE ORIGINAL BOUNDS-CHECKS `corner` IN ONE HALF OF ITSELF
AND NOT THE OTHER. All four shore switches guard it with an unsigned
`cmp / ja`, while `shl esi, 4` at 004622ED and `shl ebx, 4` at 0046239B index
the sixteen-entry offset tables with no check at all, so `corner == 4` in the
original reads the Y table as the X table and a negative one reads below the
array. It is latent: the sole caller, MapWin::gen_terrain_poly, derives the
argument from a value it has already reduced. This recovery reproduces the
guarded half exactly and leaves the unguarded half as an out-of-range index
rather than hard-coding the adjacent table, so an even `point` with a `corner`
outside 0..3 is unreachable-and-unspecified here where it is
unreachable-and-accidental there.

Verification note: THE THREE-STEP SCAN IS TWO-THIRDS DEAD, in the original as
much as here. Its body reads MapTiles, MapIsFlat and the two bounds and writes
nothing, so an iteration that does not return is unobservable. It can only
return on `point == 1 && i == 0` or `point == 3 && i == 2`, so iteration 1
never returns for any input, iteration 2 never returns for point 1, iteration 0
never returns for point 3, and for point 0 - or any point that is even and not
2 - the entire loop cannot return at all. It is transcribed as written because
it is what 004624BE through 0046256A and 004625AB through 00462657 execute.
The sweep's two surviving `i < 3` -> `i <= 3` mutants are that fact: a fourth
iteration matches neither `i == 0` nor `i == 2` and so cannot return either.

Verification note: the scan's own direction arithmetic proves it further. The
scan starts at `corner * 2 - 4` and (c*2-4) & 7 == (1 + c*2 + 3) & 7, so
iteration 0 tests exactly the neighbour point 1 averaged against; likewise
(c*2-2) & 7 == (3 + c*2 + 3) & 7, so iteration 2 tests point 3's. The scan
never looks at a tile the average did not already read.

Verification note: THE LOWER CLAMP ARMS CANNOT FIRE. `contour` is a uint8_t
field, so the value fed to each `if (v < 0) v = 0;` is in 0..255 and the arm is
unreachable; only the upper arm can act. The original tests it all the same
(00462262, 004622A5, 0046235B), so fifteen mutants survive across the three
sites - the `0 -> 1` and the drop of each `v = 0`, the `0 -> 1` in each
`if (v < 0)`, and each `< -> <=`, which for v == 0 assigns 0 to a 0. The three
`> -> >=` on the upper arm are equivalent for the same reason: they assign 79
to a 79. The final clamp before the return is NOT in that class - point 2's -3
can take the mean below zero and its +7 can take it past 79, and point 0
returns a raw contour byte that can be up to 255 - but its own `0 -> 1` and
`< -> <=` are still equivalent, because `detail == 0` returns 0 either way, and
so is its `> -> >=` at 79.
*/
int __cdecl alt_get_ocean_detail(int x, int y, int corner, int point) {
    if (!on_map(x, y)) {
        return 0;
    }
    int detail;
    if (!point) {
        detail = site_tile(x, y)->contour;
    } else if (point & 1) {
        // An edge midpoint: this tile averaged with the one across that edge.
        int dir = (point + corner * 2 + 3) & 7;
        int x_edge = site_xrange(x + RadiusBaseX[dir]);
        int y_edge = y + RadiusBaseY[dir];
        detail = site_tile(x, y)->contour;
        if (detail < 0) {
            detail = 0;
        } else if (detail > 79) {
            detail = 79;
        }
        if (on_map(x_edge, y_edge)) {
            int across = site_tile(x_edge, y_edge)->contour;
            if (across < 0) {
                across = 0;
            } else if (across > 79) {
                across = 79;
            }
            detail += across;
        }
        detail >>= 1;
    } else {
        // The corner itself: the four tiles that meet there.
        detail = 0;
        for (int i = 0; i < 4; i++) {
            int x_meet = site_xrange(x + CornerOffsetX[corner][i]);
            int y_meet = y + CornerOffsetY[corner][i];
            if (!on_map(x_meet, y_meet)) {
                continue;
            }
            int sample = site_tile(x_meet, y_meet)->contour;
            if (sample < 0) {
                sample = 0;
            } else if (sample > 79) {
                sample = 79;
            }
            detail += sample;
        }
        detail >>= 2;
    }
    if (point == 2) {
        int x_above = site_xrange(x + CornerOffsetX[corner][1]);
        int y_above = y + CornerOffsetY[corner][1];
        if (on_map(x_above, y_above)
            && (site_tile(x_above, y_above)->climate & 0xE0) >= ALT_BIT_SHORE_LINE) {
            detail += 7;
        }
        int x_below = site_xrange(x + CornerOffsetX[corner][3]);
        int y_below = y + CornerOffsetY[corner][3];
        if (on_map(x_below, y_below)
            && (site_tile(x_below, y_below)->climate & 0xE0) >= ALT_BIT_SHORE_LINE) {
            detail -= 3;
        }
    } else if ((site_tile(x, y)->climate & 0xE0) < ALT_BIT_SHORE_LINE) {
        // Water looking for land.
        int dir = corner * 2 - 4;
        for (int i = 0; i < 3; i++, dir++) {
            int x_scan = site_xrange(x + RadiusBaseX[dir & 7]);
            int y_scan = y + RadiusBaseY[dir & 7];
            if (!on_map(x_scan, y_scan)
                || (site_tile(x_scan, y_scan)->climate & 0xE0) < ALT_BIT_SHORE_LINE) {
                continue;
            }
            if (point == 1 && i == 0) {
                return alt_shore_detail(corner, 57, 67, 67, 57);
            }
            if (point == 3 && i == 2) {
                return alt_shore_detail(corner, 67, 67, 57, 57);
            }
        }
    } else {
        // Land looking for water.
        int dir = corner * 2 - 4;
        for (int i = 0; i < 3; i++, dir++) {
            int x_scan = site_xrange(x + RadiusBaseX[dir & 7]);
            int y_scan = y + RadiusBaseY[dir & 7];
            if (!on_map(x_scan, y_scan)
                || (site_tile(x_scan, y_scan)->climate & 0xE0) >= ALT_BIT_SHORE_LINE) {
                continue;
            }
            if (point == 1 && i == 0) {
                return alt_shore_detail(corner, 67, 57, 57, 67);
            }
            if (point == 3 && i == 2) {
                return alt_shore_detail(corner, 57, 57, 67, 67);
            }
        }
    }
    if (detail < 0) {
        return 0;
    }
    if (detail > 79) {
        return 79;
    }
    return detail;
}

// Non-inline forwarder to `bit_set` (map.h): base_mark (base.cpp) is the one
// call site where the image emits a real `call 0x591D60` for BIT_BASE_RADIUS
// rather than inlining the bit test/set that every other caller gets. The
// E8 target is a relocation on both sides and is discounted, so this
// symbol's own name costs nothing.
void __cdecl bit_set_call(int x, int y, int bit, BOOL set) {
    bit_set(x, y, bit, set);
}
