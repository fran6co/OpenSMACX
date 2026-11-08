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
#include "popup.h"
#include "general.h"
#include "win.h"
#include "map.h"
#include "map_coordinates.h"
#include "alpha.h"
#include "base.h"
#include "faction.h"
#include "game.h"
#include "interface_palette.h"
#include "path.h"
#include "random.h"
#include "strings.h"
#include "technology.h"
#include "terraforming.h"
#include "veh.h"
#include "log.h"
#include "lock.h"     // map_tile_bytes() - site_at/set_dirty walk its records
#include "netdaemon.h"
#include "net_checksum_report.h"
#include "console.h"
#include "netmsg.h"   // NetMsgPrimary - synch_display posts through it
#include "mapwin.h"
#include "worldwin.h"
#include "xpops.h"
#include "palette.h"

// Shared renderer corner coordinates, formerly duplicated by map.h in each TU.
// Original get_alt reads signed dwords at (corner - 1) * 16 + neighbor * 4;
// alt_get_ocean_detail and the polygon renderer use the same four-by-four rows.
// RECOVERY_OBJECT: v1 0x00685EA0 ?CornerOffsetX@@3QAY03$$CBHA
const int CornerOffsetX[4][4] = {
    { -2, -1,  0, -1 },
    { -1,  0,  1,  0 },
    {  0,  1,  2,  1 },
    { -1,  0,  1,  0 },
};
// RECOVERY_OBJECT: v1 0x00685EE0 ?CornerOffsetY@@3QAY03$$CBHA
const int CornerOffsetY[4][4] = {
    {  0, -1,  0,  1 },
    { -1, -2, -1,  0 },
    {  0, -1,  0,  1 },
    {  1,  0,  1,  2 },
};

// Single shared storage for the parallel radius offsets and ring limits.
// Pinned neighboring starts delimit 289 signed dwords per offset axis;
// the nine ring limits are independently read by MapWin::UNK6.
// RECOVERY_OBJECT: v2 0x0066F8C4 0x0066F8E8 ?RadiusRange@@3QBHB 0x0046FC80
const int RadiusRange[9] = { 1, 9, 25, 49, 81, 121, 169, 225, 289 };

// RECOVERY_OBJECT: v2 0x0066EFBC 0x0066F440 ?RadiusOffsetX@@3QBHB 0x005C2CB0
const int RadiusOffsetX[289] = {
     0,  1,  2,  1,  0, -1, -2,  -1,   0,   2,   2,  -2,  -2,   1,   3,   3,   1,  -1,  -3,  -3,
    -1,  4, -4,  0,  0,  1,  2,   3,   4,   5,   5,   4,   3,   2,   1,  -1,  -2,  -3,  -4,  -5,
    -5, -4, -3, -2, -1,  0,  6,   0,  -6,   0,   1,   2,   3,   4,   5,   6,   7,   8,   7,   6,
     5,  4,  3,  2,  1,  0, -1,  -2,  -3,  -4,  -5,  -6,  -7,  -8,  -7,  -6,  -5,  -4,  -3,  -2,
    -1,  0,  1,  2,  3,  4,  5,   6,   7,   8,   9,  10,   9,   8,   7,   6,   5,   4,   3,   2,
     1,  0, -1, -2, -3, -4, -5,  -6,  -7,  -8,  -9, -10,  -9,  -8,  -7,  -6,  -5,  -4,  -3,  -2,
    -1,  0,  1,  2,  3,  4,  5,   6,   7,   8,   9,  10,  11,  12,  11,  10,   9,   8,   7,   6,
     5,  4,  3,  2,  1,  0, -1,  -2,  -3,  -4,  -5,  -6,  -7,  -8,  -9, -10, -11, -12, -11, -10,
    -9, -8, -7, -6, -5, -4, -3,  -2,  -1,   0,   1,   2,   3,   4,   5,   6,   7,   8,   9,  10,
    11, 12, 13, 14, 13, 12, 11,  10,   9,   8,   7,   6,   5,   4,   3,   2,   1,   0,  -1,  -2,
    -3, -4, -5, -6, -7, -8, -9, -10, -11, -12, -13, -14, -13, -12, -11, -10,  -9,  -8,  -7,  -6,
    -5, -4, -3, -2, -1,  0,  1,   2,   3,   4,   5,   6,   7,   8,   9,  10,  11,  12,  13,  14,
    15, 16, 15, 14, 13, 12, 11,  10,   9,   8,   7,   6,   5,   4,   3,   2,   1,   0,  -1,  -2,
    -3, -4, -5, -6, -7, -8, -9, -10, -11, -12, -13, -14, -15, -16, -15, -14, -13, -12, -11, -10,
    -9, -8, -7, -6, -5, -4, -3,  -2,  -1, };

