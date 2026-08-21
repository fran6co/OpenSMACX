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

/*
Purpose: Calculate the cost for the faction to be able to mind control the specified base. The 3rd
         parameter determines if this cost is for cornering the market (true) or via probe (false).
// ORIGINAL: 0x0059EA80 ?mind_control@@YAHHHH@Z 0x0059EA80-0x0059EE42
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
            cost = vector_dist(dx, dy);
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
    if (!(FactionsStatus[0] & (1 << faction_id)) && (FactionsStatus[0] & (1 << target_faction_id))) {
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
*/
int __cdecl success_rates(int id, int morale, int diff_modifier, int base_id) {
    if (morale < 1) {
        morale = 1;
    }
    StringTemp[0] = '\0';
    int success_rate;
    if (diff_modifier < 0) {
        strcat(StringTemp, "100%");
        success_rate = diff_modifier;
    } else {
        // The image evaluates prb_defense twice, once for failure_rate and again
        // for loss_rate - it never caches it in a shared local, and each use
        // makes its own real has_fac_built() call (via has_fac_built_call's
        // bitmask() E8, matching the image's two `call 0x50ba00` sites).
        int prb_defense = (base_id != -1 && has_fac_built_call(FAC_COVERT_OPS_CENTER, base_id))
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

        int prb_defense_2 = (base_id != -1 && has_fac_built_call(FAC_COVERT_OPS_CENTER, base_id))
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
