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
#include <ddraw.h>  // IDirectDrawPalette::SetEntries in set()
#include "palette.h"
#include "general.h"  // mem_get
#include "buffer.h"   // Buffer::sync_to_palette in set()
#include "random.h"   // random in init()

Palette g_PALETTE1;  // 0x0094C590
HPALETTE PaletteInitialized;  // 0x009B8178
int PaletteUsesSystemColours;  // 0x009B8188
Palette *PaletteActive;                 // 0x009B8180
Buffer ScreenBuffer;                    // 0x009B7490
int PaletteSeedCache;                   // 0x009B8184
IDirectDrawPalette *DirectDrawPalette;  // 0x009BC4A0

// The HSV colour math's constants, values READ OUT OF THE IMAGE'S .DATA at
// the addresses noted, named from the arithmetic that uses them. cl emits
// its own .rdata copies and the relocation mask makes the addressing match
// the image's absolute references - verified by measure on all three
// bodies, which is why there is no pointer indirection left here.
namespace {
const double hsv_percent_scale = 0.01;              // 0x0066EB38
const double hsv_deg_to_rad = 0.017453292519444445; // pi/180, 0x00670280
const double hsv_zero = 0.0;                        // 0x0066FA70
const double hsv_sector_offset_b_minus_r = 2.0;     // 0x00670C48
const double hsv_sector_offset_r_minus_g = 4.0;     // 0x0066FDC8
const double hsv_hue_wrap_degrees = 360.0;          // 0x00670C30
const double hsv_degrees_per_sector = 60.0;         // 0x00670C40
const double hsv_value_byte_scale = 0.00390625;     // 1/255, 0x00670C50
}

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
Purpose: Take the palette's colours from a decoded DIB and give it a fresh
         generation tag.
// ORIGINAL: 0x005FE650 ?set_from_dib@Palette@@QAEHPAUDib@@@Z 0x005FE650-0x005FE6C7 BYTE_EXACT
// size      120 bytes
// prototype int (__thiscall ?set_from_dib@Palette@@QAEHPAUDib@@@Z)(Palette* this, Dib*)
// callers   1   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00625810
Return Value: 3 for a null DIB, 7 while the palette is unavailable, or 0
Status: Complete

THE 236 IN THE MIDDLE IS THE WHOLE POINT. `esi = 0xec` counts entries 10
through 245, which is every colour EXCEPT the twenty Windows reserves - the
first ten and the last ten, the same split `SystemColours` describes. A DIB
decoded from a PCX brings 256 colours; only the middle 236 are the
palette's to take.

The loop is the RGBQUAD-to-PALETTEENTRY swizzle: `bmiColors` is blue, green,
red, reserved and `entries_` is red, green, blue, flags, so the two ends run
in opposite directions and `peFlags` is left alone.

The tail is `Palette::init`'s seed loop, written out again: zero, then
random until non-zero. That is what makes zero mean "no palette cached" for
every reader of `Buffer::palette_seed_`, and it is why this function is what
`Buffer::load_pcx` calls before syncing.
*/
int Palette::set_from_dib(Dib *dib) {
    if (dib == nullptr) {
        return 3;
    }
    if (PaletteInitialized == 0) {
        return 7;
    }
    for (int entry = 10; entry < 246; ++entry) {
        entries_[entry].peRed = dib->bmiColors[entry].rgbRed;
        entries_[entry].peGreen = dib->bmiColors[entry].rgbGreen;
        entries_[entry].peBlue = dib->bmiColors[entry].rgbBlue;
    }
    seed_ = 0;
    do {
        seed_ = random(0, 0xffff);
    } while (seed_ == 0);
    return 0;
}

/*
Purpose: Convert process palette RGB entries into Windows RGBQUAD order.
// ORIGINAL: 0x005FE560 ?get_rgbquad@Palette@@QAEHPAURGBQUAD@@HH@Z 0x005FE560-0x005FE5BD BYTE_EXACT
// symbol    ?get_rgbquad@Palette@@QAEHPAUtagRGBQUAD@@HH@Z
// size      93 bytes
// prototype int (__thiscall ?get_rgbquad@Palette@@QAEHPAURGBQUAD@@HH@Z)(Palette* this, RGBQUAD*, int, int)
// callers   7   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// notes     Runtime redirect installed by DllMain after byte-signature validation
Return Value: 3 for a null output, 7 while the palette is unavailable, or 0
Status: Complete

INDEX, DO NOT WALK. This used to march two `volatile uint8_t *` cursors,
built by casting `this` to an integer and adding `start * 4` - which is what
`&entries_[start]` says, spelled as arithmetic because the loop predated
knowing that `entries_` starts at offset 0. It reproduced 18 of the 37
instructions.

The image's two loop pointers are BIASED: `eax` is `output + 1` and `ecx` is
`&entries_[start] + 2`, so the stores read `[eax + 1]`, `[eax]`, `[eax - 1]`,
`[eax + 2]`. Marching a pointer never produces that - six spellings were
measured, including references, post-increment, and reordered fields, and
every one of them settled on unbiased pointers and 18 of 37. Indexing
`output[i]` and `entries_[start + i]` off a single counter hands the compiler
the whole address computation, and the bias is what it chooses: 37 of 37.

Every cast went with it, and so did the `volatile`, which was there to stop
the optimiser reordering stores this now expresses directly.
*/
int Palette::get_rgbquad(RGBQUAD *output, int start, int count) {
    if (!output) {
        return 3;
    }
    if (PaletteInitialized == 0) {
        return 7;
    }
    for (int i = 0; i < count; ++i) {
        output[i].rgbRed = entries_[start + i].peRed;
        output[i].rgbGreen = entries_[start + i].peGreen;
        output[i].rgbBlue = entries_[start + i].peBlue;
        output[i].rgbReserved = 0;
    }
    return 0;
}

/*
Purpose: Record the window the palette applies to.
// ORIGINAL: 0x005FE4F0 ?set_active_window@Palette@@QAAXPAVWin@@@Z 0x005FE4F0-0x005FE4F1 BYTE_EXACT
// symbol    ?set_active_window@Palette@@SAXPAVWin@@@Z
// size      1 bytes
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

/*
Purpose: Drop the animation slot registered under the given key.
// ORIGINAL: 0x005FE950 ?UNK3@Palette@@QAEXH@Z 0x005FE950-0x005FEAC4
// symbol    ?remove_animation@Palette@@QAEXH@Z
// size      372 bytes
// prototype void (__thiscall)(Palette* this, int)
// kind      game

MOVED HERE FROM src/recovered/units/005fe950.cpp, half recovered: 5 of the
image's 115 instructions. The MEANING is settled and read off the image -
find the slot by key, delete its `Time`, copy its stored colours back over
`entries_[first .. first+count)`, re-apply the palette through the same
`sync_to_palette` / `AnimatePalette` path `set` uses, free the buffer, then
shift the later slots down so the table stays packed. The name is an
inference from that behaviour, so the catalogue's `UNK3` stays on the
marker and `symbol` carries what this tree emits.
Return Value: n/a
Status: Half recovered - the body is right, the codegen is not
*/
void Palette::remove_animation(int key) {
    int index = get_pos(key);
    if (internal_[index].key != static_cast<uint32_t>(key)) {
        return;
    }
    PaletteInternal &slot = internal_[index];
    if (slot.time) {
        delete slot.time;
        slot.time = nullptr;
    }
    // THE STORED COLOURS GO BACK. `colours` holds what the entries this slot
    // animates looked like before it started.
    memcpy(&entries_[slot.first], slot.colours,
           slot.count * sizeof(PALETTEENTRY));
    if (BufferDirectDraw == nullptr) {
        if (PaletteInitialized) {
            PaletteActive = this;
            ScreenBuffer.sync_to_palette(this);
            if (PaletteUsesSystemColours == 0 && PaletteSeedCache != seed_) {
                AnimatePalette(PaletteInitialized, 10, 236, &entries_[10]);
                PaletteSeedCache = seed_;
            }
        }
    } else if (DirectDrawPalette) {
        DirectDrawPalette->SetEntries(0, 0, 256, entries_);
    }
    if (slot.colours) {
        free(slot.colours);
        slot.colours = nullptr;
    }
    // PACK THE TABLE: every later slot moves down one.
    for (int later = index; later < 4; ++later) {
        internal_[later].key = internal_[later + 1].key;
        internal_[later].first = internal_[later + 1].first;
        internal_[later].count = internal_[later + 1].count;
        internal_[later].time = internal_[later + 1].time;
        internal_[later].colours = internal_[later + 1].colours;
    }
}

