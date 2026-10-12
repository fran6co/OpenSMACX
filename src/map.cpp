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
// ORIGINAL: 0x004C38B0 _$E21 0x004C38B0-0x004C38C6 FILE
// RECOVERY_BINDING: v1 {"evidence": ["Pinned CRT slot0x682604 selects original initializer0x4c38b0..0x4c38c6, registering fini0x4c38d0..0x4c38da; complete independent decode establishes exclusive boundaries.", "Source-owned object?WorldClimateWorldWin@@3VWorldWin@@A identifies original storage0x8e9f60. Startup arguments/calls: call 0x4c4bf0; push 0x4c38d0. Cleanup receiver/calls: mov ecx, 0x8e9f60; jmp 0x4c4a70. Constructor/destructor targets retain their existing class-owned source mappings.", "Fresh physical src/map.cpp COFF selects _$E21 through .CRT$XCU and registers _$E19, both referencing the genuine source global. Original NetMsg argument triples and implicit PlanWin/Console most-derived flags are preserved by the existing declarations; complete virtual-base teardown is compiler-owned, not authored offsets.", "Current local helper symbols interpret present emission, not original debug names or historical TU boundaries. Object starts do not prove allocation extents. This binding establishes neither cross-TU global order, masked-reference correctness, strict linked preservation, dependency-provider equivalence nor runtime/game acceptance; excluded guarded sprite cleanup remains separate."], "reason": "Bind original startup identity to the genuine compiler-generated lifetime of ?WorldClimateWorldWin@@3VWorldWin@@A in its actual source owner; no manual lifecycle callback is authored."}
// ORIGINAL: 0x004C38D0 _$E19 0x004C38D0-0x004C38DA FILE
// RECOVERY_BINDING: v1 {"evidence": ["Pinned CRT slot0x682604 selects original initializer0x4c38b0..0x4c38c6, registering fini0x4c38d0..0x4c38da; complete independent decode establishes exclusive boundaries.", "Source-owned object?WorldClimateWorldWin@@3VWorldWin@@A identifies original storage0x8e9f60. Startup arguments/calls: call 0x4c4bf0; push 0x4c38d0. Cleanup receiver/calls: mov ecx, 0x8e9f60; jmp 0x4c4a70. Constructor/destructor targets retain their existing class-owned source mappings.", "Fresh physical src/map.cpp COFF selects _$E21 through .CRT$XCU and registers _$E19, both referencing the genuine source global. Original NetMsg argument triples and implicit PlanWin/Console most-derived flags are preserved by the existing declarations; complete virtual-base teardown is compiler-owned, not authored offsets.", "Current local helper symbols interpret present emission, not original debug names or historical TU boundaries. Object starts do not prove allocation extents. This binding establishes neither cross-TU global order, masked-reference correctness, strict linked preservation, dependency-provider equivalence nor runtime/game acceptance; excluded guarded sprite cleanup remains separate."], "reason": "Bind original exit identity to the genuine compiler-generated lifetime of ?WorldClimateWorldWin@@3VWorldWin@@A in its actual source owner; no manual lifecycle callback is authored."}
#include "stdafx.h"
#include "popup.h"
#include "general.h"
#include "win.h"
#include "map.h"
#include "fractal.h"  // Fractal - world_erosion / world_fungus build one on the stack
#include "alpha.h"
#include "base.h"
#include "faction.h"
#include "game.h"
#include "interface_palette.h"
#include "path.h"
#include "random.h"
#include "strings.h"
#include "technology.h"
#include "veh.h"
#include "log.h"
#include "lock.h"     // map_tile_bytes() - site_at/set_dirty walk its records
#include "netdaemon.h"
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

// Shared landmark lookups belong to these source arrays, not the old process.
// Direction data has 49 independently identified rows of five bytes; keep
// the following three-byte image gap out of the object. Initializers remain
// pinned ignored configure-time derivatives, not tracked binary dumps.
// RECOVERY_OBJECT: v2 0x00685684 0x00685779 ?AltDirectionTable@@3QBEB 0x005927D0
extern const unsigned char AltDirectionTable[49 * 5] = {
#include "terrain_alt_direction.inc"
};
static_assert(sizeof(AltDirectionTable) == 245, "landmark direction span");

// Both the writable rows and immutable templates have 113-byte accessed
// source windows. Original upper allocation bounds are unproved; their v1
// bindings claim starts only. Writable initial windows are loader-zero.
// RECOVERY_OBJECT: v1 0x007D3B74 ?AltBonusArid@@3PAEA
unsigned char AltBonusArid[113];
// RECOVERY_OBJECT: v1 0x0068577C ?AltTemplateArid@@3QBEB
extern const unsigned char AltTemplateArid[113] = {
#include "terrain_landmark_arid.inc"
};
// RECOVERY_OBJECT: v1 0x007F65D0 ?AltBonusMoist@@3PAEA
unsigned char AltBonusMoist[113];
// RECOVERY_OBJECT: v1 0x006857F0 ?AltTemplateMoist@@3QBEB
extern const unsigned char AltTemplateMoist[113] = {
#include "terrain_landmark_moist.inc"
};
// RECOVERY_OBJECT: v1 0x007F655C ?AltBonusRainy@@3PAEA
unsigned char AltBonusRainy[113];
// RECOVERY_OBJECT: v1 0x0068593C ?AltTemplateRainy@@3QBEB
extern const unsigned char AltTemplateRainy[113] = {
#include "terrain_landmark_rainy.inc"
};
// RECOVERY_OBJECT: v1 0x007D3A78 ?AltBonusFlat@@3PAEA
unsigned char AltBonusFlat[113];
// RECOVERY_OBJECT: v1 0x00685864 ?AltTemplateFlat@@3QBEB
extern const unsigned char AltTemplateFlat[113] = {
#include "terrain_landmark_flat.inc"
};
// RECOVERY_OBJECT: v1 0x007F64E8 ?AltBonusOcean@@3PAEA
unsigned char AltBonusOcean[113];
// RECOVERY_OBJECT: v1 0x006859B0 ?AltTemplateOcean@@3QBEB
extern const unsigned char AltTemplateOcean[113] = {
#include "terrain_landmark_ocean.inc"
};

// world_build copies eleven words from AltNaturalDefault to AltNatural.
// world_rivers selects AltContour by the native altitude band. The three
// adjacent original tables are separate real arrays, not pointers into an
// old executable. Preserve the initial values in pinned ignored includes;
// recovered callers read and mutate these source-owned objects only.
// RECOVERY_OBJECT: v2 0x0068FAF4 0x0068FB20 ?AltNaturalDefault@@3PAIA 0x005C86E0
uint32_t AltNaturalDefault[11] = {
#include "terrain_altitude_default.inc"
};
// RECOVERY_OBJECT: v2 0x0068FB20 0x0068FB4C ?AltContour@@3PAIA 0x005C38B0
uint32_t AltContour[11] = {
#include "terrain_altitude_contour.inc"
};
// RECOVERY_OBJECT: v2 0x0068FB4C 0x0068FB78 ?AltNatural@@3PAIA 0x005C86E0
uint32_t AltNatural[11] = {
#include "terrain_altitude_natural.inc"
};
static_assert(sizeof(AltNaturalDefault) == 44, "default altitude table extent");
static_assert(sizeof(AltContour) == 44, "river contour table extent");
static_assert(sizeof(AltNatural) == 44, "mutable altitude table extent");

// The paired diagonal steps are also the farm raster's one-based lookup
// data at 0x66EF28/0x66EF3C. farm_get skips index zero and uses [index - 1].
// Keep the two measured four-dword windows independently source-owned.
// RECOVERY_OBJECT: v2 0x0066EF28 0x0066EF38 ?RiverStepX@@3QBHB 0x00409100
const int RiverStepX[4] = { 1, 1, -1, -1 };
// RECOVERY_OBJECT: v2 0x0066EF3C 0x0066EF4C ?RiverStepY@@3QBHB 0x00409100
const int RiverStepY[4] = { -1, 1, 1, -1 };

// Nine original direction indices, bounded by the next RadiusOffsetX
// table at 0x0066EFBC. MapWin's legend and tutorial navigation consume
// this permutation; it is independent data, not a callable/address alias.
// RECOVERY_OBJECT: v2 0x0066EF98 0x0066EFBC ?MapDirectionPermutation@@3QBHB 0x0046F8C0
const int MapDirectionPermutation[9] = { 9, 6, 3, 2, 1, 4, 7, 8, 5 };

// Loader-zero pointer cells at 94A30C/94A310. map_init allocates both
// arrays; map_shutdown releases them with the matching CRT free.
// Resampling retains its previous allocation and dimensions in these
// original scratch cells (945AD0/945AD4/945AD8).
// RECOVERY_OBJECT: v1 0x00945AD0 ?MapResamplePreviousTiles@@3PAUMap@@A
Map *MapResamplePreviousTiles;
// RECOVERY_OBJECT: v1 0x00945AD4 ?MapResamplePreviousWidth@@3HA
int MapResamplePreviousWidth;
// RECOVERY_OBJECT: v1 0x00945AD8 ?MapResamplePreviousHeight@@3HA
int MapResamplePreviousHeight;

// Metadata scalars formerly lived here; landmarks_ and file_path_ formerly
// lived in alpha.cpp. All had static zero initialization. Their exact native
// address/offset relationship is now declared in map.h in one real allocation.
// MapLongitude (68FAF0) remains an independently initialized row-stride object.
// RECOVERY_OBJECT: v3 0x0094A30C ?MapTiles@@3PAUMap@@A member ?MapStorage@@3UMapStorageWindow@@A tiles_
// RECOVERY_OBJECT: v3 0x0094A310 ?MapAbstractTiles@@3PAEA member ?MapStorage@@3UMapStorageWindow@@A abstract_tiles_
// RECOVERY_OBJECT: v3 0x00949870 ?MapLongitudeBounds@@3HA member ?MapStorage@@3UMapStorageWindow@@A metadata_.longitude_bounds_
// RECOVERY_OBJECT: v3 0x00949874 ?MapLatitudeBounds@@3HA member ?MapStorage@@3UMapStorageWindow@@A metadata_.latitude_bounds_
// RECOVERY_OBJECT: v3 0x00949884 ?MapArea@@3IA member ?MapStorage@@3UMapStorageWindow@@A metadata_.area_
// RECOVERY_OBJECT: v3 0x0094988C ?MapIsFlat@@3HA member ?MapStorage@@3UMapStorageWindow@@A metadata_.is_flat_
MapStorageWindow MapStorage;

// The tile-flag mask the fungus pass clears off the Map record it has just
// given bit 0x20 to: world_fungus does `tile->bit &= ~MapTileFlagClearMask`
// (0x005C35EB, `not edx` then `and [ecx + eax*4 + 8], edx`), and the same
// dword is the value argument at four more sites that set tiles through
// 0x00591D60 (0x004CE6D5 is one). Eight readers in the whole image, all of
// them reads, and the shipped .data holds 0x00288050 - the same constant the
// 0x004CE6C9 call site pushes as a literal plus 0xc. No image symbol and no
// name anywhere in the tree before this.
uint32_t MapTileFlagClearMask = 0x00288050;  // 0x006919EC, defined here
// EIGHT OF THEM, one per player: the two sites below clear
// `sizeof(int) * MaxPlayerNum` bytes, which is what the extent is.
int MapBaseSubmergedCount[8];  // 0x009B2290
int MapBaseIdClosestSubmergedVeh[8];  // 0x009B22BC
uint32_t BrushVal1;  // 0x009B22B0
uint32_t BrushVal2;  // 0x009B22B8
uint32_t WorldBuildVal1;  // 0x009B22B4

// RECOVERY_OBJECT: v1 0x0068FAF0 ?MapLongitude@@3IA
uint32_t MapLongitude = 1;  // half-width tile-row stride; original default is 1
LPCSTR MapExtension = "MP";

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
Purpose: Bounds check and handling of x coordinate for round maps.
// ORIGINAL: 0x0048BEE0 ?xrange@@YAHH@Z 0x0048BEE0-0x0048BF05 BYTE_EXACT
// body      src/map.h
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
        if (!ignore_comm && !(GameDataStateStorage.fields_.game_state_ & STATE_OMNISCIENT_VIEW)
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
        && !has_treaty(faction_id, owner, DTREATY_VENDETTA)) {
        return base_id;
    }
    return -1;
}

/*
Purpose: For the specified tile, calculate the quality of the terrain.
// ORIGINAL: 0x004ECB90 ?crappy@@YAHHH@Z 0x004ECB90-0x004ECBE1
// TRIED (2026-09-03): the two tile reads as `uint8_t` locals (the image's
//            `and dl,0x18` / `and al,0xc0` are BYTE forms, and a `uint32_t`
//            local turns them into dword `and edx,0x18` / `and ecx,0xc0`) -
//            kept, though the agreeing count does not move. Also measured:
//            `int` poor_quality, both reads up front, `register`, explicit
//            `poor_quality = poor_quality + 1`, and binding `poor_quality`
//            from the first comparison - all 4/32 or worse. The image holds
//            `poor_quality` in ESI (`push esi` in the prologue) with the
//            rainfall byte in dl and the rocky byte in al; this tree keeps
//            it in eax and cannot be pushed onto the callee-saved register.
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
    bitmask_call(region_src & RegionBounds, &region_src, &mask);
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
        int x_radius = xrange(x + VehMoveDeltaX[i]);
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
        int x_radius = xrange(x + VehMoveDeltaX[i]);
        int y_radius = y + VehMoveDeltaY[i];
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
        int x_radius = xrange(x + VehMoveDeltaX[i]);
        int y_radius = y + VehMoveDeltaY[i];
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
// body      src/map.h
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
    if (base_coast(base_id) < 0 || GameDataStateStorage.fields_.base_current_count_ <= 0) {
        return false;
    }
    uint32_t faction_id = Bases[base_id].faction_id_current;
    for (int i = 0; i < GameDataStateStorage.fields_.base_current_count_; i++) {
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
// body      src/map.h
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
Purpose: Return the first neighboring tile index that satisfies the coast/border checks, or 0.
         Earlier recovery prose treated the point-B comparison and integer result as unnecessary;
         the pinned function reads both regions and returns the successful neighbor index.
// ORIGINAL: 0x0056B480 ?coast_or_border@@YAHHHHHH@Z 0x0056B480-0x0056B5A9
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
    if ((int)faction_id != whose_territory(faction_id, x_point_a, y_point_a, NULL, false)) {
        return 0; // faction doesn't control point A
    }
    uint32_t region_b = region_at(x_point_b, y_point_b);
    for (int i = 1; i <= 8; i++) {
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
// RECOVERY_BINDING: v1 {"reason":"Read the genuine shared contour target table rather than a header-local copy of its initial values.","evidence":["Pinned5919D6 loads68FB2C, AltContour[3];591A13 compares against68FB20+4*(climate>>5) with signed591A1A JLE.","src/map.cpp owns AltContour at68FB20..68FB4C; the former ElevDetail header array was duplicate storage, not an independent original object."]}
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
// TRIED (2026-09-03): `contour - MapSeaLevel - ElevDetail[3]` operand order,
//            a `int sea = MapSeaLevel` local used for both subterms, and
//            computing `elev` inside each branch - all exactly 57/67, the
//            load's slot does not follow from any of them.
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
    int elev = 50 * (contour - AltContour[3] - MapSeaLevel);
    if (contour > (int)AltContour[tile->climate >> 5]) {
        elev += (x * 113 + y * 217 + MapSeaLevel * 301) % 50;
    } else {
        elev += 10;
    }
    return range(elev, -3000, 3500);
}

/*
Purpose: Calculate the natural altitude of the specified tile.
// ORIGINAL: 0x005918A0 ?alt_natural@@YAHHH@Z 0x005918A0-0x005918EF
// body      src/map.h
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
// ORIGINAL: 0x00500180 ?alt_detail_at@@YAHHH@Z 0x00500180-0x005001A9
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
*/
// BODY IN map.h, as `MEASURED inline`: the image writes it out at
// some call sites and calls it at others, and a .cpp definition is only ever
// one of those. The marker stays here because that is where the catalogue
// reads it.


/*
Purpose: Set the altitude details for the specified tile.
// ORIGINAL: 0x00591260 ?alt_put_detail@@YAXHHH@Z 0x00591260-0x00591288 SEMANTIC
// body      src/map.h
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
*/
// BODY IN map.h, as `MEASURED inline`: the image writes it out at
// some call sites and calls it at others, and a .cpp definition is only ever
// one of those. The marker stays here because that is where the catalogue
// reads it.


/*
Purpose: Set the region for the specified tile.
// ORIGINAL: 0x00591B90 ?region_set@@YAXHHH@Z 0x00591B90-0x00591BB8 SEMANTIC
// body      src/map.h
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
// body      src/map.h
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
//            something that costs more elsewhere. Left storing `val3` immediately. Also tried a
//            `uint32_t bit2 = tile->bit2;` local read BEFORE the `val3` store, OR'd and stored
//            AFTER (matching the image's load-then-store-val3-then-or-store-bit2 order exactly) -
//            forced a fourth callee-saved register (`push esi`), which shifts every subsequent
//            offset and drops it to 8/25. Confirms the plateau is a spill-slot ceiling, not order.
// TRIED (2026-09-03): `tile->bit2 |= 0x400000` written BEFORE the `val3`
//            store (16/25); `uint32_t bit2 = tile->bit2 | 0x400000` local,
//            stored after (15/25); `tile->bit2 = tile->bit2 | 0x400000` then
//            the `val3` store (15/25). The image's 2-instruction interleave
//            (bit2 load+or, val3 store, bit2 store) is VC6 scheduling a load
//            above an independent store, not a source order this tree can
//            name. Left at 22/25.
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
Purpose: Set the bit for the specified tile.
// ORIGINAL: 0x00591D30 ?bit_put@@YAXHHH@Z 0x00591D30-0x00591D58 SEMANTIC
// body      src/map.h
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
// body      src/map.h
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
Purpose: Name the faction working a tile that this faction may enter by treaty.
// ORIGINAL: 0x00591E90 sub_591e90 0x00591E90-0x00591EF9 FILE BYTE_EXACT
// symbol    ?treaty_using_faction@@YAHHHH@Z
// NO IMAGE CALLER AND NO IMAGE REFERENCE AT ALL (no E8 to it, no 4-byte
//        image of its address anywhere in the file), so the name is derived
//        from the body, not from a catalogue entry: it returns the tile's
//        `using` faction when the two are not the same faction, the tile is
//        land, and the treaty word says TREATY without PACT - the "right of
//        passage through an ally's territory" test. Everything it reads was
//        already named: `val3 & 7` is using_at(), the altitude nibble of
//        `climate` is the ALT_BIT_* ladder, and the per-faction word is
//        PlayersData[].diplo_treaties[] - 0x96C9E0 + faction * 0x20CC + the
//        subject * 4, which is the same arithmetic spying_recovery.h spells
//        `SpyingStatusTable[faction * 2099 + subject]`.
// LEVER: (1) ONE exit. The image's five failures all jump to a shared
//        `or eax,-1`, so the body is a nest whose arms fall through to a
//        single `return -1` - a `||` chain of early returns gives five
//        separate epilogues. (2) `& ~0x1F`, not `& 0xE0` - the complement
//        stops VC6 narrowing the whole test to `and dl,0xe0 / cmp dl,0x60 /
//        jb`, which is where `xor edx,edx / mov dl,[eax] / and edx,
//        0xffffffE0 / cmp edx,0x60 / jl` comes from (game.cpp lever 4,
//        measured again here). 3/39 -> BYTE_EXACT 39/39.
// size      105 bytes
// prototype
// callers   0   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
Return Value: The using faction id, or -1
Status: Complete
*/
int __cdecl treaty_using_faction(int x, int y, int faction_id) {
    if (faction_id > 0) {
        Map *tile = map_loc(x, y);
        if ((tile->climate & ~0x1F) >= ALT_BIT_SHORE_LINE) {
            int using_id = tile->val3 & 7;
            if (using_id > 0 && using_id != faction_id) {
                uint32_t treaty = PlayersData[faction_id].diplo_treaties[using_id];
                if (!(treaty & DTREATY_PACT) && (treaty & DTREATY_TREATY)) {
                    return using_id;
                }
            }
        }
    }
    return -1;
}


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
// body      src/map.h
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
Purpose: Return 1 immediately for a unity-bonus tile. Otherwise select the
         last pact or own base in the eight-neighbor movement table, stopping
         early on an own base. Return 1 if that base has an adjacent tile
         other than the input whose altitude is below shore and which either
         lacks fungus or lies below the ocean-shelf altitude; else return 0.

// ORIGINAL: 0x00565160 sub_565160 0x00565160-0x0056531A FILE
// symbol    _sub_565160
// TRIED (seed): two-phase radius-table scan (find owning base, then scan
//            around it) with direct treaty-flags array indexing; reaches #11
//            before the second bonus_at-style helper call ordering
// size      442 bytes
// prototype
// callers   0   call targets   2
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x004E3A50 0x00592030
Return Value: 1 for the bonus or a qualifying neighbor, 0 otherwise.
Status: Coverage transcription - unclaimed body. The original mapping and
        seed evidence remain in git:65b567af55eb22d163bbd28afcc335c340847278:src/unrecovered/00565160.cpp without a
        BYTE_EXACT claim. VehMoveDeltaX/Y provide the scan order, Bases
        provide the x/y/owner triple, PlayersData owns the pact flags, and
        Map owns the climate and fungus bits. The image calls bonus_at out
        of line; source inlining remains a separate measurement question.
