/*
 * OpenSMACX - an open source clone of Sid Meier's Alpha Centauri.
 * Copyright (C) 2013-2021 Brendan Casey
 *
 * OpenSMACX is free software: you can redistribute it and / or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 */
#pragma once
#include "palette.h"

/*
 * The process palette, at 0x0094C590 in the image. The name is the image's
 * own: its dynamic initialiser `??__Eg_PALETTE1@@YAXXZ` and teardown
 * `??__Fg_PALETTE1@@YAXXZ` were recovered and have since been retired - the
 * CRT owns both halves for the real object (construction before WinMain,
 * atexit destruction at exit). Moving this genuine global changes the source
 * TU that emits those helpers; their exact current symbol attribution remains
 * a coordinator-native obligation, not an assumed identity.
 *
 * AN OBJECT, NOT A `Palette *` TO A FIXED ADDRESS. Both spellings name the
 * same storage while the DLL is injected into the shipped image, but they are
 * not the same C++: `WinMain` passes the palette to `jackal_init_real` as
 * `push 0x94c590`, which is the address of a global, and a `Palette *`
 * variable would compile that same source into `push dword ptr [g_PALETTE1]`
 * - a load the image does not perform. Declaring it as what it is costs
 * nothing and removes an indirection from every future call site.
 */
extern Palette g_PALETTE1;

/* The preferences gamma working palette remains a real Palette object. Its
 * original extent witness is the 0x00491400 PrefWin use. Moving the global
 * also moves compiler-generated startup/atexit contributions; verify their
 * attribution natively rather than inferring it from this source owner.
 */
extern Palette g_PALETTE2;

/*
 * Interface swatches sampled from text.pcx. The walk's low edge at [0] and
 * high edge at [194] retain the original 195-int extent; the mandated row
 * starts at [162] and centered-title consumers read [180]/[181].
 */
extern int TextColourTable[195];
