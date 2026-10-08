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
#pragma once
#include <windows.h>

class Palette;
class Font;

BOOL __cdecl jackal_version_check(LPCSTR version);
// The Jackal engine's bring-up and teardown, called once each from WinMain.
// `jackal_init_real` returns ZERO on success - WinMain's `test eax, eax` at
// 0x0045FA50 jumps INTO the game when the result is zero and returns 0 from
// WinMain when it is not, which is the opposite sense of every neighbouring
// call and the one thing the first transcription of that body got backwards.
// `LPSTR` rather than `LPCSTR` for the window title because the image's own
// name for it - ?jackal_init_real@@YAHPAUPalette@@PAUFont@@PADHHHH@Z - spells
// that parameter `PAD`, and the mangling is what the byte match will look the
// body up by when 0x0062D3A0 lands.
int __cdecl jackal_init_real(Palette *palette, Font *font,
                                       LPSTR window_name, int tgl_direct_draw,
                                       int display_width, int display_height,
                                       int colour_depth);
void __cdecl jackal_close();

// 0x009BC4B0. jackal_init_real stores its `tgl_direct_draw` argument here on
// entry and sets bit 0 once every subsystem has come up, so it is the engine's
// own state word: the mode it was asked for, plus "initialised".
extern int JackalInitFlags;
