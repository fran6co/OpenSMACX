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
#include "sprite.h"

 /*
  * Popup dialog helper
  *
  * A four-argument convenience wrapper over the nine-argument popup builder,
  * supplying a shared caption buffer, the two defaulted flags and the two
  * placeholders the short form does not expose.
  */
typedef int (__cdecl func_x_pops_full)(char *, const char *, int, char *, int,
                                       Sprite *, int, int, int (__cdecl *)());
extern func_x_pops_full *XPopsOriginalFull;
extern char *XPopsCaptionBuffer;

DLLEXPORT int __cdecl x_pops_short(const char *label, int value, Sprite *sprite,
                                   int (__cdecl *callback)());
