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
#include "win.h"

/*
Purpose: Determine whether a point is inside a rectangle using Win32 edge semantics.
// ORIGINAL: 0x005FA7E0 ?in_box@@YAHHHPAURECT@@@Z 0x005FA7E0-0x005FA814 BYTE_EXACT
// symbol    ?in_box@@YAHHHPBUtagRECT@@@Z
// size      52 bytes
// prototype 
// callers   24   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// notes     Runtime redirect installed by DllMain after byte-signature validation
Return Value: Is the point inside the rectangle? true/false
Status: Complete

// LEVER: guard clauses, not one `&&` chain. The image is four independent
// tests, each with its OWN `xor eax,eax; ret` on failure - not a shared
// early-return block - and the last comparison returns straight out of a
// `setl` with no branch at all. A single boolean expression folds the four
// tests into shared control flow the image does not have.
*/
BOOL __cdecl in_box(int x, int y, const RECT *rect) {
    if (x < rect->left) {
        return FALSE;
    }
    if (x >= rect->right) {
        return FALSE;
    }
    if (y < rect->top) {
        return FALSE;
    }
    return y < rect->bottom;
}

/*
Purpose: Determine whether a point is inside an origin-and-dimensions rectangle.
// ORIGINAL: 0x005FA7A0 ?in_box@@YAHHHHHHH@Z 0x005FA7A0-0x005FA7DB BYTE_EXACT
// LEVER: four SEPARATE early-return guards (`if (x<left) return 0; ... if (x>=right) return 0; if (y<top) return 0; ...`) instead of the merged `x>=right || y<top`, and plain signed `+` instead of the `int_from_bits`/`uint32_t` bit-cast - the image never combines the two mid checks and never needs the cast. 0/25 -> BYTE_EXACT.
// size      59 bytes
// prototype 
// callers   1   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// notes     Runtime redirect installed by DllMain after byte-signature validation
Status: Complete
*/
int __cdecl in_box(int x, int y, int left, int top, int width, int height) {
    if (x < left) {
        return 0;
    }
    const int right = left + width;
    if (x >= right) {
        return 0;
    }
    if (y < top) {
        return 0;
    }
    const int bottom = top + height;
    return y < bottom;
}
