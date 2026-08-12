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
#include "palette.h"

int *PaletteInitialized = reinterpret_cast<int *>(0x009B8178);

/*
Purpose: Convert process palette RGB entries into Windows RGBQUAD order.
ORIGINAL: 0x005FE560
// name      ?get_rgbquad@Palette@@QAEHPAURGBQUAD@@HH@Z
// size      93 bytes
// spans     0x005FE560-0x005FE5BD
// prototype int (__thiscall ?get_rgbquad@Palette@@QAEHPAURGBQUAD@@HH@Z)(Palette* this, RGBQUAD*, int, int)
// callers   7   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// notes     Runtime redirect installed by DllMain after byte-signature validation
Return Value: 3 for a null output, 7 while the palette is unavailable, or 0
Status: Complete
*/
int Palette::get_rgbquad(RGBQUAD *output, int start, int count) {
    if (!output) {
        return 3;
    }
    if (*PaletteInitialized == 0) {
        return 7;
    }
    if (count <= 0) {
        return 0;
    }
    const uint32_t source_bits = reinterpret_cast<uintptr_t>(this)
        + static_cast<uint32_t>(start) * 4U;
    volatile uint8_t *source = reinterpret_cast<volatile uint8_t *>(
        static_cast<uintptr_t>(source_bits));
    volatile uint8_t *destination =
        reinterpret_cast<volatile uint8_t *>(output);
    do {
        const uint8_t red = source[0];
        destination[2] = red;
        const uint8_t green = source[1];
        destination[1] = green;
        const uint8_t blue = source[2];
        destination[0] = blue;
        destination[3] = 0;
        source += 4;
        destination += 4;
        --count;
    } while (count != 0);
    return 0;
}

int __fastcall palette_get_rgbquad_redirect(
        Palette *self, void *, RGBQUAD *output, int start, int count) {
    return self->get_rgbquad(output, start, count);
}

/*
Purpose: Record the window the palette applies to.
ORIGINAL: 0x005FE4F0 BYTE_EXACT
// name      ?set_active_window@Palette@@QAAXPAVWin@@@Z
// size      1 bytes
// spans     0x005FE4F0-0x005FE4F1
// prototype 
// callers   8   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Status: Complete
Verification note: the original body is a single ret - it accepts the window
and does nothing with it. There is no state to observe and no statement to
perturb, so the mutation harness reports no mutants for this function; the
ABI gate covers its calling convention and the smoke gate covers its callers.
*/
void Palette::set_active_window(Win *) {
}

void __cdecl palette_set_active_window_redirect(Win *window) {
    Palette::set_active_window(window);
}

/*
Purpose: Find the slot for a value in the five-entry internal table, returning
         the index of the matching entry or the first empty one, and 5 when the
         table is full without a match.
ORIGINAL: 0x005FED10
// name      ?get_pos@Palette@@QAEHH@Z
// size      37 bytes
// spans     0x005FED10-0x005FED35
// prototype int (__thiscall ?get_pos@Palette@@QAEHH@Z)(Palette* this, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: the slot index, 0 to 5
Status: Complete
*/
int Palette::get_pos(int value) {
    for (int index = 0; index < 5; ++index) {
        const int slot = static_cast<int>(internal_[index].field_0);
        if (slot == value || slot == -1) {
            return index;
        }
    }
    return 5;
}

int __fastcall palette_get_pos_redirect(Palette *self, void *, int value) {
    return self->get_pos(value);
}
