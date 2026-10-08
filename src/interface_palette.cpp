/*
 * OpenSMACX - an open source clone of Sid Meier's Alpha Centauri.
 * Copyright (C) 2013-2021 Brendan Casey
 *
 * OpenSMACX is free software: you can redistribute it and / or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 */
#include "stdafx.h"
#include "interface_palette.h"
#include "game.h"
#include "buffer.h"
#include "gamma.h"
#include "alpha.h"  // prefs_get

extern float PrefWinGammaCorrection; // prefwin.cpp owns original6846A8

// RECOVERY_OBJECT: v1 0x0094C590 ?g_PALETTE1@@3VPalette@@A
Palette g_PALETTE1;
// RECOVERY_OBJECT: v2 0x009523A0 0x009527F4 ?g_PALETTE2@@3VPalette@@A 0x00491400
Palette g_PALETTE2;  // 0x009523A0, the preferences gamma working palette

// THE INTERFACE TEXT COLOURS. One .bss run the boot fills by sampling
// text.pcx: 0x008C6B5C through 0x008C6E68, 195 ints. The walk writes
// p[-1] before it writes p[0], so the run starts one int below the lowest
// table base the image names (0x008C6B60) and that lower edge is part of it.
// The centred-title writers read [180] and [181], original 0x008C6E2C
// and 0x008C6E30. These are initialized here, not independent Buffer globals.
// The mandate row begins at [162] (original 0x008C6DE4), shared by map and
// PickTech consumers.
// RECOVERY_OBJECT: v1 0x008C6B5C ?TextColourTable@@3PAHA
int TextColourTable[195];

