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
#include "mapwin.h"

// Original4712D0..471316, historically claimed exact as mapwin_unk1.
// The adapter itself is genuine code, with a nullable derived-to-base cast;
// its gen_terrain_poly dependency remains the retained renderer obligation.
// ORIGINAL: 0x004712D0 mapwin_unk1 0x004712D0-0x00471316 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Original4712D0..471316; retired seed git:5605e9af:src/recovered/units/004712d0.cpp records this identity and exact span, including its BYTE_EXACT obligation and historical ABI notes.", "Original conditional receiver conversion reads the GraphicWin virtual-base displacement and adds the Buffer subobject offset. C++ base conversion supplies the same null handling without manual vbtable arithmetic."], "reason": "This nine-argument cdecl adapter performs a nullable genuine MapWin-to-Buffer conversion before entering the common ten-argument renderer. Preserve its original mapwin_unk1 identity and historical seed obligation; gen_window_terrain describes the recovered source role."}
// symbol    ?gen_window_terrain@@YAHPAVMapWin@@HHHHHHHH@Z
int __cdecl gen_window_terrain(MapWin *window, int zoom, int faction,
                              int base_id, int flags, int x, int y,
                              int pixel_x, int pixel_y) {
    return gen_terrain_poly(static_cast<Buffer *>(window), window,
                            zoom, faction, base_id, flags,
                            x, y, pixel_x, pixel_y);
}
