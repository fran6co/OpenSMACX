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
#include "buffer.h"   // Buffer::sync_to_palette in set()
#include "random.h"   // random in init()

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
    // 0x404 is 4 + 256 * 4: a LOGPALETTE with a full 256-entry table.
    LOGPALETTE *header = (LOGPALETTE *)mem_get(0x404);
    if (!header) {
        return;
    }
    header->palVersion = 0x300;
    header->palNumEntries = 0x100;
    PaletteUsesSystemColours = use_system_colours;
    if (use_system_colours) {
        HDC screen = GetDC(0);
        GetSystemPaletteEntries(screen, 0, 256, header->palPalEntry);
        ReleaseDC(0, screen);
    } else {
        // The twenty static colours, RGBQUAD (blue, green, red) on the way in
        // and PALETTEENTRY (red, green, blue) on the way out - hence the
        // reversal. Entries 0-9 and 246-255 are the range GDI reserves.
        PALETTEENTRY *entry = header->palPalEntry;
        const uint8_t *colour = SystemColours;
        const uint8_t *high_colour = SystemColours + 40;
        for (int i = 0; i < 10; ++i) {
            entry[0].peRed = colour[2];
            entry[0].peGreen = colour[1];
            entry[0].peBlue = colour[0];
            entry[0].peFlags = 0;
            entry[246].peRed = high_colour[2];
            entry[246].peGreen = high_colour[1];
            entry[246].peBlue = high_colour[0];
            entry[246].peFlags = 0;
            ++entry;
            colour += 4;
            high_colour += 4;
        }
        // Everything between the two reserved runs is ours to animate.
        PALETTEENTRY *free_entry = header->palPalEntry + 10;
        for (int remaining = 236; remaining != 0; --remaining) {
            free_entry->peFlags = 5;  // PC_RESERVED | PC_NOCOLLAPSE
            ++free_entry;
        }
        // Four entries are handed back to the system by INDEX: PC_EXPLICIT
        // means peRed holds a system palette slot rather than an intensity.
        PALETTEENTRY *reserved = header->palPalEntry + 8;
        for (int pair = 0; pair < 2; ++pair) {
            reserved[0].peRed = (uint8_t)(pair + 8);
            reserved[0].peGreen = 0;
            reserved[0].peBlue = 0;
            reserved[0].peFlags = 2;  // PC_EXPLICIT
            // BUG IN THE ORIGINAL: entry 246 is given system index 245 and
            // entry 247 index 246, one below each entry's own slot. The four
            // lines above get this right for entries 8 and 9.
            reserved[238].peRed = (uint8_t)(pair - 11);
            reserved[238].peGreen = 0;
            reserved[238].peBlue = 0;
            reserved[238].peFlags = 2;  // PC_EXPLICIT
            ++reserved;
        }
    }
    PaletteInitialized = CreatePalette(header);
    free(header);
}

/*
ORIGINAL: 0x005FE460
// name      ?set@Palette@@QAEHXZ
// size      139 bytes
// spans     0x005FE460-0x005FE4EB
// prototype int (__thiscall ?set@Palette@@QAEHXZ)(Palette* this)
// callers   12   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005DE8F0
//
// Promoted 2026-08-15 from src/unrecovered/005fe460.cpp to retire its
// pending_bodies forwarder. AnimatePalette is read straight out of the IAT
// slot 0x006690A8, and the DirectDraw-path branch calls through the surface
// vtable at offset 0x18, both as the original does.
Status: Complete
*/
static int *const g_006690a8 = (int *)0x006690A8;
static int *const g_009b7490 = (int *)0x009B7490;
static int *const g_009b8178 = (int *)0x009B8178;
static int *const g_009b8180 = (int *)0x009B8180;
static int *const g_009b8184 = (int *)0x009B8184;
static int *const g_009b8188 = (int *)0x009B8188;
static int *const g_009bc494 = (int *)0x009BC494;
static int *const g_009bc4a0 = (int *)0x009BC4A0;