// RECOVERY_OBJECT: v2 0x0066F440 0x0066F8C4 ?RadiusOffsetY@@3QBHB 0x005C2CB0
const int RadiusOffsetY[289] = {
     0,  -1,   0,   1,   2,   1,   0,  -1,  -2,  -2,   2,   2,  -2,  -3, -1,  1,  3,  3,  1, -1,
    -3,   0,   0,   4,  -4,  -5,  -4,  -3,  -2,  -1,   1,   2,   3,   4,  5,  5,  4,  3,  2,  1,
    -1,  -2,  -3,  -4,  -5,   6,   0,  -6,   0,  -8,  -7,  -6,  -5,  -4, -3, -2, -1,  0,  1,  2,
     3,   4,   5,   6,   7,   8,   7,   6,   5,   4,   3,   2,   1,   0, -1, -2, -3, -4, -5, -6,
    -7, -10,  -9,  -8,  -7,  -6,  -5,  -4,  -3,  -2,  -1,   0,   1,   2,  3,  4,  5,  6,  7,  8,
     9,  10,   9,   8,   7,   6,   5,   4,   3,   2,   1,   0,  -1,  -2, -3, -4, -5, -6, -7, -8,
    -9, -12, -11, -10,  -9,  -8,  -7,  -6,  -5,  -4,  -3,  -2,  -1,   0,  1,  2,  3,  4,  5,  6,
     7,   8,   9,  10,  11,  12,  11,  10,   9,   8,   7,   6,   5,   4,  3,  2,  1,  0, -1, -2,
    -3,  -4,  -5,  -6,  -7,  -8,  -9, -10, -11, -14, -13, -12, -11, -10, -9, -8, -7, -6, -5, -4,
    -3,  -2,  -1,   0,   1,   2,   3,   4,   5,   6,   7,   8,   9,  10, 11, 12, 13, 14, 13, 12,
    11,  10,   9,   8,   7,   6,   5,   4,   3,   2,   1,   0,  -1,  -2, -3, -4, -5, -6, -7, -8,
    -9, -10, -11, -12, -13, -16, -15, -14, -13, -12, -11, -10,  -9,  -8, -7, -6, -5, -4, -3, -2,
    -1,   0,   1,   2,   3,   4,   5,   6,   7,   8,   9,  10,  11,  12, 13, 14, 15, 16, 15, 14,
    13,  12,  11,  10,   9,   8,   7,   6,   5,   4,   3,   2,   1,   0, -1, -2, -3, -4, -5, -6,
    -7,  -8,  -9, -10, -11, -12, -13, -14, -15, };

// The paired diagonal steps are also the farm raster's one-based lookup
// data at 0x66EF28/0x66EF3C. farm_get skips index zero and uses [index - 1].
// Keep the two measured four-dword windows independently source-owned.
// RECOVERY_OBJECT: v2 0x0066EF28 0x0066EF38 ?RiverStepX@@3QBHB 0x00409100
const int RiverStepX[4] = { 1, 1, -1, -1 };
// RECOVERY_OBJECT: v2 0x0066EF3C 0x0066EF4C ?RiverStepY@@3QBHB 0x00409100
const int RiverStepY[4] = { -1, 1, 1, -1 };

// Pinned persistent reads cover 0x006858D8..0x00685908 and
// 0x00685908..0x0068593C in writable .data. These bounded windows do not
// establish original source constness, full allocation extents, or writers.
// RECOVERY_OBJECT: v2 0x006858D8 0x00685908 ?WorldCanyonOffsetValues@@3PAIA 0x005C7DFD
uint32_t WorldCanyonOffsetValues[12] = { 26, 27, 9, 1, 2, 0, 3, 4, 17, 23, 36, 35 };
// RECOVERY_OBJECT: v2 0x00685908 0x0068593C ?world_ridge_offset_val@@3PAIA 0x005C8299
uint32_t world_ridge_offset_val[13] = { 47, 44, 24, 20, 8, 7, 0, 5, 4, 17, 23, 35, 45 };