/*
Purpose: Find the palette entry nearest the given colour.
// ORIGINAL: 0x005FF280 ?UNK7@Palette@@QAEHHHHHHH@Z 0x005FF280-0x005FF46B
// symbol    ?closest@Palette@@QAEHHHHHHH@Z
// size      491 bytes
// prototype int (__thiscall)(Palette* this, int, int, int, int, int, int)
// kind      game

MOVED HERE FROM src/unrecovered/005ff280.cpp, half recovered. It is not
byte-exact - 3 of the image's 150 instructions - and the divergence is at
instruction 2, where the image saves ONLY `esi` before the early return and
pushes `ebx/ebp/edi` after it while we save three up front. Everything
downstream is shifted by that, so branch-level work does not show; six
source spellings moved it not at all. What IS settled is the meaning, read
off the image: the distance is summed green^2 + red^2 + blue^2 in that
order, the masks are hoisted before the loop, and `skip_animated` selects a
pass that first marks every entry an active slot reserves.

The name is an inference from behaviour, which is why the catalogue's
`UNK7` stays on the marker and `symbol` carries what this tree emits.
Return Value: the nearest entry's index, or 7 if there is no palette
Status: Half recovered - see above
*/
int Palette::closest(int red, int green, int blue, int start, int count,
                     int skip_animated) {
    int best = 200000;
    if (!PaletteInitialized) {
        return 7;
    }
    int best_index = 0;
    const int end = start + count;
    const int want_red = red & 0xFF;
    const int want_green = green & 0xFF;
    const int want_blue = blue & 0xFF;

    if (!skip_animated) {
        for (int index = start; index < end; ++index) {
            const int dr = entries_[index].peRed - want_red;
            const int dg = entries_[index].peGreen - want_green;
            const int db = entries_[index].peBlue - want_blue;
            const int distance = dg * dg + dr * dr + db * db;
            if (distance < best) {
                best = distance;
                best_index = index;
            }
        }
        return best_index;
    }

    // EVERY ENTRY AN ACTIVE SLOT OWNS IS OFF LIMITS. A slot is active when
    // its key is not -1, and it reserves `count` entries from `first`.
    int reserved[256];
    for (int index = 0; index < 256; ++index) {
        reserved[index] = 0;
    }
    for (int slot = 0; slot < 5; ++slot) {
        if (internal_[slot].key != 0xFFFFFFFFu) {
            const unsigned first = internal_[slot].first;
            const unsigned stop = first + internal_[slot].count;
            for (unsigned entry = first; entry < stop; ++entry) {
                reserved[entry] = 1;
            }
        }
    }
    for (index = start; index < end; ++index) {
        if (reserved[index]) {
            continue;
        }
        const int dr = entries_[index].peRed - want_red;
        const int dg = entries_[index].peGreen - want_green;
        const int db = entries_[index].peBlue - want_blue;
        const int distance = dg * dg + dr * dr + db * db;
        if (distance < best) {
            best = distance;
            best_index = index;
        }
    }
    return best_index;
}

/*
Purpose: Find the slot for a value in the five-entry internal table, returning
         the index of the matching entry or the first empty one, and 5 when the
         table is full without a match.
// ORIGINAL: 0x005FED10 ?get_pos@Palette@@QAEHH@Z 0x005FED10-0x005FED35 BYTE_EXACT
// size      37 bytes
// prototype int (__thiscall ?get_pos@Palette@@QAEHH@Z)(Palette* this, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: the slot index, 0 to 5
Status: Complete
*/
int Palette::get_pos(int value) {
    // THE COUNTER IS THE ANSWER. Falling out of the loop leaves `index` at
    // 5, which is the not-found result, so the image returns it rather than
    // loading the constant: no `mov eax, 5` before the epilogue.
    int index = 0;
    for (; index < 5; ++index) {
        const int slot = static_cast<int>(internal_[index].key);
        // `value == slot`, not `slot == value`: the image compares in that
        // order and the two spell `cmp esi, ecx` and `cmp ecx, esi`.
        if (value == slot || slot == -1) {
            break;
        }
    }
    return index;
}

/*
Purpose: Build the process palette and hand it to GDI, replacing whatever
         palette was there before.
// ORIGINAL: 0x005FEBB0 ?init_palette_class@Palette@@SAXH@Z 0x005FEBB0-0x005FECF0
// CORRECTED from ?init_palette_class@Palette@@QAAXH@Z
//   `QAA` is a public NON-STATIC member declared __cdecl - it takes a `this`
//   as its first stack argument. The bytes say there is none. This body reads
//   its `int` at `[esp + 0xc]` after `push ebx` / `push esi`, which is
//   `[entry esp + 4]` - the FIRST argument, where `this` would be. And its
//   only caller settles it independently: `jackal_init_real` is BYTE_EXACT
//   compiled from `Palette::init_palette_class(tgl_direct_draw & 2)`, a call
//   that pushes one argument and no receiver. `SA` is the static spelling,
//   which is what `palette.h` has always declared.
// TRIED: source-form search this body is a REGISTER-PRESSURE plateau, not a
//   control-flow one, and no spelling reached past it. Measured 2026-08-22 over
//   all ten flag sets: best 0.881 at `/c /O2 /Gy /GR- /GX`, 0 of 99 instructions
//   agreeing, this tree emitting 103 against the image's 99. The whole delta is
//   ONE spilled local. The image keeps the LOGPALETTE pointer in esi from
//   0x005FEBD4 to the closing `push esi` at 0x005FECE2 and never touches memory
//   for it; this tree opens with `push ecx` and writes `mov [esp+8], ebp`
//   immediately after the null check, then re-reads it at `mov eax, [esp+0x10]`
//   to seed the 236-entry loop. The cause is one register: the image's `ebx`
//   doubles as the zero constant AND the byte temp inside the reserved-colour
//   loop - `mov bl, [edx+eax]` ... `xor ebx, ebx` ... `mov [eax+0x3da], bl` -
//   where this tree spends `dl` on the temp and keeps `ebx` zeroed, which is one
//   live value too many and pushes the header out to the stack. Every store,
//   every strength-reduced offset (SystemColours+0x22/0x23/0x24 less the header)
//   and both loop bounds already agree; the earlier notes in this block record
//   the spellings that got them there. An earlier pass measured 27 source forms
//   and 13 flag sets against this same wall.
// size      320 bytes
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
        // FULLY INDEXED, not walked. A pointer stepping through the
        // entries makes VC6 pick its own induction variable and the whole
        // loop diverges; indexing both sides off `i` is what `Palette::init`
        // needed too, and what the image emits.
        for (int i = 0; i < 10; ++i) {
            header->palPalEntry[i].peRed = SystemColours[i * 4 + 2];
            header->palPalEntry[i].peGreen = SystemColours[i * 4 + 1];
            header->palPalEntry[i].peBlue = SystemColours[i * 4];
            header->palPalEntry[i].peFlags = 0;
            header->palPalEntry[i + 246].peRed = SystemColours[i * 4 + 42];
            header->palPalEntry[i + 246].peGreen = SystemColours[i * 4 + 41];
            header->palPalEntry[i + 246].peBlue = SystemColours[i * 4 + 40];
            header->palPalEntry[i + 246].peFlags = 0;
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
// ORIGINAL: 0x005FE460 ?set@Palette@@QAEHXZ 0x005FE460-0x005FE4EB BYTE_EXACT
// size      139 bytes
// prototype int (__thiscall ?set@Palette@@QAEHXZ)(Palette* this)
// callers   12   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005DE8F0
//
// Promoted 2026-08-15 from src/unrecovered/005fe460.cpp to retire its
// pending_bodies forwarder, then rewritten from raw self_addr/g_ pointer
// arithmetic into ordinary member and global access: the DirectDraw branch
// publishes through the surface vtable's slot 6, the GDI branch syncs the
// screen buffer and animates the ramp entries when the seed changed.
Status: Complete
*/
int Palette::set() {
    if (BufferDirectDraw) {
        // DirectDraw owns the screen, so the palette is published through
        // IDirectDrawPalette::SetEntries rather than GDI.
        if (DirectDrawPalette) {
            DirectDrawPalette->SetEntries(0, 0, 256, entries_);
        }
        return 0;
    }
    if (PaletteInitialized == 0) {
        return 7;
    }
    PaletteActive = this;
    ScreenBuffer.sync_to_palette(this);
    if (PaletteUsesSystemColours == 0) {
        // Animate only when the seed changed; entries 10-245 are the ramp.
        if (PaletteSeedCache != seed_) {
            AnimatePalette(PaletteInitialized, 10, 236, &entries_[10]);
            PaletteSeedCache = seed_;
        }
    }
    return 0;
}

