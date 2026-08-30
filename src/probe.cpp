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
#include "probe.h"
#include "base.h"
#include "game.h"
#include "veh.h"
#include "strings.h"
#include "map.h"

extern uint32_t LockEnableMask;  // 0x009A64E8, defined in lock.cpp

BOOL ProbeHasAlgoEnhancement;  // 0x00945B30
uint32_t ProbeTargetFactionID;  // 0x00945B34
BOOL ProbeTargetHasHSA;  // 0x00945B38

/*
Purpose: Calculate the amount of energy that can be stolen from a base based on its population.
// ORIGINAL: 0x0050C4B0 ?steal_energy@@YAHH@Z 0x0050C4B0-0x0050C504 BYTE_EXACT
// size      84 bytes
// prototype int (__cdecl ?steal_energy@@YAHH@Z)(int baseID)
// callers   1   call targets   0
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     (none)
Return Value: Energy
Status: Complete
*/
int __cdecl steal_energy(int base_id) {
    uint32_t faction_id = Bases[base_id].faction_id_current;
    int energy = PlayersData[faction_id].energy_reserves;
    return (energy <= 0) ? 0 
        : ((energy * Bases[base_id].population_size) / (PlayersData[faction_id].pop_total + 1));
}

// An out-of-line `vector_dist` for mind_control's one call site. The lesson
// this was measured from is the LEVER line on the 0x0059EA80 marker below,
// which is the only place `decomp.reader` looks.
#pragma auto_inline(off)
int __cdecl vector_dist_call(int x_distance, int y_distance) {
    return vector_dist(x_distance, y_distance);
}
#pragma auto_inline(on)

