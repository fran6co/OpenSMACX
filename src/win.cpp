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
#include "win.h"

/*
Purpose: Determine whether a point is inside a rectangle using Win32 edge semantics.
Original Offset: 005FA7E0
Return Value: Is the point inside the rectangle? true/false
Status: Complete
*/
BOOL __cdecl in_box(int x, int y, const RECT *rect) {
    return x >= rect->left && x < rect->right && y >= rect->top && y < rect->bottom;
}