/*
// ORIGINAL: 0x005FE500 ?close@Palette@@QAEXXZ 0x005FE500-0x005FE554 BYTE_EXACT
// size      84 bytes
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
    // SYMMETRY IS WHAT MAKES THIS EXACT. Reading `time` into a local and
    // nulling the field touches it twice while `colours` - tested, freed and
    // nulled - is touched three times, and VC6 bases the loop on the
    // busier field: every displacement then lands eight bytes off the
    // image's, which reads as SHAPE_EXACT with the right registers and the
    // wrong constants. Treat the two the same way and the base lands where
    // the image puts it, at `this + 0x408`.
    for (int i = 0; i < 5; ++i) {
        internal_[i].key = -1;
        if (internal_[i].time) {
            delete internal_[i].time;
            internal_[i].time = nullptr;
        }
        if (internal_[i].colours) {
            free(internal_[i].colours);
            internal_[i].colours = nullptr;
        }
        internal_[i].first = 0;
        internal_[i].count = 0;
    }
}

/*
// ORIGINAL: 0x005FE330 ?init@Palette@@QAEXXZ 0x005FE330-0x005FE45F BYTE_EXACT
// symbol    ?init@Palette@@QAEHXZ
// size      303 bytes
// prototype void (__thiscall ?init@Palette@@QAEXXZ)(Palette* this)
// callers   5   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005FE500 0x00625810
//
// Promoted 2026-08-15 from src/recovered/units/005fe330.cpp and rewritten
// from raw self_addr pointer arithmetic into ordinary member access. Reloads
// the 256 entries - from the system palette when PaletteUsesSystemColours is
// set, otherwise from the built-in SystemColours plus a grey ramp - then
// picks a fresh non-zero seed.
Status: Complete
*/
int Palette::init() {
    close();
    if (PaletteUsesSystemColours) {
        HDC screen = GetDC(0);
        GetSystemPaletteEntries(screen, 0, 256, entries_);
        ReleaseDC(0, screen);
    } else {
        // The twenty static colours, RGBQUAD (blue, green, red) on the way in
        // and PALETTEENTRY (red, green, blue) on the way out - hence the
        // reversal. Entries 0-9 and 246-255 are the range GDI reserves.
        for (int i = 0; i < 10; ++i) {
            entries_[i].peRed = SystemColours[i * 4 + 2];
            entries_[i].peGreen = SystemColours[i * 4 + 1];
            entries_[i].peBlue = SystemColours[i * 4];
            entries_[i].peFlags = 4;
            entries_[i + 246].peRed = SystemColours[i * 4 + 42];
            entries_[i + 246].peGreen = SystemColours[i * 4 + 41];
            entries_[i + 246].peBlue = SystemColours[i * 4 + 40];
            entries_[i + 246].peFlags = 4;
        }
        // Everything between the two reserved runs is ours to animate; the
        // original fills it with a grey ramp. VC6 leaks a for-loop's declared
        // variable into the enclosing scope, so this loop must not redeclare
        // the `i` the colour loop above already declared.
        for (int grey = 10; grey < 246; ++grey) {
            entries_[grey].peRed = (uint8_t)grey;
            entries_[grey].peGreen = (uint8_t)grey;
            entries_[grey].peBlue = (uint8_t)grey;
            entries_[grey].peFlags = 5;  // PC_RESERVED | PC_NOCOLLAPSE
        }
        // Four entries are handed back to the system by INDEX: PC_EXPLICIT
        // means peRed holds a system palette slot rather than an intensity.
        for (int pair = 0; pair < 2; ++pair) {
            entries_[8 + pair].peRed = (uint8_t)(8 + pair);
            entries_[8 + pair].peGreen = 0;
            entries_[8 + pair].peBlue = 0;
            entries_[8 + pair].peFlags = 2;  // PC_EXPLICIT
            // BUG IN THE ORIGINAL: entry 246 is given system index 245 and
            // entry 247 index 246, one below each entry's own slot.
            entries_[246 + pair].peRed = (uint8_t)(pair - 11);
            entries_[246 + pair].peGreen = 0;
            entries_[246 + pair].peBlue = 0;
            entries_[246 + pair].peFlags = 2;  // PC_EXPLICIT
        }
    }

    // Pick a non-zero generation seed; Buffer caches it to skip republishing
    // an unchanged palette.
    seed_ = 0;
    do {
        seed_ = random(0, 0xffff);
    } while (seed_ == 0);
    return 0;
}


// ---------------------------------------------------------------------------
// DEFINED IN THE HEADER, CLAIMED HERE.
//
// These pieces are written in-class so the image's own inlining reproduces -
// a constructor or destructor the compiler is expected to fold into its
// caller. A marker cannot live beside them: `decomp.reader` globs `*.cpp`
// and `*.c`, and a comparison compiles a TRANSLATION UNIT, so a marker in a
// header could be neither read nor measured. VC6 emits each of them into
// this unit's object as its own COMDAT anyway, which is what the comparison
// pulls out, and the `body` fact says where to go to edit one.
//
// The ratchet still covers the header: this unit includes it, so breaking an
// in-class body here fails the claim below. Measured, not assumed.
// ---------------------------------------------------------------------------

/*
// ORIGINAL: 0x005FE2A0 ??0Palette@@QAE@XZ 0x005FE2A0-0x005FE2D9 BYTE_EXACT
// size      57 bytes
// prototype void (__thiscall ??0Palette@@QAE@XZ)(Palette* this)
// callers   8   call targets   1
// kind      game
// flags     sp_ready;purged_ok
// calls     0x005FE330
*/

Palette::Palette() {
    seed_ = 0;
    for (int i = 0; i < 5; ++i) {
        internal_[i].key = -1;
        internal_[i].time = nullptr;
        internal_[i].first = 0;
        internal_[i].count = 0;
        internal_[i].colours = nullptr;
    }
    init();
}


/*
// ORIGINAL: 0x005FE2E0 ??1Palette@@QAE@XZ 0x005FE2E0-0x005FE2E5 BYTE_EXACT
// body      src/palette.h
// size      5 bytes
// prototype void (__thiscall ??1Palette@@QAE@XZ)(Palette* this)
// callers   18   call targets   0
// kind      thunk
// flags     hidden;thunk;sp_ready;purged_ok
// calls     (none)
*/

