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

// These wrappers use the CRT stream, not the separate Random/Rand LCG.
void __cdecl my_srand(int reseed);
#include <stdlib.h>

// rnd is a header inline: its out-of-line copy 0x00579770 heads the run of
// header-inline copies in enemy_move's unit (x_dist, site_at, is_known,
// base_who, ...), the image calls it only from the budget-exhausted giants
// (enemy_move 5, probe 9), and everywhere else it expands to `rand() % bounds`
// (the constant-bound guard folds away). The label argument is unused; the
// labels of expanded sites stay in the image's data as unreferenced literals.
MEASURED inline unsigned int __cdecl rnd(int bounds, char *input) {   // 00579770
    if (bounds - 1 <= 0) {
        return 0;
    }
    int value = rand() % bounds;
    return value;
}
