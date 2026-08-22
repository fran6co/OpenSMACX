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

int __fastcall palette_get_rgbquad_redirect(
        Palette *self, void *, RGBQUAD *output, int start, int count) {
    return self->get_rgbquad(output, start, count);
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

void __cdecl palette_set_active_window_redirect(Win *window) {
    Palette::set_active_window(window);
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

int __fastcall palette_get_pos_redirect(Palette *self, void *, int value) {
    return self->get_pos(value);
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
// RULED-OUT: source-form search this body is a REGISTER-PRESSURE plateau, not a
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
// ORIGINAL: 0x005FE8B0 ?start_cycle@Palette@@QAEHH@Z 0x005FE8B0-0x005FE8FB
// size      75 bytes
// prototype int (__thiscall ?start_cycle@Palette@@QAEHH@Z)(Palette* this, int)
// callers   1   call targets   1
// kind      game
// calls     0x00616650
Return Value: No errors (0); no slot holds that key (0xB)
Status: Semantics transcribed from the image

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
    const PaletteInternal *entry = internal_;
    int slot = 0;
    do {
        // Read once, compared twice: `mov edx, [esi]` then two `cmp`s
        // against the register, not two memory compares.
        const int held = static_cast<int>(entry->key);
        if (key == held || held == -1) {
            break;
        }
        ++slot;
        ++entry;
    } while (slot < 5);
    if (static_cast<int>(internal_[slot].key) != key) {
        return 0xB;
    }
    internal_[slot].time->start();
    return 0;
}