/*
Purpose: Restart the timer of the colour-cycling slot that owns `key`.
// ORIGINAL: 0x005FE8B0 ?start_cycle@Palette@@QAEHH@Z 0x005FE8B0-0x005FE8FB BYTE_EXACT
// LEVER: two separate early-exit `if`s, not one chained `key==held || held==-1` - a single OR condition made VC6 peel the loop's first read outside it and rotate the test to the bottom (30 compiled instructions against the image's 27). Two `if`s with the SAME break target reproduces the image's two distinct `je`s off one `mov edx,[esi]`. Also needed a `while (slot < 5)` in place of `do { ... } while (slot < 5)` - the do-while form still peeled even with the two-if split - and `slot` declared BEFORE `entry` (not after): the image's `xor eax,eax` precedes its `lea esi,[ecx+0x404]`, and declaration order is store order here.
// size      75 bytes
// prototype int (__thiscall ?start_cycle@Palette@@QAEHH@Z)(Palette* this, int)
// callers   1   call targets   1
// kind      game
// calls     0x00616650
Return Value: No errors (0); no slot holds that key (0xB)
Status: Complete

PROMOTED FROM src/recovered/units/005fe8b0.cpp, which reached the slots
through `reinterpret_cast<CycleEntry *>(self + 0x404)`.

THE SCAN CAN RUN OFF THE END. Five slots, and the loop leaves `slot` at 5
when none of them matched - the image's `cmp eax, 5; jl` exits with eax == 5
and then reads `[eax*16 + 0x404]`, which is 0x454: one dword past the object.
It compares that against the key and returns 0xB when it differs, which it
almost always will. Reproduced rather than fixed; a bounds check here is a
different program.
*/
int Palette::start_cycle(int key) {
    // A WALKING POINTER AND AN INDEX, both live. The image steps `esi` by
    // 0x10 through the slots and counts in `eax`, then recomputes the
    // address from the INDEX afterwards - `shl eax, 4; add eax, ecx`.
    // Indexing in the loop lets VC6 peel the first iteration and rotate the
    // test to the bottom, which is a different shape entirely.
    int slot = 0;
    const PaletteInternal *entry = internal_;
    while (slot < 5) {
        // Read once, compared twice: `mov edx, [esi]` then two `cmp`s
        // against the register, not two memory compares. TWO SEPARATE
        // `if`s, not one chained `||` - the image jumps to the same target
        // from two distinct `je`s, which is what two early-exit `if`s
        // produce; a single OR condition made VC6 peel the first read
        // outside the loop and rotate the test to the bottom instead.
        const int held = static_cast<int>(entry->key);
        if (key == held) {
            break;
        }
        if (held == -1) {
            break;
        }
        ++slot;
        ++entry;
    }
    if (static_cast<int>(internal_[slot].key) != key) {
        return 0xB;
    }
    internal_[slot].time->start();
    return 0;
}

/*
Purpose: Release the process-wide HPALETTE and clear it, so "initialised" and
         "non-null" stay the same question.
// ORIGINAL: 0x005FECF0 ?close_palette_class@Palette@@QAAXXZ 0x005FECF0-0x005FED0B BYTE_EXACT
// LEVER: PROMOTED out of src/recovered/units/005fecf0.cpp, where the claim proved the ARTIFACT reproduced and the shipped program contained nothing. The artifact reached the HPALETTE through `(int *)0x009B8178` and the destructor through `(*(CleanupFn *)0x00669058)`; both are named here - PaletteInitialized is the object this tree already models, and 0x00669058 is DeleteObject's import thunk, so a plain `DeleteObject` call compiles to the image's own `call dword ptr [0x669058]`.
// symbol    ?close_palette_class@Palette@@SAXXZ
// size      27 bytes
// prototype void (__cdecl ?close_palette_class@Palette@@QAAXXZ)()
// kind      game
Return Value: n/a
Status: Complete
*/
void Palette::close_palette_class() {
    if (PaletteInitialized) {
        DeleteObject(PaletteInitialized);
        PaletteInitialized = nullptr;
    }
}

// ORIGINAL: 0x005FF470 ?get_nearest_palette_index@Palette@@QAEHEEEH@Z 0x005FF470-0x005FF627 FILE
// TRIED: separate loops for the a4==0 (linear scan) and a4!=0 (group-reserved scan) paths with a stack reserved[0x100] array; diverges at #1, stack layout order
// size      439 bytes
// prototype int (__thiscall ?get_nearest_palette_index@Palette@@QAEHEEEH@Z)(Palette* this, unsigned int8, unsigned int8, unsigned int8, int)
// callers   5   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)


int Palette::get_nearest_palette_index(unsigned char a1, unsigned char a2, unsigned char a3, int a4) {
    int best_dist = 200000;
    int best_index = 0;
    char *self = reinterpret_cast<char *>(this);

    if (a4 == 0) {
        unsigned char *p = reinterpret_cast<unsigned char *>(self) + 2;
        for (int i = 0; i < 0x100; ++i, p += 4) {
            int red = (int)p[-2] - (int)a1;
            int green = (int)p[-1] - (int)a2;
            int blue = (int)p[0] - (int)a3;
            int dist = red * red + green * green + blue * blue;
            if (dist < best_dist) {
                best_dist = dist;
                best_index = i;
            }
        }
        return best_index;
    }

    int reserved[0x100];
    for (int k = 0; k < 0x100; ++k) {
        reserved[k] = 0;
    }

    for (int g = 0; g < 5; ++g) {
        int id = *(int *)(self + 0x404 + g * 0x10);
        if (id != -1) {
            unsigned char start = *(unsigned char *)(self + 0x40c + g * 0x10);
            unsigned char count = *(unsigned char *)(self + 0x40d + g * 0x10);
            int end = (int)start + (int)count;
            if ((int)start < end) {
                for (int k = start; k < end; ++k) {
                    reserved[k] = 1;
                }
            }
        }
    }

    unsigned char *p2 = reinterpret_cast<unsigned char *>(self) + 10 * 4 + 2;
    for (int i = 10; i < 0xf6; ++i, p2 += 4) {
        if (reserved[i] != 0) {
            continue;
        }
        int red = (int)p2[-2] - (int)a1;
        int green = (int)p2[-1] - (int)a2;
        int blue = (int)p2[0] - (int)a3;
        int dist = red * red + green * green + blue * blue;
        if (dist < best_dist) {
            best_dist = dist;
            best_index = i;
        }
    }
    return best_index;
}

// ORIGINAL: 0x005FF1A0 ?UNK5@Palette@@QAEHHHHH@Z 0x005FF1A0-0x005FF21C FILE
// symbol    ?make_remap_table@Palette@@QAEHHHHH@Z
// working copy - scaffold materialised by --work
// size      124 bytes
// prototype int (__thiscall ?UNK5@Palette@@QAEHHHHH@Z)(Palette* this, int, int, int, int)
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005FF470

int Palette::make_remap_table(int a1, int a2, int a3, int a4) {
    char *self = reinterpret_cast<char *>(this);
    if (a1 == 0 || a2 == 0) {
        return 0x10;
    }
    char *dest = (char *)a2;
    int i = 0;
    if (0 < a3) {
        do {
            dest[i] = (char)i;
            i = i + 1;
        } while (i < a3);
    }
    int end = a3 + a4;
    for (i = end; i < 0x100; i = i + 1) {
        dest[i] = (char)i;
    }
    if (a3 < end) {
        unsigned char *p = (unsigned char *)(self + 1 + a3 * 4);
        int idx = a3;
        do {
            int result = get_nearest_palette_index(p[-1], p[0], p[1], 1);
            dest[idx] = (char)result;
            idx = idx + 1;
            p = p + 4;
        } while (idx < end);
    }
    return 0;
}