/*
Purpose: Calculate the cost for the faction to be able to mind control the specified base. The 3rd
         parameter determines if this cost is for cornering the market (true) or via probe (false).
// ORIGINAL: 0x0059EA80 ?mind_control@@YAHHHH@Z 0x0059EA80-0x0059EE42
// LEVER: `#pragma auto_inline(off)` AROUND A PLAIN FORWARDER IN THIS FILE is how
//   a `MEASURED inline` helper is called out of line at ONE site. VC6 6.0 has no
//   `__declspec(noinline)` and the tree had recorded that as a hard ceiling; the
//   pragma does the same job, and it needs nothing outside the caller's own
//   translation unit. `map.h` marks `vector_dist` `MEASURED inline` because the
//   image both CALLS it (0x004F8090) and writes it out at 0x005A5910 - and it
//   CALLS it here, at 0x0059EB60. MEASURED: the direct-call multiset went from 13
//   (nine game calls plus FOUR `_abs`, because the inlined vector_dist brought its
//   own two) to the image's exact 12 - three `bitmask`, `has_fac`, three
//   `stack_check`, `stack_fix`, `veh_at`, `vector_dist`, two `_abs`. Similarity
//   0.212 -> 0.244, compiled length 301 -> 315 against an image of 350. The E8's
//   target is a relocation on both sides, so the forwarder's own name costs
//   nothing. The tree's existing way of forcing an E8 - declare in a header,
//   define in a .cpp, as `bitmask_call` / `bit_set_call` / `sleep_call` do - is
//   the better home for this one, but it needs `map.h` and `map.cpp`.
// LEVER: /Oi- IS THE FLAG SET THIS BODY WANTS, and the call list is what says so
//   rather than the score: the image calls `_abs` out of line twice
//   (0x00644F3A at 0x0059EB25 and 0x0059EB5B), which no /Oi build can produce.
//   It also needs /Ob2, not /Ob0 - the image INLINES `x_dist` and
//   `has_fac_built` (calling `bitmask` directly), and under /Ob0 this tree calls
//   both by name. There is no /O2 /Ob0 /Oi- set, so no single set answers both.
// TRIED: declaration order of `target_faction_id` / `target_x` / `target_y`.
//   The image loads y, then faction, then x, which reads like a source order; all
//   three permutations score exactly 22/350 and 0.244, so VC6 schedules those
//   loads itself and the order in the source says nothing.
// TRIED: the remaining gap is 35 instructions and it is one block, not spread
//   out. `listing_diff` aligns everything up to image instruction 80 and
//   everything from 286 on, and cannot align the 199 image instructions between -
//   the three inlined `has_fac_built` bit tests, the four halvings, the
//   stack_check arithmetic and the corner-market arm. That is a body-shape
//   question that wants its own pass, not another spelling of the preamble.
// size      962 bytes
// prototype int (__cdecl ?mind_control@@YAHHHH@Z)(int baseID, int factionID, BOOL isCornerMarket)
// callers   2   call targets   7
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x00421670 0x004F8090 0x0050BA00 0x005B8E10 0x005B9580 0x005BFE90 0x00644F3A
// LEVER: the image never calls `vulnerable()` (0x0059E980) here at all - it
//        is hand-inlined into mind_control, and at THIS call site the
//        compiler keeps `has_fac(FAC_HEADQUARTERS, i, 0)` (0x00421670) and
//        `vector_dist(dx, dy)` (0x004F8090) as REAL calls, unlike
//        vulnerable()'s own standalone body (which folds has_fac_built's
//        bitmask() and open-codes vector_dist's abs/largest/smallest math
//        instead - see that function's own LEVER note). Hand-inlining the
//        loop here, using has_fac (not has_fac_built) for the HQ check and
//        the real 2-arg vector_dist(dx, dy) call, is what reaches the
//        image's has_fac/abs/abs/vector_dist call sequence; a plain call to
//        vulnerable() never appears in the image's call list for this
//        function under any flag set. `is_human`/`has_treaty` are likewise
//        hand-inlined at each use - both are already header `inline`, but
//        VC6 declines to fold them into a function this large, so a plain
//        call to either stood where the image has none.
Return Value: Mind control cost
Status: Complete
*/
int __cdecl mind_control(int base_id, int faction_id, BOOL is_corner_market) {
    uint32_t target_faction_id = Bases[base_id].faction_id_current;
    int target_x = Bases[base_id].x;
    int target_y = Bases[base_id].y;
    int cost = 12; // vulnerable(): default when faction has no HQ
    for (int i = 0; i < BaseCurrentCount; i++) {
        if (Bases[i].faction_id_current == target_faction_id && has_fac(FAC_HEADQUARTERS, i, 0)) {
            int dx = x_dist(target_x, Bases[i].x);
            int dy = abs(target_y - Bases[i].y);
            cost = vector_dist_call(dx, dy);
            break;
        }
    }
    if (cost <= 0) {
        if (!is_corner_market) {
            return -1;
        }
        cost = 1;
    }
    if (has_fac_built_call(FAC_GENEJACK_FACTORY, base_id)) {
        cost *= 2;
    }
    if (has_fac_built_call(FAC_CHILDREN_CRECHE, base_id)) {
        cost /= 2; // Reduces base's vulnerability to enemy mind control
    }
    if (has_fac_built_call(FAC_PUNISHMENT_SPHERE, base_id)) {
        cost /= 2;
    }
    if (Bases[base_id].nerve_staple_turns_left) {
        cost /= 2;
    }
    int veh_id = stack_fix(veh_at(target_x, target_y));
    cost = ((stack_check(veh_id, 2, PLAN_COMBAT, -1, -1)
        + stack_check(veh_id, 2, PLAN_OFFENSIVE, -1, -1))
        * (stack_check(veh_id, 6, ABL_POLY_ENCRYPTION, -1, -1) + 1)
        + PlayersData[faction_id].mind_control_total / 4 + Bases[base_id].population_size)
        * ((PlayersData[target_faction_id].corner_market_active
            + PlayersData[target_faction_id].energy_reserves + 1200) / (cost + 4));
    if (!((LockEnableMask & 0xFF) & (1 << faction_id)) && ((LockEnableMask & 0xFF) & (1 << target_faction_id))) {
        int diff = PlayersData[target_faction_id].diff_level;
        if (diff > DLVL_LIBRARIAN) {
            cost = (cost * 3) / diff;
        }
    }
    BOOL has_pact = PlayersData[faction_id].diplo_treaties[target_faction_id] & DTREATY_PACT;
    if (is_corner_market) {
        if (has_pact) {
            cost /= 2;
        }
        if (PlayersData[faction_id].diplo_treaties[target_faction_id] & DTREATY_TREATY) {
            cost /= 2;
        }
        int tech_comm_target = PlayersData[target_faction_id].tech_commerce_bonus;
        tech_comm_target *= tech_comm_target;
        int tech_comm_probe = PlayersData[faction_id].tech_commerce_bonus;
        tech_comm_probe *= tech_comm_probe;
        cost = (cost * (tech_comm_target + 1)) / (tech_comm_probe + 1);
    } else if (has_pact) {
        cost *= 2;
    }
    if (Bases[base_id].faction_id_former == faction_id) {
        cost /= 2;
    }
    uint32_t base_state = Bases[base_id].state;
    if (base_state & BSTATE_DRONE_RIOTS_ACTIVE) {
        cost /= 2;
    }
    if (base_state & BSTATE_GOLDEN_AGE_ACTIVE) {
        cost *= 2;
    }
    if (PlayersData[target_faction_id].diplo_treaties[faction_id] & DTREATY_ATROCITY_VICTIM) {
        cost *= 2;
    } else if (PlayersData[target_faction_id].diplo_treaties[faction_id] & DTREATY_WANT_REVENGE) {
        cost += cost / 2;
    }
    return cost;
}