int Palette::set() {
    if (*g_009bc494 == 0) {
        if (*g_009b8178 == 0) {
            return 7;
        }
        *g_009b8180 = reinterpret_cast<int>(this);
        reinterpret_cast<Buffer *>(g_009b7490)->sync_to_palette(this);
        if (*g_009b8188 == 0) {
            int cur = *reinterpret_cast<int *>(reinterpret_cast<char *>(this) + 0x400);
            if (*g_009b8184 != cur) {
                typedef int(__stdcall *AnimatePaletteFn)(int, int, int, void *);
                AnimatePaletteFn animate = reinterpret_cast<AnimatePaletteFn>(*g_006690a8);
                animate(*g_009b8178, 0xa, 0xec, reinterpret_cast<char *>(this) + 0x28);
                *g_009b8184 = cur;
            }
        }
        return 0;
    } else if (*g_009bc4a0 != 0) {
        int objPtr = *g_009bc4a0;
        int vtbl = *reinterpret_cast<int *>(objPtr);
        typedef int(__stdcall *Fn18)(int, int, int, int, int);
        Fn18 fn = reinterpret_cast<Fn18>(*reinterpret_cast<int *>(vtbl + 0x18));
        fn(objPtr, 0, 0, 0x100, reinterpret_cast<int>(this));
        return 0;
    }
    return 0;
}

/*
ORIGINAL: 0x005FE500
// name      ?close@Palette@@QAEXXZ
// size      84 bytes
// spans     0x005FE500-0x005FE554
// prototype void (__thiscall ?close@Palette@@QAEXXZ)(Palette* this)
// callers   5   call targets   3
// kind      game
// flags     sp_ready;purged_ok
// calls     0x00616200 0x00644EF2 0x0064557F
//
// Promoted 2026-08-15 from src/recovered/units/005fe500.cpp. Tears down the
// five animation entries at this+0x408 (each a Time*, a flag byte pair and a
// heap pointer). Dependency of init() below.
Status: Complete
*/
void Palette::close() {
    char *base = reinterpret_cast<char *>(this) + 0x408;
    int n = 5;
    do {
        Time *t = *reinterpret_cast<Time **>(base);
        *reinterpret_cast<int *>(base - 4) = -1;
        if (t != 0) {
            t->~Time();
            operator delete(t);
            *reinterpret_cast<Time **>(base) = 0;
        }
        void *ptr = *reinterpret_cast<void **>(base + 8);
        if (ptr != 0) {
            free(ptr);
            *reinterpret_cast<void **>(base + 8) = 0;
        }
        *reinterpret_cast<unsigned char *>(base + 4) = 0;
        *reinterpret_cast<unsigned char *>(base + 5) = 0;
        base += 0x10;
    } while (--n);
}

/*
ORIGINAL: 0x005FE330
// name      ?init@Palette@@QAEXXZ
// size      303 bytes
// spans     0x005FE330-0x005FE45F
// prototype void (__thiscall ?init@Palette@@QAEXXZ)(Palette* this)
// callers   5   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005FE500 0x00625810
//
// Promoted 2026-08-15 from src/recovered/units/005fe330.cpp to retire its
// pending_bodies forwarder. GetDC/GetSystemPaletteEntries/ReleaseDC are read
// straight out of their IAT slots; the two table-fill loops and the trailing
// random-seed write are byte-for-byte the original's pointer arithmetic.
Status: Complete
*/
typedef void *(__stdcall *GetDcFn)(void *);
typedef int(__stdcall *GetPalEntriesFn)(void *, int, int, void *);
typedef int(__stdcall *ReleaseDcFn)(void *, void *);

static int *const g_006690ac = (int *)0x006690AC;
static int *const g_0066927c = (int *)0x0066927C;
static int *const g_00669280 = (int *)0x00669280;
static int *const g_0067022c = (int *)0x0067022C;
static int *const g_00670252 = (int *)0x00670252;
static int *const g_00670253 = (int *)0x00670253;
static int *const g_00670254 = (int *)0x00670254;

