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
#include "graphicwin.h"

// Application-region copy of a JACKAL.LIB class member. The image places
// GraphicWin::get_graphic_win at 0x004042B0 among the early game objects,
// not in GraphicWin's library member (src/jackal/graphicwin.cpp,
// 0x005D4CF0..). The original was a header-inline COMDAT. It stays out of
// line in this application unit until the header form is restored.

// ORIGINAL: 0x004042B0 sub_4042b0 0x004042B0-0x004042B3 FILE BYTE_EXACT
// symbol    ?get_graphic_win@GraphicWin@@UAEPAV1@XZ
// Win slot 61 and Buffer slot 1 identify the GraphicWin owning the surface.
// The original self-query is shared by derived windows.
GraphicWin *GraphicWin::get_graphic_win() {
    return this;
}