/*
Purpose: Calculate the success and survival rates for a probe action based on the probe's morale and
         the difficulty of the action. These are used to generate a chances probability string for
         provided id. A baseID is an optional parameter to factor in its probe defenses.
// ORIGINAL: 0x0059EEE0 ?success_rates@@YAHHHHH@Z 0x0059EEE0-0x0059F11E
// size      574 bytes
// prototype int (__cdecl ?success_rates@@YAHHHHH@Z)(uint32_t id, uint32_t morale, uint32_t diffModifier, int baseID)
// callers   1   call targets   4
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x0050BA00 0x00625EC0 0x00645470 0x0064FC88
Return Value: Success rate of probe
Status: Complete
// LEVER: /Oy- - the doc's own `frame` tag says the image keeps a real ebp
//   frame, and it also earns the `lea eax,[esi+1]; test eax,eax` idiom for
//   `base_id != -1`; spelling that comparison as `(base_id + 1) != 0`
//   instead of `base_id != -1` reproduces it (both prb_defense sites).
//   `StringTemp[0] = '\0'` scheduled BEFORE the `morale < 1` guard (both
//   unconditional; the image hoists the clear first) also helped.
// TRIED: 23/208 (0.916 similar), plateaued at this size. The remaining gap
//   is the `has_fac_built`-shaped bit test in the block that reads
//   `soc_effect_active.probe` - the image indexes a table with a `lea`
//   chain (`esi+esi*8`, `esi+edx*2`, `esi+eax*4`) this tree's equivalent
//   expression does not reproduce, plus pervasive esi/edi register-role
//   swaps through the rest of the body. This is a body-shape question for
//   its own pass, not another spelling of the preamble - see the
//   `mind_control` LEVER note just above this function for the same shape
//   of ceiling.
*/
int __cdecl success_rates(int id, int morale, int diff_modifier, int base_id) {
    // STRINGTEMP CLEARED BEFORE THE MORALE GUARD. The image's
    // `mov byte ptr [StringTemp], 0` is scheduled ahead of the `cmp ebx, 1`
    // that guards `morale = 1` - both are unconditional, but the image
    // hoists the string clear first.
    StringTemp[0] = '\0';
    if (morale < 1) {
        morale = 1;
    }
    int success_rate;
    if (diff_modifier < 0) {
        strcat(StringTemp, "100%");
        success_rate = diff_modifier;
    } else {
        // The image evaluates prb_defense twice, once for failure_rate and again
        // for loss_rate - it never caches it in a shared local, and each use
        // makes its own real has_fac_built() call (via has_fac_built_call's
        // bitmask() E8, matching the image's two `call 0x50ba00` sites).
        int prb_defense = ((base_id + 1) != 0 && has_fac_built_call(FAC_COVERT_OPS_CENTER, base_id))
            ? 2 : 0;
        prb_defense = range(PlayersData[ProbeTargetFactionID].soc_effect_active.probe
            + prb_defense, -2, 0);
        int failure_rate = (diff_modifier * 100) / ((morale / 2) - prb_defense + 1);
        if (ProbeHasAlgoEnhancement && !ProbeTargetHasHSA) {
            failure_rate /= 2; // Algo Ench: failure cut in half when acting against normal targets
        }
        success_rate = 100 - failure_rate;
        if (ProbeTargetHasHSA) {
            success_rate /= 2; // Chance of success is half what the chance would have been w/o HSA
        }
        say_num(success_rate);
        strcat(StringTemp, "%");

        int prb_defense_2 = ((base_id + 1) != 0 && has_fac_built_call(FAC_COVERT_OPS_CENTER, base_id))
            ? 2 : 0;
        prb_defense_2 = range(PlayersData[ProbeTargetFactionID].soc_effect_active.probe
            + prb_defense_2, -2, 0);
        int loss_rate = ((diff_modifier + 1) * 100) / (morale - prb_defense_2);
        if (ProbeHasAlgoEnhancement && !ProbeTargetHasHSA) {
            loss_rate /= 2;
        }
        int survival_rate = 100 - loss_rate;
        if (ProbeTargetHasHSA) {
            // BUG IN THE ORIGINAL: this hits success_rate a second time instead
            // of halving survival_rate - the image's ProbeTargetHasHSA branch
            // here divides edi (still holding success_rate) rather than esi
            // (survival_rate). Reproduced deliberately; a "fixed" version that
            // divides survival_rate here does not match the shipped bytes.
            success_rate /= 2;
        }
        if (survival_rate != success_rate) {
            strcat(StringTemp, ", ");
            say_num(survival_rate);
            strcat(StringTemp, "%");
        }
    }
    parse_says(id, StringTemp, -1, -1);
    return success_rate;
}
