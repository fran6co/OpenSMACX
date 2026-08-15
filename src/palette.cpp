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
#include "general.h"  // mem_get

Palette g_PALETTE1;  // 0x0094C590
HPALETTE PaletteInitialized;  // 0x009B8178
int PaletteUsesSystemColours;  // 0x009B8188

// 0x0067022C - see palette.h. Blue, green, red, reserved, per RGBQUAD.
const uint8_t SystemColours[80] = {
      0,   0,   0, 0,   // 0   black
      0,   0, 128, 0,   // 1   dark red
      0, 128,   0, 0,   // 2   dark green
      0, 128, 128, 0,   // 3   olive
    128,   0,   0, 0,   // 4   dark blue
    128,   0, 128, 0,   // 5   dark magenta
    128, 128,   0, 0,   // 6   teal
    192, 192, 192, 0,   // 7   light grey
    192, 220, 192, 0,   // 8   money green
    240, 202, 166, 0,   // 9   sky blue
    240, 251, 255, 0,   // 246 cream
    164, 160, 160, 0,   // 247 medium grey
    128, 128, 128, 0,   // 248 dark grey
      0,   0, 255, 0,   // 249 red
      0, 255,   0, 0,   // 250 green
      0, 255, 255, 0,   // 251 yellow
    255,   0,   0, 0,   // 252 blue
    255,   0, 255, 0,   // 253 magenta
    255, 255,   0, 0,   // 254 cyan
    255, 255, 255, 0,   // 255 white
};

Palette *PaletteCurrent;  // 0x009B8174

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
    if (PaletteInitialized == 0) {
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

/*
Purpose: Build the process palette and hand it to GDI, replacing whatever
         palette was there before.
ORIGINAL: 0x005FEBB0
// name      ?init_palette_class@Palette@@SAXH@Z
// CORRECTED from ?init_palette_class@Palette@@QAAXH@Z
//   `QAA` is a public NON-STATIC member declared __cdecl - it takes a `this`
//   as its first stack argument. The bytes say there is none. This body reads
//   its `int` at `[esp + 0xc]` after `push ebx` / `push esi`, which is
//   `[entry esp + 4]` - the FIRST argument, where `this` would be. And its
//   only caller settles it independently: `jackal_init_real` is BYTE_EXACT
//   compiled from `Palette::init_palette_class(tgl_direct_draw & 2)`, a call
//   that pushes one argument and no receiver. `SA` is the static spelling,
//   which is what `palette.h` has always declared.
// size      320 bytes
// spans     0x005FEBB0-0x005FECF0
// prototype
// callers   1   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005D4510 0x00644EF2
// indirect  0x005FEBBE 0x005FEBFB 0x005FEC0E 0x005FEC16 0x005FECD6
Return Value: none
Status: Complete
*/
void Palette::init_palette_class(int use_system_colours) {
    HPALETTE existing = PaletteInitialized;
    if (existing) {
        DeleteObject(existing);
        PaletteInitialized = 0;
    }
    uint8_t *header = (uint8_t *)mem_get(0x404);
    if (!header) {
        return;
    }
    *(uint16_t *)header = 0x300;
    *(uint16_t *)(header + 2) = 0x100;
    PaletteUsesSystemColours = use_system_colours;
    if (use_system_colours) {
        HDC screen = GetDC(0);
        GetSystemPaletteEntries(screen, 0, 256, (PALETTEENTRY *)(header + 4));
        ReleaseDC(0, screen);
    } else {
        // The twenty static colours, RGBQUAD (blue, green, red) on the way in
        // and PALETTEENTRY (red, green, blue) on the way out - hence the
        // reversal. Entries 0-9 and 246-255 are the range GDI reserves, and
        // 984 is (246 - 0) * 4: the same walk writes both ends.
        uint8_t *entry = header + 4;
        const uint8_t *colour = SystemColours;
        const uint8_t *high_colour = SystemColours + 40;
        for (int i = 0; i < 10; ++i) {
            entry[0] = colour[2];
            entry[1] = colour[1];
            entry[2] = colour[0];
            entry[3] = 0;
            entry[984] = high_colour[2];
            entry[985] = high_colour[1];
            entry[986] = high_colour[0];
            entry[987] = 0;
            entry += 4;
            colour += 4;
            high_colour += 4;
        }
        // Everything between the two reserved runs is ours to animate.
        uint8_t *flags = header + 0x2F;
        for (int remaining = 236; remaining != 0; --remaining) {
            *flags = 5;  // PC_RESERVED | PC_NOCOLLAPSE
            flags += 4;
        }
        // Four entries are handed back to the system by INDEX: PC_EXPLICIT
        // means peRed holds a system palette slot rather than an intensity.
        uint8_t *reserved = header + 4 + 8 * 4;
        for (int pair = 0; pair < 2; ++pair) {
            reserved[0] = (uint8_t)(pair + 8);
            reserved[1] = 0;
            reserved[2] = 0;
            reserved[3] = 2;  // PC_EXPLICIT
            // BUG IN THE ORIGINAL: entry 246 is given system index 245 and
            // entry 247 index 246, one below each entry's own slot. The four
            // lines above get this right for entries 8 and 9.
            reserved[952] = (uint8_t)(pair - 11);
            reserved[953] = 0;
            reserved[954] = 0;
            reserved[955] = 2;  // PC_EXPLICIT
            reserved += 4;
        }
    }
    PaletteInitialized = CreatePalette((const LOGPALETTE *)header);
    free(header);
}
