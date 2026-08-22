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
#include "temp.h"
#include "council.h"
#include "base.h"


/*
Purpose: Calculate faction's vote count. Used for Planetary Governor and Supreme Leader.
// ORIGINAL: 0x0052AD30 ?council_votes@@YAHH@Z 0x0052AD30-0x0052AE19 BYTE_EXACT
// LEVER: `Base *base = Bases; ...; base++` (walk, not index) for the
// population sum - `Bases[i]` plateaus. `votes` is a signed `int`, not
// `uint32_t` - the +50% arm needs the image's `cdq/sub/sar` signed-divide
// fixup, which an unsigned local turns into a plain `shr`. Each
// `has_project(...)` result is stored to a named `BOOL` before the `if`,
// matching the image's `sete`/`test`/`je` rather than a folded `cmp`/`jne`.
// The bonus loop walks `faction_bonus_val1` and reads
// `faction_bonus_id[i]` as `bonus_val1[-8]` (8 ints back, same registers
// the image keeps) rather than indexing both arrays separately. And that
// pointer is computed inside `if (bonus_count > 0) { ... }`, not
// unconditionally before the loop - without the guard VC6 schedules the
// `lea` ahead of the image's own `test`/`jle`.
// size      233 bytes
// prototype int (__cdecl ?council_votes@@YAHH@Z)(int factionID)
// callers   9   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
Return Value: Faction vote count
Status: Complete
*/
int __cdecl council_votes(int faction_id) {
    if (is_alien_faction(faction_id)) {
        return 0;
    }
    int votes = 0;
    Base *base = Bases;
    for (int i = 0; i < BaseCurrentCount; i++, base++) {
        if (base->faction_id_current == faction_id) {
            votes += base->population_size;
        }
    }
    BOOL has_empath_guild = has_project(SP_EMPATH_GUILD, faction_id);
    if (has_empath_guild) {
        votes += votes / 2; // +50% votes
    }
    BOOL has_clinical_immortality = has_project(SP_CLINICAL_IMMORTALITY, faction_id);
    if (has_clinical_immortality) {
        votes *= 2; // Doubles votes
    }
    int bonus_count = Players[faction_id].faction_bonus_count;
    if (bonus_count > 0) {
        int *bonus_val1 = Players[faction_id].faction_bonus_val1;
        for (i = 0; i < bonus_count; i++, bonus_val1++) {
            if (bonus_val1[-8] == RULE_VOTES) { // faction_bonus_id[i], 8 ints back
                int votes_bonus = *bonus_val1;
                if (votes_bonus >= 0) {
                    votes *= votes_bonus; // Peacekeeper bonus
                }
            }
        }
    }
    return votes;
}

/*
Purpose: Check whether a faction's leader is eligible to be a Planetary Governor candidate.
// ORIGINAL: 0x0052AE20 ?eligible@@YAHH@Z 0x0052AE20-0x0052AEA4 SEMANTIC
// LEVER: `faction_count` declared and zeroed BEFORE the `is_alien_faction`
// guard (the image's `mov [ebp-4],0` runs ahead of the alien test even
// though only the non-alien path reads it). `int`, not `uint32_t`, for
// `faction_count`/`votes`/the loop `i` - the image's `jle`/`jl`/`setl`
// tail is signed, an unsigned local turns it into `jbe`/`jb`/`sbb;neg`.
// TRIED: reordering is_alive's `FactionsStatus[1] & (1 << faction_id)` -
// no effect. Residual (53/54, SEMANTIC): `is_alive`'s inlined `test`
// encodes its two operands in the image's order (`test dl, al`) vs ours
// (`test al, dl`) - same flags, different byte, not chased further.
// size      132 bytes
// prototype 
// callers   6   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x0052AD30
Return Value: Is the leader eligible (top two vote totals)? true/false
Status: Complete
*/
BOOL __cdecl eligible(int faction_id) {
    int faction_count = 0;
    if (is_alien_faction(faction_id)) {
        return false;
    }
    int votes = council_votes(faction_id);
    for (int i = 1; i < MaxPlayerNum; i++) {
        if (faction_id != i && is_alive(i) && council_votes(i) > votes) {
            faction_count++;
        }
    }
    return faction_count < 2;
}
