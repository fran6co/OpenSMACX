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

// timer_daemon reads this climate gate before dispatch; its wider contract remains unresolved.
// RECOVERY_OBJECT: v2 0x0068F21C 0x0068F220 ?WorldClimateSkipTerrainClear@@3IA 0x0058E360
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

extern int BaseAtDebugPopupFlag;