/*
Purpose: Build the interface's text colours: load text.pcx into a scratch
         buffer, publish its palette as the process palette, then sample the
         swatch rows out of it two pixels at a time into TextColourTable.
         Each colour lands as a pair - the pixel and, when that pixel is
         palette index 9 (the transparent one), -1 in its place - and the
         eleven groups cover the run from [1] to the end. The last group
         samples three pixels a row and does no index-9 substitution.
// ORIGINAL: 0x004BA220 ?textcolor_init@@YAXXZ 0x004BA220-0x004BA5B3;0x00659BD0-0x00659BE5 BYTE_EXACT
// size      915 bytes
// prototype void (__cdecl ?textcolor_init@@YAXXZ)(void)
// callers   1   call targets   5
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005D7210 0x005D7410 0x005D7DE0 0x005E2210 0x005FE460
// LEVER: byte-exact, promoted from the src/recovered/units/004ba220.cpp
//   archive unit. The bound is compared as an INT - `(int)p < (int)&...` -
//   because a pointer comparison renders unsigned and VC6 emits `jb` where
//   the image has `jl`. Every loop is a do-while, not a for: the image tests
//   at the bottom with no guard jump in front of the first iteration. And the
//   swatch is one array, not the archive's twelve separate symbols: a real
//   array keeps both ends of every walk as `offset` relocations, where a
//   `static int *const` spelling folds the address into the store and turns
//   `mov [esi-4], eax` into `mov dword ptr [0x8c6b8c], eax`.
// LEVER: the verdict is at /c /O2 /Ob0 /Gy /GR- /Oy- /GX, the one set of the
//   twenty the ladder searches that reproduces the teardown. buffer.h defines
//   `~Buffer()` in the class - MEASURED there, on ??1Buffer's own claim - so
//   the /O2 sets expand it into four teardown instructions, `mov
//   [ebp-0x594], 0` ahead of the `call 0x5d7410` the image makes, and 232/242
//   agreeing is where that leaves an /O2 run. The image's compiler called the
//   destructor out of line; ours, given the same header, does not.
Return Value: n/a
Status: homed on this pass; its forwarder and its archive unit are gone.
*/
void __cdecl textcolor_init() {
    Buffer buf;
    int c;
    int *p;
    int y;

    buf.load_pcx("text.pcx", 0, 0xa, 0xec);
    g_PALETTE1.set();

    y = 0x15;
    p = &TextColourTable[13];
    do {
        c = buf.get_pixel(8, y);
        p[-1] = c;
        c = buf.get_pixel(0x12, y);
        p[0] = c;
        if (c == 9) {
            p[0] = -1;
        }
        p += 2;
        y += 0x10;
    } while ((int)p < (int)&TextColourTable[31]);

    y = 0x24;
    p = &TextColourTable[83];
    do {
        c = buf.get_pixel(0xcd, y);
        p[-1] = c;
        c = buf.get_pixel(0xd7, y);
        p[0] = c;
        if (c == 9) {
            p[0] = -1;
        }
        p += 2;
        y += 0x10;
    } while ((int)p < (int)&TextColourTable[141]);

    y = 0x24;
    p = &TextColourTable[141];
    do {
        c = buf.get_pixel(0x173, y);
        p[-1] = c;
        c = buf.get_pixel(0x17d, y);
        p[0] = c;
        if (c == 9) {
            p[0] = -1;
        }
        p += 2;
        y += 0x10;
    } while ((int)p < (int)&TextColourTable[163]);

    y = 0x24;
    p = &TextColourTable[181];
    do {
        c = buf.get_pixel(0x234, y);
        p[-1] = c;
        c = buf.get_pixel(0x23e, y);
        p[0] = c;
        if (c == 9) {
            p[0] = -1;
        }
        p += 2;
        y += 0x10;
    } while ((int)p < (int)&TextColourTable[195]);

    y = 0xb0;
    p = &TextColourTable[69];
    do {
        c = buf.get_pixel(0x232, y);
        p[-1] = c;
        c = buf.get_pixel(0x23c, y);
        p[0] = c;
        if (c == 9) {
            p[0] = -1;
        }
        p += 2;
        y += 0x10;
    } while ((int)p < (int)&TextColourTable[83]);

    y = 0x25;
    p = &TextColourTable[163];
    do {
        c = buf.get_pixel(0x2ce, y);
        p[-1] = c;
        c = buf.get_pixel(0x2d8, y);
        p[0] = c;
        if (c == 9) {
            p[0] = -1;
        }
        p += 2;
        y += 0x10;
    } while ((int)p < (int)&TextColourTable[171]);

    y = 0xbe;
    p = &TextColourTable[31];
    do {
        c = buf.get_pixel(7, y);
        p[-1] = c;
        c = buf.get_pixel(0x11, y);
        p[0] = c;
        if (c == 9) {
            p[0] = -1;
        }
        p += 2;
        y += 0x10;
    } while ((int)p < (int)&TextColourTable[39]);

    y = 0x126;
    p = &TextColourTable[39];
    do {
        c = buf.get_pixel(6, y);
        p[-1] = c;
        c = buf.get_pixel(0x10, y);
        p[0] = c;
        if (c == 9) {
            p[0] = -1;
        }
        p += 2;
        y += 0x10;
    } while ((int)p < (int)&TextColourTable[51]);

    y = 0x11e;
    p = &TextColourTable[171];
    do {
        c = buf.get_pixel(0x175, y);
        p[-1] = c;
        c = buf.get_pixel(0x17f, y);
        p[0] = c;
        if (c == 9) {
            p[0] = -1;
        }
        p += 2;
        y += 0x10;
    } while ((int)p < (int)&TextColourTable[181]);

    y = 0x19e;
    p = &TextColourTable[1];
    do {
        c = buf.get_pixel(0x175, y);
        p[-1] = c;
        c = buf.get_pixel(0x17f, y);
        p[0] = c;
        if (c == 9) {
            p[0] = -1;
        }
        p += 2;
        y += 0x10;
    } while ((int)p < (int)&TextColourTable[13]);

    y = 0x1a4;
    p = &TextColourTable[51];
    do {
        c = buf.get_pixel(8, y);
        p[-1] = c;
        c = buf.get_pixel(0x12, y);
        p[0] = c;
        c = buf.get_pixel(0x1c, y);
        p[1] = c;
        p += 3;
        y += 0x10;
    } while ((int)p < (int)&TextColourTable[69]);
}

// ORIGINAL: 0x00451A10 ?load_palette@@YAXXZ 0x00451A10-0x00451C44;0x00655042-0x006550F9
// RECOVERY_BINDING: v1 {"evidence": ["historical seed:src/recovered/units/00451a10.cpp", "task:asset-startup-loader-owner", "Native Buffer then Gamma lifetime; prefs key/default100; float operand66C540(0.01f) and existing PrefWinGammaCorrection6846A8; Palette1 PCX range10/236, Palette2 copy direction and Gamma apply1. Separate VGA port helper5D4240 is not this loader or a proved equivalent device path."], "reason": "Recover the complete asset loader through source-owned C++ objects, arrays and normal lifecycle while retaining original mappings and historical findings; no asset/runtime acceptance claimed."}
void __cdecl load_palette() {
    Buffer buffer;
    Gamma gamma;
    int gamma_percent;
    float gamma_value;

    gamma_percent = prefs_get("Gamma Correction", 100, 0);
    gamma_value = static_cast<float>(gamma_percent) * 0.01f;
    PrefWinGammaCorrection = gamma_value;

    buffer.load_pcx("PALETTE.PCX", &g_PALETTE1, 0x0A, 0xEC);
    g_PALETTE2.copy_from(&g_PALETTE1);
    gamma.exec(&g_PALETTE1, gamma_value, 1);
}