// ORIGINAL: 0x005FE5C0 ?UNK1@Palette@@QAEHHHH@Z 0x005FE5C0-0x005FE646 FILE
// symbol    ?set_rgbquad@Palette@@QAEHHHH@Z
// working copy - scaffold materialised by --work
// size      134 bytes
// prototype int (__thiscall ?UNK1@Palette@@QAEHHHH@Z)(Palette* this, int, int, int)
// callers   2   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00625810

int Palette::set_rgbquad(int a1, int a2, int a3) {
    char *self = reinterpret_cast<char *>(this);
    if (a1 == 0) {
        return 3;
    }
    if (PaletteInitialized == nullptr) {
        return 7;
    }
    if (a3 > 0) {
        unsigned char *dst = reinterpret_cast<unsigned char *>(self + 2 + a2 * 4);
        unsigned char *src = reinterpret_cast<unsigned char *>(a1 + 1);
        int count = a3;
        do {
            dst[-2] = src[1];
            dst[-1] = src[0];
            dst[0] = src[-1];
            dst[1] = 5;
            dst += 4;
            src += 4;
            count--;
        } while (count != 0);
    }
    *reinterpret_cast<int *>(self + 0x400) = 0;
    int result;
    do {
        result = random(0, 0xffff);
        *reinterpret_cast<int *>(self + 0x400) = result;
    } while (result == 0);
    return 0;
}

// ORIGINAL: 0x005FEE80 ?UNK4@Palette@@QAEHPAXHHHHH@Z 0x005FEE80-0x005FEFE7
// symbol    ?make_blend_table@Palette@@QAEHPAXHHHHH@Z
// TRIED: `int i` reused across the two fill loops is a VC6 for-scope leak (C2374), so the loop counters are named `i0`/`i1`. The blend loop's R/G/B channel math and the `get_nearest_palette_index` call are transcribed directly from the Ghidra pseudocode (the CONCAT31 casts there are just "pass the low byte", nothing else). 0.81 mnemonic similarity; first divergence at #3 is in the prologue stack-frame setup for the two 0x400-byte local copies, not chased further.
// size      359 bytes
// prototype int (__thiscall ?UNK4@Palette@@QAEHPAXHHHHH@Z)(Palette* this, void*, int, int, int, int, int)
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005FF470 0x00645930



int Palette::make_blend_table(void *a1, int a2, int a3, int a4, int a5, int a6) {
    if (a1 == 0) {
        return 0x10;
    }
    if (PaletteInitialized == nullptr) {
        return 7;
    }
    if (a2 == 0) {
        return 0x10;
    }

    uint8_t *dest = reinterpret_cast<uint8_t *>(a2);
    for (int i0 = 0; i0 < a3; ++i0) {
        dest[i0] = static_cast<uint8_t>(i0);
    }

    int hi = a3 + a4;
    for (int i1 = hi; i1 < 0x100; ++i1) {
        dest[i1] = static_cast<uint8_t>(i1);
    }

    uint8_t selfCopy[0x400];
    uint8_t srcCopy[0x400];
    memcpy(selfCopy, this, 0x400);
    memcpy(srcCopy, a1, 0x400);

    if (a3 < hi) {
        int divisor = a5 + a6;
        int i = a3;
        do {
            uint8_t r = static_cast<uint8_t>(
                (static_cast<unsigned int>(srcCopy[i * 4]) * a6 +
                 static_cast<unsigned int>(selfCopy[i * 4]) * a5) / divisor);
            selfCopy[i * 4] = r;

            uint8_t g = static_cast<uint8_t>(
                (static_cast<unsigned int>(srcCopy[i * 4 + 1]) * a6 +
                 static_cast<unsigned int>(selfCopy[i * 4 + 1]) * a5) / divisor);
            selfCopy[i * 4 + 1] = g;

            int bSum = static_cast<unsigned int>(srcCopy[i * 4 + 2]) * a6 +
                       static_cast<unsigned int>(selfCopy[i * 4 + 2]) * a5;
            int b = bSum / divisor;
            selfCopy[i * 4 + 2] = static_cast<uint8_t>(b);

            uint8_t idx = static_cast<uint8_t>(
                get_nearest_palette_index(r, g, static_cast<uint8_t>(b), 0));

            ++i;
            dest[i - 1] = idx;
        } while (i < hi);
    }

    return 0;
}

// ORIGINAL: 0x005FEFF0 ?create_table_from_color@Palette@@QAEHHPAEHHHH@Z 0x005FEFF0-0x005FF19E
// TRIED: MISMATCH #3 test/push - Palette has no named per-field layout (all generic field_XX_), so `this` is treated as a raw unsigned char* (the 256-entry colour table, then the 5-record "special colours" table at +0x40c) rather than named members.
// size      430 bytes
// prototype int (__thiscall ?create_table_from_color@Palette@@QAEHHPAEHHHH@Z)(Palette* this, int, unsigned int8*, int, int, int, int)
// callers   2   call targets   1
// kind      game
// flags     sp_ready;purged_ok
// calls     0x005FF470
// To start: tools/decomp_status.py --work 0x005FEFF0


int Palette::create_table_from_color(int a1, unsigned char * a2, int a3, int a4, int a5, int a6) {
    unsigned char *const colors = reinterpret_cast<unsigned char *>(this);

    if (PaletteInitialized == nullptr) {
        return 7;
    }
    if (a2 == 0) {
        return 0x10;
    }

    int i = 0;
    if (0 < a3) {
        do {
            a2[i] = static_cast<unsigned char>(i);
            ++i;
        } while (i < a3);
    }
    int range_end = a3 + a4;
    for (i = range_end; i < 0x100; ++i) {
        a2[i] = static_cast<unsigned char>(i);
    }

    int index = a3;
    uint32_t target = *reinterpret_cast<uint32_t *>(colors + (a1 & 0xff) * 4);

    if (a3 < range_end) {
        int divisor = a5 + a6;
        unsigned char blend[0x400];
        unsigned char *dest = blend + a3 * 4;
        unsigned char *src = colors + 2 + a3 * 4;
        do {
            dest[0] = static_cast<unsigned char>(
                (static_cast<int>(src[-2]) * a5 + static_cast<int>(target & 0xff) * a6) / divisor);
            dest[1] = static_cast<unsigned char>(
                (static_cast<int>(src[-1]) * a5 + static_cast<int>((target >> 8) & 0xff) * a6) / divisor);
            int sum = static_cast<int>(src[0]) * a5 + static_cast<int>((target >> 0x10) & 0xff) * a6;
            int blue = sum / divisor;
            dest[2] = static_cast<unsigned char>(blue);
            unsigned char nearest = static_cast<unsigned char>(
                get_nearest_palette_index(dest[0], dest[1], dest[2], 1));
            src += 4;
            dest += 4;
            a2[index] = nearest;
            ++index;
        } while (index < range_end);
    }

    unsigned char *record = colors + 0x40c;
    int remaining = 5;
    do {
        if (*reinterpret_cast<int32_t *>(record - 8) != -1) {
            unsigned int start = record[0];
            if (start < static_cast<unsigned int>(record[1] + start)) {
                do {
                    a2[start] = static_cast<unsigned char>(start);
                    ++start;
                } while (static_cast<int>(start) < static_cast<int>(record[0] + record[1]));
            }
        }
        record += 0x10;
        --remaining;
    } while (remaining != 0);

    return 0;
}

// ORIGINAL: 0x005FF220 ?UNK6@Palette@@QAEHHHH@Z 0x005FF220-0x005FF277 FILE
// symbol    ?map_to_palette@Palette@@QAEHHHH@Z
// size      87 bytes
// prototype int (__thiscall ?UNK6@Palette@@QAEHHHH@Z)(Palette* this, int, int, int)
// callers   0   call targets   1
// kind      game
// flags     sp_ready;purged_ok
// calls     0x005FF280
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x005FF220
// measured tier  MISMATCH
// divergence     9
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/005ff220/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?UNK6@Palette@@QAEHHHH@Z  at 0x005FF220  (87 bytes)
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


