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
#include "technology.h"
#include "strings.h"
#include "veh.h"
#include "faction.h"

// Technology-family functions whose image addresses lie OUTSIDE the technology
// TU (0x005B9C10-0x005BEC3D). say_tech(int, BOOL) sits in the faction TU's
// tail and valid_tech_leap in the goals/prototype-design run; both call
// technology entries out of line, so they must not share technology.cpp.
// They stay here until those TUs are regrouped.

/*
Purpose: Get tech string for techID and store it into stringTemp buffer.
// ORIGINAL: 0x005591A0 ?say_tech@@YAXHH@Z 0x005591A0-0x005591BA BYTE_EXACT
// size      26 bytes
// prototype
// callers   2   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005B9C40
Return Value: n/a
Status: Complete
*/
void __cdecl say_tech(int tech_id, BOOL category_lvl) {
    say_tech(StringTemp, tech_id, category_lvl);
}

/*
Purpose: Determine whether a faction is able to jump up the tech tree for the specified tech.
// ORIGINAL: 0x0057CE50 ?valid_tech_leap@@YAHHH@Z 0x0057CE50-0x0057CF9E BYTE_EXACT
// size      334 bytes
// prototype int (__cdecl ?valid_tech_leap@@YAHHH@Z)(int techID, int factionID)
// callers   1   call targets   2
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x0057F0B0 0x005B9F20
Return Value: Is a tech leap possible? true/false
Status: Complete
*/
// TRIED: has_tech_call() (technology.h) exists for exactly this - a real
// out-of-line has_tech() the way best_specialist (base.cpp) needs one - but
// that idiom only works CROSS-TU. has_tech's `MEASURED inline` body lives in
// technology.h, included at the top of THIS file, so it is visible to every
// function below it uniformly; there is no position in technology.cpp where
// valid_tech_leap could sit "before" it the way tech_avail's own marker
// describes (that split predates has_tech's move into the header and no
// longer holds - tech_avail measures MISMATCH here too, not BYTE_EXACT).
// `#pragma inline_depth(0)` around this function has NO effect (measured):
// it limits the RECURSION depth of nested inline expansion, not whether a
// direct, first-level call to an explicitly `inline` function expands, and
// neither does `/Ob0` on this flag set. has_tech_call still reads correctly
// here even though it does not yet reproduce the image's real `call
// 0x5b9f20` at these 4 sites - fixing it needs has_tech's definition moved
// out of the header, which has ~108 other call sites across the tree and is
// out of scope for this body.
//   (STALE: has_tech_call DOES reproduce the image's four `call 0x5b9f20`
//   now - call_diff agrees on every call this body makes.)
// LEVER: the nine failure paths share one out-of-line `return false` because
//   they are `||`-combined conditions (pattern: `if (a || b) return 0;`), not
//   separate guards or a goto; the two `return true;` sites then stay inline.
// LEVER: indexed loops, not pointer locals - `Reactor[i].preq_tech`,
//   `Weapon[i].preq_tech`, `Weapon[i].offense_rating`. The image strength-
//   reduces each into a field pointer and keeps the weapon counter alive
//   (`shl esi, 4; movsx ecx, byte ptr [esi + 0x94ae68]`) only because the
//   body indexes the array again; a `RulesWeapon *weapon` local read the
//   field off the pointer and lost the call/shl/add/movsx order.
BOOL __cdecl valid_tech_leap(int tech_id, int faction_id) {
    if (Technology[tech_id].preq_tech_1 >= 0
        && (!has_tech(Technology[Technology[tech_id].preq_tech_1].preq_tech_1, faction_id)
            || !has_tech(Technology[Technology[tech_id].preq_tech_1].preq_tech_2, faction_id))) {
        return false; // missing a prerequisite of the 1st prerequisite
    }
    if (Technology[tech_id].preq_tech_2 >= 0
        && (!has_tech(Technology[Technology[tech_id].preq_tech_2].preq_tech_1, faction_id)
            || !has_tech(Technology[Technology[tech_id].preq_tech_2].preq_tech_2, faction_id))) {
        return false; // missing a prerequisite of the 2nd prerequisite
    }
    if (Technology[tech_id].preq_tech_1 <= TechDisabled
        || Technology[tech_id].preq_tech_2 <= TechDisabled) {
        return false; // disabled
    }
    for (int i = 0; i < MaxReactorNum; i++) {
        if (tech_id == Reactor[i].preq_tech) {
            return false; // leap not possible for reactor tech
        }
    }
    for (int j = 0; j < MaxWeaponNum; j++) {
        if (Weapon[j].preq_tech == tech_id) { // may end early if 2 weapons have the same preq
            // `ranking` is compared signed in the image (`jg`).
            if ((int)PlayersData[faction_id].ranking > 2 // lowest two ranking factions
                || Weapon[j].offense_rating > weapon_budget(faction_id, 99, false) + 2) {
                return false;
            }
            return true;
        }
    }
    return true;
}
