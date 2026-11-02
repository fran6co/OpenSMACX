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

#include <stdlib.h>

// These wrappers use the CRT stream, not the separate Random/Rand LCG.
void __cdecl my_srand(int reseed);
unsigned int __cdecl rnd(int bounds, char *input);

inline unsigned int __cdecl bounded_crt_random(int bounds) {
    if (bounds - 1 <= 0) {
        return 0;
    }
    return rand() % bounds;
}
