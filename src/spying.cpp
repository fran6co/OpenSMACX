/*
 * OpenSMACX - an open source clone of Sid Meier's Alpha Centauri.
 * Copyright (C) 2013-2021 Brendan Casey
 *
 * OpenSMACX is free software: you can redistribute it and / or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 */
#include "stdafx.h"
#include "spying_recovery.h"
#include "faction.h"  // Players, RFLAG_ALIEN
#include "console.h"
#include "base.h"  // GovernorFaction is the same Council-owned identity.

// Historical SpyingBaseIndex interpreted 0x009A6524 as an independent
// scalar. It is SecretProject.empath_guild (+0x10 in the table at
// 0x009A6514); project construction/removal and setup_game own its writes.

/*
Purpose: Determine whether the current faction has intelligence visibility of
         the given subject faction.
// ORIGINAL: 0x0055BC00 ?spying@@YAHH@Z 0x0055BC00-0x0055BC7C BYTE_EXACT
// LEVER: ONE `||` CHAIN IN A `return`, not three early-return `if`s. The image reaches its `mov eax, 1` epilogue from three different jumps and its `xor eax, eax` epilogue from two - shared tails, which is the opposite of the usual split-the-guards lever and is what a single `return A || B || C;` produces. Separate `if (...) return 1;` statements emit an inline epilogue after each test: 9 of 44, similarity 0.632. The chain alone took it to 0.860.
// LEVER: the MIDDLE operand is a TERNARY, not an `&&`. `base >= 0 && byte == faction` short-circuits with a plain `je`, where the image materialises the comparison - `xor eax, eax / cmp edx, ecx / sete al / test eax, eax / jne` - which is what `(base >= 0 ? (byte == faction) : 0) != 0` emits. That was the last three instructions: 42 compiled against 44, then 44 of 44. `!!(...)`, `static_cast<int>(...) != 0`, an extra `(...) != 0` and the comparison written the other way round all stay at 42.
// size      124 bytes
// prototype 
// callers   19   call targets   0
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     (none)
// notes     Runtime redirect installed by DllMain after byte-signature validation
Status: Complete

*/
int __cdecl spying(int subject) {
    const int faction = ConsoleGlobal.faction_id_;
    return (PlayersData[faction].diplo_treaties[subject]
            & 0x1000U) != 0
           || (SecretProject.empath_guild >= 0
               ? (Bases[SecretProject.empath_guild].faction_id_current == faction)
               : 0) != 0
           || (faction == GovernorFaction
               && (Players[static_cast<size_t>(subject)].rule_flags &
                   RFLAG_ALIEN) == 0);
}