// Nine original direction indices, bounded by the next RadiusOffsetX
// table at 0x0066EFBC. MapWin's legend and tutorial navigation consume
// this permutation; it is independent data, not a callable/address alias.
// RECOVERY_OBJECT: v2 0x0066EF98 0x0066EFBC ?MapDirectionPermutation@@3QBHB 0x0046F8C0
const int MapDirectionPermutation[9] = { 9, 6, 3, 2, 1, 4, 7, 8, 5 };


// The tile-flag mask the fungus pass clears off the Map record it has just
// given bit 0x20 to: world_fungus does `tile->bit &= ~MapTileFlagClearMask`
// (0x005C35EB, `not edx` then `and [ecx + eax*4 + 8], edx`), and the same
// dword is the value argument at four more sites that set tiles through
// 0x00591D60 (0x004CE6D5 is one). Eight readers in the whole image, all of
// them reads, and the shipped .data holds 0x00288050 - the same constant the
// 0x004CE6C9 call site pushes as a literal plus 0xc. No image symbol and no
// name anywhere in the tree before this.
// Start binding only: the original DWORD reads establish the accessed value,
// not an enclosing allocation or the absence of indirect writers.
// RECOVERY_OBJECT: v1 0x006919EC ?MapTileFlagClearMask@@3IA
uint32_t MapTileFlagClearMask = 0x00288050;
// EIGHT OF THEM, one per player: the two sites below clear
// `sizeof(int) * MaxPlayerNum` bytes, which is what the extent is.
// world_alt_set (0x005C2033/0x005C2038) clears eight DWORDs here;
// alt_set (0x00591432/0x00591453) reads and increments the faction's count.
// These bindings retain starts; the observed 32-byte windows do not prove
// the boundaries of any enclosing original allocation.
// RECOVERY_OBJECT: v1 0x009B2290 ?MapBaseSubmergedCount@@3PAHA
int MapBaseSubmergedCount[8];
// world_alt_set (0x005C2042/0x005C2049) fills eight DWORDs with -1.
// alt_set writes at 0x00591632; action_terraform reads at 0x004CA46C and
// 0x004CA47E with distinct Console-faction and vehicle-faction indices.
// Bound: `mov ecx, 8; rep stosd` resets the eight faction slots (0x005C2049); 0x009B22DC is a separate dword.
// RECOVERY_OBJECT: v2 0x009B22BC 0x009B22DC ?MapBaseIdClosestSubmergedVeh@@3PAHA 0x005C2049
int MapBaseIdClosestSubmergedVeh[8];
int BrushVal1;  // 0x009B22B0
// RECOVERY_OBJECT: v4 0x009B22B8 ?BrushVal2@@3IA target ?BrushVal2@@3HA
int BrushVal2;  // 0x009B22B8
int WorldBuildVal1;  // 0x009B22B4


/*
Purpose: Check whether the coordinates are on the map.
// ORIGINAL: 0x004712A0 ?on_map@@YAHHH@Z 0x004712A0-0x004712CC BYTE_EXACT
// body      src/map.h
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
Purpose: Find closest base to territory owned by another faction not at war with specified faction.
// ORIGINAL: 0x004E3FA0 ?base_territory@@YAHHHH@Z 0x004E3FA0-0x004E4017 BYTE_EXACT
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
int __cdecl base_territory(int faction_id, int x, int y) {
    int base_id;
    int owner = whose_territory(faction_id, x, y, &base_id, false);
    if (owner >= 0 && faction_id != (uint32_t)owner && (is_human(faction_id) || is_human(owner))
        && (PlayersData[faction_id].diplo_treaties[owner] & DTREATY_VENDETTA) == 0) {
        return base_id;
    }
    return -1;
}

/*
Purpose: For the specified tile, calculate the quality of the terrain.
// ORIGINAL: 0x004ECB90 ?crappy@@YAHHH@Z 0x004ECB90-0x004ECBE1 BYTE_EXACT
// LEVER: every test re-reads map_loc() (VC6 CSEs the pointer), rocky as
//         `& ~0x3F` (byte and, dword cmp), rainfall cast to uint8_t.
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
    int poor_quality = 0;
    if (!(map_loc(x, y)->climate & (RAINFALL_RAINY | RAINFALL_MOIST))) {
        poor_quality = 1; // neither moist or rainy
    }
    if ((map_loc(x, y)->val3 & ~0x3F) == TERRAIN_BIT_ROCKY) {
        poor_quality++; // rocky
    }
    if ((map_loc(x, y)->val3 & ~0x3F) == ROCKINESS_FLAT
        && (uint8_t)(map_loc(x, y)->climate & (RAINFALL_RAINY | RAINFALL_MOIST)) < RAINFALL_RAINY) {
        poor_quality++; // flat, moist or arid
    }
    return poor_quality;
}

/*
Purpose: Take the absolute distance between two points as parameters to calculate how far out they
         radiate. This is mainly used to determine proximity or how far away the two points are from
         each other in a rough circle shape (see RadiusOffsetX[]/RadiusOffsetY[]).
// ORIGINAL: 0x004F8090 ?vector_dist@@YAHHH@Z 0x004F8090-0x004F80CB BYTE_EXACT
// body      src/map.h
// LEVER: the two `abs()` stay real calls (image: `call _abs` 0x00644F3A
//            twice) because stdafx.h's project-wide `#pragma function` line
//            names abs - /Oi would otherwise expand it inline at every site.
//            An `abs_call` forwarder carried this body before the line was
//            extended; same root cause as path.cpp's `memset_call`.
// size      59 bytes
// prototype int (__cdecl ?vector_dist@@YAHHH@Z)(int xCoord, int yCoord)
// callers   5   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00644F3A
Return Value: Distance radius
Status: Complete
*/
// BODY IN map.h as `MEASURED inline`: vulnerable inlines it while
// base_ecology, enemy_move, mind_control and probe call it.

