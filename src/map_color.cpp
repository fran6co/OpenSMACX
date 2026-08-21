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

// Four mandate colours, one dword every eight bytes; index 3 is the default.
// Isolated from map.cpp so the leaf-test target can compile it without map's
// whole gameplay closure, mirroring src/basepop_font.cpp.

/*
Purpose: Return the display colour for a mandate selector. Selectors 0, 1, and 2
         map to the first three table entries; any other value returns the
         fourth. The original decrements the selector twice against zero, so the
         branch structure is preserved for exactness.
// ORIGINAL: 0x0050E820 ?mandate_color@@YAHH@Z 0x0050E820-0x0050E84D
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
    volatile uint32_t *const table = MandateColors;
    if (mandate == 0) {
        return static_cast<int>(table[0x00 / 4]);
    }
    if (mandate == 1) {
        return static_cast<int>(table[0x08 / 4]);
    }
    if (mandate == 2) {
        return static_cast<int>(table[0x10 / 4]);
    }
    return static_cast<int>(table[0x18 / 4]);
}

int __cdecl mandate_color_redirect(int mandate) {
    return mandate_color(mandate);
}
