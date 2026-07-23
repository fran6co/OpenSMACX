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
#include "xpops.h"

func_x_pops_full *XPopsOriginalFull = (func_x_pops_full *)0x005BF930;
char *XPopsCaptionBuffer = (char *)0x009B8AA8;

/*
Purpose: Raise a popup with the defaults the short form implies - the shared
         caption buffer, no title, no override text, and both flags set.
Original Offset: 005BF7D0
Return Value: whatever the full form returns
Status: Complete
*/
int __cdecl x_pops_short(const char *label, int value, Sprite *sprite,
                         int (__cdecl *callback)()) {
    return XPopsOriginalFull(XPopsCaptionBuffer, label, -1, nullptr, value,
                             sprite, 1, 1, callback);
}

/*
Purpose: The three-argument popup form: like the short form but with the value
         defaulted to zero and no override text.
Original Offset: 005BF5D0
Return Value: whatever the full form returns
Status: Complete
*/
int __cdecl x_pops_minimal(const char *label, Sprite *sprite,
                           int (__cdecl *callback)()) {
    return XPopsOriginalFull(XPopsCaptionBuffer, label, -1, nullptr, 0,
                             sprite, 1, 1, callback);
}