/*
Purpose: Take two points and calculate how far out they radiate. This is mainly used to determine
         proximity or how far away the two points are from each other in a rough circle shape.
// ORIGINAL: 0x005A5910 ?vector_dist@@YAHHHHH@Z 0x005A5910-0x005A5987 BYTE_EXACT
// body      src/map.h
// Historical LEVER: inlining the two-argument overload here removed the
//        original function call from this body. The overload now has an
//        ordinary map.cpp definition above; same-translation-unit visibility
//        is retained for measurement rather than forcing a call boundary.
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
// BODY IN map.h as `MEASURED inline`: vulnerable inlines it; probe calls it.

/*
Purpose: Check whether there is a path between two regions. It seems to only take into account land
         destinations. TODO: Revisit in the future when Continent/Path is complete.
// ORIGINAL: 0x0050DDC0 ?sea_coast@@YAHHH@Z 0x0050DDC0-0x0050DE00 BYTE_EXACT
// LEVER: two spellings, both visible in the image's own body. (1) the
//            offset out-param is the PARAMETER ITSELF - `bitmask_call(...,
//            &region_src, &mask)`, then `sea_coasts[region_src]` reads it
//            back - not a separate `int offset` local. The image's
//            `lea ecx, [ebp + 0xc]` is `&region_src`, and its second read of
//            [ebp+0xc] at 0x0050DDB is the offset coming back out of
//            bitmask. (2) the call is `bitmask_call`, the real E8 - at /Ob2
//            the header `inline bitmask` expands at this site and the whole
//            body collapses to 24 instructions with no call at all.
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
    int mask;
    bitmask(region_src & RegionBounds, &region_src, &mask);
    return (Continents[region_dst].sea_coasts[region_src] & mask) != 0;
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
        bitmask(region_src & RegionBounds, &offset, &mask);
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
// ORIGINAL: 0x0050DE50 ?base_on_sea@@YAHHH@Z 0x0050DE50-0x0050DF28 BYTE_EXACT
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
        int x_radius = wrap_map_x(x + VehMoveDeltaX[i]);
        int y_radius = y + VehMoveDeltaY[i];
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
// ORIGINAL: 0x0050DF30 ?base_coast@@YAHH@Z 0x0050DF30-0x0050E021 BYTE_EXACT
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
    for (int i = 0; i < 8; i++) { // is_coast()
        int x_radius = wrap_map_x(x + VehMoveDeltaX[i]);
        int y_radius = y + VehMoveDeltaY[i];
        if (on_map(x_radius, y_radius) && is_ocean(x_radius, y_radius)) {
            region = region_at(x_radius, y_radius);
            int compare = Continents[region].tile_count;
            if (region >= 127) {
                compare = 1;
            }
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
// ORIGINAL: 0x0050E030 ?port_to_coast@@YAHHH@Z 0x0050E030-0x0050E15A BYTE_EXACT
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
    if (region == region_at(x, y)) {
        return true;
    }
    for (int i = 0; i < 8; i++) { // is_coast()
        int x_radius = wrap_map_x(x + VehMoveDeltaX[i]);
        int y_radius = y + VehMoveDeltaY[i];
        if (on_map(x_radius, y_radius) && is_ocean(x_radius, y_radius)) {
            // sea_coast() (0x0050DDC0, BYTE_EXACT as its own out-of-line
            // function) is hand-inlined here: the image writes its body out
            // at this call site (bitmask() is the only call it keeps),
            // rather than calling 0x0050DDC0.
            int offset;
            int mask;
            bitmask(region_at(x_radius, y_radius) & RegionBounds, &offset, &mask);
            if ((Continents[region].sea_coasts[offset] & mask) != 0) {
                return true;
            }
            i += (2 - (i & 1)); // skips adjacent tiles
        }
    }
    return false;
}

static __forceinline BOOL __cdecl port_to_port_body(int base_id_src, int base_id_dst) {
    int last_region = -1;
    int x = Bases[base_id_src].x;
    int y = Bases[base_id_src].y;
    uint32_t i = 0;
    for (;;) { // is_coast()
        int x_radius = wrap_map_x(x + VehMoveDeltaX[i]);
        int y_radius = y + VehMoveDeltaY[i];
        if (on_map(x_radius, y_radius) && is_ocean(x_radius, y_radius)) {
            int region_src = region_at(x_radius, y_radius);
            if (region_src != last_region) { // reduce redundant checks especially sea bases
                last_region = region_src;
                // Inlined base_on_sea(base_id_dst, region_src): the image
                // writes this out here rather than calling it.
                int region_sea = region_src & RegionBounds;
                if (region_sea < RegionBounds) {
                    int x_dst = Bases[base_id_dst].x;
                    int y_dst = Bases[base_id_dst].y;
                    for (uint32_t j = 0; j < 8; j++) {
                        int x_radius_dst = wrap_map_x(x_dst + VehMoveDeltaX[j]);
                        int y_radius_dst = y_dst + VehMoveDeltaY[j];
                        if (on_map(x_radius_dst, y_radius_dst) && is_ocean(x_radius_dst, y_radius_dst)
                            && (region_at(x_radius_dst, y_radius_dst) & RegionBounds) == region_sea) {
                            return true;
                        }
                    }
                }
            }
        }
        if (++i >= 8) {
            return false;
        }
    }
}

/*
Purpose: Check to see if two port bases share a common body of water determined by region.
// ORIGINAL: 0x0050E160 ?port_to_port@@YAHHH@Z 0x0050E160-0x0050E306 BYTE_EXACT
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
BOOL __cdecl port_to_port(int base_id_src, int base_id_dst) {
    return port_to_port_body(base_id_src, base_id_dst);
}

/*
Purpose: Determine if a base has access to ports or more than one coastal region. This helps
         prioritize whether naval transports should be built.
// ORIGINAL: 0x0050E310 ?transport_base@@YAHH@Z 0x0050E310-0x0050E3BD BYTE_EXACT
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
    int sea_coast_count = 0;
    for (int i = 1; i < RegionBounds; i++) {
        int offset;
        int mask;
        bitmask(region & RegionBounds, &offset, &mask);
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
    if (base_coast(base_id) < 0) {
        return false;
    }
    for (int i = 0; i < GameDataStateStorage.fields_.base_current_count_; i++) {
        if (Bases[i].faction_id_current != Bases[base_id].faction_id_current
            && port_to_port(base_id, i)) {
            return true;
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
    if (home_base_id < 0 || home_base_id == base_id) {
        return false;
    }
    uint8_t triad = get_triad(veh_id);
    if (triad == TRIAD_AIR) {
        return true;
    }
    int region_base = region_at(Bases[base_id].x, Bases[base_id].y);
    if (region_base == region_at(Bases[home_base_id].x, Bases[home_base_id].y)
        && ((triad == TRIAD_SEA) == (region_base >= 64))) {
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
// body      src/map.h
// size      29 bytes
// prototype 
// callers   1   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
Return Value: Is region bad? true/false
Status: Complete

/*
Purpose: Get the bit shifted (down) altitude of the specified tile.
// ORIGINAL: 0x00500150 ?alt_at@@YAHHH@Z 0x00500150-0x0050017B BYTE_EXACT
// body      src/map.h
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
// ORIGINAL: 0x00500180 ?alt_detail_at@@YAHHH@Z 0x00500180-0x005001A9 BYTE_EXACT
// body      src/map.h
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

/*
Purpose: Get the region of the specified tile.
// ORIGINAL: 0x00500220 ?region_at@@YAHHH@Z 0x00500220-0x00500249 BYTE_EXACT
// body      src/map.h
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

/*
Purpose: Get the bit of the specified tile.
// ORIGINAL: 0x005001B0 ?bit_at@@YAHHH@Z 0x005001B0-0x005001D5 BYTE_EXACT
// body      src/map.h
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
Purpose: Check whether the specified tile is part of an ocean.
// ORIGINAL: 0x005001E0 ?is_ocean@@YAHHH@Z 0x005001E0-0x00500211 BYTE_EXACT
// body      src/map.h
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
// body      src/map.h
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
Purpose: Check whether there is a sensor available in the specified tile.
// ORIGINAL: 0x005BF010 ?is_sensor@@YAHHH@Z 0x005BF010-0x005BF12D BYTE_EXACT
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
            int dist_y = y - Bases[base_id].y;
            if (!dist_y || (dist_y >= 0 ? dist_y : Bases[base_id].y - y) == 2) {
                if (has_fac(FAC_GEOSYNC_SURVEY_POD, base_id, 0)) {
                    return 2; // Geosynchronous Survey Pod
                }
            }
        }
    }
    return 0; // No sensor found
}


/*
Purpose: Check if faction controls the initial tile (code offset 0) of the Manifold Nexus.
// ORIGINAL: 0x005BF130 ?has_temple@@YAHH@Z 0x005BF130-0x005BF1EE BYTE_EXACT
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
// TRIED (2026-09-03): hoisting `int mask = 1 << faction_id` ahead of the
//            loop, dropping the `bit2` local in favour of two inline
//            accessor calls (51/70, worse), and swapping the `&` operands -
//            all still 65/70. The image keeps the tile index in eax across
//            the whose_territory call and puts the mask in edx; this tree
//            puts the index in esi and the mask in eax.
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

// timer_daemon reads this climate gate before dispatch; its wider contract remains unresolved.
// RECOVERY_OBJECT: v1 0x0068F21C ?WorldClimateSkipTerrainClear@@3IA
uint32_t WorldClimateSkipTerrainClear = 1;  // 0x0068F21C
// RECOVERY_OBJECT: v1 0x009B22EC ?WorldClimateSkipTerritoryReset@@3IA
uint32_t WorldClimateSkipTerritoryReset;  // 0x009B22EC
// The map-refresh request the regeneration steps honour. world_build raises
// it alongside WorldClimateSkipTerritoryReset while it is regenerating the
// terrain and clears it again immediately before world_climate; every
// world_* step between the two (`world_shorelines`, and world_build's own
// inner loops) reads it as "the visible map windows may be refreshed":
// `if ((GameState & 0x80) != 0 && WorldClimateRedrawRequest != 0)
//      console_map_win()->clear_terrain(), draw_map(1);`
uint32_t WorldClimateRedrawRequest;  // 0x009B22E8
// Original CRT table slot 0x00682604 selects initializer 0x004C38B0, which
// constructs this WorldWin and registers teardown 0x004C38D0. That teardown
// destroys the same receiver at 0x008E9F60. Its complete extent is not yet proven.

/*
Purpose: Check for any type of zone of control conflicts (base and/or unit).
// ORIGINAL: 0x005C89F0 ?zoc_any@@YAHHHH@Z 0x005C89F0-0x005C8AC0 BYTE_EXACT
// The image inlines anything_at (MEASURED inline in map.h); the `owner >= 0`
// is zoc_any's own test on its return.
// TRIED: same xrange/on_map-loop family ceiling as zoc_veh (0x005C8AC0,
//            right below) and base_on_sea. Remaining gap is one register
//            choice, twice: the image holds the hoisted `MapIsFlat & 1` in
//            ECX (spilled to [ebp-4]) and the radius temp in EAX, this tree
//            the other way round - measured, not chased. Hoisting the flat
//            test into a named local before the loop compiles to the same
//            12/82, so the hoist is not the difference either.
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
        int x_radius = wrap_map_x(x + VehMoveDeltaX[i]);
        int y_radius = y + VehMoveDeltaY[i];
        if (on_map(x_radius, y_radius)) {
            int owner = anything_at(x_radius, y_radius);
            if (owner >= 0 && (uint32_t)owner != faction_id
                && !(PlayersData[faction_id].diplo_treaties[owner] & DTREATY_PACT)) {
                return owner + 1;
            }
        }
    }
    return 0;
}

/*
Purpose: Check for unit related zone of control conflicts.
// ORIGINAL: 0x005C8AC0 ?zoc_veh@@YAHHHH@Z 0x005C8AC0-0x005C8B97 BYTE_EXACT
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
    int ret = 0;
    for (uint32_t i = 0; i < 8; i++) {
        int x_radius = wrap_map_x(x + VehMoveDeltaX[i]);
        int y_radius = y + VehMoveDeltaY[i];
        if (on_map(x_radius, y_radius)) {
            int owner = veh_who(x_radius, y_radius);
            if (owner >= 0 && owner != faction_id
                && !(PlayersData[faction_id].diplo_treaties[owner] & DTREATY_PACT)) {
                owner++;
                if (ret <= owner) {
                    ret = owner; // any point in continuing after 1st instance of ZOC is found?
                }
            }
        }
    }
    return ret;
}

/*
Purpose: Check for unit related zone of control conflicts taking into account land or ocean.
// ORIGINAL: 0x005C8BA0 ?zoc_sea@@YAHHHH@Z 0x005C8BA0-0x005C8D36 BYTE_EXACT
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
        int x_radius = wrap_map_x(x + VehMoveDeltaX[i]);
        int y_radius = y + VehMoveDeltaY[i];
        if (on_map(x_radius, y_radius)) {
            int owner = veh_who(x_radius, y_radius);
            if (owner >= 0 && (uint32_t)owner != faction_id
                && is_ocean(x_radius, y_radius) == is_ocean_tile
                && !(PlayersData[faction_id].diplo_treaties[owner] & DTREATY_PACT)) {
                for (int veh_id = veh_at(x_radius, y_radius); veh_id >= 0;
                    veh_id = Vehs[veh_id].next_veh_id_stack) {
                    if (Vehs[veh_id].faction_id != faction_id) {
                        if (Vehs[veh_id].visibility & (1 << faction_id)) {
                            return owner + 1;
                        }
                        if (!IsMultiplayerNet) {
                            const int16_t invisible =
                                static_cast<int16_t>(Vehs[veh_id].flags & VFLAG_INVISIBLE);
                            if (invisible == 0) {
                                return owner + 1;
                            }
                        }
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

// Homed from src/map_color.cpp (2026-09-02): its leaf-test isolation
// rationale died with the leaf suite, the same retirement that freed
// basepop_font.cpp. The mandate row belongs to TextColourTable[162..168].
/*
Purpose: Return the display colour for a mandate selector. Selectors 0, 1, and 2
         map to the first three table entries; any other value returns the
         fourth. The original decrements the selector twice against zero, so the
         branch structure is preserved for exactness.
// ORIGINAL: 0x0050E820 ?mandate_color@@YAHH@Z 0x0050E820-0x0050E84D BYTE_EXACT
// LEVER: a `switch`, not a chain of `if`s. The image's dispatch is `sub eax, 0 / je` then `dec eax / je` twice - VC6's small-switch lowering, which keeps the selector live in eax and lays the arms out in reverse after the chain. Three separate `if (mandate == N)` compare `[ebp+8]` against an immediate at every arm and scored 5 of 21; the switch scores 21 of 21.
// size      45 bytes
// prototype int (__cdecl ?mandate_color@@YAHH@Z)(int category)
// callers   9   call targets   0
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     (none)
Return Value: Colour dword from the mandate colour table
Status: Complete
*/
int __cdecl mandate_color(int mandate) {
    switch (mandate) {
    case 0:
        return TextColourTable[162];
    case 1:
        return TextColourTable[164];
    case 2:
        return TextColourTable[166];
    default:
        return TextColourTable[168];
    }
}