int Palette::map_to_palette(int a1, int a2, int a3) {
    if (a1 == 0) {
        return 0x10;
    }
    char *self = reinterpret_cast<char *>(this);
    int *dst = reinterpret_cast<int *>(a1 + 0x28);
    unsigned char *src = reinterpret_cast<unsigned char *>(self + 0x29);
    int count = 0xec;
    do {
        int idx = closest(src[-1], src[0], src[1], a2, a3, 1);
        src += 4;
        *dst = *reinterpret_cast<int *>(self + idx * 4);
        dst += 1;
    } while (--count);
    return 0;
}

// ORIGINAL: 0x005FEAD0 ?timer_callback@Palette@@QAAXHH@Z 0x005FEAD0-0x005FEBA8 FILE
// symbol    ?timer_callback@Palette@@SAXHH@Z
// TRIED: 228 vs 216 bytes; the extra named locals (idx/found/entries/ iStartIndex/cEntries/i) push more callee-saved registers before the first `mov ebp,a2`, where the original loads a2 into ebp before any other push. Did not try collapsing to fewer locals given budget.
// working copy - scaffold materialised by --work
// size      216 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x005FEB91


void __cdecl Palette::timer_callback(int a1, int a2) {
    if (a2 == 0 || PaletteUsesSystemColours != 0) {
        return;
    }

    char *base = reinterpret_cast<char *>(a2);
    int idx = 0;
    char *entry = base + 0x404;
    do {
        int field0 = *reinterpret_cast<int *>(entry);
        if (a1 == field0 || field0 == -1) {
            break;
        }
        idx++;
        entry += 0x10;
    } while (idx < 5);

    char *found = base + idx * 0x10;
    if (*reinterpret_cast<int *>(found + 0x404) != a1) {
        return;
    }

    unsigned int iStartIndex = *reinterpret_cast<unsigned char *>(found + 0x40c);
    unsigned int cEntries = *reinterpret_cast<unsigned char *>(found + 0x40d);

    PALETTEENTRY *entries = reinterpret_cast<PALETTEENTRY *>(base + iStartIndex * 4);
    unsigned char lastBlue = entries[0].peBlue;
    unsigned char lastGreen = entries[0].peGreen;
    unsigned char lastRed = entries[0].peRed;

    unsigned int i;
    for (i = 0; i < cEntries - 1; i++) {
        entries[i].peRed = entries[i + 1].peRed;
        entries[i].peGreen = entries[i + 1].peGreen;
        entries[i].peBlue = entries[i + 1].peBlue;
    }

    entries[cEntries - 1].peRed = lastRed;
    entries[cEntries - 1].peGreen = lastGreen;
    entries[cEntries - 1].peBlue = lastBlue;

    AnimatePalette(
        PaletteInitialized, iStartIndex, cEntries, entries);

    PaletteSeedCache = *reinterpret_cast<int *>(base + 0x400);
}

// ORIGINAL: 0x005FF930 ?UNK8@Palette@@QAEHHHHHH@Z 0x005FF930-0x005FFB0B
// symbol    ?fade_to@Palette@@QAEHHHHHH@Z
// TRIED: the third RGB channel is written through a pair of pointers precomputed once before the loop (`stackA_adj`/`stackB_adj` plus the destination pointer) rather than a fresh `k*4+2` index each iteration; kept as a direct index into the two 1024-byte stack copies, which is semantically the same value. `seed_` at offset 0x400 (right after `field_3FC_`) matches the reseed loop exactly. Landing the closest control-flow- faithful form (divergence starts at instruction #2, on the `this`-copy prologue).
// size      475 bytes
// prototype int (__thiscall ?UNK8@Palette@@QAEHHHHHH@Z)(Palette* this, void*, UINT iStartIndex, UINT cEntries, int, int)
// callers   0   call targets   2
// kind      game
// flags     sp_ready;purged_ok
// calls     0x00625810 0x00645930
// indirect  0x005FF9D2 0x005FFA94 0x005FFA9A 0x005FFAB1



int Palette::fade_to(int a1, int a2, int a3, int a4, int a5) {
  if (PaletteUsesSystemColours != 0) {
    return 0;
  }
  if (PaletteInitialized == nullptr) {
    return 7;
  }
  if (a1 == 0) {
    return 0x10;
  }

  unsigned char stackA[0x400];
  unsigned char stackB[0x400];
  memcpy(stackA, this, 0x400);
  memcpy(stackB, reinterpret_cast<void *>(a1), 0x400);

  

  int frame = 0;
  if (a4 != -1 && a4 + 1 >= 0) {
    int weight = a4;
    do {
      DWORD t0 = timeGetTime();
      if (a2 < a2 + a3) {
        unsigned char *dst =
            reinterpret_cast<unsigned char *>(a1) + 2 + a2 * 4;
        int k = a2;
        do {
          dst[-2] = static_cast<char>(
              (static_cast<unsigned int>(stackA[k * 4]) * weight +
               static_cast<unsigned int>(stackB[k * 4]) * frame) /
              a4);
          dst[-1] = static_cast<char>(
              (static_cast<unsigned int>(stackA[k * 4 + 1]) * weight +
               static_cast<unsigned int>(stackB[k * 4 + 1]) * frame) /
              a4);
          dst[0] = static_cast<char>(
              (static_cast<unsigned int>(stackA[k * 4 + 2]) * weight +
               static_cast<unsigned int>(stackB[k * 4 + 2]) * frame) /
              a4);
          ++k;
          dst += 4;
        } while (k < a2 + a3);
      }

      AnimatePalette(PaletteInitialized, a2, a3,
                      reinterpret_cast<const PALETTEENTRY *>(a1 + a2 * 4));

      DWORD t1 = timeGetTime();
      unsigned int elapsed = t1 - t0;
      while (elapsed < static_cast<unsigned int>(a5)) {
        t1 = timeGetTime();
        elapsed = t1 - t0;
      }

      ++frame;
      --weight;
    } while (frame < a4 + 1);
  }

  seed_ = 0;
  int r;
  do {
    r = random(0, 0xffff);
    seed_ = r;
  } while (r == 0);

  return 0;
}

// ORIGINAL: 0x005FE700 ?init_cycle@Palette@@QAEHHHHK@Z 0x005FE700-0x005FE8AC;0x00662D60-0x00662D75 FILE
// TRIED: plain 'new Time()'/'operator delete'/'free' expressions relying on VC6's own SEH codegen instead of hand-written __try/__finally; diverges at #1, prologue register order
// size      449 bytes
// prototype int (__thiscall ?init_cycle@Palette@@QAEHHHHK@Z)(Palette* this, int, int, int, unsigned int)
// callers   1   call targets   8
// kind      game
// flags     sp_ready;purged_ok
// calls     0x005D4510 0x006161D0 0x00616200 0x006162D0 0x00644EF2 0x0064557F 0x0064558A 0x00645930


int Palette::init_cycle(int a1, int a2, int a3, unsigned long a4) {
    char *self = reinterpret_cast<char *>(this);
    unsigned int a2masked = (unsigned char)a2;

    if (a2masked < 10) {
        return 3;
    }
    if ((int)(0xf6 - a2masked) < (int)a4) {
        return 3;
    }
    if (a1 == -1) {
        return 3;
    }
    if (PaletteInitialized == nullptr) {
        return 7;
    }

    int slot = 0;
    while (slot < 5) {
        int id = *(int *)(self + 0x404 + slot * 0x10);
        if (a1 == id || id == -1) {
            break;
        }
        ++slot;
    }

    char *entry = self + 0x404 + slot * 0x10;

    if (*(int *)entry == a1) {
        Time *t = *(Time **)(entry + 4);
        if (t != 0) {
            t->~Time();
            operator delete(t);
            *(Time **)(entry + 4) = 0;
        }
        char *extra = self + (slot + 0x41) * 0x10;
        void *p = *(void **)extra;
        if (p != 0) {
            free(p);
            *(void **)extra = 0;
        }
    }

    *(int *)entry = a1;
    *(unsigned char *)(entry + 9) = (unsigned char)a4;
    *(unsigned char *)(entry + 8) = (unsigned char)a2masked;

    void *mem = mem_get(a4 * 4);
    char *extra = self + (slot + 0x41) * 0x10;
    *(void **)extra = mem;
    if (mem == 0) {
        return 4;
    }

    memcpy(mem, self + a2masked * 4, a4 * 4);

    Time *t = new Time();
    *(Time **)(entry + 4) = t;
    if (t == 0) {
        return 4;
    }

    t->init(Palette::timer_callback, a1, (int)self, (int)a4, 5);
    return 0;
}