*/
extern "C" int __cdecl sub_565160(int faction_id, int tile_x, int tile_y) {
    if (bonus_at(tile_x, tile_y, 0) == 3) {
        return 1;
    }
    int found = -1;
    unsigned int owner = 0;
    for (int idx = 0; idx < 8; ++idx) {
        int x = VehMoveDeltaX[idx] + tile_x;
        if ((MapIsFlat & 1) == 0) {
            if (x < 0) {
                x += MapLongitudeBounds;
            } else if (x >= MapLongitudeBounds) {
                x -= MapLongitudeBounds;
            }
        }
        int y = VehMoveDeltaY[idx] + tile_y;
        if (y < 0 || y >= MapLatitudeBounds || x < 0 || x >= MapLongitudeBounds) {
            continue;
        }
        int base_id = base_at(x, y);
        if (base_id < 0) {
            continue;
        }
        owner = Bases[base_id].faction_id_current;
        if (owner != static_cast<unsigned int>(faction_id)) {
            if ((PlayersData[faction_id].diplo_treaties[owner] & DTREATY_PACT) == 0) {
                continue;
            }
        }
        found = base_id;
        if (owner == static_cast<unsigned int>(faction_id)) {
            break;
        }
    }
    if (found < 0) {
        return 0;
    }
    short base_x = Bases[found].x;
    short base_y = Bases[found].y;
    int no_wrap = MapIsFlat & 1;
    for (int g = 0; g < 8; ++g) {
        int x = VehMoveDeltaX[g] + base_x;
        if (no_wrap == 0) {
            if (x < 0) {
                x += MapLongitudeBounds;
            } else if (x >= MapLongitudeBounds) {
                x -= MapLongitudeBounds;
            }
        }
        int y = VehMoveDeltaY[g] + base_y;
        if (x == tile_x && y == tile_y) {
            continue;
        }
        if (y < 0 || y >= MapLatitudeBounds || x < 0 || x >= MapLongitudeBounds) {
            continue;
        }
        const Map *tile = map_tiles() + MapLongitude * y + (x >> 1);
        unsigned char altitude = tile->climate & 0xe0;
        if (altitude < ALT_BIT_SHORE_LINE) {
            if ((tile->bit & BIT_FUNGUS) == 0) {
                return 1;
            }
            if (altitude < ALT_BIT_OCEAN_SHELF) {
                return 1;
            }
        }
    }
    return 0;
}

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
    if (GameDataStateStorage.fields_.game_rules_ & RULES_NO_UNITY_SCATTERING) {
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
Purpose: Plant fungus on a tile and take its radius away from every owner.
// ORIGINAL: 0x00592480 sub_592480 0x00592480-0x00592550 FILE BYTE_EXACT
// symbol    ?fungus_radius@@YAXHH@Z
// NO IMAGE CALLER AND NO IMAGE REFERENCE AT ALL, so the name is derived from
//        the body, not from a catalogue entry: the centre tile gains
//        BIT_FUNGUS and loses MINE | SOLAR_TIDAL | FOREST | CONDENSER |
//        ECH_MIRROR | THERMAL_BORE (0x01E00050 - the terraforms fungus
//        cannot coexist with), and then the same
//        21-tile walk site_radius() does clears each tile's owner nibble.
// size      208 bytes
// prototype
// callers   0   call targets   0
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl fungus_radius(int x, int y) {
    map_loc(x, y)->bit |= BIT_FUNGUS;
    map_loc(x, y)->bit &= ~(BIT_MINE | BIT_SOLAR_TIDAL | BIT_FOREST
                            | BIT_CONDENSER | BIT_ECH_MIRROR
                            | BIT_THERMAL_BORE);
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
// body      src/map.h
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
// ORIGINAL: 0x00592600 ?new_landmark@@YAHHHPAD@Z 0x00592600-0x0059264B BYTE_EXACT
// LEVER (2026-09-03): two fixes, 3/28 -> BYTE_EXACT 28/28. (1) The guard
//            tests the GLOBAL, and the local is only bound afterwards -
//            `int landmark_offset = MapLandmarkCount++`, not a local read
//            hoisted above the guard. Reading the count into a local first
//            made VC6 push esi ahead of the early return and run the guard
//            in it. (2) The three writes index `MapLandmark[landmark_offset]`
//            directly instead of going through a `Landmark *lm`: the image
//            folds the array base into each access's own displacement
//            (`mov [eax + 0x949894], ecx` / `lea edx, [eax + 0x94989c]`),
//            where a pointer materialises base + offset once and pays
//            `add eax, 8` for the name. Mirror of lever 7 - here the
//            array form is what the image has, not the pointer form.
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
    if (MapLandmarkCount >= MaxLandmarkNum) {
        return -1;
    }
    int landmark_offset = MapLandmarkCount++;
    MapLandmark[landmark_offset].x = x;
    MapLandmark[landmark_offset].y = y;
    strcpy(MapLandmark[landmark_offset].name, name);
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
Return Value: Neighbor index 1..8 or 1..20, or 0 if no qualifying ocean neighbor is found.
Status: Complete
*/
// Current restored contract: return the qualifying neighbor's index (1..8 or 1..20), or 0.
int __cdecl is_coast(int x, int y, BOOL is_base_radius) {
    int radius = is_base_radius ? 21 : 9;
    int i = 1;
    if (i >= radius) {
        return 0;
    }
    int is_flat = MapIsFlat & 1;
    for (; i < radius; i++) {
        int x_radius = x + RadiusOffsetX[i];
        if (!is_flat) {
            if (x_radius < 0) {
                x_radius += MapLongitudeBounds;
            } else if (x_radius >= (int)MapLongitudeBounds) {
                x_radius -= MapLongitudeBounds;
            }
        }
        int y_radius = y + RadiusOffsetY[i];
        if (on_map(x_radius, y_radius) && is_ocean(x_radius, y_radius)) {
            return i;
        }
    }
    return 0;
}

/*
Purpose: Check whether the specified tile is part of an ocean.
// ORIGINAL: 0x005001E0 ?is_ocean@@YAHHH@Z 0x005001E0-0x00500211
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
            for (int veh_id = 0; veh_id < GameDataStateStorage.fields_.vehicle_current_count_; veh_id++) {
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
            for (int base_id = 0; base_id < GameDataStateStorage.fields_.base_current_count_; base_id++) {
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
// body      src/map.h
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
// body      src/map.h
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
// body      src/map.h
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
// (src/recovered/0064603f.cpp - deleted 2026-09-03 with the CRT-range
// demotions - at 0x0064603F, a CRT-internal
// `__lock_file`/`_fwrite`/`__unlock_file` wrapper, which was BYTE_EXACT
// there but NOT a build input) rather than the public `fwrite()`. Neither
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
    if (!fwrite(&MapStorage, 0xAA4, 1, map_file)) {
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
    if (!fread(&MapStorage, 0xAA4, 1, map_file)) {
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

// Superseded seed deleted; seed paths below are historical Git provenance.
// Historical seed observations (not fresh measurements or acceptance):
// TRIED: single cached srcIndex/width/height locals across the record copy loop cause /O2 to spill to a `sub esp` stack frame where the original keeps everything in registers and the [ebp+8]/ [ebp+0xc] parameter slots as scratch; first divergence is the prologue itself (mov ebp,esp vs sub esp,N). Semantics verified against the Ghidra decompile and disasm field-by-field.
// ORIGINAL: 0x00592B40 sub_592b40 0x00592B40-0x00592D6F
// RECOVERY_BINDING: v1 {"evidence": ["source:src/unrecovered/00592b40.cpp: canonical full559byte original span and retained TRIED evidence", "source:src/map.cpp: existing map_init allocation/failure and map read/write lifetime", "source:recovery/tasks/map-allocation-owner-closure.json: coordinator-reviewed producer/reader closure"], "reason": "Original592B40 is the map allocation owners shrink/resample producer: it saves prior allocation/dimensions, obtains new map storage through map_init, copies terrain fields and releases old tiles. Source-owned pointer cells preserve actual allocation and consumer relationships."}
// symbol    ?map_resample@@YAXHH@Z
void __cdecl map_resample(int width, int height) {
    int old_width = MapLongitudeBounds;
    int old_height = MapLatitudeBounds;
    MapResamplePreviousWidth = old_width;
    MapResamplePreviousHeight = old_height;
    if (width >= old_width || height >= old_height) {
        return;
    }
    Map *old_tiles = map_tiles();
    MapResamplePreviousTiles = old_tiles;
    map_tiles() = 0;
    MapLongitudeBounds = width;
    MapLatitudeBounds = height;
    // The original does not release the previous abstract allocation here,
    // nor test map_init's result. Preserve that lifecycle and failure path.
    map_init();
    MapSeaLevelCouncil = 0;
    MapLandmarkCount = 0;
    MapSeaLevel = 0;
    MapRandSeed = random(0, 0x7fff) + 1;
    Map *new_tiles = map_tiles();
    for (int i = 0; i < static_cast<int>(MapArea); ++i) {
        Map *tile = new_tiles + i;
        tile->climate = ALT_BIT_OCEAN;
        tile->contour = 20;
        tile->val2 = 15;
        tile->region = 0;
        tile->visibility = 0;
        tile->bit = 0;
        tile->val3 = 0;
        ZeroMemory(&tile->bit2, sizeof(tile->bit2) + sizeof(tile->bit_visible));
    }
    for (int y = 0; y < MapLatitudeBounds; ++y) {
        for (int x = y & 1; x < MapLongitudeBounds; x += 2) {
            int source_y = MapResamplePreviousHeight * y / MapLatitudeBounds;
            int source_x = (MapResamplePreviousWidth * x / MapLongitudeBounds) >> 1;
            Map *destination = new_tiles + MapLongitude * y + (x >> 1);
            Map *source = old_tiles + (old_width >> 1) * source_y + source_x;
            destination->contour = source->contour;
            destination->bit = source->bit;
            destination->bit2 = source->bit2;
            destination->val3 = source->val3 & 0xc0;
        }
    }
    if (old_tiles) {
        free(old_tiles);
    }
    // The scratch pointer deliberately remains stale after free, as original.
    world_climate();
}

/*
Purpose: Get the region value for the specified tile.
// ORIGINAL: 0x00591210 ?abstract_at@@YAHHH@Z 0x00591210-0x00591230 SEMANTIC
// body      src/map.h
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
// body      src/map.h
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
        int x_radius = xrange(x_src + VehMoveDeltaX[i]);
        int y_radius = y_src + VehMoveDeltaY[i];
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
// body      src/map.h
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
// RECOVERY_BINDING: v1 {"reason":"Use the original full longitude bound for signed half-width thresholds and wrap adjustments, not the half-width tile-row stride.","evidence":["Pinned5A663A loads949870 MapLongitudeBounds into ESI; subsequent thresholds halve this register and wrapping adds/subtracts its unhalved value.","MapLongitude is the distinct68FAF0 packed-row stride; the historical LEVER below incorrectly interpreted the masked relocation target."]}
// Historical LEVER (superseded wrong-global interpretation): this body had been copy-adapted from radius_move's wraparound test (compare against MapLongitude, adjust by MapLongitudeBounds), but the image never loads MapLongitudeBounds here at all - it computes `MapLongitude / 2` (a signed `cdq; sub; sar` halving, done twice) as the threshold AND adjusts by the un-halved `MapLongitude` itself. Matching that literally (`x_radius_off < -(ML/2)` / `+= ML`, and the mirrored upper bound) is BYTE_EXACT.
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
    if (x_radius_off < -((int)MapLongitudeBounds / 2)) {
        x_radius_off += MapLongitudeBounds;
    }
    if (x_radius_off > ((int)MapLongitudeBounds / 2)) {
        x_radius_off -= MapLongitudeBounds;
    }
    int y_radius_off = y_dst - y_src;
    int direction_x = (x_radius_off > 0) ? 1 : (x_radius_off >= 0) - 1;
    int direction_y = (y_radius_off > 0) ? 1 : (y_radius_off >= 0) - 1;
    for (int i = 0; i < 9; i++) {
        if (direction_x == (VehMoveDeltaX[i] > 0 ? 1 : (VehMoveDeltaX[i] >= 0) - 1)
            && direction_y == (VehMoveDeltaY[i] > 0 ? 1 : (VehMoveDeltaY[i] >= 0) - 1)) {
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
ALT_BIT_OCEAN_SHELF or above the faction must hold Rules.tech_improve_fungus_sqr - the same
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
        && !has_tech(Rules.tech_improve_fungus_sqr, faction_id)) {
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
    int i, alt;
    // RadiusRange[0..1] is [1,9): the eight nearest neighbors, not
    // [9,25). Original 5C20A9/5C2221 reads one entry before its bound.
    for (i = 1, alt = altitude - 1; i < altitude; i++, alt--) {
        has_set_alt = false;
        for (int j = RadiusRange[i - 1]; j < RadiusRange[i]; j++) {
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
        for (int j = RadiusRange[i - 1]; j < RadiusRange[i]; j++) {
            int x_radius = xrange(x + RadiusOffsetX[j]);
            int y_radius = y + RadiusOffsetY[j];
            // Original 5C22AE/5C22B0 skips existing heights <= this cap.
            // This ring lowers high neighbors; it must not raise low ones.
            if (on_map(x_radius, y_radius) && alt_at(x_radius, y_radius) > alt) {
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
                    int x_rad_base = xrange(x_radius + VehMoveDeltaX[j]);
                    int y_rad_base = y_radius + VehMoveDeltaY[j];
                    if (on_map(x_rad_base, y_rad_base) && !is_ocean(x_rad_base, y_rad_base)
                        && !region_at(x_rad_base, y_rad_base)) {
                        skip = true;
                    }
                }
                if (!skip) {
                    if (altitude != 3) {
                        for (uint32_t j = 0, k = 0; j < 7; j += 3, k += 2) {
                            int x_rad_base = xrange(x_radius + VehMoveDeltaX[k + 1]);
                            int y_rad_base = y_radius + VehMoveDeltaY[k + 1];
                            if (on_map(x_rad_base, y_rad_base)
                                && altitude_at(x_rad_base, y_rad_base) == ALT_BIT_SHORE_LINE) {
                                int x_rad_base2 = xrange(x_radius + VehMoveDeltaX[(k + 2) % 8]);
                                int y_rad_base2 = y_radius + VehMoveDeltaY[(k + 2) % 8];
                                int x_rad_base3 = xrange(x_radius + VehMoveDeltaX[k % 8]);
                                int y_rad_base3 = y_radius + VehMoveDeltaY[k % 8];
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
                        if (GameDataStateStorage.fields_.game_state_ & STATE_OMNISCIENT_VIEW && altitude > 3) {
                            temp_set(x_radius, y_radius, 1);
                        }
                        BrushVal1 += 1;
                        BrushVal2 += 1;
                    }
                }
            }
        }
    }
    if (GameDataStateStorage.fields_.game_state_ & STATE_OMNISCIENT_VIEW && use_draw_radius) {
        draw_radius(x, y, 2, 2);
    }
}

/*
Purpose: Paint land to assist in the creation of the world terrain.
// ORIGINAL: 0x005C27F0 ?paint_land@@YAXHHHH@Z 0x005C27F0-0x005C28E6
// LEVER: two fixes, 0.849 -> 0.904 similar (9/89). (1) `x_rad_base`/
//        `y_rad_base` are a CUMULATIVE random walk - the image's inlined
//        `xrange()` adds `VehMoveDeltaX[offset]` to the PREVIOUS `esi`
//        (`x_rad_base`), and `edi` (`y_rad_base`) is a plain running
//        `add edi, VehMoveDeltaY[offset]` with no reset - not an offset from
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
                // Pinned5C2841 keeps the low dword of IMUL, then5C284C
                // uses signed JLE on that wrapped product plus two. Large
                // positive file-loaded settings survive world_build's test.
                uint32_t coverage = static_cast<uint32_t>(MapLandCoverage);
                uint32_t square = coverage * coverage;
                uint32_t threshold_bits = square + 2u;
                int threshold;
                memcpy(&threshold, &threshold_bits, sizeof(threshold));
                ++unk_val;
                int count;
                memcpy(&count, &unk_val, sizeof(count));
                if (count > threshold) {
                    // Original5C284E..2854 is low-dword SUB/LEA and AND7.
                    offset = (static_cast<uint32_t>(offset) - square
                        + unk_val - 2u) & 7u;
                }
            } else {
                search = offset;
                unk_val = 0;
            }
            x_rad_base = xrange(x_rad_base + VehMoveDeltaX[offset]);
            y_rad_base += VehMoveDeltaY[offset];
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
    int radius = WorldBuilder.continent_mod * coverage * coverage + WorldBuilder.continent_base;
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
        if (ratio > WorldBuilder.cont_size_ratio5 || BrushVal1 >= WorldBuildVal1) {
            radius /= 4;
            if (BrushVal1 >= WorldBuildVal1 && ratio > WorldBuilder.cont_size_ratio5) {
                radius = rand() % 3 + 1;
            }
        } else if (ratio <= WorldBuilder.cont_size_ratio4) {
            if (ratio <= WorldBuilder.cont_size_ratio3) {
                if (ratio <= WorldBuilder.cont_size_ratio2) {
                    if (ratio < WorldBuilder.cont_size_ratio1 && MapLandCoverage > 1) {
                        radius += radius / 2;
                    }
                } else {
                    uint32_t r_bound_bits = 4u - static_cast<uint32_t>(MapLandCoverage);
                    int r_bound;
                    memcpy(&r_bound, &r_bound_bits, sizeof(r_bound));
                    // Original5C2A9D LEA wraps before TEST/JLE5C2AA0/2.
                    uint32_t bound_test_bits = r_bound_bits - 1u;
                    int bound_test;
                    memcpy(&bound_test, &bound_test_bits, sizeof(bound_test));
                    if ((bound_test > 0) ? rand() % r_bound : 0) {
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
        int plat_mod = WorldBuilder.plateau_mod * (2u - static_cast<uint32_t>(MapOceanCoverage));
        if (altitude != ALT_1_LEVEL_ABOVE_SEA) {
            plat_mod /= 4;
        }
        uint32_t plat_base = WorldBuilder.plateau_base;
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
    // Native5C3697..36B6 retains this loaded area for the signed clearing
    // loop and the low-dword target product; a high-bit area skips clearing.
    uint32_t area_bits = MapArea;
    int area;
    memcpy(&area, &area_bits, sizeof(area));
    int i;
    for (i = 0; i < area; i++) {
        tile->bit &= ~(BIT_RIVERBED);
        tile++;
    }
    int riverbed_count = 0;
    // Pinned5C36C4..36FF: low-dword multiply/add/subtract, signed /3,
    // another low-dword multiply by the retained area, then signed /3200.
    // Unsigned builder storage must not turn these two quotients unsigned.
    uint32_t rain_bits = WorldBuilder.rivers_base
        + static_cast<uint32_t>(MapCloudCover) * WorldBuilder.rivers_rain_mod;
    uint32_t target_bits = (4u - static_cast<uint32_t>(MapOceanCoverage)) * rain_bits;
    int target;
    memcpy(&target, &target_bits, sizeof(target));
    uint32_t scaled_bits = area_bits * static_cast<uint32_t>(target / 3);
    int scaled;
    memcpy(&scaled, &scaled_bits, sizeof(scaled));
    int max_riverbeds = scaled / 3200;
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
    Paths.continents();
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
//   WorldBuilder.solar_energy` (and the three siblings) is `int / uint32_t`,
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
    int temp_heat = MapLatitudeBounds / (int)WorldBuilder.solar_energy;
    int thermal_banding = MapLatitudeBounds / (int)WorldBuilder.thermal_band;
    int thermal_deviance = MapLatitudeBounds / (int)WorldBuilder.thermal_deviance;
    int global_warming = MapLatitudeBounds / (int)WorldBuilder.global_warming;
    for (int y = 0; y < MapLatitudeBounds; y++) {
        for (int x = y & 1; x < MapLongitudeBounds; x += 2) {
            if ((bit2_at(x, y) & (BIT2_UNK_80000000 | BIT2_CRATER)) != BIT2_CRATER
                || code_at(x, y) >= 21) {
                int rand_seed = random(0, thermal_deviance * 2);
                // The native load follows the first random call; INC423C
                // wraps even when a file-loaded orbit is INT_MAX.
                uint32_t orbit_bound_bits = static_cast<uint32_t>(MapPlanetaryOrbit) + 1u;
                int orbit_bound;
                memcpy(&orbit_bound, &orbit_bound_bits, sizeof(orbit_bound));
                int rand_orbit = random(0, orbit_bound);
                int val1 = (MapLatitudeBounds / 2) - rand_seed - y + thermal_deviance;
                if (val1 < 0) { // abs?
                    val1 = rand_seed - (MapLatitudeBounds / 2) - thermal_deviance + y;
                }
                // Reload the orbit after random as at426E. The products and
                // SUB/LEA4279..4290 keep low dwords; IDIV4294 is signed.
                uint32_t orbit_heat_bits = static_cast<uint32_t>(temp_heat)
                    * (static_cast<uint32_t>(MapPlanetaryOrbit) - 1u);
                uint32_t council_heat_bits = static_cast<uint32_t>(MapSeaLevelCouncil)
                    * static_cast<uint32_t>(global_warming);
                uint32_t band_bits = static_cast<uint32_t>(thermal_banding / 2)
                    + (static_cast<uint32_t>(val1) - orbit_heat_bits - council_heat_bits) * 2u;
                int band;
                memcpy(&band, &band_bits, sizeof(band));
                int val2 = band / thermal_banding;
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
    if (GameDataStateStorage.fields_.turn_current_num_ > 150) {
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
uint32_t WorldClimateSkipTerrainClear = 1;  // 0x0068F21C
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
// RECOVERY_OBJECT: v1 0x008E9F60 ?WorldClimateWorldWin@@3VWorldWin@@A
WorldWin WorldClimateWorldWin;

// veh_init (0x005C03D0) is still an archived body with no product definition,
// and world_fungus is its only caller in this file - the same handoff shape
// console.cpp carries at its head. Its forwarder is in pending_bodies.cpp.
int __cdecl veh_init(int unit_type, int faction_id, int x_coord, int y_coord);

/*
Purpose: Fill the fractal's raw 19x19 field with random values, copy it into
         the smooth field the sampler reads, and wrap both fields' edges.
// ORIGINAL: 0x005C1E20 ?init@Fractal@@QAEXXZ 0x005C1E20-0x005C1F12 FILE
// symbol    ?init@Fractal@@QAEXXZ
// size      242 bytes
// prototype void (__thiscall ?init@Fractal@@QAEXXZ)(Fractal* this)
// callers   2   call targets   1
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x0064601D  (the C library rand)
// BUG IN THE ORIGINAL: the weighted pass in the middle sums each grid_[1]
// byte times 4 against eight RadiusOffsetX neighbours of grid_[1] and then
// throws the sum away - `mov esi, [ebp-4]` right after the inner loop
// discards it and reloads the row pointer before the store, which stores the
// byte it just read. Kept rather than eliminated: VC6 did not eliminate it
// either, so the dead arithmetic is what the shipped bytes do. Ghidra's own
// decompilation of this block drops it entirely (leaving a no-op pointer
// walk), which is the trap; the loop below is written from the raw bytes.
// LEVER: the same one that took ?sample to BYTE_EXACT - subscript the two
// fields as `grid_[1][row][col]` rather than walking a flat pointer. 37 of
// 86, against 7 of 86 for the walked-pointer spelling.
// TRIED: a walked row pointer for the first loop with a do-while row counter
// (the shape the image's own registers suggest) - 19 of 86, worse, because it
// reintroduces the named pointer locals the image does not spill. The image
// keeps `this` in EBX unspilled, recomputes each loop's pointer from it and
// reuses ONE stack slot (first the row counter, then the grid_[1] row
// pointer); this tree still reserves three slots (`sub esp, 0xc` against
// `push ecx`) and swaps ESI/EDI. Register allocation, not control flow.
*/
void Fractal::init() {
    // grid_[1]: 18 rows of 18 columns of rand() % 16, then the two wraps.
    for (int row = 0; row < 18; ++row) {
        for (int col = 0; col < 18; ++col) {
            int value = rand();
            value = value & 0x8000000f;
            if (value < 0) {
                value = ((value - 1) | 0xfffffff0) + 1;
            }
            grid_[1][row][col] = static_cast<char>(value);
        }
        grid_[1][row][18] = grid_[1][row][0];
    }
    for (int wrap_col = 0; wrap_col < 18; ++wrap_col) {
        grid_[1][18][wrap_col] = grid_[1][0][wrap_col];
    }

    // grid_[0][row][col] = grid_[1][row][col] for rows 0..16, cols 0..16.
    // The dead weighted sum is the image's; see the BUG note above.
    for (int band_row = 0; band_row <= 16; ++band_row) {
        for (int band_col = 0; band_col <= 16; ++band_col) {
            int total = grid_[1][band_row][band_col] * 4;
            const int *weight = RadiusOffsetX + 1;
            do {
                int step = *weight;
                weight += 1;
                total += (&grid_[1][0][0])[20 + band_row + step * 20
                                          + band_col];
            } while (weight <= RadiusOffsetX + 8);
            grid_[0][band_row][band_col] = grid_[1][band_row][band_col];
        }
    }

    // grid_[0] col 16 = col 0, then row 16 = row 0.
    for (int dup_row = 0; dup_row <= 16; ++dup_row) {
        grid_[0][dup_row][16] = grid_[0][dup_row][0];
    }
    for (int dup_col = 0; dup_col <= 16; ++dup_col) {
        grid_[0][16][dup_col] = grid_[0][0][dup_col];
    }
}

/*
Purpose: Blend the four grid_[0] taps around a 0..255 coordinate pair into one
         0..15 sample. Both world passes call it at two spacings to get two
         octaves of noise.
// ORIGINAL: 0x005C1F20 ?UNK1@Fractal@@QAEHHH@Z 0x005C1F20-0x005C1FB9 FILE BYTE_EXACT
// symbol    ?sample@Fractal@@QAEHHH@Z
// size      153 bytes
// prototype int (__thiscall ?UNK1@Fractal@@QAEHHH@Z)(Fractal* this, int, int)
// callers   3   call targets   0
// kind      game
// flags     (none)
// calls     (none)
*/
int Fractal::sample(int x_coord, int y_coord) {
    // Subtracted IN PLACE - the image loads each argument and `sub`s 0x80,
    // and reusing the parameter is what puts col_fraction in its stack slot.
    x_coord -= 0x80;
    y_coord -= 0x80;
    int row = (x_coord >> 8) & 0xf;
    int col = (y_coord >> 8) & 0xf;
    int col_fraction = (y_coord >> 3) & 0x1f;
    int row_fraction = (x_coord >> 3) & 0x1f;

    int top = grid_[0][row + 1][col] * row_fraction
              + grid_[0][row][col] * (32 - row_fraction);
    int bottom = grid_[0][row][col + 1] * (32 - row_fraction)
                 + grid_[0][row + 1][col + 1] * row_fraction;

    return (top * (32 - col_fraction) + bottom * col_fraction) >> 5;
}

/*
Purpose: Two octaves of sample() reduced to one 0..100 intensity - the value
         the fungus and erosion passes threshold against. Both callers spell
         the same arithmetic inline, which is how the name was read off them.
// ORIGINAL: 0x005C1FC0 ?UNK2@Fractal@@QAEHHHH@Z 0x005C1FC0-0x005C2020 FILE
// symbol    ?level@Fractal@@QAEHHHH@Z
// size      96 bytes
// prototype int (__thiscall ?UNK2@Fractal@@QAEHHHH@Z)(Fractal* this, int, int, int)
// callers   0   call targets   1
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x005C1F20
// LEVER: both scaled coordinates live in registers ACROSS the first call -
// the image shifts the same two registers again for the second octave - so
// the octaves are spelled as `coarse_x` / `coarse_y` locals, and the *5 lands
// before the second call. 33 of 40.
// TRIED: `sample(x << 4, y << 4)` then `(x << 4) << shift` inline at the call
// sites, which re-derives the shift from the argument each time (10 of 40);
// the clamp as a nested ternary `value < 0 ? 0 : value > 100 ? 100 : value`
// against two early returns - identical 33 of 40, the zero arm is the only
// disagreement either way (the image jumps forward to it, this tree inlines
// it behind a `jns`), so the shape is block placement, not source form.
*/
int Fractal::level(int x_coord, int y_coord, int shift) {
    // Both scaled coordinates live in registers across the first call - the
    // image shifts the SAME two registers again for the second octave - and
    // the *5 lands before the second call.
    int coarse_x = x_coord << 4;
    int coarse_y = y_coord << 4;
    int coarse = sample(coarse_x, coarse_y) * 5;
    int fine = sample(coarse_x << shift, coarse_y << shift);
    int value = (coarse + fine * 2) * 7 >> 8;
    if (value < 0) {
        return 0;
    }
    if (value > 100) {
        return 100;
    }
    return value;
}

/*
Purpose: Paint xenofungus over the map. A Fractal field is sampled at two
         spacings per tile to get a 0..100 intensity, the intensity bands and
         the native-life-forms setting decide whether the tile takes fungus,
         and tiles that pass get bit 0x20 set in their `bit` dword and every
         other flag in MapTileFlagClearMask stripped. On a 1-in-40 roll a
         fungus vehicle is seeded on the tile, which SMACX only (see
         ExpansionEnabled).
// ORIGINAL: 0x005C3440 ?world_fungus@@YAXXZ 0x005C3440-0x005C3678 FILE
// size      568 bytes
// prototype void (__cdecl ?world_fungus@@YAXXZ)()
// callers   2   call targets   5
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x005C03D0 0x005C1E20 0x005C1F20 0x005FCB20 0x0064601D
// LEVER: homed 2026-09-05 over src/fractal.h. The image's `sub esp, 0x2e4`
// is the Fractal object (722 bytes) plus the four int locals - no padding -
// which is what pinned sizeof(Fractal) to 0x2D2 and settled the class layout.
// Every fixed address in it is a tree name except MapTileFlagClearMask
// (0x006919EC, named on this pass - see map.cpp's definition).
// call_diff MORE is the known `MEASURED inline` do_all_non_input ceiling
// (AGENT_BRIEF: the image CALLS 0x005FCB20 once per row, this tree expands
// each site into check_net/do_net/do_non_input).
// TRIED: the y loop counter wants to live in EDI (the image's first
// callee-saved push) and lands in EBX here, which shifts every later
// register. Declaring `int y_coord = 0` BEFORE fungus_level with the rows as
// a do/while - the image's own statement order, and worth 7 of 183 -> 29 of
// 183 - is the best of four measured shapes: a for-loop inside the guard
// (7 of 183), an early `return` instead of the guard (7 of 183), and
// computing coarse_y/fine_y before the x-loop guard all scored worse. The
// remaining 154 disagreeing instructions are that one register assignment
// cascading; the call sequence, both sample spacings and every tile-field
// offset are the image's.
*/
void __cdecl world_fungus() {
    Fractal fractal;
    fractal.init();

    int y_coord = 0;
    int fungus_level = (static_cast<uint32_t>(MapNativeLifeForms) - 1u) * WorldBuilder.fungus;

    if (MapLatitudeBounds > 0) {
        do {
            // Both octaves' scaled coordinates are loop-invariant per row.
            int coarse_y = y_coord << 4;
            int fine_y = coarse_y << 4;

            for (int x_coord = y_coord & 1; x_coord < MapLongitudeBounds;
                 x_coord += 2) {
                int coarse_x = x_coord << 4;
                int level = fractal.sample(coarse_x, coarse_y) * 5
                            + fractal.sample(coarse_x << 4, fine_y) * 2;
                level = level * 7 >> 8;
                if (level < 0) {
                    level = 0;
                } else if (level > 100) {
                    level = 100;
                }

                Map *tile = map_tiles() + MapLongitude * y_coord
                            + (x_coord >> 1);
                if ((tile->bit2 & 0x80000002) == 2
                    && (tile->bit2 & 0xff000000) < 0x9000000) {
                    continue;
                }

                if (level < 20 - fungus_level
                    || level > fungus_level * 2 + 25) {
                    if (level < 40 - fungus_level
                        || level > fungus_level * 2 + 45) {
                        if (level < 60 - fungus_level
                            || level > fungus_level * 2 + 65) {
                            // Only the first and last rows fall through here.
                            if (y_coord != 0
                                && y_coord != MapLatitudeBounds - 1) {
                                continue;
                            }
                            if (rand() % 2 == 0) {
                                continue;
                            }
                        }
                    }
                }

                if ((tile->val3 & ~0x1F) >= 0x80) {
                    continue;
                }
                if ((tile->climate & 0xE0) >= 0x60) {
                    continue;
                }

                tile->bit |= 0x20;
                tile->bit &= ~MapTileFlagClearMask;

                if (rand() % 40 == 0 && (tile->climate & 0xE0) >= 0x60
                    && ExpansionEnabled != 0) {
                    veh_init(0x13, 0, x_coord, y_coord);
                }
            }

            do_all_non_input();
            y_coord++;
        } while (y_coord < MapLatitudeBounds);
    }
}

/*
Purpose: Re-cut every land tile's altitude bands from the shore-to-peak
         threshold table, then walk the map six more times spreading each
         natural altitude level one step outwards; refresh the visible map
         windows as it goes when the omniscient view is on and the
         regeneration is allowed to redraw.
// ORIGINAL: 0x005C3F70 ?world_shorelines@@YAXXZ 0x005C3F70-0x005C40E3 FILE
// size      371 bytes
// prototype void (__cdecl ?world_shorelines@@YAXXZ)()
// callers   3   call targets   7
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x0046AF40 0x0046B190 0x0046FD90 0x00591290 0x00591AD0 0x005C2020 0x005FCB20
// LEVER: homed from src/recovered/units/005c3f70.cpp on this pass. Its five
//   fixed-address bindings are the tree's names - MapLatitudeBounds,
//   MapLongitudeBounds, MapLongitude (the tile row stride), MapSeaLevel,
//   AltNatural (the threshold table the closing walk steps down), the
//   LockMapTable slot for the tile records themselves - so nothing is bound
//   twice. The byte read of GameState's omniscient bit is spelled as the
//   dword test the rest of this TU uses.
// TRIED: byte-exactness is capped by the same ceiling AGENT_BRIEF records for
//   Path::continent - the image CALLS 0x005FCB20 (do_all_non_input) twice and
//   this tree's `MEASURED inline` in temp.h expands each site into
//   check_net/do_net/do_non_input, 14 calls against the image's 8 and 167
//   instructions against 131 (call_diff MORE). The call sequence itself is
//   the image's: alt_set, temp_set, draw_tile, do_all_non_input,
//   world_alt_set, do_all_non_input, MapWin::clear_terrain, draw_map.
Status: homed, not yet byte-exact.
*/
void __cdecl world_shorelines() {
    for (int y = 0; y < MapLatitudeBounds; y++) {
        for (int x = y & 1; x < MapLongitudeBounds; x += 2) {
            int tile_index = MapLongitude * y + (x >> 1);
            int altitude = 0;
            Map *tile = &map_tiles()[tile_index];
            int depth = tile->contour - MapSeaLevel;

            int natural_index = 6;
            do {
                if (depth >= static_cast<int32_t>(AltNatural[natural_index])) {
                    altitude = natural_index;
                    break;
                }
                natural_index--;
            } while (natural_index >= 0);

            alt_set(x, y, altitude);

            if ((GameDataStateStorage.fields_.game_state_ & STATE_OMNISCIENT_VIEW) != 0
                && WorldClimateRedrawRequest != 0) {
                if (altitude >= 3) {
                    int temperature = altitude - 3;
                    if (temperature < 0) {
                        temperature = 0;
                    } else if (temperature > 2) {
                        temperature = 2;
                    }
                    temp_set(x, y, temperature);
                }
                draw_tile(x, y, 2);
            }
        }
        do_all_non_input();
    }

    for (int natural = 6; natural >= 2; natural--) {
        for (int y = 0; y < MapLatitudeBounds; y++) {
            for (int x = y & 1; x < MapLongitudeBounds; x += 2) {
                int tile_index = MapLongitude * y + (x >> 1);
                Map *tile = &map_tiles()[tile_index];
                int rocky = tile->climate >> 5;
                if (rocky == natural) {
                    world_alt_set(x, y, rocky, 1);
                }
            }
            do_all_non_input();
        }
    }

    if ((GameDataStateStorage.fields_.game_state_ & STATE_OMNISCIENT_VIEW) != 0
        && WorldClimateRedrawRequest != 0) {
        MapWinTable[0]->clear_terrain();
        draw_map(1);
    }
}

// Pinned NUL-terminated log text at 0x00691E2C and 0x00691E4C. The
// following alignment bytes are not part of either string's source extent.
// RECOVERY_OBJECT: v2 0x00691E2C 0x00691E4A _WorldClimateStartLog 0x005C5A30
static char WorldClimateStartLog[] = "-- ADJUSTING WORLD CLIMATE --";
// RECOVERY_OBJECT: v2 0x00691E4C 0x00691E69 _WorldClimateCompleteLog 0x005C5A30
static char WorldClimateCompleteLog[] = "-- WORLD CLIMATE COMPLETE --";

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
    log_say(WorldClimateStartLog, 0, 0, 0);
    world_shorelines();
    world_temperature();
    world_rivers();
    world_rainfall();
    Paths.continents();
    world_analysis();
    if (WorldClimateSkipTerritoryReset == 0) {
        reset_territory();
    }

    random_reseed(timeGetTime());

    if (WorldClimateSkipTerrainClear == 0) {
        MapWin **cursor = MapWinTable;
        MapWin **const end = cursor + MapWinTableSlots;
        do {
            MapWin *const window = *cursor;
            if (window->field_1DD74_ != 0) {
                window->clear_terrain();
            }
            ++cursor;
        } while (cursor < end);
        WorldClimateWorldWin.clear_terrain();
    }

    log_say(WorldClimateCompleteLog, 0, 0, 0);
    do_checksums(3);
}

/*
Purpose: Set the default altitude details for the specified tile.
// ORIGINAL: 0x005C58C0 ?world_alt_put_detail@@YAXHH@Z 0x005C58C0-0x005C58DB
// body      src/map.h
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
// ORIGINAL: 0x005C58E0 ?world_polar_caps@@YAXXZ 0x005C58E0-0x005C5A2F BYTE_EXACT
// BUG IN THE RECOVERY: the second pole write was `x - 1` where the image
//            computes `x + 1` (`lea eax, [esi + 1]` at 0x5C590A) - the image
//            pairs (x, 0) with (x+1, MapLatitudeBounds-1), so x=0 wrote
//            (-1, MapLatitudeBounds-1), wrapping to the far column, and every
//            pair was one tile out. `x + 1` is also what makes this
//            BYTE_EXACT.
// LEVER: three call-shape fixes, 1/128 -> BYTE_EXACT 128/128. (1) The six
//            pole writes go through the non-inline forwarder
//            `alt_put_detail_call` (bottom of this file): the image calls
//            alt_put_detail (0x00591260) for real and call_diff named the
//            inlined `world_alt_put_detail` a WRONG CALLEE; the header
//            inline definition folds those six sites away. (2) The four
//            random columns are `rnd(MapLongitudeBounds / 2, NULL)` written
//            out as its own body (guard clause + `rand() % seed`) - the
//            image calls rand() directly, never rnd(), and it computes the
//            halving from MapLongitudeBounds itself rather than reading the
//            pre-halved MapLongitude. (3) The second loop's bound is the
//            expression, not a local: `uint32_t bounds =
//            MapLongitudeBounds / 16` lets VC6 hoist the bound and count
//            down (`dec edi / jne`), where the image re-evaluates it every
//            iteration and counts up (`inc edi / cmp / jl`). Loop variable
//            is `int`, not `uint8_t` (the image's `x` lives in esi as a
//            dword), and the AltNatural[3] argument is not cast to
//            uint8_t - the image pushes the full dword.
// size      335 bytes
// prototype 
// callers   2   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00591260 0x0064601D
Return Value: n/a
Status: Complete - testing
*/
void __cdecl world_polar_caps() {
    // world_alt_put_detail(x, y) written out at every site: the image calls
    // alt_put_detail (0x00591260) directly from all six - `call_diff` names
    // the wrapper as a WRONG CALLEE - and calls rand() rather than rnd().
    for (int x = 0; x < MapLongitudeBounds; x += 2) {
        alt_put_detail_call(x, 0, AltNatural[3]);
        alt_put_detail_call(x + 1, MapLatitudeBounds - 1, AltNatural[3]);
    }
    for (int i = 0; i < MapLongitudeBounds / 16; i++) {
        // rnd(MapLongitude, NULL)'s own body (0x00579770): the image has no
        // call to rnd here, only to rand().
        int seed = MapLongitudeBounds / 2;
        int x_rand;
        if (seed - 1 <= 0) {
            x_rand = 0;
        } else {
            x_rand = rand() % seed;
        }
        alt_put_detail_call(x_rand * 2, 0, AltNatural[3]);
        seed = MapLongitudeBounds / 2;
        if (seed - 1 <= 0) {
            x_rand = 0;
        } else {
            x_rand = rand() % seed;
        }
        alt_put_detail_call(x_rand * 2 + 1, 1, AltNatural[3]);
        seed = MapLongitudeBounds / 2;
        if (seed - 1 <= 0) {
            x_rand = 0;
        } else {
            x_rand = rand() % seed;
        }
        alt_put_detail_call(x_rand * 2 + 1, MapLatitudeBounds - 1, AltNatural[3]);
        seed = MapLongitudeBounds / 2;
        if (seed - 1 <= 0) {
            x_rand = 0;
        } else {
            x_rand = rand() % seed;
        }
        alt_put_detail_call(x_rand * 2, MapLatitudeBounds - 2, AltNatural[3]);
    }
}

/*
Purpose: Set up the world contours.
// ORIGINAL: 0x005C5AE0 ?world_linearize_contours@@YAXXZ 0x005C5AE0-0x005C5BC3
// RECOVERY_BINDING: v1 {"reason":"Linearize tile contours from the default input intervals into the real contour target table, preserving the original signed division and final table publication.","evidence":["Pinned5C5B35/4F/58 loadAltContour at68FB20/24/30;5C5B3E/62 loadAltNaturalDefault at68FAF4/68FAF8;CMP6/JGE selects the +4 target.","Pinned5C5B73 IMUL retains low32bits, followed by CDQ/IDIV at5C5B78/79 and wrappedADD at5C5B7B;5C5B80 passes the full result to the byte-writing alt_put_detail owner.","Pinned5C5B98 calls do_all_non_input after each row;5C5BB1..5C5BBB copies11dwords fromAltContour68FB20 toAltNatural68FB4C only after all rows."]}
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
            const int band = alt_natural(x, y);
            const uint32_t detail = static_cast<uint32_t>(alt_detail_at(x, y));
            const uint32_t contour = AltContour[band];
            const uint32_t input_floor = AltNaturalDefault[band];
            const uint32_t target = AltContour[band
                + ((band >= ALT_3_LEVELS_ABOVE_SEA) ? 4 : 1)];
            const int32_t denominator = static_cast<int32_t>(
                AltNaturalDefault[band + 1] - input_floor);
            // IMUL wraps before CDQ/IDIV interprets the numerator as signed.
            // Preserve the original divide faults; there is no zero guard.
            const int32_t numerator = static_cast<int32_t>(
                (target - contour) * (detail - input_floor));
            const uint32_t result = contour
                + static_cast<uint32_t>(numerator / denominator);
            alt_put_detail(x, y, static_cast<int32_t>(result));
        }
        do_all_non_input();
    }
    // `memcpy`, and the ARRAYS, not their addresses. The image ends with
    // `mov ecx, 0xb; mov esi, 0x68fb20; mov edi, 0x68fb4c; rep movsd` - the
    // intrinsic, copying AltContour over AltNatural. `memcpy_s` emits a call
    // with four arguments, and `&AltNatural` wrote over the POINTER rather
    // than the array it names.
    memcpy(AltNatural, AltContour, sizeof(AltContour));
}

/*
Purpose: Determine if the specified tile is near a landmark.
// ORIGINAL: 0x005C5BD0 ?near_landmark@@YAHHH@Z 0x005C5BD0-0x005C5C61
// body      src/map.h
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
    new_landmark(x, y, StringTable.get(Natural[LM_CRATER].name));
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
            bit2_set(x_radius, y_radius, BIT2_JUNGLE, true);
            code_set(x_radius, y_radius, i);
        }
    }
    new_landmark(x, y, StringTable.get(Natural[LM_JUNGLE].name));
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
    new_landmark(x, y, StringTable.get(Natural[LM_SARGASSO].name));
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
            if (is_ocean(x_radius, y_radius)) {
                if (i && i < 9) {
                    bit_set(x_radius, y_radius, BIT_MONOLITH, true); // in ocean tiles?
                }
            } else {
                bit_set(x_radius, y_radius, BIT_FUNGUS | BIT_SUPPLY_REMOVE, true);
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
    new_landmark(x, y + 2, StringTable.get(Natural[LM_RUINS].name));
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
            bit2_set(x_radius, y_radius, BIT2_DUNES, true);
            rocky_set(x_radius, y_radius, ROCKINESS_FLAT);
            code_set(x_radius, y_radius, i);
        }
    }
    new_landmark(x, y + 2, StringTable.get(Natural[LM_DUNES].name));
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
    new_landmark(x, y, StringTable.get(Natural[LM_URANIUM].name));
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
                    StringTable.get(Natural[LM_FRESH].name));
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
    GameDataStateStorage.fields_.mount_planet_x_ = x;
    GameDataStateStorage.fields_.mount_planet_y_ = y;
    if (!is_not_landmark) {
        new_landmark(x, y, StringTable.get(Natural[LM_VOLCANO].name));
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
                bit2_set(x_radius, y_radius, BIT2_BOREHOLE, true);
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
    bit2_set(x, y, BIT2_BOREHOLE, true);
    new_landmark(x, y, StringTable.get(Natural[LM_BOREHOLE].name));
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
            bit2_set(x_radius, y_radius, BIT2_NEXUS, true);
            code_set(x_radius, y_radius, i);
        }
    }
    new_landmark(x, y, StringTable.get(Natural[LM_NEXUS].name));
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
                bit2_set(x_radius, y_radius, BIT2_UNITY, true);
                code_set(x_radius, y_radius, i);
                if (!i || i == 8 || i == 10 || i == 19) {
                    bit_set(x_radius, y_radius, BIT_SUPPLY_POD, true);
                } else {
                    bit_set(x_radius, y_radius, BIT_SUPPLY_REMOVE, true);
                }
            }
        }
        new_landmark(x, y, StringTable.get(Natural[LM_UNITY].name));
    }
}

/*
Purpose: Setup the 'Fossil Ridge' landmark (SMACX only).
// ORIGINAL: 0x005C7A80 ?world_fossil@@YAXHH@Z 0x005C7A80-0x005C7CA1
// HISTORICAL KNOWN DIVERGENCE (now restored): the `if (ExpansionEnabled)`
//            guard at the top of this body has no counterpart in the image -
//            0x005C7A80 goes straight into the site search, and the only
//            guard is in world_landmarks (0x005C8629). It is this tree's own
//            "added check similar to Unity Wreckage", so calling world_fossil
//            from anywhere but world_landmarks (Console::editor_natural does)
//            behaves differently from the shipped image with the expansion
//            disabled. Removing it is a behaviour change, not a spelling
//            fix, and needs a decision before the body can be chased.
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
            bit2_set(x_radius, y_radius, BIT2_FOSSIL, true);
            code_set(x_radius, y_radius, i);
        }
    }
    new_landmark(x, y, StringTable.get(Natural[LM_FOSSIL].name));
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
// Historical implementation note: ALT4/ALT3 and BIT2/code were rolled into one loop; the
// pinned function performs the altitude pass first and a separate mask/code pass afterward.
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
        }
    }
    for (uint32_t j = 0; j < 12; j++) {
        int x_radius = xrange(x + RadiusOffsetX[WorldCanyonOffsetValues[j]]);
        int y_radius = y + RadiusOffsetY[WorldCanyonOffsetValues[j]];
        if (on_map(x_radius, y_radius)) {
            bit2_set(x_radius, y_radius, BIT2_CANYON, true);
            code_set(x_radius, y_radius, WorldCanyonOffsetValues[j]);
        }
    }
    new_landmark(x, y + 2, StringTable.get(Natural[LM_CANYON].name));
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
            // 5C80F5 supplies the terrain bit, not Natural[]'s landmark id.
            bit2_set(x_radius, y_radius, BIT2_MESA, true);
            code_set(x_radius, y_radius, i);
        }
    }
    new_landmark(x, y + 2, StringTable.get(Natural[LM_MESA].name));
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
// Historical implementation note: BIT2/code was rolled into the ALT4 loop; the pinned function
// performs separate 13-entry altitude and mask/code passes using the original offset table.
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
    const uint32_t world_ridge_offset_val[13] = { 47, 44, 24, 20, 8, 7, 0, 5, 4, 17, 23, 35, 45 };
    for (uint32_t i = 0; i < 13; i++) {
        int x_radius = xrange(x + RadiusOffsetX[world_ridge_offset_val[i]]);
        int y_radius = y + RadiusOffsetY[world_ridge_offset_val[i]];
        if (on_map(x_radius, y_radius)) {
            world_alt_set(x_radius, y_radius, ALT_1_LEVEL_ABOVE_SEA, true);
        }
    }
    for (uint32_t j = 0; j < 13; j++) {
        int x_radius = xrange(x + RadiusOffsetX[world_ridge_offset_val[j]]);
        int y_radius = y + RadiusOffsetY[world_ridge_offset_val[j]];
        if (on_map(x_radius, y_radius)) {
            bit2_set(x_radius, y_radius, BIT2_RIDGE, true);
            code_set(x_radius, y_radius, world_ridge_offset_val[j]);
        }
    }
    new_landmark(x, y + 2, StringTable.get(Natural[LM_RIDGE].name));
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
    // if/else, which the disassembly does not have. The earlier call comparison
    // caught it (this tree made 13 calls against the image's 9),
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
    new_landmark(x, y, StringTable.get(Natural[LM_GEOTHERMAL].name));
}

/*
Purpose: Set up all the landmarks.
// ORIGINAL: 0x005C8630 ?world_landmarks@@YAXXZ 0x005C8630-0x005C86DC BYTE_EXACT
// RECOVERY_BINDING: v1 {"reason":"Preserve the original landmark generation sequence without the later Nessus Canyon addition.","evidence":["Pinned5C8630..5C86DC calls fifteen landmark generators and tail-jumps592940 fixup_landmarks; no world_canyon call follows world_geothermal.","Pinned5C8671..5C8690 tests ExpansionEnabled before unity and reloads it before fossil only within the enabled path."]}
// BUG IN THE RECOVERY: the two ExpansionEnabled guards were dropped from here
//            ("removed redundant check") but the image performs BOTH -
//            `mov eax,[0x9a6488]; test eax,eax; je` before world_unity and
//            again before world_fossil (0x5C8622 and 0x5C8629). world_unity
//            re-tests it internally (0x5C7756); world_fossil does NOT, so the
//            caller's guard is the only thing that keeps the Fossil Field off
//            a non-expansion map. Restoring them took 30/57 -> 55/57.
// TRIED: the last 5 instructions are the deliberate `world_canyon(-1, -1)`
//            this tree adds after world_geothermal ("Change: add Nessus
//            Canyon"): it is one more `push -1 / push -1 / call` block plus
//            8 bytes of stack the image's `add esp, 0x30` does not account
//            for, so no byte-exact match is reachable while that landmark is
//            generated. Historical deviation, not the current faithful sequence.
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
    if (ExpansionEnabled) {
        world_unity(-1, -1);
        if (ExpansionEnabled) {
            world_fossil(-1, -1);
        }
    }
    world_temple(-1, -1);
    world_borehole(-1, -1);
    world_sargasso(-1, -1);
    world_dune(-1, -1);
    world_fresh(-1, -1);
    world_geothermal(-1, -1);
    fixup_landmarks();
}

/*
Purpose: Check for any type of zone of control conflicts (base and/or unit).
// ORIGINAL: 0x005C89F0 ?zoc_any@@YAHHHH@Z 0x005C89F0-0x005C8AC0
// LEVER (2026-09-03): anything_at (0x005798E0) written out inline - the image
//            makes ZERO calls here and `call_diff` flagged the real call as
//            MORE-by-one; 3/82 -> 12/82 and call_diff is now clean. The
//            occupant test, the owner<MaxPlayerNum bound and the owner>=0
//            test are the three the image emits in that order, so the
//            `owner >= 0` that looks redundant after `val2 & 0xF` is
//            zoc_any's own test on anything_at's return, kept deliberately.
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
        int x_radius = xrange(x + VehMoveDeltaX[i]);
        int y_radius = y + VehMoveDeltaY[i];
        if (on_map(x_radius, y_radius)) {
            // Inlined anything_at(x_radius, y_radius): the image writes its
            // body out at this call site rather than calling 0x005798E0.
            Map *tile = map_loc(x_radius, y_radius);
            if (tile->bit & (BIT_VEH_IN_TILE | BIT_BASE_IN_TILE)) {
                int owner = tile->val2 & 0xF;
                if (owner < MaxPlayerNum
                    && owner >= 0 && (uint32_t)owner != faction_id
                    && !has_treaty(faction_id, owner, DTREATY_PACT)) {
                    return owner + 1;
                }
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
        int x_radius = xrange(x + VehMoveDeltaX[i]);
        int y_radius = y + VehMoveDeltaY[i];
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
        int x_radius = xrange(x + VehMoveDeltaX[i]);
        int y_radius = y + VehMoveDeltaY[i];
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
Purpose: Is any of the eight tiles a tile's odd-q ring holds land?
// ORIGINAL: 0x004620C0 sub_4620c0 0x004620C0-0x00462184 FILE
// symbol    ?is_land_near@@YAHHH@Z
// NO IMAGE CALLER AND NO IMAGE REFERENCE AT ALL, so the name is derived from
//        the body, not from a catalogue entry. The eight offsets it walks are
//        the first eight entries of map.h's VehMoveDeltaX/VehMoveDeltaY - the
//        image's own copies of those tables are 0x0066EF50 and 0x0066EF74,
//        and the ninth entry is the tile itself, which this skips. "Land" is
//        the altitude nibble of `climate` reaching ALT_BIT_SHORE_LINE, the
//        same test is_ocean() inverts.
// TRIED: nullary contract head (reads [ebp+8]/[ebp+0xc], no ecx use -> 2
//        plain int args, not __thiscall).
// TRIED: 3/68 after `!on_map(x, y)` for the guard and `& 0xE0` for the
//        altitude nibble (this one the image NARROWS to `and al,0xe0 / cmp
//        eax,0x60`, the opposite of 0x00591E90 next to it, so `& ~0x1F` is
//        wrong here). The wall is the prologue: the image spends `push ecx`
//        on a stack slot before its callee-saved pushes and keeps
//        `MapIsFlat & 1` in it, reloading it every iteration along with the
//        `x` and `y` parameters, where this tree enregisters the flag in EBX
//        and the arguments - the receiver-spill wall the brief documents for
//        the Palette bodies. 68 instructions against 75 and every one after
//        the slot is register-shifted. Left here.
// size      196 bytes
// prototype
// callers   0   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
Return Value: Is land within the ring? true/false
Status: Complete
*/
BOOL __cdecl is_land_near(int x, int y) {
    if (!on_map(x, y)) {
        return false;
    }
    for (int i = 0; i < 8; i++) {
        int x_radius = xrange(x + VehMoveDeltaX[i]);
        int y_radius = y + VehMoveDeltaY[i];
        if (on_map(x_radius, y_radius)
            && (map_loc(x_radius, y_radius)->climate & 0xE0) >= ALT_BIT_SHORE_LINE) {
            return true;
        }
    }
    return false;
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
        int x_edge = site_xrange(x + VehMoveDeltaX[dir]);
        int y_edge = y + VehMoveDeltaY[dir];
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
            int x_scan = site_xrange(x + VehMoveDeltaX[dir & 7]);
            int y_scan = y + VehMoveDeltaY[dir & 7];
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
            int x_scan = site_xrange(x + VehMoveDeltaX[dir & 7]);
            int y_scan = y + VehMoveDeltaY[dir & 7];
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

// Non-inline forwarder to `alt_put_detail` (map.h), for the same reason as
// `bit_set_call` above: world_polar_caps (0x005C58E0) is where the image makes
// six real `call 0x591260`s, and an inline definition in the header gets those
// six sites folded away instead.
void __cdecl alt_put_detail_call(int x, int y, int detail) {
    alt_put_detail(x, y, detail);
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

// ORIGINAL: 0x005797D0 ?site_at@@YAHHHH@Z 0x005797D0-0x00579832 FILE
// TRIED: single combined expression and explicit temp reads of the first two parameters both match 44 of 46 mnemonics at 98 of 98 bytes; VC6 interleaves the push/mov prologue differently (push, push, mov, mov instead) - register scheduling, not logic
// working copy - scaffold materialised by --work
// size      98 bytes
// prototype 
// callers   1   call targets   2
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x00591B50 0x005C4FD0
int __cdecl site_at(int x, int y, int is_ocean_site) {
    Map *tile = map_loc(x, y);
    if (tile->bit & BIT_BASE_RADIUS) {
        return 0;
    }
    int site = tile->val2 >> 4;
    if (site == 0) {
        site = world_site(x, y, is_ocean_site);
        site_set(x, y, site);
    }
    return site;
}

// ORIGINAL: 0x00592A80 ?set_dirty@@YAXXZ 0x00592A80-0x00592AE6 FILE BYTE_EXACT
// size      102 bytes
// prototype 
// callers   6   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// PRESERVED UNIT - measured MNEMONIC_ONLY.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x00592A80
// measured tier  MNEMONIC_ONLY
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/00592a80/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?set_dirty@@YAXXZ  at 0x00592A80  (102 bytes)
//
// A VERIFICATION ARTIFACT, not product source: classes are opaque and
// globals are bound to fixed addresses, because both are byte-visible
// and both differ from the style src/ is written in.
//
// VC6 DIALECT - this must compile under cl 12.00.8168, which is the
// only compiler this project builds with. Avoid: auto, nullptr, constexpr,
// static_assert, enum class, range-for, lambdas, long long, <cstdint>,
// and declaring `int i` twice in one function (VC6 leaks for-scope).
// static_cast/reinterpret_cast are fine and are the right spelling.
//
// SOURCE-FORM RULES, each one learned by a fan-out agent that lost
// attempts to it. They are here rather than in a prompt so the next
// agent does not rediscover them:
//
//  * A ternary is not an if. `x ? a : b` lowers to `jne` with the arms
//    swapped; `if (x) {...}` gives `je`.
//  * VC6 NEVER hoists a same-polarity guard's body out of line. To get
//    two special cases branching FAR away, write them negated and
//    NESTED - `if (a != X) { if (a != Y) { return D; } ...; }` - not as
//    sequential guard clauses, which inline each body instead.
//  * VC6 rejects `__thiscall` on a free function pointer (C4234). For
//    an indirect virtual call use the generated VCall shim below; for a
//    thiscall function POINTER, take a member-function pointer of a
//    dummy class instead of spelling the convention.
//  * Loop form is visible: while / do-while / for lower differently,
//    and so does counting up versus down.
//  * If the original dedicates a callee-saved register to a constant
//    across the whole body (an extra `push ebx`/`push edi` in the
//    prologue), it had enough register pressure to do so. That is a
//    hard case - the tool reports a similarity ratio so you can tell a
//    near miss from a wrong body.
void __cdecl set_dirty() {
    int y = 0;
    Map *tile = map_tiles();
    if (MapLatitudeBounds > 0) {
        int limit = MapLongitudeBounds;
        do {
            int x = y & 1;
            if (x < limit) {
                do {
                    tile->bit2 |= BIT_CONDENSER;
                    x += 2;
                    tile++;
                    limit = MapLongitudeBounds;
                } while (x < limit);
            }
            y++;
        } while (y < MapLatitudeBounds);
        UnkBitfield1 |= 4;
        return;
    }
    UnkBitfield1 |= 4;
}

// ORIGINAL: 0x005A66E0 ?encrypt_write@@YAHPAXIIPAUFILE@@@Z 0x005A66E0-0x005A67A8 FILE
// symbol    ?encrypt_write@@YAHPAXIIPAU_iobuf@@@Z
// this tree's FILE is struct _iobuf, so the tag differs from the image's
// TRIED: nothing ruled out; the [ebp+0xb]/[ebp+0xf]/[ebp+0x13] scratch bytes are stack-slot reuse of the first three parameters' high byte, not real locals - modelled as plain unsigned char key1/key2/byte1/byte2 instead. MISMATCH #3, push vs mov, whole-function frame order
// working copy - scaffold materialised by --work
// size      200 bytes
// prototype int (__cdecl ?encrypt_write@@YAHPAXIIPAUFILE@@@Z)(void*, unsigned int, unsigned int, FILE* file)
// callers   3   call targets   3
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005D4510 0x00644EF2 0x0064603F
int __cdecl encrypt_write(void *data, unsigned int size, unsigned int count,
                          FILE *file) {
    unsigned char *src = reinterpret_cast<unsigned char *>(data);
    unsigned int raw_size = size * count;
    unsigned int padded = raw_size;
    if ((raw_size & 1) != 0) {
        padded = raw_size + 1;
    }
    unsigned char *buf = reinterpret_cast<unsigned char *>(mem_get(padded));
    if (buf != 0) {
        unsigned char key1 = 0x80;
        unsigned char key2 = 0x80;
        unsigned int i = 0;
        unsigned char *dst = buf;
        unsigned int pairs = padded >> 1;
        if (pairs != 0) {
            do {
                key2 = key2 + 1;
                key1 = key1 - 1;
                unsigned char byte1 = *src;
                unsigned char byte2;
                unsigned char *next = src + 1;
                if (i < raw_size) {
                    byte2 = *next;
                    next = src + 2;
                } else {
                    byte2 = 0;
                }
                unsigned char enc1 = byte1;
                if (byte1 != 0 && (byte1 ^ key2) != 0) {
                    enc1 = byte1 ^ key2;
                }
                unsigned char enc2 = byte2;
                if (byte2 != 0) {
                    unsigned char x = byte2 ^ key1;
                    if (x != 0) {
                        enc2 = x;
                    }
                }
                *dst = enc2;
                dst[1] = enc1;
                dst += 2;
                ++i;
                src = next;
            } while (i < pairs);
        }
        int result = fwrite(buf, padded, 1, file);
        free(buf);
        return result;
    }
    return 0;
}

// ORIGINAL: 0x005A67B0 ?encrypt_read@@YAHPAXIIPAUFILE@@@Z 0x005A67B0-0x005A6886 FILE
// symbol    ?encrypt_read@@YAHPAXIIPAU_iobuf@@@Z
// this tree's FILE is struct _iobuf, so the tag differs from the image's
// working copy - scaffold materialised by --work
// size      214 bytes
// prototype int (__cdecl ?encrypt_read@@YAHPAXIIPAUFILE@@@Z)(void* buffer, unsigned int block, unsigned int count, FILE* file)
// callers   3   call targets   3
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005D4510 0x00644EF2 0x00646178
int __cdecl encrypt_read(void *data, unsigned int size, unsigned int count,
                         FILE *file) {
    unsigned int total = size * count;
    unsigned int allocSize = total;
    if (allocSize & 1) {
        allocSize = allocSize + 1;
    }
    unsigned char *buf = reinterpret_cast<unsigned char *>(mem_get(allocSize));
    if (buf == 0) {
        return 0;
    }
    int n = fread(buf, allocSize, 1, file);
    if (n == 0) {
        free(buf);
        return 0;
    }
    unsigned char prevA = 0x80;
    unsigned char prevB = 0x80;
    unsigned char *src = buf;
    unsigned char *dst = reinterpret_cast<unsigned char *>(data);
    unsigned int remaining = total;
    for (unsigned int pairs = allocSize >> 1; pairs != 0; --pairs) {
        ++prevB;
        unsigned char b1 = src[1];
        --prevA;
        unsigned char b0 = src[0];
        src += 2;
        if (b1 != 0 && (unsigned char)(b1 ^ prevB) != 0) {
            b1 = b1 ^ prevB;
        }
        if (b0 != 0 && (unsigned char)(b0 ^ prevA) != 0) {
            b0 = b0 ^ prevA;
        }
        *dst = b1;
        unsigned char *next = dst + 1;
        if (remaining > 1) {
            *next = b0;
            next = dst + 2;
        }
        remaining -= 2;
        dst = next;
    }
    free(buf);
    return 1;
}

// ORIGINAL: 0x005C32A0 ?world_rocky@@YAXXZ 0x005C32A0-0x005C343F FILE
// size      415 bytes
// prototype 
// callers   2   call targets   5
// kind      game
// flags     sp_ready;purged_ok
// calls     0x00591BC0 0x00591D60 0x00591F00 0x005FCB20 0x0064601D
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x005C32A0
// measured tier  MISMATCH
// divergence     66
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?world_rocky@@YAXXZ  at 0x005C32A0  (415 bytes)
//
// A VERIFICATION ARTIFACT, not product source: classes are opaque and
// globals are bound to fixed addresses, because both are byte-visible
// and both differ from the style src/ is written in.
//
// The VC6 dialect limits and the source-form rules used to live here.
// They are knowledge, not scaffolding, so they now live in the agent
// system prompt (mizuchi.yaml, plugins.claude-runner.systemPrompt),
// where they can be edited without regenerating anything and are in
// context from the first token rather than behind a file read. This
// emitter computes declarations; it does not carry lessons.
void __cdecl world_rocky() {
    for (int y = 0; y < MapLatitudeBounds; y++) {
        for (int x = y & 1; x < MapLongitudeBounds; x += 2) {
            int half_x = x >> 1;
            Map *rec = &map_tiles()[MapLongitude * y + half_x];
            // offset 0xc is the tile's bit2: a landmark code already on the
            // tile disqualifies it from being re-rolled.
            uint32_t val3 = rec->bit2;

            bool skip = false;
            if ((val3 & 0x80000001) == 1 && static_cast<int>(val3 & 0xff000000) < 0x15000000) {
                skip = true;
            } else if ((val3 & 0x80004000) == 0x4000) {
                unsigned int hi = val3 & 0xff000000;
                if (static_cast<int>(hi) < 0x9000000 || hi == 0xc000000 || hi == 0xa000000 ||
                    hi == 0x13000000 || hi == 0xf000000 || hi == 0x14000000 || hi == 0x10000000) {
                    skip = true;
                }
            }

            if (skip) {
                continue;
            }

            int minerals = minerals_at(x, y);
            rocky_set(x, y, minerals);

            Map *tile = &map_tiles()[MapLongitude * y + half_x];
            uint32_t flags = tile->bit2;

            if ((flags & 0x80) != 0 && (tile->climate & 0xe0) <= 0x5f) {
                continue;
            }

            unsigned int bitval;
            if ((tile->climate & 0xe0) < 0x60) {
                bitval = 0x10000;
            } else {
                bool useRandomFlat = false;
                if ((flags & 0x80000004) == 4) {
                    unsigned int r = rand();
                    r &= 0x80000001;
                    bool zero;
                    if (static_cast<int>(r) < 0) {
                        zero = ((r - 1) | 0xfffffffe) == 0xffffffff;
                    } else {
                        zero = (r == 0);
                    }
                    useRandomFlat = zero;
                }
                if (useRandomFlat) {
                    bitval = 0x10000;
                } else {
                    unsigned char b = tile->climate;
                    if ((b & 0xe0) < 0x81 && (b & 0x18) < 9) {
                        continue;
                    }
                    bitval = 0x20000;
                }
            }
            bit_set(x, y, bitval, 1);
        }
        do_all_non_input();
    }
}

// ORIGINAL: 0x005C4410 ?world_UNK1@@YAXHH@Z 0x005C4410-0x005C4467 FILE
// size      87 bytes
// prototype void (__cdecl ?world_UNK1@@YAXHH@Z)(int xCoord, int yCoord)
// callers   0   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00591A80
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x005C4410
// measured tier  MISMATCH
// divergence     18
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/005c4410/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?world_UNK1@@YAXHH@Z  at 0x005C4410  (87 bytes)
//
// A VERIFICATION ARTIFACT, not product source: classes are opaque and
// globals are bound to fixed addresses, because both are byte-visible
// and both differ from the style src/ is written in.
//
// VC6 DIALECT - this must compile under cl 12.00.8168, which is the
// only compiler this project builds with. Avoid: auto, nullptr, constexpr,
// static_assert, enum class, range-for, lambdas, long long, <cstdint>,
// and declaring `int i` twice in one function (VC6 leaks for-scope).
// static_cast/reinterpret_cast are fine and are the right spelling.
//
// SOURCE-FORM RULES, each one learned by a fan-out agent that lost
// attempts to it. They are here rather than in a prompt so the next
// agent does not rediscover them:
//
//  * A ternary is not an if. `x ? a : b` lowers to `jne` with the arms
//    swapped; `if (x) {...}` gives `je`.
//  * VC6 NEVER hoists a same-polarity guard's body out of line. To get
//    two special cases branching FAR away, write them negated and
//    NESTED - `if (a != X) { if (a != Y) { return D; } ...; }` - not as
//    sequential guard clauses, which inline each body instead.
//  * VC6 rejects `__thiscall` on a free function pointer (C4234). For
//    an indirect virtual call use the generated VCall shim below; for a
//    thiscall function POINTER, take a member-function pointer of a
//    dummy class instead of spelling the convention.
//  * Loop form is visible: while / do-while / for lower differently,
//    and so does counting up versus down.
//  * If the original dedicates a callee-saved register to a constant
//    across the whole body (an extra `push ebx`/`push edi` in the
//    prologue), it had enough register pressure to do so. That is a
//    hard case - the tool reports a similarity ratio so you can tell a
//    near miss from a wrong body.
void __cdecl world_UNK1(int x, int y) {
    int rainfall = ((map_loc(x, y)->climate >> 3) & 3) + 1;
    if (rainfall > 2) {
        climate_set(x, y, 2);
        return;
    }
    climate_set(x, y, rainfall);
}

// ORIGINAL: 0x00532C30 ?synch_display@@YAXHHHH@Z 0x00532C30-0x00532DA2 FILE BYTE_EXACT
// size      370 bytes
// prototype 
// callers   1   call targets   3
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x0047A890 0x005FCBB0 0x00625E30
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x00532C30
// measured tier  MISMATCH
// divergence     124
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/00532c30/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?synch_display@@YAXHHHH@Z  at 0x00532C30  (370 bytes)
//
// A VERIFICATION ARTIFACT, not product source: classes are opaque and
// globals are bound to fixed addresses, because both are byte-visible
// and both differ from the style src/ is written in.
//
// The VC6 dialect limits and the source-form rules used to live here.
// They are knowledge, not scaffolding, so they now live in the agent
// system prompt (mizuchi.yaml, plugins.claude-runner.systemPrompt),
// where they can be edited without regenerating anything and are in
// context from the first token rather than behind a file read. This
// emitter computes declarations; it does not carry lessons.
void __cdecl synch_display(int stage, int arg1, int arg2, int arg3) {
    switch (stage) {
    case 0:
        parse_num(0, arg1);
        parse_num(1, arg2);
        NetMsgPrimary.pop("SYNCHINGGAME2", 0, 1, 0);
        do_all_draws();
        return;
    case 6:
        parse_num(0, arg1);
        parse_num(1, arg2);
        parse_num(2, arg3);
        NetMsgPrimary.pop("SYNCHINGPLAYERS2", 0, 1, 0);
        do_all_draws();
        return;
    case 4:
        parse_num(0, arg1);
        parse_num(1, arg2);
        NetMsgPrimary.pop("SYNCHINGWORLD2", 0, 1, 0);
        do_all_draws();
        return;
    case 0x30:
        parse_num(0, arg1);
        parse_num(1, arg2);
        parse_num(2, arg3);
        NetMsgPrimary.pop("SYNCHINGWORLD3", 0, 1, 0);
        do_all_draws();
        return;
    case 0x12:
        parse_num(0, arg1);
        parse_num(1, arg2);
        NetMsgPrimary.pop("SYNCHINGVEHICLES2", 0, 1, 0);
        do_all_draws();
        return;
    case 0x14:
        parse_num(0, arg1);
        parse_num(1, arg2);
        NetMsgPrimary.pop("SYNCHINGBASES2", 0, 1, 0);
        do_all_draws();
        break;
    }
}

/*
Purpose: Write a secret project's name, and its cost bracket when asked.
// ORIGINAL: 0x00590990 ?say_project@@YAXPAXHH@Z 0x00590990-0x00590A0E FILE
// size      126 bytes
// prototype void (__cdecl ?say_project@@YAXPAXHH@Z)(void*, int, int)
// callers   0   call targets   2
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x006169A0 0x00645470
// ONE image reference outside its own body, as a raw function pointer in the
//        file's .data, so the catalogue gives no parameter names; they are
//        read off the body (an output buffer, a project index, a flag).
//        SecretProjectRules is the 0x30-stride rules record whose first field
//        is the original LPSTR name, matching the Facility[] representation;
//        0x2CC into the label block is label_get(179).
// TRIED: (as 00590990.cpp) the artifact's `strcat(StringTemp, ...)`,
//        i.e. no append of StringTemp to the output - it is what the image
//        does, and it is kept here.
// symbol    ?say_project@@YAXPAXHH@Z
// TRIED: 32/40. Everything left is evaluation order around the first
//        `StringTable.get`: the image hoists the receiver
//        (`mov ecx, 0x9b90d8`) to the top of the body, before the
//        `project_id * 0x30` index arithmetic, and reads the name into EAX;
//        this tree computes the argument first and reads the name into ECX,
//        which also costs the receiver load its own instruction. Spelling the
//        receiver through a local `Strings &`, or the name through a local
//        `LPSTR`, only adds the store the image does not have. Not chased
//        further this pass.
Return Value: n/a
Status: Complete
*/
void __cdecl say_project(void *text, int project_id, int with_cost) {
    char *output = static_cast<char *>(text);
    strcat(output, StringTable.get(Facility[FacilitySPStart + project_id].name));
    if (with_cost != 0) {
        strcat(output, " ");
        strcat(output, "(");
        strcat(StringTemp, label_get(179));
        strcat(output, ")");
    }
}

// ORIGINAL: 0x00590BA0 ?say_vehicles@@YAXPAXHH@Z 0x00590BA0-0x00590C14 FILE
// size      116 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x006169A0 0x00645470
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x00590BA0
// measured tier  MISMATCH
// divergence     4
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/00590ba0/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?say_vehicles@@YAXPAXHH@Z  at 0x00590BA0  (116 bytes)
//
// A VERIFICATION ARTIFACT, not product source: classes are opaque and
// globals are bound to fixed addresses, because both are byte-visible
// and both differ from the style src/ is written in.
//
// VC6 DIALECT - this must compile under cl 12.00.8168, which is the
// only compiler this project builds with. Avoid: auto, nullptr, constexpr,
// static_assert, enum class, range-for, lambdas, long long, <cstdint>,
// and declaring `int i` twice in one function (VC6 leaks for-scope).
// static_cast/reinterpret_cast are fine and are the right spelling.
//
// SOURCE-FORM RULES, each one learned by a fan-out agent that lost
// attempts to it. They are here rather than in a prompt so the next
// agent does not rediscover them:
//
//  * A ternary is not an if. `x ? a : b` lowers to `jne` with the arms
//    swapped; `if (x) {...}` gives `je`.
//  * VC6 NEVER hoists a same-polarity guard's body out of line. To get
//    two special cases branching FAR away, write them negated and
//    NESTED - `if (a != X) { if (a != Y) { return D; } ...; }` - not as
//    sequential guard clauses, which inline each body instead.
//  * VC6 rejects `__thiscall` on a free function pointer (C4234). For
//    an indirect virtual call use the generated VCall shim below; for a
//    thiscall function POINTER, take a member-function pointer of a
//    dummy class instead of spelling the convention.
//  * Loop form is visible: while / do-while / for lower differently,
//    and so does counting up versus down.
//  * If the original dedicates a callee-saved register to a constant
//    across the whole body (an extra `push ebx`/`push edi` in the
//    prologue), it had enough register pressure to do so. That is a
//    hard case - the tool reports a similarity ratio so you can tell a
//    near miss from a wrong body.
void __cdecl say_vehicles(void *text, int chassis_id, int with_height) {
    char *dst = reinterpret_cast<char *>(text);
    strcat(dst, VehPrototypes[chassis_id].veh_name);
    if (with_height != 0) {
        strcat(dst, " ");
        strcat(dst, "(");
        strcat(StringTemp, label_get(0x314 / 4));
        strcat(dst, StringTemp);
        strcat(dst, ")");
    }
}

// ORIGINAL: 0x00592250 ?say_loc@@YAXPADHHHHH@Z 0x00592250-0x005923FD FILE
// size      429 bytes
// prototype void (__cdecl ?say_loc@@YAXPADHHHHH@Z)(int8*, int xCoord, int yCoord, int, int, int)
// callers   4   call targets   5
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x004E3A50 0x004E3D50 0x006169A0 0x00645470 0x0064FC88
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x00592250
// measured tier  MISMATCH
// divergence     3
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/00592250/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?say_loc@@YAXPADHHHHH@Z  at 0x00592250  (429 bytes)
//
// A VERIFICATION ARTIFACT, not product source: classes are opaque and
// globals are bound to fixed addresses, because both are byte-visible
// and both differ from the style src/ is written in.
//
// The VC6 dialect limits and the source-form rules used to live here.
// They are knowledge, not scaffolding, so they now live in the agent
// system prompt (mizuchi.yaml, plugins.claude-runner.systemPrompt),
// where they can be edited without regenerating anything and are in
// context from the first token rather than behind a file read. This
// emitter computes declarations; it does not carry lessons.
void __cdecl say_loc(char *text, int x, int y, int want_name, int trailer_mode,
                     int want_coords) {
    char number[80];
    int base_id = base_at(x, y);
    if (want_name != 0) {
        if (base_id < 0) {
            want_coords = 0;
            base_id = base_find(x, y, -1, -1, -1, ConsoleGlobal.faction_id_);
            if (base_id < 0) {
                goto trailer;
            }
            strcat(text, label_get(0xf8 / 4));
            strcat(text, " ");
        } else {
            goto skip_first_check;
        }
    }
    if (base_id < 0) {
        goto trailer;
    }
skip_first_check:
    if (want_coords != 0) {
        strcat(text, label_get(0x20 / 4));
        strcat(text, " ");
    }
    strcat(text, Bases[base_id].name_string);
    if (trailer_mode != 0) {
        strcat(text, " ");
    }

trailer:
    if (trailer_mode == 1 || (trailer_mode == 2 && base_id < 0)) {
        strcat(text, "(");
        _itoa(x, number, 10);
        strcat(text, number);
        strcat(text, ", ");
        _itoa(y, number, 10);
        strcat(text, number);
        strcat(text, ")");

        int flags = GameDataStateStorage.fields_.game_state_;
        if ((flags & 0x40) != 0 && (flags & 0x1000) != 0) {
            strcat(StringTemp, " ");
            Map *tile = map_loc(x, y);
            _itoa(tile->region, number, 10);
            strcat(StringTemp, number);
        }
    }
}

// ORIGINAL: 0x00592AF0 ?clear_dirty@@YAXXZ 0x00592AF0-0x00592B3C FILE BYTE_EXACT
// size      76 bytes
// prototype 
// callers   2   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)

void __cdecl clear_dirty() {
    int i = 0;
    Map *record = map_tiles();
    if (MapLatitudeBounds > 0) {
        int threshold = MapLongitudeBounds;
        do {
            int j = i & 1;
            if (j < threshold) {
                do {
                    record->bit2 |= 0x400000;
                    j += 2;
                    ++record;
                    threshold = MapLongitudeBounds;
                } while (j < threshold);
            }
            i++;
        } while (i < MapLatitudeBounds);
    }
}



/*
Purpose: Rank the prototypes a faction can build, best first, into ProtoSortIds.
// ORIGINAL: 0x005A6270 ?proto_sort@@YAXH@Z 0x005A6270-0x005A63C9 FILE
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
                && !has_tech_call(VehPrototypes[proto_id].preq_tech, faction_id))) {
            if (proto_id < MaxVehProtoFactionNum) {
                key = 999999999;
            } else {
                key = 0x34FB5E38;
                ProtoSortCount++;
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
// ORIGINAL: 0x005A63D0 ?proto_sort_2@@YAXH@Z 0x005A63D0-0x005A650B
// RECOVERY_BINDING: v1 {"evidence": ["image:0x005A63D0-0x005A650B", "source:src/recovered/units/005a63d0.cpp", "source:src/faction.h:PlayerData.proto_id_active"], "reason": "Recover design-choice ordering with typed prototype, weapon and faction state and source-owned shared output; original IDs remain separate from status ranking keys."}
void __cdecl proto_sort_2(int faction_id) {
    ProtoSortCount = 0;
    int sort_keys[128];
    for (int index = 0; index < 128; ++index) {
        int prototype_id = index % MaxVehProtoFactionNum;
        if (index >= MaxVehProtoFactionNum) {
            prototype_id += faction_id * MaxVehProtoFactionNum;
        }
        ProtoSortIds[index] = prototype_id;
        const VehPrototype &prototype = VehPrototypes[prototype_id];
        int key = prototype_id;
        bool available = (prototype.flags & PROTO_ACTIVE) != 0;
        if (available && prototype_id < MaxVehProtoFactionNum) {
            available = Weapon[prototype.weapon_id].offense_rating >= 0
                && prototype_id != 15
                && has_tech_call(prototype.preq_tech, faction_id);
        }
        if (!available) {
            if (prototype_id < MaxVehProtoFactionNum) {
                key = 999999999;
            } else {
                key += 0x400;
                ++ProtoSortCount;
            }
        } else {
            if (prototype.obsolete_factions & (1 << faction_id)) {
                if (prototype_id < MaxVehProtoFactionNum) {
                    key += 0x2000;
                } else {
                    key += PlayersData[faction_id].proto_id_active[prototype_id]
                        ? 0x800 : 0x1000;
                }
            }
            ++ProtoSortCount;
        }
        sort_keys[index] = key;
    }
    sort(128, ProtoSortIds, sort_keys);
}

/*
Purpose: Rebuild the altitude-bonus tables from the landmarks on the map.
// ORIGINAL: 0x00592940 ?fixup_landmarks@@YAXXZ 0x00592940-0x00592A74 FILE
// size      308 bytes
// prototype void (__cdecl ?fixup_landmarks@@YAXXZ)(void)
// callers   2   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// LEVER: the five 0x31-byte template runs the memcpy seeds the bonus tables
//        from are the ones the 0x0066EF28 note in map.h lists - 0x0068577C
//        (Arid), 0x006857F0 (Moist), 0x00685864 (Flat), 0x0068593C (Rainy),
//        0x006859B0 (Ocean) - and the tables they seed are the same
//        AltBonus* bindings mapwin.cpp's get_alt reads, so they moved to
//        map.h for this body to reach.
// TRIED: 10/100, every divergence register allocation. The copy loop and the
//        two bit arms agree; what is left is that the image spends `push ebx`
//        on a third callee-saved register and keeps `xor ebx,ebx` in it for
//        the `= 0` stores and the `bit2 == 0` test, where this tree materialises
//        each zero at its use - the extra-callee-saved-register case the brief
//        calls a hard ceiling. Its y loop also lives in a second stack slot
//        (`[ebp-8]`) that this tree does not need. Not chased further.
// TRIED: as 00592940.cpp the whole body was spelled with a raw
//        `int32_t *records` on the tile array; `map_loc()` names it and lets
//        the same `bit2` read through a member.
// BUG IN THE ORIGINAL: the BIT2_RIDGE arm (flags & 0x800) clears
//        AltBonusFlat - the SAME table and the SAME five entries the
//        BIT2_MESA arm (flags & 0x100) just cleared, unconditionally and
//        with nothing in between. Whatever the second arm was meant to
//        touch, it is a duplicate today; it is reproduced as written,
//        because removing it would change the program the image ships.
Return Value: n/a
Status: Complete
*/

void __cdecl fixup_landmarks() {
    // 113 bytes each: the image inlines every one as `mov ecx, 0x1c / rep
    // movsd / movsb`, which is 28 dwords plus the last byte.
    memcpy(AltBonusOcean, AltTemplateOcean, 113);
    memcpy(AltBonusFlat, AltTemplateFlat, 113);
    memcpy(AltBonusRainy, AltTemplateRainy, 113);
    memcpy(AltBonusArid, AltTemplateArid, 113);
    memcpy(AltBonusMoist, AltTemplateMoist, 113);

    for (int y = 0; y < MapLatitudeBounds; y++) {
        for (int x = y & 1; x < MapLongitudeBounds; x += 2) {
            Map *tile = map_loc(x, y);
            if (tile->bit2 == 0) {
                continue;
            }
            // Native5929F3 uses SAR, not an unsigned top-byte extraction.
            int type = static_cast<int32_t>(tile->bit2) >> 24;
            if (type >= 0) {
                continue;
            }
            type = -1 - type;
            const unsigned char *digits = AltDirectionTable + type * 5;
            if (tile->bit2 & BIT2_VOLCANO) {
                for (int i = 0; i < 5; i++) {
                    AltBonusArid[digits[i]] = 0;
                }
            }
            if (tile->bit2 & BIT2_CRATER) {
                for (int i = 0; i < 5; i++) {
                    // Native592A1D resets EBX before the crater byte stores.
                    AltBonusMoist[digits[i]] = 0;
                }
            }
        }
    }
}

/*
Purpose: Carve rivers. Every land tile is tried as a source in turn: the
         walker picks a weighted random direction each step, prefers downhill
         and existing channels, stops at the sea or after 999 steps, and marks
         the tiles it ran through. Tiles that stayed dry keep bit 0x200 out of
         their `bit` dword; bit 0x80 is then set on every tile a river crossed.
// ORIGINAL: 0x005C38B0 ?world_rivers@@YAXXZ 0x005C38B0-0x005C3F66 FILE
// size      1718 bytes
// prototype void (__cdecl ?world_rivers@@YAXXZ)()
// callers   1   call targets   6
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x00591260 0x00591290 0x005FCB20 0x006257E0 0x00625810
// LEVER: homed 2026-09-05 out of src/unrecovered/005c38b0.cpp (deleted).
//   Every fixed address was already a tree name once AltContour (0x0068FB20)
//   landed: RiverStepX / RiverStepY are the paired diagonal tables the
//   RadiusOffset note in map.h documents, and the rest are MapLongitude /
//   MapLongitudeBounds / MapLatitudeBounds / MapArea / MapIsFlat /
//   map_tiles(). Its six calls are random_reseed, random (three sites),
//   alt_put_detail and do_all_non_input.
// TRIED: byte-exactness is out of reach on this pass. The image tests
//   `tile->bit` with byte-width forms (`test ch, 1` for the 0x100 bit) and
//   interleaves its fourteen stack locals differently from any spelling
//   measured here; the archived transcription stopped at instruction #15 of
//   the framed build for the same reason. The control flow, the field offsets
//   and the call sequence below are the image's, so call_diff and
//   dead_branches can both check it. The five ways a step can end early are
//   NOT interchangeable, and this is the part a rewrite gets wrong: hitting
//   water (bit 0x80) or a three-river junction ABANDONS the whole walk with
//   to_sea cleared and the head not advanced, while a candidate whose own
//   eight neighbours disqualify it only drops out of the four-direction loop.
*/
void __cdecl world_rivers() {
    // Every tile: forget that a river ran through it.
    for (int i = 0; i < static_cast<int>(MapArea); ++i) {
        map_tiles()[i].bit &= 0xfffffd7f;
    }

    if (MapLatitudeBounds > 0) {
        int y_coord = 0;
        do {
            for (int x_coord = y_coord & 1; x_coord < MapLongitudeBounds;
                 x_coord += 2) {
                Map *source = map_tiles() + MapLongitude * y_coord
                              + (x_coord >> 1);
                if (((source->bit & 0x100) == 0)
                    || ((source->climate & 0xE0) <= 0x5F)) {
                    continue;
                }

                // A fresh source: forget every earlier river's path first.
                for (int i = 0; i < static_cast<int>(MapArea); ++i) {
                    map_tiles()[i].bit2 &= 0xff7fffff;
                }

                random_reseed(y_coord * 0x2f + x_coord * 0x1f);
                int river_dir = random(0, 4);

                int band = source->climate >> 5;
                int contour = source->contour;
                source->bit2 |= 0x800000;

                int cur_x = x_coord;
                int cur_y = y_coord;
                int joined = 0;      // stepped into an existing river
                int to_sea = 1;      // the walk has not hit water yet
                int hill_run = 1;    // tiles climbed at this altitude
                int step_parity = 1;
                int steps = 0;
                uint32_t bits = source->bit;

                while ((bits & 0x1000000) == 0) {
                    int last_dir = random(0, 2);
                    int turn = last_dir + (river_dir - (step_parity & 1)) & 3;
                    int straight = river_dir ^ 2;
                    int nb_contour = 0;
                    int nb_climate = 0;
                    int cand_split[3];
                    int cand_dir[3];
                    int cand_weight[3];
                    int cand_count = 0;
                    int total_weight = 0;
                    int weight_base = 8;
                    int best_contour = 9999;
                    bool abandon = false;

                    for (int dir = 0; dir < 4; dir++) {
                        if (dir == straight) {
                            continue;
                        }
                        int next_x = RiverStepX[dir] + cur_x;
                        int flat = MapIsFlat & 1;
                        if (flat == 0) {
                            if (next_x < 0) {
                                next_x += MapLongitudeBounds;
                            } else if (MapLongitudeBounds <= next_x) {
                                next_x -= MapLongitudeBounds;
                            }
                        }
                        int next_y = RiverStepY[dir] + cur_y;
                        if (next_y < 0 || next_y >= MapLatitudeBounds
                            || next_x < 0 || next_x >= MapLongitudeBounds) {
                            continue;
                        }

                        Map *neighbour = map_tiles()
                                         + MapLongitude * next_y
                                         + (next_x >> 1);
                        if ((neighbour->bit & 0x80) != 0) {
                            abandon = true;
                            break;
                        }
                        if ((neighbour->bit & 0x100) != 0) {
                            // A junction, never a source: count this
                            // neighbour's own river neighbours.
                            int rivers = 0;
                            for (int k = 0; k < 4; k++) {
                                int nx = RiverStepX[k] + next_x;
                                if (flat == 0) {
                                    if (nx < 0) {
                                        nx += MapLongitudeBounds;
                                    } else if (MapLongitudeBounds <= nx) {
                                        nx -= MapLongitudeBounds;
                                    }
                                }
                                int ny = RiverStepY[k] + next_y;
                                if (ny < 0 || ny >= MapLatitudeBounds
                                    || nx < 0 || nx >= MapLongitudeBounds) {
                                    continue;
                                }
                                if ((map_tiles()[MapLongitude * ny + (nx >> 1)]
                                         .bit
                                     & 0x100)
                                    != 0) {
                                    rivers++;
                                }
                            }
                            if (rivers > 1) {
                                abandon = true;
                                break;
                            }
                        }

                        nb_climate = neighbour->climate;
                        nb_contour = neighbour->contour;
                        if ((nb_climate >> 5) > band) {
                            // Downhill or level only.
                            continue;
                        }

                        int drop = (contour - nb_contour) + 2;
                        if (drop < 1) {
                            drop = 1;
                        } else if (drop > 9999) {
                            drop = 9999;
                        }
                        int weight = drop * 0x20;
                        if (dir == turn) {
                            weight += 0x1f;
                        }
                        if (contour < nb_contour) {
                            if (best_contour < nb_contour) {
                                continue;
                            }
                            weight = weight_base;
                            weight_base++;
                            best_contour = nb_contour;
                        }

                        int next_split = 0;
                        int climb = 0;
                        for (int k = 0; k < 4; k++) {
                            if (k == (dir ^ 2)) {
                                continue;
                            }
                            int kx = RiverStepX[k] + next_x;
                            if (flat == 0) {
                                if (kx < 0) {
                                    kx += MapLongitudeBounds;
                                } else if (MapLongitudeBounds <= kx) {
                                    kx -= MapLongitudeBounds;
                                }
                            }
                            int ky = RiverStepY[k] + next_y;
                            if (ky < 0 || ky >= MapLatitudeBounds || kx < 0
                                || kx >= MapLongitudeBounds) {
                                continue;
                            }
                            Map *around = map_tiles() + MapLongitude * ky
                                          + (kx >> 1);
                            if ((around->climate & 0xE0) < 0x60
                                && (nb_climate & 0xE0) > 0x5F && hill_run < 5) {
                                weight /= 2;
                            }
                            if (nb_contour < around->contour) {
                                climb++;
                                if ((around->bit & 0x80) != 0) {
                                    break;
                                }
                            }
                            if ((around->bit2 & 0x800000) != 0) {
                                if (joined != 0
                                    || (around->climate & 0xE0) < 0x60) {
                                    break;
                                }
                                next_split = 1;
                                weight = weight / 8;
                            }
                        }
                        if (weight == 0) {
                            continue;
                        }
                        if (climb == 3) {
                            weight = 1;
                        }
                        total_weight += weight;
                        cand_weight[cand_count] = weight;
                        cand_split[cand_count] = next_split;
                        cand_dir[cand_count] = dir;
                        cand_count++;
                    }

                    if (abandon) {
                        to_sea = 0;
                        break;
                    }
                    if (cand_count == 0) {
                        break;
                    }

                    int pick = random(0, total_weight);
                    river_dir = 0;
                    for (int i = 0; i < cand_count; i++) {
                        if (pick < cand_weight[i]) {
                            joined = cand_split[i];
                            river_dir = cand_dir[i];
                            break;
                        }
                        pick -= cand_weight[i];
                    }

                    cur_x += RiverStepX[river_dir];
                    if ((MapIsFlat & 1) == 0) {
                        if (cur_x < 0) {
                            cur_x += MapLongitudeBounds;
                        } else if (MapLongitudeBounds <= cur_x) {
                            cur_x -= MapLongitudeBounds;
                        }
                    }
                    cur_y += RiverStepY[river_dir];

                    Map *head = map_tiles() + MapLongitude * cur_y
                                + (cur_x >> 1);
                    band = head->climate >> 5;
                    int next_contour = head->contour;

                    if (contour < next_contour) {
                        // Drop the tile to its band's nominal contour, but
                        // never below one step under the tile we came from.
                        int target = AltContour[band];
                        if (AltContour[band] < contour - 1) {
                            target = contour - 1;
                        }
                        alt_put_detail(cur_x, cur_y, target);
                        next_contour = target;
                    }

                    steps++;
                    contour = next_contour;
                    if (steps > 999) {
                        break;
                    }
                    head->bit2 |= 0x800000;
                    step_parity++;
                    hill_run++;
                    bits = head->bit;
                }

                if (band > 2) {
                    Map *mouth = map_tiles() + MapLongitude * cur_y
                                 + (cur_x >> 1);
                    if (((mouth->bit & 0x80) == 0) && (to_sea != 0)
                        && (((mouth->bit & 3) == 0)
                            || (7 < (mouth->val2 & 0xf)))) {
                        mouth->bit |= 0x200;
                    }
                }

                // Every tile this river crossed is now a river tile.
                for (int crossed = 0; crossed < static_cast<int>(MapArea);
                     ++crossed) {
                    if ((map_tiles()[crossed].bit2 & 0x800000) != 0) {
                        map_tiles()[crossed].bit |= 0x80;
                    }
                }
            }
            do_all_non_input();
            y_coord++;
        } while (y_coord < MapLatitudeBounds);
    }

    for (int swept = 0; swept < static_cast<int>(MapArea); ++swept) {
        map_tiles()[swept].bit2 &= 0xff7fffff;
    }
}

// Historical source-seed record retained during owner migration.
// The dated BLOCKED note below is historical evidence, not current status.
// BLOCKED (map pass, 2026-09-05): NOT homed - world_build (map.cpp) reaches it
//   through a pending_bodies.cpp forwarder. Everything this body needs is now
//   NAMED, which was the only thing missing: Fractal is src/fractal.h
//   (char grid_[2][19][19], 722 bytes - this body's `sub esp, 0x314` is that
//   plus its fourteen stack locals), VehMoveDeltaX/VehMoveDeltaY are map.h's
//   tables at 0x0066EF50/0x0066EF74, 0x0066EFE0/0x0066F464 are
//   RadiusOffsetX[9]/RadiusOffsetY[9], 0x0068FB50 is &AltNatural[1],
//   0x00945B00 is path.cpp's Paths (Paths.continents() at 0x005C2D0F), and the
//   WorldBuilder fields at 0x009502F0..0x00950304 are deep_water / shelf /
//   plains / beach / hills. The rewrite is tree-style only: keep this
//   artifact's five goto targets distinct - 0x005C2DF6 is "do the level
//   computation", 0x005C3058 is its band switch, 0x005C30DE is the post-walk
//   coast clamp, and the two that SETTLE for the tile are 0x005C321F and
//   0x005C3058's tail - and note that `local_1c` / `local_8` are read before
//   they are written on the band==2 path (BUG IN THE ORIGINAL, carry-over from
//   the previous tile; reproduce it, do not fix it).

// ORIGINAL: 0x005C2CB0 ?world_erosion@@YAXXZ 0x005C2CB0-0x005C3299
// RECOVERY_BINDING: v1 {"evidence": ["Pinned image 01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf: complete original entry 0x005C2CB0 with span 0x005C2CB0-0x005C3299; native disassembly traced across both erosion passes.", "src/fractal.h owns the 722-byte Fractal sampled by the two native calls; src/map.h owns Map, map_tiles, neighbor tables and AltNatural; src/path.h owns Paths.continents; map.cpp includes the genuine WorldBuilder and terrain interfaces.", "Pinned native flow initializes the per-tile intensity at 0x005C2DC1, 0x005C2DCA or 0x005C2DCF before altitude dispatch at 0x005C2DF6, and initializes neighbor counters before their walks at 0x005C2E18, 0x005C2EB1 and 0x005C2F43. The seed's uninitialized/carry warning is historical, not a proved current defect; the owner body follows the pinned initialized paths.", "Pinned branch targets 0x005C30B6/0x005C3221 settle altitude-2 low threshold directly; 0x005C30DE applies the independent altitude-3 neighbor clamp before coast blending."], "reason": "Home the pinned 0x005C2CB0 terrain-erosion body in map.cpp's real Fractal, Map, WorldBuilder, Paths and altitude-table owners while preserving the original image identity and extent."}
// TRIED: Ghidra-shaped goto/label transcription (verified instruction-by- instruction against the raw disassembly for control flow, push order and branch polarity) compiles and matches through instruction #41; diverges afterward on register-allocation shape (shl vs mov) inside the inner tile loop. Not chased further given the function's size (~500 instructions).
void __cdecl world_erosion() {
    Fractal fractal;
    fractal.init();

    int map_scale = 1;
    int world_scale = 1;
    if (MapLandCoverage == 1) {
        map_scale = quick_root(0xC80);
        world_scale = quick_root(static_cast<int>(MapArea));
    }

    int pass;
    for (pass = 0; pass < 2; ++pass) {
        if (pass != 0)
            world_shorelines();
        Paths.continents();

        int y;
        for (y = 0; y < MapLatitudeBounds; ++y) {
            int x = y & 1;
            for (; x < MapLongitudeBounds; x += 2) {
                Map *const tile = &map_tiles()[
                    MapLongitude * y + (x >> 1)];
                const unsigned contour = tile->contour;
                const unsigned altitude = tile->climate >> 5;
                int intensity = fractal.sample(x << 4, y << 4) * 5;
                const int fine = fractal.sample(x << 7, y << 7);
                intensity = (intensity + fine * 2) * 7 >> 8;
                if (intensity < 0)
                    intensity = 0;
                else if (intensity > 100)
                    intensity = 100;

                const bool process = (pass == 0) ? altitude <= 2 : altitude > 2;
                if (!process)
                    continue;

                int result = 0;
                int local_1c;
                int local_8;
                int region = -1;
                int neighbor_index;

                if (altitude == 2 || altitude == 3) {
                    // Count low-altitude tiles in the four-neighbor ring.
                    local_8 = 0;
                    for (neighbor_index = 0; neighbor_index < 8;
                         ++neighbor_index) {
                        int nx = x + VehMoveDeltaX[neighbor_index];
                        if (!(MapIsFlat & 1)) {
                            if (nx < 0)
                                nx += MapLongitudeBounds;
                            else if (nx >= MapLongitudeBounds)
                                nx -= MapLongitudeBounds;
                        }
                        const int ny = y + VehMoveDeltaY[neighbor_index];
                        if (ny >= 0 && ny < MapLatitudeBounds
                            && nx >= 0 && nx < MapLongitudeBounds) {
                            const Map *const neighbor = &map_tiles()[
                                MapLongitude * ny + (nx >> 1)];
                            if ((neighbor->climate & 0xE0) < 0x41)
                                ++local_8;
                        }
                    }

                    if (altitude != 3) {
                        local_1c = 0;
                        region = -1;
                        for (neighbor_index = 0; neighbor_index < 8;
                             ++neighbor_index) {
                            int nx = x + VehMoveDeltaX[neighbor_index];
                            if (!(MapIsFlat & 1)) {
                                if (nx < 0)
                                    nx += MapLongitudeBounds;
                                else if (nx >= MapLongitudeBounds)
                                    nx -= MapLongitudeBounds;
                            }
                            const int ny = y + VehMoveDeltaY[neighbor_index];
                            if (ny < 0 || ny >= MapLatitudeBounds
                                || nx < 0 || nx >= MapLongitudeBounds)
                                continue;
                            const Map *const neighbor = &map_tiles()[
                                MapLongitude * ny + (nx >> 1)];
                            if ((neighbor->climate & 0xE0) <= 0x40)
                                continue;
                            const int next_region = neighbor->region;
                            if (region < 0) {
                                region = next_region;
                            } else if (region != next_region) {
                                if (static_cast<int>(MapArea * 0x32 / 0xC80)
                                    <= static_cast<int>(Continents[region].tile_count)
                                     + static_cast<int>(Continents[next_region].tile_count))
                                    local_1c = 1;
                                if (Continents[region].tile_count
                                    < Continents[next_region].tile_count)
                                    region = next_region;
                            }
                        }
                    } else {
                        local_1c = 0;
                        for (neighbor_index = 0; neighbor_index < 16;
                             ++neighbor_index) {
                            int nx = x + RadiusOffsetX[9 + neighbor_index];
                            if (!(MapIsFlat & 1)) {
                                if (nx < 0)
                                    nx += MapLongitudeBounds;
                                else if (nx >= MapLongitudeBounds)
                                    nx -= MapLongitudeBounds;
                            }
                            const int ny = y + RadiusOffsetY[9 + neighbor_index];
                            if (ny < 0 || ny >= MapLatitudeBounds
                                || nx < 0 || nx >= MapLongitudeBounds)
                                continue;
                            const Map *const neighbor = &map_tiles()[
                                MapLongitude * ny + (nx >> 1)];
                            if ((neighbor->climate & 0xE0) <= 0x40)
                                continue;
                            const unsigned next_region = neighbor->region;
                            if (next_region < 0x40
                                && next_region != tile->region)
                                ++local_1c;
                        }
                    }

                    if (altitude == 2) {
                        if (local_1c != 0) {
                            local_8 += 4;
                            if (local_8 < 0)
                                local_8 = 0;
                            else if (local_8 > 8)
                                local_8 = 8;
                        }
                        int adjusted = intensity;
                        if (intensity < static_cast<int>(contour))
                            adjusted = static_cast<int>(WorldBuilder.shelf)
                                     + intensity;
                        if (adjusted < static_cast<int>(AltNatural[altitude + 1])) {
                            result = adjusted;
                            goto settle_tile;
                        }

                        const int value = (local_8 - 3)
                            * static_cast<int>(WorldBuilder.shelf);
                        intensity = -((value + ((value >> 31) & 3)) >> 2);
                        result = adjusted + intensity;
                        goto settle_tile;
                    } else if (local_1c != 0) {
                        // Native 0x5C30DE applies this carry to altitude 3
                        // independently of altitude 2's earlier clamp.
                        local_8 += 4;
                        if (local_8 < 0)
                            local_8 = 0;
                        else if (local_8 > 8)
                            local_8 = 8;
                    }

                    result = ((8 - local_8)
                              * WorldBuilder.plains
                              * world_scale / (map_scale * 8)) + intensity;
                    if (result < static_cast<int>(contour)) {
                        int ocean_coverage = MapOceanCoverage < 1 ? 1
                            : (MapOceanCoverage > 2 ? 2
                                : static_cast<int>(MapOceanCoverage));
                        intensity = (((((static_cast<int>(WorldBuilder.hills)
                            * static_cast<int>(MapErosiveForces)) / 2)
                            * (8 - local_8) / (ocean_coverage * 8))
                            + WorldBuilder.beach) * world_scale) / map_scale;
                        result += intensity;
                    } else {
                        int roughness = local_8 < 0 ? 0
                            : (local_8 > 8 ? 8 : local_8);
                        int ocean_coverage = MapOceanCoverage < 1 ? 1
                            : (MapOceanCoverage > 2 ? 2
                                : static_cast<int>(MapOceanCoverage));
                        result = ((result - static_cast<int>(contour))
                            * (8 - roughness) * static_cast<int>(MapErosiveForces))
                            / (ocean_coverage * 8) + static_cast<int>(contour);
                    }
                } else if (altitude < 2) {
                    result = intensity - WorldBuilder.deep_water;
                } else if (altitude == 4) {
                    if (static_cast<int>(contour) < intensity)
                        intensity = intensity * 2 - static_cast<int>(contour);
                    result = intensity
                        + static_cast<int>(WorldBuilder.hills)
                          * static_cast<int>(MapErosiveForces)
                        + WorldBuilder.peaks;
                } else if (altitude > 4) {
                    result = static_cast<int>(WorldBuilder.hills)
                        * static_cast<int>(MapErosiveForces) + intensity;
                }

settle_tile:
                result = (result + 1 + static_cast<int>(contour)) / 2;
                if (MapErosiveForces == 0 && result > 0x4A)
                    result = 0x4A;
                alt_put_detail(x, y, result);
            }
            do_all_non_input();
        }
    }
}

/*
Purpose: Build the whole planet: reset the altitude table and the Map* rules
         run, wipe the tiles, raise continents and hills until the land targets
         are met, then hand the result to erosion, the polar caps, the
         shorelines and every landmark/terrain pass, and refresh the map
         windows. Repeats up to four times until world_validate accepts the
         planet or the omniscient-view bit aborts the retry.
// ORIGINAL: 0x005C86E0 ?world_build@@YAXXZ 0x005C86E0-0x005C899D FILE
// size      701 bytes
// prototype void (__cdecl ?world_build@@YAXXZ)()
// callers   2   call targets   30
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x0046B190 0x0046FD90 0x004C45E0 0x00591040 0x00592940 0x005C28F0
//           0x005C2B40 0x005C2CB0 0x005C32A0 0x005C3440 0x005C3680 0x005C3F70
//           0x005C40F0 0x005C4170 0x005C4470 0x005C5A30 0x005C5AE0 0x005C5EF0
//           ... and the seventeen world_* landmark passes.
// LEVER: homed 2026-09-05. Every callee was already homed in this file or
//   declared in map.h, and every fixed address already had a tree name but
//   three: 0x0068FAF4 and 0x0068FB20 are now AltNaturalDefault and AltContour
//   (map.h, the two 11-int tables that bracket AltNatural), and 0x009B22EC /
//   0x009B22E8 are map.cpp's own WorldClimateSkipTerritoryReset /
//   WorldClimateRedrawRequest, now extern in map.h.
//   The archived transcription this replaces MISSED the `mov [0x94988c], esi`
//   at 0x005C8769 - world_build clears MapIsFlat before raising anything,
//   which is why the flat-map guard at 0x005C8839 tests the same dword.
// TRIED, all measured: the eleven-int AltNatural reset as a pointer-bump
//   do/while chasing the image's `rep movsd` (VC6 kept a one-pointer loop
//   with a +0x58 displacement and the body went frameless, 15 of 215 against
//   22 of 215 for the plain for-loop); hoisting MapWinTable into a local
//   reference for the tail walk. call_diff FEWER by one is the map_win_table
//   accessor: the image folds 0x007D3C3C into `mov esi, imm32` inside the
//   loop, this tree's `MapWinSlots &map_win_table()` in mapwin.h is a real
//   CALL at all three sites here, and forcing it inline is a mapwin.h edit
//   that would touch game.cpp's and time.cpp's walks as well as this body.
//   The image's register plan (EBX = continent count live across the whole
//   body, ESI/EDI parked on the copy) is otherwise the wall.
*/
void __cdecl world_build() {
    int continent_count = 0;

    int attempt = 0;
    do {
        // TRIED: the pointer-bump do/while spelling of this copy, hoping for
        // the image's single `rep movsd`; VC6 kept a one-pointer loop with a
        // +0x58 displacement and the whole body came out frameless (/GX
        // selected) at 15 of 215 against 22 of 215 for the plain for-loop
        // below. The `rep movsd` is a counted-copy idiom this tree has not
        // reached from source yet.
        for (int i = 0; i < 11; ++i) {
            AltNatural[i] = AltNaturalDefault[i];
        }

        WorldClimateRedrawRequest = 1;
        WorldClimateSkipTerritoryReset = 1;

        int *const rule_end = MapStorage.metadata_.planet_rules_ + 7;
        for (int *rules = MapStorage.metadata_.planet_rules_; rules < rule_end; ++rules) {
            if (*rules >= 0) {
                continue;
            }
            *rules = rand() % 3;
        }

        map_wipe();

        MapIsFlat = 0;
        int land_target = (2u - static_cast<uint32_t>(MapOceanCoverage)) * WorldBuilder.land_mod
                          + WorldBuilder.land_base;
        land_target = land_target * MapArea / 3200;
        int island_target = WorldBuilder.islands * MapArea / 3200;
        BrushVal1 = 0;
        WorldBuildVal1 = land_target;

        if ((GameDataStateStorage.fields_.game_state_ & 0x80) != 0) {
            MapWinTable[0]->clear_terrain();
            draw_map(1);
        }

        int builds = 0;
        while ((BrushVal1 < WorldBuildVal1 || continent_count < island_target)
               && builds < 100) {
            builds++;
            continent_count++;
            build_continent(continent_count);
        }

        if (MapErosiveForces >= 1) {
            int hills = (static_cast<uint32_t>(MapErosiveForces) - 1u) * WorldBuilder.hills_mod
                        + WorldBuilder.hills_base;
            hills = hills * MapArea / 3200;
            if (MapOceanCoverage == 2) {
                hills = hills / 2;
            }
            if (hills > 0) {
                do {
                    build_hills(4);
                    hills--;
                } while (hills != 0);
            }
        }

        world_erosion();

        if ((MapIsFlat & 1) == 0 && MapOceanCoverage < 2
            && (MapOceanCoverage == 0 || rand() % 3 == 0)) {
            world_polar_caps();
        }

        world_shorelines();
    } while ((GameDataStateStorage.fields_.game_state_ & 0x80) == 0 && world_validate() != 0
             && (++attempt, attempt < 4));

    world_temperature();
    world_riverbeds();
    world_fungus();
    world_monsoon(-1, -1);
    world_crater(-1, -1);
    world_volcano(-1, -1, 0);
    world_mesa(-1, -1);
    world_ridge(-1, -1);
    world_diamond(-1, -1);
    world_ruin(-1, -1);
    if (ExpansionEnabled != 0) {
        world_unity(-1, -1);
        if (ExpansionEnabled != 0) {
            world_fossil(-1, -1);
        }
    }
    world_temple(-1, -1);
    world_borehole(-1, -1);
    world_sargasso(-1, -1);
    world_dune(-1, -1);
    world_fresh(-1, -1);
    world_geothermal(-1, -1);

    fixup_landmarks();
    world_linearize_contours();
    WorldClimateRedrawRequest = 0;
    world_climate();
    world_rocky();
    WorldClimateSkipTerritoryReset = 0;

    if (WorldClimateSkipTerrainClear == 0) {
        MapWinSlots &table = MapWinTable;
        for (int slot = 0; slot < static_cast<int>(MapWinTableSlots); slot++) {
            if (table[slot]->field_1DD74_ != 0) {
                table[slot]->clear_terrain();
            }
        }
        WorldClimateWorldWin.clear_terrain();
    }
}

extern int BaseAtDebugPopupFlag;

// ORIGINAL: 0x0051E760 ?do_checksums@@YAXH@Z 0x0051E760-0x0051F0A7
// RECOVERY_BINDING: v1 {"reason":"Recover checksum diagnostics through the real network producer, source-owned 21-word turn checksum storage, typed faction names and original local/shared text buffers. Preserve the original unwritten diagnostic tail; no byte or runtime acceptance claim.","evidence":["original:0x51E760-0x51F0A7; image:01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf","git:b4e2d110:src/unrecovered/0051e760.cpp","Native51E830..51E840 copies21 words; producer appends26 diagnostic words only when BaseAtDebugPopupFlag is nonzero.","Original section and faction header loops mutate StringTemp; the remaining formatted reports use local buffers and _itoa radix10.","recovery/tasks/base-colony-economy-coverage-wave.json; recovery/tests/test_turn_checksums_behavior.py"]}
// TRIED: literal transcription of the ~25 itoa+strcat+log_say_hex report blocks and 3 walk loops (verified every log_say/log_say_hex call is (msg,value,0,0) with the message text pushed last=1st-param, confirmed against raw disasm push order, not assumed); the switch's dead "cmp esi,4;jne" fallthrough at 0x51E82B is provably unreachable (case4's own short jmp skips over it) and is omitted. Used a file-scope #define REPORT(...)/#undef macro instead of a helper function to stay inside "no helper" while avoiding 25x hand-duplicated blocks. Compiles; first divergence at instr #13 (deeper than most landings this batch), not chased to byte-exact given size (2375 bytes).
// kind      game
void __cdecl do_checksums(int type) {
    if (BaseAtDebugPopupFlag == 0 && type != 4) {
        return;
    }

    // NetDaemon::game_checksum writes 21 common values and, when the debug
    // popup flag is set, appends the diagnostic values through slot 46.
    // Keep the tail uninitialized when the producer does not write it: the
    // original type-4 consumer still reads it; no deterministic value is established.
    int checksums[47];
    char text[256];
    char number[50];
    NetDaemonNet.game_checksum(checksums);

    log_say(" ", 0, 0, 0);
    if (type <= 4) {
        switch (type) {
        case 0:
            log_say("Checksums at end of turn", GameDataStateStorage.fields_.turn_current_num_, 0, 0);
            break;
        case 1:
            log_say("Checksums at game start", GameDataStateStorage.fields_.turn_current_num_, 0, 0);
            break;
        case 2:
            log_say("Checksums before world regen", GameDataStateStorage.fields_.turn_current_num_, 0, 0);
            break;
        case 3:
            log_say("Checksums after world regen", GameDataStateStorage.fields_.turn_current_num_, 0, 0);
            break;
        case 4:
            log_say("Checksums at beginning of turn", GameDataStateStorage.fields_.turn_current_num_, 0, 0);
            memcpy(NetTurnChecksums, checksums, 0x54);
            break;
        }
    }

    int common_index;
    for (common_index = 0; common_index < 5; ++common_index) {
        const char *label;
        switch (common_index) {
        case 0: label = "GAME checksum = "; break;
        case 1: label = "LEADERS checksum = "; break;
        case 2: label = "MAP checksum = "; break;
        case 3: label = "VEHICLES checksum = "; break;
        default: label = "BASES checksum = "; break;
        }
        text[0] = '\0';
        _itoa(checksums[common_index], number, 10);
        strcat(text, number);
        strcat(text, " ");
        strcat(text, label);
        log_say_hex(text, checksums[common_index], 0, 0);
    }

    log_say(" ", 0, 0, 0);
    int section_index;
    for (section_index = 0; section_index < 16; ++section_index) {
        StringTemp[0] = '\0';
        strcat(StringTemp, "Map SECTION ");
        _itoa(section_index, number, 10);
        strcat(StringTemp, number);
        strcat(StringTemp, " = ");

        text[0] = '\0';
        _itoa(checksums[section_index + 5], number, 10);
        strcat(text, number);
        strcat(text, " ");
        strcat(text, StringTemp);
        log_say_hex(text, checksums[section_index + 5], 0, 0);
    }

    int aggregate_index;
    for (aggregate_index = 0; aggregate_index < 3; ++aggregate_index) {
        const char *label;
        switch (aggregate_index) {
        case 0: label = "Rules checksum = "; break;
        case 1: label = "Projects checksum = "; break;
        default: label = "Tech checksum = "; break;
        }
        text[0] = '\0';
        _itoa(checksums[21 + aggregate_index], number, 10);
        strcat(text, number);
        strcat(text, " ");
        strcat(text, label);
        log_say_hex(text, checksums[21 + aggregate_index], 0, 0);
    }

    int player_index;
    for (player_index = 1; player_index < MaxPlayerNum; ++player_index) {
        StringTemp[0] = '\0';
        strcat(StringTemp, Players[player_index].adj_name_faction);
        strcat(StringTemp, " checksum = ");
        text[0] = '\0';
        _itoa(checksums[24 + player_index - 1], number, 10);
        strcat(text, number);
        strcat(text, " ");
        strcat(text, StringTemp);
        log_say_hex(text, checksums[24 + player_index - 1], 0, 0);
    }

    int map_report_index;
    for (map_report_index = 0; map_report_index < 7; ++map_report_index) {
        const char *label;
        switch (map_report_index) {
        case 0: label = "Map climate checksum = "; break;
        case 1: label = "Map region checksum = "; break;
        case 2: label = "Map bit checksum = "; break;
        case 3: label = "Map contour checksum = "; break;
        case 4: label = "Map terr checksum = "; break;
        case 5: label = "Map bitwho[0] checksum = "; break;
        default: label = "Params checksum = "; break;
        }
        text[0] = '\0';
        _itoa(checksums[31 + map_report_index], number, 10);
        strcat(text, number);
        strcat(text, " ");
        strcat(text, label);
        log_say_hex(text, checksums[31 + map_report_index], 0, 0);
    }

    int vehicle_faction;
    for (vehicle_faction = 0; vehicle_faction < MaxPlayerNum; ++vehicle_faction) {
        StringTemp[0] = '\0';
        strcat(StringTemp, Players[vehicle_faction].adj_name_faction);
        strcat(StringTemp, " Vehicles checksum = ");
        text[0] = '\0';
        _itoa(checksums[38 + vehicle_faction], number, 10);
        strcat(text, number);
        strcat(text, " ");
        strcat(text, StringTemp);
        log_say_hex(text, checksums[38 + vehicle_faction], 0, 0);
    }

    text[0] = '\0';
    _itoa(checksums[46], number, 10);
    strcat(text, number);
    strcat(text, " Bases checksum = ");
    log_say_hex(text, checksums[46], 0, 0);
}

extern int TerraformTileFaction;
extern int TerraformDrownedBaseId;

static void world_rainfall_pass2(int saved_faction) {
    int pass2_count = 0;
    int rain_carry = 0;
    int cloud_count = 0;
    int y = 0;
    Map *tile_cursor = MapTiles;

    for (y = 0; y < MapLatitudeBounds; ++y) {
        int x = y & 1;
        for (; x < MapLongitudeBounds; x += 2) {
            int rainfall_class = 0;
            int adjustment = 0;
            int tile_baseline = 0;
            if (y > 0 && y < MapLatitudeBounds - 1) {
                Map *tile = &MapTiles[MapLongitude * y + (x >> 1)];
                const uint8_t climate = tile->climate;
                const uint32_t flags = tile->bit2;
                const int rainfall_level = climate & 7;
                const int altitude = climate >> 5;
                const uint8_t scratch = tile_cursor->unk_1;
                const uint32_t coastal = flags & 0x80000040u;
                if (coastal == 0x40u || (flags & 0x80000100u) == 0x100u)
                    rain_carry = 1;

                if (scratch & 0x20) {
                    rainfall_class = tile_baseline = rain_carry = 1;
                    cloud_count = 0;
                }

                const int cloud_cover = static_cast<int>(MapCloudCover);
                if (scratch & 0x80) {
                    rain_carry = rainfall_level > 1 ? 1 : 0;
                    pass2_count = 0;
                    cloud_count = 0;
                    if (cloud_cover >= 1)
                        adjustment = 1;
                }

                if (scratch & 0x10) {
                    if (rain_carry != 0)
                        --rain_carry;
                    --adjustment;
                }

                if ((scratch & 0x40) && altitude <= 4) {
                    pass2_count = 0;
                    cloud_count = 0;
                    if (adjustment < 0)
                        adjustment = 0;
                    rainfall_class = tile_baseline;
                }

                if (altitude > 3) {
                    ++pass2_count;
                    const int pass_limit =
                        static_cast<int>(WorldBuilder.rainfall_coeff)
                        * cloud_cover * 4 + 1;
                    if (pass2_count > pass_limit && rain_carry != 0)
                        --rain_carry;
                } else if (altitude < 3) {
                    cloud_count = 0;
                    rain_carry = 1;
                    if (pass2_count != 0) {
                        pass2_count += -1 - cloud_cover;
                        if (pass2_count < 1) {
                            pass2_count = 0;
                            cloud_count = 0;
                        }
                    }
                } else if (rain_carry == 0) {
                    if (pass2_count != 0) {
                        pass2_count += -1 - cloud_cover;
                        if (pass2_count < 1) {
                            pass2_count = 0;
                            cloud_count = 0;
                            if (!(cloud_cover == 0 && altitude >= 3))
                                rain_carry = 1;
                        }
                    }
                } else if (pass2_count != 0) {
                    pass2_count += -1 - cloud_cover;
                    if (pass2_count < 1) {
                        pass2_count = 0;
                        cloud_count = 0;
                        if (!(cloud_cover == 0 && altitude >= 3))
                            rain_carry = 1;
                    }
                } else {
                    ++cloud_count;
                    const int cloud_limit = cloud_cover * 4 + 2;
                    if (cloud_count > cloud_limit) {
                        rain_carry = 0;
                        cloud_count = 0;
                    }
                }

                if (tile->bit & BIT_RIVER) {
                    if (((x >> 1) % 3) != 0 && adjustment < 0)
                        adjustment = 0;
                    rain_carry = 1;
                    cloud_count = 0;
                }

                if (scratch & 0x02) {
                    --rainfall_class;
                    rain_carry = 0;
                } else if (rain_carry != 0) {
                    adjustment += rain_carry;
                }

                if (altitude <= 4 + (rainfall_level != 0)) {
                    rainfall_class += adjustment;
                } else if (cloud_cover - altitude + rainfall_level + 5 >= 2
                           && adjustment > 0) {
                    ++rainfall_class;
                }

                if ((flags & 0x80000004u) == 0x4u
                    && (climate & 0xE0) >= 0x60 && altitude < 6) {
                    int marker = static_cast<int8_t>(flags >> 24);
                    if (marker < 0) marker = -marker;
                    if (marker < 0x31) {
                        rainfall_class += 2;
                    } else if (marker < 0x51 && altitude < 5) {
                        rainfall_class += 2;
                    } else {
                        ++rainfall_class;
                    }
                }

                if (coastal == 0x40u)
                    rainfall_class = 0;

                if ((flags & 0x80004000u) == 0x4000u || rainfall_class < 0) {
                    rainfall_class = 0;
                } else if (rainfall_class > 2) {
                    rainfall_class = 2;
                }

                if (scratch & 0x01) {
                    rain_carry = 1;
                    if (rainfall_class < 2)
                        ++rainfall_class;
                }
                if (scratch & 0x04) {
                    rain_carry = 1;
                    if (rainfall_class < 2)
                        ++rainfall_class;
                }

                if (GameDataStateStorage.fields_.turn_current_num_ != 0) {
                    const int old_class = (tile_cursor->climate >> 3) & 3;
                    if (old_class != rainfall_class && TerraformDrownedBaseId < 0) {
                        if (old_class > rainfall_class) {
                            bool search = true;
                            if (TerraformTileFaction >= 0
                                && (static_cast<uint8_t>(
                                        GameDataStateStorage.fields_.lock_masks_.enable_mask_)
                                    & (1u << (TerraformTileFaction & 0x1F))))
                                search = false;
                            if (search) {
                                const int found = base_find(x, y);
                                if (found >= 0)
                                    TerraformTileFaction =
                                        Bases[found].faction_id_current;
                            }
                        }
                        const int found = base_find(x, y, saved_faction);
                        if (found >= 0 && BaseFindDist <= 2)
                            TerraformDrownedBaseId = found;
                    }
                }

                tile_cursor->climate = static_cast<uint8_t>(
                    (tile_cursor->climate & 0xE7) | (rainfall_class << 3));
            } else {
                tile_cursor->climate = static_cast<uint8_t>(tile_cursor->climate & 0xE7);
            }
            ++tile_cursor;
        }
        do_all_non_input();
    }
}

static int world_rainfall_xrange(int x, int width, int flat) {
    if ((flat & 1) == 0) {
        if (x < 0)
            x += width;
        else if (x >= width)
            x -= width;
    }
    return x;
}

// Historical seed deferral, superseded by the reconstruction below:
// NOTE (map pass, 2026-09-05): NOT homed - it stays behind its
//   pending_bodies.cpp forwarder, and this artifact is a CONDENSATION, not a
//   register-level transcription, so homing it means re-deriving pass 2's
//   middle ~80 instructions from the raw bytes first. It does not touch
//   Fractal at all (its only calls are do_all_non_input x3, base_find x2 and
//   climate_set) and it names no new globals, so the Fractal class pass that
//   unblocked world_erosion and world_fungus leaves this one where it was.
//   Its frame is `sub esp, 0x38` with no stack object.
// ORIGINAL: 0x005C4470 ?world_rainfall@@YAXXZ 0x005C4470-0x005C4FC1
// RECOVERY_BINDING: v1 {"reason":"Recover the complete three-pass rainfall algorithm through existing Map storage, WorldBuilder terrain parameters and real base/climate APIs. The common land-terrain join and west-to-east scan continuation come from the original control flow, not the old condensed seed. Restore these original state, callback-read and column-arithmetic effects without normalizing the zero-carry, lock-byte or persistent-cursor quirks. The retained TRIED condensation describes seed history; these branch corrections do not establish full-game or linked-preservation acceptance.","evidence":["original:0x005C4470-0x005C4FC1; image:01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf","git:8ebd9101:src/unrecovered/005c4470.cpp","Original0x5C45AD joins at0x5C47DB before all-tile altitude processing; original0x5C499D/0x5C49A7 join at0x5C49E5 before the east scan.","src/map.h Map and radius tables; src/alpha.h RulesWorldbuilder; src/base.h base_find; src/console.h ConsoleGlobal; existing src/veh.cpp terraform results.","Pinned5C4665/46F3/4774 and4919/49BB/49EB/4A86 reload cloud cover at the consumer and cloudmass scan tails; row callbacks service networking. Entry5C4487/448C resets drowned-base then tile-faction.","Pinned5C4C2A/57/5D/6B separates altitude0..2 carry/reset/counter decay from exactly3;5C4CE2 reaches4C6B without forcing carry.5C4B33 preserves x>>1 through river IDIV3 at4CA3.","Pinned5C4ACF/4B84/4DD7/4E61/4E89 retains a separate Map cursor for scratch, live old/final climate and each processed record, while4B27..49 indexes live terrain storage. Base callbacks may alter climate;5C4DFE tests only the lock-mask low byte."]}
// TRIED: no Ghidra hypothesis was available for this address, so this is a from-scratch disassembly transcription (three map passes: windward-hint marking, 2-bit rainfall-class finalization with base_find caching, moderate-rain propagation). Pass 2's middle heuristic (unk_1 bit cascade feeding the 0-3 class value) is a best-effort condensation of ~80 instructions, not a literal register replay - the surrounding loop bounds, byte-merge write and base_find call sites are transcribed directly.
void __cdecl world_rainfall() {
    const int saved_faction = ConsoleGlobal.faction_id_;
    TerraformDrownedBaseId = -1;
    TerraformTileFaction = -1;
    int i;
    for (i = 0; i < static_cast<int>(MapArea); ++i)
        MapTiles[i].unk_1 = 0;

    int y;
    for (y = 0; y < MapLatitudeBounds; ++y) {
        int x = y & 1;
        for (; x < MapLongitudeBounds; x += 2) {
            Map *tile = &MapTiles[MapLongitude * y + (x >> 1)];
            if (tile->bit & BIT_CONDENSER) {
                for (int dir_cond = 0; dir_cond < 9; ++dir_cond) {
                    int nx = world_rainfall_xrange(
                        x + RadiusOffsetX[dir_cond], MapLongitudeBounds, MapIsFlat);
                    int ny = y + RadiusOffsetY[dir_cond];
                    if (ny >= 0 && ny < MapLatitudeBounds
                        && nx >= 0 && nx < MapLongitudeBounds) {
                        Map *neighbor = &MapTiles[
                            MapLongitude * ny + (nx >> 1)];
                        if (neighbor->unk_1 & 1)
                            neighbor->unk_1 |= 4;
                        else
                            neighbor->unk_1 |= 1;
                    }
                }
            }

            if ((tile->climate & 0xE0) < 0x60) {
                int west = x;
                for (int step_west = 0; step_west < 3; ++step_west) {
                    const int left_x = world_rainfall_xrange(
                        west - 2, MapLongitudeBounds, MapIsFlat);
                    const int center_x = world_rainfall_xrange(
                        west, MapLongitudeBounds, MapIsFlat);
                    if (left_x >= 0 && left_x < MapLongitudeBounds
                        && center_x >= 0 && center_x < MapLongitudeBounds) {
                        Map *left = &MapTiles[
                            MapLongitude * y + (left_x >> 1)];
                        Map *center = &MapTiles[
                            MapLongitude * y + (center_x >> 1)];
                        if ((left->climate >> 5) >= (center->climate >> 5)
                            && step_west <= static_cast<int>(MapCloudCover))
                            left->unk_1 &= 0xEF;
                    }
                    west -= 2;
                }

                const int east_x = world_rainfall_xrange(
                    x + 2, MapLongitudeBounds, MapIsFlat);
                if (east_x >= 0 && east_x < MapLongitudeBounds)
                    MapTiles[MapLongitude * y + (east_x >> 1)].unk_1 |= 0x80;

                const int x4_cloud_cover = static_cast<int>(MapCloudCover);
                if (x4_cloud_cover > 0) {
                    const int x4 = world_rainfall_xrange(
                        x + 4, MapLongitudeBounds, MapIsFlat);
                    if (x4 >= 0 && x4 < MapLongitudeBounds) {
                        Map *tile4 = &MapTiles[
                            MapLongitude * y + (x4 >> 1)];
                        if ((tile4->climate & 7) > 1 || x4_cloud_cover > 1) {
                            tile4->unk_1 |= 0x80;
                            if (static_cast<int>(MapCloudCover) > 1) {
                                const int x6 = world_rainfall_xrange(
                                    x + 6, MapLongitudeBounds, MapIsFlat);
                                if (x6 >= 0 && x6 < MapLongitudeBounds
                                    && (MapTiles[MapLongitude * y + (x6 >> 1)]
                                        .climate & 7) != 0)
                                    MapTiles[MapLongitude * y + (x6 >> 1)]
                                        .unk_1 |= 0x80;
                            }
                        }
                    }
                }
            }

            // Native 0x5C45AD's jge reaches this shared terrain-class block,
            // not the end of the low-altitude ocean/cloud-hint block above.
            const int altitude = tile->climate >> 5;
            if (altitude <= 4) {
                if (altitude >= 3 && (tile->bit & BIT_FOREST)) {
                    for (int row_forest = y - 1; row_forest <= y + 1;
                         ++row_forest) {
                        const int nx = world_rainfall_xrange(
                            x + 1, MapLongitudeBounds, MapIsFlat);
                        if (row_forest >= 0 && row_forest < MapLatitudeBounds
                            && nx >= 0 && nx < MapLongitudeBounds)
                            MapTiles[MapLongitude * row_forest + (nx >> 1)]
                                .unk_1 |= 0x40;
                    }
                    const int nx2 = world_rainfall_xrange(
                        x + 2, MapLongitudeBounds, MapIsFlat);
                    if (nx2 >= 0 && nx2 < MapLongitudeBounds)
                        MapTiles[MapLongitude * y + (nx2 >> 1)]
                            .unk_1 |= 0x40;
                }
            } else {
                const int cloudmass = static_cast<int>(
                    altitude <= 5 ? WorldBuilder.cloudmass_hills
                                  : WorldBuilder.cloudmass_peaks);
                int west_limit = (static_cast<int>(MapCloudCover) - 1)
                    * static_cast<int>(WorldBuilder.rainfall_coeff)
                    + cloudmass;
                if (west_limit >= 1) {
                    int col = x - 2;
                    for (int distance_west = -1;
                         distance_west >= -west_limit; --distance_west) {
                        const int nx = world_rainfall_xrange(
                            col, MapLongitudeBounds, MapIsFlat);
                        if (nx >= 0 && nx < MapLongitudeBounds) {
                            Map *neighbor = &MapTiles[
                                MapLongitude * y + (nx >> 1)];
                            if ((neighbor->climate & 0xE0) < 0x60) {
                                break;
                            }
                            if ((neighbor->climate >> 5) >= altitude) {
                                break;
                            }
                            neighbor->unk_1 |= 0x20;
                        }
                        col -= 2;
                        west_limit = (static_cast<int>(MapCloudCover) - 1)
                            * static_cast<int>(WorldBuilder.rainfall_coeff)
                            + cloudmass;
                    }
                }

                int east_limit = cloudmass
                    - (static_cast<int>(MapCloudCover) - 1)
                      * static_cast<int>(WorldBuilder.rainfall_coeff);
                if (east_limit >= 1) {
                    int col = x + 2;
                    for (int distance_east = 1;
                         distance_east <= east_limit; ++distance_east) {
                        const int nx = world_rainfall_xrange(
                            col, MapLongitudeBounds, MapIsFlat);
                        if (nx >= 0 && nx < MapLongitudeBounds) {
                            Map *neighbor = &MapTiles[
                                MapLongitude * y + (nx >> 1)];
                            if ((neighbor->climate & 0xE0) < 0x60)
                                break;
                            if ((neighbor->climate >> 5) >= altitude)
                                break;
                            neighbor->unk_1 |= 0x10;
                        }
                        col += 2;
                        east_limit = cloudmass
                            - (static_cast<int>(MapCloudCover) - 1)
                              * static_cast<int>(WorldBuilder.rainfall_coeff);
                    }
                }
            }
        }
        do_all_non_input();
    }

    world_rainfall_pass2(saved_faction);

    for (y = 0; y < MapLatitudeBounds; ++y) {
        int x = y & 1;
        if (x < MapLongitudeBounds) {
            for (; x < MapLongitudeBounds; x += 2) {
                Map *tile = &MapTiles[MapLongitude * y + (x >> 1)];
                if ((tile->climate & 0x18) == 0) {
                    for (int dir_neighbor = 0; dir_neighbor < 8;
                         ++dir_neighbor) {
                        const int nx = world_rainfall_xrange(
                            x + VehMoveDeltaX[dir_neighbor], MapLongitudeBounds,
                            MapIsFlat);
                        const int ny = y + VehMoveDeltaY[dir_neighbor];
                        if (ny < 0 || ny >= MapLatitudeBounds
                            || nx < 0 || nx >= MapLongitudeBounds)
                            continue;
                        const Map *neighbor = &MapTiles[
                            MapLongitude * ny + (nx >> 1)];
                        if ((neighbor->climate & 0xE0) < 0x60)
                            continue;
                        if ((neighbor->climate & 0x18) == 0x10) {
                            climate_set(x, y, 1);
                            break;
                        }
                    }
                }
            }
        }
        do_all_non_input();
    }
}

// ORIGINAL: 0x00591290 ?alt_set@@YAXHHH@Z 0x00591290-0x00591894
// RECOVERY_BINDING: v1 {"evidence": ["git:1fe327399e8313c7d34d3b190f391eeeafdb0c28:src/unrecovered/00591290.cpp", "pinned:terranx_original.exe:01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf", "recovery/tasks/game-logic-seed-owner-coverage.json"], "reason": "Recover signed altitude changes through real Map/Base/Veh/faction owners, preserving native submersion, facility, compaction and callback paths. Explicit unsigned shift retains x86 low-byte altitude encoding without C++ signed-shift undefined behavior. Historical mappings and seed lessons are retained; no new byte/layout/runtime/linked-preservation claim."}
// TRIED: no Ghidra hint for this one - straight register-variable/goto transcription from the raw disassembly (44-byte tile struct, 0x833-stride owner tables, round-to-zero /2 idiom, extern lever on the lea-taken 0x00946d54 table). Compiles; diverges at instruction #0 (push vs mov), so the prologue/frame shape itself differs, not yet investigated further.
void __cdecl alt_set(int x, int y, int altitude) {
    static char submerged_title[] = "BASESUBMERGED";
    static char submerged_lost_title[] = "BASESUBMERGED2";

    if (GameDataStateStorage.fields_.turn_current_num_ == 0) {
        return;
    }

    Map *const tile = map_loc(x, y);
    const int old_altitude = (tile->climate & 0xE0) >> 5;
    const int was_water = old_altitude < ALT_SHORE_LINE;
    const int becomes_water = altitude < ALT_SHORE_LINE;

    if (altitude != old_altitude) {
        delete_landmark(x, y);

        if (was_water != becomes_water) {
            tile->bit2 &= 0x7E1377A3u;
            for (int faction = 0; faction < 7; ++faction) {
                tile->bit_visible[faction] = tile->bit;
            }

            if (GameDataStateStorage.fields_.turn_current_num_ != 0 &&
                WorldClimateSkipTerritoryReset == 0 && becomes_water) {
                const int base_id = base_at(x, y);
                if (base_id >= 0 &&
                    !has_fac_built_call(FAC_PRESSURE_DOME, base_id)) {
                    Base &base = Bases[base_id];
                    const int faction_id = base.faction_id_current;

                    const int pressure_cost = (rand() % 3) + 2;
                    const int half_population =
                        static_cast<int>(base.population_size) / 2;
                    int population_loss;
                    if (half_population < pressure_cost ||
                        pressure_cost > 10) {
                        population_loss = pressure_cost;
                    } else if (half_population <= 10) {
                        population_loss = half_population;
                    } else {
                        population_loss = 10;
                    }
                    base.population_size = static_cast<int8_t>(
                        base.population_size - population_loss);

                    set_fac(FAC_PRESSURE_DOME, base_id, 1);
                    ++MapBaseSubmergedCount[faction_id];
                    // The native writer sets this neutral PlayerData flag;
                    // no semantic flag name is established in the owner.
                    PlayersData[faction_id].flags |= 0x00080000u;

                    if (is_objective(base_id) && base.population_size <= 0) {
                        base.population_size = 1;
                    }

                    if (base.population_size <= 0) {
                        if (PlayersData[faction_id].current_num_bases <= 1) {
                            base.population_size = 1;
                            if (VehBattleUnkTgl[0] == 0) {
                                const int local_faction =
                                    ConsoleGlobal.faction_id_;
                                if (faction_id == local_faction) {
                                    ConsoleGlobal.focus(base.x, base.y,
                                                        local_faction);
                                    parse_says(0, base.name_string, -1, -1);
                                    X_pop(submerged_title,
                                                        "subbase_sm.pcx", 0);
                                } else if (base.visibility &
                                           (1u << (local_faction & 0x1F))) {
                                    ConsoleGlobal.focus(base.x, base.y,
                                                        local_faction);
                                    parse_says(0, base.name_string, -1, -1);
                                    parse_says(1,
                                        Players[faction_id].adj_name_faction,
                                        -1, -1);
                                    X_pop(submerged_lost_title,
                                                        "subbase_sm.pcx", 0);
                                }
                            }
                        } else {
                            if ((!IsMultiplayerNet ||
                                 ConsoleControlTurnActive != 0) &&
                                VehBattleUnkTgl[0] == 0) {
                                const int local_faction =
                                    ConsoleGlobal.faction_id_;
                                if (faction_id == local_faction) {
                                    ConsoleGlobal.focus(base.x, base.y,
                                                        local_faction);
                                    parse_says(0, base.name_string, -1, -1);
                                    popp(ScriptPopupFile, "BASESUBMERGED1", 0,
                                         "subbase_sm.pcx", 0);
                                } else if (base.visibility &
                                           (1u << (local_faction & 0x1F))) {
                                    ConsoleGlobal.focus(base.x, base.y,
                                                        local_faction);
                                    parse_says(0, base.name_string, -1, -1);
                                    parse_says(1,
                                        Players[faction_id].adj_name_faction,
                                        -1, -1);
                                    popp(ScriptPopupFile, "BASESUBMERGED3", 0,
                                         "subbase_sm.pcx", 0);
                                }
                            }

                            base_kill(base_id);

                            int vehicle_id = stack_fix(veh_at(x, y));
                            while (vehicle_id >= 0) {
                                Veh &vehicle = Vehs[vehicle_id];
                                int next_vehicle = vehicle.next_veh_id_stack;
                                const int triad = Chassis[
                                    VehPrototypes[vehicle.proto_id].chassis_id
                                ].triad;
                                if (triad != TRIAD_AIR &&
                                    (vehicle.flags & 0x20u) == 0 &&
                                    ((triad == TRIAD_SEA) != becomes_water)) {
                                    const int vehicle_faction =
                                        vehicle.faction_id;
                                    MapBaseIdClosestSubmergedVeh[vehicle_faction] =
                                        base_find(x, y, vehicle_faction);
                                    veh_kill(vehicle_id);
                                    if (next_vehicle > vehicle_id) {
                                        --next_vehicle;
                                    }
                                }
                                vehicle_id = next_vehicle;
                            }
                        }
                    }
                }
            }
        }

        // Land emerging from the sea can gain one rocky level; a land tile
        // that is lowered can lose one. The probability divisors and the
        // asymmetric altitude bounds follow the two native branches.
        if (!becomes_water) {
            const int rockiness = tile->val3 >> 6;
            if (altitude > old_altitude) {
                if (rockiness < ROCKINESS_ROCKY &&
                    rockiness + 1 <= altitude - ALT_SHORE_LINE) {
                    const int divisor = rockiness + 4;
                    if (divisor <= 1 || rand() % divisor == 0) {
                        tile->val3 = static_cast<uint8_t>(
                            ((rockiness + 1) << 6) | (tile->val3 & 0x3F));
                        UnkBitfield1 |= 1;
                    }
                }
            } else if (rockiness > ROCKINESS_FLAT &&
                       rockiness > altitude - ALT_SHORE_LINE) {
                const int divisor = rockiness + 1;
                if (divisor > 1 && rand() % divisor != 0) {
                    tile->val3 = static_cast<uint8_t>(
                        ((rockiness + 3) << 6) | (tile->val3 & 0x3F));
                    UnkBitfield1 |= 1;
                }
            }
        }
    }

    tile->climate = static_cast<uint8_t>((tile->climate & 0x1F) |
                                         (static_cast<uint32_t>(altitude) << 5));
    UnkBitfield1 |= 4;
}


// ORIGINAL: 0x005927D0 ?delete_landmark@@YAXHH@Z 0x005927D0-0x00592938
// RECOVERY_BINDING: v1 {"evidence": ["git:1fe327399e8313c7d34d3b190f391eeeafdb0c28:src/recovered/units/005927d0.cpp", "pinned:terranx_original.exe:01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf", "recovery/tasks/game-logic-seed-owner-coverage.json"], "reason": "Recover landmark clearing through real Map members, shared source-owned lookup arrays and MapWin callbacks. Preserve the duplicate Flat-table clear, numeric9,999,999 cache sentinel, store-before-clear order and native absence of extra window/bounds guards. Historical mappings and seed lessons are retained; no new byte/layout/runtime/linked-preservation claim."}
void __cdecl delete_landmark(int x, int y) {
    Map *tile = map_tiles() + MapLongitude * y + (x >> 1);
    const uint32_t old_flags = tile->bit2;
    if (old_flags == 0) {
        return;
    }

    tile->bit2 = old_flags & ~0x80u;
    const int type = static_cast<int32_t>(tile->bit2) >> 24;
    if (type < 0) {
        return;
    }

    const unsigned char *digits = AltDirectionTable + type * 5;
    if ((old_flags & 0x1u) != 0) {
        for (int i = 0; i < 5; ++i) AltBonusMoist[digits[i]] = 0;
    }
    if ((old_flags & 0x2u) != 0) {
        for (int i = 0; i < 5; ++i) AltBonusArid[digits[i]] = 0;
    }
    if ((old_flags & 0x200u) != 0) {
        for (int i = 0; i < 5; ++i) AltBonusRainy[digits[i]] = 0;
    }
    if ((old_flags & 0x100u) != 0) {
        for (int i = 0; i < 5; ++i) AltBonusFlat[digits[i]] = 0;
    }
    if ((old_flags & 0x800u) != 0) {
        // The native 0x59288C arm clears the flat table again. Keep this
        // duplicate rather than assigning a guessed table to the ridge flag.
        for (int i = 0; i < 5; ++i) AltBonusFlat[digits[i]] = 0;
    }

    const uint32_t encoded_type =
        (static_cast<uint32_t>(-1 - type) & 0xffu) << 24;
    tile->bit2 = (tile->bit2 & 0x00ffffffu) | encoded_type;
    UnkBitfield1 |= 4u;

    for (size_t slot = 0; slot < MapWinTableSlots; ++slot) {
        MapWin *window = MapWinTable[slot];
        if (window->field_1DD74_ != 0) {
            // Native 0x592915 stores immediate 0x0098967F at +8; this is a
            // numeric cache sentinel (9,999,999), not a pointer.
            window->field_8_ = 0x0098967Fu;
            window->clear_alt(x, y, 1);
        }
    }
}
// PeImage: the only words read from rows 5/6 are both initially zero.
// RECOVERY_OBJECT: v1 0x00691930 ?MapTerrainBonusYieldRow5FirstWord@@3HB
extern const int MapTerrainBonusYieldRow5FirstWord = 0;
// RECOVERY_OBJECT: v1 0x00691950 ?MapTerrainBonusYieldRow6FirstWord@@3HB
extern const int MapTerrainBonusYieldRow6FirstWord = 0;

/*
Purpose: One tile's nutrient-bonus rows for the worker placement display:
         select the 32-byte bonus row by the tile bit's row-4 flag, add the
         rainy-tile bonus, the rocky-and-raised double bonus, and the
         special-resource double, and top it off with the rocky bits.

// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x004C99D0 sub_4c99d0 0x004C99D0-0x004C9A43 FILE
// symbol    _sub_4c99d0
// size      115 bytes
// prototype
// callers   0   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
Return Value: the tile's bonus nutrient count
Status: Coverage transcription - unclaimed body
*/
extern "C" int __cdecl sub_4c99d0(int x_coord, int y_coord) {
    const Map *tile = map_tiles() + MapLongitude * y_coord + (x_coord >> 1);
    const uint32_t bits = tile->bit;
    int result = (bits & 4) != 0
        ? MapTerrainBonusYieldRow6FirstWord
        : MapTerrainBonusYieldRow5FirstWord;
    if ((bits & 0x80) != 0) {
        result++;
    }
    if ((bits & 0x20) != 0 && (tile->climate & 0xE0) >= 0x40) {
        result += 2;
    }
    if ((bits & 0x200000) != 0) {
        return result + 2;
    }
    return result + (tile->val3 >> 6);
}
/*
Purpose: Refresh one tile's ownership marks for the local faction: clear
         the tile's claim bits, drop the special-resource flag and the
         tree's clear mask, raise the lake-marker when the tile is not
         rocky-raised terrain, and redraw the tile when the faction sees
         it or the scenario's reveal flag is up.

// ORIGINAL: 0x004CE6B0 sub_4ce6b0 0x004CE6B0-0x004CE78D FILE
// symbol    _sub_4ce6b0
// TRIED: caching the recomputed table index in one local instead of re-reading the row stride fresh each use loses the frame; the cached `half` matches the original's stack-slot reuse of the a2 parameter.
// size      221 bytes
// prototype
// callers   0   call targets   3
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x0046AF40 0x00591D60 0x00591E50 0x005C2020
Return Value: n/a
Status: Coverage transcription - unclaimed body
*/
extern "C" void __cdecl sub_4ce6b0(int x_coord, int y_coord) {
    bit_set(x_coord, y_coord, 0x20, 1);
    bit_set(x_coord, y_coord, 0x28805C, 0);
    bit_set(x_coord, y_coord, MapTileFlagClearMask, 0);

    const Map *tile = map_tiles() + MapLongitude * y_coord + (x_coord >> 1);
    if ((tile->climate & 0xE0) < 0x40) {
        world_alt_set(x_coord, y_coord, 2, 0);
    }

    const int faction = ConsoleGlobal.faction_id_;
    const uint32_t entry = PlayersData[faction].flags;
    if ((entry & 0x200) != 0
        || (tile->visibility & (1 << (faction & 0x1F))) != 0
        || (GameDataStateStorage.fields_.game_state_ & 0x80) != 0) {
        synch_bit(x_coord, y_coord, faction);
        draw_tile(x_coord, y_coord, 2);
    }
}

// Neutral source captions for the initial bytes at 0x687B60 and 0x687B6C: original `push` references at 0x4C9713/0x4C972B and terminating NULs bound 12/17 bytes. The original .data section is writable; original symbols and wider allocation extents are unproved.
// RECOVERY_OBJECT: v2 0x00687B60 0x00687B6C ?ActionBuildLogCaption@@3PADA 0x004C9713
char ActionBuildLogCaption[12] = "Action Base";
// RECOVERY_OBJECT: v2 0x00687B6C 0x00687B7D ?ActionBuildTurnLogCaption@@3PADA 0x004C972B
char ActionBuildTurnLogCaption[17] = "Action Base Turn";

/*
Purpose: Found a base from the built colony pod: log it, clear rock under
         the site, focus the view for the owning faction, settle the
         territory and treaty fallout, run base_init, copy the pod's home
         base's governor settings, name the base, kill the pod and record
         the colony-founded monument trigger.
// HISTORICAL SCAFFOLD ROLE: SCAFFOLD Transitional leaf collection, not completed genuine owner recovery; preserve body evidence during owner migration.
// ORIGINAL: 0x004C96E0 ?action_build@@YAHHPAD@Z 0x004C96E0-0x004C99C5 FILE
// RECOVERY_BINDING: v1 {"evidence": ["source:src/map.cpp typed Vehs/Bases/PlayersData and base/map service declarations", "source callers:Console::new_base, enemy_move, NetDaemon::process_message; each owner includes map.h", "pinned original data at 0x687B60/0x687B6C has bounded NUL-terminated initial spans; former capacity unproved"], "reason": "Own base construction in the map/game-data API because its typed reads and writes use Vehs, Bases, PlayersData and existing map/base services. Keep the two caption arrays bounded to independently evidenced initial bytes."}
// symbol    ?action_build@@YAHHPAD@Z
// TRIED: byte-offset record indexing (a1*0x34) for the 0x952828/2a/2c/30/ 36/4f/56 base-record tables and the 0x20cc-stride diplomacy tables at 0x96c9f4/f8/0x96da3c and 0x134-stride base tables at 0x97d044/4a/53/78, matching Ghidra's decompile; diverges at instr #0 (prologue shape) given the function's size (741 bytes).
// size      741 bytes
Status: Coverage transcription of the retained seed
        (git:65b567af55eb22d163bbd28afcc335c340847278:src/unrecovered/004c96e0.cpp, plain FILE). The veh record reads
        are the typed Vehs[] members (x/y, state, flags, faction_id,
        visibility); the base writes are Bases[] (governor, name_string,
        visibility, faction_id_current); the diplomacy words are
        PlayersData[].last_turn_new_base and .diplo_treaties. Two seeds
        readings are corrected to the pinned layouts: the home-base read
        is Vehs[a1].home_base_id (0x2E, stride 0x34) where the seed's
        unmeasured note carried a 0x1A stride, and the is-human tests are
        faction.h's is_human over LockEnableMask. The original +0x105C
        faction word is PlayerData::current_num_bases. base_init and
        mon_colony_founded use their genuine source owners. veh_skip
        expands from its MEASURED inline
        where the image calls 0x005C1D20 - the accepted house form.
*/

int __cdecl action_build(int veh_id, char *base_name) {
    const int x = Vehs[veh_id].x;
    const int y = Vehs[veh_id].y;
    const unsigned char owner = Vehs[veh_id].faction_id;

    log_say(ActionBuildLogCaption, x, y, owner);
    log_say(ActionBuildTurnLogCaption, GameDataStateStorage.fields_.turn_current_num_, 0,
            0);

    if (rocky_at(x, y) > 0x40) {
        rocky_set(x, y, 1);
    }

    if (owner == ConsoleGlobal.faction_id_ ||
        (Vehs[veh_id].faction_id == ConsoleGlobal.faction_id_ &&
         (Vehs[veh_id].flags & 0x440) != 0x440) ||
        ((1 << (ConsoleGlobal.faction_id_ & 0x1f)) &
         Vehs[veh_id].visibility) != 0 ||
        (GameDataStateStorage.fields_.game_state_ & STATE_OMNISCIENT_VIEW) != 0) {
        ConsoleGlobal.focus(x, y, owner);
    }

    int target = whose_territory(owner, x, y, 0, 0);

    if (PlayersData[owner].current_num_bases != 0 && target >= 0 &&
        static_cast<unsigned int>(target) != owner &&
        (PlayersData[owner].diplo_treaties[target] & 0x11) == 0 &&
        (is_human(owner) || is_human(target))) {
        if ((PlayersData[owner].diplo_treaties[target] & 2) == 0) {
            treaty_on(owner, target, 0x10);
        } else {
            double_cross(owner, target, -1);
        }
    }

    int base_id = base_init(owner, x, y);
    if (base_id >= 0) {
        if (is_human(owner)) {
            const short other_base = Vehs[veh_id].home_base_id;
            if (other_base >= 0) {
                Bases[base_id].governor = Bases[other_base].governor;
            }
            if ((Vehs[veh_id].state & 0x200) != 0) {
                Bases[base_id].governor |= 0x80000000;
            }
        }
        PlayersData[owner].last_turn_new_base = GameDataStateStorage.fields_.turn_current_num_;

        if (base_name != 0 && strlen(base_name) != 0) {
            strcpy(Bases[base_id].name_string, base_name);
            make_base_unique(base_id);
        }

        veh_kill(veh_id);

        if (owner == ConsoleGlobal.faction_id_ ||
            Bases[base_id].faction_id_current == ConsoleGlobal.faction_id_ ||
            (Bases[base_id].visibility &
             (1 << (ConsoleGlobal.faction_id_ & 0x1f))) != 0 ||
            (GameDataStateStorage.fields_.game_state_ & STATE_OMNISCIENT_VIEW) != 0) {
            draw_map(1);
        }

        for (int i = 1; i < 8; i++) {
            del_site(i, 8, x, y, 2);
        }

        mon_colony_founded(owner, Bases[base_id].name_string);
        return base_id;
    }

    veh_skip(veh_id);
    return base_id;
}