/*
Purpose: Rank the prototypes a faction can build, best first, into ProtoSortIds.
// ORIGINAL: 0x005A6270 ?proto_sort@@YAXH@Z 0x005A6270-0x005A63C9 FILE BYTE_EXACT
// size      345 bytes
// prototype void (__cdecl ?proto_sort@@YAXH@Z)(int factionID)
// callers   2   call targets   4
// calls     0x0057D360 0x0057D3F0 0x005B5690 0x005B9F20
// SYMBOL FACTS. ?proto_sort@@YAXH@Z is the catalogue's name; the two fixed
//        slots it shares with proto_sort_2 are bound here because nothing
//        else in the tree names them and the pair is one table - 0x00945B40
//        is how many entries the sort filled and 0x00945B44 the id list
//        itself, contiguous, which is why the image writes them 4 bytes
//        apart. The key's four terms are read straight off the layout:
//        Chassis[].triad (+0x49), Chassis[].speed (+0x48), and the
//        prototype's plan / chassis_id / armor_id / flags / preq_tech.
// symbol    ?proto_sort@@YAXH@Z
// TRIED: 0/109. `has_tech` is `MEASURED inline` in technology.h, so this body
//        INLINES it where the image calls it (136 compiled instructions
//        against 109); `has_tech_call` closes that (115 against 109). The
//        rest is the frame: the image keeps a frame pointer and allocates
//        0x204 for the 512-byte key array plus one dword, where this tree
//        allocates 0x20c and goes frame-less under both flag sets, so
//        everything from instruction 0 is shifted. Left as a compiling body.
Return Value: n/a
Status: Complete
*/
// RECOVERY_OBJECT: v2 0x00945B40 0x00945B44 ?ProtoSortCount@@3HA 0x005A63D0
int ProtoSortCount;
// RECOVERY_OBJECT: v2 0x00945B44 0x00945D44 ?ProtoSortIds@@3PAHA 0x005A63D0
int ProtoSortIds[128];