// ORIGINAL: 0x005FFB10 ?UNK9@Palette@@QAEHHHHHH@Z 0x005FFB10-0x005FFD7F FILE
// symbol    ?fade_to_entry@Palette@@QAEHHHHHH@Z
// TRIED: flipping the outer if/else polarity to test BufferDirectDraw != nullptr first (matching the disasm's fall-through order literally) moved the divergence from #13 (a lone je/jne polarity swap) to #0 (a different prologue entirely - no `sub esp,0x428`), so the `== 0`-first form below is the better body. Full control flow, field offsets and the interpolation formula (backup*stepsRemaining + target*progressCounter)/a4 were hand-traced from the raw disasm stack-offset arithmetic, not from Ghidra's decompile, which mis-simplifies the backup-buffer pointer trick.
// working copy - scaffold materialised by --work
// size      623 bytes
// prototype int (__thiscall ?UNK9@Palette@@QAEHHHHHH@Z)(Palette* this, UINT, UINT iStartIndex, UINT cEntries, int, int)
// callers   0   call targets   3
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005DE8F0 0x00625810 0x00645930
// indirect  0x005FFB82 0x005FFBCC 0x005FFC18 0x005FFD05 0x005FFD0B 0x005FFD1A

int Palette::fade_to_entry(int a1, int a2, int a3, int a4, int a5) {
    if (PaletteInitialized == nullptr) {
        return 7;
    }
    if (PaletteUsesSystemColours != 0) {
        return 0;
    }

    unsigned char *self = reinterpret_cast<unsigned char *>(this);
    unsigned int targetColor = *reinterpret_cast<unsigned int *>(self + a1 * 4);
    unsigned char backup[0x400];
    memcpy(backup, this, 0x400);

    if (BufferDirectDraw == nullptr) {
        if (PaletteInitialized != nullptr) {
            PaletteActive = this;
            reinterpret_cast<Buffer *>(0x009B7490)->sync_to_palette(this);
            if (PaletteUsesSystemColours == 0 &&
                PaletteSeedCache != *reinterpret_cast<int *>(self + 0x400)) {
                AnimatePalette(PaletteInitialized, 10, 0xec,
                               reinterpret_cast<PALETTEENTRY *>(self + 0x28));
                PaletteSeedCache = *reinterpret_cast<int *>(self + 0x400);
            }
        }
    } else if (DirectDrawPalette != nullptr) {
        DirectDrawPalette->SetEntries(0, 0, 0x100,
                                      reinterpret_cast<PALETTEENTRY *>(this));
    }

    if (a4 + 1 > 0) {
        unsigned int targetR = targetColor & 0xff;
        unsigned int targetG = (targetColor >> 8) & 0xff;
        unsigned int targetB = (targetColor >> 16) & 0xff;
        int stepsRemaining = a4;
        int progressCounter = 0;
        do {
            unsigned int tick1 = timeGetTime();
            int endIndex = a2 + a3;
            if (a2 < endIndex) {
                unsigned char *dst = self + a2 * 4;
                unsigned char *src = backup + a2 * 4;
                int count = endIndex - a2;
                int r = targetR * progressCounter;
                int g = targetG * progressCounter;
                int b = targetB * progressCounter;
                do {
                    dst[0] = (unsigned char)((src[0] * stepsRemaining + r) / a4);
                    dst[1] = (unsigned char)((src[1] * stepsRemaining + g) / a4);
                    dst[2] = (unsigned char)((src[2] * stepsRemaining + b) / a4);
                    dst += 4;
                    src += 4;
                    --count;
                } while (count != 0);
            }
            AnimatePalette(PaletteInitialized, a2, a3,
                           reinterpret_cast<PALETTEENTRY *>(self + a2 * 4));
            unsigned int tick2 = timeGetTime();
            while (tick2 - tick1 < (unsigned int)a5) {
                tick2 = timeGetTime();
            }
            ++progressCounter;
            --stepsRemaining;
        } while (progressCounter < a4 + 1);
    }

    *reinterpret_cast<int *>(self + 0x400) = 0;
    int newSeed;
    do {
        newSeed = (int)random(0, 0xffff);
        *reinterpret_cast<int *>(self + 0x400) = newSeed;
    } while (newSeed == 0);

    return 0;
}

// ORIGINAL: 0x00628DB0 ?RGB_to_HSV@@YAXPAUPALETTEENTRY@@PAUHSV@@@Z 0x00628DB0-0x00628F27 FILE
// symbol    ?RGB_to_HSV@@YAXPAUtagPALETTEENTRY@@PAUHSV@@@Z
// size      375 bytes
// prototype 
// callers   3   call targets   0
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     (none)
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x00628DB0
// measured tier  MISMATCH
// divergence     2
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/00628db0/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?RGB_to_HSV@@YAXPAUPALETTEENTRY@@PAUHSV@@@Z  at 0x00628DB0  (375 bytes)
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

void __cdecl RGB_to_HSV(PALETTEENTRY * a1, HSV * a2) {
    unsigned char *p = reinterpret_cast<unsigned char *>(a1);
    double *out = reinterpret_cast<double *>(a2);
    if (a2 != 0 && a1 != 0) {
        const double scale = hsv_value_byte_scale;
        const double zero = hsv_zero;
        int ir = p[0];
        int ig = p[1];
        int ib = p[2];
        double r = static_cast<double>(ir) * scale;
        double g = static_cast<double>(ig) * scale;
        double b = static_cast<double>(ib) * scale;
        double vmax = r;
        if (r < g) vmax = g;
        if (vmax < b) vmax = b;
        double vmin = r;
        if (g < r) vmin = g;
        if (b < vmax) vmin = b;
        out[2] = vmax;
        double s = zero;
        if (vmax != zero) s = (vmax - vmin) / vmax;
        out[1] = s;
        if (s == zero) {
            *reinterpret_cast<int *>(out) = 0;
            *(reinterpret_cast<int *>(out) + 1) = static_cast<int>(0xbff00000);
            return;
        }
        double delta = vmax - vmin;
        double h;
        if (r == vmax) {
            h = (g - b) / delta;
        } else if (g == vmax) {
            h = (b - r) / delta + hsv_sector_offset_b_minus_r;
        } else {
            h = (r - g) / delta + hsv_sector_offset_r_minus_g;
        }
        out[0] = h;
        double h2 = out[0] * hsv_degrees_per_sector;
        bool neg = h2 < zero;
        out[0] = h2;
        if (neg) {
            out[0] = h2 + hsv_hue_wrap_degrees;
        }
    }
}

// ORIGINAL: 0x005FF630 ?get_nearest_palette_index@Palette@@QAEHPAUHSV@@0H@Z 0x005FF630-0x005FF92D FILE
// TRIED: MISMATCH #5 push/mov - and/sub-esp frame differs because the helper hsv_sq_distance() is a separate function call rather than inlined FPU code sharing one scratch stack slot per the original's single sub-esp-8 staging area reused across both sin/cos call sites in each loop body.
// working copy - scaffold materialised by --work
// size      765 bytes
// prototype int (__thiscall ?get_nearest_palette_index@Palette@@QAEHPAUHSV@@0H@Z)(Palette* this, HSV*, HSV*, int)
// callers   1   call targets   2
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x006463E4 0x00646494

