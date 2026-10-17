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
#include "crt_random.h"
#include "log.h"

/*
Purpose: Initialize the pseudo-random number generator.
// ORIGINAL: 0x00538FB0 ?my_srand@@YAHH@Z 0x00538FB0-0x00538FD2 BYTE_EXACT
// symbol    ?my_srand@@YAXH@Z
// size      34 bytes
// prototype int (__cdecl ?my_srand@@YAHH@Z)(int reseed)
// callers   6   call targets   2
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x006262F0 0x00646010
Return Value: n/a
Status: Complete
*/
void __cdecl my_srand(int reseed) {
    // ONE `srand`, not two. The image's `add esp, 0x14` accounts for exactly
    // log_say's four arguments and one seed; ours was `add esp, 0x18`, four
    // bytes and one `push`/`call` more. Seeding twice with the same value is
    // the same as seeding once, so nothing about the game changes.
    log_say("Reseed to", reseed, 0, 0);
    srand(reseed);
}

/*
Purpose: Calculate a random value within the provided bounds. The unused 2nd parameter was possibly 
         meant to have the random value append to it.
// ORIGINAL: 0x00579770 ?rnd@@YAHHPAD@Z 0x00579770-0x00579790 BYTE_EXACT
// LEVER: ternary flipped to a guard-clause `if (bounds - 1 <= 0) return 0;`
//        then fall through to `return rand() % bounds;` - the image jumps
//        TO the rand() work and falls through to the early return.
// symbol    ?rnd@@YAIHPAD@Z
// size      32 bytes
// prototype int (__cdecl ?rnd@@YAHHPAD@Z)(int seed, int8*)
// callers   2   call targets   1
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x0064601D
Return Value: Bounded random value
Status: Complete
*/
uint32_t __cdecl rnd(int bounds, LPSTR UNUSED(input)) {
    if (bounds - 1 <= 0) {
        return 0;
    }
    return rand() % bounds;
}