void __cdecl proto_sort(int faction_id) {
    int sort_keys[128];
    ProtoSortCount = 0;
    int i = 0;
    do {
        int proto_id = (i % 64) + (i >= MaxVehProtoFactionNum ? faction_id * 64 : 0);
        ProtoSortIds[i] = proto_id;
        int key;
        if (!(VehPrototypes[proto_id].flags & PROTO_ACTIVE)
            || (proto_id < MaxVehProtoFactionNum
                && !has_tech(VehPrototypes[proto_id].preq_tech, faction_id))) {
            if (proto_id >= MaxVehProtoFactionNum) {
                key = 0x34FB5E38;
                ProtoSortCount++;
            } else {
                key = 999999999;
            }
        } else {
            key = (Chassis[VehPrototypes[proto_id].chassis_id].triad
                   + VehPrototypes[proto_id].plan * 16) << 18;
            key += (32 - weap_val(proto_id, faction_id)) << 9;
            key += (8 - Chassis[VehPrototypes[proto_id].chassis_id].speed) << 6;
            key += 8 - arm_val(VehPrototypes[proto_id].armor_id, faction_id);
            ProtoSortCount++;
        }
        sort_keys[i] = key;
        i++;
    } while (i < 128);
    sort(128, ProtoSortIds, sort_keys);
}