void Palette::init() {
    close();
    uint32_t self_addr = reinterpret_cast<uint32_t>(this);

    if (*g_009b8188 != 0) {
        void *hdc = (*reinterpret_cast<GetDcFn *>(g_0066927c))(0);
        (*reinterpret_cast<GetPalEntriesFn *>(g_006690ac))(hdc, 0, 0x100, this);
        (*reinterpret_cast<ReleaseDcFn *>(g_00669280))(0, hdc);
    } else {
        uint32_t edi = reinterpret_cast<uint32_t>(g_0067022c) - self_addr;
        uint32_t ebp = reinterpret_cast<uint32_t>(g_00670254) - self_addr;
        uint32_t edx = reinterpret_cast<uint32_t>(g_00670253) - self_addr;
        uint32_t ebx = reinterpret_cast<uint32_t>(g_00670252) - self_addr;
        uint32_t ecx = reinterpret_cast<uint32_t>(g_0067022c);
        uint32_t eax = self_addr + 2;
        do {
            uint8_t bl = *reinterpret_cast<uint8_t *>(edi + eax);
            ecx += 4;
            *reinterpret_cast<uint8_t *>(eax - 2) = bl;
            bl = *reinterpret_cast<uint8_t *>(ecx - 3);
            *reinterpret_cast<uint8_t *>(eax - 1) = bl;
            bl = *reinterpret_cast<uint8_t *>(ecx - 4);
            *reinterpret_cast<uint8_t *>(eax) = bl;
            *reinterpret_cast<uint8_t *>(eax + 1) = 4;
            bl = *reinterpret_cast<uint8_t *>(eax + ebp);
            *reinterpret_cast<uint8_t *>(eax + 0x3d6) = bl;
            bl = *reinterpret_cast<uint8_t *>(eax + edx);
            *reinterpret_cast<uint8_t *>(eax + 0x3d7) = bl;
            bl = *reinterpret_cast<uint8_t *>(eax + ebx);
            *reinterpret_cast<uint8_t *>(eax + 0x3d8) = bl;
            *reinterpret_cast<uint8_t *>(eax + 0x3d9) = 4;
            eax += 4;
        } while (ecx < reinterpret_cast<uint32_t>(g_00670254));

        uint8_t counter = 0xa;
        uint32_t ramp = self_addr + 0x2a;
        do {
            *reinterpret_cast<uint8_t *>(ramp - 2) = counter;
            *reinterpret_cast<uint8_t *>(ramp - 1) = counter;
            *reinterpret_cast<uint8_t *>(ramp) = counter;
            *reinterpret_cast<uint8_t *>(ramp + 1) = 5;
            counter++;
            ramp += 4;
        } while (counter < 0xf6);

        uint32_t index = 0;
        uint32_t last = self_addr + 0x21;
        do {
            uint8_t hi = static_cast<uint8_t>(index);
            last += 4;
            hi += 8;
            *reinterpret_cast<uint8_t *>(last - 5) = hi;
            uint8_t lo = static_cast<uint8_t>(index);
            *reinterpret_cast<uint8_t *>(last - 4) = 0;
            *reinterpret_cast<uint8_t *>(last - 3) = 0;
            lo -= 0xb;
            *reinterpret_cast<uint8_t *>(last - 2) = 2;
            *reinterpret_cast<uint8_t *>(last + 0x3b3) = lo;
            *reinterpret_cast<uint8_t *>(last + 0x3b4) = 0;
            *reinterpret_cast<uint8_t *>(last + 0x3b5) = 0;
            *reinterpret_cast<uint8_t *>(last + 0x3b6) = 2;
            index++;
        } while (index < 2);
    }

    *reinterpret_cast<int32_t *>(self_addr + 0x400) = 0;
    int32_t value;
    do {
        value = random(0, 0xffff);
        *reinterpret_cast<int32_t *>(self_addr + 0x400) = value;
    } while (value == 0);
}
