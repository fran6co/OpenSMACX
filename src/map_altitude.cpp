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
#include "map.h"

// Semantic owner for shared altitude operations: world-generation callers
// remain in map.cpp, while these canonical bodies share one translation unit.

/*
Purpose: Calculate the natural altitude of the specified tile.
// ORIGINAL: 0x005918A0 ?alt_natural@@YAHHH@Z 0x005918A0-0x005918EF
// body      src/map_altitude.cpp
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
int __cdecl alt_natural(int x, int y) {
    // Native SUB wraps the metadata dword, then JGE orders the result and
    // threshold as signed32. These bit conversions use the VC6/x86 contract.
    const int32_t contour = static_cast<int32_t>(
        static_cast<uint32_t>(alt_detail_at(x, y)) - static_cast<uint32_t>(MapSeaLevel));
    int natural = ALT_3_LEVELS_ABOVE_SEA;
    while (contour < static_cast<int32_t>(AltNatural[natural]) && natural) {
        natural--;
    }
    return natural;
}

/*
Purpose: Set the altitude details for the specified tile.
// ORIGINAL: 0x00591260 ?alt_put_detail@@YAXHHH@Z 0x00591260-0x00591288 BYTE_EXACT
// body      src/map_altitude.cpp
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
void __cdecl alt_put_detail(int x, int y, int detail) {
    map_loc(x, y)->contour = detail;
}

/*
Purpose: Set both the altitude and natural altitude for the specified tile. The altitude_natural
         parameter can be between 0 to 9.
// ORIGINAL: 0x005918F0 ?alt_set_both@@YAXHHH@Z 0x005918F0-0x005919C0
// LEVER: historical call_diff work found the image calls `alt_natural`
//   from world_linearize_contours but expands its scan here. Keeping the
//   world caller in map.cpp and this setter with the canonical helpers in
//   this TU expresses that caller boundary while allowing the local helper
//   expansion. The `rnd()` call was replaced with its own body, since a
//   cross-TU call can never inline. 0.513 -> 0.843
//   similar. TRIED beyond that: `Map*`/pointer hoists for
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