// The record name FF630's recovered body thinks in; same 0x18 layout as
// struct HSV above.
namespace {
struct HsvRecord {
    double hue_;
    double sat_;
    double val_;
};
}

static double hsv_sq_distance(const HsvRecord *a, const HsvRecord *b, double k) {
    double dv = a->val_ - b->val_;
    double sin_a = ((double(__cdecl *)(double))sin)(a->hue_ * k) * a->sat_;
    double sin_b = ((double(__cdecl *)(double))sin)(b->hue_ * k) * b->sat_;
    double cos_a = ((double(__cdecl *)(double))cos)(a->hue_ * k) * a->sat_;
    double cos_b = ((double(__cdecl *)(double))cos)(b->hue_ * k) * b->sat_;
    double ds = sin_a - sin_b;
    double dc = cos_a - cos_b;
    return dv * dv + ds * ds + dc * dc;
}

int Palette::get_nearest_palette_index(HSV * a1, HSV * a2, int a3) {
    double k = hsv_deg_to_rad;
    double best = 200000.0;
    int best_index = 0;
    int i;

    if (PaletteInitialized == nullptr) {
        return 7;
    }

    const HsvRecord *query = (const HsvRecord *)a1;

    if (a3 != 0) {
        int used[256];
        for (i = 0; i < 0x100; i++) {
            used[i] = 0;
        }

        for (i = 0; i < 5; i++) {
            if (internal_[i].key != 0xffffffff) {
                unsigned int start = internal_[i].first;
                unsigned int len = internal_[i].count;
                unsigned int end = start + len;
                if (start < end) {
                    unsigned int j;
                    for (j = start; j < end; j++) {
                        used[j] = 1;
                    }
                }
            }
        }

        {
            const HsvRecord *arr = (const HsvRecord *)((char *)a2 + 0xf8) + 10;
            for (i = 10; i < 0xf6; i++) {
                if (used[i] == 0) {
                    double d = hsv_sq_distance(query, arr, k);
                    if (d < best) {
                        best = d;
                        best_index = i;
                    }
                }
                arr++;
            }
        }
        return best_index;
    }

    {
        const HsvRecord *arr = (const HsvRecord *)((char *)a2 + 8);
        for (i = 0; i < 0x100; i++) {
            double d = hsv_sq_distance(query, arr, k);
            if (d < best) {
                best = d;
                best_index = i;
            }
            arr++;
        }
    }
    return best_index;
}

// ORIGINAL: 0x005FED40 ?create_table@Palette@@QAEHPAEHHH@Z 0x005FED40-0x005FEE78
// TRIED: manual dword-by-dword copy loop for the HSV struct (compiler reordered vs. the `rep movsd` struct-assignment form used below); computing `&hsv[0]` at the call site instead of before the loop. Best reached: MISMATCH, edit_count 4/~110 mnemonics, 313 bytes vs 312 - one `fmul qword ptr [addr]` compiled as `mov eax,addr; fmul qword ptr [eax]` instead of the direct absolute form, apparently because `&hsv[0]`'s address happened to occupy eax at that point in the original and not here.
//            2026-08-23, two more shapes measured from the homed body: spelling the
//            constant as *(double *)0x0066EB38 lowers byte-identically (0.9626), and
//            hoisting a &local1 pointer ahead of the loop to busy eax does not compile
//            as written - the lever is whatever makes VC6 compute the call's argument
//            addresses BEFORE the v-multiply, which is an ordering this tree has not found.
// size      312 bytes
// prototype int (__thiscall ?create_table@Palette@@QAEHPAEHHH@Z)(Palette* this, unsigned int8*, int, int, int)
// callers   1   call targets   3
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005FF630 0x00628DB0 0x00645550

int Palette::create_table(unsigned char * a1, int a2, int a3, int a4) {
    struct HSVLocal { double h; double s; double v; };
    HSVLocal hsv[256];
    HSVLocal local1;
    HSVLocal *ref0;
    int i;
    int upper;

    if (PaletteInitialized == nullptr) {
        return 7;
    }
    if (a1 == 0) {
        return 0x10;
    }
    if (a4 < -100 || a4 > 100) {
        return 3;
    }
    for (i = 0; i < a2; i++) {
        a1[i] = (unsigned char)i;
    }
    upper = a2 + a3;
    for (i = upper; i < 0x100; i++) {
        a1[i] = (unsigned char)i;
    }
    for (i = 0; i < 0x100; i++) {
        RGB_to_HSV((PALETTEENTRY *)((unsigned char *)this + i * 4), (HSV *)&hsv[i]);
    }
    ref0 = &hsv[0];
    for (i = a2; i < upper; i++) {
        local1 = hsv[i];
        local1.v = (double)(a4 + 100) * local1.v * hsv_percent_scale;
        a1[i] = (unsigned char)get_nearest_palette_index((HSV *)&local1, (HSV *)ref0, 1);
    }
    return 0;
}

// ORIGINAL: 0x005FE6D0 ?reseed@Palette@@QAEXXZ 0x005FE6D0-0x005FE6F8 BYTE_EXACT
// size      40 bytes
// prototype void (__thiscall ?reseed@Palette@@QAEXXZ)(Palette* this)
// callers   3   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00625810
// 0x005FE6D0  ?reseed@Palette@@QAEXXZ  ->  ?reseed@Palette@@QAEXXZ
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.


void Palette::reseed() {
    char *self = reinterpret_cast<char *>(this);
    uint32_t *seed = reinterpret_cast<uint32_t *>(self + 0x400);
    *seed = 0;
    while (*seed == 0) {
        *seed = random(0, 0xffff);
    }
}

// ORIGINAL: 0x005FE900 ?UNK2@Palette@@QAEXH@Z 0x005FE900-0x005FE941 FILE BYTE_EXACT
// symbol    ?stop_animation@Palette@@QAEXH@Z
// size      65 bytes
// prototype void (__thiscall ?UNK2@Palette@@QAEXH@Z)(Palette* this, int)
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00616730
// PRESERVED UNIT - measured BYTE_EXACT.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x005FE900
// measured tier  BYTE_EXACT
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/005fe900/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?UNK2@Palette@@QAEXH@Z  at 0x005FE900  (65 bytes)
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


void Palette::stop_animation(int a1) {
    char *self = reinterpret_cast<char *>(this);
    int i;
    for (i = 0; i < 5; i++) {
        int v = *reinterpret_cast<int *>(self + 0x404 + i * 0x10);
        if (a1 == v || v == -1) {
            break;
        }
    }
    char *found = self + i * 0x10;
    if (*reinterpret_cast<int *>(found + 0x404) == a1) {
        reinterpret_cast<Time *>(*reinterpret_cast<int *>(found + 0x408))->stop();
    }
}

// ORIGINAL: 0x005FE2F0 ?__as@Palette@@QAEHQAUPalette@@@Z 0x005FE2F0-0x005FE32F FILE BYTE_EXACT
// symbol    ??4Palette@@QAEAAV0@PAV0@@Z
// size      63 bytes
// prototype int (__thiscall ?__as@Palette@@QAEHQAUPalette@@@Z)(Palette* this, Palette*)
// callers   2   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00625810 0x00645930
// PRESERVED UNIT - measured BYTE_EXACT.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x005FE2F0
// measured tier  BYTE_EXACT
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/005fe2f0/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?__as@Palette@@QAEHQAUPalette@@@Z  at 0x005FE2F0  (63 bytes)
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


Palette &Palette::operator=(Palette *a1) {
    char *self = reinterpret_cast<char *>(this);
    memcpy(self, a1, 0x400);

    int *seed = reinterpret_cast<int *>(self + 0x400);
    *seed = 0;
    do {
        *seed = random(0, 0xffff);
    } while (*seed == 0);

    return *this;
}
