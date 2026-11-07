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
#include "crt_random_inline.h"
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
// Pinned arguments identify these NUL-terminated keys in the original;
// the literal spans cover their text and terminators, not surrounding storage.
// RECOVERY_LITERAL: v1 0x0068D430 0x0068D43A ??_C@_09BLHC@Reseed?5to?$AA@ 0x00538FB0
void __cdecl my_srand(int reseed) {
    // ONE `srand`, not two. The image's `add esp, 0x14` accounts for exactly
    // log_say's four arguments and one seed; ours was `add esp, 0x18`, four
    // bytes and one `push`/`call` more. Seeding twice with the same value is
    // the same as seeding once, so nothing about the game changes.
    log_say("Reseed to", reseed, 0, 0);
    srand(reseed);
}