// Order design choices by availability and obsolete status. Original IDs stay
// in the output table; the added status bands are sorting keys only.
// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x005A63D0 ?proto_sort_2@@YAXH@Z 0x005A63D0-0x005A650B BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["image:0x005A63D0-0x005A650B", "source:src/recovered/units/005a63d0.cpp", "source:src/faction.h:PlayerData.proto_id_active"], "reason": "Recover design-choice ordering with typed prototype, weapon and faction state and source-owned shared output; original IDs remain separate from status ranking keys."}
void __cdecl proto_sort_2(int faction_id) {
    ProtoSortCount = 0;
    int sort_keys[128];
    for (int index = 0; index < 128; ++index) {
        int prototype_id = (index >= MaxVehProtoFactionNum ? faction_id * MaxVehProtoFactionNum : 0)
            + index % MaxVehProtoFactionNum;
        ProtoSortIds[index] = prototype_id;
        if ((VehPrototypes[prototype_id].flags & PROTO_ACTIVE) != 0
            && (prototype_id >= MaxVehProtoFactionNum
                || (Weapon[VehPrototypes[prototype_id].weapon_id].offense_rating >= 0
                    && prototype_id != 15
                    && has_tech(VehPrototypes[prototype_id].preq_tech, faction_id)))) {
            if (VehPrototypes[prototype_id].obsolete_factions & (1 << faction_id)) {
                if (prototype_id >= MaxVehProtoFactionNum) {
                    prototype_id += PlayersData[faction_id].proto_id_active[prototype_id]
                        ? 0x800 : 0x1000;
                } else {
                    prototype_id += 0x2000;
                }
            }
            ++ProtoSortCount;
        } else {
            if (prototype_id >= MaxVehProtoFactionNum) {
                prototype_id += 0x400;
                ++ProtoSortCount;
            } else {
                prototype_id = 999999999;
            }
        }
        sort_keys[index] = prototype_id;
    }
    sort(128, ProtoSortIds, sort_keys);
}

extern int BaseAtDebugPopupFlag;


